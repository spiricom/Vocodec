/*
 * ui.h
 *
 *  Created on: Aug 30, 2019
 *      Author: jeffsnyder
 */
#ifndef UI_H_
#define UI_H_

#define NUM_ADC_CHANNELS 6
#define NUM_BUTTONS 10

//PresetNil is used as a counter for the size of the enum
typedef enum _VocodecPreset
{
	VocoderInternalPoly = 0,
	VocoderInternalMono,
	VocoderExternal,
	Pitchshift,
	AutotuneMono,
	AutotunePoly,
	SamplerButtonPress,
	SamplerAutoGrabInternal,
	SamplerAutoGrabExternal,
	DistortionTanH,
	DistortionShaper,
	Wavefolder,
	BitCrusher,
	Delay,
	Reverb,
	Reverb2,
	LivingString,
	PresetNil
} VocodecPreset;

typedef enum _VocodecButton
{
	ButtonEdit = 0,
	ButtonLeft,
	ButtonRight,
	ButtonDown,
	ButtonUp,
	ButtonA,
	ButtonB,
	ButtonC,
	ButtonD,
	ButtonE,
	ButtonNil
} VocodecButton;

typedef enum _ButtonAction
{
	ActionPress = 0,
	ActionRelease,
	ActionHoldInstant,
	ActionHoldContinuous,
	ActionNil
} ButtonAction;

extern uint16_t ADC_values[NUM_ADC_CHANNELS];

extern uint8_t buttonValues[NUM_BUTTONS];
//extern uint8_t buttonPressed[NUM_BUTTONS];
//extern uint8_t buttonReleased[NUM_BUTTONS];

extern int8_t writeKnobFlag;
extern int8_t writeButtonFlag;
extern int8_t writeActionFlag;

extern uint8_t currentPreset;
extern uint8_t previousPreset;
extern uint8_t loadingPreset;
// Display values
extern char* modeNames[PresetNil];
extern char* modeNamesDetails[PresetNil];
extern char* shortModeNames[PresetNil];
extern char* knobParamNames[PresetNil][NUM_ADC_CHANNELS];
extern float knobParams[NUM_ADC_CHANNELS];
extern uint8_t buttonActionsSFX[NUM_BUTTONS][ActionNil];
extern char* (*buttonActionFunctions[PresetNil])(VocodecButton, ButtonAction);

void initModeNames(void);

void buttonCheck(void);

void adcCheck(void);

void clearButtonActions(void);

void changeTuning(void);

void writeCurrentPresetToFlash(void);

char* UIVocoderIPButtons(VocodecButton button, ButtonAction action);
char* UIVocoderIMButtons(VocodecButton button, ButtonAction action);
char* UIVocoderEButtons(VocodecButton button, ButtonAction action);
char* UIPitchShiftButtons(VocodecButton button, ButtonAction action);
char* UINeartuneButtons(VocodecButton button, ButtonAction action);
char* UIAutotuneButtons(VocodecButton button, ButtonAction action);
char* UISamplerBPButtons(VocodecButton button, ButtonAction action);
char* UISamplerAuto1Buttons(VocodecButton button, ButtonAction action);
char* UISamplerAuto2Buttons(VocodecButton button, ButtonAction action);
char* UIDistortionTanhButtons(VocodecButton button, ButtonAction action);
char* UIDistortionShaperButtons(VocodecButton button, ButtonAction action);
char* UIWaveFolderButtons(VocodecButton button, ButtonAction action);
char* UIBitcrusherButtons(VocodecButton button, ButtonAction action);
char* UIDelayButtons(VocodecButton button, ButtonAction action);
char* UIReverbButtons(VocodecButton button, ButtonAction action);
char* UIReverb2Buttons(VocodecButton button, ButtonAction action);
char* UILivingStringButtons(VocodecButton button, ButtonAction action);

#endif /* UI_H_ */

