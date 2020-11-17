/*
 * ui.c
 *
 *  Created on: Feb 05, 2018
 *      Author: jeffsnyder
 */

#ifndef __cplusplus
#include "main.h"
#include "audiostream.h"
#include "eeprom.h"
#endif

#include "oled.h"
#include "sfx.h"
#include "ui.h"
#include "tunings.h"

#ifdef __cplusplus
namespace vocodec
{
    extern "C"
    {
#else
    uint16_t ADC_values[NUM_ADC_CHANNELS] __ATTR_RAM_D2;
#endif

        void initModeNames(Vocodec* vcd)
        {
            for (int i = 0; i < NUM_ADC_CHANNELS; i++)
            {
                vcd->floatADCUI[i] = -1.0f;
                vcd->orderedParams[i] = i;
            }
            vcd->orderedParams[6] = ButtonA;
            vcd->orderedParams[7] = ButtonB;

            for (int i = 0; i < PresetNil; i++)
            {
                vcd->cvAddParam[i] = -1;
            }

            vcd->modeNames[Vocoder] = "VOCODER1";
            vcd->shortModeNames[Vocoder] = "VL";
            vcd->modeNamesDetails[Vocoder] = "LPC";
            vcd->numPages[Vocoder] = 2;
            vcd->knobParamNames[Vocoder][0] = "VOLUME";
            vcd->knobParamNames[Vocoder][1] = "WARP";
            vcd->knobParamNames[Vocoder][2] = "QUALITY";
            vcd->knobParamNames[Vocoder][3] = "SAWtoPULSE";
            vcd->knobParamNames[Vocoder][4] = "NOISTHRESH";
            vcd->knobParamNames[Vocoder][5] = "BREATH";
            vcd->knobParamNames[Vocoder][6] = "TILT";
            vcd->knobParamNames[Vocoder][7] = "PULSEWIDTH";
            vcd->knobParamNames[Vocoder][8] = "PULSESHAPE";
            vcd->knobParamNames[Vocoder][9] = "";


            vcd->modeNames[VocoderCh] = "VOCODER2";
            vcd->shortModeNames[VocoderCh] = "VC";
            vcd->modeNamesDetails[VocoderCh] = "CHANNEL";
            vcd->numPages[VocoderCh] = 3;
            vcd->knobParamNames[VocoderCh][0] = "VOLUME";
            vcd->knobParamNames[VocoderCh][1] = "WARP";
            vcd->knobParamNames[VocoderCh][2] = "QUALITY";
            vcd->knobParamNames[VocoderCh][3] = "BANDWIDTH";
            vcd->knobParamNames[VocoderCh][4] = "NOISTHRESH";
            vcd->knobParamNames[VocoderCh][5] = "SAWtoPULSE";
            vcd->knobParamNames[VocoderCh][6] = "PULSEWIDTH";
            vcd->knobParamNames[VocoderCh][7] = "PULSESHAPE";
            vcd->knobParamNames[VocoderCh][8] = "BREATH";
            vcd->knobParamNames[VocoderCh][9] = "SPEED";
            vcd->knobParamNames[VocoderCh][10] = "BANDSQUISH";
            vcd->knobParamNames[VocoderCh][11] = "BANDOFF";
            vcd->knobParamNames[VocoderCh][12] = "TILT";
            vcd->knobParamNames[VocoderCh][13] = "STEREO";
            vcd->knobParamNames[VocoderCh][14] = "BARKPULL";

            vcd->modeNames[Pitchshift] = "PITCHSHIFT";
            vcd->shortModeNames[Pitchshift] = "PS";
            vcd->modeNamesDetails[Pitchshift] = "";
            vcd->numPages[Pitchshift] = 2;
            vcd->knobParamNames[Pitchshift][0] = "SHIFT";
            vcd->knobParamNames[Pitchshift][1] = "FINE";
            vcd->knobParamNames[Pitchshift][2] = "F AMT";
            vcd->knobParamNames[Pitchshift][3] = "FORMANT";
            vcd->knobParamNames[Pitchshift][4] = "RANGE";
            vcd->knobParamNames[Pitchshift][5] = "OFFSET";
            vcd->knobParamNames[Pitchshift][6] = "";
            vcd->knobParamNames[Pitchshift][7] = "";
            vcd->knobParamNames[Pitchshift][8] = "";
            vcd->knobParamNames[Pitchshift][9] = "";

            vcd->modeNames[AutotuneMono] = "AUTOTUNE";
            vcd->shortModeNames[AutotuneMono] = "NT";
            vcd->modeNamesDetails[AutotuneMono] = "";
            vcd->numPages[AutotuneMono] = 1;
            vcd->knobParamNames[AutotuneMono][0] = "PICKINESS";
            vcd->knobParamNames[AutotuneMono][1] = "AMOUNT";
            vcd->knobParamNames[AutotuneMono][2] = "SPEED";
            vcd->knobParamNames[AutotuneMono][3] = "LEAPALLOW";
            vcd->knobParamNames[AutotuneMono][4] = "HYSTERESIS";


            vcd->modeNames[AutotunePoly] = "HARMONIZE";
            vcd->shortModeNames[AutotunePoly] = "AT";
            vcd->modeNamesDetails[AutotunePoly] = "";
            vcd->numPages[AutotunePoly] = 1;
            vcd->knobParamNames[AutotunePoly][0] = "PICKINESS";
            vcd->knobParamNames[AutotunePoly][1] = "";
            vcd->knobParamNames[AutotunePoly][2] = "";
            vcd->knobParamNames[AutotunePoly][3] = "";
            vcd->knobParamNames[AutotunePoly][4] = "";


            vcd->modeNames[SamplerButtonPress] = "SAMPLER BP";
            vcd->shortModeNames[SamplerButtonPress] = "SB";
            vcd->modeNamesDetails[SamplerButtonPress] = "PRESS BUTTON A";
            vcd->numPages[SamplerButtonPress] = 1;
            vcd->knobParamNames[SamplerButtonPress][0] = "START";
            vcd->knobParamNames[SamplerButtonPress][1] = "LENGTH";
            vcd->knobParamNames[SamplerButtonPress][2] = "SPEED";
            vcd->knobParamNames[SamplerButtonPress][3] = "SPEEDMULT";
            vcd->knobParamNames[SamplerButtonPress][4] = "CROSSFADE";


            vcd->modeNames[SamplerKeyboard] = "KEYSAMPLER";
            vcd->shortModeNames[SamplerKeyboard] = "KS";
            vcd->modeNamesDetails[SamplerKeyboard] = "KEY TO REC";
            vcd->numPages[SamplerKeyboard] = 2;
            vcd->knobParamNames[SamplerKeyboard][0] = "START";
            vcd->knobParamNames[SamplerKeyboard][1] = "LENGTH";
            vcd->knobParamNames[SamplerKeyboard][2] = "SPEED";
            vcd->knobParamNames[SamplerKeyboard][3] = "SPEEDMULT";
            vcd->knobParamNames[SamplerKeyboard][4] = "LOOP ON";
            vcd->knobParamNames[SamplerKeyboard][5] = "CROSSFADE";
            vcd->knobParamNames[SamplerKeyboard][6] = "VELO SENS";
            vcd->knobParamNames[SamplerKeyboard][7] = "";
            vcd->knobParamNames[SamplerKeyboard][8] = "";
            vcd->knobParamNames[SamplerKeyboard][9] = "";


            vcd->modeNames[SamplerAutoGrab] = "AUTOSAMP";
            vcd->shortModeNames[SamplerAutoGrab] = "AS";
            vcd->modeNamesDetails[SamplerAutoGrab] = "AUDIO TRIG'D";
            vcd->numPages[SamplerAutoGrab] = 2;
            vcd->knobParamNames[SamplerAutoGrab][0] = "THRESHOLD";
            vcd->knobParamNames[SamplerAutoGrab][1] = "WINDOW";
            vcd->knobParamNames[SamplerAutoGrab][2] = "SPEED";
            vcd->knobParamNames[SamplerAutoGrab][3] = "CROSSFADE";
            vcd->knobParamNames[SamplerAutoGrab][4] = "";
            vcd->knobParamNames[SamplerAutoGrab][5] = "LEN RAND";
            vcd->knobParamNames[SamplerAutoGrab][6] = "SPD RAND";
            vcd->knobParamNames[SamplerAutoGrab][7] = "";
            vcd->knobParamNames[SamplerAutoGrab][8] = "";
            vcd->knobParamNames[SamplerAutoGrab][9] = "";

            vcd->modeNames[Distortion] = "DISTORT";
            vcd->shortModeNames[Distortion] = "DT";
            vcd->modeNamesDetails[Distortion] = "WITH EQ";
            vcd->numPages[Distortion] = 1;
            vcd->knobParamNames[Distortion][0] = "PRE GAIN";
            vcd->knobParamNames[Distortion][1] = "TILT";
            vcd->knobParamNames[Distortion][2] = "MID GAIN";
            vcd->knobParamNames[Distortion][3] = "MID FREQ";
            vcd->knobParamNames[Distortion][4] = "POST GAIN";

            vcd->modeNames[Wavefolder] = "WAVEFOLD";
            vcd->shortModeNames[Wavefolder] = "WF";
            vcd->modeNamesDetails[Wavefolder] = "SERGE STYLE";
            vcd->numPages[Wavefolder] = 1;
            vcd->knobParamNames[Wavefolder][0] = "GAIN";
            vcd->knobParamNames[Wavefolder][1] = "OFFSET1";
            vcd->knobParamNames[Wavefolder][2] = "OFFSET2";
            vcd->knobParamNames[Wavefolder][3] = "POST GAIN";
            vcd->knobParamNames[Wavefolder][4] = "";

            vcd->modeNames[BitCrusher] = "BITCRUSH";
            vcd->shortModeNames[BitCrusher] = "BC";
            vcd->modeNamesDetails[BitCrusher] = "AHH HALP ME";
            vcd->numPages[BitCrusher] = 2;
            vcd->knobParamNames[BitCrusher][0] = "QUALITY";
            vcd->knobParamNames[BitCrusher][1] = "SAMP RATIO";
            vcd->knobParamNames[BitCrusher][2] = "ROUNDING";
            vcd->knobParamNames[BitCrusher][3] = "OPERATION";
            vcd->knobParamNames[BitCrusher][4] = "POST GAIN";
            vcd->knobParamNames[BitCrusher][5] = "PRE GAIN";
            vcd->knobParamNames[BitCrusher][6] = "";
            vcd->knobParamNames[BitCrusher][7] = "";
            vcd->knobParamNames[BitCrusher][8] = "";
            vcd->knobParamNames[BitCrusher][9] = "";

            vcd->modeNames[Delay] = "DELAY";
            vcd->shortModeNames[Delay] = "DL";
            vcd->modeNamesDetails[Delay] = "STEREO";
            vcd->numPages[Delay] = 2;
            vcd->knobParamNames[Delay][0] = "DELAY_L";
            vcd->knobParamNames[Delay][1] = "DELAY_R";
            vcd->knobParamNames[Delay][2] = "HIGHPASS";
            vcd->knobParamNames[Delay][3] = "LOWPASS";
            vcd->knobParamNames[Delay][4] = "FEEDBACK";
            vcd->knobParamNames[Delay][5] = "POST GAIN";
            vcd->knobParamNames[Delay][6] = "";
            vcd->knobParamNames[Delay][7] = "";
            vcd->knobParamNames[Delay][8] = "";
            vcd->knobParamNames[Delay][9] = "";

            vcd->modeNames[Reverb] = "REVERB1";
            vcd->shortModeNames[Reverb] = "RV";
            vcd->modeNamesDetails[Reverb] = "DATTORRO ALG";
            vcd->numPages[Reverb] = 1;
            vcd->knobParamNames[Reverb][0] = "SIZE";
            vcd->knobParamNames[Reverb][1] = "FB LOPASS";
            vcd->knobParamNames[Reverb][2] = "IN HIPASS";
            vcd->knobParamNames[Reverb][3] = "IN LOPASS";
            vcd->knobParamNames[Reverb][4] = "FB GAIN";


            vcd->modeNames[Reverb2] = "REVERB2";
            vcd->shortModeNames[Reverb2] = "RV";
            vcd->modeNamesDetails[Reverb2] = "NREVERB ALG";
            vcd->numPages[Reverb2] = 1;
            vcd->knobParamNames[Reverb2][0] = "SIZE";
            vcd->knobParamNames[Reverb2][1] = "LOWPASS";
            vcd->knobParamNames[Reverb2][2] = "HIGHPASS";
            vcd->knobParamNames[Reverb2][3] = "PEAK_FREQ";
            vcd->knobParamNames[Reverb2][4] = "PEAK_GAIN";

            vcd->modeNames[LivingString] = "STRING1";
            vcd->shortModeNames[LivingString] = "LS";
            vcd->modeNamesDetails[LivingString] = "SYMP STRING";
            vcd->numPages[LivingString] = 3;
            vcd->knobParamNames[LivingString][0] = "FREQ1";
            vcd->knobParamNames[LivingString][1] = "DETUNE";
            vcd->knobParamNames[LivingString][2] = "DECAY";
            vcd->knobParamNames[LivingString][3] = "DAMPING";
            vcd->knobParamNames[LivingString][4] = "PICK POS";
            vcd->knobParamNames[LivingString][5] = "PREP POS";
            vcd->knobParamNames[LivingString][6] = "PREP FORCE";
            vcd->knobParamNames[LivingString][7] = "LET RING";
            vcd->knobParamNames[LivingString][8] = "";
            vcd->knobParamNames[LivingString][9] = "";
            vcd->knobParamNames[LivingString][10] = "FREQ2";
            vcd->knobParamNames[LivingString][11] = "FREQ3";
            vcd->knobParamNames[LivingString][12] = "FREQ4";
            vcd->knobParamNames[LivingString][13] = "FREQ5";
            vcd->knobParamNames[LivingString][14] = "FREQ6";

            vcd->modeNames[LivingStringSynth] = "STRING2";
            vcd->shortModeNames[LivingStringSynth] = "SS";
            vcd->modeNamesDetails[LivingStringSynth] = "STRING SYNTH";
            vcd->numPages[LivingStringSynth] = 2;
            vcd->knobParamNames[LivingStringSynth][0] = "PLUCK VOL";
            vcd->knobParamNames[LivingStringSynth][1] = "PLUCK TONE";
            vcd->knobParamNames[LivingStringSynth][2] = "DECAY";
            vcd->knobParamNames[LivingStringSynth][3] = "DAMPING";
            vcd->knobParamNames[LivingStringSynth][4] = "PICK_POS";
            vcd->knobParamNames[LivingStringSynth][5] = "PREP POS";
            vcd->knobParamNames[LivingStringSynth][6] = "PREP FORCE";
            vcd->knobParamNames[LivingStringSynth][7] = "LET RING";
            vcd->knobParamNames[LivingStringSynth][8] = "FB LEVEL";
            vcd->knobParamNames[LivingStringSynth][9] = "RELEASE";

            vcd->modeNames[ClassicSynth] = "POLYSYNTH";
            vcd->shortModeNames[ClassicSynth] = "CS";
            vcd->modeNamesDetails[ClassicSynth] = "VCO+VCF";
            vcd->numPages[ClassicSynth] = 4;
            vcd->knobParamNames[ClassicSynth][0] = "VOLUME";
            vcd->knobParamNames[ClassicSynth][1] = "LOWPASS";
            vcd->knobParamNames[ClassicSynth][2] = "KEYFOLLOW";
            vcd->knobParamNames[ClassicSynth][3] = "DETUNE";
            vcd->knobParamNames[ClassicSynth][4] = "FILTER Q";
            vcd->knobParamNames[ClassicSynth][5] = "ATTACK";
            vcd->knobParamNames[ClassicSynth][6] = "DECAY";
            vcd->knobParamNames[ClassicSynth][7] = "SUSTAIN";
            vcd->knobParamNames[ClassicSynth][8] = "RELEASE";
            vcd->knobParamNames[ClassicSynth][9] = "LEAK";
            vcd->knobParamNames[ClassicSynth][10] = "F_ATTACK";
            vcd->knobParamNames[ClassicSynth][11] = "F_DECAY";
            vcd->knobParamNames[ClassicSynth][12] = "F_SUSTAIN";
            vcd->knobParamNames[ClassicSynth][13] = "F_RELEASE";
            vcd->knobParamNames[ClassicSynth][14] = "F_LEAK";
            vcd->knobParamNames[ClassicSynth][15] = "F_AMOUNT";
            vcd->knobParamNames[ClassicSynth][16] = "SAW/PULSE";
            vcd->knobParamNames[ClassicSynth][17] = "";
            vcd->knobParamNames[ClassicSynth][18] = "";
            vcd->knobParamNames[ClassicSynth][19] = "";

            vcd->modeNames[Rhodes] = "RHODES";
            vcd->shortModeNames[Rhodes] = "RD";
            vcd->modeNamesDetails[Rhodes] = "DARK";
            vcd->numPages[Rhodes] = 5;
            vcd->knobParamNames[Rhodes][0] = "BRIGHTNESS";
            vcd->knobParamNames[Rhodes][1] = "TREM DEPTH";
            vcd->knobParamNames[Rhodes][2] = "TREM RATE";
            vcd->knobParamNames[Rhodes][3] = "DRIVE";
            vcd->knobParamNames[Rhodes][4] = "PAN SPREAD";
            vcd->knobParamNames[Rhodes][5] = "ATTACK";
            vcd->knobParamNames[Rhodes][6] = "DECAY";
            vcd->knobParamNames[Rhodes][7] = "SUSTAIN";
            vcd->knobParamNames[Rhodes][8] = "RELEASE";
            vcd->knobParamNames[Rhodes][9] = "LEAK";
            vcd->knobParamNames[Rhodes][10] = "INDEX1";
            vcd->knobParamNames[Rhodes][11] = "INDEX2";
            vcd->knobParamNames[Rhodes][12] = "INDEX3";
            vcd->knobParamNames[Rhodes][13] = "INDEX4";
            vcd->knobParamNames[Rhodes][14] = "INDEX5";
            vcd->knobParamNames[Rhodes][15] = "RATIO1";
            vcd->knobParamNames[Rhodes][16] = "RATIO2";
            vcd->knobParamNames[Rhodes][17] = "RATIO3";
            vcd->knobParamNames[Rhodes][18] = "RATIO4";
            vcd->knobParamNames[Rhodes][19] = "RATIO5";
            vcd->knobParamNames[Rhodes][20] = "RATIO6";
            vcd->knobParamNames[Rhodes][21] = "FEEDBACK";
            vcd->knobParamNames[Rhodes][22] = "TUNE SNAP";
            vcd->knobParamNames[Rhodes][23] = "RAND DECAY";
            vcd->knobParamNames[Rhodes][24] = "RAND SUST";
        }

        void buttonCheck(Vocodec* vcd)
        {
#ifndef __cplusplus
            if (codecReady)
#endif
            {
#ifndef __cplusplus
                /*
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
                 */

                //A little more efficient since it avoids a function call
                vcd->buttonValues[0] =!(GPIOB->IDR & GPIO_PIN_13);
                vcd->buttonValues[1] =!(GPIOB->IDR & GPIO_PIN_12);
                vcd->buttonValues[2] =!(GPIOB->IDR & GPIO_PIN_14);
                vcd->buttonValues[3] =!(GPIOD->IDR & GPIO_PIN_11);
                vcd->buttonValues[4] =!(GPIOB->IDR & GPIO_PIN_15);
                vcd->buttonValues[5] =!(GPIOB->IDR & GPIO_PIN_1);
                vcd->buttonValues[6] =!(GPIOD->IDR & GPIO_PIN_7);
                vcd->buttonValues[7] =!(GPIOB->IDR & GPIO_PIN_11);
                vcd->buttonValues[8] =!(GPIOG->IDR & GPIO_PIN_11);
                vcd->buttonValues[9] =!(GPIOB->IDR & GPIO_PIN_10);
#endif


                for (int i = 0; i < NUM_BUTTONS; i++)
                {
                    if (vcd->buttonValues[i] != vcd->buttonValuesPrev[i])
                    {
                        vcd->buttonHysteresis[i]++;
                    }
                    if (vcd->cleanButtonValues[i] == 1)
                    {
                        vcd->buttonActionsSFX[i][ActionHoldContinuous] = 1;
                        vcd->buttonActionsUI[i][ActionHoldContinuous] = 1;
                        vcd->writeButtonFlag = i;
                        vcd->writeActionFlag = ActionHoldContinuous;
                    }
                    if (vcd->buttonHysteresis[i] < vcd->buttonHysteresisThreshold)
                    {
                        if (vcd->buttonCounters[i] <vcd-> buttonHoldMax) vcd->buttonCounters[i]++;
                        if ((vcd->buttonCounters[i] >= vcd->buttonHoldThreshold) && (vcd->cleanButtonValues[i] == 1))
                        {
                            vcd->buttonActionsSFX[i][ActionHoldInstant] = 1;
                            vcd->buttonActionsUI[i][ActionHoldInstant] = 1;
                            vcd->writeButtonFlag = i;
                            vcd->writeActionFlag = ActionHoldInstant;
                        }
                    }
                    else
                    {
                        vcd->cleanButtonValues[i] = vcd->buttonValues[i];
                        vcd->buttonHysteresis[i] = 0;
                        vcd->buttonCounters[i] = 0;

                        if (vcd->cleanButtonValues[i] == 1)
                        {
                            vcd->buttonActionsSFX[i][ActionPress] = 1;
                            vcd->buttonActionsUI[i][ActionPress] = 1;
                            vcd->writeButtonFlag = i;
                            vcd->writeActionFlag = ActionPress;
                        }
                        else if (vcd->cleanButtonValues[i] == 0)
                        {
                            vcd->buttonActionsSFX[i][ActionRelease] = 1;
                            vcd->buttonActionsUI[i][ActionRelease] = 1;
                            vcd->writeButtonFlag = i;
                            vcd->writeActionFlag = ActionRelease;
                        }
                        vcd->buttonValuesPrev[i] = vcd->buttonValues[i];
                    }
                }

                // make some if statements if you want to find the "attack" of the buttons (getting the "press" action)

                /// DEFINE GLOBAL BUTTON BEHAVIOR HERE

                if (vcd->buttonActionsUI[ButtonLeft][ActionPress] == 1)
                {
                    if (vcd->currentPreset <= 0) vcd->currentPreset = (VocodecPresetType)((int)PresetNil - 1);
                    else vcd->currentPreset = (VocodecPresetType)((int)vcd->currentPreset - 1);
                    checkPage(vcd);
                    vcd->loadingPreset = 1;
                    OLED_writePreset(vcd);
                    writeCurrentPresetToFlash(vcd);
                    clearButtonActions(vcd);
                }
                if (vcd->buttonActionsUI[ButtonRight][ActionPress] == 1)
                {
                    if (vcd->currentPreset >= PresetNil - 1) vcd->currentPreset = (VocodecPresetType)0;
                    else vcd->currentPreset = (VocodecPresetType)((int)vcd->currentPreset + 1);
                    checkPage(vcd);
                    vcd->loadingPreset = 1;
                    OLED_writePreset(vcd);
                    writeCurrentPresetToFlash(vcd);
                    clearButtonActions(vcd);
                }
                if (vcd->buttonActionsUI[ButtonD][ActionPress] == 1)
                {
                    if (vcd->currentTuning == 0)
                    {
                        vcd->currentTuning = NUM_TUNINGS - 1;
                    }
                    else
                    {
                        vcd->currentTuning = (vcd->currentTuning - 1);
                    }
                    changeTuning(vcd);
                    OLED_writeTuning(vcd);
                    vcd->buttonActionsUI[ButtonD][ActionPress] = 0;
                }
                if (vcd->buttonActionsUI[ButtonE][ActionPress] == 1)
                {

                    vcd->currentTuning = (vcd->currentTuning + 1) % NUM_TUNINGS;
                    changeTuning(vcd);
                    OLED_writeTuning(vcd);
                    vcd->buttonActionsUI[ButtonE][ActionPress] = 0;
                }

                if (vcd->buttonActionsUI[ButtonEdit][ActionPress])
                {
                    OLED_writeEditScreen(vcd);
                    setLED_Edit(vcd, 1);
                    vcd->buttonActionsUI[ButtonEdit][ActionPress] = 0;
                }
                if (vcd->buttonActionsUI[ButtonEdit][ActionHoldContinuous] == 1)
                {
                    if (vcd->buttonActionsUI[ButtonC][ActionPress] == 1)
                    {
                        vcd->keyCenter = (vcd->keyCenter + 1) % 12;
                        OLEDclearLine(vcd, SecondLine);
                        OLEDwriteString(vcd, "KEY: ", 5, 0, SecondLine);
                        OLEDwritePitchClass(vcd, vcd->keyCenter+60, 64, SecondLine);
                        vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
                        vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                        vcd->buttonActionsUI[ButtonEdit][ActionHoldContinuous] = 0;
                    }
                    if (vcd->buttonActionsUI[ButtonDown][ActionPress])
                    {
                        vcd->cvAddParam[vcd->currentPreset] = -1;
                        vcd->buttonActionsUI[ButtonDown][ActionPress] = 0;
                        vcd->buttonActionsSFX[ButtonDown][ActionPress] = 0;
                        vcd->buttonActionsUI[ButtonEdit][ActionHoldContinuous] = 0;
                    }

                    //            OLEDdrawFloatArray(audioDisplayBuffer, -1.0f, 1.0f, 128, displayBufferIndex, 0, BothLines);
                }
                if (vcd->buttonActionsUI[ButtonEdit][ActionRelease] == 1)
                {
                    OLED_writePreset(vcd);
                    setLED_Edit(vcd, 0);
                    vcd->buttonActionsSFX[ButtonEdit][ActionRelease] = 0;
                    vcd->buttonActionsUI[ButtonEdit][ActionRelease] = 0;
                    vcd->buttonActionsUI[ButtonEdit][ActionHoldContinuous] = 0;
                    vcd->buttonActionsSFX[ButtonEdit][ActionHoldContinuous] = 0;

                }
                if (vcd->buttonActionsUI[ButtonDown][ActionPress] == 1)
                {

                    decrementPage(vcd);
                    OLEDwriteString(vcd, "P", 1, 110, FirstLine);
                    OLEDwriteInt(vcd, vcd->knobPage, 1, 120, FirstLine);
                    vcd->buttonActionsUI[ButtonDown][ActionPress] = 0;
                }
                if (vcd->buttonActionsUI[ButtonUp][ActionPress] == 1)
                {
                    incrementPage(vcd);
                    OLEDwriteString(vcd, "P", 1, 110, FirstLine);
                    OLEDwriteInt(vcd, vcd->knobPage, 1, 120, FirstLine);
                    vcd->buttonActionsUI[ButtonUp][ActionPress] = 0;
                }

                // Trying out an audio display
                //        if (buttonActionsUI[ButtonEdit][ActionPress] == 1)
                //        {
                //            currentParamIndex++;
                //            if (currentParamIndex > 7) currentParamIndex = 0;
                //            int controlLen = strlen(controlNames[currentParamIndex]);
                //            OLEDwriteString(controlNames[currentParamIndex], controlLen, 0, SecondLine);
                //            OLEDwriteString(" ", 1, getCursorX(), SecondLine);
                //            OLEDwriteString(paramNames[orderedParams[currentParamIndex]], getCursorX(), SecondLine);
                //            buttonActionsUI[ButtonEdit][ActionPress] = 0;
                //        }
            }
        }

        void adcCheck(Vocodec* vcd)
        {
            //read the analog inputs and smooth them with ramps
            for (int i = 0; i < 6; i++)
            {
                //floatADC[i] = (float) (ADC_values[i]>>8) * INV_TWO_TO_8;
                vcd->floatADC[i] = (float) ((*vcd->ADC_values)[i]>>6) * INV_TWO_TO_10;
            }
            if (vcd->firstADCPass)
            {
                for (int i = 0 ; i < 6; i++)
                {
                    vcd->lastFloatADC[i] = vcd->floatADC[i];
                }
                vcd->firstADCPass = 0;
            }
            for (int i = 0; i < 6; i++)
            {

                if (fabsf(vcd->floatADC[i] - vcd->lastFloatADC[i]) > vcd->adcHysteresisThreshold)
                {
                    if (vcd->buttonActionsUI[ButtonEdit][ActionHoldContinuous])
                    {
                        if (i != 5) vcd->cvAddParam[vcd->currentPreset] = i + (vcd->knobPage * KNOB_PAGE_SIZE);;
                        vcd->buttonActionsUI[ButtonEdit][ActionHoldContinuous] = 0;
                    }
                    vcd->lastFloatADC[i] = vcd->floatADC[i];
                    if (i == 5)
                    {
                        vcd->writeKnobFlag =
                        vcd->cvAddParam[vcd->currentPreset] - (vcd->knobPage * KNOB_PAGE_SIZE);
                    }
                    else vcd->writeKnobFlag = i;
                    vcd->knobActive[i] = 1;
                }
                // only do the following check after the knob has already passed the above
                // check once and floatADCUI has been set in OLED_writeKnobParameter
                if (vcd->floatADCUI[i] >= 0.0f)
                {
                    if (fabsf(vcd->smoothedADC[i] - vcd->floatADCUI[i]) > vcd->adcHysteresisThreshold)
                    {
                        if (i == 5)
                        {
                            vcd->writeKnobFlag =
                            vcd->cvAddParam[vcd->currentPreset] - (vcd->knobPage * KNOB_PAGE_SIZE);
                        }
                        else vcd->writeKnobFlag = i;
                    }
                }
                if (vcd->knobActive[i]) tExpSmooth_setDest(&vcd->adc[i], vcd->floatADC[i]);
            }

        }

        void clearButtonActions(Vocodec* vcd)
        {
            for (int b = 0; b < ButtonNil; b++)
            {
                for (int a = 0; a < ActionNil; a++)
                {
                    vcd->buttonActionsUI[b][a] = 0;
                    vcd->buttonActionsSFX[b][a] = 0;
                    vcd->writeButtonFlag = -1;
                    vcd->writeActionFlag = -1;
                }
            }
        }

        void changeTuning(Vocodec* vcd)
        {
            for (int i = 0; i < 12; i++)
            {
                vcd->centsDeviation[i] = tuningPresets[vcd->currentTuning][i];

            }
            if (vcd->currentTuning == 0)
            {
                //setLED_C(0);
            }
            else
            {
                ///setLED_C(1);
            }
            if (vcd->currentPreset == AutotuneMono)
            {
                calculateNoteArray(vcd);
            }
        }


        void writeCurrentPresetToFlash(Vocodec* vcd)
        {
#ifndef __cplusplus
            if((EE_WriteVariable(VirtAddVarTab[0],  vcd->currentPreset)) != HAL_OK)
            {
                Error_Handler();
            }
#endif
        }

        void incrementPage(Vocodec* vcd)
        {
            vcd->knobPage = (vcd->knobPage + 1) % vcd->numPages[vcd->currentPreset];
            setKnobValues(vcd, vcd->presetKnobValues[vcd->currentPreset] + (vcd->knobPage * KNOB_PAGE_SIZE));

        }

        void decrementPage(Vocodec* vcd)
        {
            if (vcd->knobPage == 0) vcd->knobPage = vcd->numPages[vcd->currentPreset] - 1;
            else vcd->knobPage--;
            setKnobValues(vcd, vcd->presetKnobValues[vcd->currentPreset] + (vcd->knobPage * KNOB_PAGE_SIZE));
        }

        void checkPage(Vocodec* vcd)
        {
            if (vcd->knobPage >= vcd->numPages[vcd->currentPreset])
                vcd->knobPage = vcd->numPages[vcd->currentPreset] - 1;
            setKnobValues(vcd, vcd->presetKnobValues[vcd->currentPreset] + (vcd->knobPage * KNOB_PAGE_SIZE));
        }

        void resetKnobValues(Vocodec* vcd)
        {
            for (int i = 0; i < NUM_ADC_CHANNELS; i++)
            {
                vcd->knobActive[i] = 0;
                vcd->floatADCUI[i] = -1.0f;
                float value = 0.0f;
                if (i != 5)
                {
                    value =
                    vcd->presetKnobValues[vcd->currentPreset][i + (vcd->knobPage * KNOB_PAGE_SIZE)];
                }
                tExpSmooth_setValAndDest(&vcd->adc[i], value);
                vcd->smoothedADC[i] = value;
            }
        }

        void setKnobValues(Vocodec* vcd, float* values)
        {
            for (int i = 0; i < KNOB_PAGE_SIZE; i++)
            {
                int knob = i;
                // if the knob is being replaced by cv pedal, set cv pedal instead
                if (knob + (vcd->knobPage * KNOB_PAGE_SIZE) == vcd->cvAddParam[vcd->currentPreset])
                {
                    knob = 5;
                }
                vcd->knobActive[knob] = 0;
                vcd->floatADCUI[knob] = -1.0f;
                tExpSmooth_setValAndDest(&vcd->adc[knob], values[knob]);

                vcd->smoothedADC[knob] = values[knob];
            }
        }

        void setKnobValue(Vocodec* vcd, int knob, float value)
        {
            // if the knob is being replaced by cv pedal, set cv pedal instead
            if (knob + (vcd->knobPage * KNOB_PAGE_SIZE) == vcd->cvAddParam[vcd->currentPreset])
            {
                knob = 5;
            }
            vcd->knobActive[knob] = 0;
            vcd->floatADCUI[knob] = -1.0f;
            tExpSmooth_setValAndDest(&vcd->adc[knob], value);
            vcd->smoothedADC[knob] = value;
        }

        void deactivateKnob(Vocodec* vcd, int knob)
        {
            vcd->knobActive[knob] = 0;
            vcd->floatADCUI[knob] = -1.0f;
        }

        void deactivateAllKnobs(Vocodec* vcd)
        {
            for (int i = 0; i < NUM_ADC_CHANNELS; i++)
            {
                vcd->knobActive[i] = 0;
                vcd->floatADCUI[i] = -1.0f;
            }
        }

        const char* UIVocoderButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress] == 1)
            {
                writeString = (vcd->vocoderParams.numVoices > 1) ? "POLY" : "MONO";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonB][ActionPress] == 1)
            {
                writeString = vcd->vocoderParams.internalExternal ? "EXTERNAL" : "INTERNAL";
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonC][ActionPress] == 1)
            {
                writeString = vcd->vocoderParams.freeze ? "FROZEN" : "UNFROZEN";
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UIVocoderChButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress] == 1)
            {
                writeString = (vcd->vocoderChParams.numVoices > 1) ? "POLY" : "MONO";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonB][ActionPress] == 1)
            {
                writeString = vcd->vocoderChParams.internalExternal ? "EXTERNAL" : "INTERNAL";
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonC][ActionPress] == 1)
            {
                writeString = vcd->vocoderChParams.freeze ? "FROZEN" : "UNFROZEN";
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UIPitchShiftButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            return writeString;
        }

        const char* UINeartuneButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress])
            {
                writeString = vcd->neartuneParams.useChromatic ? "AUTOCHROM ON" : "AUTOCHROM OFF";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }

            if (vcd->buttonActionsUI[ButtonC][ActionPress])
            {
                writeString = vcd->neartuneParams.lock ? "CHORD LOCK ON" : "CHORD LOCK OFF";
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }


            return writeString;
        }

        const char* UIAutotuneButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            return writeString;
        }

        const char* UISamplerBPButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonC][ActionPress])
            {
                OLEDclearLine(vcd, SecondLine);
                OLEDwriteFloat(vcd, vcd->sampleLength, 0, SecondLine);
                OLEDwriteString(vcd, vcd->samplerBPParams.paused ? "STOPPED" : "PLAYING", 7, 48, SecondLine);
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }

            if (vcd->buttonActionsUI[ButtonB][ActionPress])
            {
                OLEDclearLine(vcd, SecondLine);
                OLEDwriteString(vcd, vcd->samplerBPParams.playMode == PlayLoop ? "FORWARD     " : "BACKANDFORTH", 12, 0, SecondLine);
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonA][ActionHoldContinuous])
            {
                OLEDclearLine(vcd, SecondLine);
                OLEDwriteString(vcd, "RECORDING", 9, 0, SecondLine);
                OLEDwriteFloat(vcd, vcd->sampleLength, 84, SecondLine);
                vcd->buttonActionsUI[ButtonA][ActionHoldContinuous] = 0;
            }
            if (vcd->buttonActionsUI[ButtonA][ActionRelease])
            {
                OLEDclearLine(vcd, SecondLine);
                OLEDwriteFloat(vcd, vcd->sampleLength, 0, SecondLine);
                OLEDwriteString(vcd, vcd->samplerBPParams.paused ? "STOPPED" : "PLAYING", 7, 48, SecondLine);
                vcd->buttonActionsUI[ButtonA][ActionRelease] = 0;
            }
            return writeString;
        }

        const char* UISamplerKButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";


            if (vcd->buttonActionsUI[ExtraMessage][ActionHoldContinuous] || vcd->buttonActionsUI[ButtonA][ActionPress])

            {
                OLEDclearLine(vcd, SecondLine);
                OLEDwritePitch(vcd, vcd->currentSamplerKeyGlobal + LOWEST_SAMPLER_KEY, 0, SecondLine, false);
                OLEDwriteFloat(vcd, vcd->sampleLength, OLEDgetCursor(vcd), SecondLine);
                vcd->buttonActionsUI[ExtraMessage][ActionHoldContinuous] = 0;
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }

            if (vcd->buttonActionsUI[ButtonB][ActionPress])
            {
                writeString = vcd->samplerKParams.controlAllKeys ? "MOD ALL" : "MOD SINGLE";
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UISamplerAutoButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress])
            {
                if (vcd->samplerAutoParams.playMode == PlayLoop)
                {
                    writeString = "LOOP";
                }
                else if (vcd->samplerAutoParams.playMode == PlayBackAndForth)
                {
                    writeString = "BACK'N'FORTH";
                }
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonB][ActionPress])
            {
                writeString = vcd->samplerAutoParams.triggerChannel ? "CH2 TRIG" : "CH1 TRIG";
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonC][ActionPress])
            {
                writeString = vcd->samplerAutoParams.quantizeRate ? "QUANT SPEED" : "CONT SPEED";
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UIDistortionButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress])
            {
                writeString = vcd->distortionParams.mode ? "SHAPER" : "TANH";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UIWaveFolderButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress])
            {
                writeString = vcd->waveFolderParams.mode ? "TWO IN SERIES" : "OVERSAMPLED";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UIBitcrusherButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";

            if (vcd->buttonActionsUI[ButtonA][ActionPress])
            {
                writeString = vcd->bitcrusherParams.stereo ? "STEREO" : "MONO";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UIDelayButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress])
            {
                writeString = vcd->delayParams.shaper ? "SHAPER ON" : "SHAPER OFF";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonB][ActionPress])
            {
                writeString = vcd->delayParams.uncapFeedback ? "FB UNCAP" : "FB CAP";
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonC][ActionPress])
            {
                writeString = vcd->delayParams.freeze ? "FROZEN" : "UNFROZEN";
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UIReverbButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonC][ActionPress])
            {
                writeString = vcd->reverbParams.freeze ? "FROZEN" : "UNFROZEN";
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonB][ActionPress])
            {
                writeString = vcd->reverbParams.uncapFeedback ? "FB UNCAP" : "FB CAP";
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UIReverb2Buttons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonC][ActionPress])
            {
                writeString = vcd->reverb2Params.freeze ? "FROZEN" : "UNFROZEN";
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UILivingStringButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress] == 1)
            {
                writeString = (vcd->livingStringParams.ignoreFreqKnobs > 0) ? "MIDI PITCH" : "KNOB PITCH";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonC][ActionPress] == 1)
            {
                writeString = (vcd->livingStringParams.feedback > 0) ? "FB MODE" : "DECAY MODE";
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }

            if (vcd->buttonActionsUI[ButtonB][ActionPress] == 1)
            {
                writeString = (vcd->livingStringParams.independentStrings > 0) ? "INDIV CONTROL" : "KNOB1=>ALL";
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UILivingStringSynthButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress] == 1)
            {
                writeString = (vcd->livingStringSynthParams.numVoices > 1) ? "POLY" : "MONO";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonB][ActionPress] == 1)
            {
                writeString = (vcd->livingStringSynthParams.audioIn > 0) ? "AUDIO IN" : "NO AUDIO IN";
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonC][ActionPress] == 1)
            {
                writeString = (vcd->livingStringSynthParams.feedback > 0) ? "FB MODE" : "DECAY MODE";
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UIClassicSynthButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress] == 1)
            {
                writeString = (vcd->classicSynthParams.numVoices > 1) ? "POLY" : "MONO";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonB][ActionPress])
            {
                //writeString = knobPage == 0 ? "SETTINGS" : "ADSR";
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
            }
            return writeString;
        }

        const char* UIRhodesButtons(Vocodec* vcd, VocodecButton button, ButtonAction action)
        {
            const char* writeString = "";
            if (vcd->buttonActionsUI[ButtonA][ActionPress] == 1)
            {
                writeString = (vcd->rhodesParams.numVoices > 1) ? "POLY" : "MONO";
                vcd->buttonActionsUI[ButtonA][ActionPress] = 0;
            }
            if (vcd->buttonActionsUI[ButtonB][ActionPress] == 1)
            {
                vcd->buttonActionsUI[ButtonB][ActionPress] = 0;
                OLEDclearLine(vcd, SecondLine);
                OLEDwriteString(vcd, vcd->soundNames[vcd->rhodesParams.sound], 6, 0, SecondLine);
            }
            if (vcd->buttonActionsUI[ButtonC][ActionPress] == 1)
            {
                writeString = (vcd->rhodesParams.tremoloStereo > 0) ? "STEREO TREM" : "MONO TREM";
                vcd->buttonActionsUI[ButtonC][ActionPress] = 0;
            }
            return writeString;
        }

#ifdef __cplusplus
    }
} // extern "C"
#endif


