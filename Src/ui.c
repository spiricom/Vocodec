/*
 * ui.c
 *
 *  Created on: Feb 05, 2018
 *      Author: jeffsnyder
 */
#include "main.h"
#include "ui.h"
#include "ssd1306.h"
#include "tunings.h"
#include "eeprom.h"

uint16_t ADC_values[NUM_ADC_CHANNELS] __ATTR_RAM_D2;

#define NUM_CHARACTERS_PER_PRESET_NAME 16
char* modeNames[PresetNil];
char* modeNamesDetails[PresetNil];
char* shortModeNames[PresetNil];
char* knobParamNames[PresetNil][NUM_ADC_CHANNELS];

float knobParams[NUM_ADC_CHANNELS];
uint8_t buttonParams[2];
char* (*buttonParamFunctions[PresetNil])(uint8_t);

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

void initUIFunctionPointers(void)
{
	buttonParamFunctions[VocoderInternalPoly] = UIVocoderIPButtons;
	buttonParamFunctions[VocoderInternalMono] = UIVocoderIMButtons;
	buttonParamFunctions[VocoderExternal] = UIVocoderEButtons;
	buttonParamFunctions[Pitchshift] = UIPitchShiftButtons;
	buttonParamFunctions[AutotuneMono] = UINeartuneButtons;
	buttonParamFunctions[AutotunePoly] = UIAutotuneButtons;
	buttonParamFunctions[SamplerButtonPress] = UISamplerBPButtons;
	buttonParamFunctions[SamplerAutoGrabInternal] = UISamplerAuto1Buttons;
	buttonParamFunctions[SamplerAutoGrabExternal] = UISamplerAuto2Buttons;
	buttonParamFunctions[DistortionTanH] = UIDistortionTanhButtons;
	buttonParamFunctions[DistortionShaper] = UIDistortionShaperButtons;
	buttonParamFunctions[Wavefolder] = UIWaveFolderButtons;
	buttonParamFunctions[BitCrusher] = UIBitcrusherButtons;
	buttonParamFunctions[Delay] = UIDelayButtons;
	buttonParamFunctions[Reverb] = UIReverbButtons;
	buttonParamFunctions[Reverb2] = UIReverb2Buttons;
	buttonParamFunctions[LivingString] = UILivingStringButtons;
}

void writeCurrentPresetToFlash(void)
{
	if((EE_WriteVariable(VirtAddVarTab[0],  currentPreset)) != HAL_OK)
	{
		Error_Handler();
	}
}


//buttonValues[0] //edit
//buttonValues[1] //left
//buttonValues[2] //right
//buttonValues[3] //down
//buttonValues[4] //up
//buttonValues[5] // A
//buttonValues[6] // B
//buttonValues[7] // C
//buttonValues[8] // D
//buttonValues[9] // E
char* UIVocoderIPButtons(uint8_t whichParam)
{
	// whichParam is the button that changed
	// use it to access buttonsParams which can contain various button states
	// can also define behavior that depends on the state of other buttons
	return " ";
}

char* UIVocoderIMButtons(uint8_t whichParam)
{
	return " ";
}

char* UIVocoderEButtons(uint8_t whichParam)
{
	return " ";
}

char* UIPitchShiftButtons(uint8_t whichParam)
{
	return " ";
}

char* UINeartuneButtons(uint8_t whichParam)
{
	return " ";
}

char* UIAutotuneButtons(uint8_t whichParam)
{
	return " ";
}

char* UISamplerBPButtons(uint8_t whichParam)
{
	return " ";
}

char* UISamplerAuto1Buttons(uint8_t whichParam)
{
	return " ";
}


char* UISamplerAuto2Buttons(uint8_t whichParam)
{
	return " ";
}

char* UIDistortionTanhButtons(uint8_t whichParam)
{
	return " ";
}

char* UIDistortionShaperButtons(uint8_t whichParam)
{
	return " ";
}

char* UIWaveFolderButtons(uint8_t whichParam)
{
	return " ";
}

char* UIBitcrusherButtons(uint8_t whichParam)
{
	return " ";
}

char* UIDelayButtons(uint8_t whichParam)
{
	return " ";
}

char* UIReverbButtons(uint8_t whichParam)
{
	return " ";
}

char* UIReverb2Buttons(uint8_t whichParam)
{
	return " ";
}

char* UILivingStringButtons(uint8_t whichParam)
{
	return " ";
}
