/**
 ******************************************************************************
 * @file    esp.c
 * @brief   ESP task implementation - ESP01S WiFi module communication
 * @details This module handles communication with the ESP01S WiFi module via
 *         UART2. It first sends WiFi credentials to ESP01S and waits for
 *         acknowledgment (0xFF), then periodically sends sensor data with
 *         data reception acknowledgment.
 ******************************************************************************
 */

#include "esp.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

#define ESP_SEND_INTERVAL_MS    1000
#define ESP_TX_BUF_SIZE        256
#define ESP_WIFI_RETRY_MAX     30

typedef enum {
    ESP_STATE_INIT,
    ESP_STATE_CONNECTING,
    ESP_STATE_CONNECTED,
    ESP_STATE_ERROR
} esp_state_t;

static volatile esp_state_t g_esp_state = ESP_STATE_INIT;

extern volatile SystemDataSet_t g_sys_data;

static char g_tx_buf[ESP_TX_BUF_SIZE];

extern UART_HandleTypeDef huart2;

#define ESP_UART_HANDLE   (&huart2)

/**
 * @brief Send WiFi credentials to ESP01S and wait for connection
 * @retval 1 if WiFi connected successfully, 0 otherwise
 */
static uint8_t ESP_WiFiConnect(void)
{
    int len;
    uint8_t rx_byte;
    uint8_t retry_count = 0;
    uint32_t start_tick;

    g_esp_state = ESP_STATE_CONNECTING;

    len = snprintf(g_tx_buf, ESP_TX_BUF_SIZE,
        "WIFI:%s,%s\r\n",
        ESP_WIFI_SSID,
        ESP_WIFI_PASSWORD
    );

    HAL_UART_Transmit(ESP_UART_HANDLE, (uint8_t *)g_tx_buf, len, HAL_MAX_DELAY);

    start_tick = osKernelGetTickCount();

    while (retry_count < ESP_WIFI_RETRY_MAX) {
        if (HAL_UART_Receive(ESP_UART_HANDLE, &rx_byte, 1, 1000) == HAL_OK) {
            if (rx_byte == 0xFF) {
                g_esp_state = ESP_STATE_CONNECTED;
                return 1;
            }
        }

        if ((osKernelGetTickCount() - start_tick) >= 3000) {
            retry_count++;
            if (retry_count < ESP_WIFI_RETRY_MAX) {
                memset(g_tx_buf, 0, ESP_TX_BUF_SIZE);
                len = snprintf(g_tx_buf, ESP_TX_BUF_SIZE,
                    "WIFI:%s,%s\r\n",
                    ESP_WIFI_SSID,
                    ESP_WIFI_PASSWORD
                );
                HAL_UART_Transmit(ESP_UART_HANDLE, (uint8_t *)g_tx_buf, len, HAL_MAX_DELAY);
                start_tick = osKernelGetTickCount();
            }
        }

        osDelay(10);
    }

    g_esp_state = ESP_STATE_ERROR;
    return 0;
}

/**
 * @brief Send data packet to ESP01S via UART
 * @retval 1 if data acknowledged, 0 otherwise
 */
static uint8_t ESP_SendData(void)
{
    int len;
    uint8_t rx_byte;
    uint8_t ack_received = 0;

    len = snprintf(g_tx_buf, ESP_TX_BUF_SIZE,
        "{\"adc\":[%d,%d,%d],"
        "\"temp\":%.1f,\"humi\":%.1f,"
        "\"CO2\":%d,\"sw\":[%d,%d]"
        "}\r\n",
        g_sys_data.sensor_values[0],
        g_sys_data.sensor_values[1],
        g_sys_data.sensor_values[2],
        g_sys_data.temperature,   
        g_sys_data.humidity,
        g_sys_data.CO2,
        g_sys_data.switch_state[0],
        g_sys_data.switch_state[1] 
    );

    if (len > 0 && len < ESP_TX_BUF_SIZE) {
        HAL_UART_Transmit(ESP_UART_HANDLE, (uint8_t *)g_tx_buf, len, HAL_MAX_DELAY);

        if (HAL_UART_Receive(ESP_UART_HANDLE, &rx_byte, 1, 300) == HAL_OK) {
            if (rx_byte == 0xAA) {
                ack_received = 1;
            }
        }
    }

    return ack_received;
}

/**
 * @brief ESP task entry function
 * @param argument: Task argument (unused)
 * @retval None
 * @note Overrides weak definition in freertos.c
 */
void Task_ESP_Handler(void *argument)
{
    (void)argument;

    for (;;) {
        if (g_esp_state != ESP_STATE_CONNECTED) {
            if (ESP_WiFiConnect()) {
                g_esp_state = ESP_STATE_CONNECTED;
            } else {
                osDelay(2000);
            }
        } else {
            ESP_SendData();
            osDelay(ESP_SEND_INTERVAL_MS);
        }
    }
}
