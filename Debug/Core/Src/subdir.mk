################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/bdma.c \
../Core/Src/debug.c \
../Core/Src/dma.c \
../Core/Src/fmc.c \
../Core/Src/gpio.c \
../Core/Src/i2c.c \
../Core/Src/main.c \
../Core/Src/rng.c \
../Core/Src/sai.c \
../Core/Src/sdmmc.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/system_stm32h7xx.c \
../Core/Src/tim.c 

OBJS += \
./Core/Src/adc.o \
./Core/Src/bdma.o \
./Core/Src/debug.o \
./Core/Src/dma.o \
./Core/Src/fmc.o \
./Core/Src/gpio.o \
./Core/Src/i2c.o \
./Core/Src/main.o \
./Core/Src/rng.o \
./Core/Src/sai.o \
./Core/Src/sdmmc.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/system_stm32h7xx.o \
./Core/Src/tim.o 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/bdma.d \
./Core/Src/debug.d \
./Core/Src/dma.d \
./Core/Src/fmc.d \
./Core/Src/gpio.d \
./Core/Src/i2c.d \
./Core/Src/main.d \
./Core/Src/rng.d \
./Core/Src/sai.d \
./Core/Src/sdmmc.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/system_stm32h7xx.d \
./Core/Src/tim.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -DUSE_HAL_DRIVER -DSTM32H743xx -I"/Users/josnyder/dev/Vocodec/Core/Inc" -I"/Users/josnyder/dev/Vocodec/Drivers/STM32H7xx_HAL_Driver/Inc" -I"/Users/josnyder/dev/Vocodec/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy" -I"/Users/josnyder/dev/Vocodec/Middlewares/Third_Party/FatFs/src" -I"/Users/josnyder/dev/Vocodec/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"/Users/josnyder/dev/Vocodec/Drivers/CMSIS/Include" -I"/Users/josnyder/dev/Vocodec/leaf" -I"/Users/josnyder/dev/Vocodec/leaf/LEAF" -I/Users/jeffsnyder/dev/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I/Users/jeffsnyder/dev/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I"/Users/josnyder/dev/Vocodec/Middlewares/ST/STM32_USB_Host_Library/Core/Inc" -I"/Users/josnyder/dev/Vocodec/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc" -I/Users/Matthew/Desktop/PrincetonL/Vocodec/leaf -I"/Users/josnyder/dev/Vocodec/FATFS/Target" -I"/Users/josnyder/dev/Vocodec/FATFS/App"  -O3 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


