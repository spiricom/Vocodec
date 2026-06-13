/*
 * audiostream.c
 *
 *  Created on: Aug 30, 2019
 *      Author: jeffsnyder
 */


/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"
#include "main.h"
#include "leaf.h"
#include "codec.h"
#include "ui.h"

#include "tunings.h"
#include "i2c.h"
#include "gpio.h"

#include "tim.h"
#include "usbh_MIDI.h"
#include "MIDI_application.h"
#include "synth.h"

//the audio buffers are put in the D2 RAM area because that is a memory location that the DMA has access to.
int32_t audioOutBuffer[AUDIO_BUFFER_SIZE] __ATTR_RAM_D2_DMA;
int32_t audioInBuffer[AUDIO_BUFFER_SIZE] __ATTR_RAM_D2_DMA;

char small_memory[SMALL_MEM_SIZE];
char medium_memory[MED_MEM_SIZE] __ATTR_RAM_D1;
char large_memory[LARGE_MEM_SIZE] __ATTR_SDRAM;
tMempool mediumPool;
tMempool largePool;


HAL_StatusTypeDef transmit_status;
HAL_StatusTypeDef receive_status;


uint32_t codecReady = 0;

uint32_t frameCounter = 0;

volatile uint32_t newPluck = 0 ;

tOversampler downSampler;

BOOL bufferCleared = TRUE;

uint32_t retrigMode = 0;
uint32_t retrigHappened = 0;

float mtofTable[MTOF_TABLE_SIZE]__ATTR_RAM_D2;

float atoDbTable[ATODB_TABLE_SIZE]__ATTR_RAM_D2;
float dbtoATable[DBTOA_TABLE_SIZE]__ATTR_RAM_D2;


float atodbTableScalar;
float atodbTableOffset;
float dbtoaTableScalar;
float dbtoaTableOffset;

uint32_t ccIn[8];
volatile uint8_t knobFrozen[20];
tExpSmooth knobSmoothers[20];
uint32_t resetStringInputs = 0;


void audioFrame(uint16_t buffer_offset);
uint32_t audioTick(float* samples);


uint32_t clipCounter[4] = {0,0,0,0};
uint32_t clipped[4] = {0,0,0,0};
uint32_t clipHappened[4] = {0,0,0,0};

uint8_t currentMIDINote = 60;

union breakFloat{
	float f;
	uint8_t b[4];
};
//envelope tables
float decayExpBuffer[DECAY_EXP_BUFFER_SIZE];
float decayExpBufferSizeMinusOne;


LEAF leaf;
tExpSmooth adc[6];
float frameLoadPercentage = 0.0f;
float frameMult = 1.0f / (AUDIO_FRAME_SIZE * 10000.0f);
uint32_t frameLoadOverCount = 0;


float volumePedal  = 0.0f;
float masterVolFromBrain = 0.5f;
float masterVolFromBrainForSynth = 0.25f;

volatile float stringMIDIPitches[NUM_STRINGS_PER_BOARD];
float knobScaled[20];
volatile uint8_t knobFrozen[20];
float pedalScaled[10];

tSimplePoly myPoly;

/**********************************************/

LEAFErrorType errorTypes = 0;

void LEAF_myError(LEAF* const, LEAFErrorType theError)
{
	errorTypes = theError;
}static float FORCE_INLINE aToDbTableLookup(float in)
{
    in = fastabsf(in);
    float floatIndex = LEAF_clip (0, (in * atodbTableScalar) - atodbTableOffset, ATODB_TABLE_SIZE_MINUS_ONE);
    uint32_t inAmpIndex = (uint32_t) floatIndex;
    uint32_t inAmpIndexPlusOne = inAmpIndex + 1;
    if (inAmpIndexPlusOne > ATODB_TABLE_SIZE_MINUS_ONE)
    {
    	inAmpIndexPlusOne = ATODB_TABLE_SIZE_MINUS_ONE;
    }
    float alpha = floatIndex - (float)inAmpIndex;
    return ((atoDbTable[inAmpIndex] * (1.0f - alpha)) + (atoDbTable[inAmpIndexPlusOne] * alpha));
}

static float FORCE_INLINE aToDbTableLookupFast(float in)
{
    in = fastabsf(in);
    uint32_t inAmpIndex = LEAF_clip (0, (in * atodbTableScalar) - atodbTableOffset, ATODB_TABLE_SIZE_MINUS_ONE);
    return atoDbTable[inAmpIndex];
}

float FORCE_INLINE dbToATableLookup(float in)
{
    float floatIndex = LEAF_clip (0, (in * dbtoaTableScalar) - dbtoaTableOffset, DBTOA_TABLE_SIZE_MINUS_ONE);
    uint32_t inDBIndex = (uint32_t) floatIndex;
    uint32_t inDBIndexPlusOne = inDBIndex + 1;
    if (inDBIndexPlusOne > DBTOA_TABLE_SIZE_MINUS_ONE)
    {
    	inDBIndexPlusOne = DBTOA_TABLE_SIZE_MINUS_ONE;
    }
    float alpha = floatIndex - (float)inDBIndex;
    return ((dbtoATable[inDBIndex] * (1.0f - alpha)) + (dbtoATable[inDBIndexPlusOne] * alpha));
}

static float FORCE_INLINE dbToATableLookupFast(float in)
{
    uint32_t inDBIndex = LEAF_clip (0, (in * dbtoaTableScalar) - dbtoaTableOffset, DBTOA_TABLE_SIZE_MINUS_ONE);
    return dbtoATable[inDBIndex];
}


float FORCE_INLINE mtofTableLookup(float tempMIDI)
{
	float tempIndexF = ((LEAF_clip(-163.0f, tempMIDI, 163.0f) * 100.0f) + 16384.0f);
	int tempIndexI = (int)tempIndexF;
	tempIndexF = tempIndexF -tempIndexI;
	float freqToSmooth1 = mtofTable[tempIndexI & 32767];
	float freqToSmooth2 = mtofTable[(tempIndexI + 1) & 32767];
	return ((freqToSmooth1 * (1.0f - tempIndexF)) + (freqToSmooth2 * tempIndexF));
}

void processKnobs()
{
	for (int i = 0; i < 8; i++)
	{
		int32_t newByte = ccIn[i] << 1;

		if (prevKnobByte[i] == 256)
		{
			prevKnobByte[i] = newByte;
		}
		else if (knobFrozen[i])
		{
			if ((newByte > (prevKnobByte[i] + 3)) || (newByte < (prevKnobByte[i] - 3)))
			{
				knobFrozen[i] = 0;
				prevKnobByte[i] = newByte;
			}
		}
		else
		{
			tExpSmooth_setDest(knobSmoothers[i], (newByte * 0.003921568627451f)); //scaled 0.0 to 1.0
			prevKnobByte[i] = newByte;
		}

	}

	for (int i = 8; i < 12; i++)
	{
		int32_t newByte = ADC_values[i-8] >> 8;
		if (prevKnobByte[i] == 256)
		{
			prevKnobByte[i] = newByte;
		}
		else if (knobFrozen[i])
		{
			if ((newByte > (prevKnobByte[i] + 3)) || (newByte < (prevKnobByte[i] - 3)))
			{
				knobFrozen[i] = 0;
				prevKnobByte[i] = newByte;
			}

		}
		else
		{
			tExpSmooth_setDest(knobSmoothers[i], (newByte * 0.003921568627451f)); //scaled 0.0 to 1.0
			prevKnobByte[i] = newByte;
		}

	}

}

void audioInit(I2C_HandleTypeDef* hi2c, SAI_HandleTypeDef* hsaiOut, SAI_HandleTypeDef* hsaiIn)
{
	// Initialize LEAF.

	LEAF_init(&leaf, SAMPLE_RATE, small_memory, SMALL_MEM_SIZE, &randomNumber);

	LEAF_setErrorCallback(&leaf, LEAF_myError);

	tMempool_init (&mediumPool, medium_memory, MED_MEM_SIZE, &leaf);

	tMempool_init (&largePool, large_memory, LARGE_MEM_SIZE, &leaf);

	//synthInit();

	//ramps to smooth the knobs

	for (int i = 0; i < 6; i++)
	{
		tExpSmooth_init(&adc[i],0.0f, 0.3f,&leaf);
	}

	for (int i = 0; i < 20; i++)
		{
			tExpSmooth_init(&knobSmoothers[i],0.0f,0.001f, &leaf);
		}
		for (int i = 0; i < 10; i++)
		{
			//tExpSmooth_init(&pedalSmoothers[i],0.0f,0.001f,&leaf);
		}
		tSimplePoly_init(&myPoly, 1, &leaf);
	LEAF_generate_exp(decayExpBuffer, 0.001f, 0.0f, 1.0f, -0.0008f, DECAY_EXP_BUFFER_SIZE); // exponential decay buffer falling from 1 to 0
		decayExpBufferSizeMinusOne = DECAY_EXP_BUFFER_SIZE - 1;

		LEAF_generate_atodb(atoDbTable, ATODB_TABLE_SIZE, 0.00001f, 1.0f);
		LEAF_generate_dbtoa(dbtoATable, DBTOA_TABLE_SIZE, -90.0f, 50.0f);

		atodbTableScalar = ATODB_TABLE_SIZE_MINUS_ONE/(1.0f-0.00001f);
		atodbTableOffset = 0.00001f * atodbTableScalar;
		dbtoaTableScalar = DBTOA_TABLE_SIZE_MINUS_ONE/(50.0f+90.0f);
		dbtoaTableOffset = -90.0f * dbtoaTableScalar;

		LEAF_generate_mtof(mtofTable, -163.8375f, 163.8375f,  MTOF_TABLE_SIZE); //mtof table for fast calc
	HAL_Delay(10);




	for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
	{

			audioOutBuffer[ i] = (int32_t)(0.0f * TWO_TO_23);
	}
	audioInitSynth();
	audioSwitchToSynth();
	HAL_Delay(1);

	// set up the I2S driver to send audio data to the codec (and retrieve input as well)
	transmit_status = HAL_SAI_Transmit_DMA(hsaiOut, (uint8_t *)&audioOutBuffer[0], AUDIO_BUFFER_SIZE);
	receive_status = HAL_SAI_Receive_DMA(hsaiIn, (uint8_t *)&audioInBuffer[0], AUDIO_BUFFER_SIZE);

	// with the CS4271 codec IC, the SAI Transmit and Receive must be happening before the chip will respond to
	// I2C setup messages (it seems to use the masterclock input as it's own internal clock for i2c data, etc)
	// so while we used to set up codec before starting SAI, now we need to set up codec afterwards, and set a flag to make sure it's ready
	//now to send all the necessary messages to the codec
	AudioCodec_init(hi2c);
	HAL_Delay(1);


	//now reconfigue so buttons C and E can be used (they were also connected to I2C for codec setup)
	//HAL_I2C_MspDeInit(hi2c);

	//GPIO_InitTypeDef GPIO_InitStruct = {0};

    //PB10, PB11     ------> buttons C and E
    //GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    //GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    //GPIO_InitStruct.Pull = GPIO_PULLUP;
    //HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

volatile int frameCount = 0;
volatile float frameLoad = 0.0f;

volatile float frameMax = 0.0f;
volatile int setFrameMax = 1;

volatile int freeCheck = 0;
volatile uint32_t overrun = 0;
void audioFrame(uint16_t buffer_offset)
{
	volatile uint32_t tempCount5 = DWT->CYCCNT;

	if (codecReady)
	{

		//volatile uint32_t tempCount5 = 0;
		//volatile uint32_t tempCount6 = 0;
		int i;
		//int32_t current_sample;
		uint32_t clipCatcher = 0;

		//tempCount5 = DWT->CYCCNT;


		//adcCheck(&vocodec);

		// if the USB write pointer has advanced (indicating unread data is in the buffer),
		// or the overflow bit is set, meaning that the write pointer wrapped around and the read pointer hasn't caught up to it yet
		// then process that new data this frame
		if ((myUSB_FIFO_overflowBit) || (myUSB_FIFO_writePointer > myUSB_FIFO_readPointer))
		{
			ProcessReceivedMidiDatas();
		}
		if (presetReady)
		{
			//cycleCountVals[1][2] = 0;
			processKnobs();
			audioFrameSynth(buffer_offset);
		}

#if 0

		if (!vocodec.loadingPreset)
		{

			for (int i = 0; i < NUM_ADC_CHANNELS; i++)
			{
				vocodec.smoothedADC[i] = LEAF_clip(0.0f, tExpSmooth_tick(vocodec.adc[i]), 1.0f);
			}

		}

		//if the codec isn't ready, keep the buffer as all zeros
		//otherwise, start computing audio!

		bufferCleared = TRUE;



		for (i = 0; i < (HALF_BUFFER_SIZE); i += 2)
		{
			float theSamples[2];
			theSamples[0] = ((float)(audioInBuffer[buffer_offset + i] << 8)) * INV_TWO_TO_31;
			theSamples[1] = ((float)(audioInBuffer[buffer_offset + i + 1] << 8)) * INV_TWO_TO_31;

			clipCatcher |= audioTick(theSamples);
			audioOutBuffer[buffer_offset + i] = (int32_t)(theSamples[1] * TWO_TO_23);
			audioOutBuffer[buffer_offset + i + 1] = (int32_t)(theSamples[0] * TWO_TO_23);
		}

		if (!vocodec.loadingPreset)
		{
			bufferCleared = 0;
		}



		if (bufferCleared)
		{
			//
		}


		for (int i = 0; i < 4; i++)
		{
			if ((clipCatcher >> i) & 1)
			{
				switch (i)
				{
					case 0:
						setLED_leftin_clip(&vocodec, 1);
						break;
					case 1:
						setLED_rightin_clip(&vocodec, 1);
						break;
					case 2:
						setLED_leftout_clip(&vocodec, 1);
						break;
					case 3:
						setLED_rightout_clip(&vocodec, 1);
						break;
				}
				clipCounter[i] = 80;
				clipped[i] = 1;
				clipHappened[i] = 0;
			}

			if ((clipCounter[i] > 0) && (clipped[i] == 1))
			{
				clipCounter[i]--;
			}

			else if ((clipCounter[i] == 0) && (clipped[i] == 1))
			{
				switch (i)
				{
					case 0:
						setLED_leftin_clip(&vocodec, 0);
						break;
					case 1:
						setLED_rightin_clip(&vocodec, 0);
						break;
					case 2:
						setLED_leftout_clip(&vocodec, 0);
						break;
					case 3:
						setLED_rightout_clip(&vocodec, 0);
						break;
				}
				clipped[i] = 0;
			}
		}

		frameCount = DWT->CYCCNT-tempCount5;
		frameLoad = ((float)frameCount * frameMult);
		if (frameLoad > frameMax)
		{
			frameMax = frameLoad;
			if (frameMax > 1.0f)
			{
				overrun++;
			}
		}

		if (setFrameMax)
		{
			frameMax = 0.0f;
			setFrameMax = 0;
		}
#endif


	}
/*
	tempCount6 = DWT->CYCCNT;

	cycleCountVals[0][2] = 0;

	cycleCountVals[0][1] = tempCount6-tempCount5;
	if (cycleCountVals[0][1] > 1280000)
	{
		setLED_Edit(1);
		//overflow
	}
	CycleCounterTrackMinAndMax(0);
	*/

}


// code to display waveform on OLED
/*
	displayBlockVal += fabsf(sample);
	displayBlockCount++;
	if (displayBlockCount >= DISPLAY_BLOCK_SIZE)
	{
		displayBlockVal *= INV_TWO_TO_9;
		audioDisplayBuffer[displayBufferIndex] = displayBlockVal;
		displayBlockVal = 0.0f;
		displayBlockCount = 0;
		displayBufferIndex++;
		if (displayBufferIndex >= 128) displayBufferIndex = 0;
	}
*/


uint32_t audioTick(float* samples)
{
	uint32_t clips = 0;
	if (loadingPreset)
	{
		samples[0] = 0.0f;
		samples[1] = 0.0f;
		return 0;
	}
	//uint32_t tempCount5 = DWT->CYCCNT;



	if ((samples[1] >= 0.999999f) || (samples[1] <= -0.999999f))
	{
		clips |= 1;
	}

	if ((samples[0] >= 0.999999f) || (samples[0] <= -0.999999f))
	{
		clips |= 2;
	}


	//uint16_t current_env = atoDbTable[(uint32_t)(tEnvelopeFollower_tick(vocodec.LED_envelope[0], LEAF_clip(-1.0f, samples[1], 1.0f)) * ATODB_TABLE_SIZE_MINUS_ONE)];
	//__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, current_env);
	//current_env = atoDbTable[(uint32_t)(tEnvelopeFollower_tick(vocodec.LED_envelope[2], LEAF_clip(-1.0f, samples[0], 1.0f)) * ATODB_TABLE_SIZE_MINUS_ONE)];
	//__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, current_env);

	//synthSetFreq(mtof(currentMIDINote));
	//samples[0] = synthTick();

	samples[1] = samples[0];

	//now the samples array is output
	if ((samples[1] > 1.0f) || (samples[1] < -1.0f))
	{
		clips |= 4;
	}

	if ((samples[0] > 1.0f) || (samples[0] < -1.0f))
	{
		clips |= 8;
	}
	//current_env = atoDbTable[(uint32_t)(tEnvelopeFollower_tick(vocodec.LED_envelope[1], LEAF_clip(-1.0f, samples[1], 1.0f)) * ATODB_TABLE_SIZE_MINUS_ONE)];
	//__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, current_env);
	//current_env = atoDbTable[(uint32_t)(tEnvelopeFollower_tick(vocodec.LED_envelope[3], LEAF_clip(-1.0f, samples[0], 1.0f)) * ATODB_TABLE_SIZE_MINUS_ONE)];
	//__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, current_env);

	//uint32_t tempCount6 = DWT->CYCCNT;
	//cycleCountVals[1][1] = tempCount6-tempCount5;
	//CycleCounterTrackMinAndMax(1);
	return clips;
}


void noteOn(int key, int velocity)
{
	currentMIDINote = key;
	if (velocity > 0)
	{
		tSimplePoly_noteOn(myPoly, key, velocity);
	}
	else
	{
		tSimplePoly_noteOff(myPoly, key);
	}



	stringInputs[0] = tSimplePoly_getVelocity(myPoly, 0) * 512;
	stringMIDIPitches[0] = tSimplePoly_getPitch(myPoly, 0);
	newPluck = 1;
}
void noteOff(int key, int velocity)
{

	tSimplePoly_noteOff(myPoly, key);


	stringInputs[0] = tSimplePoly_getVelocity(myPoly, 0) * 512;
	stringMIDIPitches[0] = tSimplePoly_getPitch(myPoly, 0);
	newPluck = 1;
}
void pitchBend( int data)
{
	;
}
void sustainOn()

{
	;
}

void sustainOff()
{
	;
}
void toggleBypass()
{
	;
}
void toggleSustain()
{
	;
}

void ctrlInput(int ctrl, int value)
{
	switch(ctrl)
	{
		case 74:
			ccIn[0] = value;
			break;
		case 71:
			ccIn[1] = value;
			break;
		case 5:
			ccIn[2] = value;
			break;
		case 84:
			ccIn[3] = value;
			break;
		case 78:
			ccIn[4] = value;
			break;
		case 76:
			ccIn[5] = value;
			break;
		case 77:
			ccIn[6] = value;
			break;
		case 10:
			ccIn[7] = value;
			break;

	}

}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
	//setLED_Edit(&vocodec, 1);
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{

}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{

}


void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
	audioFrame(HALF_BUFFER_SIZE);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
	audioFrame(0);
}
