/*
 * ui.c
 *
 *  Created on: Feb 05, 2018
 *      Author: jeffsnyder
 */
#include "main.h"
#include "audiostream.h"
#include "sfx.h"
#include "oled.h"
#include "ui.h"
#include "tunings.h"
#include "eeprom.h"
#include "leaf.h"

uint16_t ADC_values[NUM_ADC_CHANNELS] __ATTR_RAM_D2;
float floatADC[NUM_ADC_CHANNELS];
float lastFloatADC[NUM_ADC_CHANNELS];
float floatADCUI[NUM_ADC_CHANNELS];
float adcHysteresisThreshold = 0.004f;

uint8_t knobPage = 0;
uint8_t numPages[PresetNil];

uint8_t buttonValues[NUM_BUTTONS]; // Actual state of the buttons
uint8_t buttonValuesPrev[NUM_BUTTONS];
uint8_t cleanButtonValues[NUM_BUTTONS]; // Button values after hysteresis
uint32_t buttonHysteresis[NUM_BUTTONS];
uint32_t buttonHysteresisThreshold = 5;
uint32_t buttonCounters[NUM_BUTTONS]; // How long a button has been in its current state
uint32_t buttonHoldThreshold = 200;
uint32_t buttonHoldMax = 200;
//uint8_t buttonPressed[NUM_BUTTONS];
//uint8_t buttonReleased[NUM_BUTTONS];

int8_t writeKnobFlag = -1;
int8_t writeButtonFlag = -1;
int8_t writeActionFlag = -1;

#define NUM_CHARACTERS_PER_PRESET_NAME 16
char* modeNames[PresetNil];
char* modeNamesDetails[PresetNil];
char* shortModeNames[PresetNil];

char* knobParamNames[PresetNil][NUM_PRESET_KNOB_VALUES];

int8_t currentParamIndex = -1;
uint8_t orderedParams[8];

uint8_t buttonActionsSFX[NUM_BUTTONS][ActionNil];
uint8_t buttonActionsUI[NUM_BUTTONS][ActionNil];
float displayValues[NUM_PRESET_KNOB_VALUES];
int8_t cvAddParam[PresetNil];
char* (*buttonActionFunctions[PresetNil])(VocodecButton, ButtonAction);

VocodecPresetType currentPreset = 0;
VocodecPresetType previousPreset = PresetNil;
uint8_t loadingPreset = 0;

void initModeNames(void)
{
	for (int i = 0; i < NUM_ADC_CHANNELS; i++)
	{
		floatADCUI[i] = -1.0f;
		orderedParams[i] = i;
	}
	orderedParams[6] = ButtonA;
	orderedParams[7] = ButtonB;

	for (int i = 0; i < PresetNil; i++)
	{
		cvAddParam[i] = -1;
	}

	modeNames[Vocoder] = "VOCODER1";
	shortModeNames[Vocoder] = "VL";
	modeNamesDetails[Vocoder] = "LPC";
	numPages[Vocoder] = 2;
	knobParamNames[Vocoder][0] = "VOLUME";
	knobParamNames[Vocoder][1] = "WARP";
	knobParamNames[Vocoder][2] = "QUALITY";
	knobParamNames[Vocoder][3] = "SAWtoPULSE";
	knobParamNames[Vocoder][4] = "NOISETHRESH";
	knobParamNames[Vocoder][5] = "BREATHINESS";
	knobParamNames[Vocoder][6] = "PULSEWIDTH";
	knobParamNames[Vocoder][7] = "PULSESHAPE";
	knobParamNames[Vocoder][8] = "";
	knobParamNames[Vocoder][9] = "";


	modeNames[VocoderCh] = "VOCODER2";
	shortModeNames[VocoderCh] = "VC";
	modeNamesDetails[VocoderCh] = "CHANNEL";
	numPages[VocoderCh] = 3;
	knobParamNames[VocoderCh][0] = "VOLUME";
	knobParamNames[VocoderCh][1] = "WARP";
	knobParamNames[VocoderCh][2] = "QUALITY";
	knobParamNames[VocoderCh][3] = "BANDWIDTH";
	knobParamNames[VocoderCh][4] = "NOISTHRESH";
	knobParamNames[VocoderCh][5] = "SAWtoPULSE";
	knobParamNames[VocoderCh][6] = "PULSEWIDTH";
	knobParamNames[VocoderCh][7] = "PULSESHAPE";
	knobParamNames[VocoderCh][8] = "BREATHINESS";
	knobParamNames[VocoderCh][9] = "SPEED";
	knobParamNames[VocoderCh][10] = "BANDSQUISH";
	knobParamNames[VocoderCh][11] = "BANDOFFSET";
	knobParamNames[VocoderCh][12] = "TILT";
	knobParamNames[VocoderCh][13] = "STEREO";
	knobParamNames[VocoderCh][14] = "BARKPULL";

	modeNames[Pitchshift] = "PITCHSHIFT";
	shortModeNames[Pitchshift] = "PS";
	modeNamesDetails[Pitchshift] = "";
	numPages[Pitchshift] = 1;
	knobParamNames[Pitchshift][0] = "SHIFT";
	knobParamNames[Pitchshift][1] = "FINE";
	knobParamNames[Pitchshift][2] = "F AMT";
	knobParamNames[Pitchshift][3] = "FORMANT";
	knobParamNames[Pitchshift][4] = "";
	knobParamNames[Pitchshift][5] = "";

	modeNames[AutotuneMono] = "AUTOTUNE";
	shortModeNames[AutotuneMono] = "NT";
	modeNamesDetails[AutotuneMono] = "";
	numPages[AutotuneMono] = 1;
	knobParamNames[AutotuneMono][0] = "FID THRESH";
	knobParamNames[AutotuneMono][1] = "AMOUNT";
	knobParamNames[AutotuneMono][2] = "SPEED";
	knobParamNames[AutotuneMono][3] = "";
	knobParamNames[AutotuneMono][4] = "";
	knobParamNames[AutotuneMono][5] = "";

	modeNames[AutotunePoly] = "HARMONIZE";
	shortModeNames[AutotunePoly] = "AT";
	modeNamesDetails[AutotunePoly] = "";
	numPages[AutotunePoly] = 1;
	knobParamNames[AutotunePoly][0] = "FID THRESH";
	knobParamNames[AutotunePoly][1] = "ALPHA";
	knobParamNames[AutotunePoly][2] = "TOLERANCE";
	knobParamNames[AutotunePoly][3] = "";
	knobParamNames[AutotunePoly][4] = "";
	knobParamNames[AutotunePoly][5] = "";

	modeNames[SamplerButtonPress] = "SAMPLER BP";
	shortModeNames[SamplerButtonPress] = "SB";
	modeNamesDetails[SamplerButtonPress] = "PRESS BUTTON A";
	numPages[SamplerButtonPress] = 1;
	knobParamNames[SamplerButtonPress][0] = "START";
	knobParamNames[SamplerButtonPress][1] = "LENGTH";
	knobParamNames[SamplerButtonPress][2] = "SPEED";
	knobParamNames[SamplerButtonPress][3] = "CROSSFADE";
	knobParamNames[SamplerButtonPress][4] = "";
	knobParamNames[SamplerButtonPress][5] = "";

	modeNames[SamplerKeyboard] = "KEYSAMPLER";
	shortModeNames[SamplerKeyboard] = "KS";
	modeNamesDetails[SamplerKeyboard] = "A OR KEY TO REC";
	numPages[SamplerKeyboard] = 1;
	knobParamNames[SamplerKeyboard][0] = "START";
	knobParamNames[SamplerKeyboard][1] = "LENGTH";
	knobParamNames[SamplerKeyboard][2] = "SPEED";
	knobParamNames[SamplerKeyboard][3] = "CROSSFADE";
	knobParamNames[SamplerKeyboard][4] = "";
	knobParamNames[SamplerKeyboard][5] = "";

	modeNames[SamplerAutoGrab] = "AUTOSAMP";
	shortModeNames[SamplerAutoGrab] = "AS";
	modeNamesDetails[SamplerAutoGrab] = "AUDIO TRIG'D";
	numPages[SamplerAutoGrab] = 2;
	knobParamNames[SamplerAutoGrab][0] = "THRESHOLD";
	knobParamNames[SamplerAutoGrab][1] = "WINDOW";
	knobParamNames[SamplerAutoGrab][2] = "SPEED";
	knobParamNames[SamplerAutoGrab][3] = "CROSSFADE";
	knobParamNames[SamplerAutoGrab][4] = "";
	knobParamNames[SamplerAutoGrab][5] = "LENRAND+/-";
	knobParamNames[SamplerAutoGrab][6] = "SPDRAND+/-";
	knobParamNames[SamplerAutoGrab][7] = "";
	knobParamNames[SamplerAutoGrab][8] = "";
	knobParamNames[SamplerAutoGrab][9] = "";

	modeNames[Distortion] = "DISTORT";
	shortModeNames[Distortion] = "DT";
	modeNamesDetails[Distortion] = "WITH EQ";
	numPages[Distortion] = 1;
	knobParamNames[Distortion][0] = "PRE GAIN";
	knobParamNames[Distortion][1] = "TILT";
	knobParamNames[Distortion][2] = "MID GAIN";
	knobParamNames[Distortion][3] = "MID FREQ";
	knobParamNames[Distortion][4] = "POST GAIN";
	knobParamNames[Distortion][5] = "";

	modeNames[Wavefolder] = "WAVEFOLD";
	shortModeNames[Wavefolder] = "WF";
	modeNamesDetails[Wavefolder] = "SERGE STYLE";
	numPages[Wavefolder] = 1;
	knobParamNames[Wavefolder][0] = "GAIN";
	knobParamNames[Wavefolder][1] = "OFFSET1";
	knobParamNames[Wavefolder][2] = "OFFSET2";
	knobParamNames[Wavefolder][3] = "POST GAIN";
	knobParamNames[Wavefolder][4] = "";
	knobParamNames[Wavefolder][5] = "";

	modeNames[BitCrusher] = "BITCRUSH";
	shortModeNames[BitCrusher] = "BC";
	modeNamesDetails[BitCrusher] = "AHH HALP ME";
	numPages[BitCrusher] = 2;
	knobParamNames[BitCrusher][0] = "QUALITY";
	knobParamNames[BitCrusher][1] = "SAMP RATIO";
	knobParamNames[BitCrusher][2] = "ROUNDING";
	knobParamNames[BitCrusher][3] = "OPERATION";
	knobParamNames[BitCrusher][4] = "POST GAIN";
	knobParamNames[BitCrusher][5] = "PRE GAIN";

	modeNames[Delay] = "DELAY";
	shortModeNames[Delay] = "DL";
	modeNamesDetails[Delay] = "STEREO";
	numPages[Delay] = 1;
	knobParamNames[Delay][0] = "DELAY_L";
	knobParamNames[Delay][1] = "DELAY_R";
	knobParamNames[Delay][2] = "FEEDBACK";
	knobParamNames[Delay][3] = "LOWPASS";
	knobParamNames[Delay][4] = "HIGHPASS";
	knobParamNames[Delay][5] = "";

	modeNames[Reverb] = "REVERB1";
	shortModeNames[Reverb] = "RV";
	modeNamesDetails[Reverb] = "DATTORRO ALG";
	numPages[Reverb] = 1;
	knobParamNames[Reverb][0] = "SIZE";
	knobParamNames[Reverb][1] = "IN LOPASS";
	knobParamNames[Reverb][2] = "IN HIPASS";
	knobParamNames[Reverb][3] = "FB LOPASS";
	knobParamNames[Reverb][4] = "FB GAIN";
	knobParamNames[Reverb][5] = "";

	modeNames[Reverb2] = "REVERB2";
	shortModeNames[Reverb2] = "RV";
	modeNamesDetails[Reverb2] = "NREVERB ALG";
	numPages[Reverb2] = 1;
	knobParamNames[Reverb2][0] = "SIZE";
	knobParamNames[Reverb2][1] = "LOWPASS";
	knobParamNames[Reverb2][2] = "HIGHPASS";
	knobParamNames[Reverb2][3] = "PEAK_FREQ";
	knobParamNames[Reverb2][4] = "PEAK_GAIN";
	knobParamNames[Reverb2][5] = "";

	modeNames[LivingString] = "STRING1";
	shortModeNames[LivingString] = "LS";
	modeNamesDetails[LivingString] = "SYMP STRING";
	numPages[LivingString] = 2;
	knobParamNames[LivingString][0] = "FREQ";
	knobParamNames[LivingString][1] = "DETUNE";
	knobParamNames[LivingString][2] = "DECAY";
	knobParamNames[LivingString][3] = "DAMPING";
	knobParamNames[LivingString][4] = "PICK POS";
	knobParamNames[LivingString][5] = "PREP POS";
	knobParamNames[LivingString][6] = "PREP INDEX";

	modeNames[LivingStringSynth] = "STRING2";
	shortModeNames[LivingStringSynth] = "SS";
	modeNamesDetails[LivingStringSynth] = "STRING SYNTH";
	numPages[LivingStringSynth] = 1;
	knobParamNames[LivingStringSynth][0] = "";
	knobParamNames[LivingStringSynth][1] = "";
	knobParamNames[LivingStringSynth][2] = "DECAY";
	knobParamNames[LivingStringSynth][3] = "DAMPING";
	knobParamNames[LivingStringSynth][4] = "PICK_POS";
	knobParamNames[LivingStringSynth][5] = "";

	modeNames[ClassicSynth] = "POLYSYNTH";
	shortModeNames[ClassicSynth] = "CS";
	modeNamesDetails[ClassicSynth] = "VCO+VCF";
	numPages[ClassicSynth] = 4;
	knobParamNames[ClassicSynth][0] = "VOLUME";
	knobParamNames[ClassicSynth][1] = "LOWPASS";
	knobParamNames[ClassicSynth][2] = "KEYFOLLOW";
	knobParamNames[ClassicSynth][3] = "DETUNE";
	knobParamNames[ClassicSynth][4] = "FILTER Q";
	knobParamNames[ClassicSynth][5] = "ATTACK";
	knobParamNames[ClassicSynth][6] = "DECAY";
	knobParamNames[ClassicSynth][7] = "SUSTAIN";
	knobParamNames[ClassicSynth][8] = "RELEASE";
	knobParamNames[ClassicSynth][9] = "LEAK";
	knobParamNames[ClassicSynth][10] = "F_ATTACK";
	knobParamNames[ClassicSynth][11] = "F_DECAY";
	knobParamNames[ClassicSynth][12] = "F_SUSTAIN";
	knobParamNames[ClassicSynth][13] = "F_RELEASE";
	knobParamNames[ClassicSynth][14] = "F_LEAK";
	knobParamNames[ClassicSynth][15] = "F_AMOUNT";
	knobParamNames[ClassicSynth][16] = "SAW/PULSE";
	knobParamNames[ClassicSynth][17] = "";
	knobParamNames[ClassicSynth][18] = "";
	knobParamNames[ClassicSynth][19] = "";

	modeNames[Rhodes] = "RHODES";
	shortModeNames[Rhodes] = "RD";
	modeNamesDetails[Rhodes] = "DARK";
	numPages[Rhodes] = 2;
	knobParamNames[Rhodes][0] = "BRIGHTNESS";
	knobParamNames[Rhodes][1] = "TREM DEPTH";
	knobParamNames[Rhodes][2] = "TREM RATE";
	knobParamNames[Rhodes][3] = "DRIVE";
	knobParamNames[Rhodes][4] = "PAN SPREAD";
	knobParamNames[Rhodes][5] = "ATK MULT";
	knobParamNames[Rhodes][6] = "DEC MULT";
	knobParamNames[Rhodes][7] = "SUSTAIN";
	knobParamNames[Rhodes][8] = "RELEASE";
	knobParamNames[Rhodes][9] = "LEAK";
	knobParamNames[Rhodes][10] = "INDEX1";
	knobParamNames[Rhodes][11] = "INDEX2";
	knobParamNames[Rhodes][12] = "INDEX3";
	knobParamNames[Rhodes][13] = "INDEX4";
	knobParamNames[Rhodes][14] = "INDEX5";
	knobParamNames[Rhodes][15] = "FEEDBACK";
	knobParamNames[Rhodes][16] = "RATIO1";
	knobParamNames[Rhodes][17] = "RATIO2";
	knobParamNames[Rhodes][18] = "RATIO3";
	knobParamNames[Rhodes][19] = "RATIO4";
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
			if (buttonValues[i] != buttonValuesPrev[i])
			{
				buttonHysteresis[i]++;
			}
			if (cleanButtonValues[i] == 1)
			{
				buttonActionsSFX[i][ActionHoldContinuous] = TRUE;
				buttonActionsUI[i][ActionHoldContinuous] = TRUE;
				writeButtonFlag = i;
				writeActionFlag = ActionHoldContinuous;
			}
			if (buttonHysteresis[i] < buttonHysteresisThreshold)
			{
				if (buttonCounters[i] < buttonHoldMax) buttonCounters[i]++;
				if ((buttonCounters[i] >= buttonHoldThreshold) && (cleanButtonValues[i] == 1))
				{
					buttonActionsSFX[i][ActionHoldInstant] = TRUE;
					buttonActionsUI[i][ActionHoldInstant] = TRUE;
					writeButtonFlag = i;
					writeActionFlag = ActionHoldInstant;
				}
			}
			else
			{
				cleanButtonValues[i] = buttonValues[i];
				buttonHysteresis[i] = 0;
				buttonCounters[i] = 0;

				if (cleanButtonValues[i] == 1)
				{
					buttonActionsSFX[i][ActionPress] = TRUE;
					buttonActionsUI[i][ActionPress] = TRUE;
					writeButtonFlag = i;
					writeActionFlag = ActionPress;
				}
				else if (cleanButtonValues[i] == 0)
				{
					buttonActionsSFX[i][ActionRelease] = TRUE;
					buttonActionsUI[i][ActionRelease] = TRUE;
					writeButtonFlag = i;
					writeActionFlag = ActionRelease;
				}
				buttonValuesPrev[i] = buttonValues[i];
			}
		}

		// make some if statements if you want to find the "attack" of the buttons (getting the "press" action)
		// we'll need if statements for each button  - maybe should go to functions that are dedicated to each button?

		/// DEFINE GLOBAL BUTTON BEHAVIOR HERE

		if (buttonActionsUI[ButtonLeft][ActionPress] == 1)
		{
			previousPreset = currentPreset;

			if (currentPreset <= 0) currentPreset = PresetNil - 1;
			else currentPreset--;

			loadingPreset = 1;
			OLED_writePreset();
			writeCurrentPresetToFlash();
			clearButtonActions();
		}
		// right press
		if (buttonActionsUI[ButtonRight][ActionPress] == 1)
		{
			previousPreset = currentPreset;
			if (currentPreset >= PresetNil - 1) currentPreset = 0;
			else currentPreset++;

			loadingPreset = 1;
			OLED_writePreset();
			writeCurrentPresetToFlash();
			clearButtonActions();
		}
		if (buttonActionsUI[ButtonC][ActionPress] == 1)
		{
			if (buttonActionsUI[ButtonEdit][ActionHoldContinuous] == 0) buttonActionsUI[ButtonC][ActionPress] = 0;
		}
		if (buttonActionsUI[ButtonD][ActionPress] == 1)
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
			OLED_writeTuning();
			buttonActionsUI[ButtonD][ActionPress] = 0;
		}
		if (buttonActionsUI[ButtonE][ActionPress] == 1)
		{

			currentTuning = (currentTuning + 1) % NUM_TUNINGS;
			changeTuning();
			OLED_writeTuning();
			buttonActionsUI[ButtonE][ActionPress] = 0;
		}

		if (buttonActionsUI[ButtonEdit][ActionPress])
		{
			OLED_writeEditScreen();
			buttonActionsUI[ButtonEdit][ActionPress] = 0;
		}
		if (buttonActionsUI[ButtonEdit][ActionHoldContinuous] == 1)
		{
			if (buttonActionsUI[ButtonC][ActionPress] == 1)
			{
				keyCenter = (keyCenter + 1) % 12;
				OLEDclearLine(SecondLine);
				OLEDwriteString("KEY: ", 5, 0, SecondLine);
				OLEDwritePitchClass(keyCenter+60, 64, SecondLine);
				buttonActionsUI[ButtonC][ActionPress] = 0;
			}
			if (buttonActionsUI[ButtonDown][ActionPress])
			{
				cvAddParam[currentPreset] = -1;
				buttonActionsUI[ButtonDown][ActionPress] = 0;
			}
//			OLEDdrawFloatArray(audioDisplayBuffer, -1.0f, 1.0f, 128, displayBufferIndex, 0, BothLines);
		}
		if (buttonActionsUI[ButtonEdit][ActionRelease] == 1)
		{
			OLED_writePreset();
			buttonActionsUI[ButtonEdit][ActionRelease] = 0;
		}
		if (buttonActionsUI[ButtonDown][ActionPress] == 1)
		{

			decrementPage();
			OLEDwriteString("P", 1, 110, FirstLine);
			OLEDwriteInt(knobPage, 1, 120, FirstLine);
			buttonActionsUI[ButtonDown][ActionPress] = 0;
		}
		if (buttonActionsUI[ButtonUp][ActionPress] == 1)
		{
			incrementPage();
			OLEDwriteString("P", 1, 110, FirstLine);
			OLEDwriteInt(knobPage, 1, 120, FirstLine);
			buttonActionsUI[ButtonUp][ActionPress] = 0;
		}

		// Trying out an audio display
//		if (buttonActionsUI[ButtonEdit][ActionPress] == 1)
//		{
//			currentParamIndex++;
//			if (currentParamIndex > 7) currentParamIndex = 0;
//			int controlLen = strlen(controlNames[currentParamIndex]);
//			OLEDwriteString(controlNames[currentParamIndex], controlLen, 0, SecondLine);
//			OLEDwriteString(" ", 1, getCursorX(), SecondLine);
//			OLEDwriteString(paramNames[orderedParams[currentParamIndex]], getCursorX(), SecondLine);
//			buttonActionsUI[ButtonEdit][ActionPress] = 0;
//		}
	}
}

void adcCheck()
{
	//read the analog inputs and smooth them with ramps
	for (int i = 0; i < 6; i++)
	{
		//floatADC[i] = (float) (ADC_values[i]>>8) * INV_TWO_TO_8;
		floatADC[i] = (float) (ADC_values[i]>>6) * INV_TWO_TO_10;
	}
	for (int i = 0; i < 6; i++)
	{
		if (fastabsf(floatADC[i] - lastFloatADC[i]) > adcHysteresisThreshold)
		{
			if (buttonActionsUI[ButtonEdit][ActionHoldContinuous])
			{
				if (i != 5) cvAddParam[currentPreset] = i + (knobPage * KNOB_PAGE_SIZE);;
				buttonActionsUI[ButtonEdit][ActionHoldContinuous] = 0;
			}
			lastFloatADC[i] = floatADC[i];
			if (i == 5) writeKnobFlag = cvAddParam[currentPreset] - (knobPage * KNOB_PAGE_SIZE);
			else writeKnobFlag = i;
			knobActive[i] = 1;
		}
		// only do the following check after the knob has already passed the above
		// check once and floatADCUI has been set in OLED_writeKnobParameter
		if (floatADCUI[i] >= 0.0f)
		{
			if (fastabsf(smoothedADC[i] - floatADCUI[i]) > adcHysteresisThreshold)
			{
				if (i == 5) writeKnobFlag = cvAddParam[currentPreset] - (knobPage * KNOB_PAGE_SIZE);
				else writeKnobFlag = i;
			}
		}
		if (knobActive[i]) tExpSmooth_setDest(&adc[i], floatADC[i]);
	}
}

void clearButtonActions()
{
	for (int b = 0; b < ButtonNil; b++)
	{
		for (int a = 0; a < ActionNil; a++)
		{
			buttonActionsUI[b][a] = 0;
			buttonActionsSFX[b][a] = 0;
			writeButtonFlag = -1;
			writeActionFlag = -1;
		}
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
		calculateNoteArray();
	}
}

void writeCurrentPresetToFlash(void)
{
	if((EE_WriteVariable(VirtAddVarTab[0],  currentPreset)) != HAL_OK)
	{
		Error_Handler();
	}
}

void incrementPage(void)
{
	knobPage = (knobPage + 1) % numPages[currentPreset];
	setKnobValues(presetKnobValues[currentPreset] + (knobPage * KNOB_PAGE_SIZE));

}

void decrementPage(void)
{
	if (knobPage == 0) knobPage = numPages[currentPreset] - 1;
	else knobPage--;
	setKnobValues(presetKnobValues[currentPreset] + (knobPage * KNOB_PAGE_SIZE));
}

void resetKnobValues(void)
{
	for (int i = 0; i < NUM_ADC_CHANNELS; i++)
	{
		knobActive[i] = 0;
		floatADCUI[i] = -1.0f;
		float value = 0.0f;
		if (i != 5) value = presetKnobValues[currentPreset][i + (knobPage * KNOB_PAGE_SIZE)];
		tExpSmooth_setValAndDest(&adc[i], value);
		smoothedADC[i] = value;
	}
}

void setKnobValues(float* values)
{
	for (int i = 0; i < KNOB_PAGE_SIZE; i++)
	{
		int knob = i;
		// if the knob is being replaced by cv pedal, set cv pedal instead
		if (knob + (knobPage * KNOB_PAGE_SIZE) == cvAddParam[currentPreset])
		{
			knob = 5;
		}
		knobActive[knob] = 0;
		floatADCUI[knob] = -1.0f;
		tExpSmooth_setValAndDest(&adc[knob], values[knob]);

		smoothedADC[knob] = values[knob];
	}
}

void setKnobValue(int knob, float value)
{
	// if the knob is being replaced by cv pedal, set cv pedal instead
	if (knob + (knobPage * KNOB_PAGE_SIZE) == cvAddParam[currentPreset])
	{
		knob = 5;
	}
	knobActive[knob] = 0;
	floatADCUI[knob] = -1.0f;
	tExpSmooth_setValAndDest(&adc[knob], value);
	smoothedADC[knob] = value;
}

void deactivateKnob(int knob)
{
	knobActive[knob] = 0;
	floatADCUI[knob] = -1.0f;
}

void deactivateAllKnobs()
{
	for (int i = 0; i < NUM_ADC_CHANNELS; i++)
	{
		knobActive[i] = 0;
		floatADCUI[i] = -1.0f;
	}
}

char* UIVocoderButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress] == 1)
	{
		writeString = (numVoices > 1) ? "POLY" : "MONO";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonB][ActionPress] == 1)
	{
		writeString = internalExternal ? "EXTERNAL" : "INTERNAL";
		buttonActionsUI[ButtonB][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonC][ActionPress] == 1)
	{
		writeString = vocChFreeze ? "FROZEN" : "UNFROZEN";
		buttonActionsUI[ButtonC][ActionPress] = 0;
	}
	return writeString;
}

char* UIVocoderChButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress] == 1)
	{
		writeString = (numVoices > 1) ? "POLY" : "MONO";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonB][ActionPress] == 1)
	{
		writeString = internalExternal ? "EXTERNAL" : "INTERNAL";
		buttonActionsUI[ButtonB][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonC][ActionPress] == 1)
	{
		writeString = vocChFreeze ? "FROZEN" : "UNFROZEN";
		buttonActionsUI[ButtonC][ActionPress] = 0;
	}
	return writeString;
}

char* UIPitchShiftButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}

char* UINeartuneButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress])
	{
		writeString = autotuneChromatic ? "AUTOCHROM ON" : "AUTOCHROM OFF";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	return writeString;
}

char* UIAutotuneButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}

char* UISamplerBPButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonDown][ActionPress])
	{
		OLEDclearLine(SecondLine);
		OLEDwriteFloat(sampleLength, 0, SecondLine);
		OLEDwriteString(samplePlaying ? "PLAYING" : "STOPPED", 7, 48, SecondLine);
		buttonActionsUI[ButtonDown][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonA][ActionHoldContinuous])
	{
		OLEDclearLine(SecondLine);
		OLEDwriteString("RECORDING", 9, 0, SecondLine);
		OLEDwriteFloat(sampleLength, 84, SecondLine);
		buttonActionsUI[ButtonA][ActionHoldContinuous] = 0;
	}
	if (buttonActionsUI[ButtonA][ActionRelease])
	{
		OLEDclearLine(SecondLine);
		OLEDwriteFloat(sampleLength, 0, SecondLine);
		OLEDwriteString(samplePlaying ? "PLAYING" : "STOPPED", 7, 48, SecondLine);
		buttonActionsUI[ButtonA][ActionRelease] = 0;
	}
	return writeString;
}

char* UISamplerKButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";

	// should try to clean this up somehow...
	if (((buttonActionsUI[ButtonA][ActionRelease] || buttonActionsUI[ButtonB][ActionPress]) ||
		 (buttonActionsUI[ButtonDown][ActionPress] || buttonActionsUI[ButtonUp][ActionPress])) ||
		 (buttonActionsUI[ButtonC][ActionHoldContinuous] || buttonActionsUI[ButtonA][ActionHoldContinuous]))
		// ^ these are some dummy values that get set in sfx.c frame so we can trigger this based on midi
	{
		OLEDclearLine(SecondLine);
		OLEDwritePitch(currentSamplerKey + LOWEST_SAMPLER_KEY, 0, SecondLine, false);
		OLEDwriteFloat(sampleLength, OLEDgetCursor(), SecondLine);
		buttonActionsUI[ButtonA][ActionRelease] = 0;
		buttonActionsUI[ButtonA][ActionHoldContinuous] = 0;
		buttonActionsUI[ButtonB][ActionPress] = 0;
		buttonActionsUI[ButtonDown][ActionPress] = 0;
		buttonActionsUI[ButtonUp][ActionPress] = 0;
		buttonActionsUI[ButtonC][ActionHoldContinuous] = 0;
	}
	return writeString;
}

char* UISamplerAutoButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress])
	{
		if (samplerMode == PlayLoop)
		{
			writeString = "LOOP";
		}
		else if (samplerMode == PlayBackAndForth)
		{
			writeString = "BACK'N'FORTH";
		}
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonB][ActionPress])
	{
		writeString = triggerChannel ? "CH2 TRIG" : "CH1 TRIG";
		buttonActionsUI[ButtonB][ActionPress] = 0;
	}
	return writeString;
}

char* UIDistortionButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress])
	{
		writeString = distortionMode ? "SHAPER" : "TANH";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	return writeString;
}

char* UIWaveFolderButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress])
	{
		writeString = foldMode ? "TWO IN SERIES" : "OVERSAMPLED";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	return writeString;
}

char* UIBitcrusherButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}

char* UIDelayButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress])
	{
		writeString = delayShaper ? "SHAPER ON" : "SHAPER OFF";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonB][ActionPress])
	{
		writeString = capFeedback ? "FB CAP" : "FB UNCAP";
		buttonActionsUI[ButtonB][ActionPress] = 0;
	}
	return writeString;
}

char* UIReverbButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress])
	{
		writeString = freeze ? "FREEZE" : "UNFREEZE";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonB][ActionPress])
	{
		writeString = capFeedback ? "FB CAP" : "FB UNCAP";
		buttonActionsUI[ButtonB][ActionPress] = 0;
	}
	return writeString;
}

char* UIReverb2Buttons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress])
	{
		writeString = freeze ? "FREEZE" : "UNFREEZE";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	return writeString;
}

char* UILivingStringButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}

char* UILivingStringSynthButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress] == 1)
	{
		writeString = (numVoices > 1) ? "POLY" : "MONO";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	return writeString;
}

char* UIClassicSynthButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress] == 1)
	{
		writeString = (numVoices > 1) ? "POLY" : "MONO";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonB][ActionPress])
	{
		//writeString = knobPage == 0 ? "SETTINGS" : "ADSR";
		buttonActionsUI[ButtonB][ActionPress] = 0;
	}
	return writeString;
}

char* UIRhodesButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress] == 1)
	{
		writeString = (numVoices > 1) ? "POLY" : "MONO";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	if (buttonActionsSFX[ButtonB][ActionPress] == 1)
	{
		buttonActionsSFX[ButtonB][ActionPress] = 0;
		Rsound = (Rsound + 1 ) % 4; // switch to another rhodes preset sound
		OLEDclearLine(SecondLine);
		OLEDwriteString(soundNames[Rsound], 6, 0, SecondLine);
	}

	return writeString;
}
