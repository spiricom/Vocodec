/*
 * oled.c
 *
 *  Created on: Feb 05, 2020
 *      Author: Matthew Wang
 */
#include "main.h"
#include "oled.h"
#include "ui.h"
#include "ssd1306.h"
#include "gfx.h"
#include "custom_fonts.h"
#include "audiostream.h"
#include "tunings.h"
#include "eeprom.h"

int8_t writeParameterFlag = -1;
GFX theGFX;

char oled_buffer[32];
uint32_t currentTuning = 0;

float floatADC[NUM_ADC_CHANNELS];
float lastFloatADC[NUM_ADC_CHANNELS];
float hysteresisThreshold = 0.001f;

uint8_t buttonValues[NUM_BUTTONS];
uint8_t buttonValuesPrev[NUM_BUTTONS];
uint32_t buttonCounters[NUM_BUTTONS];
uint8_t buttonPressed[NUM_BUTTONS];
uint8_t buttonReleased[NUM_BUTTONS];

void OLED_init(I2C_HandleTypeDef* hi2c)
{
	  //start up that OLED display
	  ssd1306_begin(hi2c, SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

	  HAL_Delay(5);

	  //clear the OLED display buffer
	  for (int i = 0; i < 512; i++)
	  {
		  buffer[i] = 0;
	  }
	  initUIFunctionPointers();
	  initModeNames();
	  //display the blank buffer on the OLED
	  //ssd1306_display_full_buffer();

	  //initialize the graphics library that lets us write things in that display buffer
	  GFXinit(&theGFX, 128, 32);

	  //set up the monospaced font

	  //GFXsetFont(&theGFX, &C649pt7b); //funny c64 text monospaced but very large
	  //GFXsetFont(&theGFX, &DINAlternateBold9pt7b); //very serious and looks good - definitely not monospaced can fit 9 Ms
	  //GFXsetFont(&theGFX, &DINCondensedBold9pt7b); // very condensed and looks good - definitely not monospaced can fit 9 Ms
	  GFXsetFont(&theGFX, &EuphemiaCAS9pt7b); //this one is elegant but definitely not monospaced can fit 9 Ms
	  //GFXsetFont(&theGFX, &GillSans9pt7b); //not monospaced can fit 9 Ms
	  //GFXsetFont(&theGFX, &Futura9pt7b); //not monospaced can fit only 7 Ms
	  //GFXsetFont(&theGFX, &FUTRFW8pt7b); // monospaced, pretty, (my old score font) fits 8 Ms
	  //GFXsetFont(&theGFX, &nk57_monospace_cd_rg9pt7b); //fits 12 characters, a little crammed
	  //GFXsetFont(&theGFX, &nk57_monospace_no_rg9pt7b); // fits 10 characters
	  //GFXsetFont(&theGFX, &nk57_monospace_no_rg7pt7b); // fits 12 characters
	  //GFXsetFont(&theGFX, &nk57_monospace_no_bd7pt7b); //fits 12 characters
	  //GFXsetFont(&theGFX, &nk57_monospace_cd_rg7pt7b); //fits 18 characters

	  GFXsetTextColor(&theGFX, 1, 0);
	  GFXsetTextSize(&theGFX, 1);

	  //ssd1306_display_full_buffer();

	  OLEDclear();
	  OLED_writePreset();
	  OLED_draw();
	//sdd1306_invertDisplay(1);
}

void setLED_Edit(uint8_t onOff)
{
	if (onOff)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
	}
}


void setLED_USB(uint8_t onOff)
{
	if (onOff)
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET);
	}
}


void setLED_1(uint8_t onOff)
{
	if (onOff)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
	}
}

void setLED_2(uint8_t onOff)
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


void setLED_A(uint8_t onOff)
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

void setLED_B(uint8_t onOff)
{
	if (onOff)
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_RESET);
	}
}

void setLED_C(uint8_t onOff)
{
	if (onOff)
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_RESET);
	}
}

void setLED_leftout_clip(uint8_t onOff)
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

void setLED_rightout_clip(uint8_t onOff)
{
	if (onOff)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	}
}

void setLED_leftin_clip(uint8_t onOff)
{
	if (onOff)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
	}
}

void setLED_rightin_clip(uint8_t onOff)
{
	if (onOff)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	}
}

void OLED_process(void)
{
	if (writeParameterFlag >= 0)
	{
		OLED_writeParameter(writeParameterFlag);
		writeParameterFlag = -1;
	}
	OLED_draw();
}

void OLED_writePreset()
{
	GFXsetFont(&theGFX, &EuphemiaCAS9pt7b);
	OLEDclear();
	char tempString[24];
	itoa((currentPreset+1), tempString, 10);
	strcat(tempString, ":");
	strcat(tempString, modeNames[currentPreset]);
	int myLength = strlen(tempString);
	//OLEDwriteInt(currentPreset+1, 2, 0, FirstLine);
	//OLEDwriteString(":", 1, 20, FirstLine);
	//OLEDwriteString(modeNames[currentPreset], 12, 24, FirstLine);
	OLEDwriteString(tempString, myLength, 0, FirstLine);
	GFXsetFont(&theGFX, &EuphemiaCAS7pt7b);
	OLEDwriteString(modeNamesDetails[currentPreset], strlen(modeNamesDetails[currentPreset]), 0, SecondLine);
	//save new preset to flash memory
}

void OLED_writeParameter(uint8_t whichParam)
{
	if (whichParam < NUM_ADC_CHANNELS)
	{
		int myLength = strlen(knobParamNames[currentPreset][whichParam]);
		if (myLength > 0)
		{
			GFXsetFont(&theGFX, &EuphemiaCAS7pt7b);
			OLEDclearLine(SecondLine);
			OLEDwriteString(knobParamNames[currentPreset][whichParam], strlen(knobParamNames[currentPreset][whichParam]), 0, SecondLine);
			int xpos = GFXgetCursorX(&theGFX);
			OLEDwriteString(" ", 1, xpos, SecondLine);
			xpos = GFXgetCursorX(&theGFX);
			OLEDwriteFloat(knobParams[whichParam], xpos, SecondLine);
			//OLEDwriteString(knobParamNames[currentPreset][whichParam], strlen(knobParamNames[currentPreset][whichParam]), 0, SecondLine);
		}
	}
	else
	{
		whichParam -= NUM_ADC_CHANNELS;
		GFXsetFont(&theGFX, &EuphemiaCAS7pt7b);
		OLEDclearLine(SecondLine);
		char* buttonDisplay = buttonParamFunctions[currentPreset](whichParam);
		OLEDwriteString(buttonDisplay, strlen(buttonDisplay), 0, SecondLine);
	}

}

void changeTuning()
{
	for (int i = 0; i < 12; i++)
	{
		centsDeviation[i] = tuningPresets[currentTuning][i];

	}
	if (currentTuning == 0)
	{
		//setLED_C(0);
	}
	else
	{
		///setLED_C(1);
	}
	if (currentPreset == AutotuneMono)
	{
		calculatePeriodArray();
	}
	GFXsetFont(&theGFX, &EuphemiaCAS7pt7b);
	OLEDclearLine(SecondLine);
	OLEDwriteString("T ", 2, 0, SecondLine);
	OLEDwriteInt(currentTuning, 2, 12, SecondLine);
	OLEDwriteString(tuningNames[currentTuning], 6, 40, SecondLine);
}

void OLED_draw()
{
	ssd1306_display_full_buffer();
}

/// OLED Stuff

void OLEDdrawPoint(int16_t x, int16_t y, uint16_t color)
{
	GFXwritePixel(&theGFX, x, y, color);
	//ssd1306_display_full_buffer();
}

void OLEDdrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
	GFXwriteLine(&theGFX, x0, y0, x1, y1, color);
	//ssd1306_display_full_buffer();
}

void OLEDdrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	GFXfillCircle(&theGFX, x0, y0, r, color);
	//ssd1306_display_full_buffer();
}


void OLEDclear()
{
	GFXfillRect(&theGFX, 0, 0, 128, 32, 0);
	//ssd1306_display_full_buffer();
}

void OLEDclearLine(OLEDLine line)
{
	GFXfillRect(&theGFX, 0, (line%2)*16, 128, 16*((line/2)+1), 0);
	//ssd1306_display_full_buffer();
}

void OLEDwriteString(char* myCharArray, uint8_t arrayLength, uint8_t startCursor, OLEDLine line)
{
	uint8_t cursorX = startCursor;
	uint8_t cursorY = 15 + (16 * (line%2));
	GFXsetCursor(&theGFX, cursorX, cursorY);

	GFXfillRect(&theGFX, startCursor, line*16, arrayLength*12, (line*16)+16, 0);
	for (int i = 0; i < arrayLength; ++i)
	{
		GFXwrite(&theGFX, myCharArray[i]);
	}
	//ssd1306_display_full_buffer();
}

void OLEDwriteLine(char* myCharArray, uint8_t arrayLength, OLEDLine line)
{
	if (line == FirstLine)
	{
		GFXfillRect(&theGFX, 0, 0, 128, 16, 0);
		GFXsetCursor(&theGFX, 4, 15);
	}
	else if (line == SecondLine)
	{
		GFXfillRect(&theGFX, 0, 16, 128, 16, 0);
		GFXsetCursor(&theGFX, 4, 31);
	}
	else if (line == BothLines)
	{
		GFXfillRect(&theGFX, 0, 0, 128, 32, 0);
		GFXsetCursor(&theGFX, 4, 15);
	}
	for (int i = 0; i < arrayLength; ++i)
	{
		GFXwrite(&theGFX, myCharArray[i]);
	}
	//ssd1306_display_full_buffer();
}

void OLEDwriteInt(uint32_t myNumber, uint8_t numDigits, uint8_t startCursor, OLEDLine line)
{
	int len = OLEDparseInt(oled_buffer, myNumber, numDigits);

	OLEDwriteString(oled_buffer, len, startCursor, line);
}

void OLEDwriteIntLine(uint32_t myNumber, uint8_t numDigits, OLEDLine line)
{
	int len = OLEDparseInt(oled_buffer, myNumber, numDigits);

	OLEDwriteLine(oled_buffer, len, line);
}

void OLEDwritePitch(float midi, uint8_t startCursor, OLEDLine line)
{
	int len = OLEDparsePitch(oled_buffer, midi);

	OLEDwriteString(oled_buffer, len, startCursor, line);
}

void OLEDwritePitchClass(float midi, uint8_t startCursor, OLEDLine line)
{
	int len = OLEDparsePitchClass(oled_buffer, midi);

	OLEDwriteString(oled_buffer, len, startCursor, line);
}

void OLEDwritePitchLine(float midi, OLEDLine line)
{
	int len = OLEDparsePitch(oled_buffer, midi);

	OLEDwriteLine(oled_buffer, len, line);
}

void OLEDwriteFixedFloat(float input, uint8_t numDigits, uint8_t numDecimal, uint8_t startCursor, OLEDLine line)
{
	int len = OLEDparseFixedFloat(oled_buffer, input, numDigits, numDecimal);

	OLEDwriteString(oled_buffer, len, startCursor, line);
}

void OLEDwriteFixedFloatLine(float input, uint8_t numDigits, uint8_t numDecimal, OLEDLine line)
{
	int len = OLEDparseFixedFloat(oled_buffer, input, numDigits, numDecimal);

	OLEDwriteLine(oled_buffer, len, line);
}


void OLEDwriteFloat(float input, uint8_t startCursor, OLEDLine line)
{
	int numDigits = 5;
	int numDecimal = 1;

	if (fastabsf(input)<1.0f)
	{
		numDigits = 3;
		numDecimal = 2;
	}

	else if (fastabsf(input)<10.0f)
	{
		numDigits = 4;
		numDecimal = 2;
	}

	else if (fastabsf(input)<100.0f)
	{
		numDigits = 5;
		numDecimal = 2;
	}

	else if (fastabsf(input)<1000.0f)
	{
		numDigits = 5;
		numDecimal = 1;
	}
	else if (fastabsf(input)<10000.0f)
	{
		numDigits = 5;
		numDecimal = 0;
	}
	else if (fastabsf(input)<100000.0f)
	{
		numDigits = 6;
		numDecimal = 0;
	}
	else if (fastabsf(input)<1000000.0f)
	{
		numDigits = 7;
		numDecimal = 0;
	}
	else if (fastabsf(input)<10000000.0f)
	{
		numDigits = 8;
		numDecimal = 0;
	}

	int len = OLEDparseFixedFloat(oled_buffer, input, numDigits, numDecimal);

	OLEDwriteString(oled_buffer, len, startCursor, line);
}

void buttonCheck(void)
{
	if (codecReady)
	{
		buttonValues[0] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13); //edit
		buttonValues[1] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12); //left
		buttonValues[2] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14); //right
		buttonValues[3] = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11); //down
		buttonValues[4] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15); //up
		buttonValues[5] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);  // A
		buttonValues[6] = !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7);  // B
		buttonValues[7] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11); // C
		buttonValues[8] = !HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_11); // D
		buttonValues[9] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10); // E

		for (int i = 0; i < NUM_BUTTONS; i++)
		{
			// changed this so that presses and releases need to be zeroed by the code that reads them
			//buttonPressed[i] = 0;
			//buttonReleased[i] = 0;
			if ((buttonValues[i] != buttonValuesPrev[i]) && (buttonCounters[i] < 1))
			{
				buttonCounters[i]++;
			}
			if ((buttonValues[i] != buttonValuesPrev[i]) && (buttonCounters[i] >= 1))
			{
				if (buttonValues[i] == 1)
				{
					buttonPressed[i] = 1;
				}
				else if (buttonValues[i] == 0)
				{
					buttonReleased[i] = 1;
				}
				buttonValuesPrev[i] = buttonValues[i];
				buttonCounters[i] = 0;
				if (i == 5 || i == 6) writeParameterFlag = i + NUM_ADC_CHANNELS;
			}
		}

		// make some if statements if you want to find the "attack" of the buttons (getting the "press" action)
		// we'll need if statements for each button  - maybe should go to functions that are dedicated to each button?

		// TODO: buttons C and E are connected to pins that are used to set up the codec over I2C - we need to reconfigure those pins in some kind of button init after the codec is set up. not done yet.

		if (buttonPressed[0] == 1)
		{
			/*
			setLED_Edit(1);
			setLED_USB(1);
			setLED_1(1);
			setLED_2(1);
			setLED_A(1);
			setLED_B(1);
			setLED_C(1);
			setLED_leftout_clip(1);

			setLED_leftin_clip(1);
			setLED_rightout_clip(1);
			setLED_rightin_clip(1);
			*/

		}

		/// DEFINE GLOBAL BEHAVIOR FOR BUTTONS HERE
		// left press
		if (buttonPressed[1] == 1)
		{
			previousPreset = currentPreset;

			if (currentPreset <= 0) currentPreset = PresetNil - 1;
			else currentPreset--;

			loadingPreset = 1;
			OLED_writePreset();
			writeCurrentPresetToFlash();
			buttonPressed[1] = 0;
		}

		// right press
		if (buttonPressed[2] == 1)
		{
			previousPreset = currentPreset;
			if (currentPreset >= PresetNil - 1) currentPreset = 0;
			else currentPreset++;

			loadingPreset = 1;
			OLED_writePreset();
			writeCurrentPresetToFlash();
			buttonPressed[2] = 0;
		}

		if (buttonPressed[7] == 1)
		{
			//GFXsetFont(&theGFX, &DINCondensedBold9pt7b);
			keyCenter = (keyCenter + 1) % 12;
			OLEDclearLine(SecondLine);
			OLEDwriteString("KEY: ", 5, 0, SecondLine);
			OLEDwritePitchClass(keyCenter+60, 64, SecondLine);
			buttonPressed[7] = 0;
		}

		if (buttonPressed[8] == 1)
		{
			if (currentTuning == 0)
			{
				currentTuning = NUM_TUNINGS - 1;
			}
			else
			{
				currentTuning = (currentTuning - 1);
			}
			changeTuning();
			buttonPressed[8] = 0;

		}

		if (buttonPressed[9] == 1)
		{

			currentTuning = (currentTuning + 1) % NUM_TUNINGS;
			changeTuning();
			buttonPressed[9] = 0;
		}
	}
}

void adcCheck()
{
	//read the analog inputs and smooth them with ramps
	for (int i = 0; i < 6; i++)
	{
		//floatADC[i] = (float) (ADC_values[i]>>8) * INV_TWO_TO_8;
		floatADC[i] = (float) (ADC_values[i]>>6) * INV_TWO_TO_10;


		if (fastabsf(floatADC[i] - lastFloatADC[i]) > hysteresisThreshold)
		{
			lastFloatADC[i] = floatADC[i];
			writeParameterFlag = i;
		}
		tRamp_setDest(&adc[i], floatADC[i]);
	}
}
