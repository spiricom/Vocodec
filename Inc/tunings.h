/*
 * tunings.h
 *
 *  Created on: Dec 18, 2019
 *      Author: josnyder
 */

#ifndef TUNINGS_H_
#define TUNINGS_H_

#ifdef __cplusplus
#include <stdint.h>
namespace vocodec
{
    extern "C"
    {
#endif

#define NUM_TUNINGS 63

        extern const char tuningNames[NUM_TUNINGS][13];
        extern const float tuningPresets[NUM_TUNINGS][12];

#ifdef __cplusplus
    }
} // extern "C"
#endif

#endif /* TUNINGS_H_ */
