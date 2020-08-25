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
#include "tunings.h"

#ifdef __cplusplus
namespace vocodec
{
    extern "C"
    {
#endif
        
#define INC_MISC_WT 0
#define USE_FILTERTAN_TABLE 1
        
#ifdef __cplusplus
        char small_memory[SMALL_MEM_SIZE];
        char medium_memory[MED_MEM_SIZE];
        char large_memory[LARGE_MEM_SIZE];
#else
        char small_memory[SMALL_MEM_SIZE];
        char medium_memory[MED_MEM_SIZE] __ATTR_RAM_D1;
        char large_memory[LARGE_MEM_SIZE] __ATTR_SDRAM;
#endif
        
        void (*allocFunctions[PresetNil])(void);
        void (*frameFunctions[PresetNil])(void);
        void (*tickFunctions[PresetNil])(float*);
        void (*freeFunctions[PresetNil])(void);
        
        tMempool mediumPool;
        tMempool largePool;
        
        float defaultPresetKnobValues[PresetNil][NUM_PRESET_KNOB_VALUES];
        float presetKnobValues[PresetNil][NUM_PRESET_KNOB_VALUES];
        int knobActive[NUM_ADC_CHANNELS];
        float prevDisplayValues[NUM_PRESET_KNOB_VALUES];
        
        //audio objects
        tFormantShifter fs;
        tRetune autotuneMono;
        tAutotune autotunePoly;
        tRetune retune;
        tRetune retune2;
        tRamp pitchshiftRamp;
        tRamp nearWetRamp;
        
        tSimplePoly poly;
        tExpSmooth polyRamp[NUM_VOC_VOICES];
        
        tExpSmooth comp;
        
        tBuffer buff;
        tBuffer asBuff[2];
        tSampler sampler;
        tSampler asSampler[2];
        
        // we have about 172 seconds of space to
        // divide across this number of keys
        
        tBuffer keyBuff[NUM_SAMPLER_KEYS];
        tSampler keySampler[NUM_SAMPLER_KEYS];
        
        tEnvelopeFollower envfollow;
        
        tOversampler oversampler;
        
        tLockhartWavefolder wavefolder1;
        tLockhartWavefolder wavefolder2;
        tHighpass wfHP;
        tCrusher crush;
        tCrusher crush2;
        
        tTapeDelay delay;
        tSVF delayLP;
        tSVF delayHP;
        tTapeDelay delay2;
        tSVF delayLP2;
        tSVF delayHP2;
        tHighpass delayShaperHp;
        tFeedbackLeveler feedbackControl;
        
        tExpSmooth smoother1;
        tExpSmooth smoother2;
        tExpSmooth smoother3;
        
        tExpSmooth neartune_smoother;
        
#define EXP_BUFFER_SIZE 128
        float expBuffer[EXP_BUFFER_SIZE];
        float expBufferSizeMinusOne = EXP_BUFFER_SIZE - 1;
        
#define DECAY_EXP_BUFFER_SIZE 512
        float decayExpBuffer[DECAY_EXP_BUFFER_SIZE];
        float decayExpBufferSizeMinusOne = DECAY_EXP_BUFFER_SIZE - 1;
        
        
#define NUM_STRINGS 6
        tComplexLivingString theString[NUM_STRINGS];
        
        float myDetune[NUM_STRINGS];
        float synthDetune[NUM_VOC_VOICES][NUM_OSC_PER_VOICE];
        //control objects
        float notes[128];
        int chordArray[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        int chromaticArray[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        int lockArray[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
        
        float freq[NUM_VOC_VOICES];
        
        float oversamplerArray[MAX_OVERSAMPLER_RATIO];
        
        
        
        void initGlobalSFXObjects()
        {
            calculateNoteArray();
            
            tSimplePoly_init(&poly, NUM_VOC_VOICES);
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                tExpSmooth_init(&polyRamp[i], 0.0f, 0.02f);
                freq[i] = 220.0f;
            }
            
            tExpSmooth_init(&comp, 1.0f, 0.01f);
            
            LEAF_generate_exp(expBuffer, 1000.0f, -1.0f, 0.0f, -0.0008f, EXP_BUFFER_SIZE); //exponential buffer rising from 0 to 1
            LEAF_generate_exp(decayExpBuffer, 0.001f, 0.0f, 1.0f, -0.0008f, DECAY_EXP_BUFFER_SIZE); // exponential decay buffer falling from 1 to 0
            
            
            
            // Note that these are the actual knob values
            // not the parameter value
            // (i.e. 0.5 for fine pitch is actually 0.0 fine pitch)
            defaultPresetKnobValues[Vocoder][0] = 0.4f; // volume
            defaultPresetKnobValues[Vocoder][1] = 0.5f; // warp factor
            defaultPresetKnobValues[Vocoder][2] = 0.85f; // quality
            defaultPresetKnobValues[Vocoder][3] = 0.0f; // sawToPulse
            defaultPresetKnobValues[Vocoder][4] = 0.2f; // noise threshold
            defaultPresetKnobValues[Vocoder][5] = 0.02f; // breathiness
            defaultPresetKnobValues[Vocoder][6] = 0.5f; // tilt
            defaultPresetKnobValues[Vocoder][7] = 0.5f; // pulse width
            defaultPresetKnobValues[Vocoder][8] = 0.5f; // pulse shape
            defaultPresetKnobValues[Vocoder][9] = 0.0f;
            
            
            defaultPresetKnobValues[VocoderCh][0] = 0.4f; // volume
            defaultPresetKnobValues[VocoderCh][1] = 0.5f; // warp factor
            defaultPresetKnobValues[VocoderCh][2] = 1.0f; // quality
            defaultPresetKnobValues[VocoderCh][3] = 0.5f; //band width
            defaultPresetKnobValues[VocoderCh][4] = 0.2f; //noise thresh
            defaultPresetKnobValues[VocoderCh][5] = 0.0f;// saw->pulse fade
            defaultPresetKnobValues[VocoderCh][6] = 0.5f; // pulse length
            defaultPresetKnobValues[VocoderCh][7] = 0.5f; // pulse width
            defaultPresetKnobValues[VocoderCh][8] = 0.0f; // breathiness
            defaultPresetKnobValues[VocoderCh][9] = 0.66f; // envelope speed
            defaultPresetKnobValues[VocoderCh][10] = 0.5f;// squish
            defaultPresetKnobValues[VocoderCh][11] = 0.5f; // offset
            defaultPresetKnobValues[VocoderCh][12] = 0.5f; // tilt
            defaultPresetKnobValues[VocoderCh][13] = 0.0f; // stereo
            defaultPresetKnobValues[VocoderCh][14] = 0.0f; // barkpull
            
            
            defaultPresetKnobValues[Pitchshift][0] = 0.5f; // pitch
            defaultPresetKnobValues[Pitchshift][1] = 0.5f; // fine pitch
            defaultPresetKnobValues[Pitchshift][2] = 0.0f; // f amount
            defaultPresetKnobValues[Pitchshift][3] = 0.5f; // formant
            defaultPresetKnobValues[Pitchshift][4] = 0.5f; //range
            defaultPresetKnobValues[Pitchshift][5] = 0.25f; //offset
            defaultPresetKnobValues[Pitchshift][6] = 0.25f;
            defaultPresetKnobValues[Pitchshift][7] = 0.25f;
            defaultPresetKnobValues[Pitchshift][8] = 0.25f;
            defaultPresetKnobValues[Pitchshift][9] = 0.25f;
            
            defaultPresetKnobValues[AutotuneMono][0] = 0.0f; // pickiness
            defaultPresetKnobValues[AutotuneMono][1] = 1.0f; // amount
            defaultPresetKnobValues[AutotuneMono][2] = 0.5f; // speed
            defaultPresetKnobValues[AutotuneMono][3] = 1.0f; // leap allow
            defaultPresetKnobValues[AutotuneMono][4] = 0.25f; // hysteresis
            
            defaultPresetKnobValues[AutotunePoly][0] = 1.0f; // fidelity thresh
            defaultPresetKnobValues[AutotunePoly][1] = 0.5f;
            defaultPresetKnobValues[AutotunePoly][2] = 0.1f;
            defaultPresetKnobValues[AutotunePoly][3] = 0.0f;
            defaultPresetKnobValues[AutotunePoly][4] = 0.0f;
            
            defaultPresetKnobValues[SamplerButtonPress][0] = 0.0f; // start
            defaultPresetKnobValues[SamplerButtonPress][1] = 1.0f; // end
            defaultPresetKnobValues[SamplerButtonPress][2] = 0.75f; // speed
            defaultPresetKnobValues[SamplerButtonPress][3] = 0.5f; // speed mult
            defaultPresetKnobValues[SamplerButtonPress][4] = 0.4f;//crossfade
            
            defaultPresetKnobValues[SamplerKeyboard][0] = 0.0f; // start
            defaultPresetKnobValues[SamplerKeyboard][1] = 1.0f; // end
            defaultPresetKnobValues[SamplerKeyboard][2] = 0.75f; // speed
            defaultPresetKnobValues[SamplerKeyboard][3] = 0.5f; // speed mult
            defaultPresetKnobValues[SamplerKeyboard][4] = 0.0f; //looping on
            defaultPresetKnobValues[SamplerKeyboard][5] = 0.4f;//crossfade
            defaultPresetKnobValues[SamplerKeyboard][6] = 0.0f;//velocity sensitivity
            
            defaultPresetKnobValues[SamplerAutoGrab][0] = 0.95f; // thresh
            defaultPresetKnobValues[SamplerAutoGrab][1] = 0.5f; // window
            defaultPresetKnobValues[SamplerAutoGrab][2] = 0.75f; // speed
            defaultPresetKnobValues[SamplerAutoGrab][3] = 0.25f; // crossfade
            defaultPresetKnobValues[SamplerAutoGrab][4] = 0.0f;
            defaultPresetKnobValues[SamplerAutoGrab][5] = 0.0f; // len rand
            defaultPresetKnobValues[SamplerAutoGrab][6] = 0.0f; // speed rand
            defaultPresetKnobValues[SamplerAutoGrab][7] = 0.0f;
            defaultPresetKnobValues[SamplerAutoGrab][8] = 0.0f;
            defaultPresetKnobValues[SamplerAutoGrab][9] = 0.0f;
            
            defaultPresetKnobValues[Distortion][0] = .25f; // pre gain
            defaultPresetKnobValues[Distortion][1] = 0.5f; // tilt (low and high shelves, opposing gains)
            defaultPresetKnobValues[Distortion][2] = 0.5f; // mid gain
            defaultPresetKnobValues[Distortion][3] = 0.5f; // mid freq
            defaultPresetKnobValues[Distortion][4] = 0.25f; //post gain
            
            defaultPresetKnobValues[Wavefolder][0] = 0.4f; // gain
            defaultPresetKnobValues[Wavefolder][1] = 0.5f; // offset1
            defaultPresetKnobValues[Wavefolder][2] = 0.5f; // offset2
            defaultPresetKnobValues[Wavefolder][3] = 0.75f; // post gain
            defaultPresetKnobValues[Wavefolder][4] = 0.0f;
            
            defaultPresetKnobValues[BitCrusher][0] = 0.1f; // quality
            defaultPresetKnobValues[BitCrusher][1] = 0.5f; // samp ratio
            defaultPresetKnobValues[BitCrusher][2] = 0.0f; // rounding
            defaultPresetKnobValues[BitCrusher][3] = 0.0f; // operation
            defaultPresetKnobValues[BitCrusher][4] = 0.5f; // post gain
            defaultPresetKnobValues[BitCrusher][5] = 0.0f; // pre gain
            
            defaultPresetKnobValues[Delay][0] = 0.25f; // delayL
            defaultPresetKnobValues[Delay][1] = 0.25f; // delayR
            defaultPresetKnobValues[Delay][2] = 0.0f; // highpass
            defaultPresetKnobValues[Delay][3] = 1.0f; // lowpass
            defaultPresetKnobValues[Delay][4] = 0.5f; // feedback
            defaultPresetKnobValues[Delay][5] = 1.0f; // post gain
            
            defaultPresetKnobValues[Reverb][0] = 0.5f; // size
            defaultPresetKnobValues[Reverb][1] = 0.5f; // in lowpass
            defaultPresetKnobValues[Reverb][2] = 0.5f; // in highpass
            defaultPresetKnobValues[Reverb][3] = 0.5f; // fb lowpass
            defaultPresetKnobValues[Reverb][4] = 0.5f; // fb gain
            
            defaultPresetKnobValues[Reverb2][0] = 0.2f; // size
            defaultPresetKnobValues[Reverb2][1] = 0.5f; // lowpass
            defaultPresetKnobValues[Reverb2][2] = 0.5f; // highpass
            defaultPresetKnobValues[Reverb2][3] = 0.5f; // peak freq
            defaultPresetKnobValues[Reverb2][4] = 0.5f; // peak gain
            
            defaultPresetKnobValues[LivingString][0] = 0.3f; // freq 1
            defaultPresetKnobValues[LivingString][1] = 0.1f; // detune
            defaultPresetKnobValues[LivingString][2] = 0.3f; // decay
            defaultPresetKnobValues[LivingString][3] = 0.9f; // damping
            defaultPresetKnobValues[LivingString][4] = 0.5f; // pick pos
            defaultPresetKnobValues[LivingString][5] = 0.25f; // prep pos
            defaultPresetKnobValues[LivingString][6] = 0.0f; // prep index
            defaultPresetKnobValues[LivingString][7] = 0.0f; // let ring
            defaultPresetKnobValues[LivingString][8] = 0.8f;
            defaultPresetKnobValues[LivingString][9] = 0.5f;
            defaultPresetKnobValues[LivingString][10] = 0.3f;// freq 2
            defaultPresetKnobValues[LivingString][11] = 0.3f;// freq 3
            defaultPresetKnobValues[LivingString][12] = 0.3f;// freq 4
            defaultPresetKnobValues[LivingString][13] = 0.3f;// freq 5
            defaultPresetKnobValues[LivingString][14] = 0.3f;// freq 6
            
            defaultPresetKnobValues[LivingStringSynth][0] = 0.5f;
            defaultPresetKnobValues[LivingStringSynth][1] = 0.5f;
            defaultPresetKnobValues[LivingStringSynth][2] = .85f; // decay
            defaultPresetKnobValues[LivingStringSynth][3] = 1.0f; // damping
            defaultPresetKnobValues[LivingStringSynth][4] = 0.4f; // pick pos
            defaultPresetKnobValues[LivingStringSynth][5] = 0.25f; // prep pos
            defaultPresetKnobValues[LivingStringSynth][6] = 0.0f; // prep index
            defaultPresetKnobValues[LivingStringSynth][7] = 0.0f; // let ring
            defaultPresetKnobValues[LivingStringSynth][8] = 0.3f; // feedback volume
            defaultPresetKnobValues[LivingStringSynth][9] = 0.4f; // release time
            
            defaultPresetKnobValues[ClassicSynth][0] = 0.5f; // volume
            defaultPresetKnobValues[ClassicSynth][1] = 0.5f; // lowpass
            defaultPresetKnobValues[ClassicSynth][2] = 0.2f; // detune
            defaultPresetKnobValues[ClassicSynth][3] = 0.0f;
            defaultPresetKnobValues[ClassicSynth][4] = 0.0f;
            defaultPresetKnobValues[ClassicSynth][5] = 0.0f;
            defaultPresetKnobValues[ClassicSynth][6] = 0.06f;
            defaultPresetKnobValues[ClassicSynth][7] = 0.9f;
            defaultPresetKnobValues[ClassicSynth][8] = 0.1f;
            defaultPresetKnobValues[ClassicSynth][9] = 0.1f;
            defaultPresetKnobValues[ClassicSynth][10] = 0.0f;
            defaultPresetKnobValues[ClassicSynth][11] = 0.06f;
            defaultPresetKnobValues[ClassicSynth][12] = 0.9f;
            defaultPresetKnobValues[ClassicSynth][13] = 0.1f;
            defaultPresetKnobValues[ClassicSynth][14] = 0.1f;
            defaultPresetKnobValues[ClassicSynth][15] = 0.0f;
            defaultPresetKnobValues[ClassicSynth][16] = 0.06f;
            defaultPresetKnobValues[ClassicSynth][17] = 0.9f;
            defaultPresetKnobValues[ClassicSynth][18] = 0.1f;
            defaultPresetKnobValues[ClassicSynth][19] = 0.1f;
            
            defaultPresetKnobValues[Rhodes][0] = 0.25f;
            defaultPresetKnobValues[Rhodes][1] = 0.25f;
            defaultPresetKnobValues[Rhodes][2] = 0.25f;
            defaultPresetKnobValues[Rhodes][3] = 0.5f;
            defaultPresetKnobValues[Rhodes][4] = 0.0f; //stereo spread
            defaultPresetKnobValues[Rhodes][5] = 0.05f;
            defaultPresetKnobValues[Rhodes][6] = 0.05f;
            defaultPresetKnobValues[Rhodes][7] = 0.9f;
            defaultPresetKnobValues[Rhodes][8] = 0.1007f;
            defaultPresetKnobValues[Rhodes][9] = 0.5f;
            defaultPresetKnobValues[Rhodes][10] = 0.05f;
            defaultPresetKnobValues[Rhodes][11] = 0.05f;
            defaultPresetKnobValues[Rhodes][12] = 0.9f;
            defaultPresetKnobValues[Rhodes][13] = 0.1007f;
            defaultPresetKnobValues[Rhodes][14] = 0.5f;
            defaultPresetKnobValues[Rhodes][15] = 0.8f;
            defaultPresetKnobValues[Rhodes][16] = 0.6f;
            defaultPresetKnobValues[Rhodes][17] = 0.7f;
            defaultPresetKnobValues[Rhodes][18] = 0.5f;
            defaultPresetKnobValues[Rhodes][19] = 0.5f;
            defaultPresetKnobValues[Rhodes][20] = 0.5f;
            defaultPresetKnobValues[Rhodes][21] = 0.0f;
            defaultPresetKnobValues[Rhodes][22] = 0.00f;
            defaultPresetKnobValues[Rhodes][23] = 0.00f;
            defaultPresetKnobValues[Rhodes][24] = 0.00f;
            
            for (int p = 0; p < PresetNil; p++)
            {
                for (int v = 0; v < NUM_PRESET_KNOB_VALUES; v++)
                {
                    presetKnobValues[p][v] = defaultPresetKnobValues[p][v];
                }
            }
        }
        
        void initFunctionPointers(void)
        {
            allocFunctions[Vocoder] = SFXVocoderAlloc;
            frameFunctions[Vocoder] = SFXVocoderFrame;
            tickFunctions[Vocoder] = SFXVocoderTick;
            freeFunctions[Vocoder] = SFXVocoderFree;
            
            allocFunctions[VocoderCh] = SFXVocoderChAlloc;
            frameFunctions[VocoderCh] = SFXVocoderChFrame;
            tickFunctions[VocoderCh] = SFXVocoderChTick;
            freeFunctions[VocoderCh] = SFXVocoderChFree;
            
            allocFunctions[Pitchshift] = SFXPitchShiftAlloc;
            frameFunctions[Pitchshift] = SFXPitchShiftFrame;
            tickFunctions[Pitchshift] = SFXPitchShiftTick;
            freeFunctions[Pitchshift] = SFXPitchShiftFree;
            
            allocFunctions[AutotuneMono] = SFXNeartuneAlloc;
            frameFunctions[AutotuneMono] = SFXNeartuneFrame;
            tickFunctions[AutotuneMono] = SFXNeartuneTick;
            freeFunctions[AutotuneMono] = SFXNeartuneFree;
            
            allocFunctions[AutotunePoly] = SFXAutotuneAlloc;
            frameFunctions[AutotunePoly] = SFXAutotuneFrame;
            tickFunctions[AutotunePoly] = SFXAutotuneTick;
            freeFunctions[AutotunePoly] = SFXAutotuneFree;
            
            allocFunctions[SamplerButtonPress] = SFXSamplerBPAlloc;
            frameFunctions[SamplerButtonPress] = SFXSamplerBPFrame;
            tickFunctions[SamplerButtonPress] = SFXSamplerBPTick;
            freeFunctions[SamplerButtonPress] = SFXSamplerBPFree;
            
            allocFunctions[SamplerKeyboard] = SFXSamplerKAlloc;
            frameFunctions[SamplerKeyboard] = SFXSamplerKFrame;
            tickFunctions[SamplerKeyboard] = SFXSamplerKTick;
            freeFunctions[SamplerKeyboard] = SFXSamplerKFree;
            
            allocFunctions[SamplerAutoGrab] = SFXSamplerAutoAlloc;
            frameFunctions[SamplerAutoGrab] = SFXSamplerAutoFrame;
            tickFunctions[SamplerAutoGrab] = SFXSamplerAutoTick;
            freeFunctions[SamplerAutoGrab] = SFXSamplerAutoFree;
            
            allocFunctions[Distortion] = SFXDistortionAlloc;
            frameFunctions[Distortion] = SFXDistortionFrame;
            tickFunctions[Distortion] = SFXDistortionTick;
            freeFunctions[Distortion] = SFXDistortionFree;
            
            allocFunctions[Wavefolder] = SFXWaveFolderAlloc;
            frameFunctions[Wavefolder] = SFXWaveFolderFrame;
            tickFunctions[Wavefolder] = SFXWaveFolderTick;
            freeFunctions[Wavefolder] = SFXWaveFolderFree;
            
            allocFunctions[BitCrusher] = SFXBitcrusherAlloc;
            frameFunctions[BitCrusher] = SFXBitcrusherFrame;
            tickFunctions[BitCrusher] = SFXBitcrusherTick;
            freeFunctions[BitCrusher] = SFXBitcrusherFree;
            
            allocFunctions[Delay] = SFXDelayAlloc;
            frameFunctions[Delay] = SFXDelayFrame;
            tickFunctions[Delay] = SFXDelayTick;
            freeFunctions[Delay] = SFXDelayFree;
            
            allocFunctions[Reverb] = SFXReverbAlloc;
            frameFunctions[Reverb] = SFXReverbFrame;
            tickFunctions[Reverb] = SFXReverbTick;
            freeFunctions[Reverb] = SFXReverbFree;
            
            allocFunctions[Reverb2] = SFXReverb2Alloc;
            frameFunctions[Reverb2] = SFXReverb2Frame;
            tickFunctions[Reverb2] = SFXReverb2Tick;
            freeFunctions[Reverb2] = SFXReverb2Free;
            
            allocFunctions[LivingString] = SFXLivingStringAlloc;
            frameFunctions[LivingString] = SFXLivingStringFrame;
            tickFunctions[LivingString] = SFXLivingStringTick;
            freeFunctions[LivingString] = SFXLivingStringFree;
            
            allocFunctions[LivingStringSynth] = SFXLivingStringSynthAlloc;
            frameFunctions[LivingStringSynth] = SFXLivingStringSynthFrame;
            tickFunctions[LivingStringSynth] = SFXLivingStringSynthTick;
            freeFunctions[LivingStringSynth] = SFXLivingStringSynthFree;
            
            allocFunctions[ClassicSynth] = SFXClassicSynthAlloc;
            frameFunctions[ClassicSynth] = SFXClassicSynthFrame;
            tickFunctions[ClassicSynth] = SFXClassicSynthTick;
            freeFunctions[ClassicSynth] = SFXClassicSynthFree;
            
            allocFunctions[Rhodes] = SFXRhodesAlloc;
            frameFunctions[Rhodes] = SFXRhodesFrame;
            tickFunctions[Rhodes] = SFXRhodesTick;
            freeFunctions[Rhodes] = SFXRhodesFree;
        }
        
        ///1 vocoder internal poly
        
        tTalkboxFloat vocoder;
        tNoise vocoderNoise;
        tZeroCrossingCounter zerox;
        tSawtooth osc[NUM_VOC_VOICES * NUM_OSC_PER_VOICE];
        tRosenbergGlottalPulse glottal[NUM_VOC_VOICES];
        int numVoices = NUM_VOC_VOICES;
        int internalExternal = 0;
        int vocFreezeLPC = 0;
        tExpSmooth noiseRamp;
        tNoise breathNoise;
        tHighpass noiseHP;
        tVZFilter shelf1;
        tVZFilter shelf2;
        
        void SFXVocoderAlloc()
        {
            leaf.clearOnAllocation = 1;
            tTalkboxFloat_init(&vocoder, 1024);
            tTalkboxFloat_setWarpOn(&vocoder, 1);
            tNoise_init(&vocoderNoise, WhiteNoise);
            tZeroCrossingCounter_init(&zerox, 16);
            tSimplePoly_setNumVoices(&poly, numVoices);
            tExpSmooth_init(&noiseRamp, 0.0f, 0.005f);
            
            //tilt filter
            tVZFilter_init(&shelf1, Lowshelf, 80.0f, 6.0f);
            tVZFilter_init(&shelf2, Highshelf, 12000.0f, 6.0f);
            
            tNoise_init(&breathNoise, WhiteNoise);
            tHighpass_init(&noiseHP, 4500.0f);
            
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                
                tSawtooth_init(&osc[i]);
                
                tRosenbergGlottalPulse_init(&glottal[i]);
                tRosenbergGlottalPulse_setOpenLengthAndPulseLength(&glottal[i], 0.3f, 0.4f);
            }
            setLED_A(numVoices == 1);
            setLED_B(internalExternal);
            vocFreezeLPC = 0;
            setLED_C(vocFreezeLPC);
            
        }
        
        void SFXVocoderFrame()
        {
            if (buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                numVoices = (numVoices > 1) ? 1 : NUM_VOC_VOICES;
                tSimplePoly_setNumVoices(&poly, numVoices);
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(numVoices == 1);
            }
            if (buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                internalExternal = !internalExternal;
                buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(internalExternal);
            }
            if (buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                vocFreezeLPC = !vocFreezeLPC;
                tTalkboxFloat_setFreeze(&vocoder, vocFreezeLPC);
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vocFreezeLPC);
            }
            
            displayValues[0] = presetKnobValues[Vocoder][0]; //vocoder volume
            displayValues[1] = (presetKnobValues[Vocoder][1] * 0.4f) - 0.2f; //warp factor
            displayValues[2] = presetKnobValues[Vocoder][2] * 1.1f; //quality
            displayValues[3] = presetKnobValues[Vocoder][3]; //crossfade between sawtooth and glottal pulse
            displayValues[4] = presetKnobValues[Vocoder][4]; //noise thresh
            displayValues[5] = presetKnobValues[Vocoder][5]; //breathy
            displayValues[6] = (presetKnobValues[Vocoder][6] * 30.0f) - 15.0f;; //tilt filter
            displayValues[7] = presetKnobValues[Vocoder][7]; //pulse length
            displayValues[8] = presetKnobValues[Vocoder][8]; //open length
            
            tTalkboxFloat_setWarpFactor(&vocoder, displayValues[1]);
            tTalkboxFloat_setQuality(&vocoder, displayValues[2]);
            
            for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
            {
                tExpSmooth_setDest(&polyRamp[i], (tSimplePoly_getVelocity(&poly, i) > 0));
                calculateFreq(i);
                tSawtooth_setFreq(&osc[i], freq[i]);
                tRosenbergGlottalPulse_setFreq(&glottal[i], freq[i]);
                tRosenbergGlottalPulse_setOpenLengthAndPulseLength(&glottal[i], displayValues[8] * displayValues[7], displayValues[7]);
            }
            
            if (tSimplePoly_getNumActiveVoices(&poly) != 0)
            {
                tExpSmooth_setDest(&comp, sqrtf(1.0f / (float)tSimplePoly_getNumActiveVoices(&poly)));
            }
            else
            {
                tExpSmooth_setDest(&comp, 0.0f);
            }
            
            tVZFilter_setGain(&shelf1, fasterdbtoa(-1.0f * displayValues[6]));
            tVZFilter_setGain(&shelf2, fastdbtoa(displayValues[6]));
        }
        
        void SFXVocoderTick(float* input)
        {
            
            float zerocross = 0.0f;
            float noiseRampVal = 0.0f;
            float sample = 0.0f;
            
            if (internalExternal == 1)
            {
                sample = input[0];
            }
            else
            {
                zerocross = tZeroCrossingCounter_tick(&zerox, input[1]);
                
                if (!vocChFreeze)
                {
                    tExpSmooth_setDest(&noiseRamp,zerocross > ((displayValues[4])-0.1f));
                }
                noiseRampVal = tExpSmooth_tick(&noiseRamp);
                
                float noiseSample = tNoise_tick(&vocoderNoise) * noiseRampVal * 0.6f;
                
                
                for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
                {
                    sample += tSawtooth_tick(&osc[i]) * tExpSmooth_tick(&polyRamp[i]) * (1.0f - displayValues[3]);
                    sample += tRosenbergGlottalPulse_tickHQ(&glottal[i]) * tExpSmooth_tick(&polyRamp[i]) * 1.9f * displayValues[3];
                }
                
                
                //switch with consonant noise
                sample = (sample * (1.0f - (0.3f * displayValues[5])) * (1.0f-noiseRampVal)) + noiseSample;
                //add breathiness
                sample += (tHighpass_tick(&noiseHP, tNoise_tick(&breathNoise)) * displayValues[5] * 1.5f);
                sample *= tExpSmooth_tick(&comp);
            }
            
            sample = tanhf(sample);
            
            sample = tTalkboxFloat_tick(&vocoder, sample, input[1]);
            sample = tVZFilter_tick(&shelf1, sample); //put it through the low shelf
            sample = tVZFilter_tick(&shelf2, sample); // now put that result through the high shelf
            sample *= displayValues[0] * 0.6f;
            sample = tanhf(sample);
            input[0] = sample;
            input[1] = sample;
        }
        
        
        
        void SFXVocoderFree(void)
        {
            tTalkboxFloat_free(&vocoder);
            tNoise_free(&vocoderNoise);
            tZeroCrossingCounter_free(&zerox);
            tExpSmooth_free(&noiseRamp);
            
            tNoise_free(&breathNoise);
            tHighpass_free(&noiseHP);
            
            
            tVZFilter_free(&shelf1);
            tVZFilter_free(&shelf2);
            
            
            
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                tSawtooth_free(&osc[i]);
                tRosenbergGlottalPulse_free(&glottal[i]);
            }
        }
        
        
        tVZFilter analysisBands[MAX_NUM_VOCODER_BANDS][MAX_VOCODER_FILTER_ORDER];
        tVZFilter synthesisBands[MAX_NUM_VOCODER_BANDS][MAX_VOCODER_FILTER_ORDER];
        
        tExpSmooth envFollowers[MAX_NUM_VOCODER_BANDS];
        int numberOfVocoderBands = 22;
        int prevNumberOfVocoderBands = 22;
        float invNumberOfVocoderBands = 0.03125f;
        
        
        int whichKnobThisTime = 0;
        
        int currentBandToAlter = 0;
        int analysisOrSynthesis = 0;
        int alteringBands = 0;
        
        float prevMyQ = 1.0f;
        float invMyQ = 1.0f;
        float prevWarpFactor = 1.0f;
        float bandGains[MAX_NUM_VOCODER_BANDS];
        
        float bandWidthInSemitones;
        float bandWidthInOctaves;  // divide by 12
        
        float thisBandwidth;
        float prevBandSquish = 1.0f;
        float prevBandOffset = 30.0f;
        float prevMyTilt = 0.0f;
        float prevBarkPull = 0.0f;
        
        tVZFilter vocodec_highshelf;
        int vocChFreeze = 0;
        
        float barkBandFreqs[24] = {100.0f, 150.0f, 250.0f, 350.0f, 450.0f, 570.0f, 700.0f, 840.0f, 1000.0f, 1170.0f, 1370.0f, 1600.0f, 1850.0f, 2150.0f, 2500.0f, 2900.0f, 3400.0f, 4000.0f, 4800.0f, 5800.0f, 7000.0f, 8500.0f, 10500.0f, 12000.0f};
        float barkBandWidths[24] = {1.0f, 1.0f, 0.5849f, 0.4150f, 0.3505f, 0.304854f, 0.2895066175f, 0.256775415f, 0.231325545833333f, 0.233797185f, 0.220768679166667f, 0.216811389166667f, 0.217591435f, 0.214124805f, 0.218834601666667f, 0.222392421666667f, 0.2321734425f, 0.249978253333333f, 0.268488835833333f, 0.272079545833333f, 0.266786540833333f, 0.3030690675f, 0.3370349875f, 0.36923381f};
        
        tHighpass chVocFinalHP1;
        tHighpass chVocFinalHP2;
        
        void SFXVocoderChAlloc()
        {
            leaf.clearOnAllocation = 1;
            displayValues[0] = presetKnobValues[VocoderCh][0]; //vocoder volume
            
            displayValues[1] = (presetKnobValues[VocoderCh][1] * 0.8f) - 0.4f; //warp factor
            
            displayValues[2] = (uint8_t)(presetKnobValues[VocoderCh][2] * 16.9f) + 8.0f; //quality
            
            displayValues[3] = (presetKnobValues[VocoderCh][3]* 2.0f) + 0.1f; //band width
            
            displayValues[4] = presetKnobValues[VocoderCh][4]; //noise thresh
            
            displayValues[5] = presetKnobValues[VocoderCh][5]; //crossfade between sawtooth and glottal pulse
            
            displayValues[6] = presetKnobValues[VocoderCh][6]; //pulse width
            
            displayValues[7] = presetKnobValues[VocoderCh][7]; //pulse shape
            
            displayValues[8] = presetKnobValues[VocoderCh][8]; //breathiness
            
            displayValues[9] = presetKnobValues[VocoderCh][9]; //speed
            
            displayValues[10] = presetKnobValues[VocoderCh][10] * 2.0f; //bandsquish
            
            displayValues[11] = presetKnobValues[VocoderCh][11] * 60.0f; //bandoffset
            
            displayValues[12] = (presetKnobValues[VocoderCh][12] * 2.0f) - 1.0f; //tilt
            
            displayValues[13] = presetKnobValues[VocoderCh][13]; //stereo
            
            displayValues[14] = presetKnobValues[VocoderCh][14]; //odd/even
            float myQ = displayValues[3];
            
            
            invNumberOfVocoderBands = 1.0f / ((float)numberOfVocoderBands-0.99f);
            bandWidthInSemitones = 99.0f * invNumberOfVocoderBands;
            bandWidthInOctaves = bandWidthInSemitones * 0.083333333333333f;  // divide by 12
            thisBandwidth = bandWidthInOctaves * myQ;
            
            tVZFilter_init(&vocodec_highshelf, Highshelf, 6000.0f, 3.0f);
            tVZFilter_setGain(&vocodec_highshelf, 4.0f);
            
            for (int i = 0; i < MAX_NUM_VOCODER_BANDS; i++)
            {
                
                float bandFreq = faster_mtof(((float)i * bandWidthInSemitones) + 30.0f); //midinote 28 (41Hz) to midinote 134 (18814Hz) is 106 midinotes, divide that by how many bands to find out how far apart to put the bands
                
                bandGains[i] = 1.0f;
                
                if (i == 0)
                {
                    tVZFilter_init(&analysisBands[i][0], Lowpass, bandFreq, thisBandwidth);
                    tVZFilter_init(&analysisBands[i][1], Lowpass, bandFreq, thisBandwidth);
                    
                    tVZFilter_init(&synthesisBands[i][0], Lowpass, bandFreq,thisBandwidth);
                    tVZFilter_init(&synthesisBands[i][1], Lowpass, bandFreq,thisBandwidth);
                    
                }
                else if (i == (MAX_NUM_VOCODER_BANDS-1))
                {
                    tVZFilter_init(&analysisBands[i][0], Highpass, bandFreq, thisBandwidth);
                    tVZFilter_init(&analysisBands[i][1], Highpass, bandFreq, thisBandwidth);
                    
                    tVZFilter_init(&synthesisBands[i][0], Highpass, bandFreq, thisBandwidth);
                    tVZFilter_init(&synthesisBands[i][1], Highpass, bandFreq, thisBandwidth);
                    
                }
                else
                {
                    tVZFilter_init(&analysisBands[i][0], BandpassPeak, bandFreq, thisBandwidth);
                    tVZFilter_init(&analysisBands[i][1], BandpassPeak, bandFreq, thisBandwidth);
                    
                    tVZFilter_init(&synthesisBands[i][0], BandpassPeak, bandFreq, thisBandwidth);
                    tVZFilter_init(&synthesisBands[i][1], BandpassPeak, bandFreq, thisBandwidth);
                    
                }
                tExpSmooth_init(&envFollowers[i], 0.0f, 0.001f); // factor of .001 is 10 ms?
            }
            tNoise_init(&breathNoise, WhiteNoise);
            tNoise_init(&vocoderNoise, WhiteNoise);
            tZeroCrossingCounter_init(&zerox, 256);
            tSimplePoly_setNumVoices(&poly, numVoices);
            tExpSmooth_init(&noiseRamp, 0.0f, 0.05f);
            tHighpass_init(&noiseHP, 5000.0f);
            tHighpass_init(&chVocFinalHP1, 20.0f);
            tHighpass_init(&chVocFinalHP2, 20.0f);
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                
                tSawtooth_init(&osc[i]);
                
                tRosenbergGlottalPulse_init(&glottal[i]);
                tRosenbergGlottalPulse_setOpenLength(&glottal[i], 0.3f);
                tRosenbergGlottalPulse_setPulseLength(&glottal[i], 0.4f);
            }
            setLED_A(numVoices == 1);
            setLED_B(internalExternal);
            setLED_C(vocChFreeze);
            
            
            
            
        }
        
        float oneMinusStereo = 1.0f;
        float chVocOutputGain = 1.0f;
        void SFXVocoderChFrame()
        {
            
            if (buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                numVoices = (numVoices > 1) ? 1 : NUM_VOC_VOICES;
                tSimplePoly_setNumVoices(&poly, numVoices);
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(numVoices == 1);
            }
            if (buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                internalExternal = !internalExternal;
                buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(internalExternal);
            }
            
            if (buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                vocChFreeze = !vocChFreeze;
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(vocChFreeze);
            }
            
            
            
            displayValues[0] = presetKnobValues[VocoderCh][0]; //vocoder volume
            
            displayValues[1] = (presetKnobValues[VocoderCh][1] * 0.8f) - 0.4f; //warp factor
            
            displayValues[2] = (uint8_t)(presetKnobValues[VocoderCh][2] * 16.9f) + 8.0f; //quality
            
            displayValues[3] = (presetKnobValues[VocoderCh][3]* 2.0f) + 0.1f; //band width
            
            displayValues[4] = presetKnobValues[VocoderCh][4]; //noise thresh
            
            displayValues[5] = presetKnobValues[VocoderCh][5]; //crossfade between sawtooth and glottal pulse
            
            displayValues[6] = presetKnobValues[VocoderCh][6]; //pulse width
            
            displayValues[7] = presetKnobValues[VocoderCh][7]; //pulse shape
            
            displayValues[8] = presetKnobValues[VocoderCh][8]; //breathiness
            
            displayValues[9] = presetKnobValues[VocoderCh][9]; //speed
            
            displayValues[10] = presetKnobValues[VocoderCh][10] + 0.5f; //bandsquish
            
            displayValues[11] = presetKnobValues[VocoderCh][11] * 60.0f; //bandoffset
            
            displayValues[12] = (presetKnobValues[VocoderCh][12] * 4.0f) - 2.0f; //tilt
            
            displayValues[13] = presetKnobValues[VocoderCh][13]; //stereo
            
            displayValues[14] = presetKnobValues[VocoderCh][14]; //snap to bark scale
            
            
            oneMinusStereo = 1.0f - displayValues[13];
            chVocOutputGain = 9.0f * displayValues[0];
            
            for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
            {
                tExpSmooth_setDest(&polyRamp[i], (tSimplePoly_getVelocity(&poly, i) > 0));
                calculateFreq(i);
                tSawtooth_setFreq(&osc[i], freq[i]);
                tRosenbergGlottalPulse_setFreq(&glottal[i], freq[i]);
                tRosenbergGlottalPulse_setOpenLengthAndPulseLength(&glottal[i], displayValues[6] * displayValues[7], displayValues[6]);
            }
            
            float warpFactor = 1.0f + displayValues[1];
            numberOfVocoderBands = displayValues[2];
            float myQ = displayValues[3];
            float bandSquish = displayValues[10];
            float bandOffset = displayValues[11];
            float myTilt = displayValues[12];
            float barkPull = displayValues[14];
            float oneMinusBarkPull = 1.0f - barkPull;
            
            
            if ((numberOfVocoderBands != prevNumberOfVocoderBands) || (myQ != prevMyQ) || (warpFactor != prevWarpFactor) || (bandSquish != prevBandSquish) || (bandOffset != prevBandOffset) || (myTilt != prevMyTilt) || (barkPull != prevBarkPull))
            {
                alteringBands = 1;
                invNumberOfVocoderBands = 1.0f / ((float)numberOfVocoderBands-0.99f);
                bandWidthInSemitones = 94.0f * bandSquish * invNumberOfVocoderBands; //was 90
                bandWidthInOctaves = bandWidthInSemitones * 0.083333333333333f;  // divide by 12
                thisBandwidth = bandWidthInOctaves * myQ;
                invMyQ = 1.0f / myQ;
            }
            if (alteringBands)
            {
                
                float tempWarpFactor = warpFactor;
                float bandFreq = faster_mtof(((float)currentBandToAlter * bandWidthInSemitones) + bandOffset); //midinote 28 (41Hz) to midinote 134 (18814Hz) is 106 midinotes, divide that by how many bands to find out how far apart to put the bands
                
                //warp to bark scale if knob 16 is up
                bandFreq = (bandFreq * oneMinusBarkPull) + (barkBandFreqs[currentBandToAlter] * barkPull);
                
                if (bandFreq > 5000.0f) // a way to keep the upper bands fixed so consonants are not stretched even though vowels are
                {
                    tempWarpFactor = 1.0f;
                }
                
                if (bandFreq > 16000.0f)
                {
                    bandFreq = 16000.0f;
                }
                
                float bandBandwidth = (thisBandwidth * oneMinusBarkPull) + (barkBandWidths[currentBandToAlter] *  barkPull * myQ);
                float myHeight = (float)currentBandToAlter * invNumberOfVocoderBands; //x value
                float tiltOffset = (1.0f - ((myTilt * 0.5f) + 0.5f)) + 0.5f;
                float tiltY = displayValues[12] * myHeight + tiltOffset;
                bandGains[currentBandToAlter] = invMyQ * tiltY;
                
                if (analysisOrSynthesis == 0)
                {
                    tVZFilter_setFreqAndBandwidth(&analysisBands[currentBandToAlter][0], bandFreq, bandBandwidth);
                    //set these to match without computing for increased efficiency
                    analysisBands[currentBandToAlter][1]->B = analysisBands[currentBandToAlter][0]->B;
                    analysisBands[currentBandToAlter][1]->fc = analysisBands[currentBandToAlter][0]->fc;
                    analysisBands[currentBandToAlter][1]->R2 = analysisBands[currentBandToAlter][0]->R2;
                    analysisBands[currentBandToAlter][1]->cL = analysisBands[currentBandToAlter][0]->cL;
                    analysisBands[currentBandToAlter][1]->cB = analysisBands[currentBandToAlter][0]->cB;
                    analysisBands[currentBandToAlter][1]->cH = analysisBands[currentBandToAlter][0]->cH;
                    analysisBands[currentBandToAlter][1]->h = analysisBands[currentBandToAlter][0]->h;
                    analysisBands[currentBandToAlter][1]->g = analysisBands[currentBandToAlter][0]->g;
                    analysisOrSynthesis++;
                }
                else
                {
                    tVZFilter_setFreqAndBandwidth(&synthesisBands[currentBandToAlter][0], bandFreq * tempWarpFactor, bandBandwidth);
                    //set these to match without computing for increased efficiency
                    synthesisBands[currentBandToAlter][1]->B = synthesisBands[currentBandToAlter][0]->B;
                    synthesisBands[currentBandToAlter][1]->fc = synthesisBands[currentBandToAlter][0]->fc;
                    synthesisBands[currentBandToAlter][1]->R2 = synthesisBands[currentBandToAlter][0]->R2;
                    synthesisBands[currentBandToAlter][1]->cL = synthesisBands[currentBandToAlter][0]->cL;
                    synthesisBands[currentBandToAlter][1]->cB = synthesisBands[currentBandToAlter][0]->cB;
                    synthesisBands[currentBandToAlter][1]->cH = synthesisBands[currentBandToAlter][0]->cH;
                    synthesisBands[currentBandToAlter][1]->h = synthesisBands[currentBandToAlter][0]->h;
                    synthesisBands[currentBandToAlter][1]->g = synthesisBands[currentBandToAlter][0]->g;
                    currentBandToAlter++;
                    analysisOrSynthesis = 0;
                }
                
                
                if ((currentBandToAlter >= numberOfVocoderBands) && (analysisOrSynthesis == 0))
                {
                    alteringBands = 0;
                    currentBandToAlter = 0;
                }
            }
            
            prevNumberOfVocoderBands = numberOfVocoderBands;
            prevMyQ = myQ;
            prevWarpFactor = warpFactor;
            prevBandSquish = bandSquish;
            prevBandOffset = bandOffset;
            prevMyTilt = myTilt;
            prevBarkPull = barkPull;
            
            for (int i = 0; i < numberOfVocoderBands; i++)
            {
                tExpSmooth_setFactor(&envFollowers[i], (displayValues[9] * 0.0015f) + 0.0001f);
            }
            
            if (tSimplePoly_getNumActiveVoices(&poly) != 0)
            {
                tExpSmooth_setDest(&comp, sqrtf(1.0f / (float)tSimplePoly_getNumActiveVoices(&poly)));
            }
            else
            {
                tExpSmooth_setDest(&comp, 0.0f);
            }
        }
        
        
        void SFXVocoderChTick(float* input)
        {
            
            float sample = 0.0f;
            
            //a little treble emphasis
            input[1] = tVZFilter_tick(&vocodec_highshelf, input[1]);
            
            if (internalExternal == 1)
            {
                sample = input[0];
            }
            else
            {
                float zerocross = tZeroCrossingCounter_tick(&zerox, input[1]);
                
                if (!vocChFreeze)
                {
                    tExpSmooth_setDest(&noiseRamp,zerocross > ((displayValues[4])-0.1f));
                }
                float noiseRampVal = tExpSmooth_tick(&noiseRamp);
                
                float noiseSample = tNoise_tick(&vocoderNoise) * noiseRampVal;
                
                for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
                {
                    float tempRamp = tExpSmooth_tick(&polyRamp[i]);
                    if (tempRamp > 0.0001f)
                    {
                        if (displayValues[5] < 0.5f)
                        {
                            sample += tSawtooth_tick(&osc[i]) * tempRamp;
                        }
                        else
                        {
                            sample += tRosenbergGlottalPulse_tick(&glottal[i]) * tempRamp;
                        }
                    }
                }
                //switch with consonant noise
                sample = (sample * (1.0f - (0.3f * displayValues[8])) * (1.0f-noiseRampVal)) + noiseSample;
                //add breathiness
                sample += (tHighpass_tick(&noiseHP, tNoise_tick(&breathNoise)) * displayValues[8] * 2.0f);
                sample *= tExpSmooth_tick(&comp);
                
            }
            
            sample = fast_tanh4(sample);
            
            float output[2] = {0.0f, 0.0f};
            input[1] = input[1] * (displayValues[0] * 30.0f);
            for (int i = 0; i < numberOfVocoderBands; i++)
            {
                int oddEven = i % 2;
                float tempSamp = input[1];
                if (!vocChFreeze)
                {
                    tempSamp = tVZFilter_tickEfficient(&analysisBands[i][0], tempSamp);
                    tempSamp = tVZFilter_tickEfficient(&analysisBands[i][1], tempSamp);
                    tExpSmooth_setDest(&envFollowers[i], fabsf(tempSamp));
                }
                
                tempSamp = tExpSmooth_tick(&envFollowers[i]);
                //here is the envelope followed gain of the modulator signal
                tempSamp = LEAF_clip(0.0f, tempSamp, 2.0f);
                float tempSynth = sample;
                tempSynth = tVZFilter_tickEfficient(&synthesisBands[i][0], tempSynth);
                tempSynth = tVZFilter_tickEfficient(&synthesisBands[i][1], tempSynth);
                output[oddEven] += tempSynth * tempSamp * bandGains[i];
                
            }
            
            float finalSample1 = tHighpass_tick(&chVocFinalHP1, (output[0] + (output[1] * oneMinusStereo)) * chVocOutputGain);
            float finalSample2 = tHighpass_tick(&chVocFinalHP2, (output[1] + (output[0] * oneMinusStereo)) * chVocOutputGain);
            input[0] = 0.98f * fast_tanh4(finalSample1);
            input[1] = 0.98f * fast_tanh4(finalSample2);
        }
        
        
        
        
        void SFXVocoderChFree(void)
        {
            for (int i = 0; i < MAX_NUM_VOCODER_BANDS; i++)
            {
                tVZFilter_free(&analysisBands[i][0]);
                tVZFilter_free(&analysisBands[i][1]);
                tVZFilter_free(&synthesisBands[i][0]);
                tVZFilter_free(&synthesisBands[i][1]);
                tExpSmooth_free(&envFollowers[i]);
            }
            tNoise_free(&breathNoise);
            tNoise_free(&vocoderNoise);
            tZeroCrossingCounter_free(&zerox);
            tExpSmooth_free(&noiseRamp);
            tHighpass_free(&noiseHP);
            tVZFilter_free(&vocodec_highshelf);
            tHighpass_free(&chVocFinalHP1);
            tHighpass_free(&chVocFinalHP2);
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                tSawtooth_free(&osc[i]);
                tRosenbergGlottalPulse_free(&glottal[i]);
            }
        }
        
        // pitch shift
        float pitchShiftRange = 2.0f;
        float pitchShiftOffset = -1.0f;
        
        void SFXPitchShiftAlloc()
        {
            
            tFormantShifter_init(&fs, 7);
            tRetune_initToPool(&retune, NUM_RETUNE, 1024, 512, &mediumPool);
            tRetune_initToPool(&retune2, NUM_RETUNE, 1024, 512, &mediumPool);
            tRamp_init(&pitchshiftRamp, 100.0f, 1);
            tRamp_setVal(&pitchshiftRamp, 1.0f);
            
            tSimplePoly_setNumVoices(&poly, 1);
            tExpSmooth_init(&smoother1, 0.0f, 0.01f);
            tExpSmooth_init(&smoother2, 0.0f, 0.01f);
            tExpSmooth_init(&smoother3, 0.0f, 0.01f);
        }
        
        void SFXPitchShiftFrame()
        {
            
            
        }
        
        void SFXPitchShiftTick(float* input)
        {
            //pitchFactor = (smoothedADC[0]*3.75f)+0.25f;
            float sample = 0.0f;
            
            float myPitchFactorCoarse = (presetKnobValues[Pitchshift][0]*2.0f) - 1.0f;
            float myPitchFactorFine = ((presetKnobValues[Pitchshift][1]*2.0f) - 1.0f) * 0.1f;
            float myPitchFactorCombined = myPitchFactorFine + myPitchFactorCoarse;
            displayValues[0] = myPitchFactorCombined;
            displayValues[1] = myPitchFactorCombined;
            
            float keyPitch = (float)tSimplePoly_getPitchAndCheckActive(&poly, 0);
            if (keyPitch >= 0)
            {
                keyPitch = LEAF_midiToFrequency(keyPitch) * 0.003822629969419f ;
            }
            else
            {
                keyPitch = 1.0f;
            }
            
            float myPitchFactor = fastexp2f(myPitchFactorCombined);
            myPitchFactor *= keyPitch;
            tRetune_setPitchFactor(&retune, myPitchFactor, 0);
            tRetune_setPitchFactor(&retune2, myPitchFactor, 0);
            
            
            displayValues[2] = LEAF_clip( 0.0f,((presetKnobValues[Pitchshift][2]) * 3.0f) - 0.2f,3.0f);
            
            displayValues[3] = fastexp2f((presetKnobValues[Pitchshift][3]*2.0f) - 1.0f);
            tExpSmooth_setDest(&smoother3, displayValues[2]);
            tFormantShifter_setIntensity(&fs, tExpSmooth_tick(&smoother3)+.1f);
            tFormantShifter_setShiftFactor(&fs, displayValues[3]);
            if (displayValues[2] > 0.01f)
            {
                tRamp_setDest(&pitchshiftRamp, -1.0f);
            }
            else
            {
                tRamp_setDest(&pitchshiftRamp, 1.0f);
            }
            
            float crossfadeVal = tRamp_tick(&pitchshiftRamp);
            float myGains[2];
            LEAF_crossfade(crossfadeVal, myGains);
            tExpSmooth_setDest(&smoother1, myGains[0]);
            tExpSmooth_setDest(&smoother2, myGains[1]);
            
            
            
            float formantsample = tanhf(tFormantShifter_remove(&fs, input[1]));
            
            
            
            
            float* samples = tRetune_tick(&retune2, formantsample);
            formantsample = samples[0];
            sample = input[1];
            samples = tRetune_tick(&retune, sample);
            sample = samples[0];
            
            formantsample = tanhf(tFormantShifter_add(&fs, formantsample)) * tExpSmooth_tick(&smoother2) ;
            sample = (sample * (tExpSmooth_tick(&smoother1))) +  formantsample;
            
            input[0] = sample;
            input[1] = sample;
            
        }
        
        
        
        
        void SFXPitchShiftFree(void)
        {
            tFormantShifter_free(&fs);
            tRetune_free(&retune);
            tRetune_free(&retune2);
            
            tRamp_free(&pitchshiftRamp);
            
            tExpSmooth_free(&smoother1);
            tExpSmooth_free(&smoother2);
            tExpSmooth_free(&smoother3);
        }
        
        
        
        
        //5 autotune mono
        int autotuneChromatic = 0;
        int autotuneLock = 0;
        float lastSnap = 1.0f;
        float detectedNote = 60.0f;
        float desiredSnap = 60.0f;
        float destinationNote = 60.0f;
        float destinationFactor = 1.0f;
        float factorDiff = 0.0f;
        float changeAmount = 0.0f;
        void SFXNeartuneAlloc()
        {
            leaf.clearOnAllocation = 1;
            tRetune_init(&autotuneMono, 1, 512, 256);
            calculateNoteArray();
            tExpSmooth_init(&neartune_smoother, 1.0f, .007f);
            tRamp_init(&nearWetRamp, 20.0f, 1);
            setLED_A(autotuneChromatic);
            setLED_C(autotuneLock);
            lastSnap = 1.0f;
        }
        
        void SFXNeartuneFrame()
        {
            
            if ((tSimplePoly_getNumActiveVoices(&poly) != 0) || (autotuneChromatic == 1) || (autotuneLock == 1))
            {
                tRamp_setDest(&nearWetRamp, 1.0f);
            }
            else
            {
                tRamp_setDest(&nearWetRamp, -1.0f);
            }
            
            if (buttonActionsSFX[ButtonA][ActionPress])
            {
                autotuneChromatic = !autotuneChromatic;
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(autotuneChromatic);
            }
            
            if (buttonActionsSFX[ButtonC][ActionPress])
            {
                autotuneLock = !autotuneLock;
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(autotuneLock);
                if (autotuneLock)
                {
                    for (int i = 0; i < 12; i++)
                    {
                        lockArray[i] = chordArray[i];
                    }
                }
                else
                {
                    for (int i = 0; i < 12; i++)
                    {
                        lockArray[i] = 0;
                    }
                }
            }
            
            
            
        }
        
        
        
        void SFXNeartuneTick(float* input)
        {
            float sample = 0.0f;
            
            displayValues[0] = 0.5f + (presetKnobValues[AutotuneMono][0] * 0.49f); //fidelity
            tRetune_setFidelityThreshold(&autotuneMono, displayValues[0]);
            displayValues[1] = LEAF_clip(0.0f, presetKnobValues[AutotuneMono][1] * 1.1f, 1.0f); // amount of forcing to new pitch
            displayValues[2] = presetKnobValues[AutotuneMono][2]; //speed to get to desired pitch shift
            
            displayValues[3] = presetKnobValues[AutotuneMono][3] * 12.0f;
            displayValues[4] = (presetKnobValues[AutotuneMono][4] * 0.5f) + 0.5f;
            
            
            
            if (displayValues[2] > .90f)
            {
                displayValues[2] = 1.0f;
            }
            tExpSmooth_setFactor(&neartune_smoother, expBuffer[(int)(displayValues[2] * displayValues[2] * displayValues[2] * expBufferSizeMinusOne)]);
            float destFactor = tExpSmooth_tick(&neartune_smoother);
            
            float detectedPeriod = tRetune_getInputPeriod(&autotuneMono);
            if (detectedPeriod > 0.0f)
            {
                
                
                detectedNote = LEAF_frequencyToMidi(1.0f / detectedPeriod);
                
                desiredSnap = nearestNoteWithHysteresis(detectedNote, displayValues[4]);
                
                if (desiredSnap > 0.0f)
                {
                    
                    
                    destinationNote = (desiredSnap * displayValues[1]) + (detectedNote * (1.0f - displayValues[1]));
                    
                    factorDiff = (fabsf(destinationNote-lastSnap));
                    changeAmount = (fabsf(destinationNote-detectedNote));
                    
                    //if ((factorDiff < displayValues[3]) || (updatePitch == 1) || (diffCounter > maxDiffCounter))
                    
                    if ((changeAmount < 11.9))
                    {
                        destinationFactor = (LEAF_midiToFrequency(destinationNote) / LEAF_midiToFrequency(detectedNote));
                        tExpSmooth_setDest(&neartune_smoother, destinationFactor);
                        lastSnap = destinationNote;
                    }
                    
                }
                else
                {
                    tExpSmooth_setDest(&neartune_smoother, 1.0f);
                }
                
            }
            
            
            
            
            tRetune_setPitchFactor(&autotuneMono, destFactor, 0);
            float* samples = tRetune_tick(&autotuneMono, input[1]);
            //tAutotune_setFreq(&autotuneMono, leaf.sampleRate / nearestPeriod(tAutotune_getInputPeriod(&autotuneMono)), 0);
            
            float fades[2];
            LEAF_crossfade(tRamp_tick(&nearWetRamp), fades);
            
            sample = samples[0] * fades[0];
            sample += input[1] * fades[1]; // crossfade to dry signal if no notes held down.
            input[0] = sample;
            input[1] = sample;
        }
        
        void SFXNeartuneFree(void)
        {
            tRetune_free(&autotuneMono);
            tExpSmooth_free(&neartune_smoother);
            tRamp_free(&nearWetRamp);
        }
        
        
        
        //6 autotune
        void SFXAutotuneAlloc()
        {
            tAutotune_initToPool(&autotunePoly, NUM_AUTOTUNE, 1024, 512, &mediumPool);
            tSimplePoly_setNumVoices(&poly, NUM_AUTOTUNE);
            
            //tAutotune_init(&autotunePoly, NUM_AUTOTUNE, 2048, 1024); //old settings
        }
        
        void SFXAutotuneFrame()
        {
            for (int i = 0; i < tSimplePoly_getNumVoices(&poly); ++i)
            {
                calculateFreq(i);
                tExpSmooth_setDest(&polyRamp[i], (tSimplePoly_getVelocity(&poly, i) > 0));
            }
            int tempNumVoices = tSimplePoly_getNumActiveVoices(&poly);
            if (tempNumVoices != 0) tExpSmooth_setDest(&comp, 1.0f / (float)tempNumVoices);
        }
        
        void SFXAutotuneTick(float* input)
        {
            float sample = 0.0f;
            displayValues[0] = 0.5f + (presetKnobValues[AutotunePoly][0] * 0.47f);
            
            //displayValues[1] = presetKnobValues[AutotunePoly][1];
            
            //displayValues[2] = presetKnobValues[AutotunePoly][2];
            
            tAutotune_setFidelityThreshold(&autotunePoly, displayValues[0]);
            //tAutotune_setAlpha(&autotunePoly, displayValues[1]);
            //tAutotune_setTolerance(&autotunePoly, displayValues[2]);
            
            
            for (int i = 0; i < tSimplePoly_getNumVoices(&poly); ++i)
            {
                tAutotune_setFreq(&autotunePoly, freq[i], i);
            }
            
            float* samples = tAutotune_tick(&autotunePoly, input[1]);
            
            for (int i = 0; i < tSimplePoly_getNumVoices(&poly); ++i)
            {
                sample += samples[i] * tExpSmooth_tick(&polyRamp[i]);
            }
            sample *= tExpSmooth_tick(&comp);
            input[0] = sample;
            input[1] = sample;
        }
        
        void SFXAutotuneFree(void)
        {
            tAutotune_free(&autotunePoly);
        }
        
        
        //7 sampler - button press
        int samplePlayStart = 0;
        int samplePlayLength = 0;
        float sampleLength = 0.0f;
        int crossfadeLength = 0;
        float samplerRate = 1.0f;
        int samplePlaying = 1;
        tExpSmooth startSmooth;
        tExpSmooth lengthSmooth;
        int bpMode = 0;
        
        void SFXSamplerBPAlloc()
        {
            tBuffer_initToPool(&buff, leaf.sampleRate * 172.0f, &largePool);
            tBuffer_setRecordMode(&buff, RecordOneShot);
            tSampler_init(&sampler, &buff);
            tSampler_setMode(&sampler, (PlayMode)(bpMode + 1));
            tExpSmooth_init(&startSmooth, 0.0f, 0.01f);
            tExpSmooth_init(&lengthSmooth, 0.0f, 0.01f);
        }
        
        void SFXSamplerBPFrame()
        {
            
        }
        
        
        
        void SFXSamplerBPTick(float* input)
        {
            float sample = 0.0f;
            int recordPosition = tBuffer_getRecordPosition(&buff);
            float* knobs = presetKnobValues[SamplerButtonPress];
            
            if (buttonActionsSFX[ButtonC][ActionPress])
            {
                if (samplePlaying)
                {
                    samplePlaying = 0;
                    tSampler_stop(&sampler);
                    displayValues[1] = LEAF_clip(0.0f, knobs[1] * sampleLength, sampleLength * (1.0f - knobs[0]));
                }
                else
                {
                    samplePlaying = 1;
                    tSampler_play(&sampler);
                }
                buttonActionsSFX[ButtonC][ActionPress] = 0;
            }
            
            if (buttonActionsSFX[ButtonB][ActionPress])
            {
                bpMode = !bpMode;
                tSampler_setMode(&sampler, (PlayMode)(bpMode + 1));
                buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(bpMode);
            }
            
            if (buttonActionsSFX[ButtonA][ActionPress])
            {
                tSampler_stop(&sampler);
                tBuffer_record(&buff);
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(1);
            }
            if (buttonActionsSFX[ButtonA][ActionRelease])
            {
                tBuffer_stop(&buff);
                if (samplePlaying) tSampler_play(&sampler);
                buttonActionsSFX[ButtonA][ActionRelease] = 0;
                setLED_A(0);
            }
            
            
            
            sampleLength = (float)recordPosition * leaf.invSampleRate;
            displayValues[0] = knobs[0] * sampleLength;
            displayValues[1] = LEAF_clip(0.0f, knobs[1] * sampleLength, sampleLength * (1.0f - knobs[0]));
            displayValues[2] = (knobs[2] - 0.5f) * 4.0f;
            float rate = roundf((knobs[3] - 0.5f) * 14.0f);
            if (rate < 0.0f)
            {
                (rate = 1.0f / fabsf(rate-1.0f));
            }
            else
            {
                rate += 1.0f;
            }
            displayValues[3] = rate;
            
            displayValues[4] = knobs[4] * 4000.0f;
            
            samplerRate = displayValues[3] * displayValues[2];
            
            tExpSmooth_setDest(&startSmooth, knobs[0] * recordPosition);
            tExpSmooth_setDest(&lengthSmooth, knobs[1] * recordPosition);
            
            samplePlayStart = tExpSmooth_tick(&startSmooth);
            samplePlayLength = tExpSmooth_tick(&lengthSmooth);
            crossfadeLength = displayValues[4];
            tSampler_setStart(&sampler, samplePlayStart);
            tSampler_setLength(&sampler, samplePlayLength);
            tSampler_setRate(&sampler, samplerRate);
            tSampler_setCrossfadeLength(&sampler, crossfadeLength);
            
            tBuffer_tick(&buff, input[1]);
            sample = tanhf(tSampler_tick(&sampler));
            input[0] = sample;
            input[1] = sample;
        }
        
        void SFXSamplerBPFree(void)
        {
            tBuffer_free(&buff);
            tSampler_free(&sampler);
            tExpSmooth_free(&startSmooth);
            tExpSmooth_free(&lengthSmooth);
        }
        
        // keyboard sampler
        int currentSamplerKeyGlobal = 60 - LOWEST_SAMPLER_KEY;
        int samplerKeyHeld[NUM_SAMPLER_KEYS];
        
        tExpSmooth kSamplerGains[NUM_SAMPLER_KEYS];
        int waitingForDeactivation[NUM_SAMPLER_VOICES];
        int controlAllKeys = 0;
        int prevSamplerKey = 60;
        
        float samp_thresh = 0.0f;
        int detectedAttackPos[NUM_SAMPLER_KEYS];
        float sampleRates[NUM_SAMPLER_KEYS];
        float sampleRatesMult[NUM_SAMPLER_KEYS];
        int loopOns[NUM_SAMPLER_KEYS];
        float samplePlayStarts[NUM_SAMPLER_KEYS];
        float samplePlayLengths[NUM_SAMPLER_KEYS];
        float crossfadeLengths[NUM_SAMPLER_KEYS];
        float prevKnobs[6];
        
        void SFXSamplerKAlloc()
        {
            leaf.clearOnAllocation = 0; //needs this in case the box loads on this one first
            currentSamplerKeyGlobal = 60 - LOWEST_SAMPLER_KEY;
            
            for (int i = 0; i < NUM_SAMPLER_KEYS; i++)
            {
                //leaf.sampleRate * 172.0f
                
                tBuffer_initToPool(&keyBuff[i], leaf.sampleRate * 3.5f, &largePool);
                tBuffer_setRecordMode(&keyBuff[i], RecordOneShot);
                tSampler_init(&keySampler[i], &keyBuff[i]);
                tSampler_setMode(&keySampler[i], PlayLoop);
                
                samplePlayStarts[i] = 0;
                samplePlayLengths[i] = 0;
                detectedAttackPos[i] = 0;
                crossfadeLengths[i] = 1000;
                samplerKeyHeld[i] = 0;
                tExpSmooth_init(&kSamplerGains[i], 0.0f, 0.04f);
                sampleRates[i] = 1.0f;
                sampleRatesMult[i] = 1.0f;
                loopOns[i] = 1;
            }
            tSimplePoly_setNumVoices(&poly, NUM_SAMPLER_VOICES);
            for (int i = 0; i < NUM_SAMPLER_VOICES; i++)
            {
                waitingForDeactivation[i] = -1;
            }
            setLED_B(controlAllKeys);
            samp_thresh = 0.02f;
        }
        
        
        
        
        void SFXSamplerKFrame()
        {
            int currentSamplerKey = currentSamplerKeyGlobal;
            if (samplerKeyHeld[currentSamplerKey])
            {
                if ((tBuffer_isActive(&keyBuff[currentSamplerKey])) || (currentSamplerKey != prevSamplerKey)) //only write if recording
                {
                    buttonActionsUI[ExtraMessage][ActionHoldContinuous] = 1;
                    writeButtonFlag = ExtraMessage;
                    writeActionFlag = ActionHoldContinuous;
                }
                prevSamplerKey = currentSamplerKey;
            }
            
            if (buttonActionsSFX[ButtonA][ActionPress])
            {
                tBuffer_setRecordPosition(&keyBuff[currentSamplerKey],0);
                tBuffer_setRecordedLength(&keyBuff[currentSamplerKey],0);
                buttonActionsSFX[ButtonA][ActionPress] = 0;
            }
            if (buttonActionsSFX[ButtonB][ActionPress])
            {
                buttonActionsSFX[ButtonB][ActionPress] = 0;
                controlAllKeys = !controlAllKeys;
                setLED_B(controlAllKeys);
            }
            
            if (buttonActionsSFX[ButtonC][ActionPress])
            {
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                if (!controlAllKeys)
                {
                    
                    float currentPower = 0.0f;
                    float previousPower = 0.0f;
                    int buffLength = tBuffer_getRecordedLength(&keyBuff[currentSamplerKey]);
                    if (detectedAttackPos[currentSamplerKey] > 0)
                    {
                        detectedAttackPos[currentSamplerKey] += 4800;
                        previousPower = tBuffer_get(&keyBuff[currentSamplerKey], (detectedAttackPos[currentSamplerKey] -1) % buffLength);
                    }
                    
                    int foundAttack = 0;
                    int i = 0;
                    while(foundAttack == 0)
                    {
                        //starts at previous detected attack position, and wraps around to read the whole buffer
                        float testSample = tBuffer_get(&keyBuff[currentSamplerKey], (i + detectedAttackPos[currentSamplerKey]) % buffLength);
                        currentPower = testSample*testSample;
                        
                        if ((currentPower > samp_thresh) && (currentPower > (previousPower + 0.0005f)))
                        {
                            //found one!
                            
                            //back up a few samples to catch the full attack
                            int thePos = (i + detectedAttackPos[currentSamplerKey] - 480) % buffLength;
                            if (thePos < 0)
                            {
                                thePos = 0;
                            }
                            samplePlayStarts[currentSamplerKey] = thePos;
                            detectedAttackPos[currentSamplerKey] = thePos;
                            foundAttack = 1;
                            OLEDclearLine(SecondLine);
                            OLEDwriteString("ATKDETECT ", 10, 0, SecondLine);
                            OLEDwriteFloat((samplePlayStarts[currentSamplerKey] / (float)buffLength) * (buffLength * leaf.invSampleRate), OLEDgetCursor(), SecondLine);
                        }
                        i++;
                        
                        //if finished searching the whole buffer
                        if (i >= buffLength)
                        {
                            // didn't find one, put it back at the start
                            detectedAttackPos[currentSamplerKey] = 0;
                            OLEDclearLine(SecondLine);
                            OLEDwriteString("NO ATK FOUND ", 10, 0, SecondLine);
                            foundAttack = 1;
                        }
                    }
                }
                
                else
                {
                    for (int i = 0; i < NUM_SAMPLER_VOICES; i++)
                    {
                        for (int key = 0; key < NUM_SAMPLER_KEYS; key++)
                        {
                            
                            int buffLength = tBuffer_getRecordedLength(&keyBuff[key]);
                            if (buffLength > 0)
                            {
                                float currentPower = 0.0f;
                                float previousPower = 0.0f;
                                int foundAttack = 0;
                                int i = 0;
                                if (detectedAttackPos[key] > 0)
                                {
                                    detectedAttackPos[key] += 4800;
                                    previousPower = tBuffer_get(&keyBuff[key], (detectedAttackPos[key] -1) % buffLength);
                                }
                                
                                while(foundAttack == 0)
                                {
                                    //starts at previous detected attack position, and wraps around to read the whole buffer
                                    float testSample = tBuffer_get(&keyBuff[key], (i + detectedAttackPos[key]) % buffLength);
                                    currentPower = testSample*testSample;
                                    
                                    if ((currentPower > samp_thresh) && (currentPower > (previousPower + 0.0005f)))
                                    {
                                        //found one!
                                        
                                        //back up a few samples to catch the full attack
                                        int thePos = (i + detectedAttackPos[key] - 480) % buffLength;
                                        if (thePos < 0)
                                        {
                                            thePos = 0;
                                        }
                                        samplePlayStarts[key] = thePos;
                                        detectedAttackPos[key] = thePos;
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
        }
        
        //TODO:  marking this spot
        
        
        
        
        void SFXSamplerKTick(float* input)
        {
            float sample = 0.0f;
            float* knobs = presetKnobValues[SamplerKeyboard];
            
            int currentSamplerKey = currentSamplerKeyGlobal;
            
            if (!controlAllKeys)
            {
                int recordedLength = tBuffer_getRecordedLength(&keyBuff[currentSamplerKey]);
                sampleLength = recordedLength * leaf.invSampleRate;
                
                displayValues[0] = knobs[0] * sampleLength;
                
                displayValues[1] = LEAF_clip(0.0f, knobs[1] * sampleLength, sampleLength * (1.0f - knobs[0]));
                
                displayValues[2] = (knobs[2] - 0.5f) * 4.0f;
                
                
                float rate = roundf((knobs[3] - 0.5f) * 14.0f);
                if (rate < 0.0f)
                {
                    (rate = 1.0f / fabsf(rate-1.0f));
                }
                else
                {
                    rate += 1.0f;
                }
                displayValues[3] = rate;
                
                displayValues[4] = roundf(knobs[4]);
                
                
                displayValues[5] = knobs[5] * 4000.0f;
                
                displayValues[6] = knobs[6];
                
                //check if display values are new
                if (fabsf(knobs[0]-prevKnobs[0]) > 0.0005f)
                {
                    samplePlayStarts[currentSamplerKey]= (knobs[0] * recordedLength);// + detectedAttackPos[currentSamplerKey];
                    
                }
                
                if (fabsf(knobs[1]-prevKnobs[1])  > 0.0005f)
                {
                    samplePlayLengths[currentSamplerKey] = (knobs[1] * recordedLength);// - detectedAttackPos[currentSamplerKey];
                    
                }
                
                if (fabsf(knobs[2]-prevKnobs[2])  > 0.0005f)
                {
                    sampleRates[currentSamplerKey] = displayValues[2];
                    
                }
                
                if (fabsf(knobs[3]-prevKnobs[3])  > 0.0005f)
                {
                    sampleRatesMult[currentSamplerKey] = displayValues[3];
                    
                }
                
                
                
                if (fabsf(knobs[4]-prevKnobs[4]) > 0.0005f)
                {
                    
                    loopOns[currentSamplerKey] = roundf(knobs[4]);
                    
                }
                
                if (fabsf(knobs[5]-prevKnobs[5])> 0.0005f)
                {
                    
                    crossfadeLengths[currentSamplerKey] = displayValues[5];
                }
                
                
                tSampler_setStart(&keySampler[currentSamplerKey], samplePlayStarts[currentSamplerKey]);
                tSampler_setLength(&keySampler[currentSamplerKey], samplePlayLengths[currentSamplerKey]);
                tSampler_setCrossfadeLength(&keySampler[currentSamplerKey], crossfadeLengths[currentSamplerKey]);
                tSampler_setRate(&keySampler[currentSamplerKey], sampleRates[currentSamplerKey] * sampleRatesMult[currentSamplerKey]);
                tSampler_setMode(&keySampler[currentSamplerKey], (PlayMode)loopOns[currentSamplerKey]);
                
                for (int i = 0; i < NUM_SAMPLER_VOICES; ++i)
                {
                    if (tSimplePoly_isOn(&poly, i) > 0)
                    {
                        int key = tSimplePoly_getPitch(&poly, i) - LOWEST_SAMPLER_KEY;
                        if ((0 <= key) && (key < NUM_SAMPLER_KEYS))
                        {
                            tBuffer_tick(&keyBuff[key], input[1]);
                        }
                    }
                }
            }
            
            else
            {
                
                for (int i = 0; i < NUM_SAMPLER_VOICES; i++)
                {
                    displayValues[0] = knobs[0];
                    
                    displayValues[1] = LEAF_clip(0.0f, knobs[1], (1.0f - knobs[0]));
                    
                    displayValues[2] = (knobs[2] - 0.5f) * 4.0f;
                    
                    
                    
                    float rate = roundf((knobs[3] - 0.5f) * 14.0f);
                    if (rate < 0.0f)
                    {
                        (rate = 1.0f / fabsf(rate-1.0f));
                    }
                    else
                    {
                        rate += 1.0f;
                    }
                    displayValues[3] = rate;
                    
                    displayValues[4] = roundf(knobs[4]);
                    
                    displayValues[5] = knobs[5] * 4000.0f;
                    
                    displayValues[6] = knobs[6];
                    
                    //get the keys sounding right now
                    if (tSimplePoly_isOn(&poly, i) > 0)
                    {
                        int key = tSimplePoly_getPitch(&poly, i) - LOWEST_SAMPLER_KEY;
                        if ((0 <= key) && (key < NUM_SAMPLER_KEYS))
                        {
                            tBuffer_tick(&keyBuff[key], input[1]);
                            
                            int recordedLength = tBuffer_getRecordedLength(&keyBuff[key]);
                            sampleLength = recordedLength * leaf.invSampleRate;
                            
                            //check if display values are new
                            if (knobs[0] != prevKnobs[0])
                            {
                                samplePlayStarts[key]= (knobs[0] * recordedLength);
                                
                            }
                            
                            if (knobs[1] != prevKnobs[1])
                            {
                                samplePlayLengths[key] = (knobs[1] * recordedLength);
                                
                            }
                            
                            if (knobs[2] != prevKnobs[2])
                            {
                                sampleRates[key] = displayValues[2];
                                
                            }
                            
                            if (knobs[3] != prevKnobs[3])
                            {
                                sampleRatesMult[key] = displayValues[3];
                                
                            }
                            
                            
                            
                            if (knobs[4] != prevKnobs[4])
                            {
                                
                                loopOns[key] = roundf(knobs[4]);
                                
                            }
                            
                            if (knobs[5] != prevKnobs[5])
                            {
                                
                                crossfadeLengths[key] = displayValues[5];
                            }
                            
                            tSampler_setStart(&keySampler[key], samplePlayStarts[key]);
                            tSampler_setLength(&keySampler[key], samplePlayLengths[key]);
                            tSampler_setCrossfadeLength(&keySampler[key], crossfadeLengths[key]);
                            tSampler_setRate(&keySampler[key], sampleRates[key] * sampleRatesMult[key]);
                            tSampler_setMode(&keySampler[key], (PlayMode)loopOns[key]);
                            
                        }
                        
                    }
                }
                
            }
            
            
            for (int i = 0; i < 6; i++)
            {
                prevKnobs[i] = knobs[i];
            }
            
            for (int i = 0; i < NUM_SAMPLER_KEYS; i++)
            {
                float tempGain = tExpSmooth_tick(&kSamplerGains[i]);
                if ( tempGain > 0.001f)
                {
                    sample += tSampler_tick(&keySampler[i]) * tempGain;
                }
                else
                {
                    for (int j = 0; j< NUM_SAMPLER_VOICES; j++)
                    {
                        if (waitingForDeactivation[j] == (i + LOWEST_SAMPLER_KEY))
                        {
                            tSimplePoly_deactivateVoice(&poly, j);
                            waitingForDeactivation[j] = -1;
                        }
                    }
                }
            }
            
            
            sample = tanhf(sample) * 0.98f;
            input[0] = sample;
            input[1] = sample;
        }
        
        
        void SFXSamplerKFree(void)
        {
            for (int i = 0; i < NUM_SAMPLER_KEYS; i++)
            {
                tBuffer_free(&keyBuff[i]);
                tSampler_free(&keySampler[i]);
                tExpSmooth_free(&kSamplerGains[i]);
            }
        }
        
        
        
        //8 sampler - auto
        
        volatile float currentPower = 0.0f;
        volatile float previousPower = 0.0f;
        
        volatile int samp_triggered = 0;
        uint32_t sample_countdown = 0;
        PlayMode samplerMode = PlayLoop;
        int triggerChannel = 0;
        int currentSampler = 0;
        int pitchQuantization = 0;
        int randLengthVal = 0;
        float randRateVal = 0.0f;
        
        
        tExpSmooth cfxSmooth;
#define MAX_AUTOSAMP_LENGTH 192000
        
        void SFXSamplerAutoAlloc()
        {
            tBuffer_initToPool(&asBuff[0], MAX_AUTOSAMP_LENGTH, &largePool);
            tBuffer_setRecordMode(&asBuff[0], RecordOneShot);
            tBuffer_initToPool(&asBuff[1], MAX_AUTOSAMP_LENGTH, &largePool);
            tBuffer_setRecordMode(&asBuff[1], RecordOneShot);
            tSampler_init(&asSampler[0], &asBuff[0]);
            tSampler_setMode(&asSampler[0], PlayLoop);
            tSampler_init(&asSampler[1], &asBuff[1]);
            tSampler_setMode(&asSampler[1], PlayLoop);
            
            tEnvelopeFollower_init(&envfollow, 0.00001f, 0.9999f);
            tExpSmooth_init(&cfxSmooth, 0.0f, 0.01f);
            
            setLED_A(samplerMode == PlayBackAndForth);
            setLED_B(triggerChannel);
            currentSampler = 1;
            sample_countdown = 0;
            randLengthVal = leaf.random() * 10000.0f;
            randRateVal = (leaf.random() - 0.5f) * 4.0f;
            
            setLED_C(pitchQuantization);
        }
        
        void SFXSamplerAutoFrame()
        {
            if (buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                pitchQuantization = !pitchQuantization;
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(pitchQuantization);
            }
            
            
        }
        
        int fadeDone = 0;
        int finalWindowSize = 5000;
        
        void SFXSamplerAutoTick(float* input)
        {
            float sample = 0.0f;
            if (triggerChannel > 0)
            {
                currentPower = tEnvelopeFollower_tick(&envfollow, input[0]);
            }
            else
            {
                currentPower = tEnvelopeFollower_tick(&envfollow, input[1]);
            }
            
            float* knobs = presetKnobValues[SamplerAutoGrab];
            
            samp_thresh = 1.0f - knobs[0];
            displayValues[0] = samp_thresh;
            
            int window_size = expBuffer[(int)(knobs[1] * expBufferSizeMinusOne)] * MAX_AUTOSAMP_LENGTH;
            displayValues[1] = window_size;
            
            float rate;
            if (pitchQuantization)
            {
                rate = roundf((knobs[2] - 0.5f) * 14.0f);
                if (rate < 0.0f) rate = 1.0f / fabsf(rate-1.0f);
                else rate += 1.0f;
            }
            else
            {
                rate = (knobs[2] - 0.5f) * 4.0f;
            }
            displayValues[2] = rate;
            
            crossfadeLength = knobs[3] * 1000.0f;
            displayValues[3] = crossfadeLength;
            
            float randLengthAmount = knobs[5] * 5000.0f;
            if (randLengthAmount < 20.0f) randLengthAmount = 0.0f;
            displayValues[5] = randLengthAmount;
            
            float randRateAmount;
            
            if (pitchQuantization)
            {
                randRateAmount = roundf(knobs[6] * 8.0f);
            }
            else
            {
                randRateAmount = knobs[6] * 2.0f;
                if (randRateAmount < 0.01) randRateAmount = 0.0f;
            }
            
            displayValues[6] = randRateAmount;
            
            
            tSampler_setCrossfadeLength(&asSampler[0], crossfadeLength);
            tSampler_setCrossfadeLength(&asSampler[1], crossfadeLength);
            
            
            if ((currentPower > (samp_thresh)) && (currentPower > (previousPower + 0.001f)) && (samp_triggered == 0) && (sample_countdown == 0) && (fadeDone == 1))
            {
                randLengthVal = (leaf.random() - 0.5f) * randLengthAmount * 2.0f;
                if (pitchQuantization) randRateVal = roundf(leaf.random() * randRateAmount) + 1.0f;
                else randRateVal = (leaf.random() - 0.5f) * randRateAmount * 2.0f;
                
                samp_triggered = 1;
                setLED_1(1);
                
                finalWindowSize = LEAF_clip(4, window_size + randLengthVal, MAX_AUTOSAMP_LENGTH);
                sample_countdown = finalWindowSize;
                tSampler_stop(&asSampler[!currentSampler]);
                tBuffer_record(&asBuff[!currentSampler]);
            }
            
            
            tBuffer_tick(&asBuff[0], input[1]);
            tBuffer_tick(&asBuff[1], input[1]);
            
            if (sample_countdown > 0)
            {
                sample_countdown--;
            }
            else if (samp_triggered == 1)
            {
                setLED_1(0);
                
                currentSampler = !currentSampler;
                
                tSampler_play(&asSampler[currentSampler]);
                
                tExpSmooth_setDest(&cfxSmooth,(float)currentSampler);
                
                samp_triggered = 0;
                fadeDone = 0;
            }
            
            if (pitchQuantization)
            {
                tSampler_setRate(&asSampler[0], rate * randRateVal);
                tSampler_setRate(&asSampler[1], rate * randRateVal);
            }
            else
            {
                tSampler_setRate(&asSampler[0], rate + randRateVal);
                tSampler_setRate(&asSampler[1], rate + randRateVal);
            }
            finalWindowSize = LEAF_clip(4, window_size + randLengthVal, MAX_AUTOSAMP_LENGTH);
            tSampler_setEnd(&asSampler[0], finalWindowSize);
            tSampler_setEnd(&asSampler[1], finalWindowSize);
            
            if (buttonActionsSFX[ButtonA][ActionPress])
            {
                if (samplerMode == PlayLoop)
                {
                    tSampler_setMode(&asSampler[0], PlayBackAndForth);
                    tSampler_setMode(&asSampler[1], PlayBackAndForth);
                    samplerMode = PlayBackAndForth;
                    setLED_A(1);
                    buttonActionsSFX[ButtonA][ActionPress] = 0;
                }
                else if (samplerMode == PlayBackAndForth)
                {
                    tSampler_setMode(&asSampler[0], PlayLoop);
                    tSampler_setMode(&asSampler[1], PlayLoop);
                    samplerMode = PlayLoop;
                    setLED_A(0);
                    buttonActionsSFX[ButtonA][ActionPress] = 0;
                }
            }
            if (buttonActionsSFX[ButtonB][ActionPress])
            {
                triggerChannel = (triggerChannel > 0) ? 0 : 1;
                buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(triggerChannel);
            }
            
            float fade = tExpSmooth_tick(&cfxSmooth);
            if (fabsf(cfxSmooth->curr - cfxSmooth->dest) < 0.00001f)
            {
                fadeDone = 1;
            }
            float volumes[2];
            LEAF_crossfade((fade * 2.0f) - 1.0f, volumes);
            sample = (tSampler_tick(&asSampler[0]) * volumes[1]) + (tSampler_tick(&asSampler[1]) * volumes[0]);
            
            previousPower = currentPower;
            input[0] = sample * 0.99f;
            input[1] = sample * 0.99f;
            
        }
        
        void SFXSamplerAutoFree(void)
        {
            tBuffer_free(&asBuff[0]);
            tBuffer_free(&asBuff[1]);
            tSampler_free(&asSampler[0]);
            tSampler_free(&asSampler[1]);
            tEnvelopeFollower_free(&envfollow);
            tExpSmooth_free(&cfxSmooth);
        }
        
        //10 distortion tanh
        int distortionMode = 0;
        tVZFilter bell1;
        int distOS_ratio = 4;
        
        void SFXDistortionAlloc()
        {
            leaf.clearOnAllocation = 1;
            tOversampler_init(&oversampler, distOS_ratio, 0);
            tVZFilter_init(&shelf1, Lowshelf, 80.0f, 6.0f);
            tVZFilter_init(&shelf2, Highshelf, 12000.0f, 6.0f);
            tVZFilter_init(&bell1, Bell, 1000.0f, 1.9f);
            tVZFilter_setSampleRate(&shelf1, leaf.sampleRate * distOS_ratio);
            tVZFilter_setSampleRate(&shelf2, leaf.sampleRate * distOS_ratio);
            tVZFilter_setSampleRate(&bell1, leaf.sampleRate * distOS_ratio);
            setLED_A(distortionMode);
            
            leaf.clearOnAllocation = 0;
        }
        
        void SFXDistortionFrame()
        {
            if (buttonActionsSFX[ButtonA][ActionPress])
            {
                distortionMode = !distortionMode;
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(distortionMode);
            }
            displayValues[1] = (presetKnobValues[Distortion][1] * 30.0f) - 15.0f;
            displayValues[2] = (presetKnobValues[Distortion][2] * 34.0f) - 17.0f;
            displayValues[3] = faster_mtof(presetKnobValues[Distortion][3] * 77.0f + 42.0f);
            
            tVZFilter_setGain(&shelf1, fastdbtoa(-1.0f * displayValues[1]));
            tVZFilter_setGain(&shelf2, fastdbtoa(displayValues[1]));
            tVZFilter_setFreq(&bell1, displayValues[3]);
            tVZFilter_setGain(&bell1, fastdbtoa(displayValues[2]));
            
        }
        
        void SFXDistortionTick(float* input)
        {
            //knob 0 = gain
            
            float sample = input[1];
            displayValues[0] = ((presetKnobValues[Distortion][0] * 20.0f) + 1.0f); // 15.0f
            displayValues[4] = presetKnobValues[Distortion][4]; // 15.0f
            sample = sample * displayValues[0];
            
            tOversampler_upsample(&oversampler, sample, oversamplerArray);
            for (int i = 0; i < distOS_ratio; i++)
            {
                if (distortionMode > 0) oversamplerArray[i] = LEAF_shaper(oversamplerArray[i], 1.0f);
                else oversamplerArray[i] = tanhf(oversamplerArray[i]);
                oversamplerArray[i] = tVZFilter_tick(&shelf1, oversamplerArray[i]); //put it through the low shelf
                oversamplerArray[i] = tVZFilter_tick(&shelf2, oversamplerArray[i]); // now put that result through the high shelf
                oversamplerArray[i] = tVZFilter_tick(&bell1, oversamplerArray[i]); // now add a bell (or peaking eq) filter
                oversamplerArray[i] = tanhf(oversamplerArray[i] * presetKnobValues[Distortion][4]) * 0.95f;
            }
            sample = tOversampler_downsample(&oversampler, oversamplerArray);
            
            input[0] = sample;
            input[1] = sample;
            
        }
        
        void SFXDistortionFree(void)
        {
            tOversampler_free(&oversampler);
            tVZFilter_free(&shelf1);
            tVZFilter_free(&shelf2);
            tVZFilter_free(&bell1);
        }
        
        // distortion wave folder
        
        
        int foldMode = 0;
        
        float oversampleBuf[2];
        
        void SFXWaveFolderAlloc()
        {
            leaf.clearOnAllocation = 1;
            tLockhartWavefolder_init(&wavefolder1);
            tLockhartWavefolder_init(&wavefolder2);
            tHighpass_init(&wfHP, 10.0f);
            tOversampler_init(&oversampler, 2, 0);
            setLED_A(foldMode);
            leaf.clearOnAllocation = 0;
        }
        
        void SFXWaveFolderFrame()
        {
            if (buttonActionsSFX[ButtonA][ActionPress])
            {
                foldMode = !foldMode;
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(foldMode);
            }
            
            //knobParams[3] = (smoothedADC[3] * 2.0f) - 1.0f;
        }
        
        void SFXWaveFolderTick(float* input)
        {
            //knob 0 = gain
            float sample = input[1];
            
            displayValues[0] = (presetKnobValues[Wavefolder][0] * 4.0f);
            
            displayValues[1] = presetKnobValues[Wavefolder][1] - 0.5f;
            
            displayValues[2] = presetKnobValues[Wavefolder][2] - 0.5f;
            float gain = displayValues[0];
            displayValues[3] = presetKnobValues[Wavefolder][3];
            
            
            //sample = sample * gain * 0.33f;
            
            sample = sample * gain;
            //sample = sample + knobParams[1];
            if (foldMode == 0)
            {
                tOversampler_upsample(&oversampler, sample, oversamplerArray);
                for (int i = 0; i < 2; i++)
                {
                    oversamplerArray[i] = sample + displayValues[1];
                    oversamplerArray[i] *= displayValues[0];
                    oversamplerArray[i] = tanhf(oversamplerArray[i]);
                    //oversamplerArray[i] *= knobParams[0] * 1.5f;
                    
                    oversamplerArray[i] = tLockhartWavefolder_tick(&wavefolder1, oversamplerArray[i]);
                    //oversamplerArray[i] = tLockhartWavefolder_tick(&wavefolder2, oversamplerArray[i]);
                    //sample = sample * gain;
                    
                    //oversamplerArray[i] = tLockhartWavefolder_tick(&wavefolder2, oversamplerArray[i]);
                    //oversamplerArray[i] = sample + knobParams[3];
                    //sample *= .6f;
                    //oversamplerArray[i] = tLockhartWavefolder_tick(&wavefolder3, oversamplerArray[i]);
                    //sample = tLockhartWavefolder_tick(&wavefolder4, sample);
                    //oversamplerArray[i] *= .8f;
                    oversamplerArray[i] = tanhf(oversamplerArray[i]);
                }
                sample = tHighpass_tick(&wfHP, tOversampler_downsample(&oversampler, oversamplerArray)) * displayValues[3];
                input[0] = sample;
                input[1] = sample;
            }
            else
            {
                
                sample = sample + displayValues[1];
                sample *= displayValues[0];
                sample = LEAF_tanh(sample);
                //oversamplerArray[i] *= knobParams[0] * 1.5f;
                
                sample = tLockhartWavefolder_tick(&wavefolder1, sample);
                
                
                
                sample = sample + displayValues[2];
                sample *= displayValues[0];
                sample = LEAF_tanh(sample);
                //oversamplerArray[i] *= knobParams[0] * 1.5f;
                
                sample = tLockhartWavefolder_tick(&wavefolder2, sample);
                
                sample = tOversampler_tick(&oversampler, sample, oversampleBuf, &LEAF_tanh);
                sample = tHighpass_tick(&wfHP, sample) * displayValues[3];
                //sample *= 0.99f;
                input[0] = sample;
                input[1] = sample;
            }
            
        }
        
        void SFXWaveFolderFree(void)
        {
            tLockhartWavefolder_free(&wavefolder1);
            tLockhartWavefolder_free(&wavefolder2);
            tHighpass_free(&wfHP);
            tOversampler_free(&oversampler);
        }
        
        int crusherStereo = 0;
        //13 bitcrusher
        void SFXBitcrusherAlloc()
        {
            tCrusher_init(&crush);
            tCrusher_init(&crush2);
            setLED_A(crusherStereo);
        }
        
        void SFXBitcrusherFrame()
        {
            if (buttonActionsSFX[ButtonA][ActionPress])
            {
                crusherStereo = !crusherStereo;
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(crusherStereo);
            }
        }
        
        void SFXBitcrusherTick(float* input)
        {
            float sample = 0.0f;
            displayValues[0] = (presetKnobValues[BitCrusher][0] * 0.99f )+ 0.01f;
            tCrusher_setQuality (&crush, presetKnobValues[BitCrusher][0]);
            tCrusher_setQuality (&crush2, presetKnobValues[BitCrusher][0]);
            displayValues[1] = presetKnobValues[BitCrusher][1];
            tCrusher_setSamplingRatio (&crush, presetKnobValues[BitCrusher][1] * 0.5f);
            tCrusher_setSamplingRatio (&crush2, presetKnobValues[BitCrusher][1] * 0.5f);
            displayValues[2] = presetKnobValues[BitCrusher][2] * 0.1f;
            tCrusher_setRound (&crush, displayValues[2]);
            tCrusher_setRound (&crush2, displayValues[2]);
            displayValues[3] = (uint32_t) (presetKnobValues[BitCrusher][3] * 8.0f);
            tCrusher_setOperation (&crush, presetKnobValues[BitCrusher][3]);
            tCrusher_setOperation (&crush2, presetKnobValues[BitCrusher][3]);
            displayValues[4] = presetKnobValues[BitCrusher][4];
            displayValues[5] = (presetKnobValues[BitCrusher][5] * 5.0f) + 1.0f;
            float volumeComp;
            
            if (displayValues[0] < 0.1f)
            {
                volumeComp = 1.0f;
            }
            else
            {
                volumeComp = (1.0f / (displayValues[3] + 1.0f));
            }
            sample = tanhf(tCrusher_tick(&crush, input[1] * displayValues[5])) * displayValues[4] * volumeComp;
            if (crusherStereo)
            {
                input[1] = tanhf(tCrusher_tick(&crush2, input[0] * displayValues[5])) * displayValues[4] * volumeComp;
            }
            else
            {
                input[1] = sample;
            }
            input[0] = sample;
        }
        
        void SFXBitcrusherFree(void)
        {
            tCrusher_free(&crush);
            tCrusher_free(&crush2);
        }
        
        
        //delay
        int delayShaper = 0;
        int capFeedback = 0;
        
        void SFXDelayAlloc()
        {
            leaf.clearOnAllocation = 1;
            tTapeDelay_initToPool(&delay, 2000, 30000, &mediumPool);
            tTapeDelay_initToPool(&delay2, 2000, 30000, &mediumPool);
            tSVF_init(&delayLP, SVFTypeLowpass, 16000.f, .7f);
            tSVF_init(&delayHP, SVFTypeHighpass, 20.f, .7f);
            
            tSVF_init(&delayLP2, SVFTypeLowpass, 16000.f, .7f);
            tSVF_init(&delayHP2, SVFTypeHighpass, 20.f, .7f);
            
            tHighpass_init(&delayShaperHp, 20.0f);
            tFeedbackLeveler_init(&feedbackControl, .99f, 0.01f, 0.125f, 0);
            delayShaper = 0;
            capFeedback = 1;
            freeze = 0;
            setLED_A(delayShaper);
            leaf.clearOnAllocation = 0;
        }
        
        void SFXDelayFrame()
        {
            if (buttonActionsSFX[ButtonA][ActionPress])
            {
                delayShaper = !delayShaper;
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(delayShaper);
            }
            if (buttonActionsSFX[ButtonB][ActionPress])
            {
                capFeedback = !capFeedback;
                buttonActionsSFX[ButtonB][ActionPress] = 0;
            }
            
            if (buttonActionsSFX[ButtonC][ActionPress])
            {
                freeze = !freeze;
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(freeze);
            }
            
            
        }
        
        float delayFB1;
        float delayFB2;
        
        void SFXDelayTick(float* input)
        {
            displayValues[0] = presetKnobValues[Delay][0] * 30000.0f;
            displayValues[1] = presetKnobValues[Delay][1] * 30000.0f;
            displayValues[2] = faster_mtof((presetKnobValues[Delay][2] * 128) + 10.0f);
            displayValues[3] = faster_mtof((presetKnobValues[Delay][3] * 128) + 10.0f);
            displayValues[4] = capFeedback ? LEAF_clip(0.0f, presetKnobValues[Delay][4] * 1.1f, 0.9f) : presetKnobValues[Delay][4] * 1.1f;
            displayValues[5] = presetKnobValues[Delay][5];
            
            tSVF_setFreq(&delayHP, displayValues[2]);
            tSVF_setFreq(&delayHP2, displayValues[2]);
            tSVF_setFreq(&delayLP, displayValues[3]);
            tSVF_setFreq(&delayLP2, displayValues[3]);
            
            //swap tanh for shaper and add cheap fixed highpass after both shapers
            
            float input1, input2;
            
            if (delayShaper == 0)
            {
                input1 = tFeedbackLeveler_tick(&feedbackControl, tanhf(input[1] + (delayFB1 * displayValues[4])));
                input2 = tFeedbackLeveler_tick(&feedbackControl, tanhf(input[1] + (delayFB2 * displayValues[4])));
            }
            else
            {
                input1 = tFeedbackLeveler_tick(&feedbackControl, tHighpass_tick(&delayShaperHp, LEAF_shaper(input[1] + (delayFB1 * displayValues[4] * 0.5f), 0.5f)));
                input2 = tFeedbackLeveler_tick(&feedbackControl, tHighpass_tick(&delayShaperHp, LEAF_shaper(input[1] + (delayFB2 * displayValues[4] * 0.5f), 0.5f)));
            }
            tTapeDelay_setDelay(&delay, displayValues[0]);
            tTapeDelay_setDelay(&delay2, displayValues[1]);
            
            if (!freeze)
            {
                delayFB1 = tTapeDelay_tick(&delay, input1);
                delayFB2 = tTapeDelay_tick(&delay2, input2);
            }
            
            else
            {
                delayFB1 = tTapeDelay_tick(&delay, delayFB1);
                delayFB2 = tTapeDelay_tick(&delay2, delayFB2);
            }
            
            delayFB1 = tSVF_tick(&delayLP, delayFB1);
            delayFB2 = tSVF_tick(&delayLP2, delayFB2);
            
            delayFB1 = tanhf(tSVF_tick(&delayHP, delayFB1));
            delayFB2 = tanhf(tSVF_tick(&delayHP2, delayFB2));
            
            input[0] = delayFB1 * displayValues[5];
            input[1] = delayFB2 * displayValues[5];
            
        }
        
        void SFXDelayFree(void)
        {
            tTapeDelay_free(&delay);
            tTapeDelay_free(&delay2);
            tSVF_free(&delayLP);
            tSVF_free(&delayHP);
            tSVF_free(&delayLP2);
            tSVF_free(&delayHP2);
            
            tHighpass_free(&delayShaperHp);
            tFeedbackLeveler_free(&feedbackControl);
        }
        
        
        
        //reverb
        int freeze = 0;
        
        tDattorroReverb reverb;
        tExpSmooth sizeSmoother;
        
        void SFXReverbAlloc()
        {
            leaf.clearOnAllocation = 1;
            tDattorroReverb_initToPool(&reverb, &mediumPool);
            tExpSmooth_init(&sizeSmoother, 0.5f, 0.001f);
            tDattorroReverb_setMix(&reverb, 1.0f);
            freeze = 0;
            capFeedback = 1;
            leaf.clearOnAllocation = 0;
        }
        
        void SFXReverbFrame()
        {
            if (buttonActionsSFX[ButtonB][ActionPress])
            {
                capFeedback = !capFeedback;
                buttonActionsSFX[ButtonB][ActionPress] = 0;
            }
            displayValues[1] = faster_mtof(presetKnobValues[Reverb][1]*129.0f);
            tDattorroReverb_setFeedbackFilter(&reverb, displayValues[1]);
            displayValues[2] =  faster_mtof(presetKnobValues[Reverb][2]*123.0f);
            tDattorroReverb_setHP(&reverb, displayValues[2]);
            displayValues[3] = faster_mtof(presetKnobValues[Reverb][3]*129.0f);
            tDattorroReverb_setInputFilter(&reverb, displayValues[3]);
        }
        
        void SFXReverbTick(float* input)
        {
            float stereo[2];
            float sample = 0.0f;
            if (buttonActionsSFX[ButtonC][ActionPress])
            {
                freeze = !freeze;
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                tDattorroReverb_setFreeze(&reverb, freeze);
                setLED_C(freeze);
            }
            
            //tDattorroReverb_setInputDelay(&reverb, smoothedADC[1] * 200.f);
            input[1] *= 4.0f;
            displayValues[0] = presetKnobValues[Reverb][0];
            tExpSmooth_setDest(&sizeSmoother, (displayValues[0] * 0.9f) + 0.1f);
            float tempSize = tExpSmooth_tick(&sizeSmoother);
            tDattorroReverb_setSize(&reverb, tempSize);
            displayValues[4] = capFeedback ? LEAF_clip(0.0f, presetKnobValues[Reverb][4], 0.5f) : presetKnobValues[Reverb][4];
            tDattorroReverb_setFeedbackGain(&reverb, displayValues[4]);
            tDattorroReverb_tickStereo(&reverb, input[1], stereo);
            sample = tanhf(stereo[0]) * 0.99f;
            input[0] = sample;
            input[1] = tanhf(stereo[1]) * 0.99f;
        }
        
        void SFXReverbFree(void)
        {
            tDattorroReverb_free(&reverb);
            tExpSmooth_free(&sizeSmoother);
        }
        
        
        //reverb2
        
        tNReverb reverb2;
        tSVF lowpass;
        tSVF highpass;
        tSVF bandpass;
        tSVF lowpass2;
        tSVF highpass2;
        tSVF bandpass2;
        
        void SFXReverb2Alloc()
        {
            leaf.clearOnAllocation = 1;
            tNReverb_initToPool(&reverb2, 1.0f, &mediumPool);
            tNReverb_setMix(&reverb2, 1.0f);
            tSVF_init(&lowpass, SVFTypeLowpass, 18000.0f, 0.75f);
            tSVF_init(&highpass, SVFTypeHighpass, 40.0f, 0.75f);
            tSVF_init(&bandpass, SVFTypeBandpass, 2000.0f, 1.0f);
            tSVF_init(&lowpass2, SVFTypeLowpass, 18000.0f, 0.75f);
            tSVF_init(&highpass2, SVFTypeHighpass, 40.0f, 0.75f);
            tSVF_init(&bandpass2, SVFTypeBandpass, 2000.0f, 1.0f);
            freeze = 0;
            leaf.clearOnAllocation = 0;
        }
        
        void SFXReverb2Frame()
        {
            
            
        }
        
        
        void SFXReverb2Tick(float* input)
        {
            float stereoOuts[2];
            float sample = 0.0f;
            displayValues[0] = presetKnobValues[Reverb2][0] * 4.0f;
            if (!freeze)
            {
                tNReverb_setT60(&reverb2, displayValues[0]);
                
            }
            else
            {
                tNReverb_setT60(&reverb2, 1000.0f);
                input[1] = 0.0f;
            }
            
            displayValues[1] = faster_mtof(presetKnobValues[Reverb2][1]*135.0f);
            tSVF_setFreq(&lowpass, displayValues[1]);
            tSVF_setFreq(&lowpass2, displayValues[1]);
            displayValues[2] = faster_mtof(presetKnobValues[Reverb2][2]*128.0f);
            tSVF_setFreq(&highpass, displayValues[2]);
            tSVF_setFreq(&highpass2, displayValues[2]);
            displayValues[3] = faster_mtof(presetKnobValues[Reverb2][3]*128.0f);
            tSVF_setFreq(&bandpass, displayValues[3]);
            tSVF_setFreq(&bandpass2, displayValues[3]);
            
            displayValues[4] = (presetKnobValues[Reverb2][4] * 4.0f) - 2.0f;
            
            if (buttonActionsSFX[ButtonC][ActionPress])
            {
                freeze = !freeze;
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(freeze);
            }
            
            if (buttonActionsSFX[ButtonA][ActionPress])
            {
                freeze = !freeze;
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_C(freeze);
            }
            
            tNReverb_tickStereo(&reverb2, input[1], stereoOuts);
            float leftOut = tSVF_tick(&lowpass, stereoOuts[0]);
            leftOut = tSVF_tick(&highpass, leftOut);
            leftOut += tSVF_tick(&bandpass, leftOut) * displayValues[4];
            
            float rightOutTemp = tSVF_tick(&lowpass2, stereoOuts[1]);
            rightOutTemp = tSVF_tick(&highpass2, rightOutTemp);
            rightOutTemp += tSVF_tick(&bandpass, rightOutTemp) * displayValues[4];
            sample = tanhf(leftOut);
            input[0] = sample;
            input[1] = tanhf(rightOutTemp);
            
        }
        
        void SFXReverb2Free(void)
        {
            tNReverb_free(&reverb2);
            tSVF_free(&lowpass);
            tSVF_free(&highpass);
            tSVF_free(&bandpass);
            tSVF_free(&lowpass2);
            tSVF_free(&highpass2);
            tSVF_free(&bandpass2);
        }
        
        
        int ignoreFreqKnobs = 0;
        int levMode = 0;
        int independentStrings = 0;
        tExpSmooth stringGains[NUM_STRINGS];
        //Living String
        void SFXLivingStringAlloc()
        {
            levMode = 0;
            tSimplePoly_setNumVoices(&poly, NUM_STRINGS);
            for (int i = 0; i < NUM_STRINGS; i++)
            {
                myDetune[i] = (leaf.random() * 0.3f) - 0.15f;
                //tComplexLivingString_init(&theString[i],  myFreq, 0.4f, 0.0f, 16000.0f, .999f, .5f, .5f, 0.1f, 0);
                tComplexLivingString_initToPool(&theString[i], 440.f, 0.8f, 0.3f, 0.f, 9000.f, 1.0f, 0.3f, 0.01f, 0.125f, levMode, &mediumPool);
                tExpSmooth_init(&stringGains[i], 0.0f, 0.002f);
            }
            ignoreFreqKnobs = 0;
            setLED_A(ignoreFreqKnobs);
            setLED_B(independentStrings);
            setLED_C(levMode);
            
        }
        
        void SFXLivingStringFrame()
        {
            if (buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                ignoreFreqKnobs = !ignoreFreqKnobs;
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(ignoreFreqKnobs);
            }
            if (buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                levMode = !levMode;
                for (int i = 0; i < NUM_STRINGS; i++)
                {
                    tComplexLivingString_setLevMode(&theString[i], levMode);
                }
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(levMode);
            }
            if (buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                independentStrings = !independentStrings;
                buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(independentStrings);
            }
            displayValues[0] = LEAF_midiToFrequency((presetKnobValues[LivingString][0] * 90.0f)); //freq
            displayValues[1] = presetKnobValues[LivingString][1]; //detune
            displayValues[2] = presetKnobValues[LivingString][2]; //decay
            displayValues[3] = mtof((presetKnobValues[LivingString][3] * 130.0f)+12.0f); //lowpass
            displayValues[4] = (presetKnobValues[LivingString][4] * 0.48f) + 0.5f;//pickPos
            displayValues[5] = (presetKnobValues[LivingString][5] * 0.48f) + 0.02f;//prepPos
            displayValues[6] = ((tanhf((presetKnobValues[LivingString][6] * 8.0f) - 4.0f)) * 0.5f) + 0.5f;//prep Index
            displayValues[7] = presetKnobValues[LivingString][7];// let ring
            
            if (!independentStrings)
            {
                if (!ignoreFreqKnobs)
                {
                    
                    
                    
                    for (int i = 0; i < NUM_STRINGS; i++)
                    {
                        float freqVal = displayValues[0] * (i+1);
                        tComplexLivingString_setFreq(&theString[i], (1.0f + (myDetune[i] * displayValues[1])) * freqVal);
                        tComplexLivingString_setDecay(&theString[i], (displayValues[2] * 0.015f) + 0.995f);
                        tComplexLivingString_setDampFreq(&theString[i], displayValues[3]);
                        tComplexLivingString_setPickPos(&theString[i], displayValues[4]);
                        tComplexLivingString_setPrepPos(&theString[i], displayValues[5]);
                        tComplexLivingString_setPrepIndex(&theString[i], displayValues[6]);
                        tExpSmooth_setDest(&stringGains[i], 1.0f);
                    }
                }
                else
                {
                    for (int i = 0; i < NUM_STRINGS; i++)
                    {
                        
                        calculateFreq(i);
                        float freqVal = freq[i];
                        tComplexLivingString_setFreq(&theString[i], (1.0f + (myDetune[i] * displayValues[1])) * freqVal);
                        tComplexLivingString_setDecay(&theString[i], (displayValues[2] * 0.015f) + 0.995f);
                        tComplexLivingString_setDampFreq(&theString[i], displayValues[3]);
                        tComplexLivingString_setPickPos(&theString[i], displayValues[4]);
                        tComplexLivingString_setPrepPos(&theString[i], displayValues[5]);
                        tComplexLivingString_setPrepIndex(&theString[i], displayValues[6]);
                        if (tSimplePoly_isOn(&poly, i))
                        {
                            tExpSmooth_setDest(&stringGains[i], 1.0f);
                        }
                        else
                        {
                            tExpSmooth_setDest(&stringGains[i], displayValues[7]);
                        }
                    }
                }
            }
            else
            {
                displayValues[10] = LEAF_midiToFrequency((presetKnobValues[LivingString][10] * 90.0f)); //freq
                displayValues[11] = LEAF_midiToFrequency((presetKnobValues[LivingString][11] * 90.0f)); //freq
                displayValues[12] = LEAF_midiToFrequency((presetKnobValues[LivingString][12] * 90.0f)); //freq
                displayValues[13] = LEAF_midiToFrequency((presetKnobValues[LivingString][13] * 90.0f)); //freq
                displayValues[14] = LEAF_midiToFrequency((presetKnobValues[LivingString][14] * 90.0f)); //freq
                
                for (int i = 0; i < NUM_STRINGS; i++)
                {
                    float freqVal = i == 0 ? displayValues[0] : displayValues[9+i];
                    tComplexLivingString_setFreq(&theString[i], (1.0f + (myDetune[i] * displayValues[1])) * freqVal);
                    tComplexLivingString_setDecay(&theString[i], (displayValues[2] * 0.015f) + 0.995f);
                    tComplexLivingString_setDampFreq(&theString[i], displayValues[3]);
                    tComplexLivingString_setPickPos(&theString[i], displayValues[4]);
                    tComplexLivingString_setPrepPos(&theString[i], displayValues[5]);
                    tComplexLivingString_setPrepIndex(&theString[i], displayValues[6]);
                    tExpSmooth_setDest(&stringGains[i], 1.0f);
                }
            }
        }
        
        
        void SFXLivingStringTick(float* input)
        {
            float sample = 0.0f;
            for (int i = 0; i < NUM_STRINGS; i++)
            {
                float tick = tComplexLivingString_tick(&theString[i], input[1]);
                sample += tick * tExpSmooth_tick(&stringGains[i]);
                
            }
            sample *= 0.1625f;
            input[0] = sample;
            input[1] = sample;
            
            
        }
        
        void SFXLivingStringFree(void)
        {
            for (int i = 0; i < NUM_STRINGS; i++)
            {
                tComplexLivingString_free(&theString[i]);
                tExpSmooth_free(&stringGains[i]);
            }
        }
        
        int voicePluck = 0;
        tSlide stringOutEnvs[NUM_STRINGS];
        tSlide stringInEnvs[NUM_STRINGS];
        tADSR4 pluckEnvs[NUM_STRINGS];
        int levModeStr = 0;
        tNoise stringPluckNoise;
        
        tVZFilter pluckFilt;
        float samplesPerMs = 1;
        
        //Living String Synth
        void SFXLivingStringSynthAlloc()
        {
            levMode = 1;
            leaf.clearOnAllocation = 0;
            tSimplePoly_setNumVoices(&poly, NUM_STRINGS);
            for (int i = 0; i < NUM_STRINGS; i++)
            {
                tComplexLivingString_initToPool(&theString[i], 440.f, 0.2f, 0.3f, 0.f, 9000.f, 1.0f, 0.0f, 0.01f, 0.125f, levModeStr, &mediumPool);
                tSlide_init(&stringOutEnvs[i], 10.0f, 1000.0f);
                tSlide_init(&stringInEnvs[i], 12.0f, 1000.0f);
                tADSR4_init(&pluckEnvs[i], 4.0f, 70.0f, 0.0f, 5.0f, decayExpBuffer, DECAY_EXP_BUFFER_SIZE);
                
            }
            tVZFilter_init(&pluckFilt, BandpassPeak, 2000.0f, 4.0f);
            tNoise_init(&stringPluckNoise, WhiteNoise);
            setLED_A(numVoices == 1);
            setLED_B(voicePluck);
            setLED_C(levModeStr);
            samplesPerMs = leaf.sampleRate / 1000.0f;
        }
        
        
        
        void SFXLivingStringSynthFrame()
        {
            if (buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                numVoices = (numVoices > 1) ? 1 : NUM_STRINGS;
                tSimplePoly_setNumVoices(&poly, numVoices);
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(numVoices == 1);
            }
            if (buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                voicePluck = !voicePluck;
                
                buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_B(voicePluck);
            }
            
            if (buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                levModeStr = !levModeStr;
                for (int i = 0; i < NUM_STRINGS; i++)
                {
                    tComplexLivingString_setLevMode(&theString[i], levModeStr);
                }
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(levModeStr);
            }
            
            
            displayValues[0] = presetKnobValues[LivingStringSynth][0] * 10.0f; //pluck volume
            displayValues[1] = presetKnobValues[LivingStringSynth][1]; //lowpass
            displayValues[2] = presetKnobValues[LivingStringSynth][2]; //decay
            displayValues[3] = faster_mtof((presetKnobValues[LivingStringSynth][3] * 119.0f)+20.0f); //lowpass
            displayValues[4] = (presetKnobValues[LivingStringSynth][4] * 0.44f) + 0.52f;//pick Pos
            displayValues[5] = (presetKnobValues[LivingStringSynth][5] * 0.44f) + 0.04f;//prep Pos
            displayValues[6] = ((LEAF_tanh((presetKnobValues[LivingStringSynth][6] * 8.5f) - 4.25f)) * 0.5f) + 0.5f;//prep Index
            displayValues[7] = presetKnobValues[LivingStringSynth][7];//let Ring
            displayValues[8] = presetKnobValues[LivingStringSynth][8];//feedback level
            displayValues[9] = expBuffer[(int)(presetKnobValues[LivingStringSynth][9] * expBufferSizeMinusOne)] * 8192.0f;//release time
            for (int i = 0; i < NUM_STRINGS; i++)
            {
                //tComplexLivingString_setFreq(&theString[i], (i + (1.0f+(myDetune[i] * knobParams[1]))) * knobParams[0]);
                tComplexLivingString_setDecay(&theString[i], ((displayValues[2]  * 0.02f) + 0.98f));
                tComplexLivingString_setDampFreq(&theString[i], displayValues[3]);
                tComplexLivingString_setPickPos(&theString[i], displayValues[4]);
                tComplexLivingString_setPrepPos(&theString[i], displayValues[5]);
                tComplexLivingString_setPrepIndex(&theString[i], displayValues[6]);
                tSlide_setDownSlide(&stringOutEnvs[i], displayValues[9] * samplesPerMs);
                //tADSR4_setDecay(&pluckEnvs[i], displayValues[9]);
                
            }
            tVZFilter_setFreq(&pluckFilt, faster_mtof((displayValues[1] * 100.0f)+20.0f));
            for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
            {
                //tRamp_setDest(&polyRamp[i], (tPoly_getVelocity(&poly, i) > 0));
                calculateFreq(i);
                tComplexLivingString_setFreq(&theString[i], freq[i]);
                //tComplexLivingString_setDampFreq(&theString[i], LEAF_clip(40.0f, freq[i] + displayValues[3], 23000.0f));
                float voiceOn = (tSimplePoly_getVelocity(&poly, i) > 0);
                if (levModeStr)
                {
                    tComplexLivingString_setTargetLev(&theString[i],voiceOn * displayValues[8]);
                }
                else
                {
                    tComplexLivingString_setTargetLev(&theString[i],1.0f);
                }
                if (voiceOn)
                {
                    tSlide_setDest(&stringOutEnvs[i], 1.0f);
                    tSlide_setDest(&stringInEnvs[i], 1.0f);
                }
                else
                {
                    tSlide_setDest(&stringOutEnvs[i], displayValues[7]);
                    tSlide_setDest(&stringInEnvs[i], 0.0f);
                }
                
            }
        }
        
        
        void SFXLivingStringSynthTick(float* input)
        {
            float sample = 0.0f;
            
            float inputSample = 0.0f;
            //float pluck = (displayValues[1] * tNoise_tick(&stringPluckNoise)) + ((1.0f - displayValues[1]) * tNoise_tick(&stringPluckNoiseDark));
            float pluck = displayValues[0] * tNoise_tick(&stringPluckNoise);
            pluck = tVZFilter_tick(&pluckFilt, pluck);
            
            for (int i = 0; i < NUM_STRINGS; i++)
            {
                
                //float pluck = tNoise_tick(&stringPluckNoise);
                inputSample = (input[1] * voicePluck) + (pluck * tADSR4_tick(&pluckEnvs[i]));
                //inputSample = (input[1] * voicePluck) + (tVZFilter_tick(&pluckFilt, (tNoise_tick(&stringPluckNoise))) * tADSR4_tick(&pluckEnvs[i]));
                sample += tComplexLivingString_tick(&theString[i], (inputSample * tSlide_tickNoInput(&stringOutEnvs[i]))) * tSlide_tickNoInput(&stringOutEnvs[i]);
            }
            sample *= 0.1625f;
            sample = LEAF_tanh(sample) * 0.98f;
            input[0] = sample;
            input[1] = sample;
        }
        
        void SFXLivingStringSynthFree(void)
        {
            for (int i = 0; i < NUM_STRINGS; i++)
            {
                tComplexLivingString_free(&theString[i]);
                tSlide_free(&stringInEnvs[i]);
                tSlide_free(&stringOutEnvs[i]);
                tADSR4_free(&pluckEnvs[i]);
            }
            tVZFilter_free(&pluckFilt);
            tNoise_free(&stringPluckNoise);
        }
        
        
        // CLASSIC SUBTRACTIVE SYNTH
        
        
        tEfficientSVF synthLP[NUM_VOC_VOICES];
        uint16_t filtFreqs[NUM_VOC_VOICES];
        tADSR4 polyEnvs[NUM_VOC_VOICES];
        tADSR4 polyFiltEnvs[NUM_VOC_VOICES];
        tCycle pwmLFO1;
        tCycle pwmLFO2;
        void SFXClassicSynthAlloc()
        {
            leaf.clearOnAllocation = 1;
            tSimplePoly_setNumVoices(&poly, numVoices);
            
            
            float* knobs = presetKnobValues[ClassicSynth];
            
            displayValues[0] = knobs[0]; //synth volume
            
            displayValues[1] = knobs[1] * 4096.0f; //lowpass cutoff
            
            displayValues[2] = knobs[2]; //keyfollow filter cutoff
            
            displayValues[3] = knobs[3]; //detune
            
            displayValues[4] = (knobs[4] * 2.0f) + 0.4f; //filter Q
            
            displayValues[5] = expBuffer[(int)(knobs[5] * expBufferSizeMinusOne)] * 8192.0f; //att
            
            displayValues[6] = expBuffer[(int)(knobs[6] * expBufferSizeMinusOne)] * 8192.0f; //dec
            
            displayValues[7] = knobs[7]; //sus
            
            displayValues[8] = expBuffer[(int)(knobs[8] * expBufferSizeMinusOne)] * 8192.0f; //rel
            
            displayValues[9] = knobs[9]; //leak
            
            displayValues[10] = expBuffer[(int)(knobs[10] * expBufferSizeMinusOne)] * 8192.0f; //att
            
            displayValues[11] = expBuffer[(int)(knobs[11] * expBufferSizeMinusOne)] * 8192.0f; //dec
            
            displayValues[12] = knobs[12]; //sus
            
            displayValues[13] = expBuffer[(int)(knobs[13] * expBufferSizeMinusOne)] * 8192.0f; //rel
            
            displayValues[14] = knobs[14]; //leak
            
            displayValues[15] = knobs[15] * 4095.0f;  // filter envelope amount
            
            displayValues[16] = knobs[16];  // fade between sawtooth and glottal pulse
            
            
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
                {
                    tSawtooth_init(&osc[(i * NUM_OSC_PER_VOICE) + j]);
                    synthDetune[i][j] = ((leaf.random() * 0.0264f) - 0.0132f);
                    tRosenbergGlottalPulse_init(&glottal[(i * NUM_OSC_PER_VOICE) + j]);
                    tRosenbergGlottalPulse_setOpenLength(&glottal[(i * NUM_OSC_PER_VOICE) + j], 0.3f);
                    tRosenbergGlottalPulse_setPulseLength(&glottal[(i * NUM_OSC_PER_VOICE) + j], 0.4f);
                }
                
                tEfficientSVF_init(&synthLP[i], SVFTypeLowpass, 2000, displayValues[4]);
                tADSR4_init(&polyEnvs[i], displayValues[5], displayValues[6], displayValues[7], displayValues[8], decayExpBuffer, DECAY_EXP_BUFFER_SIZE);
                tADSR4_setLeakFactor(&polyEnvs[i],((1.0f - displayValues[9]) * 0.00005f) + 0.99995f);
                tADSR4_init(&polyFiltEnvs[i], displayValues[10], displayValues[11], displayValues[12], displayValues[13], decayExpBuffer, DECAY_EXP_BUFFER_SIZE);
                tADSR4_setLeakFactor(&polyFiltEnvs[i], ((1.0f - displayValues[14]) * 0.00005f) + 0.99995f);
                
            }
            tCycle_init(&pwmLFO1);
            tCycle_init(&pwmLFO2);
            tCycle_setFreq(&pwmLFO1, 63.0f);
            tCycle_setFreq(&pwmLFO2, 72.11f);
            
            setLED_A(numVoices == 1);
            leaf.clearOnAllocation = 0;
            //            cycleCountVals[0][2] = 2;
        }
        
        void SFXClassicSynthFrame()
        {
            if (buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                numVoices = (numVoices > 1) ? 1 : NUM_VOC_VOICES;
                tSimplePoly_setNumVoices(&poly, numVoices);
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(numVoices == 1);
            }
            if (buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                //                cycleCountVals[0][1] = 0;
                buttonActionsSFX[ButtonB][ActionPress] = 0;
                setLED_Edit(0);
            }
            
            float* knobs = presetKnobValues[ClassicSynth];
            
            if (writeKnobFlag != -1)
            {
                switch(writeKnobFlag + (knobPage * KNOB_PAGE_SIZE))
                {
                    case 0:
                        displayValues[0] = knobs[0]; //synth volume
                        break;
                    case 1:
                        displayValues[1] = knobs[1] * 4096.0f; //lowpass cutoff
                        break;
                    case 2:
                        displayValues[2] = knobs[2]; //keyfollow filter cutoff
                        break;
                    case 3:
                        displayValues[3] = knobs[3]; //detune
                        break;
                    case 4:
                        displayValues[4] = (knobs[4] * 2.0f) + 0.4f; //filter Q
                        for (int i = 0; i < numVoices; i++)
                        {
                            tEfficientSVF_setQ(&synthLP[i],displayValues[4]);
                        }
                        break;
                    case 5:
                        displayValues[5] = expBuffer[(int)(knobs[5] * expBufferSizeMinusOne)] * 8192.0f; //att
                        for (int i = 0; i < numVoices; i++)
                        {
                            tADSR4_setAttack(&polyEnvs[i], displayValues[5]);
                        }
                        break;
                    case 6:
                        displayValues[6] = expBuffer[(int)(knobs[6] * expBufferSizeMinusOne)] * 8192.0f; //dec
                        for (int i = 0; i < numVoices; i++)
                        {
                            tADSR4_setDecay(&polyEnvs[i], displayValues[6]);
                        }
                        break;
                    case 7:
                        displayValues[7] = knobs[7]; //sus
                        for (int i = 0; i < numVoices; i++)
                        {
                            tADSR4_setSustain(&polyEnvs[i], displayValues[7]);
                        }
                        break;
                    case 8:
                        displayValues[8] = expBuffer[(int)(knobs[8] * expBufferSizeMinusOne)] * 8192.0f; //rel
                        for (int i = 0; i < numVoices; i++)
                        {
                            tADSR4_setRelease(&polyEnvs[i], displayValues[8]);
                        }
                        break;
                    case 9:
                        displayValues[9] = knobs[9]; //leak
                        for (int i = 0; i < numVoices; i++)
                        {
                            tADSR4_setLeakFactor(&polyEnvs[i], ((1.0f - displayValues[9]) * 0.00005f) + 0.99995f);
                        }
                        break;
                    case 10:
                        displayValues[10] = expBuffer[(int)(knobs[10] * expBufferSizeMinusOne)] * 8192.0f; //att
                        for (int i = 0; i < numVoices; i++)
                        {
                            tADSR4_setAttack(&polyFiltEnvs[i], displayValues[10]);
                        }
                        break;
                    case 11:
                        displayValues[11] = expBuffer[(int)(knobs[11] * expBufferSizeMinusOne)] * 8192.0f; //dec
                        for (int i = 0; i < numVoices; i++)
                        {
                            tADSR4_setDecay(&polyFiltEnvs[i], displayValues[11]);
                        }
                        break;
                    case 12:
                        displayValues[12] = knobs[12]; //sus
                        for (int i = 0; i < numVoices; i++)
                        {
                            tADSR4_setSustain(&polyFiltEnvs[i], displayValues[12]);
                        }
                        break;
                    case 13:
                        displayValues[13] = expBuffer[(int)(knobs[13] * expBufferSizeMinusOne)] * 8192.0f; //rel
                        for (int i = 0; i < numVoices; i++)
                        {
                            tADSR4_setRelease(&polyFiltEnvs[i], displayValues[13]);
                        }
                        break;
                    case 14:
                        displayValues[14] = knobs[14]; //leak
                        for (int i = 0; i < numVoices; i++)
                        {
                            tADSR4_setLeakFactor(&polyFiltEnvs[i], ((1.0f - displayValues[14]) * 0.00005f) + 0.99995f);
                        }
                        break;
                    case 15:
                        displayValues[15] = knobs[15] * 4095.0f;  // filter envelope amount
                        break;
                    case 16:
                        displayValues[16] = knobs[16];  // fade between sawtooth and glottal pulse
                        break;
                }
            }
            
            
            float tempLFO1 = (tCycle_tick(&pwmLFO1) * 0.25f) + 0.5f; // pulse length
            float tempLFO2 = ((tCycle_tick(&pwmLFO2) * 0.25f) + 0.5f) * tempLFO1; // open length
            
            for (int i = 0; i < numVoices; i++)
            {
                float myMidiNote = tSimplePoly_getPitch(&poly, i);
                
                calculateFreq(i);
                
                
                for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
                {
                    float tempFreq = freq[i] * (1.0f + (synthDetune[i][j] * displayValues[3]));
                    tSawtooth_setFreq(&osc[(i * NUM_OSC_PER_VOICE) + j], tempFreq);
                    tRosenbergGlottalPulse_setFreq(&glottal[(i * NUM_OSC_PER_VOICE) + j], tempFreq);
                    tRosenbergGlottalPulse_setPulseLength(&glottal[(i * NUM_OSC_PER_VOICE) + j], tempLFO1);
                    tRosenbergGlottalPulse_setOpenLength(&glottal[(i * NUM_OSC_PER_VOICE) + j], tempLFO2);
                }
                
                
                float keyFollowFilt = myMidiNote * displayValues[2] * 64.0f;
                float tempFreq2 = displayValues[1] +  keyFollowFilt;
                tempFreq2 = LEAF_clip(0.0f, tempFreq2, 4095.0f);
                filtFreqs[i] = (uint16_t) tempFreq2;
                
                if (numVoices > 1)
                {
                    if (poly->voices[i][0] == -2)
                    {
                        if (polyEnvs[i]->whichStage == env_idle)
                        {
                            tSimplePoly_deactivateVoice(&poly, i);
                        }
                    }
                }
            }
        }
        
        
        void SFXClassicSynthTick(float* input)
        {
            float sample = 0.0f;
            
            for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
            {
                float tempSample = 0.0f;
                float env = tADSR4_tick(&polyEnvs[i]);
                
                for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
                {
                    tempSample += tSawtooth_tick(&osc[(i * NUM_OSC_PER_VOICE) + j]) * env * (1.0f-displayValues[16]);
                    tempSample += tRosenbergGlottalPulse_tick(&glottal[(i * NUM_OSC_PER_VOICE) + j]) * env * (displayValues[16]);
                }
                tEfficientSVF_setFreq(&synthLP[i], LEAF_clip(0.0f, (filtFreqs[i] + (displayValues[15] * tADSR4_tick(&polyFiltEnvs[i]))), 4095.0f));
                sample += tEfficientSVF_tick(&synthLP[i], tempSample);
            }
            sample *= INV_NUM_OSC_PER_VOICE * displayValues[0];
            
            
            sample = tanhf(sample);
            input[0] = sample;
            input[1] = sample;
        }
        
        
        
        void SFXClassicSynthFree(void)
        {
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
                {
                    tSawtooth_free(&osc[(i * NUM_OSC_PER_VOICE) + j]);
                    tRosenbergGlottalPulse_free(&glottal[(i * NUM_OSC_PER_VOICE) + j]);
                }
                tEfficientSVF_free(&synthLP[i]);
                tADSR4_free(&polyEnvs[i]);
                tADSR4_free(&polyFiltEnvs[i]);
            }
            
            tCycle_free(&pwmLFO1);
            tCycle_free(&pwmLFO2);
        }
        
        
        
        ///FM RHODES ELECTRIC PIANO SYNTH
        
        
        tCycle FM_sines[NUM_VOC_VOICES][6];
        float FM_freqRatios[5][6] = {{1.0f, 1.00001f, 1.0f, 3.0f, 1.0f, 1.0f}, {2.0f, 2.0001f, .99999f, 3.0f, 5.0f, 8.0f},  {1.0f, 2.0f, 1.0f, 7.0f, 3.0f, 4.0f}, {1.0f, 2.0f, 1.0f, 7.0f, 3.0f, 4.0f}, {1.0f, 2.0f, 1.0f, 7.0f, 3.0f, 4.0f}};
        float FM_indices[5][6] = {{800.0f, 0.0f, 120.0f, 32.0f, 3.0f, 1.0f}, {100.0f, 100.0f, 300.0f, 300.0f, 10.0f, 5.0f}, {500.0f, 50.0f, 500.0f, 10.0f,0.0f, 0.0f}, {50.0f, 128.0f, 1016.0f, 528.0f, 4.0f, 0.0f},{50.0f, 128.0f, 1016.0f, 528.0f, 4.0f, 0.0f}};
        tADSR4 FM_envs[NUM_VOC_VOICES][6];
        float feedback_output = 0.0f;
        
        float panValues[NUM_VOC_VOICES];
        tCycle tremolo;
        int tremoloStereo = 0;
        
        int Rsound = 0;
        
        const char* soundNames[5];
        tExpSmooth susSmoothers[6];
        float prevKnobValues[25];
        float overtoneSnap = 1.0f;
        float randomDecays[6] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        float randomSustains[6] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        float sustainsFinal[6];
        
        //FM Rhodes
        void SFXRhodesAlloc()
        {
            leaf.clearOnAllocation = 1;
            soundNames[0] = "DARK  ";
            soundNames[1] = "LIGHT ";
            soundNames[2] = "BASS  ";
            soundNames[3] = "PAD   ";
            soundNames[4] = "CUSTOM";
            
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                for (int j = 0; j < 6; j++)
                {
                    tCycle_init(&FM_sines[i][j]);
                    tADSR4_init(&FM_envs[i][j], 10, 1000, 0.5f, 100.0f, decayExpBuffer, DECAY_EXP_BUFFER_SIZE);
                    tADSR4_setLeakFactor(&FM_envs[i][j], 0.99998f);
                }
            }
            for (int i = 0; i < 6; i++)
            {
                tExpSmooth_init(&susSmoothers[i], 1.0f, 0.01f);
            }
            tCycle_init(&tremolo);
            tCycle_setFreq(&tremolo, 3.0f);
            tSimplePoly_setNumVoices(&poly, NUM_VOC_VOICES);
            
            setLED_A(numVoices == 1);
            setLED_C(tremoloStereo == 1);
            OLEDclearLine(SecondLine);
            OLEDwriteString(soundNames[Rsound], 6, 0, SecondLine);
            leaf.clearOnAllocation = 0;
            
        }
        
        
        
        void SFXRhodesFrame()
        {
            if (buttonActionsSFX[ButtonA][ActionPress] == 1)
            {
                numVoices = (numVoices > 1) ? 1 : NUM_VOC_VOICES;
                tSimplePoly_setNumVoices(&poly, numVoices);
                buttonActionsSFX[ButtonA][ActionPress] = 0;
                setLED_A(numVoices == 1);
            }
            if (buttonActionsSFX[ButtonB][ActionPress] == 1)
            {
                buttonActionsSFX[ButtonB][ActionPress] = 0;
                Rsound = (Rsound + 1 ) % 5; // switch to another rhodes preset sound
            }
            if (buttonActionsSFX[ButtonC][ActionPress] == 1)
            {
                tremoloStereo = !tremoloStereo;
                buttonActionsSFX[ButtonC][ActionPress] = 0;
                setLED_C(tremoloStereo == 1);
                
            }
            
            displayValues[0] = presetKnobValues[Rhodes][0] * 4.0f; // brightness
            displayValues[1] = presetKnobValues[Rhodes][1]; // tremelo amount
            displayValues[2] = presetKnobValues[Rhodes][2] * 10.0f; //tremelo rate
            displayValues[3] = presetKnobValues[Rhodes][3] * 1.3f; //drive
            displayValues[4] = presetKnobValues[Rhodes][4]; //pan spread
            displayValues[5] = expBuffer[(int)(presetKnobValues[Rhodes][5] * expBufferSizeMinusOne)] * 8192.0f;
            displayValues[6] = expBuffer[(int)(presetKnobValues[Rhodes][6] * expBufferSizeMinusOne)] * 8192.0f;
            displayValues[7] = presetKnobValues[Rhodes][7];
            displayValues[8] = expBuffer[(int)(presetKnobValues[Rhodes][8] * expBufferSizeMinusOne)] * 8192.0f;
            displayValues[9] = presetKnobValues[Rhodes][9];
            
            FM_indices[4][0] = displayValues[10] = presetKnobValues[Rhodes][10] * 1000.0f;
            FM_indices[4][1] = displayValues[11] = presetKnobValues[Rhodes][11] * 1000.0f;
            FM_indices[4][2] = displayValues[12] = presetKnobValues[Rhodes][12] * 1000.0f;
            FM_indices[4][3] = displayValues[13] = presetKnobValues[Rhodes][13] * 1000.0f;
            FM_indices[4][4] = displayValues[14] = presetKnobValues[Rhodes][14] * 1000.0f;
            FM_indices[4][5] = displayValues[21] = LEAF_clip(0.0f, ((presetKnobValues[Rhodes][21] * 1000.0f) - 10.0f), 1000.0f); // feedback
            //pitch ratios for custom rhodes preset
            for (int k = 15; k < 21; k++)
            {
                if (presetKnobValues[Rhodes][k] != prevKnobValues[k])
                {
                    float rawRate = (presetKnobValues[Rhodes][k] - 0.5f) * 14.0f;
                    float snapRate = roundf(rawRate);
                    float rate = (snapRate * overtoneSnap) + (rawRate * (1.0f - overtoneSnap));
                    if (rate < 0.0f) rate = 1.0f / fabsf(rate-1.0f);
                    else rate += 1.0f;
                    displayValues[k] = rate;
                    FM_freqRatios[4][k-15] = rate;
                }
                prevKnobValues[k] = presetKnobValues[Rhodes][k];
            }
            //overtone snap
            if (presetKnobValues[Rhodes][22] != prevKnobValues[22])
            {
                overtoneSnap = displayValues[22] = presetKnobValues[Rhodes][22];
                for (int k = 15; k < 21; k++)
                {
                    
                    float rawRate = (presetKnobValues[Rhodes][k] - 0.5f) * 14.0f;
                    float snapRate = roundf(rawRate);
                    float rate = (snapRate * overtoneSnap) + (rawRate * (1.0f - overtoneSnap));
                    if (rate < 0.0f) rate = 1.0f / fabsf(rate-1.0f);
                    else rate += 1.0f;
                    displayValues[k] = rate;
                    FM_freqRatios[4][k-15] = rate;
                }
            }
            prevKnobValues[22] = presetKnobValues[Rhodes][22];
            // random decays
            displayValues[23] = presetKnobValues[Rhodes][23];
            if (prevDisplayValues[23] != displayValues[23])
            {
                
                for (int i = 0; i < 6; i++)
                {
                    float randomNumberDraw = (leaf.random() * 2.0f) + 0.08f;
                    randomDecays[i] = (1.0f - displayValues[23]) + (randomNumberDraw * displayValues[23]);
                }
                
                for (int i = 0; i < NUM_VOC_VOICES; i++)
                {
                    for (int j = 0; j < 6; j++)
                    {
                        tADSR4_setDecay(&FM_envs[i][j],(LEAF_clip(10.0f, displayValues[6] * randomDecays[j], 20000.0f))); //FM_decays[Rsound][j] * displayValues[6]);
                    }
                }
            }
            prevKnobValues[23] = presetKnobValues[Rhodes][23];
            
            // random sustains
            displayValues[24] = presetKnobValues[Rhodes][24];
            if (prevDisplayValues[24] != displayValues[24])
            {
                
                for (int i = 0; i < 6; i++)
                {
                    float randomNumberDraw = leaf.random() * 2.0f;
                    randomSustains[i] = (1.0f - displayValues[24]) + (randomNumberDraw * displayValues[24]);
                    tExpSmooth_setDest(&susSmoothers[i], displayValues[7] * randomSustains[i]);
                }
            }
            prevDisplayValues[24] = displayValues[24];
            
            for (int k = 5; k < 10; k++)
            {
                if (prevDisplayValues[k] != displayValues[k])
                {
                    switch(k)
                    {
                            
                        case 5:
                            for (int i = 0; i < NUM_VOC_VOICES; i++)
                            {
                                for (int j = 0; j < 6; j++)
                                {
                                    //tADSR_setAttack(&FM_envs[i][j], FM_attacks[Rsound][j] * displayValues[5]);
                                    //                                    cycleCountVals[1][2] = 0;
                                    //                                    uint64_t tempCount1 = DWT->CYCCNT;
                                    tADSR4_setAttack(&FM_envs[i][j], displayValues[5] );
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
                                    tADSR4_setDecay(&FM_envs[i][j],(LEAF_clip(7.0f, displayValues[6] * randomDecays[j], 20000.0f)));
                                }
                            }
                            break;
                        case 7:
                            for (int i = 0; i < 6; i++)
                            {
                                tExpSmooth_setDest(&susSmoothers[i], displayValues[7] * randomSustains[i]);
                            }
                            break;
                        case 8:
                            for (int i = 0; i < NUM_VOC_VOICES; i++)
                            {
                                for (int j = 0; j < 6; j++)
                                {
                                    tADSR4_setRelease(&FM_envs[i][j], displayValues[8]);
                                }
                            }
                            break;
                        case 9:
                            for (int i = 0; i < NUM_VOC_VOICES; i++)
                            {
                                for (int j = 0; j < 6; j++)
                                {
                                    tADSR4_setLeakFactor(&FM_envs[i][j], ((1.0f - displayValues[9])  * 0.00004f) + 0.99996f);
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
                prevDisplayValues[k] = displayValues[k];
            }
            
            for (int i = 0; i < numVoices; i++)
            {
                calculateFreq(i);
                if (numVoices > 1)
                {
                    if (poly->voices[i][0] == -2)
                    {
                        if ((FM_envs[i][0]->whichStage == env_idle) && (FM_envs[i][2]->whichStage == env_idle))
                        {
                            tSimplePoly_deactivateVoice(&poly, i);
                        }
                    }
                }
            }
            
            tCycle_setFreq(&tremolo, displayValues[2]);
        }
        
        void SFXRhodesTick(float* input)
        {
            float leftSample = 0.0f;
            float rightSample = 0.0f;
            
            for (int i = 0; i < 6; i++)
            {
                sustainsFinal[i] = tExpSmooth_tick(&susSmoothers[i]);
            }
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                for (int j = 0; j < 6; j++)
                {
                    tADSR4_setSustain(&FM_envs[i][j], sustainsFinal[j]); //FM_sustains[Rsound][j] * displayValues[7]);
                }
            }
            
            for (int i = 0; i < numVoices; i++)
            {
                float myFrequency = freq[i];
                float sample = 0.0f;
                tCycle_setFreq(&FM_sines[i][5], (myFrequency  * FM_freqRatios[Rsound][5]) + (FM_indices[Rsound][5] * feedback_output * displayValues[0]));
                feedback_output = tCycle_tick(&FM_sines[i][5]);
                tCycle_setFreq(&FM_sines[i][4], (myFrequency  * FM_freqRatios[Rsound][4]) + (FM_indices[Rsound][4] * feedback_output * displayValues[0] * tADSR4_tick(&FM_envs[i][5])));
                tCycle_setFreq(&FM_sines[i][3], (myFrequency  * FM_freqRatios[Rsound][3]) + (FM_indices[Rsound][3] * displayValues[0] * tCycle_tick(&FM_sines[i][4]) * tADSR4_tickNoInterp(&FM_envs[i][4])));
                tCycle_setFreq(&FM_sines[i][2], (myFrequency  * FM_freqRatios[Rsound][2]) + (FM_indices[Rsound][2] * displayValues[0] * tCycle_tick(&FM_sines[i][3]) * tADSR4_tickNoInterp(&FM_envs[i][3])));
                tCycle_setFreq(&FM_sines[i][1], myFrequency  * FM_freqRatios[Rsound][1]);
                tCycle_setFreq(&FM_sines[i][0],( myFrequency  * FM_freqRatios[Rsound][0]) + (FM_indices[Rsound][0] * displayValues[0] * tCycle_tick(&FM_sines[i][1]) * tADSR4_tickNoInterp(&FM_envs[i][1])));
                sample += (tCycle_tick(&FM_sines[i][2]) * tADSR4_tickNoInterp(&FM_envs[i][2]));
                sample += tCycle_tick(&FM_sines[i][0]) * tADSR4_tickNoInterp(&FM_envs[i][0]);
                leftSample += sample*((0.5f * (1.0f - displayValues[4])) + (displayValues[4] * (1.0f - panValues[i])));
                rightSample += sample*((0.5f * (1.0f - displayValues[4])) + (displayValues[4] * (panValues[i])));
            }
            
            leftSample *= 0.4f;
            rightSample *= 0.4f;
            float tremoloSignal = ((tCycle_tick(&tremolo) * 0.5f) + 0.5f) * displayValues[1];
            
            if (tremoloStereo)
            {
                leftSample *= (tremoloSignal + (1.0f - displayValues[1]));
                rightSample *= ((1.0f-tremoloSignal) + (1.0f - displayValues[1]));
            }
            else
            {
                leftSample *= (tremoloSignal + (1.0f - displayValues[1]));
                rightSample *= ((tremoloSignal) + (1.0f - displayValues[1]));
            }
            
            leftSample *= displayValues[3]; //drive
            leftSample = tanhf(leftSample);
            
            rightSample *= displayValues[3]; //drive
            rightSample = tanhf(rightSample);
            
            
            input[0] = leftSample;
            input[1] = rightSample;
            
        }
        
        void SFXRhodesFree(void)
        {
            for (int i = 0; i < NUM_VOC_VOICES; i++)
            {
                for (int j = 0; j < 6; j++)
                {
                    tCycle_free(&FM_sines[i][j]);
                    tADSR4_free(&FM_envs[i][j]);
                }
                
            }
            for (int i = 0; i < 6; i++)
            {
                tExpSmooth_free(&susSmoothers[i]);
            }
            tCycle_free(&tremolo);
            
        }
        
        
        
        // midi functions
        
        float pitchBendValue = 0.0f;
        
        void calculateFreq(int voice)
        {
            float tempNote = (float)tSimplePoly_getPitch(&poly, voice) + pitchBendValue;
            float tempPitchClass = ((((int)tempNote) - keyCenter) % 12 );
            float tunedNote = tempNote + centsDeviation[(int)tempPitchClass];
            freq[voice] = LEAF_midiToFrequency(tunedNote);
        }
        
        float calculateTunedMidiNote(float tempNote)
        {
            tempNote += pitchBendValue;
            float tempPitchClass = ((((int)tempNote) - keyCenter) % 12 ) ;
            return (tempNote + centsDeviation[(int)tempPitchClass]);
        }
        
        void calculateNoteArray()
        {
            for (int i = 0; i < 128; i++)
            {
                float tempNote = i;
                float tempPitchClass = ((((int)tempNote) - keyCenter) % 12 );
                float tunedNote = tempNote + centsDeviation[(int)tempPitchClass];
                notes[i] = tunedNote;
            }
        }
        
        
        float nearestNote(float note)
        {
            float leastDifference = fabsf(note - notes[0]);
            float difference;
            int index = 0;
            int* chord;
            
            if (autotuneChromatic > 0)
            {
                chord = chromaticArray;
            }
            else
            {
                chord = chordArray;
            }
            //if (autotuneLock > 0) chord = lockArray;
            
            for(int i = 1; i < 128; i++)
            {
                if (chord[i%12] > 0)
                {
                    difference = fabsf(note - notes[i]);
                    if(difference < leastDifference)
                    {
                        leastDifference = difference;
                        index = i;
                    }
                }
            }
            
            return notes[index];
            
        }
        
        
        int lastNearNote = -1;
        
        
        float nearestNoteWithHysteresis(float note, float hysteresis)
        {
            float leastDifference = fastabsf(note - notes[0]);
            float difference;
            int nearIndex = 0;
            int* chord;
            float output = 0.0f;
            
            if (autotuneChromatic > 0)
            {
                chord = chromaticArray;
            }
            else
            {
                chord = chordArray;
            }
            if (autotuneLock > 0)
            {
                chord = lockArray;
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
                        difference = fastabsf(note - notes[i]);
                        if(difference < leastDifference)
                        {
                            leastDifference = difference;
                            nearIndex = i;
                        }
                    }
                }
                
                if (lastNearNote == -1)
                {
                    output = notes[nearIndex];
                    lastNearNote = nearIndex;
                    return output;
                }
                if (nearIndex != lastNearNote)
                {
                    //check if it's beyond the hysteresis
                    
                    //find closest note in chord upward from lastNearNote
                    int upNote = 0;
                    int downNote = 128;
                    int i = lastNearNote;
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
                    i = lastNearNote;
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
                    float upperNearHyst = (notes[upNote] - notes[lastNearNote]) * hysteresis;
                    float lowerNearHyst = (notes[lastNearNote] - notes[downNote]) * -hysteresis;
                    
                    float theDifference = note - notes[lastNearNote];
                    if ((theDifference > upperNearHyst) || (theDifference < lowerNearHyst))
                    {
                        output = notes[nearIndex];
                        lastNearNote = nearIndex;
                        
                    }
                    else
                    {
                        output =notes[lastNearNote];
                    }
                }
                else
                {
                    output = notes[nearIndex];
                }
            }
            else
            {
                output = -1.0f; //signal that there are no notes to snap to
            }
            return output;
        }
        
        int newBuffer[NUM_SAMPLER_KEYS];
        
        void noteOn(int key, int velocity)
        {
            
            if (!velocity)
            {
                noteOff(key, velocity);
            }
            
            else
            {
                
                chordArray[key%12]++;
                
                
                if (currentPreset == AutotuneMono)
                {
                    if (autotuneLock)
                    {
                        lockArray[key%12]++;
                    }
                }
                
                
                
                
                
                
                if (currentPreset == Rhodes)
                {
                    
                    int whichVoice = tSimplePoly_noteOn(&poly, key, velocity);
                    
                    if (whichVoice >= 0)
                    {
                        for (int j = 0; j < 6; j++)
                        {
                            tADSR4_on(&FM_envs[whichVoice][j], velocity * 0.0078125f);
                        }
                        panValues[whichVoice] = key * 0.0078125f; // divide by 128.0f
                    }
                }
                else if (currentPreset == ClassicSynth)
                {
                    
                    int whichVoice = tSimplePoly_noteOn(&poly, key, velocity);
                    
                    if (whichVoice >= 0)
                    {
                        tADSR4_on(&polyEnvs[whichVoice], velocity * 0.0078125f);
                        tADSR4_on(&polyFiltEnvs[whichVoice], velocity * 0.0078125f);
                    }
                }
                
                
                else if (currentPreset == SamplerKeyboard)
                {
                    if ((key >= LOWEST_SAMPLER_KEY) && key < (LOWEST_SAMPLER_KEY + NUM_SAMPLER_KEYS))
                    {
                        int whichVoice = tSimplePoly_noteOn(&poly, key, velocity);
                        if (whichVoice >= 0)
                        {
                            
                            currentSamplerKeyGlobal = key - LOWEST_SAMPLER_KEY;
                            ///if (!controlAllKeys)
                            //{
                            //setKnobValues(keyKnobValues[currentSamplerKey]);
                            //prevKnob2 = keyKnobValues[currentSamplerKey][2];
                            //prevKnob4 = keyKnobValues[currentSamplerKey][4];
                            //}
                            if (tBuffer_getRecordedLength(&keyBuff[currentSamplerKeyGlobal]) == 0)
                            {
                                tBuffer_record(&keyBuff[currentSamplerKeyGlobal]);
                                newBuffer[currentSamplerKeyGlobal] = 1;
                            }
                            else
                            {
                                tSampler_play(&keySampler[currentSamplerKeyGlobal]);
                                if (newBuffer[currentSamplerKeyGlobal])
                                {
                                    int recordLength = tBuffer_getRecordedLength(&keyBuff[currentSamplerKeyGlobal]);
                                    samplePlayLengths[currentSamplerKeyGlobal] = recordLength;
                                    newBuffer[currentSamplerKeyGlobal] = 0;
                                }
                                float tempGain = (velocity * 0.0078125f * displayValues[6]) + (1.0f - displayValues[6]);
                                tExpSmooth_setDest(&kSamplerGains[currentSamplerKeyGlobal], tempGain);
                            }
                            samplerKeyHeld[currentSamplerKeyGlobal] = 1;
                        }
                    }
                }
                else if (currentPreset == LivingStringSynth)
                {
                    int whichVoice = tSimplePoly_noteOn(&poly, key, velocity);
                    if (whichVoice >= 0)
                    {
                        tADSR4_on(&pluckEnvs[whichVoice], velocity * 0.0078125f);
                    }
                }
                else
                {
                    tSimplePoly_noteOn(&poly, key, velocity);
                }
                setLED_2(1);
            }
        }
        
        void noteOff(int key, int velocity)
        {
            if (chordArray[key%12] > 0) chordArray[key%12]--;
            
            
            
            if (currentPreset == Rhodes)
            {
                int voice;
                if (tSimplePoly_getNumVoices(&poly) > 1)
                {
                    voice = tSimplePoly_markPendingNoteOff(&poly, key); //if we're polyphonic, we need to let release envelopes happen and not mark voices free when they are not
                }
                else
                {
                    voice = tSimplePoly_noteOff(&poly, key); //if we're monophonic, we need to allow fast voice stealing and returning to previous stolen notes without regard for the release envelopes
                }
                if (voice >= 0)
                {
                    for (int j = 0; j < 6; j++)
                    {
                        tADSR4_off(&FM_envs[voice][j]);
                    }
                }
                
            }
            else if (currentPreset == ClassicSynth)
            {
                int voice;
                if (tSimplePoly_getNumVoices(&poly) > 1)
                {
                    voice = tSimplePoly_markPendingNoteOff(&poly, key); //if we're polyphonic, we need to let release envelopes happen and not mark voices free when they are not
                }
                else
                {
                    voice = tSimplePoly_noteOff(&poly, key); //if we're monophonic, we need to allow fast voice stealing and returning to previous stolen notes without regard for the release envelopes
                }
                
                if (voice >= 0)
                {
                    tADSR4_off(&polyEnvs[voice]);
                    tADSR4_off(&polyFiltEnvs[voice]);
                }
            }
            
            else if (currentPreset == SamplerKeyboard)
            {
                int voice;
                
                
                if (key >= LOWEST_SAMPLER_KEY && key < LOWEST_SAMPLER_KEY + NUM_SAMPLER_KEYS)
                {
                    voice = tSimplePoly_markPendingNoteOff(&poly, key); //if we're polyphonic, we need to let release envelopes happen and not mark voices free when they are not
                    
                    
                    if (tBuffer_isActive(&keyBuff[key-LOWEST_SAMPLER_KEY]) == 1)
                    {
                        tBuffer_stop(&keyBuff[key-LOWEST_SAMPLER_KEY]);
                        UISamplerKButtons(ButtonUp, ActionPress);
                    }
                    else
                    {
                        tExpSmooth_setDest(&kSamplerGains[key-LOWEST_SAMPLER_KEY], 0.0f);
                    }
                    samplerKeyHeld[key-LOWEST_SAMPLER_KEY] = 0;
                    UISamplerKButtons(ButtonC, ActionHoldContinuous);
                    tSampler_stop(&keySampler[key-LOWEST_SAMPLER_KEY]);
                    waitingForDeactivation[voice] = key;
                }
            }
            else if (currentPreset == LivingStringSynth)
            {
                int voice;
                
                voice = tSimplePoly_noteOff(&poly, key); //if we're monophonic, we need to allow fast voice stealing and returning to previous stolen notes without regard for the release envelopes
                
                if (voice >= 0)
                {
                    tADSR4_off(&pluckEnvs[voice]);
                }
            }
            else
            {
                tSimplePoly_noteOff(&poly, key);
            }
            
            if (tSimplePoly_getNumActiveVoices(&poly) < 1)
            {
                setLED_2(0);
            }
            
        }
        
        
        void pitchBend(int data)
        {
            pitchBendValue = (data - 8192) * 0.000244140625f;
        }
        
        
        void sustainOff()
        {
            
        }
        
        void sustainOn()
        {
            
        }
        
        void toggleBypass()
        {
            
        }
        
        void toggleSustain()
        {
            
        }
        
        void ctrlInput(int ctrl, int value)
        {
            
        }
        
#ifdef __cplusplus
    }
} // extern "C"
#endif
