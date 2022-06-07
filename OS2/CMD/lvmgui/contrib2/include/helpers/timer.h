
/*
 *@@sourcefile threads.h:
 *      header file for treads.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_DOSPROCESS
 *@@include #include <os2.h>
 *@@include #include "helpers\threads.h"
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

#if __cplusplus
extern "C" {
#endif

#ifndef TIMER_HEADER_INCLUDED
    #define TIMER_HEADER_INCLUDED

    /*
     *@@ XTIMERSET:
     *
     *@@added V0.9.9 (2001-02-28) [umoeller]
     */

    typedef struct _XTIMERSET
    {
        HAB         hab;
        HWND        hwndOwner;          // owner of XTimers (who has the PM timer)
        USHORT      idPMTimer;          // ID of main PM timer
        USHORT      idPMTimerRunning;   // if != 0, PM timer is running
        ULONG       ulPMTimeout;        // current PM timeout; if 0, recalculation
                                        // is needed.
        PVOID       pvllXTimers;        // linked list of current XTIMER structures, auto-free
    } XTIMERSET, *PXTIMERSET;

    PXTIMERSET XWPENTRY tmrCreateSet(HWND hwndOwner, USHORT usPMTimerID);
    typedef PXTIMERSET XWPENTRY TMRCREATESET(HWND hwndOwner, USHORT usPMTimerID);
    typedef TMRCREATESET *PTMRCREATESET;

    VOID XWPENTRY tmrDestroySet(PXTIMERSET pSet);
    typedef VOID XWPENTRY TMRDESTROYSET(PXTIMERSET pSet);
    typedef TMRDESTROYSET *PTMRDESTROYSET;

    USHORT XWPENTRY tmrStartXTimer(PXTIMERSET pSet, HWND hwnd, USHORT usTimerID, ULONG ulTimeout);
    typedef USHORT XWPENTRY TMRSTARTXTIMER(PXTIMERSET pSet, HWND hwnd, USHORT usTimerID, ULONG ulTimeout);
    typedef TMRSTARTXTIMER *PTMRSTARTXTIMER;

    BOOL XWPENTRY tmrStopXTimer(PXTIMERSET pSet, HWND hwnd, USHORT usTimerID);
    typedef BOOL XWPENTRY TMRSTOPXTIMER(PXTIMERSET pSet, HWND hwnd, USHORT usTimerID);
    typedef TMRSTOPXTIMER *PTMRSTOPXTIMER;

    VOID XWPENTRY tmrTimerTick(PXTIMERSET pSet);
    typedef VOID XWPENTRY TMRTIMERTICK(PXTIMERSET pSet);
    typedef TMRTIMERTICK *PTMRTIMERTICK;

#endif

#if __cplusplus
}
#endif

