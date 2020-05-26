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
float params[NUM_PRESET_KNOB_VALUES];
uint8_t knobActive[NUM_ADC_CHANNELS];

//audio objects
tFormantShifter fs;
tAutotune autotuneMono;
tAutotune autotunePoly;
tRetune retune;
tRetune retune2;
tRamp pitchshiftRamp;
tRamp nearWetRamp;
tRamp nearDryRamp;
tSimplePoly poly;
tRamp polyRamp[NUM_VOC_VOICES];

tRamp comp;

tBuffer buff;
tBuffer buff2;
tSampler sampler;

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


#define EXP_BUFFER_SIZE 128
float expBuffer[EXP_BUFFER_SIZE];
float expBufferSizeMinusOne = EXP_BUFFER_SIZE - 1;

#define DECAY_EXP_BUFFER_SIZE 512
float decayExpBuffer[DECAY_EXP_BUFFER_SIZE];
float decayExpBufferSizeMinusOne = DECAY_EXP_BUFFER_SIZE - 1;


#define NUM_STRINGS 6
tLivingString theString[NUM_STRINGS];

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
		tRamp_initToPool(&polyRamp[i], 10.0f, 1, &smallPool);
	}

	tRamp_init(&nearWetRamp, 10.0f, 1);
	tRamp_init(&nearDryRamp, 10.0f, 1);
	tRamp_init(&comp, 10.0f, 1);

	LEAF_generate_exp(expBuffer, 1000.0f, -1.0f, 0.0f, -0.0008f, EXP_BUFFER_SIZE);
	LEAF_generate_exp(decayExpBuffer, 0.001f, 0.0f, 1.0f, -0.0008f, DECAY_EXP_BUFFER_SIZE);
	/*
	 * 	knobParamNames[Vocoder][0] = "VOLUME";
	knobParamNames[Vocoder][1] = "WARP";
	knobParamNames[Vocoder][2] = "QUALITY";
	knobParamNames[Vocoder][3] = "SAWtoPULSE";
	knobParamNames[Vocoder][4] = "NOISE THRESH";
	knobParamNames[Vocoder][5] = "BREATHINESS";
	knobParamNames[Vocoder][6] = "PULSE WIDTH";
	knobParamNames[Vocoder][7] = "PULSE SHAPE";
	knobParamNames[Vocoder][8] = "";
	knobParamNames[Vocoder][9] = "";


	modeNames[VocoderCh] = "VOCODER CH";
	shortModeNames[VocoderCh] = "VC";
	modeNamesDetails[VocoderCh] = "";
	numPages[VocoderCh] = 2;
	knobParamNames[VocoderCh][0] = "VOLUME";
	knobParamNames[VocoderCh][1] = "WARP";
	knobParamNames[VocoderCh][2] = "QUALITY";
	knobParamNames[VocoderCh][3] = "BANDWIDTH";
	knobParamNames[VocoderCh][4] = "NOISE THRESH";
	knobParamNames[VocoderCh][5] = "SAWtoPULSE";
	knobParamNames[VocoderCh][6] = "PULSE WIDTH";
	knobParamNames[VocoderCh][7] = "PULSE SHAPE";
	knobParamNames[VocoderCh][8] = "BREATHINESS";
	knobParamNames[VocoderCh][9] = "SPEED";
	 */
	// Note that these are the actual knob values
	// not the parameter value
	// (i.e. 0.5 for fine pitch is actually 0.0 fine pitch)
	defaultPresetKnobValues[Vocoder][0] = 0.6f; // volume
	defaultPresetKnobValues[Vocoder][1] = 0.5f; // warp factor
	defaultPresetKnobValues[Vocoder][2] = 0.75f; // quality
	defaultPresetKnobValues[Vocoder][3] = 0.5f; // pulse length
	defaultPresetKnobValues[Vocoder][4] = 0.25f; // noise threshold
	defaultPresetKnobValues[Vocoder][5] = 0.0f; // breathiness
	defaultPresetKnobValues[Vocoder][6] = 0.5f; // pulse width
	defaultPresetKnobValues[Vocoder][7] = 0.5f; // pulse shape
	defaultPresetKnobValues[Vocoder][8] = 0.0f;
	defaultPresetKnobValues[Vocoder][9] = 0.0f;


	defaultPresetKnobValues[VocoderCh][0] = 0.6f; // volume
	defaultPresetKnobValues[VocoderCh][1] = 0.5f; // warp factor
	defaultPresetKnobValues[VocoderCh][2] = 1.0f; // quality
	defaultPresetKnobValues[VocoderCh][3] = 0.5f; //band width
	defaultPresetKnobValues[VocoderCh][4] = 0.25f; //noise thresh
	defaultPresetKnobValues[VocoderCh][5] = 0.0f;// saw->pulse fade
	defaultPresetKnobValues[VocoderCh][6] = 0.5f; // pulse length
	defaultPresetKnobValues[VocoderCh][7] = 0.5f; // pulse width
	defaultPresetKnobValues[VocoderCh][8] = 0.0f; // breathiness
	defaultPresetKnobValues[VocoderCh][9] = 0.5f; // envelope speed
	defaultPresetKnobValues[VocoderCh][10] = 0.5f;// squish
	defaultPresetKnobValues[VocoderCh][11] = 0.5f; // offset
	defaultPresetKnobValues[VocoderCh][12] = 0.0f; // tilt
	defaultPresetKnobValues[VocoderCh][13] = 0.0f; // stereo
	defaultPresetKnobValues[VocoderCh][14] = 0.5f; // odd-even


	defaultPresetKnobValues[Pitchshift][0] = 1.0f; // pitch
	defaultPresetKnobValues[Pitchshift][1] = 0.5f; // fine pitch
	defaultPresetKnobValues[Pitchshift][2] = 0.0f; // f amount
	defaultPresetKnobValues[Pitchshift][3] = 0.5f; // formant
	defaultPresetKnobValues[Pitchshift][4] = 0.0f;
	defaultPresetKnobValues[Pitchshift][5] = 0.0f;

	defaultPresetKnobValues[AutotuneMono][0] = 1.0f; // fidelity thresh
	defaultPresetKnobValues[AutotuneMono][1] = 1.0f; // amount
	defaultPresetKnobValues[AutotuneMono][2] = 1.0f; // speed
	defaultPresetKnobValues[AutotuneMono][3] = 0.0f;
	defaultPresetKnobValues[AutotuneMono][4] = 0.0f;
	defaultPresetKnobValues[AutotuneMono][5] = 0.0f;

	defaultPresetKnobValues[AutotunePoly][0] = 1.0f; // fidelity thresh
	defaultPresetKnobValues[AutotunePoly][1] = 0.5f;
	defaultPresetKnobValues[AutotunePoly][2] = 0.1f;
	defaultPresetKnobValues[AutotunePoly][3] = 0.0f;
	defaultPresetKnobValues[AutotunePoly][4] = 0.0f;
	defaultPresetKnobValues[AutotunePoly][5] = 0.0f;

	defaultPresetKnobValues[SamplerButtonPress][0] = 0.0f; // start
	defaultPresetKnobValues[SamplerButtonPress][1] = 1.0f; // end
	defaultPresetKnobValues[SamplerButtonPress][2] = 0.75f; // speed
	defaultPresetKnobValues[SamplerButtonPress][3] = 0.25f; // crossfade
	defaultPresetKnobValues[SamplerButtonPress][4] = 0.0f;
	defaultPresetKnobValues[SamplerButtonPress][5] = 0.0f;

	defaultPresetKnobValues[SamplerKeyboard][0] = 0.0f; // start
	defaultPresetKnobValues[SamplerKeyboard][1] = 1.0f; // end
	defaultPresetKnobValues[SamplerKeyboard][2] = 0.75f; // speed
	defaultPresetKnobValues[SamplerKeyboard][3] = 0.25f; // crossfade
	defaultPresetKnobValues[SamplerKeyboard][4] = 0.0f;
	defaultPresetKnobValues[SamplerKeyboard][5] = 0.0f;

	defaultPresetKnobValues[SamplerAutoGrab][0] = 0.95f; // thresh
	defaultPresetKnobValues[SamplerAutoGrab][1] = 0.5f; // window
	defaultPresetKnobValues[SamplerAutoGrab][2] = 0.0f; // rel thresh
	defaultPresetKnobValues[SamplerAutoGrab][3] = 0.25f; // crossfade
	defaultPresetKnobValues[SamplerAutoGrab][4] = 0.0f;
	defaultPresetKnobValues[SamplerAutoGrab][5] = 0.0f;

	defaultPresetKnobValues[Distortion][0] = .25f; // pre gain
	defaultPresetKnobValues[Distortion][1] = 0.5f; // tilt (low and high shelfs, opposing gains
	defaultPresetKnobValues[Distortion][2] = 0.5f; // mid gain
	defaultPresetKnobValues[Distortion][3] = 0.5f; // mid freq
	defaultPresetKnobValues[Distortion][4] = 0.25f; //post gain
	defaultPresetKnobValues[Distortion][5] = 0.0f;

	defaultPresetKnobValues[Wavefolder][0] = 0.25f; // gain
	defaultPresetKnobValues[Wavefolder][1] = 0.5f; // offset1
	defaultPresetKnobValues[Wavefolder][2] = 0.5f; // offset2
	defaultPresetKnobValues[Wavefolder][3] = 0.10f; // post gain
	defaultPresetKnobValues[Wavefolder][4] = 0.0f;
	defaultPresetKnobValues[Wavefolder][5] = 0.0f;

	defaultPresetKnobValues[BitCrusher][0] = 0.1f; // quality
	defaultPresetKnobValues[BitCrusher][1] = 0.5f; // samp ratio
	defaultPresetKnobValues[BitCrusher][2] = 0.0f; // rounding
	defaultPresetKnobValues[BitCrusher][3] = 0.0f; // operation
	defaultPresetKnobValues[BitCrusher][4] = 0.5f; // post gain
	defaultPresetKnobValues[BitCrusher][5] = 0.0f;

	defaultPresetKnobValues[Delay][0] = 0.25f; // delayL
	defaultPresetKnobValues[Delay][1] = 0.25f; // delayR
	defaultPresetKnobValues[Delay][2] = 0.5f; // feedback
	defaultPresetKnobValues[Delay][3] = 1.0f; // lowpass
	defaultPresetKnobValues[Delay][4] = 0.0f; // highpass
	defaultPresetKnobValues[Delay][5] = 0.0f;

	defaultPresetKnobValues[Reverb][0] = 0.5f; // size
	defaultPresetKnobValues[Reverb][1] = 0.5f; // in lowpass
	defaultPresetKnobValues[Reverb][2] = 0.5f; // in highpass
	defaultPresetKnobValues[Reverb][3] = 0.5f; // fb lowpass
	defaultPresetKnobValues[Reverb][4] = 0.5f; // fb gain
	defaultPresetKnobValues[Reverb][5] = 0.0f;

	defaultPresetKnobValues[Reverb2][0] = 0.2f; // size
	defaultPresetKnobValues[Reverb2][1] = 0.5f; // lowpass
	defaultPresetKnobValues[Reverb2][2] = 0.5f; // highpass
	defaultPresetKnobValues[Reverb2][3] = 0.5f; // peak freq
	defaultPresetKnobValues[Reverb2][4] = 0.5f; // peak gain
	defaultPresetKnobValues[Reverb2][5] = 0.0f;

	defaultPresetKnobValues[LivingString][0] = 0.5f; // freq
	defaultPresetKnobValues[LivingString][1] = 0.5f; // detune
	defaultPresetKnobValues[LivingString][2] = 0.5f; // decay
	defaultPresetKnobValues[LivingString][3] = 0.8f; // damping
	defaultPresetKnobValues[LivingString][4] = 0.5f; // pick pos
	defaultPresetKnobValues[LivingString][5] = 0.0f;

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

tTalkbox vocoder;
tNoise vocoderNoise;
tZeroCrossing zerox;
tSaw osc[NUM_VOC_VOICES * NUM_OSC_PER_VOICE];
tRosenbergGlottalPulse glottal[NUM_VOC_VOICES];
uint8_t numVoices = NUM_VOC_VOICES;
uint8_t internalExternal = 0;
tRamp noiseRamp;

void SFXVocoderAlloc()
{
	tTalkbox_init(&vocoder, 1024);
	tTalkbox_setWarpOn(&vocoder, 1);
	tNoise_initToPool(&vocoderNoise, WhiteNoise, &smallPool);
	tZeroCrossing_initToPool(&zerox, 64, &smallPool);
	tSimplePoly_setNumVoices(&poly, numVoices);
	tRamp_initToPool(&noiseRamp, 10, 1, &smallPool);
	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{

		tSaw_initToPool(&osc[i], &smallPool);

		tRosenbergGlottalPulse_initToPool(&glottal[i], &smallPool);
		tRosenbergGlottalPulse_setOpenLength(&glottal[i], 0.3f);
		tRosenbergGlottalPulse_setPulseLength(&glottal[i], 0.4f);
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

	for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
	{
		tRamp_setDest(&polyRamp[i], (tSimplePoly_getVelocity(&poly, i) > 0));
		calculateFreq(i);
		tSaw_setFreq(&osc[i], freq[i]);
		tRosenbergGlottalPulse_setFreq(&glottal[i], freq[i]);
	}

	if (tSimplePoly_getNumActiveVoices(&poly) != 0)
	{
		tRamp_setDest(&comp, 1.0f / tSimplePoly_getNumActiveVoices(&poly));
	}
	else
	{
		tRamp_setDest(&comp, 0.0f);
	}
}

void SFXVocoderTick(float audioIn)
{

	float zerocross = 0.0f;
	float noiseRampVal = 0.0f;
	float sample = 0.0f;
	// presetKnobValues are ALL the 0-1 values for each preset (numpresets*15), set from smoothedADC
	// in audiostream frame before sfx frame function are called, and in audiostream tick before sfx ticks are called

	// params are the presetKnobValues (15) after adjusting the range and should be used for setting objects

	// displayValues (5) are the values to be written to the screen and should depend on knobPage,
	// usually equal to params but sometimes different

	// ** we could replace params with local variables in each function, but the other two need to be global
	displayValues[3] = params[3] = presetKnobValues[Vocoder][3]; //pulse length

	displayValues[4] = params[4] = presetKnobValues[Vocoder][4]; //crossfade between sawtooth and glottal pulse

	if (internalExternal == 1) sample = rightIn;


	else
	{
		zerocross = tZeroCrossing_tick(&zerox, audioIn);

		//currently reusing the sawtooth/pulse fade knob for noise amount but need to separate these -JS
		if (zerocross > ((params[4])-0.1f))
		{
			tRamp_setDest(&noiseRamp, 1.0f);
		}
		else
		{
			tRamp_setDest(&noiseRamp, 0.0f);
		}

		noiseRampVal = tRamp_tick(&noiseRamp);

		float noiseSample = tNoise_tick(&vocoderNoise) * 0.8f * noiseRampVal;

		for (int i = 0; i < tSimplePoly_getNumActiveVoices(&poly); i++)
		{
			sample += tSaw_tick(&osc[i]) * tRamp_tick(&polyRamp[i]) * (1.0f-params[4]);

			tRosenbergGlottalPulse_setPulseLength(&glottal[i], params[3] );

			//tRosenbergGlottalPulse_setOpenLength(&glottal[i], smoothedADC[2] * smoothedADC[1]);
			sample += tRosenbergGlottalPulse_tick(&glottal[i]) * tRamp_tick(&polyRamp[i]) * params[4];
		}

		sample = (sample * (1.0f-noiseRampVal)) + noiseSample;
		sample *= tRamp_tick(&comp);
	}
	displayValues[0] = params[0] = presetKnobValues[Vocoder][0]; //vocoder volume

	displayValues[1] = params[1] = (presetKnobValues[Vocoder][1] * 0.4f) - 0.2f; //warp factor
	tTalkbox_setWarpFactor(&vocoder, params[1]);

	displayValues[2] = params[2] = (presetKnobValues[Vocoder][2] * 1.3f); //quality
	tTalkbox_setQuality(&vocoder, params[2]);

	sample = tTalkbox_tick(&vocoder, sample, audioIn);
	sample *= params[0] * 0.5f;
	sample = tanhf(sample);
	leftOut = sample;
	rightOut = sample;
}

void SFXVocoderFree(void)
{
	tTalkbox_free(&vocoder);
	tNoise_freeFromPool(&vocoderNoise, &smallPool);
	tZeroCrossing_freeFromPool(&zerox, &smallPool);
	tRamp_freeFromPool(&noiseRamp, &smallPool);
	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{
		tSaw_freeFromPool(&osc[i], &smallPool);
		tRosenbergGlottalPulse_freeFromPool(&glottal[i], &smallPool);
	}
}


#define MAX_NUM_VOCODER_BANDS 15
tVZFilter analysisBands[MAX_NUM_VOCODER_BANDS][2];
tVZFilter synthesisBands[MAX_NUM_VOCODER_BANDS][2];
tPowerFollower envFollowers[MAX_NUM_VOCODER_BANDS];
uint8_t numberOfVocoderBands = 15;
uint8_t prevNumberOfVocoderBands = 15;
float invNumberOfVocoderBands = 0.1f;
tNoise breathNoise;
tHighpass noiseHP;
float warpFactor = 1.0f;
int currentBandToAlter = 0;
int alteringBands = 0;

float myQ = 1.0f;
float prevMyQ = 1.0f;
float invMyQ = 1.0f;
float prevWarpFactor = 1.0f;
float bandGains[MAX_NUM_VOCODER_BANDS];

float bandWidthInSemitones;
float bandWidthInOctaves;  // divide by 12

float thisBandwidth;
float bandSquish = 1.0f;
float prevBandSquish = 1.0f;
float bandOffset = 30.0f;
float prevBandOffset = 30.0f;

void SFXVocoderChAlloc()
{
	invNumberOfVocoderBands = 1.0f / ((float)numberOfVocoderBands-0.99f);
	bandWidthInSemitones = 99.0f * invNumberOfVocoderBands;
	bandWidthInOctaves = bandWidthInSemitones * 0.083333333333333f;  // divide by 12
	thisBandwidth = bandWidthInOctaves * myQ;
	tOversampler_initToPool(&oversampler, 2, 0, &smallPool);
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
		//tSlide_initToPool(&envFollowers[i], 4800, 4800, &smallPool); //10ms logarithmic rise and fall at 48k sample rate
		tPowerFollower_initToPool(&envFollowers[i], 0.0005f, &smallPool); // factor of .001 is 10 ms?
	}
	tNoise_initToPool(&breathNoise, WhiteNoise, &smallPool);
	tNoise_initToPool(&vocoderNoise, WhiteNoise, &smallPool);
	tZeroCrossing_initToPool(&zerox, 16, &smallPool);
	tSimplePoly_setNumVoices(&poly, numVoices);
	tRamp_initToPool(&noiseRamp, 10, 1, &smallPool);
	tHighpass_initToPool(&noiseHP, 5000.0f, &smallPool);
	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{

		tSaw_initToPool(&osc[i], &smallPool);

		tRosenbergGlottalPulse_initToPool(&glottal[i], &smallPool);
		tRosenbergGlottalPulse_setOpenLength(&glottal[i], 0.3f);
		tRosenbergGlottalPulse_setPulseLength(&glottal[i], 0.4f);
	}
	setLED_A(numVoices == 1);
	setLED_B(internalExternal);
}



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

	for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
	{
		tRamp_setDest(&polyRamp[i], (tSimplePoly_getVelocity(&poly, i) > 0));
		calculateFreq(i);
		tSaw_setFreq(&osc[i], freq[i]);
		tRosenbergGlottalPulse_setFreq(&glottal[i], freq[i]);
	}

	displayValues[0] = presetKnobValues[VocoderCh][0]; //vocoder volume

	displayValues[1] = (presetKnobValues[VocoderCh][1] * 0.8f) - 0.4f; //warp factor

	displayValues[2] = (uint8_t)(presetKnobValues[VocoderCh][2] * 13.9f) + 2.0f; //quality

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


	warpFactor = 1.0f + displayValues[1];
	numberOfVocoderBands = displayValues[2];
	myQ = displayValues[3];
	bandSquish = displayValues[10];
	bandOffset = displayValues[11];

	if ((numberOfVocoderBands != prevNumberOfVocoderBands) || (myQ != prevMyQ) || (warpFactor != prevWarpFactor) || (bandSquish != prevBandSquish) || (bandOffset != prevBandOffset))
	{
		alteringBands = 1;
		invNumberOfVocoderBands = 1.0f / ((float)numberOfVocoderBands-0.99f);
		bandWidthInSemitones = 90.0f * bandSquish * invNumberOfVocoderBands;
		bandWidthInOctaves = bandWidthInSemitones * 0.083333333333333f;  // divide by 12
		thisBandwidth = bandWidthInOctaves * myQ;
		invMyQ = 1.0f / myQ;
	}
	if (alteringBands)
	{
		float bandFreq = faster_mtof((currentBandToAlter * bandWidthInSemitones) + bandOffset); //midinote 28 (41Hz) to midinote 134 (18814Hz) is 106 midinotes, divide that by how many bands to find out how far apart to put the bands
		if (bandFreq > 4500.0f) // a way to keep the upper bands fixed so consonants are not stretched even though vowels are
		{
			warpFactor = 1.0f;
		}
		//alternate computation
		//float bandFreq = 40.0f * powf(bandSpacing, ( (float)currentBandToAlter / (float)numberOfVocoderBands));

		if (bandFreq > 16000.0f)
		{
			bandFreq = 16000.0f;
		}
		bandGains[currentBandToAlter] = invMyQ;

		tVZFilter_setFreqAndBandwidth(&analysisBands[currentBandToAlter][0], bandFreq, thisBandwidth);
		//set these to match without computing for increased efficiency
		analysisBands[currentBandToAlter][1]->B = analysisBands[currentBandToAlter][0]->B;
		analysisBands[currentBandToAlter][1]->fc = analysisBands[currentBandToAlter][0]->fc;
		analysisBands[currentBandToAlter][1]->R2 = analysisBands[currentBandToAlter][0]->R2;
		analysisBands[currentBandToAlter][1]->cL = analysisBands[currentBandToAlter][0]->cL;
		analysisBands[currentBandToAlter][1]->cB = analysisBands[currentBandToAlter][0]->cB;
		analysisBands[currentBandToAlter][1]->cH = analysisBands[currentBandToAlter][0]->cH;
		analysisBands[currentBandToAlter][1]->h = analysisBands[currentBandToAlter][0]->h;
		analysisBands[currentBandToAlter][1]->g = analysisBands[currentBandToAlter][0]->g;

		//tVZFilter_setFreqAndBandwidth(&analysisBands[currentBandToAlter][1], bandFreq, thisBandwidth);
		tVZFilter_setFreqAndBandwidth(&synthesisBands[currentBandToAlter][0], bandFreq * warpFactor, thisBandwidth);
		//tVZFilter_setFreqAndBandwidth(&synthesisBands[currentBandToAlter][1], bandFreq * warpFactor, thisBandwidth);
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
		if (currentBandToAlter >= numberOfVocoderBands)
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

	for (int i = 0; i < numberOfVocoderBands; i++)
	{
		tPowerFollower_setFactor(&envFollowers[i], (displayValues[9] * 0.0012f) + 0.0001f);
	}

	if (tSimplePoly_getNumActiveVoices(&poly) != 0)
	{
		tRamp_setDest(&comp, 1.0f / tSimplePoly_getNumActiveVoices(&poly));
	}
	else
	{
		tRamp_setDest(&comp, 0.0f);
	}
}

float tempSamp = 0.0f;


//freeze (maybe C button?)
//odd even / stereo/mono (maybe a knob?)
//filter tilt (could be just gain tilt of existing band gains)

void SFXVocoderChTick(float audioIn)
{
	float zerocross = 0.0f;
	float noiseRampVal = 0.0f;
	float sample = 0.0f;

	if (internalExternal == 1)
	{
		sample = rightIn;
	}
	else
	{
		zerocross = tZeroCrossing_tick(&zerox, audioIn);


		if (zerocross > ((displayValues[4])-0.1f))
		{
			tRamp_setDest(&noiseRamp, 1.0f);
		}
		else
		{
			tRamp_setDest(&noiseRamp, 0.0f);
		}

		noiseRampVal = tRamp_tick(&noiseRamp);

		float noiseSample = tNoise_tick(&vocoderNoise) * 0.8f * noiseRampVal;

		for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
		{
			float tempRamp = tRamp_tick(&polyRamp[i]);
			if (tempRamp > 0.001f)
			{
				sample += tSaw_tick(&osc[i]) * tempRamp *  (1.0f-displayValues[5]);

				tRosenbergGlottalPulse_setPulseLength(&glottal[i], displayValues[6] );

				tRosenbergGlottalPulse_setOpenLength(&glottal[i], displayValues[6] * displayValues[7]);
				sample += tRosenbergGlottalPulse_tick(&glottal[i]) * tempRamp * displayValues[5];
			}
		}
		//switch with consonant noise
		sample = (sample * (1.0f - (0.3f * displayValues[8])) * (1.0f-noiseRampVal)) + noiseSample;
		//add breathiness
		sample += (tHighpass_tick(&noiseHP, tNoise_tick(&breathNoise)) * displayValues[8] * 4.0f);
		sample *= tRamp_tick(&comp);

	}

	sample = tanhf(sample);


	tempSamp = 0.0f;
	float output[2] = {0.0f, 0.0f};
	audioIn = audioIn * (displayValues[0] * 10.0f);
	for (int i = 0; i < numberOfVocoderBands; i++)
	{
		uint8_t oddEven = i % 2;
		tempSamp = tVZFilter_tickEfficient(&analysisBands[i][0], audioIn);
		tempSamp = tVZFilter_tickEfficient(&analysisBands[i][1], tempSamp);
		tempSamp = tPowerFollower_tick(&envFollowers[i], tempSamp);
		tempSamp = LEAF_clip(0.0f, tempSamp, 2.0f);
		float tempOut = tVZFilter_tickEfficient(&synthesisBands[i][0], sample);
		//output += (tVZFilter_tickEfficient(&synthesisBands[i][1], tempOut) * tempSamp * bandGains[i]);
		output[oddEven] += (tVZFilter_tickEfficient(&synthesisBands[i][1], tempOut) * tempSamp * bandGains[i]);
	}

	sample = tanhf((output[0] + (output[1] * (1.0f - displayValues[13]))) * (displayValues[0] * 10.0f));
	leftOut = sample;
	rightOut = tanhf((output[1] + (output[0] * (1.0f - displayValues[13]))) * (displayValues[0] * 10.0f));

	//sample = tOversampler_tick(&oversampler, sample, tanhf);
	//sample = tanhf(sample);


}

void SFXVocoderChFree(void)
{
	for (int i = 0; i < MAX_NUM_VOCODER_BANDS; i++)
	{
		tVZFilter_free(&analysisBands[i][0]);
		tVZFilter_free(&synthesisBands[i][1]);

		tVZFilter_free(&analysisBands[i][0]);
		tVZFilter_free(&synthesisBands[i][1]);



		//tSlide_initToPool(&envFollowers[i], 4800, 4800, &smallPool); //10ms logarithmic rise and fall at 48k sample rate
		tPowerFollower_freeFromPool(&envFollowers[i], &smallPool); // factor of .001 is 10 ms?
	}
	tNoise_freeFromPool(&breathNoise, &smallPool);
	tNoise_freeFromPool(&vocoderNoise, &smallPool);
	tZeroCrossing_freeFromPool(&zerox, &smallPool);
	tRamp_freeFromPool(&noiseRamp, &smallPool);
	tHighpass_freeFromPool(&noiseHP, &smallPool);
	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{
		tSaw_freeFromPool(&osc[i], &smallPool);
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


	tExpSmooth_init(&smoother1, 0.0f, 0.01f);
	tExpSmooth_init(&smoother2, 0.0f, 0.01f);
	tExpSmooth_init(&smoother3, 0.0f, 0.01f);
}

void SFXPitchShiftFrame()
{


}

void SFXPitchShiftTick(float audioIn)
{
	//pitchFactor = (smoothedADC[0]*3.75f)+0.25f;
	float sample = 0.0f;

	float myPitchFactorCoarse = (presetKnobValues[Pitchshift][0]*2.0f) - 1.0f;
	float myPitchFactorFine = ((presetKnobValues[Pitchshift][1]*2.0f) - 1.0f) * 0.1f;
	float myPitchFactorCombined = myPitchFactorFine + myPitchFactorCoarse;
	displayValues[0] = myPitchFactorCombined;
	displayValues[1] = myPitchFactorCombined;
	float myPitchFactor = fastexp2f(myPitchFactorCombined);
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



	float formantsample = tanhf(tFormantShifter_remove(&fs, audioIn));




	float* samples = tRetune_tick(&retune2, formantsample);
	formantsample = samples[0];
	sample = audioIn;
	samples = tRetune_tick(&retune, sample);
	sample = samples[0];

	formantsample = tanhf(tFormantShifter_add(&fs, formantsample)) * tExpSmooth_tick(&smoother2) ;
	sample = (sample * (tExpSmooth_tick(&smoother1))) +  formantsample;
	leftOut = sample;
	rightOut = sample;


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




//5 neartune
uint8_t autotuneChromatic = 0;

void SFXNeartuneAlloc()
{
	tAutotune_init(&autotuneMono, 1, 512, 256);
	calculateNoteArray();
	tExpSmooth_init(&neartune_smoother, 100.0f, .007f);
	setLED_A(autotuneChromatic);
}

void SFXNeartuneFrame()
{

	if ((tSimplePoly_getNumActiveVoices(&poly) != 0) || (autotuneChromatic == 1))
	{
		tRamp_setDest(&nearWetRamp, 1.0f);
		tRamp_setDest(&nearDryRamp, 0.0f);
	}
	else
	{
		tRamp_setDest(&nearWetRamp, 0.0f);
		tRamp_setDest(&nearDryRamp, 1.0f);
	}

	if (buttonActionsSFX[ButtonA][ActionPress])
	{
		autotuneChromatic = !autotuneChromatic;
		buttonActionsSFX[ButtonA][ActionPress] = 0;
		setLED_A(autotuneChromatic);
	}
}

void SFXNeartuneTick(float audioIn)
{
	float sample = 0.0f;

	displayValues[0] = 0.75f + (presetKnobValues[AutotuneMono][0] * 0.22f);
	tAutotune_setFidelityThreshold(&autotuneMono, displayValues[0]);

	displayValues[1] = LEAF_clip(0.0f, presetKnobValues[AutotuneMono][1] * 1.1f, 1.0f); // amount of forcing to new pitch
	displayValues[2] = presetKnobValues[AutotuneMono][2]; //speed to get to desired pitch shift
	tExpSmooth_setFactor(&neartune_smoother, (displayValues[2] * .01f));

	float detectedPeriod = tAutotune_getInputPeriod(&autotuneMono);
	if (detectedPeriod > 0.0f)
	{
		float detectedNote = LEAF_frequencyToMidi(leaf.sampleRate / detectedPeriod);
		float desiredSnap = nearestNote(detectedPeriod);

		float destinationNote = (desiredSnap * displayValues[1]) + (detectedNote * (1.0f - displayValues[0]));
		float destinationFreq = LEAF_midiToFrequency(destinationNote);
		tExpSmooth_setDest(&neartune_smoother, destinationFreq);
	}
	tAutotune_setFreq(&autotuneMono, tExpSmooth_tick(&neartune_smoother), 0);

	float* samples = tAutotune_tick(&autotuneMono, audioIn);
	//tAutotune_setFreq(&autotuneMono, leaf.sampleRate / nearestPeriod(tAutotune_getInputPeriod(&autotuneMono)), 0);
	sample = samples[0] * tRamp_tick(&nearWetRamp);
	sample += audioIn * tRamp_tick(&nearDryRamp); // crossfade to dry signal if no notes held down.
	leftOut = sample;
	rightOut = sample;
}



void SFXNeartuneFree(void)
{
	tAutotune_free(&autotuneMono);
	tExpSmooth_free(&neartune_smoother);
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
		tRamp_setDest(&polyRamp[i], (tSimplePoly_getVelocity(&poly, i) > 0));
	}
	int tempNumVoices = tSimplePoly_getNumActiveVoices(&poly);
	if (tempNumVoices != 0) tRamp_setDest(&comp, 1.0f / (float)tempNumVoices);
}

void SFXAutotuneTick(float audioIn)
{
	float sample = 0.0f;
	displayValues[0] = 0.5f + (presetKnobValues[AutotunePoly][0] * 0.47f);

	displayValues[1] = presetKnobValues[AutotunePoly][1];

	displayValues[2] = presetKnobValues[AutotunePoly][2];

	tAutotune_setFidelityThreshold(&autotunePoly, displayValues[0]);
	tAutotune_setAlpha(&autotunePoly, displayValues[1]);
	tAutotune_setTolerance(&autotunePoly, displayValues[2]);


	for (int i = 0; i < tSimplePoly_getNumVoices(&poly); ++i)
	{
		tAutotune_setFreq(&autotunePoly, freq[i], i);
	}

	float* samples = tAutotune_tick(&autotunePoly, audioIn);

	for (int i = 0; i < tSimplePoly_getNumVoices(&poly); ++i)
	{
		sample += samples[i] * tRamp_tick(&polyRamp[i]);
	}
	sample *= tRamp_tick(&comp);
	leftOut = sample;
	rightOut = sample;
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

void SFXSamplerBPTick(float audioIn)
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

	tBuffer_tick(&buff, audioIn);
	sample = tanhf(tSampler_tick(&sampler));
	leftOut = sample;
	rightOut = sample;
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

void SFXSamplerKTick(float audioIn)
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
				tBuffer_tick(&keyBuff[key], audioIn);
				sample += tSampler_tick(&keySampler[key]);
			}
		}
	}

	sample = tanhf(sample);
	leftOut = sample;
	rightOut = sample;
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
uint8_t firstTrigger = 0;

void SFXSamplerAutoAlloc()
{
	tBuffer_initToPool(&buff2, leaf.sampleRate * 2.0f, &largePool);
	tBuffer_setRecordMode(&buff2, RecordOneShot);
	tSampler_init(&sampler, &buff2);
	tSampler_setMode(&sampler, PlayLoop);
	tEnvelopeFollower_init(&envfollow, 0.05f, 0.9999f);
	setLED_A(samplerMode == PlayBackAndForth);
	setLED_B(triggerChannel);
	firstTrigger = 1;
}

void SFXSamplerAutoFrame()
{

}

void SFXSamplerAutoTick(float audioIn)
{
	float sample = 0.0f;
	if (triggerChannel > 0) currentPower = tEnvelopeFollower_tick(&envfollow, rightIn);
	else currentPower = tEnvelopeFollower_tick(&envfollow, audioIn);

	float* knobs = presetKnobValues[SamplerAutoGrab];

	samp_thresh = 1.0f - knobs[0];
	displayValues[0] = samp_thresh;
	int window_size = knobs[1] * 10000.0f;
	displayValues[1] = window_size;
	displayValues[3] = knobs[3] * 1000.0f;
	crossfadeLength = displayValues[3];

	tSampler_setCrossfadeLength(&sampler, crossfadeLength);

	if ((currentPower > (samp_thresh)) && (currentPower > previousPower + 0.001f) && (samp_triggered == 0) && (sample_countdown == 0))
	{
		samp_triggered = 1;
		setLED_1(1);
		tBuffer_record(&buff2);
		if (firstTrigger)
		{
			tSampler_play(&sampler);
			firstTrigger = 0;
		}
		buff2->recordedLength = buff2->bufferLength;
		sample_countdown = window_size + 24;//arbitrary extra time to avoid resampling while playing previous sample - better solution would be alternating buffers and crossfading
		powerCounter = 1000;
	}

	if (sample_countdown > 0)
	{
		sample_countdown--;
	}


	tSampler_setEnd(&sampler,window_size);
	tBuffer_tick(&buff2, audioIn);
	//on it's way down
	if (currentPower <= previousPower)
	{
		if (powerCounter > 0)
		{
			powerCounter--;
		}
		else if (samp_triggered == 1)
		{
			setLED_1(0);
			samp_triggered = 0;
		}
	}
	if (buttonActionsSFX[ButtonA][ActionPress])
	{
		if (samplerMode == PlayLoop)
		{
			tSampler_setMode(&sampler, PlayBackAndForth);
			samplerMode = PlayBackAndForth;
			setLED_A(1);
			buttonActionsSFX[ButtonA][ActionPress] = 0;
		}
		else if (samplerMode == PlayBackAndForth)
		{
			tSampler_setMode(&sampler, PlayLoop);
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
	sample = tSampler_tick(&sampler);
	leftOut = sample;
	rightOut = sample;
	previousPower = currentPower;
}

void SFXSamplerAutoFree(void)
{
	tBuffer_freeFromPool(&buff2, &largePool);
	tSampler_free(&sampler);
	tEnvelopeFollower_free(&envfollow);
}

//10 distortion tanh
uint8_t distortionMode = 0;
tDiodeFilter dFilt;
tVZFilter shelf1;
tVZFilter shelf2;
tVZFilter bell1;
int distOS_ratio = 2;
void SFXDistortionAlloc()
{
	leaf.clearOnAllocation = 1;
	tOversampler_init(&oversampler, distOS_ratio, OVERSAMPLER_HQ);
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

void SFXDistortionTick(float audioIn)
{
	//knob 0 = gain

	float sample = audioIn;
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
		oversamplerArray[i] = tanhf(oversamplerArray[i] * presetKnobValues[Distortion][4]);
	}
	sample = tOversampler_downsample(&oversampler, oversamplerArray);
	leftOut = sample;
	rightOut = sample;

	//sample = tOversampler_tick(&oversampler, sample, &tanhf);
}

void SFXDistortionFree(void)
{
	tOversampler_free(&oversampler);
	tVZFilter_freeFromPool(&shelf1, &smallPool);
	tVZFilter_freeFromPool(&shelf2, &smallPool);
	tVZFilter_freeFromPool(&bell1, &smallPool);
}

//12 distortion wave folder


int foldMode = 0;


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

void SFXWaveFolderTick(float audioIn)
{
	//knob 0 = gain
	float sample = audioIn;

	displayValues[0] = (presetKnobValues[Wavefolder][0] * 2.0f);

	displayValues[1] = (presetKnobValues[Wavefolder][1] * 2.0f) - 1.0f;

	displayValues[2] = (presetKnobValues[Wavefolder][2] * 2.0f) - 1.0f;
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
			oversamplerArray[i] *= displayValues[0] * 2.0f;
			oversamplerArray[i] = LEAF_tanh(oversamplerArray[i]);
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
			oversamplerArray[i] = LEAF_tanh(oversamplerArray[i]);
		}
		sample = tHighpass_tick(&wfHP, tOversampler_downsample(&oversampler, oversamplerArray)) * displayValues[3];
		leftOut = sample;
		rightOut = sample;
	}
	else
	{

		sample = sample + displayValues[1];
		sample *= displayValues[0] * 2.0f;
		sample = LEAF_tanh(sample);
		//oversamplerArray[i] *= knobParams[0] * 1.5f;

		sample = tLockhartWavefolder_tick(&wavefolder1, sample);



		sample = sample + displayValues[2];
		sample *= displayValues[0] * 2.0f;
		sample = LEAF_tanh(sample);
		//oversamplerArray[i] *= knobParams[0] * 1.5f;

		sample = tLockhartWavefolder_tick(&wavefolder2, sample);

		sample = tOversampler_tick(&oversampler, sample, &LEAF_tanh);
		sample = tHighpass_tick(&wfHP, sample) * displayValues[3];
		//sample *= 0.99f;
		leftOut = sample;
		rightOut = sample;
	}

}

void SFXWaveFolderFree(void)
{
	tLockhartWavefolder_freeFromPool(&wavefolder1, &smallPool);
	tLockhartWavefolder_freeFromPool(&wavefolder2, &smallPool);
	tHighpass_freeFromPool(&wfHP, &smallPool);
	tOversampler_free(&oversampler);
}


//13 bitcrusher
void SFXBitcrusherAlloc()
{
	tCrusher_init(&crush);
	tCrusher_init(&crush2);
}

void SFXBitcrusherFrame()
{
}

void SFXBitcrusherTick(float audioIn)
{
	float sample = 0.0f;
	displayValues[0] = presetKnobValues[BitCrusher][0];
	tCrusher_setQuality (&crush, presetKnobValues[BitCrusher][0]);
	tCrusher_setQuality (&crush2, presetKnobValues[BitCrusher][0]);
	displayValues[1] = presetKnobValues[BitCrusher][1];
	tCrusher_setSamplingRatio (&crush, presetKnobValues[BitCrusher][1]);
	tCrusher_setSamplingRatio (&crush2, presetKnobValues[BitCrusher][1]);
	displayValues[2] = presetKnobValues[BitCrusher][2];
	tCrusher_setRound (&crush, presetKnobValues[BitCrusher][2]);
	tCrusher_setRound (&crush2, presetKnobValues[BitCrusher][2]);
	displayValues[3] = presetKnobValues[BitCrusher][3];
	tCrusher_setOperation (&crush, presetKnobValues[BitCrusher][3]);
	tCrusher_setOperation (&crush2, presetKnobValues[BitCrusher][3]);
	displayValues[4] = presetKnobValues[BitCrusher][4];
	sample = tanh(tCrusher_tick(&crush, audioIn)) * displayValues[4];
	leftOut = sample;
	rightOut = tanh(tCrusher_tick(&crush2, rightIn)) * displayValues[4];

}

void SFXBitcrusherFree(void)
{
	tCrusher_free(&crush);
	tCrusher_free(&crush2);
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
}
float delayFB1;
float delayFB2;

void SFXDelayTick(float audioIn)
{
	float sample = 0.0f;
	displayValues[0] = presetKnobValues[Delay][0] * 30000.0f;
	displayValues[1] = presetKnobValues[Delay][1] * 30000.0f;
	displayValues[2] = capFeedback ? LEAF_clip(0.0f, presetKnobValues[Delay][2] * 1.1f, 0.9f) : presetKnobValues[Delay][2] * 1.1f;
	displayValues[3] = faster_mtof((presetKnobValues[Delay][3] * 128) + 10.0f);
	displayValues[4] = faster_mtof((presetKnobValues[Delay][4] * 128) + 10.0f);

	tSVF_setFreq(&delayLP, displayValues[3]);
	tSVF_setFreq(&delayLP2, displayValues[3]);
	tSVF_setFreq(&delayHP, displayValues[4]);
	tSVF_setFreq(&delayHP2, displayValues[4]);

	//swap tanh for shaper and add cheap fixed highpass after both shapers

	float input1, input2;

	if (delayShaper == 0)
	{
		input1 = tFeedbackLeveler_tick(&feedbackControl, tanhf(audioIn + (delayFB1 * displayValues[2])));
		input2 = tFeedbackLeveler_tick(&feedbackControl, tanhf(audioIn + (delayFB2 * displayValues[2])));
	}
	else
	{
		input1 = tFeedbackLeveler_tick(&feedbackControl, tHighpass_tick(&delayShaperHp, LEAF_shaper(audioIn + (delayFB1 * displayValues[2] * 0.5f), 0.5f)));
		input2 = tFeedbackLeveler_tick(&feedbackControl, tHighpass_tick(&delayShaperHp, LEAF_shaper(audioIn + (delayFB2 * displayValues[2] * 0.5f), 0.5f)));
	}
	tTapeDelay_setDelay(&delay, displayValues[0]);

	delayFB1 = tTapeDelay_tick(&delay, input1);

	tTapeDelay_setDelay(&delay2, displayValues[1]);

	delayFB2 = tTapeDelay_tick(&delay2, input2);

	delayFB1 = tSVF_tick(&delayLP, delayFB1) * 0.95f;
	delayFB2 = tSVF_tick(&delayLP2, delayFB2)* 0.95f;

	delayFB1 = tanhf(tSVF_tick(&delayHP, delayFB1)* 0.95f);
	delayFB2 = tanhf(tSVF_tick(&delayHP2, delayFB2)* 0.95f);

	sample = delayFB1;
	leftOut = sample;
	rightOut = delayFB2;

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

void SFXReverbAlloc()
{
	leaf.clearOnAllocation = 1;
	tDattorroReverb_init(&reverb);
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
	displayValues[1] = faster_mtof(presetKnobValues[Reverb][1]*135.0f);
	tDattorroReverb_setInputFilter(&reverb, displayValues[1]);
	displayValues[2] =  faster_mtof(presetKnobValues[Reverb][2]*128.0f);
	tDattorroReverb_setHP(&reverb, displayValues[2]);
	displayValues[3] = faster_mtof(presetKnobValues[Reverb][3]*135.0f);
	tDattorroReverb_setFeedbackFilter(&reverb, displayValues[3]);
}

void SFXReverbTick(float audioIn)
{
	float stereo[2];
	float sample = 0.0f;
	if (buttonActionsSFX[ButtonA][ActionPress])
	{
		if (freeze == 0)
		{
			freeze = 1;
			tDattorroReverb_setFreeze(&reverb, 1);
			setLED_1(1);
		}
		else
		{
			freeze = 0;
			tDattorroReverb_setFreeze(&reverb, 0);
			setLED_1(0);
		}
		buttonActionsSFX[ButtonA][ActionPress] = 0;
	}

	//tDattorroReverb_setInputDelay(&reverb, smoothedADC[1] * 200.f);
	audioIn *= 4.0f;
	displayValues[0] = presetKnobValues[Reverb][0];
	tDattorroReverb_setSize(&reverb, displayValues[0]);
	displayValues[4] = capFeedback ? LEAF_clip(0.0f, presetKnobValues[Reverb][4], 0.5f) : presetKnobValues[Reverb][4];
	tDattorroReverb_setFeedbackGain(&reverb, displayValues[4]);
	tDattorroReverb_tickStereo(&reverb, audioIn, stereo);
	sample = tanhf(stereo[0]) * 0.99f;
	leftOut = sample;
	rightOut = tanhf(stereo[1]) * 0.99f;
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


void SFXReverb2Tick(float audioIn)
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
		audioIn = 0.0f;
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

	if (buttonActionsSFX[ButtonA][ActionPress])
	{
		if (freeze == 0)
		{
			freeze = 1;
			setLED_1(1);
		}
		else
		{
			freeze = 0;
			setLED_1(0);
		}
		buttonActionsSFX[ButtonA][ActionPress] = 0;
	}


	tNReverb_tickStereo(&reverb2, audioIn, stereoOuts);
	float leftOut = tSVF_tick(&lowpass, stereoOuts[0]);
	leftOut = tSVF_tick(&highpass, leftOut);
	leftOut += tSVF_tick(&bandpass, leftOut) * displayValues[4];

	float rightOutTemp = tSVF_tick(&lowpass2, stereoOuts[1]);
	rightOutTemp = tSVF_tick(&highpass2, rightOutTemp);
	rightOutTemp += tSVF_tick(&bandpass, rightOutTemp) * displayValues[4];
	sample = tanhf(leftOut);
	leftOut = sample;
	rightOut = tanhf(rightOutTemp);

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


//Living String
void SFXLivingStringAlloc()
{
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		myFreq = (randomNumber() * 300.0f) + 60.0f;
		myDetune[i] = (randomNumber() * 0.3f) - 0.15f;
		//tLivingString_init(&theString[i],  myFreq, 0.4f, 0.0f, 16000.0f, .999f, .5f, .5f, 0.1f, 0);
		tLivingString_init(&theString[i], 440.f, 0.2f, 0.f, 9000.f, 1.0f, 0.3f, 0.01f, 0.125f, 0);
	}
}

void SFXLivingStringFrame()
{
	displayValues[0] = mtof((presetKnobValues[LivingString][0] * 135.0f)); //freq
	displayValues[1] = presetKnobValues[LivingString][1]; //detune
	displayValues[2] = ((presetKnobValues[LivingString][2] * 0.09999999f) + 0.9f);
	displayValues[3] = mtof((presetKnobValues[LivingString][3] * 130.0f)+12.0f); //lowpass
	displayValues[4] = (presetKnobValues[LivingString][4] * 0.5) + 0.02f;//pickPos
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		tLivingString_setFreq(&theString[i], (i + (1.0f+(myDetune[i] * displayValues[1]))) * displayValues[0]);
		tLivingString_setDecay(&theString[i], displayValues[2]);
		tLivingString_setDampFreq(&theString[i], displayValues[3]);
		tLivingString_setPickPos(&theString[i], displayValues[4]);
	}

}


void SFXLivingStringTick(float audioIn)
{
	float sample = 0.0f;
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		sample += tLivingString_tick(&theString[i], audioIn);
	}
	sample *= 0.0625f;
	leftOut = sample;
	rightOut = sample;


}

void SFXLivingStringFree(void)
{
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		tLivingString_free(&theString[i]);
	}
}


//Living String
void SFXLivingStringSynthAlloc()
{
	tSimplePoly_setNumVoices(&poly, NUM_STRINGS);
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		tLivingString_init(&theString[i], 440.f, 0.2f, 0.f, 9000.f, 1.0f, 0.0f, 0.01f, 0.125f, 1);
	}
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
	//displayValues[0] = mtof((smoothedADC[0] * 135.0f)); //freq
	//displayValues[1] = smoothedADC[1]; //detune
	displayValues[2] = ((presetKnobValues[LivingStringSynth][2] * 0.09999999f) + 0.9f);
	displayValues[3] = mtof((presetKnobValues[LivingStringSynth][3] * 130.0f)+12.0f); //lowpass
	displayValues[4] = (presetKnobValues[LivingStringSynth][4] * 0.5) + 0.02f;//pickPos
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		//tLivingString_setFreq(&theString[i], (i + (1.0f+(myDetune[i] * knobParams[1]))) * knobParams[0]);
		tLivingString_setDecay(&theString[i], displayValues[2]);
		tLivingString_setDampFreq(&theString[i], displayValues[3]);
		tLivingString_setPickPos(&theString[i], displayValues[4]);
	}

	for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
	{
		//tRamp_setDest(&polyRamp[i], (tPoly_getVelocity(&poly, i) > 0));
		calculateFreq(i);
		tLivingString_setFreq(&theString[i], freq[i]);
		tLivingString_setTargetLev(&theString[i],(tSimplePoly_getVelocity(&poly, i) > 0));
	}

}


void SFXLivingStringSynthTick(float audioIn)
{
	float sample = 0.0f;
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		sample += tLivingString_tick(&theString[i], audioIn);
	}
	sample *= 0.0625f;
	leftOut = sample;
	rightOut = sample;
}

void SFXLivingStringSynthFree(void)
{
	for (int i = 0; i < NUM_STRINGS; i++)
	{
		tLivingString_free(&theString[i]);
	}
}


// CLASSIC SUBTRACTIVE SYNTH

float synthMidiNotes[NUM_VOC_VOICES];
tEfficientSVF synthLP[NUM_VOC_VOICES];
uint16_t filtFreqs[NUM_VOC_VOICES];
tADSR4 polyEnvs[NUM_VOC_VOICES];

void SFXClassicSynthAlloc()
{
	tSimplePoly_setNumVoices(&poly, numVoices);
	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{
		for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
		{
			tSaw_initToPool(&osc[(i * NUM_OSC_PER_VOICE) + j], &smallPool);
			synthDetune[i][j] = ((leaf.random() * 0.5f) - 0.25f);
		}

		tEfficientSVF_initToPool(&synthLP[i], SVFTypeLowpass, 6000.0f, 0.8f, &smallPool);
		tADSR4_initToPool(&polyEnvs[i], 7.0f, 64.0f, 0.9f, 100.0f, decayExpBuffer, DECAY_EXP_BUFFER_SIZE, &smallPool);
		tADSR4_setLeakFactor(&polyEnvs[i], 0.999987f);
	}

	setLED_A(numVoices == 1);
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
		incrementPage();
		buttonActionsSFX[ButtonB][ActionPress] = 0;
		setLED_B(knobPage == 1);
	}

	float* knobs = presetKnobValues[ClassicSynth];

	params[0] = knobs[0]; //synth volume
	params[1] = knobs[1] * 4096.0f; //lowpass cutoff
	params[2] = knobs[2]; //keyfollow filter cutoff
	params[3] = knobs[3]; //detune
	params[4] = (knobs[4] * 2.0f) + 0.4f; //filter Q

	params[5] = (knobs[5] * 993.0f) + 7.0f; //att
	params[6] = (knobs[6] * 993.0f) + 7.0f; //dec
    params[7] = knobs[7]; //sus
    params[8] = (knobs[8] * 993.0f) + 7.0f; //rel
    params[9] = (knobs[9] > 0.98) ? 0.9985f : (((1.0f - knobs[9]*knobs[9]) * 0.0015f) + 0.9985f); //leak

    displayValues[0] = params[knobPage * KNOB_PAGE_SIZE];
    displayValues[1] = params[1 + (knobPage * KNOB_PAGE_SIZE)];
    displayValues[2] = params[2 + (knobPage * KNOB_PAGE_SIZE)];
    displayValues[3] = params[3 + (knobPage * KNOB_PAGE_SIZE)];
    displayValues[4] = params[4 + (knobPage * KNOB_PAGE_SIZE)];
	if (knobPage == 1) displayValues[4] = knobs[9];

	for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
	{
		float myMidiNote = calculateTunedMidiNote((float)tSimplePoly_getPitch(&poly, i));

		for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
		{
			tSaw_setFreq(&osc[(i * NUM_OSC_PER_VOICE) + j], LEAF_midiToFrequency(myMidiNote + (synthDetune[i][j] * params[3])));
		}
		float keyFollowFilt = myMidiNote * params[2] * 64.0f;
		float tempFreq = params[1] +  keyFollowFilt;
		tempFreq = LEAF_clip(0.0f, tempFreq, 4095.0f);

		filtFreqs[i] = (uint16_t) tempFreq;
		tEfficientSVF_setQ(&synthLP[i],params[4]);

		tADSR4_setAttack(&polyEnvs[i], params[5]);
		tADSR4_setDecay(&polyEnvs[i], params[6]);
		tADSR4_setSustain(&polyEnvs[i], params[7]);
		tADSR4_setRelease(&polyEnvs[i], params[8]);
		tADSR4_setLeakFactor(&polyEnvs[i], params[9]);
	}
}

//waveshaper?

void SFXClassicSynthTick(float audioIn)
{
	float sample = 0.0f;
	//if (tPoly_getNumActiveVoices(&poly) != 0) tRamp_setDest(&comp, 1.0f / tPoly_getNumActiveVoices(&poly));

	for (int i = 0; i < tSimplePoly_getNumVoices(&poly); i++)
	{
		float tempSample = 0.0f;
		float env = tADSR4_tick(&polyEnvs[i]);

		for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
		{
			tempSample += tSaw_tick(&osc[(i * NUM_OSC_PER_VOICE) + j]) * env;
		}
//		tempSample += tSawtooth_tick(&osc[i]) * amplitudeTemp;
//		tempSample += tSawtooth_tick(&osc[i + NUM_VOC_VOICES]) * amplitudeTemp;
//		tempSample += tSawtooth_tick(&osc[i] + (NUM_VOC_VOICES * 2)) * amplitudeTemp;
		tEfficientSVF_setFreq(&synthLP[i], filtFreqs[i]);
		sample += tEfficientSVF_tick(&synthLP[i], tempSample);
	}
	sample *= INV_NUM_OSC_PER_VOICE * params[0];


	sample = tanhf(sample);
	leftOut = sample;
	rightOut = sample;
}

void SFXClassicSynthFree(void)
{
	for (int i = 0; i < NUM_VOC_VOICES; i++)
	{
		for (int j = 0; j < NUM_OSC_PER_VOICE; j++)
		{
			tSaw_freeFromPool(&osc[(i * NUM_OSC_PER_VOICE) + j], &smallPool);
		}
		tEfficientSVF_freeFromPool(&synthLP[i], &smallPool);
		tADSR4_freeFromPool(&polyEnvs[i], &smallPool);
	}

}



///FM RHODES ELECTRIC PIANO SYNTH
//TODO: fix adsr/poly handling so that poly noteoffs are delayed until the voice sound is finished
//Add option to not reassign stolen voices in poly
//test ADSR4 using original table, seeing if flash read is faster than


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
	}
}


//TODO:
// scale all ADSR times between 0-1, generate an exponential curve and map knobs to that.
// then adapt preset sound timings to match the 0-1 knob values

//
void SFXRhodesTick(float audioIn)
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


	leftOut = leftSample;
	rightOut = rightSample;

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


void calculateFreq(int voice)
{
	float tempNote = (float)tSimplePoly_getPitch(&poly, voice);
	float tempPitchClass = ((((int)tempNote) - keyCenter) % 12 );
	float tunedNote = tempNote + centsDeviation[(int)tempPitchClass];
	freq[voice] = LEAF_midiToFrequency(tunedNote);
}

float calculateTunedMidiNote(float tempNote)
{
	float tempPitchClass = ((((int)tempNote) - keyCenter) % 12 );
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


float nearestNote(float period)
{
	float note = LEAF_frequencyToMidi(leaf.sampleRate / period);
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


void noteOn(int key, int velocity)
{

	if (!velocity)
	{
		noteOff(key, velocity);
	}

	else
	{

		chordArray[key%12]++;
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


		int whichVoice = tSimplePoly_noteOn(&poly, key, velocity);
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

	int voice = tSimplePoly_noteOff(&poly, key);
	if (voice >= 0)
	{
		//tRamp_setDest(&polyRamp[voice], 0.0f);
		if (currentPreset == Rhodes)
		{
			for (int j = 0; j < 6; j++)
			{
				tADSR4_off(&FM_envs[voice][j]);
			}
		}
		else if (currentPreset == ClassicSynth)
		{
			tADSR4_off(&polyEnvs[voice]);
		}
	}

	if (tSimplePoly_getNumActiveVoices(&poly) < 1)
	{
		setLED_2(0);
	}

}


void pitchBend(int data)
{
	//tPoly_setPitchBend(&poly, (data - 8192) * 0.000244140625f);
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

