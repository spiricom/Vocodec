################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FATFS/Target/bsp_driver_sd.c \
../FATFS/Target/fatfs_platform.c \
../FATFS/Target/sd_diskio.c 

OBJS += \
./FATFS/Target/bsp_driver_sd.o \
./FATFS/Target/fatfs_platform.o \
./FATFS/Target/sd_diskio.o 

C_DEPS += \
./FATFS/Target/bsp_driver_sd.d \
./FATFS/Target/fatfs_platform.d \
./FATFS/Target/sd_diskio.d 


# Each subdirectory must supply rules for building sources it contributes
FATFS/Target/%.o: ../FATFS/Target/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -DUSE_HAL_DRIVER -DSTM32H743xx -I"/Users/josnyder/dev/Vocodec/Core/Inc" -I"/Users/josnyder/dev/Vocodec/Drivers/STM32H7xx_HAL_Driver/Inc" -I"/Users/josnyder/dev/Vocodec/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy" -I"/Users/josnyder/dev/Vocodec/Middlewares/Third_Party/FatFs/src" -I"/Users/josnyder/dev/Vocodec/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"/Users/josnyder/dev/Vocodec/Drivers/CMSIS/Include" -I"/Users/josnyder/dev/Vocodec/leaf" -I"/Users/josnyder/dev/Vocodec/leaf/LEAF" -I/Users/jeffsnyder/dev/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I/Users/jeffsnyder/dev/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I"/Users/josnyder/dev/Vocodec/Middlewares/ST/STM32_USB_Host_Library/Core/Inc" -I"/Users/josnyder/dev/Vocodec/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc" -I/Users/Matthew/Desktop/PrincetonL/Vocodec/leaf -I"/Users/josnyder/dev/Vocodec/FATFS/Target" -I"/Users/josnyder/dev/Vocodec/FATFS/App"  -O3 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


