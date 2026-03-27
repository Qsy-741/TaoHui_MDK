/**
 ******************************************************************************
 * @file    sensor.c
 * @brief   Sensor task implementation - ADC and DHT11 sensor data acquisition
 * @details This module handles periodic sensor data collection including:
 *         - 4-channel ADC via DMA
 *         - DHT11 temperature and humidity sensor
 *         Data is updated to the global SystemDataSet_t structure.
 ******************************************************************************
 */

#include "sensor.h"
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
// #include "dht11.h"
#include <string.h>
#include "tim.h"

#define ADC_CHANNEL_COUNT    4

extern volatile SystemDataSet_t g_sys_data;
// extern volatile DHT11_State_t dht11_state;
// extern volatile uint32_t last_time;
// extern volatile uint8_t dht11_buffer[5];

static uint16_t g_adc_dma_buffer[ADC_CHANNEL_COUNT];

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim2;
extern osThreadId Task_SensorHandle;

// DHT11 状态机枚举
typedef enum {
    DHT11_IDLE = 0,
    DHT11_WAIT_RESPONSE,
    DHT11_RECEIVING_DATA
} DHT11_State_t;

volatile DHT11_State_t dht11_state = DHT11_IDLE;
volatile uint16_t last_time = 0;
volatile uint8_t bit_count = 0;
volatile uint8_t dht11_buffer[5] = {0}; // 存放温湿度数据

// 配置DHT11_GPIO_PIN为推挽输出
static void DHT11_Pin_Output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

// 配置DHT11_GPIO_PIN为下降沿外部中断
static void DHT11_Pin_EXTI(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; // 下降沿触发
    GPIO_InitStruct.Pull = GPIO_PULLUP;          // 必须上拉
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

// 外部中断回调函数
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == DHT11_GPIO_PIN)     
    {
        if (dht11_state == DHT11_IDLE) return; // 空闲状态不处理

        // 获取当前定时器计数值 (微秒)
        uint16_t current_time = __HAL_TIM_GET_COUNTER(&htim2);
        // 计算与上一个下降沿的时间差 (无符号16位减法自动处理溢出)
        uint16_t delta_t = current_time - last_time;
        last_time = current_time;

        if (dht11_state == DHT11_WAIT_RESPONSE) 
        {
            // DHT11响应信号：拉低80us + 拉高80us ≈ 160us
            // 允许一定的误差范围 140us ~ 180us
            if (delta_t > 140 && delta_t < 180) {
                dht11_state = DHT11_RECEIVING_DATA;
                bit_count = 0;
                for(int i=0; i<5; i++) dht11_buffer[i] = 0; // 清空缓存
            }
        } 
        else if (dht11_state == DHT11_RECEIVING_DATA) 
        {
            // 数据位解析
            if (delta_t >= 60 && delta_t <= 90) {
                // 收到 '0' (约 76us)
                // 默认缓存里已经是0了，只需位移不用修改
                bit_count++;
            } 
            else if (delta_t >= 110 && delta_t <= 140) {
                // 收到 '1' (约 120us)
                uint8_t byte_idx = bit_count / 8;
                uint8_t bit_idx  = 7 - (bit_count % 8);
                dht11_buffer[byte_idx] |= (1 << bit_idx);
                bit_count++;
            }
            else {
                // 干扰或错误数据，复位状态机
                dht11_state = DHT11_IDLE; 
            }

            // 接收完40位数据 (5个字节)
            if (bit_count >= 40) {
                dht11_state = DHT11_IDLE;
                // 发送线程标志唤醒CMSIS-RTOS2任务
                osThreadFlagsSet(Task_SensorHandle, 0x01);
            }
        }
    }
}

/**
 * @brief Read ADC values via DMA
 * @retval None
 * @note Triggers ADC conversion and waits for DMA completion
 */
static void Sensor_ReadADC(void)
{
    memset(g_adc_dma_buffer, 0, sizeof(g_adc_dma_buffer));

    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_adc_dma_buffer, ADC_CHANNEL_COUNT) == HAL_OK) {
        uint32_t timeout = osKernelGetTickCount() + 1000;
        while (__HAL_ADC_GET_FLAG(&hadc1, ADC_FLAG_EOC) == 0) {
            if (osKernelGetTickCount() >= timeout) {
                HAL_ADC_Stop_DMA(&hadc1);
                return;
            }
            osDelay(1);
        }
        HAL_ADC_Stop_DMA(&hadc1);
    }
}

/**
 * @brief Sensor task entry function
 * @param argument: Task argument (unused)
 * @retval None
 * @note Overrides weak definition in freertos.c
 */
void Task_Sensor_Handler(void *argument)
{
    (void)argument;
    // 启动微秒级定时器
    HAL_TIM_Base_Start(&htim2);
    uint32_t cycle_tick = osKernelGetTickCount();

    for (;;) {
        Sensor_ReadADC();

        g_sys_data.adc_values[0] = (float)g_adc_dma_buffer[0] * 3.3f / 4095.0f;
        g_sys_data.adc_values[1] = (float)g_adc_dma_buffer[1] * 3.3f / 4095.0f;
        g_sys_data.adc_values[2] = (float)g_adc_dma_buffer[2] * 3.3f / 4095.0f;
        g_sys_data.adc_values[3] = (float)g_adc_dma_buffer[3] * 3.3f / 4095.0f;

        DHT11_Pin_Output();
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
        // FreeRTOS延时20ms (此时CPU释放给其他任务，非阻塞！)
        osDelay(20); 
        // 拉高引脚，准备接收
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
        // 2. 切换到中断模式，开启接收状态机
        last_time = __HAL_TIM_GET_COUNTER(&htim2);
        dht11_state = DHT11_WAIT_RESPONSE;
        DHT11_Pin_EXTI();
        // 3. 挂起任务，等待中断接收完毕 (设置超时时间为 1000ms)
        // osThreadFlagsWait 相当于一个轻量级的二值信号量
        uint32_t flags = osThreadFlagsWait(0x01, osFlagsWaitAny, pdMS_TO_TICKS(1000));
        if (flags == 0x01) 
        {
            // 成功收到40位数据，校验校验和
            uint8_t sum = dht11_buffer[0] + dht11_buffer[1] + dht11_buffer[2] + dht11_buffer[3];
            if (sum == dht11_buffer[4]) 
            {
                g_sys_data.humidity = dht11_buffer[0] + (float)dht11_buffer[1]/10.0f;
                g_sys_data.temperature = dht11_buffer[2] + (float)dht11_buffer[3]/10.0f;
            }
        }
        else
        {
            // 超时，说明传感器未连接或读取失败
            dht11_state = DHT11_IDLE; // 复位状态机
        }

        vTaskDelayUntil(&cycle_tick, pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
    }
}
