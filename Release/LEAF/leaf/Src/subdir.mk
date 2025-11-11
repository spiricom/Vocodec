################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-analysis.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-delay.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-distortion.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-dynamics.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-effects.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-electrical.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-envelopes.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-filters.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-instruments.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-math.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-mempool.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-midi.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-oscillators.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-physical.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-reverb.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-sampling.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-tables.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf-vocal.c \
/Users/josnyder/dev/LEAF/leaf/Src/leaf.c 

OBJS += \
./LEAF/leaf/Src/leaf-analysis.o \
./LEAF/leaf/Src/leaf-delay.o \
./LEAF/leaf/Src/leaf-distortion.o \
./LEAF/leaf/Src/leaf-dynamics.o \
./LEAF/leaf/Src/leaf-effects.o \
./LEAF/leaf/Src/leaf-electrical.o \
./LEAF/leaf/Src/leaf-envelopes.o \
./LEAF/leaf/Src/leaf-filters.o \
./LEAF/leaf/Src/leaf-instruments.o \
./LEAF/leaf/Src/leaf-math.o \
./LEAF/leaf/Src/leaf-mempool.o \
./LEAF/leaf/Src/leaf-midi.o \
./LEAF/leaf/Src/leaf-oscillators.o \
./LEAF/leaf/Src/leaf-physical.o \
./LEAF/leaf/Src/leaf-reverb.o \
./LEAF/leaf/Src/leaf-sampling.o \
./LEAF/leaf/Src/leaf-tables.o \
./LEAF/leaf/Src/leaf-vocal.o \
./LEAF/leaf/Src/leaf.o 

C_DEPS += \
./LEAF/leaf/Src/leaf-analysis.d \
./LEAF/leaf/Src/leaf-delay.d \
./LEAF/leaf/Src/leaf-distortion.d \
./LEAF/leaf/Src/leaf-dynamics.d \
./LEAF/leaf/Src/leaf-effects.d \
./LEAF/leaf/Src/leaf-electrical.d \
./LEAF/leaf/Src/leaf-envelopes.d \
./LEAF/leaf/Src/leaf-filters.d \
./LEAF/leaf/Src/leaf-instruments.d \
./LEAF/leaf/Src/leaf-math.d \
./LEAF/leaf/Src/leaf-mempool.d \
./LEAF/leaf/Src/leaf-midi.d \
./LEAF/leaf/Src/leaf-oscillators.d \
./LEAF/leaf/Src/leaf-physical.d \
./LEAF/leaf/Src/leaf-reverb.d \
./LEAF/leaf/Src/leaf-sampling.d \
./LEAF/leaf/Src/leaf-tables.d \
./LEAF/leaf/Src/leaf-vocal.d \
./LEAF/leaf/Src/leaf.d 


# Each subdirectory must supply rules for building sources it contributes
LEAF/leaf/Src/leaf-analysis.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-analysis.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-delay.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-delay.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-distortion.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-distortion.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-dynamics.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-dynamics.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-effects.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-effects.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-electrical.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-electrical.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-envelopes.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-envelopes.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-filters.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-filters.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-instruments.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-instruments.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-math.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-math.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-mempool.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-mempool.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-midi.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-midi.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-oscillators.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-oscillators.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-physical.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-physical.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-reverb.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-reverb.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-sampling.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-sampling.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-tables.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-tables.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf-vocal.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf-vocal.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
LEAF/leaf/Src/leaf.o: /Users/josnyder/dev/LEAF/leaf/Src/leaf.c LEAF/leaf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I"/Users/josnyder/dev/LEAF/leaf" -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LEAF-2f-leaf-2f-Src

clean-LEAF-2f-leaf-2f-Src:
	-$(RM) ./LEAF/leaf/Src/leaf-analysis.cyclo ./LEAF/leaf/Src/leaf-analysis.d ./LEAF/leaf/Src/leaf-analysis.o ./LEAF/leaf/Src/leaf-analysis.su ./LEAF/leaf/Src/leaf-delay.cyclo ./LEAF/leaf/Src/leaf-delay.d ./LEAF/leaf/Src/leaf-delay.o ./LEAF/leaf/Src/leaf-delay.su ./LEAF/leaf/Src/leaf-distortion.cyclo ./LEAF/leaf/Src/leaf-distortion.d ./LEAF/leaf/Src/leaf-distortion.o ./LEAF/leaf/Src/leaf-distortion.su ./LEAF/leaf/Src/leaf-dynamics.cyclo ./LEAF/leaf/Src/leaf-dynamics.d ./LEAF/leaf/Src/leaf-dynamics.o ./LEAF/leaf/Src/leaf-dynamics.su ./LEAF/leaf/Src/leaf-effects.cyclo ./LEAF/leaf/Src/leaf-effects.d ./LEAF/leaf/Src/leaf-effects.o ./LEAF/leaf/Src/leaf-effects.su ./LEAF/leaf/Src/leaf-electrical.cyclo ./LEAF/leaf/Src/leaf-electrical.d ./LEAF/leaf/Src/leaf-electrical.o ./LEAF/leaf/Src/leaf-electrical.su ./LEAF/leaf/Src/leaf-envelopes.cyclo ./LEAF/leaf/Src/leaf-envelopes.d ./LEAF/leaf/Src/leaf-envelopes.o ./LEAF/leaf/Src/leaf-envelopes.su ./LEAF/leaf/Src/leaf-filters.cyclo ./LEAF/leaf/Src/leaf-filters.d ./LEAF/leaf/Src/leaf-filters.o ./LEAF/leaf/Src/leaf-filters.su ./LEAF/leaf/Src/leaf-instruments.cyclo ./LEAF/leaf/Src/leaf-instruments.d ./LEAF/leaf/Src/leaf-instruments.o ./LEAF/leaf/Src/leaf-instruments.su ./LEAF/leaf/Src/leaf-math.cyclo ./LEAF/leaf/Src/leaf-math.d ./LEAF/leaf/Src/leaf-math.o ./LEAF/leaf/Src/leaf-math.su ./LEAF/leaf/Src/leaf-mempool.cyclo ./LEAF/leaf/Src/leaf-mempool.d ./LEAF/leaf/Src/leaf-mempool.o ./LEAF/leaf/Src/leaf-mempool.su ./LEAF/leaf/Src/leaf-midi.cyclo ./LEAF/leaf/Src/leaf-midi.d ./LEAF/leaf/Src/leaf-midi.o ./LEAF/leaf/Src/leaf-midi.su ./LEAF/leaf/Src/leaf-oscillators.cyclo ./LEAF/leaf/Src/leaf-oscillators.d ./LEAF/leaf/Src/leaf-oscillators.o ./LEAF/leaf/Src/leaf-oscillators.su ./LEAF/leaf/Src/leaf-physical.cyclo ./LEAF/leaf/Src/leaf-physical.d ./LEAF/leaf/Src/leaf-physical.o ./LEAF/leaf/Src/leaf-physical.su ./LEAF/leaf/Src/leaf-reverb.cyclo ./LEAF/leaf/Src/leaf-reverb.d ./LEAF/leaf/Src/leaf-reverb.o ./LEAF/leaf/Src/leaf-reverb.su ./LEAF/leaf/Src/leaf-sampling.cyclo ./LEAF/leaf/Src/leaf-sampling.d ./LEAF/leaf/Src/leaf-sampling.o ./LEAF/leaf/Src/leaf-sampling.su ./LEAF/leaf/Src/leaf-tables.cyclo ./LEAF/leaf/Src/leaf-tables.d ./LEAF/leaf/Src/leaf-tables.o ./LEAF/leaf/Src/leaf-tables.su ./LEAF/leaf/Src/leaf-vocal.cyclo ./LEAF/leaf/Src/leaf-vocal.d ./LEAF/leaf/Src/leaf-vocal.o ./LEAF/leaf/Src/leaf-vocal.su ./LEAF/leaf/Src/leaf.cyclo ./LEAF/leaf/Src/leaf.d ./LEAF/leaf/Src/leaf.o ./LEAF/leaf/Src/leaf.su

.PHONY: clean-LEAF-2f-leaf-2f-Src

