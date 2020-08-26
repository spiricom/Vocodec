/*
 * oled.c
 *
 *  Created on: Feb 05, 2020
 *      Author: Matthew Wang
 */

#ifndef __cplusplus
#include "main.h"
#include "ssd1306.h"
#include "audiostream.h"
#else
#include "PluginEditor.h"
#endif

#include "oled.h"
#include "ui.h"
#include "gfx.h"
#include "custom_fonts.h"
#include "tunings.h"

#ifdef __cplusplus
#include <string.h>
#define itoa(v, s, b) (strcpy(s, std::to_string(v).c_str()))
namespace vocodec
{
    extern "C"
    {
#endif

GFX theGFX;
char oled_buffer[32];

void OLED_init(I2C_HandleTypeDef* hi2c)
{
#ifndef __cplusplus
	  //start up that OLED display
	  ssd1306_begin(hi2c, SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
#endif
    
	  //HAL_Delay(5);

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
	  GFXsetFont(&theGFX, &EuphemiaCAS8pt7b); //this one is elegant but definitely not monospaced can fit 9 Ms
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

void initUIFunctionPointers(void)
{
	buttonActionFunctions[Vocoder] = UIVocoderButtons;
	buttonActionFunctions[VocoderCh] = UIVocoderChButtons;
	buttonActionFunctions[Pitchshift] = UIPitchShiftButtons;
	buttonActionFunctions[AutotuneMono] = UINeartuneButtons;
	buttonActionFunctions[AutotunePoly] = UIAutotuneButtons;
	buttonActionFunctions[SamplerButtonPress] = UISamplerBPButtons;
	buttonActionFunctions[SamplerKeyboard] = UISamplerKButtons;
	buttonActionFunctions[SamplerAutoGrab] = UISamplerAutoButtons;
	buttonActionFunctions[Distortion] = UIDistortionButtons;
	buttonActionFunctions[Wavefolder] = UIWaveFolderButtons;
	buttonActionFunctions[BitCrusher] = UIBitcrusherButtons;
	buttonActionFunctions[Delay] = UIDelayButtons;
	buttonActionFunctions[Reverb] = UIReverbButtons;
	buttonActionFunctions[Reverb2] = UIReverb2Buttons;
	buttonActionFunctions[LivingString] = UILivingStringButtons;
	buttonActionFunctions[LivingStringSynth] = UILivingStringSynthButtons;
	buttonActionFunctions[ClassicSynth] = UIClassicSynthButtons;
	buttonActionFunctions[Rhodes] = UIRhodesButtons;
}

#ifndef __cplusplus
void setLED_Edit(int onOff)
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


void setLED_USB(int onOff)
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


void setLED_1(int onOff)
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
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	}
}

void setLED_B(int onOff)
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

void setLED_C(int onOff)
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

void setLED_rightout_clip(int onOff)
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

void setLED_leftin_clip(int onOff)
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

void setLED_rightin_clip(int onOff)
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
#endif

int getCursorX()
{
	return GFXgetCursorX(&theGFX);
}

void OLED_process(void)
{
	if (writeKnobFlag >= 0)
	{
		OLED_writeKnobParameter(writeKnobFlag);
		writeKnobFlag = -1;
	}
	if (writeButtonFlag >= 0 && writeActionFlag >= 0) //These should always be set together
	{
		OLED_writeButtonAction(writeButtonFlag, writeActionFlag);
		writeButtonFlag = -1;
		writeActionFlag = -1;
	}
//	OLED_draw();
}

void OLED_writePreset()
{
	GFXsetFont(&theGFX, &EuphemiaCAS8pt7b);
	OLEDclear();
	char tempString[24];
	itoa((currentPreset+1), tempString, 10);
	strcat(tempString, ":");
	strcat(tempString, modeNames[currentPreset]);
	int myLength = (int)strlen(tempString);
	//OLEDwriteInt(currentPreset+1, 2, 0, FirstLine);
	//OLEDwriteString(":", 1, 20, FirstLine);
	//OLEDwriteString(modeNames[currentPreset], 12, 24, FirstLine);
	OLEDwriteString(tempString, myLength, 0, FirstLine);
	GFXsetFont(&theGFX, &EuphemiaCAS7pt7b);
	OLEDwriteString(modeNamesDetails[currentPreset], (int)strlen(modeNamesDetails[currentPreset]), 0, SecondLine);
	//save new preset to flash memory
}

void OLED_writeEditScreen()
{
	GFXsetFont(&theGFX, &EuphemiaCAS7pt7b);
	OLEDclear();
    const char* firstSet = "KNOB:SET CV PED";
	const char* firstClear = "DOWN:CLR CV PED";
	if (cvAddParam[currentPreset] >= 0) OLEDwriteString(firstClear, (int)strlen(firstClear), 0, FirstLine);
	else OLEDwriteString(firstSet, (int)strlen(firstSet), 0, FirstLine);
	OLEDwriteString("C:SET KEY CENTER", 16, 0, SecondLine);
}

void OLED_writeKnobParameter(int whichKnob)
{
	// Knob params
	if (whichKnob < KNOB_PAGE_SIZE)
	{
		int whichParam = whichKnob + (knobPage * KNOB_PAGE_SIZE);
		floatADCUI[whichKnob] = smoothedADC[whichKnob];
		int len = (int)strlen(knobParamNames[currentPreset][whichParam]);
		if (len > 0)
		{
			GFXsetFont(&theGFX, &EuphemiaCAS7pt7b);
			OLEDclearLine(SecondLine);
			OLEDwriteString(knobParamNames[currentPreset][whichParam], len, 0, SecondLine);
			OLEDwriteString(" ", 1, getCursorX(), SecondLine);
			OLEDwriteFloat(displayValues[whichKnob + (knobPage * KNOB_PAGE_SIZE)], getCursorX(), SecondLine);
			//OLEDwriteString(paramNames[currentPreset][whichParam], strlen(paramNames[currentPreset][whichParam]), 0, SecondLine);
		}
	}
}

void OLED_writeButtonAction(int whichButton, int whichAction)
{
	// Could change this so that buttonActionFunctions does the actual OLEDwrite
	// if we want more flexibility on what buttons display
	const char* str = buttonActionFunctions[currentPreset]((VocodecButton)whichButton, (ButtonAction)whichAction);
	int len = (int)strlen(str);
	if (len > 0)
	{
		GFXsetFont(&theGFX, &EuphemiaCAS7pt7b);
		OLEDclearLine(SecondLine);
		OLEDwriteString(str, len, 0, SecondLine);
	}
}

void OLED_writeTuning()
{
	GFXsetFont(&theGFX, &EuphemiaCAS7pt7b);
	OLEDclearLine(SecondLine);
	OLEDwriteString("T", 2, 0, SecondLine);
	OLEDwriteInt(currentTuning, 2, 12, SecondLine);
	OLEDwriteString(tuningNames[currentTuning], 12, 36, SecondLine);
}

#ifndef __cplusplus
void OLED_draw()
{
	ssd1306_display_full_buffer();
}
#endif

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

void OLEDwriteString(const char* myCharArray, int arrayLength, int startCursor, OLEDLine line)
{
	int cursorX = startCursor;
	int cursorY = 12 + (16 * (line%2));
	GFXsetCursor(&theGFX, cursorX, cursorY);

	GFXfillRect(&theGFX, startCursor, line*16, arrayLength*12, (line*16)+16, 0);
	for (int i = 0; i < arrayLength; ++i)
	{
		GFXwrite(&theGFX, myCharArray[i]);
	}
	//ssd1306_display_full_buffer();
}

void OLEDwriteLine(const char* myCharArray, int arrayLength, OLEDLine line)
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
}

void OLEDwriteInt(uint32_t myNumber, int numDigits, int startCursor, OLEDLine line)
{
	int len = OLEDparseInt(oled_buffer, myNumber, numDigits);

	OLEDwriteString(oled_buffer, len, startCursor, line);
}

void OLEDwriteIntLine(uint32_t myNumber, int numDigits, OLEDLine line)
{
	int len = OLEDparseInt(oled_buffer, myNumber, numDigits);

	OLEDwriteLine(oled_buffer, len, line);
}

void OLEDwritePitch(float midi, int startCursor, OLEDLine line, int showCents)
{
	int len = OLEDparsePitch(oled_buffer, midi, showCents);

	OLEDwriteString(oled_buffer, len, startCursor, line);
}

void OLEDwritePitchClass(float midi, int startCursor, OLEDLine line)
{
	int len = OLEDparsePitchClass(oled_buffer, midi);

	OLEDwriteString(oled_buffer, len, startCursor, line);
}

void OLEDwritePitchLine(float midi, OLEDLine line, int showCents)
{
	int len = OLEDparsePitch(oled_buffer, midi, showCents);

	OLEDwriteLine(oled_buffer, len, line);
}

void OLEDwriteFixedFloat(float input, int numDigits, int numDecimal, int startCursor, OLEDLine line)
{
	int len = OLEDparseFixedFloat(oled_buffer, input, numDigits, numDecimal);

	OLEDwriteString(oled_buffer, len, startCursor, line);
}

void OLEDwriteFixedFloatLine(float input, int numDigits, int numDecimal, OLEDLine line)
{
	int len = OLEDparseFixedFloat(oled_buffer, input, numDigits, numDecimal);

	OLEDwriteLine(oled_buffer, len, line);
}


void OLEDwriteFloat(float input, int startCursor, OLEDLine line)
{
	int numDigits = 5;
	int numDecimal = 1;

    float f = fabsf(input);
	if (f<1.0f)
	{
		numDigits = 3;
		numDecimal = 2;
	}

	else if (f<10.0f)
	{
		numDigits = 4;
		numDecimal = 2;
	}

	else if (f<100.0f)
	{
		numDigits = 5;
		numDecimal = 2;
	}

	else if (f<1000.0f)
	{
		numDigits = 5;
		numDecimal = 1;
	}
	else if (f<10000.0f)
	{
		numDigits = 5;
		numDecimal = 0;
	}
	else if (f<100000.0f)
	{
		numDigits = 6;
		numDecimal = 0;
	}
	else if (f<1000000.0f)
	{
		numDigits = 7;
		numDecimal = 0;
	}
	else if (f<10000000.0f)
	{
		numDigits = 8;
		numDecimal = 0;
	}

	int len = OLEDparseFixedFloat(oled_buffer, input, numDigits, numDecimal);

	OLEDwriteString(oled_buffer, len, startCursor, line);
}

void OLEDdrawFloatArray(float* input, float min, float max, int size, int offset, int startCursor, OLEDLine line)
{
	int baseline = 0;
	if (line == SecondLine) baseline = 16;
	int height = 16;
	if (line == BothLines) height = 32;

	GFXfillRect(&theGFX, startCursor, (line%2)*16, size, 16*((line/2)+1), 0);

	for (int i = 0; i < size; i++)
	{
		int h = ((float)(height) / (max - min)) * (input[i] - min);
		GFXwritePixel(&theGFX, startCursor + size - 1 - ((i + offset) % size), baseline + h, 1);
//		GFXwriteFastVLine(&theGFX, startCursor + size - ((i + offset) % size), center - (h/2), 1, 1);
	}
}

int OLEDgetCursor()
{
	return (int)GFXgetCursorX(&theGFX);
}

#ifdef __cplusplus
    }
} // extern "C"
#endif
