/**
 ******************************************************************************
 * @file    display.h
 * @brief   Display task header - OLED display management
 * @details This module provides the interface for the display task which
 *         manages the SSD1306 OLED display. The display shows system data
 *         in a scrolling page format.
 ******************************************************************************
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

void Task_Display_Handler(void *argument);

#endif
