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

float masterVolFromBrainForSynth = 0.25f;


float mtofTable[MTOF_TABLE_SIZE]__ATTR_RAM_D2;

float atoDbTable[ATODB_TABLE_SIZE]__ATTR_RAM_D2;
float dbtoATable[DBTOA_TABLE_SIZE]__ATTR_RAM_D2;

void audioFrame(uint16_t buffer_offset);
uint32_t audioTick(float* samples);


uint32_t clipCounter[4] = {0,0,0,0};
uint32_t clipped[4] = {0,0,0,0};
uint32_t clipHappened[4] = {0,0,0,0};

uint8_t currentMIDINote = 60;


LEAF leaf;
tExpSmooth adc[6];
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

	LEAF_init(&leaf, SAMPLE_RATE, small_memory, SMALL_MEM_SIZE, &randomNumber);

	LEAF_setErrorCallback(&leaf, LEAF_myError);

	tMempool_init (&mediumPool, medium_memory, MED_MEM_SIZE, &leaf);

	tMempool_init (&largePool, large_memory, LARGE_MEM_SIZE, &leaf);

	synthInit();

	//ramps to smooth the knobs

	for (int i = 0; i < 6; i++)
	{
		tExpSmooth_init(&adc[i],0.0f, 0.3f,&leaf);
	}


	LEAF_generate_atodbPositiveClipped(atoDbTable, -120.0f, 380.f, ATODB_TABLE_SIZE);

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


#endif
		for (i = 0; i < (HALF_BUFFER_SIZE); i += 2)
		{
			float theSamples[2];
			theSamples[0] = ((float)(audioInBuffer[buffer_offset + i] << 8)) * INV_TWO_TO_31;
			theSamples[1] = ((float)(audioInBuffer[buffer_offset + i + 1] << 8)) * INV_TWO_TO_31;

			clipCatcher |= audioTick(theSamples);
			audioOutBuffer[buffer_offset + i] = (int32_t)(theSamples[1] * TWO_TO_23);
			audioOutBuffer[buffer_offset + i + 1] = (int32_t)(theSamples[0] * TWO_TO_23);
		}
#if 0
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

	//cycleCountVals[1][2] = 0;


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

	synthSetFreq(mtof(currentMIDINote));
	samples[0] = synthTick();

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
}
void noteOff(int key, int velocity)
{
	;
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
	;

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
