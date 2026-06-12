/*
 * synth.c
 *
 *  Created on: Jun 11, 2026
 *      Author: josnyder
 */

#include "synth.h"
#include "audiostream.h"
tCycle myTest;

void synthInit(void)
{
	tCycle_init(&myTest, &leaf);
	tCycle_setFreq(myTest, 440.0f);
}

void synthSetFreq(float freq)
{
	tCycle_setFreq(myTest, freq);
}
float synthTick(void)
{
	return tCycle_tick(myTest);

}
