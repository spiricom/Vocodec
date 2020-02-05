/*
 * ui.h
 *
 *  Created on: Aug 30, 2019
 *      Author: jeffsnyder
 */
#ifndef UI_H_
#define UI_H_

#include "sfx.h"

#define NUM_ADC_CHANNELS 6
#define NUM_BUTTONS 10

extern uint16_t ADC_values[NUM_ADC_CHANNELS];

extern uint8_t buttonValues[NUM_BUTTONS];
extern uint8_t buttonPressed[NUM_BUTTONS];
extern uint8_t buttonReleased[NUM_BUTTONS];

extern uint8_t currentPreset;
extern uint8_t previousPreset;
extern uint8_t loadingPreset;
// Display values
extern char* modeNames[PresetNil];
extern char* modeNamesDetails[PresetNil];
extern char* shortModeNames[PresetNil];
extern char* knobParamNames[PresetNil][NUM_ADC_CHANNELS];
extern float knobParams[NUM_ADC_CHANNELS];
extern uint8_t buttonParams[2];
extern char* (*buttonParamFunctions[PresetNil])(uint8_t);

void initModeNames(void);

void initUIFunctionPointers(void);

void writeCurrentPresetToFlash(void);

char* UIVocoderIPButtons(uint8_t whichParam);
char* UIVocoderIMButtons(uint8_t whichParam);
char* UIVocoderEButtons(uint8_t whichParam);
char* UIPitchShiftButtons(uint8_t whichParam);
char* UINeartuneButtons(uint8_t whichParam);
char* UIAutotuneButtons(uint8_t whichParam);
char* UISamplerBPButtons(uint8_t whichParam);
char* UISamplerAuto1Buttons(uint8_t whichParam);
char* UISamplerAuto2Buttons(uint8_t whichParam);
char* UIDistortionTanhButtons(uint8_t whichParam);
char* UIDistortionShaperButtons(uint8_t whichParam);
char* UIWaveFolderButtons(uint8_t whichParam);
char* UIBitcrusherButtons(uint8_t whichParam);
char* UIDelayButtons(uint8_t whichParam);
char* UIReverbButtons(uint8_t whichParam);
char* UIReverb2Buttons(uint8_t whichParam);
char* UILivingStringButtons(uint8_t whichParam);

#endif /* UI_H_ */

