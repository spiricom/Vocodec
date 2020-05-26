/*
 * MIDI_application.c
 *
 *  Created on: 6 d�c. 2014
 *      Author: Xavier Halgand
 *
 *	Modified on: 9/12/16 by C.P. to handle the MIDI_IDLE state properly, and
 *	added required code to be compatible with "NucleoSynth"
 *
 *	11/8/17 by C.P.: Version 0.7.7 - Use for Casio CTK-6200 Keyboard
 */

/* Includes ------------------------------------------------------------------*/
#include "MIDI_application.h"
#include "usb_host.h"

uint8_t MIDI_RX_Buffer[2][RX_BUFF_SIZE] __ATTR_RAM_D2; // MIDI reception buffer
uint8_t MIDI_read_buffer = 0;
uint8_t MIDI_write_buffer = 1;
uint8_t key, velocity, ctrl, data, sustainInverted;
uint8_t USB_message[4];
uint8_t CCs[128];
/* Private define ------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/




/*-----------------------------------------------------------------------------*/
/**
 * @brief  Main routine for MIDI application, looped in main.c
 * @param  None
 * @retval none
 */
void MIDI_Application(void)
{
	if(Appli_state == APPLICATION_READY)
	{
		USBH_MIDI_Receive(&hUsbHostFS, MIDI_RX_Buffer[MIDI_write_buffer], RX_BUFF_SIZE); // just once at the beginning, start the first reception
		Appli_state = APPLICATION_RUNNING;
	}
	if(Appli_state == APPLICATION_RUNNING)
	{
			//....pffff......grrrrr......
	}

	if(Appli_state == APPLICATION_DISCONNECT)
	{
		Appli_state = APPLICATION_IDLE;
		USBH_MIDI_Stop(&hUsbHostFS);
		HAL_Delay(10);
		MX_USB_HOST_DeInit();
		HAL_Delay(10);
		MX_USB_HOST_Init();

	}

}





void parse_MIDI_Message(void)
{
	// Handle MIDI messages
	switch(USB_message[1])
	{
		case (0x80): // Note Off
			key = USB_message[2];
			velocity = USB_message[3];

			noteOff(key, velocity);

			break;
		case (0x90): // Note On
			key = USB_message[2];
			velocity = USB_message[3];

			noteOn(key, velocity);

			break;
		case (0xA0):
			break;
		case (0xB0):
			ctrl = USB_message[2];
			data = USB_message[3];
			CCs[ctrl] = data;
			switch(ctrl)
			{
				case (64): // sustain
					if (data)
					{
						if (sustainInverted) 	sustainOff();
						else					sustainOn();
					}
					else
					{
						if (sustainInverted) 	sustainOn();
						else					sustainOff();
					}
					break;
			}


  break;
		case (0xC0): // Program Change
			break;
		case (0xD0): // Mono Aftertouch
			break;
		case (0xE0): // Pitch Bend
			pitchBend((USB_message[2]) + (USB_message[3] << 7));
			break;
	}

}

/*-----------------------------------------------------------------------------*/
void ProcessReceivedMidiDatas(void)
{
	uint8_t miniBufferPosition = 0;
	uint8_t processed = 0;
	while (((myUSB_FIFO_writePointer > myUSB_FIFO_readPointer) || (myUSB_FIFO_overflowBit)) &&
			(processed < 32)) // maximum notes to process in a frame * 4
	{

		miniBufferPosition = (myUSB_FIFO_readPointer % 4);
		USB_message[miniBufferPosition] = myUSB_FIFO[myUSB_FIFO_readPointer];
		myUSB_FIFO_readPointer++;
		if (miniBufferPosition == 3) //we must have reached the end of a 4-byte message
		{
			parse_MIDI_Message();
		}
		if (myUSB_FIFO_readPointer >= USB_FIFO_SIZE)
		{
			myUSB_FIFO_overflowBit = 0;
			myUSB_FIFO_readPointer = 0;
		}
		processed++;
	}
}



/*-----------------------------------------------------------------------------*/
/**
 * @brief  MIDI data receive callback.
 * @param  phost: Host handle
 * @retval None
 */
void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost)
{

}

