/**
 ******************************************************************************
 * @file    usbh_MIDI.c
 * @author	Xavier Halgand
 * @version
 * @date
 * @brief   This file is the MIDI Layer Handlers for USB Host MIDI streaming class.
 *
 *
 ******************************************************************************
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
 
 /* WE MADE MODIFICATIONS TO PACTIVE CLASS SETTING IN USBH_CORE.C */

/* Includes ------------------------------------------------------------------*/
#include "usbh_MIDI.h"
#include "usb_host.h"
#include "oled.h"
#include "MIDI_Application.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

/** @defgroup USBH_MIDI_CORE_Private_FunctionPrototypes
 * @{
 */

static USBH_StatusTypeDef USBH_MIDI_InterfaceInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_MIDI_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_MIDI_ClassRequest (USBH_HandleTypeDef *phost);

static void MIDI_ProcessTransmission(USBH_HandleTypeDef *phost);

static void MIDI_ProcessReception(USBH_HandleTypeDef *phost);

/*-------------------------------------------------------------------------*/

USBH_ClassTypeDef  MIDI_Class =
{
		"MIDI",
		USB_AUDIO_CLASS,
		USBH_MIDI_InterfaceInit,
		USBH_MIDI_InterfaceDeInit,
		USBH_MIDI_ClassRequest,
		USBH_MIDI_Process, // background process called in HOST_CLASS state (core state machine)
		USBH_MIDI_SOFProcess,
		NULL // MIDI handle structure
};


uint8_t USB_status_waiting = 0;

/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  USBH_MIDI_InterfaceInit
 *         The function init the MIDI class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_MIDI_InterfaceInit (USBH_HandleTypeDef *phost)
{	

	USBH_StatusTypeDef status = USBH_FAIL ;
	uint8_t interface = 0;
	MIDI_HandleTypeDef *MIDI_Handle;

	//USB_MIDI_ChangeConnectionState(0);

	HAL_Delay(100);
	interface = USBH_FindInterface(phost, USB_AUDIO_CLASS, USB_MIDISTREAMING_SubCLASS, 0xFF);

	if(interface == 0xFF) /* No Valid Interface */
	{
		USBH_DbgLog ("Cannot Find the interface for MIDI Interface Class.", phost->pActiveClass->Name);
		status = USBH_FAIL;
	}
	else
	{
		USBH_SelectInterface (phost, interface);

		phost->pActiveClass->pData = (MIDI_HandleTypeDef *)USBH_malloc (sizeof(MIDI_HandleTypeDef));
		MIDI_Handle =  phost->pActiveClass->pData;

		if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress & 0x80)
		{
			MIDI_Handle->InEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress);
			MIDI_Handle->InEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
		}
		else
		{
			MIDI_Handle->OutEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress);
			MIDI_Handle->OutEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
		}

		if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress & 0x80)
		{
			MIDI_Handle->InEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress);
			MIDI_Handle->InEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].wMaxPacketSize;
		}
		else
		{
			MIDI_Handle->OutEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress);
			MIDI_Handle->OutEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].wMaxPacketSize;
		}

		MIDI_Handle->OutPipe = USBH_AllocPipe(phost, MIDI_Handle->OutEp);
		MIDI_Handle->InPipe = USBH_AllocPipe(phost, MIDI_Handle->InEp);


		/* Open the new channels */
		USBH_OpenPipe  (phost,
				MIDI_Handle->OutPipe,
				MIDI_Handle->OutEp,
				phost->device.address,
				phost->device.speed,
				USB_EP_TYPE_BULK,
				MIDI_Handle->OutEpSize);

		USBH_OpenPipe  (phost,
				MIDI_Handle->InPipe,
				MIDI_Handle->InEp,
				phost->device.address,
				phost->device.speed,
				USB_EP_TYPE_BULK, // changing this to INTR seems to be option B from this post https://community.st.com/s/question/0D50X00009XkYz2SAF/hal-usb-midi-host-code-midi-stays-in-idle-mode-no-data-received
				MIDI_Handle->InEpSize);

		//USB_MIDI_ChangeConnectionState(1);
		MIDI_Handle->state = MIDI_IDLE_STATE;


		USBH_LL_SetToggle  (phost, MIDI_Handle->InPipe,0);
		USBH_LL_SetToggle  (phost, MIDI_Handle->OutPipe,0);

		USB_status_waiting = 0;


		status = USBH_OK;
	}
	return status;
}


/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  USBH_MIDI_InterfaceDeInit
 *         The function DeInit the Pipes used for the MIDI class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit (USBH_HandleTypeDef *phost)
{
	MIDI_HandleTypeDef *MIDI_Handle =  phost->pActiveClass->pData;
	HAL_Delay(100);
	if ( MIDI_Handle->OutPipe)
	{
		USBH_ClosePipe(phost, MIDI_Handle->OutPipe);
		USBH_FreePipe  (phost, MIDI_Handle->OutPipe);
		MIDI_Handle->OutPipe = 0;     /* Reset the Channel as Free */
	}

	if ( MIDI_Handle->InPipe)
	{
		USBH_ClosePipe(phost, MIDI_Handle->InPipe);
		USBH_FreePipe  (phost, MIDI_Handle->InPipe);
		MIDI_Handle->InPipe = 0;     /* Reset the Channel as Free */
	}

	if(phost->pActiveClass->pData)
	{
		USBH_free (phost->pActiveClass->pData);
		phost->pActiveClass->pData = 0;
	}


	return USBH_OK;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  USBH_MIDI_ClassRequest
 *         The function is responsible for handling Standard requests
 *         for MIDI class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_MIDI_ClassRequest (USBH_HandleTypeDef *phost)
{   

	phost->pUser(phost, HOST_USER_CLASS_ACTIVE);

	return USBH_OK;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

/**
  * @brief  USBH_MIDI_Stop
  *         Stop current MIDI Transmission
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_MIDI_Stop(USBH_HandleTypeDef *phost)
{
  MIDI_HandleTypeDef *MIDI_Handle =  phost->pActiveClass->pData;

  if(phost->gState == HOST_CLASS)
  {
    MIDI_Handle->state = MIDI_IDLE_STATE;

    USBH_ClosePipe(phost, MIDI_Handle->InPipe);
    USBH_ClosePipe(phost, MIDI_Handle->OutPipe);
  }
  //MX_USB_HOST_DeInit();
  HAL_HCD_MspInit(phost->pData);
  return USBH_OK;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  USBH_MIDI_Process
 *         The function is for managing state machine for MIDI data transfers
 *         (background process)
 * @param  phost: Host handle
 * @retval USBH Status
 */


static USBH_StatusTypeDef USBH_MIDI_Process (USBH_HandleTypeDef *phost)
{

	return USBH_OK;
}
/*------------------------------------------------------------------------------------------------------------------------------*/

/**
  * @brief  USBH_MIDI_SOFProcess
  *         The function is for managing SOF callback
  * @param  phost: Host handle
  * @retval USBH Status
*/

static USBH_StatusTypeDef USBH_MIDI_SOFProcess (USBH_HandleTypeDef *phost)
{
	MIDI_HandleTypeDef *MIDI_Handle =  phost->pActiveClass->pData;
		uint32_t length = 0;
		USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
		HCD_HandleTypeDef *hostHandle = phost->pData;


		if (MIDI_Appli_state == MIDI_APPLICATION_RUNNING)
		{

			if (USB_status_waiting == 1)
			{

				//if successful transaction happened
				URB_Status = USBH_LL_GetURBState(phost, MIDI_Handle->InPipe);

				if(URB_Status == USBH_URB_DONE )
				{
					length = USBH_LL_GetLastXferSize(phost, MIDI_Handle->InPipe);

	/*
					if(((MIDI_Handle->RxDataLength - length) > 0) && (length > MIDI_Handle->InEpSize))
					{
						setLED_Edit(1);
						MIDI_Handle->RxDataLength -= length ;
						MIDI_Handle->pRxData += length;
						USBH_BulkReceiveData(phost,
															MIDI_Handle->pRxData,
															MIDI_Handle->InEpSize,
															MIDI_Handle->InPipe);
					}
	*/
					//else
					{
						if (length > 0)
						{
							//switch buffers so that we read the one we just wrote and prepare to write to the other one
							MIDI_read_buffer = !MIDI_read_buffer;
							MIDI_write_buffer = !MIDI_write_buffer;

						}
						USB_status_waiting = 0;
						ProcessReceivedMidiDatas(length);
					}
				}
				if((URB_Status == USBH_URB_NOTREADY )) //|| (URB_Status == USBH_URB_IDLE ) || (URB_Status == USBH_URB_ERROR))
				{
					//setLED_C(1);
					//USB_status_waiting = 0;
				}
				else if(hostHandle->hc[(MIDI_Handle->InPipe)].state == HC_DATATGLERR)
				{

					setLED_USB(1);
					//repeat the toggle value to tell the device that the transmission failed
					//hostHandle->hc[(MIDI_Handle->InPipe)].toggle_in = !hostHandle->hc[(MIDI_Handle->InPipe)].toggle_in;
					//USBH_ClrFeature(phost, MIDI_Handle->InPipe);
					HAL_HCD_ResetPort(hostHandle);
					USB_status_waiting = 0;

				}
			}
			else if (USB_status_waiting == 0)
			{
				USB_status_waiting = 1;
				MIDI_Handle->RxDataLength = 64;
				MIDI_Handle->pRxData = &MIDI_RX_Buffer[MIDI_write_buffer][0];
				MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA_WAIT,
				USBH_BulkReceiveData(phost,
								&MIDI_RX_Buffer[MIDI_write_buffer][0],
								MIDI_Handle->InEpSize,
								MIDI_Handle->InPipe);

			}//wait status
		} //application running


		return USBH_OK;
}
  
/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  This function return last recieved data size
 * @param  None
 * @retval None
 */
uint32_t USBH_MIDI_GetLastReceivedDataSize(USBH_HandleTypeDef *phost)
{
	MIDI_HandleTypeDef *MIDI_Handle =  phost->pActiveClass->pData;

	if(phost->gState == HOST_CLASS)
	{
		return USBH_LL_GetLastXferSize(phost, MIDI_Handle->InPipe);
	}
	else
	{
		return 0;
	}
}

/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  This function prepares the state before issuing the class specific commands
 * @param  None
 * @retval None
 */
USBH_StatusTypeDef  USBH_MIDI_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint16_t length)
{
	USBH_StatusTypeDef Status = USBH_BUSY;
	MIDI_HandleTypeDef *MIDI_Handle =  phost->pActiveClass->pData;

	//if((MIDI_Handle->state == MIDI_IDLE_STATE) || (MIDI_Handle->state == MIDI_TRANSFER_DATA))
	{
		MIDI_Handle->pTxData = pbuff;
		MIDI_Handle->TxDataLength = length;
		MIDI_Handle->state = MIDI_TRANSFER_DATA;
		MIDI_Handle->data_tx_state = MIDI_SEND_DATA;
		Status = USBH_OK;
#if (USBH_USE_OS == 1)
		osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
	}
	return Status;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  This function prepares the state before issuing the class specific commands
 * @param  None
 * @retval None
 */

USBH_StatusTypeDef  USBH_MIDI_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint16_t length)
{
	USBH_StatusTypeDef Status = USBH_BUSY;
	MIDI_HandleTypeDef *MIDI_Handle =  phost->pActiveClass->pData;

	//if((MIDI_Handle->state == MIDI_IDLE_STATE) || (MIDI_Handle->state == MIDI_TRANSFER_DATA))
	{
		MIDI_Handle->pRxData = pbuff;
		MIDI_Handle->RxDataLength = length;
		MIDI_Handle->state = MIDI_TRANSFER_DATA;
		MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA;
		Status = USBH_OK;
		//first reception start (future receptions are handled by SOF interrupt)
		USBH_InterruptReceiveData(phost,
											MIDI_Handle->pRxData,
											MIDI_Handle->InEpSize,
											MIDI_Handle->InPipe);
		/*
		USBH_BulkReceiveData (phost,
			MIDI_RX_Buffer,
			MIDI_Handle->InEpSize,
			MIDI_Handle->InPipe);
			*/
#if (USBH_USE_OS == 1)
		osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
	}
	return Status;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  The function is responsible for sending data to the device
 *  @param  pdev: Selected device
 * @retval None
 */
static void MIDI_ProcessTransmission(USBH_HandleTypeDef *phost)
{
	;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  This function responsible for reception of data from the device
 *  @param  pdev: Selected device
 * @retval None
 */


static void MIDI_ProcessReception(USBH_HandleTypeDef *phost)
{
	MIDI_HandleTypeDef *MIDI_Handle =  phost->pActiveClass->pData;
	USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
	uint32_t length;
	HCD_HandleTypeDef *hostHandle = phost->pData;
	//switch(MIDI_Handle->data_rx_state)
	//{
/*
	case MIDI_RECEIVE_DATA:

		USBH_BulkReceiveData (phost,
				MIDI_Handle->pRxData,
				MIDI_Handle->InEpSize,
				MIDI_Handle->InPipe);

		MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA_WAIT;

		break;
		*/

	//case MIDI_RECEIVE_DATA_WAIT:
/*
		URB_Status = USBH_LL_GetURBState(phost, MIDI_Handle->InPipe);

		if(URB_Status == USBH_URB_DONE )
		{
			length = USBH_LL_GetLastXferSize(phost, MIDI_Handle->InPipe);

			if(((MIDI_Handle->RxDataLength - length) > 0) && (length > MIDI_Handle->InEpSize))
			{
				MIDI_Handle->RxDataLength -= length ;
				MIDI_Handle->pRxData += length;
				MIDI_Handle->data_rx_state = MIDI_RECEIVE_DATA;
			}
			else
			{
				MIDI_Handle->data_rx_state = MIDI_IDLE;
				USBH_MIDI_ReceiveCallback(phost, length);
			}

#if (USBH_USE_OS == 1)
			osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
		}

		else if((URB_Status == USBH_URB_NOTREADY) && (hostHandle->hc[(MIDI_Handle->InPipe)].state == HC_DATATGLERR))
		{
			MIDI_Handle->state = MIDI_ERROR_STATE;
			setLED_USB(1);
			//USBH_MIDI_Receive(phost, MIDI_RX_Buffer, RX_BUFF_SIZE);
		}
*/
		//break;

	//default:
		//break;
	//}
}


/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  The function informs user that data have been transmitted.
 *  @param  pdev: Selected device
 * @retval None
 */
__weak void USBH_MIDI_TransmitCallback(USBH_HandleTypeDef *phost)
{

}

/*------------------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief  The function informs user that data have been received.
 * @retval None
 */
__weak void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost, uint32_t myLength)
{

}

/**************************END OF FILE*********************************************************/
