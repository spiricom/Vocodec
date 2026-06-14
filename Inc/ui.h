/*
 * ui.h
 *
 *  Created on: Aug 30, 2019
 *      Author: jeffsnyder
 */
#ifndef UI_H_
#define UI_H_

#ifndef __cplusplus
#include "leaf.h"
#endif

#define NUM_BUTTONS 8


extern uint8_t buttonValues[NUM_BUTTONS];
extern uint8_t buttonValuesPrev[NUM_BUTTONS];
extern uint32_t buttonCounters[NUM_BUTTONS];
extern uint32_t buttonPressed[NUM_BUTTONS];



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

void processButtons(void);


#endif /* UI_H_ */
