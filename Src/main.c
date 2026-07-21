/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "rng.h"
#include "sai.h"
#include "sdmmc.h"
#include "tim.h"
#include "usart.h"
#include "usb_host.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ui.h"
#include "leaf.h"
#include "audiostream.h"
#include "eeprom.h"
#include "MIDI_application.h"
#include "synth.h"
#include "oled.h"
#include "lsm6dsl.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t count;

//FLASH storage EEPROM emulation variables
FLASH_OBProgramInitTypeDef OBInit;
uint16_t VarDataTab = 0;
uint16_t VarValue = 0;



volatile uint32_t receivingSysex = 0;
volatile uint32_t parsingSysex = 0;

volatile uint32_t uartTest = 0;
uint32_t sysexPointerMask = 4095;
uint32_t sysexWritePointer = 0;
uint32_t sysexReadPointer = 0;
uint32_t lastBufferStuff = 0;
uint32_t lastEndReceive = 0;
uint32_t masterTimer = 0;
uint32_t newSysexStart = 0;
uint32_t prevLastParseCall = 0;
uint32_t lastParseCall = 0;

uint32_t sysexReset = 1;
uint32_t sysexMessageStartPointsWritePosition = 0;
uint32_t sysexMessageStartPointsReadPosition = 0;
uint32_t sysexMessageStartPoints[256];
uint32_t skipSysexHeader = 0;
uint32_t sysexHeaderCount = 2;
uint32_t lastBufferBegin[2];
uint32_t prevLastBufferBegin[2];
uint32_t sysexParseInProgress = 0;
uint32_t currentFloat = 0;
uint32_t presetNumberToWrite = 0;
presetArraySectionState presetArraySection = presetNameSection;
float myTestVal = 0.0f;
float valCheck = 0.0f;
uint32_t valsCount = 0;
uint32_t mapCount = 0;
uint32_t sysexParseError = 0;
uint32_t mapCountExpectation = 0;
uint32_t readyToWritePreset = 0;
uint32_t parseEBPPreset = 0;
union breakFloat {
 float f;
 uint8_t b[4];
 uint32_t u32;
};
#define PRESET_NAME_LENGTH_IN_BYTES 14
#define MACRO_NAME_LENGTH_IN_BYTES 9
#define CONTROL_NAME_LENGTH_IN_BYTES 10
#define NUM_MACROS 8
#define NUM_CONTROLS 4



#define NUM_COUNTER_CYCLES_TO_AVERAGE 32
volatile int64_t cycleCountVals[4][3];
volatile int64_t cycleCountValsAverager[4][NUM_COUNTER_CYCLES_TO_AVERAGE];
volatile uint16_t cycleCountAveragerCounter[4] = {0,0,0,0};
float cycleCountAverages[4][3];

volatile uint32_t loadingPreset = 0;
volatile uint32_t currentPreset = 0;
volatile uint32_t currentPresetSize = 0;
volatile uint32_t presetReady = 0;
volatile float audioMasterLevel = 0.0f;
volatile uint32_t prevPreset = 0;
volatile uint32_t diskBusy = 1;

volatile uint32_t loadFailed = 0;
int32_t volatile prevKnobByte[20];
FILINFO fno;
FIL fdst;
DIR dir;

uint8_t sysexBuffer[4096];
uint32_t sysexPointer = 0;
uint8_t buffer[4096] __ATTR_RAM_D2;
volatile uint16_t bufferPos = 0;
FRESULT res;
const TCHAR path = 0;

volatile uint8_t presetName[14];
volatile uint8_t presetNamesArray[MAX_NUM_PRESETS][14]__ATTR_RAM_D2;
volatile uint8_t presetNumberToLoad = 0;
volatile uint32_t presetWaitingToParse = 0;
volatile uint32_t presetWaitingToWrite = 0;
volatile uint32_t presetWaitingToLoad = 0;

volatile uint32_t waitingToParseSingleParameterChange = 0;

volatile uint32_t waitingToParseSingleMappingChange = 0;

volatile uint8_t macroNamesArray[MAX_NUM_PRESETS][20][10]__ATTR_RAM_D2;

float loadedKnobParams[20];

uint_fast8_t knobTicked[12];

param params[NUM_PARAMS];
mapping mappings[MAX_NUM_MAPPINGS];
uint8_t numMappings = 0;

uint8_t effectsActive[4] = {0,0,0,0};

filterSetter filterSetters[NUM_FILT];
lfoSetter lfoSetters[NUM_LFOS];
effectSetter effectSetters[NUM_EFFECT];
float defaultScaling = 1.0f;

uint8_t fxPre = 0;
uint8_t pedalControlsMaster = 0;

#define SCALE_TABLE_SIZE 2048
float resTable[SCALE_TABLE_SIZE];
float envTimeTable[SCALE_TABLE_SIZE];
float lfoRateTable[SCALE_TABLE_SIZE];
int oscsEnabled[3] = {0,0,0};
float midiKeyDivisor;
float midiKeySubtractor;
uint_fast8_t pedalTicked[10];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
void MPU_Conf(void);
void SDRAM_Initialization_sequence(void);
static void CycleCounterInit( void );
void getPresetNamesFromSDCard(void);
static int checkForSDCardPreset(uint8_t numberToLoad);
void parsePreset(int, int);
void parseSysexPreset();
void parseSingleParameterChange();
void parseSingleMappingChange();
static void writePresetToSDCard(int fileSize);
#define testDataSize 32
volatile uint8_t testData[testDataSize];

volatile uint8_t errorTime1 = 0;

volatile uint8_t errorTime2 = 0;
volatile uint8_t errorTime3 = 0;
volatile uint8_t errorTime4 = 0;
volatile uint8_t testInt = 0;

volatile uint16_t ADC_values[7] __ATTR_RAM_D2_DMA;

volatile uint32_t OLED_changed = 0;

#define UART_BUFFER_SIZE 2
#define UART_HALF_BUFFER_SIZE 1
#define UART_BUFFER_MASK 1
volatile uint8_t UART_buffer[UART_BUFFER_SIZE] __ATTR_RAM_D2_DMA;

volatile uint32_t sysexReadyToParse = 0;


uint32_t parseThatMF = 0;

void errorFunction(int i)
{
	errorTime1 = large_memory[i];

	errorTime2 = testData[i];

	errorTime3 = testData[i];
	errorTime4 = large_memory[i + 32];
}

void SDRAM_test()
{
	uint32_t startingPoint = 0;
	SCB_CleanDCache();
	while (startingPoint < (LARGE_MEM_SIZE-testDataSize))
	{
		for (uint32_t i = 0; i < testDataSize; i++)
		{
			testData[i] = i;
			large_memory[i+startingPoint] = testData[i];
		}
		SCB_CleanDCache();
		for (uint32_t i = 0; i < testDataSize; i++)
		{
			testData[i] = i;
			testInt = large_memory[i+startingPoint];
			if (testInt != testData[i])
			{
				errorFunction(i);
			}
		}
		startingPoint += testDataSize;
	}
}



void     SENSOR_IO_Init(void)
{
	;
}
void     SENSOR_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
	uint8_t tempBuf[2];
	tempBuf[0] = Reg;
	tempBuf[1] = Value;
	HAL_I2C_Master_Transmit(&hi2c2, Addr, tempBuf, 2, 2000);
}
uint8_t  SENSOR_IO_Read(uint8_t Addr, uint8_t Reg)
{
	uint8_t tempBuf[1];
	tempBuf[0] = Reg;
	HAL_I2C_Master_Transmit(&hi2c2, Addr, tempBuf, 1, 2000);
	HAL_I2C_Master_Receive(&hi2c2, Addr, tempBuf, 1, 2000);
	return tempBuf[0];
}
uint16_t SENSOR_IO_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
	uint8_t tempBuf[1];
	tempBuf[0] = Reg;
	HAL_I2C_Master_Transmit(&hi2c2, Addr, tempBuf, 1, 2000);
	HAL_I2C_Master_Receive(&hi2c2, Addr, Buffer, Length, 2000);
	return 1;
}
void     SENSOR_IO_WriteMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
	uint8_t tempBuf[1];
	tempBuf[0] = Reg;
	HAL_I2C_Master_Transmit(&hi2c2, Addr, tempBuf, 1, 2000);
	HAL_I2C_Master_Transmit(&hi2c2, Addr, Buffer, Length, 2000);

}




/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

volatile int16_t accelData[3];
volatile float gyroData[3];

uint16_t accelInitValues = 0b0000010010100000;
uint16_t gyroInitValues = 0b0000010010100000;

uint8_t whoamiAccel = 0;
uint8_t whoamiGyro = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  //disabling I and D cache because they cause issues with the USB initialization when -o3 optimization is on
  /* Enable I-Cache---------------------------------------------------------*/
  SCB_DisableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_DisableDCache();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FMC_Init();
  MX_ADC1_Init();
  MX_I2C2_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  MX_SAI1_Init();
  MX_RNG_Init();
  MX_USB_HOST_Init();
  MX_TIM4_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

  /*
  HAL_Delay(100);
  LSM6DSL_AccInit(accelInitValues);
  HAL_Delay(2);
  LSM6DSL_GyroInit(gyroInitValues);
  HAL_Delay(2);
  whoamiAccel = LSM6DSL_AccReadID();
  HAL_Delay(2);
  whoamiGyro = LSM6DSL_GyroReadID();
*/
  /// it seems we need to enable caching after setting up the USB Host Controller -
  // otherwise turning on -o3 optimization causes unreliable behavior where it's not set up correctly and never reaches the USB interrupt for connection
  /* Enable I-Cache---------------------------------------------------------*/
 SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();


  //HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  //HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  //HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  //HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  //HAL_Delay(1);
  // Emulated EEPROM Init
  HAL_FLASH_Unlock();
  if( EE_Init() != EE_OK)
  {
    Error_Handler();
  }
  if((EE_ReadVariable(VirtAddVarTab[0],  &VarDataTab)) != HAL_OK) // read what the preset was before last power-off
  {
	//if it can't read something, it's probably because this brain has never been programmed, so write a value in there to start with
	  if((EE_WriteVariable(VirtAddVarTab[0],  0)) != HAL_OK)
	{
		Error_Handler();
	}
  }

  CycleCounterInit();

  //pull reset pin on audio codec low to make sure it's stable
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);

  uint32_t tempFPURegisterVal = __get_FPSCR();
  tempFPURegisterVal |= (1<<24); // set the FTZ (flush-to-zero) bit in the FPU control register  // this makes checking for denormals not necessary as they are automatically set to zero by the hardware
  __set_FPSCR(tempFPURegisterVal);


  HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);

  if (HAL_ADC_Start_DMA(&hadc1,(uint32_t*)ADC_values, NUM_ADC_CHANNELS) != HAL_OK)
  {
      Error_Handler();
  }

  HAL_Delay(10);
  //HAL_Delay(10);

  SDRAM_Initialization_sequence();

  for (int i = 0; i < 20; i++)
  {
	  prevKnobByte[i] = 256;//to give a clearly wrong value for the knobs to see when they haven't been initialized with real 0-255 numbers yet
  }
  for (int i = 0; i < 4096; i++)
    {
  	  buffer[i] = 0;
    } //put in some values to make the array valid as a preset
    buffer[15+112] = NUM_PARAMS;
    buffer[NUM_PARAMS*2+16+112] = 0xef;
    buffer[NUM_PARAMS*2+17+112] = 0xef;
    buffer[NUM_PARAMS*2+19+112] = 1;
    buffer[NUM_PARAMS*2+25+112] = 0xfe;
    buffer[NUM_PARAMS*2+26+112] = 0xfe;

    LEAF_generate_table_skew_non_sym(resTable, 0.01f, 10.0f, 0.5f, SCALE_TABLE_SIZE);
    LEAF_generate_table_skew_non_sym(envTimeTable, 0.0f, 20000.0f, 4000.0f, SCALE_TABLE_SIZE);
    LEAF_generate_table_skew_non_sym(lfoRateTable, 0.0f, 30.0f, 2.0f, SCALE_TABLE_SIZE);
  HAL_Delay(10);


  if (VarDataTab < MAX_NUM_PRESETS) //make sure the stored data is a number not past the number of available presets
  {
  	  currentPreset = VarDataTab; //if it's good, start at that remembered preset number
  }
  else
  {
  	  currentPreset = 0; //if the data is messed up for some reason, just initialize at the first preset (preset 0)
  }
  getPresetNamesFromSDCard();
  checkForSDCardPreset(currentPreset);
  presetNumberToLoad = currentPreset;
  presetWaitingToLoad = 1;

  OLED_init(&hi2c2);

  OLED_writePreset();

  audioInit(&hi2c2, &hsai_BlockA1, &hsai_BlockB1);

  for (int i = 0; i < UART_BUFFER_SIZE; i++)
  {
	  UART_buffer[i] = 0;
  }
  //HAL_UARTEx_ReceiveToIdle_DMA(&huart6,UART_buffer,UART_BUFFER_SIZE);
  HAL_UART_Receive_DMA(&huart6,UART_buffer,UART_BUFFER_SIZE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	//HAL_Delay(10);


	  //LSM6DSL_AccReadXYZ(accelData);
	  //HAL_Delay(1);

	  //LSM6DSL_GyroReadXYZAngRate(gyroData);
	  //HAL_Delay(1);
	MIDI_Application();

    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */

    if (presetWaitingToLoad > 0)
    {
    	checkForSDCardPreset(presetNumberToLoad);
    }

    if (parseThatMF > 0)
    {
    	parseSysexPreset();
    }
    if (presetWaitingToWrite > 0)
    {
    	writePresetToSDCard(presetWaitingToWrite);
    }
    if (presetWaitingToParse > 0)
    {
    	parsePreset(presetWaitingToParse, presetNumberToLoad);
    }
    if (waitingToParseSingleParameterChange > 0)
    {
    	parseSingleParameterChange();
    }
    if (waitingToParseSingleMappingChange > 0)
    {
    	parseSingleMappingChange();
    }


    OLED_process();
	if ((hi2c2.State == HAL_I2C_STATE_READY) && (OLED_changed))
	{
	  OLED_draw();
	}

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 10;
  RCC_OscInitStruct.PLL.PLLN = 384;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_SAI1;
  PeriphClkInitStruct.PLL2.PLL2M = 25;
  PeriphClkInitStruct.PLL2.PLL2N = 344;
  PeriphClkInitStruct.PLL2.PLL2P = 7;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 1;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


#define SDRAM_TIMEOUT ((uint32_t)0xFFFF)



#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)

#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)

#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)

#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0003)

#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)

#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)

#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)

#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)

#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)

#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)

#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)



//#define SDRAM_REFRESH_COUNT                   	 ((uint32_t)956)// 7.9us in cycles of 8.333333ns + 20 cycles as recommended by datasheet page 866/3289 for STM32H743

#define SDRAM_REFRESH_COUNT                   	 ((uint32_t)0x0569)// 7.9us in cycles of 8.333333ns + 20 cycles as recommended by datasheet page 866/3289 for STM32H743

void SDRAM_Initialization_sequence(void)

{

    __IO uint32_t tmpmrd = 0;

    FMC_SDRAM_CommandTypeDef Command;

    /* Step 1: Configure a clock configuration enable command */

    Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;

    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;

    Command.AutoRefreshNumber = 1;

    Command.ModeRegisterDefinition = 0;



    /* Send the command */

    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);



    /* Step 2: Insert 100 us minimum delay */

    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */

    HAL_Delay(1);



    /* Step 3: Configure a PALL (precharge all) command */

    Command.CommandMode = FMC_SDRAM_CMD_PALL;

    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;

    Command.AutoRefreshNumber = 1;

    Command.ModeRegisterDefinition = 0;



    /* Send the command */

    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);



    /* Step 5: Program the external memory mode register */

    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_4 | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL

        | SDRAM_MODEREG_CAS_LATENCY_2 | SDRAM_MODEREG_OPERATING_MODE_STANDARD

        | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;



    Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;

    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;

    Command.AutoRefreshNumber = 1;

    Command.ModeRegisterDefinition = tmpmrd;



    /* Send the command */

    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);



    /* Step 4: Configure the 1st Auto Refresh command */

    Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;

    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;

    Command.AutoRefreshNumber = 8;

    Command.ModeRegisterDefinition = 0;



    /* Send the command */

    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);



    /* Step 2: Insert 100 us minimum delay */

    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */

    HAL_Delay(1);



    /* Step 5: Configure the 2nd Auto Refresh command */

    Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;

    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;

    Command.AutoRefreshNumber = 8;

    Command.ModeRegisterDefinition = 0;



    /* Send the command */

    HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);



    /* Step 6: Set the refresh rate counter */

    /* Set the device refresh rate */

    HAL_SDRAM_ProgramRefreshRate(&hsdram1, SDRAM_REFRESH_COUNT);

}

// Simple LCG random number generator — no stdlib dependency.
// Returns a value in [0, 1).
static uint32_t _leaf_lcg_state = 1664525u;
float randomNumber(void) {
    _leaf_lcg_state = _leaf_lcg_state * 1664525u + 1013904223u;
    return (_leaf_lcg_state >> 8) * (1.0f / 16777216.0f);
}


uint8_t BSP_SD_IsDetected(void)
{
  __IO uint8_t status = SD_PRESENT;

  //if (BSP_PlatformIsDetected() == 0x0)
  //{
  //  status = SD_NOT_PRESENT;
  //}

  return status;
}

volatile uint32_t r0;
volatile uint32_t r1;
volatile uint32_t r2;
volatile uint32_t r3;
volatile uint32_t r12;
volatile uint32_t lr; // Link register.
volatile uint32_t pc; // Program counter.
volatile uint32_t psr;// Program status register.

/*
	static void HardFault_Handler(void)
	{
	    __asm volatile
	    (
	        " tst lr, #4                                                n"
	        " ite eq                                                    n"
	        " mrseq r0, msp                                             n"
	        " mrsne r0, psp                                             n"
	        " ldr r1, [r0, #24]                                         n"
	        " ldr r2, handler2_address_const                            n"
	        " bx r2                                                     n"
	        " handler2_address_const: .word prvGetRegistersFromStack    n"
	    );
	}

*/
void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
//These are volatile to try and prevent the compiler/linker optimising them
//away as the variables never actually get used.  If the debugger won't show the
//values of the variables, make them global by moving their declaration outside
//of this function.



	r0 = pulFaultStackAddress[ 0 ];
	r1 = pulFaultStackAddress[ 1 ];
	r2 = pulFaultStackAddress[ 2 ];
	r3 = pulFaultStackAddress[ 3 ];

	r12 = pulFaultStackAddress[ 4 ];
	lr = pulFaultStackAddress[ 5 ];
	pc = pulFaultStackAddress[ 6 ];
	psr = pulFaultStackAddress[ 7 ];

	// When the following line is hit, the variables contain the register values.
	for( ;; );
}



/*
uint8_t LEAF_error(uint8_t errorCode)
{
	setLED_C(1);
	while(1)
		{
			;
		}

	return 0;
}
*/

// helper function to initialize measuring unit (cycle counter) */
static void CycleCounterInit( void )
{
  /* Enable TRC */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* Unlock DWT registers */
  if ((*(uint32_t*)0xE0001FB4) & 1)
    *(uint32_t*)0xE0001FB0 = 0xC5ACCE55;

  /* clear the cycle counter */
  DWT->CYCCNT = 0;

  /* start the cycle counter */
  DWT->CTRL = 0x40000001;

  for (int i = 0; i < 4; i++)
  {
	  cycleCountAverages[i][0] = 0.0f;
	  cycleCountAverages[i][1] = 0.0f;
	  cycleCountAverages[i][2] = 0.0f;
  }
}





//this keeps min and max, but doesn't do the array for averaging - a bit less expensive
void CycleCounterTrackMinAndMax( int whichCount)
{
	if ((cycleCountVals[whichCount][2] == 0) && (cycleCountVals[whichCount][1] > 0)) //the [2] spot in the array will be set to 1 if an interrupt happened during the cycle count -- need to set that in any higher-priority interrupts to make that true
	{
		if ((cycleCountVals[whichCount][1] < cycleCountAverages[whichCount][1]) || (cycleCountAverages[whichCount][1] == 0))
		{
			cycleCountAverages[whichCount][1] = cycleCountVals[whichCount][1];
		}
		//update max value ([2])
		if (cycleCountVals[whichCount][1] > cycleCountAverages[whichCount][2])
		{
			cycleCountAverages[whichCount][2] = cycleCountVals[whichCount][1];
		}
	}
}

//these are expensive but give an average of several counts
void CycleCounterAddToAverage( int whichCount)
{
	if ((cycleCountVals[whichCount][2] == 0) && (cycleCountVals[whichCount][1] > 0)) //the [2] spot in the array will be set to 1 if an interrupt happened during the cycle count -- need to set that in any higher-priority interrupts to make that true
	{
		cycleCountValsAverager[whichCount][cycleCountAveragerCounter[whichCount]] = cycleCountVals[whichCount][1];
	}
	else
	{
		cycleCountValsAverager[whichCount][cycleCountAveragerCounter[whichCount]] = -1;
	}
	cycleCountAveragerCounter[whichCount]++;
	if (cycleCountAveragerCounter[whichCount] >= NUM_COUNTER_CYCLES_TO_AVERAGE)
	{
		cycleCountAveragerCounter[whichCount]  = 0;
	}
}

//these are expensive but give an average of several counts
void CycleCounterAverage( int whichCount)
{
	float totalCycles = 0.0f;
	float numberOfCountedSamples = 0.0f;
	for (int i = 0; i < NUM_COUNTER_CYCLES_TO_AVERAGE; i++)
	{
		if (cycleCountValsAverager[whichCount][i] >= 0) //check if the count is valid (not interrupted by an interrupt)
		{
			totalCycles += cycleCountValsAverager[whichCount][i];

			//update min value ([1])
			if ((cycleCountValsAverager[whichCount][i] < cycleCountAverages[whichCount][1]) || (cycleCountAverages[whichCount][1] == 0))
			{
				cycleCountAverages[whichCount][1] = cycleCountValsAverager[whichCount][i];
			}
			//update max value ([2])
			if (cycleCountValsAverager[whichCount][i] > cycleCountAverages[whichCount][2])
			{
				cycleCountAverages[whichCount][2] = cycleCountValsAverager[whichCount][i];
			}
			numberOfCountedSamples++;
		}
	}
	if (numberOfCountedSamples > 0.0f)
	{
		cycleCountAverages[whichCount][0] = totalCycles / numberOfCountedSamples;
	}
	else
	{
		cycleCountAverages[whichCount][0] = 0.0f;
	}

}

void getPresetNamesFromSDCard(void)
{
	if(BSP_SD_IsDetected())
	{
		for (int i = 0; i < AUDIO_BUFFER_SIZE; i+=2)
		{
			audioOutBuffer[i] = 0;
			audioOutBuffer[i + 1] = 0;
		}
		diskBusy = 1;

		loadFailed = 0;
		//HAL_Delay(300);

		disk_initialize(0);

	    disk_status(0);

		if(f_mount(&SDFatFS,  SDPath, 1) == FR_OK)
		{

			FRESULT res;
			/* Start to search for preset files */


			//turn the integer value into a 2 digit string
			char charBuf[10];
			char finalString[10];

			for(int i = 0; i < MAX_NUM_PRESETS; i++)
			{
				itoa(i, charBuf, 10);
				int len = ((strlen(charBuf)));
				if (len == 1)
				{
					finalString[2] = charBuf[1];
					finalString[1] = charBuf[0];
					finalString[0] = '0';
					strcat(finalString, "*.ebp");
				}

				else
				{
					strcat(charBuf, "*.ebp");
					strcpy(finalString, charBuf);
				}


				res = f_findfirst(&dir, &fno, SDPath, finalString);
				unsigned int bytesRead;
				if(res == FR_OK)
				{
					if(f_open(&SDFile, fno.fname, FA_OPEN_ALWAYS | FA_READ) == FR_OK)
					{
						f_read(&SDFile, &buffer, f_size(&SDFile), &bytesRead);
						f_close(&SDFile);
						uint16_t bufferIndex = 0;
						//skip the first 4 bytes if there is a version number stored in the preset
						if (buffer[bufferIndex] == 17)
						{
							bufferIndex = 4;
						}
						//14-byte name
						for (int j = 0; j < 14; j++)
						{
							presetNamesArray[i][j] = buffer[bufferIndex];
							bufferIndex++;
						}
						//9-byte macros
						for (int j = 0; j < 8; j++)
						{
							for (int k = 0; k < 9; k++)
							{
								macroNamesArray[i][j][k] = buffer[bufferIndex];
								bufferIndex++;
							}
						}
						//10-byte macros
						for (int j = 0; j < 4; j++)
						{
							for (int k = 0; k < 10; k++)
							{
								macroNamesArray[i][j+8][k] = buffer[bufferIndex];
								bufferIndex++;
							}
						}
					}
				}
			}

		}

	}
	diskBusy = 0;
	return;
}
volatile uint32_t checkCount = 0;
volatile uint32_t parseCount = 0;
static int checkForSDCardPreset(uint8_t numberToLoad)
{

	int found = 0;
	volatile uint32_t tempCountCheck = DWT->CYCCNT;

	//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
	if(BSP_SD_IsDetected())
	{
		for (int i = 0; i < AUDIO_BUFFER_SIZE; i+=2)
		{
			audioOutBuffer[i] = 0;
			audioOutBuffer[i + 1] = 0;
		}
		__disable_irq();
		diskBusy = 1;
		loadFailed = 0;
		//HAL_Delay(300);
		presetWaitingToLoad = 0;
		disk_initialize(0);

	    disk_status(0);

		if(f_mount(&SDFatFS,  SDPath, 1) == FR_OK)
		{

			FRESULT res;
			/* Start to search for preset files */
			char charBuf[10];
			char finalString[10];

			//turn the integer value into a 2 digit string

			itoa(numberToLoad, charBuf, 10);
			int len = ((strlen(charBuf)));
			if (len == 1)
			{
				finalString[2] = charBuf[1];
				finalString[1] = charBuf[0];
				finalString[0] = '0';
				strcat(finalString, "*.ebp");
			}

			else
			{
				strcat(charBuf, "*.ebp");
				strcpy(finalString, charBuf);
			}

			res = f_findfirst(&dir, &fno, SDPath, finalString);
			unsigned int  bytesRead;
			if(res == FR_OK)
			{
				if(f_open(&SDFile, fno.fname, FA_OPEN_ALWAYS | FA_READ) == FR_OK)
				{
					f_read(&SDFile, &buffer, f_size(&SDFile), &bytesRead);
					presetWaitingToParse = bytesRead;
					f_close(&SDFile);
					found = 1;
				}
			}
		}
	}
	if (!found)
	{
		loadFailed = 1;
	}
	else
	{
		prevPreset = numberToLoad;
		currentPreset = numberToLoad;
	}
	diskBusy = 0;
	checkCount = DWT->CYCCNT - tempCountCheck;
	__enable_irq();
	//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
	return found;
}

static void writePresetToSDCard(int fileSize)
{
	__disable_irq();
	 for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
	 {
		 audioOutBuffer[i] = 0;
	 }
	if(BSP_SD_IsDetected())
	{
		//if(f_mount(&SDFatFS,  SDPath, 1) == FR_OK)
		{
			//if(res == FR_OK)
			{
				for (int i = 0; i < AUDIO_BUFFER_SIZE; i+=2)
				{
					audioOutBuffer[i] = 0;
					audioOutBuffer[i + 1] = 0;
				}
				diskBusy = 1;
				//make sure the number is not above 2 digits
			    if (presetNumberToWrite > 99)
			    {
			    	presetNumberToWrite = 99;
			    }


			    //first, delete any existing presets that share the same number
			    FRESULT res;
				/* Start to search for preset files */
				char charBufC[10];
				char finalStringC[10];

				//turn the integer value into a 2 digit string

				itoa(presetNumberToWrite, charBufC, 10);
				int len = ((strlen(charBufC)));
				if (len == 1)
				{
					finalStringC[2] = charBufC[1];
					finalStringC[1] = charBufC[0];
					finalStringC[0] = '0';
					strcat(finalStringC, "*.ebp");
				}

				else
				{
					strcat(charBufC, "*.ebp");
					strcpy(finalStringC, charBufC);
				}

				uint32_t keepChecking = 1;
				while(keepChecking)
				{
					res = f_findfirst(&dir, &fno, SDPath, finalStringC);

					//delete if found
					if((res == FR_OK) && (fno.fname[0]))
					{
						f_unlink (fno.fname);
					}
					else
					{
						keepChecking = 0;
					}
				}

			    //turn the integer value into a 2 digit string
				char charBuf[22];
				char finalString[22];
				itoa(presetNumberToWrite, charBuf, 10);
				len = ((strlen(charBuf)));
				if (len == 1)
				{
					finalString[21] = 0;
					finalString[20] = 'p';
					finalString[19] = 'b';
					finalString[18] = 'e';
					finalString[17] = '.';
					for (int i = 0; i < 14; i++)
					{
						finalString[i+3] = buffer[i+4];
						//replace spaces with underscores for filename
						if (finalString[i+3] == 32)
						{
							finalString[i+3] = '_';
						}
					}
					finalString[2] = '_';
					finalString[1] = charBuf[0];
					finalString[0] = '0';

				}

				else
				{
					finalString[21] = 0;
					finalString[20] = 'p';
					finalString[19] = 'b';
					finalString[18] = 'e';
					finalString[17] = '.';
					for (int i = 0; i < 14; i++)
					{
						finalString[i+3] = buffer[i+4];
						//replace spaces with underscores for filename
						if (finalString[i+3] == 32)
						{
							finalString[i+3] = '_';
						}
					}
					finalString[2] = '_';
					finalString[1] = charBuf[1];
					finalString[0] = charBuf[0];


				}

				if(f_open(&SDFile, finalString, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
				{
					unsigned int bytesRead;
					f_write(&SDFile, &buffer, fileSize, &bytesRead);
					f_close(&SDFile);
				}

			}
			//f_mount(0, "", 0); //unmount
		}
	}
	presetWaitingToWrite = 0;
	currentPreset = presetNumberToWrite;
	diskBusy = 0;
	__enable_irq();
}

void __ATTR_ITCMRAM blankFunction(float a, int b, int c)
{
	;
}

float __ATTR_ITCMRAM scaleDefault(float input)
{
	//input = LEAF_clip(0.f, input, 1.f);
	return input;
}

float __ATTR_ITCMRAM scaleTwo(float input)
{
	//input = LEAF_clip(0.f, input, 1.f);
	return (input * 2.0f);
}

float __ATTR_ITCMRAM scaleOscPitch(float input)
{
	//input = LEAF_clip(0.0f, input, 1.0f);
	return ((input * 2.0f) - 1.0f);
}

float __ATTR_ITCMRAM scaleOscFine(float input)
{
	//input = LEAF_clip(0.0f, input, 1.f);
	return (input * 200.0f) - 100.0f;
}

float __ATTR_ITCMRAM scaleOscFreq(float input)
{
	//input = LEAF_clip(0.f, input, 1.f);
	return (input * 4000.0f) - 2000.0f;
}

float __ATTR_ITCMRAM scaleTranspose(float input)
{
	input = LEAF_clip(0.0f, input, 1.f);
	return (input * 96.0f) - 48.0f;
}

float __ATTR_ITCMRAM scalePitchBend(float input)
{
	input = LEAF_clip(0.f, input, 1.f);
	return (input * 48.0f);
}

float __ATTR_ITCMRAM scaleFilterCutoff(float input)
{
	input = LEAF_clip(0.f, input, 1.f);
	return (input * 127.0f);
}

float __ATTR_ITCMRAM scaleFilterResonance(float input)
{
	//lookup table for filter res
	input = LEAF_clip(0.1f, input, 1.0f);
	//scale to lookup range
	input *= 2047.0f;
	int inputInt = (int)input;
	float inputFloat = (float)inputInt - input;
	int nextPos = LEAF_clip(0, inputInt + 1, 2047);
	return LEAF_clip(0.1f, (resTable[inputInt] * (1.0f - inputFloat)) + (resTable[nextPos] * inputFloat), 10.0f);
	//return
}

float __ATTR_ITCMRAM scaleEnvTimes(float input)
{
	//lookup table for env times
	input = LEAF_clip(0.0f, input, 1.0f);
	//scale to lookup range
	input *= 2047.0f;
	int inputInt = (int)input;
	float inputFloat = (float)inputInt - input;
	int nextPos = LEAF_clip(0, inputInt + 1, 2047);
	return (envTimeTable[inputInt] * (1.0f - inputFloat)) + (envTimeTable[nextPos] * inputFloat);

	//return
}

float __ATTR_ITCMRAM scaleLFORates(float input)
{
	//lookup table for LFO rates
	input = LEAF_clip(0.0f, input, 1.0f);
	//scale to lookup range
	input *= 2047.0f;
	int inputInt = (int)input;
	float inputFloat = (float)inputInt - input;
	int nextPos = LEAF_clip(0, inputInt + 1, 2047);
	return (lfoRateTable[inputInt] * (1.0f - inputFloat)) + (lfoRateTable[nextPos] * inputFloat);
	//return
}

float __ATTR_ITCMRAM scaleFinalLowpass(float input)
{
	input = LEAF_clip(0.f, input, 1.f);
	return ((input * 70.0f) + 58.0f);
}

volatile uint8_t chorusAssignment = 255;
volatile uint8_t delayAssignment = 255;

void setEffectsFunctions(FXType effectType, int i)
{
	effectsActive[i] = 1;
	switch (effectType)
	{
		  case None:
			  effectTick[i] = &blankTick;
			  effectSetters[i].setParam1 = &blankFunction;
			  effectSetters[i].setParam2 = &blankFunction;
			  effectSetters[i].setParam3 = &blankFunction;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  effectsActive[i] = 0;
			  break;
		  case Softclip:
			  effectTick[i] = &softClipTick;
			  effectSetters[i].setParam1 = &clipperGainSet;
			  effectSetters[i].setParam2 = &offsetParam2;
			  effectSetters[i].setParam3 = &param3Soft;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;

			  break;
		  case Hardclip:
			  effectTick[i] = &hardClipTick;
			  effectSetters[i].setParam1 = &clipperGainSet;
			  effectSetters[i].setParam2 = &offsetParam2;
			  effectSetters[i].setParam3 = &param3Hard;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case PolynomialShaper:
			  effectTick[i] = &polynomialShaperTick;
			  effectSetters[i].setParam1 = &clipperGainSet;
			  effectSetters[i].setParam2 = &offsetParam2;
			  effectSetters[i].setParam3 = &param3Poly;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case ABSaturator:
			  effectTick[i] = &satTick;
			  effectSetters[i].setParam1 = &clipperGainSet;
			  effectSetters[i].setParam2 = &offsetParam2;
			  effectSetters[i].setParam3 = &blankFunction;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case Tanh:
			  effectTick[i] = &tanhTick;
			  effectSetters[i].setParam1 = &clipperGainSet;
			  effectSetters[i].setParam2 = &offsetParam2;
			  effectSetters[i].setParam3 = &blankFunction;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case Shaper2:
			  effectTick[i] = &shaperTick;
			  effectSetters[i].setParam1 = &param1Linear;
			  effectSetters[i].setParam2 = &offsetParam2;
			  effectSetters[i].setParam3 = &param3Linear;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case Compressor:
			  effectTick[i] = &compressorTick;
			  effectSetters[i].setParam1 = &compressorParam1;
			  effectSetters[i].setParam2 = &compressorParam2;
			  effectSetters[i].setParam3 = &compressorParam3;
			  effectSetters[i].setParam4 = &compressorParam4;
			  effectSetters[i].setParam5 = &compressorParam5;
			  break;
		  case Chorus:
			  //there isn't enough memory for multiple chorus effects. Disable any other ticks using them
			  if (chorusAssignment != 255)
			  {
				  effectTick[chorusAssignment] = &blankTick;
				  effectSetters[chorusAssignment].setParam1 = &blankFunction;
				  effectSetters[chorusAssignment].setParam2 = &blankFunction;
				  effectSetters[chorusAssignment].setParam3 = &blankFunction;
				  effectSetters[chorusAssignment].setParam4 = &blankFunction;
				  effectSetters[chorusAssignment].setParam5 = &blankFunction;
			  }
			  chorusAssignment = i;
			  effectTick[i] = &chorusTick;
			  effectSetters[i].setParam1 = &chorusParam1;
			  effectSetters[i].setParam2 = &chorusParam2;
			  effectSetters[i].setParam3 = &chorusParam3;
			  effectSetters[i].setParam4 = &chorusParam4;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case Bitcrush:
			  effectTick[i] = &bcTick;
			  effectSetters[i].setParam1 = &clipperGainSet;
			  effectSetters[i].setParam2 = &param2BC;
			  effectSetters[i].setParam3 = &param3BC;
			  effectSetters[i].setParam4 = &param4BC;
			  effectSetters[i].setParam5 = &param5BC;
			  break;
		  case TiltFilter:
			  effectTick[i] = &tiltFilterTick;
			  effectSetters[i].setParam1 = &tiltParam1;
			  effectSetters[i].setParam2 = &tiltParam2;
			  effectSetters[i].setParam3 = &tiltParam3;
			  effectSetters[i].setParam4 = &tiltParam4;
			  effectSetters[i].setParam5 = &param5Linear;
			  break;
		  case Wavefolder:
			  effectTick[i] = &wavefolderTick;
			  effectSetters[i].setParam1 = &wavefolderParam1;
			  effectSetters[i].setParam2 = &offsetParam2;
			  effectSetters[i].setParam3 = &wavefolderParam3;
			  effectSetters[i].setParam4 = &param4Linear;
			  effectSetters[i].setParam5 = &param5Linear;
			  break;

		  case Delay:
			  //there isn't enough memory for multiple delay effects. Disable any other ticks using them
			  if (delayAssignment != 255)
			  {
				  effectTick[delayAssignment] = &blankTick;
				  effectSetters[delayAssignment].setParam1 = &blankFunction;
				  effectSetters[delayAssignment].setParam2 = &blankFunction;
				  effectSetters[delayAssignment].setParam3 = &blankFunction;
				  effectSetters[delayAssignment].setParam4 = &blankFunction;
				  effectSetters[delayAssignment].setParam5 = &blankFunction;
			  }
			  delayAssignment = i;
			  effectTick[i] = &delayTick;
			  effectSetters[i].setParam1 = &delayParam1;
			  effectSetters[i].setParam2 = &delayParam2;
			  effectSetters[i].setParam3 = &delayParam3;
			  effectSetters[i].setParam4 = &delayParam4;
			  effectSetters[i].setParam5 = &delayParam5;
			  break;

		  case FXLowpass :
			  effectTick[i] = &FXlowpassTick;
			  effectSetters[i].setParam1 = &FXLowpassParam1;
			  effectSetters[i].setParam2 = &blankFunction;
			  effectSetters[i].setParam3 = &FXLowpassParam3;
			  effectSetters[i].setParam4 = &blankFunction;;
			  effectSetters[i].setParam5 = &blankFunction;;
			  break;
		  case FXHighpass :
			  effectTick[i] = &FXhighpassTick;
			  effectSetters[i].setParam1 = &FXHighpassParam1;
			  effectSetters[i].setParam2 = &blankFunction;
			  effectSetters[i].setParam3 = &FXHighpassParam3;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case FXBandpass :
			  effectTick[i] = &FXbandpassTick;
			  effectSetters[i].setParam1 = &FXBandpassParam1;
			  effectSetters[i].setParam2 = &blankFunction;
			  effectSetters[i].setParam3 = &FXBandpassParam3;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case FXDiode :
			  effectTick[i] = &FXdiodeLowpassTick;
			  effectSetters[i].setParam1 = &FXDiodeParam1;
			  effectSetters[i].setParam2 = &blankFunction;
			  effectSetters[i].setParam3 = &FXDiodeParam3;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case FXPeak :
			  effectTick[i] = &FXVZpeakTick;
			  effectSetters[i].setParam1 = &FXPeakParam1;
			  effectSetters[i].setParam2 = &FXPeakParam2;
			  effectSetters[i].setParam3 = &FXPeakParam3;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case FXLowShelf :
			  effectTick[i] = &FXVZlowshelfTick;
			  effectSetters[i].setParam1 = &FXLowShelfParam1;
			  effectSetters[i].setParam2 = &FXLowShelfParam2;
			  effectSetters[i].setParam3 = &FXLowShelfParam3;
			  effectSetters[i].setParam4 = &blankFunction;
			  effectSetters[i].setParam5 = &blankFunction;
			  break;
		  case FXHighShelf :
			  effectTick[i] = FXVZhighshelfTick;
			  effectSetters[i].setParam1 = &FXHighShelfParam1;;
			  effectSetters[i].setParam2 = &FXHighShelfParam2;;
			  effectSetters[i].setParam3 = &FXHighShelfParam3;;
			  effectSetters[i].setParam4 = &blankFunction;;
			  effectSetters[i].setParam5 = &blankFunction;;
			  break;
		  case FXNotch :
			  effectTick[i] = FXVZbandrejectTick;
			  effectSetters[i].setParam1 = &FXNotchParam1;;
			  effectSetters[i].setParam2 = &FXNotchParam2;;
			  effectSetters[i].setParam3 = &FXNotchParam3;;
			  effectSetters[i].setParam4 = &blankFunction;;
			  effectSetters[i].setParam5 = &blankFunction;;
			  break;
		  case FXLadder :
			  effectTick[i] = &FXLadderLowpassTick;
			  effectSetters[i].setParam1 = &FXLadderParam1;;
			  effectSetters[i].setParam2 = &blankFunction;;
			  effectSetters[i].setParam3 = &FXLadderParam3;;
			  effectSetters[i].setParam4 = &blankFunction;;
			  effectSetters[i].setParam5 = &blankFunction;;
			  break;
		  default:
			  break;
	}
}

void setOscilllatorShapes(int oscshape, int i)
{
	switch (oscshape)
	{
		  case 0:
			  shapeTick[i] = &sawSquareTick;
			  break;
		  case 1:
			  shapeTick[i] = &sineTriTick;
			  break;
		  case 2:
			  shapeTick[i] = &sawTick;
			  break;
		  case 3:
			  shapeTick[i] = &pulseTick;
			  break;
		  case 4:
			  shapeTick[i] = &sineTick;
			  break;
		  case 5:
			  shapeTick[i] = &triTick;
			  break;
		  //case 6:
			  //shapeTick[i] = &userTick;
			  //break;
		  default:
			  break;
	}
}
void setFilterTypes(int filterType, int i)
{
	switch (filterType)
		{
			  case 0:
				  filterTick[i] = &lowpassTick;
				  filterSetters[i].setQ = &lowpassSetQ;
				  filterSetters[i].setGain = &lowpassSetGain;
				  break;
			  case 1:
				  filterTick[i] = &highpassTick;
				  filterSetters[i].setQ = &highpassSetQ;
				  filterSetters[i].setGain = &highpassSetGain;
				  break;
			  case 2:
				  filterTick[i] = &bandpassTick;
				  filterSetters[i].setQ = &bandpassSetQ;
				  filterSetters[i].setGain = &bandpassSetGain;
				  break;
			  case 3:
				  filterTick[i] = &diodeLowpassTick;
				  filterSetters[i].setQ = &diodeLowpassSetQ;
				  filterSetters[i].setGain = &diodeLowpassSetGain;
				  break;
			  case 4:
				  filterTick[i] = &VZpeakTick;
				  filterSetters[i].setQ = &VZpeakSetQ;
				  filterSetters[i].setGain = &VZpeakSetGain;
				  break;
			  case 5:
				  filterTick[i] = &VZlowshelfTick;
				  filterSetters[i].setQ = &VZlowshelfSetQ;
				  filterSetters[i].setGain = &VZlowshelfSetGain;
				  break;
			  case 6:
				  filterTick[i] = &VZhighshelfTick;
				  filterSetters[i].setQ = &VZhighshelfSetQ;
				  filterSetters[i].setGain = &VZhighshelfSetGain;
				  break;
			  case 7:
				  filterTick[i] = &VZbandrejectTick;
				  filterSetters[i].setQ = &VZbandrejectSetQ;
				  filterSetters[i].setGain = &VZbandrejectSetGain;
				  break;
			  case 8:
				  filterTick[i] = &LadderLowpassTick;
				  filterSetters[i].setQ = &LadderLowpassSetQ;
				  filterSetters[i].setGain = &LadderLowpassSetGain;
				  break;
			  default:
				  break;
		}
}

void setLFOShapes(int LFOShape, int i)
{
	switch(LFOShape)
	{
		case SineTriLFOShapeSet:
			lfoShapeTick[i] = &lfoSineTriTick;
			lfoSetters[i].setRate = &lfoSineTriSetRate;
			lfoSetters[i].setShape = &lfoSineTriSetShape;
			lfoSetters[i].setPhase = &lfoSineTriSetPhase;
			break;
		case SawPulseLFOShapeSet:
			lfoShapeTick[i] = &lfoSawSquareTick;
			lfoSetters[i].setRate = &lfoSawSquareSetRate;
			lfoSetters[i].setShape = &lfoSawSquareSetShape;
			lfoSetters[i].setPhase = &lfoSawSquareSetPhase;
			break;
		case SineLFOShapeSet:
			lfoShapeTick[i] = &lfoSineTick;
			lfoSetters[i].setRate = &lfoSineSetRate;
			lfoSetters[i].setShape = &lfoSineSetShape;
			lfoSetters[i].setPhase = &lfoSineSetPhase;
			break;
		case TriLFOShapeSet:
			lfoShapeTick[i] = &lfoTriTick;
			lfoSetters[i].setRate = &lfoTriSetRate;
			lfoSetters[i].setShape = &lfoTriSetShape;
			lfoSetters[i].setPhase = &lfoTriSetPhase;
			break;
		case SawLFOShapeSet:
			lfoShapeTick[i] = &lfoSawTick;
			lfoSetters[i].setRate = &lfoSawSetRate;
			lfoSetters[i].setShape = &lfoSawSetShape;
			lfoSetters[i].setPhase = &lfoSawSetPhase;
			break;
		case PulseLFOShapeSet:
			lfoShapeTick[i] = &lfoPulseTick;
			lfoSetters[i].setRate = &lfoPulseSetRate;
			lfoSetters[i].setShape = &lfoPulseSetShape;
			lfoSetters[i].setPhase = &lfoPulseSetPhase;
			break;
	}
}


uint8_t rowNumber = 0;
uint8_t arrayNumber = 0;
uint16_t positionInRowLine = 0;

uint8_t fromHex(char value)
{
	if (('0' <= value) && (value <= '9'))
		return (uint8_t) (value - '0');
	if (('a' <= value) && (value <= 'f'))
		return (uint8_t) (10 + value - 'a');
	if (('A' <= value) && (value <= 'F'))
		return (uint8_t) (10 + value - 'A');
	return 0;
}
uint8_t fromAscii(uint8_t input1, uint8_t input2)
{
	return ((fromHex(input1)<<4) | (fromHex(input2)));
}


void  parsePreset(int size, int presetNumber)
{
	//turn off the volume while changing parameters
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	volatile uint32_t tempCountParse = DWT->CYCCNT;
	uint16_t presetVersionNumber = 0;
	currentPresetSize = size;
	 __disable_irq();
	 presetReady = 0;
	 for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
	 {
		 audioOutBuffer[i] = 0;
	 }
	audioMasterLevel = 0.0f;
	//osc params


	chorusAssignment = 255;
	delayAssignment = 255;

	uint16_t bufferIndex = 0;

	//should add an indicator at the beginning of the version number.
	// maybe 4 bytes:
		// [0] = 17   marker1 letting the parser know that a preset version number will follow
	    // [1] = 18   marker2 letting the parser know that a preset version number will follow // or 19 to say it's storing an internal preset
	    // [2] = version number major (i.e. the 01 in 1.04)
	    // [3] = version number minor (i.e. the 04 in 1.04)

	//if a version number is indicated, then set the presetVersionNumber value for later use, otherwise use default versionNumber of 0
	if (buffer[bufferIndex] == 17)
	{
		if (buffer[bufferIndex + 1] == 18)
		{
			presetVersionNumber = ((buffer[bufferIndex + 2] << 8) + buffer[bufferIndex + 3]);

			bufferIndex = 4;
		}


		if (buffer[bufferIndex + 1] == 19) //this means its an internal model, not the subtractive synth
		{
			presetVersionNumber = ((buffer[bufferIndex + 2] << 8) + buffer[bufferIndex + 3]);



			bufferIndex = 4;


			//read first 14 items in buffer as the 14 character string that is the name of the preset
			for (int i = 0; i < 14; i++)
			{
				presetName[i] = buffer[bufferIndex];
				presetNamesArray[presetNumber][i] = buffer[bufferIndex];
				bufferIndex++;
			}

			bufferIndex = 20;

			bufferIndex++;


			//9-byte macros
			for (int j = 0; j < 8; j++)
			{
				for (int k = 0; k < 9; k++)
				{
					macroNamesArray[presetNumber][j][k] = buffer[bufferIndex];
					bufferIndex++;
				}
			}
			//10-byte macros
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 10; k++)
				{
					macroNamesArray[presetNumber][j+8][k] = buffer[bufferIndex];
					bufferIndex++;
				}
			}

			//remaining 9-byte macros
			for (int j = 0; j < 8; j++)
			{
				for (int k = 0; k < 9; k++)
				{
					macroNamesArray[presetNumber][j+12][k] = buffer[bufferIndex];
					bufferIndex++;
				}
			}

			for (int i = 0; i < 20; i++)
			{
				//copy the parameters into the default KnobParams Buffer
				loadedKnobParams[i] = INV_TWO_TO_16 * ((buffer[bufferIndex] << 8) + buffer[bufferIndex+1]);
				bufferIndex = bufferIndex + 2;
			}
			presetWaitingToParse = 0;



			audioMasterLevel = 1.0f;

			__enable_irq();
			presetReady = 1;
			//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
			diskBusy = 0;

			return;
		}



	}

	//read first 14 items in buffer as the 14 character string that is the name of the preset
	for (int i = 0; i < 14; i++)
	{
		presetName[i] = buffer[bufferIndex];
		presetNamesArray[presetNumber][i] = buffer[bufferIndex];
		bufferIndex++;
	}
	//9-byte macros
	for (int j = 0; j < 8; j++)
	{
		for (int k = 0; k < 9; k++)
		{
			macroNamesArray[presetNumber][j][k] = buffer[bufferIndex];
			bufferIndex++;
		}
	}
	//10-byte macros
	for (int j = 0; j < 4; j++)
	{
		for (int k = 0; k < 10; k++)
		{
			macroNamesArray[presetNumber][j+8][k] = buffer[bufferIndex];
			bufferIndex++;
		}
	}

	// blank out 9-byte macros after
	for (int j = 0; j < 8; j++)
	{
		for (int k = 0; k < 9; k++)
		{
			macroNamesArray[presetNumber][j+12][k] = 32;
		}
	}

	//read first element in buffer (after the 14 character name) as a count of how many parameters
	uint16_t paramCount = (buffer[bufferIndex] << 8) + buffer[bufferIndex+1];
	if (paramCount > size)
	{
		//error in transmission - give up and don't parse!
		audioMasterLevel = 1.0f;
		presetWaitingToParse = 0;
		 presetReady = 0;

		__enable_irq();
		return;
	}

	//check the validity of the transfer by verifying that the param array and mapping arrays both end with the required 0xefef values
	//should make this a real checksum
	uint16_t paramEndCheck = (buffer[paramCount*2+bufferIndex+2] << 8) + buffer[paramCount*2+bufferIndex+3];
	if (paramEndCheck != 0xefef)
	{
		//error in transmission - give up and don't parse!
		audioMasterLevel = 1.0f;
		presetWaitingToParse = 0;
		 presetReady = 0;
		__enable_irq();

		return;
	}
	uint16_t mappingCount = (buffer[paramCount*2+bufferIndex+4] << 8) + buffer[paramCount*2+bufferIndex+5];


	//20 is the 6 bytes plus the 14 characters
	uint16_t mappingEndLocation = 0;
	if (presetVersionNumber == 0)
	{
		mappingEndLocation = (paramCount * 2) + (mappingCount * 5) + bufferIndex+6;
	}
	else
	//paramCount is * 2 because they are 2 bytes per param, mappingCount * 6 because they are 6 bytes per mapping (changed from 5 to allow for sending of slot locations)
	{
		mappingEndLocation = (paramCount * 2) + (mappingCount * 6) + bufferIndex+6;
	}

	if (mappingEndLocation > size)
	{
		//error in transmission - give up and don't parse!
		audioMasterLevel = 1.0f;
		presetWaitingToParse = 0;
		 presetReady = 0;
		__enable_irq();

		return;
	}

	uint16_t mappingEndCheck = (buffer[mappingEndLocation] << 8) + buffer[mappingEndLocation+1];
	if (mappingEndCheck != 0xfefe) //this check value is 0xfefe
	{
		//error in transmission - give up and don't parse!
		audioMasterLevel = 1.0f;
		presetWaitingToParse = 0;
		 presetReady = 0;
		__enable_irq();

		return;
	}





	 //move past the name characters (4byte version, 14 bytes name, 9-byte * 8, 10-byte * 4) and paramcount position (2 bytes) in the buffer to start parsing the parameter data
	bufferIndex = bufferIndex + 2;

	//now read the parameters

	for (int i = 0; i < paramCount; i++)
	{
		for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
		{
			params[i].zeroToOneVal[v] = INV_TWO_TO_16 * ((buffer[bufferIndex] << 8) + buffer[bufferIndex+1]);
		}
		//need to map all of the params to their scaled parameters and set them to the realVals
		params[i].scaleFunc = &scaleDefault;

		//blank function means that it doesn't actually set a final value, we will read directly from the realVals when we need it
		params[i].setParam = &blankFunction;

		bufferIndex += 2;
	}


	//if loading old presets that don't have as many params, blank out the empty slots
	for (int i = paramCount; i < NUM_PARAMS; i++)
	{
		for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
		{
			params[i].zeroToOneVal[v] = 0.0f;
		}
		params[i].scaleFunc = &scaleDefault;
		params[i].setParam = &blankFunction;
	}


	//params[Master].scaleFunc = &scaleTwo;
	params[Transpose].scaleFunc = &scaleTranspose;
	params[PitchBendRange].scaleFunc = &scalePitchBend;
	//params[NoiseAmp].scaleFunc = &scaleTwo;
	params[Osc1Pitch].scaleFunc = &scaleOscPitch;
	params[Osc1Fine].scaleFunc = &scaleOscFine;
	params[Osc1Freq].scaleFunc = &scaleOscFreq;
	//params[Osc1Amp].scaleFunc = &scaleTwo;
	params[Osc1Harmonics].scaleFunc = &scaleOscPitch;
	params[Osc2Pitch].scaleFunc = &scaleOscPitch;
	params[Osc2Fine].scaleFunc = &scaleOscFine;
	params[Osc2Freq].scaleFunc = &scaleOscFreq;
	//params[Osc2Amp].scaleFunc = &scaleTwo;
	params[Osc2Harmonics].scaleFunc = &scaleOscPitch;
	params[Osc3Pitch].scaleFunc = &scaleOscPitch;
	params[Osc3Fine].scaleFunc = &scaleOscFine;
	params[Osc3Freq].scaleFunc = &scaleOscFreq;
	//params[Osc3Amp].scaleFunc = &scaleTwo;
	params[Osc3Harmonics].scaleFunc = &scaleOscPitch;
	params[Filter1Cutoff].scaleFunc = &scaleFilterCutoff;
	params[Filter1Resonance].scaleFunc = &scaleFilterResonance;
	params[Filter2Cutoff].scaleFunc = &scaleFilterCutoff;
	params[Filter2Resonance].scaleFunc = &scaleFilterResonance;
	params[Envelope1Attack].scaleFunc = &scaleEnvTimes;
	params[Envelope1Decay].scaleFunc = &scaleEnvTimes;
	params[Envelope1Release].scaleFunc = &scaleEnvTimes;
	params[Envelope2Attack].scaleFunc = &scaleEnvTimes;
	params[Envelope2Decay].scaleFunc = &scaleEnvTimes;
	params[Envelope2Release].scaleFunc = &scaleEnvTimes;
	params[Envelope3Attack].scaleFunc = &scaleEnvTimes;
	params[Envelope3Decay].scaleFunc = &scaleEnvTimes;
	params[Envelope3Release].scaleFunc = &scaleEnvTimes;
	params[Envelope4Attack].scaleFunc = &scaleEnvTimes;
	params[Envelope4Decay].scaleFunc = &scaleEnvTimes;
	params[Envelope4Release].scaleFunc = &scaleEnvTimes;
	params[LFO1Rate].scaleFunc = &scaleLFORates;
	params[LFO2Rate].scaleFunc = &scaleLFORates;
	params[LFO3Rate].scaleFunc = &scaleLFORates;
	params[LFO4Rate].scaleFunc = &scaleLFORates;
	//params[OutputAmp].scaleFunc = &scaleTwo;
	params[OutputTone].scaleFunc  = &scaleFinalLowpass;
	for (int i = 0; i < NUM_EFFECT; i++)
	{
		for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
		{
			FXType effectType = roundf(params[Effect1FXType + (EffectParamsNum * i)].zeroToOneVal[v] * (NUM_EFFECT_TYPES-1));
			param *FXAlias = &params[Effect1Param1 + (EffectParamsNum*i)];


			if (effectType > FXLowpass) // this assumes filters are the last effects
			{
				FXAlias[2].scaleFunc = &scaleFilterResonance;
			}
			setEffectsFunctions(effectType, i);
		}
	}
	for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
	{
		for (int i = 0; i < NUM_PARAMS; i++)
		{
			params[i].realVal[v] = params[i].scaleFunc(params[i].zeroToOneVal[v]);
		}
	}
	uint8_t enabledCount = 0;


	for (int i = 0; i < NUM_OSC; i++)
	{
		int oscshape = roundf(params[Osc1ShapeSet + (OscParamsNum * i)].realVal[0] * (NUM_OSC_SHAPES-1));
		setOscilllatorShapes(oscshape, i);
		if (params[Osc1 + (OscParamsNum * i)].realVal[0]  > 0.5f)
		{
			enabledCount++;
			oscsEnabled[i] = 1;
			oscOn[i] = 1;
		}
		else
		{
			oscsEnabled[i] = 0;
			oscOn[i] = 0;
		}
	}
	//set amplitude of oscillators based on how many are enabled
	oscAmpMult = oscAmpMultArray[enabledCount];

	if (params[Noise].realVal[0]  > 0.5f)
	{
		noiseOn = 1;
	}
	else
	{
		noiseOn = 0;
	}

	for (int i = 0; i < NUM_FILT; i++)
	{
		int filterType = roundf(params[Filter1Type + (i * FilterParamsNum)].realVal[0] * (NUM_FILTER_TYPES-1));
		setFilterTypes(filterType, i);
	}

	for (int i = 0; i < NUM_LFOS; i++)
	{
		int LFOShape = roundf(params[LFO1ShapeSet + (i * LFOParamsNum)].realVal[0] * (NUM_LFO_SHAPES-1));
		setLFOShapes(LFOShape, i);
	}


	//noiseparams
	params[NoiseTilt].setParam = &noiseSetTilt;
	params[NoisePeakFreq].setParam = &noiseSetFreq;
	params[NoisePeakGain].setParam  = &noiseSetGain;
	///////Setters for paramMapping
	params[Master].setParam = &setMaster;
	params[Transpose].setParam = &setTranspose;
	params[PitchBendRange].setParam = &setPitchBendRange;
	params[OutputTone].setParam = &setFinalLowpass;

	params[Osc1Pitch].setParam = &setFreqMultPitch;
	params[Osc2Pitch].setParam = &setFreqMultPitch;
	params[Osc3Pitch].setParam = &setFreqMultPitch;

	params[Osc1Harmonics].setParam = &setFreqMultHarm;
	params[Osc2Harmonics].setParam = &setFreqMultHarm;
	params[Osc3Harmonics].setParam = &setFreqMultHarm;

	params[Effect1Param1].setParam = effectSetters[0].setParam1;
	params[Effect1Param2].setParam = effectSetters[0].setParam2;
	params[Effect1Param3].setParam = effectSetters[0].setParam3;
	params[Effect1Param4].setParam = effectSetters[0].setParam4;
	params[Effect1Param5].setParam = effectSetters[0].setParam5;
	params[Effect1Mix].setParam = &fxMixSet;
	params[Effect1PostGain].setParam = &fxPostGainSet;
	params[Effect2Param1].setParam = effectSetters[1].setParam1;
	params[Effect2Param2].setParam = effectSetters[1].setParam2;
	params[Effect2Param3].setParam = effectSetters[1].setParam3;
	params[Effect2Param4].setParam = effectSetters[1].setParam4;
	params[Effect2Param5].setParam = effectSetters[1].setParam5;
	params[Effect2Mix].setParam = &fxMixSet;
	params[Effect2PostGain].setParam = &fxPostGainSet;
	params[Effect3Param1].setParam = effectSetters[2].setParam1;
	params[Effect3Param2].setParam = effectSetters[2].setParam2;
	params[Effect3Param3].setParam = effectSetters[2].setParam3;
	params[Effect3Param4].setParam = effectSetters[2].setParam4;
	params[Effect3Param5].setParam = effectSetters[2].setParam5;
	params[Effect3Mix].setParam = &fxMixSet;
	params[Effect3PostGain].setParam = &fxPostGainSet;
	params[Effect4Param1].setParam = effectSetters[3].setParam1;
	params[Effect4Param2].setParam = effectSetters[3].setParam2;
	params[Effect4Param3].setParam = effectSetters[3].setParam3;
	params[Effect4Param4].setParam = effectSetters[3].setParam4;
	params[Effect4Param5].setParam = effectSetters[3].setParam5;
	params[Effect4Mix].setParam = &fxMixSet;
	params[Effect4PostGain].setParam = &fxPostGainSet;
	params[Filter1Resonance].setParam = filterSetters[0].setQ;
	params[Filter1Gain].setParam = filterSetters[0].setGain;
	params[Filter2Resonance].setParam = filterSetters[1].setQ;
	params[Filter2Gain].setParam = filterSetters[1].setGain;
	params[Envelope1Attack].setParam = &setEnvelopeAttack;
	params[Envelope1Decay].setParam = &setEnvelopeDecay;
	params[Envelope1Sustain].setParam = &setEnvelopeSustain;
	params[Envelope1Release].setParam = &setEnvelopeRelease;
	params[Envelope1Leak].setParam = &setEnvelopeLeak;
	params[Envelope2Attack].setParam = &setEnvelopeAttack;
	params[Envelope2Decay].setParam = &setEnvelopeDecay;
	params[Envelope2Sustain].setParam = &setEnvelopeSustain;
	params[Envelope2Release].setParam = &setEnvelopeRelease;
	params[Envelope2Leak].setParam = &setEnvelopeLeak;
	params[Envelope3Attack].setParam = &setEnvelopeAttack;
	params[Envelope3Decay].setParam = &setEnvelopeDecay;
	params[Envelope3Sustain].setParam = &setEnvelopeSustain;
	params[Envelope3Release].setParam = &setEnvelopeRelease;
	params[Envelope3Leak].setParam = &setEnvelopeLeak;
	params[Envelope4Attack].setParam = &setEnvelopeAttack;
	params[Envelope4Decay].setParam = &setEnvelopeDecay;
	params[Envelope4Sustain].setParam = &setEnvelopeSustain;
	params[Envelope4Release].setParam = &setEnvelopeRelease;
	params[Envelope4Leak].setParam = &setEnvelopeLeak;
	params[LFO1Rate].setParam = lfoSetters[0].setRate;
	params[LFO2Rate].setParam = lfoSetters[1].setRate;
	params[LFO3Rate].setParam = lfoSetters[2].setRate;
	params[LFO4Rate].setParam = lfoSetters[3].setRate;
	params[LFO1Shape].setParam = lfoSetters[0].setShape;
	params[LFO2Shape].setParam = lfoSetters[1].setShape;
	params[LFO3Shape].setParam = lfoSetters[2].setShape;
	params[LFO4Shape].setParam = lfoSetters[3].setShape;
	params[LFO1Phase].setParam = lfoSetters[0].setPhase;
	params[LFO2Phase].setParam = lfoSetters[1].setPhase;
	params[LFO3Phase].setParam = lfoSetters[2].setPhase;
	params[LFO4Phase].setParam = lfoSetters[3].setPhase;
	params[OutputAmp].setParam = &setAmp;



	for (int i = 0; i < NUM_PARAMS; i++)
	{
		params[i].objectNumber = 0;
		//oscillators
		if ((i >= Osc1) && (i < Osc2))
		{
			params[i].objectNumber = 0;
		}
		else if ((i >= Osc2) && (i < Osc3))
		{
			params[i].objectNumber = 1;
		}
		else if ((i >= Osc3) && (i < Effect1FXType))
		{
			params[i].objectNumber = 2;
		}
		//effects
		//filters
		else if ((i >= Filter1) && (i < Filter2))
		{
			params[i].objectNumber = 0;
		}
		else if ((i >= Filter2) && (i < Envelope1Attack))
		{
			params[i].objectNumber = 1;
		}
		//envelopes
		else if ((i >= Envelope1Attack) && (i < Envelope2Attack))
		{
			params[i].objectNumber = 0;
		}
		else if ((i >= Envelope2Attack) && (i < Envelope3Attack))
		{
			params[i].objectNumber = 1;
		}
		else if ((i >= Envelope3Attack) && (i < Envelope4Attack))
		{
			params[i].objectNumber = 2;
		}
		else if ((i >= Envelope4Attack) && (i < LFO1Rate))
		{
			params[i].objectNumber = 3;
		}
		//lfos
		else if ((i >= LFO1Rate) && (i < LFO2Rate))
		{
			params[i].objectNumber = 0;
		}
		else if ((i >= LFO2Rate) && (i < LFO3Rate))
		{
			params[i].objectNumber = 1;
		}
		else if ((i >= LFO3Rate) && (i < LFO4Rate))
		{
			params[i].objectNumber = 2;
		}
		else if ((i >= LFO4Rate) && (i < OutputAmp))
		{
			params[i].objectNumber = 3;
		}
		//effects
		else if ((i >= Effect1FXType) && (i < Effect2FXType))
		{
			params[i].objectNumber = 0;
		}
		else if ((i >= Effect2FXType) && (i < Effect3FXType))
		{
			params[i].objectNumber = 1;
		}
		else if ((i >= Effect3FXType) && (i < Effect4FXType))
		{
			params[i].objectNumber = 2;
		}
		else if ((i >= Effect4FXType) && (i < Filter1))
		{
			params[i].objectNumber = 3;
		}

		for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
		{
			params[i].setParam(params[i].realVal[v], params[i].objectNumber, v);
		}

	}
	float tempDenomMidi = LEAF_clip(1.0f, ((params[MIDIKeyMax].realVal[0]*127.0f) - (params[MIDIKeyMin].realVal[0]*127.0f)), 127.0f);

	midiKeyDivisor = 1.0f / tempDenomMidi;
	midiKeySubtractor = (params[MIDIKeyMin].realVal[0] * 127.0f);
	fxPre = params[FXOrder].realVal[0] > 0.5f;
	if (presetVersionNumber > 0)
	{
		pedalControlsMaster = params[PedalControlsMaster].realVal[0] > 0.5f;
	}
	else
	{
		pedalControlsMaster = 1;
	}
	//mappings parsing

	//move past the countcheck elements (already checked earlier)
	bufferIndex += 2;

	//move past the mappingCount elements (already stored that value earlier)
	bufferIndex += 2;

	numMappings = 0;
	for (int i = 0; i < NUM_LFOS; i++)
	{
		lfoOn[i] = 0;
	}
	for (int i = 0; i < NUM_ENV; i++)
	{
		envOn[i] = 0;
	}

	for (int i = 0; i < 12; i++)
	{
		knobFrozen[i] = 0;
		knobTicked[i] = 0;
	}

	//blank out all current mappings
	for (int i = 0; i < MAX_NUM_MAPPINGS; i++)
	{
		mappings[i].destNumber = 255;
		mappings[i].hookActive[0] = 0;
		mappings[i].hookActive[1] = 0;
		mappings[i].hookActive[2] = 0;
		mappings[i].numHooks = 0;
	}



	for (int i = 0; i < mappingCount; i++)
	{
		uint8_t destNumber = buffer[bufferIndex+1];
		uint8_t whichMapping = 0;
		uint8_t whichHook = 0;
		uint8_t foundOne = 0;


		// TODO: replace this search with explicit mapping slots instead
			// we need to add sending of mapping slots

		if (presetVersionNumber > 0)
		{
			whichHook = buffer[bufferIndex+5]; //slotID sent as last bit of data in new preset sending versions
		}
		//search to see if this destination already has other mappings
		for (int j = 0; j < MAX_NUM_MAPPINGS; j++)
		{
			if (mappings[j].destNumber == destNumber)
			{
				//found one, use this mapping and add another hook to it
				whichMapping = j;
				if (presetVersionNumber == 0)
				{
					whichHook = mappings[j].numHooks;
				}
				foundOne = 1;
			}
		}
		if (foundOne == 0)
		{
			//didn't find another mapping with this destination, start a new mapping
			whichMapping = numMappings;

			numMappings++;
			if (presetVersionNumber == 0)
			{
				whichHook = 0;
			}
			mappings[whichMapping].destNumber = destNumber;
			mappings[whichMapping].dest = &params[destNumber];

		}
		mappings[whichMapping].sourceSmoothed[whichHook] = 1;

		int source = buffer[bufferIndex];

		for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
		{
			mappings[whichMapping].sourceValPtr[whichHook][v] = &sourceValues[source][v];
		}

		if (source < 4) //if it's oscillators or noise (the first 4 elements of the source array), don't smooth to allow FM
		{
			mappings[whichMapping].sourceSmoothed[whichHook] = 0;

		}
		if ((source >= LFO_SOURCE_OFFSET) && (source < (LFO_SOURCE_OFFSET + NUM_LFOS)))
		{
			lfoOn[source - LFO_SOURCE_OFFSET] = 1;
		}
		if ((source >= ENV_SOURCE_OFFSET) && (source < (ENV_SOURCE_OFFSET + NUM_ENV)))
		{
			envOn[source - ENV_SOURCE_OFFSET] = 1;
		}
		if ((source >= OSC_SOURCE_OFFSET) && (source < (OSC_SOURCE_OFFSET+NUM_OSC)))
		{
			oscOn[source - OSC_SOURCE_OFFSET] = 1;
		}
		if ((source >= NOISE_SOURCE_OFFSET) && (source < (NOISE_SOURCE_OFFSET+1)))
		{
			noiseOn = 1;
		}
		if ((source >= MACRO_SOURCE_OFFSET) && (source < (MACRO_SOURCE_OFFSET + NUM_MACROS + NUM_CONTROL)))
		{
			//if it's a macro, also set its value and set the knob to frozen state so it'll hold until the knob is moved.

			uint8_t whichMacro = source - MACRO_SOURCE_OFFSET;
			for (int v = 0; v < numStringsThisBoard; v++)
			{
				sourceValues[source][v] = params[whichMacro + MACRO_PARAMS_OFFSET].realVal[v];
			}
			//set starting point for the knob smoothers to smooth from
			tExpSmooth_setValAndDest(knobSmoothers[whichMacro], params[whichMacro + MACRO_PARAMS_OFFSET].realVal[0]);
			knobFrozen[whichMacro] = 1;
			knobTicked[whichMacro] = 1;
		}
		if ((source >= PEDAL_SOURCE_OFFSET) && (source < (PEDAL_SOURCE_OFFSET + 10)))
		{
			pedalTicked[source - PEDAL_SOURCE_OFFSET] = 1;
		}
		int scalar = buffer[bufferIndex+2];
		for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
		{
			if (scalar == 0xff)
			{
				mappings[whichMapping].scalarSourceValPtr[whichHook][v] = &defaultScaling;
			}
			else
			{
				mappings[whichMapping].scalarSourceValPtr[whichHook][v] = &sourceValues[scalar][v];
				if ((scalar >= LFO_SOURCE_OFFSET) && (scalar < (LFO_SOURCE_OFFSET + NUM_LFOS)))
				{
					lfoOn[scalar - LFO_SOURCE_OFFSET] = 1;
				}
				if ((scalar >= ENV_SOURCE_OFFSET) && (scalar < (ENV_SOURCE_OFFSET + NUM_ENV)))
				{
					envOn[scalar - ENV_SOURCE_OFFSET] = 1;
				}
				if ((scalar >= OSC_SOURCE_OFFSET) && (scalar < (OSC_SOURCE_OFFSET + NUM_OSC)))
				{
					oscOn[scalar - OSC_SOURCE_OFFSET] = 1;
				}
				if ((scalar >= NOISE_SOURCE_OFFSET) && (scalar < (NOISE_SOURCE_OFFSET + 1)))
				{
					noiseOn = 1;
				}
				if ((scalar >= MACRO_SOURCE_OFFSET) && (scalar < (MACRO_SOURCE_OFFSET + NUM_MACROS + NUM_CONTROL)))
				{
					//if it's a macro, also set its value and set the knob to frozen state so it'll hold until the knob is moved.
					uint8_t whichMacro = scalar - MACRO_SOURCE_OFFSET;
					for (int v = 0; v < numStringsThisBoard; v++)
					{
						sourceValues[scalar][v] = params[whichMacro + MACRO_PARAMS_OFFSET].realVal[v];
					}
					//set starting point for the knob smoothers to smooth from
					tExpSmooth_setValAndDest(knobSmoothers[whichMacro], params[whichMacro + MACRO_PARAMS_OFFSET].realVal[0]);
					knobFrozen[whichMacro] = 1;
					knobTicked[whichMacro] = 1;
				}
				if ((scalar >= PEDAL_SOURCE_OFFSET) && (scalar < (PEDAL_SOURCE_OFFSET + 10)))
				{
					pedalTicked[scalar - PEDAL_SOURCE_OFFSET] = 1;
				}
			}
		}
		int16_t amountInt = (buffer[bufferIndex+3] << 8) + buffer[bufferIndex+4];
		float amountFloat = (float)amountInt * INV_TWO_TO_15;
//		//if the source is bipolar (oscillators, noise, and LFOs) then double the amount because it comes in as only half the range
//		if ((source < 4) || ((source >= LFO_SOURCE_OFFSET) && (source < (LFO_SOURCE_OFFSET + NUM_LFOS))))
//		{
//			amountFloat *= 2.0f;
//		}
		mappings[whichMapping].amount[whichHook] = amountFloat;
		mappings[whichMapping].hookActive[whichHook] = 1;
		mappings[whichMapping].numHooks++;

		if (presetVersionNumber > 0)
		{
			bufferIndex += 6;
		}
		else
		{
			bufferIndex += 5;
		}

	}

	uint8_t totalFilters = 0;
	if (params[Filter1].zeroToOneVal[0])
	{
		totalFilters++;
	}
	if (params[Filter2].zeroToOneVal[0])
	{
		totalFilters++;
	}

	//tick mappings once with no smoothing to set initial values
	for (int i = 0; i < numMappings; i++)
	{
		if (mappings[i].destNumber != 255)
		{
			for (int v = 0; v < numStringsThisBoard; v++)
			{
				float unsmoothedValue = 0.0f;

				for (int j = 0; j < 3; j++)
				{
					if (mappings[i].hookActive[j])
					{
						float sum = *mappings[i].sourceValPtr[j][v] * mappings[i].amount[j] * *mappings[i].scalarSourceValPtr[j][v];

						unsmoothedValue += sum;

					}
				}
				float finalVal = unsmoothedValue + mappings[i].dest->zeroToOneVal[v];

				//now scale the value with the correct scaling function
				mappings[i].dest->realVal[v] = mappings[i].dest->scaleFunc(finalVal);

				//and pop that value where it belongs by setting the actual parameter
				mappings[i].dest->setParam(mappings[i].dest->realVal[v], mappings[i].dest->objectNumber, v);
			}
		}
	}

	presetWaitingToParse = 0;
	audioMasterLevel = 1.0f;
	oscToTick = NUM_OSC;
	overSampled = 1;
	changeOversampling(overSampled);
	//currentPreset = presetNumberToWrite;
	OLED_writePreset();

	parseCount = DWT->CYCCNT - tempCountParse;
	__enable_irq();
	presetReady = 1;
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	diskBusy = 0;
	//receivingI2C = 0;

}





//translate that preset! this is to take the complete multi-chunk sysex midi message that has been received and stored
// and format it in the binary EBP (electrobass preset) format. Most of the 7-bit messages have to be reconstructed into floats and then stored as 16-bit ints.

void parseSysexPreset()
{
	parsingSysex = 1;
	presetReady = 0;
	uint32_t messageStart = sysexMessageStartPoints[sysexMessageStartPointsReadPosition];
	uint32_t messageEnd = sysexMessageStartPoints[(sysexMessageStartPointsReadPosition + 1) & 255];
	sysexMessageStartPointsReadPosition = (sysexMessageStartPointsReadPosition + 1) & 255;
	sysexReadPointer = messageStart;

    if (sysexBuffer[sysexReadPointer & sysexPointerMask] == 0)
    {
    	sysexParseInProgress = 1; // set a flag that we've started a sysex preset transfer. May take multiple sysex parse calls on the chunks to complete
        currentFloat = 0;
        presetArraySection = presetNameSection;
        presetNumberToWrite = sysexBuffer[(sysexReadPointer+1) & sysexPointerMask];
        buffer[0] = sysexBuffer[(sysexReadPointer+2) & sysexPointerMask];
        buffer[1] = sysexBuffer[(sysexReadPointer+3) & sysexPointerMask];
        buffer[2] = sysexBuffer[(sysexReadPointer+4) & sysexPointerMask];
        buffer[3] = sysexBuffer[(sysexReadPointer+5) & sysexPointerMask];

        union breakFloat theVal;
        uint32_t i = 6;
        sysexReadPointer = i + sysexReadPointer;
        uint8_t stoppingPoint = PRESET_NAME_LENGTH_IN_BYTES+i;
        for (; i < stoppingPoint; i++)
        {
        	buffer[i-2] = sysexBuffer[sysexReadPointer & sysexPointerMask] & 127; // pass on the first 14 elements as 8-bit bytes (they are the chars for the name string)
            presetNamesArray[presetNumberToWrite][i-6] = sysexBuffer[sysexReadPointer & sysexPointerMask] & 127;
            sysexReadPointer++;
        }

        presetArraySection = macroNamesSection;


        for (int j = 0; j < (NUM_MACROS); j++)
        {
            for (int k = 0; k < MACRO_NAME_LENGTH_IN_BYTES; k++)
            {
            	buffer[i-2] = sysexBuffer[sysexReadPointer & sysexPointerMask] & 127; // pass on the first 14 elements as 8-bit bytes (they are the chars for the name string)
                macroNamesArray[presetNumberToWrite][j][k] = sysexBuffer[sysexReadPointer & sysexPointerMask] & 127; // pass on the first 14 elements as 8-bit bytes (they are the chars for the name string)
                i++;
                sysexReadPointer++;
            }
        }
        for (int j = 0; j < NUM_CONTROLS; j++)
        {
            for (int k = 0; k < CONTROL_NAME_LENGTH_IN_BYTES; k++)
            {
            	buffer[i-2] = sysexBuffer[sysexReadPointer & sysexPointerMask] & 127; // pass on the first 14 elements as 8-bit bytes (they are the chars for the name string)
            	macroNamesArray[presetNumberToWrite][j+8][k] = sysexBuffer[sysexReadPointer & sysexPointerMask] & 127; // pass on the first 14 elements as 8-bit bytes (they are the chars for the name string)
                 i++;
                sysexReadPointer++;
            }
        }

        uint16_t valsStart = 4 + PRESET_NAME_LENGTH_IN_BYTES + (MACRO_NAME_LENGTH_IN_BYTES * NUM_MACROS) + (CONTROL_NAME_LENGTH_IN_BYTES * NUM_CONTROLS);

        presetArraySection = initialValsSection;

        for (; sysexReadPointer < (messageEnd); sysexReadPointer = (sysexReadPointer+5))
        {
            theVal.u32 = 0;
            theVal.u32 |= ((sysexBuffer[sysexReadPointer & sysexPointerMask ] &15) << 28);
            theVal.u32 |= (sysexBuffer[(sysexReadPointer+1) & sysexPointerMask] << 21);
            theVal.u32 |= (sysexBuffer[(sysexReadPointer+2) & sysexPointerMask] << 14);
            theVal.u32 |= (sysexBuffer[(sysexReadPointer+3) & sysexPointerMask] << 7);
            theVal.u32 |= (sysexBuffer[(sysexReadPointer+4) & sysexPointerMask] & 127);
            myTestVal = theVal.f;
            if (presetArraySection == initialValsSection)
            {

                if (currentFloat == 0)
                {
                    valsCount = (uint16_t) theVal.f;
                    buffer[valsStart + currentFloat++] = valsCount >> 8;
                    buffer[valsStart + currentFloat++] = valsCount & 0xff;
                }
                else if (currentFloat < ((valsCount+1)*2))
                {
                    uint16_t intVal = (uint16_t)(theVal.f * 65535.0f);
                    buffer[valsStart + currentFloat++] = intVal >> 8;
                    buffer[valsStart + currentFloat++] = intVal & 0xff;
                }
                else if (currentFloat == ((valsCount+1)*2))
                {
                    valCheck = theVal.f;
                    if ((valCheck < -1.5f) && (valCheck > -2.5f))
                    {
                    	buffer[valsStart + currentFloat++] = 0xef;
                    	buffer[valsStart + currentFloat++] = 0xef;
                        presetArraySection = mapCountNextSection;
                        mapCount = 0;
                    }
                    else
                    {
                        //error state
                    	sysexParseError++;
                    	parsingSysex = 0;
                    }
                }
            }
            else if (presetArraySection == mapCountNextSection)
            {
                mapCountExpectation = (uint16_t)theVal.f;
                buffer[valsStart + currentFloat++] = mapCountExpectation >> 8;
                buffer[valsStart + currentFloat++] = mapCountExpectation & 0xff;
                presetArraySection = mappingSection;
                numMappings = 0;
            }
            else if (presetArraySection == mappingSection)
            {
                // this is the order
                // source (int), target (int), scalarSource (arrives as -1.0f if no scalar, send as 255 if no scalar)(int), range (float -1.0 to 1.0), slot# (in uint8_t)
                if (numMappings < mapCountExpectation)
                {
                    if ((mapCount % 5) == 0)
                    {
                    	buffer[valsStart + currentFloat++] = (uint8_t)theVal.f;
                    }
                    else if  (mapCount % 5 == 1)
                    {
                    	buffer[valsStart + currentFloat++] = (uint8_t)theVal.f;
                    }
                    else if (mapCount % 5 == 2) //check if the scalar source is -1 (if so send 255 instead of a valid source number)
                    {
                        if (theVal.f < 0.0f)
                        {
                        	buffer[valsStart + currentFloat++] = 0xff;
                        }
                        else
                        {
                        	buffer[valsStart + currentFloat++] = (uint8_t)theVal.f;
                        }
                    }
                    else if (mapCount % 5 == 3)
                    {
                        int16_t intVal = (int16_t)(theVal.f * 32767.0f); //keep it signed to allow negative numbers
                        buffer[valsStart + currentFloat++] = intVal >> 8;
                        buffer[valsStart + currentFloat++] = intVal & 0xff;

                    }
                    else
                    {
                    	buffer[valsStart + currentFloat++] = (uint8_t)theVal.f;
                        numMappings++;
                    }
                    mapCount++;
                }


                else
                {
                    //mapcount ended
                    if ((theVal.f < -2.5f) && (theVal.f > -3.5f))
                    {
                    	buffer[valsStart + currentFloat++] = 0xfe;
                    	buffer[valsStart + currentFloat++] = 0xfe;
                        presetArraySection = presetEndSection;
                        parsingSysex = 0;
                        presetWaitingToWrite = valsStart + currentFloat;
                        presetWaitingToParse = valsStart + currentFloat;
                        parseThatMF = 0;
                        //prese = presetNumberToWrite;
                        //messageArraySize = valsStart + currentFloat;
                    }
                    else
                    {
                        //error state
                    	sysexParseError++;
                       // sysexPointer = 0;
                        parsingSysex = 0;
                        parseThatMF = 0;

                    }
                }
            }

        }
    }
}

void parseSingleParameterChange()
{

	if (presetReady)
	{

		//sysexMessageInProgress = 1; // set a flag that we've started a sysex preset transfer. May take multiple sysex parse calls on the chunks to complete
		union breakFloat theVal;
		uint32_t i = (2 + sysexMessageStartPoints[sysexMessageStartPointsReadPosition]);
		sysexMessageStartPointsReadPosition = (sysexMessageStartPointsReadPosition + 1) & 255;

		//get the destination number
		theVal.u32 = 0;
		theVal.u32 |= ((sysexBuffer[i & sysexPointerMask] &15) << 28);
		theVal.u32 |= (sysexBuffer[(i+1) & sysexPointerMask] << 21);
		theVal.u32 |= (sysexBuffer[(i+2) & sysexPointerMask] << 14);
		theVal.u32 |= (sysexBuffer[(i+3) & sysexPointerMask] << 7);
		theVal.u32 |= (sysexBuffer[(i+4) & sysexPointerMask] & 127);
		uint16_t whichParam  = (uint16_t)roundf(theVal.f);

		 i = (i+5);

		 //get the parameter value
		 theVal.u32 = 0;
		 theVal.u32 |= ((sysexBuffer[i & sysexPointerMask] &15) << 28);
		 theVal.u32 |= (sysexBuffer[(i+1)& sysexPointerMask] << 21);
		 theVal.u32 |= (sysexBuffer[(i+2)& sysexPointerMask] << 14);
		 theVal.u32 |= (sysexBuffer[(i+3)& sysexPointerMask] << 7);
		 theVal.u32 |= (sysexBuffer[(i+4)& sysexPointerMask] & 127);

		 //uint16_t intVal = (uint16_t)(theVal.f * 65535.0f);



		for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
		{
			//get the zero-to-one-value
			params[whichParam].zeroToOneVal[v] = theVal.f;
		}

		if ((whichParam == Effect1FXType) || (whichParam == Effect2FXType) || (whichParam == Effect3FXType) || (whichParam == Effect4FXType))
		{
			uint8_t whichEffect = (whichParam - Effect1FXType) / EffectParamsNum;
			FXType effectType = roundf(params[whichParam].zeroToOneVal[0] * (NUM_EFFECT_TYPES-1));
			param *FXAlias = &params[whichParam + 1];

			if (effectType > FXLowpass)
			{
				FXAlias[2].scaleFunc = &scaleFilterResonance;
			}
			setEffectsFunctions(effectType, whichEffect);
			FXAlias[0].setParam = effectSetters[whichEffect].setParam1;
			FXAlias[1].setParam = effectSetters[whichEffect].setParam2;
			FXAlias[2].setParam = effectSetters[whichEffect].setParam3;
			FXAlias[3].setParam = effectSetters[whichEffect].setParam4;
			FXAlias[4].setParam = effectSetters[whichEffect].setParam5;
		}

		for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
		{
			//set the real value based on the scale function
			params[whichParam].realVal[v] = params[whichParam].scaleFunc(params[whichParam].zeroToOneVal[v]);
			//set the actual parameter
			params[whichParam].setParam(params[whichParam].realVal[v], params[whichParam].objectNumber, v);
		}
		if ((whichParam == Osc1ShapeSet) || (whichParam == Osc2ShapeSet) || (whichParam == Osc3ShapeSet))
		{
			int whichOsc =(whichParam - Osc1ShapeSet) / OscParamsNum;
			int oscshape = roundf(params[whichParam].realVal[0] * (NUM_OSC_SHAPES-1));
			setOscilllatorShapes(oscshape, whichOsc);
		}
		if ((whichParam == Osc1) || (whichParam == Osc2) ||(whichParam == Osc3))
		{
			int whichOsc = (whichParam - Osc1) / OscParamsNum;
			if (params[whichParam].realVal[0]  > 0.5f)
			{
				oscsEnabled[whichOsc] = 1;
				oscOn[whichOsc] = 1;
			}
			else
			{
				oscsEnabled[whichOsc] = 0;
			}
			int enabledCount = 0;

			for (int j = 0; j < 3; j++)
			{
				enabledCount += oscsEnabled[j];
			}
			oscAmpMult = oscAmpMultArray[enabledCount];
		}
		if ((whichParam == Noise))
		{
			if (params[whichParam].realVal[0]  > 0.5f)
			{
				noiseOn = 1;
			}
		}
		if ((whichParam == Filter1Type) || (whichParam == Filter2Type))
		{
			int whichFilter = (whichParam - Filter1Type) / FilterParamsNum;
			int filterType = roundf(params[whichParam].realVal[0] * (NUM_FILTER_TYPES-1));
			setFilterTypes(filterType, whichFilter);
			int filterResParamNum = Filter1Resonance + (whichFilter * FilterParamsNum);
			int filterGainParamNum = Filter1Gain + (whichFilter * FilterParamsNum);
			params[filterResParamNum].setParam = filterSetters[whichFilter].setQ;
			params[filterGainParamNum].setParam = filterSetters[whichFilter].setGain;

			//set the resonance and gain params of that filter
			for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
			{
				params[filterResParamNum].setParam(params[filterResParamNum].realVal[v], params[filterResParamNum].objectNumber, v);
				params[filterGainParamNum].setParam(params[filterGainParamNum].realVal[v], params[filterGainParamNum].objectNumber, v);
			}
		}
		if ((whichParam == LFO1ShapeSet) || (whichParam == LFO2ShapeSet) || (whichParam == LFO3ShapeSet) || (whichParam == LFO4ShapeSet))
		{
			int whichLFO = (whichParam - LFO1ShapeSet) / LFOParamsNum;
			int LFOShape = roundf(params[whichParam].realVal[0] * (NUM_LFO_SHAPES-1));
			setLFOShapes(LFOShape, whichLFO);
			int rateParamNum = LFO1Rate + (whichLFO * LFOParamsNum);
			int shapeParamNum = LFO1Shape + (whichLFO * LFOParamsNum);
			int phaseParamNum = LFO1Phase + (whichLFO * LFOParamsNum);
			params[rateParamNum].setParam = lfoSetters[whichLFO].setRate;
			params[shapeParamNum].setParam = lfoSetters[whichLFO].setShape;
			params[phaseParamNum].setParam = lfoSetters[whichLFO].setPhase;

			//set the lfo params for that particular new lfo shape
			for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
			{
				params[rateParamNum].setParam(params[rateParamNum].realVal[v], params[rateParamNum].objectNumber, v);
				params[shapeParamNum].setParam(params[shapeParamNum].realVal[v], params[shapeParamNum].objectNumber, v);
				params[phaseParamNum].setParam(params[phaseParamNum].realVal[v], params[phaseParamNum].objectNumber, v);
			}
		}
		if ((whichParam == MIDIKeyMax) || (whichParam == MIDIKeyMin))
		{
			midiKeyDivisor = 1.0f / ((params[MIDIKeyMax].realVal[0]*127.0f) - (params[MIDIKeyMin].realVal[0]*127.0f));
			midiKeySubtractor = (params[MIDIKeyMin].realVal[0] * 127.0f);
		}
		/*
		if (whichParam == Transpose)
		{
			masterTranspose = params[Transpose].realVal[0];
		}
		*/
		if (whichParam == FXOrder)
		{
			fxPre = params[FXOrder].realVal[0] > 0.5f;
		}
		if (whichParam == PedalControlsMaster)
		{
			pedalControlsMaster = params[PedalControlsMaster].realVal[0] > 0.5f;
		}
	}
	waitingToParseSingleParameterChange = 0;
}

uint32_t sendMappingChangeUpdate = 0;

void parseSingleMappingChange()
{
	if (presetReady)
	{
		//sysexMessageInProgress = 1; // set a flag that we've started a sysex preset transfer. May take multiple sysex parse calls on the chunks to complete
		union breakFloat theVal;
		uint32_t i = (2 + sysexMessageStartPoints[sysexMessageStartPointsReadPosition]);
		sysexMessageStartPointsReadPosition = (sysexMessageStartPointsReadPosition + 1) & 255;
		//get the destination number
		theVal.u32 = 0;
		theVal.u32 |= ((sysexBuffer[i & sysexPointerMask] &15) << 28);
		theVal.u32 |= (sysexBuffer[(i+1) & sysexPointerMask] << 21);
		theVal.u32 |= (sysexBuffer[(i+2) & sysexPointerMask] << 14);
		theVal.u32 |= (sysexBuffer[(i+3) & sysexPointerMask] << 7);
		theVal.u32 |= (sysexBuffer[(i+4) & sysexPointerMask] & 127);
		uint16_t destNumber  = (uint16_t)roundf(theVal.f);



		uint8_t whichSlot = sysexBuffer[(i+5) & sysexPointerMask]; //slot id
		uint8_t mappingChangeType = sysexBuffer[(i+6) & sysexPointerMask]; //mapping change type

		i = (i+7);

		//get the parameter value
		theVal.u32 = 0;
		theVal.u32 |= ((sysexBuffer[i & sysexPointerMask] &15) << 28);
		theVal.u32 |= (sysexBuffer[(i+1) & sysexPointerMask] << 21);
		theVal.u32 |= (sysexBuffer[(i+2) & sysexPointerMask] << 14);
		theVal.u32 |= (sysexBuffer[(i+3) & sysexPointerMask] << 7);
		theVal.u32 |= (sysexBuffer[(i+4) & sysexPointerMask] & 127);
		uint8_t tempMappingArray[2];
		if (mappingChangeType == 0) // source id
		{
			tempMappingArray[0] = 0;
			tempMappingArray[1] = (int16_t)(roundf(theVal.f));
		}
		else if (mappingChangeType == 1) // amount
		{
			int16_t intVal = (int16_t)(theVal.f * 32767.0f);
			tempMappingArray[0] = intVal >> 8;
			tempMappingArray[1] = intVal & 0xff;
		}
		else // scalar source
		{
			tempMappingArray[0] = 0;
			tempMappingArray[1] = (int16_t)(roundf(theVal.f));
		}
		int16_t mappingChangeValue = ((tempMappingArray[0]<< 8) + tempMappingArray[1]);

		//sysexMessageInProgress = 0;
		sendMappingChangeUpdate = 1;

		uint8_t whichMapping = 0;
		uint8_t foundOne = 0;

		// TODO: replace this search with explicit mapping slots instead
			// we need to add sending of mapping slots

		uint8_t lowestEmptyMapping = MAX_NUM_MAPPINGS;
		//search to see if this destination already has other mappings
		for (int j = 0; j < MAX_NUM_MAPPINGS; j++)
		{
			if (mappings[j].destNumber == destNumber)
			{
				//found one, use this mapping
				whichMapping = j;
				foundOne = 1;
			}
			if ((mappings[j].destNumber == 255) && (j < lowestEmptyMapping))
			{
				lowestEmptyMapping = j;
			}
		}
		if (foundOne == 0)
		{
			//didn't find another mapping with this destination, start a new mapping
			whichMapping = lowestEmptyMapping;
			numMappings++;
			mappings[whichMapping].destNumber = destNumber;
			mappings[whichMapping].dest = &params[destNumber];
		}


//		//if the source is bipolar (oscillators, noise, and LFOs) then double the amount because it comes in as only half the range
//		if ((source < 4) || ((source >= LFO_SOURCE_OFFSET) && (source < (LFO_SOURCE_OFFSET + NUM_LFOS))))
//		{
//			amountFloat *= 2.0f;
//		}


		if (mappingChangeType == SourceID)
		{
			mappings[whichMapping].sourceSmoothed[whichSlot] = 1;
			int source = mappingChangeValue;

			if (source == 255)
			{
				//delete this hook
				mappings[whichMapping].hookActive[whichSlot] = 0;
				// if all hooks for this destination have source 255, delete this mapping
				int countHooks = 0;
				for (int i = 0; i < 3; i++)
				{
					if (mappings[whichMapping].hookActive[whichSlot] != 0)
					{
						countHooks++;
					}
				}
				//if you just removed the only hook from a mapping, mark the mapping invalid and remove it from the list
				//TODO: I think we are going to have to store a stack that represents which mappings are active and need to be ticked, otherwise it has to iterate all 32, now that we can remove one in the middle of the list.
				//or we keep track of the highest number of mapping we are ticking, and always tick up to that, ignoring elements we pass that have dest set to 255.

				if (countHooks == 0)
				{
					mappings[whichMapping].destNumber = 255;

					//since the mapping tick will no longer update it, it would stick on the last value, so reset it to the unmapped initial value
					for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
					{
						//sources are now summed - let's add the initial value
						float finalVal = mappings[whichMapping].dest->zeroToOneVal[v];


						//now scale the value with the correct scaling function
						mappings[whichMapping].dest->realVal[v] = mappings[whichMapping].dest->scaleFunc(finalVal);

						//and pop that value where it belongs by setting the actual parameter
						mappings[whichMapping].dest->setParam(mappings[whichMapping].dest->realVal[v], mappings[whichMapping].dest->objectNumber, v);
					}
				}
			}
			else
			{
				mappings[whichMapping].hookActive[whichSlot] = 1;

				for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
				{
					mappings[whichMapping].sourceValPtr[whichSlot][v] = &sourceValues[source][v];
					mappings[whichMapping].scalarSourceValPtr[whichSlot][v] = &defaultScaling; //blank out the scalar source, because otherwise it will point to some random function or a null pointer
				}
				if (source < 4) //if it's oscillators or noise (the first 4 elements of the source array), don't smooth to allow FM
				{
					mappings[whichMapping].sourceSmoothed[whichSlot] = 0;
				}
				if ((source >= LFO_SOURCE_OFFSET) && (source < (LFO_SOURCE_OFFSET + NUM_LFOS)))
				{
					lfoOn[source - LFO_SOURCE_OFFSET] = 1;
				}
				if ((source >= ENV_SOURCE_OFFSET) && (source < (ENV_SOURCE_OFFSET + NUM_ENV)))
				{
					envOn[source - ENV_SOURCE_OFFSET] = 1;
					mappings[whichMapping].sourceSmoothed[whichSlot] = 0;
				}
				if ((source >= OSC_SOURCE_OFFSET) && (source < (OSC_SOURCE_OFFSET+NUM_OSC)))
				{
					oscOn[source - OSC_SOURCE_OFFSET] = 1;
				}
				if ((source >= NOISE_SOURCE_OFFSET) && (source < (NOISE_SOURCE_OFFSET+1)))
				{
					noiseOn = 1;
				}
				mappings[whichMapping].amount[whichSlot] = 0.0f;
			}


		}
		else if (mappingChangeType == Amount)
		{
			mappings[whichMapping].amount[whichSlot] = (float)mappingChangeValue * INV_TWO_TO_15;
		}
		else if (mappingChangeType == ScalarID)
		{
			int scalar = mappingChangeValue;
			for (int v = 0; v < NUM_STRINGS_PER_BOARD; v++)
			{
				if (scalar == 0xff)
				{
					mappings[whichMapping].scalarSourceValPtr[whichSlot][v] = &defaultScaling;
				}
				else
				{
					mappings[whichMapping].scalarSourceValPtr[whichSlot][v] = &sourceValues[scalar][v];
					if ((scalar >= LFO_SOURCE_OFFSET) && (scalar < (LFO_SOURCE_OFFSET + NUM_LFOS)))
					{
						lfoOn[scalar - LFO_SOURCE_OFFSET] = 1;
					}
					if ((scalar >= ENV_SOURCE_OFFSET) && (scalar < (ENV_SOURCE_OFFSET + NUM_ENV)))
					{
						envOn[scalar - ENV_SOURCE_OFFSET] = 1;
					}
					if ((scalar >= OSC_SOURCE_OFFSET) && (scalar < (OSC_SOURCE_OFFSET + NUM_OSC)))
					{
						oscOn[scalar - OSC_SOURCE_OFFSET] = 1;
					}
					if ((scalar >= NOISE_SOURCE_OFFSET) && (scalar < (NOISE_SOURCE_OFFSET + 1)))
					{
						noiseOn = 1;
					}
					//TODO: doesn't cleanly remove lfoOn settings during streaming data - after deleting an LFO used as a scalar it will keep computing the LFO. How should we remember what the source of the scalar was when removing it? -JS
				}
			}
		}
	}
	waitingToParseSingleMappingChange = 0;
}

uint32_t currentUARTBufferPos = 0;
uint32_t uartParseCount = 0;

void UART_buffer_parse(uint32_t size)
{
#if 0
	for (int i = 0; i < size; i++)
	{

		sysexBuffer[(sysexWritePointer++) & sysexPointerMask] = UART_buffer[currentUARTBufferPos++ & UART_BUFFER_MASK];

		uint32_t offset = currentUARTBufferPos++ & UART_BUFFER_MASK;
#endif
		uint32_t offset = size;
		if (receivingSysex)
		    {

				if (UART_buffer[offset] < 128)
				{
					sysexBuffer[(sysexWritePointer++) & sysexPointerMask] = UART_buffer[offset];
					lastBufferStuff = masterTimer;
		            sysexReset = 0;
				}
				else
				{
					if (UART_buffer[offset] == 0xf7)
					{
						receivingSysex = 0;
						lastEndReceive = masterTimer;

						//parseSysex();
						return;
					 }
				}
		    }

		    else if (newSysexStart)
		    {
		    	if (!parsingSysex)
		        {
		        	if (UART_buffer[offset] == 126) // special message saying that sysex multi-chunk transmission is finished. Parse it!
		            {

		        		switch (sysexBuffer[sysexMessageStartPoints[sysexMessageStartPointsWritePosition] & sysexPointerMask])
		        		{
							case 0://new preset
								parseThatMF = 1;
								parsingSysex = 1;
								break;
							case 3: //real-time parameter change
								waitingToParseSingleParameterChange = 1;
								break;
							case 4: //real-time mapping change
								waitingToParseSingleMappingChange = 1;
								break;
							default:
								break;
		        		}


		                prevLastParseCall = lastParseCall;
		                lastParseCall = masterTimer;
		                sysexReset = 1;

		                sysexMessageStartPointsWritePosition = (sysexMessageStartPointsWritePosition + 1) & 255;
		                sysexMessageStartPoints[sysexMessageStartPointsWritePosition] = sysexWritePointer;
		                newSysexStart = 0;
		                //sysexPointer = 0;
		            }
		        	else
		        	{
		        		if (sysexHeaderCount == 2) //first byte after start byte
		        		{

							if (UART_buffer[offset] == 0 || UART_buffer[offset] == 1 || UART_buffer[offset] == 2 || UART_buffer[offset] == 3 || UART_buffer[offset] == 4)
							{
								// if this is the first chunk, put in the first and second elements (following chunks need this data stripped until the final message gets sent)
								prevLastBufferBegin[0] = lastBufferBegin[0];
								prevLastBufferBegin[1] = lastBufferBegin[1];
								lastBufferBegin[0] = masterTimer;
								lastBufferBegin[1] = UART_buffer[offset];
							}
		        		}
		        		//only store the two header bytes in the first chonk of a multi-chonk transmission
		        		// if this is the first sysex chunk of a multi-chonk transmission
						if ((sysexReset == 1) && (sysexHeaderCount > 0))
						{
							sysexBuffer[sysexWritePointer++ & sysexPointerMask] = UART_buffer[offset];
						}
						sysexHeaderCount--;
						//if you got past the first two bytes after the start byte, start just filling the buffer
						if (sysexHeaderCount == 0)
						{
							receivingSysex = 1;
							newSysexStart = 0;
						}
		        	}


		        }
		    }

			else if (UART_buffer[offset] == 0xf0) //got a start byte
			{
				newSysexStart = 1;
				sysexHeaderCount = 2; //how many header bytes will follow (save them in first chonk and drop the ones in following chonks)
			}

		    masterTimer++;
//#endif

	//}




}

volatile uint32_t sizeComingIn = 0;
volatile uint32_t sizeMinusPosition = 0;
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
#if 0
	HAL_UART_RxEventTypeTypeDef eventTypeUART = HAL_UARTEx_GetRxEventType(huart);
	sizeComingIn = Size;
	sizeMinusPosition = (Size - currentUARTBufferPos) & UART_BUFFER_MASK;
	if (eventTypeUART == HAL_UART_RXEVENT_IDLE)
	{
		UART_buffer_parse(sizeMinusPosition);
	}
	else if (eventTypeUART == HAL_UART_RXEVENT_TC)
	{

		UART_buffer_parse(sizeMinusPosition);
	}

	else if (eventTypeUART == HAL_UART_RXEVENT_HT)
	{
		UART_buffer_parse(sizeMinusPosition);
	}
#endif
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	UART_buffer_parse(1);
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	UART_buffer_parse(0);
}


/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x38000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0x60000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
while(1)
{
;
}
/* The prototype shows it is a naked function - in effect this is just an
assembly function. */
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
