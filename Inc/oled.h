/*
 * oled.h
 *
 *  Created on: Feb 05, 2020
 *      Author: Matthew Wang
 */

#ifndef OLED_H_
#define OLED_H_

extern char oled_buffer[32];
extern int8_t writeParameterFlag;
extern uint32_t currentTuning;

typedef enum _OLEDLine
{
	FirstLine = 0,
	SecondLine,
	BothLines,
	NilLine
} OLEDLine;

void OLED_init(I2C_HandleTypeDef* hi2c);

void setLED_Edit(uint8_t onOff);

void setLED_USB(uint8_t onOff);

void setLED_1(uint8_t onOff);

void setLED_2(uint8_t onOff);

void setLED_A(uint8_t onOff);

void setLED_B(uint8_t onOff);

void setLED_C(uint8_t onOff);

void setLED_leftout_clip(uint8_t onOff);

void setLED_rightout_clip(uint8_t onOff);

void setLED_leftin_clip(uint8_t onOff);

void setLED_rightin_clip(uint8_t onOff);

void OLED_process(void);

void OLED_writePreset(void);

void OLED_writeParameter(uint8_t whichParam);

void changeTuning();

void OLED_draw(void);

void OLEDclear(void);

void OLEDclearLine(OLEDLine line);

void OLEDwriteString(char* myCharArray, uint8_t arrayLength, uint8_t startCursor, OLEDLine line);

void OLEDwriteLine(char* myCharArray, uint8_t arrayLength, OLEDLine line);

void OLEDwriteInt(uint32_t myNumber, uint8_t numDigits, uint8_t startCursor, OLEDLine line);

void OLEDwriteIntLine(uint32_t myNumber, uint8_t numDigits, OLEDLine line);

void OLEDwritePitch(float midi, uint8_t startCursor, OLEDLine line);

void OLEDwritePitchClass(float midi, uint8_t startCursor, OLEDLine line);

void OLEDwritePitchLine(float midi, OLEDLine line);

void OLEDwriteFixedFloat(float input, uint8_t numDigits, uint8_t numDecimal, uint8_t startCursor, OLEDLine line);

void OLEDwriteFixedFloatLine(float input, uint8_t numDigits, uint8_t numDecimal, OLEDLine line);

void OLEDwriteFloat(float input, uint8_t startCursor, OLEDLine line);

void buttonCheck(void);

void adcCheck(void);

#endif /* OLED_H_ */

