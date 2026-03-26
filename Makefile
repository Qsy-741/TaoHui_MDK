# Makefile for STM32F103C8T6 project

# Tools
CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

# Project name
PROJECT = stm32f103c8t6_freertos

# Target device
MCU = cortex-m3

# Flags
CFLAGS = -c -mthumb -mcpu=$(MCU) -std=c11 -Wall -O0 -g3 -DSTM32F103xB -DUSE_HAL_DRIVER
CXXFLAGS = -c -mthumb -mcpu=$(MCU) -std=c++17 -Wall -O0 -g3 -DSTM32F103xB -DUSE_HAL_DRIVER
ASFLAGS = -mthumb -mcpu=$(MCU)
LDFLAGS = -mthumb -mcpu=$(MCU) -T"STM32F103C8Tx_FLASH.ld" --specs=nosys.specs -Wl,-Map=$(PROJECT).map

# Include directories
INCLUDES = \
    -I"Core/Inc" \
    -I"Drivers/CMSIS/Core/Include" \
    -I"Drivers/CMSIS/Device/ST/STM32F1xx/Include" \
    -I"Drivers/STM32F1xx_HAL_Driver/Inc" \
    -I"Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" \
    -I"Middlewares/Third_Party/FreeRTOS/Source/include" \
    -I"Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" \
    -I"Middlewares/Third_Party/FreeRTOS/Source/portable/RVDS/ARM_CM3"

# Source files
SRCS = \
    Core/Src/main.c \
    Core/Src/freertos.c \
    Core/Src/gpio.c \
    Core/Src/i2c.c \
    Core/Src/ssd1306.c \
    Core/Src/ssd1306_fonts.c \
    Core/Src/ssd1306_tests.c \
    Core/Src/stm32f1xx_hal_msp.c \
    Core/Src/stm32f1xx_hal_timebase_tim.c \
    Core/Src/stm32f1xx_it.c \
    Core/Src/system_stm32f1xx.c \
    Core/Src/usart.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c \
    Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c \
    Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
    Middlewares/Third_Party/FreeRTOS/Source/event_groups.c \
    Middlewares/Third_Party/FreeRTOS/Source/list.c \
    Middlewares/Third_Party/FreeRTOS/Source/queue.c \
    Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c \
    Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
    Middlewares/Third_Party/FreeRTOS/Source/timers.c \
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c \
    Middlewares/Third_Party/FreeRTOS/Source/portable/RVDS/ARM_CM3/port.c \
    Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
    MDK-ARM/startup_stm32f103xb.s

# Object files
OBJS = $(SRCS:.c=.o)

# Build target
all: $(PROJECT).elf $(PROJECT).hex $(PROJECT).bin

# Link
$(PROJECT).elf: $(OBJS)
    $(CC) $(LDFLAGS) -o $@ $^
    $(SIZE) $@

# Compile C files
%.o: %.c
    $(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

# Compile C++ files
%.o: %.cpp
    $(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

# Compile assembly files
%.o: %.s
    $(AS) $(ASFLAGS) -o $@ $<

# Create hex file
$(PROJECT).hex: $(PROJECT).elf
    $(OBJCOPY) -O ihex $< $@

# Create bin file
$(PROJECT).bin: $(PROJECT).elf
    $(OBJCOPY) -O binary $< $@

# Clean
clean:
    rm -f $(OBJS) $(PROJECT).elf $(PROJECT).hex $(PROJECT).bin $(PROJECT).map

# Phony targets
.PHONY: all clean