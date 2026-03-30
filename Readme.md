# TaoHui_MDK - STM32F103C8T6 FreeRTOS 智能农业系统

## 项目简介

本项目是一个基于 **STM32F103C8T6** 微控制器的智能农业控制系统，运行 **FreeRTOS v10.0.1** 实时操作系统。系统集成了多种传感器数据采集、执行器自动控制、OLED本地显示和WiFi远程上报功能。

## 硬件配置

### MCU 规格
| 项目 | 参数 |
|------|------|
| 型号 | STM32F103C8T6 |
| 内核 | ARM Cortex-M3 |
| 主频 | 72MHz (HSE + PLL) |
| Flash | 64KB |
| SRAM | 20KB |

### 引脚分配

#### 通信接口
| 功能 | 引脚 | 说明 |
|------|------|------|
| USART1 (CLI) | PA9(TX), PA10(RX) | 调试串口 / CLI命令行 |
| USART2 (ESP) | PA2(TX), PA3(RX) | ESP01S WiFi模块通信 |
| USART3 (CO2) | PB10(TX), PB11(RX) | JW01 CO2传感器通信 |
| I2C1 (OLED) | PB6(SCL), PB7(SDA) | SSD1306 OLED显示屏 |

#### 传感器
| 传感器 | 引脚 | 说明 |
|--------|------|------|
| DHT11 | PB12 | 温湿度传感器 (单总线) |
| ADC6 (水位) | PA6 | 水位传感器模拟输入 |
| ADC7 (土壤湿度) | PA7 | 土壤湿度传感器模拟输入 |
| ADC8 (光照) | PB0 | 光照强度传感器模拟输入 |

#### 执行器
| 执行器 | 引脚 | 说明 |
|--------|------|------|
| Fan1 | PA4 | 散热风扇1 |
| Fan2 | PA5 | 散热风扇2 |
| Switch | PA6 | 水泵开关 |

## 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                      FreeRTOS 调度器                         │
├─────────────┬─────────────┬─────────────┬─────────────────┤
│  Task_CLI   │ Task_ESP   │ Task_Sensor│  Task_Control   │
│  (Normal)   │ (Low)      │ (Low)      │  (Low)          │
│             │            │            │                 │
│ CLI命令解析  │ WiFi数据上报│ 传感器采集  │ 执行器控制       │
│  日志打印   │ ESP01S通信  │ ADC+DHT11  │  Fan+Switch     │
└──────┬──────┴──────┬─────┴─────┬──────┴────────┬────────┘
       │             │           │               │
       ▼             ▼           ▼               ▼
┌─────────────────────────────────────────────────────────────┐
│                    全局数据 SystemDataSet_t                  │
│  sensor_values[3] | temperature | humidity | CO2 | switch  │
└─────────────────────────────────────────────────────────────┘
       │             │           │               │
       ▼             ▼           ▼               ▼
┌──────────┐  ┌──────────┐ ┌──────────┐ ┌──────────────┐
│ USART1   │  │ USART2   │ │ USART3   │ │   GPIO       │
│ CLI调试   │  │ ESP01S   │ │ JW01 CO2 │ │ Fan/Switch   │
└──────────┘  └──────────┘ └──────────┘ └──────────────┘
       │             │           │               │
       ▼             ▼           ▼               ▼
┌──────────┐  ┌──────────┐ ┌──────────┐ ┌──────────────┐
│ 串口终端  │  │ WiFi服务器│ │ CO2传感器│ │  物理执行器   │
└──────────┘  └──────────┘ └──────────┘ └──────────────┘
```

## 全局数据结构

```c
typedef struct {
    uint16_t sensor_values[3];  // [0]:水位 [1]:土壤湿度 [2]:光照
    float temperature;          // DHT11温度 (℃)
    float humidity;             // DHT11湿度 (%)
    uint16_t CO2;               // JW01 CO2浓度 (ppm)
    uint8_t switch_state[2];    // [0]:风扇状态 [1]:水泵状态
} SystemDataSet_t;

extern volatile SystemDataSet_t g_sys_data;
```

## 任务详解

### 1. Task_CLI - 命令行交互任务

| 属性 | 值 |
|------|-----|
| 栈大小 | 256 × 4 = 1024 bytes |
| 优先级 | Normal (5) |
| 通信 | USART1 @ 115200 bps |

**功能说明：**
- 通过串口提供命令行交互界面
- 支持命令历史记录
- 自动回显功能

**内置命令：**
| 命令 | 功能 |
|------|------|
| `help` | 显示所有可用命令 |
| `clear` | 清屏 |
| `info` | 显示系统信息 |

**扩展方式：**
```c
// 在 cli.c 的 CLI_RegisterBuiltInCommands() 中添加
CLI_RegisterCommand("cmd_name", "description", CMD_HandlerFunc);
```

---

### 2. Task_ESP - WiFi通信任务

| 属性 | 值 |
|------|-----|
| 栈大小 | 512 × 4 = 2048 bytes |
| 优先级 | Low (4) |
| 通信 | USART2 @ 115200 bps |

**功能说明：**
- 启动时发送WiFi连接指令至ESP01S
- 等待ESP回复0xFF确认连接成功
- 周期性地以JSON格式发送传感器数据
- 每包数据后等待0xFF确认

**WiFi配置（esp.h）：**
```c
#define ESP_WIFI_SSID     "YourWiFiSSID"
#define ESP_WIFI_PASSWORD "YourPassword"
```

**发送数据格式：**
```json
{"adc":[x.xx,x.xx,x.xx,x.xx],"temp":xx.x,"humi":xx.x,"sw":[x,x],"CO2":xxxx}\r\n
```

---

### 3. Task_Sensor - 传感器采集任务

| 属性 | 值 |
|------|-----|
| 栈大小 | 256 × 4 = 1024 bytes |
| 优先级 | Low (4) |
| 采集周期 | 1500ms |

**数据源：**
| 传感器 | 方式 | 说明 |
|--------|------|------|
| ADC (3通道) | DMA | 水位、土壤湿度、光照 |
| DHT11 | EXTI中断+轮询 | 温湿度 |
| JW01 CO2 | USART3 DMA+IDLE | CO2浓度 |

**ADC通道配置：**
| 通道 | 引脚 | 功能 | 转换公式 |
|------|------|------|----------|
| CH6 | PA7 | 水位传感器 | voltage × factor |
| CH7 | PB0 | 土壤湿度 | voltage × factor |
| CH8 | PB1 | 光照强度 | voltage × factor |

---

### 4. Task_Control - 执行器控制任务

| 属性 | 值 |
|------|-----|
| 栈大小 | 256 × 4 = 1024 bytes |
| 优先级 | Low (4) |
| 控制周期 | 100ms |

**控制逻辑：**

| 执行器 | GPIO | 触发条件 | 动作 |
|--------|------|----------|------|
| Fan1 + Fan2 | PA4, PA5 | 温度 > 20℃ | 开启散热 |
| Switch (水泵) | PA6 | 土壤湿度 < 40% | 开启灌溉 |

**阈值配置（control.h）：**
```c
#define CONTROL_FAN_TEMP_THRESHOLD      20.0f   // 温度阈值(℃)
#define CONTROL_SWITCH_SOIL_THRESHOLD  40.0f   // 土壤湿度阈值(%)
```

---

### 5. Task_Display - OLED显示任务

| 属性 | 值 |
|------|-----|
| 栈大小 | 256 × 4 = 1024 bytes |
| 优先级 | Low (4) |
| 屏幕 | SSD1306 128×64 OLED |
| 刷新周期 | 3页滚动，每页2秒 |

**显示页面：**

| 页面 | 内容 |
|------|------|
| Page 1 | 水位、土壤湿度、光照值 |
| Page 2 | 温度、湿度、CO2浓度 |
| Page 3 | 风扇状态、水泵状态 |

---

## 目录结构

```
TaoHui_MDK/
├── Core/
│   ├── Src/
│   │   ├── main.c           # 入口函数
│   │   ├── freertos.c        # FreeRTOS任务创建
│   │   ├── gpio.c            # GPIO配置
│   │   ├── usart.c           # USART1/2/3初始化
│   │   ├── adc.c             # ADC配置
│   │   ├── dma.c             # DMA配置
│   │   ├── i2c.c             # I2C1配置(OLED)
│   │   ├── tim.c             # 定时器配置
│   │   ├── ssd1306.c         # OLED驱动
│   │   ├── cli.c             # CLI命令行任务 ⭐
│   │   ├── esp.c             # ESP01S通信任务 ⭐
│   │   ├── sensor.c          # 传感器采集任务 ⭐
│   │   ├── control.c         # 执行器控制任务 ⭐
│   │   └── stm32f1xx_it.c    # 中断服务程序
│   └── Inc/
│       ├── main.h            # 全局定义和结构体
│       ├── cli.h             # CLI任务头文件 ⭐
│       ├── esp.h              # ESP任务头文件 ⭐
│       ├── sensor.h           # Sensor任务头文件 ⭐
│       ├── control.h          # Control任务头文件 ⭐
│       ├── ssd1306.h          # OLED驱动头文件
│       ├── ssd1306_fonts.h    # OLED字体定义
│       └── FreeRTOSConfig.h   # FreeRTOS配置
├── Drivers/
│   ├── CMSIS/                # ARM内核定义
│   └── STM32F1xx_HAL_Driver/ # STM32 HAL库
└── Middlewares/
    └── FreeRTOS/             # FreeRTOS v10.0.1
```

⭐ = 自定义任务文件

## 任务优先级一览

| 任务名 | 优先级 | 栈大小 | 周期 |
|--------|--------|--------|------|
| Task_CLI | Normal (5) | 1KB | 事件触发 |
| Task_ESP | Low (4) | 2KB | 1000ms |
| Task_Sensor | Low (4) | 1KB | 1500ms |
| Task_Control | Low (4) | 1KB | 100ms |
| Task_Display | Low (4) | 1KB | 2000ms |

## 配置修改指南

### 1. WiFi配置
文件：`Core/Inc/esp.h`
```c
#define ESP_WIFI_SSID     "YourWiFiSSID"
#define ESP_WIFI_PASSWORD "YourPassword"
```

### 2. 控制阈值
文件：`Core/Inc/control.h`
```c
#define CONTROL_FAN_TEMP_THRESHOLD     20.0f   // 风扇开启温度
#define CONTROL_SWITCH_SOIL_THRESHOLD 40.0f   // 水泵开启土壤湿度
```

### 3. 传感器采集周期
文件：`Core/Inc/sensor.h`
```c
#define SENSOR_READ_INTERVAL_MS    1500   // 传感器采集周期(ms)
```

### 4. OLED滚动配置
文件：`Core/Src/display.c`
```c
#define DISPLAY_SCROLL_DELAY_MS    2000   // 每页显示时间(ms)
```

## 编译与烧录

### 编译
使用Keil MDK打开 `.uvprojx` 工程文件，编译即可。

### 烧录
1. 使用ST-Link或其他调试器连接MCU
2. 在Keil中选择对应的调试器
3. 下载程序到Flash

### 串口参数
| 参数 | 值 |
|------|-----|
| 波特率 | 115200 |
| 数据位 | 8 |
| 停止位 | 1 |
| 校验位 | None |

## 使用说明

### CLI命令使用
1. 打开串口终端（如SecureCRT、XShell等）
2. 连接USART1，波特率115200
3. 按回车键显示 `$` 提示符
4. 输入 `help` 查看所有命令

### ESP数据查看
1. 确保ESP01S已正确连接USART2
2. 确保WiFi网络可用
3. ESP连接成功后会自动发送数据到服务器
4. 每发送一次数据需等待0xFF确认

## 注意事项

1. **CubeMX重新生成代码**：如使用CubeMX重新配置并生成代码，自定义任务文件（cli.c、esp.c等）会被保留，但`freertos.c`中的任务创建代码会被覆盖。需重新在`freertos.c`的`MX_FREERTOS_Init()`中添加任务创建代码。

2. **中断优先级**：DHT11使用EXTI15_10中断，需确保中断优先级配置正确。

3. **WiFi密码**：请确保WiFi名称和密码正确，否则ESP无法连接。

4. **电源要求**：ESP01S模块工作电流较大，确保电源供应充足。

## 版本信息

- FreeRTOS: v10.0.1
- HAL库: STM32F1xx_HAL
- 开发环境: Keil MDK
- 目标芯片: STM32F103C8T6
