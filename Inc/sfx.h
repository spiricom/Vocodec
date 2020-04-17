/*
 * sfx.h
 *
 *  Created on: Dec 23, 2019
 *      Author: josnyder
 */
#ifndef SFX_H_
#define SFX_H_

#include "audiostream.h"
#include "ui.h"

#define NUM_VOC_VOICES 8
#define INV_NUM_VOC_VOICES 0.125

#define NUM_VOC_OSC 1
#define INV_NUM_VOC_OSC 1

#define NUM_OSC_PER_VOICE 8
#define INV_NUM_OSC_PER_VOICE 0.125

#define NUM_AUTOTUNE 4
#define NUM_RETUNE 1
#define OVERSAMPLER_RATIO 8
#define OVERSAMPLER_HQ FALSE

#define NUM_SAMPLER_KEYS 76
#define LOWEST_SAMPLER_KEY 28

extern float presetKnobValues[PresetNil][NUM_ADC_CHANNELS];
extern uint8_t knobActive[NUM_ADC_CHANNELS];

extern tPoly poly;
extern tRamp polyRamp[NUM_VOC_VOICES];
extern tSawtooth osc[NUM_VOC_VOICES * NUM_OSC_PER_VOICE];

extern PlayMode samplerMode;
extern float sampleLength;

extern uint32_t freeze;

void initGlobalSFXObjects();

// vocoder
extern uint8_t numVoices;
extern uint8_t internalExternal;

void SFXVocoderAlloc();
void SFXVocoderFrame();
void SFXVocoderTick(float audioIn);
void SFXVocoderFree(void);

// pitch shift
void SFXPitchShiftAlloc();
void SFXPitchShiftFrame();
void SFXPitchShiftTick(float audioIn);
void SFXPitchShiftFree(void);

// neartune
extern uint8_t autotuneChromatic;

void SFXNeartuneAlloc();
void SFXNeartuneFrame();
void SFXNeartuneTick(float audioIn);
void SFXNeartuneFree(void);

// autotune
void SFXAutotuneAlloc();
void SFXAutotuneFrame();
void SFXAutotuneTick(float audioIn);
void SFXAutotuneFree(void);

// sampler - button press
extern uint8_t samplePlaying;

void SFXSamplerBPAlloc();
void SFXSamplerBPFrame();
void SFXSamplerBPTick(float audioIn);
void SFXSamplerBPFree(void);

// sampler - keyboard
extern int currentSamplerKey;
extern int recordingSamplerKey;
extern int editingSamplerKey;
extern float recSampleLength;
extern float editSampleLength;

void SFXSamplerKAlloc();
void SFXSamplerKFrame();
void SFXSamplerKTick(float audioIn);
void SFXSamplerKFree(void);

// sampler - auto ch1
extern uint8_t triggerChannel;

void SFXSamplerAutoAlloc();
void SFXSamplerAutoFrame();
void SFXSamplerAutoTick(float audioIn);
void SFXSamplerAutoFree(void);

// distortion tanh
extern uint8_t distortionMode;

void SFXDistortionAlloc();
void SFXDistortionFrame();
void SFXDistortionTick(float audioIn);
void SFXDistortionFree(void);

// distortion wave folder
void SFXWaveFolderAlloc();
void SFXWaveFolderFrame();
void SFXWaveFolderTick(float audioIn);
void SFXWaveFolderFree(void);


// bitcrusher
void SFXBitcrusherAlloc();
void SFXBitcrusherFrame();
void SFXBitcrusherTick(float audioIn);
void SFXBitcrusherFree(void);


// delay
extern int delayShaper;

void SFXDelayAlloc();
void SFXDelayFrame();
void SFXDelayTick(float audioIn);
void SFXDelayFree(void);


// reverb
void SFXReverbAlloc();
void SFXReverbFrame();
void SFXReverbTick(float audioIn);
void SFXReverbFree(void);

// reverb2
void SFXReverb2Alloc();
void SFXReverb2Frame();
void SFXReverb2Tick(float audioIn);
void SFXReverb2Free(void);

// living string
void SFXLivingStringAlloc();
void SFXLivingStringFrame();
void SFXLivingStringTick(float audioIn);
void SFXLivingStringFree(void);

// living string synth
void SFXLivingStringSynthAlloc();
void SFXLivingStringSynthFrame();
void SFXLivingStringSynthTick(float audioIn);
void SFXLivingStringSynthFree(void);


// classic synth
void SFXClassicSynthAlloc();
void SFXClassicSynthFrame();
void SFXClassicSynthTick(float audioIn);
void SFXClassicSynthFree(void);


// rhodes
void SFXRhodesAlloc();
void SFXRhodesFrame();
void SFXRhodesTick(float audioIn);
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
//void calculatePeriodArray(void);
float nearestNote(float period);
//float nearestPeriod(float period);

void clearNotes(void);

void ctrlInput(int ctrl, int value);


#endif /* SFX_H_ */
