################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/josnyder/dev/LEAF/leaf/Externals/d_fft_mayer.c 

OBJS += \
./LEAF/leaf/Externals/d_fft_mayer.o 

C_DEPS += \
./LEAF/leaf/Externals/d_fft_mayer.d 


# Each subdirectory must supply rules for building sources it contributes
LEAF/leaf/Externals/d_fft_mayer.o: /Users/josnyder/dev/LEAF/leaf/Externals/d_fft_mayer.c LEAF/leaf/Externals/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LEAF-2f-leaf-2f-Externals

clean-LEAF-2f-leaf-2f-Externals:
	-$(RM) ./LEAF/leaf/Externals/d_fft_mayer.cyclo ./LEAF/leaf/Externals/d_fft_mayer.d ./LEAF/leaf/Externals/d_fft_mayer.o ./LEAF/leaf/Externals/d_fft_mayer.su

.PHONY: clean-LEAF-2f-leaf-2f-Externals

