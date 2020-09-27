/*
 * oled.h
 *
 *  Created on: Feb 05, 2020
 *      Author: Matthew Wang
 */

#ifndef OLED_H_
#define OLED_H_

#ifdef __cplusplus

#include <stdint.h>
// dummy typedef for non-hardware build
typedef void I2C_HandleTypeDef;
    
namespace vocodec
{
    extern "C"
    {
        
#endif

extern char oled_buffer[32];

typedef enum _OLEDLine
{
	FirstLine = 0,
	SecondLine,
	BothLines,
	NilLine
} OLEDLine;

void OLED_init(I2C_HandleTypeDef* hi2c);

void initUIFunctionPointers(void);

void setLED_Edit(int onOff);

void setLED_USB(int onOff);

void setLED_1(int onOff);

void setLED_2(int onOff);

void setLED_A(int onOff);

void setLED_B(int onOff);

void setLED_C(int onOff);

void setLED_leftout_clip(int onOff);

void setLED_rightout_clip(int onOff);

void setLED_leftin_clip(int onOff);

void setLED_rightin_clip(int onOff);

int getCursorX(void);

void OLED_process(void);

void OLED_writePreset(void);

void OLED_writeEditScreen(void);

void OLED_writeKnobParameter(int whichParam);

void OLED_writeButtonAction(int whichButton, int whichAction);

void OLED_writeTuning(void);

void OLED_draw(void);

void OLEDclear(void);

void OLEDclearLine(OLEDLine line);

void OLEDwriteString(const char* myCharArray, int arrayLength, int startCursor, OLEDLine line);

void OLEDwriteLine(const char* myCharArray, int arrayLength, OLEDLine line);

void OLEDwriteInt(uint32_t myNumber, int numDigits, int startCursor, OLEDLine line);

void OLEDwriteIntLine(uint32_t myNumber, int numDigits, OLEDLine line);

void OLEDwritePitch(float midi, int startCursor, OLEDLine line, int showCents);

void OLEDwritePitchClass(float midi, int startCursor, OLEDLine line);

void OLEDwritePitchLine(float midi, OLEDLine line, int showCents);

void OLEDwriteFixedFloat(float input, int numDigits, int numDecimal, int startCursor, OLEDLine line);

void OLEDwriteFixedFloatLine(float input, int numDigits, int numDecimal, OLEDLine line);

void OLEDwriteFloat(float input, int startCursor, OLEDLine line);

void OLEDdrawFloatArray(float* input, float min, float max, int size, int offset, int startCursor, OLEDLine line);

int OLEDgetCursor(void);
        
#ifdef __cplusplus
    }
} // extern "C"
#endif

#endif /* OLED_H_ */
