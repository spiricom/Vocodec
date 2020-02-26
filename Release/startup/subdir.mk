################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32h743xx.s 

OBJS += \
./startup/startup_stm32h743xx.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	@echo $(PWD)
	arm-none-eabi-as -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -I/Users/jeffsnyder/dev/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I/Users/jeffsnyder/dev/Genera_H7_rev3/Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I"/Users/josnyder/dev/Vocodec/leaf/LEAF" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


