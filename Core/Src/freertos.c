/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Task_CLI */
osThreadId_t Task_CLIHandle;
const osThreadAttr_t Task_CLI_attributes = {
  .name = "Task_CLI",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_ESP */
osThreadId_t Task_ESPHandle;
const osThreadAttr_t Task_ESP_attributes = {
  .name = "Task_ESP",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_Sensor */
osThreadId_t Task_SensorHandle;
const osThreadAttr_t Task_Sensor_attributes = {
  .name = "Task_Sensor",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_Control */
osThreadId_t Task_ControlHandle;
const osThreadAttr_t Task_Control_attributes = {
  .name = "Task_Control",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_Display */
osThreadId_t Task_DisplayHandle;
const osThreadAttr_t Task_Display_attributes = {
  .name = "Task_Display",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Task_CLI_Handler(void *argument);
void Task_ESP_Handler(void *argument);
void Task_Sensor_Handler(void *argument);
void Task_Control_Handler(void *argument);
void Task_Display_Handler(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_CLI */
  Task_CLIHandle = osThreadNew(Task_CLI_Handler, NULL, &Task_CLI_attributes);

  /* creation of Task_ESP */
  Task_ESPHandle = osThreadNew(Task_ESP_Handler, NULL, &Task_ESP_attributes);

  /* creation of Task_Sensor */
  Task_SensorHandle = osThreadNew(Task_Sensor_Handler, NULL, &Task_Sensor_attributes);

  /* creation of Task_Control */
  Task_ControlHandle = osThreadNew(Task_Control_Handler, NULL, &Task_Control_attributes);

  /* creation of Task_Display */
  Task_DisplayHandle = osThreadNew(Task_Display_Handler, NULL, &Task_Display_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_Task_CLI_Handler */
/**
  * @brief  Function implementing the Task_CLI thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Task_CLI_Handler */
__weak void Task_CLI_Handler(void *argument)
{
  /* USER CODE BEGIN Task_CLI_Handler */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Task_CLI_Handler */
}

/* USER CODE BEGIN Header_Task_ESP_Handler */
/**
* @brief Function implementing the Task_ESP thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_ESP_Handler */
__weak void Task_ESP_Handler(void *argument)
{
  /* USER CODE BEGIN Task_ESP_Handler */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Task_ESP_Handler */
}

/* USER CODE BEGIN Header_Task_Sensor_Handler */
/**
* @brief Function implementing the Task_Sensor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_Sensor_Handler */
__weak void Task_Sensor_Handler(void *argument)
{
  /* USER CODE BEGIN Task_Sensor_Handler */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Task_Sensor_Handler */
}

/* USER CODE BEGIN Header_Task_Control_Handler */
/**
* @brief Function implementing the Task_Control thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_Control_Handler */
__weak void Task_Control_Handler(void *argument)
{
  /* USER CODE BEGIN Task_Control_Handler */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Task_Control_Handler */
}

/* USER CODE BEGIN Header_Task_Display_Handler */
/**
* @brief Function implementing the Task_Display thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_Display_Handler */
__weak void Task_Display_Handler(void *argument)
{
  /* USER CODE BEGIN Task_Display_Handler */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Task_Display_Handler */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

