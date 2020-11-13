/*
 * oled.c
 *
 *  Created on: Feb 05, 2020
 *      Author: Matthew Wang
 */

#ifndef __cplusplus
#include "main.h"
#include "ssd1306.h"
#include "audiostream.h"
#else
#include "PluginEditor.h"
#endif

#include "oled.h"
#include "ui.h"
#include "gfx.h"
#include "custom_fonts.h"
#include "tunings.h"

#ifdef __cplusplus
#include <string.h>
#define itoa(v, s, b) (strcpy(s, std::to_string(v).c_str()))
namespace vocodec
{
    extern "C"
    {
#endif

        void OLED_init(Vocodec* vcd, I2C_HandleTypeDef* hi2c)
        {
#ifndef __cplusplus
            //start up that OLED display
            ssd1306_begin(hi2c, SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
#endif

            //HAL_Delay(5);

            //clear the OLED display buffer
            for (int i = 0; i < 512; i++)
            {
                vcd->buffer[i] = 0;
            }
            initUIFunctionPointers(vcd);
            initModeNames(vcd);
            //display the blank buffer on the OLED
            //ssd1306_display_full_buffer();

            //initialize the graphics library that lets us write things in that display buffer
            GFXinit(&vcd->theGFX, vcd->buffer, 128, 32);

            //set up the monospaced font

            //GFXsetFont(&theGFX, &C649pt7b); //funny c64 text monospaced but very large
            //GFXsetFont(&theGFX, &DINAlternateBold9pt7b); //very serious and looks good - definitely not monospaced can fit 9 Ms
            //GFXsetFont(&theGFX, &DINCondensedBold9pt7b); // very condensed and looks good - definitely not monospaced can fit 9 Ms
            GFXsetFont(&vcd->theGFX, &EuphemiaCAS8pt7b); //this one is elegant but definitely not monospaced can fit 9 Ms
            //GFXsetFont(&theGFX, &GillSans9pt7b); //not monospaced can fit 9 Ms
            //GFXsetFont(&theGFX, &Futura9pt7b); //not monospaced can fit only 7 Ms
            //GFXsetFont(&theGFX, &FUTRFW8pt7b); // monospaced, pretty, (my old score font) fits 8 Ms
            //GFXsetFont(&theGFX, &nk57_monospace_cd_rg9pt7b); //fits 12 characters, a little crammed
            //GFXsetFont(&theGFX, &nk57_monospace_no_rg9pt7b); // fits 10 characters
            //GFXsetFont(&theGFX, &nk57_monospace_no_rg7pt7b); // fits 12 characters
            //GFXsetFont(&theGFX, &nk57_monospace_no_bd7pt7b); //fits 12 characters
            //GFXsetFont(&theGFX, &nk57_monospace_cd_rg7pt7b); //fits 18 characters

            GFXsetTextColor(&vcd->theGFX, 1, 0);
            GFXsetTextSize(&vcd->theGFX, 1);

            //ssd1306_display_full_buffer();

            OLEDclear(vcd);
            OLED_writePreset(vcd);
            OLED_draw(vcd);
            //sdd1306_invertDisplay(1);
        }

        void initUIFunctionPointers(Vocodec* vcd)
        {
            vcd->buttonActionFunctions[Vocoder] = UIVocoderButtons;
            vcd->buttonActionFunctions[VocoderCh] = UIVocoderChButtons;
            vcd->buttonActionFunctions[Pitchshift] = UIPitchShiftButtons;
            vcd->buttonActionFunctions[AutotuneMono] = UINeartuneButtons;
            vcd->buttonActionFunctions[AutotunePoly] = UIAutotuneButtons;
            vcd->buttonActionFunctions[SamplerButtonPress] = UISamplerBPButtons;
            vcd->buttonActionFunctions[SamplerKeyboard] = UISamplerKButtons;
            vcd->buttonActionFunctions[SamplerAutoGrab] = UISamplerAutoButtons;
            vcd->buttonActionFunctions[Distortion] = UIDistortionButtons;
            vcd->buttonActionFunctions[Wavefolder] = UIWaveFolderButtons;
            vcd->buttonActionFunctions[BitCrusher] = UIBitcrusherButtons;
            vcd->buttonActionFunctions[Delay] = UIDelayButtons;
            vcd->buttonActionFunctions[Reverb] = UIReverbButtons;
            vcd->buttonActionFunctions[Reverb2] = UIReverb2Buttons;
            vcd->buttonActionFunctions[LivingString] = UILivingStringButtons;
            vcd->buttonActionFunctions[LivingStringSynth] = UILivingStringSynthButtons;
            vcd->buttonActionFunctions[ClassicSynth] = UIClassicSynthButtons;
            vcd->buttonActionFunctions[Rhodes] = UIRhodesButtons;
        }

#ifndef __cplusplus
        void setLED_Edit(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
            }
        }


        void setLED_USB(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET);
            }
        }


        void setLED_1(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
            }
        }

        void setLED_2(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
            }
        }


        void setLED_A(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
            }
        }

        void setLED_B(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_RESET);
            }
        }

        void setLED_C(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_RESET);
            }
        }

        void setLED_leftout_clip(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
            }
        }

        void setLED_rightout_clip(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
            }
        }

        void setLED_leftin_clip(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
            }
        }

        void setLED_rightin_clip(Vocodec* vcd, int onOff)
        {
            if (onOff)
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
            }
        }
#endif

        int getCursorX(Vocodec* vcd)
        {
            return GFXgetCursorX(&vcd->theGFX);
        }

        void OLED_process(Vocodec* vcd)
        {
            if (vcd->writeKnobFlag >= 0)
            {
                OLED_writeKnobParameter(vcd, vcd->writeKnobFlag);
                vcd->writeKnobFlag = -1;
            }
            if (vcd->writeButtonFlag >= 0 && vcd->writeActionFlag >= 0) //These should always be set together
            {
                OLED_writeButtonAction(vcd, vcd->writeButtonFlag, vcd->writeActionFlag);
                vcd->writeButtonFlag = -1;
                vcd->writeActionFlag = -1;
            }
            //    OLED_draw();
        }

        void OLED_writePreset(Vocodec* vcd)
        {
            GFXsetFont(&vcd->theGFX, &EuphemiaCAS8pt7b);
            OLEDclear(vcd);
            char tempString[24];
            itoa((vcd->currentPreset+1), tempString, 10);
            strcat(tempString, ":");
            strcat(tempString, vcd->modeNames[vcd->currentPreset]);
            int myLength = (int)strlen(tempString);
            //OLEDwriteInt(currentPreset+1, 2, 0, FirstLine);
            //OLEDwriteString(":", 1, 20, FirstLine);
            //OLEDwriteString(modeNames[currentPreset], 12, 24, FirstLine);
            OLEDwriteString(vcd, tempString, myLength, 0, FirstLine);
            GFXsetFont(&vcd->theGFX, &EuphemiaCAS7pt7b);
            OLEDwriteString(vcd, "P", 1, 110, FirstLine);
            OLEDwriteInt(vcd, vcd->knobPage, 1, 120, FirstLine);
            OLEDwriteString(vcd, vcd->modeNamesDetails[vcd->currentPreset], (int)strlen(vcd->modeNamesDetails[vcd->currentPreset]), 0, SecondLine);
            //save new preset to flash memory
        }

        void OLED_writeEditScreen(Vocodec* vcd)
        {
            GFXsetFont(&vcd->theGFX, &EuphemiaCAS7pt7b);
            OLEDclear(vcd);
            const char* firstSet = "KNOB:SET CV PED";
            const char* firstClear = "DOWN:CLR CV PED";
            if (vcd->cvAddParam[vcd->currentPreset] >= 0) OLEDwriteString(vcd, firstClear, (int)strlen(firstClear), 0, FirstLine);
            else OLEDwriteString(vcd, firstSet, (int)strlen(firstSet), 0, FirstLine);
            OLEDwriteString(vcd, "C:SET KEY CENTER", 16, 0, SecondLine);
        }

        void OLED_writeKnobParameter(Vocodec* vcd, int whichKnob)
        {
            // Knob params
            if (whichKnob < KNOB_PAGE_SIZE)
            {
                int whichParam = whichKnob + (vcd->knobPage * KNOB_PAGE_SIZE);
                vcd->floatADCUI[whichKnob] = vcd->smoothedADC[whichKnob];
                int len = (int)strlen(vcd->knobParamNames[vcd->currentPreset][whichParam]);
                if (len > 0)
                {
                    GFXsetFont(&vcd->theGFX, &EuphemiaCAS7pt7b);
                    OLEDclearLine(vcd, SecondLine);
                    OLEDwriteString(vcd, vcd->knobParamNames[vcd->currentPreset][whichParam], len, 0, SecondLine);
                    OLEDwriteString(vcd, " ", 1, getCursorX(vcd), SecondLine);
                    OLEDwriteFloat(vcd, vcd->displayValues[whichKnob + (vcd->knobPage * KNOB_PAGE_SIZE)], getCursorX(vcd), SecondLine);
                    //OLEDwriteString(paramNames[currentPreset][whichParam], strlen(paramNames[currentPreset][whichParam]), 0, SecondLine);
                }
            }
        }

        void OLED_writeButtonAction(Vocodec* vcd, int whichButton, int whichAction)
        {
            // Could change this so that buttonActionFunctions does the actual OLEDwrite
            // if we want more flexibility on what buttons display
            const char* str = vcd->buttonActionFunctions[vcd->currentPreset](vcd, (VocodecButton)whichButton, (ButtonAction)whichAction);
            int len = (int)strlen(str);
            if (len > 0)
            {
                GFXsetFont(&vcd->theGFX, &EuphemiaCAS7pt7b);
                OLEDclearLine(vcd, SecondLine);
                OLEDwriteString(vcd, str, len, 0, SecondLine);
            }
        }

        void OLED_writeTuning(Vocodec* vcd)
        {
            GFXsetFont(&vcd->theGFX, &EuphemiaCAS7pt7b);
            OLEDclearLine(vcd, SecondLine);
            OLEDwriteString(vcd, "T", 2, 0, SecondLine);
            OLEDwriteInt(vcd, vcd->currentTuning, 2, 12, SecondLine);
            OLEDwriteString(vcd, tuningNames[vcd->currentTuning], 12, 36, SecondLine);
        }

#ifndef __cplusplus
        void OLED_draw(Vocodec* vcd)
        {
            ssd1306_display_full_buffer();
        }
#endif

        /// OLED Stuff

        void OLEDdrawPoint(Vocodec* vcd, int16_t x, int16_t y, uint16_t color)
        {
            GFXwritePixel(&vcd->theGFX, x, y, color);
            //ssd1306_display_full_buffer();
        }

        void OLEDdrawLine(Vocodec* vcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
        {
            GFXwriteLine(&vcd->theGFX, x0, y0, x1, y1, color);
            //ssd1306_display_full_buffer();
        }

        void OLEDdrawCircle(Vocodec* vcd, int16_t x0, int16_t y0, int16_t r, uint16_t color)
        {
            GFXfillCircle(&vcd->theGFX, x0, y0, r, color);
            //ssd1306_display_full_buffer();
        }


        void OLEDclear(Vocodec* vcd)
        {
            GFXfillRect(&vcd->theGFX, 0, 0, 128, 32, 0);
            //ssd1306_display_full_buffer();
        }

        void OLEDclearLine(Vocodec* vcd, OLEDLine line)
        {
            GFXfillRect(&vcd->theGFX, 0, (line%2)*16, 128, 16*((line/2)+1), 0);
            //ssd1306_display_full_buffer();
        }

        void OLEDwriteString(Vocodec* vcd, const char* myCharArray, int arrayLength, int startCursor, OLEDLine line)
        {
            int cursorX = startCursor;
            int cursorY = 12 + (16 * (line%2));
            GFXsetCursor(&vcd->theGFX, cursorX, cursorY);

            GFXfillRect(&vcd->theGFX, startCursor, line*16, arrayLength*12, (line*16)+16, 0);
            for (int i = 0; i < arrayLength; ++i)
            {
                GFXwrite(&vcd->theGFX, myCharArray[i]);
            }
            //ssd1306_display_full_buffer();
        }

        void OLEDwriteLine(Vocodec* vcd, const char* myCharArray, int arrayLength, OLEDLine line)
        {
            if (line == FirstLine)
            {
                GFXfillRect(&vcd->theGFX, 0, 0, 128, 16, 0);
                GFXsetCursor(&vcd->theGFX, 4, 15);
            }
            else if (line == SecondLine)
            {
                GFXfillRect(&vcd->theGFX, 0, 16, 128, 16, 0);
                GFXsetCursor(&vcd->theGFX, 4, 31);
            }
            else if (line == BothLines)
            {
                GFXfillRect(&vcd->theGFX, 0, 0, 128, 32, 0);
                GFXsetCursor(&vcd->theGFX, 4, 15);
            }
            for (int i = 0; i < arrayLength; ++i)
            {
                GFXwrite(&vcd->theGFX, myCharArray[i]);
            }
        }

        void OLEDwriteInt(Vocodec* vcd, uint32_t myNumber, int numDigits, int startCursor, OLEDLine line)
        {
            int len = OLEDparseInt(vcd->oled_buffer, myNumber, numDigits);

            OLEDwriteString(vcd, vcd->oled_buffer, len, startCursor, line);
        }

        void OLEDwriteIntLine(Vocodec* vcd, uint32_t myNumber, int numDigits, OLEDLine line)
        {
            int len = OLEDparseInt(vcd->oled_buffer, myNumber, numDigits);

            OLEDwriteLine(vcd, vcd->oled_buffer, len, line);
        }

        void OLEDwritePitch(Vocodec* vcd, float midi, int startCursor, OLEDLine line, int showCents)
        {
            int len = OLEDparsePitch(vcd->oled_buffer, midi, showCents);

            OLEDwriteString(vcd, vcd->oled_buffer, len, startCursor, line);
        }

        void OLEDwritePitchClass(Vocodec* vcd, float midi, int startCursor, OLEDLine line)
        {
            int len = OLEDparsePitchClass(vcd->oled_buffer, midi);

            OLEDwriteString(vcd, vcd->oled_buffer, len, startCursor, line);
        }

        void OLEDwritePitchLine(Vocodec* vcd, float midi, OLEDLine line, int showCents)
        {
            int len = OLEDparsePitch(vcd->oled_buffer, midi, showCents);

            OLEDwriteLine(vcd, vcd->oled_buffer, len, line);
        }

        void OLEDwriteFixedFloat(Vocodec* vcd, float input, int numDigits, int numDecimal, int startCursor, OLEDLine line)
        {
            int len = OLEDparseFixedFloat(vcd->oled_buffer, input, numDigits, numDecimal);

            OLEDwriteString(vcd, vcd->oled_buffer, len, startCursor, line);
        }

        void OLEDwriteFixedFloatLine(Vocodec* vcd, float input, int numDigits, int numDecimal, OLEDLine line)
        {
            int len = OLEDparseFixedFloat(vcd->oled_buffer, input, numDigits, numDecimal);

            OLEDwriteLine(vcd, vcd->oled_buffer, len, line);
        }


        void OLEDwriteFloat(Vocodec* vcd, float input, int startCursor, OLEDLine line)
        {
            int numDigits = 5;
            int numDecimal = 1;

            float f = fabsf(input);
            if (f<1.0f)
            {
                numDigits = 4;
                numDecimal = 3;
            }

            else if (f<10.0f)
            {
                numDigits = 4;
                numDecimal = 2;
            }

            else if (f<100.0f)
            {
                numDigits = 5;
                numDecimal = 2;
            }

            else if (f<1000.0f)
            {
                numDigits = 5;
                numDecimal = 1;
            }
            else if (f<10000.0f)
            {
                numDigits = 5;
                numDecimal = 0;
            }
            else if (f<100000.0f)
            {
                numDigits = 6;
                numDecimal = 0;
            }
            else if (f<1000000.0f)
            {
                numDigits = 7;
                numDecimal = 0;
            }
            else if (f<10000000.0f)
            {
                numDigits = 8;
                numDecimal = 0;
            }

            int len = OLEDparseFixedFloat(vcd->oled_buffer, input, numDigits, numDecimal);

            OLEDwriteString(vcd, vcd->oled_buffer, len, startCursor, line);
        }

        void OLEDdrawFloatArray(Vocodec* vcd, float* input, float min, float max, int size, int offset, int startCursor, OLEDLine line)
        {
            int baseline = 0;
            if (line == SecondLine) baseline = 16;
            int height = 16;
            if (line == BothLines) height = 32;

            GFXfillRect(&vcd->theGFX, startCursor, (line%2)*16, size, 16*((line/2)+1), 0);

            for (int i = 0; i < size; i++)
            {
                int h = ((float)(height) / (max - min)) * (input[i] - min);
                GFXwritePixel(&vcd->theGFX, startCursor + size - 1 - ((i + offset) % size), baseline + h, 1);
                //        GFXwriteFastVLine(&theGFX, startCursor + size - ((i + offset) % size), center - (h/2), 1, 1);
            }
        }

        int OLEDgetCursor(Vocodec* vcd)
        {
            return (int)GFXgetCursorX(&vcd->theGFX);
        }

#ifdef __cplusplus
    }
} // extern "C"
#endif

