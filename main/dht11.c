//
// dht11.c:
//  a program to read from dht11 temperature sensor.
//
// Author:
//  Abdullah Ada (https://github.com/0xAbby) 30-Dec-2023
//
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"

#define PIN 2
#define DHT11_OK 0
#define DHT11_TIMEOUT_ERROR -1
#define DHT11_CRC_ERROR -2

static int countTicks(uint16_t microSeconds, int level) {
    int micros_ticks = 0;

    while(gpio_get_level(PIN) == level) { 
        if(micros_ticks++ > microSeconds) 
            return DHT11_TIMEOUT_ERROR;
        ets_delay_us(1);
    }
    return micros_ticks;
}

/* Wait for next step ~80us*/
static int checkResponse() {
    if(countTicks(80, 0) == DHT11_TIMEOUT_ERROR)
        return DHT11_TIMEOUT_ERROR;

    if(countTicks(80, 1) == DHT11_TIMEOUT_ERROR) 
        return DHT11_TIMEOUT_ERROR;

    return DHT11_OK;
}

int read_dht11() {
    uint8_t sensor_data[5] = {0,0,0,0,0};

    /* Read response */
    for(int i = 0; i < 40; i++) {
        if(countTicks(50, 0) == DHT11_TIMEOUT_ERROR)
            return DHT11_TIMEOUT_ERROR;
                
        if(countTicks(70, 1) > 28) {
            /* Bit received was a 1 */
            sensor_data[i/8] |= (1 << (7-(i%8)));
        }
    }

    if(sensor_data[4] != (sensor_data[0] + sensor_data[1] + sensor_data[2] + sensor_data[3])) {
        return DHT11_CRC_ERROR;
    }
    printf("Reading temperature....:\n");
    printf("       Temp:     %dc\n", sensor_data[2]);
    printf("       Humidity: %d%%\n", sensor_data[0]);

    return DHT11_OK;
}

void app_main() {
    while(1) {
        // delay 1 second before reading sensor
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        // send start signal
        gpio_set_direction(PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(PIN, 0);
        ets_delay_us(20 * 1000);
        gpio_set_level(PIN, 1);
        ets_delay_us(40);
        gpio_set_direction(PIN, GPIO_MODE_INPUT);

        if(checkResponse() == DHT11_TIMEOUT_ERROR)
            break;
        read_dht11();
    }
}