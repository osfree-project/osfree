/*************************************************************************
*
*  File Name   : STY_PNT.C
*
*  Description :This module contains the code for the
*               main client window painting
*
*  Concepts    : Painting of main client window.
*
*  API's       : WinBeginPaint
*                WinFillRect
*                WinEndPaint
*
*  Copyright (C) 1992 IBM Corporation
*
*      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is
*      sample code created by IBM Corporation. This sample code is not
*      part of any standard or IBM product and is provided to you solely
*      for  the purpose of assisting you in the development of your
*      applications.  The code is provided "AS IS", without
*      warranty of any kind.  IBM shall not be liable for any damages
*      arising out of your use of the sample code, even if they have been
*      advised of the possibility of such damages.                                                    *
*
************************************************************************/

/*  Include files, macros, defined constants, and externs               */

#define  INCL_WINSYS

#include <os2.h>
#include "sty_main.h"
#include "sty_xtrn.h"

/*  Global variables                                                    */

/*  Entry point declarations                                            */

/****************************************************************
 *  Name:   MainPaint
 *
 *  Description : Main client painting routine
 *
 *  Concepts :  Routine is called whenver the client window
 *              procedure receives a WM_PAINT message begins
 *              painting by calling WinBeginPaint and
 *              retriving the HPS for the window performs any
 *              painting desired ends painting by calling
 *              WinEndPaint
 *
 *  API's :  WinBeginPaint
 *           WinFillRect
 *           WinEndPaint
 *
 *  Parameters : hwnd - handle of window to paint
 *
 *  Returns :  VOID
 *
 ****************************************************************/
VOID MainPaint(HWND hwnd)
{
    RECTL rclUpdate;
    HPS hps;

    hps = WinBeginPaint(hwnd, NULLHANDLE, (PRECTL)&rclUpdate);

    /* fill update rectangle with window color */
    WinFillRect(hps, (PRECTL)&rclUpdate, SYSCLR_WINDOW);

    /*
     * Add painting routines here.  Hps is now the HPS for
     * the window and rclUpdate contains the update rectangle.
     */

    WinEndPaint(hps);

}   /* MainPaint() */
