/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define OTRUE 1
#define OFALSE 0
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define __ATTR_RAM_D1	__attribute__ ((section(".RAM_D1"))) __attribute__ ((aligned (32)))
#define __ATTR_RAM_D2	__attribute__ ((section(".RAM_D2"))) __attribute__ ((aligned (32)))
#define __ATTR_RAM_D3	__attribute__ ((section(".RAM_D3"))) __attribute__ ((aligned (32)))
#define __ATTR_SDRAM	__attribute__ ((section(".SDRAM"))) __attribute__ ((aligned (32)))

#define STM32 // define this so that LEAF knows you are building for STM32
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

extern  volatile int64_t cycleCountVals[4][3];


//simple cycle counter - writes to cycleCountVals: fills one of 16 slots with two numbers - the start count [0] and the time between start and end count [1].
#define CYCLE_COUNT_START0 {cycleCountVals[0][2] = 0; cycleCountVals[0][0] = DWT->CYCCNT;}
#define CYCLE_COUNT_START1 	{cycleCountVals[1][2] = 0;cycleCountVals[1][0] = DWT->CYCCNT;}
#define CYCLE_COUNT_START2 	{cycleCountVals[2][2] = 0;cycleCountVals[2][0] = DWT->CYCCNT;}
#define CYCLE_COUNT_START3 {cycleCountVals[3][2] = 0;cycleCountVals[3][0] = DWT->CYCCNT;}

#define CYCLE_COUNT_END0 cycleCountVals[0][1] = DWT->CYCCNT - cycleCountVals[0][0];
#define CYCLE_COUNT_END1 cycleCountVals[1][1] = DWT->CYCCNT - cycleCountVals[1][0];
#define CYCLE_COUNT_END2 cycleCountVals[2][1] = DWT->CYCCNT - cycleCountVals[2][0];
#define CYCLE_COUNT_END3 {if (!cycleCountVals[3][2]){cycleCountVals[3][1] = DWT->CYCCNT - cycleCountVals[3][0];} else {cycleCountVals[3][1] = -1;}}




float randomNumber(void);
static void HardFault_Handler( void ) __attribute__( ( naked ) );
void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress );
void writeIntToFlash(uint32_t data, uint32_t location);
uint32_t readIntFromFlash (uint32_t location);
uint8_t LEAF_error(uint8_t errorCode);
void CycleCounterStart( uint8_t );
void CycleCounterEnd( uint8_t );
void CycleCounterTrackMinAndMax( uint8_t whichCount);
void CycleCounterAddToAverage( uint8_t);
void CycleCounterAverage( uint8_t );
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
