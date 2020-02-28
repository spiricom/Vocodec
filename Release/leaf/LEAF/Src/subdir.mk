################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../leaf/LEAF/Src/leaf-analysis.c \
../leaf/LEAF/Src/leaf-delay.c \
../leaf/LEAF/Src/leaf-distortion.c \
../leaf/LEAF/Src/leaf-dynamics.c \
../leaf/LEAF/Src/leaf-effects.c \
../leaf/LEAF/Src/leaf-electrical.c \
../leaf/LEAF/Src/leaf-envelopes.c \
../leaf/LEAF/Src/leaf-filters.c \
../leaf/LEAF/Src/leaf-instruments.c \
../leaf/LEAF/Src/leaf-math.c \
../leaf/LEAF/Src/leaf-mempool.c \
../leaf/LEAF/Src/leaf-midi.c \
../leaf/LEAF/Src/leaf-oscillators.c \
../leaf/LEAF/Src/leaf-physical.c \
../leaf/LEAF/Src/leaf-reverb.c \
../leaf/LEAF/Src/leaf-sampling.c \
../leaf/LEAF/Src/leaf-tables.c \
../leaf/LEAF/Src/leaf.c 

OBJS += \
./leaf/LEAF/Src/leaf-analysis.o \
./leaf/LEAF/Src/leaf-delay.o \
./leaf/LEAF/Src/leaf-distortion.o \
./leaf/LEAF/Src/leaf-dynamics.o \
./leaf/LEAF/Src/leaf-effects.o \
./leaf/LEAF/Src/leaf-electrical.o \
./leaf/LEAF/Src/leaf-envelopes.o \
./leaf/LEAF/Src/leaf-filters.o \
./leaf/LEAF/Src/leaf-instruments.o \
./leaf/LEAF/Src/leaf-math.o \
./leaf/LEAF/Src/leaf-mempool.o \
./leaf/LEAF/Src/leaf-midi.o \
./leaf/LEAF/Src/leaf-oscillators.o \
./leaf/LEAF/Src/leaf-physical.o \
./leaf/LEAF/Src/leaf-reverb.o \
./leaf/LEAF/Src/leaf-sampling.o \
./leaf/LEAF/Src/leaf-tables.o \
./leaf/LEAF/Src/leaf.o 

C_DEPS += \
./leaf/LEAF/Src/leaf-analysis.d \
./leaf/LEAF/Src/leaf-delay.d \
./leaf/LEAF/Src/leaf-distortion.d \
./leaf/LEAF/Src/leaf-dynamics.d \
./leaf/LEAF/Src/leaf-effects.d \
./leaf/LEAF/Src/leaf-electrical.d \
./leaf/LEAF/Src/leaf-envelopes.d \
./leaf/LEAF/Src/leaf-filters.d \
./leaf/LEAF/Src/leaf-instruments.d \
./leaf/LEAF/Src/leaf-math.d \
./leaf/LEAF/Src/leaf-mempool.d \
./leaf/LEAF/Src/leaf-midi.d \
./leaf/LEAF/Src/leaf-oscillators.d \
./leaf/LEAF/Src/leaf-physical.d \
./leaf/LEAF/Src/leaf-reverb.d \
./leaf/LEAF/Src/leaf-sampling.d \
./leaf/LEAF/Src/leaf-tables.d \
./leaf/LEAF/Src/leaf.d 


# Each subdirectory must supply rules for building sources it contributes
leaf/LEAF/Src/%.o: ../leaf/LEAF/Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32H743xx -I"/Users/josnyder/dev/Vocodec/Inc" -I"/Users/josnyder/dev/Vocodec/Drivers/STM32H7xx_HAL_Driver/Inc" -I"/Users/josnyder/dev/Vocodec/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy" -I"/Users/josnyder/dev/Vocodec/Middlewares/Third_Party/FatFs/src" -I"/Users/josnyder/dev/Vocodec/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"/Users/josnyder/dev/Vocodec/Drivers/CMSIS/Include" -I"/Users/josnyder/dev/Vocodec/leaf" -I"/Users/josnyder/dev/Vocodec/leaf/LEAF" -I/Users/Matthew/Desktop/Princeton/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I/Users/Matthew/Desktop/Princeton/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I/Users/jeffsnyder/dev/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I/Users/jeffsnyder/dev/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I"/Users/josnyder/dev/Vocodec/Middlewares/ST/STM32_USB_Host_Library/Core/Inc" -I"/Users/josnyder/dev/Vocodec/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


