/*
 * MIDI_application.h
 *
 *  Created on: 6 déc. 2014
 *      Author: CNous
 */

#ifndef MIDI_APPLICATION_H_
#define MIDI_APPLICATION_H_

/* Includes ------------------------------------------------------------------*/

#include "stdio.h"
#include "usbh_core.h"
#include "usbh_MIDI.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#define RX_BUFF_SIZE   64  /* Max Received data 64 bytes */

/*------------------------------------------------------------------------------*/
typedef enum
{
	APPLICATION_IDLE = 0,
	APPLICATION_START,
	APPLICATION_READY,
	APPLICATION_RUNNING,
	APPLICATION_DISCONNECT
}
MIDI_ApplicationTypeDef;

/*------------------------------------------------------------------------------*/
extern USBH_HandleTypeDef hUsbHostFS;
extern MIDI_ApplicationTypeDef Appli_state;
extern uint8_t MIDI_RX_Buffer[2][RX_BUFF_SIZE] __ATTR_RAM_D2; // MIDI reception buffer
extern int MIDI_read_buffer;
extern int MIDI_write_buffer;
/* Exported functions ------------------------------------------------------- */

void MIDI_Application(void);
void ProcessReceivedMidiDatas(void);

/*------------------------------------------------------------------------------*/
#endif /* MIDI_APPLICATION_H_ */
