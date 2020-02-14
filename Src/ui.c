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

uint16_t ADC_values[NUM_ADC_CHANNELS] __ATTR_RAM_D2;
float floatADC[NUM_ADC_CHANNELS];
float lastFloatADC[NUM_ADC_CHANNELS];
float adcHysteresisThreshold = 0.001f;

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

char* controlNames[NUM_ADC_CHANNELS + NUM_BUTTONS];
char* paramNames[PresetNil][NUM_ADC_CHANNELS + NUM_BUTTONS];
int8_t currentParamIndex = -1;
uint8_t orderedParams[8];

uint8_t buttonActionsSFX[NUM_BUTTONS][ActionNil];
uint8_t buttonActionsUI[NUM_BUTTONS][ActionNil];
float knobParams[NUM_ADC_CHANNELS];
char* (*buttonActionFunctions[PresetNil])(VocodecButton, ButtonAction);

VocodecPresetType currentPreset = 0;
VocodecPresetType previousPreset = PresetNil;
uint8_t loadingPreset = 0;

void initModeNames(void)
{
	controlNames[0] = "1";
	controlNames[1] = "2";
	controlNames[2] = "3";
	controlNames[3] = "4";
	controlNames[4] = "5";
	controlNames[5] = "6";
	controlNames[ButtonA] = "A";
	controlNames[ButtonB] = "B";
	for (int i = 0; i < NUM_ADC_CHANNELS; i++)
	{
		orderedParams[i] = i;
	}
	orderedParams[6] = ButtonA;
	orderedParams[7] = ButtonB;

	modeNames[VocoderInternalPoly] = "VOCODER IP";
	shortModeNames[VocoderInternalPoly] = "V1";
	modeNamesDetails[VocoderInternalPoly] = "INTERNAL POLY";
	paramNames[VocoderInternalPoly][0] = "VOLUME";
	paramNames[VocoderInternalPoly][1] = "";
	paramNames[VocoderInternalPoly][2] = "";
	paramNames[VocoderInternalPoly][3] = "";
	paramNames[VocoderInternalPoly][4] = "";
	paramNames[VocoderInternalPoly][5] = "";
	paramNames[VocoderInternalPoly][ButtonA] = "";
	paramNames[VocoderInternalPoly][ButtonB] = "";

	modeNames[VocoderInternalMono] = "VOCODER IM";
	shortModeNames[VocoderInternalMono] = "V2";
	modeNamesDetails[VocoderInternalMono] = "INTERNAL MONO";
	paramNames[VocoderInternalMono][0] = "VOLUME";
	paramNames[VocoderInternalMono][1] = "";
	paramNames[VocoderInternalMono][2] = "";
	paramNames[VocoderInternalMono][3] = "";
	paramNames[VocoderInternalMono][4] = "";
	paramNames[VocoderInternalMono][5] = "";
	paramNames[VocoderInternalMono][ButtonA] = "";
	paramNames[VocoderInternalMono][ButtonB] = "";

	modeNames[VocoderExternal] = "VOCODEC E";
	shortModeNames[VocoderExternal] = "VE";
	modeNamesDetails[VocoderExternal] = "EXTERNAL";
	paramNames[VocoderExternal][0] = "";
	paramNames[VocoderExternal][1] = "";
	paramNames[VocoderExternal][2] = "";
	paramNames[VocoderExternal][3] = "";
	paramNames[VocoderExternal][4] = "";
	paramNames[VocoderExternal][5] = "";
	paramNames[VocoderExternal][ButtonA] = "";
	paramNames[VocoderExternal][ButtonB] = "";

	modeNames[Pitchshift] = "PITCHSHIFT";
	shortModeNames[Pitchshift] = "PS";
	modeNamesDetails[Pitchshift] = "";
	paramNames[Pitchshift][0] = "PITCH";
	paramNames[Pitchshift][1] = "FINE PITCH";
	paramNames[Pitchshift][2] = "F AMT";
	paramNames[Pitchshift][3] = "FORMANT";
	paramNames[Pitchshift][4] = "";
	paramNames[Pitchshift][5] = "";
	paramNames[Pitchshift][ButtonA] = "";
	paramNames[Pitchshift][ButtonB] = "";

	modeNames[AutotuneMono] = "NEARTUNE";
	shortModeNames[AutotuneMono] = "NT";
	modeNamesDetails[AutotuneMono] = "";
	paramNames[AutotuneMono][0] = "AMOUNT";
	paramNames[AutotuneMono][1] = "SPEED";
	paramNames[AutotuneMono][2] = "";
	paramNames[AutotuneMono][3] = "";
	paramNames[AutotuneMono][4] = "";
	paramNames[AutotuneMono][5] = "";
	paramNames[AutotuneMono][ButtonA] = "AUTOCHRM ON";
	paramNames[AutotuneMono][ButtonB] = "AUTOCHRM OFF";

	modeNames[AutotunePoly] = "AUTOTUNE";
	shortModeNames[AutotunePoly] = "AT";
	modeNamesDetails[AutotunePoly] = "";
	paramNames[AutotunePoly][0] = "";
	paramNames[AutotunePoly][1] = "";
	paramNames[AutotunePoly][2] = "";
	paramNames[AutotunePoly][3] = "";
	paramNames[AutotunePoly][4] = "";
	paramNames[AutotunePoly][5] = "";
	paramNames[AutotunePoly][ButtonA] = "";
	paramNames[AutotunePoly][ButtonB] = "";

	modeNames[SamplerButtonPress] = "SAMPLER BP";
	shortModeNames[SamplerButtonPress] = "SB";
	modeNamesDetails[SamplerButtonPress] = "PRESS BUTTON A";
	paramNames[SamplerButtonPress][0] = "START";
	paramNames[SamplerButtonPress][1] = "END";
	paramNames[SamplerButtonPress][2] = "SPEED";
	paramNames[SamplerButtonPress][3] = "CROSSFADE";
	paramNames[SamplerButtonPress][4] = "";
	paramNames[SamplerButtonPress][5] = "";
	paramNames[SamplerButtonPress][ButtonA] = "";
	paramNames[SamplerButtonPress][ButtonB] = "";

	modeNames[SamplerAutoGrabInternal] = "AUTOSAMP1";
	shortModeNames[SamplerAutoGrabInternal] = "A1";
	modeNamesDetails[SamplerAutoGrabInternal] = "CH1 TRIG";
	paramNames[SamplerAutoGrabInternal][0] = "THRESHOLD";
	paramNames[SamplerAutoGrabInternal][1] = "WINDOW";
	paramNames[SamplerAutoGrabInternal][2] = "REL THRESH";
	paramNames[SamplerAutoGrabInternal][3] = "CROSSFADE";
	paramNames[SamplerAutoGrabInternal][4] = "";
	paramNames[SamplerAutoGrabInternal][5] = "";
	paramNames[SamplerAutoGrabInternal][ButtonA] = "";
	paramNames[SamplerAutoGrabInternal][ButtonB] = "";

	modeNames[SamplerAutoGrabExternal] = "AUTOSAMP2";
	shortModeNames[SamplerAutoGrabExternal] = "A2";
	modeNamesDetails[SamplerAutoGrabExternal] = "CH2 TRIG";
	paramNames[SamplerAutoGrabExternal][0] = "THRESHOLD";
	paramNames[SamplerAutoGrabExternal][1] = "WINDOW";
	paramNames[SamplerAutoGrabExternal][2] = "REL THRESH";
	paramNames[SamplerAutoGrabExternal][3] = "CROSSFADE";
	paramNames[SamplerAutoGrabExternal][4] = "";
	paramNames[SamplerAutoGrabExternal][5] = "";
	paramNames[SamplerAutoGrabExternal][ButtonA] = "";
	paramNames[SamplerAutoGrabExternal][ButtonB] = "";

	modeNames[DistortionTanH] = "DISTORT1";
	shortModeNames[DistortionTanH] = "D1";
	modeNamesDetails[DistortionTanH] = "TANH FUNCTION";
	paramNames[DistortionTanH][0] = "GAIN";
	paramNames[DistortionTanH][1] = "";
	paramNames[DistortionTanH][2] = "";
	paramNames[DistortionTanH][3] = "";
	paramNames[DistortionTanH][4] = "";
	paramNames[DistortionTanH][5] = "";
	paramNames[DistortionTanH][ButtonA] = "";
	paramNames[DistortionTanH][ButtonB] = "";

	modeNames[DistortionShaper] = "DISTORT2";
	shortModeNames[DistortionShaper] = "D2";
	modeNamesDetails[DistortionShaper] = "WAVESHAPER";
	paramNames[DistortionShaper][0] = "GAIN";
	paramNames[DistortionShaper][1] = "";
	paramNames[DistortionShaper][2] = "";
	paramNames[DistortionShaper][3] = "";
	paramNames[DistortionShaper][4] = "";
	paramNames[DistortionShaper][5] = "";
	paramNames[DistortionShaper][ButtonA] = "";
	paramNames[DistortionShaper][ButtonB] = "";

	modeNames[Wavefolder] = "WAVEFOLD";
	shortModeNames[Wavefolder] = "WF";
	modeNamesDetails[Wavefolder] = "SERGE STYLE";
	paramNames[Wavefolder][0] = "GAIN";
	paramNames[Wavefolder][1] = "OFFSET1";
	paramNames[Wavefolder][2] = "OFFSET2";
	paramNames[Wavefolder][3] = "OFFSET3";
	paramNames[Wavefolder][4] = "";
	paramNames[Wavefolder][5] = "";
	paramNames[Wavefolder][ButtonA] = "";
	paramNames[Wavefolder][ButtonB] = "";

	modeNames[BitCrusher] = "BITCRUSH";
	shortModeNames[BitCrusher] = "BC";
	modeNamesDetails[BitCrusher] = "AHH HALP ME";
	paramNames[BitCrusher][0] = "QUALITY";
	paramNames[BitCrusher][1] = "SAMP RATIO";
	paramNames[BitCrusher][2] = "ROUNDING";
	paramNames[BitCrusher][3] = "OPERATION";
	paramNames[BitCrusher][4] = "GAIN";
	paramNames[BitCrusher][5] = "";
	paramNames[BitCrusher][ButtonA] = "";
	paramNames[BitCrusher][ButtonB] = "";

	modeNames[Delay] = "DELAY";
	shortModeNames[Delay] = "DL";
	modeNamesDetails[Delay] = "";
	paramNames[Delay][0] = "DELAY_L";
	paramNames[Delay][1] = "DELAY_R";
	paramNames[Delay][2] = "FEEDBACK";
	paramNames[Delay][3] = "LOWPASS";
	paramNames[Delay][4] = "HIGHPASS";
	paramNames[Delay][5] = "";
	paramNames[Delay][ButtonA] = "";
	paramNames[Delay][ButtonB] = "";

	modeNames[Reverb] = "REVERB";
	shortModeNames[Reverb] = "RV";
	modeNamesDetails[Reverb] = "DATTORRO ALG";
	paramNames[Reverb][0] = "SIZE";
	paramNames[Reverb][1] = "IN LOPASS";
	paramNames[Reverb][2] = "IN HIPASS";
	paramNames[Reverb][3] = "FB LOPASS";
	paramNames[Reverb][4] = "FB GAIN";
	paramNames[Reverb][5] = "";
	paramNames[Reverb][ButtonA] = "";
	paramNames[Reverb][ButtonB] = "";

	modeNames[Reverb2] = "REVERB2";
	shortModeNames[Reverb2] = "RV";
	modeNamesDetails[Reverb2] = "DATTORRO ALG";
	paramNames[Reverb2][0] = "SIZE";
	paramNames[Reverb2][1] = "LOWPASS";
	paramNames[Reverb2][2] = "HIGHPASS";
	paramNames[Reverb2][3] = "PEAK_FREQ";
	paramNames[Reverb2][4] = "PEAK_GAIN";
	paramNames[Reverb2][5] = "";
	paramNames[Reverb2][ButtonA] = "";
	paramNames[Reverb2][ButtonB] = "";

	modeNames[LivingString] = "STRING";
	shortModeNames[LivingString] = "LS";
	modeNamesDetails[LivingString] = "LIVING STRING";
	paramNames[LivingString][0] = "FREQ";
	paramNames[LivingString][1] = "DETUNE";
	paramNames[LivingString][2] = "DECAY";
	paramNames[LivingString][3] = "DAMPING";
	paramNames[LivingString][4] = "PICK_POS";
	paramNames[LivingString][5] = "";
	paramNames[LivingString][ButtonA] = "";
	paramNames[LivingString][ButtonB] = "";
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

		// TODO: buttons C and E are connected to pins that are used to set up the codec over I2C - we need to reconfigure those pins in some kind of button init after the codec is set up. not done yet.

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
			//GFXsetFont(&theGFX, &DINCondensedBold9pt7b);

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

		// Trying out an audio display
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
//			OLEDdrawFloatArray(audioDisplayBuffer, -1.0f, 1.0f, 128, displayBufferIndex, 0, BothLines);
			buttonActionsUI[ButtonEdit][ActionHoldContinuous] = 0;
		}
		if (buttonActionsUI[ButtonEdit][ActionRelease] == 1)
		{
			OLED_writePreset();
			buttonActionsUI[ButtonEdit][ActionRelease] = 0;
		}

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


		if (fastabsf(floatADC[i] - lastFloatADC[i]) > adcHysteresisThreshold)
		{
			lastFloatADC[i] = floatADC[i];
			writeKnobFlag = i;
		}
		tRamp_setDest(&adc[i], floatADC[i]);
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
		calculatePeriodArray();
	}
}

void writeCurrentPresetToFlash(void)
{
	if((EE_WriteVariable(VirtAddVarTab[0],  currentPreset)) != HAL_OK)
	{
		Error_Handler();
	}
}

char* UIVocoderIPButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";

	return writeString;
}

char* UIVocoderIMButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}

char* UIVocoderEButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
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
		writeString = "AUTOCHROM ON";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonB][ActionPress])
	{
		writeString = "AUTOCHROM OFF";
		buttonActionsUI[ButtonB][ActionPress] = 0;
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
	if (buttonActionsUI[ButtonA][ActionHoldContinuous])
	{
		OLEDclearLine(SecondLine);
		OLEDwriteString("REC ", 3, 0, SecondLine);
		OLEDwriteFloat(sampleLength, getCursorX(), SecondLine);
		buttonActionsUI[ButtonA][ActionHoldContinuous] = 0;
	}
	else if (buttonActionsUI[ButtonA][ActionRelease])
	{
		OLEDclearLine(SecondLine);
		OLEDwriteString("REC ", 3, 0, SecondLine);
		OLEDwriteFloat(sampleLength, getCursorX(), SecondLine);
		buttonActionsUI[ButtonA][ActionRelease] = 0;
	}
	if (buttonActionsUI[ButtonUp][ActionPress])
	{
		writeString = "SAMPLE CLEARED";
		buttonActionsUI[ButtonUp][ActionPress] = 0;
	}
	return writeString;
}

char* UISamplerAuto1Buttons(VocodecButton button, ButtonAction action)
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
	return writeString;
}


char* UISamplerAuto2Buttons(VocodecButton button, ButtonAction action)
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
	return writeString;
}

char* UIDistortionTanhButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}

char* UIDistortionShaperButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}

char* UIWaveFolderButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
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
		writeString = "SHAPER ON";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	if (buttonActionsUI[ButtonB][ActionPress])
	{
		writeString = "SHAPER OFF";
		buttonActionsUI[ButtonB][ActionPress] = 0;
	}
	return writeString;
}

char* UIReverbButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}

char* UIReverb2Buttons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	if (buttonActionsUI[ButtonA][ActionPress])
	{
		if (freeze == 0) writeString = "FREEZE";
		else writeString = "UNFREEZE";
		buttonActionsUI[ButtonA][ActionPress] = 0;
	}
	return writeString;
}

char* UILivingStringButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}
