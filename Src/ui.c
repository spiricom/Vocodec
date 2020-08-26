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
#endif

#ifndef __cplusplus
        uint16_t ADC_values[NUM_ADC_CHANNELS] __ATTR_RAM_D2;
#else
        uint16_t ADC_values[NUM_ADC_CHANNELS];
#endif
        float floatADC[NUM_ADC_CHANNELS];
        float lastFloatADC[NUM_ADC_CHANNELS];
        float floatADCUI[NUM_ADC_CHANNELS];
        float adcHysteresisThreshold = 0.004f;
        tExpSmooth adc[6];
        float smoothedADC[6];

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
        const char* modeNames[PresetNil];
        const char* modeNamesDetails[PresetNil];
        const char* shortModeNames[PresetNil];

        const char* knobParamNames[PresetNil][NUM_PRESET_KNOB_VALUES];

        int8_t currentParamIndex = -1;
        uint8_t orderedParams[8];

        uint8_t buttonActionsSFX[NUM_BUTTONS+1][ActionNil];
        uint8_t buttonActionsUI[NUM_BUTTONS+1][ActionNil];
        float displayValues[NUM_PRESET_KNOB_VALUES];
        int8_t cvAddParam[PresetNil];
        const char* (*buttonActionFunctions[PresetNil])(VocodecButton, ButtonAction);

        VocodecPresetType currentPreset = (VocodecPresetType)0;
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
            knobParamNames[Vocoder][4] = "NOISTHRESH";
            knobParamNames[Vocoder][5] = "BREATH";
            knobParamNames[Vocoder][6] = "TILT";
            knobParamNames[Vocoder][7] = "PULSEWIDTH";
            knobParamNames[Vocoder][8] = "PULSESHAPE";
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
            knobParamNames[VocoderCh][8] = "BREATH";
            knobParamNames[VocoderCh][9] = "SPEED";
            knobParamNames[VocoderCh][10] = "BANDSQUISH";
            knobParamNames[VocoderCh][11] = "BANDOFF";
            knobParamNames[VocoderCh][12] = "TILT";
            knobParamNames[VocoderCh][13] = "STEREO";
            knobParamNames[VocoderCh][14] = "BARKPULL";

            modeNames[Pitchshift] = "PITCHSHIFT";
            shortModeNames[Pitchshift] = "PS";
            modeNamesDetails[Pitchshift] = "";
            numPages[Pitchshift] = 2;
            knobParamNames[Pitchshift][0] = "SHIFT";
            knobParamNames[Pitchshift][1] = "FINE";
            knobParamNames[Pitchshift][2] = "F AMT";
            knobParamNames[Pitchshift][3] = "FORMANT";
            knobParamNames[Pitchshift][4] = "RANGE";
            knobParamNames[Pitchshift][5] = "OFFSET";
            knobParamNames[Pitchshift][6] = "";
            knobParamNames[Pitchshift][7] = "";
            knobParamNames[Pitchshift][8] = "";
            knobParamNames[Pitchshift][9] = "";

            modeNames[AutotuneMono] = "AUTOTUNE";
            shortModeNames[AutotuneMono] = "NT";
            modeNamesDetails[AutotuneMono] = "";
            numPages[AutotuneMono] = 1;
            knobParamNames[AutotuneMono][0] = "PICKINESS";
            knobParamNames[AutotuneMono][1] = "AMOUNT";
            knobParamNames[AutotuneMono][2] = "SPEED";
            knobParamNames[AutotuneMono][3] = "LEAPALLOW";
            knobParamNames[AutotuneMono][4] = "HYSTERESIS";


            modeNames[AutotunePoly] = "HARMONIZE";
            shortModeNames[AutotunePoly] = "AT";
            modeNamesDetails[AutotunePoly] = "";
            numPages[AutotunePoly] = 1;
            knobParamNames[AutotunePoly][0] = "PICKINESS";
            knobParamNames[AutotunePoly][1] = "";
            knobParamNames[AutotunePoly][2] = "";
            knobParamNames[AutotunePoly][3] = "";
            knobParamNames[AutotunePoly][4] = "";


            modeNames[SamplerButtonPress] = "SAMPLER BP";
            shortModeNames[SamplerButtonPress] = "SB";
            modeNamesDetails[SamplerButtonPress] = "PRESS BUTTON A";
            numPages[SamplerButtonPress] = 1;
            knobParamNames[SamplerButtonPress][0] = "START";
            knobParamNames[SamplerButtonPress][1] = "LENGTH";
            knobParamNames[SamplerButtonPress][2] = "SPEED";
            knobParamNames[SamplerButtonPress][3] = "SPEEDMULT";
            knobParamNames[SamplerButtonPress][4] = "CROSSFADE";



            modeNames[SamplerKeyboard] = "KEYSAMPLER";
            shortModeNames[SamplerKeyboard] = "KS";
            modeNamesDetails[SamplerKeyboard] = "KEY TO REC";
            numPages[SamplerKeyboard] = 2;
            knobParamNames[SamplerKeyboard][0] = "START";
            knobParamNames[SamplerKeyboard][1] = "LENGTH";
            knobParamNames[SamplerKeyboard][2] = "SPEED";
            knobParamNames[SamplerKeyboard][3] = "SPEEDMULT";
            knobParamNames[SamplerKeyboard][4] = "LOOP ON";
            knobParamNames[SamplerKeyboard][5] = "CROSSFADE";
            knobParamNames[SamplerKeyboard][6] = "VELO SENS";
            knobParamNames[SamplerKeyboard][7] = "";
            knobParamNames[SamplerKeyboard][8] = "";
            knobParamNames[SamplerKeyboard][9] = "";


            modeNames[SamplerAutoGrab] = "AUTOSAMP";
            shortModeNames[SamplerAutoGrab] = "AS";
            modeNamesDetails[SamplerAutoGrab] = "AUDIO TRIG'D";
            numPages[SamplerAutoGrab] = 2;
            knobParamNames[SamplerAutoGrab][0] = "THRESHOLD";
            knobParamNames[SamplerAutoGrab][1] = "WINDOW";
            knobParamNames[SamplerAutoGrab][2] = "SPEED";
            knobParamNames[SamplerAutoGrab][3] = "CROSSFADE";
            knobParamNames[SamplerAutoGrab][4] = "";
            knobParamNames[SamplerAutoGrab][5] = "LEN RAND";
            knobParamNames[SamplerAutoGrab][6] = "SPD RAND";
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

            modeNames[Wavefolder] = "WAVEFOLD";
            shortModeNames[Wavefolder] = "WF";
            modeNamesDetails[Wavefolder] = "SERGE STYLE";
            numPages[Wavefolder] = 1;
            knobParamNames[Wavefolder][0] = "GAIN";
            knobParamNames[Wavefolder][1] = "OFFSET1";
            knobParamNames[Wavefolder][2] = "OFFSET2";
            knobParamNames[Wavefolder][3] = "POST GAIN";
            knobParamNames[Wavefolder][4] = "";

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
            knobParamNames[BitCrusher][6] = "";
            knobParamNames[BitCrusher][7] = "";
            knobParamNames[BitCrusher][8] = "";
            knobParamNames[BitCrusher][9] = "";

            modeNames[Delay] = "DELAY";
            shortModeNames[Delay] = "DL";
            modeNamesDetails[Delay] = "STEREO";
            numPages[Delay] = 2;
            knobParamNames[Delay][0] = "DELAY_L";
            knobParamNames[Delay][1] = "DELAY_R";
            knobParamNames[Delay][2] = "HIGHPASS";
            knobParamNames[Delay][3] = "LOWPASS";
            knobParamNames[Delay][4] = "FEEDBACK";
            knobParamNames[Delay][5] = "POST GAIN";
            knobParamNames[Delay][6] = "";
            knobParamNames[Delay][7] = "";
            knobParamNames[Delay][8] = "";
            knobParamNames[Delay][9] = "";

            modeNames[Reverb] = "REVERB1";
            shortModeNames[Reverb] = "RV";
            modeNamesDetails[Reverb] = "DATTORRO ALG";
            numPages[Reverb] = 1;
            knobParamNames[Reverb][0] = "SIZE";
            knobParamNames[Reverb][1] = "FB LOPASS";
            knobParamNames[Reverb][2] = "IN HIPASS";
            knobParamNames[Reverb][3] = "IN LOPASS";
            knobParamNames[Reverb][4] = "FB GAIN";


            modeNames[Reverb2] = "REVERB2";
            shortModeNames[Reverb2] = "RV";
            modeNamesDetails[Reverb2] = "NREVERB ALG";
            numPages[Reverb2] = 1;
            knobParamNames[Reverb2][0] = "SIZE";
            knobParamNames[Reverb2][1] = "LOWPASS";
            knobParamNames[Reverb2][2] = "HIGHPASS";
            knobParamNames[Reverb2][3] = "PEAK_FREQ";
            knobParamNames[Reverb2][4] = "PEAK_GAIN";

            modeNames[LivingString] = "STRING1";
            shortModeNames[LivingString] = "LS";
            modeNamesDetails[LivingString] = "SYMP STRING";
            numPages[LivingString] = 3;
            knobParamNames[LivingString][0] = "FREQ1";
            knobParamNames[LivingString][1] = "DETUNE";
            knobParamNames[LivingString][2] = "DECAY";
            knobParamNames[LivingString][3] = "DAMPING";
            knobParamNames[LivingString][4] = "PICK POS";
            knobParamNames[LivingString][5] = "PREP POS";
            knobParamNames[LivingString][6] = "PREP FORCE";
            knobParamNames[LivingString][7] = "LET RING";
            knobParamNames[LivingString][8] = "";
            knobParamNames[LivingString][9] = "";
            knobParamNames[LivingString][10] = "FREQ2";
            knobParamNames[LivingString][11] = "FREQ3";
            knobParamNames[LivingString][12] = "FREQ4";
            knobParamNames[LivingString][13] = "FREQ5";
            knobParamNames[LivingString][14] = "FREQ6";

            modeNames[LivingStringSynth] = "STRING2";
            shortModeNames[LivingStringSynth] = "SS";
            modeNamesDetails[LivingStringSynth] = "STRING SYNTH";
            numPages[LivingStringSynth] = 2;
            knobParamNames[LivingStringSynth][0] = "PLUCK VOL";
            knobParamNames[LivingStringSynth][1] = "PLUCK TONE";
            knobParamNames[LivingStringSynth][2] = "DECAY";
            knobParamNames[LivingStringSynth][3] = "DAMPING";
            knobParamNames[LivingStringSynth][4] = "PICK_POS";
            knobParamNames[LivingStringSynth][5] = "PREP POS";
            knobParamNames[LivingStringSynth][6] = "PREP FORCE";
            knobParamNames[LivingStringSynth][7] = "LET RING";
            knobParamNames[LivingStringSynth][8] = "FB LEVEL";
            knobParamNames[LivingStringSynth][9] = "RELEASE";

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
            numPages[Rhodes] = 5;
            knobParamNames[Rhodes][0] = "BRIGHTNESS";
            knobParamNames[Rhodes][1] = "TREM DEPTH";
            knobParamNames[Rhodes][2] = "TREM RATE";
            knobParamNames[Rhodes][3] = "DRIVE";
            knobParamNames[Rhodes][4] = "PAN SPREAD";
            knobParamNames[Rhodes][5] = "ATTACK";
            knobParamNames[Rhodes][6] = "DECAY";
            knobParamNames[Rhodes][7] = "SUSTAIN";
            knobParamNames[Rhodes][8] = "RELEASE";
            knobParamNames[Rhodes][9] = "LEAK";
            knobParamNames[Rhodes][10] = "INDEX1";
            knobParamNames[Rhodes][11] = "INDEX2";
            knobParamNames[Rhodes][12] = "INDEX3";
            knobParamNames[Rhodes][13] = "INDEX4";
            knobParamNames[Rhodes][14] = "INDEX5";
            knobParamNames[Rhodes][15] = "RATIO1";
            knobParamNames[Rhodes][16] = "RATIO2";
            knobParamNames[Rhodes][17] = "RATIO3";
            knobParamNames[Rhodes][18] = "RATIO4";
            knobParamNames[Rhodes][19] = "RATIO5";
            knobParamNames[Rhodes][20] = "RATIO6";
            knobParamNames[Rhodes][21] = "FEEDBACK";
            knobParamNames[Rhodes][22] = "TUNE SNAP";
            knobParamNames[Rhodes][23] = "RAND DECAY";
            knobParamNames[Rhodes][24] = "RAND SUST";
        }

        void buttonCheck(void)
        {
#ifndef __cplusplus
            if (codecReady)
            {
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
                buttonValues[0] =!(GPIOB->IDR & GPIO_PIN_13);
                buttonValues[1] =!(GPIOB->IDR & GPIO_PIN_12);
                buttonValues[2] =!(GPIOB->IDR & GPIO_PIN_14);
                buttonValues[3] =!(GPIOD->IDR & GPIO_PIN_11);
                buttonValues[4] =!(GPIOB->IDR & GPIO_PIN_15);
                buttonValues[5] =!(GPIOB->IDR & GPIO_PIN_1);
                buttonValues[6] =!(GPIOD->IDR & GPIO_PIN_7);
                buttonValues[7] =!(GPIOB->IDR & GPIO_PIN_11);
                buttonValues[8] =!(GPIOG->IDR & GPIO_PIN_11);
                buttonValues[9] =!(GPIOB->IDR & GPIO_PIN_10);
#else
                {
#endif
                    for (int i = 0; i < NUM_BUTTONS; i++)
                    {
                        if (buttonValues[i] != buttonValuesPrev[i])
                        {
                            buttonHysteresis[i]++;
                        }
                        if (cleanButtonValues[i] == 1)
                        {
                            buttonActionsSFX[i][ActionHoldContinuous] = 1;
                            buttonActionsUI[i][ActionHoldContinuous] = 1;
                            writeButtonFlag = i;
                            writeActionFlag = ActionHoldContinuous;
                        }
                        if (buttonHysteresis[i] < buttonHysteresisThreshold)
                        {
                            if (buttonCounters[i] < buttonHoldMax) buttonCounters[i]++;
                            if ((buttonCounters[i] >= buttonHoldThreshold) && (cleanButtonValues[i] == 1))
                            {
                                buttonActionsSFX[i][ActionHoldInstant] = 1;
                                buttonActionsUI[i][ActionHoldInstant] = 1;
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
                                buttonActionsSFX[i][ActionPress] = 1;
                                buttonActionsUI[i][ActionPress] = 1;
                                writeButtonFlag = i;
                                writeActionFlag = ActionPress;
                            }
                            else if (cleanButtonValues[i] == 0)
                            {
                                buttonActionsSFX[i][ActionRelease] = 1;
                                buttonActionsUI[i][ActionRelease] = 1;
                                writeButtonFlag = i;
                                writeActionFlag = ActionRelease;
                            }
                            buttonValuesPrev[i] = buttonValues[i];
                        }
                    }

                    // make some if statements if you want to find the "attack" of the buttons (getting the "press" action)

                    /// DEFINE GLOBAL BUTTON BEHAVIOR HERE

                    if (buttonActionsUI[ButtonLeft][ActionPress] == 1)
                    {
                        previousPreset = currentPreset;

                        if (currentPreset <= 0) currentPreset = (VocodecPresetType)((int)PresetNil - 1);
                        else currentPreset = (VocodecPresetType)((int)currentPreset - 1);

                        loadingPreset = 1;
                        OLED_writePreset();
                        writeCurrentPresetToFlash();
                        clearButtonActions();
                    }
                    if (buttonActionsUI[ButtonRight][ActionPress] == 1)
                    {
                        previousPreset = currentPreset;
                        if (currentPreset >= PresetNil - 1) currentPreset = (VocodecPresetType)0;
                        else currentPreset = (VocodecPresetType)((int)currentPreset + 1);


                        loadingPreset = 1;
                        OLED_writePreset();
                        writeCurrentPresetToFlash();
                        clearButtonActions();
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
                        setLED_Edit(1);
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
                            buttonActionsSFX[ButtonC][ActionPress] = 0;
                            buttonActionsUI[ButtonEdit][ActionHoldContinuous] = 0;
                        }
                        if (buttonActionsUI[ButtonDown][ActionPress])
                        {
                            cvAddParam[currentPreset] = -1;
                            buttonActionsUI[ButtonDown][ActionPress] = 0;
                            buttonActionsSFX[ButtonDown][ActionPress] = 0;
                            buttonActionsUI[ButtonEdit][ActionHoldContinuous] = 0;
                        }

                        //            OLEDdrawFloatArray(audioDisplayBuffer, -1.0f, 1.0f, 128, displayBufferIndex, 0, BothLines);
                    }
                    if (buttonActionsUI[ButtonEdit][ActionRelease] == 1)
                    {
                        OLED_writePreset();
                        setLED_Edit(0);
                        buttonActionsSFX[ButtonEdit][ActionRelease] = 0;
                        buttonActionsUI[ButtonEdit][ActionRelease] = 0;
                        buttonActionsUI[ButtonEdit][ActionHoldContinuous] = 0;
                        buttonActionsSFX[ButtonEdit][ActionHoldContinuous] = 0;

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
            int firstADCPass = 1;
            void adcCheck()
            {
                //read the analog inputs and smooth them with ramps
                for (int i = 0; i < 6; i++)
                {
                    //floatADC[i] = (float) (ADC_values[i]>>8) * INV_TWO_TO_8;
                    floatADC[i] = (float) (ADC_values[i]>>6) * INV_TWO_TO_10;
                }
                if (firstADCPass)
                {
                    for (int i = 0 ; i < 6; i++)
                    {
                        lastFloatADC[i] = floatADC[i];
                    }
                    firstADCPass = 0;
                }
                for (int i = 0; i < 6; i++)
                {

                    if (fabsf(floatADC[i] - lastFloatADC[i]) > adcHysteresisThreshold)
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
                        if (fabsf(smoothedADC[i] - floatADCUI[i]) > adcHysteresisThreshold)
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
                #ifndef __cplusplus
                if((EE_WriteVariable(VirtAddVarTab[0],  currentPreset)) != HAL_OK)
                {
                    Error_Handler();
                }
                #endif
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

            const char* UIVocoderButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
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
                    writeString = vocFreezeLPC ? "FROZEN" : "UNFROZEN";
                    buttonActionsUI[ButtonC][ActionPress] = 0;
                }
                return writeString;
            }

            const char* UIVocoderChButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
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

            const char* UIPitchShiftButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                return writeString;
            }

            const char* UINeartuneButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                if (buttonActionsUI[ButtonA][ActionPress])
                {
                    writeString = autotuneChromatic ? "AUTOCHROM ON" : "AUTOCHROM OFF";
                    buttonActionsUI[ButtonA][ActionPress] = 0;
                }

                if (buttonActionsUI[ButtonC][ActionPress])
                {
                    writeString = autotuneLock ? "CHORD LOCK ON" : "CHORD LOCK OFF";
                    buttonActionsUI[ButtonC][ActionPress] = 0;
                }


                return writeString;
            }

            const char* UIAutotuneButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                return writeString;
            }

            const char* UISamplerBPButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                if (buttonActionsUI[ButtonC][ActionPress])
                {
                    OLEDclearLine(SecondLine);
                    OLEDwriteFloat(sampleLength, 0, SecondLine);
                    OLEDwriteString(samplePlaying ? "PLAYING" : "STOPPED", 7, 48, SecondLine);
                    buttonActionsUI[ButtonC][ActionPress] = 0;
                }

                if (buttonActionsUI[ButtonB][ActionPress])
                {
                    OLEDclearLine(SecondLine);
                    OLEDwriteString(bpMode ? "BACKANDFORTH" : "FORWARD     ", 12, 0, SecondLine);
                    buttonActionsUI[ButtonB][ActionPress] = 0;
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

            const char* UISamplerKButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";


                if (buttonActionsUI[ExtraMessage][ActionHoldContinuous] || buttonActionsUI[ButtonA][ActionPress])

                {
                    OLEDclearLine(SecondLine);
                    OLEDwritePitch(currentSamplerKeyGlobal + LOWEST_SAMPLER_KEY, 0, SecondLine, false);
                    OLEDwriteFloat(sampleLength, OLEDgetCursor(), SecondLine);
                    buttonActionsUI[ExtraMessage][ActionHoldContinuous] = 0;
                    buttonActionsUI[ButtonA][ActionPress] = 0;
                }

                if (buttonActionsUI[ButtonB][ActionPress])
                {
                    writeString = controlAllKeys ? "MOD ALL" : "MOD SINGLE";
                    buttonActionsUI[ButtonB][ActionPress] = 0;
                }
                return writeString;
            }

            const char* UISamplerAutoButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
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
                if (buttonActionsUI[ButtonC][ActionPress])
                {
                    writeString = pitchQuantization ? "QUANT SPEED" : "CONT SPEED";
                    buttonActionsUI[ButtonC][ActionPress] = 0;
                }
                return writeString;
            }

            const char* UIDistortionButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                if (buttonActionsUI[ButtonA][ActionPress])
                {
                    writeString = distortionMode ? "SHAPER" : "TANH";
                    buttonActionsUI[ButtonA][ActionPress] = 0;
                }
                return writeString;
            }

            const char* UIWaveFolderButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                if (buttonActionsUI[ButtonA][ActionPress])
                {
                    writeString = foldMode ? "TWO IN SERIES" : "OVERSAMPLED";
                    buttonActionsUI[ButtonA][ActionPress] = 0;
                }
                return writeString;
            }

            const char* UIBitcrusherButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";

                if (buttonActionsUI[ButtonA][ActionPress])
                {
                    writeString = crusherStereo ? "STEREO" : "MONO";
                    buttonActionsUI[ButtonA][ActionPress] = 0;
                }


                return writeString;
            }

            const char* UIDelayButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
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

            const char* UIReverbButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                if (buttonActionsUI[ButtonC][ActionPress])
                {
                    writeString = freeze ? "FREEZE" : "UNFREEZE";
                    buttonActionsUI[ButtonC][ActionPress] = 0;
                }
                if (buttonActionsUI[ButtonB][ActionPress])
                {
                    writeString = capFeedback ? "FB CAP" : "FB UNCAP";
                    buttonActionsUI[ButtonB][ActionPress] = 0;
                }
                return writeString;
            }

            const char* UIReverb2Buttons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                if (buttonActionsUI[ButtonC][ActionPress])
                {
                    writeString = freeze ? "FREEZE" : "UNFREEZE";
                    buttonActionsUI[ButtonC][ActionPress] = 0;
                }
                return writeString;
            }

            const char* UILivingStringButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                if (buttonActionsUI[ButtonA][ActionPress] == 1)
                {
                    writeString = (ignoreFreqKnobs > 0) ? "MIDI PITCH" : "KNOB PITCH";
                    buttonActionsUI[ButtonA][ActionPress] = 0;
                }
                if (buttonActionsUI[ButtonC][ActionPress] == 1)
                {
                    writeString = (levMode > 0) ? "FB MODE" : "DECAY MODE";
                    buttonActionsUI[ButtonC][ActionPress] = 0;
                }

                if (buttonActionsUI[ButtonB][ActionPress] == 1)
                {
                    writeString = (independentStrings > 0) ? "INDIV CONTROL" : "KNOB1=>ALL";
                    buttonActionsUI[ButtonB][ActionPress] = 0;
                }

                return writeString;
            }

            const char* UILivingStringSynthButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                if (buttonActionsUI[ButtonA][ActionPress] == 1)
                {
                    writeString = (numVoices > 1) ? "POLY" : "MONO";
                    buttonActionsUI[ButtonA][ActionPress] = 0;
                }
                if (buttonActionsUI[ButtonB][ActionPress] == 1)
                {
                    writeString = (voicePluck > 0) ? "AUDIO IN" : "NO AUDIO IN";
                    buttonActionsUI[ButtonB][ActionPress] = 0;
                }
                if (buttonActionsUI[ButtonC][ActionPress] == 1)
                {
                    writeString = (levModeStr > 0) ? "FB MODE" : "DECAY MODE";
                    buttonActionsUI[ButtonC][ActionPress] = 0;
                }
                return writeString;
            }

            const char* UIClassicSynthButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
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

            const char* UIRhodesButtons(VocodecButton button, ButtonAction action)
            {
                const char* writeString = "";
                if (buttonActionsUI[ButtonA][ActionPress] == 1)
                {
                    writeString = (numVoices > 1) ? "POLY" : "MONO";
                    buttonActionsUI[ButtonA][ActionPress] = 0;
                }
                if (buttonActionsUI[ButtonB][ActionPress] == 1)
                {
                    buttonActionsUI[ButtonB][ActionPress] = 0;
                    OLEDclearLine(SecondLine);
                    OLEDwriteString(soundNames[Rsound], 6, 0, SecondLine);
                }
                if (buttonActionsUI[ButtonC][ActionPress] == 1)
                {
                    buttonActionsUI[ButtonC][ActionPress] = 0;
                    OLEDclearLine(SecondLine);
                    OLEDwriteString("STEREO TREMO", 12, 0, SecondLine);
                    OLEDwriteInt(tremoloStereo, 1, 110, SecondLine);
                }
                return writeString;
            }

#ifdef __cplusplus
        }
    } // extern "C"
#endif
