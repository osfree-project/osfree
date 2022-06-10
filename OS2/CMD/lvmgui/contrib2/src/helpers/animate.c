
/*
 *@@sourcefile animate.c:
 *      contains a bit of helper code for animations.
 *
 *      This is a new file with V0.81. Most of this code used to reside
 *      in common.c with previous versions.
 *      Note that with V0.9.0, the code for icon animations has been
 *      moved to the new comctl.c file.
 *
 *      Usage: All PM programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  anm*   Animation helper functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\animate.h"
 */

/*
 *      Copyright (C) 1997-2000 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSPROCESS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINPOINTERS
#define INCL_WINSYS

#define INCL_GPILOGCOLORTABLE
#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS             // added UM 99-10-22; needed for EMX headers
#include <os2.h>

#include <stdlib.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/winh.h"

/*
 *@@category: Helpers\PM helpers\Animation helpers
 *      See animate.c.
 */

/*
 *@@ anmBlowUpBitmap:
 *      this displays an animation based on a given bitmap.
 *      The bitmap is "blown up" in that it is continually
 *      increased in size until the original size is reached.
 *      The animation is calculated so that it lasts exactly
 *      ulAnimation milliseconds, no matter how fast the
 *      system is.
 *
 *      This function does not return until the animation
 *      has completed.
 *
 *      You should run this routine in a thread with higher-
 *      than-normal priority, because otherwise the kernel
 *      might interrupt the thread, causing a somewhat jerky
 *      display.
 *
 *      Returns the count of animation steps that were drawn.
 *      This is dependent on the speed of the system.
 *
 *@@changed V0.9.7 (2000-12-08) [umoeller]: got rid of dtGetULongTime
 */

BOOL anmBlowUpBitmap(HPS hps,               // in: from WinGetScreenPS(HWND_DESKTOP)
                     HBITMAP hbm,           // in: bitmap to be displayed
                     ULONG ulAnimationTime) // in: total animation time (ms)
{
    ULONG   ulrc = 0;
    ULONG   ulInitialTime,
            ulNowTime;
            // ulCurrentSize = 10;

    if (hps)
    {
        POINTL  ptl = {0, 0};
        RECTL   rtlStretch;
        ULONG   ul,
                ulSteps = 20;
        BITMAPINFOHEADER bih;
        GpiQueryBitmapParameters(hbm, &bih);
        ptl.x = (G_cxScreen - bih.cx) / 2;
        ptl.y = (G_cyScreen - bih.cy) / 2;

        // we now use ul for the current animation step,
        // which is a pointer on a scale from 1 to ulAnimationTime;
        // ul will be recalculated after each animation
        // step according to how much time the animation
        // has cost on this display so far. This has the
        // following advantages:
        // 1) no matter how fast the system is, the animation
        //    will always last ulAnimationTime milliseconds
        // 2) since large bitmaps take more time to calculate,
        //    the animation won't appear to slow down then
        DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                        &ulInitialTime,
                        sizeof(ulInitialTime));
        ul = 1;
        ulSteps = 1000;
        do {
            LONG cx = (((bih.cx-20) * ul) / ulSteps) + 20;
            LONG cy = (((bih.cy-20) * ul) / ulSteps) + 20;
            rtlStretch.xLeft   = ptl.x + ((bih.cx - cx) / 2);
            rtlStretch.yBottom = ptl.y + ((bih.cy - cy) / 2);
            rtlStretch.xRight = rtlStretch.xLeft + cx;
            rtlStretch.yTop   = rtlStretch.yBottom + cy;

            WinDrawBitmap(hps, hbm, NULL, (PPOINTL)&rtlStretch,
                          0, 0,       // we don't need colors
                          DBM_STRETCH);

            DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                            &ulNowTime,
                            sizeof(ulNowTime));

            // recalculate ul: rule of three based on the
            // time we've spent on animation so far
            ul = (ulSteps
                    * (ulNowTime - ulInitialTime)) // time spent so far
                    / ulAnimationTime;      // time to spend altogether

            ulrc++;                         // return count

        } while (ul < ulSteps);

        // finally, draw the 1:1 version
        WinDrawBitmap(hps, hbm, NULL, &ptl,
                      0, 0,       // we don't need colors
                      DBM_NORMAL);

    } // end if (hps)

    return ulrc;
}

#define LAST_LINE_WIDTH 2

/* ******************************************************************
 *
 *   Other animations
 *
 ********************************************************************/

/*
 *@@ anmPowerOff:
 *      displays an animation that looks like a
 *      monitor being turned off; hps must have
 *      been acquired using WinGetScreenPS.
 *
 *@@changed V0.9.7 (2000-12-08) [umoeller]: got rid of dtGetULongTime
 */

VOID anmPowerOff(HPS hps,
                 ULONG ulMaxTime1, //  = 500,
                 ULONG ulMaxTime2, //  = 800,
                 ULONG ulMaxTime3, //  = 200,
                 ULONG ulWaitEnd)  //  = 300
{
    RECTL       rclScreen,
                rclNow,
                rclLast,
                rclDraw;
    ULONG       ulPhase = 1,
                ulCXLastLine;

    ULONG       ulStartTime = 0,
                ulTimeNow = 0;

    WinQueryWindowRect(HWND_DESKTOP, &rclScreen);
    ulCXLastLine = rclScreen.xRight / 3;

    WinShowPointer(HWND_DESKTOP, FALSE);

    memcpy(&rclLast, &rclScreen, sizeof(RECTL));

    // In order to draw the animation, we tell apart three
    // "phases", signified by the ulPhase variable. While
    // ulPhase != 99, we stay in a do-while loop.
    ulPhase = 1;

    do
    {
        // get current time
        DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                        &ulTimeNow,
                        sizeof(ulTimeNow));

        // get start time
        if (ulStartTime == 0)
            // this is reset when we enter a new phase
            ulStartTime = ulTimeNow;

        if (ulPhase == 1)
        {
            // Phase 1: "shrink" the screen by drawing black
            // rectangles from the edges towards the center
            // of the screen. With every loop, we draw the
            // rectangles a bit closer to the center, until
            // the center is black too. Sort of like this:

            //          ษอออออออออออออออออออออออออป
            //          บ       black             บ
            //          บ                         บ
            //          บ      ฺฤฤฤฤฤฤฤฤฤฤฤฟ      บ
            //          บ      ณ rclNow:   ณ      บ
            //          บ  ->  ณ untouched ณ  <-  บ
            //          บ      ภฤฤฤฤฤฤฤฤฤฤฤู      บ
            //          บ            ^            บ
            //          บ            |            บ
            //          ศอออออออออออออออออออออออออผ

            // This part lasts exactly (ulMaxTime1) milliseconds.

            // rclNow contains the rectangle _around_ which
            // the black rectangles are to be drawn. With
            // every iteration, rclNow is reduced in size.
            ULONG ulMaxX = (rclScreen.xRight - ulCXLastLine) / 2,
                  ulMaxY = rclScreen.yTop / 2 - LAST_LINE_WIDTH;
            ULONG ulTimePassed = (ulTimeNow - ulStartTime);
            if (ulTimePassed >= ulMaxTime1)
            {
                // time has elapsed:
                ulTimePassed = ulMaxTime1;
                // enter next phase on next loop
                ulPhase++;
                // reget start time
                ulStartTime = 0;
            }
            rclNow.xLeft = ulMaxX * ulTimePassed / ulMaxTime1;
            rclNow.yBottom = ulMaxY * ulTimePassed / ulMaxTime1;
            rclNow.xRight = (rclScreen.xRight) - rclNow.xLeft;
            rclNow.yTop = (rclScreen.yTop) - rclNow.yBottom;

            /* rclNow.xLeft = ((rclScreen.yTop / 2) * ul / ulSteps );
            rclNow.xRight = (rclScreen.xRight) - rclNow.xLeft;
            rclNow.yBottom = ((rclScreen.yTop / 2) * ul / ulSteps );
            rclNow.yTop = (rclScreen.yTop) - rclNow.yBottom; */

            /* if (rclNow.yBottom > (rclNow.yTop - LAST_WIDTH) )
            {
                rclNow.yBottom = (rclScreen.yTop / 2) - LAST_WIDTH;
                rclNow.yTop = (rclScreen.yTop / 2) + LAST_WIDTH;
            } */

            // draw black rectangle on top of rclNow
            rclDraw.xLeft = rclLast.xLeft;
            rclDraw.xRight = rclLast.xRight;
            rclDraw.yBottom = rclNow.yTop;
            rclDraw.yTop = rclLast.yTop;
            WinFillRect(hps, &rclDraw, CLR_BLACK); // exclusive

            // draw black rectangle left of rclNow
            rclDraw.xLeft = rclLast.xLeft;
            rclDraw.xRight = rclNow.xLeft;
            rclDraw.yBottom = rclLast.yBottom;
            rclDraw.yTop = rclLast.yTop;
            WinFillRect(hps, &rclDraw, CLR_BLACK); // exclusive

            // draw black rectangle right of rclNow
            rclDraw.xLeft = rclNow.xRight;
            rclDraw.xRight = rclLast.xRight;
            rclDraw.yBottom = rclLast.yBottom;
            rclDraw.yTop = rclLast.yTop;
            WinFillRect(hps, &rclDraw, CLR_BLACK); // exclusive

            // draw black rectangle at the bottom of rclNow
            rclDraw.xLeft = rclLast.xLeft;
            rclDraw.xRight = rclLast.xRight;
            rclDraw.yBottom = rclLast.yBottom;
            rclDraw.yTop = rclNow.yBottom;
            WinFillRect(hps, &rclDraw, CLR_BLACK); // exclusive

            // remember rclNow for next iteration
            memcpy(&rclLast, &rclNow, sizeof(RECTL));

            // done with "shrinking"?
            /* if ( rclNow.xRight < ((rclScreen.xRight / 2) + LAST_WIDTH) )
            {
                ulPhase = 2; // exit
                // reget start time
                ulStartTime = 0;
            } */

            if (ulPhase == 2)
            {
                // this was the last step in this phase:
                memcpy(&rclLast, &rclScreen, sizeof(RECTL));
            }
        }
        else if (ulPhase == 2)
        {
            // Phase 2: draw a horizontal white line about
            // where the last rclNow was. and shrink it
            // towards the middle. This ends with a white
            // dot in the middle of the screen.

            //          ษอออออออออออออออออออออออออป
            //          บ       black             บ
            //          บ                         บ
            //          บ                         บ
            //          บ  -->   ฤwhiteฤฤฤฤ   <-- บ
            //          บ                         บ
            //          บ                         บ
            //          บ                         บ
            //          ศอออออออออออออออออออออออออผ

            // This part lasts exactly (ulMaxTime2) milliseconds.

            // start is same as max in step 1
            ULONG ulStartX = (rclScreen.xRight - ulCXLastLine) / 2,
                  ulY      = rclScreen.yTop / 2 - LAST_LINE_WIDTH;
            ULONG ulMaxX = (rclScreen.xRight) / 2 - LAST_LINE_WIDTH;      // center
            ULONG ulTimePassed = (ulTimeNow - ulStartTime);
            if (ulTimePassed >= ulMaxTime2)
            {
                // time has elapsed:
                ulTimePassed = ulMaxTime2;
                // enter next phase on next loop
                ulPhase++;
                // reget start time
                ulStartTime = 0;
            }

            rclNow.xLeft =  ulStartX
                          + ( (ulMaxX - ulStartX) * ulTimePassed / ulMaxTime2 );
            rclNow.yBottom = ulY;
            rclNow.xRight = (rclScreen.xRight) - rclNow.xLeft;
            rclNow.yTop = (rclScreen.yTop) - rclNow.yBottom;

            // draw black rectangle left of rclNow
            rclDraw.xLeft = rclLast.xLeft;
            rclDraw.xRight = rclNow.xLeft;
            rclDraw.yBottom = rclLast.yBottom;
            rclDraw.yTop = rclLast.yTop;
            WinFillRect(hps, &rclDraw, CLR_BLACK); // exclusive

            // draw black rectangle right of rclNow
            rclDraw.xLeft = rclNow.xRight;
            rclDraw.xRight = rclLast.xRight;
            rclDraw.yBottom = rclLast.yBottom;
            rclDraw.yTop = rclLast.yTop;
            WinFillRect(hps, &rclDraw, CLR_BLACK); // exclusive

            // WinFillRect(hps, &rclNow, CLR_WHITE); // exclusive

            // remember rclNow for next iteration
            memcpy(&rclLast, &rclNow, sizeof(RECTL));

            if (ulPhase == 3)
            {
                // this was the last step in this phase:
                // keep the dot visible for a while
                DosSleep(ulMaxTime3);

                // draw a white line for phase 3
                rclLast.xLeft = ulMaxX;
                rclLast.yBottom = rclScreen.yTop / 4;
                rclLast.xRight = rclScreen.xRight - rclLast.xLeft;
                rclLast.yTop = rclScreen.yTop - rclLast.yBottom;

                WinFillRect(hps, &rclLast, CLR_WHITE); // exclusive
            }
        }
        else if (ulPhase == 3)
        {
            // Phase 3: make the white line shorter with
            // every iteration by drawing black rectangles
            // above and below it. These are drawn closer
            // to the center with each iteration.

            //          ษอออออออออออออออออออออออออป
            //          บ                         บ
            //          บ                         บ
            //          บ            |            บ
            //          บ            |            บ
            //          บ            |            บ
            //          บ                         บ
            //          บ                         บ
            //          ศอออออออออออออออออออออออออผ

            // This part lasts exactly ulMaxTime3 milliseconds.
            ULONG ulX = (rclScreen.xRight) / 2 - LAST_LINE_WIDTH,      // center
                  ulStartY = rclScreen.yTop / 4;
            ULONG ulMaxY = (rclScreen.yTop) / 2 - LAST_LINE_WIDTH;      // center
            ULONG ulTimePassed = (ulTimeNow - ulStartTime);
            if (ulTimePassed >= ulMaxTime3)
            {
                // time has elapsed:
                ulTimePassed = ulMaxTime3;
                // stop
                ulPhase = 99;
            }

            rclNow.xLeft =  ulX;
            rclNow.yBottom = ulStartY
                          + ( (ulMaxY - ulStartY) * ulTimePassed / ulMaxTime3 );
            rclNow.xRight = (rclScreen.xRight) - rclNow.xLeft;
            rclNow.yTop = (rclScreen.yTop) - rclNow.yBottom;

            // draw black rectangle on top of rclNow
            rclDraw.xLeft = rclLast.xLeft;
            rclDraw.xRight = rclLast.xRight;
            rclDraw.yBottom = rclNow.yTop;
            rclDraw.yTop = rclLast.yTop;
            WinFillRect(hps, &rclDraw, CLR_BLACK); // exclusive

            // draw black rectangle at the bottom of rclNow
            rclDraw.xLeft = rclLast.xLeft;
            rclDraw.xRight = rclLast.xRight;
            rclDraw.yBottom = rclLast.yBottom;
            rclDraw.yTop = rclNow.yBottom;
            WinFillRect(hps, &rclDraw, CLR_BLACK); // exclusive

            // remember rclNow for next iteration
            memcpy(&rclLast, &rclNow, sizeof(RECTL));

            /* rclDraw.xLeft = (rclScreen.xRight / 2) - LAST_LINE_WIDTH;
            rclDraw.xRight = (rclScreen.xRight / 2) + LAST_LINE_WIDTH;
            rclDraw.yTop = (rclScreen.yTop * 3 / 4);
            rclDraw.yBottom = (rclScreen.yTop * 3 / 4) - ((rclScreen.yTop * 1 / 4) * ul / LAST_STEPS);
            if (rclDraw.yBottom < ((rclScreen.yTop / 2) + LAST_LINE_WIDTH))
                rclDraw.yBottom = ((rclScreen.yTop / 2) + LAST_LINE_WIDTH);
            WinFillRect(hps, &rclDraw, CLR_BLACK);

            rclDraw.xLeft = (rclScreen.xRight / 2) - LAST_LINE_WIDTH;
            rclDraw.xRight = (rclScreen.xRight / 2) + LAST_LINE_WIDTH;
            rclDraw.yBottom = (rclScreen.yTop * 1 / 4);
            rclDraw.yTop = (rclScreen.yTop * 1 / 4) + ((rclScreen.yTop * 1 / 4) * ul / LAST_STEPS);
            if (rclDraw.yTop > ((rclScreen.yTop / 2) - LAST_LINE_WIDTH))
                rclDraw.yBottom = ((rclScreen.yTop / 2) - LAST_LINE_WIDTH);

            WinFillRect(hps, &rclDraw, CLR_BLACK);

            ul++;
            if (ul > LAST_STEPS) */
            //     ulPhase = 99;
        }

    } while (ulPhase != 99);

    // sleep a while
    DosSleep(ulWaitEnd / 2);
    WinFillRect(hps, &rclScreen, CLR_BLACK); // exclusive
    DosSleep(ulWaitEnd / 2);

    WinShowPointer(HWND_DESKTOP, TRUE);
}


// testcase

/* int main(int argc, char* argp)
{
    HENUM henum;
    HWND hwndTop;
    HAB hab = WinInitialize(0);
    HMQ hmq = WinCreateMsgQueue(hab, 0);
    HPS hps = WinGetScreenPS(HWND_DESKTOP);

    anmPowerOff(hps);

    henum = WinBeginEnumWindows(HWND_DESKTOP);
    while ((hwndTop = WinGetNextWindow(henum)))
        if (WinIsWindowShowing(hwndTop))
            WinInvalidateRect(hwndTop, NULL, TRUE);
    WinEndEnumWindows(henum);

    WinShowPointer(HWND_DESKTOP, TRUE);

    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    return 0;
} */
