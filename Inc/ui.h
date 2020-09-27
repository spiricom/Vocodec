/*
 * ui.h
 *
 *  Created on: Aug 30, 2019
 *      Author: jeffsnyder
 */
#ifndef UI_H_
#define UI_H_

#ifndef __cplusplus
#include "leaf.h"
#endif

#ifdef __cplusplus
#include <stdint.h>
namespace vocodec
{
    extern "C"
    {
#endif

#define NUM_ADC_CHANNELS 6
#define NUM_BUTTONS 10
#define NUM_PRESET_KNOB_VALUES 25
#define KNOB_PAGE_SIZE 5

        //PresetNil is used as a counter for the size of the enum
        typedef enum _VocodecPresetType
        {
            Vocoder = 0,
            VocoderCh,
            Pitchshift,
            AutotuneMono,
            AutotunePoly,
            SamplerButtonPress,
            SamplerKeyboard,
            SamplerAutoGrab,
            Distortion,
            Wavefolder,
            BitCrusher,
            Delay,
            Reverb,
            Reverb2,
            LivingString,
            LivingStringSynth,
            ClassicSynth,
            Rhodes,
            PresetNil
        } VocodecPresetType;

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
            ExtraMessage,
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

        extern tExpSmooth adc[6];
        extern uint16_t ADC_values[NUM_ADC_CHANNELS];

        extern float smoothedADC[6];

        extern uint8_t buttonValues[NUM_BUTTONS];
        //extern uint8_t buttonPressed[NUM_BUTTONS];
        //extern uint8_t buttonReleased[NUM_BUTTONS];

        extern int8_t writeKnobFlag;
        extern int8_t writeButtonFlag;
        extern int8_t writeActionFlag;

        extern float floatADCUI[NUM_ADC_CHANNELS];

        extern VocodecPresetType currentPreset;
        extern VocodecPresetType previousPreset;
        extern uint8_t loadingPreset;
        // Display values
        extern const char* modeNames[PresetNil];
        extern const char* modeNamesDetails[PresetNil];
        extern const char* shortModeNames[PresetNil];
        extern const char* knobParamNames[PresetNil][NUM_PRESET_KNOB_VALUES];
        extern float displayValues[NUM_PRESET_KNOB_VALUES];
        extern int8_t cvAddParam[PresetNil];
        extern uint8_t knobPage;
        extern uint8_t numPages[PresetNil];
        extern uint8_t buttonActionsUI[NUM_BUTTONS+1][ActionNil];
        extern uint8_t buttonActionsSFX[NUM_BUTTONS+1][ActionNil];
        extern const char* (*buttonActionFunctions[PresetNil])(VocodecButton, ButtonAction);

        void initModeNames(void);

        void buttonCheck(void);

        void adcCheck(void);

        void clearButtonActions(void);

        void changeTuning(void);

        void writeCurrentPresetToFlash(void);

        void incrementPage(void);

        void decrementPage(void);

        void resetKnobValues(void);

        void setKnobValues(float* values);

        void deactivateKnob(int knob);
        void deactivateAllKnobs(void);

        const char* UIVocoderButtons(VocodecButton button, ButtonAction action);
        const char* UIVocoderChButtons(VocodecButton button, ButtonAction action);
        const char* UIPitchShiftButtons(VocodecButton button, ButtonAction action);
        const char* UINeartuneButtons(VocodecButton button, ButtonAction action);
        const char* UIAutotuneButtons(VocodecButton button, ButtonAction action);
        const char* UISamplerBPButtons(VocodecButton button, ButtonAction action);
        const char* UISamplerKButtons(VocodecButton button, ButtonAction action);
        const char* UISamplerAutoButtons(VocodecButton button, ButtonAction action);
        const char* UIDistortionButtons(VocodecButton button, ButtonAction action);
        const char* UIWaveFolderButtons(VocodecButton button, ButtonAction action);
        const char* UIBitcrusherButtons(VocodecButton button, ButtonAction action);
        const char* UIDelayButtons(VocodecButton button, ButtonAction action);
        const char* UIReverbButtons(VocodecButton button, ButtonAction action);
        const char* UIReverb2Buttons(VocodecButton button, ButtonAction action);
        const char* UILivingStringButtons(VocodecButton button, ButtonAction action);
        const char* UILivingStringSynthButtons(VocodecButton button, ButtonAction action);
        const char* UIClassicSynthButtons(VocodecButton button, ButtonAction action);
        const char* UIRhodesButtons(VocodecButton button, ButtonAction action);

#ifdef __cplusplus
    }
} // extern "C"
#endif

#endif /* UI_H_ */
