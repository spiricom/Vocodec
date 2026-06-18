/*
 * ui.c
 *
 *  Created on: Feb 05, 2018
 *      Author: jeffsnyder
 */

#ifndef __cplusplus
#include "main.h"
#include "audiostream.h"
#include "eeprom.h"
#endif


#include "ui.h"
uint8_t buttonValues[NUM_BUTTONS];
uint8_t buttonValuesPrev[NUM_BUTTONS];
uint32_t buttonCounters[NUM_BUTTONS];
uint32_t buttonPressed[NUM_BUTTONS];


void setLED_1(int onOff)
{
   if (onOff)
   {
	   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
   }
   else
   {
	   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
   }
}

void setLED_USB(int onOff)
{
   if (onOff)
   {
	   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
   }
   else
   {
	   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
   }
}

void setLED_2(int onOff)
{
   if (onOff)
   {
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
   }
   else
   {
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
   }
}

void setLED_A(int onOff)
{
   if (onOff)
   {
	   HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
   }
   else
   {
	   HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
   }
}


void setLED_B(int onOff)
{
   if (onOff)
   {
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
   }
   else
   {
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
   }
}


void setLED_leftout_clip(int onOff)
{
   if (onOff)
   {
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
   }
   else
   {
	   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
   }
}



void processButtons(void)
{

    //buttonValues[0] =!(GPIOB->IDR & GPIO_PIN_13);//edit
    buttonValues[1] =!(GPIOG->IDR & GPIO_PIN_7);//left
    buttonValues[2] =!(GPIOG->IDR & GPIO_PIN_6);//right
    //buttonValues[3] =!(GPIOD->IDR & GPIO_PIN_11);//down
    //buttonValues[4] =!(GPIOB->IDR & GPIO_PIN_15);//up
    //buttonValues[5] =!(GPIOB->IDR & GPIO_PIN_1);//A
    buttonValues[6] =!(GPIOD->IDR & GPIO_PIN_11);//B
    //buttonValues[7] =!(GPIOB->IDR & GPIO_PIN_11);//C



    for (int i = 0; i < NUM_BUTTONS; i++)
	{
	  if ((buttonValues[i] != buttonValuesPrev[i]) && (buttonCounters[i] < 10))
	  {
		  buttonCounters[i]++;
	  }
	  if ((buttonValues[i] != buttonValuesPrev[i]) && (buttonCounters[i] >= 10))
	  {
		  if (buttonValues[i] == 1)
		  {
			  buttonPressed[i] = 1;
		  }
		  buttonValuesPrev[i] = buttonValues[i];
		  buttonCounters[i] = 0;
	  }
	}

    if (buttonPressed[6])
    {
    	//legato vs retrig modes
    	retrigMode = !retrigMode;
    	setLED_B(retrigMode);
    	buttonPressed[6] = 0;
    }

    if (buttonPressed[2])
    {
    	//right button
    	presetWaitingToLoad = 1;
    	presetNumberToLoad = (currentPreset + 1) % (MAX_NUM_PRESETS);
    	buttonPressed[2] = 0;
    }

    if (buttonPressed[1])
    {
    	//right button
    	presetWaitingToLoad = 1;
    	presetNumberToLoad = (currentPreset - 1) % (MAX_NUM_PRESETS);
    	buttonPressed[1] = 0;
    }


}
