/**
 ******************************************************************************
 * @file    control.c
 * @brief   Control task implementation - Fan and Switch actuator control
 * @details This module handles actuator control based on sensor readings:
 *         - Fan (Fan1, Fan2): Turn ON when temperature > threshold
 *         - Switch (Water pump): Turn ON when soil moisture < threshold
 *         Both actuators update the g_sys_data.switch_state array.
 ******************************************************************************
 */

#include "control.h"
#include "main.h"
#include "cmsis_os.h"

extern volatile SystemDataSet_t g_sys_data;

/**
 * @brief Control Fan actuators based on temperature
 * @retval None
 * @note Fan turns ON when temperature exceeds CONTROL_FAN_TEMP_THRESHOLD
 */
static void Control_UpdateFan(void)
{
    if (g_sys_data.temperature > CONTROL_FAN_TEMP_THRESHOLD) {
        HAL_GPIO_WritePin(Fan1_GPIO_Port, Fan1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(Fan2_GPIO_Port, Fan2_Pin, GPIO_PIN_RESET);
        g_sys_data.switch_state[0] = 1;
    } else {
        HAL_GPIO_WritePin(Fan1_GPIO_Port, Fan1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Fan2_GPIO_Port, Fan2_Pin, GPIO_PIN_RESET);
        g_sys_data.switch_state[0] = 0;
    }
}

/**
 * @brief Control Switch (water pump) based on soil moisture
 * @retval None
 * @note Switch turns ON when soil moisture (adc_values[1]) is below threshold
 */
static void Control_UpdateSwitch(void)
{
    if (g_sys_data.sensor_values[1] < CONTROL_SWITCH_SOIL_THRESHOLD) {
        HAL_GPIO_WritePin(Switch_GPIO_Port, Switch_Pin, GPIO_PIN_SET);
        g_sys_data.switch_state[1] = 1;
    } else {
        HAL_GPIO_WritePin(Switch_GPIO_Port, Switch_Pin, GPIO_PIN_RESET);
        g_sys_data.switch_state[1] = 0;
    }
}

/**
 * @brief Control task entry function
 * @param argument: Task argument (unused)
 * @retval None
 * @note Overrides weak definition in freertos.c
 */
void Task_Control_Handler(void *argument)
{
    (void)argument;

    uint32_t cycle_tick = osKernelGetTickCount();

    HAL_GPIO_WritePin(Fan1_GPIO_Port, Fan1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Fan2_GPIO_Port, Fan2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Switch_GPIO_Port, Switch_Pin, GPIO_PIN_RESET);

    for (;;) {
        Control_UpdateFan();
        Control_UpdateSwitch();

        vTaskDelayUntil(&cycle_tick, pdMS_TO_TICKS(CONTROL_TASK_INTERVAL_MS));
    }
}
