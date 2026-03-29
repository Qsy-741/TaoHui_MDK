/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "cli.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#include <stdint.h>

typedef struct {
    uint16_t sensor_values[3];     // [0]: water level, [1]: soil moisture, [2]: brightness
    float temperature;       // DHT11 temperature sensor
    float humidity;          // DHT11 humidity sensor
    uint16_t CO2;            // JW01 CO2 sensor (UART3)
    uint8_t switch_state[2];
} SystemDataSet_t;

extern volatile SystemDataSet_t g_sys_data;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Fan1_Pin GPIO_PIN_4
#define Fan1_GPIO_Port GPIOA
#define Fan2_Pin GPIO_PIN_5
#define Fan2_GPIO_Port GPIOA
#define Switch_Pin GPIO_PIN_6
#define Switch_GPIO_Port GPIOA
#define LightSensor_Pin GPIO_PIN_1
#define LightSensor_GPIO_Port GPIOB
#define CO2A_Pin GPIO_PIN_10
#define CO2A_GPIO_Port GPIOB
#define CO2B_Pin GPIO_PIN_11
#define CO2B_GPIO_Port GPIOB
#define DhtSensor_Pin GPIO_PIN_12
#define DhtSensor_GPIO_Port GPIOB
#define DhtSensor_EXTI_IRQn EXTI15_10_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
