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
#include "oled.h"
#include "tunings.h"
#include "i2c.h"
#include "gpio.h"
#include "sfx.h"
#include "tim.h"
#include "usbh_MIDI.h"
#include "MIDI_application.h"

//the audio buffers are put in the D2 RAM area because that is a memory location that the DMA has access to.
int32_t audioOutBuffer[AUDIO_BUFFER_SIZE] __ATTR_RAM_D2;
int32_t audioInBuffer[AUDIO_BUFFER_SIZE] __ATTR_RAM_D2;

//#define DISPLAY_BLOCK_SIZE 512
//float audioDisplayBuffer[128];
//uint8_t displayBufferIndex = 0;
//float displayBlockVal = 0.0f;
//uint32_t displayBlockCount = 0;

void audioFrame(uint16_t buffer_offset);
uint32_t audioTick(float* samples);

HAL_StatusTypeDef transmit_status;
HAL_StatusTypeDef receive_status;

uint32_t codecReady = 0;

uint32_t frameCounter = 0;

tNoise myNoise;
tCycle mySine[2];
tEnvelopeFollower LED_envelope[4];

uint32_t clipCounter[4] = {0,0,0,0};
uint32_t clipped[4] = {0,0,0,0};
uint32_t clipHappened[4] = {0,0,0,0};


BOOL bufferCleared = TRUE;

int numBuffersToClearOnLoad = 2;
int numBuffersCleared = 0;

#define ATODB_TABLE_SIZE 512
#define ATODB_TABLE_SIZE_MINUS_ONE 511
float atodbTable[ATODB_TABLE_SIZE];
float frameMult = 1.0f / (AUDIO_FRAME_SIZE * 10000.0f);
/**********************************************/

LEAFErrorType errorTypes = 0;

void LEAF_myError(LEAF* const, LEAFErrorType theError)
{
	errorTypes = theError;
}

void audioInit(I2C_HandleTypeDef* hi2c, SAI_HandleTypeDef* hsaiOut, SAI_HandleTypeDef* hsaiIn)
{
	// Initialize LEAF.

	LEAF_init(&vocodec.leaf, SAMPLE_RATE, small_memory, SMALL_MEM_SIZE, &randomNumber);

	LEAF_setErrorCallback(&vocodec.leaf, LEAF_myError);

	tMempool_init (&vocodec.largePool, large_memory, LARGE_MEM_SIZE, &vocodec.leaf);

	tMempool_init (&vocodec.mediumPool, medium_memory, MED_MEM_SIZE, &vocodec.leaf);


	initFunctionPointers(&vocodec);

	//ramps to smooth the knobs
	for (int i = 0; i < 6; i++)
	{
		tDynamicSmoother_init(&vocodec.adc[i],&vocodec.leaf);
	}

	for (int i = 0; i < 4; i++)
	{
		tEnvelopeFollower_init(&LED_envelope[i], 0.0001f, .9995f, &vocodec.leaf);
	}
	LEAF_generate_atodbPositiveClipped(atodbTable, -120.0f, 380.f, ATODB_TABLE_SIZE);
	initGlobalSFXObjects(&vocodec);

	vocodec.loadingPreset = 1;
	vocodec.previousPreset = PresetNil;

	HAL_Delay(10);

	for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
	{
		audioOutBuffer[i] = 0;
	}

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
	HAL_I2C_MspDeInit(hi2c);

	GPIO_InitTypeDef GPIO_InitStruct = {0};

    //PB10, PB11     ------> buttons C and E
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

volatile int frameCount = 0;
volatile float frameLoad = 0.0f;

volatile float frameMax = 0.0f;
volatile int setFrameMax = 1;

volatile int freeCheck = 0;
volatile uint32_t overrun = 0;
void audioFrame(uint16_t buffer_offset)
{
	volatile tempCount5 = DWT->CYCCNT;

	if (codecReady)
	{

		//volatile uint32_t tempCount5 = 0;
		//volatile uint32_t tempCount6 = 0;
		int i;
		//int32_t current_sample;
		uint32_t clipCatcher = 0;

		//tempCount5 = DWT->CYCCNT;

		buttonCheck(&vocodec);

		adcCheck(&vocodec);

		// if the USB write pointer has advanced (indicating unread data is in the buffer),
		// or the overflow bit is set, meaning that the write pointer wrapped around and the read pointer hasn't caught up to it yet
		// then process that new data this frame
		if ((myUSB_FIFO_overflowBit) || (myUSB_FIFO_writePointer > myUSB_FIFO_readPointer))
		{
			ProcessReceivedMidiDatas();
		}


		if (!vocodec.loadingPreset)
		{

			for (int i = 0; i < NUM_ADC_CHANNELS; i++)
			{
				vocodec.smoothedADC[i] = LEAF_clip(0.0f, tDynamicSmoother_tickNoInput(vocodec.adc[i]), 1.0f);
				for (int i = 0; i < KNOB_PAGE_SIZE; i++)
				{
					vocodec.presetKnobValues[vocodec.currentPreset][i + (vocodec.knobPage * KNOB_PAGE_SIZE)] = vocodec.smoothedADC[i];
				}
			}


			if (vocodec.cvAddParam[vocodec.currentPreset] >= 0)
			{
				vocodec.presetKnobValues[vocodec.currentPreset][vocodec.cvAddParam[vocodec.currentPreset]] = vocodec.smoothedADC[5];
			}

			vocodec.frameFunctions[vocodec.currentPreset](&vocodec);
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
			numBuffersCleared++;
			if (numBuffersCleared >= numBuffersToClearOnLoad)
			{
				numBuffersCleared = numBuffersToClearOnLoad;
				if (vocodec.loadingPreset)
				{

					if (vocodec.previousPreset != PresetNil)
					{
						vocodec.freeFunctions[vocodec.previousPreset](&vocodec);

					}
					setLED_A(&vocodec, 0);
					setLED_B(&vocodec, 0);
					setLED_C(&vocodec, 0);
					setLED_Edit(&vocodec, 0);
					setLED_1(&vocodec, 0);
					vocodec.knobPage = 0;
					resetKnobValues(&vocodec);
					vocodec.leaf.clearOnAllocation = 0;
					vocodec.allocFunctions[vocodec.currentPreset](&vocodec);
					vocodec.previousPreset = vocodec.currentPreset;
					vocodec.loadingPreset = 0;
					setFrameMax = 1;
					freeCheck = vocodec.leaf.allocCount - vocodec.leaf.freeCount;
				}
			}
		}
		else numBuffersCleared = 0;

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
	if (vocodec.loadingPreset)
	{
		samples[0] = 0.0f;
		samples[1] = 0.0f;
		return 0;
	}
	//uint32_t tempCount5 = DWT->CYCCNT;

	//cycleCountVals[1][2] = 0;


	if ((samples[1] >= 0.999999f) || (samples[1] <= -0.999999f))
	{
		clips |= 1;
	}

	if ((samples[0] >= 0.999999f) || (samples[0] <= -0.999999f))
	{
		clips |= 2;
	}


	uint16_t current_env = atodbTable[(uint32_t)(tEnvelopeFollower_tick(LED_envelope[0], LEAF_clip(-1.0f, samples[1], 1.0f)) * ATODB_TABLE_SIZE_MINUS_ONE)];
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, current_env);
	current_env = atodbTable[(uint32_t)(tEnvelopeFollower_tick(LED_envelope[2], LEAF_clip(-1.0f, samples[0], 1.0f)) * ATODB_TABLE_SIZE_MINUS_ONE)];
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, current_env);

	vocodec.tickFunctions[vocodec.currentPreset](&vocodec, samples);

	//now the samples array is output
	if ((samples[1] >= 0.999999f) || (samples[1] <= -0.999999f))
	{
		clips |= 4;
	}

	if ((samples[0] >= 0.999999f) || (samples[0] <= -0.999999f))
	{
		clips |= 8;
	}
	current_env = atodbTable[(uint32_t)(tEnvelopeFollower_tick(LED_envelope[1], LEAF_clip(-1.0f, samples[1], 1.0f)) * ATODB_TABLE_SIZE_MINUS_ONE)];
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, current_env);
	current_env = atodbTable[(uint32_t)(tEnvelopeFollower_tick(LED_envelope[3], LEAF_clip(-1.0f, samples[0], 1.0f)) * ATODB_TABLE_SIZE_MINUS_ONE)];
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, current_env);

	//uint32_t tempCount6 = DWT->CYCCNT;
	//cycleCountVals[1][1] = tempCount6-tempCount5;
	//CycleCounterTrackMinAndMax(1);
	return clips;
}

/*
float audioTickL(float audioIn)
{
	float sample = 0.0f;

	if (loadingPreset) return sample;

	bufferCleared = 0;


	if ((audioIn >= 0.999999f) || (audioIn <= -0.999999f))
	{
		clipHappened[0] = 1;
	}


	tickFunctions[currentPreset](audioIn);
	sample = leftOut;

	if ((sample >= 0.999999f) || (sample <= -0.999999f))
	{
		clipHappened[2] = 1;
	}


	float current_env = atodbTable[(uint32_t)(tEnvelopeFollower_tick(&LED_envelope[2], audioIn)* ATODB_TABLE_SIZE)];
	 __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, (uint32_t)current_env);

	current_env = atodbTable[(uint32_t)(tEnvelopeFollower_tick(&LED_envelope[3], leftOut)* ATODB_TABLE_SIZE)];
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (uint32_t)current_env);

	return sample;
}



float audioTickR(float audioIn)
{
	rightIn = audioIn;
	uint32_t tempCount1, tempCount2;






	if ((rightIn >= 0.999999f) || (rightIn <= -0.999999f))
	{
		clipHappened[1] = 1;
	}



	if ((rightOut >= 0.999999f) || (rightOut <= -0.999999f))
	{
		clipHappened[3] = 1;
	}


	float current_env = atodbTable[(uint32_t)(tEnvelopeFollower_tick(&LED_envelope[2], rightIn)* ATODB_TABLE_SIZE)];
	 __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, (uint32_t)current_env);


		cycleCountVals[3][2] = 0;
		tempCount1 = DWT->CYCCNT;


	current_env = atodbTable[(uint32_t)(tEnvelopeFollower_tick(&LED_envelope[3], rightOut)* ATODB_TABLE_SIZE)];
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, (uint32_t)current_env);

	tempCount2 = DWT->CYCCNT;
	cycleCountVals[3][1] = tempCount2-tempCount1;
	CycleCounterTrackMinAndMax(3);
	return rightOut;
}

*/

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
	setLED_Edit(&vocodec, 1);
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
