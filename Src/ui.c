/*
 * ui.c
 *
 *  Created on: Feb 05, 2018
 *      Author: jeffsnyder
 */
#include "main.h"
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
uint32_t buttonHysteresisThreshold = 1;
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
char* knobParamNames[PresetNil][NUM_ADC_CHANNELS];

uint8_t buttonActionsSFX[NUM_BUTTONS][ActionNil];
uint8_t buttonActionsUI[NUM_BUTTONS][ActionNil];
float knobParams[NUM_ADC_CHANNELS];
char* (*buttonActionFunctions[PresetNil])(VocodecButton, ButtonAction);

VocodecPreset currentPreset = 0;
VocodecPreset previousPreset = PresetNil;
uint8_t loadingPreset = 0;

void initModeNames(void)
{
	modeNames[VocoderInternalPoly] = "VOCODER IP";
	shortModeNames[VocoderInternalPoly] = "V1";
	modeNamesDetails[VocoderInternalPoly] = "INTERNAL POLY";
	knobParamNames[VocoderInternalPoly][0] = "VOLUME";
	knobParamNames[VocoderInternalPoly][1] = "";
	knobParamNames[VocoderInternalPoly][2] = "";
	knobParamNames[VocoderInternalPoly][3] = "";
	knobParamNames[VocoderInternalPoly][4] = "";
	knobParamNames[VocoderInternalPoly][5] = "";

	modeNames[VocoderInternalMono] = "VOCODER IM";
	shortModeNames[VocoderInternalMono] = "V2";
	modeNamesDetails[VocoderInternalMono] = "INTERNAL MONO";
	knobParamNames[VocoderInternalMono][0] = "VOLUME";
	knobParamNames[VocoderInternalMono][1] = "";
	knobParamNames[VocoderInternalMono][2] = "";
	knobParamNames[VocoderInternalMono][3] = "";
	knobParamNames[VocoderInternalMono][4] = "";
	knobParamNames[VocoderInternalMono][5] = "";

	modeNames[VocoderExternal] = "VOCODEC E";
	shortModeNames[VocoderExternal] = "VE";
	modeNamesDetails[VocoderExternal] = "EXTERNAL";
	knobParamNames[VocoderExternal][0] = "";
	knobParamNames[VocoderExternal][1] = "";
	knobParamNames[VocoderExternal][2] = "";
	knobParamNames[VocoderExternal][3] = "";
	knobParamNames[VocoderExternal][4] = "";
	knobParamNames[VocoderExternal][5] = "";

	modeNames[Pitchshift] = "PITCHSHIFT";
	shortModeNames[Pitchshift] = "PS";
	modeNamesDetails[Pitchshift] = "";
	knobParamNames[Pitchshift][0] = "PITCH";
	knobParamNames[Pitchshift][1] = "FINE PITCH";
	knobParamNames[Pitchshift][2] = "F AMT";
	knobParamNames[Pitchshift][3] = "FORMANT";
	knobParamNames[Pitchshift][4] = "";
	knobParamNames[Pitchshift][5] = "";

	modeNames[AutotuneMono] = "NEARTUNE";
	shortModeNames[AutotuneMono] = "NT";
	modeNamesDetails[AutotuneMono] = "";
	knobParamNames[AutotuneMono][0] = "AMOUNT";
	knobParamNames[AutotuneMono][1] = "SPEED";
	knobParamNames[AutotuneMono][2] = "";
	knobParamNames[AutotuneMono][3] = "";
	knobParamNames[AutotuneMono][4] = "";
	knobParamNames[AutotuneMono][5] = "";

	modeNames[AutotunePoly] = "AUTOTUNE";
	shortModeNames[AutotunePoly] = "AT";
	modeNamesDetails[AutotunePoly] = "";
	knobParamNames[AutotunePoly][0] = "";
	knobParamNames[AutotunePoly][1] = "";
	knobParamNames[AutotunePoly][2] = "";
	knobParamNames[AutotunePoly][3] = "";
	knobParamNames[AutotunePoly][4] = "";
	knobParamNames[AutotunePoly][5] = "";

	modeNames[SamplerButtonPress] = "SAMPLER BP";
	shortModeNames[SamplerButtonPress] = "SB";
	modeNamesDetails[SamplerButtonPress] = "PRESS BUTTON A";
	knobParamNames[SamplerButtonPress][0] = "START";
	knobParamNames[SamplerButtonPress][1] = "END";
	knobParamNames[SamplerButtonPress][2] = "SPEED";
	knobParamNames[SamplerButtonPress][3] = "CROSSFADE";
	knobParamNames[SamplerButtonPress][4] = "";
	knobParamNames[SamplerButtonPress][5] = "";

	modeNames[SamplerAutoGrabInternal] = "AUTOSAMP1";
	shortModeNames[SamplerAutoGrabInternal] = "A1";
	modeNamesDetails[SamplerAutoGrabInternal] = "CH1 TRIG";
	knobParamNames[SamplerAutoGrabInternal][0] = "THRESHOLD";
	knobParamNames[SamplerAutoGrabInternal][1] = "WINDOW";
	knobParamNames[SamplerAutoGrabInternal][2] = "REL THRESH";
	knobParamNames[SamplerAutoGrabInternal][3] = "CROSSFADE";
	knobParamNames[SamplerAutoGrabInternal][4] = "";
	knobParamNames[SamplerAutoGrabInternal][5] = "";

	modeNames[SamplerAutoGrabExternal] = "AUTOSAMP2";
	shortModeNames[SamplerAutoGrabExternal] = "A2";
	modeNamesDetails[SamplerAutoGrabExternal] = "CH2 TRIG";
	knobParamNames[SamplerAutoGrabExternal][0] = "";
	knobParamNames[SamplerAutoGrabExternal][1] = "";
	knobParamNames[SamplerAutoGrabExternal][2] = "";
	knobParamNames[SamplerAutoGrabExternal][3] = "";
	knobParamNames[SamplerAutoGrabExternal][4] = "";
	knobParamNames[SamplerAutoGrabExternal][5] = "";

	modeNames[DistortionTanH] = "DISTORT1";
	shortModeNames[DistortionTanH] = "D1";
	modeNamesDetails[DistortionTanH] = "TANH FUNCTION";
	knobParamNames[DistortionTanH][0] = "GAIN";
	knobParamNames[DistortionTanH][1] = "";
	knobParamNames[DistortionTanH][2] = "";
	knobParamNames[DistortionTanH][3] = "";
	knobParamNames[DistortionTanH][4] = "";
	knobParamNames[DistortionTanH][5] = "";

	modeNames[DistortionShaper] = "DISTORT2";
	shortModeNames[DistortionShaper] = "D2";
	modeNamesDetails[DistortionShaper] = "WAVESHAPER";
	knobParamNames[DistortionShaper][0] = "GAIN";
	knobParamNames[DistortionShaper][1] = "";
	knobParamNames[DistortionShaper][2] = "";
	knobParamNames[DistortionShaper][3] = "";
	knobParamNames[DistortionShaper][4] = "";
	knobParamNames[DistortionShaper][5] = "";

	modeNames[Wavefolder] = "WAVEFOLD";
	shortModeNames[Wavefolder] = "WF";
	modeNamesDetails[Wavefolder] = "SERGE STYLE";
	knobParamNames[Wavefolder][0] = "GAIN";
	knobParamNames[Wavefolder][1] = "OFFSET1";
	knobParamNames[Wavefolder][2] = "OFFSET2";
	knobParamNames[Wavefolder][3] = "OFFSET3";
	knobParamNames[Wavefolder][4] = "";
	knobParamNames[Wavefolder][5] = "";

	modeNames[BitCrusher] = "BITCRUSH";
	shortModeNames[BitCrusher] = "BC";
	modeNamesDetails[BitCrusher] = "AHH HALP ME";
	knobParamNames[BitCrusher][0] = "QUALITY";
	knobParamNames[BitCrusher][1] = "SAMP RATIO";
	knobParamNames[BitCrusher][2] = "ROUNDING";
	knobParamNames[BitCrusher][3] = "OPERATION";
	knobParamNames[BitCrusher][4] = "GAIN";
	knobParamNames[BitCrusher][5] = "";

	modeNames[Delay] = "DELAY";
	shortModeNames[Delay] = "DL";
	modeNamesDetails[Delay] = "";
	knobParamNames[Delay][0] = "DELAY_L";
	knobParamNames[Delay][1] = "DELAY_R";
	knobParamNames[Delay][2] = "Feedback";
	knobParamNames[Delay][3] = "LowPass";
	knobParamNames[Delay][4] = "HighPass";
	knobParamNames[Delay][5] = "";

	modeNames[Reverb] = "REVERB";
	shortModeNames[Reverb] = "RV";
	modeNamesDetails[Reverb] = "DATTORRO ALG";
	knobParamNames[Reverb][0] = "SIZE";
	knobParamNames[Reverb][1] = "IN LOPASS";
	knobParamNames[Reverb][2] = "IN HIPASS";
	knobParamNames[Reverb][3] = "FB LOPASS";
	knobParamNames[Reverb][4] = "FB GAIN";
	knobParamNames[Reverb][5] = "";

	modeNames[Reverb2] = "REVERB2";
	shortModeNames[Reverb2] = "RV";
	modeNamesDetails[Reverb2] = "DATTORRO ALG";
	knobParamNames[Reverb2][0] = "SIZE";
	knobParamNames[Reverb2][1] = "LOWPASS";
	knobParamNames[Reverb2][2] = "HIGHPASS";
	knobParamNames[Reverb2][3] = "PEAK_FREQ";
	knobParamNames[Reverb2][4] = "PEAK_GAIN";
	knobParamNames[Reverb2][5] = "";

	modeNames[LivingString] = "STRING";
	shortModeNames[LivingString] = "LS";
	modeNamesDetails[LivingString] = "LIVING STRING";
	knobParamNames[LivingString][0] = "FREQ";
	knobParamNames[LivingString][1] = "DETUNE";
	knobParamNames[LivingString][2] = "DECAY";
	knobParamNames[LivingString][3] = "DAMPING";
	knobParamNames[LivingString][4] = "PICK_POS";
	knobParamNames[LivingString][5] = "";
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
			if (buttonHysteresis[i] < buttonHysteresisThreshold)
			{
				if (buttonCounters[i] < buttonHoldMax) buttonCounters[i]++;
				if ((buttonCounters[i] >= buttonHoldThreshold) && (cleanButtonValues[i] == 1))
				{
					buttonActionsSFX[i][ActionHold] = TRUE;
					buttonActionsUI[i][ActionHold] = TRUE;
					writeButtonFlag = i;
					writeActionFlag = ActionHold;
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
			buttonActionsUI[ButtonLeft][ActionPress] = 0;
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
			buttonActionsUI[ButtonRight][ActionPress] = 0;
		}
		if (buttonActionsUI[ButtonC][ActionPress] == 1)
		{
			//GFXsetFont(&theGFX, &DINCondensedBold9pt7b);
			keyCenter = (keyCenter + 1) % 12;
			OLEDclearLine(SecondLine);
			OLEDwriteString("KEY: ", 5, 0, SecondLine);
			OLEDwritePitchClass(keyCenter+60, 64, SecondLine);
			buttonActionsUI[ButtonC][ActionPress] = 0;
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

	// Can do this
	if (action == ActionPress)
	{
		if (button == ButtonA)
		{
			writeString = "AUTOCHROM ON";
			buttonActionsUI[ButtonA][ActionPress] = 0;
		}
		else if (button == ButtonB)
		{
			writeString = "AUTOCHROM OFF";
			buttonActionsUI[ButtonB][ActionPress] = 0;
		}
	}
	// or this
//	if (buttonActionsUI[ButtonA][ActionPress])
//	{
//		writeString = "AUTOCHROM ON";
//		buttonActionsUI[ButtonA][ActionPress] = 0;
//	}
//	if (buttonActionsUI[ButtonB][ActionPress])
//	{
//		writeString = "AUTOCHROM OFF";
//		buttonActionsUI[ButtonB][ActionPress] = 0;
//	}
	// first is probably better because this and sfx are not run at the same interval
	// ex. if a and b are pressed in the same UI block but separate audio blocks, and
	// a is pushed second, the display and audio state with not match
	// can't think of a way around that
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
	return writeString;
}

char* UISamplerAuto1Buttons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}


char* UISamplerAuto2Buttons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
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
	return writeString;
}

char* UILivingStringButtons(VocodecButton button, ButtonAction action)
{
	char* writeString = "";
	return writeString;
}
