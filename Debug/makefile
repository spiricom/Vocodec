################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include leaf/leaf/Src/subdir.mk
-include leaf/leaf/Externals/subdir.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include Middlewares/Third_Party/FatFs/src/option/subdir.mk
-include Middlewares/Third_Party/FatFs/src/subdir.mk
-include Middlewares/ST/STM32_USB_Host_Library/Core/Src/subdir.mk
-include Drivers/STM32H7xx_HAL_Driver/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
cube_for_new_genera_brain4.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
cube_for_new_genera_brain4.list \

OBJCOPY_BIN += \
cube_for_new_genera_brain4.bin \


# All Target
all: cube_for_new_genera_brain4.elf secondary-outputs

# Tool invocations
cube_for_new_genera_brain4.elf: $(OBJS) $(USER_OBJS) /Users/josnyder/dev/Vocodec/STM32H743ZITX_FLASH.ld
	arm-none-eabi-gcc -o "cube_for_new_genera_brain4.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m7 -T"/Users/josnyder/dev/Vocodec/STM32H743ZITX_FLASH.ld" --specs=nosys.specs -Wl,-Map="cube_for_new_genera_brain4.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

cube_for_new_genera_brain4.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "cube_for_new_genera_brain4.list"
	@echo 'Finished building: $@'
	@echo ' '

cube_for_new_genera_brain4.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "cube_for_new_genera_brain4.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
