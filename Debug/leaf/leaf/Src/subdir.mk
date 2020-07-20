################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../leaf/leaf/Src/leaf-analysis.c \
../leaf/leaf/Src/leaf-delay.c \
../leaf/leaf/Src/leaf-distortion.c \
../leaf/leaf/Src/leaf-dynamics.c \
../leaf/leaf/Src/leaf-effects.c \
../leaf/leaf/Src/leaf-electrical.c \
../leaf/leaf/Src/leaf-envelopes.c \
../leaf/leaf/Src/leaf-filters.c \
../leaf/leaf/Src/leaf-instruments.c \
../leaf/leaf/Src/leaf-math.c \
../leaf/leaf/Src/leaf-mempool.c \
../leaf/leaf/Src/leaf-midi.c \
../leaf/leaf/Src/leaf-oscillators.c \
../leaf/leaf/Src/leaf-physical.c \
../leaf/leaf/Src/leaf-reverb.c \
../leaf/leaf/Src/leaf-sampling.c \
../leaf/leaf/Src/leaf-tables.c \
../leaf/leaf/Src/leaf.c 

OBJS += \
./leaf/leaf/Src/leaf-analysis.o \
./leaf/leaf/Src/leaf-delay.o \
./leaf/leaf/Src/leaf-distortion.o \
./leaf/leaf/Src/leaf-dynamics.o \
./leaf/leaf/Src/leaf-effects.o \
./leaf/leaf/Src/leaf-electrical.o \
./leaf/leaf/Src/leaf-envelopes.o \
./leaf/leaf/Src/leaf-filters.o \
./leaf/leaf/Src/leaf-instruments.o \
./leaf/leaf/Src/leaf-math.o \
./leaf/leaf/Src/leaf-mempool.o \
./leaf/leaf/Src/leaf-midi.o \
./leaf/leaf/Src/leaf-oscillators.o \
./leaf/leaf/Src/leaf-physical.o \
./leaf/leaf/Src/leaf-reverb.o \
./leaf/leaf/Src/leaf-sampling.o \
./leaf/leaf/Src/leaf-tables.o \
./leaf/leaf/Src/leaf.o 

C_DEPS += \
./leaf/leaf/Src/leaf-analysis.d \
./leaf/leaf/Src/leaf-delay.d \
./leaf/leaf/Src/leaf-distortion.d \
./leaf/leaf/Src/leaf-dynamics.d \
./leaf/leaf/Src/leaf-effects.d \
./leaf/leaf/Src/leaf-electrical.d \
./leaf/leaf/Src/leaf-envelopes.d \
./leaf/leaf/Src/leaf-filters.d \
./leaf/leaf/Src/leaf-instruments.d \
./leaf/leaf/Src/leaf-math.d \
./leaf/leaf/Src/leaf-mempool.d \
./leaf/leaf/Src/leaf-midi.d \
./leaf/leaf/Src/leaf-oscillators.d \
./leaf/leaf/Src/leaf-physical.d \
./leaf/leaf/Src/leaf-reverb.d \
./leaf/leaf/Src/leaf-sampling.d \
./leaf/leaf/Src/leaf-tables.d \
./leaf/leaf/Src/leaf.d 


# Each subdirectory must supply rules for building sources it contributes
leaf/leaf/Src/leaf-analysis.o: ../leaf/leaf/Src/leaf-analysis.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-analysis.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-delay.o: ../leaf/leaf/Src/leaf-delay.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-delay.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-distortion.o: ../leaf/leaf/Src/leaf-distortion.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-distortion.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-dynamics.o: ../leaf/leaf/Src/leaf-dynamics.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-dynamics.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-effects.o: ../leaf/leaf/Src/leaf-effects.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-effects.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-electrical.o: ../leaf/leaf/Src/leaf-electrical.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-electrical.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-envelopes.o: ../leaf/leaf/Src/leaf-envelopes.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-envelopes.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-filters.o: ../leaf/leaf/Src/leaf-filters.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-filters.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-instruments.o: ../leaf/leaf/Src/leaf-instruments.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-instruments.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-math.o: ../leaf/leaf/Src/leaf-math.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-math.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-mempool.o: ../leaf/leaf/Src/leaf-mempool.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-mempool.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-midi.o: ../leaf/leaf/Src/leaf-midi.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-midi.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-oscillators.o: ../leaf/leaf/Src/leaf-oscillators.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-oscillators.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-physical.o: ../leaf/leaf/Src/leaf-physical.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-physical.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-reverb.o: ../leaf/leaf/Src/leaf-reverb.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-reverb.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-sampling.o: ../leaf/leaf/Src/leaf-sampling.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-sampling.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf-tables.o: ../leaf/leaf/Src/leaf-tables.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf-tables.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
leaf/leaf/Src/leaf.o: ../leaf/leaf/Src/leaf.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32H743xx -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/josnyder/dev/Vocodec/leaf/leaf" -Ofast -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -fstack-usage -MMD -MP -MF"leaf/leaf/Src/leaf.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

