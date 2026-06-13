/*
 * synth.h
 *
 *  Created on: Jun 11, 2026
 *      Author: josnyder
 */

#ifndef SYNTH_H_
#define SYNTH_H_

void synthInit(void);
void synthSetFreq(float freq);
float synthTick(void);


#include "parameters.h"
#include "leaf.h"
#include "main.h"
#include "audiostream.h"


extern float sourceValues[NUM_SOURCES][NUM_STRINGS_PER_BOARD];
extern uint8_t lfoOn[NUM_LFOS];
extern uint8_t envOn[NUM_ENV];
extern uint8_t oscOn[NUM_OSC];
extern uint8_t noiseOn;
extern float oscAmpMult;
extern float oscAmpMultArray[4];
extern uint8_t numStringsThisBoard;
extern uint8_t oscToTick;
extern uint8_t filterToTick;
extern uint32_t overSampled;
extern uint8_t numEffectToTick;
extern volatile uint16_t stringInputs[NUM_STRINGS];
void audioFrameSynth(uint16_t buffer_offset);
float audioTickSynth();
void audioInitSynth();
void  audioFreeSynth();
void  audioSwitchToSynth();
void changeOversampling(uint32_t newOS);
void oscillator_tick(float note, int string);

typedef void (*shapeTick_t)(float*, int, float, float, int, int string);
typedef void (*lfoShapeTick_t)(float*, int, int string);
void sawSquareTick(float* sample, int v, float freq, float shape, int sync, int string);
void sineTriTick(float* sample, int v, float freq, float shape, int sync, int string);
void sawTick(float* sample, int v, float freq, float shape, int sync, int string);
void pulseTick(float* sample, int v, float freq, float shape, int sync, int string);
void sineTick(float* sample, int v, float freq, float shape, int sync, int string);
void triTick(float* sample, int v, float freq, float shape, int sync, int string);
void userTick(float* sample, int v, float freq, float shape, int sync, int string);

void lfoSawSquareTick(float* sample, int v, int string);
void lfoSineTriTick(float* sample, int v, int string);
void lfoSineTick(float* sample, int v, int string);
void lfoTriTick(float* sample, int v, int string);
void lfoSawTick(float* sample, int v, int string);
void lfoPulseTick(float* sample, int v, int string);

void lfoSawSquareSetRate(float r, int v, int string);
void lfoSineTriSetRate(float r, int v, int string);
void lfoSineSetRate(float r, int v, int string);
void lfoTriSetRate(float r, int v, int string);
void lfoSawSetRate(float r, int v, int string);
void lfoPulseSetRate(float r, int v, int string);


void lfoSawSquareSetPhase(float p, int v, int string);
void lfoSineTriSetPhase(float p, int v, int string);
void lfoSineSetPhase(float p, int v, int string);
void lfoTriSetPhase(float p, int v, int string);
void lfoSawSetPhase(float p, int v, int string);
void lfoPulseSetPhase(float p, int v, int string);


void lfoSawSquareSetShape(float s, int v, int string);
void lfoSineTriSetShape(float s, int v, int string);
void lfoSineSetShape(float s, int v, int string);
void lfoTriSetShape(float s, int v, int string);
void lfoSawSetShape(float s, int v, int string);
void lfoPulseSetShape(float s, int v, int string);

extern shapeTick_t shapeTick[NUM_OSC];
extern lfoShapeTick_t lfoShapeTick[NUM_LFOS];

void noise_tick(int string);
void noiseSetFreq(float value, int v, int string);
void noiseSetGain(float value, int v, int string);
void noiseSetTilt(float value, int v, int string);

float filter_tick(float* samples, float note, int string);

typedef void (*filterTick_t)(float*, int, float, int);
void setFreqMultPitch(float pitch, int osc, int string);
void setFreqMultHarm(float harm, int osc, int string);
void lowpassTick(float* sample, int v, float cutoff, int string);
void highpassTick(float* sample, int v, float cutoff, int string);
void bandpassTick(float* sample, int v, float cutoff, int string);
void diodeLowpassTick(float* sample, int v, float cutoff, int string);
void LadderLowpassTick(float* sample, int v, float cutoff, int string);
void VZlowshelfTick(float* sample, int v, float cutoff, int string);
void VZhighshelfTick(float* sample, int v, float cutoff, int string);
void VZpeakTick(float* sample, int v, float cutoff, int string);
void VZbandrejectTick(float* sample, int v, float cutoff, int string);

void filterSetCutoff(float cutoff, int v, int string);
void filterSetKeyfollow(float keyfollow, int v, int string);
void lowpassSetQ(float q, int v, int string);
void highpassSetQ(float q, int v, int string);
void bandpassSetQ(float q, int v, int string);
void diodeLowpassSetQ(float q, int v, int string);
void VZpeakSetQ(float bw, int v, int string);
void VZlowshelfSetQ(float bw, int v, int string);
void VZhighshelfSetQ(float bw, int v, int string);
void VZbandrejectSetQ(float bw, int v, int string);
void LadderLowpassSetQ(float q, int v, int string);
void lowpassSetGain(float gain, int v, int string);
void highpassSetGain(float gain, int v, int string);
void bandpassSetGain(float gain, int v, int string);
void diodeLowpassSetGain(float gain, int v, int string);
void VZpeakSetGain(float gain, int v, int string);
void VZlowshelfSetGain(float gain, int v, int string);
void VZhighshelfSetGain(float gain, int v, int string);
void VZbandrejectSetGain(float gain, int v, int string);
void LadderLowpassSetGain(float gain, int v, int string);
extern filterTick_t filterTick[NUM_FILT];

//envelope functions
void setEnvelopeAttack(float a, int v, int string);
void setEnvelopeDecay(float d, int v, int string);
void setEnvelopeSustain(float s, int v, int string);
void setEnvelopeRelease(float r, int v, int string);
void setEnvelopeLeak(float leak, int v, int string);

//effects


typedef float (*effectTick_t)(float sample,int v, int string);
extern effectTick_t effectTick[NUM_EFFECT];
float blankTick(float sample, int v, int string);
float tiltFilterTick(float sample,int v, int string);
float hardClipTick(float sample, int v, int string);
float tanhTick(float sample, int v, int string);
float softClipTick(float sample, int v, int string);
float polynomialShaperTick(float sample, int v, int string);
float delayTick(float sample, int v, int string);
float satTick(float sample, int v, int string);
float bcTick(float sample, int v, int string);
float compressorTick(float sample, int v, int string);
float shaperTick(float sample, int v, int string);
float wavefolderTick(float sample, int v, int string);
float chorusTick(float sample, int v, int string);

void param1Linear(float value, int v, int string);
void clipperGainSet(float value, int v, int string);
void wavefolderParam1(float value, int v, int string);
void wavefolderParam3(float value, int v, int string);
void tiltParam1(float value, int v, int string);
void tiltParam2(float value, int v, int string);
void tiltParam3(float value, int v, int string);
void tiltParam4(float value, int v, int string);
void compressorParam1(float value, int v, int string);
void compressorParam2(float value, int v, int string);
void compressorParam3(float value, int v, int string);
void compressorParam4(float value, int v, int string);
void compressorParam5(float value, int v, int string);

void delayParam1(float value, int v, int string);
void delayParam2(float value, int v, int string);
void delayParam3(float value, int v, int string);
void delayParam4(float value, int v, int string);
void delayParam5(float value, int v, int string);

void offsetParam2(float value, int v, int string);
void param2Linear(float value, int v, int string);
void param2BC(float value, int v, int string);

void param3Linear(float value, int v, int string);
void param3Soft(float value, int v, int string);
void param3Hard(float value, int v, int string);
void param3Poly(float value, int v, int string);
void param3BC(float value, int v, int string);

void param4BC(float value, int v, int string);
void param4Linear(float value, int v, int string);

void param5BC(float value, int v, int string);
void param5Linear(float value, int v, int string);
void fxMixSet(float value, int v, int string);
void fxPostGainSet(float value, int v, int string);
float wavefolderTick(float sample, int v, int string);
void chorusParam1(float value, int v, int string);
void chorusParam2(float value, int v, int string);
void chorusParam3(float value, int v, int string);
void chorusParam4(float value, int v, int string);

void FXLowpassParam1(float value, int v, int string);
void FXLowpassParam3(float value, int v, int string);
void FXHighpassParam1(float value, int v, int string);
void FXHighpassParam3(float value, int v, int string);
void FXBandpassParam1(float value, int v, int string);
void FXBandpassParam3(float value, int v, int string);
void FXDiodeParam1(float value, int v, int string);
void FXDiodeParam3(float value, int v, int string);
void FXPeakParam1(float value, int v, int string);
void FXPeakParam2(float value, int v, int string);
void FXPeakParam3(float value, int v, int string);
void FXLowShelfParam1(float value, int v, int string);
void FXLowShelfParam2(float value, int v, int string);
void FXLowShelfParam3(float value, int v, int string);
void FXHighShelfParam1(float value, int v, int string);
void FXHighShelfParam2(float value, int v, int string);
void FXHighShelfParam3(float vlaue, int v, int string);
void FXNotchParam1(float value, int v, int string);
void FXNotchParam2(float value, int v, int string);
void FXNotchParam3(float value, int v, int string);
void FXLadderParam1(float value, int v, int string);
void FXLadderParam3(float value, int v, int string);
float FXlowpassTick(float sample, int v, int string);
float FXhighpassTick(float sample, int v, int string);
float FXbandpassTick(float sample, int v, int string);
float FXdiodeLowpassTick(float sample, int v, int string);
float FXLadderLowpassTick(float sample, int v, int string);
float FXVZlowshelfTick(float sample, int v, int string);
float FXVZhighshelfTick(float sample, int v, int string);
float FXVZpeakTick(float sample, int v, int string);
float FXVZbandrejectTick(float sample, int v, int string);

void tickMappings(void);
void envelope_tick(int string);
void lfo_tick(int string);


//master functions
void setAmp(float amp, int v, int string);
void setFinalLowpass(float in, int v, int string);
void setMaster(float amp, int v, int string);
void setTranspose(float in, int v, int string);
void setPitchBendRange(float in, int v, int string);

void changeOversampling(uint32_t newOS);

#endif /* SYNTH_H_ */
