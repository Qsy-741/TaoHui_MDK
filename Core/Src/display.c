/**
 ******************************************************************************
 * @file    display.c
 * @brief   Display task implementation - OLED scroll display
 * @details This module manages the SSD1306 OLED display, implementing a
 *         scrolling page display for system sensor data and status.
 *         Display is divided into 3 pages that rotate every 2 seconds:
 *         - Page 0: ADC values (4 channels)
 *         - Page 1: Temperature and humidity
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

/**
 * @brief Display task context structure
 */
typedef struct {
    uint8_t current_page;
    uint32_t last_switch_tick;
} display_context_t;

static display_context_t g_display_ctx;

extern volatile SystemDataSet_t g_sys_data;

static void Display_FillPage(int page);
static void Display_ShowPage0(void);
static void Display_ShowPage1(void);
static void Display_ShowPage2(void);
static void Display_UpdateScroll(void);

/**
 * @brief Display task entry function
 * @param argument: Task argument (unused)
 * @retval None
 * @note Overrides weak definition in freertos.c
 */
void Task_Display_Handler(void *argument)
{
    (void)argument;

    g_display_ctx.current_page = 0;
    g_display_ctx.last_switch_tick = osKernelGetTickCount();
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
    osDelay(500);

    for (;;) {
        Display_UpdateScroll();
        osDelay(DISPLAY_SCROLL_DELAY_MS);
    }
}

/**
 * @brief Update display by scrolling to next page
 * @retval None
 */
static void Display_UpdateScroll(void)
{
    ssd1306_Fill(Black);
    Display_FillPage(g_display_ctx.current_page);
    ssd1306_UpdateScreen();

    g_display_ctx.current_page++;
    if (g_display_ctx.current_page >= DISPLAY_PAGE_COUNT) {
        g_display_ctx.current_page = 0;
    }
}

/**
 * @brief Fill display with content of specified page
 * @param page: Page index (0, 1, 2)
 * @retval None
 */
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
 * @brief Display Page 0 - ADC sensor values
 * @retval None
 */
static void Display_ShowPage0(void)
{
    char line[32];

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("=== Sensor Data ===", Font_6x8, White);

    snprintf(line, sizeof(line), "ADC1: %.2f", (double)g_sys_data.adc_values[0]);
    ssd1306_SetCursor(0, 12);
    ssd1306_WriteString(line, Font_6x8, White);

    snprintf(line, sizeof(line), "ADC2: %.2f", (double)g_sys_data.adc_values[1]);
    ssd1306_SetCursor(0, 24);
    ssd1306_WriteString(line, Font_6x8, White);

    snprintf(line, sizeof(line), "ADC3: %.2f", (double)g_sys_data.adc_values[2]);
    ssd1306_SetCursor(0, 36);
    ssd1306_WriteString(line, Font_6x8, White);

    snprintf(line, sizeof(line), "ADC4: %.2f", (double)g_sys_data.adc_values[3]);
    ssd1306_SetCursor(0, 48);
    ssd1306_WriteString(line, Font_6x8, White);
}

/**
 * @brief Display Page 1 - Environment data (temperature, humidity)
 * @retval None
 */
static void Display_ShowPage1(void)
{
    char line[32];

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("=== Environment ===", Font_6x8, White);

    snprintf(line, sizeof(line), "Temp: %.1f C", (double)g_sys_data.temperature);
    ssd1306_SetCursor(0, 16);
    ssd1306_WriteString(line, Font_6x8, White);

    snprintf(line, sizeof(line), "Humi: %.1f %%", (double)g_sys_data.humidity);
    ssd1306_SetCursor(0, 32);
    ssd1306_WriteString(line, Font_6x8, White);
}

/**
 * @brief Display Page 2 - Switch states
 * @retval None
 */
static void Display_ShowPage2(void)
{
    char line[32];

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("=== Switch State ===", Font_6x8, White);

    for (int i = 0; i < 4; i++) {
        snprintf(line, sizeof(line), "SW%d: %s", i + 1,
                 g_sys_data.switch_state[i] ? "ON " : "OFF");
        ssd1306_SetCursor(0, 12 + i * 12);
        ssd1306_WriteString(line, Font_6x8, White);
    }
}
