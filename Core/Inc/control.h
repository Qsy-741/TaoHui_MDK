/**
 ******************************************************************************
 * @file    control.h
 * @brief   Control task header - Fan and Switch actuator control
 ******************************************************************************
 */

#ifndef __CONTROL_H__
#define __CONTROL_H__

#define CONTROL_FAN_TEMP_THRESHOLD     20.0f
#define CONTROL_SWITCH_SOIL_THRESHOLD  40.0f
#define CONTROL_TASK_INTERVAL_MS       100

void Task_Control_Handler(void *argument);

#endif
