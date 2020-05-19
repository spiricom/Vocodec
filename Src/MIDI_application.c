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
		HAL_Delay(100);
		MX_USB_HOST_DeInit();
		HAL_Delay(100);
		MX_USB_HOST_Init();

	}

}

/*-----------------------------------------------------------------------------*/
void ProcessReceivedMidiDatas(void)
{
	uint16_t numberOfPackets;

	uint8_t whichPacket = 0;

	numberOfPackets = USBH_MIDI_GetLastReceivedDataSize(&hUsbHostFS) >> 2; //each USB midi package is 4 bytes long

	if (numberOfPackets != 0)
	{
		while(numberOfPackets--)
		{
			// Handle MIDI messages
			switch(MIDI_RX_Buffer[MIDI_read_buffer][whichPacket+1])
			{
				case (0x80): // Note Off
					key = MIDI_RX_Buffer[MIDI_read_buffer][whichPacket+2];
					velocity = MIDI_RX_Buffer[MIDI_read_buffer][whichPacket+3];

					noteOff(key, velocity);

					break;
				case (0x90): // Note On
					key = MIDI_RX_Buffer[MIDI_read_buffer][whichPacket+2];
					velocity = MIDI_RX_Buffer[MIDI_read_buffer][whichPacket+3];

					noteOn(key, velocity);

					break;
				case (0xA0):
					break;
				case (0xB0):
					ctrl = MIDI_RX_Buffer[MIDI_read_buffer][whichPacket+2];
					data = MIDI_RX_Buffer[MIDI_read_buffer][whichPacket+3];
					CCs[ctrl] = data;
					switch(ctrl)
					{
						case (0x01):
							break;
						case (0x02):
							break;
						case (0x03):
							break;
						case (0x04):
							break;
						case (0x0D):
							break;
						case (0x4B):
							break;
						case (0x4C):
							break;
						case (0x5C):
							break;
						case (0x5F):
							break;
						case (0x49):
							break;
						case (0x48):
							break;
						case (0x5B):
							break;
						case (0x5D):
							break;
						case (0x4A):
							break;
						case (0x47):
							break;
						case (0x05):
							break;
						case (0x54):
							break;
						case (0x10):
							break;
						case (0x11):
							break;
						case (0x12):
							break;
						case (0x07):
							break;
						case (0x13):
							break;
						case (0x14):
							break;
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
					pitchBend((MIDI_RX_Buffer[MIDI_read_buffer][whichPacket+2]) + (MIDI_RX_Buffer[MIDI_read_buffer][whichPacket+3] << 7));
					break;
				case (0xF0):
					break;
			}
			whichPacket = (whichPacket + 4);

		}
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
	MIDI_write_buffer = !MIDI_write_buffer;
	MIDI_read_buffer = !MIDI_read_buffer; //switch buffers for double buffer fun
	ProcessReceivedMidiDatas();
	USBH_MIDI_Receive(&hUsbHostFS, &MIDI_RX_Buffer[MIDI_write_buffer], RX_BUFF_SIZE); // start a new reception
}

