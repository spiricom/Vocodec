/*
 * synth.c
 *
 *  Created on: Jun 11, 2026
 *      Author: josnyder
 */

#include "synth.h"
#include "audiostream.h"


#define OVERSAMPLE 4
float inv_oversample = 1.0f / OVERSAMPLE;
volatile float antiClickFade = 0.0f;
volatile uint32_t nanHappened = 0;
uint32_t prevOversample;
volatile uint32_t nanChuckTest  = 0;
volatile uint16_t previousStringInputs[12];

float oversamplerArray[OVERSAMPLE];

tOversampler os[NUM_STRINGS_PER_BOARD];
uint8_t numStringsThisBoard = NUM_STRINGS_PER_BOARD;
float octave = -12.0f;
float stringOctave[NUM_STRINGS_PER_BOARD];
float transpose = 0.0f;
//master
float amplitude[NUM_STRINGS_PER_BOARD];
float finalMaster[NUM_STRINGS_PER_BOARD];
volatile uint16_t stringInputs[NUM_STRINGS];

//function pointers
shapeTick_t shapeTick[NUM_OSC];
filterTick_t filterTick[NUM_FILT];
lfoShapeTick_t lfoShapeTick[NUM_LFOS];
effectTick_t effectTick[NUM_EFFECT];


//oscillators
tPBSaw saw[NUM_OSC][NUM_STRINGS_PER_BOARD];
tPBPulse pulse[NUM_OSC][NUM_STRINGS_PER_BOARD];
tCycle sine[NUM_OSC][NUM_STRINGS_PER_BOARD];
tPBTriangle tri[NUM_OSC][NUM_STRINGS_PER_BOARD];

uint8_t oscToTick = NUM_OSC;
uint8_t filterToTick = NUM_FILT;
uint32_t overSampled = 1;
uint8_t numEffectToTick = NUM_EFFECT;

// Using seperate objects for pairs to easily maintain phase relation
tPBSawSquare sawPaired[NUM_OSC][NUM_STRINGS_PER_BOARD];
tPBSineTriangle sinePaired[NUM_OSC][NUM_STRINGS_PER_BOARD];
uint8_t oscOn[NUM_OSC];
uint8_t noiseOn;


//noise
float noiseOuts[2][NUM_STRINGS_PER_BOARD];

tTiltFilter noiseTilt[NUM_STRINGS_PER_BOARD];
tVZFilterBell noiseBell1[NUM_STRINGS_PER_BOARD];
tIntPhasor lfoSaw[NUM_LFOS][NUM_STRINGS_PER_BOARD];
tSquareLFO lfoPulse[NUM_LFOS][NUM_STRINGS_PER_BOARD];
tCycle lfoSine[NUM_LFOS][NUM_STRINGS_PER_BOARD];
tTriLFO lfoTri[NUM_LFOS][NUM_STRINGS_PER_BOARD];
tSawSquareLFO lfoSawSquare[NUM_LFOS][NUM_STRINGS_PER_BOARD];
tSineTriLFO lfoSineTri[NUM_LFOS][NUM_STRINGS_PER_BOARD];
uint8_t lfoOn[NUM_LFOS];


//oscillator outputs
float outSamples[2][NUM_OSC][NUM_STRINGS_PER_BOARD];

//source vals
float sourceValues[NUM_SOURCES][NUM_STRINGS_PER_BOARD];

tExpSmooth mapSmoothers[MAX_NUM_MAPPINGS][NUM_STRINGS_PER_BOARD];
tExpSmooth pitchSmoother;
tExpSmooth filterCutoffSmoother[NUM_FILT][NUM_STRINGS_PER_BOARD];



float freqMult[NUM_OSC][NUM_STRINGS_PER_BOARD];
float midiAdd[NUM_OSC][NUM_STRINGS_PER_BOARD];
float bendRangeMultiplier = 0.002929866324849f; //default to divide by 48


//filters
tDiodeFilter diodeFilters[NUM_FILT][NUM_STRINGS_PER_BOARD];
tVZFilterBell VZfilterPeak[NUM_FILT][NUM_STRINGS_PER_BOARD];
tVZFilterLS VZfilterLS[NUM_FILT][NUM_STRINGS_PER_BOARD];
tVZFilterHS VZfilterHS[NUM_FILT][NUM_STRINGS_PER_BOARD];
tVZFilterBR VZfilterBR[NUM_FILT][NUM_STRINGS_PER_BOARD];
tSVF lowpass[NUM_FILT][NUM_STRINGS_PER_BOARD];
tSVF highpass[NUM_FILT][NUM_STRINGS_PER_BOARD];
tSVF bandpass[NUM_FILT][NUM_STRINGS_PER_BOARD];
tLadderFilter Ladderfilter[NUM_FILT][NUM_STRINGS_PER_BOARD];

float filterGain[NUM_FILT][NUM_STRINGS_PER_BOARD];

//envelopes
tADSRT envs[NUM_ENV][NUM_STRINGS_PER_BOARD];
uint8_t envOn[NUM_ENV];


//effects
 tHighpass dcBlock1[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tHighpass dcBlock2[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tTiltFilter FXTilt[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 //tVZFilterLS shelf1[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 //tVZFilterHS shelf2[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tVZFilterBell bell1[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tCompressor comp[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tCrusher bc[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tLockhartWavefolder wf[NUM_EFFECT][NUM_STRINGS_PER_BOARD];

 tCycle mod1[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tCycle mod2[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float fxMix[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float fxPostGain[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tDiodeFilter FXdiodeFilters[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tVZFilterBell FXVZfilterPeak[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tVZFilterLS FXVZfilterLS[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tVZFilterHS FXVZfilterHS[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tVZFilterBR FXVZfilterBR[NUM_EFFECT][NUM_STRINGS_PER_BOARD];

 tFeedbackLeveler feedbackControl[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tSVF FXlowpass[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tSVF FXhighpass[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tSVF FXbandpass[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 tLadderFilter FXLadderfilter[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float delayFB[NUM_EFFECT][NUM_STRINGS_PER_BOARD];

 //only one each of chorus delays and tapedelay
 tLinearDelay delay1[NUM_STRINGS_PER_BOARD];
 tLinearDelay delay2[NUM_STRINGS_PER_BOARD];
 tTapeDelay tapeDelay[NUM_STRINGS_PER_BOARD];


 tSVF_LP finalLowpass[NUM_STRINGS_PER_BOARD];

 uint8_t voiceSounding = 0;

 uint8_t randomValPointer = 0;

 float oscOuts[2][NUM_OSC][NUM_STRINGS_PER_BOARD];
float oscAmpMult = 1.0f;
 float oscAmpMultArray[4] = {0.0f, 1.0f, 0.707106781186548f, 0.5f};
 uint32_t timeFilt = 0;
 uint32_t timeTick = 0;
 uint32_t timeMap = 0;
 uint32_t timeOS = 0;
 uint32_t timeSmoothing = 0;
 uint32_t timeGettingNote = 0;
 uint32_t timeNoise = 0;
 uint32_t timePerStringTick = 0;
 uint32_t timeLFO = 0;
 uint32_t timeEnv = 0;
 uint32_t timeOsc = 0;
 uint32_t oscCountdown = 0;

 uint32_t timeMIDI = 0;
 const int syncMap[3] = {2, 0, 1};
 float prevNote = 0.0f;

 float param1[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float param2[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float param3[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float param4[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float param5[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float shapeDividerS[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float shapeDividerH[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float polyDivider[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float wfState[NUM_EFFECT][NUM_STRINGS_PER_BOARD];
 float invCurFB[NUM_EFFECT][NUM_STRINGS_PER_BOARD];


 tExpSmooth volumeSmoother;
 volatile float velocitySensitivity = 1.0f;

void audioInitSynth()
{
	for (int i = 0; i < OVERSAMPLE; i++)
    {
        oversamplerArray[i] = 0.0f;
    }
	for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
	{

		tOversampler_init(&os[v], OVERSAMPLE, 0, &leaf);

		for(int i = 0; i < NUM_OSC; i++)
		{
			tPBSaw_init(&saw[i][v], &leaf);

			tPBPulse_init(&pulse[i][v], &leaf);

			tCycle_init(&sine[i][v],  &leaf);

			tPBTriangle_init(&tri[i][v],  &leaf);

			tPBSawSquare_init(&sawPaired[i][v], &leaf);

			tPBSineTriangle_init(&sinePaired[i][v],&leaf);


			freqMult[i][v] = 1.0f;
			midiAdd[i][v] = 0.0f;

		}


		tExpSmooth_init(&pitchSmoother, 0.0f, 0.01f, &leaf);
		tExpSmooth_setValAndDest(pitchSmoother, 0.5f);
		tExpSmooth_init(&volumeSmoother, 0.0f, 0.001f, &leaf);
		tExpSmooth_setValAndDest(volumeSmoother, 1.0f);
		for (int i = 0; i < NUM_FILT; i++)
		{
			tSVF_init(&lowpass[i][v], SVFTypeLowpass, 2000.f, 0.7f, &leaf);
			tSVF_init(&highpass[i][v], SVFTypeHighpass, 2000.f, 0.7f, &leaf);
			tSVF_init(&bandpass[i][v], SVFTypeBandpass, 2000.f, 0.7f, &leaf);
			tDiodeFilter_init(&diodeFilters[i][v], 2000.f, 0.5f, &leaf);
			tVZFilterBell_init(&VZfilterPeak[i][v], 2000.f, 1.9f, 1.0f, &leaf);
			tVZFilterLS_init(&VZfilterLS[i][v], 2000.f, 0.6f, 1.0f, &leaf);
			tVZFilterHS_init(&VZfilterHS[i][v], 2000.f, 0.6f, 1.0f, &leaf);
			tVZFilterBR_init(&VZfilterBR[i][v], 2000.f, 1.0f, &leaf);
			tLadderFilter_init(&Ladderfilter[i][v], 2000.f, 1.0f, &leaf);
			//tLadderFilter_setOversampling(&Ladderfilter[i][v], 2);
			tExpSmooth_init(&filterCutoffSmoother[i][v], 0.0f, 0.001f, &leaf);
			tExpSmooth_setValAndDest(filterCutoffSmoother[i][v], 0.5f);
		}

		for (int i = 0; i < NUM_LFOS; i++)
		{
			tIntPhasor_init(&lfoSaw[i][v], &leaf);
			tSquareLFO_init(&lfoPulse[i][v], &leaf);
			tCycle_init(&lfoSine[i][v], &leaf);
			tTriLFO_init(&lfoTri[i][v], &leaf);

			tSineTriLFO_init(&lfoSineTri[i][v], &leaf);
			tSawSquareLFO_init(&lfoSawSquare[i][v], &leaf);
		}


		for (int i = 0; i < NUM_ENV; i++)
		{
			tADSRT_init(&envs[i][v], 0.1f,1000.f,0.5f,1.0f,decayExpBuffer, DECAY_EXP_BUFFER_SIZE, &leaf);
			tADSRT_setLeakFactor(envs[i][v], ((1.0f - 0.1f) * 0.00005f) + 0.99995f);
		}

		//noise
		tTiltFilter_init(&noiseTilt[v], 1000.0f, &leaf);
		tVZFilterBell_init(&noiseBell1[v],1000.0f, 1.9f, 1.09f, &leaf);

		for (int i = 0; i < NUM_EFFECT; i++)
		{
			tCrusher_init(&bc[i][v],&leaf);
			tHighpass_init(&dcBlock1[i][v], 5.0f,&leaf);
			tHighpass_setSampleRate(dcBlock1[i][v], SAMPLE_RATE * OVERSAMPLE);
			tHighpass_init(&dcBlock2[i][v], 5.0f,&leaf);
			tHighpass_setSampleRate(dcBlock2[i][v], SAMPLE_RATE * OVERSAMPLE);
			tTiltFilter_init(&FXTilt[i][v],1000.0f, &leaf);
			tTiltFilter_setSampleRate(FXTilt[i][v], SAMPLE_RATE * OVERSAMPLE);
			tVZFilterBell_init(&bell1[i][v], 1000.0f, 1.9f, 1.0f, &leaf);
			tVZFilterBell_setSampleRate(bell1[i][v], SAMPLE_RATE * OVERSAMPLE);
			tCompressor_init(&comp[i][v], &leaf);
			tCompressor_setTables(comp[i][v], atoDbTable, dbtoATable, 0.00001f, 4.0f, -90.0f, 30.0f, ATODB_TABLE_SIZE, DBTOA_TABLE_SIZE);
			tCompressor_setSampleRate(comp[i][v], SAMPLE_RATE * OVERSAMPLE);
			tCycle_init(&mod1[i][v], &leaf);
			tCycle_setSampleRate(mod1[i][v], SAMPLE_RATE * OVERSAMPLE);
			tCycle_init(&mod2[i][v], &leaf);
			tCycle_setSampleRate(mod2[i][v], SAMPLE_RATE * OVERSAMPLE);
			tCycle_setFreq(mod1[i][v], 0.2f);
			tCycle_setFreq(mod2[i][v], 0.22222222222f);


	        tFeedbackLeveler_init(&feedbackControl[i][v], .99f, 0.01f, 0.125f, 0, &leaf);

			//filters
			tSVF_init(&FXlowpass[i][v], SVFTypeLowpass, 2000.f, 0.7f, &leaf);
			tSVF_setSampleRate(FXlowpass[i][v],SAMPLE_RATE * OVERSAMPLE);
			tSVF_init(&FXhighpass[i][v], SVFTypeHighpass, 2000.f, 0.7f, &leaf);
			tSVF_setSampleRate(FXhighpass[i][v],SAMPLE_RATE * OVERSAMPLE);
			tSVF_init(&FXbandpass[i][v], SVFTypeBandpass, 2000.f, 0.7f, &leaf);
			tSVF_setSampleRate(FXbandpass[i][v],SAMPLE_RATE * OVERSAMPLE);
			tDiodeFilter_init(&FXdiodeFilters[i][v], 2000.f, 1.0f, &leaf);
			tDiodeFilter_setSampleRate(FXdiodeFilters[i][v], SAMPLE_RATE * OVERSAMPLE);
			tVZFilterBell_init(&FXVZfilterPeak[i][v], 2000.f, 1.6f, 1.0f, &leaf);
			tVZFilterBell_setSampleRate(FXVZfilterPeak[i][v], SAMPLE_RATE * OVERSAMPLE);
			tVZFilterLS_init(&FXVZfilterLS[i][v], 2000.f, 0.6f, 1.0f, &leaf);
			tVZFilterLS_setSampleRate(FXVZfilterLS[i][v], SAMPLE_RATE * OVERSAMPLE);
			tVZFilterHS_init(&FXVZfilterHS[i][v], 2000.f, 0.6f, 1.0f, &leaf);
			tVZFilterHS_setSampleRate(FXVZfilterHS[i][v], SAMPLE_RATE * OVERSAMPLE);
			tVZFilterBR_init(&FXVZfilterBR[i][v], 2000.f, 1.0f, &leaf);
			tVZFilterBR_setSampleRate(FXVZfilterBR[i][v], SAMPLE_RATE * OVERSAMPLE);
			tLadderFilter_init(&FXLadderfilter[i][v], 2000.f, 1.0f, &leaf);
			tLadderFilter_setSampleRate(FXLadderfilter[i][v], SAMPLE_RATE * OVERSAMPLE);
		}

		for (int i = 0; i < MAX_NUM_MAPPINGS; i++)
		{
			tExpSmooth_init(&mapSmoothers[i][v], 0.0f, 0.2f, &leaf);
		}

		tSVF_LP_init(&finalLowpass[v], 19000.f, 0.2f, &leaf);
	}

}

void  audioFreeSynth()
{
	for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
	{
		//for (int i = 0; i < NUM_EFFECT; i++)
		{
			tTapeDelay_free(&tapeDelay[v]);
			tLinearDelay_free(&delay2[v]);
			tLinearDelay_free(&delay1[v]);
		}
	}
}

void  audioSwitchToSynth()
{

	for (int i = 0; i < 20; i++)
	{
		tExpSmooth_setFactor(knobSmoothers[i], 0.01f);
		//tExpSmooth_setValAndDest(knobSmoothers[i], string2Defaults[i]);
		knobFrozen[i] = 1;
	}

	for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
	{
		//for (int i = 0; i < NUM_EFFECT; i++)
		{
			leaf.clearOnAllocation = 1;
			tLinearDelay_initToPool(&delay1[v], 4000.0f*OVERSAMPLE, 4096*OVERSAMPLE, &mediumPool);
			tLinearDelay_initToPool(&delay2[v], 4000.0f*OVERSAMPLE, 4096*OVERSAMPLE, &mediumPool);
			tTapeDelay_initToPool(&tapeDelay[v], 15000.0f*OVERSAMPLE, 20000*OVERSAMPLE, &largePool);
		}
	}
	antiClickFade = 0.0f;

}

void __ATTR_ITCMRAM audioFrameSynth(uint16_t buffer_offset)
{
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	uint32_t tempCountFrame = DWT->CYCCNT;
	int32_t current_sample = 0;

	if (resetStringInputs)
	{
		for (int i = 0; i < numStringsThisBoard; i++)
		{
			//note off
			for (int v = 0; v < NUM_ENV; v++)
			{
				tADSRT_clear(envs[v][i]);
				previousStringInputs[i] = 0;
			}
		}
		resetStringInputs = 0;
		newPluck = 1;
	}
	if (newPluck)
	{
		for (int i = 0; i < numStringsThisBoard; i++)
		{
			if (((previousStringInputs[i] == 0) && (stringInputs[i] > 0)) || (retrigMode && (stringInputs[i] > 0)))
			{
				float amplitz = stringInputs[i] * 0.000015259021897f;
				stringOctave[i] = octave;
				retrigHappened = 1;
				//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

				for (int v = 0; v < NUM_ENV; v++)
				{
					if (envOn[v])
					{
						param* envParams = &params[ENVELOPE_PARAMS_OFFSET + v * EnvelopeParamsNum];
						float useVelocity = envParams[EnvelopeVelocity].realVal[i];
						float envVel = amplitz;
						//if (useVelocity == 0)
						envVel = (velocitySensitivity) + ((1.0f - velocitySensitivity) * amplitz);
						tADSRT_on(envs[v][i], envVel);
						voiceSounding = 1;
					}
				}
				for (int v = 0; v < NUM_LFOS; v++)
				{
					if (lfoOn[v])
					{
						param* lfoParams = &params[LFO_PARAMS_OFFSET + v * LFOParamsNum];
						float noteOnSync = lfoParams[LFOSync].realVal[i];
						if (noteOnSync > 0.5f)
						{
							lfoParams[LFOPhase].setParam(lfoParams[LFOPhase].realVal[i], v, i);
						}
					}
				}
				//sample and hold noise
				sourceValues[RANDOM_SOURCE_OFFSET][i] = randomNumber(); // scale between zero and one
				sourceValues[VELOCITY_SOURCE_OFFSET][i] = amplitz;

			}
			else if ((previousStringInputs[i] > 0) && (stringInputs[i] == 0))
			{
				//note off
				for (int v = 0; v < NUM_ENV; v++)
				{
					if (envOn[v])
					{
						tADSRT_off(envs[v][i]);
					}
				}
			}
			previousStringInputs[i] = stringInputs[i];
		}
		newPluck = 0;
	}
	//mono operation, no need to compute right channel. Also for loop iterating by 2 instead of 1 to avoid if statement.
	for (int i = 0; i < HALF_BUFFER_SIZE; i+=2)
	{
		current_sample = (int32_t)(audioTickSynth() * TWO_TO_23);
		current_sample = LEAF_clip((-1 * TWO_TO_23) + 1, current_sample, TWO_TO_23 - 1);

		audioOutBuffer[buffer_offset + i] = current_sample;
		audioOutBuffer[buffer_offset + i + 1] = current_sample;
	}


/*
	if (switchStrings)
	{
		switchStringModel(switchStrings);
	}
	switchStrings = 0;
*/
	uint32_t timeFrame = DWT->CYCCNT - tempCountFrame;
		frameLoadPercentage = (float)timeFrame * frameMult;
		if (frameLoadPercentage > .99f)
		{
			frameLoadOverCount++;
			if (frameLoadOverCount > 3)
			{

				if (overSampled == 1)
				{
					overSampled = 0;
					changeOversampling(overSampled);
				}
				else if (oscToTick > 0)
				{
					oscToTick--;
				}
			}
		}
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
}

volatile float outVol1 = 0.0f;
volatile float outVol2 = 0.0f;
volatile uint32_t timeVolumeLookup = 0;
volatile uint32_t timeVolumePoly = 0;



volatile uint32_t nanChecker = 0;

float __ATTR_ITCMRAM audioTickSynth(void)
{
	uint32_t tempCountTick = DWT->CYCCNT;
	float masterSample = 0.0f;

	antiClickFade = 0.001f + 0.999f * antiClickFade;
	//run mapping ticks for all strings
	uint32_t tempCountMap = DWT->CYCCNT;
	tickMappings();
	timeMap = DWT->CYCCNT - tempCountMap;

	uint32_t tempSmoothing = DWT->CYCCNT;
	float volumeSmoothed = tExpSmooth_tick(volumeSmoother);

	for (int i = 0; i < 12; i++)
	{
		if (knobTicked[i])
		{
			knobScaled[i] = tExpSmooth_tick(knobSmoothers[i]);
			for (int v = 0; v < numStringsThisBoard; v++)
			{
				sourceValues[MACRO_SOURCE_OFFSET + i][v] = knobScaled[i];
			}
		}
	}
	tExpSmooth_setDest(knobSmoothers[13], (float)ADC_values[5] * INV_TWO_TO_16);
	tExpSmooth_setDest(knobSmoothers[12], (float)ADC_values[4] * INV_TWO_TO_16);
	float finalVolumeKnob = tExpSmooth_tick(knobSmoothers[13]);
	velocitySensitivity = tExpSmooth_tick(knobSmoothers[12]);

	for (int v = 0; v < numStringsThisBoard; v++)
	{
		sourceValues[EXPRESSION_PEDAL_SOURCE_OFFSET][v] = volumePedal;
	}
	timeSmoothing = DWT->CYCCNT - tempSmoothing;

	float note[numStringsThisBoard];
	uint32_t tempPerStringTick = DWT->CYCCNT;
	for (int v = 0; v < numStringsThisBoard; v++)
	{
		float sample = 0.0f;

		uint32_t tempCountGettingNote = DWT->CYCCNT;
		float myBend = ((float)bendData) * 0.00006104f; //now it's 0 to 1
		myBend = myBend - 0.5f; //now it's -0.5 to 0.5
		myBend = myBend * 4.0f; //now it's -2 to 2

		note[v] = stringMIDIPitches[v] + stringOctave[v] + transpose + myBend;

		if (note[v] < 0.0f)
		{
			note[v] = 0.0f;
		}
		if (note[v] > 127.0f)
		{
			note[v] = 127.0f;
		}

		sourceValues[MIDI_KEY_SOURCE_OFFSET][v] = (note[v] - midiKeySubtractor) * midiKeyDivisor;

		timeGettingNote = DWT->CYCCNT - tempCountGettingNote;

		uint32_t tempCountEnv = DWT->CYCCNT;
		envelope_tick(v);
		timeEnv = DWT->CYCCNT - tempCountEnv;

		uint32_t tempCountLFO = DWT->CYCCNT;
		lfo_tick(v);
		timeLFO = DWT->CYCCNT - tempCountLFO;

		uint32_t tempCountOsc = DWT->CYCCNT;
		oscillator_tick(note[v], v);
		timeOsc = DWT->CYCCNT - tempCountOsc;


		//sample = oscOuts[0][0][v] *amplitude[v];
		//sample = tSVF_LP_tick(finalLowpass[v], sample) * masterVolFromBrainForSynth;
#if 1
		uint32_t tempCountNoise = DWT->CYCCNT;
		if (noiseOn)
		{
			noise_tick(v);
		}
		timeNoise = DWT->CYCCNT - tempCountNoise;

		float filterSamps[2] = {0.0f, 0.0f};
		for (int i = 0; i < oscToTick; i++)
		{
			filterSamps[0] += oscOuts[0][i][v];
			filterSamps[1] += oscOuts[1][i][v];
		}
		if isnan(filterSamps[0])
		{
			nanChecker++;
		}
		if isnan(filterSamps[1])
		{
			nanChecker++;
		}
		filterSamps[0] += noiseOuts[0][v];
		filterSamps[1] += noiseOuts[1][v];

		if isnan(filterSamps[0])
		{
			nanChecker++;
		}
		if isnan(filterSamps[1])
		{
			nanChecker++;
		}
		uint32_t tempCountFilt = DWT->CYCCNT;
		sample = filter_tick(&filterSamps[0], note[v], v);
		timeFilt = DWT->CYCCNT - tempCountFilt;
		if isnan(sample)
		{
			nanChecker++;
		}

		if (fxPre)
		{
			sample *= amplitude[v];
		}
		if isnan(sample)
		{
			nanChecker++;
		}
		uint32_t tempCountOS = DWT->CYCCNT;

		if (overSampled)
		{

			//oversample for non-linear effects (distortion, etc)
			//using the arm interpolation and decimation cuts 100 cycles off of the processing
			//arm_fir_interpolate_f32(&osI[v], &sample, (float*)&oversamplerArray, 1);
			tOversampler_upsample(os[v], sample, oversamplerArray);


			for (int i = 0; i < 4; i++)
			{
				if (effectsActive[i])
				{
					for (int j = 0; j < OVERSAMPLE; j++)
					{
						float dry = oversamplerArray[j]; //store the dry value to mix later
						if isnan(dry)
						{
							nanChecker++;
						}
						oversamplerArray[j] = effectTick[i](dry, i, v); //run the effect
						if isnan(oversamplerArray[j])
						{
							nanChecker++;
						}
						oversamplerArray[j] = ((1.0f - fxMix[i][v]) * dry) + (fxMix[i][v] * oversamplerArray[j]); //mix in dry/wet at the "mix" amount
						oversamplerArray[j] *= fxPostGain[i][v]; //apply postgain

					}
				}
			}
			for (int j = 0; j < OVERSAMPLE; j++)
			{
				oversamplerArray[j] = tanhf(oversamplerArray[j]);
#if 0
				if (oversamplerArray[j] > 0.999999f)
				{
					oversamplerArray[j] = 0.999999f;
				}
				else if (oversamplerArray[j] < -0.999999f)
				{
					oversamplerArray[j] = -0.999999f;
				}
#endif
			}
			//downsample to get back to normal sample rate
			//arm_fir_decimate_f32(&osD[v], (float*)&oversamplerArray, &sample, 2);
			sample = tOversampler_downsample(os[v], oversamplerArray);
		}
		else
		{
			for (int i = 0; i < NUM_EFFECT; i++)
			{
				if (effectsActive[i])
				{
					float dry = sample;
					sample = effectTick[i](sample, i, v); //run the effect
					sample =((1.0f - fxMix[i][v]) * dry) + (fxMix[i][v] * sample);
					sample *= fxPostGain[i][v];

				}
			}

			if (sample > 0.999999f)
			{
				sample = 0.999999f;
			}
			else if (sample < -0.999999f)
			{
				sample = -0.999999f;
			}
		}

		timeOS = DWT->CYCCNT - tempCountOS;

		if (!fxPre)
		{
			sample *= amplitude[v];
		}
		if isnan(sample)
		{
			nanChecker++;
		}
		sample = tSVF_LP_tick(finalLowpass[v], sample) * masterVolFromBrainForSynth;
		if isnan(sample)
		{
			nanChecker++;
		}
#endif
		masterSample += sample * finalMaster[v];
		if isnan(sample)
		{
			nanChecker++;
		}
	}
	//uint32_t tempVolumePoly = DWT->CYCCNT;
	timePerStringTick = DWT->CYCCNT - tempPerStringTick;
/*
	uint32_t tempVolumeLookup = DWT->CYCCNT;
	float volIdx = LEAF_clip(47.0f, ((volumeSmoothed * 80.0f) + 47.0f), 127.0f);
	int volIdxInt = (int) volIdx;
	float alpha = volIdx-volIdxInt;
	int volIdxIntPlus = (volIdxInt + 1) & 127;
	float omAlpha = 1.0f - alpha;
	float outVol = volumeAmps128[volIdxInt] * omAlpha;
	outVol += volumeAmps128[volIdxIntPlus] * alpha;

	timeVolumeLookup = DWT->CYCCNT - tempVolumeLookup;
	outVol1 = outVol;
*/

	//float outVol = 0.0265625f - (0.2467348f * volumeSmoothed) + (1.253049f * volumeSmoothed * volumeSmoothed);

	//float outVol = 0.008315613f + 0.3774075f*volumeSmoothed - 1.785774f*volumeSmoothed*volumeSmoothed + 4.218241f*volumeSmoothed*volumeSmoothed*volumeSmoothed - 0.8576009f*volumeSmoothed*volumeSmoothed*volumeSmoothed*volumeSmoothed - 0.9656285f*volumeSmoothed*volumeSmoothed*volumeSmoothed*volumeSmoothed*volumeSmoothed;
	//float outVol = 0.006721744f + 0.4720157f*volumeSmoothed - 2.542849f*volumeSmoothed*volumeSmoothed + 6.332339f*volumeSmoothed*volumeSmoothed*volumeSmoothed - 3.271672f*volumeSmoothed*volumeSmoothed*volumeSmoothed*volumeSmoothed;


	//if (pedalControlsMaster)
	{
	//	masterSample *= outVol;
	}
	//timeVolumePoly = DWT->CYCCNT - tempVolumePoly;

	timeTick = DWT->CYCCNT - tempCountTick;

	float tempOut = masterSample * audioMasterLevel * 0.98f * antiClickFade;
	tempOut *= finalVolumeKnob;
	if (tempOut > 0.99999f)
	{
		tempOut = 0.99999f;
	}
	else if (tempOut < -0.99999f)
	{
		tempOut = -0.99999f;
	}
	return tempOut;
}



void changeOversampling(uint32_t newOS)
{
	if (newOS != prevOversample)
	{
		uint32_t osMult = 1;
		if (newOS)
		{
			osMult = OVERSAMPLE * SAMPLE_RATE;
		}
		else
		{
			osMult = SAMPLE_RATE;
		}
		//uint32_t osMult = (newOS + 1) * SAMPLE_RATE; // change os to 1-2 range
		for (int v = 0; v < numStringsThisBoard; v++)
		{
			for (int i = 0; i < NUM_EFFECT; i++)
			{
				tHighpass_setSampleRate(dcBlock1[i][v], osMult);
				tHighpass_setSampleRate(dcBlock2[i][v], osMult);
				tTiltFilter_setSampleRate(FXTilt[i][v], osMult);
				//tVZFilterLS_setSampleRate(&shelf1[i][v], osMult);
				//tVZFilterLS_setFreqFast(&shelf1[i][v], shelf1[i][v]->cutoffMIDI);
				//tVZFilterHS_setSampleRate(&shelf2[i][v], osMult);
				//tVZFilterHS_setFreqFast(&shelf2[i][v], shelf2[i][v]->cutoffMIDI);
				tVZFilterBell_setSampleRate(bell1[i][v], osMult);
				tVZFilterBell_setFreqFast(bell1[i][v], bell1[i][v]->cutoffMIDI);
				tCompressor_setSampleRate(comp[i][v], osMult);
				tCycle_setSampleRate(mod1[i][v], osMult);
				tCycle_setSampleRate(mod2[i][v], osMult);
				tSVF_setSampleRate(FXlowpass[i][v],osMult);
				tSVF_setFreqFast(FXlowpass[i][v], FXlowpass[i][v]->cutoffMIDI);
				tSVF_setSampleRate(FXhighpass[i][v],osMult);
				tSVF_setFreqFast(FXhighpass[i][v], FXhighpass[i][v]->cutoffMIDI);
				tSVF_setSampleRate(FXbandpass[i][v],osMult);
				tSVF_setFreqFast(FXbandpass[i][v], FXbandpass[i][v]->cutoffMIDI);
				tDiodeFilter_setSampleRate(FXdiodeFilters[i][v], osMult);
				tDiodeFilter_setFreqFast(FXdiodeFilters[i][v], FXdiodeFilters[i][v]->cutoffMIDI);
				tVZFilterBell_setSampleRate(FXVZfilterPeak[i][v], osMult);
				tVZFilterBell_setFreqFast(FXVZfilterPeak[i][v], FXVZfilterPeak[i][v]->cutoffMIDI);
				tVZFilterLS_setSampleRate(FXVZfilterLS[i][v], osMult);
				tVZFilterLS_setFreqFast(FXVZfilterLS[i][v], FXVZfilterLS[i][v]->cutoffMIDI);
				tVZFilterHS_setSampleRate(FXVZfilterHS[i][v], osMult);
				tVZFilterHS_setFreqFast(FXVZfilterHS[i][v], FXVZfilterHS[i][v]->cutoffMIDI);
				tVZFilterBR_setSampleRate(FXVZfilterBR[i][v], osMult);
				tVZFilterBR_setFreqFast(FXVZfilterBR[i][v], FXVZfilterBR[i][v]->cutoffMIDI);
				tLadderFilter_setSampleRate(FXLadderfilter[i][v], osMult);
				tLadderFilter_setFreqFast(FXLadderfilter[i][v], FXLadderfilter[i][v]->cutoffMIDI);
			}
		}
	}
	prevOversample = newOS;
}








void __ATTR_ITCMRAM oscillator_tick(float note, int string)
{
	for (int i = 0; i < NUM_OSC; i++)
	{
		oscOuts[0][i][string] = 0.0f;
		oscOuts[1][i][string] = 0.0f;
	}
	//to avoid big glissandos when intentionally jumping like an octave switch
	//if ((note > prevNote + 3.0f) || (note < prevNote - 3.0f))
	if (retrigHappened)
	{
		tExpSmooth_setValAndDest(pitchSmoother, note);
		retrigHappened = 0;
	}
	else
	{
		tExpSmooth_setDest(pitchSmoother, note);
	}
	prevNote = note;
	float mainFreq = tExpSmooth_tick(pitchSmoother);
	for (int osc = 0; osc < oscToTick; osc++)
	{
		if (oscOn[osc])
		{
			param* oscParams = &params[OSC_PARAMS_OFFSET + osc * OscParamsNum];

			float fine = oscParams[OscFine].realVal[string];
			float freqOffset= oscParams[OscFreq].realVal[string];
			float shape = oscParams[OscShape].realVal[string];
			float amp = oscParams[OscAmp].realVal[string];
			float filterSend = oscParams[OscFilterSend].realVal[string];
			//int sync = oscParams[OscisSync].realVal[string] > 0.5f; // probably faster than previous roundf version but haven't tested
			float tempMIDI = (mainFreq + (fine*0.01f)) + midiAdd[osc][string];
			float finalFreq = (mtofTableLookup(tempMIDI) * freqMult[osc][string]) + freqOffset;
			float sample = 0.0f;


			//shapeTick[osc](&sample, osc, finalFreq, shape, sync, string);
			shapeTick[osc](&sample, osc, finalFreq, shape, 0, string);

			sample *= amp;

			//sourceValues[OSC_SOURCE_OFFSET + osc] = sample; // the define of zero may be wasteful
			sourceValues[osc][string] = sample;

			sample *= oscAmpMult; // divide down gain if more than one oscillator is sounding (computed at preset load)

			oscOuts[0][osc][string] = sample * (filterSend) * oscParams[OscEnabled].realVal[string];
			oscOuts[1][osc][string] = sample * (1.0f - filterSend) * oscParams[OscEnabled].realVal[string];
		}
	}
}


void __ATTR_ITCMRAM  sawSquareTick(float* sample, int v, float freq, float shape, int sync, int string)
{
	tPBSawSquare_setFreq(sawPaired[v][string], freq);
	tPBSawSquare_setShape(sawPaired[v][string], shape);
    //if (sync)
    {
    	//tMBSawPulse_sync(&sawPaired[v][string], sourceValues[syncMap[OSC_SOURCE_OFFSET + v]][string]);

    }
    *sample += tPBSawSquare_tick(sawPaired[v][string]);
}

void __ATTR_ITCMRAM  sineTriTick(float* sample, int v, float freq, float shape, int sync, int string)
{
    tPBSineTriangle_setFreq(sinePaired[v][string], freq);
    tPBSineTriangle_setShape(sinePaired[v][string],shape);
   // if (sync)
   // {
   // 	tPBSineTriangle_sync(&sinePaired[v][string], sourceValues[syncMap[OSC_SOURCE_OFFSET + v]][string]);
   // }
    *sample += tPBSineTriangle_tick(sinePaired[v][string]);
}

void __ATTR_ITCMRAM  sawTick(float* sample, int v, float freq, float shape, int sync, int string)
{
	tPBSaw_setFreq(saw[v][string], freq);
    //if (sync)
	//{
	//	tPBSaw_sync(&saw[v][string], sourceValues[syncMap[OSC_SOURCE_OFFSET + v]][string]);
	//}
    *sample += tPBSaw_tick(saw[v][string]);
}

void __ATTR_ITCMRAM  pulseTick(float* sample, int v, float freq, float shape, int sync, int string)
{
    tPBPulse_setFreq(pulse[v][string], freq);
    tPBPulse_setWidth(pulse[v][string], shape);
    //if (sync)
	//{
	//	tPBPulse_sync(&pulse[v][string], sourceValues[syncMap[OSC_SOURCE_OFFSET + v]][string]);
	//}
    *sample += tPBPulse_tick(pulse[v][string]);
}

void __ATTR_ITCMRAM  sineTick(float* sample, int v, float freq, float shape, int sync, int string)
{
    tCycle_setFreq(sine[v][string], freq);
    *sample += tCycle_tick(sine[v][string]);
}

void __ATTR_ITCMRAM  triTick(float* sample, int v, float freq, float shape, int sync, int string)
{
    tPBTriangle_setFreq(tri[v][string], freq);
    tPBTriangle_setSkew(tri[v][string], shape);
    //if (sync)
	//{
	//	tMBTriangle_sync(&tri[v][string], sourceValues[syncMap[OSC_SOURCE_OFFSET + v]][string]);
	//}
    *sample += tPBTriangle_tick(tri[v][string]);
}

void __ATTR_ITCMRAM  userTick(float* sample, int v, float freq, float shape, int sync, int string)
{
    //tWaveOscS_setFreq(&wave[v], freq);
    //tWaveOscS_setIndex(&wave[v], shape);
    //*sample += tWaveOscS_tick(&wave[v]);
}



float __ATTR_ITCMRAM filter_tick(float* samples, float note, int string)
{
	float cutoff[2] = {0.0f, 0.0f};
	uint8_t enabledFilt[2] = {0,0};
	for (int f = 0; f < NUM_FILT; f++)
	{
		param* filtParams = &params[FILTER_PARAMS_OFFSET + f * FilterParamsNum];
		float enabled = filtParams[FilterEnabled].realVal[string];
		enabledFilt[f] = (enabled > 0.5f);
		if (!enabledFilt[f]) continue;

		float MIDIcutoff = filtParams[FilterCutoff].realVal[string];
		float keyFollow = filtParams[FilterKeyFollow].realVal[string];

	    cutoff[f]  = MIDIcutoff + (note  * keyFollow);

		//smoothing may not be necessary
		//tExpSmooth_setDest(&filterCutoffSmoother[f][string], cutoff[f]);
		//cutoff[f] = tExpSmooth_tick(&filterCutoffSmoother[f][string]);
	}

	float  sp = params[FilterSeriesParallelMix].realVal[string];

	if (enabledFilt[0])
	{
		filterTick[0](&samples[0], 0, cutoff[0], string);
	}
	float sendToFilter2 = samples[0] * (1.0f - sp);
	samples[1] += sendToFilter2;
	//compute what gets sent to the second filter
	if (enabledFilt[1])
	{
		filterTick[1](&samples[1], 1, cutoff[1], string);
	}
	return samples[1] + (samples[0] * sp);
}


void __ATTR_ITCMRAM  lowpassTick(float* sample, int v, float cutoff, int string)
{
	tSVF_setFreqFast(lowpass[v][string], cutoff);
	*sample = tSVF_tickLP(lowpass[v][string], *sample);
    *sample *= filterGain[v][string];
}

void __ATTR_ITCMRAM  highpassTick(float* sample, int v, float cutoff, int string)
{
	tSVF_setFreqFast(highpass[v][string], cutoff);
	*sample = tSVF_tickHP(highpass[v][string], *sample);
    *sample *= filterGain[v][string];
}

void __ATTR_ITCMRAM  bandpassTick(float* sample, int v, float cutoff, int string)
{
	tSVF_setFreqFast(bandpass[v][string], cutoff);
	*sample = tSVF_tickBP(bandpass[v][string], *sample);
    *sample *= filterGain[v][string];
}

void __ATTR_ITCMRAM  diodeLowpassTick(float* sample, int v, float cutoff, int string)
{
	tDiodeFilter_setFreqFast(diodeFilters[v][string], cutoff);
	*sample = tDiodeFilter_tickEfficient(diodeFilters[v][string], *sample);
    *sample *= filterGain[v][string];
}

void __ATTR_ITCMRAM  VZpeakTick(float* sample, int v, float cutoff, int string)
{
	tVZFilterBell_setFreqFast(VZfilterPeak[v][string], cutoff);
	*sample = tVZFilterBell_tick(VZfilterPeak[v][string], *sample);
}

void __ATTR_ITCMRAM  VZlowshelfTick(float* sample, int v, float cutoff, int string)
{
	tVZFilterLS_setFreqFast(VZfilterLS[v][string], cutoff);
	*sample = tVZFilterLS_tick(VZfilterLS[v][string], *sample);
}
void __ATTR_ITCMRAM  VZhighshelfTick(float* sample, int v, float cutoff, int string)
{
	tVZFilterHS_setFreqFast(VZfilterHS[v][string], cutoff);
	*sample = tVZFilterHS_tick(VZfilterHS[v][string], *sample);
}
void __ATTR_ITCMRAM  VZbandrejectTick(float* sample, int v, float cutoff, int string)
{
	tVZFilterBR_setFreqFast(VZfilterBR[v][string], cutoff);
	*sample = tVZFilterBR_tick(VZfilterBR[v][string], *sample);
    *sample *= filterGain[v][string];
}

void __ATTR_ITCMRAM  LadderLowpassTick(float* sample, int v, float cutoff, int string)
{
	tLadderFilter_setFreqFast(Ladderfilter[v][string], cutoff);
	*sample = tLadderFilter_tick(Ladderfilter[v][string], *sample);
    *sample *= filterGain[v][string];
}


float midiAdd[NUM_OSC][NUM_STRINGS_PER_BOARD];

void __ATTR_ITCMRAM setFreqMultPitch(float pitch, int osc, int string)
{
	pitch *= 24.0f;
	if (params[OSC_PARAMS_OFFSET + osc * OscParamsNum + OscisStepped].realVal[string] > 0.5f) ///check for value of 1 since this is a float
	{
		pitch = roundf(pitch);
	}
	midiAdd[osc][string] = pitch;
}

void __ATTR_ITCMRAM setFreqMultHarm(float harm, int osc, int string)
{
	harm *= 15.0f;
	if (params[OSC_PARAMS_OFFSET + osc * OscParamsNum + OscisStepped].realVal[string] > 0.5f) ///check for value of 1 since this is a float
	{
		harm = roundf(harm);
	}

	if (harm >= 0)
	{
		freqMult[osc][string] = (harm + 1);
	}
	else
	{
		freqMult[osc][string] = (1.0f / fabsf((harm - 1)));
	}
}




void __ATTR_ITCMRAM  lowpassSetQ(float q, int v, int string)
{
    tSVF_setQ(lowpass[v][string], q);
}

void __ATTR_ITCMRAM  highpassSetQ(float q, int v, int string)
{
    tSVF_setQ(highpass[v][string], q);
}

void __ATTR_ITCMRAM  bandpassSetQ(float q, int v, int string)
{
    tSVF_setQ(bandpass[v][string], q);
}

void __ATTR_ITCMRAM  diodeLowpassSetQ(float q, int v, int string)
{
	tDiodeFilter_setQ(diodeFilters[v][string], q);
}

void __ATTR_ITCMRAM  VZpeakSetQ(float q, int v, int string)
{
	tVZFilterBell_setBandwidth(VZfilterPeak[v][string], q*20.0f);
}

void __ATTR_ITCMRAM  VZlowshelfSetQ(float q, int v, int string)
{
	tVZFilterLS_setResonance(VZfilterLS[v][string], q);
}

void __ATTR_ITCMRAM  VZhighshelfSetQ(float q, int v, int string)
{
	tVZFilterHS_setResonance(VZfilterHS[v][string], q);
}

void __ATTR_ITCMRAM  VZbandrejectSetQ(float q, int v, int string)
{
	tVZFilterBR_setResonance(VZfilterBR[v][string], q);
}

void __ATTR_ITCMRAM  LadderLowpassSetQ(float q, int v, int string)
{
	tLadderFilter_setQ(Ladderfilter[v][string], q);
}

void __ATTR_ITCMRAM  lowpassSetGain(float gain, int v, int string)
{
    filterGain[v][string] = dbToATableLookup((gain * 24.0f) - 12.0f);
}

void __ATTR_ITCMRAM  highpassSetGain(float gain, int v, int string)
{
	filterGain[v][string] = dbToATableLookup((gain * 24.0f) - 12.0f);
}

void __ATTR_ITCMRAM  bandpassSetGain(float gain, int v, int string)
{
	filterGain[v][string] = dbToATableLookup((gain * 24.0f) - 12.0f);
}

void __ATTR_ITCMRAM  diodeLowpassSetGain(float gain, int v, int string)
{
	filterGain[v][string] = dbToATableLookup((gain * 24.0f) - 12.0f);
}

void __ATTR_ITCMRAM  VZpeakSetGain(float gain, int v, int string)
{
	 tVZFilterBell_setGain(VZfilterPeak[v][string], dbToATableLookup((gain * 50.f) - 25.f));
}

void __ATTR_ITCMRAM  VZlowshelfSetGain(float gain, int v, int string)
{
	tVZFilterLS_setGain(VZfilterLS[v][string], dbToATableLookup((gain * 50.f) - 25.f));
}

void __ATTR_ITCMRAM  VZhighshelfSetGain(float gain, int v, int string)
{
	tVZFilterHS_setGain(VZfilterHS[v][string], dbToATableLookup((gain * 50.f) - 25.f));
}

void  __ATTR_ITCMRAM VZbandrejectSetGain(float gain, int v, int string)
{
	filterGain[v][string] = dbToATableLookup((gain * 24.0f) - 12.0f);
}

void  __ATTR_ITCMRAM  LadderLowpassSetGain(float gain, int v, int string)
{
	filterGain[v][string] = dbToATableLookup((gain * 24.0f) - 12.0f);
}


void __ATTR_ITCMRAM envelope_tick(int string)
{
	for (int v = 0; v < NUM_ENV; v++)
	{
		if (envOn[v])
		{
			sourceValues[ENV_SOURCE_OFFSET + v][string] = tADSRT_tick(envs[v][string]);
		}
	}
}


void __ATTR_ITCMRAM lfo_tick(int string)
{
	for (int i = 0; i < NUM_LFOS; i++)
	{
		if (lfoOn[i])
		{
			float sample = 0.0f;
			lfoShapeTick[i](&sample,i, string);
			sourceValues[LFO_SOURCE_OFFSET + i][string] = sample;
		}
	}
}


void  __ATTR_ITCMRAM  setEnvelopeAttack(float a, int v, int string)
{
	a = a + 0.001f;
	tADSRT_setAttack(envs[v][string], a);
}

void  __ATTR_ITCMRAM  setEnvelopeDecay(float d, int v, int string)
{
	d = d + 0.001f;
	tADSRT_setDecay(envs[v][string], d);
}

void  __ATTR_ITCMRAM  setEnvelopeSustain(float s, int v, int string)
{
	tADSRT_setSustain(envs[v][string], s);
}

void  __ATTR_ITCMRAM  setEnvelopeRelease(float r, int v, int string)
{
	r = r + 0.001f;
	tADSRT_setRelease(envs[v][string], r);
}

void  __ATTR_ITCMRAM  setEnvelopeLeak(float leak, int v, int string)
{
	tADSRT_setLeakFactor(envs[v][string], 0.99995f + 0.00005f*(1.f-leak));
}

void  __ATTR_ITCMRAM  setAmp(float amp, int v, int string)
{
	amplitude[string] = amp;
}

void  __ATTR_ITCMRAM  setMaster(float amp,  int v, int string)
{
	finalMaster[string] = amp;
}

void  __ATTR_ITCMRAM  setTranspose(float in, int v, int string)
{
	transpose = in;
}

void  __ATTR_ITCMRAM  setPitchBendRange(float in, int v, int string)
{
	bendRangeMultiplier = 1.0f / (16383.0f / (in * 2.0f));
}

void  __ATTR_ITCMRAM  setFinalLowpass(float in, int v, int string)
{
	tSVF_LP_setFreqFast(finalLowpass[string], in);
}


void __ATTR_ITCMRAM tickMappings(void)
{
	//interruptChecker = 0;
	for (int i = 0; i < numMappings; i++)
	{
		if (mappings[i].destNumber != 255)
		{
			for (int v = 0; v < numStringsThisBoard; v++)
			{
				float unsmoothedValue = 0.0f;
				float smoothedValue = 0.0f;
				for (int j = 0; j < 3; j++)
				{
					if (mappings[i].hookActive[j])
					{
						float sum = *mappings[i].sourceValPtr[j][v] * mappings[i].amount[j] * *mappings[i].scalarSourceValPtr[j][v];
						if (mappings[i].sourceSmoothed[j])
						{
							smoothedValue += sum;
						}
						else
						{
							unsmoothedValue += sum;
						}
					}
				}
				//sources are now summed - let's add the initial value
				smoothedValue += mappings[i].dest->zeroToOneVal[v];

				tExpSmooth_setDest(mapSmoothers[i][v], smoothedValue);
				smoothedValue = tExpSmooth_tick(mapSmoothers[i][v]);
				float finalVal = unsmoothedValue + smoothedValue;

				//now scale the value with the correct scaling function
				mappings[i].dest->realVal[v] = mappings[i].dest->scaleFunc(finalVal);
				//mappings[i].dest->realVal[v] = mappings[i].dest->scaleFunc(unsmoothedValue + mappings[i].dest->zeroToOneVal[v]);
				//and pop that value where it belongs by setting the actual parameter
				mappings[i].dest->setParam(mappings[i].dest->realVal[v], mappings[i].dest->objectNumber, v);
			}
		}
	}


}




void __ATTR_ITCMRAM lfoSawSquareTick(float* sample, int v, int string)
{
	*sample = tSawSquareLFO_tick(lfoSawSquare[v][string]);
}

void __ATTR_ITCMRAM lfoSineTriTick(float* sample, int v, int string)
{
	*sample = tSineTriLFO_tick(lfoSineTri[v][string]);
}

void __ATTR_ITCMRAM lfoSineTick(float* sample, int v, int string)
{
    *sample = tCycle_tick(lfoSine[v][string]);
}

void __ATTR_ITCMRAM lfoTriTick(float* sample, int v, int string)
{
    *sample = tTriLFO_tick(lfoTri[v][string]);
}
void __ATTR_ITCMRAM lfoSawTick(float* sample, int v, int string)
{
    *sample = (tIntPhasor_tick(lfoSaw[v][string]) * 2.0f) - 1.0f;
}

void __ATTR_ITCMRAM lfoPulseTick(float* sample, int v, int string)
{
    *sample = tSquareLFO_tick(lfoPulse[v][string]);
}

void __ATTR_ITCMRAM lfoSawSquareSetRate(float r, int v, int string)
{
	tSawSquareLFO_setFreq(lfoSawSquare[v][string],r);
}

void __ATTR_ITCMRAM lfoSineTriSetRate(float r, int v, int string)
{
	tSineTriLFO_setFreq(lfoSineTri[v][string],r);
}
void __ATTR_ITCMRAM lfoSineSetRate(float r, int v, int string)
{
	tCycle_setFreq(lfoSine[v][string], r);
}
void __ATTR_ITCMRAM lfoTriSetRate(float r, int v, int string)
{
	tTriLFO_setFreq(lfoTri[v][string], r);
}
void __ATTR_ITCMRAM lfoSawSetRate(float r, int v, int string)
{
	tIntPhasor_setFreq(lfoSaw[v][string], r);
}
void __ATTR_ITCMRAM lfoPulseSetRate(float r, int v, int string)
{
	 tSquareLFO_setFreq(lfoPulse[v][string], r);
}


void __ATTR_ITCMRAM lfoSawSquareSetPhase(float p, int v, int string)
{
	tSawSquareLFO_setPhase(lfoSawSquare[v][string],p);
}
void __ATTR_ITCMRAM lfoSineTriSetPhase(float p, int v, int string)
{
	tSineTriLFO_setPhase(lfoSineTri[v][string], p);
}
void __ATTR_ITCMRAM lfoSineSetPhase(float p, int v, int string)
{
	tCycle_setPhase(lfoSine[v][string],p);
}
void __ATTR_ITCMRAM lfoTriSetPhase(float p, int v, int string)
{
	tTriLFO_setPhase(lfoTri[v][string],p);
}
void __ATTR_ITCMRAM lfoSawSetPhase(float p, int v, int string)
{
	tIntPhasor_setPhase(lfoSaw[v][string], p);
}
void __ATTR_ITCMRAM lfoPulseSetPhase(float p, int v, int string)
{
	tSquareLFO_setPhase(lfoPulse[v][string], p);
}


void __ATTR_ITCMRAM lfoSawSquareSetShape(float s, int v, int string)
{
	tSawSquareLFO_setShape(lfoSawSquare[v][string],s);
}
void __ATTR_ITCMRAM lfoSineTriSetShape(float s, int v, int string)
{
	tSineTriLFO_setShape(lfoSineTri[v][string],s);
}
void __ATTR_ITCMRAM lfoSineSetShape(float s, int v, int string)
{
	//none
}
void __ATTR_ITCMRAM lfoTriSetShape(float s, int v, int string)
{
	//none
}
void __ATTR_ITCMRAM lfoSawSetShape(float s, int v, int string)
{
	//none
}
void __ATTR_ITCMRAM lfoPulseSetShape(float s, int v, int string)
{
	tSquareLFO_setPulseWidth(lfoPulse[v][string], s);
}




void __ATTR_ITCMRAM  param1Linear(float value, int v, int string)
{
	param1[v][string] = value;
}

void __ATTR_ITCMRAM  clipperGainSet(float value, int v, int string)
{
	param1[v][string] = dbToATableLookup(value * 24.0f);
}
void __ATTR_ITCMRAM  wavefolderParam1(float value, int v, int string)
{
	param1[v][string] = dbToATableLookup(value * 12.0f);
}
void __ATTR_ITCMRAM  wavefolderParam3(float value, int v, int string)
{
	//value = (value * 0.99f) + 0.00f; //avoid zero
	invCurFB[v][string] = (1.0f / (1.0f + value));
	param3[v][string] = value;
}

void __ATTR_ITCMRAM  tiltParam1(float value, int v, int string)
{
	tTiltFilter_setTilt(FXTilt[v][string], value*12.0f - 6.0f);
}

void __ATTR_ITCMRAM  tiltParam2(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
	value = LEAF_clip(0.0f, (value-16.0f) * 35.929824561403509f, 4095.0f);
	tVZFilterBell_setFreq(bell1[v][string], value);
}
void __ATTR_ITCMRAM  tiltParam3(float value, int v, int string)
{
	tVZFilterBell_setBandwidth(bell1[v][string], (value +1.0f)*6.0f);//TODO: make this match plugin
}
void __ATTR_ITCMRAM  tiltParam4(float value, int v, int string)
{
	tVZFilterBell_setGain(bell1[v][string], dbToATableLookup((value * 34.0f) - 17.0f));
}

void __ATTR_ITCMRAM  compressorParam1(float value, int v, int string)
{
	comp[v][string]->T = value*-24.0f;
}
void __ATTR_ITCMRAM  compressorParam2(float value, int v, int string)
{
	comp[v][string]->R = ((value*10.0f)+1.0f);
	comp[v][string]->invR  = 1.0f / comp[v][string]->R;
}

void __ATTR_ITCMRAM  compressorParam3(float value, int v, int string)
{
	comp[v][string]->M = value * 18.0f;
}

void __ATTR_ITCMRAM  compressorParam4(float value, int v, int string)
{
	value = (value +  0.001f);
	comp[v][string]->tauAttack = fastExp4(-1.0f/(value * comp[v][string]->sampleRate));
}

void __ATTR_ITCMRAM  compressorParam5(float value, int v, int string)
{
	value = (value + 0.001f);
	comp[v][string]->tauRelease = fastExp4(-1.0f/(value * comp[v][string]->sampleRate));
}

void __ATTR_ITCMRAM  offsetParam2(float value, int v, int string)
{
	param2[v][string] = (value * 2.0f) - 1.0f;
}
void __ATTR_ITCMRAM param2Linear(float value, int v, int string)
{
	param2[v][string] = value;
}
void __ATTR_ITCMRAM param3Linear(float value, int v, int string)
{
	param3[v][string] = value;
}
void __ATTR_ITCMRAM param3Soft(float value, int v, int string)
{
	param3[v][string] = (value * .99f) + 0.01f;
	shapeDividerS[v][string] = 1.0f / (param3[v][string] - ((param3[v][string]*param3[v][string]*param3[v][string]) * 0.3333333f));
}

void __ATTR_ITCMRAM param3Hard(float value, int v, int string)
{
	param3[v][string] = ((value * .99f) + 0.01f) * HALF_PI;
	float tempDiv = sinf(param3[v][string]);
	if (tempDiv == 0.0f)
	{
		tempDiv = 0.001f;
	}
	shapeDividerH[v][string] = 1.0f/tempDiv;
}

void __ATTR_ITCMRAM param3Poly(float value, int v, int string)
{
	param3[v][string] = ((value * .99f) + 0.01f)* HALF_PI;

    float tempDiv = fastSine(param3[v][string]);
    if (tempDiv == 0.0f)
    {
        tempDiv = 0.001f;
    }
    polyDivider[v][string] = 1.0f/tempDiv;
}

void __ATTR_ITCMRAM param4Linear(float value, int v, int string)
{
	param4[v][string] = value;
}

void __ATTR_ITCMRAM param5Linear(float value, int v, int string)
{
	param5[v][string] = value;
}
void __ATTR_ITCMRAM param2BC(float value, int v, int string)
{
	tCrusher_setQuality (bc[v][string],value);
}
void __ATTR_ITCMRAM param3BC(float value, int v, int string)
{
	value = ((1.0f - value)* inv_oversample) + 0.01f;
	tCrusher_setSamplingRatio (bc[v][string], value);
}
void __ATTR_ITCMRAM param4BC(float value, int v, int string)
{
	tCrusher_setRound(bc[v][string], value);
}
void __ATTR_ITCMRAM param5BC(float value, int v, int string)
{
	tCrusher_setOperation(bc[v][string], value);
}



void __ATTR_ITCMRAM fxMixSet(float value, int v, int string)
{
	fxMix[v][string] = value;
}

void __ATTR_ITCMRAM fxPostGainSet(float value, int v, int string)
{
	fxPostGain[v][string] = dbToATableLookup((value * 24.0f) - 12.0f);
}


//got the idea from https://ccrma.stanford.edu/~jatin/ComplexNonlinearities/Wavefolder.html  -JS
//much more efficient than the lockhart, and can be further optimized with lookups
float __ATTR_ITCMRAM wavefolderTick(float sample, int v, int string)
{
    sample = sample * param1[v][string] + ((param2[v][string] * param1[v][string]));
    float curFB = param3[v][string];
    float curFF = param4[v][string];

    //softclip approx for tanh saturation in original code
    float ffSample = sample;
    if (ffSample <= -1.0f)
    {
    	ffSample = -1.0f;
    } else if (ffSample >= 1.0f)
    {
    	ffSample = 1.0f;
    }
    ffSample = ffSample - ((ffSample * ffSample * ffSample)* 0.3333333f);
    ffSample *= 1.499999f;
    float ff = (curFF * ffSample) + ((1.0f - curFF) * sample);

    //softclip approx for tanh saturation in original code
    float fbSample = wfState[v][string];
    if (fbSample <= -1.0f)
    {
    	fbSample = -1.0f;
    } else if (fbSample >= 1.0f)
    {
    	fbSample = 1.0f;
    }
    fbSample = fbSample - ((fbSample * fbSample * fbSample)* 0.3333333f);
    fbSample *= 1.499999f;
    float fb = curFB * fbSample;

    wfState[v][string] = (ff + fb) - param5[v][string] * sinf(TWO_PI * sample); //maybe switch for our own sine lookup (avoid the if statements in the CMSIS code)
    sample = wfState[v][string] * invCurFB[v][string];
    sample = tHighpass_tick(dcBlock1[v][string], sample);
    return sample;
}


void __ATTR_ITCMRAM chorusParam1(float value, int v, int string)
{
	param1[v][string] = value * 5780.0f + 10.0f;
}
void __ATTR_ITCMRAM chorusParam2(float value, int v, int string)
{
	param2[v][string] = value * 0.1f;
}

void __ATTR_ITCMRAM chorusParam3(float value, int v, int string)
{
    tCycle_setFreq(mod1[v][string], (value * 0.4f) + 0.01f);
}

void __ATTR_ITCMRAM chorusParam4(float value, int v, int string)
{
    tCycle_setFreq(mod2[v][string], (value * 0.4444444f) + 0.011f);
}


void __ATTR_ITCMRAM delayParam1(float value, int v, int string)
{
    tTapeDelay_setDelay(tapeDelay[string], value * 30000.0f + 1.0f);
}

void __ATTR_ITCMRAM delayParam2(float value, int v, int string)
{
	param2[v][string] = LEAF_clip(0.0f, value * 1.1f, 1.1f);
}

void __ATTR_ITCMRAM delayParam3(float value, int v, int string)
{
    tSVF_setFreqFast(FXlowpass[v][string], (value * 127.0f));
}

void __ATTR_ITCMRAM delayParam4(float value, int v, int string)
{
    tSVF_setFreqFast(FXhighpass[v][string], (value * 127.0f));
}

void __ATTR_ITCMRAM delayParam5(float value, int v, int string)
{
	param5[v][string] = (value * 1.5f) + 1.0f;
}



float delayTick(float sample, int v, int string)
{
    sample *= param5[v][string];

    sample = sample + (delayFB[v][string] * param2[v][string]);

    sample = fast_tanh5(sample);

    sample = tFeedbackLeveler_tick(feedbackControl[v][string], sample);

    delayFB[v][string] = tTapeDelay_tick(tapeDelay[string], sample);
    delayFB[v][string] = tSVF_tick(FXlowpass[v][string], delayFB[v][string]);
    sample = tSVF_tick(FXhighpass[v][string], delayFB[v][string]);
    sample = fast_tanh5(sample);
    delayFB[v][string] = sample;
    return sample;
}

float __ATTR_ITCMRAM chorusTick(float sample, int v, int string)
{
	tLinearDelay_setDelay(delay1[string], param1[v][string] * .707f * (1.0f + param2[v][string] * tCycle_tick(mod1[v][string])));
    tLinearDelay_setDelay(delay2[string], param1[v][string] * .5f * (1.0f - param2[v][string] * tCycle_tick(mod2[v][string])));
    float temp = tLinearDelay_tick(delay1[string], sample) - sample;
    temp += tLinearDelay_tick(delay2[string], sample) - sample;
    temp = tHighpass_tick(dcBlock1[v][string], temp);
	//float temp = 0.0f;
    return -temp;
}

float __ATTR_ITCMRAM shaperTick(float sample, int v, int string)
{
    sample = sample * (param1[v][string]+1.0f);
    float temp = LEAF_shaper(sample + (param2[v][string] * param1[v][string]),param3[v][string]);
    temp = tHighpass_tick(dcBlock1[v][string], temp);
    return temp;
}

float __ATTR_ITCMRAM blankTick(float sample, int v, int string)
{
    return sample;
}

float __ATTR_ITCMRAM tiltFilterTick(float sample, int v, int string)
{
    sample = tTiltFilter_tick(FXTilt[v][string], sample);
    sample = tVZFilterBell_tick(bell1[v][string], sample);
    return sample;
}

float __ATTR_ITCMRAM tanhTick(float sample, int v, int string)
{
    float gain = param1[v][string];
	sample = sample * gain;
    gain = gain * 0.5f;
    //need to do something with shape param
    float temp = fast_tanh5(sample + (param2[v][string]*gain));
    temp = tHighpass_tick(dcBlock1[v][string], temp);
    //temp *= param4[v][string];
    temp = fast_tanh5(temp);
    //temp = tHighpass_tick(&dcBlock2, temp);
    return temp;
}


float __ATTR_ITCMRAM softClipTick(float sample, int v, int string)
{
    sample = sample * param1[v][string];
    sample = sample + param2[v][string];
    //float shape = param3[v][string];
    if (sample <= -1.0f)
    {
        sample = -1.0f;
    } else if (sample >= 1.0f)
    {
        sample = 1.0f;
    }
    {
        sample = 1.5f * (sample) - (((sample * sample * sample))* 0.3333333f);
        //sample = sample * shapeDividerS[v][string];
    }

    sample = tHighpass_tick(dcBlock1[v][string], sample);
    return sample;
}


float __ATTR_ITCMRAM hardClipTick(float sample, int v, int string)
{

    sample = sample * param1[v][string];
    sample = sample + param2[v][string];
    if (sample <= -1.0f)
    {
        sample = -1.0f;
    } else if (sample >= 1.0f)
    {
        sample = 1.0f;
    }
    {
        sample = sinf(  (sinf(sample*param3[v][string]) * shapeDividerH[v][string]) * param3[v][string]);
        sample = sample * shapeDividerH[v][string];
    }

    sample = tHighpass_tick(dcBlock1[v][string], sample);
    return sample;
}


float __ATTR_ITCMRAM polynomialShaperTick(float sample, int v, int string)
{

    sample = sample * param1[v][string];
    sample = sample + param2[v][string];

    if (sample <= -1.0f)
    {
        sample = -1.0f;
    } else if (sample >= 1.0f)
    {
        sample = 1.0f;
    }

	sample = fastSine(  (fastSine(sample*param3[v][string]) * polyDivider[v][string]) * param3[v][string]);
	sample = sample * polyDivider[v][string];


    sample = tHighpass_tick(dcBlock1[v][string], sample);
    return sample;
}
float __ATTR_ITCMRAM satTick(float sample, int v, int string)
{;
    sample = sample * param1[v][string];
    float temp = (sample + (param2[v][string] * param1[v][string])) / (1.0f + fabsf(sample + param2[v][string]));
    temp = tHighpass_tick(dcBlock1[v][string], temp);
    temp = tHighpass_tick(dcBlock2[v][string], temp);
    temp = fast_tanh5(temp);
    return temp;
}



float __ATTR_ITCMRAM bcTick(float sample, int v, int string)
{
    sample = sample * param1[v][string];
    return tCrusher_tick(bc[v][string], sample);
}


float __ATTR_ITCMRAM compressorTick(float sample, int v, int string)
{
    return tCompressor_tick(comp[v][string], sample);
}

float __ATTR_ITCMRAM  FXlowpassTick(float sample, int v, int string)
{
	return tSVF_tickLP(FXlowpass[v][string], sample);
}

float __ATTR_ITCMRAM  FXhighpassTick(float sample, int v, int string)
{
	return tSVF_tickHP(FXhighpass[v][string], sample);
}

float __ATTR_ITCMRAM  FXbandpassTick(float sample, int v, int string)
{
	return tSVF_tickBP(FXbandpass[v][string], sample);
}

float __ATTR_ITCMRAM  FXdiodeLowpassTick(float sample, int v, int string)
{
	return tDiodeFilter_tickEfficient(FXdiodeFilters[v][string], sample);
}

float __ATTR_ITCMRAM  FXVZpeakTick(float sample, int v, int string)
{
	return tVZFilterBell_tick(FXVZfilterPeak[v][string], sample);
}

float __ATTR_ITCMRAM  FXVZlowshelfTick(float sample, int v, int string)
{
	return tVZFilterLS_tick(FXVZfilterLS[v][string], sample);
}
float __ATTR_ITCMRAM  FXVZhighshelfTick(float sample, int v, int string)
{
	return tVZFilterHS_tick(FXVZfilterHS[v][string], sample);
}
float __ATTR_ITCMRAM  FXVZbandrejectTick(float sample, int v, int string)
{
	return  tVZFilterBR_tick(FXVZfilterBR[v][string], sample);
}

float __ATTR_ITCMRAM  FXLadderLowpassTick(float sample, int v, int string)
{
	return tLadderFilter_tick(FXLadderfilter[v][string], sample);
}
//cutoffparams

void  __ATTR_ITCMRAM FXLowpassParam1(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
	tSVF_setFreqFast(FXlowpass[v][string], value);
}
void __ATTR_ITCMRAM FXHighpassParam1(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
	tSVF_setFreqFast(FXhighpass[v][string], value);
}

void __ATTR_ITCMRAM FXBandpassParam1(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
    tSVF_setFreqFast(FXbandpass[v][string], value);
}

void __ATTR_ITCMRAM FXDiodeParam1(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
	tDiodeFilter_setFreqFast(FXdiodeFilters[v][string], value);
}
void __ATTR_ITCMRAM FXPeakParam1(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
	tVZFilterBell_setFreq(FXVZfilterPeak[v][string], value);
}
void __ATTR_ITCMRAM FXLowShelfParam1(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
    tVZFilterLS_setFreqFast(FXVZfilterLS[v][string], value);
}
void __ATTR_ITCMRAM FXHighShelfParam1(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
	tVZFilterHS_setFreqFast(FXVZfilterHS[v][string], value);
}
void __ATTR_ITCMRAM FXNotchParam1(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
	tVZFilterBR_setFreqFast(FXVZfilterBR[v][string], value);
}
void __ATTR_ITCMRAM FXLadderParam1(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
	tLadderFilter_setFreqFast(FXLadderfilter[v][string], value);
}

//gain params

void __ATTR_ITCMRAM FXPeakParam2(float value, int v, int string)
{
	 tVZFilterBell_setGain(FXVZfilterPeak[v][string], dbToATableLookup((value * 50.f) - 25.f));
}

void __ATTR_ITCMRAM FXLowShelfParam2(float value, int v, int string)
{
	tVZFilterLS_setGain(FXVZfilterLS[v][string], dbToATableLookup((value * 50.f) - 25.f));
}

void __ATTR_ITCMRAM FXHighShelfParam2(float value, int v, int string)
{
	tVZFilterHS_setGain(FXVZfilterHS[v][string], dbToATableLookup((value * 50.f) - 25.f));
}

void __ATTR_ITCMRAM FXNotchParam2(float value, int v, int string)
{
	tVZFilterBR_setGain(FXVZfilterBR[v][string], dbToATableLookup((value * 50.f) - 25.f));

}
//resonance params
void __ATTR_ITCMRAM FXLowpassParam3(float value, int v, int string)
{
	tSVF_setQ(FXlowpass[v][string], value);
}

void __ATTR_ITCMRAM FXHighpassParam3(float value, int v, int string)
{
    tSVF_setQ(FXhighpass[v][string], value);
}

void __ATTR_ITCMRAM FXBandpassParam3(float value, int v, int string)
{
    tSVF_setQ(FXbandpass[v][string], value);
}

void __ATTR_ITCMRAM FXDiodeParam3(float value, int v, int string)
{
	tDiodeFilter_setQ(FXdiodeFilters[v][string], value);
}


void __ATTR_ITCMRAM FXPeakParam3(float value, int v, int string)
{
	tVZFilterBell_setBandwidth(FXVZfilterPeak[v][string], value);
}


void __ATTR_ITCMRAM FXLowShelfParam3(float value, int v, int string)
{
	tVZFilterLS_setResonance(FXVZfilterLS[v][string], value);
}


void __ATTR_ITCMRAM FXHighShelfParam3(float value, int v, int string)
{
	tVZFilterHS_setResonance(FXVZfilterHS[v][string], value);
}


void __ATTR_ITCMRAM FXNotchParam3(float value, int v, int string)
{
	tVZFilterBR_setResonance(FXVZfilterBR[v][string], value);
}


void __ATTR_ITCMRAM FXLadderParam3(float value, int v, int string)
{
	tLadderFilter_setQ(FXLadderfilter[v][string], value);
}




/////NOISE///

void __ATTR_ITCMRAM noiseSetTilt(float value, int v, int string)
{
	//tTiltFilter_setTilt(&noiseTilt[string], dbToATableLookup(((value * 30.0f) - 15.0f)));
	tTiltFilter_setTilt(noiseTilt[string], value*10.0f - 5.0f);
}


void __ATTR_ITCMRAM noiseSetGain(float value, int v, int string)
{
	tVZFilterBell_setGain(noiseBell1[string], dbToATableLookup((value* 34.0f) - 17.0f));
}

void __ATTR_ITCMRAM noiseSetFreq(float value, int v, int string)
{
	value = (value * 77.0f) + 42.0f;
	tVZFilterBell_setFreq(noiseBell1[string], faster_mtof(value));
	//value = LEAF_clip(0.0f, (value-16.0f) * 35.929824561403509f, 4095.0f);

}


void __ATTR_ITCMRAM noise_tick(int string)
{
	float enabled = params[Noise].realVal[string];
	float amp = params[NoiseAmp].realVal[string];
	float filterSend = params[NoiseFilterSend].realVal[string];
	amp = amp < 0.f ? 0.f : amp;
	float sample = (randomNumber() * 2.0f) - 1.0f;
	sample = tTiltFilter_tick(noiseTilt[string], sample);
	sample = tVZFilterBell_tick(noiseBell1[string], sample);
	sample = sample * amp;
	if (!isfinite(sample))
	{
		nanChuckTest++;
		sample = 0.0f;
	}
	float normSample = (sample + 1.f) * 0.5f;
	sourceValues[NOISE_SOURCE_OFFSET][string] = normSample;
	noiseOuts[0][string] = sample * filterSend *  enabled;
	noiseOuts[1][string] = sample * (1.f-filterSend) * enabled ;
}


#if 0
tCycle myTest;

void synthInit(void)
{
	tCycle_init(&myTest, &leaf);
	tCycle_setFreq(myTest, 440.0f);
}

void synthSetFreq(float freq)
{
	tCycle_setFreq(myTest, freq);
}
float synthTick(void)
{
	return tCycle_tick(myTest);

}
#endif
