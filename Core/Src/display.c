/**
 ******************************************************************************
 * @file    display.c
 * @brief   Display task implementation - OLED scroll display
 * @details This module manages the SSD1306 OLED display, implementing a
 *         scrolling page display for system sensor data and status.
 *         Display is divided into 3 pages that rotate every 2 seconds:
 *         - Page 0: ADC values (water level, soil moisture, brightness)
 *         - Page 1: Environment (temperature, humidity, CO2)
 *         - Page 2: Switch states
 ******************************************************************************
 */

#include "display.h"
#include "main.h"
#include "cmsis_os.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdio.h>

#define DISPLAY_SCROLL_DELAY_MS    2000
#define DISPLAY_PAGE_COUNT         3

extern volatile SystemDataSet_t g_sys_data;

static void Display_FillPage(int page);
static void Display_ShowPage0(void);
static void Display_ShowPage1(void);
static void Display_ShowPage2(void);
static void Display_UpdateScroll(void);

static uint8_t g_current_page = 0;

void Task_Display_Handler(void *argument)
{
    (void)argument;

    g_current_page = 0;
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
    osDelay(500);

    for (;;) {
        Display_UpdateScroll();
        osDelay(DISPLAY_SCROLL_DELAY_MS);
    }
}

static void Display_UpdateScroll(void)
{
    ssd1306_Fill(Black);
    Display_FillPage(g_current_page);
    ssd1306_UpdateScreen();

    g_current_page++;
    if (g_current_page >= DISPLAY_PAGE_COUNT) {
        g_current_page = 0;
    }
}

static void Display_FillPage(int page)
{
    switch (page) {
        case 0:
            Display_ShowPage0();
            break;
        case 1:
            Display_ShowPage1();
            break;
        case 2:
            Display_ShowPage2();
            break;
    }
}

/**
 * @brief Display Page 0 - ADC sensor values (water level, soil moisture, brightness)
 * @retval None
 */
static void Display_ShowPage0(void)
{
    char line[32];

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("=== ADC Sensors ===", Font_7x10, White);

    snprintf(line, sizeof(line), "Water: %.2f%%", (double)g_sys_data.sensor_values[0]);
    ssd1306_SetCursor(0, 12);
    ssd1306_WriteString(line, Font_7x10, White);

    snprintf(line, sizeof(line), "Soil:  %.2f%%", (double)g_sys_data.sensor_values[1]);
    ssd1306_SetCursor(0, 24);
    ssd1306_WriteString(line, Font_7x10, White);

    snprintf(line, sizeof(line), "Light: %.2f%%", (double)g_sys_data.sensor_values[2]);
    ssd1306_SetCursor(0, 36);
    ssd1306_WriteString(line, Font_7x10, White);
}

/**
 * @brief Display Page 1 - Environment data (temperature, humidity, CO2)
 * @retval None
 */
static void Display_ShowPage1(void)
{
    char line[32];

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("=== Environment ===", Font_7x10, White);

    snprintf(line, sizeof(line), "Temp:  %.1f C", (double)g_sys_data.temperature);
    ssd1306_SetCursor(0, 12);
    ssd1306_WriteString(line, Font_7x10, White);

    snprintf(line, sizeof(line), "Humi:  %.1f %%", (double)g_sys_data.humidity);
    ssd1306_SetCursor(0, 24);
    ssd1306_WriteString(line, Font_7x10, White);

    snprintf(line, sizeof(line), "CO2:   %d ppm", g_sys_data.CO2);
    ssd1306_SetCursor(0, 36);
    ssd1306_WriteString(line, Font_7x10, White);
}

static void Display_ShowPage2(void)
{
    char line[32];

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("=== Switch State ===", Font_7x10, White);

    for (int i = 0; i < 2; i++) {
        snprintf(line, sizeof(line), "SW%d: %s", i + 1,
                 g_sys_data.switch_state[i] ? "ON " : "OFF");
        ssd1306_SetCursor(0, 12 + i * 12);
        ssd1306_WriteString(line, Font_7x10, White);
    }
}
