/*
 * sfx.h
 *
 *  Created on: Dec 23, 2019
 *      Author: josnyder
 */
#ifndef SFX_H_
#define SFX_H_

#ifndef __cplusplus
#include "audiostream.h"
#else
#include <JuceHeader.h>
#endif

#include "ui.h"

#ifdef __cplusplus
namespace vocodec
{
    extern "C"
    {
#endif
        
        //#define SMALL_MEM_SIZE 16328
#define SMALL_MEM_SIZE 80328
#define MED_MEM_SIZE 519000
#define LARGE_MEM_SIZE 33554432 //32 MBytes - size of SDRAM IC
        
#ifdef __cplusplus
        extern char small_memory[SMALL_MEM_SIZE];
        extern char medium_memory[MED_MEM_SIZE];
        extern char large_memory[LARGE_MEM_SIZE];
#else
        extern char small_memory[SMALL_MEM_SIZE];
        extern char medium_memory[MED_MEM_SIZE] __ATTR_RAM_D1;
        extern char large_memory[LARGE_MEM_SIZE] __ATTR_SDRAM;
#endif
        
        extern void (*allocFunctions[PresetNil])(void);
        extern void (*frameFunctions[PresetNil])(void);
        extern void (*tickFunctions[PresetNil])(float*);
        extern void (*freeFunctions[PresetNil])(void);
        
        extern tMempool mediumPool;
        extern tMempool largePool;
        
#define NUM_VOC_VOICES 8
#define INV_NUM_VOC_VOICES 0.125
        
#define NUM_VOC_OSC 1
#define INV_NUM_VOC_OSC 1
        
#define NUM_OSC_PER_VOICE 3
#define INV_NUM_OSC_PER_VOICE 0.33f
        
#define NUM_AUTOTUNE 4
#define NUM_RETUNE 1
#define MAX_OVERSAMPLER_RATIO 4
#define OVERSAMPLER_HQ FALSE
        
#define NUM_SAMPLER_VOICES 6 // need to limit this because too many samplers going can take too long
#define NUM_SAMPLER_KEYS 49
#define LOWEST_SAMPLER_KEY 36
        
        extern float defaultPresetKnobValues[PresetNil][NUM_PRESET_KNOB_VALUES];
        extern float presetKnobValues[PresetNil][NUM_PRESET_KNOB_VALUES];
        extern int knobActive[NUM_ADC_CHANNELS];
        
        extern tSimplePoly poly;
        extern tExpSmooth polyRamp[NUM_VOC_VOICES];
        extern tSawtooth osc[NUM_VOC_VOICES * NUM_OSC_PER_VOICE];
        
        extern PlayMode samplerMode;
        extern float sampleLength;
        
        extern int freeze;
        
        void initGlobalSFXObjects();
        
        void initFunctionPointers(void);
        
        // vocoder
        extern int numVoices;
        extern int internalExternal;
        
        extern int vocFreezeLPC;
        //LPC Vocoder
        void SFXVocoderAlloc();
        void SFXVocoderFrame();
        void SFXVocoderTick(float* input);
        void SFXVocoderFree(void);
        
#define MAX_VOCODER_FILTER_ORDER 2
#define MAX_NUM_VOCODER_BANDS 24
        
        extern int vocChFreeze;
        //channel Vocoder
        void SFXVocoderChAlloc();
        void SFXVocoderChFrame();
        void SFXVocoderChTick(float* input);
        void SFXVocoderChFree(void);
        
        // pitch shift
        void SFXPitchShiftAlloc();
        void SFXPitchShiftFrame();
        void SFXPitchShiftTick(float* input);
        void SFXPitchShiftFree(void);
        
        // neartune
        extern int autotuneChromatic;
        extern int autotuneLock;
        void SFXNeartuneAlloc();
        void SFXNeartuneFrame();
        void SFXNeartuneTick(float* input);
        void SFXNeartuneFree(void);
        
        // autotune
        void SFXAutotuneAlloc();
        void SFXAutotuneFrame();
        void SFXAutotuneTick(float* input);
        void SFXAutotuneFree(void);
        
        // sampler - button press
        extern int samplePlaying;
        extern int bpMode;
        void SFXSamplerBPAlloc();
        void SFXSamplerBPFrame();
        void SFXSamplerBPTick(float* input);
        void SFXSamplerBPFree(void);
        
        // sampler - keyboard
        extern int currentSamplerKeyGlobal;
        extern int editingSamplerKey;
        extern float recSampleLength;
        extern float editSampleLength;
        extern int controlAllKeys;
        
        void SFXSamplerKAlloc();
        void SFXSamplerKFrame();
        void SFXSamplerKTick(float* input);
        void SFXSamplerKFree(void);
        
        // sampler - auto ch1
        extern int triggerChannel;
        extern int pitchQuantization;
        
        void SFXSamplerAutoAlloc();
        void SFXSamplerAutoFrame();
        void SFXSamplerAutoTick(float* input);
        void SFXSamplerAutoFree(void);
        
        // distortion tanh
        extern int distortionMode;
        
        void SFXDistortionAlloc();
        void SFXDistortionFrame();
        void SFXDistortionTick(float* input);
        void SFXDistortionFree(void);
        
        // distortion wave folder
        extern int foldMode;
        void SFXWaveFolderAlloc();
        void SFXWaveFolderFrame();
        void SFXWaveFolderTick(float* input);
        void SFXWaveFolderFree(void);
        
        extern int crusherStereo;
        // bitcrusher
        void SFXBitcrusherAlloc();
        void SFXBitcrusherFrame();
        void SFXBitcrusherTick(float* input);
        void SFXBitcrusherFree(void);
        
        
        // delay
        extern int delayShaper;
        extern int capFeedback;
        
        void SFXDelayAlloc();
        void SFXDelayFrame();
        void SFXDelayTick(float* input);
        void SFXDelayFree(void);
        
        
        // reverb
        void SFXReverbAlloc();
        void SFXReverbFrame();
        void SFXReverbTick(float* input);
        void SFXReverbFree(void);
        
        // reverb2
        void SFXReverb2Alloc();
        void SFXReverb2Frame();
        void SFXReverb2Tick(float* input);
        void SFXReverb2Free(void);
        
        // living string
        extern int levMode;
        extern int ignoreFreqKnobs;
        extern int independentStrings;
        void SFXLivingStringAlloc();
        void SFXLivingStringFrame();
        void SFXLivingStringTick(float* input);
        void SFXLivingStringFree(void);
        
        // living string synth
        extern int voicePluck;
        extern int levModeStr;
        void SFXLivingStringSynthAlloc();
        void SFXLivingStringSynthFrame();
        void SFXLivingStringSynthTick(float* input);
        void SFXLivingStringSynthFree(void);
        
        
        // classic synth
        extern int csKnobPage;
        void SFXClassicSynthAlloc();
        void SFXClassicSynthFrame();
        void SFXClassicSynthTick(float* input);
        void SFXClassicSynthFree(void);
        
        // rhodes
        extern const char* soundNames[5];
        extern int Rsound;
        extern int tremoloStereo;
        void SFXRhodesAlloc();
        void SFXRhodesFrame();
        void SFXRhodesTick(float* input);
        void SFXRhodesFree(void);
        
        
        // MIDI FUNCTIONS
        void noteOn(int key, int velocity);
        void noteOff(int key, int velocity);
        void pitchBend(int data);
        void sustainOn(void);
        void sustainOff(void);
        void toggleBypass(void);
        void toggleSustain(void);
        
        void calculateFreq(int voice);
        
        float calculateTunedMidiNote(float tempNote);
        
        
        void calculateNoteArray(void);
        float nearestNote(float period);
        float nearestNoteWithHysteresis(float note, float hysteresis);
        
        void clearNotes(void);
        
        void ctrlInput(int ctrl, int value);
        
        
#ifdef __cplusplus
    }
} // extern "C"
#endif


#endif /* SFX_H_ */
