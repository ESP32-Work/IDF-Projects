#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "esp32-led-blink";

void app_main(void)
{
    // Configure the GPIO as an output
    gpio_reset_pin(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    while (1) {
        // Turn on the LED
        gpio_set_level(GPIO_NUM_2, 1);
        ESP_LOGI(TAG, "LED turned on");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // Turn off the LED
        gpio_set_level(GPIO_NUM_2, 0);
        ESP_LOGI(TAG, "LED turned off");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}