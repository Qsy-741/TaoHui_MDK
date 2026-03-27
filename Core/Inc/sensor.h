/**
 ******************************************************************************
 * @file    sensor.h
 * @brief   Sensor task header - ADC and DHT11 sensor data acquisition
 ******************************************************************************
 */

#ifndef __SENSOR_H__
#define __SENSOR_H__

#define DHT11_GPIO_PORT     GPIOB
#define DHT11_GPIO_PIN     GPIO_PIN_10

#define SENSOR_READ_INTERVAL_MS    1500

void Task_Sensor_Handler(void *argument);

#endif
