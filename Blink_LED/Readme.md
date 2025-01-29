# LED Blink Project

This project demonstrates how to blink an LED on an ESP32 using the ESP-IDF framework.

## Prerequisites

- ESP-IDF v5.2.2 or later
- An ESP32 development board
- A USB cable to connect the ESP32 to your computer

## Setup

1. Set up the ESP-IDF environment:
    ```sh
    . /home/ibrahim/esp/v5.2.2/esp-idf/export.sh
    ```

2. Create the project:
    ```sh
    idf.py create-project -p . LED_Blink
    ```

## Build and Flash

1. Build the project:
    ```sh
    idf.py build
    ```

2. Flash the project to the ESP32:
    ```sh
    idf.py -p /dev/ttyUSB0 flash
    ```

3. Monitor the output:
    ```sh
    idf.py monitor
    ```

## Code Overview

The main code for this project is in [LED_Blink.c](main/LED_Blink.c):

```c
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
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Turn off the LED
        gpio_set_level(GPIO_NUM_2, 0);
        ESP_LOGI(TAG, "LED turned off");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
```