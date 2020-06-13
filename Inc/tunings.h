/*
 * tunings.h
 *
 *  Created on: Dec 18, 2019
 *      Author: josnyder
 */

#ifndef TUNINGS_H_
#define TUNINGS_H_

#define NUM_TUNINGS 63

extern float centsDeviation[12];
extern const float tuningPresets[NUM_TUNINGS][12];
extern uint32_t currentTuning;
extern uint8_t keyCenter;
extern const char tuningNames[NUM_TUNINGS][12];


#endif /* TUNINGS_H_ */
