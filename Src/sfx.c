/*
 * sfx.c
 *
 *  Created on: Dec 23, 2019
 *      Author: josnyder
 */

#ifndef __cplusplus
#include "main.h"
#include "MIDI_application.h"
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
        
#define INC_MISC_WT 0
#define USE_FILTERTAN_TABLE 1
        
        const float defaultBarkBandFreqs[24] = {100.0f, 150.0f, 250.0f, 350.0f, 450.0f, 570.0f, 700.0f, 840.0f, 1000.0f, 1170.0f, 1370.0f, 1600.0f, 1850.0f, 2150.0f, 2500.0f, 2900.0f, 3400.0f, 4000.0f, 4800.0f, 5800.0f, 7000.0f, 8500.0f, 10500.0f, 12000.0f};
        
        const float defaultBarkBandWidths[24] = {1.0f, 1.0f, 0.5849f, 0.4150f, 0.3505f, 0.304854f, 0.2895066175f, 0.256775415f, 0.231325545833333f, 0.233797185f, 0.220768679166667f, 0.216811389166667f, 0.217591435f, 0.214124805f, 0.218834601666667f, 0.222392421666667f, 0.2321734425f, 0.249978253333333f, 0.268488835833333f, 0.272079545833333f, 0.266786540833333f, 0.3030690675f, 0.3370349875f, 0.36923381f};
        
        const float default_FM_freqRatios[5][6] = {{1.0f, 1.00001f, 1.0f, 3.0f, 1.0f, 1.0f}, {2.0f, 2.0001f, .99999f, 3.0f, 5.0f, 8.0f},  {1.0f, 2.0f, 1.0f, 7.0f, 3.0f, 4.0f}, {1.0f, 2.0f, 1.0f, 7.0f, 3.0f, 4.0f}, {1.0f, 2.0f, 1.0f, 7.0f, 3.0f, 4.0f}};
        const float default_FM_indices[5][6] = {{800.0f, 0.0f, 120.0f, 32.0f, 3.0f, 1.0f}, {100.0f, 100.0f, 300.0f, 300.0f, 10.0f, 5.0f}, {500.0f, 50.0f, 500.0f, 10.0f,0.0f, 0.0f}, {50.0f, 128.0f, 1016.0f, 528.0f, 4.0f, 0.0f},{50.0f, 128.0f, 1016.0f, 528.0f, 4.0f, 0.0f}};
        
#ifndef __cplusplus
        Vocodec vocodec;
        char small_memory[SMALL_MEM_SIZE];
        char medium_memory[MED_MEM_SIZE] __ATTR_RAM_D1;
        char large_memory[LARGE_MEM_SIZE] __ATTR_SDRAM;
#endif

        void SFX_init(Vocodec* vcd, uint16_t (*ADC_values)[NUM_ADC_CHANNELS],
                      void (*loadFunction)(Vocodec* vcd))
        {
            vcd->ADC_values = ADC_values;

            vcd->expBufferSizeMinusOne = EXP_BUFFER_SIZE - 1;
            vcd->decayExpBufferSizeMinusOne = DECAY_EXP_BUFFER_SIZE - 1;

            for (int i = 0; i < 12; ++i)
            {
                vcd->chordArray[i] = 0;
                vcd->chromaticArray[i] = 1;
                vcd->lockArray[i] = 0;
            }

            //============================================//

            vcd->vocoderParams.numVoices = NUM_VOC_VOICES;
            vcd->vocoderParams.internalExternal = 1;
            vcd->vocoderParams.freeze = 0;

            vcd->vocoderChParams.numVoices = NUM_VOC_VOICES;
            vcd->vocoderChParams.internalExternal = 1;
            vcd->vocoderChParams.freeze = 0;

            vcd->pitchShiftParams._ = 0;

            vcd->neartuneParams.useChromatic = 0;
            vcd->neartuneParams.lock = 0;

            vcd->autotuneParams._ = 0;

            vcd->samplerBPParams.playMode = PlayLoop;
            vcd->samplerBPParams.paused = 0;

            vcd->samplerKParams.controlAllKeys = 0;

            vcd->samplerAutoParams.playMode = PlayLoop;
            vcd->samplerAutoParams.triggerChannel = 0;
            vcd->samplerAutoParams.quantizeRate = 0;

            vcd->distortionParams.mode = 0;

            vcd->waveFolderParams.mode = 0;

            vcd->bitcrusherParams.stereo = 0;

            vcd->delayParams.shaper = 0;
            vcd->delayParams.uncapFeedback = 0;
            vcd->delayParams.freeze = 0;

            vcd->reverbParams.uncapFeedback = 0;
            vcd->reverbParams.freeze = 0;

            vcd->reverb2Params.freeze = 0;

            vcd->livingStringParams.ignoreFreqKnobs = 0;
            vcd->livingStringParams.independentStrings = 0;
            vcd->livingStringParams.feedback = 0;

            vcd->livingStringSynthParams.numVoices = NUM_STRINGS_SYNTH;
            vcd->livingStringSynthParams.audioIn = 0;
            vcd->livingStringSynthParams.feedback = 0;

            vcd->classicSynthParams.numVoices = NUM_VOC_VOICES;

            vcd->rhodesParams.numVoices = NUM_VOC_VOICES;
            vcd->rhodesParams.sound = 0;
            vcd->rhodesParams.tremoloStereo = 0;


            //TODO initialize tape emulation params


            //============================================//

            vcd->numberOfVocoderBands = 22;
            vcd->prevNumberOfVocoderBands = 22;
            vcd->invNumberOfVocoderBands = 0.03125f;

            vcd->whichKnobThisTime = 0;

            vcd->currentBandToAlter = 0;
            vcd->analysisOrSynthesis = 0;
            vcd->alteringBands = 0;

            vcd->prevMyQ = 1.0f;
            vcd->invMyQ = 1.0f;
            vcd->prevWarpFactor = 1.0f;

            vcd->prevBandSquish = 1.0f;
            vcd->prevBandOffset = 30.0f;
            vcd->prevMyTilt = 0.0f;
            vcd->prevBarkPull = 0.0f;

            for (int i = 0; i < 24; ++i)
            {
                vcd->barkBandFreqs[i] = defaultBarkBandFreqs[i];
                vcd->barkBandWidths[i] = defaultBarkBandWidths[i];
            }

            vcd->oneMinusStereo = 1.0f;
            vcd->chVocOutputGain = 1.0f;

            // pitch shift
            vcd->pitchShiftRange = 2.0f;
            vcd->pitchShiftOffset = -1.0f;

            //5 autotune mono
            vcd->lastSnap = 1.0f;
            vcd->detectedNote = 60.0f;
            vcd->desiredSnap = 60.0f;
            vcd->destinationNote = 60.0f;
            vcd->destinationFactor = 1.0f;
            vcd->factorDiff = 0.0f;
            vcd->changeAmount = 0.0f;

            //6 autotune

            //7 sampler - button press
            vcd->samplePlayStart = 0;
            vcd->samplePlayLength = 0;
            vcd->sampleLength = 0.0f;
            vcd->crossfadeLength = 0;
            vcd->samplerRate = 1.0f;

            // keyboard sampler
            vcd->currentSamplerKeyGlobal = 60 - LOWEST_SAMPLER_KEY;
            vcd->prevSamplerKey = 60;

            vcd->samp_thresh = 0.0f;

            //8 sampler - auto

            vcd->currentPower = 0.0f;
            vcd->previousPower = 0.0f;

            vcd->samp_triggered = 0;
            vcd->sample_countdown = 0;
            vcd->currentSampler = 0;
            vcd->randLengthVal = 0;
            vcd->randRateVal = 0.0f;

            vcd->fadeDone = 0;
            vcd->finalWindowSize = 5000;

            //10 distortion tanh
            vcd->distOS_ratio = 4;

            // distortion wave folder

            //13 bitcrusher

            //delay

            //reverb

            //Living String

            //Living String Synth
            vcd->samplesPerMs = 1;

            // CLASSIC SUBTRACTIVE SYNTH


            ///FM RHODES ELECTRIC PIANO SYNTH

            vcd->soundNames[0] = "DARK  ";
            vcd->soundNames[1] = "LIGHT ";
            vcd->soundNames[2] = "BASS  ";
            vcd->soundNames[3] = "PAD   ";
            vcd->soundNames[4] = "CUSTOM";

            for (int i = 0; i < 5; ++i)
            {
                for (int j = 0; j < 6; ++j)
                {
                    vcd->FM_freqRatios[i][j] = default_FM_freqRatios[i][j];
                    vcd->FM_indices[i][j] = default_FM_indices[i][j];
                }
            }

            vcd->feedback_output = 0.0f;

            vcd->overtoneSnap = 1.0f;
            for (int i = 0; i < 6; ++i)
            {
                vcd->randomDecays[i] = 1.0f;
                vcd->randomSustains[i] = 1.0f;
            }
            
            // midi functions

            vcd->pitchBendValue = 0.0f;
            vcd->lastNearNote = -1;


            // UI /////////

            vcd->adcHysteresisThreshold = 0.004f;

            vcd->knobPage = 0;
#ifndef __cplusplus
            vcd->buttonHysteresisThreshold = 5;
#else
            vcd->buttonHysteresisThreshold = 1;
#endif
            vcd->buttonHoldThreshold = 200;
            vcd->buttonHoldMax = 200;

            vcd->writeKnobFlag = -1;
            vcd->writeButtonFlag = -1;
            vcd->writeActionFlag = -1;

            vcd->currentParamIndex = -1;

            vcd->currentPreset = (VocodecPresetType)0;
            vcd->previousPreset = PresetNil;
            vcd->loadingPreset = 0;

            vcd->firstADCPass = 1;

            for (int i = 0; i < (int)VocodecLightNil; ++i)
                vcd->lightStates[i] = 0;

            // TUNING
            
            for (int i = 0; i < 12; ++i)
                vcd->centsDeviation[i] = 0.0f;
            vcd->currentTuning = 0;
            vcd->keyCenter = 0;
            
            initFunctionPointers(vcd);
            initPresetParams(vcd);
        }

        void initPresetParams(Vocodec* vcd)
        {
            // Note that these are the actual knob values
            // not the parameter value
            // (i.e. 0.5 for fine pitch is actually 0.0 fine pitch)
            vcd->defaultPresetKnobValues[Vocoder][0] = 0.4f; // volume
            vcd->defaultPresetKnobValues[Vocoder][1] = 0.5f; // warp factor
            vcd->defaultPresetKnobValues[Vocoder][2] = 0.85f; // quality
            vcd->defaultPresetKnobValues[Vocoder][3] = 0.0f; // sawToPulse
            vcd->defaultPresetKnobValues[Vocoder][4] = 0.2f; // noise threshold
            vcd->defaultPresetKnobValues[Vocoder][5] = 0.02f; // breathiness
            vcd->defaultPresetKnobValues[Vocoder][6] = 0.5f; // tilt
            vcd->defaultPresetKnobValues[Vocoder][7] = 0.5f; // pulse width
            vcd->defaultPresetKnobValues[Vocoder][8] = 0.5f; // pulse shape
            vcd->defaultPresetKnobValues[Vocoder][9] = 0.0f;
            
            vcd->defaultPresetKnobValues[VocoderCh][0] = 0.4f; // volume
            vcd->defaultPresetKnobValues[VocoderCh][1] = 0.5f; // warp factorvcd->
            vcd->defaultPresetKnobValues[VocoderCh][2] = 1.0f; // quality
            vcd->defaultPresetKnobValues[VocoderCh][3] = 0.5f; //band width
            vcd->defaultPresetKnobValues[VocoderCh][4] = 0.2f; //noise thresh
            vcd->defaultPresetKnobValues[VocoderCh][5] = 0.0f;// saw->pulse fade
            vcd->defaultPresetKnobValues[VocoderCh][6] = 0.5f; // pulse length
            vcd->defaultPresetKnobValues[VocoderCh][7] = 0.5f; // pulse width
            vcd->defaultPresetKnobValues[VocoderCh][8] = 0.0f; // breathiness
            vcd->defaultPresetKnobValues[VocoderCh][9] = 0.66f; // envelope speed
            vcd->defaultPresetKnobValues[VocoderCh][10] = 0.5f;// squish
            vcd->defaultPresetKnobValues[VocoderCh][11] = 0.5f; // offset
            vcd->defaultPresetKnobValues[VocoderCh][12] = 0.5f; // tilt
            vcd->defaultPresetKnobValues[VocoderCh][13] = 0.0f; // stereo
            vcd->defaultPresetKnobValues[VocoderCh][14] = 0.0f; // barkpull
            
            vcd->defaultPresetKnobValues[Pitchshift][0] = 0.5f; // pitch
            vcd->defaultPresetKnobValues[Pitchshift][1] = 0.5f; // fine pitch
            vcd->defaultPresetKnobValues[Pitchshift][2] = 0.0f; // f amount
            vcd->defaultPresetKnobValues[Pitchshift][3] = 0.5f; // formant
            vcd->defaultPresetKnobValues[Pitchshift][4] = 0.5f; //range
            vcd->defaultPresetKnobValues[Pitchshift][5] = 0.25f; //offset
            vcd->defaultPresetKnobValues[Pitchshift][6] = 0.25f;
            vcd->defaultPresetKnobValues[Pitchshift][7] = 0.25f;
            vcd->defaultPresetKnobValues[Pitchshift][8] = 0.25f;
            vcd->defaultPresetKnobValues[Pitchshift][9] = 0.25f;
            
            vcd->defaultPresetKnobValues[AutotuneMono][0] = 0.4f; // pickiness
            vcd->defaultPresetKnobValues[AutotuneMono][1] = 1.0f; // amount
            vcd->defaultPresetKnobValues[AutotuneMono][2] = 0.5f; // speed
            vcd->defaultPresetKnobValues[AutotuneMono][3] = 1.0f; // leap allow
            vcd->defaultPresetKnobValues[AutotuneMono][4] = 0.25f; // hysteresis
            
            vcd->defaultPresetKnobValues[AutotunePoly][0] = 0.6f; // periodicity thresh
            vcd->defaultPresetKnobValues[AutotunePoly][1] = 0.5f;
            vcd->defaultPresetKnobValues[AutotunePoly][2] = 0.1f;
            vcd->defaultPresetKnobValues[AutotunePoly][3] = 0.0f;
            vcd->defaultPresetKnobValues[AutotunePoly][4] = 0.0f;
            
            vcd->defaultPresetKnobValues[SamplerButtonPress][0] = 0.0f; // start
            vcd->defaultPresetKnobValues[SamplerButtonPress][1] = 1.0f; // end
            vcd->defaultPresetKnobValues[SamplerButtonPress][2] = 0.75f; // speed
            vcd->defaultPresetKnobValues[SamplerButtonPress][3] = 0.5f; // speed mult
            vcd->defaultPresetKnobValues[SamplerButtonPress][4] = 0.4f;//crossfade
            
            vcd->defaultPresetKnobValues[SamplerKeyboard][0] = 0.0f; // start
            vcd->defaultPresetKnobValues[SamplerKeyboard][1] = 1.0f; // end
            vcd->defaultPresetKnobValues[SamplerKeyboard][2] = 0.75f; // speed
            vcd->defaultPresetKnobValues[SamplerKeyboard][3] = 0.5f; // speed mult
            vcd->defaultPresetKnobValues[SamplerKeyboard][4] = 0.0f; //looping on
            vcd->defaultPresetKnobValues[SamplerKeyboard][5] = 0.4f;//crossfade
            vcd->defaultPresetKnobValues[SamplerKeyboard][6] = 0.0f;//velocity sensitivity
            
            vcd->defaultPresetKnobValues[SamplerAutoGrab][0] = 0.95f; // thresh
            vcd->defaultPresetKnobValues[SamplerAutoGrab][1] = 0.5f; // window
            vcd->defaultPresetKnobValues[SamplerAutoGrab][2] = 0.75f; // speed
            vcd->defaultPresetKnobValues[SamplerAutoGrab][3] = 0.25f; // crossfade
            vcd->defaultPresetKnobValues[SamplerAutoGrab][4] = 0.0f;
            vcd->defaultPresetKnobValues[SamplerAutoGrab][5] = 0.0f; // len rand
            vcd->defaultPresetKnobValues[SamplerAutoGrab][6] = 0.0f; // speed rand
            vcd->defaultPresetKnobValues[SamplerAutoGrab][7] = 0.0f;
            vcd->defaultPresetKnobValues[SamplerAutoGrab][8] = 0.0f;
            vcd->defaultPresetKnobValues[SamplerAutoGrab][9] = 0.0f;
            
            vcd->defaultPresetKnobValues[Distortion][0] = .25f; // pre gain
            vcd->defaultPresetKnobValues[Distortion][1] = 0.5f; // tilt (low and high shelves, opposing gains)
            vcd->defaultPresetKnobValues[Distortion][2] = 0.5f; // mid gain
            vcd->defaultPresetKnobValues[Distortion][3] = 0.5f; // mid freq
            vcd->defaultPresetKnobValues[Distortion][4] = 0.25f; //post gain
            
            vcd->defaultPresetKnobValues[Wavefolder][0] = 0.4f; // gain
            vcd->defaultPresetKnobValues[Wavefolder][1] = 0.5f; // offset1
            vcd->defaultPresetKnobValues[Wavefolder][2] = 0.5f; // offset2
            vcd->defaultPresetKnobValues[Wavefolder][3] = 0.75f; // post gain
            vcd->defaultPresetKnobValues[Wavefolder][4] = 0.0f;
            
            vcd->defaultPresetKnobValues[BitCrusher][0] = 0.1f; // quality
            vcd->defaultPresetKnobValues[BitCrusher][1] = 0.5f; // samp ratio
            vcd->defaultPresetKnobValues[BitCrusher][2] = 0.0f; // rounding
            vcd->defaultPresetKnobValues[BitCrusher][3] = 0.0f; // operation
            vcd->defaultPresetKnobValues[BitCrusher][4] = 0.5f; // post gain
            vcd->defaultPresetKnobValues[BitCrusher][5] = 0.0f; // pre gain
            
            vcd->defaultPresetKnobValues[Delay][0] = 0.25f; // delayL
            vcd->defaultPresetKnobValues[Delay][1] = 0.25f; // delayR
            vcd->defaultPresetKnobValues[Delay][2] = 0.0f; // highpass
            vcd->defaultPresetKnobValues[Delay][3] = 1.0f; // lowpass
            vcd->defaultPresetKnobValues[Delay][4] = 0.5f; // feedback
            vcd->defaultPresetKnobValues[Delay][5] = 1.0f; // post gain
            
            vcd->defaultPresetKnobValues[Reverb][0] = 0.5f; // size
            vcd->defaultPresetKnobValues[Reverb][1] = 0.5f; // in lowpass
            vcd->defaultPresetKnobValues[Reverb][2] = 0.5f; // in highpass
            vcd->defaultPresetKnobValues[Reverb][3] = 0.5f; // fb lowpass
            vcd->defaultPresetKnobValues[Reverb][4] = 0.5f; // fb gain
            
            vcd->defaultPresetKnobValues[Reverb2][0] = 0.2f; // size
            vcd->defaultPresetKnobValues[Reverb2][1] = 0.5f; // lowpass
            vcd->defaultPresetKnobValues[Reverb2][2] = 0.5f; // highpass
            vcd->defaultPresetKnobValues[Reverb2][3] = 0.5f; // peak freq
            vcd->defaultPresetKnobValues[Reverb2][4] = 0.5f; // peak gain
            
            vcd->defaultPresetKnobValues[LivingString][0] = 0.3f; // freq 1
            vcd->defaultPresetKnobValues[LivingString][1] = 0.0f; // detune
            vcd->defaultPresetKnobValues[LivingString][2] = 0.3f; // decay
            vcd->defaultPresetKnobValues[LivingString][3] = 0.9f; // damping
            vcd->defaultPresetKnobValues[LivingString][4] = 0.5f; // pick pos
            vcd->defaultPresetKnobValues[LivingString][5] = 0.25f; // prep pos
            vcd->defaultPresetKnobValues[LivingString][6] = 0.0f; // prep index
            vcd->defaultPresetKnobValues[LivingString][7] = 0.0f; // let ring
            vcd->defaultPresetKnobValues[LivingString][8] = 0.8f;
            vcd->defaultPresetKnobValues[LivingString][9] = 0.5f;
            vcd->defaultPresetKnobValues[LivingString][10] = 0.3f;// freq 2
            vcd->defaultPresetKnobValues[LivingString][11] = 0.3f;// freq 3
            vcd->defaultPresetKnobValues[LivingString][12] = 0.3f;// freq 4
            vcd->defaultPresetKnobValues[LivingString][13] = 0.3f;// freq 5
            vcd->defaultPresetKnobValues[LivingString][14] = 0.3f;// freq 6
            
            vcd->defaultPresetKnobValues[LivingStringSynth][0] = 0.5f;
            vcd->defaultPresetKnobValues[LivingStringSynth][1] = 0.5f;
            vcd->defaultPresetKnobValues[LivingStringSynth][2] = 0.85f; // decay
            vcd->defaultPresetKnobValues[LivingStringSynth][3] = 0.5f; // brightness
            vcd->defaultPresetKnobValues[LivingStringSynth][4] = 0.4f; // pick pos
            vcd->defaultPresetKnobValues[LivingStringSynth][5] = 0.25f; // prep pos
            vcd->defaultPresetKnobValues[LivingStringSynth][6] = 0.0f; // prep index
            vcd->defaultPresetKnobValues[LivingStringSynth][7] = 0.0f; // let ring
            vcd->defaultPresetKnobValues[LivingStringSynth][8] = 0.3f; // feedback volume
            vcd->defaultPresetKnobValues[LivingStringSynth][9] = 0.4f; // release time
            vcd->defaultPresetKnobValues[LivingStringSynth][10] = 0.0f; // prep removal amount
            vcd->defaultPresetKnobValues[LivingStringSynth][11] = 0.0f; // pickup pos
            
            vcd->defaultPresetKnobValues[ClassicSynth][0] = 0.5f; // volume
            vcd->defaultPresetKnobValues[ClassicSynth][1] = 0.5f; // lowpass
            vcd->defaultPresetKnobValues[ClassicSynth][2] = 0.2f; // detune
            vcd->defaultPresetKnobValues[ClassicSynth][3] = 0.0f;
            vcd->defaultPresetKnobValues[ClassicSynth][4] = 0.0f;
            vcd->defaultPresetKnobValues[ClassicSynth][5] = 0.0f;
            vcd->defaultPresetKnobValues[ClassicSynth][6] = 0.06f;
            vcd->defaultPresetKnobValues[ClassicSynth][7] = 0.9f;
            vcd->defaultPresetKnobValues[ClassicSynth][8] = 0.1f;
            vcd->defaultPresetKnobValues[ClassicSynth][9] = 0.1f;
            vcd->defaultPresetKnobValues[ClassicSynth][10] = 0.0f;
            vcd->defaultPresetKnobValues[ClassicSynth][11] = 0.06f;
            vcd->defaultPresetKnobValues[ClassicSynth][12] = 0.9f;
            vcd->defaultPresetKnobValues[ClassicSynth][13] = 0.1f;
            vcd->defaultPresetKnobValues[ClassicSynth][14] = 0.1f;
            vcd->defaultPresetKnobValues[ClassicSynth][15] = 0.0f;
            vcd->defaultPresetKnobValues[ClassicSynth][16] = 0.06f;
            vcd->defaultPresetKnobValues[ClassicSynth][17] = 0.9f;
            vcd->defaultPresetKnobValues[ClassicSynth][18] = 0.1f;
            vcd->defaultPresetKnobValues[ClassicSynth][19] = 0.1f;
            
            vcd->defaultPresetKnobValues[Rhodes][0] = 0.25f;
            vcd->defaultPresetKnobValues[Rhodes][1] = 0.25f;
            vcd->defaultPresetKnobValues[Rhodes][2] = 0.25f;
            vcd->defaultPresetKnobValues[Rhodes][3] = 0.5f;
            vcd->defaultPresetKnobValues[Rhodes][4] = 0.0f; //stereo spread
            vcd->defaultPresetKnobValues[Rhodes][5] = 0.05f;
            vcd->defaultPresetKnobValues[Rhodes][6] = 0.05f;
            vcd->defaultPresetKnobValues[Rhodes][7] = 0.9f;
            vcd->defaultPresetKnobValues[Rhodes][8] = 0.1007f;
            vcd->defaultPresetKnobValues[Rhodes][9] = 0.5f;
            vcd->defaultPresetKnobValues[Rhodes][10] = 0.05f;
            vcd->defaultPresetKnobValues[Rhodes][11] = 0.05f;
            vcd->defaultPresetKnobValues[Rhodes][12] = 0.9f;
            vcd->defaultPresetKnobValues[Rhodes][13] = 0.1007f;
            vcd->defaultPresetKnobValues[Rhodes][14] = 0.5f;
            vcd->defaultPresetKnobValues[Rhodes][15] = 0.8f;
            vcd->defaultPresetKnobValues[Rhodes][16] = 0.6f;
            vcd->defaultPresetKnobValues[Rhodes][17] = 0.7f;
            vcd->defaultPresetKnobValues[Rhodes][18] = 0.5f;
            vcd->defaultPresetKnobValues[Rhodes][19] = 0.5f;
            vcd->defaultPresetKnobValues[Rhodes][20] = 0.5f;
            vcd->defaultPresetKnobValues[Rhodes][21] = 0.0f;
            vcd->defaultPresetKnobValues[Rhodes][22] = 0.00f;
            vcd->defaultPresetKnobValues[Rhodes][23] = 0.00f;
            vcd->defaultPresetKnobValues[Rhodes][24] = 0.00f;

            vcd->defaultPresetKnobValues[Tape][0] = 0.25f;
             vcd->defaultPresetKnobValues[Tape][1] = 0.25f;
             vcd->defaultPresetKnobValues[Tape][2] = 0.25f;
             vcd->defaultPresetKnobValues[Tape][3] = 0.5f;
             vcd->defaultPresetKnobValues[Tape][4] = 0.0f;


#ifdef __cplusplus
            vcd->defaultPresetKnobValues[WavetableSynth][0] = 0.5f; // gains
            vcd->defaultPresetKnobValues[WavetableSynth][1] = 0.5f;
            vcd->defaultPresetKnobValues[WavetableSynth][2] = 0.5f;
            vcd->defaultPresetKnobValues[WavetableSynth][3] = 0.5f;
            vcd->defaultPresetKnobValues[WavetableSynth][4] = 0.0f; // index
            vcd->defaultPresetKnobValues[WavetableSynth][5] = 0.0f; // attack
            vcd->defaultPresetKnobValues[WavetableSynth][6] = 0.06f; // decay
            vcd->defaultPresetKnobValues[WavetableSynth][7] = 0.9f; // sustain
            vcd->defaultPresetKnobValues[WavetableSynth][8] = 0.1f; // release
            vcd->defaultPresetKnobValues[WavetableSynth][9] = 0.1f; // leak
            vcd->defaultPresetKnobValues[WavetableSynth][10] = 0.0f; // phases
            vcd->defaultPresetKnobValues[WavetableSynth][11] = 0.0f;
            vcd->defaultPresetKnobValues[WavetableSynth][12] = 0.0f;
            vcd->defaultPresetKnobValues[WavetableSynth][13] = 0.0f;
#endif
            for (int p = 0; p < PresetNil; p++)
            {
                for (int v = 0; v < NUM_PRESET_KNOB_VALUES; v++)
                {
                    vcd->presetKnobValues[p][v] = vcd->defaultPresetKnobValues[p][v];
                }
            }
        }

        void initFunctionPointers(Vocodec* vcd)
        {
            vcd->allocFunctions[Vocoder] = SFXVocoderAlloc;
            vcd->frameFunctions[Vocoder] = SFXVocoderFrame;
            vcd->tickFunctions[Vocoder] = SFXVocoderTick;
            vcd->freeFunctions[Vocoder] = SFXVocoderFree;
            
            vcd->allocFunctions[VocoderCh] = SFXVocoderChAlloc;
            vcd->frameFunctions[VocoderCh] = SFXVocoderChFrame;
            vcd->tickFunctions[VocoderCh] = SFXVocoderChTick;
            vcd->freeFunctions[VocoderCh] = SFXVocoderChFree;
            
            vcd->allocFunctions[Pitchshift] = SFXPitchShiftAlloc;
            vcd->frameFunctions[Pitchshift] = SFXPitchShiftFrame;
            vcd->tickFunctions[Pitchshift] = SFXPitchShiftTick;
            vcd->freeFunctions[Pitchshift] = SFXPitchShiftFree;
            
            vcd->allocFunctions[AutotuneMono] = SFXNeartuneAlloc;
            vcd->frameFunctions[AutotuneMono] = SFXNeartuneFrame;
            vcd->tickFunctions[AutotuneMono] = SFXNeartuneTick;
            vcd->freeFunctions[AutotuneMono] = SFXNeartuneFree;
            
            vcd->allocFunctions[AutotunePoly] = SFXAutotuneAlloc;
            vcd->frameFunctions[AutotunePoly] = SFXAutotuneFrame;
            vcd->tickFunctions[AutotunePoly] = SFXAutotuneTick;
            vcd->freeFunctions[AutotunePoly] = SFXAutotuneFree;
            
            vcd->allocFunctions[SamplerButtonPress] = SFXSamplerBPAlloc;
            vcd->frameFunctions[SamplerButtonPress] = SFXSamplerBPFrame;
            vcd->tickFunctions[SamplerButtonPress] = SFXSamplerBPTick;
            vcd->freeFunctions[SamplerButtonPress] = SFXSamplerBPFree;
            
            vcd->allocFunctions[SamplerKeyboard] = SFXSamplerKAlloc;
            vcd->frameFunctions[SamplerKeyboard] = SFXSamplerKFrame;
            vcd->tickFunctions[SamplerKeyboard] = SFXSamplerKTick;
            vcd->freeFunctions[SamplerKeyboard] = SFXSamplerKFree;
            
            vcd->allocFunctions[SamplerAutoGrab] = SFXSamplerAutoAlloc;
            vcd->frameFunctions[SamplerAutoGrab] = SFXSamplerAutoFrame;
            vcd->tickFunctions[SamplerAutoGrab] = SFXSamplerAutoTick;
            vcd->freeFunctions[SamplerAutoGrab] = SFXSamplerAutoFree;
            
            vcd->allocFunctions[Distortion] = SFXDistortionAlloc;
            vcd->frameFunctions[Distortion] = SFXDistortionFrame;
            vcd->tickFunctions[Distortion] = SFXDistortionTick;
            vcd->freeFunctions[Distortion] = SFXDistortionFree;
            
            vcd->allocFunctions[Wavefolder] = SFXWaveFolderAlloc;
            vcd->frameFunctions[Wavefolder] = SFXWaveFolderFrame;
            vcd->tickFunctions[Wavefolder] = SFXWaveFolderTick;
            vcd->freeFunctions[Wavefolder] = SFXWaveFolderFree;
            
            vcd->allocFunctions[BitCrusher] = SFXBitcrusherAlloc;
            vcd->frameFunctions[BitCrusher] = SFXBitcrusherFrame;
            vcd->tickFunctions[BitCrusher] = SFXBitcrusherTick;
            vcd->freeFunctions[BitCrusher] = SFXBitcrusherFree;
            
            vcd->allocFunctions[Delay] = SFXDelayAlloc;
            vcd->frameFunctions[Delay] = SFXDelayFrame;
            vcd->tickFunctions[Delay] = SFXDelayTick;
            vcd->freeFunctions[Delay] = SFXDelayFree;
            
            vcd->allocFunctions[Reverb] = SFXReverbAlloc;
            vcd->frameFunctions[Reverb] = SFXReverbFrame;
            vcd->tickFunctions[Reverb] = SFXReverbTick;
            vcd->freeFunctions[Reverb] = SFXReverbFree;
            
            vcd->allocFunctions[Reverb2] = SFXReverb2Alloc;
            vcd->frameFunctions[Reverb2] = SFXReverb2Frame;
            vcd->tickFunctions[Reverb2] = SFXReverb2Tick;
            vcd->freeFunctions[Reverb2] = SFXReverb2Free;
            
            vcd->allocFunctions[LivingString] = SFXLivingStringAlloc;
            vcd->frameFunctions[LivingString] = SFXLivingStringFrame;
            vcd->tickFunctions[LivingString] = SFXLivingStringTick;
            vcd->freeFunctions[LivingString] = SFXLivingStringFree;
            
            vcd->allocFunctions[LivingStringSynth] = SFXLivingStringSynthAlloc;
            vcd->frameFunctions[LivingStringSynth] = SFXLivingStringSynthFrame;
            vcd->tickFunctions[LivingStringSynth] = SFXLivingStringSynthTick;
            vcd->freeFunctions[LivingStringSynth] = SFXLivingStringSynthFree;
            
            vcd->allocFunctions[ClassicSynth] = SFXClassicSynthAlloc;
            vcd->frameFunctions[ClassicSynth] = SFXClassicSynthFrame;
            vcd->tickFunctions[ClassicSynth] = SFXClassicSynthTick;
            vcd->freeFunctions[ClassicSynth] = SFXClassicSynthFree;
            
            vcd->allocFunctions[Rhodes] = SFXRhodesAlloc;
            vcd->frameFunctions[Rhodes] = SFXRhodesFrame;
            vcd->tickFunctions[Rhodes] = SFXRhodesTick;
            vcd->freeFunctions[Rhodes] = SFXRhodesFree;

            vcd->allocFunctions[Tape] = SFXTapeAlloc;
            vcd->frameFunctions[Tape] = SFXTapeFrame;
            vcd->tickFunctions[Tape] = SFXTapeTick;
            vcd->freeFunctions[Tape] = SFXTapeFree;

#ifdef __cplusplus
            vcd->allocFunctions[WavetableSynth] = SFXWavetableSynthAlloc;
            vcd->frameFunctions[WavetableSynth] = SFXWavetableSynthFrame;
            vcd->tickFunctions[WavetableSynth] = SFXWavetableSynthTick;
            vcd->freeFunctions[WavetableSynth] = SFXWavetableSynthFree;
#endif
        }
        
        void initGlobalSFXObjects(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            calculateNoteArray(vcd);

            tSimplePoly_init(&vcd->poly, NUM_VOC_VOICES, &vcd->leaf);
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                tExpSmooth_init(&vcd->polyRamp[i], 0.0f, 0.02f, &vcd->leaf);
                vcd->freq[i] = 220.0f;
            }

            tExpSmooth_init(&vcd->comp, 1.0f, 0.01f, &vcd->leaf);

            LEAF_generate_exp(vcd->expBuffer, 1000.0f, -1.0f, 0.0f, -0.0008f, EXP_BUFFER_SIZE); //exponential buffer rising from 0 to 1
            LEAF_generate_exp(vcd->decayExpBuffer, 0.001f, 0.0f, 1.0f, -0.0008f, DECAY_EXP_BUFFER_SIZE); // exponential decay buffer falling from 1 to
            vcd->leaf.clearOnAllocation = 0;
        }

        void freeGlobalSFXObjects(Vocodec* vcd)
        {
            tSimplePoly_free(&vcd->poly);
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                tExpSmooth_free(&vcd->polyRamp[i]);
            }
            tExpSmooth_free(&vcd->comp);
        }
        
        ///1 vocoder internal pol
        void SFXVocoderAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            tTalkboxLfloat_init(&vcd->vocoder, 1024, &vcd->leaf);
            tTalkboxLfloat_setWarpOn(vcd->vocoder, 1);
            tNoise_init(&vcd->vocoderNoise, WhiteNoise, &vcd->leaf);
            tZeroCrossingCounter_init(&vcd->zerox, 16, &vcd->leaf);
            tSimplePoly_setNumVoices(vcd->poly, vcd->vocoderParams.numVoices);
            tExpSmooth_init(&vcd->noiseRamp, 0.0f, 0.005f, &vcd->leaf);

            //tilt filter
            tVZFilter_init(&vcd->shelf1, Lowshelf, 80.0f, 6.0f, &vcd->leaf);
            tVZFilter_init(&vcd->shelf2, Highshelf, 12000.0f, 6.0f, &vcd->leaf);

            tNoise_init(&vcd->breathNoise, WhiteNoise, &vcd->leaf);
            tHighpass_init(&vcd->noiseHP, 4500.0f, &vcd->leaf);

            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {

                tSawtooth_init(&vcd->osc[i], &vcd->leaf);

                tRosenbergGlottalPulse_init(&vcd->glottal[i], &vcd->leaf);
                tRosenbergGlottalPulse_setOpenLengthAndPulseLength(vcd->glottal[i], 0.3f, 0.4f);
            }
            setLED_A(vcd, vcd->vocoderParams.numVoices == 1);
            setLED_B(vcd, vcd->vocoderParams.internalExternal);
            setLED_C(vcd, vcd->vocoderParams.freeze);
            vcd->leaf.clearOnAllocation = 0;
        }
        
        volatile  uint32_t vcount1F = 0;
        volatile  uint32_t vcount1M = 0;
        void SFXVocoderFrame(Vocodec* vcd)
        {
        	uint32_t vcount1 = DWT->CYCCNT;

        	if (vcd->buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                vcd->vocoderParams.numVoices = (vcd->vocoderParams.numVoices > 1) ? 1 : NUM_VOC_VOICES;
                tSimplePoly_setNumVoices(vcd->poly, vcd->vocoderParams.numVoices);
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->vocoderParams.numVoices == 1);
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                vcd->vocoderParams.internalExternal = !vcd->vocoderParams.internalExternal;
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(vcd, vcd->vocoderParams.internalExternal);
            }
            if (vcd->buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                vcd->vocoderParams.freeze = !vcd->vocoderParams.freeze;
                tTalkboxLfloat_setFreeze(vcd->vocoder, vcd->vocoderParams.freeze);
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vcd, vcd->vocoderParams.freeze);
            }
            
            vcd->displayValues[0] = vcd->presetKnobValues[Vocoder][0]; //vocoder volume
            vcd->displayValues[1] = (vcd->presetKnobValues[Vocoder][1] * 0.4f) - 0.2f; //warp factor
            vcd->displayValues[2] = vcd->presetKnobValues[Vocoder][2] * 1.1f; //quality
            vcd->displayValues[3] = vcd->presetKnobValues[Vocoder][3]; //crossfade between sawtooth and glottal pulse
            vcd->displayValues[4] = vcd->presetKnobValues[Vocoder][4]; //noise thresh
            vcd->displayValues[5] = vcd->presetKnobValues[Vocoder][5]; //breathy
            vcd->displayValues[6] = (vcd->presetKnobValues[Vocoder][6] * 30.0f) - 15.0f;; //tilt filter
            vcd->displayValues[7] = vcd->presetKnobValues[Vocoder][7]; //pulse length
            vcd->displayValues[8] = vcd->presetKnobValues[Vocoder][8]; //open length
            
            tTalkboxLfloat_setWarpFactor( vcd->vocoder, vcd->displayValues[1]);
            tTalkboxLfloat_setQuality( vcd->vocoder, vcd->displayValues[2]);
            
            for (int i = 0; i < tSimplePoly_getNumVoices(vcd->poly); i++)
            {
                tExpSmooth_setDest(vcd->polyRamp[i], (tSimplePoly_getVelocity(vcd->poly, i) > 0));
                calculateFreq(vcd, i);
                tSawtooth_setFreq(vcd->osc[i], vcd->freq[i]);
                tRosenbergGlottalPulse_setFreq(vcd->glottal[i], vcd->freq[i]);
                tRosenbergGlottalPulse_setOpenLengthAndPulseLength(vcd->glottal[i], vcd->displayValues[8] * vcd->displayValues[7], vcd->displayValues[7]);
            }
            
            if (tSimplePoly_getNumActiveVoices(vcd->poly) != 0)
            {
                tExpSmooth_setDest(vcd->comp, sqrtf(1.0f / (float)tSimplePoly_getNumActiveVoices(vcd->poly)));
            }
            else
            {
                tExpSmooth_setDest(vcd->comp, 0.0f);
            }
            
            tVZFilter_setGain(vcd->shelf1, fasterdbtoa(-1.0f * vcd->displayValues[6]));
            tVZFilter_setGain(vcd->shelf2, fastdbtoa(vcd->displayValues[6]));
        	vcount1F = DWT->CYCCNT-vcount1;
        	if (vcount1F > vcount1M)
        	{
        		vcount1M = vcount1F;
        	}

        }
        
        volatile uint32_t vcount2F = 0;
        volatile uint32_t vcount2M = 0;

        volatile uint32_t vcount3F = 0;
        volatile uint32_t vcount3M = 0;

        volatile uint32_t vcount4F = 0;
        volatile uint32_t vcount4M = 0;
        volatile uint32_t vcount5F = 0;
        volatile uint32_t vcount5M = 0;

        void SFXVocoderTick(Vocodec* vcd, float* input)
        {
            
            float zerocross = 0.0f;
            float noiseRampVal = 0.0f;
            float sample = 0.0f;
            
            if (vcd->vocoderParams.internalExternal == 1)
            {
                sample = input[0];
            }
            else
            {
            	uint32_t vcount2 = DWT->CYCCNT;

                zerocross = tZeroCrossingCounter_tick(vcd->zerox, input[1]);

                if (!vcd->vocoderParams.freeze)
                {
                    tExpSmooth_setDest(vcd->noiseRamp, zerocross > ((vcd->displayValues[4])-0.1f));
                }
                noiseRampVal = tExpSmooth_tick(vcd->noiseRamp);
                
                float noiseSample = tNoise_tick(vcd->vocoderNoise) * noiseRampVal * 0.6f;
                
                
                for (int i = 0; i < tSimplePoly_getNumVoices(vcd->poly); i++)
                {
                    sample += tSawtooth_tick(vcd->osc[i]) *
                    tExpSmooth_tick(vcd->polyRamp[i]) * (1.0f - vcd->displayValues[3]);
                    sample += tRosenbergGlottalPulse_tickHQ(vcd->glottal[i]) * tExpSmooth_tick(vcd->polyRamp[i]) * 1.9f * vcd->displayValues[3];
                }
                
                
                //switch with consonant noise
                sample = (sample * (1.0f - (0.3f * vcd->displayValues[5])) * (1.0f-noiseRampVal)) + noiseSample;
                //add breathiness
                sample += (tHighpass_tick(vcd->noiseHP, tNoise_tick(vcd->breathNoise)) *
                           vcd->displayValues[5] * 1.5f);
                sample *= tExpSmooth_tick(vcd->comp);

            	vcount2F = DWT->CYCCNT-vcount2;
            	if (vcount2F > vcount2M)
            	{
            		vcount2M = vcount2F;
            	}
            }

        	uint32_t vcount3 = DWT->CYCCNT;
            sample = tanhf(sample);
        	vcount3F = DWT->CYCCNT-vcount3;
        	if (vcount3F > vcount3M)
        	{
        		vcount3M = vcount3F;
        	}
        	uint32_t vcount4 = DWT->CYCCNT;
            sample = tTalkboxLfloat_tick(vcd->vocoder, sample, input[1]);

        	vcount4F = DWT->CYCCNT-vcount4;
        	if (vcount4F > vcount4M)
        	{
        		vcount4M = vcount4F;
        	}
        	uint32_t vcount5 = DWT->CYCCNT;
            sample = tVZFilter_tickEfficient(vcd->shelf1, sample); //put it through the low shelf
            sample = tVZFilter_tickEfficient(vcd->shelf2, sample); // now put that result through the high shelf
            sample *= vcd->displayValues[0] * 0.6f;
            sample = tanhf(sample);
            input[0] = sample;
            input[1] = sample;
            vcount5F = DWT->CYCCNT-vcount5;
			if (vcount5F > vcount5M)
			{
				vcount5M = vcount5F;
			}
        }
        
        void SFXVocoderFree(Vocodec* vcd)
        {
        	tTalkboxLfloat_free(&vcd->vocoder);
            tNoise_free(&vcd->vocoderNoise);
            tZeroCrossingCounter_free(&vcd->zerox);
            tExpSmooth_free(&vcd->noiseRamp);
            
            tNoise_free(&vcd->breathNoise);
            tHighpass_free(&vcd->noiseHP);
            
            tVZFilter_free(&vcd->shelf1);
            tVZFilter_free(&vcd->shelf2);
            
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                tSawtooth_free(&vcd->osc[i]);
                tRosenbergGlottalPulse_free(&vcd->glottal[i]);
            }
        }
        
        
        
        void SFXVocoderChAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            vcd->displayValues[0] = vcd->presetKnobValues[VocoderCh][0]; //vocoder volume
            vcd->displayValues[1] = (vcd->presetKnobValues[VocoderCh][1] * 0.8f) - 0.4f; //warp factor
            vcd->displayValues[2] = (uint8_t)(vcd->presetKnobValues[VocoderCh][2] * 16.9f) + 8.0f; //quality
            vcd->displayValues[3] = (vcd->presetKnobValues[VocoderCh][3]* 2.0f) + 0.1f; //band width
            vcd->displayValues[4] = vcd->presetKnobValues[VocoderCh][4]; //noise thresh
            vcd->displayValues[5] = vcd->presetKnobValues[VocoderCh][5]; //crossfade between saw & glottal pulse
            vcd->displayValues[6] = vcd->presetKnobValues[VocoderCh][6]; //pulse width
            vcd->displayValues[7] = vcd->presetKnobValues[VocoderCh][7]; //pulse shape
            vcd->displayValues[8] = vcd->presetKnobValues[VocoderCh][8]; //breathiness
            vcd->displayValues[9] = vcd->presetKnobValues[VocoderCh][9]; //speed
            vcd->displayValues[10] = vcd->presetKnobValues[VocoderCh][10] * 2.0f; //bandsquish
            vcd->displayValues[11] = vcd->presetKnobValues[VocoderCh][11] * 60.0f; //bandoffset
            vcd->displayValues[12] = (vcd->presetKnobValues[VocoderCh][12] * 2.0f) - 1.0f; //tilt
            vcd->displayValues[13] = vcd->presetKnobValues[VocoderCh][13]; //stereo
            vcd->displayValues[14] = vcd->presetKnobValues[VocoderCh][14]; //odd/even
            
            float myQ = vcd->displayValues[3];
            
            vcd->invNumberOfVocoderBands = 1.0f / ((float)vcd->numberOfVocoderBands-0.99f);
            vcd->bandWidthInSemitones = 99.0f * vcd->invNumberOfVocoderBands;
            vcd->bandWidthInOctaves = vcd->bandWidthInSemitones * 0.083333333333333f;  // divide by 12
            vcd->thisBandwidth = vcd->bandWidthInOctaves * myQ;
            
            tVZFilter_init(&vcd->vocodec_highshelf, Highshelf, 6000.0f, 3.0f, &vcd->leaf);
            tVZFilter_setGain(vcd->vocodec_highshelf, 4.0f);
            
            for (int i = 0; i < MAX_NUM_VOCODER_BANDS; i++)
            {
                
                float bandFreq = faster_mtof(((float)i * vcd->bandWidthInSemitones) + 30.0f); //midinote 28 (41Hz) to midinote 134 (18814Hz) is 106 midinotes, divide that by how many bands to find out how far apart to put the bands
                
                vcd->bandGains[i] = 1.0f;
                
                if (i == 0)
                {
                    tVZFilter_init(&vcd->analysisBands[i][0], Lowpass, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);
                    tVZFilter_init(&vcd->analysisBands[i][1], Lowpass, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);

                    tVZFilter_init(&vcd->synthesisBands[i][0], Lowpass, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);
                    tVZFilter_init(&vcd->synthesisBands[i][1], Lowpass, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);

                }
                else if (i == (MAX_NUM_VOCODER_BANDS-1))
                {
                    tVZFilter_init(&vcd->analysisBands[i][0], Highpass, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);
                    tVZFilter_init(&vcd->analysisBands[i][1], Highpass, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);

                    tVZFilter_init(&vcd->synthesisBands[i][0], Highpass, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);
                    tVZFilter_init(&vcd->synthesisBands[i][1], Highpass, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);

                }
                else
                {
                    tVZFilter_init(&vcd->analysisBands[i][0], BandpassPeak, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);
                    tVZFilter_init(&vcd->analysisBands[i][1], BandpassPeak, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);

                    tVZFilter_init(&vcd->synthesisBands[i][0], BandpassPeak, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);
                    tVZFilter_init(&vcd->synthesisBands[i][1], BandpassPeak, bandFreq,
                                   vcd->thisBandwidth, &vcd->leaf);

                }
                tExpSmooth_init(&vcd->envFollowers[i], 0.0f, 0.001f, &vcd->leaf); // factor of .001 is 10 ms?
            }
            tNoise_init(&vcd->breathNoise, WhiteNoise, &vcd->leaf);
            tNoise_init(&vcd->vocoderNoise, WhiteNoise, &vcd->leaf);
            tZeroCrossingCounter_init(&vcd->zerox, 256, &vcd->leaf);
            tSimplePoly_setNumVoices(vcd->poly, vcd->vocoderChParams.numVoices);
            tExpSmooth_init(&vcd->noiseRamp, 0.0f, 0.05f, &vcd->leaf);
            tHighpass_init(&vcd->noiseHP, 5000.0f, &vcd->leaf);
            tHighpass_init(&vcd->chVocFinalHP1, 20.0f, &vcd->leaf);
            tHighpass_init(&vcd->chVocFinalHP2, 20.0f, &vcd->leaf);
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                tSawtooth_init(&vcd->osc[i], &vcd->leaf);

                tRosenbergGlottalPulse_init(&vcd->glottal[i], &vcd->leaf);
                tRosenbergGlottalPulse_setOpenLength(vcd->glottal[i], 0.3f);
                tRosenbergGlottalPulse_setPulseLength(vcd->glottal[i], 0.4f);
            }
            setLED_A(vcd, vcd->vocoderChParams.numVoices == 1);
            setLED_B(vcd, vcd->vocoderChParams.internalExternal);
            setLED_C(vcd, vcd->vocoderChParams.freeze);
        }
        
        void SFXVocoderChFrame(Vocodec* vcd)
        {
            
            if (vcd->buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                vcd->vocoderChParams.numVoices = (vcd->vocoderChParams.numVoices > 1) ? 1 : NUM_VOC_VOICES;
                tSimplePoly_setNumVoices(vcd->poly, vcd->vocoderChParams.numVoices);
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->vocoderChParams.numVoices == 1);
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                vcd->vocoderChParams.internalExternal = !vcd->vocoderChParams.internalExternal;
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(vcd, vcd->vocoderChParams.internalExternal);
            }
            
            if (vcd->buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                vcd->vocoderChParams.freeze = !vcd->vocoderChParams.freeze;
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vcd, vcd->vocoderChParams.freeze);
            }

            vcd->displayValues[0] = vcd->presetKnobValues[VocoderCh][0]; //vocoder volume
            vcd->displayValues[1] = (vcd->presetKnobValues[VocoderCh][1] * 0.8f) - 0.4f; //warp factor
            vcd->displayValues[2] = (uint8_t)(vcd->presetKnobValues[VocoderCh][2] * 16.9f) + 8.0f; //quality
            vcd->displayValues[3] = (vcd->presetKnobValues[VocoderCh][3]* 2.0f) + 0.1f; //band width
            vcd->displayValues[4] = vcd->presetKnobValues[VocoderCh][4]; //noise thresh
            vcd->displayValues[5] = vcd->presetKnobValues[VocoderCh][5]; //crossfade between saw & glottal pulse
            vcd->displayValues[6] = vcd->presetKnobValues[VocoderCh][6]; //pulse width
            vcd->displayValues[7] = vcd->presetKnobValues[VocoderCh][7]; //pulse shape
            vcd->displayValues[8] = vcd->presetKnobValues[VocoderCh][8]; //breathiness
            vcd->displayValues[9] = vcd->presetKnobValues[VocoderCh][9]; //speed
            vcd->displayValues[10] = vcd->presetKnobValues[VocoderCh][10] + 0.5f; //bandsquish
            vcd->displayValues[11] = vcd->presetKnobValues[VocoderCh][11] * 60.0f; //bandoffset
            vcd->displayValues[12] = (vcd->presetKnobValues[VocoderCh][12] * 4.0f) - 2.0f; //tilt
            vcd->displayValues[13] = vcd->presetKnobValues[VocoderCh][13]; //stereo
            vcd->displayValues[14] = vcd->presetKnobValues[VocoderCh][14]; //snap to bark scale
            
            vcd->oneMinusStereo = 1.0f - vcd->displayValues[13];
            vcd->chVocOutputGain = 9.0f * vcd->displayValues[0];
            
            for (int i = 0; i < tSimplePoly_getNumVoices(vcd->poly); i++)
            {
                tExpSmooth_setDest(vcd->polyRamp[i], (tSimplePoly_getVelocity(vcd->poly, i) > 0));
                calculateFreq(vcd, i);
                tSawtooth_setFreq(vcd->osc[i], vcd->freq[i]);
                tRosenbergGlottalPulse_setFreq(vcd->glottal[i], vcd->freq[i]);
                tRosenbergGlottalPulse_setOpenLengthAndPulseLength(vcd->glottal[i], vcd->displayValues[6] *
                                                                   vcd->displayValues[7], vcd->displayValues[6]);
            }
            
            float warpFactor = 1.0f + vcd->displayValues[1];
            vcd->numberOfVocoderBands = vcd->displayValues[2];
            float myQ = vcd->displayValues[3];
            float bandSquish = vcd->displayValues[10];
            float bandOffset = vcd->displayValues[11];
            float myTilt = vcd->displayValues[12];
            float barkPull = vcd->displayValues[14];
            float oneMinusBarkPull = 1.0f - barkPull;
            
            
            if ((vcd->numberOfVocoderBands != vcd->prevNumberOfVocoderBands) || (myQ != vcd->prevMyQ) || (warpFactor != vcd->prevWarpFactor) || (bandSquish != vcd->prevBandSquish) || (bandOffset != vcd->prevBandOffset) || (myTilt != vcd->prevMyTilt) || (barkPull != vcd->prevBarkPull))
            {
                vcd->alteringBands = 1;
                vcd->invNumberOfVocoderBands = 1.0f / ((float)vcd->numberOfVocoderBands-0.99f);
                vcd->bandWidthInSemitones = 94.0f * bandSquish * vcd->invNumberOfVocoderBands; //was 90
                vcd->bandWidthInOctaves = vcd->bandWidthInSemitones * 0.083333333333333f;  // divide by 12
                vcd->thisBandwidth = vcd->bandWidthInOctaves * myQ;
                vcd->invMyQ = 1.0f / myQ;
            }
#ifndef __cplusplus
            if (vcd->alteringBands)
            {
                
                float tempWarpFactor = warpFactor;
                float bandFreq = faster_mtof(((float)vcd->currentBandToAlter * vcd->bandWidthInSemitones) + bandOffset); //midinote 28 (41Hz) to midinote 134 (18814Hz) is 106 midinotes, divide that by how many bands to find out how far apart to put the bands
                
                //warp to bark scale if knob 16 is up
                bandFreq = (bandFreq * oneMinusBarkPull) + (vcd->barkBandFreqs[vcd->currentBandToAlter] * barkPull);
                
                if (bandFreq > 5000.0f) // a way to keep the upper bands fixed so consonants are not stretched even though vowels are
                {
                    tempWarpFactor = 1.0f;
                }
                
                if (bandFreq > 16000.0f)
                {
                    bandFreq = 16000.0f;
                }
                
                float bandBandwidth = (vcd->thisBandwidth * oneMinusBarkPull) + (vcd->barkBandWidths[vcd->currentBandToAlter] *  barkPull * myQ);
                float myHeight = (float)vcd->currentBandToAlter * vcd->invNumberOfVocoderBands; //x value
                float tiltOffset = (1.0f - ((myTilt * 0.5f) + 0.5f)) + 0.5f;
                float tiltY = vcd->displayValues[12] * myHeight + tiltOffset;
                vcd->bandGains[vcd->currentBandToAlter] = vcd->invMyQ * tiltY;
                
                if (vcd->analysisOrSynthesis == 0)
                {
                    tVZFilter_setFreqAndBandwidth(vcd->analysisBands[vcd->currentBandToAlter][0], bandFreq, bandBandwidth);
                    //set these to match without computing for increased efficiency
                    vcd->analysisBands[vcd->currentBandToAlter][1]->B =
                    vcd->analysisBands[vcd->currentBandToAlter][0]->B;

                    vcd->analysisBands[vcd->currentBandToAlter][1]->fc =
                    vcd->analysisBands[vcd->currentBandToAlter][0]->fc;

                    vcd->analysisBands[vcd->currentBandToAlter][1]->R2 =
                    vcd->analysisBands[vcd->currentBandToAlter][0]->R2;

                    vcd->analysisBands[vcd->currentBandToAlter][1]->cL =
                    vcd->analysisBands[vcd->currentBandToAlter][0]->cL;

                    vcd->analysisBands[vcd->currentBandToAlter][1]->cB =
                    vcd->analysisBands[vcd->currentBandToAlter][0]->cB;

                    vcd->analysisBands[vcd->currentBandToAlter][1]->cH =
                    vcd->analysisBands[vcd->currentBandToAlter][0]->cH;

                    vcd->analysisBands[vcd->currentBandToAlter][1]->h =
                    vcd->analysisBands[vcd->currentBandToAlter][0]->h;

                    vcd->analysisBands[vcd->currentBandToAlter][1]->g =
                    vcd->analysisBands[vcd->currentBandToAlter][0]->g;

                    vcd->analysisOrSynthesis++;
                }
                else
                {
                    tVZFilter_setFreqAndBandwidth(vcd->synthesisBands[vcd->currentBandToAlter][0], bandFreq * tempWarpFactor, bandBandwidth);
                    //set these to match without computing for increased efficiency
                    vcd->synthesisBands[vcd->currentBandToAlter][1]->B =
                    vcd->synthesisBands[vcd->currentBandToAlter][0]->B;

                    vcd->synthesisBands[vcd->currentBandToAlter][1]->fc =
                    vcd->synthesisBands[vcd->currentBandToAlter][0]->fc;

                    vcd->synthesisBands[vcd->currentBandToAlter][1]->R2 =
                    vcd->synthesisBands[vcd->currentBandToAlter][0]->R2;

                    vcd->synthesisBands[vcd->currentBandToAlter][1]->cL =
                    vcd->synthesisBands[vcd->currentBandToAlter][0]->cL;

                    vcd->synthesisBands[vcd->currentBandToAlter][1]->cB =
                    vcd->synthesisBands[vcd->currentBandToAlter][0]->cB;

                    vcd->synthesisBands[vcd->currentBandToAlter][1]->cH =
                    vcd->synthesisBands[vcd->currentBandToAlter][0]->cH;

                    vcd->synthesisBands[vcd->currentBandToAlter][1]->h =
                    vcd->synthesisBands[vcd->currentBandToAlter][0]->h;

                    vcd->synthesisBands[vcd->currentBandToAlter][1]->g =
                    vcd->synthesisBands[vcd->currentBandToAlter][0]->g;

                    vcd->currentBandToAlter++;
                    vcd->analysisOrSynthesis = 0;
                }
                
                
                if ((vcd->currentBandToAlter >= vcd->numberOfVocoderBands) && (vcd->analysisOrSynthesis == 0))
                {
                    vcd->alteringBands = 0;
                    vcd->currentBandToAlter = 0;
                }
            }
#else
            if (vcd->alteringBands)
            {

                float tempWarpFactor = warpFactor;
                for (int i = 0; i < vcd->numberOfVocoderBands; i++)
                {
                    float bandFreq = faster_mtof(((float)i * vcd->bandWidthInSemitones) + bandOffset); //midinote 28 (41Hz) to midinote 134 (18814Hz) is 106 midinotes, divide that by how many bands to find out how far apart to put the bands

                    //warp to bark scale if knob 16 is up
                    bandFreq = (bandFreq * oneMinusBarkPull) + (vcd->barkBandFreqs[i] * barkPull);

                    if (bandFreq > 5000.0f) // a way to keep the upper bands fixed so consonants are not stretched even though vowels are
                    {
                        tempWarpFactor = 1.0f;
                    }

                    if (bandFreq > 16000.0f)
                    {
                        bandFreq = 16000.0f;
                    }

                    float bandBandwidth = (vcd->thisBandwidth * oneMinusBarkPull) + (vcd->barkBandWidths[i] *  barkPull * myQ);
                    float myHeight = (float)i * vcd->invNumberOfVocoderBands; //x value
                    float tiltOffset = (1.0f - ((myTilt * 0.5f) + 0.5f)) + 0.5f;
                    float tiltY = vcd->displayValues[12] * myHeight + tiltOffset;
                    vcd->bandGains[i] = vcd->invMyQ * tiltY;


                    tVZFilter_setFreqAndBandwidth(vcd->analysisBands[i][0], bandFreq, bandBandwidth);
                    //set these to match without computing for increased efficiency
                    vcd->analysisBands[i][1]->B =
                    vcd->analysisBands[i][0]->B;

                    vcd->analysisBands[i][1]->fc =
                    vcd->analysisBands[i][0]->fc;

                    vcd->analysisBands[i][1]->R2 =
                    vcd->analysisBands[i][0]->R2;

                    vcd->analysisBands[i][1]->cL =
                    vcd->analysisBands[i][0]->cL;

                    vcd->analysisBands[i][1]->cB =
                    vcd->analysisBands[i][0]->cB;

                    vcd->analysisBands[i][1]->cH =
                    vcd->analysisBands[i][0]->cH;

                    vcd->analysisBands[i][1]->h =
                    vcd->analysisBands[i][0]->h;

                    vcd->analysisBands[i][1]->g =
                    vcd->analysisBands[i][0]->g;


                    tVZFilter_setFreqAndBandwidth(vcd->synthesisBands[i][0], bandFreq * tempWarpFactor, bandBandwidth);
                    //set these to match without computing for increased efficiency
                    vcd->synthesisBands[i][1]->B =
                    vcd->synthesisBands[i][0]->B;

                    vcd->synthesisBands[i][1]->fc =
                    vcd->synthesisBands[i][0]->fc;

                    vcd->synthesisBands[i][1]->R2 =
                    vcd->synthesisBands[i][0]->R2;

                    vcd->synthesisBands[i][1]->cL =
                    vcd->synthesisBands[i][0]->cL;

                    vcd->synthesisBands[i][1]->cB =
                    vcd->synthesisBands[i][0]->cB;

                    vcd->synthesisBands[i][1]->cH =
                    vcd->synthesisBands[i][0]->cH;

                    vcd->synthesisBands[i][1]->h =
                    vcd->synthesisBands[i][0]->h;

                    vcd->synthesisBands[i][1]->g =
                    vcd->synthesisBands[i][0]->g;


                    vcd->alteringBands = 0;

                }
            }
#endif
            vcd->prevNumberOfVocoderBands = vcd->numberOfVocoderBands;
            vcd->prevMyQ = myQ;
            vcd->prevWarpFactor = warpFactor;
            vcd->prevBandSquish = bandSquish;
            vcd->prevBandOffset = bandOffset;
            vcd->prevMyTilt = myTilt;
            vcd->prevBarkPull = barkPull;
            
            for (int i = 0; i < vcd->numberOfVocoderBands; i++)
            {
                tExpSmooth_setFactor(vcd->envFollowers[i], (vcd->displayValues[9] * 0.0015f) + 0.0001f);
            }
            
            if (tSimplePoly_getNumActiveVoices(vcd->poly) != 0)
            {
                tExpSmooth_setDest(vcd->comp, sqrtf(1.0f / (float)tSimplePoly_getNumActiveVoices(vcd->poly)));
            }
            else
            {
                tExpSmooth_setDest(vcd->comp, 0.0f);
            }
        }
        
        void SFXVocoderChTick(Vocodec* vcd, float* input)
        {
            
            float sample = 0.0f;
            
            //a little treble emphasis
            input[1] = tVZFilter_tick(vcd->vocodec_highshelf, input[1]);
            
            if (vcd->vocoderChParams.internalExternal == 1)
            {
                sample = input[0];
            }
            else
            {
                float zerocross = tZeroCrossingCounter_tick(vcd->zerox, input[1]);

                if (!vcd->vocoderChParams.freeze)
                {
                    tExpSmooth_setDest(vcd->noiseRamp,zerocross > ((vcd->displayValues[4])-0.1f));
                }
                float noiseRampVal = tExpSmooth_tick(vcd->noiseRamp);
                
                float noiseSample = tNoise_tick(vcd->vocoderNoise) * noiseRampVal;
                
                for (int i = 0; i < tSimplePoly_getNumVoices(vcd->poly); i++)
                {
                    float tempRamp = tExpSmooth_tick(vcd->polyRamp[i]);
                    if (tempRamp > 0.0001f)
                    {
#ifndef __cplusplus
                        if (vcd->displayValues[5] < 0.5f)
                        {
                            sample += tSawtooth_tick(vcd->osc[i]) * tempRamp;
                        }
                        else
                        {
                            sample += tRosenbergGlottalPulse_tick(vcd->glottal[i]) * tempRamp;
                        }
#else
                        sample += tSawtooth_tick(vcd->osc[i]) * tempRamp * (1.0f - vcd->displayValues[5]);
                        sample += tRosenbergGlottalPulse_tickHQ(vcd->glottal[i]) * tempRamp * vcd->displayValues[5];
#endif

                    }
                }
                //switch with consonant noise
                sample = (sample * (1.0f - (0.3f * vcd->displayValues[8])) * (1.0f-noiseRampVal)) + noiseSample;
                //add breathiness
                sample += (tHighpass_tick(vcd->noiseHP, tNoise_tick(vcd->breathNoise)) * vcd->displayValues[8] * 2.0f);
                sample *= tExpSmooth_tick(vcd->comp);
                
            }
#ifndef __cplusplus
            sample = fast_tanh4(sample);
#else
            sample = tanhf(sample);
#endif
            float output[2] = {0.0f, 0.0f};
            input[1] = input[1] * (vcd->displayValues[0] * 30.0f);
            for (int i = 0; i < vcd->numberOfVocoderBands; i++)
            {
                int oddEven = i % 2;
                float tempSamp = input[1];
                if (!vcd->vocoderChParams.freeze)
                {
#ifndef __cplusplus
                    tempSamp = tVZFilter_tickEfficient(vcd->analysisBands[i][0], tempSamp);
                    tempSamp = tVZFilter_tickEfficient(vcd->analysisBands[i][1], tempSamp);
#else
                    tempSamp = tVZFilter_tick(vcd->analysisBands[i][0], tempSamp);
                    tempSamp = tVZFilter_tick(vcd->analysisBands[i][1], tempSamp);
#endif
                    tExpSmooth_setDest(vcd->envFollowers[i], fabsf(tempSamp));
                }
                
                tempSamp = tExpSmooth_tick(vcd->envFollowers[i]);
                //here is the envelope followed gain of the modulator signal
                tempSamp = LEAF_clip(0.0f, tempSamp, 2.0f);
                float tempSynth = sample;
#ifndef __cplusplus
                tempSynth = tVZFilter_tickEfficient(vcd->synthesisBands[i][0], tempSynth);
                tempSynth = tVZFilter_tickEfficient(vcd->synthesisBands[i][1], tempSynth);
#else
                tempSynth = tVZFilter_tick(vcd->synthesisBands[i][0], tempSynth);
                tempSynth = tVZFilter_tick(vcd->synthesisBands[i][1], tempSynth);
#endif
                output[oddEven] += tempSynth * tempSamp * vcd->bandGains[i];
            }
            
            float finalSample1 = tHighpass_tick(vcd->chVocFinalHP1, (output[0] + (output[1] * vcd->oneMinusStereo)) * vcd->chVocOutputGain);
            float finalSample2 = tHighpass_tick(vcd->chVocFinalHP2, (output[1] + (output[0] * vcd->oneMinusStereo)) * vcd->chVocOutputGain);
#ifndef __cplusplus
            input[0] = 0.98f * fast_tanh4(finalSample1);
            input[1] = 0.98f * fast_tanh4(finalSample2);
#else
            input[0] = 0.98f * tanhf(finalSample1);
            input[1] = 0.98f * tanhf(finalSample2);
#endif

        }

        void SFXVocoderChFree(Vocodec* vcd)
        {
            for (int i = 0; i < MAX_NUM_VOCODER_BANDS; i++)
            {
                tVZFilter_free(&vcd->analysisBands[i][0]);
                tVZFilter_free(&vcd->analysisBands[i][1]);
                tVZFilter_free(&vcd->synthesisBands[i][0]);
                tVZFilter_free(&vcd->synthesisBands[i][1]);
                tExpSmooth_free(&vcd->envFollowers[i]);
            }
            tNoise_free(&vcd->breathNoise);
            tNoise_free(&vcd->vocoderNoise);
            tZeroCrossingCounter_free(&vcd->zerox);
            tExpSmooth_free(&vcd->noiseRamp);
            tHighpass_free(&vcd->noiseHP);
            tVZFilter_free(&vcd->vocodec_highshelf);
            tHighpass_free(&vcd->chVocFinalHP1);
            tHighpass_free(&vcd->chVocFinalHP2);
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                tSawtooth_free(&vcd->osc[i]);
                tRosenbergGlottalPulse_free(&vcd->glottal[i]);
            }
        }
        
        // pitch shift
        void SFXPitchShiftAlloc(Vocodec* vcd)
        {
            tFormantShifter_init(&vcd->fs, 7, &vcd->leaf);
            tRetune_initToPool(&vcd->retune, NUM_RETUNE, mtof(42), mtof(84), 1024, &vcd->mediumPool);
            tRetune_initToPool(&vcd->retune2, NUM_RETUNE, mtof(42), mtof(84), 1024, &vcd->mediumPool);
            vcd->retune2->index = 512;
            tRamp_init(&vcd->pitchshiftRamp, 100.0f, 1, &vcd->leaf);
            tRamp_setVal(vcd->pitchshiftRamp, 1.0f);

            tSimplePoly_setNumVoices(vcd->poly, 1);
            tExpSmooth_init(&vcd->smoother1, 0.0f, 0.01f, &vcd->leaf);
            tExpSmooth_init(&vcd->smoother2, 0.0f, 0.01f, &vcd->leaf);
            tExpSmooth_init(&vcd->smoother3, 0.0f, 0.01f, &vcd->leaf);

            setLED_A(vcd, 0);
            setLED_B(vcd, 0);
            setLED_C(vcd, 0);
        }
        
        void SFXPitchShiftFrame(Vocodec* vcd)
        {
            ;
        }
        
        void SFXPitchShiftTick(Vocodec* vcd, float* input)
        {
            //pitchFactor = (smoothedADC[0]*3.75f)+0.25f;
            float sample = 0.0f;
            
            float keyPitch = (float)tSimplePoly_getPitchAndCheckActive(vcd->poly, 0);
            if (keyPitch >= 0)
            {
                keyPitch = LEAF_midiToFrequency(keyPitch) * 0.003822629969419f ;
            }
            else
            {
                keyPitch = 1.0f;
            }
            
            float myPitchFactorCoarse = (vcd->presetKnobValues[Pitchshift][0]*2.0f) - 1.0f;
            float myPitchFactorFine = ((vcd->presetKnobValues[Pitchshift][1]*2.0f) - 1.0f) * 0.1f;
            float myPitchFactorCombined = myPitchFactorFine + myPitchFactorCoarse;

            vcd->displayValues[0] = myPitchFactorCombined;
            vcd->displayValues[1] = myPitchFactorCombined;
            vcd->displayValues[2] = LEAF_clip( 0.0f,((vcd->presetKnobValues[Pitchshift][2]) * 3.0f) - 0.2f,3.0f);
            vcd->displayValues[3] = fastexp2f((vcd->presetKnobValues[Pitchshift][3]*2.0f) - 1.0f);

            float myPitchFactor = fastexp2f(myPitchFactorCombined);
            myPitchFactor *= keyPitch;
            tRetune_tuneVoice(vcd->retune, 0, myPitchFactor);
            tRetune_tuneVoice(vcd->retune2, 0, myPitchFactor);
            
            tExpSmooth_setDest(vcd->smoother3, vcd->displayValues[2]);
            tFormantShifter_setIntensity(vcd->fs, tExpSmooth_tick(vcd->smoother3)+.1f);
            tFormantShifter_setShiftFactor(vcd->fs, vcd->displayValues[3]);
            if (vcd->displayValues[2] > 0.01f)
            {
                tRamp_setDest(vcd->pitchshiftRamp, -1.0f);
            }
            else
            {
                tRamp_setDest(vcd->pitchshiftRamp, 1.0f);
            }
            
            float crossfadeVal = tRamp_tick(vcd->pitchshiftRamp);
            float myGains[2];
            LEAF_crossfade(crossfadeVal, myGains);
            tExpSmooth_setDest(vcd->smoother1, myGains[0]);
            tExpSmooth_setDest(vcd->smoother2, myGains[1]);
            
            float formantsample = tanhf(tFormantShifter_remove(vcd->fs, input[1]));

            float* samples = tRetune_tick(vcd->retune2, formantsample);
            formantsample = samples[0];
            sample = input[1];
            samples = tRetune_tick(vcd->retune, sample);
            sample = samples[0];
            
            formantsample = tanhf(tFormantShifter_add(vcd->fs, formantsample)) * tExpSmooth_tick(vcd->smoother2) ;
            sample = (sample * (tExpSmooth_tick(vcd->smoother1))) +  formantsample;
            
            input[0] = sample;
            input[1] = sample;
            
        }

        void SFXPitchShiftFree(Vocodec* vcd)
        {
            tFormantShifter_free(&vcd->fs);
            tRetune_free(&vcd->retune);
            tRetune_free(&vcd->retune2);
            
            tRamp_free(&vcd->pitchshiftRamp);
            
            tExpSmooth_free(&vcd->smoother1);
            tExpSmooth_free(&vcd->smoother2);
            tExpSmooth_free(&vcd->smoother3);
        }
        

        
        //5 autotune mono
        void SFXNeartuneAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            tRetune_initToPool(&vcd->autotuneMono, 1, mtof(42), mtof(84), 1024, &vcd->mediumPool);
            calculateNoteArray(vcd);
            tExpSmooth_init(&vcd->neartune_smoother, 1.0f, .007f, &vcd->leaf);
            tRamp_init(&vcd->nearWetRamp, 20.0f, 1, &vcd->leaf);
            vcd->leaf.clearOnAllocation = 0;
            setLED_A(vcd, vcd->neartuneParams.useChromatic);
            setLED_B(vcd, 0);
            setLED_C(vcd, vcd->neartuneParams.lock);
            vcd->lastSnap = 1.0f;
        }
        
        void SFXNeartuneFrame(Vocodec* vcd)
        {
            
            if ((tSimplePoly_getNumActiveVoices(vcd->poly) != 0) ||
                (vcd->neartuneParams.useChromatic == 1) || (vcd->neartuneParams.lock == 1))
            {
                tRamp_setDest(vcd->nearWetRamp, 1.0f);
            }
            else
            {
                tRamp_setDest(vcd->nearWetRamp, -1.0f);
            }
            
            if (vcd->buttonActionsSFX[ButtonA][ActionPress])
            {
                vcd->neartuneParams.useChromatic = !vcd->neartuneParams.useChromatic;
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->neartuneParams.useChromatic);
            }
            
            if (vcd->buttonActionsSFX[ButtonC][ActionPress])
            {
                vcd->neartuneParams.lock = !vcd->neartuneParams.lock;
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vcd, vcd->neartuneParams.lock);
                if (vcd->neartuneParams.lock)
                {
                    for (int i = 0; i < 12; i++)
                    {
                        vcd->lockArray[i] = vcd->chordArray[i];
                    }
                }
                else
                {
                    for (int i = 0; i < 12; i++)
                    {
                        vcd->lockArray[i] = 0;
                    }
                }
            }
            
            vcd->displayValues[0] = 0.5f + (vcd->presetKnobValues[AutotuneMono][0] * 0.49f); //fidelity
//            tRetune_setFidelityThreshold(vcd->autotuneMono, vcd->displayValues[0]);
            vcd->displayValues[1] = LEAF_clip(0.0f, vcd->presetKnobValues[AutotuneMono][1] * 1.1f, 1.0f); // amount of forcing to new pitch
            vcd->displayValues[2] = vcd->presetKnobValues[AutotuneMono][2]; //speed to get to desired pitch shift
            
            vcd->displayValues[3] = vcd->presetKnobValues[AutotuneMono][3] * 12.0f;
            vcd->displayValues[4] = (vcd->presetKnobValues[AutotuneMono][4] * 0.5f) + 0.5f;
        }
        
        void SFXNeartuneTick(Vocodec* vcd, float* input)
        {
            float sample = 0.0f;
            
            vcd->displayValues[0] = 0.95f + (vcd->presetKnobValues[AutotuneMono][0] * 0.05f); //fidelity
            tRetune_setPickiness(vcd->autotuneMono, vcd->displayValues[0]);
            vcd->displayValues[1] = LEAF_clip(0.0f, vcd->presetKnobValues[AutotuneMono][1] * 1.1f, 1.0f); // amount of forcing to new pitch
            vcd->displayValues[2] = vcd->presetKnobValues[AutotuneMono][2]; //speed to get to desired pitch shift
            
            vcd->displayValues[3] = vcd->presetKnobValues[AutotuneMono][3] * 12.0f;
            vcd->displayValues[4] = (vcd->presetKnobValues[AutotuneMono][4] * 0.5f) + 0.5f;
            
            if (vcd->displayValues[2] > .90f)
            {
                vcd->displayValues[2] = 1.0f;
            }
            tExpSmooth_setFactor(vcd->neartune_smoother, vcd->expBuffer[(int)(vcd->displayValues[2] * vcd->displayValues[2] * vcd->displayValues[2] * vcd->expBufferSizeMinusOne)]);
            float destFactor = tExpSmooth_tick(vcd->neartune_smoother);
            
            float detected = tRetune_getInputFrequency(vcd->autotuneMono);
            if (detected > 0.0f)
            {
                vcd->detectedNote = LEAF_frequencyToMidi(detected);
                
                vcd->desiredSnap = nearestNoteWithHysteresis(vcd, vcd->detectedNote, vcd->displayValues[4]);
                
                if (vcd->desiredSnap > 0.0f)
                {
                    vcd->destinationNote = (vcd->desiredSnap * vcd->displayValues[1]) + (vcd->detectedNote * (1.0f - vcd->displayValues[1]));
                    
                    vcd->factorDiff = (fabsf(vcd->destinationNote - vcd->lastSnap));
                    vcd->changeAmount = (fabsf(vcd->destinationNote - vcd->detectedNote));
                    
                    //if ((factorDiff < displayValues[3]) || (updatePitch == 1) || (diffCounter > maxDiffCounter))
                    
                    if ((vcd->changeAmount < 11.9))
                    {
                        vcd->destinationFactor = (LEAF_midiToFrequency(vcd->destinationNote) /
                                                  LEAF_midiToFrequency(vcd->detectedNote));
                        tExpSmooth_setDest(vcd->neartune_smoother, vcd->destinationFactor);
                        vcd->lastSnap = vcd->destinationNote;
                    }
                    
                }
                else
                {
                    tExpSmooth_setDest(vcd->neartune_smoother, 1.0f);
                }
                
            }

            tRetune_tuneVoice(vcd->autotuneMono, 0, destFactor);
            float* samples = tRetune_tick(vcd->autotuneMono, input[1]);
            //tAutotune_setFreq(autotuneMono, leaf.sampleRate / nearestPeriod(tAutotune_getInputPeriod(autotuneMono)), 0);
            
            float fades[2];
            LEAF_crossfade(tRamp_tick(vcd->nearWetRamp), fades);
            
            sample = samples[0] * fades[0];
            sample += input[1] * fades[1]; // crossfade to dry signal if no notes held down.
            input[0] = sample;
            input[1] = sample;
        }
        
        void SFXNeartuneFree(Vocodec* vcd)
        {
            tRetune_free(&vcd->autotuneMono);
            tExpSmooth_free(&vcd->neartune_smoother);
            tRamp_free(&vcd->nearWetRamp);
        }

        //6 autotune
        void SFXAutotuneAlloc(Vocodec* vcd)
        {
            tRetune_initToPool(&vcd->autotunePoly, NUM_AUTOTUNE, mtof(42), mtof(84), 1024, &vcd->mediumPool);
            tRetune_setMode(vcd->autotunePoly, 1);
            tCycle_init(&vcd->tremolo, &vcd->leaf);
            tSimplePoly_setNumVoices(vcd->poly, NUM_AUTOTUNE);
            setLED_A(vcd, 0);
            setLED_B(vcd, 0);
            setLED_C(vcd, 0);
        }
        
        void SFXAutotuneFrame(Vocodec* vcd)
        {
            for (int i = 0; i < tSimplePoly_getNumVoices(vcd->poly); ++i)
            {
                calculateFreq(vcd, i);
                tExpSmooth_setDest(vcd->polyRamp[i], (tSimplePoly_getVelocity(vcd->poly, i) > 0));
            }
            int tempNumVoices = tSimplePoly_getNumActiveVoices(vcd->poly);
            if (tempNumVoices != 0) tExpSmooth_setDest(vcd->comp, 1.0f / (float)tempNumVoices);

            vcd->displayValues[0] = 0.5f + (vcd->presetKnobValues[AutotunePoly][0] * 0.47f);
        }
        
        void SFXAutotuneTick(Vocodec* vcd, float* input)
        {
            float sample = 0.0f;
            vcd->displayValues[0] = 0.95f + (vcd->presetKnobValues[AutotunePoly][0] * 0.05f);

            tRetune_setPickiness(vcd->autotunePoly, vcd->displayValues[0]);
            
            //displayValues[1] = presetKnobValues[AutotunePoly][1];
            
            //displayValues[2] = presetKnobValues[AutotunePoly][2];
            
//            tAutotune_setFidelityThreshold(vcd->autotunePoly, vcd->displayValues[0]);
            //tAutotune_setAlpha(autotunePoly, displayValues[1]);
            //tAutotune_setTolerance(autotunePoly, displayValues[2]);

            for (int i = 0; i < tSimplePoly_getNumVoices(vcd->poly); ++i)
            {
                tRetune_tuneVoice(vcd->autotunePoly, i, vcd->freq[i]);
            }
            
            float* samples = tRetune_tick(vcd->autotunePoly, input[1]);
            
            for (int i = 0; i < tSimplePoly_getNumVoices(vcd->poly); ++i)
            {
                float gain = tExpSmooth_tick(vcd->polyRamp[i]);
                sample += samples[i] * gain;
            }
            sample *= tExpSmooth_tick(vcd->comp);
            input[0] = sample;
            input[1] = sample;

//            tCycle_setFreq(vcd->tremolo, tRetune_getInputFrequency(vcd->autotunePoly));

//            float s = tCycle_tick(vcd->tremolo) *0.2f;
//            input[0] += s;
//            input[1] += s;
        }
        
        void SFXAutotuneFree(Vocodec* vcd)
        {
            tRetune_free(&vcd->autotunePoly);
            tCycle_free(&vcd->tremolo);
        }
        
        
        //7 sampler - button press
        void SFXSamplerBPAlloc(Vocodec* vcd)
        {
            // Shouldn't rely on sample rate to set the size here because high rates will cause problems
            // 7585200 is a bit arbitrary, there's should be some more space
            tBuffer_initToPool(&vcd->buff, 7585200/*vcd->leaf.sampleRate * 172.0f*/, &vcd->largePool);
            tBuffer_setRecordMode(vcd->buff, RecordOneShot);
            tSampler_init(&vcd->sampler, &vcd->buff, &vcd->leaf);
            tSampler_setMode(vcd->sampler, (PlayMode)(vcd->samplerBPParams.playMode));
            tExpSmooth_init(&vcd->startSmooth, 0.0f, 0.01f, &vcd->leaf);
            tExpSmooth_init(&vcd->lengthSmooth, 0.0f, 0.01f, &vcd->leaf);
            setLED_A(vcd, 0);
            setLED_B(vcd, vcd->samplerBPParams.playMode == PlayBackAndForth);
            setLED_C(vcd, 0);
        }
        
        void SFXSamplerBPFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonC][ActionPress])
            {
                if (!vcd->samplerBPParams.paused)
                {
                    vcd->samplerBPParams.paused = 1;
                    tSampler_stop(vcd->sampler);
                    vcd->displayValues[1] =
                    LEAF_clip(0.0f, vcd->presetKnobValues[SamplerButtonPress][1] * vcd->sampleLength, vcd->sampleLength * (1.0f - vcd->presetKnobValues[SamplerButtonPress][0]));
                }
                else
                {
                    vcd->samplerBPParams.paused = 0;
                    tSampler_play(vcd->sampler);
                }
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
            }
            
            if (vcd->buttonActionsSFX[ButtonB][ActionPress])
            {
                if (vcd->samplerBPParams.playMode == PlayLoop) vcd->samplerBPParams.playMode = PlayBackAndForth;
                else vcd->samplerBPParams.playMode = PlayLoop;
                tSampler_setMode(vcd->sampler, (PlayMode)(vcd->samplerBPParams.playMode));
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(vcd, vcd->samplerBPParams.playMode == PlayBackAndForth);
            }
            
            if (vcd->buttonActionsSFX[ButtonA][ActionPress])
            {
                tSampler_stop(vcd->sampler);
                tBuffer_record(vcd->buff);
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, 1);
            }
            if (vcd->buttonActionsSFX[ButtonA][ActionRelease])
            {
                tBuffer_stop(vcd->buff);
                if (!vcd->samplerBPParams.paused) tSampler_play(vcd->sampler);
                vcd->buttonActionsSFX[ButtonA][ActionRelease] = 0;
                setLED_A(vcd, 0);
            }
            
            int recordPosition = tBuffer_getRecordPosition(vcd->buff);
            float* knobs = vcd->presetKnobValues[SamplerButtonPress];
            
            vcd->sampleLength = (float)recordPosition * vcd->leaf.invSampleRate;
            vcd->displayValues[0] = knobs[0] * vcd->sampleLength;
            vcd->displayValues[1] = LEAF_clip(0.0f, knobs[1] * vcd->sampleLength,
                                              vcd->sampleLength * (1.0f - knobs[0]));
            vcd->displayValues[2] = (knobs[2] - 0.5f) * 4.0f;
            float rate = roundf((knobs[3] - 0.5f) * 14.0f);
            if (rate < 0.0f)
            {
                (rate = 1.0f / fabsf(rate-1.0f));
            }
            else
            {
                rate += 1.0f;
            }
            vcd->displayValues[3] = rate;
            
            vcd->displayValues[4] = knobs[4] * 4000.0f;
        }

        void SFXSamplerBPTick(Vocodec* vcd, float* input)
        {
            float sample = 0.0f;
            int recordPosition = tBuffer_getRecordPosition(vcd->buff);
            float* knobs = vcd->presetKnobValues[SamplerButtonPress];

            vcd->sampleLength = (float)recordPosition * vcd->leaf.invSampleRate;
            vcd->displayValues[0] = knobs[0] * vcd->sampleLength;
            vcd->displayValues[1] = LEAF_clip(0.0f, knobs[1] * vcd->sampleLength,
                                              vcd->sampleLength * (1.0f - knobs[0]));
            vcd->displayValues[2] = (knobs[2] - 0.5f) * 4.0f;
            float rate = roundf((knobs[3] - 0.5f) * 14.0f);
            if (rate < 0.0f)
            {
                (rate = 1.0f / fabsf(rate-1.0f));
            }
            else
            {
                rate += 1.0f;
            }
            vcd->displayValues[3] = rate;
            
            vcd->displayValues[4] = knobs[4] * 4000.0f;
            
            vcd->samplerRate = vcd->displayValues[3] * vcd->displayValues[2];
            
            tExpSmooth_setDest(vcd->startSmooth, knobs[0] * recordPosition);
            tExpSmooth_setDest(vcd->lengthSmooth, knobs[1] * recordPosition);
            
            vcd->samplePlayStart = tExpSmooth_tick(vcd->startSmooth);
            vcd->samplePlayLength = tExpSmooth_tick(vcd->lengthSmooth);
            vcd->crossfadeLength = vcd->displayValues[4];
            tSampler_setStart(vcd->sampler, vcd->samplePlayStart);
            tSampler_setLength(vcd->sampler, vcd->samplePlayLength);
            tSampler_setRate(vcd->sampler, vcd->samplerRate);
            tSampler_setCrossfadeLength(vcd->sampler, vcd->crossfadeLength);
            
            tBuffer_tick(vcd->buff, input[1]);
            sample = tanhf(tSampler_tick(vcd->sampler));
            input[0] = sample;
            input[1] = sample;
        }
        
        void SFXSamplerBPFree(Vocodec* vcd)
        {
            tBuffer_free(&vcd->buff);
            tSampler_free(&vcd->sampler);
            tExpSmooth_free(&vcd->startSmooth);
            tExpSmooth_free(&vcd->lengthSmooth);
        }
        
        // keyboard sampler
        void SFXSamplerKAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 0; //needs this in case the box loads on this one first
            vcd->currentSamplerKeyGlobal = 60 - LOWEST_SAMPLER_KEY;
            
            for (int i = 0; i < NUM_SAMPLER_KEYS; i++)
            {
                tBuffer_initToPool(&vcd->keyBuff[i], 154350/*vcd->leaf.sampleRate * 3.5f*/, &vcd->largePool);
                tBuffer_setRecordMode(vcd->keyBuff[i], RecordOneShot);
                tSampler_init(&vcd->keySampler[i], &vcd->keyBuff[i], &vcd->leaf);
                tSampler_setMode(vcd->keySampler[i], PlayLoop);
                
                vcd->samplePlayStarts[i] = 0;
                vcd->samplePlayLengths[i] = 0;
                vcd->detectedAttackPos[i] = 0;
                vcd->crossfadeLengths[i] = 1000;
                vcd->samplerKeyHeld[i] = 0;
                tExpSmooth_init(&vcd->kSamplerGains[i], 0.0f, 0.04f, &vcd->leaf);
                vcd->sampleRates[i] = 1.0f;
                vcd->sampleRatesMult[i] = 1.0f;
                vcd->loopOns[i] = 1;
            }
            tSimplePoly_setNumVoices(vcd->poly, NUM_SAMPLER_VOICES);
            for (int i = 0; i < NUM_SAMPLER_VOICES; i++)
            {
                vcd->waitingForDeactivation[i] = -1;
            }
            setLED_A(vcd, 0);
            setLED_B(vcd, vcd->samplerKParams.controlAllKeys);
            setLED_C(vcd, 0);
            vcd->samp_thresh = 0.02f;
        }
        
        void SFXSamplerKFrame(Vocodec* vcd)
        {
            int currentSamplerKey = vcd->currentSamplerKeyGlobal;
            if (vcd->samplerKeyHeld[currentSamplerKey])
            {
                if ((tBuffer_isActive(vcd->keyBuff[currentSamplerKey])) ||
                    (currentSamplerKey != vcd->prevSamplerKey)) //only write if recording
                {
                    vcd->buttonActionsUI[ExtraMessage][ActionHoldContinuous] = 1;
                    vcd->writeButtonFlag = ExtraMessage;
                    vcd->writeActionFlag = ActionHoldContinuous;
                }
                vcd->prevSamplerKey = currentSamplerKey;
            }
            
            if (vcd->buttonActionsSFX[ButtonA][ActionPress])
            {
                tBuffer_setRecordPosition(vcd->keyBuff[currentSamplerKey],0);
                tBuffer_setRecordedLength(vcd->keyBuff[currentSamplerKey],0);
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress])
            {
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                vcd->samplerKParams.controlAllKeys = !vcd->samplerKParams.controlAllKeys;
                setLED_B(vcd, vcd->samplerKParams.controlAllKeys);
            }
            
            if (vcd->buttonActionsSFX[ButtonC][ActionPress])
            {
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                if (!vcd->samplerKParams.controlAllKeys)
                {
                    int buffLength = tBuffer_getRecordedLength(vcd->keyBuff[currentSamplerKey]);
                    if (buffLength > 0)
                    {
                        int foundAttack = 0;
                        int i = 0;
                        float currentPower = 0.0f;
                        float previousPower = 0.0f;

                        if (vcd->detectedAttackPos[currentSamplerKey] > 0)
                        {
                            vcd->detectedAttackPos[currentSamplerKey] += 4800;
                            previousPower = tBuffer_get(vcd->keyBuff[currentSamplerKey], (vcd->detectedAttackPos[currentSamplerKey] -1) % buffLength);
                        }
                        
                        while(foundAttack == 0)
                        {
                            //starts at previous detected attack position, and wraps around to read the whole buffer
                            float testSample = tBuffer_get(vcd->keyBuff[currentSamplerKey], (i + vcd->detectedAttackPos[currentSamplerKey]) % buffLength);
                            currentPower = testSample*testSample;

                            if ((currentPower > vcd->samp_thresh) && (currentPower > (previousPower + 0.0005f)))
                            {
                                //found one!

                                //back up a few samples to catch the full attack
                                int thePos = (i + vcd->detectedAttackPos[currentSamplerKey] - 480) % buffLength;
                                if (thePos < 0)
                                {
                                    thePos = 0;
                                }
                                vcd->samplePlayStarts[currentSamplerKey] = thePos;
                                vcd->detectedAttackPos[currentSamplerKey] = thePos;
                                foundAttack = 1;
                                OLEDclearLine(vcd, SecondLine);
                                OLEDwriteString(vcd, "ATKDETECT ", 10, 0, SecondLine);
                                OLEDwriteFloat(vcd, (vcd->samplePlayStarts[currentSamplerKey] / (float)buffLength) * (buffLength * vcd->leaf.invSampleRate), OLEDgetCursor(vcd), SecondLine);
                            }
                            i++;
                            
                            //if finished searching the whole buffer
                            if (i >= buffLength)
                            {
                                // didn't find one, put it back at the start
                                vcd->detectedAttackPos[currentSamplerKey] = 0;
                                OLEDclearLine(vcd, SecondLine);
                                OLEDwriteString(vcd, "NO ATK FOUND ", 10, 0, SecondLine);
                                foundAttack = 1;
                            }
                        }
                    }
                }
                
                else
                {
                    for (int i = 0; i < NUM_SAMPLER_VOICES; i++)
                    {
                        for (int key = 0; key < NUM_SAMPLER_KEYS; key++)
                        {
                            
                            int buffLength = tBuffer_getRecordedLength(vcd->keyBuff[key]);
                            if (buffLength > 0)
                            {
                                float currentPower = 0.0f;
                                float previousPower = 0.0f;
                                int foundAttack = 0;
                                int i = 0;
                                if (vcd->detectedAttackPos[key] > 0)
                                {
                                    vcd->detectedAttackPos[key] += 4800;
                                    previousPower = tBuffer_get(vcd->keyBuff[key], (vcd->detectedAttackPos[key] -1) % buffLength);
                                }
                                
                                while(foundAttack == 0)
                                {
                                    //starts at previous detected attack position, and wraps around to read the whole buffer
                                    float testSample = tBuffer_get(vcd->keyBuff[key], (i + vcd->detectedAttackPos[key]) % buffLength);
                                    currentPower = testSample*testSample;
                                    
                                    if ((currentPower > vcd->samp_thresh) && (currentPower > (previousPower + 0.0005f)))
                                    {
                                        //found one!
                                        
                                        //back up a few samples to catch the full attack
                                        int thePos = (i + vcd->detectedAttackPos[key] - 480) % buffLength;
                                        if (thePos < 0)
                                        {
                                            thePos = 0;
                                        }
                                        vcd->samplePlayStarts[key] = thePos;
                                        vcd->detectedAttackPos[key] = thePos;
                                        foundAttack = 1;
                                    }
                                    i++;
                                    
                                    //if finished searching the whole buffer
                                    if (i >= buffLength)
                                    {
                                        // didn't find one, put it back at the start
                                        //detectedAttackPos[key] = 0;
                                        foundAttack = 1;
                                    }
                                }
                            }
                        }
                    }
                    
                }
            }

            float* knobs = vcd->presetKnobValues[SamplerKeyboard];

            if (!vcd->samplerKParams.controlAllKeys)
            {
                int recordedLength = tBuffer_getRecordedLength(vcd->keyBuff[currentSamplerKey]);
                vcd->sampleLength = recordedLength * vcd->leaf.invSampleRate;

                vcd->displayValues[0] = knobs[0] * vcd->sampleLength;

                vcd->displayValues[1] = LEAF_clip(0.0f, knobs[1] * vcd->sampleLength, vcd->sampleLength * (1.0f - knobs[0]));

                vcd->displayValues[2] = (knobs[2] - 0.5f) * 4.0f;


                float rate = roundf((knobs[3] - 0.5f) * 14.0f);
                if (rate < 0.0f)
                {
                    (rate = 1.0f / fabsf(rate-1.0f));
                }
                else
                {
                    rate += 1.0f;
                }
                vcd->displayValues[3] = rate;

                vcd->displayValues[4] = roundf(knobs[4]);

                vcd->displayValues[5] = knobs[5] * 4000.0f;

                vcd->displayValues[6] = knobs[6];
            }

            else
            {

                for (int i = 0; i < NUM_SAMPLER_VOICES; i++)
                {
                    vcd->displayValues[0] = knobs[0];

                    vcd->displayValues[1] = LEAF_clip(0.0f, knobs[1], (1.0f - knobs[0]));

                    vcd->displayValues[2] = (knobs[2] - 0.5f) * 4.0f;

                    float rate = roundf((knobs[3] - 0.5f) * 14.0f);
                    if (rate < 0.0f)
                    {
                        (rate = 1.0f / fabsf(rate-1.0f));
                    }
                    else
                    {
                        rate += 1.0f;
                    }
                    vcd->displayValues[3] = rate;

                    vcd->displayValues[4] = roundf(knobs[4]);

                    vcd->displayValues[5] = knobs[5] * 4000.0f;

                    vcd->displayValues[6] = knobs[6];
                }

            }
        }

        void SFXSamplerKTick(Vocodec* vcd, float* input)
        {
            float sample = 0.0f;
            float* knobs = vcd->presetKnobValues[SamplerKeyboard];
            
            int currentSamplerKey = vcd->currentSamplerKeyGlobal;
            
            if (!vcd->samplerKParams.controlAllKeys)
            {
                int recordedLength = tBuffer_getRecordedLength(vcd->keyBuff[currentSamplerKey]);
                vcd->sampleLength = recordedLength * vcd->leaf.invSampleRate;
                
                vcd->displayValues[0] = knobs[0] * vcd->sampleLength;
                
                vcd->displayValues[1] = LEAF_clip(0.0f, knobs[1] * vcd->sampleLength, vcd->sampleLength * (1.0f - knobs[0]));
                
                vcd->displayValues[2] = (knobs[2] - 0.5f) * 4.0f;
                
                
                float rate = roundf((knobs[3] - 0.5f) * 14.0f);
                if (rate < 0.0f)
                {
                    (rate = 1.0f / fabsf(rate-1.0f));
                }
                else
                {
                    rate += 1.0f;
                }
                vcd->displayValues[3] = rate;
                
                vcd->displayValues[4] = roundf(knobs[4]);
                
                vcd->displayValues[5] = knobs[5] * 4000.0f;
                
                vcd->displayValues[6] = knobs[6];
                
                //check if display values are new
                if (fabsf(knobs[0] - vcd->prevKnobs[0]) > 0.0005f)
                {
                    vcd->samplePlayStarts[currentSamplerKey]= (knobs[0] * recordedLength);// + detectedAttackPos[currentSamplerKey];
                    
                }
                
                if (fabsf(knobs[1] - vcd->prevKnobs[1])  > 0.0005f)
                {
                    vcd->samplePlayLengths[currentSamplerKey] = (knobs[1] * recordedLength);// - detectedAttackPos[currentSamplerKey];
                    
                }
                
                if (fabsf(knobs[2] - vcd->prevKnobs[2])  > 0.0005f)
                {
                    vcd->sampleRates[currentSamplerKey] = vcd->displayValues[2];
                    
                }
                
                if (fabsf(knobs[3] - vcd->prevKnobs[3])  > 0.0005f)
                {
                    vcd->sampleRatesMult[currentSamplerKey] = vcd->displayValues[3];
                }

                if (fabsf(knobs[4] - vcd->prevKnobs[4]) > 0.0005f)
                {
                    vcd->loopOns[currentSamplerKey] = roundf(knobs[4]);
                }
                
                if (fabsf(knobs[5] - vcd->prevKnobs[5])> 0.0005f)
                {
                    vcd->crossfadeLengths[currentSamplerKey] = vcd->displayValues[5];
                }
                
                tSampler_setStart(vcd->keySampler[currentSamplerKey],
                                  vcd->samplePlayStarts[currentSamplerKey]);
                tSampler_setLength(vcd->keySampler[currentSamplerKey],
                                   vcd->samplePlayLengths[currentSamplerKey]);
                tSampler_setCrossfadeLength(vcd->keySampler[currentSamplerKey],
                                            vcd->crossfadeLengths[currentSamplerKey]);
                tSampler_setRate(vcd->keySampler[currentSamplerKey],
                                 vcd->sampleRates[currentSamplerKey] * vcd->sampleRatesMult[currentSamplerKey]);
                tSampler_setMode(vcd->keySampler[currentSamplerKey],
                                 (PlayMode)vcd->loopOns[currentSamplerKey]);
                
                for (int i = 0; i < NUM_SAMPLER_VOICES; ++i)
                {
                    if (tSimplePoly_isOn(vcd->poly, i) > 0)
                    {
                        int key = tSimplePoly_getPitch(vcd->poly, i) - LOWEST_SAMPLER_KEY;
                        if ((0 <= key) && (key < NUM_SAMPLER_KEYS))
                        {
                            tBuffer_tick(vcd->keyBuff[key], input[1]);
                        }
                    }
                }
            }
            
            else
            {
                for (int i = 0; i < NUM_SAMPLER_VOICES; i++)
                {
                    vcd->displayValues[0] = knobs[0];
                    
                    vcd->displayValues[1] = LEAF_clip(0.0f, knobs[1], (1.0f - knobs[0]));
                    
                    vcd->displayValues[2] = (knobs[2] - 0.5f) * 4.0f;
                    
                    
                    
                    float rate = roundf((knobs[3] - 0.5f) * 14.0f);
                    if (rate < 0.0f)
                    {
                        (rate = 1.0f / fabsf(rate-1.0f));
                    }
                    else
                    {
                        rate += 1.0f;
                    }
                    vcd->displayValues[3] = rate;
                    
                    vcd->displayValues[4] = roundf(knobs[4]);
                    
                    vcd->displayValues[5] = knobs[5] * 4000.0f;
                    
                    vcd->displayValues[6] = knobs[6];
                    
                    //get the keys sounding right now
                    if (tSimplePoly_isOn(vcd->poly, i) > 0)
                    {
                        int key = tSimplePoly_getPitch(vcd->poly, i) - LOWEST_SAMPLER_KEY;
                        if ((0 <= key) && (key < NUM_SAMPLER_KEYS))
                        {
                            tBuffer_tick(vcd->keyBuff[key], input[1]);
                            
                            int recordedLength = tBuffer_getRecordedLength(vcd->keyBuff[key]);
                            vcd->sampleLength = recordedLength * vcd->leaf.invSampleRate;
                            
                            //check if display values are new
                            if (knobs[0] != vcd->prevKnobs[0])
                            {
                                vcd->samplePlayStarts[key]= (knobs[0] * recordedLength);
                            }
                            
                            if (knobs[1] != vcd->prevKnobs[1])
                            {
                                vcd->samplePlayLengths[key] = (knobs[1] * recordedLength);
                            }
                            
                            if (knobs[2] != vcd->prevKnobs[2])
                            {
                                vcd->sampleRates[key] = vcd->displayValues[2];
                            }
                            
                            if (knobs[3] != vcd->prevKnobs[3])
                            {
                                vcd->sampleRatesMult[key] = vcd->displayValues[3];
                            }
                            
                            if (knobs[4] != vcd->prevKnobs[4])
                            {
                                vcd->loopOns[key] = roundf(knobs[4]);
                            }
                            
                            if (knobs[5] != vcd->prevKnobs[5])
                            {
                                vcd->crossfadeLengths[key] = vcd->displayValues[5];
                            }
                            
                            tSampler_setStart(vcd->keySampler[key], vcd->samplePlayStarts[key]);
                            tSampler_setLength(vcd->keySampler[key], vcd->samplePlayLengths[key]);
                            tSampler_setCrossfadeLength(vcd->keySampler[key], vcd->crossfadeLengths[key]);
                            tSampler_setRate(vcd->keySampler[key],
                                             vcd->sampleRates[key] * vcd->sampleRatesMult[key]);
                            tSampler_setMode(vcd->keySampler[key], (PlayMode)vcd->loopOns[key]);
                            
                        }
                        
                    }
                }
            }
            
            
            for (int i = 0; i < 6; i++)
            {
                vcd->prevKnobs[i] = knobs[i];
            }
            
            for (int i = 0; i < NUM_SAMPLER_KEYS; i++)
            {
                float tempGain = tExpSmooth_tick(vcd->kSamplerGains[i]);
                if ( tempGain > 0.001f)
                {
                    sample += tSampler_tick(vcd->keySampler[i]) * tempGain;
                }
                else
                {
                    for (int j = 0; j< NUM_SAMPLER_VOICES; j++)
                    {
                        if (vcd->waitingForDeactivation[j] == (i + LOWEST_SAMPLER_KEY))
                        {
                            tSimplePoly_deactivateVoice(vcd->poly, j);
                            vcd->waitingForDeactivation[j] = -1;
                        }
                    }
                }
            }
            
            
            sample = tanhf(sample) * 0.98f;
            input[0] = sample;
            input[1] = sample;
        }
        
        void SFXSamplerKFree(Vocodec* vcd)
        {
            for (int i = 0; i < NUM_SAMPLER_KEYS; i++)
            {
                tBuffer_free(&vcd->keyBuff[i]);
                tSampler_free(&vcd->keySampler[i]);
                tExpSmooth_free(&vcd->kSamplerGains[i]);
            }
        }
        
        
        
        //8 sampler - auto
        void SFXSamplerAutoAlloc(Vocodec* vcd)
        {
            tBuffer_initToPool(&vcd->asBuff[0], MAX_AUTOSAMP_LENGTH, &vcd->largePool);
            tBuffer_setRecordMode(vcd->asBuff[0], RecordOneShot);
            tBuffer_initToPool(&vcd->asBuff[1], MAX_AUTOSAMP_LENGTH, &vcd->largePool);
            tBuffer_setRecordMode(vcd->asBuff[1], RecordOneShot);
            tSampler_init(&vcd->asSampler[0], &vcd->asBuff[0], &vcd->leaf);
            tSampler_setMode(vcd->asSampler[0], PlayLoop);
            tSampler_init(&vcd->asSampler[1], &vcd->asBuff[1], &vcd->leaf);
            tSampler_setMode(vcd->asSampler[1], PlayLoop);

            tEnvelopeFollower_init(&vcd->envfollow, 0.00001f, 0.9999f, &vcd->leaf);
            tExpSmooth_init(&vcd->cfxSmooth, 0.0f, 0.01f, &vcd->leaf);

            vcd->currentSampler = 1;
            vcd->sample_countdown = 0;
            vcd->randLengthVal = vcd->leaf.random() * 10000.0f;
            vcd->randRateVal = (vcd->leaf.random() - 0.5f) * 4.0f;
            
            setLED_A(vcd, vcd->samplerAutoParams.playMode == PlayBackAndForth);
            setLED_B(vcd, vcd->samplerAutoParams.triggerChannel);
            setLED_C(vcd, vcd->samplerAutoParams.quantizeRate);
        }
        
        void SFXSamplerAutoFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonA][ActionPress])
            {
                if (vcd->samplerAutoParams.playMode == PlayLoop)
                {
                    tSampler_setMode(vcd->asSampler[0], PlayBackAndForth);
                    tSampler_setMode(vcd->asSampler[1], PlayBackAndForth);
                    vcd->samplerAutoParams.playMode = PlayBackAndForth;
                    setLED_A(vcd, 1);
                    vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                }
                else if (vcd->samplerAutoParams.playMode == PlayBackAndForth)
                {
                    tSampler_setMode(vcd->asSampler[0], PlayLoop);
                    tSampler_setMode(vcd->asSampler[1], PlayLoop);
                    vcd->samplerAutoParams.playMode = PlayLoop;
                    setLED_A(vcd, 0);
                    vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                }
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress])
            {
                vcd->samplerAutoParams.triggerChannel = (vcd->samplerAutoParams.triggerChannel > 0) ? 0 : 1;
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(vcd, vcd->samplerAutoParams.triggerChannel);
            }
            if (vcd->buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                vcd->samplerAutoParams.quantizeRate = !vcd->samplerAutoParams.quantizeRate;
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vcd, vcd->samplerAutoParams.quantizeRate);
            }

            float* knobs = vcd->presetKnobValues[SamplerAutoGrab];

            vcd->samp_thresh = 1.0f - knobs[0];
            vcd->displayValues[0] = vcd->samp_thresh;

            int window_size = vcd->expBuffer[(int)(knobs[1] * vcd->expBufferSizeMinusOne)] * MAX_AUTOSAMP_LENGTH;
            vcd->displayValues[1] = window_size;

            float rate;
            if (vcd->samplerAutoParams.quantizeRate)
            {
                rate = roundf((knobs[2] - 0.5f) * 14.0f);
                if (rate < 0.0f) rate = 1.0f / fabsf(rate-1.0f);
                else rate += 1.0f;
            }
            else
            {
                rate = (knobs[2] - 0.5f) * 4.0f;
            }
            vcd->displayValues[2] = rate;

            vcd->crossfadeLength = knobs[3] * 1000.0f;
            vcd->displayValues[3] = vcd->crossfadeLength;

            float randLengthAmount = knobs[5] * 5000.0f;
            if (randLengthAmount < 20.0f) randLengthAmount = 0.0f;
            vcd->displayValues[5] = randLengthAmount;
            
            float randRateAmount;

            if (vcd->samplerAutoParams.quantizeRate)
            {
                randRateAmount = roundf(knobs[6] * 8.0f);
            }
            else
            {
                randRateAmount = knobs[6] * 2.0f;
                if (randRateAmount < 0.01) randRateAmount = 0.0f;
            }
            
            vcd->displayValues[6] = randRateAmount;
        }
        
        void SFXSamplerAutoTick(Vocodec* vcd, float* input)
        {
            float sample = 0.0f;
            if (vcd->samplerAutoParams.triggerChannel > 0)
            {
                vcd->currentPower = tEnvelopeFollower_tick(vcd->envfollow, input[0]);
            }
            else
            {
                vcd->currentPower = tEnvelopeFollower_tick(vcd->envfollow, input[1]);
            }
            
            float* knobs = vcd->presetKnobValues[SamplerAutoGrab];
            
            vcd->samp_thresh = 1.0f - knobs[0];
            vcd->displayValues[0] = vcd->samp_thresh;
            
            int window_size = vcd->expBuffer[(int)(knobs[1] * vcd->expBufferSizeMinusOne)] * MAX_AUTOSAMP_LENGTH;
            vcd->displayValues[1] = window_size;
            
            float rate;
            if (vcd->samplerAutoParams.quantizeRate)
            {
                rate = roundf((knobs[2] - 0.5f) * 14.0f);
                if (rate < 0.0f) rate = 1.0f / fabsf(rate-1.0f);
                else rate += 1.0f;
            }
            else
            {
                rate = (knobs[2] - 0.5f) * 4.0f;
            }
            vcd->displayValues[2] = rate;
            
            vcd->crossfadeLength = knobs[3] * 1000.0f;
            vcd->displayValues[3] = vcd->crossfadeLength;
            
            float randLengthAmount = knobs[5] * 5000.0f;
            if (randLengthAmount < 20.0f) randLengthAmount = 0.0f;
            vcd->displayValues[5] = randLengthAmount;
            
            float randRateAmount;
            
            if (vcd->samplerAutoParams.quantizeRate)
            {
                randRateAmount = roundf(knobs[6] * 8.0f);
            }
            else
            {
                randRateAmount = knobs[6] * 2.0f;
                if (randRateAmount < 0.01) randRateAmount = 0.0f;
            }
            
            vcd->displayValues[6] = randRateAmount;
            
            
            tSampler_setCrossfadeLength(vcd->asSampler[0], vcd->crossfadeLength);
            tSampler_setCrossfadeLength(vcd->asSampler[1], vcd->crossfadeLength);
            
            
            if ((vcd->currentPower > (vcd->samp_thresh)) && (vcd->currentPower > (vcd->previousPower + 0.001f)) && (vcd->samp_triggered == 0) && (vcd->sample_countdown == 0) && (vcd->fadeDone == 1))
            {
                vcd->randLengthVal = (vcd->leaf.random() - 0.5f) * randLengthAmount * 2.0f;
                if (vcd->samplerAutoParams.quantizeRate) vcd->randRateVal = roundf(vcd->leaf.random() * randRateAmount) + 1.0f;
                else vcd->randRateVal = (vcd->leaf.random() - 0.5f) * randRateAmount * 2.0f;
                
                vcd->samp_triggered = 1;
                setLED_1(vcd, 1);
                
                vcd->finalWindowSize = LEAF_clip(4, window_size + vcd->randLengthVal, MAX_AUTOSAMP_LENGTH);
                vcd->sample_countdown = vcd->finalWindowSize;
                tSampler_stop(vcd->asSampler[!vcd->currentSampler]);
                tBuffer_record(vcd->asBuff[!vcd->currentSampler]);
            }
            
            
            tBuffer_tick(vcd->asBuff[0], input[1]);
            tBuffer_tick(vcd->asBuff[1], input[1]);
            
            if (vcd->sample_countdown > 0)
            {
                vcd->sample_countdown--;
            }
            else if (vcd->samp_triggered == 1)
            {
                setLED_1(vcd, 0);
                
                vcd->currentSampler = !vcd->currentSampler;
                
                tSampler_play(vcd->asSampler[vcd->currentSampler]);
                
                tExpSmooth_setDest(vcd->cfxSmooth, (float)vcd->currentSampler);
                
                vcd->samp_triggered = 0;
                vcd->fadeDone = 0;
            }
            
            if (vcd->samplerAutoParams.quantizeRate)
            {
                tSampler_setRate(vcd->asSampler[0], rate * vcd->randRateVal);
                tSampler_setRate(vcd->asSampler[1], rate * vcd->randRateVal);
            }
            else
            {
                tSampler_setRate(vcd->asSampler[0], rate + vcd->randRateVal);
                tSampler_setRate(vcd->asSampler[1], rate + vcd->randRateVal);
            }
            vcd->finalWindowSize = LEAF_clip(4, window_size + vcd->randLengthVal, MAX_AUTOSAMP_LENGTH);
            tSampler_setEnd(vcd->asSampler[0], vcd->finalWindowSize);
            tSampler_setEnd(vcd->asSampler[1], vcd->finalWindowSize);
            
            float fade = tExpSmooth_tick(vcd->cfxSmooth);
            if (fabsf(vcd->cfxSmooth->curr - vcd->cfxSmooth->dest) < 0.00001f)
            {
                vcd->fadeDone = 1;
            }
            float volumes[2];
            LEAF_crossfade((fade * 2.0f) - 1.0f, volumes);
            sample = (tSampler_tick(vcd->asSampler[0]) * volumes[1]) + (tSampler_tick(vcd->asSampler[1]) * volumes[0]);
            
            vcd->previousPower = vcd->currentPower;
            input[0] = sample * 0.99f;
            input[1] = sample * 0.99f;
        }
        
        void SFXSamplerAutoFree(Vocodec* vcd)
        {
            tBuffer_free(&vcd->asBuff[0]);
            tBuffer_free(&vcd->asBuff[1]);
            tSampler_free(&vcd->asSampler[0]);
            tSampler_free(&vcd->asSampler[1]);
            tEnvelopeFollower_free(&vcd->envfollow);
            tExpSmooth_free(&vcd->cfxSmooth);
            setLED_1(vcd, 0);
        }
        
        //10 distortion tanh
        void SFXDistortionAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            tOversampler_init(&vcd->oversampler, vcd->distOS_ratio, 0, &vcd->leaf);
            tVZFilter_init(&vcd->shelf1, Lowshelf, 80.0f, 6.0f, &vcd->leaf);
            tVZFilter_init(&vcd->shelf2, Highshelf, 12000.0f, 6.0f, &vcd->leaf);
            tVZFilter_init(&vcd->bell1, Bell, 1000.0f, 1.9f, &vcd->leaf);
            tVZFilter_setSampleRate(vcd->shelf1, vcd->leaf.sampleRate * vcd->distOS_ratio);
            tVZFilter_setSampleRate(vcd->shelf2, vcd->leaf.sampleRate * vcd->distOS_ratio);
            tVZFilter_setSampleRate(vcd->bell1, vcd->leaf.sampleRate * vcd->distOS_ratio);
            setLED_A(vcd, vcd->distortionParams.mode);
            setLED_B(vcd, 0);
            setLED_C(vcd, 0);
            
            vcd->leaf.clearOnAllocation = 0;
        }
        
        void SFXDistortionFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonA][ActionPress])
            {
                vcd->distortionParams.mode = !vcd->distortionParams.mode;
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->distortionParams.mode);
            }
            vcd->displayValues[0] = ((vcd->presetKnobValues[Distortion][0] * 20.0f) + 1.0f); // 15.0
            vcd->displayValues[1] = (vcd->presetKnobValues[Distortion][1] * 30.0f) - 15.0f;
            vcd->displayValues[2] = (vcd->presetKnobValues[Distortion][2] * 34.0f) - 17.0f;
            vcd->displayValues[3] = faster_mtof(vcd->presetKnobValues[Distortion][3] * 77.0f + 42.0f);
            vcd->displayValues[4] = vcd->presetKnobValues[Distortion][4]; // 15.0f
            
            tVZFilter_setGain(vcd->shelf1, fastdbtoa(-1.0f * vcd->displayValues[1]));
            tVZFilter_setGain(vcd->shelf2, fastdbtoa(vcd->displayValues[1]));
            tVZFilter_setFreq(vcd->bell1, vcd->displayValues[3]);
            tVZFilter_setGain(vcd->bell1, fastdbtoa(vcd->displayValues[2]));
            
        }
        
        void SFXDistortionTick(Vocodec* vcd, float* input)
        {
            float sample = input[1];
            vcd->displayValues[0] = ((vcd->presetKnobValues[Distortion][0] * 20.0f) + 1.0f); // 15.0f
            vcd->displayValues[4] = vcd->presetKnobValues[Distortion][4]; // 15.0f
            sample = sample * vcd->displayValues[0];
            
            tOversampler_upsample(vcd->oversampler, sample, vcd->oversamplerArray);
            for (int i = 0; i < vcd->distOS_ratio; i++)
            {
                if (vcd->distortionParams.mode > 0)
                    vcd->oversamplerArray[i] = LEAF_shaper(vcd->oversamplerArray[i], 1.0f);
                else vcd->oversamplerArray[i] = tanhf(vcd->oversamplerArray[i]);
                vcd->oversamplerArray[i] = tVZFilter_tick(vcd->shelf1, vcd->oversamplerArray[i]); //put it through the low shelf
                vcd->oversamplerArray[i] = tVZFilter_tick(vcd->shelf2, vcd->oversamplerArray[i]); // now put that result through the high shelf
                vcd->oversamplerArray[i] = tVZFilter_tick(vcd->bell1, vcd->oversamplerArray[i]); // now add a bell (or peaking eq) filter
                vcd->oversamplerArray[i] = tanhf(vcd->oversamplerArray[i] * vcd->presetKnobValues[Distortion][4]) * 0.95f;
            }
            sample = tOversampler_downsample(vcd->oversampler, vcd->oversamplerArray);
            
            input[0] = sample;
            input[1] = sample;
            
        }
        
        void SFXDistortionFree(Vocodec* vcd)
        {
            tOversampler_free(&vcd->oversampler);
            tVZFilter_free(&vcd->shelf1);
            tVZFilter_free(&vcd->shelf2);
            tVZFilter_free(&vcd->bell1);
        }
        
        // distortion wave folder
        void SFXWaveFolderAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            tLockhartWavefolder_init(&vcd->wavefolder1, &vcd->leaf);
            tLockhartWavefolder_init(&vcd->wavefolder2, &vcd->leaf);
            tHighpass_init(&vcd->wfHP, 10.0f, &vcd->leaf);
            tOversampler_init(&vcd->oversampler, 2, 0, &vcd->leaf);
            setLED_A(vcd, vcd->waveFolderParams.mode);
            setLED_B(vcd, 0);
            setLED_C(vcd, 0);
            vcd->leaf.clearOnAllocation = 0;
        }
        
        void SFXWaveFolderFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonA][ActionPress])
            {
                vcd->waveFolderParams.mode = !vcd->waveFolderParams.mode;
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->waveFolderParams.mode);
            }
            
            vcd->displayValues[0] = (vcd->presetKnobValues[Wavefolder][0] * 4.0f);

            vcd->displayValues[1] = vcd->presetKnobValues[Wavefolder][1] - 0.5f;

            vcd->displayValues[2] = vcd->presetKnobValues[Wavefolder][2] - 0.5f;

            vcd->displayValues[3] = vcd->presetKnobValues[Wavefolder][3];
        }
        
        void SFXWaveFolderTick(Vocodec* vcd, float* input)
        {
            //knob 0 = gain
            float sample = input[1];
            
            vcd->displayValues[0] = (vcd->presetKnobValues[Wavefolder][0] * 4.0f);
            
            vcd->displayValues[1] = vcd->presetKnobValues[Wavefolder][1] - 0.5f;
            
            vcd->displayValues[2] = vcd->presetKnobValues[Wavefolder][2] - 0.5f;
            float gain = vcd->displayValues[0];
            vcd->displayValues[3] = vcd->presetKnobValues[Wavefolder][3];
            
            
            //sample = sample * gain * 0.33f;
            
            sample = sample * gain;
            //sample = sample + knobParams[1];
            if (vcd->waveFolderParams.mode == 0)
            {
                tOversampler_upsample(vcd->oversampler, sample, vcd->oversamplerArray);
                for (int i = 0; i < 2; i++)
                {
                    vcd->oversamplerArray[i] = sample + vcd->displayValues[1];
                    vcd->oversamplerArray[i] *= vcd->displayValues[0];
                    vcd->oversamplerArray[i] = tanhf(vcd->oversamplerArray[i]);
                    //oversamplerArray[i] *= knobParams[0] * 1.5f;
                    
                    vcd->oversamplerArray[i] = tLockhartWavefolder_tick(vcd->wavefolder1, vcd->oversamplerArray[i]);
                    //oversamplerArray[i] = tLockhartWavefolder_tick(wavefolder2, oversamplerArray[i]);
                    //sample = sample * gain;
                    
                    //oversamplerArray[i] = tLockhartWavefolder_tick(wavefolder2, oversamplerArray[i]);
                    //oversamplerArray[i] = sample + knobParams[3];
                    //sample *= .6f;
                    //oversamplerArray[i] = tLockhartWavefolder_tick(wavefolder3, oversamplerArray[i]);
                    //sample = tLockhartWavefolder_tick(wavefolder4, sample);
                    //oversamplerArray[i] *= .8f;
                    vcd->oversamplerArray[i] = tanhf(vcd->oversamplerArray[i]);
                }
                sample = tHighpass_tick(vcd->wfHP, tOversampler_downsample(vcd->oversampler, vcd->oversamplerArray)) * vcd->displayValues[3];
                input[0] = sample;
                input[1] = sample;
            }
            else
            {
                
                sample = sample + vcd->displayValues[1];
                sample *= vcd->displayValues[0];
                sample = LEAF_tanh(sample);
                //oversamplerArray[i] *= knobParams[0] * 1.5f;
                
                sample = tLockhartWavefolder_tick(vcd->wavefolder1, sample);
                
                
                
                sample = sample + vcd->displayValues[2];
                sample *= vcd->displayValues[0];
                sample = LEAF_tanh(sample);
                //oversamplerArray[i] *= knobParams[0] * 1.5f;
                
                sample = tLockhartWavefolder_tick(vcd->wavefolder2, sample);
                
                sample = tOversampler_tick(vcd->oversampler, sample, vcd->oversampleBuf, &LEAF_tanh);
                sample = tHighpass_tick(vcd->wfHP, sample) * vcd->displayValues[3];
                //sample *= 0.99f;
                input[0] = sample;
                input[1] = sample;
            }
            
        }
        
        void SFXWaveFolderFree(Vocodec* vcd)
        {
            tLockhartWavefolder_free(&vcd->wavefolder1);
            tLockhartWavefolder_free(&vcd->wavefolder2);
            tHighpass_free(&vcd->wfHP);
            tOversampler_free(&vcd->oversampler);
        }
        

        //13 bitcrusher
        void SFXBitcrusherAlloc(Vocodec* vcd)
        {
            tCrusher_init(&vcd->crush, &vcd->leaf);
            tCrusher_init(&vcd->crush2, &vcd->leaf);
            setLED_A(vcd, vcd->bitcrusherParams.stereo);
            setLED_B(vcd, 0);
            setLED_C(vcd, 0);
        }
        
        void SFXBitcrusherFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonA][ActionPress])
            {
                vcd->bitcrusherParams.stereo = !vcd->bitcrusherParams.stereo;
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->bitcrusherParams.stereo);
            }

            vcd->displayValues[0] = (vcd->presetKnobValues[BitCrusher][0] * 0.99f )+ 0.01f;
            vcd->displayValues[1] = vcd->presetKnobValues[BitCrusher][1];
            vcd->displayValues[2] = vcd->presetKnobValues[BitCrusher][2] * 0.1f;
            vcd->displayValues[3] = (uint32_t) (vcd->presetKnobValues[BitCrusher][3] * 8.0f);
            vcd->displayValues[4] = vcd->presetKnobValues[BitCrusher][4];
            vcd->displayValues[5] = (vcd->presetKnobValues[BitCrusher][5] * 5.0f) + 1.0f;
        }
        
        void SFXBitcrusherTick(Vocodec* vcd, float* input)
        {
            float sample = 0.0f;
            vcd->displayValues[0] = (vcd->presetKnobValues[BitCrusher][0] * 0.99f )+ 0.01f;
            tCrusher_setQuality (vcd->crush, vcd->presetKnobValues[BitCrusher][0]);
            tCrusher_setQuality (vcd->crush2, vcd->presetKnobValues[BitCrusher][0]);
            vcd->displayValues[1] = vcd->presetKnobValues[BitCrusher][1];
            tCrusher_setSamplingRatio (vcd->crush, vcd->presetKnobValues[BitCrusher][1] * 0.5f);
            tCrusher_setSamplingRatio (vcd->crush2, vcd->presetKnobValues[BitCrusher][1] * 0.5f);
            vcd->displayValues[2] = vcd->presetKnobValues[BitCrusher][2] * 0.1f;
            tCrusher_setRound (vcd->crush, vcd->displayValues[2]);
            tCrusher_setRound (vcd->crush2, vcd->displayValues[2]);
            vcd->displayValues[3] = (uint32_t) (vcd->presetKnobValues[BitCrusher][3] * 8.0f);
            tCrusher_setOperation (vcd->crush, vcd->presetKnobValues[BitCrusher][3]);
            tCrusher_setOperation (vcd->crush2, vcd->presetKnobValues[BitCrusher][3]);
            vcd->displayValues[4] = vcd->presetKnobValues[BitCrusher][4];
            vcd->displayValues[5] = (vcd->presetKnobValues[BitCrusher][5] * 5.0f) + 1.0f;
            float volumeComp;
            
            if (vcd->displayValues[0] < 0.1f)
            {
                volumeComp = 1.0f;
            }
            else
            {
                volumeComp = (1.0f / (vcd->displayValues[3] + 1.0f));
            }
            sample = tanhf(tCrusher_tick(vcd->crush, input[1] * vcd->displayValues[5])) * vcd->displayValues[4] * volumeComp;
            if (vcd->bitcrusherParams.stereo)
            {
                input[0] = tanhf(tCrusher_tick(vcd->crush2, input[0] * vcd->displayValues[5])) * vcd->displayValues[4] * volumeComp;
            }
            else
            {
                input[0] = sample;
            }
            input[1] = sample;
        }
        
        void SFXBitcrusherFree(Vocodec* vcd)
        {
            tCrusher_free(&vcd->crush);
            tCrusher_free(&vcd->crush2);
        }
        
        
        //delay
        void SFXDelayAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            tTapeDelay_initToPool(&vcd->delay, 2000, 30000, &vcd->mediumPool);
            tTapeDelay_initToPool(&vcd->delay2, 2000, 30000, &vcd->mediumPool);
            tSVF_init(&vcd->delayLP, SVFTypeLowpass, 16000.f, .7f, &vcd->leaf);
            tSVF_init(&vcd->delayHP, SVFTypeHighpass, 20.f, .7f, &vcd->leaf);

            tSVF_init(&vcd->delayLP2, SVFTypeLowpass, 16000.f, .7f, &vcd->leaf);
            tSVF_init(&vcd->delayHP2, SVFTypeHighpass, 20.f, .7f, &vcd->leaf);

            tHighpass_init(&vcd->delayShaperHp, 20.0f, &vcd->leaf);
            tFeedbackLeveler_init(&vcd->feedbackControl, .99f, 0.01f, 0.125f, 0, &vcd->leaf);
            setLED_A(vcd, vcd->delayParams.shaper);
            setLED_B(vcd, vcd->delayParams.uncapFeedback);
            setLED_C(vcd, vcd->delayParams.freeze);
            vcd->leaf.clearOnAllocation = 0;
        }
        
        void SFXDelayFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonA][ActionPress])
            {
                vcd->delayParams.shaper = !vcd->delayParams.shaper;
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->delayParams.shaper);
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress])
            {
                vcd->delayParams.uncapFeedback = !vcd->delayParams.uncapFeedback;
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(vcd, vcd->delayParams.uncapFeedback);
            }
            
            if (vcd->buttonActionsSFX[ButtonC][ActionPress])
            {
                vcd->delayParams.freeze = !vcd->delayParams.freeze;
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vcd, vcd->delayParams.freeze);
            }
            
            vcd->displayValues[0] = vcd->presetKnobValues[Delay][0] * 30000.0f;
            vcd->displayValues[1] = vcd->presetKnobValues[Delay][1] * 30000.0f;
            float cutoff1 = LEAF_clip(10.0f,
                                      faster_mtof((vcd->presetKnobValues[Delay][2] * 133) + 3.0f),
                                      20000.0f);
            float cutoff2 = LEAF_clip(10.0f,
                                      faster_mtof((vcd->presetKnobValues[Delay][3] * 133) + 3.0f),
                                      20000.0f);
            vcd->displayValues[2] = cutoff1;
            vcd->displayValues[3] = cutoff2;

            vcd->displayValues[4] = vcd->delayParams.uncapFeedback ?
            vcd->presetKnobValues[Delay][4] * 1.1f :
            LEAF_clip(0.0f, vcd->presetKnobValues[Delay][4] * 1.1f, 0.9f);
            
            vcd->displayValues[5] = vcd->presetKnobValues[Delay][5];
        }

        // Add stereo param so in1 -> out1 and in2 -> out2 instead of in1 -> out1 & out2 (like bitcrusher)?
        void SFXDelayTick(Vocodec* vcd, float* input)
        {
            vcd->displayValues[0] = vcd->presetKnobValues[Delay][0] * 30000.0f;
            vcd->displayValues[1] = vcd->presetKnobValues[Delay][1] * 30000.0f;
            float cutoff1 = LEAF_clip(10.0f,
                                      faster_mtof((vcd->presetKnobValues[Delay][2] * 133) + 3.0f),
                                      20000.0f);
            float cutoff2 = LEAF_clip(10.0f,
                                      faster_mtof((vcd->presetKnobValues[Delay][3] * 133) + 3.0f),
                                      20000.0f);
            vcd->displayValues[2] = cutoff1;
            vcd->displayValues[3] = cutoff2;

            vcd->displayValues[4] = vcd->delayParams.uncapFeedback ?
            vcd->presetKnobValues[Delay][4] * 1.1f :
            LEAF_clip(0.0f, vcd->presetKnobValues[Delay][4] * 1.1f, 0.9f);

            vcd->displayValues[5] = vcd->presetKnobValues[Delay][5];
            
            tSVF_setFreq(vcd->delayHP, vcd->displayValues[2]);
            tSVF_setFreq(vcd->delayHP2, vcd->displayValues[2]);
            tSVF_setFreq(vcd->delayLP, vcd->displayValues[3]);
            tSVF_setFreq(vcd->delayLP2, vcd->displayValues[3]);
            
            //swap tanh for shaper and add cheap fixed highpass after both shapers
            
            float input1, input2;
            
            if (vcd->delayParams.shaper == 0)
            {
                input1 = tFeedbackLeveler_tick(vcd->feedbackControl, tanhf(input[1] + (vcd->delayFB1 * vcd->displayValues[4])));
                input2 = tFeedbackLeveler_tick(vcd->feedbackControl, tanhf(input[1] + (vcd->delayFB2 * vcd->displayValues[4])));
            }
            else
            {
                input1 = tFeedbackLeveler_tick(vcd->feedbackControl, tHighpass_tick(vcd->delayShaperHp, LEAF_shaper(input[1] + (vcd->delayFB1 * vcd->displayValues[4] * 0.5f), 0.5f)));
                input2 = tFeedbackLeveler_tick(vcd->feedbackControl, tHighpass_tick(vcd->delayShaperHp, LEAF_shaper(input[1] + (vcd->delayFB2 * vcd->displayValues[4] * 0.5f), 0.5f)));
            }
            tTapeDelay_setDelay(vcd->delay, vcd->displayValues[0]);
            tTapeDelay_setDelay(vcd->delay2, vcd->displayValues[1]);
            
            if (!vcd->delayParams.freeze)
            {
                vcd->delayFB1 = tTapeDelay_tick(vcd->delay, input1);
                vcd->delayFB2 = tTapeDelay_tick(vcd->delay2, input2);
            }
            
            else
            {
                vcd->delayFB1 = tTapeDelay_tick(vcd->delay, vcd->delayFB1);
                vcd->delayFB2 = tTapeDelay_tick(vcd->delay2, vcd->delayFB2);
            }
            
            vcd->delayFB1 = tSVF_tick(vcd->delayLP, vcd->delayFB1);
            vcd->delayFB2 = tSVF_tick(vcd->delayLP2, vcd->delayFB2);
            
            vcd->delayFB1 = tanhf(tSVF_tick(vcd->delayHP, vcd->delayFB1));
            vcd->delayFB2 = tanhf(tSVF_tick(vcd->delayHP2, vcd->delayFB2));
            
            input[0] = vcd->delayFB1 * vcd->displayValues[5];
            input[1] = vcd->delayFB2 * vcd->displayValues[5];
            
        }
        
        void SFXDelayFree(Vocodec* vcd)
        {
            tTapeDelay_free(&vcd->delay);
            tTapeDelay_free(&vcd->delay2);
            tSVF_free(&vcd->delayLP);
            tSVF_free(&vcd->delayHP);
            tSVF_free(&vcd->delayLP2);
            tSVF_free(&vcd->delayHP2);
            
            tHighpass_free(&vcd->delayShaperHp);
            tFeedbackLeveler_free(&vcd->feedbackControl);
        }
        
        
        
        //reverb
        void SFXReverbAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            tDattorroReverb_initToPool(&vcd->reverb, &vcd->mediumPool);
            tExpSmooth_init(&vcd->sizeSmoother, 0.5f, 0.001f, &vcd->leaf);
            tDattorroReverb_setMix(vcd->reverb, 1.0f);
            vcd->leaf.clearOnAllocation = 0;
            setLED_A(vcd, 0);
            setLED_B(vcd, vcd->reverbParams.uncapFeedback);
            setLED_C(vcd, vcd->reverbParams.freeze);
        }
        
        void SFXReverbFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonB][ActionPress])
            {
                vcd->reverbParams.uncapFeedback = !vcd->reverbParams.uncapFeedback;
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(vcd, vcd->reverbParams.uncapFeedback);
            }
            if (vcd->buttonActionsSFX[ButtonC][ActionPress])
            {
                vcd->reverbParams.freeze = !vcd->reverbParams.freeze;
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                tDattorroReverb_setFreeze(vcd->reverb, vcd->reverbParams.freeze);
                setLED_C(vcd, vcd->reverbParams.freeze);
            }
            vcd->displayValues[0] = vcd->presetKnobValues[Reverb][0];
            vcd->displayValues[1] = faster_mtof(vcd->presetKnobValues[Reverb][1]*129.0f);
            tDattorroReverb_setFeedbackFilter(vcd->reverb, vcd->displayValues[1]);
            vcd->displayValues[2] =  faster_mtof(vcd->presetKnobValues[Reverb][2]*123.0f);
            tDattorroReverb_setHP(vcd->reverb, vcd->displayValues[2]);
            vcd->displayValues[3] = faster_mtof(vcd->presetKnobValues[Reverb][3]*129.0f);
            tDattorroReverb_setInputFilter(vcd->reverb, vcd->displayValues[3]);
            vcd->displayValues[4] = vcd->reverbParams.uncapFeedback ?
            vcd->presetKnobValues[Reverb][4] :
            LEAF_clip(0.0f, vcd->presetKnobValues[Reverb][4], 0.5f);
        }
        
        // Add stereo param so in1 + in2 -> out1 & out2 instead of in1 -> out1 & out2 ?
        void SFXReverbTick(Vocodec* vcd, float* input)
        {
            float stereo[2];
            float sample = 0.0f;
            
            //tDattorroReverb_setInputDelay(&reverb, smoothedADC[1] * 200.f);
            input[1] *= 4.0f;
            vcd->displayValues[0] = vcd->presetKnobValues[Reverb][0];
            tExpSmooth_setDest(vcd->sizeSmoother, (vcd->displayValues[0] * 0.9f) + 0.1f);
            float tempSize = tExpSmooth_tick(vcd->sizeSmoother);
            tDattorroReverb_setSize(vcd->reverb, tempSize);

            vcd->displayValues[4] = vcd->reverbParams.uncapFeedback ?
            vcd->presetKnobValues[Reverb][4] :
            LEAF_clip(0.0f, vcd->presetKnobValues[Reverb][4], 0.5f);

            tDattorroReverb_setFeedbackGain(vcd->reverb, vcd->displayValues[4]);
            tDattorroReverb_tickStereo(vcd->reverb, input[1], stereo);
            sample = tanhf(stereo[0]) * 0.99f;
            input[0] = sample;
            input[1] = tanhf(stereo[1]) * 0.99f;
        }
        
        void SFXReverbFree(Vocodec* vcd)
        {
            tDattorroReverb_free(&vcd->reverb);
            tExpSmooth_free(&vcd->sizeSmoother);
        }
        
        
        //reverb2
        void SFXReverb2Alloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            tNReverb_initToPool(&vcd->reverb2, 1.0f, &vcd->mediumPool);
            tNReverb_setMix(vcd->reverb2, 1.0f);
            tSVF_init(&vcd->lowpass, SVFTypeLowpass, 18000.0f, 0.75f, &vcd->leaf);
            tSVF_init(&vcd->highpass, SVFTypeHighpass, 40.0f, 0.75f, &vcd->leaf);
            tSVF_init(&vcd->bandpass, SVFTypeBandpass, 2000.0f, 1.0f, &vcd->leaf);
            tSVF_init(&vcd->lowpass2, SVFTypeLowpass, 18000.0f, 0.75f, &vcd->leaf);
            tSVF_init(&vcd->highpass2, SVFTypeHighpass, 40.0f, 0.75f, &vcd->leaf);
            tSVF_init(&vcd->bandpass2, SVFTypeBandpass, 2000.0f, 1.0f, &vcd->leaf);
            vcd->leaf.clearOnAllocation = 0;
            setLED_A(vcd, 0);
            setLED_B(vcd, 0);
            setLED_C(vcd, vcd->reverb2Params.freeze);
        }
        
        void SFXReverb2Frame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonC][ActionPress])
            {
                vcd->reverb2Params.freeze = !vcd->reverb2Params.freeze;
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vcd, vcd->reverb2Params.freeze);
            }
            
            vcd->displayValues[0] = vcd->presetKnobValues[Reverb2][0] * 4.0f;
            vcd->displayValues[1] = faster_mtof(vcd->presetKnobValues[Reverb2][1]*135.0f);
            vcd->displayValues[2] = faster_mtof(vcd->presetKnobValues[Reverb2][2]*128.0f);
            vcd->displayValues[3] = faster_mtof(vcd->presetKnobValues[Reverb2][3]*128.0f);
            vcd->displayValues[4] = (vcd->presetKnobValues[Reverb2][4] * 4.0f) - 2.0f;
        }
        
        void SFXReverb2Tick(Vocodec* vcd, float* input)
        {
            float stereoOuts[2];
            float sample = 0.0f;
            vcd->displayValues[0] = vcd->presetKnobValues[Reverb2][0] * 4.0f;
            if (!vcd->reverb2Params.freeze)
            {
                tNReverb_setT60(vcd->reverb2, vcd->displayValues[0]);
            }
            else
            {
                tNReverb_setT60(vcd->reverb2, 1000.0f);
                input[1] = 0.0f;
            }
            
            vcd->displayValues[1] = faster_mtof(vcd->presetKnobValues[Reverb2][1]*135.0f);
            tSVF_setFreq(vcd->lowpass, vcd->displayValues[1]);
            tSVF_setFreq(vcd->lowpass2, vcd->displayValues[1]);
            vcd->displayValues[2] = faster_mtof(vcd->presetKnobValues[Reverb2][2]*128.0f);
            tSVF_setFreq(vcd->highpass, vcd->displayValues[2]);
            tSVF_setFreq(vcd->highpass2, vcd->displayValues[2]);
            vcd->displayValues[3] = faster_mtof(vcd->presetKnobValues[Reverb2][3]*128.0f);
            tSVF_setFreq(vcd->bandpass, vcd->displayValues[3]);
            tSVF_setFreq(vcd->bandpass2, vcd->displayValues[3]);
            
            vcd->displayValues[4] = (vcd->presetKnobValues[Reverb2][4] * 4.0f) - 2.0f;
            
            tNReverb_tickStereo(vcd->reverb2, input[1], stereoOuts);
            float leftOut = tSVF_tick(vcd->lowpass, stereoOuts[0]);
            leftOut = tSVF_tick(vcd->highpass, leftOut);
            leftOut += tSVF_tick(vcd->bandpass, leftOut) * vcd->displayValues[4];
            
            float rightOutTemp = tSVF_tick(vcd->lowpass2, stereoOuts[1]);
            rightOutTemp = tSVF_tick(vcd->highpass2, rightOutTemp);
            rightOutTemp += tSVF_tick(vcd->bandpass, rightOutTemp) * vcd->displayValues[4];
            sample = tanhf(leftOut);
            input[0] = sample;
            input[1] = tanhf(rightOutTemp);
            
        }
        
        void SFXReverb2Free(Vocodec* vcd)
        {
            tNReverb_free(&vcd->reverb2);
            tSVF_free(&vcd->lowpass);
            tSVF_free(&vcd->highpass);
            tSVF_free(&vcd->bandpass);
            tSVF_free(&vcd->lowpass2);
            tSVF_free(&vcd->highpass2);
            tSVF_free(&vcd->bandpass2);
        }
        
        
        //Living String
        void SFXLivingStringAlloc(Vocodec* vcd)
        {
            tSimplePoly_setNumVoices(vcd->poly, NUM_STRINGS);
            for (int i = 0; i < NUM_STRINGS; i++)
            {
                vcd->myDetune[i] = (vcd->leaf.random() * 0.3f) - 0.15f;
                //tComplexLivingString_init(theString[i],  myFreq, 0.4f, 0.0f, 16000.0f, .999f, .5f, .5f, 0.1f, 0);
                tComplexLivingString_initToPool(&vcd->theString[i], 440.f, 0.8f, 0.3f, 0.f, 9000.f, 1.0f, 0.3f, 0.01f, 0.125f, vcd->livingStringParams.feedback, &vcd->mediumPool);
                tExpSmooth_init(&vcd->stringGains[i], 0.0f, 0.002f, &vcd->leaf);
            }
            setLED_A(vcd, vcd->livingStringParams.ignoreFreqKnobs);
            setLED_B(vcd, vcd->livingStringParams.independentStrings);
            setLED_C(vcd, vcd->livingStringParams.feedback);
            
        }
        
        void SFXLivingStringFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                vcd->livingStringParams.ignoreFreqKnobs = !vcd->livingStringParams.ignoreFreqKnobs;
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->livingStringParams.ignoreFreqKnobs);
            }
            if (vcd->buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                vcd->livingStringParams.feedback = !vcd->livingStringParams.feedback;
                for (int i = 0; i < NUM_STRINGS; i++)
                {
                    tComplexLivingString_setLevMode(vcd->theString[i], vcd->livingStringParams.feedback);
                }
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vcd, vcd->livingStringParams.feedback);
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                vcd->livingStringParams.independentStrings = !vcd->livingStringParams.independentStrings;
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(vcd, vcd->livingStringParams.independentStrings);
            }
            vcd->displayValues[0] = LEAF_midiToFrequency((vcd->presetKnobValues[LivingString][0] * 90.0f)); //freq
            vcd->displayValues[1] = vcd->presetKnobValues[LivingString][1]; //detune
            vcd->displayValues[2] = vcd->presetKnobValues[LivingString][2]; //decay
            vcd->displayValues[3] = mtof((vcd->presetKnobValues[LivingString][3] * 130.0f)+12.0f); //lowpass
            vcd->displayValues[4] = (vcd->presetKnobValues[LivingString][4] * 0.48f) + 0.5f;//pickPos
            vcd->displayValues[5] = (vcd->presetKnobValues[LivingString][5] * 0.48f) + 0.02f;//prepPos
            vcd->displayValues[6] = ((tanhf((vcd->presetKnobValues[LivingString][6] * 8.0f) - 4.0f)) * 0.5f) + 0.5f;//prep Index
            vcd->displayValues[7] = vcd->presetKnobValues[LivingString][7];// let ring
            
            if (!vcd->livingStringParams.independentStrings)
            {
                if (!vcd->livingStringParams.ignoreFreqKnobs)
                {
                    
                    
                    
                    for (int i = 0; i < NUM_STRINGS; i++)
                    {
                        float freqVal = vcd->displayValues[0] * (i+1);
                        tComplexLivingString_setFreq(vcd->theString[i], (1.0f + (vcd->myDetune[i] * vcd->displayValues[1])) * freqVal);
                        tComplexLivingString_setDecay(vcd->theString[i], (vcd->displayValues[2] * 0.015f) + 0.995f);
                        tComplexLivingString_setDampFreq(vcd->theString[i], vcd->displayValues[3]);
                        tComplexLivingString_setPickPos(vcd->theString[i], vcd->displayValues[4]);
                        tComplexLivingString_setPrepPos(vcd->theString[i], vcd->displayValues[5]);
                        tComplexLivingString_setPrepIndex(vcd->theString[i], vcd->displayValues[6]);
                        tExpSmooth_setDest(vcd->stringGains[i], 1.0f);
                    }
                }
                else
                {
                    for (int i = 0; i < NUM_STRINGS; i++)
                    {
                        
                        calculateFreq(vcd, i);
                        float freqVal = vcd->freq[i];
                        tComplexLivingString_setFreq(vcd->theString[i], (1.0f + (vcd->myDetune[i] * vcd->displayValues[1])) * freqVal);
                        tComplexLivingString_setDecay(vcd->theString[i], (vcd->displayValues[2] * 0.015f) + 0.995f);
                        tComplexLivingString_setDampFreq(vcd->theString[i], vcd->displayValues[3]);
                        tComplexLivingString_setPickPos(vcd->theString[i], vcd->displayValues[4]);
                        tComplexLivingString_setPrepPos(vcd->theString[i], vcd->displayValues[5]);
                        tComplexLivingString_setPrepIndex(vcd->theString[i], vcd->displayValues[6]);
                        if (tSimplePoly_isOn(vcd->poly, i))
                        {
                            tExpSmooth_setDest(vcd->stringGains[i], 1.0f);
                        }
                        else
                        {
                            tExpSmooth_setDest(vcd->stringGains[i], vcd->displayValues[7]);
                        }
                    }
                }
            }
            else
            {
                vcd->displayValues[10] = LEAF_midiToFrequency((vcd->presetKnobValues[LivingString][10] * 90.0f)); //freq
                vcd->displayValues[11] = LEAF_midiToFrequency((vcd->presetKnobValues[LivingString][11] * 90.0f)); //freq
                vcd->displayValues[12] = LEAF_midiToFrequency((vcd->presetKnobValues[LivingString][12] * 90.0f)); //freq
                vcd->displayValues[13] = LEAF_midiToFrequency((vcd->presetKnobValues[LivingString][13] * 90.0f)); //freq
                vcd->displayValues[14] = LEAF_midiToFrequency((vcd->presetKnobValues[LivingString][14] * 90.0f)); //freq
                
                for (int i = 0; i < NUM_STRINGS; i++)
                {
                    float freqVal = i == 0 ? vcd->displayValues[0] : vcd->displayValues[9+i];
                    tComplexLivingString_setFreq(vcd->theString[i], (1.0f + (vcd->myDetune[i] * vcd->displayValues[1])) * freqVal);
                    tComplexLivingString_setDecay(vcd->theString[i], (vcd->displayValues[2] * 0.015f) + 0.995f);
                    tComplexLivingString_setDampFreq(vcd->theString[i], vcd->displayValues[3]);
                    tComplexLivingString_setPickPos(vcd->theString[i], vcd->displayValues[4]);
                    tComplexLivingString_setPrepPos(vcd->theString[i], vcd->displayValues[5]);
                    tComplexLivingString_setPrepIndex(vcd->theString[i], vcd->displayValues[6]);
                    tExpSmooth_setDest(vcd->stringGains[i], 1.0f);
                }
            }
        }
        
        
        void SFXLivingStringTick(Vocodec* vcd, float* input)
        {
            float sample = 0.0f;
            for (int i = 0; i < NUM_STRINGS; i++)
            {
                float tick = tComplexLivingString_tick(vcd->theString[i], tanhf(input[1]));
                sample += tick * tExpSmooth_tick(vcd->stringGains[i]);
            }
            sample *= 0.1625f;
            input[0] = sample;
            input[1] = sample;
        }
        
        void SFXLivingStringFree(Vocodec* vcd)
        {
            for (int i = 0; i < NUM_STRINGS; i++)
            {
                tComplexLivingString_free(&vcd->theString[i]);
                tExpSmooth_free(&vcd->stringGains[i]);
            }
        }
        
        
        //Living String Synth
        void SFXLivingStringSynthAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 0;
            tSimplePoly_setNumVoices(vcd->poly, vcd->livingStringSynthParams.numVoices);
            for (int i = 0; i < NUM_STRINGS_SYNTH; i++)
            {
                tLivingString2_initToPool(&vcd->theString2[i], 440.f, 0.9f, 0.6f, 1.0f, 0.0f, 1.0f, 0.999f, .99f, 0.01f, 0.125f, vcd->livingStringSynthParams.feedback, &vcd->largePool);
                tSlide_init(&vcd->stringOutEnvs[i], 10.0f, 1000.0f, &vcd->leaf);
                tSlide_init(&vcd->stringInEnvs[i], 12.0f, 1000.0f, &vcd->leaf);
                tADSRT_init(&vcd->pluckEnvs[i], 4.0f, 70.0f, 0.0f, 5.0f, vcd->decayExpBuffer, DECAY_EXP_BUFFER_SIZE, &vcd->leaf);
                tEnvelopeFollower_init(&vcd->prepEnvs[i], 0.001f, 0.9999f, &vcd->leaf);
            }
            tVZFilter_init(&vcd->pluckFilt, BandpassPeak, 2000.0f, 4.0f, &vcd->leaf);
            tNoise_init(&vcd->stringPluckNoise, WhiteNoise, &vcd->leaf);
            setLED_A(vcd, vcd->livingStringSynthParams.numVoices == 1);
            setLED_B(vcd, vcd->livingStringSynthParams.audioIn);
            setLED_C(vcd, vcd->livingStringSynthParams.feedback);
            vcd->samplesPerMs = vcd->leaf.sampleRate / 1000.0f;
#ifdef __cplusplus
            tExpSmooth_init(&vcd->pickPosSmooth, 0.5f, 0.0001f, &vcd->leaf);
            tExpSmooth_init(&vcd->prepPosSmooth, 0.5f, 0.0001f, &vcd->leaf);
            tExpSmooth_init(&vcd->pickupPosSmooth, 0.5f, 0.0001f, &vcd->leaf);
#endif

        }
        
        void SFXLivingStringSynthFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                vcd->livingStringSynthParams.numVoices = (vcd->livingStringSynthParams.numVoices > 1) ? 1 : NUM_STRINGS_SYNTH;
                tSimplePoly_setNumVoices(vcd->poly, vcd->livingStringSynthParams.numVoices);
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->livingStringSynthParams.numVoices == 1);
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                vcd->livingStringSynthParams.audioIn = !vcd->livingStringSynthParams.audioIn;
                
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(vcd, vcd->livingStringSynthParams.audioIn);
            }
            
            if (vcd->buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                vcd->livingStringSynthParams.feedback = !vcd->livingStringSynthParams.feedback;
                for (int i = 0; i < NUM_STRINGS_SYNTH; i++)
                {
                    tLivingString2_setLevMode(vcd->theString2[i], vcd->livingStringSynthParams.feedback);
                }
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vcd, vcd->livingStringSynthParams.feedback);
            }
            
            
            vcd->displayValues[0] = vcd->presetKnobValues[LivingStringSynth][0] * 10.0f; //pluck volume
            vcd->displayValues[1] = vcd->presetKnobValues[LivingStringSynth][1]; //lowpass
            vcd->displayValues[2] = vcd->presetKnobValues[LivingStringSynth][2]; //decay
            vcd->displayValues[3] = vcd->presetKnobValues[LivingStringSynth][3]; //brightness
            vcd->displayValues[4] = (vcd->presetKnobValues[LivingStringSynth][4] * 0.9f) + 0.05f;//pick Pos
            vcd->displayValues[5] = (vcd->presetKnobValues[LivingStringSynth][5] * 0.9f) + 0.05f;//prep Pos
            vcd->displayValues[6] = ((LEAF_tanh((vcd->presetKnobValues[LivingStringSynth][6] * 8.5f) - 4.25f)) * 0.5f) + 0.5f;//prep Index
            vcd->displayValues[7] = vcd->presetKnobValues[LivingStringSynth][7];//let Ring
            vcd->displayValues[8] = vcd->presetKnobValues[LivingStringSynth][8] * 0.1f;//feedback level
            vcd->displayValues[9] = vcd->expBuffer[(int)(vcd->presetKnobValues[LivingStringSynth][9] * vcd->expBufferSizeMinusOne)] * 8192.0f;//release time
            vcd->displayValues[10] = vcd->presetKnobValues[LivingStringSynth][10]; //preparation removal amount (hot potato style)
            vcd->displayValues[11] = vcd->presetKnobValues[LivingStringSynth][11]; //pickup position
#ifndef __cplusplus
            for (int i = 0; i < NUM_STRINGS_SYNTH; i++)
            {
                //tComplexLivingString_setFreq(theString[i], (i + (1.0f+(myDetune[i] * knobParams[1]))) * knobParams[0]);
                tLivingString2_setDecay(vcd->theString2[i], ((vcd->displayValues[2]  * 10.0f) + 0.01f));
                tLivingString2_setBrightness(vcd->theString2[i], vcd->displayValues[3]);
                tLivingString2_setPickPos(vcd->theString2[i], vcd->displayValues[4]);
                tLivingString2_setPrepPos(vcd->theString2[i], vcd->displayValues[5]);
                //tLivingString2_setPrepIndex(vcd->theString2[i], vcd->displayValues[6]);
                tSlide_setDownSlide(vcd->stringOutEnvs[i], vcd->displayValues[9] * vcd->samplesPerMs);
                //tADSRT_setDecay(pluckEnvs[i], displayValues[9]);

            }
#else
            for (int i = 0; i < NUM_STRINGS_SYNTH; i++)
            {
                //tComplexLivingString_setFreq(theString[i], (i + (1.0f+(myDetune[i] * knobParams[1]))) * knobParams[0]);
                tLivingString2_setDecay(vcd->theString2[i], ((vcd->displayValues[2]  * 10.0f) + 0.01f));
                tLivingString2_setBrightness(vcd->theString2[i], vcd->displayValues[3]);

                tSlide_setDownSlide(vcd->stringOutEnvs[i], vcd->displayValues[9] * vcd->samplesPerMs);
                //tADSRT_setDecay(pluckEnvs[i], displayValues[9]);
                
            }
            tExpSmooth_setDest(vcd->pickPosSmooth, vcd->displayValues[4]);
            tExpSmooth_setDest(vcd->prepPosSmooth, vcd->displayValues[5]);
            tExpSmooth_setDest(vcd->pickupPosSmooth, vcd->displayValues[11]);
#endif
            tVZFilter_setFreq(vcd->pluckFilt, faster_mtof((vcd->displayValues[1] * 100.0f)+20.0f));
            for (int i = 0; i < tSimplePoly_getNumVoices(vcd->poly); i++)
            {
                //tRamp_setDest(polyRamp[i], (tPoly_getVelocity(poly, i) > 0));
                calculateFreq(vcd, i);

                //tComplexLivingString_setDampFreq(theString[i], LEAF_clip(40.0f, freq[i] + displayValues[3], 23000.0f));
                float voiceOn = (tSimplePoly_getVelocity(vcd->poly, i) > 0);
                if (vcd->livingStringSynthParams.feedback)
                {
                    tLivingString2_setTargetLev(vcd->theString2[i], voiceOn * vcd->displayValues[8]);
                }
                else
                {
                    tLivingString2_setTargetLev(vcd->theString2[i],1.0f);
                }
                if (voiceOn)
                {
                    tSlide_setDest(vcd->stringOutEnvs[i], 1.0f);
                    tSlide_setDest(vcd->stringInEnvs[i], 1.0f);
                }
                else
                {
                    tSlide_setDest(vcd->stringOutEnvs[i], vcd->displayValues[7]);
                    tSlide_setDest(vcd->stringInEnvs[i], 0.0f);
                }
#ifndef __cplusplus
                tLivingString2_setFreq(vcd->theString2[i], vcd->freq[i]);
                tLivingString2_updateDelays(vcd->theString2[i]);

#endif
                
            }
        }
        
        void SFXLivingStringSynthTick(Vocodec* vcd, float* input)
        {
            float sample = 0.0f;
            
            float inputSample = 0.0f;
            //float pluck = (displayValues[1] * tNoise_tick(stringPluckNoise)) + ((1.0f - displayValues[1]) * tNoise_tick(stringPluckNoiseDark));
            float pluck = vcd->displayValues[0] * tNoise_tick(vcd->stringPluckNoise);
            pluck = tVZFilter_tick(vcd->pluckFilt, pluck);
           #ifdef __cplusplus
            
            float pickPos = tExpSmooth_tick(vcd->pickPosSmooth);
            float prepPos =tExpSmooth_tick(vcd->prepPosSmooth);
            float pickupPos = tExpSmooth_tick(vcd->pickupPosSmooth);
            for (int i = 0; i < NUM_STRINGS_SYNTH; i++)
            {
                tLivingString2_setPickPos(vcd->theString2[i], pickPos);
                tLivingString2_setPrepPos(vcd->theString2[i], prepPos);
                tLivingString2_setPickupPos(vcd->theString2[i], pickupPos);
            }
            #endif
            for (int i = 0; i < NUM_STRINGS_SYNTH; i++)
            {
                //float pluck = tNoise_tick(stringPluckNoise);


#ifdef __cplusplus
                tLivingString2_setFreq(vcd->theString2[i], vcd->freq[i]);
                float envelopeVal = tADSRT_tick(vcd->pluckEnvs[i]);
                float prepIndexToSend = (vcd->displayValues[6] * tEnvelopeFollower_tick(vcd->prepEnvs[i], envelopeVal) * vcd->displayValues[10]) + ((1.0f - vcd->displayValues[10]) * vcd->displayValues[6]);
                tLivingString2_setPrepIndex(vcd->theString2[i], prepIndexToSend);

                inputSample = tanhf((input[1] * vcd->livingStringSynthParams.audioIn) + (pluck * envelopeVal));// + (prevSamp * 0.001f);
                //TODO:
                //inputSample = (input[1] * voicePluck) + (tVZFilter_tick(pluckFilt, (tNoise_tick(stringPluckNoise))) * tADSR4_tick(pluckEnvs[i]));
                sample += tLivingString2_tick(vcd->theString2[i], ((inputSample * tSlide_tickNoInput(vcd->stringOutEnvs[i])))) * tSlide_tickNoInput(vcd->stringOutEnvs[i]);

 #else
                float envelopeVal = tADSRT_tick(vcd->pluckEnvs[i]);
                float prepIndexToSend = (vcd->displayValues[6] * tEnvelopeFollower_tick(vcd->prepEnvs[i], envelopeVal) * vcd->displayValues[10]) + ((1.0f - vcd->displayValues[10]) * vcd->displayValues[6]);
                tLivingString2_setPrepIndex(vcd->theString2[i], prepIndexToSend);
                inputSample = fast_tanh((input[1] * vcd->livingStringSynthParams.audioIn) + (pluck * envelopeVal));// + (prevSamp * 0.001f);
                //TODO:
                //inputSample = (input[1] * voicePluck) + (tVZFilter_tick(pluckFilt, (tNoise_tick(stringPluckNoise))) * tADSR4_tick(pluckEnvs[i]));
                sample += tLivingString2_tickEfficient(vcd->theString2[i], ((inputSample * tSlide_tickNoInput(vcd->stringOutEnvs[i])))) * tSlide_tickNoInput(vcd->stringOutEnvs[i]);
                
 #endif


            }
            sample *= 0.25f;
#ifndef __cplusplus
            sample = fast_tanh(sample) * 0.98f;
#else
             sample = tanhf(sample) * 0.98f;
#endif
            input[0] = sample;
            input[1] = sample;
        }
        
        void SFXLivingStringSynthFree(Vocodec* vcd)
        {
            for (int i = 0; i < NUM_STRINGS_SYNTH; i++)
            {
                tLivingString2_free(&vcd->theString2[i]);
                tSlide_free(&vcd->stringOutEnvs[i]);
                tSlide_free(&vcd->stringInEnvs[i]);
                tADSRT_free(&vcd->pluckEnvs[i]);
                tEnvelopeFollower_free(&vcd->prepEnvs[i]);
            }
            tVZFilter_free(&vcd->pluckFilt);
            tNoise_free(&vcd->stringPluckNoise);
#ifdef __cplusplus
            tExpSmooth_free(&vcd->pickPosSmooth);
            tExpSmooth_free(&vcd->prepPosSmooth);
            tExpSmooth_free(&vcd->pickupPosSmooth);
#endif
        }
        
        
        // CLASSIC SUBTRACTIVE SYNTH
        void SFXClassicSynthAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            tSimplePoly_setNumVoices(vcd->poly, vcd->classicSynthParams.numVoices);
            
            float* knobs = vcd->presetKnobValues[ClassicSynth];
            
            vcd->displayValues[0] = knobs[0]; //synth volume
            
            vcd->displayValues[1] = knobs[1] * 134.0f; //lowpass cutoff
            
            vcd->displayValues[2] = knobs[2]; //keyfollow filter cutoff
            
            vcd->displayValues[3] = knobs[3]; //detune
            
            vcd->displayValues[4] = (knobs[4] * 2.0f) + 0.4f; //filter Q
            
            vcd->displayValues[5] = vcd->expBuffer[(int)(knobs[5] * vcd->expBufferSizeMinusOne)] * 8192.0f; //att
            
            vcd->displayValues[6] = vcd->expBuffer[(int)(knobs[6] * vcd->expBufferSizeMinusOne)] * 8192.0f; //dec
            
            vcd->displayValues[7] = knobs[7]; //sus
            
            vcd->displayValues[8] = vcd->expBuffer[(int)(knobs[8] * vcd->expBufferSizeMinusOne)] * 8192.0f; //rel
            
            vcd->displayValues[9] = knobs[9]; //leak
            
            vcd->displayValues[10] = vcd->expBuffer[(int)(knobs[10] * vcd->expBufferSizeMinusOne)] * 8192.0f; //att
            
            vcd->displayValues[11] = vcd->expBuffer[(int)(knobs[11] * vcd->expBufferSizeMinusOne)] * 8192.0f; //dec
            
            vcd->displayValues[12] = knobs[12]; //sus
            
            vcd->displayValues[13] = vcd->expBuffer[(int)(knobs[13] * vcd->expBufferSizeMinusOne)] * 8192.0f; //rel
            
            vcd->displayValues[14] = knobs[14]; //leak
            
            vcd->displayValues[15] = knobs[15] * 134.0f;  // filter envelope amount
            
            vcd->displayValues[16] = knobs[16];  // fade between sawtooth and glottal pulse
            
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
                {
                    int k = (i * NUM_OSC_PER_VOICE) + j;
                    tSawtooth_init(&vcd->osc[k], &vcd->leaf);
                    vcd->synthDetune[i][j] = ((vcd->leaf.random() * 0.0264f) - 0.0132f);
                    tRosenbergGlottalPulse_init(&vcd->glottal[k], &vcd->leaf);
                    tRosenbergGlottalPulse_setOpenLength(vcd->glottal[k], 0.3f);
                    tRosenbergGlottalPulse_setPulseLength(vcd->glottal[k], 0.4f);
                }
                
                tSVF_LP_init(&vcd->synthLP[i], 2000.0f, vcd->displayValues[4], &vcd->leaf);
                tADSRT_init(&vcd->polyEnvs[i], vcd->displayValues[5], vcd->displayValues[6], vcd->displayValues[7], vcd->displayValues[8], vcd->decayExpBuffer, DECAY_EXP_BUFFER_SIZE, &vcd->leaf);
                tADSRT_setLeakFactor(vcd->polyEnvs[i],((1.0f - vcd->displayValues[9]) * 0.00005f) + 0.99995f);
                tADSRT_init(&vcd->polyFiltEnvs[i], vcd->displayValues[10], vcd->displayValues[11], vcd->displayValues[12], vcd->displayValues[13], vcd->decayExpBuffer, DECAY_EXP_BUFFER_SIZE, &vcd->leaf);
                tADSRT_setLeakFactor(vcd->polyFiltEnvs[i], ((1.0f - vcd->displayValues[14]) * 0.00005f) + 0.99995f);

            }
            tCycle_init(&vcd->pwmLFO1, &vcd->leaf);
            tCycle_init(&vcd->pwmLFO2, &vcd->leaf);
#ifdef __cplusplus
            tCycle_setFreq(&vcd->pwmLFO1, 0.63f);
            tCycle_setFreq(&vcd->pwmLFO2, 0.7211f);
#else
            tCycle_setFreq(vcd->pwmLFO1, 63.0f);//ticked in frame, so needs to be 128 times slower than if ticked in tick
            tCycle_setFreq(vcd->pwmLFO2, 72.011f);//ticked in frame, so needs to be 128 times slower than if ticked in tick
#endif

            vcd->leaf.clearOnAllocation = 0;
            //            cycleCountVals[0][2] = 2;
            setLED_A(vcd, vcd->classicSynthParams.numVoices == 1);
            setLED_B(vcd, 0);
            setLED_C(vcd, 0);
        }
        
        void SFXClassicSynthFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                vcd->classicSynthParams.numVoices = (vcd->classicSynthParams.numVoices > 1) ? 1 : NUM_VOC_VOICES;
                tSimplePoly_setNumVoices(vcd->poly, vcd->classicSynthParams.numVoices);
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->classicSynthParams.numVoices == 1);
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                //                cycleCountVals[0][1] = 0;
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_Edit(vcd, 0);
            }
            
            float* knobs = vcd->presetKnobValues[ClassicSynth];
            
            if (vcd->writeKnobFlag != -1)
            {
                switch(vcd->writeKnobFlag + (vcd->knobPage * KNOB_PAGE_SIZE))
                {
                    case 0:
                        vcd->displayValues[0] = knobs[0]; //synth volume
                        break;
                    case 1:
                        vcd->displayValues[1] = knobs[1] * 134.0f; //lowpass cutoff
                        break;
                    case 2:
                        vcd->displayValues[2] = knobs[2]; //keyfollow filter cutoff
                        break;
                    case 3:
                        vcd->displayValues[3] = knobs[3]; //detune
                        break;
                    case 4:
                        vcd->displayValues[4] = (knobs[4] * 2.0f) + 0.4f; //filter Q
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                        	tSVF_LP_setQ(vcd->synthLP[i], vcd->displayValues[4]);
                        }
                        break;
                    case 5:
                        vcd->displayValues[5] = vcd->expBuffer[(int)(knobs[5] * vcd->expBufferSizeMinusOne)] * 8192.0f; //att
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                            tADSRT_setAttack(vcd->polyEnvs[i], vcd->displayValues[5]);
                        }
                        break;
                    case 6:
                        vcd->displayValues[6] = vcd->expBuffer[(int)(knobs[6] * vcd->expBufferSizeMinusOne)] * 8192.0f; //dec
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                            tADSRT_setDecay(vcd->polyEnvs[i], vcd->displayValues[6]);
                        }
                        break;
                    case 7:
                        vcd->displayValues[7] = knobs[7]; //sus
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                            tADSRT_setSustain(vcd->polyEnvs[i], vcd->displayValues[7]);
                        }
                        break;
                    case 8:
                        vcd->displayValues[8] = vcd->expBuffer[(int)(knobs[8] * vcd->expBufferSizeMinusOne)] * 8192.0f; //rel
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                            tADSRT_setRelease(vcd->polyEnvs[i], vcd->displayValues[8]);
                        }
                        break;
                    case 9:
                        vcd->displayValues[9] = knobs[9]; //leak
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                            tADSRT_setLeakFactor(vcd->polyEnvs[i], ((1.0f - vcd->displayValues[9]) * 0.00005f) + 0.99995f);
                        }
                        break;
                    case 10:
                        vcd->displayValues[10] = vcd->expBuffer[(int)(knobs[10] * vcd->expBufferSizeMinusOne)] * 8192.0f; //att
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                            tADSRT_setAttack(vcd->polyFiltEnvs[i], vcd->displayValues[10]);
                        }
                        break;
                    case 11:
                        vcd->displayValues[11] = vcd->expBuffer[(int)(knobs[11] * vcd->expBufferSizeMinusOne)] * 8192.0f; //dec
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                            tADSRT_setDecay(vcd->polyFiltEnvs[i], vcd->displayValues[11]);
                        }
                        break;
                    case 12:
                        vcd->displayValues[12] = knobs[12]; //sus
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                            tADSRT_setSustain(vcd->polyFiltEnvs[i], vcd->displayValues[12]);
                        }
                        break;
                    case 13:
                        vcd->displayValues[13] = vcd->expBuffer[(int)(knobs[13] * vcd->expBufferSizeMinusOne)] * 8192.0f; //rel
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                            tADSRT_setRelease(vcd->polyFiltEnvs[i], vcd->displayValues[13]);
                        }
                        break;
                    case 14:
                        vcd->displayValues[14] = knobs[14]; //leak
                        for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
                        {
                            tADSRT_setLeakFactor(vcd->polyFiltEnvs[i], ((1.0f - vcd->displayValues[14]) * 0.00005f) + 0.99995f);
                        }
                        break;
                    case 15:
                        vcd->displayValues[15] = knobs[15] * 4095.0f;  // filter envelope amount
                        break;
                    case 16:
                        vcd->displayValues[16] = knobs[16];  // fade between sawtooth and glottal pulse
                        break;
                }
            }
#ifndef __cplusplus
            float tempLFO1 = (tCycle_tick(vcd->pwmLFO1) * 0.25f) + 0.5f; // pulse length
            float tempLFO2 = ((tCycle_tick(vcd->pwmLFO2) * 0.25f) + 0.5f) * tempLFO1; // open length
            
            for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
            {
                float myMidiNote = tSimplePoly_getPitch(vcd->poly, i);
                
                calculateFreq(vcd, i);
                
                
                for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
                {
                    float tempFreq = vcd->freq[i] * (1.0f + (vcd->synthDetune[i][j] * vcd->displayValues[3]));
                    tSawtooth_setFreq(vcd->osc[(i * NUM_OSC_PER_VOICE) + j], tempFreq);
                    tRosenbergGlottalPulse_setFreq(vcd->glottal[(i * NUM_OSC_PER_VOICE) + j], tempFreq);
                    tRosenbergGlottalPulse_setPulseLength(vcd->glottal[(i * NUM_OSC_PER_VOICE) + j], tempLFO1);
                    tRosenbergGlottalPulse_setOpenLength(vcd->glottal[(i * NUM_OSC_PER_VOICE) + j], tempLFO2);
                }
                
                
                float keyFollowFilt = myMidiNote * vcd->displayValues[2];
                float tempFreq2 = vcd->displayValues[1] +  keyFollowFilt;
                tempFreq2 = LEAF_clip(0.0f, tempFreq2, 134.0f);
                vcd->filtFreqs[i] = (uint16_t) tempFreq2;
                
                if (vcd->classicSynthParams.numVoices > 1)
                {
                    if (vcd->poly->voices[i][0] == -2)
                    {
                        if (vcd->polyEnvs[i]->whichStage == env_idle)
                        {
                            tSimplePoly_deactivateVoice(vcd->poly, i);
                        }
                    }
                }
            }
#endif
        }

        void SFXClassicSynthTick(Vocodec* vcd, float* input)
        {
            float sample = 0.0f;
#ifdef __cplusplus
            //==============================================================================
            // This was in frame. Needs to be in tick or we get a bit carry over pitch from the last note played
            float tempLFO1 = (tCycle_tick(vcd->pwmLFO1) * 0.25f) + 0.5f; // pulse length
            float tempLFO2 = ((tCycle_tick(vcd->pwmLFO2) * 0.25f) + 0.5f) * tempLFO1; // open length

            for (int i = 0; i < vcd->classicSynthParams.numVoices; i++)
            {
                float myMidiNote = tSimplePoly_getPitch(vcd->poly, i);

                calculateFreq(vcd, i);


                for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
                {
                    float tempFreq = vcd->freq[i] * (1.0f + (vcd->synthDetune[i][j] * vcd->displayValues[3]));
                    tSawtooth_setFreq(vcd->osc[(i * NUM_OSC_PER_VOICE) + j], tempFreq);
                    tRosenbergGlottalPulse_setFreq(vcd->glottal[(i * NUM_OSC_PER_VOICE) + j], tempFreq);
                    tRosenbergGlottalPulse_setPulseLength(vcd->glottal[(i * NUM_OSC_PER_VOICE) + j], tempLFO1);
                    tRosenbergGlottalPulse_setOpenLength(vcd->glottal[(i * NUM_OSC_PER_VOICE) + j], tempLFO2);
                }


                float keyFollowFilt = myMidiNote * vcd->displayValues[2] * 64.0f;
                float tempFreq2 = vcd->displayValues[1] +  keyFollowFilt;
                tempFreq2 = LEAF_clip(0.0f, tempFreq2, 4095.0f);
                vcd->filtFreqs[i] = (uint16_t) tempFreq2;

                if (vcd->classicSynthParams.numVoices > 1)
                {
                    if (vcd->poly->voices[i][0] == -2)
                    {
                        if (vcd->polyEnvs[i]->whichStage == env_idle)
                        {
                            tSimplePoly_deactivateVoice(vcd->poly, i);
                        }
                    }
                }
            }
#endif
            //==============================================================================
            
            for (int i = 0; i < tSimplePoly_getNumVoices(vcd->poly); i++)
            {
                float tempSample = 0.0f;
                float env = tADSRT_tick(vcd->polyEnvs[i]);
                
                for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
                {
                    tempSample += tSawtooth_tick(vcd->osc[(i * NUM_OSC_PER_VOICE) + j]) * env * (1.0f - vcd->displayValues[16]);
#ifndef __cplusplus
                    tempSample += tRosenbergGlottalPulse_tick(vcd->glottal[(i * NUM_OSC_PER_VOICE) + j]) * env * (vcd->displayValues[16]);
#else
                    tempSample += tRosenbergGlottalPulse_tickHQ(vcd->glottal[(i * NUM_OSC_PER_VOICE) + j]) * env * (vcd->displayValues[16]);
#endif
                }
                tSVF_LP_setFreqFast(vcd->synthLP[i], LEAF_clip(0.0f, (vcd->filtFreqs[i] + (vcd->displayValues[15] * tADSRT_tick(vcd->polyFiltEnvs[i]))), 134.0f));
                sample += tSVF_LP_tick(vcd->synthLP[i], tempSample);
            }
            sample *= INV_NUM_OSC_PER_VOICE * vcd->displayValues[0];
            
            
            sample = tanhf(sample);
            input[0] = sample;
            input[1] = sample;
        }

        void SFXClassicSynthFree(Vocodec* vcd)
        {
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
                {
                    tSawtooth_free(&vcd->osc[(i * NUM_OSC_PER_VOICE) + j]);
                    tRosenbergGlottalPulse_free(&vcd->glottal[(i * NUM_OSC_PER_VOICE) + j]);
                }
                tSVF_LP_free(&vcd->synthLP[i]);
                tADSRT_free(&vcd->polyEnvs[i]);
                tADSRT_free(&vcd->polyFiltEnvs[i]);
            }
            
            tCycle_free(&vcd->pwmLFO1);
            tCycle_free(&vcd->pwmLFO2);
        }
        
        
        
        ///FM RHODES ELECTRIC PIANO SYNTH
        void SFXRhodesAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                for (int j = 0; j < 6; j++)
                {
                    tCycle_init(&vcd->FM_sines[i][j], &vcd->leaf);
                    tADSRT_init(&vcd->FM_envs[i][j], 10, 1000, 0.5f, 100.0f, vcd->decayExpBuffer, DECAY_EXP_BUFFER_SIZE, &vcd->leaf);
                    tADSRT_setLeakFactor(vcd->FM_envs[i][j], 0.99998f);
                }
            }
            for (int i = 0; i < 6; i++)
            {
                tExpSmooth_init(&vcd->susSmoothers[i], 1.0f, 0.01f, &vcd->leaf);
            }
            tCycle_init(&vcd->tremolo, &vcd->leaf);
            tCycle_setFreq(vcd->tremolo, 3.0f);
            tSimplePoly_setNumVoices(vcd->poly, vcd->rhodesParams.numVoices);
            vcd->leaf.clearOnAllocation = 0;
            
            setLED_A(vcd, vcd->rhodesParams.numVoices == 1);
            setLED_B(vcd, 0);
            setLED_C(vcd, vcd->rhodesParams.tremoloStereo);
            OLEDclearLine(vcd, SecondLine);
            OLEDwriteString(vcd, vcd->soundNames[vcd->rhodesParams.sound], 6, 0, SecondLine);
        }
        
        void SFXRhodesFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                vcd->rhodesParams.numVoices = (vcd->rhodesParams.numVoices > 1) ? 1 : NUM_VOC_VOICES;
                tSimplePoly_setNumVoices(vcd->poly, vcd->rhodesParams.numVoices);
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->rhodesParams.numVoices == 1);
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                vcd->rhodesParams.sound = (vcd->rhodesParams.sound + 1 ) % 5; // switch to another rhodes preset sound
            }
            if (vcd->buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                vcd->rhodesParams.tremoloStereo = !vcd->rhodesParams.tremoloStereo;
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vcd, vcd->rhodesParams.tremoloStereo == 1);
                
            }
            
            vcd->displayValues[0] = vcd->presetKnobValues[Rhodes][0] * 4.0f; // brightness
            vcd->displayValues[1] = vcd->presetKnobValues[Rhodes][1]; // tremelo amount
            vcd->displayValues[2] = vcd->presetKnobValues[Rhodes][2] * 10.0f; //tremelo rate
            vcd->displayValues[3] = vcd->presetKnobValues[Rhodes][3] * 1.3f; //drive
            vcd->displayValues[4] = vcd->presetKnobValues[Rhodes][4]; //pan spread
            vcd->displayValues[5] = vcd->expBuffer[(int)(vcd->presetKnobValues[Rhodes][5] * vcd->expBufferSizeMinusOne)] * 8192.0f;
            vcd->displayValues[6] = vcd->expBuffer[(int)(vcd->presetKnobValues[Rhodes][6] * vcd->expBufferSizeMinusOne)] * 8192.0f;
            vcd->displayValues[7] = vcd->presetKnobValues[Rhodes][7];
            vcd->displayValues[8] = vcd->expBuffer[(int)(vcd->presetKnobValues[Rhodes][8] * vcd->expBufferSizeMinusOne)] * 8192.0f;
            vcd->displayValues[9] = vcd->presetKnobValues[Rhodes][9];
            
            vcd->FM_indices[4][0] = vcd->displayValues[10] = vcd->presetKnobValues[Rhodes][10] * 1000.0f;
            vcd->FM_indices[4][1] = vcd->displayValues[11] = vcd->presetKnobValues[Rhodes][11] * 1000.0f;
            vcd->FM_indices[4][2] = vcd->displayValues[12] = vcd->presetKnobValues[Rhodes][12] * 1000.0f;
            vcd->FM_indices[4][3] = vcd->displayValues[13] = vcd->presetKnobValues[Rhodes][13] * 1000.0f;
            vcd->FM_indices[4][4] = vcd->displayValues[14] = vcd->presetKnobValues[Rhodes][14] * 1000.0f;
            vcd->FM_indices[4][5] = vcd->displayValues[21] = LEAF_clip(0.0f, ((vcd->presetKnobValues[Rhodes][21] * 1000.0f) - 10.0f), 1000.0f); // feedback
            //pitch ratios for custom rhodes preset
            for (int k = 15; k < 21; k++)
            {
                if (vcd->presetKnobValues[Rhodes][k] != vcd->prevKnobValues[k])
                {
                    float rawRate = (vcd->presetKnobValues[Rhodes][k] - 0.5f) * 14.0f;
                    float snapRate = roundf(rawRate);
                    float rate = (snapRate * vcd->overtoneSnap) + (rawRate * (1.0f - vcd->overtoneSnap));
                    if (rate < 0.0f) rate = 1.0f / fabsf(rate-1.0f);
                    else rate += 1.0f;
                    vcd->displayValues[k] = rate;
                    vcd->FM_freqRatios[4][k-15] = rate;
                }
                vcd->prevKnobValues[k] = vcd->presetKnobValues[Rhodes][k];
            }
            //overtone snap
            if (vcd->presetKnobValues[Rhodes][22] != vcd->prevKnobValues[22])
            {
                vcd->overtoneSnap = vcd->displayValues[22] = vcd->presetKnobValues[Rhodes][22];
                for (int k = 15; k < 21; k++)
                {
                    
                    float rawRate = (vcd->presetKnobValues[Rhodes][k] - 0.5f) * 14.0f;
                    float snapRate = roundf(rawRate);
                    float rate = (snapRate * vcd->overtoneSnap) + (rawRate * (1.0f - vcd->overtoneSnap));
                    if (rate < 0.0f) rate = 1.0f / fabsf(rate-1.0f);
                    else rate += 1.0f;
                    vcd->displayValues[k] = rate;
                    vcd->FM_freqRatios[4][k-15] = rate;
                }
            }
            vcd->prevKnobValues[22] = vcd->presetKnobValues[Rhodes][22];
            // random decays
            vcd->displayValues[23] = vcd->presetKnobValues[Rhodes][23];
            if (vcd->prevDisplayValues[23] != vcd->displayValues[23])
            {
                
                for (int i = 0; i < 6; i++)
                {
                    float randomNumberDraw = (vcd->leaf.random() * 2.0f) + 0.08f;
                    vcd->randomDecays[i] = (1.0f - vcd->displayValues[23]) + (randomNumberDraw * vcd->displayValues[23]);
                }
                
                for (int i = 0; i < NUM_VOC_VOICES; i++)
                {
                    for (int j = 0; j < 6; j++)
                    {
                        tADSRT_setDecay(vcd->FM_envs[i][j],(LEAF_clip(10.0f, vcd->displayValues[6] * vcd->randomDecays[j], 20000.0f))); //FM_decays[Rsound][j] * displayValues[6]);
                    }
                }
            }
            vcd->prevKnobValues[23] = vcd->presetKnobValues[Rhodes][23];
            
            // random sustains
            vcd->displayValues[24] = vcd->presetKnobValues[Rhodes][24];
            if (vcd->prevDisplayValues[24] != vcd->displayValues[24])
            {
                
                for (int i = 0; i < 6; i++)
                {
                    float randomNumberDraw = vcd->leaf.random() * 2.0f;
                    vcd->randomSustains[i] = (1.0f - vcd->displayValues[24]) + (randomNumberDraw * vcd->displayValues[24]);
                    tExpSmooth_setDest(vcd->susSmoothers[i], vcd->displayValues[7] * vcd->randomSustains[i]);
                }
            }
            vcd->prevDisplayValues[24] = vcd->displayValues[24];
            
            for (int k = 5; k < 10; k++)
            {
                if (vcd->prevDisplayValues[k] != vcd->displayValues[k])
                {
                    switch(k)
                    {
                            
                        case 5:
                            for (int i = 0; i < NUM_VOC_VOICES; i++)
                            {
                                for (int j = 0; j < 6; j++)
                                {
                                    //tADSR_setAttack(FM_envs[i][j], FM_attacks[Rsound][j] * displayValues[5]);
                                    //                                    cycleCountVals[1][2] = 0;
                                    //                                    uint64_t tempCount1 = DWT->CYCCNT;
                                    tADSRT_setAttack(vcd->FM_envs[i][j], vcd->displayValues[5] );
                                    //                                    uint64_t tempCount2 = DWT->CYCCNT;
                                    //                                    cycleCountVals[1][1] = tempCount2-tempCount1;
                                    //                                    CycleCounterTrackMinAndMax(1);
                                }
                            }
                            break;
                        case 6:
                            for (int i = 0; i < NUM_VOC_VOICES; i++)
                            {
                                for (int j = 0; j < 6; j++)
                                {
                                    tADSRT_setDecay(vcd->FM_envs[i][j],(LEAF_clip(7.0f, vcd->displayValues[6] * vcd->randomDecays[j], 20000.0f)));
                                }
                            }
                            break;
                        case 7:
                            for (int i = 0; i < 6; i++)
                            {
                                tExpSmooth_setDest(vcd->susSmoothers[i], vcd->displayValues[7] * vcd->randomSustains[i]);
                            }
                            break;
                        case 8:
                            for (int i = 0; i < NUM_VOC_VOICES; i++)
                            {
                                for (int j = 0; j < 6; j++)
                                {
                                    tADSRT_setRelease(vcd->FM_envs[i][j], vcd->displayValues[8]);
                                }
                            }
                            break;
                        case 9:
                            for (int i = 0; i < NUM_VOC_VOICES; i++)
                            {
                                for (int j = 0; j < 6; j++)
                                {
                                    tADSRT_setLeakFactor(vcd->FM_envs[i][j], ((1.0f - vcd->displayValues[9])  * 0.00004f) + 0.99996f);
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
                vcd->prevDisplayValues[k] = vcd->displayValues[k];
            }
#ifndef __cplusplus
            //==============================================================================

            for (int i = 0; i < vcd->rhodesParams.numVoices; i++)
            {
                calculateFreq(vcd, i);
                if (vcd->rhodesParams.numVoices > 1)
                {
                    if (vcd->poly->voices[i][0] == -2)
                    {
                        if ((vcd->FM_envs[i][0]->whichStage == env_idle) && (vcd->FM_envs[i][2]->whichStage == env_idle))
                        {
                            tSimplePoly_deactivateVoice(vcd->poly, i);
                        }
                    }
                }
            }

            tCycle_setFreq(vcd->tremolo, vcd->displayValues[2]);

            //==============================================================================
#endif
        }

        void SFXRhodesTick(Vocodec* vcd, float* input)
        {
            float leftSample = 0.0f;
            float rightSample = 0.0f;
#ifdef __cplusplus
            //==============================================================================
            // This was in frame. Needs to be in tick or we get a bit carry over pitch from the last note played
            for (int i = 0; i < vcd->rhodesParams.numVoices; i++)
            {
                calculateFreq(vcd, i);
                if (vcd->rhodesParams.numVoices > 1)
                {
                    if (vcd->poly->voices[i][0] == -2)
                    {
                        if ((vcd->FM_envs[i][0]->whichStage == env_idle) && (vcd->FM_envs[i][2]->whichStage == env_idle))
                        {
                            tSimplePoly_deactivateVoice(&vcd->poly, i);
                        }
                    }
                }
            }
            
            tCycle_setFreq(&vcd->tremolo, vcd->displayValues[2]);

            //==============================================================================
#endif
            for (int i = 0; i < 6; i++)
            {
                vcd->sustainsFinal[i] = tExpSmooth_tick(vcd->susSmoothers[i]);
            }
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                for (int j = 0; j < 6; j++)
                {
                    tADSRT_setSustain(vcd->FM_envs[i][j], vcd->sustainsFinal[j]); //FM_sustains[Rsound][j] * displayValues[7]);
                }
            }
            
            for (int i = 0; i < vcd->rhodesParams.numVoices; i++)
            {
                float myFrequency = vcd->freq[i];
                float sample = 0.0f;
                int r = vcd->rhodesParams.sound;
                tCycle_setFreq(vcd->FM_sines[i][5], (myFrequency * vcd->FM_freqRatios[r][5]) + (vcd->FM_indices[r][5] * vcd->feedback_output * vcd->displayValues[0]));

                vcd->feedback_output = tCycle_tick(vcd->FM_sines[i][5]);
                tCycle_setFreq(vcd->FM_sines[i][4], (myFrequency * vcd->FM_freqRatios[r][4]) + (vcd->FM_indices[r][4] * vcd->feedback_output * vcd->displayValues[0] * tADSRT_tick(vcd->FM_envs[i][5])));

                tCycle_setFreq(vcd->FM_sines[i][3], (myFrequency * vcd->FM_freqRatios[r][3]) + (vcd->FM_indices[r][3] * vcd->displayValues[0] * tCycle_tick(vcd->FM_sines[i][4]) * tADSRT_tickNoInterp(vcd->FM_envs[i][4])));

                tCycle_setFreq(vcd->FM_sines[i][2], (myFrequency * vcd->FM_freqRatios[r][2]) + (vcd->FM_indices[r][2] * vcd->displayValues[0] * tCycle_tick(vcd->FM_sines[i][3]) * tADSRT_tickNoInterp(vcd->FM_envs[i][3])));

                tCycle_setFreq(vcd->FM_sines[i][1], myFrequency * vcd->FM_freqRatios[r][1]);

                tCycle_setFreq(vcd->FM_sines[i][0],( myFrequency  * vcd->FM_freqRatios[r][0]) + (vcd->FM_indices[r][0] * vcd->displayValues[0] * tCycle_tick(vcd->FM_sines[i][1]) * tADSRT_tickNoInterp(vcd->FM_envs[i][1])));

                sample += (tCycle_tick(vcd->FM_sines[i][2]) * tADSRT_tickNoInterp(vcd->FM_envs[i][2]));

                sample += tCycle_tick(vcd->FM_sines[i][0]) * tADSRT_tickNoInterp(vcd->FM_envs[i][0]);

                leftSample += sample * ((0.5f * (1.0f - vcd->displayValues[4])) + (vcd->displayValues[4] * (1.0f - vcd->panValues[i])));

                rightSample += sample * ((0.5f * (1.0f - vcd->displayValues[4])) + (vcd->displayValues[4] * (vcd->panValues[i])));
            }
            
            leftSample *= 0.4f;
            rightSample *= 0.4f;
            float tremoloSignal = ((tCycle_tick(vcd->tremolo) * 0.5f) + 0.5f) * vcd->displayValues[1];
            
            if (vcd->rhodesParams.tremoloStereo)
            {
                leftSample *= (tremoloSignal + (1.0f - vcd->displayValues[1]));
                rightSample *= ((1.0f-tremoloSignal) + (1.0f - vcd->displayValues[1]));
            }
            else
            {
                leftSample *= (tremoloSignal + (1.0f - vcd->displayValues[1]));
                rightSample *= ((tremoloSignal) + (1.0f - vcd->displayValues[1]));
            }
            
            leftSample *= vcd->displayValues[3]; //drive
            leftSample = tanhf(leftSample);
            
            rightSample *= vcd->displayValues[3]; //drive
            rightSample = tanhf(rightSample);
            
            
            input[0] = leftSample;
            input[1] = rightSample;
            
        }
        
        void SFXRhodesFree(Vocodec* vcd)
        {
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                for (int j = 0; j < 6; j++)
                {
                    tCycle_free(&vcd->FM_sines[i][j]);
                    tADSRT_free(&vcd->FM_envs[i][j]);
                }
                
            }
            for (int i = 0; i < 6; i++)
            {
                tExpSmooth_free(&vcd->susSmoothers[i]);
            }
            tCycle_free(&vcd->tremolo);
            
        }
        


        //delay
        void SFXTapeAlloc(Vocodec* vcd)
        {
            vcd->leaf.clearOnAllocation = 1;
            tTapeDelay_initToPool(&vcd->delay, 2000, 30000, &vcd->mediumPool);
            tTapeDelay_initToPool(&vcd->delay2, 2000, 30000, &vcd->mediumPool);
            tSVF_init(&vcd->delayLP, SVFTypeLowpass, 16000.f, .7f, &vcd->leaf);
            tSVF_init(&vcd->delayHP, SVFTypeHighpass, 20.f, .7f, &vcd->leaf);

            tSVF_init(&vcd->delayLP2, SVFTypeLowpass, 16000.f, .7f, &vcd->leaf);
            tSVF_init(&vcd->delayHP2, SVFTypeHighpass, 20.f, .7f, &vcd->leaf);
            tRamp_init(&vcd->reelSmooth, 1300.0f, 1, &vcd->leaf);

            tHighpass_init(&vcd->delayShaperHp, 20.0f, &vcd->leaf);
            tHighpass_init(&vcd->dcBlock1, 40.0f, &vcd->leaf);
            tFeedbackLeveler_init(&vcd->feedbackControl, .99f, 0.01f, 0.125f, 0, &vcd->leaf);
            tOversampler_init(&vcd->oversampler, 2, 0, &vcd->leaf);
            setLED_A(vcd, vcd->tapeParams.shaper);
            setLED_B(vcd, vcd->tapeParams.uncapFeedback);
            setLED_C(vcd, vcd->tapeParams.freeze);
            vcd->leaf.clearOnAllocation = 0;
        }

        void SFXTapeFrame(Vocodec* vcd)
        {

        	if (vcd->buttonActionsSFX[ButtonA][ActionPress])
            {
                vcd->tapeParams.shaper = (vcd->tapeParams.shaper + 1) % numDist;
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->tapeParams.shaper % 1);
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress])
            {
                vcd->tapeParams.uncapFeedback = !vcd->tapeParams.uncapFeedback;
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(vcd, vcd->tapeParams.uncapFeedback);
            }

            if (vcd->buttonActionsSFX[ButtonC][ActionPress])
            {
                vcd->tapeParams.freeze = !vcd->tapeParams.freeze;
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vcd, vcd->tapeParams.freeze);
            }

            vcd->displayValues[0] = vcd->presetKnobValues[Tape][0] * 30000.0f;
            vcd->displayValues[1] = vcd->presetKnobValues[Tape][1] * 30000.0f;
            float cutoff1 = LEAF_clip(10.0f,
                                      faster_mtof((vcd->presetKnobValues[Tape][2] * 133) + 3.0f),
                                      20000.0f);
            float cutoff2 = LEAF_clip(10.0f,
                                      faster_mtof((vcd->presetKnobValues[Tape][3] * 133) + 3.0f),
                                      20000.0f);
            vcd->displayValues[2] = cutoff1;
            vcd->displayValues[3] = cutoff2;

            vcd->displayValues[4] = vcd->tapeParams.uncapFeedback ?
            vcd->presetKnobValues[Tape][4] * 1.1f :
            LEAF_clip(0.0f, vcd->presetKnobValues[Tape][4] * 1.1f, 0.9f);

            vcd->displayValues[5] = vcd->presetKnobValues[Tape][5];
        }




        // Add stereo param so in1 -> out1 and in2 -> out2 instead of in1 -> out1 & out2 (like bitcrusher)?
        void SFXTapeTick(Vocodec* vcd, float* input)
        {
            vcd->displayValues[0] = roundf(vcd->presetKnobValues[Tape][0] * 2.0f);
            if (vcd->displayValues[0] > 1.1f)
            {
            	vcd->displayValues[0] = 21172.0f;
            }
            else if ((vcd->displayValues[0] < 1.1f) && (vcd->displayValues[0] > 0.7f))
			{
            	vcd->displayValues[0] = 10586.0f;
			}
            else if (vcd->displayValues[0] < 0.7f)
			{
            	vcd->displayValues[0] = 5293.0f;
			}

            tRamp_setDest(vcd->reelSmooth, vcd->displayValues[0]);
            float tapeSpeed = tRamp_tick(vcd->reelSmooth);

            vcd->displayValues[1] = vcd->presetKnobValues[Tape][1] * 30000.0f;
            float cutoff1 = LEAF_clip(10.0f,
                                      faster_mtof((vcd->presetKnobValues[Tape][2] * 133) + 3.0f),
                                      20000.0f);
            float cutoff2 = LEAF_clip(10.0f,
                                      faster_mtof((vcd->presetKnobValues[Tape][3] * 133) + 3.0f),
                                      20000.0f);
            //vcd->displayValues[2] = 30.0f;
            //vcd->displayValues[3] = 16000.0f;


            vcd->displayValues[1] = vcd->presetKnobValues[Tape][1]*1.1f;
            float param1 = vcd->displayValues[1];
            vcd->displayValues[2] = vcd->presetKnobValues[Tape][2];
            float param2 = vcd->displayValues[2];
            vcd->displayValues[3] = vcd->presetKnobValues[Tape][3];
            float param3 = vcd->displayValues[3];
            vcd->displayValues[4] = vcd->tapeParams.uncapFeedback ?
            vcd->presetKnobValues[Tape][4] * 1.1f :
            LEAF_clip(0.0f, vcd->presetKnobValues[Tape][4] * 1.1f, 0.9f);

            vcd->displayValues[5] = vcd->presetKnobValues[Tape][5];

            tSVF_setFreq(vcd->delayHP, 50.0f);
            //tSVF_setFreq(vcd->delayHP2, vcd->displayValues[2]);
            tSVF_setFreq(vcd->delayLP, 12000.0f);
            //tSVF_setFreq(vcd->delayLP2, vcd->displayValues[3]);

            //swap tanh for shaper and add cheap fixed highpass after both shapers

            float input1, input2;

            tOversampler_upsample(vcd->oversampler, input[1], vcd->oversamplerArray);
			for (int i = 0; i < 2; i++)
			{
	            if (vcd->tapeParams.shaper == 0)
	            {

	            	vcd->oversamplerArray[i] = tFeedbackLeveler_tick(vcd->feedbackControl, tanhf((input[1]*param1) + (vcd->delayFB1 * vcd->displayValues[4])));
	                //input2 = tFeedbackLeveler_tick(vcd->feedbackControl, tanhf(input[1] + (vcd->delayFB2 * vcd->displayValues[4])));
	            }
	            else if (vcd->tapeParams.shaper == 1)
	            {
	            	vcd->oversamplerArray[i] = tFeedbackLeveler_tick(vcd->feedbackControl, tHighpass_tick(vcd->delayShaperHp, LEAF_shaper((input[1]*param1) + (vcd->delayFB1 * vcd->displayValues[4] * 0.5f), 0.5f)));
	                //input2 = tFeedbackLeveler_tick(vcd->feedbackControl, tHighpass_tick(vcd->delayShaperHp, LEAF_shaper(input[1] + (vcd->delayFB2 * vcd->displayValues[4] * 0.5f), 0.5f)));
	            }
	            else if (vcd->tapeParams.shaper == 2)
	            {
	            	//tFeedbackLeveler_tick(vcd->feedbackControl, tHighpass_tick(vcd->delayShaperHp, LEAF_shaper(input[1] + (vcd->delayFB1 * vcd->displayValues[4] * 0.5f), 0.5f)));
	                //input2 = tFeedbackLeveler_tick(vcd->feedbackControl, tHighpass_tick(vcd->delayShaperHp, LEAF_shaper(input[1] + (vcd->delayFB2 * vcd->displayValues[4] * 0.5f), 0.5f)));
	            	float sample = (((input[1]*param1)+param2)) + (vcd->delayFB1 * vcd->displayValues[4]);

	            	param3 = (param3 * .99f) + 0.01f;
	            	float shapeDividerS = 1.0f / (param3 - ((param3*param3*param3) * 0.3333333f));
	            	sample = sample * param1;
	                sample = sample + param2;
	                float shape = param3;
	                if (sample <= -1.0f)
	                {
	                    sample = -1.0f;
	                } else if (sample >= 1.0f)
	                {
	                    sample = 1.0f;
	                }
	                {
	                    sample = (shape * sample) - ((shape * (sample * sample * sample))* 0.3333333f);
	                    sample = sample * shapeDividerS;
	                }

	                sample = tHighpass_tick(vcd->dcBlock1, sample);
	                //sample *= fxPostGain[v];
	                vcd->oversamplerArray[i] = sample;
	            }

	            else if (vcd->tapeParams.shaper == 3)
	            {
	            	tFeedbackLeveler_tick(vcd->feedbackControl, tHighpass_tick(vcd->delayShaperHp, LEAF_shaper(input[1] + (vcd->delayFB1 * vcd->displayValues[4] * 0.5f), 0.5f)));
	                //input2 = tFeedbackLeveler_tick(vcd->feedbackControl, tHighpass_tick(vcd->delayShaperHp, LEAF_shaper(input[1] + (vcd->delayFB2 * vcd->displayValues[4] * 0.5f), 0.5f)));
	            	float sample = ((input[1]*param1)+param2) + (vcd->delayFB1 * vcd->displayValues[4]);

	            	param3 = ((param3 * .99f) + 0.01f) * HALF_PI;
	            	float shapeDividerH = 1.0f / sinf(param3);
	                if (sample <= -1.0f)
	                {
	                    sample = -1.0f;
	                } else if (sample >= 1.0f)
	                {
	                    sample = 1.0f;
	                }
	                {
	                    sample = sinf(  (sinf(sample*param3) * shapeDividerH) * param3);
	                    sample = sample * shapeDividerH;
	                }
	                sample = tHighpass_tick(vcd->dcBlock1, sample);
	                //sample *= fxPostGain[v];
	                vcd->oversamplerArray[i] = sample;
	            }

	            else if (vcd->tapeParams.shaper == 4)
	            {
	            	//tFeedbackLeveler_tick(vcd->feedbackControl, tHighpass_tick(vcd->delayShaperHp, LEAF_shaper(input[1] + (vcd->delayFB1 * vcd->displayValues[4] * 0.5f), 0.5f)));
	                //input2 = tFeedbackLeveler_tick(vcd->feedbackControl, tHighpass_tick(vcd->delayShaperHp, LEAF_shaper(input[1] + (vcd->delayFB2 * vcd->displayValues[4] * 0.5f), 0.5f)));
	            	float sample = input[1] + (vcd->delayFB1 * vcd->displayValues[4]);

	                sample = sample * param1;
	                float temp = (sample + (param2 * param1)) / (1.0f + fabs(sample + param2));
	                temp = tHighpass_tick(vcd->dcBlock1, temp);
	                temp = tanhf(temp);
	                //temp *= fxPostGain[v];
	                //sample *= fxPostGain[v];
	                vcd->oversamplerArray[i] = sample;
	            }

	            else if (vcd->tapeParams.shaper == 5)
	            {
	            	float sample = input[1] + (vcd->delayFB1 * vcd->displayValues[4]);
	            	sample = sample * param1 + ((param2 * param1));

					float curFB = param3;
					float curFF = 0.4f;
					float ff = (curFF * tanhf(sample)) + ((1.0f - curFF) * sample); //these saturation functions could be soft clip or hard clip or tanh approx
					float fb = curFB * tanhf(vcd->wfState);
					vcd->wfState = (ff + fb) - 0.5f * sinf(TWO_PI * sample); //maybe switch for our own sine lookup (avoid the if statements in the CMSIS code)
					sample = vcd->wfState * vcd->invCurFB;
					sample = tHighpass_tick(vcd->dcBlock1, sample);
					//sample *= fxPostGain[v];
					vcd->oversamplerArray[i] = sample;
	            }



			}
			input1 = tOversampler_downsample(vcd->oversampler, vcd->oversamplerArray);


            tTapeDelay_setDelay(vcd->delay, tapeSpeed);
            //tTapeDelay_setDelay(vcd->delay2, tapeSpeed);

            if (!vcd->tapeParams.freeze)
            {
                vcd->delayFB1 = tTapeDelay_tick(vcd->delay, input1);
                //vcd->delayFB2 = tTapeDelay_tick(vcd->delay2, input2);
            }

            else
            {
                vcd->delayFB1 = tTapeDelay_tick(vcd->delay, vcd->delayFB1);
                //vcd->delayFB2 = tTapeDelay_tick(vcd->delay2, vcd->delayFB2);
            }

            vcd->delayFB1 = tSVF_tick(vcd->delayLP, vcd->delayFB1);
            //vcd->delayFB2 = tSVF_tick(vcd->delayLP2, vcd->delayFB2);

            vcd->delayFB1 = tanhf(tSVF_tick(vcd->delayHP, vcd->delayFB1));
            //vcd->delayFB2 = tanhf(tSVF_tick(vcd->delayHP2, vcd->delayFB2));

            input[0] = vcd->delayFB1;// * vcd->displayValues[5];
            input[1] = vcd->delayFB1;
            //input[1] = vcd->delayFB2;// * vcd->displayValues[5];

        }

        void SFXTapeFree(Vocodec* vcd)
        {
        	tTapeDelay_free(&vcd->delay);
            tTapeDelay_free(&vcd->delay2);
            tSVF_free(&vcd->delayLP);
            tSVF_free(&vcd->delayHP);

            tSVF_free(&vcd->delayLP2);
            tSVF_free(&vcd->delayHP2);
            tRamp_free(&vcd->reelSmooth);

            tHighpass_free(&vcd->delayShaperHp);
            tHighpass_free(&vcd->dcBlock1);
            tFeedbackLeveler_free(&vcd->feedbackControl);
            tOversampler_free(&vcd->oversampler);
        }
        //
#ifdef __cplusplus
        void SFXWavetableSynthAlloc(Vocodec* vcd)
        {
            tWaveSynth_initToPool(&vcd->waveSynth, NUM_VOC_VOICES, vcd->loadedTables, vcd->loadedTableSizes, 4, 20000.0f, &vcd->largePool);
            vcd->wavetableSynthParams.loadIndex = 0;

            tSimplePoly_setNumVoices(&vcd->poly, vcd->wavetableSynthParams.numVoices);

            float* knobs = vcd->presetKnobValues[ClassicSynth];

            vcd->displayValues[5] = vcd->expBuffer[(int)(knobs[5] * vcd->expBufferSizeMinusOne)] * 8192.0f; //att
            vcd->displayValues[6] = vcd->expBuffer[(int)(knobs[6] * vcd->expBufferSizeMinusOne)] * 8192.0f; //dec
            vcd->displayValues[7] = knobs[7]; //sus
            vcd->displayValues[8] = vcd->expBuffer[(int)(knobs[8] * vcd->expBufferSizeMinusOne)] * 8192.0f; //rel
            vcd->displayValues[9] = knobs[9]; //leak

            for (int i = 0; i < NUM_VOC_VOICES; ++i)
            {
                tADSRT_init(&vcd->polyEnvs[i], vcd->displayValues[5], vcd->displayValues[6], vcd->displayValues[7], vcd->displayValues[8], vcd->decayExpBuffer, DECAY_EXP_BUFFER_SIZE, &vcd->leaf);
                tADSRT_setLeakFactor(&vcd->polyEnvs[i],((1.0f - vcd->displayValues[9]) * 0.00005f) + 0.99995f);
            }

            setLED_A(vcd, vcd->wavetableSynthParams.numVoices == 1);
            setLED_B(vcd, 0);
            setLED_C(vcd, 0);
        }

        void SFXWavetableSynthFrame(Vocodec* vcd)
        {
            if (vcd->buttonActionsSFX[ButtonA][ActionPress])
            {
                vcd->wavetableSynthParams.numVoices = vcd->wavetableSynthParams.numVoices > 1 ? 1 : NUM_VOC_VOICES;
                tSimplePoly_setNumVoices(&vcd->poly, vcd->wavetableSynthParams.numVoices);
                vcd->buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(vcd, vcd->wavetableSynthParams.numVoices == 1);
            }
            if (vcd->buttonActionsSFX[ButtonB][ActionPress])
            {
                vcd->wavetableSynthParams.loadIndex++;
                if (vcd->wavetableSynthParams.loadIndex >= 4) vcd->wavetableSynthParams.loadIndex = 0;
                vcd->buttonActionsSFX[ButtonB][ActionPress] = 0;
            }
            if (vcd->buttonActionsSFX[ButtonC][ActionPress])
            {
                vcd->loadWav(vcd);
                vcd->buttonActionsSFX[ButtonC][ActionPress] = 0;
            }
            if (vcd->newWavLoaded > 0)
            {
                tWaveSynth_free(&vcd->waveSynth);
                tWaveSynth_initToPool(&vcd->waveSynth, NUM_VOC_VOICES, vcd->loadedTables, vcd->loadedTableSizes, 4, 20000.0f, &vcd->largePool);
                vcd->newWavLoaded = 0;
            }

            float* knobs = vcd->presetKnobValues[WavetableSynth];

            // Gains
            vcd->displayValues[0] = knobs[0];
            vcd->displayValues[1] = knobs[1];
            vcd->displayValues[2] = knobs[2];
            vcd->displayValues[3] = knobs[3];
            vcd->displayValues[4] = knobs[4];

            // ADSR
            vcd->displayValues[5] = vcd->expBuffer[(int)(knobs[5] * vcd->expBufferSizeMinusOne)] * 8192.0f; //att
            vcd->displayValues[6] = vcd->expBuffer[(int)(knobs[6] * vcd->expBufferSizeMinusOne)] * 8192.0f; //dec
            vcd->displayValues[7] = knobs[7]; //sus
            vcd->displayValues[8] = vcd->expBuffer[(int)(knobs[8] * vcd->expBufferSizeMinusOne)] * 8192.0f; //rel
            vcd->displayValues[9] = knobs[9]; //leak

            for (int i = 0; i < vcd->wavetableSynthParams.numVoices; i++)
            {
                tADSRT_setAttack(&vcd->polyEnvs[i], vcd->displayValues[5]);
                tADSRT_setDecay(&vcd->polyEnvs[i], vcd->displayValues[6]);
                tADSRT_setSustain(&vcd->polyEnvs[i], vcd->displayValues[7]);
                tADSRT_setRelease(&vcd->polyEnvs[i], vcd->displayValues[8]);
                tADSRT_setLeakFactor(&vcd->polyEnvs[i], ((1.0f - vcd->displayValues[9]) * 0.00005f) + 0.99995f);
            }

            // Phases
            vcd->displayValues[10] = knobs[10];
            vcd->displayValues[11] = knobs[11];
            vcd->displayValues[12] = knobs[12];
            vcd->displayValues[13] = knobs[13];

            for (int i = 0; i < tSimplePoly_getNumVoices(&vcd->poly); i++)
            {
                tExpSmooth_setDest(&vcd->polyRamp[i], (tSimplePoly_getVelocity(&vcd->poly, i) > 0));
                calculateFreq(vcd, i);
                tWaveSynth_setFreq(&vcd->waveSynth, i, vcd->freq[i]);
            }

            tWaveSynth_setIndex(&vcd->waveSynth, vcd->presetKnobValues[WavetableSynth][4]);
            for (int i = 0; i < 4; ++i)
            {
                tWaveSynth_setIndexGain(&vcd->waveSynth, i, vcd->displayValues[i]);
                tWaveSynth_setIndexPhase(&vcd->waveSynth, i, vcd->displayValues[10+i]);
            }
        }

        // Add stereo param so in1 + in2 -> out1 & out2 instead of in1 -> out1 & out2 ?
        void SFXWavetableSynthTick(Vocodec* vcd, float* input)
        {
            float sample = 0.0f;

            for (int i = 0; i < tSimplePoly_getNumVoices(&vcd->poly); ++i)
            {
                if (vcd->wavetableSynthParams.numVoices > 1)
                {
                    if (vcd->poly->voices[i][0] == -2)
                    {
                        if (vcd->polyEnvs[i]->whichStage == env_idle)
                        {
                            tSimplePoly_deactivateVoice(&vcd->poly, i);
                        }
                    }
                }

                float env = tADSRT_tick(&vcd->polyEnvs[i]);

                sample += tWaveSynth_tickVoice(&vcd->waveSynth, i) * tExpSmooth_tick(&vcd->polyRamp[i]) * env;
            }

            input[0] = tanhf(sample);
            input[1] = input[0];
        }

        void SFXWavetableSynthFree(Vocodec* vcd)
        {
            tWaveSynth_free(&vcd->waveSynth);
            for (int i = 0; i < NUM_VOC_VOICES; ++i)
            {
                tADSRT_free(&vcd->polyEnvs[i]);
            }
        }

#endif
        // midi functions
        void calculateFreq(Vocodec* vcd, int voice)
        {
            float tempNote = (float)tSimplePoly_getPitch(vcd->poly, voice) + vcd->pitchBendValue;
            float tempPitchClass = ((((int)tempNote) - vcd->keyCenter) % 12 );
            float tunedNote = tempNote + vcd->centsDeviation[(int)tempPitchClass];
            vcd->freq[voice] = LEAF_midiToFrequency(tunedNote);
        }
        
        float calculateTunedMidiNote(Vocodec* vcd, float tempNote)
        {
            tempNote += vcd->pitchBendValue;
            float tempPitchClass = ((((int)tempNote) - vcd->keyCenter) % 12 ) ;
            return (tempNote + vcd->centsDeviation[(int)tempPitchClass]);
        }
        
        void calculateNoteArray(Vocodec* vcd)
        {
            for (int i = 0; i < 128; i++)
            {
                float tempNote = i;
                float tempPitchClass = ((((int)tempNote) - vcd->keyCenter) % 12 );
                float tunedNote = tempNote + vcd->centsDeviation[(int)tempPitchClass];
                vcd->notes[i] = tunedNote;
            }
        }
        
        float nearestNote(Vocodec* vcd, float note)
        {
            float leastDifference = fabsf(note - vcd->notes[0]);
            float difference;
            int index = 0;
            int* chord;
            
            if (vcd->neartuneParams.useChromatic > 0)
            {
                chord = vcd->chromaticArray;
            }
            else
            {
                chord = vcd->chordArray;
            }
            //if (autotuneLock > 0) chord = lockArray;
            
            for(int i = 1; i < 128; i++)
            {
                if (chord[i%12] > 0)
                {
                    difference = fabsf(note - vcd->notes[i]);
                    if(difference < leastDifference)
                    {
                        leastDifference = difference;
                        index = i;
                    }
                }
            }
            
            return vcd->notes[index];
            
        }
        
        float nearestNoteWithHysteresis(Vocodec* vcd, float note, float hysteresis)
        {
            float leastDifference = fabsf(note - vcd->notes[0]);
            float difference;
            int nearIndex = 0;
            int* chord;
            float output = 0.0f;
            
            if (vcd->neartuneParams.useChromatic > 0)
            {
                chord = vcd->chromaticArray;
            }
            else
            {
                chord = vcd->chordArray;
            }
            if (vcd->neartuneParams.lock > 0)
            {
                chord = vcd->lockArray;
            }
            int hasNotes = 0;
            for (int i = 0; i < 12; i++)
            {
                if (chord[i] > 0)
                {
                    hasNotes = 1;
                }
                
            }
            if (hasNotes)
            {
                
                for(int i = 1; i < 128; i++)
                {
                    if (chord[i%12] > 0)
                    {
                        difference = fabsf(note - vcd->notes[i]);
                        if(difference < leastDifference)
                        {
                            leastDifference = difference;
                            nearIndex = i;
                        }
                    }
                }
                
                if (vcd->lastNearNote == -1)
                {
                    output = vcd->notes[nearIndex];
                    vcd->lastNearNote = nearIndex;
                    return output;
                }
                if (nearIndex != vcd->lastNearNote)
                {
                    //check if it's beyond the hysteresis
                    
                    //find closest note in chord upward from lastNearNote
                    int upNote = 0;
                    int downNote = 128;
                    int i = vcd->lastNearNote;
                    while ((i < 128) && (upNote == 0))
                    {
                        i++;
                        if (chord[i%12] > 0)
                        {
                            upNote = i;
                        }
                        if (i == 128)
                        {
                            upNote = 128;
                        }
                        
                    }
                    i = vcd->lastNearNote;
                    while ((i > 0) && (downNote == 128))
                    {
                        i--;
                        if (chord[i%12] > 0)
                        {
                            downNote = i;
                        }
                        if (i == 0)
                        {
                            downNote = 0;
                        }
                    }
                    //now should have adjacent notes in array available
                    //calculate the differences that should be necessary to move away
                    float upperNearHyst = (vcd->notes[upNote] - vcd->notes[vcd->lastNearNote]) * hysteresis;
                    float lowerNearHyst = (vcd->notes[vcd->lastNearNote] - vcd->notes[downNote]) * -hysteresis;
                    
                    float theDifference = note - vcd->notes[vcd->lastNearNote];
                    if ((theDifference > upperNearHyst) || (theDifference < lowerNearHyst))
                    {
                        output = vcd->notes[nearIndex];
                        vcd->lastNearNote = nearIndex;
                        
                    }
                    else
                    {
                        output = vcd->notes[vcd->lastNearNote];
                    }
                }
                else
                {
                    output = vcd->notes[nearIndex];
                }
            }
            else
            {
                output = -1.0f; //signal that there are no notes to snap to
            }
            return output;
        }

        void noteOn(Vocodec* vcd, int key, int velocity)
        {
            
            if (!velocity)
            {
                noteOff(vcd, key, velocity);
            }
            
            else
            {
                
                vcd->chordArray[key%12]++;
                
                
                if (vcd->currentPreset == AutotuneMono)
                {
                    if (vcd->neartuneParams.lock)
                    {
                        vcd->lockArray[key%12]++;
                    }
                }

                if (vcd->currentPreset == Rhodes)
                {
                    int whichVoice = tSimplePoly_noteOn(vcd->poly, key, velocity);
                    
                    if (whichVoice >= 0)
                    {
                        for (int j = 0; j < 6; j++)
                        {
                            tADSRT_on(vcd->FM_envs[whichVoice][j], velocity * 0.0078125f);
                        }
                        vcd->panValues[whichVoice] = key * 0.0078125f; // divide by 128.0f
                    }
                }
                else if (vcd->currentPreset == ClassicSynth)
                {
                    int whichVoice = tSimplePoly_noteOn(vcd->poly, key, velocity);
                    
                    if (whichVoice >= 0)
                    {
                        tADSRT_on(vcd->polyEnvs[whichVoice], velocity * 0.0078125f);
                        tADSRT_on(vcd->polyFiltEnvs[whichVoice], velocity * 0.0078125f);
                    }
                }
#ifdef __cplusplus
                else if (vcd->currentPreset == WavetableSynth)
                {
                    int whichVoice = tSimplePoly_noteOn(vcd->poly, key, velocity);
                    
                    if (whichVoice >= 0)
                    {
                        tADSRT_on(vcd->polyEnvs[whichVoice], velocity * 0.0078125f);
                    }
                }
#endif
                else if (vcd->currentPreset == SamplerKeyboard)
                {
                    if ((key >= LOWEST_SAMPLER_KEY) && key < (LOWEST_SAMPLER_KEY + NUM_SAMPLER_KEYS))
                    {
                        int whichVoice = tSimplePoly_noteOn(vcd->poly, key, velocity);
                        if (whichVoice >= 0)
                        {
                            
                            vcd->currentSamplerKeyGlobal = key - LOWEST_SAMPLER_KEY;
                            ///if (!controlAllKeys)
                            //{
                            //setKnobValues(keyKnobValues[currentSamplerKey]);
                            //prevKnob2 = keyKnobValues[currentSamplerKey][2];
                            //prevKnob4 = keyKnobValues[currentSamplerKey][4];
                            //}
                            if (tBuffer_getRecordedLength(vcd->keyBuff[vcd->currentSamplerKeyGlobal]) == 0)
                            {
                                tBuffer_record(vcd->keyBuff[vcd->currentSamplerKeyGlobal]);
                                vcd->newBuffer[vcd->currentSamplerKeyGlobal] = 1;
                            }
                            else
                            {
                                tSampler_play(vcd->keySampler[vcd->currentSamplerKeyGlobal]);
                                if (vcd->newBuffer[vcd->currentSamplerKeyGlobal])
                                {
                                    int recordLength = tBuffer_getRecordedLength(vcd->keyBuff[vcd->currentSamplerKeyGlobal]);
                                    vcd->samplePlayLengths[vcd->currentSamplerKeyGlobal] = recordLength;
                                    vcd->newBuffer[vcd->currentSamplerKeyGlobal] = 0;
                                }
                                float tempGain = (velocity * 0.0078125f * vcd->displayValues[6]) + (1.0f - vcd->displayValues[6]);
                                tExpSmooth_setDest(vcd->kSamplerGains[vcd->currentSamplerKeyGlobal], tempGain);
                            }
                            vcd->samplerKeyHeld[vcd->currentSamplerKeyGlobal] = 1;
                        }
                    }
                }
                else if (vcd->currentPreset == LivingStringSynth)
                {
                    int whichVoice = tSimplePoly_noteOn(vcd->poly, key, velocity);
                    if (whichVoice >= 0)
                    {
                        tADSRT_on(vcd->pluckEnvs[whichVoice], velocity * 0.0078125f);
                    }
                }
                else
                {
                    tSimplePoly_noteOn(vcd->poly, key, velocity);
                }
                setLED_2(vcd, 1);
            }
        }
        
        void noteOff(Vocodec* vcd, int key, int velocity)
        {
            if (vcd->chordArray[key%12] > 0) vcd->chordArray[key%12]--;

            if (vcd->currentPreset == Rhodes)
            {
                int voice;
                if (tSimplePoly_getNumVoices(vcd->poly) > 1)
                {
                    voice = tSimplePoly_markPendingNoteOff(vcd->poly, key); //if we're polyphonic, we need to let release envelopes happen and not mark voices free when they are not
                }
                else
                {
                    voice = tSimplePoly_noteOff(vcd->poly, key); //if we're monophonic, we need to allow fast voice stealing and returning to previous stolen notes without regard for the release envelopes
                }
                if (voice >= 0)
                {
                    for (int j = 0; j < 6; j++)
                    {
                        tADSRT_off(vcd->FM_envs[voice][j]);
                    }
                }
                
            }
            else if (vcd->currentPreset == ClassicSynth)
            {
                int voice;
                if (tSimplePoly_getNumVoices(vcd->poly) > 1)
                {
                    voice = tSimplePoly_markPendingNoteOff(vcd->poly, key); //if we're polyphonic, we need to let release envelopes happen and not mark voices free when they are not
                }
                else
                {
                    voice = tSimplePoly_noteOff(vcd->poly, key); //if we're monophonic, we need to allow fast voice stealing and returning to previous stolen notes without regard for the release envelopes
                }
                
                if (voice >= 0)
                {
                    tADSRT_off(vcd->polyEnvs[voice]);
                    tADSRT_off(vcd->polyFiltEnvs[voice]);
                }
            }
#ifdef __cplusplus
            else if (vcd->currentPreset == WavetableSynth)
            {
                int voice;
                if (tSimplePoly_getNumVoices(vcd->poly) > 1)
                {
                    voice = tSimplePoly_markPendingNoteOff(vcd->poly, key); //if we're polyphonic, we need to let release envelopes happen and not mark voices free when they are not
                }
                else
                {
                    voice = tSimplePoly_noteOff(vcd->poly, key); //if we're monophonic, we need to allow fast voice stealing and returning to previous stolen notes without regard for the release envelopes
                }

                if (voice >= 0)
                {
                    tADSRT_off(vcd->polyEnvs[voice]);
                }
            }
#endif
            
            else if (vcd->currentPreset == SamplerKeyboard)
            {
                int voice;
                
                
                if (key >= LOWEST_SAMPLER_KEY && key < LOWEST_SAMPLER_KEY + NUM_SAMPLER_KEYS)
                {
                    voice = tSimplePoly_markPendingNoteOff(vcd->poly, key); //if we're polyphonic, we need to let release envelopes happen and not mark voices free when they are not
                    
                    
                    if (tBuffer_isActive(vcd->keyBuff[key-LOWEST_SAMPLER_KEY]) == 1)
                    {
                        tBuffer_stop(vcd->keyBuff[key-LOWEST_SAMPLER_KEY]);
//                        UISamplerKButtons(vcd, ButtonUp, ActionPress);
                    }
                    else
                    {
                        tExpSmooth_setDest(vcd->kSamplerGains[key-LOWEST_SAMPLER_KEY], 0.0f);
                    }
                    vcd->samplerKeyHeld[key-LOWEST_SAMPLER_KEY] = 0;
//                    UISamplerKButtons(vcd, ButtonC, ActionHoldContinuous);
                    tSampler_stop(vcd->keySampler[key-LOWEST_SAMPLER_KEY]);

                    if (voice >= 0)
                        vcd->waitingForDeactivation[voice] = key;
                }
            }
            else if (vcd->currentPreset == LivingStringSynth)
            {
                int voice;
                
                voice = tSimplePoly_noteOff(vcd->poly, key); //if we're monophonic, we need to allow fast voice stealing and returning to previous stolen notes without regard for the release envelopes
                
                if (voice >= 0)
                {
                    tADSRT_off(vcd->pluckEnvs[voice]);
                }
            }
            else
            {
                tSimplePoly_noteOff(vcd->poly, key);
            }
            
            if (tSimplePoly_getNumActiveVoices(vcd->poly) < 1)
            {
                setLED_2(vcd, 0);
            }
            
        }
        
        
        void pitchBend(Vocodec* vcd, int data)
        {
            vcd->pitchBendValue = (data - 8192) * 0.000244140625f;
        }
        
        
        void sustainOff(Vocodec* vcd)
        {
            
        }
        
        void sustainOn(Vocodec* vcd)
        {
            
        }
        
        void toggleBypass(Vocodec* vcd)
        {
            
        }
        
        void toggleSustain(Vocodec* vcd)
        {
            
        }
        
        void ctrlInput(Vocodec* vcd, int ctrl, int value)
        {
        	//vocodec.presetKnobValues[vocodec.currentPreset][ctrl] = value * 0.00787402f; //would need to make the part that writes the knobs switch out for this dynamically to make it work
            
        }
        
#ifdef __cplusplus
    }
} // extern "C"
#endif
