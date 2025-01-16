/*
 * sfx.h
 *
 *  Created on: Dec 23, 2019
 *      Author: josnyder
 */
#ifndef SFX_H_
#define SFX_H_

#ifndef __cplusplus
//#include "audiostream.h"
#else
#include <JuceHeader.h>
#endif

#include "gfx.h"
#include "leaf.h"

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
        
#define MAX_VOCODER_FILTER_ORDER 2
#define MAX_NUM_VOCODER_BANDS 24

#define EXP_BUFFER_SIZE 128

#define DECAY_EXP_BUFFER_SIZE 512

#ifndef __cplusplus
#define NUM_STRINGS 6
#define NUM_STRINGS_SYNTH 5
#else
    #define NUM_STRINGS 8
   #define NUM_STRINGS_SYNTH 8
#endif

#define MAX_AUTOSAMP_LENGTH 192000

        // UI
#define NUM_ADC_CHANNELS 6
#define NUM_BUTTONS 10
#define NUM_PRESET_KNOB_VALUES 25
#define KNOB_PAGE_SIZE 5


#define numDist 6

#define NUM_CHARACTERS_PER_PRESET_NAME 16
        //

#ifndef __cplusplus
        extern char small_memory[SMALL_MEM_SIZE];
        extern char medium_memory[MED_MEM_SIZE] __ATTR_RAM_D1;
        extern char large_memory[LARGE_MEM_SIZE] __ATTR_SDRAM;
#endif

        //PresetNil is used as a counter for the size of the enum
        typedef enum _VocodecPresetType
        {
            Vocoder = 0,
            VocoderCh,
            Pitchshift,
            AutotuneMono,
            AutotunePoly,
            SamplerButtonPress,
            SamplerKeyboard,
            SamplerAutoGrab,
            Distortion,
            Wavefolder,
            BitCrusher,
            Delay,
            Reverb,
            Reverb2,
            LivingString,
            LivingStringSynth,
            ClassicSynth,
            Rhodes,
			Tape,
#ifdef __cplusplus
            WavetableSynth,
#endif
            PresetNil
        } VocodecPresetType;

        typedef enum _VocodecButton
        {
            ButtonEdit = 0,
            ButtonLeft,
            ButtonRight,
            ButtonDown,
            ButtonUp,
            ButtonA,
            ButtonB,
            ButtonC,
            ButtonD,
            ButtonE,
            ExtraMessage,
            ButtonNil
        } VocodecButton;

        typedef enum _ButtonAction
        {
            ActionPress = 0,
            ActionRelease,
            ActionHoldInstant,
            ActionHoldContinuous,
            ActionNil
        } ButtonAction;

        typedef enum _VocodecLightType
        {
            VocodecLightUSB = 0,
            VocodecLight1,
            VocodecLight2,
            VocodecLightA,
            VocodecLightB,
            VocodecLightC,
            VocodecLightEdit,
            VocodecLightIn1Meter,
            VocodecLightIn1Clip,
            VocodecLightIn2Meter,
            VocodecLightIn2Clip,
            VocodecLightOut1Meter,
            VocodecLightOut1Clip,
            VocodecLightOut2Meter,
            VocodecLightOut2Clip,
            VocodecLightNil
        } VocodecLightType;

        typedef struct _VocoderButtonParams
        {
            int numVoices;
            int internalExternal;
            int freeze;
        } VocoderButtonParams;

        typedef struct _VocoderChButtonParams
        {
            int numVoices;
            int internalExternal;
            int freeze;
        } VocoderChButtonParams;

        typedef struct _PitchShiftButtonParams
        {
            int _;
        } PitchShiftButtonParams;

        typedef struct _NeartuneButtonParams
        {
            int useChromatic;
            int lock;
        } NeartuneButtonParams;

        typedef struct _AutotuneButtonParams
        {
            int _;
        } AutotuneButtonParams;

        typedef struct _SamplerBPButtonParams
        {
            PlayMode playMode;
            int paused;
        } SamplerBPButtonParams;

        typedef struct _SamplerKButtonParams
        {
            int controlAllKeys;
        } SamplerKButtonParams;

        typedef struct _SamplerAutoButtonParams
        {
            PlayMode playMode;
            int triggerChannel;
            int quantizeRate;
        } SamplerAutoButtonParams;

        typedef struct _DistortionButtonParams
        {
            int mode;
        } DistortionButtonParams;

        typedef struct _WaveFolderButtonParams
        {
            int mode;
        } WaveFolderButtonParams;

        typedef struct _BitcrusherButtonParams
        {
            int stereo;
        } BitcrusherButtonParams;

        typedef struct _DelayButtonParams
        {
            int shaper;
            int uncapFeedback;
            int freeze;
        } DelayButtonParams;

        typedef struct _ReverbButtonParams
        {
            int uncapFeedback;
            int freeze;
        } ReverbButtonParams;

        typedef struct _Reverb2ButtonParams
        {
            int freeze;
        } Reverb2ButtonParams;

        typedef struct _LivingStringButtonParams
        {
            int ignoreFreqKnobs;
            int independentStrings;
            int feedback;
        } LivingStringButtonParams;
        
        typedef struct _LivingStringSynthButtonParams
        {
            int numVoices;
            int audioIn;
            int feedback;
        } LivingStringSynthButtonParams;
        
        typedef struct _ClassicSynthButtonParams
        {
            int numVoices;
        } ClassicSynthButtonParams;
        
        typedef struct _RhodesButtonParams
        {
            int numVoices;
            int sound;
            int tremoloStereo;
        } RhodesButtonParams;
        
        typedef struct _TapeButtonParams
        {
            int shaper;
            int uncapFeedback;
            int freeze;
        } TapeButtonParams;

        typedef struct _WavetableSynthButtonParams
        {
            int numVoices;
            int loadIndex;
        } WavetableSynthButtonParams;

        typedef struct _Vocodec Vocodec;
        struct _Vocodec
        {
            LEAF leaf;

            tMempool mediumPool;
            tMempool largePool;

            void (*allocFunctions[PresetNil])(Vocodec* vcd);
            void (*frameFunctions[PresetNil])(Vocodec* vcd);
            void (*tickFunctions[PresetNil])(Vocodec* vcd, float*);
            void (*freeFunctions[PresetNil])(Vocodec* vcd);

            float defaultPresetKnobValues[PresetNil][NUM_PRESET_KNOB_VALUES];
            float presetKnobValues[PresetNil][NUM_PRESET_KNOB_VALUES];
            int knobActive[NUM_ADC_CHANNELS];
            float prevDisplayValues[NUM_PRESET_KNOB_VALUES];

            VocoderButtonParams vocoderParams;
            VocoderChButtonParams vocoderChParams;
            PitchShiftButtonParams pitchShiftParams;
            NeartuneButtonParams neartuneParams;
            AutotuneButtonParams autotuneParams;
            SamplerBPButtonParams samplerBPParams;
            SamplerKButtonParams samplerKParams;
            SamplerAutoButtonParams samplerAutoParams;
            DistortionButtonParams distortionParams;
            WaveFolderButtonParams waveFolderParams;
            BitcrusherButtonParams bitcrusherParams;
            DelayButtonParams delayParams;
            ReverbButtonParams reverbParams;
            Reverb2ButtonParams reverb2Params;
            LivingStringButtonParams livingStringParams;
            LivingStringSynthButtonParams livingStringSynthParams;
            ClassicSynthButtonParams classicSynthParams;
            RhodesButtonParams rhodesParams;
            TapeButtonParams tapeParams;



            WavetableSynthButtonParams wavetableSynthParams;
            tWaveSynth waveSynth;
            float* loadedTables[4];
            int loadedTableSizes[4];
            void (*loadWav)(Vocodec* vcd);
            int attemptFileLoad;
            int newWavLoaded;
            char* loadedFilePaths[4];

            //audio objects
            tFormantShifter fs;
            tRetune autotuneMono;
            tRetune autotunePoly;
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

            float expBuffer[EXP_BUFFER_SIZE];
            float expBufferSizeMinusOne;

            float decayExpBuffer[DECAY_EXP_BUFFER_SIZE];
            float decayExpBufferSizeMinusOne;

            tComplexLivingString theString[NUM_STRINGS];
            tLivingString2 theString2[NUM_STRINGS_SYNTH];

            float myDetune[NUM_STRINGS];
            float synthDetune[NUM_VOC_VOICES][NUM_OSC_PER_VOICE];
            //control objects
            float notes[128];
            int chordArray[12];
            int chromaticArray[12];
            int lockArray[12];

            float freq[NUM_VOC_VOICES];

            float oversamplerArray[MAX_OVERSAMPLER_RATIO];

            ///1 vocoder internal poly

            tTalkboxFloat vocoder;
            tNoise vocoderNoise;
            tZeroCrossingCounter zerox;
            tSawtooth osc[NUM_VOC_VOICES * NUM_OSC_PER_VOICE];
            tRosenbergGlottalPulse glottal[NUM_VOC_VOICES * NUM_OSC_PER_VOICE];
            tExpSmooth noiseRamp;
            tNoise breathNoise;
            tHighpass noiseHP;
            tVZFilter shelf1;
            tVZFilter shelf2;

            tHighpass dcBlock1;

            float wfState;
            float invCurFB;

            tVZFilter analysisBands[MAX_NUM_VOCODER_BANDS][MAX_VOCODER_FILTER_ORDER];
            tVZFilter synthesisBands[MAX_NUM_VOCODER_BANDS][MAX_VOCODER_FILTER_ORDER];

            tExpSmooth envFollowers[MAX_NUM_VOCODER_BANDS];
            int numberOfVocoderBands;
            int prevNumberOfVocoderBands;
            float invNumberOfVocoderBands;

            int whichKnobThisTime;

            int currentBandToAlter;
            int analysisOrSynthesis;
            int alteringBands;

            float prevMyQ;
            float invMyQ;
            float prevWarpFactor;
            float bandGains[MAX_NUM_VOCODER_BANDS];

            float bandWidthInSemitones;
            float bandWidthInOctaves;  // divide by 12

            float thisBandwidth;
            float prevBandSquish;
            float prevBandOffset;
            float prevMyTilt;
            float prevBarkPull;

            tVZFilter vocodec_highshelf;

            float barkBandFreqs[24];
            float barkBandWidths[24];

            tHighpass chVocFinalHP1;
            tHighpass chVocFinalHP2;

            float oneMinusStereo;
            float chVocOutputGain;

            // pitch shift
            float pitchShiftRange;
            float pitchShiftOffset;

            //5 autotune mono
            float lastSnap;
            float detectedNote;
            float desiredSnap;
            float destinationNote;
            float destinationFactor;
            float factorDiff;
            float changeAmount;

            //6 autotune

            //7 sampler - button press
            int samplePlayStart;
            int samplePlayLength;
            float sampleLength;
            int crossfadeLength;
            float samplerRate;
            tExpSmooth startSmooth;
            tExpSmooth lengthSmooth;

            // keyboard sampler
            int currentSamplerKeyGlobal;
            int samplerKeyHeld[NUM_SAMPLER_KEYS];

            tExpSmooth kSamplerGains[NUM_SAMPLER_KEYS];
            int waitingForDeactivation[NUM_SAMPLER_VOICES];
            int prevSamplerKey;

            float samp_thresh;
            int detectedAttackPos[NUM_SAMPLER_KEYS];
            float sampleRates[NUM_SAMPLER_KEYS];
            float sampleRatesMult[NUM_SAMPLER_KEYS];
            int loopOns[NUM_SAMPLER_KEYS];
            float samplePlayStarts[NUM_SAMPLER_KEYS];
            float samplePlayLengths[NUM_SAMPLER_KEYS];
            float crossfadeLengths[NUM_SAMPLER_KEYS];
            float prevKnobs[6];

            //8 sampler - auto

            volatile float currentPower;
            volatile float previousPower;

            volatile int samp_triggered;
            uint32_t sample_countdown;
            int currentSampler;
            int randLengthVal;
            float randRateVal;

            tExpSmooth cfxSmooth;

            int fadeDone;
            int finalWindowSize;

            //10 distortion tanh
            tVZFilter bell1;
            int distOS_ratio;

            // distortion wave folder

            float oversampleBuf[2];

            //13 bitcrusher

            //delay

            float delayFB1;
            float delayFB2;

            //reverb

            tDattorroReverb reverb;
            tExpSmooth sizeSmoother;

            //reverb2

            tNReverb reverb2;
            tSVF lowpass;
            tSVF highpass;
            tSVF bandpass;
            tSVF lowpass2;
            tSVF highpass2;
            tSVF bandpass2;

            //Living String
            tExpSmooth stringGains[NUM_STRINGS];

            //Living String Synth

            tSlide stringOutEnvs[NUM_STRINGS_SYNTH];
            tSlide stringInEnvs[NUM_STRINGS_SYNTH];
            tADSRT pluckEnvs[NUM_STRINGS_SYNTH];
            tExpSmooth pickPosSmooth;
            tExpSmooth prepPosSmooth;
            tExpSmooth pickupPosSmooth;
            tNoise stringPluckNoise;
            tEnvelopeFollower prepEnvs[NUM_STRINGS_SYNTH];
            tVZFilter pluckFilt;
            float samplesPerMs;

            // CLASSIC SUBTRACTIVE SYNTH

            tEfficientSVF synthLP[NUM_VOC_VOICES];
            uint16_t filtFreqs[NUM_VOC_VOICES];
            tADSRT polyEnvs[NUM_VOC_VOICES];
            tADSRT polyFiltEnvs[NUM_VOC_VOICES];
            tCycle pwmLFO1;
            tCycle pwmLFO2;

            ///FM RHODES ELECTRIC PIANO SYNTH

            tCycle FM_sines[NUM_VOC_VOICES][6];
            float FM_freqRatios[5][6];
            float FM_indices[5][6];
            tADSRT FM_envs[NUM_VOC_VOICES][6];
            float feedback_output;

            float panValues[NUM_VOC_VOICES];
            tCycle tremolo;

            const char* soundNames[5];
            tExpSmooth susSmoothers[6];
            float prevKnobValues[25];
            float overtoneSnap;
            float randomDecays[6];
            float randomSustains[6];
            float sustainsFinal[6];

            //Tape Emulation

            tRamp reelSmooth;


            // midi functions

            float pitchBendValue;

            int lastNearNote;

            int newBuffer[NUM_SAMPLER_KEYS];


            // UI /////////

            uint16_t (*ADC_values)[NUM_ADC_CHANNELS];

            float floatADC[NUM_ADC_CHANNELS];
            float lastFloatADC[NUM_ADC_CHANNELS];
            float floatADCUI[NUM_ADC_CHANNELS];
            float adcHysteresisThreshold;
            tExpSmooth adc[6];
            float smoothedADC[6];

            uint8_t knobPage;
            uint8_t numPages[PresetNil];

            uint8_t buttonValues[NUM_BUTTONS]; // Actual state of the buttons
            uint8_t buttonValuesPrev[NUM_BUTTONS];
            uint8_t cleanButtonValues[NUM_BUTTONS]; // Button values after hysteresis
            uint32_t buttonHysteresis[NUM_BUTTONS];
            uint32_t buttonHysteresisThreshold;
            uint32_t buttonCounters[NUM_BUTTONS]; // How long a button has been in its current state
            uint32_t buttonHoldThreshold;
            uint32_t buttonHoldMax;
            //uint8_t buttonPressed[NUM_BUTTONS];
            //uint8_t buttonReleased[NUM_BUTTONS];

            int8_t writeKnobFlag;
            int8_t writeButtonFlag;
            int8_t writeActionFlag;

            const char* modeNames[PresetNil];
            const char* modeNamesDetails[PresetNil];
            const char* shortModeNames[PresetNil];

            const char* knobParamNames[PresetNil][NUM_PRESET_KNOB_VALUES];

            int8_t currentParamIndex;
            uint8_t orderedParams[8];

            uint8_t buttonActionsSFX[NUM_BUTTONS+1][ActionNil];
            uint8_t buttonActionsUI[NUM_BUTTONS+1][ActionNil];
            float displayValues[NUM_PRESET_KNOB_VALUES];
            int8_t cvAddParam[PresetNil];
            const char* (*buttonActionFunctions[PresetNil])(Vocodec*, VocodecButton, ButtonAction);

            VocodecPresetType currentPreset;
            VocodecPresetType previousPreset;
            uint8_t loadingPreset;

            int firstADCPass;

            int lightStates[VocodecLightNil];

            // TUNING

            float centsDeviation[12];
            uint32_t currentTuning;
            uint8_t keyCenter;

            // OLED

            unsigned char buffer[512];
            GFX theGFX;
            char oled_buffer[32];

        };
#ifndef __cplusplus
        extern Vocodec vocodec;
#endif
        
        void SFX_init(Vocodec* vcd, uint16_t (*ADC_values)[NUM_ADC_CHANNELS],
                      void (*loadFunction)(Vocodec* vcd));
        void initPresetParams(Vocodec* vcd);
        void initFunctionPointers(Vocodec* vcd);
        
        void initGlobalSFXObjects(Vocodec* vcd);
        void freeGlobalSFXObjects(Vocodec* vcd);
        
        //LPC Vocoder
        void SFXVocoderAlloc(Vocodec* vcd);
        void SFXVocoderFrame(Vocodec* vcd);
        void SFXVocoderTick(Vocodec* vcd, float* input);
        void SFXVocoderFree(Vocodec* vcd);
        
        //channel Vocoder
        void SFXVocoderChAlloc(Vocodec* vcd);
        void SFXVocoderChFrame(Vocodec* vcd);
        void SFXVocoderChTick(Vocodec* vcd, float* input);
        void SFXVocoderChFree(Vocodec* vcd);
        
        // pitch shift
        void SFXPitchShiftAlloc(Vocodec* vcd);
        void SFXPitchShiftFrame(Vocodec* vcd);
        void SFXPitchShiftTick(Vocodec* vcd, float* input);
        void SFXPitchShiftFree(Vocodec* vcd);
        
        // neartune
        void SFXNeartuneAlloc(Vocodec* vcd);
        void SFXNeartuneFrame(Vocodec* vcd);
        void SFXNeartuneTick(Vocodec* vcd, float* input);
        void SFXNeartuneFree(Vocodec* vcd);
        
        // autotune
        void SFXAutotuneAlloc(Vocodec* vcd);
        void SFXAutotuneFrame(Vocodec* vcd);
        void SFXAutotuneTick(Vocodec* vcd, float* input);
        void SFXAutotuneFree(Vocodec* vcd);
        
        // sampler - button press
        void SFXSamplerBPAlloc(Vocodec* vcd);
        void SFXSamplerBPFrame(Vocodec* vcd);
        void SFXSamplerBPTick(Vocodec* vcd, float* input);
        void SFXSamplerBPFree(Vocodec* vcd);
        
        // sampler - keyboard
        void SFXSamplerKAlloc(Vocodec* vcd);
        void SFXSamplerKFrame(Vocodec* vcd);
        void SFXSamplerKTick(Vocodec* vcd, float* input);
        void SFXSamplerKFree(Vocodec* vcd);
        
        // sampler - auto ch1
        void SFXSamplerAutoAlloc(Vocodec* vcd);
        void SFXSamplerAutoFrame(Vocodec* vcd);
        void SFXSamplerAutoTick(Vocodec* vcd, float* input);
        void SFXSamplerAutoFree(Vocodec* vcd);
        
        // distortion tanh
        void SFXDistortionAlloc(Vocodec* vcd);
        void SFXDistortionFrame(Vocodec* vcd);
        void SFXDistortionTick(Vocodec* vcd, float* input);
        void SFXDistortionFree(Vocodec* vcd);
        
        // distortion wave folder
        void SFXWaveFolderAlloc(Vocodec* vcd);
        void SFXWaveFolderFrame(Vocodec* vcd);
        void SFXWaveFolderTick(Vocodec* vcd, float* input);
        void SFXWaveFolderFree(Vocodec* vcd);
        
        // bitcrusher
        void SFXBitcrusherAlloc(Vocodec* vcd);
        void SFXBitcrusherFrame(Vocodec* vcd);
        void SFXBitcrusherTick(Vocodec* vcd, float* input);
        void SFXBitcrusherFree(Vocodec* vcd);
        
        // delay
        void SFXDelayAlloc(Vocodec* vcd);
        void SFXDelayFrame(Vocodec* vcd);
        void SFXDelayTick(Vocodec* vcd, float* input);
        void SFXDelayFree(Vocodec* vcd);
        
        // reverb
        void SFXReverbAlloc(Vocodec* vcd);
        void SFXReverbFrame(Vocodec* vcd);
        void SFXReverbTick(Vocodec* vcd, float* input);
        void SFXReverbFree(Vocodec* vcd);
        
        // reverb2
        void SFXReverb2Alloc(Vocodec* vcd);
        void SFXReverb2Frame(Vocodec* vcd);
        void SFXReverb2Tick(Vocodec* vcd, float* input);
        void SFXReverb2Free(Vocodec* vcd);
        
        // living string
        void SFXLivingStringAlloc(Vocodec* vcd);
        void SFXLivingStringFrame(Vocodec* vcd);
        void SFXLivingStringTick(Vocodec* vcd, float* input);
        void SFXLivingStringFree(Vocodec* vcd);
        
        // living string synth
        void SFXLivingStringSynthAlloc(Vocodec* vcd);
        void SFXLivingStringSynthFrame(Vocodec* vcd);
        void SFXLivingStringSynthTick(Vocodec* vcd, float* input);
        void SFXLivingStringSynthFree(Vocodec* vcd);
        
        // classic synth
        void SFXClassicSynthAlloc(Vocodec* vcd);
        void SFXClassicSynthFrame(Vocodec* vcd);
        void SFXClassicSynthTick(Vocodec* vcd, float* input);
        void SFXClassicSynthFree(Vocodec* vcd);
        
        // rhodes
        void SFXRhodesAlloc(Vocodec* vcd);
        void SFXRhodesFrame(Vocodec* vcd);
        void SFXRhodesTick(Vocodec* vcd, float* input);
        void SFXRhodesFree(Vocodec* vcd);
        
        // tape
        void SFXTapeAlloc(Vocodec* vcd);
        void SFXTapeFrame(Vocodec* vcd);
        void SFXTapeTick(Vocodec* vcd, float* input);
        void SFXTapeFree(Vocodec* vcd);

        // wavetable synth
        void SFXWavetableSynthAlloc(Vocodec* vcd);
        void SFXWavetableSynthFrame(Vocodec* vcd);
        void SFXWavetableSynthTick(Vocodec* vcd, float* input);
        void SFXWavetableSynthFree(Vocodec* vcd);

        // MIDI FUNCTIONS
        void noteOn(Vocodec* vcd, int key, int velocity);
        void noteOff(Vocodec* vcd, int key, int velocity);
        void pitchBend(Vocodec* vcd, int data);
        void sustainOn(Vocodec* vcd);
        void sustainOff(Vocodec* vcd);
        void toggleBypass(Vocodec* vcd);
        void toggleSustain(Vocodec* vcd);
        
        void calculateFreq(Vocodec* vcd, int voice);
        
        float calculateTunedMidiNote(Vocodec* vcd, float tempNote);
        
        
        void calculateNoteArray(Vocodec* vcd);
        float nearestNote(Vocodec* vcd, float period);
        float nearestNoteWithHysteresis(Vocodec* vcd, float note, float hysteresis);
        
        void clearNotes(Vocodec* vcd);
        
        void ctrlInput(Vocodec* vcd, int ctrl, int value);
        
        
#ifdef __cplusplus
    }
} // extern "C"
#endif


#endif /* SFX_H_ */
