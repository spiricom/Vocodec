################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/MIDI_application.c \
../Src/adc.c \
../Src/audiostream.c \
../Src/bdma.c \
../Src/bsp_driver_sd.c \
../Src/codec.c \
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
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/MIDI_application.cyclo ./Src/MIDI_application.d ./Src/MIDI_application.o ./Src/MIDI_application.su ./Src/adc.cyclo ./Src/adc.d ./Src/adc.o ./Src/adc.su ./Src/audiostream.cyclo ./Src/audiostream.d ./Src/audiostream.o ./Src/audiostream.su ./Src/bdma.cyclo ./Src/bdma.d ./Src/bdma.o ./Src/bdma.su ./Src/bsp_driver_sd.cyclo ./Src/bsp_driver_sd.d ./Src/bsp_driver_sd.o ./Src/bsp_driver_sd.su ./Src/codec.cyclo ./Src/codec.d ./Src/codec.o ./Src/codec.su ./Src/dma.cyclo ./Src/dma.d ./Src/dma.o ./Src/dma.su ./Src/eeprom.cyclo ./Src/eeprom.d ./Src/eeprom.o ./Src/eeprom.su ./Src/fatfs.cyclo ./Src/fatfs.d ./Src/fatfs.o ./Src/fatfs.su ./Src/fatfs_platform.cyclo ./Src/fatfs_platform.d ./Src/fatfs_platform.o ./Src/fatfs_platform.su ./Src/fmc.cyclo ./Src/fmc.d ./Src/fmc.o ./Src/fmc.su ./Src/gfx.cyclo ./Src/gfx.d ./Src/gfx.o ./Src/gfx.su ./Src/gfx_font.cyclo ./Src/gfx_font.d ./Src/gfx_font.o ./Src/gfx_font.su ./Src/gpio.cyclo ./Src/gpio.d ./Src/gpio.o ./Src/gpio.su ./Src/i2c.cyclo ./Src/i2c.d ./Src/i2c.o ./Src/i2c.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/oled.cyclo ./Src/oled.d ./Src/oled.o ./Src/oled.su ./Src/rng.cyclo ./Src/rng.d ./Src/rng.o ./Src/rng.su ./Src/sai.cyclo ./Src/sai.d ./Src/sai.o ./Src/sai.su ./Src/sd_diskio.cyclo ./Src/sd_diskio.d ./Src/sd_diskio.o ./Src/sd_diskio.su ./Src/sdmmc.cyclo ./Src/sdmmc.d ./Src/sdmmc.o ./Src/sdmmc.su ./Src/sfx.cyclo ./Src/sfx.d ./Src/sfx.o ./Src/sfx.su ./Src/ssd1306.cyclo ./Src/ssd1306.d ./Src/ssd1306.o ./Src/ssd1306.su ./Src/stm32h7xx_hal_msp.cyclo ./Src/stm32h7xx_hal_msp.d ./Src/stm32h7xx_hal_msp.o ./Src/stm32h7xx_hal_msp.su ./Src/stm32h7xx_it.cyclo ./Src/stm32h7xx_it.d ./Src/stm32h7xx_it.o ./Src/stm32h7xx_it.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/system_stm32h7xx.cyclo ./Src/system_stm32h7xx.d ./Src/system_stm32h7xx.o ./Src/system_stm32h7xx.su ./Src/tim.cyclo ./Src/tim.d ./Src/tim.o ./Src/tim.su ./Src/tunings.cyclo ./Src/tunings.d ./Src/tunings.o ./Src/tunings.su ./Src/ui.cyclo ./Src/ui.d ./Src/ui.o ./Src/ui.su ./Src/usb_host.cyclo ./Src/usb_host.d ./Src/usb_host.o ./Src/usb_host.su ./Src/usbh_MIDI.cyclo ./Src/usbh_MIDI.d ./Src/usbh_MIDI.o ./Src/usbh_MIDI.su ./Src/usbh_conf.cyclo ./Src/usbh_conf.d ./Src/usbh_conf.o ./Src/usbh_conf.su ./Src/usbh_platform.cyclo ./Src/usbh_platform.d ./Src/usbh_platform.o ./Src/usbh_platform.su

.PHONY: clean-Src

