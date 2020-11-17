/*
 * oled.h
 *
 *  Created on: Feb 05, 2020
 *      Author: Matthew Wang
 */

#ifndef OLED_H_
#define OLED_H_

#include "sfx.h"

#ifdef __cplusplus

#include <stdint.h>
// dummy typedef for non-hardware build
typedef void I2C_HandleTypeDef;

namespace vocodec
{
    extern "C"
    {
        
#endif
        
        typedef enum _OLEDLine
        {
            FirstLine = 0,
            SecondLine,
            BothLines,
            NilLine
        } OLEDLine;

        void OLED_init(Vocodec* vcd, I2C_HandleTypeDef* hi2c);

        void initUIFunctionPointers(Vocodec* vcd);

        void setLED_Edit(Vocodec* vcd, int onOff);

        void setLED_USB(Vocodec* vcd, int onOff);

        void setLED_1(Vocodec* vcd, int onOff);

        void setLED_2(Vocodec* vcd, int onOff);

        void setLED_A(Vocodec* vcd, int onOff);

        void setLED_B(Vocodec* vcd, int onOff);

        void setLED_C(Vocodec* vcd, int onOff);

        void setLED_leftout_clip(Vocodec* vcd, int onOff);

        void setLED_rightout_clip(Vocodec* vcd, int onOff);

        void setLED_leftin_clip(Vocodec* vcd, int onOff);

        void setLED_rightin_clip(Vocodec* vcd, int onOff);

        int getCursorX(Vocodec* vcd);

        void OLED_process(Vocodec* vcd);

        void OLED_writePreset(Vocodec* vcd);

        void OLED_writeEditScreen(Vocodec* vcd);

        void OLED_writeKnobParameter(Vocodec* vcd, int whichParam);

        void OLED_writeButtonAction(Vocodec* vcd, int whichButton, int whichAction);

        void OLED_writeTuning(Vocodec* vcd);

        void OLED_draw(Vocodec* vcd);

        void OLEDclear(Vocodec* vcd);

        void OLEDclearLine(Vocodec* vcd, OLEDLine line);

        void OLEDwriteString(Vocodec* vcd, const char* myCharArray, int arrayLength, int startCursor, OLEDLine line);

        void OLEDwriteLine(Vocodec* vcd, const char* myCharArray, int arrayLength, OLEDLine line);

        void OLEDwriteInt(Vocodec* vcd, uint32_t myNumber, int numDigits, int startCursor, OLEDLine line);

        void OLEDwriteIntLine(Vocodec* vcd, uint32_t myNumber, int numDigits, OLEDLine line);

        void OLEDwritePitch(Vocodec* vcd, float midi, int startCursor, OLEDLine line, int showCents);

        void OLEDwritePitchClass(Vocodec* vcd, float midi, int startCursor, OLEDLine line);

        void OLEDwritePitchLine(Vocodec* vcd, float midi, OLEDLine line, int showCents);

        void OLEDwriteFixedFloat(Vocodec* vcd, float input, int numDigits, int numDecimal, int startCursor, OLEDLine line);

        void OLEDwriteFixedFloatLine(Vocodec* vcd, float input, int numDigits, int numDecimal, OLEDLine line);

        void OLEDwriteFloat(Vocodec* vcd, float input, int startCursor, OLEDLine line);

        void OLEDdrawFloatArray(Vocodec* vcd, float* input, float min, float max, int size, int offset, int startCursor, OLEDLine line);

        int OLEDgetCursor(Vocodec* vcd);

#ifdef __cplusplus
    }
} // extern "C"
#endif

#endif /* OLED_H_ */
