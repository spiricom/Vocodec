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

#include "sfx.h"

#ifdef __cplusplus
#include <stdint.h>
namespace vocodec
{
    extern "C"
    {
#else
    extern uint16_t ADC_values[NUM_ADC_CHANNELS] __ATTR_RAM_D2;
#endif

        void initModeNames(Vocodec* vcd);

        void buttonCheck(Vocodec* vcd);

        void adcCheck(Vocodec* vcd);

        void clearButtonActions(Vocodec* vcd);

        void changeTuning(Vocodec* vcd);

        void writeCurrentPresetToFlash(Vocodec* vcd);

        void incrementPage(Vocodec* vcd);

        void decrementPage(Vocodec* vcd);

        void checkPage(Vocodec* vcd);

        void resetKnobValues(Vocodec* vcd);

        void setKnobValues(Vocodec* vcd, float* values);

        void deactivateKnob(Vocodec* vcd, int knob);
        void deactivateAllKnobs(Vocodec* vcd);

        const char* UIVocoderButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIVocoderChButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIPitchShiftButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UINeartuneButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIAutotuneButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UISamplerBPButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UISamplerKButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UISamplerAutoButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIDistortionButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIWaveFolderButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIBitcrusherButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIDelayButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIReverbButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIReverb2Buttons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UILivingStringButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UILivingStringSynthButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIClassicSynthButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UIRhodesButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);
        const char* UITapeButtons(Vocodec* vcd, VocodecButton button, ButtonAction action);

#ifdef __cplusplus
    }
} // extern "C"
#endif

#endif /* UI_H_ */
