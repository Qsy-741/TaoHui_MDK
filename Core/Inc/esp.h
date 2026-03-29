/**
 ******************************************************************************
 * @file    esp.h
 * @brief   ESP task header - ESP01S WiFi module communication
 ******************************************************************************
 */

#ifndef __ESP_H__
#define __ESP_H__

#define ESP_UART_BAUD_RATE    115200
#define ESP_WIFI_SSID        "WiFi7"
#define ESP_WIFI_PASSWORD    "12345678"

void Task_ESP_Handler(void *argument);

#endif
