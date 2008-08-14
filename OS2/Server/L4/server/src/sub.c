/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      sub.c                                                         */
/*                                                                            */
/* Description: This file includes the code to Kbd, Vio and Mou APIs.         */
/*                                                                            */
/* Copyright (C) IBM Corporation 2003. All Rights Reserved.                   */
/* Copyright (C) W. David Ashley 2004, 2005. All Rights Reserved.             */
/*                                                                            */
/* Author(s):                                                                 */
/*      W. David Ashley  <dashley@us.ibm.com>                                 */
/*                                                                            */
/* This software is subject to the terms of the Common Public License v1.0.   */
/* You must accept the terms of this license to use this software.            */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but        */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY */
/* or FITNESS FOR A PARTICULAR PURPOSE.                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/


#ifdef HAVE_CONFIG_H
#include "config.h"

#endif  /* #ifdef HAVE_CONFIG_H


/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#define INCL_SUB
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"
#include "l4/l4con/l4contxt.h"
#include "l4/log/l4log.h"

/*============================================================================*/
/* Functions to start/stop Linux curses support                               */
/*============================================================================*/

static BOOL vioinit = FALSE;


void VioInitCurses (VOID)
{
    if (vioinit == TRUE) 
    {
        return;
    }
    contxt_init(4096, 1000);
    vioinit = TRUE;
}


void VioDeInitCurses (VOID)
{
    if (vioinit == TRUE) {
        
    }
    return;
}


/*============================================================================*/
/* OS/2 APIs for keyboard support                                             */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* KbdCharIn                                                                  */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 KbdCharIn(PKBDKEYINFO pkbci, USHORT fWait, HKBD hkbd)
{
    LOG("function KbdCharIn not implemented yet");
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* KbdPeek                                                                    */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 KbdPeek (PKBDKEYINFO pkbci, HKBD hkbd)
{
    LOG("function KbdPeek not implemented yet");
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* KbdStringIn                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 KbdStringIn (PCH pch, PSTRINGINBUF pchIn, USHORT fsWait,
                                 HKBD hkbd)
{
    LOG("function KbdStringIn not implemented yet");
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* KbdFlushBuffer                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 KbdFlushBuffer (HKBD hkbd)
{
    LOG("function KbdFlushBuffer not implemented yet");
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* KbdSetStatus                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 KbdSetStatus (PKBDINFO pkbdinfo, HKBD hkbd)
{
    LOG("function KbdSetStatus not implemented yet");
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* KbdGetStatus                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 KbdGetStatus (PKBDINFO pkbdinfo, HKBD hdbd)
{
    LOG("function KbdGetStatus not implemented yet");
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* KbdSetCp                                                                   */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 KbdSetCp (USHORT usReserved, USHORT pidCP, HKBD hdbd)
{
    LOG("function KbdSetCp not implemented yet");
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* KbdGetCp                                                                   */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 KbdGetCp (ULONG ulReserved, PUSHORT pidCP, HKBD hkbd)
{
    LOG("function KbdGetCp not implemented yet");

    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* KbdGetHWID                                                                 */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 KbdGetHWID (PKBDHWID pkbdhwid, HKBD hkbd)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* KbdXlate                                                                   */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 KbdXlate (PKBDTRANS pkbdtrans, HKBD hkbd)
{
    return ERROR_ACCESS_DENIED;
}


/*============================================================================*/
/* OS/2 APIs for video support                                                */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* VioGetBuf                                                                  */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioGetBuf (PULONG pLVB, PUSHORT pcbLVB, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioGetCurPos                                                               */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioGetCurPos (PUSHORT pusRow, PUSHORT pusColumn,
                                  HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioSetCurPos                                                               */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioSetCurPos (USHORT usRow, USHORT usColumn, HVIO hvio)
{
        SAVEENV;

        VioInitCurses();
        RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* VioGetCurType                                                              */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioGetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioSetCurType                                                              */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioSetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioGetMode                                                                 */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioGetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioSetMode                                                                 */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioSetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioReadCellStr                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioReadCellStr (PCH pchCellStr, PUSHORT pcb, USHORT usRow,
                                    USHORT usColumn, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioReadCharStr                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioReadCharStr (PCH pchCellStr, PUSHORT pcb, USHORT usRow,
                                    USHORT usColumn, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioWriteCellStr                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioWrtCellStr (PCCH pchCellStr, USHORT cb, USHORT usRow,
                                   USHORT usColumn, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioWriteCharStr                                                            */
/*    Notes:                                                                  */
/*       - if the string to be written contains the tab, backspace or newline */
/*         characters the the output will NOT be as expected.                 */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioWrtCharStr (PCCH pchStr, USHORT cb, USHORT usRow,
                                   USHORT usColumn, HVIO hvio)
{
        SAVEENV;

        VioInitCurses();
        RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* VioScrollDn                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioScrollDn (USHORT usTopRow, USHORT usLeftCol,
                                 USHORT usBotRow, USHORT usRightCol,
                                 USHORT cbLines, PBYTE pCell, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioScrollUp                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioScrollUp (USHORT usTopRow, USHORT usLeftCol,
                                 USHORT usBotRow, USHORT usRightCol,
                                 USHORT cbLines, PBYTE pCell, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioScrollLf                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioScrollLf (USHORT usTopRow, USHORT usLeftCol,
                                 USHORT usBotRow, USHORT usRightCol,
                                 USHORT cbCol, PBYTE pCell, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioScrollRt                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioScrollRt (USHORT usTopRow, USHORT usLeftCol,
                                 USHORT usBotRow, USHORT usRightCol,
                                 USHORT cbCol, PBYTE pCell, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioWrtNAttr                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioWrtNAttr (__const__ BYTE *pAttr, USHORT cb, USHORT usRow,
                                 USHORT usColumn, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioWrtNCell                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioWrtNCell (__const__ BYTE *pCell, USHORT cb, USHORT usRow,
                                 USHORT usColumn, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioWrtNChar                                                                */
/*    Notes:                                                                  */
/*       - if the char to be written contains the tab, backspace or newline   */
/*         characters the the output will NOT be as expected.                 */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioWrtNChar (PCCH pchChar, USHORT cb, USHORT usRow,
                                 USHORT usColumn, HVIO hvio)
{
        SAVEENV;

        VioInitCurses();

        RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* VioWrtTTY                                                                  */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioWrtTTY (PCCH pch, USHORT cb, HVIO hvio)
{
        SAVEENV;

        VioInitCurses();
	// Ok. Because this is not asciiz, put char by char.
	while (cb)
	{
	  contxt_putchar(*pch);
	  pch++;
	  cb--;
        }
	
        RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* VioWrtCharStrAtt                                                           */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioWrtCharStrAtt (PCCH pch, USHORT cb, USHORT usRow,
                                      USHORT usColumn, PBYTE pAttr, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioShowBuf                                                                 */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioShowBuf (USHORT offLVB, USHORT cb, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioSetAnsi                                                                 */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioSetAnsi (USHORT fAnsi, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioGetAnsi                                                                 */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioGetAnsi (PUSHORT pfAnsi, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioSavRedrawWait                                                           */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioSavRedrawWait (USHORT usRedrawInd, PUSHORT pNotifyType,
                                      USHORT usReserved)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioSavRedrawUndo                                                           */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioSavRedrawUndo (USHORT usOwnerInd, USHORT usKillInd,
                                      USHORT usReserved)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioModeWait                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioModeWait (USHORT usReqType, PUSHORT pNotifyType,
                                 USHORT usReserved)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioModeUndo                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioModeUndo (USHORT usOwnerInd, USHORT usKillInd,
                                 USHORT usReserved)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioScrLock                                                                 */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioScrLock (USHORT fWait, PUCHAR pfNotLocked, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioScrUnLock                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioScrUnLock (HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioPopUp                                                                   */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioPopUp (PUSHORT pfWait, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioEndPopUp                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioEndPopUp (HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioGetConfig                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioGetConfig (USHORT usConfigId, PVIOCONFIGINFO pvioin,
                                  HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioGetCp                                                                   */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioGetCp (USHORT usReserved, PUSHORT pIdCodePage,
                              HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioSetCp                                                                   */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioSetCp (USHORT usReserved, USHORT idCodePage,
                              HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioGetState                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioGetState (PVOID pState, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* VioSetState                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 VioSetState (CPVOID pState, HVIO hvio)
{
    return ERROR_ACCESS_DENIED;
}


/*============================================================================*/
/* OS/2 APIs for mouse support                                                */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* MouFlushQue                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouFlushQue (HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouGetPtrPos                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouGetPtrPos (PPTRLOC pmouLoc, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouSetPtrPos                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouSetPtrPos (PPTRLOC pmouLoc, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouSetPtrShape                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouSetPtrShape (PBYTE pBuf, PPTRSHAPE pmoupsInfo,
                                    HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouGetPtrShape                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouGetPtrShape (PBYTE pBuf, PPTRSHAPE pmoupsInfo,
                                    HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouGetDevStatus                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouGetDevStatus (PUSHORT pfsDevStatus, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouGetNumButtons                                                           */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouGetNumButtons (PUSHORT pcButtons, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouGetNumMickeys                                                           */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouGetNumMickeys (PUSHORT pcMickeys, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouReadEventQue                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouReadEventQue (PMOUEVENTINFO pmouevEvent, PUSHORT pfWait,
                                     HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouGetNumQueEl                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouGetNumQueEl (PMOUQUEINFO qmouqi, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouGetEventMask                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouGetEventMask (PUSHORT pfsEvents, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouSetEventMask                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouSetEventMask (PUSHORT pfsEvents, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouGetScaleFact                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouGetScaleFact (PSCALEFACT pmouscFactors, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouSetScaleFact                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouSetScaleFact (PSCALEFACT pmouscFactors, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouRemovePtr                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouRemovePtr (PNOPTRRECT pmourtRect, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouDrawPtr                                                                 */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouDrawPtr (HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouSetDevStatus                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouSetDevStatus (PUSHORT pfsDevStatus, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouGetThreshold                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouGetThreshold(PTHRESHOLD pthreshold, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* MouSetThreshold                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET16 APIENTRY16 MouSetThreshold(PTHRESHOLD pthreshold, HMOU hmou)
{
    return ERROR_ACCESS_DENIED;
}

