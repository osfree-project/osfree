
/*
 *@@sourcefile animate.h:
 *      header file for animate.c.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_WINWINDOWMGR
 *@@include #include <os2.h>
 *@@include #include "helpers\animate.h"
 */

/*      Copyright (C) 1997-2000 Ulrich M”ller.
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

#ifndef ANIMATE_HEADER_INCLUDED
    #define ANIMATE_HEADER_INCLUDED

    BOOL anmBlowUpBitmap(HPS hps,
                         HBITMAP hbm,
                         ULONG ulAnimationTime);

    VOID anmPowerOff(HPS hps,
                     ULONG ulMaxTime1,
                     ULONG ulMaxTime2,
                     ULONG ulMaxTime3,
                     ULONG ulWaitEnd);

#endif

#if __cplusplus
}
#endif

