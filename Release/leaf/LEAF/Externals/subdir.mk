################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../leaf/LEAF/Externals/d_fft_mayer.c 

OBJS += \
./leaf/LEAF/Externals/d_fft_mayer.o 

C_DEPS += \
./leaf/LEAF/Externals/d_fft_mayer.d 


# Each subdirectory must supply rules for building sources it contributes
leaf/LEAF/Externals/%.o: ../leaf/LEAF/Externals/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32H743xx -I"/Users/josnyder/dev/Vocodec/Inc" -I"/Users/josnyder/dev/Vocodec/Drivers/STM32H7xx_HAL_Driver/Inc" -I"/Users/josnyder/dev/Vocodec/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy" -I"/Users/josnyder/dev/Vocodec/Middlewares/Third_Party/FatFs/src" -I"/Users/josnyder/dev/Vocodec/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"/Users/josnyder/dev/Vocodec/Drivers/CMSIS/Include" -I"/Users/josnyder/dev/Vocodec/leaf" -I"/Users/josnyder/dev/Vocodec/leaf/LEAF" -I/Users/Matthew/Desktop/Princeton/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I/Users/Matthew/Desktop/Princeton/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I/Users/jeffsnyder/dev/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I/Users/jeffsnyder/dev/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I"/Users/josnyder/dev/Vocodec/Middlewares/ST/STM32_USB_Host_Library/Core/Inc" -I"/Users/josnyder/dev/Vocodec/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


