#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/semphr.h"
#include "driver/uart.h"

#define GPIO_OUT_REG (*((volatile uint32_t *)0x3FF44004)) // outptu register address
#define GPIO_ENABLE_REG (*((volatile uint32_t *)0x3FF44020)) // enable reg address
#define GPIO_IN_REG (*((volatile uint32_t *)0x3FF4403C)) // input register address
#define LED_PIN 2
#define LED_PIN_1 4
#define LED_PIN_2 5
#define INPUT_PIN 19
SemaphoreHandle_t gpio_mutex; // Mutex for GPIO access

volatile uint8_t paused = 0; // Variable to track the paused state by button
volatile char mode = 'b';

void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);
}

char uart_read_command(void)
{
    char data = '\0';
    int len = uart_read_bytes(UART_NUM_0, &data, 1, 50 / portTICK_PERIOD_MS);
    if (len > 0)
    {
        uart_write_bytes(UART_NUM_0, &data, 1);
    }
    if (len > 0 && (data == '1' || data == '0' || data == 'b'))
    {
        return data;
    }
    return '\0';
}

void uart_task(void *pvParameters)
{
    while(1)
    {
        char cmd = uart_read_command();
        if (cmd != '\0')
        {
            mode = cmd;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void pause_task(void *pvParameters)
{
    while(1)
    {
        if (!(GPIO_IN_REG & (1 << INPUT_PIN))) //If pressed, statement returns 1
        {
            paused = 1;
        }
        else
        {
            paused = 0;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void led1_task(void *pvParameters)
{
    while(1)
    {
        if (mode == '1')
        {
            xSemaphoreTake(gpio_mutex, portMAX_DELAY);
            GPIO_OUT_REG |= (1 << LED_PIN); // Turn on LED1
            xSemaphoreGive(gpio_mutex);
        }
        else if (mode == '0')
        {
            xSemaphoreTake(gpio_mutex, portMAX_DELAY);
            GPIO_OUT_REG &= ~(1 << LED_PIN); // Turn off LED1
            xSemaphoreGive(gpio_mutex);
        }
        if (mode == 'b' && (!paused))//if not paused, blink led1
        {
            xSemaphoreTake(gpio_mutex, portMAX_DELAY);
            GPIO_OUT_REG ^= (1 << LED_PIN);
            xSemaphoreGive(gpio_mutex);
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void led2_task(void *pvParameters)
{
    while(1)
    {
        if (mode == '1')
        {
            xSemaphoreTake(gpio_mutex, portMAX_DELAY);
            GPIO_OUT_REG |= (1 << LED_PIN_1); // Turn on LED2
            xSemaphoreGive(gpio_mutex);
        }
        else if (mode == '0')
        {
            xSemaphoreTake(gpio_mutex, portMAX_DELAY);
            GPIO_OUT_REG &= ~(1 << LED_PIN_1); // Turn off LED2
            xSemaphoreGive(gpio_mutex);
        }
        if (mode == 'b' && (!paused))
        {
            xSemaphoreTake(gpio_mutex, portMAX_DELAY);
            GPIO_OUT_REG ^= (1 << LED_PIN_1); //same as led1
            xSemaphoreGive(gpio_mutex);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void led3_task(void *pvParameters)
{
    while(1)
    {
        if (mode == '1')
        {
            xSemaphoreTake(gpio_mutex, portMAX_DELAY);
            GPIO_OUT_REG |= (1 << LED_PIN_2); // turn on LED3
            xSemaphoreGive(gpio_mutex);
        }
        else if (mode == '0')
        {
            xSemaphoreTake(gpio_mutex, portMAX_DELAY);
            GPIO_OUT_REG &= ~(1 << LED_PIN_2); //trun off LED3
            xSemaphoreGive(gpio_mutex);
        }
        if (mode == 'b' && (!paused))
        {
            xSemaphoreTake(gpio_mutex, portMAX_DELAY);
            GPIO_OUT_REG ^= (1 << LED_PIN_2); //blink LED3
            xSemaphoreGive(gpio_mutex);
        }
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    uart_init();
    //setup gpio pins
    GPIO_ENABLE_REG |= (1 << LED_PIN) | (1 << LED_PIN_1) | (1 << LED_PIN_2);
    gpio_mutex = xSemaphoreCreateMutex();
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 1, NULL);
    xTaskCreate(pause_task, "pause_task", 2048, NULL, 2, NULL);
    xTaskCreate(led1_task, "led1_task", 2048, NULL, 3, NULL);
    xTaskCreate(led2_task, "led2_task", 2048, NULL, 4, NULL);
    xTaskCreate(led3_task, "led3_task", 2048, NULL, 5, NULL);

}