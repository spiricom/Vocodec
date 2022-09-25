################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/MIDI_application.c \
../Src/adc.c \
../Src/audiostream.c \
../Src/bdma.c \
../Src/bsp_driver_sd.c \
../Src/codec.c \
../Src/debug.c \
../Src/dma.c \
../Src/eeprom.c \
../Src/fatfs.c \
../Src/fatfs_platform.c \
../Src/fmc.c \
../Src/gfx.c \
../Src/gfx_font.c \
../Src/gpio.c \
../Src/i2c.c \
../Src/main.c \
../Src/oled.c \
../Src/rng.c \
../Src/sai.c \
../Src/sd_diskio.c \
../Src/sdmmc.c \
../Src/sfx.c \
../Src/ssd1306.c \
../Src/stm32h7xx_hal_msp.c \
../Src/stm32h7xx_it.c \
../Src/syscalls.c \
../Src/system_stm32h7xx.c \
../Src/tim.c \
../Src/tunings.c \
../Src/ui.c \
../Src/usb_host.c \
../Src/usbh_MIDI.c \
../Src/usbh_conf.c \
../Src/usbh_platform.c 

OBJS += \
./Src/MIDI_application.o \
./Src/adc.o \
./Src/audiostream.o \
./Src/bdma.o \
./Src/bsp_driver_sd.o \
./Src/codec.o \
./Src/debug.o \
./Src/dma.o \
./Src/eeprom.o \
./Src/fatfs.o \
./Src/fatfs_platform.o \
./Src/fmc.o \
./Src/gfx.o \
./Src/gfx_font.o \
./Src/gpio.o \
./Src/i2c.o \
./Src/main.o \
./Src/oled.o \
./Src/rng.o \
./Src/sai.o \
./Src/sd_diskio.o \
./Src/sdmmc.o \
./Src/sfx.o \
./Src/ssd1306.o \
./Src/stm32h7xx_hal_msp.o \
./Src/stm32h7xx_it.o \
./Src/syscalls.o \
./Src/system_stm32h7xx.o \
./Src/tim.o \
./Src/tunings.o \
./Src/ui.o \
./Src/usb_host.o \
./Src/usbh_MIDI.o \
./Src/usbh_conf.o \
./Src/usbh_platform.o 

C_DEPS += \
./Src/MIDI_application.d \
./Src/adc.d \
./Src/audiostream.d \
./Src/bdma.d \
./Src/bsp_driver_sd.d \
./Src/codec.d \
./Src/debug.d \
./Src/dma.d \
./Src/eeprom.d \
./Src/fatfs.d \
./Src/fatfs_platform.d \
./Src/fmc.d \
./Src/gfx.d \
./Src/gfx_font.d \
./Src/gpio.d \
./Src/i2c.d \
./Src/main.d \
./Src/oled.d \
./Src/rng.d \
./Src/sai.d \
./Src/sd_diskio.d \
./Src/sdmmc.d \
./Src/sfx.d \
./Src/ssd1306.d \
./Src/stm32h7xx_hal_msp.d \
./Src/stm32h7xx_it.d \
./Src/syscalls.d \
./Src/system_stm32h7xx.d \
./Src/tim.d \
./Src/tunings.d \
./Src/ui.d \
./Src/usb_host.d \
./Src/usbh_MIDI.d \
./Src/usbh_conf.d \
./Src/usbh_platform.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I"/Users/jeffsnyder/dev/Vocodec/leaf/leaf" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/MIDI_application.d ./Src/MIDI_application.o ./Src/MIDI_application.su ./Src/adc.d ./Src/adc.o ./Src/adc.su ./Src/audiostream.d ./Src/audiostream.o ./Src/audiostream.su ./Src/bdma.d ./Src/bdma.o ./Src/bdma.su ./Src/bsp_driver_sd.d ./Src/bsp_driver_sd.o ./Src/bsp_driver_sd.su ./Src/codec.d ./Src/codec.o ./Src/codec.su ./Src/debug.d ./Src/debug.o ./Src/debug.su ./Src/dma.d ./Src/dma.o ./Src/dma.su ./Src/eeprom.d ./Src/eeprom.o ./Src/eeprom.su ./Src/fatfs.d ./Src/fatfs.o ./Src/fatfs.su ./Src/fatfs_platform.d ./Src/fatfs_platform.o ./Src/fatfs_platform.su ./Src/fmc.d ./Src/fmc.o ./Src/fmc.su ./Src/gfx.d ./Src/gfx.o ./Src/gfx.su ./Src/gfx_font.d ./Src/gfx_font.o ./Src/gfx_font.su ./Src/gpio.d ./Src/gpio.o ./Src/gpio.su ./Src/i2c.d ./Src/i2c.o ./Src/i2c.su ./Src/main.d ./Src/main.o ./Src/main.su ./Src/oled.d ./Src/oled.o ./Src/oled.su ./Src/rng.d ./Src/rng.o ./Src/rng.su ./Src/sai.d ./Src/sai.o ./Src/sai.su ./Src/sd_diskio.d ./Src/sd_diskio.o ./Src/sd_diskio.su ./Src/sdmmc.d ./Src/sdmmc.o ./Src/sdmmc.su ./Src/sfx.d ./Src/sfx.o ./Src/sfx.su ./Src/ssd1306.d ./Src/ssd1306.o ./Src/ssd1306.su ./Src/stm32h7xx_hal_msp.d ./Src/stm32h7xx_hal_msp.o ./Src/stm32h7xx_hal_msp.su ./Src/stm32h7xx_it.d ./Src/stm32h7xx_it.o ./Src/stm32h7xx_it.su ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/system_stm32h7xx.d ./Src/system_stm32h7xx.o ./Src/system_stm32h7xx.su ./Src/tim.d ./Src/tim.o ./Src/tim.su ./Src/tunings.d ./Src/tunings.o ./Src/tunings.su ./Src/ui.d ./Src/ui.o ./Src/ui.su ./Src/usb_host.d ./Src/usb_host.o ./Src/usb_host.su ./Src/usbh_MIDI.d ./Src/usbh_MIDI.o ./Src/usbh_MIDI.su ./Src/usbh_conf.d ./Src/usbh_conf.o ./Src/usbh_conf.su ./Src/usbh_platform.d ./Src/usbh_platform.o ./Src/usbh_platform.su

.PHONY: clean-Src

