/*  Win_MS.h	1.5
    Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/


/*****************************************************************************
*
* The following symbols control inclusion of various parts of this file:
*
* WINVER            Windows version number (0x030a).  To exclude
*                   definitions introduced in version 3.1 (or above)
*                   #define WINVER 0x0300 before #including <windows.h>
*
*****************************************************************************/

#ifndef _INC_WINDOWS
#define _INC_WINDOWS    /* #defined if windows.h has been included */

#ifdef  MSC
#ifndef RC_INVOKED
#pragma pack(1)         /* Assume byte packing throughout */
#endif  /* RC_INVOKED */
#endif


/* If WINVER is not defined, assume version 3.1 */
#ifndef WINVER
#define WINVER  0x030a
#endif


/* Handle OEMRESOURCE for 3.0 compatibility */
#if (WINVER < 0x030a)
    #define NOOEMRESOURCE
    #ifdef OEMRESOURCE
        #undef NOOEMRESOURCE
    #endif
#endif



/****** USER typedefs, structures, and functions *****************************/

#ifndef NOUSER

/****** Computer-based-training (CBT) support *******************************/

#define WM_QUEUESYNC        0x0023

#ifndef NOWH

/* SetWindowsHook() code */
#define WH_CBT          5

#define HCBT_MOVESIZE       0
#define HCBT_MINMAX     1
#define HCBT_QS         2
#define HCBT_CREATEWND      3
#define HCBT_DESTROYWND     4
#define HCBT_ACTIVATE       5
#define HCBT_CLICKSKIPPED   6
#define HCBT_KEYSKIPPED     7
#define HCBT_SYSCOMMAND     8
#define HCBT_SETFOCUS       9

#if (WINVER >= 0x030a)
/* HCBT_CREATEWND parameters pointed to by lParam */
typedef struct tagCBT_CREATEWND
{
    CREATESTRUCT FAR* lpcs;
    HWND    hwndInsertAfter;
} CBT_CREATEWND;
typedef CBT_CREATEWND FAR* LPCBT_CREATEWND;

/* HCBT_ACTIVATE structure pointed to by lParam */
typedef struct tagCBTACTIVATESTRUCT
{
    BOOL    fMouse;
    HWND    hWndActive;
} CBTACTIVATESTRUCT;

#endif  /* WINVER >= 0x030a */
#endif  /* NOWH */

/****** Hardware hook support ***********************************************/

#ifndef NOWH
#if (WINVER >= 0x030a)
#define WH_HARDWARE     8

typedef struct tagHARDWAREHOOKSTRUCT
{
    HWND    hWnd;
    UINT    wMessage;
    WPARAM  wParam;
    LPARAM  lParam;
} HARDWAREHOOKSTRUCT;
#endif  /* WINVER >= 0x030a */
#endif  /* NOWH */


/****** Shell support *******************************************************/

#ifndef NOWH
#if (WINVER >= 0x030a)
/* SetWindowsHook() Shell hook code */
#define WH_SHELL           10

#define HSHELL_WINDOWCREATED       1
#define HSHELL_WINDOWDESTROYED     2
#define HSHELL_ACTIVATESHELLWINDOW 3

#endif  /* WINVER >= 0x030a */
#endif  /* NOWH */


/****** Journalling support *************************************************/

#ifndef NOWH
#define WH_JOURNALRECORD    0
#define WH_JOURNALPLAYBACK  1

/* Journalling hook codes */
#define HC_GETNEXT      1
#define HC_SKIP         2
#define HC_NOREMOVE     3
#define HC_NOREM        HC_NOREMOVE
#define HC_SYSMODALON       4
#define HC_SYSMODALOFF      5

/* Journalling message structure */
typedef struct tagEVENTMSG
{
    UINT    message;
    UINT    paramL;
    UINT    paramH;
    DWORD   time;
} EVENTMSG;
typedef EVENTMSG *PEVENTMSG;
typedef EVENTMSG NEAR* NPEVENTMSG;
typedef EVENTMSG FAR* LPEVENTMSG;

BOOL    WINAPI EnableHardwareInput(BOOL);

#endif  /* NOWH */

/* winnls.h (used by MFC4) */

#define CT_CTYPE1            0x00000001     /* ctype 1 information */
#define CT_CTYPE2            0x00000002     /* ctype 2 information */
#define CT_CTYPE3            0x00000004     /* ctype 3 information */

#define C3_KATAKANA		0x0010
#define C3_HIRAGANA		0x0020
#define C3_HALFWIDTH		0x0040
#define C3_FULLWIDTH		0x0080
#define C3_IDEOGRAPH		0x0100
#define C3_KASHIDA		0x0200
#define C3_LEXICAL		0x0400

#endif  /* NOUSER */

#ifdef  MSC
#ifndef RC_INVOKED
#pragma pack()          /* Revert to default packing */
#endif  /* RC_INVOKED */
#endif


#endif  /* _INC_WINDOWS */
