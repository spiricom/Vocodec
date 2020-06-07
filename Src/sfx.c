/*
 * sfx.c
 *
 *  Created on: Dec 23, 2019
 *      Author: josnyder
 */

#include "main.h"
#include "sfx.h"
#include "oled.h"
#include "tunings.h"
#include "MIDI_application.h"

#define INC_MISC_WT 0
#define USE_FILTERTAN_TABLE 1


float defaultPresetKnobValues[PresetNil][NUM_PRESET_KNOB_VALUES];
float presetKnobValues[PresetNil][NUM_PRESET_KNOB_VALUES];
uint8_t knobActive[NUM_ADC_CHANNELS];

//audio objects
tFormantShifter fs;
tRetune autotuneMono;
tAutotune autotunePoly;
tRetune retune;
tRetune retune2;
tRamp pitchshiftRamp;
tRamp nearWetRamp;
tRamp nearDryRamp;
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
tLockhartWavefolder wavefolder3;
tLockhartWavefolder wavefolder4;
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

tStack noteOffStack;

#define EXP_BUFFER_SIZE 128
float expBuffer[EXP_BUFFER_SIZE];
float expBufferSizeMinusOne = EXP_BUFFER_SIZE - 1;

#define DECAY_EXP_BUFFER_SIZE 512
float decayExpBuffer[DECAY_EXP_BUFFER_SIZE];
float decayExpBufferSizeMinusOne = DECAY_EXP_BUFFER_SIZE - 1;


#define NUM_STRINGS 6
tComplexLivingString theString[NUM_STRINGS];

float myFreq;
float myDetune[NUM_STRINGS];
float synthDetune[NUM_VOC_VOICES][NUM_OSC_PER_VOICE];
//control objects
float notes[128];
float notePeriods[128];
float noteFreqs[128];
int chordArray[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int chromaticArray[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

int lockArray[12];
float freq[NUM_VOC_VOICES];
float oversamplerArray[MAX_OVERSAMPLER_RATIO];





void initGlobalSFXObjects()
{
	calculateNoteArray();

	tSimplePoly_initToPool(&poly, NUM_VOC_VOICES, &smallPool);
	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{
		tExpSmooth_initToPool(&polyRamp[i], 0.0f, 0.02f, &smallPool);
	}

	tExpSmooth_init(&comp, 1.0f, 0.01f);
	tStack_init(&noteOffStack);

	LEAF_generate_exp(expBuffer, 1000.0f, -1.0f, 0.0f, -0.0008f, EXP_BUFFER_SIZE); //exponential buffer rising from 0 to 1
	LEAF_generate_exp(decayExpBuffer, 0.001f, 0.0f, 1.0f, -0.0008f, DECAY_EXP_BUFFER_SIZE); // exponential decay buffer falling from 1 to 0


	// Note that these are the actual knob values
	// not the parameter value
	// (i.e. 0.5 for fine pitch is actually 0.0 fine pitch)
	defaultPresetKnobValues[Vocoder][0] = 0.5f; // volume
	defaultPresetKnobValues[Vocoder][1] = 0.5f; // warp factor
	defaultPresetKnobValues[Vocoder][2] = 0.85f; // quality
	defaultPresetKnobValues[Vocoder][3] = 0.0f; // sawToPulse
	defaultPresetKnobValues[Vocoder][4] = 0.2f; // noise threshold
	defaultPresetKnobValues[Vocoder][5] = 0.02f; // breathiness
	defaultPresetKnobValues[Vocoder][6] = 0.5f; // tilt
	defaultPresetKnobValues[Vocoder][7] = 0.5f; // pulse width
	defaultPresetKnobValues[Vocoder][8] = 0.5f; // pulse shape
	defaultPresetKnobValues[Vocoder][9] = 0.0f;


	defaultPresetKnobValues[VocoderCh][0] = 0.5f; // volume
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
	defaultPresetKnobValues[Pitchshift][4] = 0.0f;

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
	defaultPresetKnobValues[SamplerButtonPress][3] = 0.25f; // crossfade
	defaultPresetKnobValues[SamplerButtonPress][4] = 0.0f;

	defaultPresetKnobValues[SamplerKeyboard][0] = 0.0f; // start
	defaultPresetKnobValues[SamplerKeyboard][1] = 1.0f; // end
	defaultPresetKnobValues[SamplerKeyboard][2] = 0.75f; // speed
	defaultPresetKnobValues[SamplerKeyboard][3] = 0.25f; // crossfade
	defaultPresetKnobValues[SamplerKeyboard][4] = 0.0f;

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
	defaultPresetKnobValues[Distortion][1] = 0.5f; // tilt (low and high shelfs, opposing gains
	defaultPresetKnobValues[Distortion][2] = 0.5f; // mid gain
	defaultPresetKnobValues[Distortion][3] = 0.5f; // mid freq
	defaultPresetKnobValues[Distortion][4] = 0.25f; //post gain

	defaultPresetKnobValues[Wavefolder][0] = 0.25f; // gain
	defaultPresetKnobValues[Wavefolder][1] = 0.5f; // offset1
	defaultPresetKnobValues[Wavefolder][2] = 0.5f; // offset2
	defaultPresetKnobValues[Wavefolder][3] = 0.3f; // post gain
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
	defaultPresetKnobValues[LivingString][7] = 0.5f;
	defaultPresetKnobValues[LivingString][8] = 0.8f;
	defaultPresetKnobValues[LivingString][9] = 0.5f;
	defaultPresetKnobValues[LivingString][10] = 0.3f;// freq 2
	defaultPresetKnobValues[LivingString][11] = 0.3f;// freq 3
	defaultPresetKnobValues[LivingString][12] = 0.3f;// freq 4
	defaultPresetKnobValues[LivingString][13] = 0.3f;// freq 5
	defaultPresetKnobValues[LivingString][14] = 0.3f;// freq 6

	defaultPresetKnobValues[LivingStringSynth][0] = 0.0f;
	defaultPresetKnobValues[LivingStringSynth][1] = 0.0f;
	defaultPresetKnobValues[LivingStringSynth][2] = 0.5f; // decay
	defaultPresetKnobValues[LivingStringSynth][3] = 0.8f; // damping
	defaultPresetKnobValues[LivingStringSynth][4] = 0.5f; // pick pos
	defaultPresetKnobValues[LivingStringSynth][5] = 0.0f;

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
	defaultPresetKnobValues[Rhodes][3] = 0.2f;
	defaultPresetKnobValues[Rhodes][4] = 0.0f; //stereo spread
	defaultPresetKnobValues[Rhodes][5] = 0.05f;
	defaultPresetKnobValues[Rhodes][6] = 0.05f;
	defaultPresetKnobValues[Rhodes][7] = 0.9f;
	defaultPresetKnobValues[Rhodes][8] = 0.1007f;
	defaultPresetKnobValues[Rhodes][9] = 0.5f;

	for (int p = 0; p < PresetNil; p++)
	{
		for (int v = 0; v < NUM_PRESET_KNOB_VALUES; v++)
		{
			presetKnobValues[p][v] = defaultPresetKnobValues[p][v];
		}
	}
}

///1 vocoder internal poly

tTalkboxFloat vocoder;
tNoise vocoderNoise;
tZeroCrossing zerox;
tSawtooth osc[NUM_VOC_VOICES * NUM_OSC_PER_VOICE];
tRosenbergGlottalPulse glottal[NUM_VOC_VOICES];
uint8_t numVoices = NUM_VOC_VOICES;
uint8_t internalExternal = 0;
int vocFreezeLPC = 0;
tExpSmooth noiseRamp;
tNoise breathNoise;
tHighpass noiseHP;
tVZFilter shelf1;
tVZFilter shelf2;

void SFXVocoderAlloc()
{
	tTalkboxFloat_initToPool(&vocoder, 1024,  &smallPool);
	tTalkboxFloat_setWarpOn(&vocoder, 1);
	tNoise_initToPool(&vocoderNoise, WhiteNoise, &smallPool);
	tZeroCrossing_initToPool(&zerox, 16, &smallPool);
	tSimplePoly_setNumVoices(&poly, numVoices);
	tExpSmooth_initToPool(&noiseRamp, 0.0f, 0.005f, &smallPool);

	//tilt filter
	tVZFilter_initToPool(&shelf1, Lowshelf, 80.0f, 6.0f, &smallPool);
	tVZFilter_initToPool(&shelf2, Highshelf, 12000.0f, 6.0f, &smallPool);

	tNoise_initToPool(&breathNoise, WhiteNoise, &smallPool);
	tHighpass_initToPool(&noiseHP, 4500.0f, &smallPool);

	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{

		tSawtooth_initToPool(&osc[i], &smallPool);

		tRosenbergGlottalPulse_initToPool(&glottal[i], &smallPool);
		tRosenbergGlottalPulse_setOpenLengthAndPulseLength(&glottal[i], 0.3f, 0.4f);
	}
	setLED_A(numVoices == 1);
	setLED_B(internalExternal);
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
		tExpSmooth_setDest(&comp, sqrtf(1.0f / tSimplePoly_getNumActiveVoices(&poly)));
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
		zerocross = tZeroCrossing_tick(&zerox, input[1]);

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
	tTalkboxFloat_freeFromPool(&vocoder,  &smallPool);
	tNoise_freeFromPool(&vocoderNoise, &smallPool);
	tZeroCrossing_freeFromPool(&zerox, &smallPool);
	tExpSmooth_freeFromPool(&noiseRamp, &smallPool);

	tNoise_freeFromPool(&breathNoise, &smallPool);
	tHighpass_freeFromPool(&noiseHP, &smallPool);

	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{
		tSawtooth_freeFromPool(&osc[i], &smallPool);
		tRosenbergGlottalPulse_freeFromPool(&glottal[i], &smallPool);
	}
}

#define MAX_VOCODER_FILTER_ORDER 2
#define MAX_NUM_VOCODER_BANDS 24
tVZFilter analysisBands[MAX_NUM_VOCODER_BANDS][MAX_VOCODER_FILTER_ORDER];
tVZFilter synthesisBands[MAX_NUM_VOCODER_BANDS][MAX_VOCODER_FILTER_ORDER];
tExpSmooth envFollowers[MAX_NUM_VOCODER_BANDS];
uint8_t numberOfVocoderBands = 22;
int filterOrder = 2;
uint8_t prevNumberOfVocoderBands = 22;
float invNumberOfVocoderBands = 0.03125f;


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
uint32_t vocChFreeze = 0;
float barkBandFreqs[24] = {100.0f, 150.0f, 250.0f, 350.0f, 450.0f, 570.0f, 700.0f, 840.0f, 1000.0f, 1170.0f, 1370.0f, 1600.0f, 1850.0f, 2150.0f, 2500.0f, 2900.0f, 3400.0f, 4000.0f, 4800.0f, 5800.0f, 7000.0f, 8500.0f, 10500.0f, 12000.0f};
float barkBandWidths[24] = {1.0f, 1.0f, 0.5849f, 0.4150f, 0.3505f, 0.304854f, 0.2895066175f, 0.256775415f, 0.231325545833333f, 0.233797185f, 0.220768679166667f, 0.216811389166667f, 0.217591435f, 0.214124805f, 0.218834601666667f, 0.222392421666667f, 0.2321734425f, 0.249978253333333f, 0.268488835833333f, 0.272079545833333f, 0.266786540833333f, 0.3030690675f, 0.3370349875f, 0.36923381f};
void SFXVocoderChAlloc()
{

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

	tVZFilter_initToPool(&vocodec_highshelf, Highshelf, 6000.0f, 3.0f, &smallPool);
	tVZFilter_setGain(&vocodec_highshelf, 4.0f);

	for (int i = 0; i < MAX_NUM_VOCODER_BANDS; i++)
	{

		float bandFreq = faster_mtof((i * bandWidthInSemitones) + 30.0f); //midinote 28 (41Hz) to midinote 134 (18814Hz) is 106 midinotes, divide that by how many bands to find out how far apart to put the bands

		bandGains[i] = 1.0f;

		if (i == 0)
		{
			tVZFilter_init(&analysisBands[i][0], Lowpass, bandFreq, thisBandwidth);
			tVZFilter_init(&analysisBands[i][1], Lowpass, bandFreq, thisBandwidth);

			tVZFilter_init(&synthesisBands[i][0], Lowpass, bandFreq, thisBandwidth);
			tVZFilter_init(&synthesisBands[i][1], Lowpass, bandFreq, thisBandwidth);

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
		tExpSmooth_initToPool(&envFollowers[i], 0.0f, 0.001f, &smallPool); // factor of .001 is 10 ms?
	}
	tNoise_initToPool(&breathNoise, WhiteNoise, &smallPool);
	tNoise_initToPool(&vocoderNoise, WhiteNoise, &smallPool);
	tZeroCrossing_initToPool(&zerox, 256, &smallPool);
	tSimplePoly_setNumVoices(&poly, numVoices);
	tExpSmooth_initToPool(&noiseRamp, 0.0f, 0.05f, &smallPool);
	tHighpass_initToPool(&noiseHP, 5000.0f, &smallPool);
	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{

		tSawtooth_initToPool(&osc[i], &smallPool);

		tRosenbergGlottalPulse_initToPool(&glottal[i], &smallPool);
		tRosenbergGlottalPulse_setOpenLength(&glottal[i], 0.3f);
		tRosenbergGlottalPulse_setPulseLength(&glottal[i], 0.4f);
	}
	setLED_A(numVoices == 1);
	setLED_B(internalExternal);
	setLED_C(vocChFreeze);




}


int whichKnobThisTime = 0;

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


	switch(whichKnobThisTime++ % 15)
	{
		case 0:
			displayValues[0] = presetKnobValues[VocoderCh][0]; //vocoder volume
			break;
		case 1:
			displayValues[1] = (presetKnobValues[VocoderCh][1] * 0.8f) - 0.4f; //warp factor
			break;
		case 2:
			displayValues[2] = (uint8_t)(presetKnobValues[VocoderCh][2] * 16.9f) + 8.0f; //quality
			break;
		case 3:
			displayValues[3] = (presetKnobValues[VocoderCh][3]* 2.0f) + 0.1f; //band width
			break;
		case 4:
			displayValues[4] = presetKnobValues[VocoderCh][4]; //noise thresh
			break;
		case 5:
			displayValues[5] = presetKnobValues[VocoderCh][5]; //crossfade between sawtooth and glottal pulse
			break;
		case 6:
			displayValues[6] = presetKnobValues[VocoderCh][6]; //pulse width
			break;
		case 7:
			displayValues[7] = presetKnobValues[VocoderCh][7]; //pulse shape
			break;
		case 8:
			displayValues[8] = presetKnobValues[VocoderCh][8]; //breathiness
			break;
		case 9:
			displayValues[9] = presetKnobValues[VocoderCh][9]; //speed
			break;
		case 10:
			displayValues[10] = presetKnobValues[VocoderCh][10] + 0.5f; //bandsquish
			break;
		case 11:
			displayValues[11] = presetKnobValues[VocoderCh][11] * 60.0f; //bandoffset
			break;
		case 12:
			displayValues[12] = (presetKnobValues[VocoderCh][12] * 4.0f) - 2.0f; //tilt
			break;
		case 13:
			displayValues[13] = presetKnobValues[VocoderCh][13]; //stereo
			break;
		case 14:
			displayValues[14] = presetKnobValues[VocoderCh][14]; //snap to bark scale
			break;

	}

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
		float bandFreq = faster_mtof((currentBandToAlter * bandWidthInSemitones) + bandOffset); //midinote 28 (41Hz) to midinote 134 (18814Hz) is 106 midinotes, divide that by how many bands to find out how far apart to put the bands

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
		float myHeight = currentBandToAlter * invNumberOfVocoderBands; //x value
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
		tExpSmooth_setDest(&comp, sqrtf(1.0f / tSimplePoly_getNumActiveVoices(&poly)));
	}
	else
	{
		tExpSmooth_setDest(&comp, 0.0f);
	}
}


//freeze (maybe C button?)
void SFXVocoderChTick(float* input)
{
	float zerocross = 0.0f;
	float noiseRampVal = 0.0f;
	float sample = 0.0f;

	//a little treble emphasis
	input[1] = tVZFilter_tick(&vocodec_highshelf, input[1]);

	if (internalExternal == 1)
	{
		sample = input[0];
	}
	else
	{
		zerocross = tZeroCrossing_tick(&zerox, input[1]);

		if (!vocChFreeze)
		{
			tExpSmooth_setDest(&noiseRamp,zerocross > ((displayValues[4])-0.1f));
		}
		noiseRampVal = tExpSmooth_tick(&noiseRamp);

		float noiseSample = tNoise_tick(&vocoderNoise) * noiseRampVal;

		for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
		{
			float tempRamp = tExpSmooth_tick(&polyRamp[i]);
			if (tempRamp > 0.001f)
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

	sample = LEAF_tanh(sample);



	float output[2] = {0.0f, 0.0f};
	input[1] = input[1] * (displayValues[0] * 30.0f);
	for (int i = 0; i < numberOfVocoderBands; i++)
	{
		uint8_t oddEven = i % 2;
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

	input[0] = 0.98f * tanhf((output[0] + (output[1] * (1.0f - displayValues[13]))) * (displayValues[0] * 9.0f));
	input[1] = 0.98f * tanhf((output[1] + (output[0] * (1.0f - displayValues[13]))) * (displayValues[0] * 9.0f));


}

void SFXVocoderChFree(void)
{
	for (int i = 0; i < MAX_NUM_VOCODER_BANDS; i++)
	{
		tVZFilter_free(&analysisBands[i][0]);
		tVZFilter_free(&analysisBands[i][1]);
		tVZFilter_free(&synthesisBands[i][0]);
		tVZFilter_free(&synthesisBands[i][1]);
		tExpSmooth_freeFromPool(&envFollowers[i], &smallPool);
	}
	tNoise_freeFromPool(&breathNoise, &smallPool);
	tNoise_freeFromPool(&vocoderNoise, &smallPool);
	tZeroCrossing_freeFromPool(&zerox, &smallPool);
	tExpSmooth_freeFromPool(&noiseRamp, &smallPool);
	tHighpass_freeFromPool(&noiseHP, &smallPool);
	tVZFilter_freeFromPool(&vocodec_highshelf, &smallPool);
	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{
		tSawtooth_freeFromPool(&osc[i], &smallPool);
		tRosenbergGlottalPulse_freeFromPool(&glottal[i], &smallPool);
	}
}

// pitch shift

void SFXPitchShiftAlloc()
{
	//tRetune_init(&retune, NUM_RETUNE, 2048, 1024);

	tFormantShifter_initToPool(&fs, 7, &smallPool);
	tRetune_init(&retune, NUM_RETUNE, 1024, 512);
	tRetune_init(&retune2, NUM_RETUNE, 1024, 512);
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

	float keyPitch = tSimplePoly_getPitchAndCheckActive(&poly, 0);
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
	tFormantShifter_freeFromPool(&fs, &smallPool);
	tRetune_free(&retune);
	tRetune_free(&retune2);

	tRamp_free(&pitchshiftRamp);

	tExpSmooth_free(&smoother1);
	tExpSmooth_free(&smoother2);
	tExpSmooth_free(&smoother3);
}




//5 autotune mono
uint8_t autotuneChromatic = 0;
uint32_t autotuneLock = 0;
float lastSnap = 1.0f;
int updatePitch = 1;

void SFXNeartuneAlloc()
{
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

float detectedNote = 60.0f;
float desiredSnap = 60.0f;
float destinationNote = 60.0f;
float destinationFactor = 1.0f;
float factorDiff = 0.0f;
int diffCounter = 0;
int maxDiffCounter = 48000;
float lastDetectedNote = 60.0f;
int stabilityCounter = 0;
int stabilityCounterThresh = 4800;
float changeAmount = 0.0f;

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
	stabilityCounterThresh = displayValues[3];
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
	tAutotune_init(&autotunePoly, NUM_AUTOTUNE, 1024, 512);
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
float maxSampleSizeSeconds = 1.0f;
uint8_t samplePlaying = 1;

void SFXSamplerBPAlloc()
{
	tBuffer_initToPool(&buff, leaf.sampleRate * 172.0f, &largePool);
	tBuffer_setRecordMode(&buff, RecordOneShot);
	tSampler_init(&sampler, &buff);
	tSampler_setMode(&sampler, PlayLoop);
}

void SFXSamplerBPFrame()
{

}

void SFXSamplerBPTick(float* input)
{
	float sample = 0.0f;
	int recordPosition = tBuffer_getRecordPosition(&buff);

	if (buttonActionsSFX[ButtonDown][ActionPress])
	{
		if (samplePlaying)
		{
			samplePlaying = 0;
			tSampler_stop(&sampler);
		}
		else
		{
			samplePlaying = 1;
			tSampler_play(&sampler);
		}
		buttonActionsSFX[ButtonDown][ActionPress] = 0;
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

	float* knobs = presetKnobValues[SamplerButtonPress];

	sampleLength = recordPosition * leaf.invSampleRate;
	displayValues[0] = knobs[0] * sampleLength;
	displayValues[1] = LEAF_clip(0.0f, knobs[1] * sampleLength, sampleLength * (1.0f - knobs[0]));
	displayValues[2] = (knobs[2] - 0.5f) * 4.0f;
	displayValues[3] = knobs[3] * 4000.0f;

	samplePlayStart = knobs[0] * recordPosition;
	samplePlayLength = knobs[1] * recordPosition;
	samplerRate = displayValues[2];
	crossfadeLength = displayValues[3];
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
	tBuffer_freeFromPool(&buff, &largePool);
	tSampler_free(&sampler);
}

// keyboard sampler
int currentSamplerKey = 60 - LOWEST_SAMPLER_KEY;
int recordingSamplerKey = 60 - LOWEST_SAMPLER_KEY;
uint8_t samplerKeyHeld[NUM_SAMPLER_KEYS];
float keyKnobValues[NUM_SAMPLER_KEYS][KNOB_PAGE_SIZE];

void SFXSamplerKAlloc()
{
	currentSamplerKey = 60 - LOWEST_SAMPLER_KEY;
	for (int i = 0; i < NUM_SAMPLER_KEYS; i++)
	{
		//leaf.sampleRate * 172.0f
		tBuffer_initToPool(&keyBuff[i], leaf.sampleRate * 2.0f, &largePool);
		tBuffer_setRecordMode(&keyBuff[i], RecordOneShot);
		tSampler_init(&keySampler[i], &keyBuff[i]);
		tSampler_setMode(&keySampler[i], PlayLoop);
		for (int j = 0; j < KNOB_PAGE_SIZE; j++)
		{
			keyKnobValues[i][j] = defaultPresetKnobValues[currentPreset][j];
		}
		samplerKeyHeld[i] = 0;
	}
	tSimplePoly_setNumVoices(&poly, NUM_SAMPLER_VOICES);
}

void SFXSamplerKFrame()
{
	if (samplerKeyHeld[currentSamplerKey])
	{
		buttonActionsUI[ButtonC][ActionHoldContinuous] = 1;
		writeButtonFlag = ButtonC;
		writeActionFlag = ActionHoldContinuous;
	}
}

void SFXSamplerKTick(float* input)
{
	float sample = 0.0f;
	if (buttonActionsSFX[ButtonDown][ActionPress])
	{
		if (currentSamplerKey > 0) currentSamplerKey--;
		else currentSamplerKey = NUM_SAMPLER_KEYS - 1;
		setKnobValues(keyKnobValues[currentSamplerKey]);
		buttonActionsSFX[ButtonDown][ActionPress] = 0;
	}
	if (buttonActionsSFX[ButtonUp][ActionPress])
	{
		currentSamplerKey++;
		if (currentSamplerKey >= NUM_SAMPLER_KEYS) currentSamplerKey = 0;
		setKnobValues(keyKnobValues[currentSamplerKey]);
		buttonActionsSFX[ButtonUp][ActionPress] = 0;
	}
	if (buttonActionsSFX[ButtonB][ActionPress])
	{
		// this just resets the record position; should probably add a setRecordPosition to Buffer
		tBuffer_record(&keyBuff[currentSamplerKey]);
		tBuffer_stop(&keyBuff[currentSamplerKey]);

		buttonActionsSFX[ButtonB][ActionPress] = 0;
	}
	if (buttonActionsSFX[ButtonA][ActionPress])
	{
		recordingSamplerKey = currentSamplerKey;
		tBuffer_record(&keyBuff[recordingSamplerKey]);
		buttonActionsSFX[ButtonA][ActionPress] = 0;
		setLED_A(1);
	}
	if (buttonActionsSFX[ButtonA][ActionRelease])
	{
		// should we assume the user wants to edit what they just recorded
		// knobs would take immediate effect, maybe causing unwanted settings
//		editingSamplerKey = currentSamplerKey;
		tBuffer_stop(&keyBuff[recordingSamplerKey]);
		buttonActionsSFX[ButtonA][ActionRelease] = 0;
		setLED_A(0);
	}

	int recordPosition = tBuffer_getRecordPosition(&keyBuff[currentSamplerKey]);
	sampleLength = recordPosition * leaf.invSampleRate;

	float* knobs = presetKnobValues[SamplerKeyboard];

	for (int i = 0; i < KNOB_PAGE_SIZE; i++)
	{
		keyKnobValues[currentSamplerKey][i] = knobs[i];
	}

	displayValues[0] = knobs[0] * sampleLength;
	displayValues[1] = LEAF_clip(0.0f, knobs[1] * sampleLength, sampleLength * (1.0f - knobs[0]));
	displayValues[2] = (knobs[2] - 0.5f) * 4.0f;
	displayValues[3] = knobs[3] * 4000.0f;

	samplePlayStart = knobs[0] * recordPosition;
	samplePlayLength = knobs[1] * recordPosition;
	samplerRate = displayValues[2];
	crossfadeLength = displayValues[3];

	tSampler_setStart(&keySampler[currentSamplerKey], samplePlayStart);
	tSampler_setLength(&keySampler[currentSamplerKey], samplePlayLength);
	tSampler_setRate(&keySampler[currentSamplerKey], samplerRate);
	tSampler_setCrossfadeLength(&keySampler[currentSamplerKey], crossfadeLength);

	for (int i = 0; i < tSimplePoly_getNumVoices(&poly); ++i)
	{
		if (tSimplePoly_isOn(&poly, i) > 0)
		{
			int key = tSimplePoly_getPitch(&poly, i) - LOWEST_SAMPLER_KEY;
			if (0 <= key && key < NUM_SAMPLER_KEYS)
			{
				tBuffer_tick(&keyBuff[key], input[1]);
				sample += tSampler_tick(&keySampler[key]);
			}
		}
	}

	sample = tanhf(sample);
	input[0] = sample;
	input[1] = sample;
}

void SFXSamplerKFree(void)
{
	for (int i = 0; i < NUM_SAMPLER_KEYS; i++)
	{
		tBuffer_freeFromPool(&keyBuff[i], &largePool);
		tSampler_free(&keySampler[i]);
	}
}



//8 sampler - auto

volatile float currentPower = 0.0f;
volatile float previousPower = 0.0f;
float samp_thresh = 0.0f;
volatile int samp_triggered = 0;
uint32_t sample_countdown = 0;
PlayMode samplerMode = PlayLoop;
uint32_t powerCounter = 0;
uint8_t triggerChannel = 0;
uint8_t currentSampler = 0;
int pitchQuantization = 0;
int randLengthVal = 0;
float randRateVal = 0.0f;
tRamp asFade;

float lastSample = 0.0f;
int firstTime = 1;
tExpSmooth cfxSmooth;
#define MAX_AUTOSAMP_LENGTH 192000

void SFXSamplerAutoAlloc()
{
	tBuffer_initToPool(&asBuff[0], MAX_AUTOSAMP_LENGTH, &largePool);
	tBuffer_setRecordMode(&asBuff[0], RecordOneShot);
	tBuffer_initToPool(&asBuff[1], MAX_AUTOSAMP_LENGTH, &largePool);
	tBuffer_setRecordMode(&asBuff[1], RecordOneShot);
	tSampler_initToPool(&asSampler[0], &asBuff[0], &smallPool);
	tSampler_setMode(&asSampler[0], PlayLoop);
	tSampler_initToPool(&asSampler[1], &asBuff[1], &smallPool);
	tSampler_setMode(&asSampler[1], PlayLoop);

	tEnvelopeFollower_initToPool(&envfollow, 0.00001f, 0.9999f, &smallPool);
	tExpSmooth_initToPool(&cfxSmooth, 0.0f, 0.01f, &smallPool);

	setLED_A(samplerMode == PlayBackAndForth);
	setLED_B(triggerChannel);
	currentSampler = 1;
	sample_countdown = 0;
	randLengthVal = randomNumber() * 10000.0f;
	randRateVal = (randomNumber() - 0.5f) * 4.0f;

	setLED_C(pitchQuantization);
}

void SFXSamplerAutoFrame()
{
	if (buttonActionsSFX[ButtonC][ActionPress] == 1)
	{
		pitchQuantization = !pitchQuantization;
		buttonActionsSFX[ButtonC][ActionPress] = 0;
		setLED_C(pitchQuantization);
		firstTime = 1;
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


	if ((currentPower > (samp_thresh)) && (currentPower > previousPower + 0.001f) && (samp_triggered == 0) && (sample_countdown == 0) && (fadeDone == 1))
	{
		randLengthVal = (randomNumber() - 0.5f) * randLengthAmount * 2.0f;
		if (pitchQuantization) randRateVal = roundf(randomNumber() * randRateAmount) + 1.0f;
		else randRateVal = (randomNumber() - 0.5f) * randRateAmount * 2.0f;

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
	input[0] = sample;
	input[1] = sample;
	lastSample = sample;
}

void SFXSamplerAutoFree(void)
{
	tBuffer_freeFromPool(&asBuff[0], &largePool);
	tBuffer_freeFromPool(&asBuff[1], &largePool);
	tSampler_freeFromPool(&asSampler[0], &smallPool);
	tSampler_freeFromPool(&asSampler[1], &smallPool);
	tEnvelopeFollower_freeFromPool(&envfollow, &smallPool);
	tExpSmooth_freeFromPool(&cfxSmooth, &smallPool);
}

//10 distortion tanh
uint8_t distortionMode = 0;
tDiodeFilter dFilt;
tVZFilter shelf1;
tVZFilter shelf2;
tVZFilter bell1;
int distOS_ratio = 4;

void SFXDistortionAlloc()
{
	leaf.clearOnAllocation = 1;
	tOversampler_init(&oversampler, distOS_ratio, 0);
	tVZFilter_initToPool(&shelf1, Lowshelf, 80.0f, 6.0f, &smallPool);
	tVZFilter_initToPool(&shelf2, Highshelf, 12000.0f, 6.0f, &smallPool);
	tVZFilter_initToPool(&bell1, Bell, 1000.0f, 1.9f, &smallPool);
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
	tVZFilter_freeFromPool(&shelf1, &smallPool);
	tVZFilter_freeFromPool(&shelf2, &smallPool);
	tVZFilter_freeFromPool(&bell1, &smallPool);
}

// distortion wave folder


int foldMode = 0;

float oversampleBuf[2];

void SFXWaveFolderAlloc()
{
	leaf.clearOnAllocation = 1;
	tLockhartWavefolder_initToPool(&wavefolder1, &smallPool);
	tLockhartWavefolder_initToPool(&wavefolder2, &smallPool);
	tHighpass_initToPool(&wfHP, 10.0f, &smallPool);
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
	tLockhartWavefolder_freeFromPool(&wavefolder1, &smallPool);
	tLockhartWavefolder_freeFromPool(&wavefolder2, &smallPool);
	tHighpass_freeFromPool(&wfHP, &smallPool);
	tOversampler_free(&oversampler);
}

uint32_t crusherStereo = 0;
//13 bitcrusher
void SFXBitcrusherAlloc()
{
	tCrusher_initToPool(&crush, &smallPool);
	tCrusher_initToPool(&crush2, &smallPool);
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
	tCrusher_freeFromPool(&crush, &smallPool);
	tCrusher_freeFromPool(&crush2, &smallPool);
}


//delay
int delayShaper = 0;
uint8_t capFeedback = 0;

void SFXDelayAlloc()
{
	leaf.clearOnAllocation = 1;
	tTapeDelay_init(&delay, 2000, 30000);
	tTapeDelay_init(&delay2, 2000, 30000);
	tSVF_init(&delayLP, SVFTypeLowpass, 16000.f, .7f);
	tSVF_init(&delayHP, SVFTypeHighpass, 20.f, .7f);

	tSVF_init(&delayLP2, SVFTypeLowpass, 16000.f, .7f);
	tSVF_init(&delayHP2, SVFTypeHighpass, 20.f, .7f);

	tHighpass_init(&delayShaperHp, 20.0f);
	tFeedbackLeveler_init(&feedbackControl, .99f, 0.01, 0.125f, 0);
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
uint32_t freeze = 0;

tDattorroReverb reverb;
tExpSmooth sizeSmoother;

void SFXReverbAlloc()
{
	leaf.clearOnAllocation = 1;
	tDattorroReverb_init(&reverb);
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
	tNReverb_init(&reverb2, 1.0f);
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

//Living String
void SFXLivingStringAlloc()
{
	levMode = 0;
	tSimplePoly_setNumVoices(&poly, NUM_STRINGS);
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		myFreq = (randomNumber() * 300.0f) + 60.0f;
		myDetune[i] = (randomNumber() * 0.3f) - 0.15f;
		//tComplexLivingString_init(&theString[i],  myFreq, 0.4f, 0.0f, 16000.0f, .999f, .5f, .5f, 0.1f, 0);
		tComplexLivingString_init(&theString[i], 440.f, 0.8f, 0.3f, 0.f, 9000.f, 1.0f, 0.3f, 0.01f, 0.125f, levMode);
	}
	ignoreFreqKnobs = 0;
	setLED_A(ignoreFreqKnobs);
	setLED_B(levMode);
}

void SFXLivingStringFrame()
{
	if (buttonActionsSFX[ButtonA][ActionPress] == 1)
	{
		ignoreFreqKnobs = !ignoreFreqKnobs;
		buttonActionsSFX[ButtonA][ActionPress] = 0;
		setLED_A(ignoreFreqKnobs);
	}
	if (buttonActionsSFX[ButtonB][ActionPress] == 1)
	{
		levMode = !levMode;
		for (int i = 0; i < NUM_STRINGS; i++)
		{
			tComplexLivingString_setLevMode(&theString[i], levMode);
		}
		buttonActionsSFX[ButtonB][ActionPress] = 0;
		setLED_B(levMode);
	}
	displayValues[0] = LEAF_midiToFrequency((presetKnobValues[LivingString][0] * 90.0f)); //freq
	displayValues[1] = presetKnobValues[LivingString][1]; //detune
	displayValues[2] = presetKnobValues[LivingString][2]; //decay
	displayValues[3] = mtof((presetKnobValues[LivingString][3] * 130.0f)+12.0f); //lowpass
	displayValues[4] = (presetKnobValues[LivingString][4] * 0.48) + 0.5f;//pickPos
	displayValues[5] = (presetKnobValues[LivingString][5] * 0.48) + 0.02f;//prepPos
	displayValues[6] = ((tanhf((presetKnobValues[LivingString][6] * 8.0f) - 4.0f)) * 0.5f) + 0.5f;//prep Index

	displayValues[10] = LEAF_midiToFrequency((presetKnobValues[LivingString][10] * 90.0f)); //freq
	displayValues[11] = LEAF_midiToFrequency((presetKnobValues[LivingString][11] * 90.0f)); //freq
	displayValues[12] = LEAF_midiToFrequency((presetKnobValues[LivingString][12] * 90.0f)); //freq
	displayValues[13] = LEAF_midiToFrequency((presetKnobValues[LivingString][13] * 90.0f)); //freq
	displayValues[14] = LEAF_midiToFrequency((presetKnobValues[LivingString][14] * 90.0f)); //freq

	for (int i = 0; i < NUM_STRINGS; i++)
	{
		float freqVal = i == 0 ? displayValues[0] : displayValues[9+i];
		int note = tSimplePoly_getPitchAndCheckActive(&poly, i);
		if (note >= 0) freqVal = LEAF_midiToFrequency(note);
		tComplexLivingString_setFreq(&theString[i], (i + (1.0f+(myDetune[i] * displayValues[1]))) * freqVal);
		tComplexLivingString_setDecay(&theString[i], (displayValues[2] * 0.015f) + 0.995f);
		tComplexLivingString_setDampFreq(&theString[i], displayValues[3]);
		tComplexLivingString_setPickPos(&theString[i], displayValues[4]);
		tComplexLivingString_setPrepPos(&theString[i], displayValues[5]);
		tComplexLivingString_setPrepIndex(&theString[i], displayValues[6]);
	}
}


void SFXLivingStringTick(float* input)
{
	float sample = 0.0f;
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		float tick = tComplexLivingString_tick(&theString[i], input[1]);
		if ((ignoreFreqKnobs && tSimplePoly_isOn(&poly, i)) || !ignoreFreqKnobs)
		{
			sample += tick;
		}

	}
	sample *= 0.0625f;
	input[0] = sample;
	input[1] = sample;


}

void SFXLivingStringFree(void)
{
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		tComplexLivingString_free(&theString[i]);
	}
}


//Living String Synth
void SFXLivingStringSynthAlloc()
{
	levMode = 1;
	tSimplePoly_setNumVoices(&poly, NUM_STRINGS);
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		tComplexLivingString_init(&theString[i], 440.f, 0.2f, 0.3f, 0.f, 9000.f, 1.0f, 0.0f, 0.01f, 0.125f, levMode);
	}
	setLED_A(numVoices == 1);
	setLED_B(levMode);
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
		levMode = !levMode;
		for (int i = 0; i < NUM_STRINGS; i++)
		{
			tComplexLivingString_setLevMode(&theString[i], levMode);
		}
		buttonActionsSFX[ButtonB][ActionPress] = 0;
		setLED_B(levMode);
	}
	//displayValues[0] = mtof((smoothedADC[0] * 135.0f)); //freq
	//displayValues[1] = smoothedADC[1]; //detune
	displayValues[2] = ((presetKnobValues[LivingStringSynth][2] * 0.09999999f) + 0.9f);
	displayValues[3] = mtof((presetKnobValues[LivingStringSynth][3] * 130.0f)+12.0f); //lowpass
	displayValues[4] = (presetKnobValues[LivingStringSynth][4] * 0.5) + 0.02f;//pickPos
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		//tComplexLivingString_setFreq(&theString[i], (i + (1.0f+(myDetune[i] * knobParams[1]))) * knobParams[0]);
		tComplexLivingString_setDecay(&theString[i], displayValues[2]);
		tComplexLivingString_setDampFreq(&theString[i], displayValues[3]);
		tComplexLivingString_setPickPos(&theString[i], displayValues[4]);
	}

	for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
	{
		//tRamp_setDest(&polyRamp[i], (tPoly_getVelocity(&poly, i) > 0));
		calculateFreq(i);
		tComplexLivingString_setFreq(&theString[i], freq[i]);
		tComplexLivingString_setTargetLev(&theString[i],(tSimplePoly_getVelocity(&poly, i) > 0));
	}
}


void SFXLivingStringSynthTick(float* input)
{
	float sample = 0.0f;
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		sample += tComplexLivingString_tick(&theString[i], input[1]);
	}
	sample *= 0.0625f;
	input[0] = sample;
	input[1] = sample;
}

void SFXLivingStringSynthFree(void)
{
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		tComplexLivingString_free(&theString[i]);
	}
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
			tSawtooth_initToPool(&osc[(i * NUM_OSC_PER_VOICE) + j], &smallPool);
			synthDetune[i][j] = ((leaf.random() * 0.0264f) - 0.0132f);
			tRosenbergGlottalPulse_initToPool(&glottal[(i * NUM_OSC_PER_VOICE) + j], &smallPool);
			tRosenbergGlottalPulse_setOpenLength(&glottal[(i * NUM_OSC_PER_VOICE) + j], 0.3f);
			tRosenbergGlottalPulse_setPulseLength(&glottal[(i * NUM_OSC_PER_VOICE) + j], 0.4f);
		}

		tEfficientSVF_initToPool(&synthLP[i], SVFTypeLowpass, 6000.0f, displayValues[4], &smallPool);
		tADSR4_initToPool(&polyEnvs[i], displayValues[5], displayValues[6], displayValues[7], displayValues[8], decayExpBuffer, DECAY_EXP_BUFFER_SIZE, &smallPool);
		tADSR4_setLeakFactor(&polyEnvs[i],((1.0f - displayValues[9]) * 0.00005f) + 0.99995f);
		tADSR4_initToPool(&polyFiltEnvs[i], displayValues[10], displayValues[11], displayValues[12], displayValues[13], decayExpBuffer, DECAY_EXP_BUFFER_SIZE, &smallPool);
		tADSR4_setLeakFactor(&polyFiltEnvs[i], ((1.0f - displayValues[14]) * 0.00005f) + 0.99995f);

	}
	tCycle_initToPool(&pwmLFO1, &smallPool);
	tCycle_initToPool(&pwmLFO2, &smallPool);
	tCycle_setFreq(&pwmLFO1, 63.0f);
	tCycle_setFreq(&pwmLFO2, 72.11f);

	setLED_A(numVoices == 1);
	leaf.clearOnAllocation = 0;
	cycleCountVals[0][2] = 2;
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
		cycleCountVals[0][1] = 0;
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
			tSawtooth_freeFromPool(&osc[(i * NUM_OSC_PER_VOICE) + j], &smallPool);
			tRosenbergGlottalPulse_freeFromPool(&glottal[(i * NUM_OSC_PER_VOICE) + j], &smallPool);
		}
		tEfficientSVF_freeFromPool(&synthLP[i], &smallPool);
		tADSR4_freeFromPool(&polyEnvs[i], &smallPool);
		tADSR4_freeFromPool(&polyFiltEnvs[i], &smallPool);
	}

	tCycle_freeFromPool(&pwmLFO1, &smallPool);
	tCycle_freeFromPool(&pwmLFO2, &smallPool);
}



///FM RHODES ELECTRIC PIANO SYNTH


tCycle FM_sines[NUM_VOC_VOICES][6];
float FM_freqRatios[4][6] = {{1.0f, 1.00001f, 1.0f, 3.0f, 1.0f, 1.0f}, {2.0f, 2.0001f, .99999f, 3.0f, 5.0f, 8.0f},  {1.0f, 2.0f, 1.0f, 7.0f, 3.0f, 4.0f}, {1.0f, 2.0f, 1.0f, 7.0f, 3.0f, 4.0f}};
float FM_indices[4][6] = {{800.0f, 0.0f, 120.0f, 32.0f, 3.0f, 1.0f}, {100.0f, 100.0f, 300.0f, 300.0f, 10.0f, 5.0f}, {500.0f, 50.0f, 500.0f, 10.0f,0.0f, 0.0f}, {50.0f, 128.0f, 1016.0f, 528.0f, 4.0f, 0.0f}};
float FM_decays[4][6] = {{64.0f, 2000.0f, 3000.0f, 3400.0f, 3200.0f, 3100.0f}, {2000.0f, 300.0f, 800.0f, 3000.0f, 340.0f, 50.0f}, {20.0f, 50.0f, 50.0f, 10.0f, 30.0f, 20.0f}, {584.0f, 1016.0f, 1016.0f, 1000.0f, 600.0f, 500.0f}};
float FM_sustains[4][6] = {{0.9f, 0.9f, 0.9f, 0.8f, 0.7f, 0.7f}, {0.5f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f, 0.3f, 0.0f, 0.0f},{0.5f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f}};
float FM_attacks[4][6] = {{7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f}, {7.0f, 7.0f, 7.0f, 7.0f, 7.0f,7.0f}, {10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f},{1000.0f, 680.0f, 250.0f, 1300.0f, 750.0f, 820.0f}};
tADSR4 FM_envs[NUM_VOC_VOICES][6];
float feedback_output = 0.0f;
float prevDisplayValues[NUM_PRESET_KNOB_VALUES];
float panValues[NUM_VOC_VOICES];
tCycle tremolo;
uint8_t tremoloStereo = 0;

int Rsound = 0;

char* soundNames[4];



//FM Rhodes
void SFXRhodesAlloc()
{
	leaf.clearOnAllocation = 1;
	soundNames[0] = "DARK  ";
	soundNames[1] = "LIGHT ";
	soundNames[2] = "BASS  ";
	soundNames[3] = "PAD   ";

	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			tCycle_initToPool(&FM_sines[i][j], &smallPool);
			tADSR4_initToPool(&FM_envs[i][j], FM_attacks[Rsound][j], FM_decays[Rsound][j], FM_sustains[Rsound][j], 100.0f, decayExpBuffer, DECAY_EXP_BUFFER_SIZE, &smallPool);
			tADSR4_setLeakFactor(&FM_envs[i][j], 0.99998f);
		}
	}
	tCycle_initToPool(&tremolo, &smallPool);
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
		Rsound = (Rsound + 1 ) % 4; // switch to another rhodes preset sound
		OLEDclearLine(SecondLine);
		OLEDwriteString(soundNames[Rsound], 6, 0, SecondLine);
	}
	if (buttonActionsSFX[ButtonC][ActionPress] == 1)
	{
		tremoloStereo = !tremoloStereo;
		buttonActionsSFX[ButtonC][ActionPress] = 0;
		setLED_C(tremoloStereo == 1);
		OLEDclearLine(SecondLine);
		OLEDwriteString("STEREO TREMO", 12, 0, SecondLine);
		OLEDwriteInt(tremoloStereo, 1, 110, SecondLine);
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
							cycleCountVals[1][2] = 0;
							uint64_t tempCount1 = DWT->CYCCNT;
							tADSR4_setAttack(&FM_envs[i][j], displayValues[5] );
							uint64_t tempCount2 = DWT->CYCCNT;
							cycleCountVals[1][1] = tempCount2-tempCount1;
							CycleCounterTrackMinAndMax(1);
						}
					}
					break;
				case 6:
					for (int i = 0; i < NUM_VOC_VOICES; i++)
					{
						for (int j = 0; j < 6; j++)
						{
							tADSR4_setDecay(&FM_envs[i][j],displayValues[6]); //FM_decays[Rsound][j] * displayValues[6]);
						}
					}
					break;
				case 7:
					for (int i = 0; i < NUM_VOC_VOICES; i++)
					{
						for (int j = 0; j < 6; j++)
						{
							tADSR4_setSustain(&FM_envs[i][j], displayValues[7]); //FM_sustains[Rsound][j] * displayValues[7]);
						}
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
}


void SFXRhodesTick(float* input)
{
	float leftSample = 0.0f;
	float rightSample = 0.0f;

	tCycle_setFreq(&tremolo, displayValues[2]);

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
			tCycle_freeFromPool(&FM_sines[i][j],&smallPool);
			tADSR4_freeFromPool(&FM_envs[i][j],&smallPool);
		}

	}
	tCycle_freeFromPool(&tremolo,&smallPool);

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
float upperNearHyst = 0.0f;
float lowerNearHyst = 0.0f;


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
			upperNearHyst = (notes[upNote] - notes[lastNearNote]) * hysteresis;
			lowerNearHyst = (notes[lastNearNote] - notes[downNote]) * -hysteresis;

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


void noteOn(int key, int velocity)
{

	if (!velocity)
	{
		noteOff(key, velocity);
	}

	else
	{

		chordArray[key%12]++;

		int whichVoice = tSimplePoly_noteOn(&poly, key, velocity);

		if (currentPreset == AutotuneMono)
		{
			if (autotuneLock)
			{
				lockArray[key%12]++;
			}
			updatePitch = 1;
		}

		if (currentPreset == SamplerKeyboard)
		{
			if (key >= LOWEST_SAMPLER_KEY && key < LOWEST_SAMPLER_KEY + NUM_SAMPLER_KEYS)
			{
				currentSamplerKey = key - LOWEST_SAMPLER_KEY;
				setKnobValues(keyKnobValues[currentSamplerKey]);
				if (tBuffer_getRecordedLength(&keyBuff[currentSamplerKey]) == 0)
				{
					tBuffer_record(&keyBuff[currentSamplerKey]);
				}
				else tSampler_play(&keySampler[currentSamplerKey]);
				samplerKeyHeld[currentSamplerKey] = 1;
			}
		}



		if (whichVoice >= 0)
		{
			if (currentPreset == Rhodes)
			{
				for (int j = 0; j < 6; j++)
				{
					tADSR4_on(&FM_envs[whichVoice][j], velocity * 0.0078125f);
				}
				panValues[whichVoice] = key * 0.0078125; // divide by 128.0f
			}
			else if (currentPreset == ClassicSynth)
			{
				tADSR4_on(&polyEnvs[whichVoice], velocity * 0.0078125f);
				tADSR4_on(&polyFiltEnvs[whichVoice], velocity * 0.0078125f);
			}
		}
		setLED_2(1);
	}
}

void noteOff(int key, int velocity)
{

	if (chordArray[key%12] > 0) chordArray[key%12]--;

	if (currentPreset == SamplerKeyboard)
	{
		if (key >= LOWEST_SAMPLER_KEY && key < LOWEST_SAMPLER_KEY + NUM_SAMPLER_KEYS)
		{
			if (tBuffer_getRecordedLength(&keyBuff[key-LOWEST_SAMPLER_KEY]) == 0)
			{
				tBuffer_stop(&keyBuff[key-LOWEST_SAMPLER_KEY]);
				UISamplerKButtons(ButtonUp, ActionPress);
			}
			else tSampler_stop(&keySampler[key-LOWEST_SAMPLER_KEY]);
			samplerKeyHeld[key-LOWEST_SAMPLER_KEY] = 0;
			UISamplerKButtons(ButtonC, ActionHoldContinuous);
		}
	}


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

