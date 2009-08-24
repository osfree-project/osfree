/*    
	OEMRoutines.c	2.31
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


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "windows.h"
#include "print.h"

#include "kerndef.h"
#include "BinTypes.h"
#include "Endian.h"
#include "Log.h"
#include "DPMI.h"
#include "Resources.h"
#include "Module.h"
#include "OEM.h"
#include "GdiDDK.h"
#include "PrinterBin.h"
#include "PrinterDC.h"
#include "make_thunk.h"
#include "ConvertRes.h"
#include "invoke_binary.h"

extern ENV *envp_global;
extern DSCR *LDT;

extern long int IT_ENUMBRUSHPROC();
extern long int IT_ENUMPENPROC();
extern long int IT_ENUMFONTPROC();

extern LPWORD hsw_ConvertWordArray(LPBYTE,int);
extern LPWORD hsw_ConvertArrayToWord(LPBYTE,int);
extern LPINT hsw_ConvertArrayToInt(LPBYTE,int);

static short int DEVICEsize = 0;

#define STK_DATA_256	256
#define STK_DATA_512	512
#define STK_DATA_576	576
#define STK_EXTDEVMODE	(2 * HANDLE_86  +  5 * LP_86  +  WORD_86)
#define STK_ADVSETUP	(2 * HANDLE_86  +  2 * LP_86)
#define STK_ENABLE	(4 * LP_86 + WORD_86)
#define STK_ENUMDFONTS	(4 * LP_86)
#define STK_ENUMOBJ	(3 * LP_86 + WORD_86)
#define STK_OUTPUT	(6 * LP_86 + 2 * WORD_86)
#define STK_DEVICEMODE	(2 * HANDLE_86  +  2 * LP_86)
#define STK_REALIZEOBJ	(4 * LP_86  +  WORD_86)
#define STK_CONTROL	(3 * LP_86  +  WORD_86)
#define STK_EXTTEXTOUT	(8 * LP_86  +  3 * WORD_86  +  INT_86)
#define STK_GETCHARWID	(5 * LP_86  +  2 * WORD_86)
#define STK_DEVICECAPS	(4 * LP_86  +  WORD_86)
#define STK_STRETCHDIB	(7 * LP_86  +  9 * WORD_86  +  DWORD_86)
#define STK_BITBLT	(4 * LP_86  +  6 * WORD_86  +  DWORD_86)
#define STK_STRETCHBLT	(5 * LP_86  +  8 * WORD_86  +  DWORD_86)
#define STK_COLORINFO	(2 * LP_86  +  DWORD_86)
#define STK_PIXEL	(2 * LP_86  +  2*INT_86 + DWORD_86)

static HSMT_OEMMODTAB *OEM_builtin_modtab;

extern HSMT_OEMENTRYTAB OEM_tab_PRINTER[];
extern HSMT_OEMENTRYTAB OEM_tab_CTL3DV2[];
extern HSMT_OEMENTRYTAB OEM_tab_CTL3D[];

/* add an entry to the module table */
int
AddOEMTable(LPSTR lpstr, HSMT_OEMENTRYTAB *oementry)
{
    	HSMT_OEMMODTAB *lpModTab;

	/* allocate an entry */
	lpModTab       = (struct tagHSMT_OEMMODTAB *)WinMalloc(sizeof(HSMT_OEMMODTAB));

	/* link to current list */
	lpModTab->next          = (struct tabHSMT_OEMMODTAB *)OEM_builtin_modtab;
	lpModTab->name          = lpstr;		/* add table name */
	lpModTab->hsmt_OEMEntry = oementry;		/* add conversion */

	/* builtin table now starts with this table */
	OEM_builtin_modtab = lpModTab;

    	return (int)lpModTab;	
}

/*
 * This function will retrieve the address of the native function (native thunk)
 *  that allows us to call the binary function.
 */

FARPROC
OEM_GetProcAddress(LPMODULEINFO lpModuleInfo, UINT uOrdinal)
{
    HSMT_OEMMODTAB *lpModTab;
    DWORD dwBinAddr = 0;
    FARPROC lpFunc;
    WORD wSel = 0;
    char szBuf[128];
    BOOL bPrintDrv = FALSE;
    LPSTR lpszDriver = "";

    if(OEM_builtin_modtab == 0) {
	AddOEMTable("CTL3DV2", OEM_tab_CTL3DV2);
	AddOEMTable("CTL3D", OEM_tab_CTL3D);
	AddOEMTable("PRINTER\0\0\0",OEM_tab_PRINTER );
    }

    /* have we loaded the binary dll yet? */
    if ( !lpModuleInfo->lpDLL )
	return (FARPROC)0;

    /* check if we have a printer, (on each call?) */
    if (GetProfileString("windows", "device", "", szBuf, 128)) {
	if ((lpszDriver = strtok(szBuf, ",")) &&
	    (lpszDriver = strtok(NULL, ",")))
	    bPrintDrv = TRUE;
    }

    /* look for the table in our list */
    /* that we have a table, and we have a match, (AND printing okay) */
    for(lpModTab = OEM_builtin_modtab;
	   lpModTab &&
           lpModTab->name &&
	   strcasecmp(lpModTab->name,lpModuleInfo->lpModuleName) &&
	   (!bPrintDrv || strcasecmp(lpModuleInfo->lpModuleName,lpszDriver) ||
	    strcasecmp(lpModTab->name,"PRINTER")) ; lpModTab = (struct tagHSMT_OEMMODTAB *)lpModTab->next);

    if (lpModuleInfo->lpEntryTable[uOrdinal].sel) {
	wSel = ((lpModuleInfo->wSelBase<<3) + 
		lpModuleInfo->lpEntryTable[uOrdinal].sel - 0x8) | 0x7;
	dwBinAddr = (DWORD)MAKELP(wSel,
		lpModuleInfo->lpEntryTable[uOrdinal].off);
    }
    if (dwBinAddr) {
	if ((lpModTab && (lpFunc = (FARPROC)lpModTab->hsmt_OEMEntry[uOrdinal].fun)))
	    return (FARPROC)make_native_thunk(dwBinAddr,(DWORD)lpFunc);
	else {
	    return (FARPROC)0;
	}
    }
    else
	return (FARPROC)0;
}

int
cnb_ExtDeviceMode(HWND hwnd, HINSTANCE hDriver, LPDEVMODE lpdmOutput,
		LPSTR lpszDevice, LPSTR lpszPort, LPDEVMODE lpdmInput,
		LPSTR lpszProfile, WORD fwMode)
{
    LPBYTE lpSP,ss,lpStruct;
    int retcode;
    WORD wSelProfile=0,wSelPort=0,wSelDevice=0;

    envp_global->reg.sp -= STK_DATA_512;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_EXTDEVMODE;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = GetPhysicalAddress((WORD)envp_global->reg.ss);
    PUTWORD(lpSP,fwMode);
    if (lpszProfile) {
	wSelProfile = ASSIGNSEL(lpszProfile, strlen(lpszProfile)+1);
	PUTDWORD(lpSP+2,MAKELP(wSelProfile,0));
    }
    else
	PUTDWORD(lpSP+2,0L);
    if (fwMode) {
	if (lpdmInput) {
	    PutDEVMODE(lpStruct,lpdmInput);
	    PUTWORD(lpSP+8,(WORD)envp_global->reg.ss);
	    PUTWORD(lpSP+6,(WORD)((DWORD)lpStruct - (DWORD)ss));
	    lpStruct += lpdmInput->dmSize + lpdmInput->dmDriverExtra;
	}
	else
	    PUTDWORD(lpSP+6,0L);
	if (lpdmOutput) {
	    PutDEVMODE(lpStruct,lpdmOutput);
	    PUTWORD(lpSP+20,(WORD)envp_global->reg.ss);
	    PUTWORD(lpSP+18,(WORD)((DWORD)lpStruct - (DWORD)ss));
	}
	else
	    PUTDWORD(lpSP+18,0L);
    }
    if (lpszPort) {
	wSelPort = ASSIGNSEL(lpszPort, strlen(lpszPort)+1);
	PUTDWORD(lpSP+10,MAKELP(wSelPort,0));
    }
    else
	PUTDWORD(lpSP+10,0L);
    if (lpszDevice) {
	wSelDevice = ASSIGNSEL(lpszDevice, strlen(lpszDevice)+1);
	PUTDWORD(lpSP+14,MAKELP(wSelDevice,0));
    }
    else
	PUTDWORD(lpSP+14,0L);
    PUTWORD(lpSP+22,(WORD)GetDataSelectorFromInstance(hDriver));
    PUTWORD(lpSP+24,(WORD)hwnd);
    invoke_binary();
    if (wSelProfile)
	FreeSelector(wSelProfile);
    if (wSelPort)
	FreeSelector(wSelPort);
    if (wSelDevice)
	FreeSelector(wSelDevice);
    if (fwMode && lpdmOutput)
	GetDEVMODE(lpdmOutput,lpStruct);
    retcode = (int)(short)LOWORD(envp_global->reg.ax);
    if (fwMode == 0 && retcode > 0)
		/* return the size of buffer for DEVMODE struct */
	retcode = retcode - 12*INT_86 - 4*UINT_86 - DWORD_86 - CCHDEVICENAME +
			sizeof(DEVMODE);
    envp_global->reg.sp += STK_DATA_512;
    return retcode;
}

WORD
cnb_AdvancedSetupDialog(HWND hwnd, HINSTANCE hDriver, LPDEVMODE lpDMin,
		LPDEVMODE lpDMout)
{
	LPBYTE	lpSP,ss,lpStruct, lpStructOut=NULL;
	int	retcode;

	envp_global->reg.sp -= STK_DATA_512;
	lpStruct = (LPBYTE)envp_global->reg.sp;
	envp_global->reg.sp -= STK_ADVSETUP;
	lpSP = (LPBYTE)envp_global->reg.sp;
	ss = GetPhysicalAddress((WORD)envp_global->reg.ss);

	if (lpDMout) {
		PutDEVMODE(lpStruct, lpDMout);
		PUTWORD(lpSP+2, (WORD)envp_global->reg.ss);
		PUTWORD(lpSP  , (WORD)((DWORD)lpStruct - (DWORD)ss));
		lpStructOut = lpStruct;
		lpStruct += lpDMin->dmSize + lpDMin->dmDriverExtra;
	}
	else
		PUTDWORD(lpSP, 0L);

	if (lpDMin) {
		PutDEVMODE(lpStruct, lpDMin);
		PUTWORD(lpSP+6, (WORD)envp_global->reg.ss);
		PUTWORD(lpSP+4, (WORD)((DWORD)lpStruct - (DWORD)ss));
	}
	else
		PUTDWORD(lpSP+4, 0L);

	PUTWORD(lpSP+8,  (WORD)GetDataSelectorFromInstance(hDriver));
	PUTWORD(lpSP+10, (WORD)hwnd);

	invoke_binary();

	if (lpDMout)
		GetDEVMODE(lpDMout,lpStructOut);

	retcode = (WORD)(short)LOWORD(envp_global->reg.ax);
	envp_global->reg.sp += STK_DATA_512;
	return retcode;
}

WORD
cnb_Enable(LPVOID lpDevInfo, WORD wStyle, LPSTR lpDestDevType,
		LPSTR lpOutputFile, LPVOID lpData)
{
    LPBYTE lpSP,ss,lpStruct;
    WORD wSel1=0,wSel2=0,wSel3=0;

    envp_global->reg.sp -= STK_DATA_512;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_ENABLE;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = GetPhysicalAddress(envp_global->reg.ss);
    if (lpData) {
	PutDEVMODE(lpStruct,(LPDEVMODE)lpData);
	PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
	PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
	lpStruct += 12*INT_86 + 4*UINT_86 + DWORD_86 + CCHDEVICENAME;
    }
    else
	PUTDWORD(lpSP,0L);
    if (lpOutputFile) {
	wSel1 = ASSIGNSEL(lpOutputFile, strlen(lpOutputFile)+1);
	PUTDWORD(lpSP+4,MAKELP(wSel1,0));
    }
    else 
	PUTDWORD(lpSP+4,0L);
    if (lpDestDevType) {
	wSel2 = ASSIGNSEL(lpDestDevType, strlen(lpDestDevType)+1);
	PUTDWORD(lpSP+8,MAKELP(wSel2,0));
    }
    else 
	PUTDWORD(lpSP+8,0L);
    PUTWORD(lpSP+12,wStyle);
    if (lpDevInfo) {
	if (wStyle & 1) {
	   PUTWORD(lpSP+16,(WORD)envp_global->reg.ss);
	   PUTWORD(lpSP+14,(WORD)((DWORD)lpStruct - (DWORD)ss));
	}
	else {
	    wSel3 = ASSIGNSEL(lpDevInfo, DEVICEsize);
	    PUTDWORD(lpSP+14,MAKELP(wSel3,0));
	}
    }
    else
	PUTDWORD(lpSP+14,0L);
    invoke_binary();
    if (wSel1)
	FreeSelector(wSel1);
    if (wSel2)
	FreeSelector(wSel2);
    if (wSel3)
	FreeSelector(wSel3);
    if (lpDevInfo && (wStyle & 1)) {
	GetGDIINFO((GDIINFO *)lpDevInfo,lpStruct);
	DEVICEsize = ((GDIINFO *)lpDevInfo)->dpDEVICEsize;
    }

    envp_global->reg.sp += STK_DATA_512;
    return (LOWORD(envp_global->reg.ax));
}

WORD
cnb_EnumDFonts(LPPDEVICE lpDestDev, LPSTR lpFaceName,
		FARPROC lpCallbackFunc, LPVOID lpClientData)
{
    LPBYTE lpSP;
    DWORD dwProc;
    WORD wSel=0,wSel1=0,wSel2=0;

    envp_global->reg.sp -= STK_ENUMDFONTS;
    lpSP = (LPBYTE)envp_global->reg.sp;

    if (lpDestDev) {
	wSel = ASSIGNSEL(lpDestDev, DEVICEsize);
	PUTDWORD(lpSP+12,MAKELP(wSel,0));
    }
    else
	PUTDWORD(lpSP+12,0L);

    if (lpFaceName) {
	wSel1 = ASSIGNSEL(lpFaceName, strlen(lpFaceName)+1);
	PUTDWORD(lpSP+8,MAKELP(wSel1,0));
    }
    else
	PUTDWORD(lpSP+8,0L);

    dwProc = (lpCallbackFunc)?
		make_binary_thunk((LPDWORD)lpCallbackFunc,(DWORD)IT_ENUMFONTPROC):0;
    PUTDWORD(lpSP+4,dwProc);

    PUTDWORD(lpSP,(DWORD)lpClientData);

    invoke_binary();
    if (wSel)
	FreeSelector(wSel);
    if (wSel1)
	FreeSelector(wSel1);
    if (wSel2)
	FreeSelector(wSel2);
    return (LOWORD(envp_global->reg.ax));
}

WORD
cnb_EnumObj(LPPDEVICE lpDestDev, WORD wStyle,
		FARPROC lpCallbackFunc, LPVOID lpClientData)
{
    LPBYTE lpSP;
    DWORD dwProc;
    WORD wSel=0,wSel1=0;

    envp_global->reg.sp -= STK_ENUMOBJ;
    lpSP = (LPBYTE)envp_global->reg.sp;

    if (lpDestDev) {
	wSel = ASSIGNSEL(lpDestDev, DEVICEsize);
	PUTDWORD(lpSP+10,MAKELP(wSel,0));
    }
    else
	PUTDWORD(lpSP+10,0L);
    PUTWORD(lpSP+8,wStyle);

    switch (wStyle) {
	case DRV_OBJ_PEN:
	    dwProc = (lpCallbackFunc)?
		make_binary_thunk((LPDWORD)lpCallbackFunc,(DWORD)IT_ENUMPENPROC):0;
	    break;
	case DRV_OBJ_BRUSH:
	    dwProc = (lpCallbackFunc)?
		make_binary_thunk((LPDWORD)lpCallbackFunc,(DWORD)IT_ENUMBRUSHPROC):0;
	    break;
	default:
	    dwProc = 0;
	    printf("EnumObj: unknown object type!!!\n");
	    break;
    }
    PUTDWORD(lpSP+4,dwProc);
    PUTDWORD(lpSP,(DWORD)lpClientData);

    invoke_binary();
    if (wSel1)
	FreeSelector(wSel1);
    if (wSel)
	FreeSelector(wSel);
    return (LOWORD(envp_global->reg.ax));
}

int
cnb_Output(LPPDEVICE lpDestDev, WORD wStyle, WORD wCount, 
		LPPOINT lpPoints, LPVOID lpPhPen, LPVOID lpPhBrush,
		LPDRAWMODE lpDrawMode,
		LPRECT lpClipRect)
{
    LPBYTE lpSP;
    LPBYTE lpStruct,ss,lpData;
    WORD wSelDest=0,wSel1=0, wSel2=0, wSel3=0, wSelDestBmp = 0;
    WORD wSelDestBits = 0;
    LPPSBITMAP lpPSBitmap;
    int nByteCount = 0;
    BYTE bufDest[40];

    envp_global->reg.sp -= STK_DATA_512;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_OUTPUT;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = (LPBYTE)GetPhysicalAddress(envp_global->reg.ss);

    if (lpDestDev) {
	if (lpDestDev->pdType) {		/* device DC */
	    wSelDest = ASSIGNSEL(lpDestDev, DEVICEsize);
	    PUTDWORD(lpSP+24, MAKELP(wSelDest,0));
	}
	else {				/* memory DC */
	    lpPSBitmap = (LPPSBITMAP)lpDestDev;
	    lpData = bufDest;
	    PutPSBITMAP(lpData,lpPSBitmap);
	    if (lpPSBitmap->bm.bmBits) {
		nByteCount = 
			lpPSBitmap->bm.bmWidthBytes*lpPSBitmap->bm.bmHeight;
		wSelDestBits = ASSIGNSEL(lpPSBitmap->bm.bmBits,nByteCount);
		PUTDWORD(lpData+10, MAKELP(wSelDestBits,0));
	    }
	    else
		PUTDWORD(lpData+10,0L);
	    if (nByteCount > 65535)
		FatalAppExit(0,"cnb_Output: huge dest bitmap");
	    wSelDestBmp = ASSIGNSEL(lpData,32);
	    PUTDWORD(lpSP+24, MAKELP(wSelDestBmp,0));
	}
    }
    else
        PUTDWORD(lpSP+24, 0L);

    PUTWORD(lpSP+22,wStyle);
    PUTWORD(lpSP+20,wCount);

    /* now convert the array of points */
    if (lpPoints) {
	wSel1 = ASSIGNSEL(hsw_ConvertArrayToWord((LPBYTE)lpPoints,2*wCount),
					2 * wCount * WORD_86);
	PUTDWORD(lpSP+16,MAKELP(wSel1,0));
    }
    else
	PUTDWORD(lpSP+16,0L);

    /* convert the physical pen */
    if (lpPhPen) {
	wSel2 = ASSIGNSEL(lpPhPen, STK_DATA_256);
	PUTDWORD(lpSP+12,MAKELP(wSel2,0));
    }
    else
	PUTDWORD(lpSP+12,0L);

    /* convert the physical brush */
    if (lpPhBrush) {
	wSel3 = ASSIGNSEL(lpPhBrush, STK_DATA_256);
	PUTDWORD(lpSP+8,MAKELP(wSel3,0));
    }
    else
	PUTDWORD(lpSP+8,0L);

    /* convert the DRAWMODE struct */
    if (lpDrawMode) {
	PutDRAWMODE(lpStruct,lpDrawMode);
	PUTWORD(lpSP+6,(WORD)envp_global->reg.ss);
	PUTWORD(lpSP+4,(WORD)((DWORD)lpStruct - (DWORD)ss));
	lpStruct += 32;
    }
    else
	PUTDWORD(lpSP+4,0L);

    /* convert the clipping rect */
    if (lpClipRect) {
	PUTWORD(lpStruct,(WORD)lpClipRect->left);
	PUTWORD(lpStruct+2,(WORD)lpClipRect->top);
	PUTWORD(lpStruct+4,(WORD)lpClipRect->right);
	PUTWORD(lpStruct+6,(WORD)lpClipRect->bottom);
	PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
	PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
    }
    else
	PUTDWORD(lpSP,0L);

    invoke_binary();
    if (wSel3)
	FreeSelector(wSel3);
    if (wSel2)
	FreeSelector(wSel2);
    if (wSel1)
	FreeSelector(wSel1);
    if (wSelDest)
	FreeSelector(wSelDest);
    if (wSelDestBmp)
	FreeSelector(wSelDestBmp);
    if (wSelDestBits)
	FreeSelector(wSelDestBits);

    envp_global->reg.sp += STK_DATA_512;
    return (int)(short)(LOWORD(envp_global->reg.ax));
}

void
cnb_DeviceMode(HWND hWnd, HINSTANCE hInstance,
		LPSTR lpDestDevType, LPSTR lpOutputFile)
{
    LPBYTE lpSP;
    WORD wSel1=0,wSel2=0;

    envp_global->reg.sp -= STK_DEVICEMODE;
    lpSP = (LPBYTE)envp_global->reg.sp;
    PUTWORD(lpSP+10,(WORD)hWnd);
    PUTWORD(lpSP+8,(WORD)GetDataSelectorFromInstance(hInstance));
    if (lpDestDevType) {
	wSel1 = ASSIGNSEL(lpDestDevType, strlen(lpDestDevType)+1);
	PUTDWORD(lpSP+4,MAKELP(wSel1,0));
    }
    else 
	PUTDWORD(lpSP+4,0L);
    if (lpOutputFile) {
	wSel2 = ASSIGNSEL(lpOutputFile, strlen(lpOutputFile)+1);
	PUTDWORD(lpSP,MAKELP(wSel2,0));
    }
    else 
	PUTDWORD(lpSP,0L);
    invoke_binary();
    if (wSel1)
	FreeSelector(wSel1);
    if (wSel2)
	FreeSelector(wSel2);
}


DWORD
cnb_RealizeObject(LPPDEVICE lpDestDev, int iStyle, LPVOID lpInObj,
		LPVOID lpOutObj, TEXTXFORM *lpTextXForm)   
{
    static int	iPhFontSize;
    LPBYTE	lpOutBin = NULL;
    LPBYTE	lpSP, lpStruct,ss;
    LPBYTE	lptxfBin=NULL;
    DWORD	retcode;
    LPLOGPEN	lplp;
    LPLOGBRUSH	lplb;
    WORD	wSel=0, wSel1 = 0, wSel2 = 0, wSelBmp = 0;
    BOOL	b2ndCall = FALSE;
    int		size;
	

    envp_global->reg.ax = envp_global->reg.dx = 0;
    envp_global->reg.sp -= STK_DATA_512;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_REALIZEOBJ;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = GetPhysicalAddress(envp_global->reg.ss);

    PUTWORD(lpSP+12, iStyle);

    if (lpDestDev) {
	wSel = ASSIGNSEL(lpDestDev, DEVICEsize);
	PUTDWORD(lpSP+14,MAKELP(wSel,0));
    }
    else 
	PUTDWORD(lpSP+14,0L);

    switch ( iStyle ) {

	case DRV_OBJ_PEN:
	    PUTDWORD(lpSP+8,0L);

	    if ( lpInObj ) {
		/* Convert the logical pen */
		lplp = (LPLOGPEN)lpInObj;
		PUTWORD(lpStruct,(WORD)lplp->lopnStyle);
		PUTWORD(lpStruct+2,(WORD)lplp->lopnWidth.x);
		PUTWORD(lpStruct+4,(WORD)lplp->lopnWidth.y);
		PUTDWORD(lpStruct+6,lplp->lopnColor);
		PUTWORD(lpSP+10,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP+8,(WORD)((DWORD)lpStruct - (DWORD)ss));
		lpStruct += 12;
	    }
	    else
		PUTDWORD(lpSP+8,0L);

	    if (lpOutObj) {
		wSel2 = ASSIGNSEL(lpOutObj, STK_DATA_256);
		PUTDWORD(lpSP+4,MAKELP(wSel2,0));
	    }
	    else
		PUTDWORD(lpSP+4,0L);

	    break;

	case DRV_OBJ_FONT:
	    if (lpTextXForm) {
		PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
		lptxfBin = lpStruct;
		lpStruct += 20;		/* 19 */
	    }
	    else 
		PUTDWORD(lpSP,0L);

	    if ( lpInObj ) {
		PutLOGFONT(lpStruct,(LPLOGFONT)lpInObj,80);
		PUTWORD(lpSP+10,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP+8,(WORD)((DWORD)lpStruct - (DWORD)ss));
		lpStruct += 52;	/* 50 */
	    }
	    else 
		PUTDWORD(lpSP+8,0L);

	    if (lpOutObj) {
		/* 2nd call */
		/* Should be in separate segment */
		if ( !(lpOutBin = (LPBYTE)WinMalloc(iPhFontSize)) ) {
		    envp_global->reg.sp += STK_DATA_512 + STK_REALIZEOBJ;
		    return 0;
		}
		b2ndCall = TRUE;
		wSel2 = ASSIGNSEL(lpOutBin, iPhFontSize);
		PUTDWORD(lpSP+4,MAKELP(wSel2,0));
	    }
	    else
		/* 1st call */
		PUTDWORD(lpSP+4,0L);

	    break;

	case DRV_OBJ_BRUSH:
	    if (lpTextXForm) {
		PUTWORD(lpStruct,((LPPOINT)lpTextXForm)->x);
		PUTWORD(lpStruct+2,((LPPOINT)lpTextXForm)->y);
		PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
		lpStruct += 4;
	    }
	    else
		PUTDWORD(lpSP,0L);

	    if ( lpInObj ) {
		LPBYTE lpTmp = lpStruct+20;

		lplb = (LOGBRUSH *)lpInObj;
		PUTWORD(lpStruct,lplb->lbStyle);

		if ( lplb->lbStyle == BS_PATTERN ) {
			wSelBmp = PutBITMAP(lpTmp, (LPBITMAP)lplb->lbColor);
			PUTWORD(lpStruct+4,(WORD)envp_global->reg.ss);
			PUTWORD(lpStruct+2,(WORD)((DWORD)lpTmp - (DWORD)ss));
			lpTmp += 16;
		}
		else
		    	PUTDWORD(lpStruct+2,lplb->lbColor);

		PUTWORD(lpStruct+6,lplb->lbHatch);
		PUTWORD(lpSP+10,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP+8,(WORD)((DWORD)lpStruct - (DWORD)ss));
	    }
	    else
		PUTDWORD(lpSP+8,0L);

	    if (lpOutObj) {
		wSel2 = ASSIGNSEL(lpOutObj, STK_DATA_256);
		PUTDWORD(lpSP+4,MAKELP(wSel2,0));
	    }
	    else
		PUTDWORD(lpSP+4,0L);

	    break;

	case DRV_OBJ_PBITMAP:
#ifdef LATER
	    PUTDWORD(lpSP+8,0L);
	    if (lpInObj) {
		wSel1 = ASSIGNSEL(lpInObj, xxx);    /* Figure it out LATER */
		PUTDWORD(lpSP+8,MAKELP(wSel1,0));
	    }
	    else {
		PUTDWORD(lpSP+8,0L);
	    }
	    if (lpOutObj) {
		wSel2 = ASSIGNSEL(lpOutObj, xxx);   /* Figure it out LATER */
		PUTDWORD(lpSP+4,MAKELP(wSel2,0));
	    }
	    else
		PUTDWORD(lpSP+4,0L);

	    break;
#endif

	case -DRV_OBJ_PEN:
	case -DRV_OBJ_FONT:
	case -DRV_OBJ_BRUSH:
	    PUTDWORD(lpSP,0L);		/* TEXTXFORM */
	    size = 0;

	    if ( iStyle == -DRV_OBJ_FONT ) {
		size = SIZEOF_DFONTINFO_BIN + ((RFONT*)lpOutObj)->iBinSize;

		if ( !(lpOutBin = (LPBYTE)WinMalloc(size)) ) {
		     envp_global->reg.sp += STK_DATA_512 + STK_REALIZEOBJ;
		     return 0;
		}
		PutRFONT(lpOutBin, (RFONT*)lpOutObj);
	    }
	    else if ( lpOutObj ) {
		lpOutBin = (LPBYTE)lpOutObj;
		size = STK_DATA_256;
	    }

	    if ( lpOutBin ) {
		wSel2 = ASSIGNSEL(lpOutBin, size);
		PUTDWORD(lpSP+4,MAKELP(wSel2,0));	/* OutObj */
		PUTDWORD(lpSP+8,MAKELP(wSel2,0));	/* InObj == OutObj */
	    }
	    else {
		PUTDWORD(lpSP+4,0L);
		PUTDWORD(lpSP+8,0L);
	    }

	    break;

	default:
	    retcode = 0x80000000L;
	    envp_global->reg.sp += STK_DATA_512 + STK_REALIZEOBJ;
	    return retcode;
    }

    invoke_binary();

    if (wSel)
	FreeSelector(wSel);
    if (wSel1)
	FreeSelector(wSel1);
    if (wSel2)
	FreeSelector(wSel2);
    if (wSelBmp)
	FreeSelector(wSelBmp);

    retcode = MAKELONG(LOWORD(envp_global->reg.ax),LOWORD(envp_global->reg.dx));

    if ( retcode ) {
	if ( iStyle == DRV_OBJ_FONT )
	    if ( b2ndCall && lpOutBin) {
		GetTEXTXFORM(lpTextXForm, lptxfBin);
		GetRFONT((RFONT*)lpOutObj, lpOutBin, retcode);
		WinFree((LPSTR)lpOutBin);
	    }
	    else {
		iPhFontSize = retcode;
		retcode += sizeof(RFONT) + SIZEOF_DFONTINFO_BIN;
	    }
    }

    envp_global->reg.sp += STK_DATA_512;
    return retcode;	/* Size of the output structure */
}


WORD
cnb_Control(LPPDEVICE lpDestDev, WORD wFunction,
		LPSTR lpInData, LPSTR lpOutData)
{
    WORD	wSel = 0, wSel1 = 0, wSel2 = 0, wSel3 = 0;
    LPBYTE	lpSP,lpStruct,ss;
    HDC		hDC;
    LPRECT	lpRect;
    LPDOCINFO	lpdi;

    envp_global->reg.ax = 0;
    envp_global->reg.sp -= STK_DATA_512;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_CONTROL;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = GetPhysicalAddress(envp_global->reg.ss);

    PUTWORD(lpSP+8,wFunction);

    if (lpDestDev) {
	wSel = ASSIGNSEL(lpDestDev, DEVICEsize);
	PUTDWORD(lpSP+10,MAKELP(wSel,0));
    }
    else
	PUTDWORD(lpSP+10,0L);

    switch(wFunction) {
	case NEWFRAME:
	case ENDDOC:
	case ABORTDOC:
	case BEGIN_PATH:
	case FLUSHOUTPUT:
	    PUTDWORD(lpSP,0L);
	    PUTDWORD(lpSP+4,0L);
	    invoke_binary();
	    break;
	case SETABORTPROC:
	    PUTDWORD(lpSP,0L);
	    hDC = GETWORD(lpInData);
	    wSel1 = ASSIGNSEL(&hDC, sizeof(HDC));
	    PUTDWORD(lpSP+4,MAKELP(wSel1,0));
	    invoke_binary();
	    break;
	case RESETDC:
	    PUTDWORD(lpSP,0L);
	    wSel1 = ASSIGNSEL(lpInData, strlen(lpInData)+1);
	    PUTDWORD(lpSP+4,MAKELP(wSel1,0));
	    invoke_binary();
	    break;

	case STARTDOC:
	    wSel1 = ASSIGNSEL(lpInData, strlen(lpInData)+1);
	    PUTDWORD(lpSP+4,MAKELP(wSel1,0));
	    if ((lpdi = (LPDOCINFO)lpOutData)) {
		PUTWORD(lpStruct, lpdi->cbSize);
		if (lpdi->lpszDocName) {
		    wSel2 = ASSIGNSEL(lpdi->lpszDocName,
				strlen(lpdi->lpszDocName)+1);
		    PUTDWORD(lpStruct+2,MAKELP(wSel2,0));
		}
		else
		    PUTDWORD(lpStruct+2,0);
		if (lpdi->lpszOutput) {
		    wSel3 = ASSIGNSEL(lpdi->lpszOutput,
				strlen(lpdi->lpszOutput)+1);
		    PUTDWORD(lpStruct+6,MAKELP(wSel3,0));
		}
		else
		    PUTDWORD(lpStruct+6,0);
		PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
	    }
	    else
		PUTDWORD(lpSP,0L);
	    invoke_binary();
	    break;

	case GETFACENAME:
	case GETTECHNOLOGY:
	    PUTDWORD(lpSP+4,0L);
	    if (lpOutData) {
		wSel1 = ASSIGNSEL(lpOutData, STK_DATA_256);
		PUTDWORD(lpSP,MAKELP(wSel1,0));
	    }
	    else
		PUTDWORD(lpSP,0L);
	    invoke_binary();
	    break;

	case GETPHYSPAGESIZE:
	case GETPRINTINGOFFSET:
	case GETSCALINGFACTOR:
	    PUTDWORD(lpSP+4,0L);
	    if (lpOutData) {
		PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
	    }
	    else
		PUTDWORD(lpSP,0L);
	    invoke_binary();
	    if (lpOutData) {
		((LPPOINT)lpOutData)->x = (int)(short)GETWORD(lpStruct);
		((LPPOINT)lpOutData)->y = (int)(short)GETWORD(lpStruct+2);
	    }
	    break;

	case ENABLEPAIRKERNING:
	case SETKERNTRACK:
	    if ( lpInData ) {
		/* cbRFsize = 0 : Don't care about font */
		PutEXTTEXTDATA(lpStruct,(WORD)envp_global->reg.ss,(DWORD)ss,0);
		PUTWORD(lpSP+6, (WORD)envp_global->reg.ss);
		PUTWORD(lpSP+4, (WORD)((DWORD)lpStruct - (DWORD)ss));
		/* Prepare APPEXTTEXTDATA (PutAPPEXTTEXTDATA() ?) */
		PUTWORD(lpStruct + SIZEOF_APPEXTTEXTDATA_BIN,
				(WORD)(((LPEXTTEXTDATA)lpInData)->lpInData->x));
	    }
	    else
		PUTDWORD(lpSP+4,0L);
	    if ( lpOutData ) {
		wSel2 = ASSIGNSEL(lpOutData, STK_DATA_256);
            	PUTDWORD(lpSP,MAKELP(wSel2,0));
	    }
	    else
		PUTDWORD(lpSP,0L);
	    invoke_binary();
	    if ( lpOutData )
		*((LPINT)lpOutData) = (int)GETWORD(lpStruct);
	    break;

	case SETLINECAP:
	case SETCOPYCOUNT:
	case ENABLERELATIVEWIDTHS:
	case QUERYESCSUPPORT:
	    if (lpInData) {
		PUTWORD(lpStruct,(WORD)(*((int *)lpInData)));
		PUTWORD(lpSP+6,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP+4,(WORD)((DWORD)lpStruct - (DWORD)ss));
		lpStruct += 4;
	    }
	    else
		PUTDWORD(lpSP+4,0L);
	    if (lpOutData) {
		PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
	    }
	    else
		PUTDWORD(lpSP,0L);
	    invoke_binary();
	    if (lpOutData) 
		*((int *)lpOutData) = (int)(short)GETWORD(lpStruct);
	    break;
	case EXT_DEVICE_CAPS:
	    if (lpInData) {
		PUTWORD(lpStruct,(WORD)(*((int *)lpInData)));
		PUTWORD(lpSP+6,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP+4,(WORD)((DWORD)lpStruct - (DWORD)ss));
		lpStruct += 4;
	    }
	    else
		PUTDWORD(lpSP+4,0L);
	    if (lpOutData) {
		PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
	    }
	    else
		PUTDWORD(lpSP,0L);
	    invoke_binary();
	    if (lpOutData) 
		*((long *)lpOutData) = GETDWORD(lpStruct);
	    break;
	case SET_BOUNDS:
	case SET_CLIP_BOX:
	    if (lpInData) {
		lpRect = (LPRECT)lpInData;
		PUTWORD(lpStruct,(WORD)lpRect->left);
		PUTWORD(lpStruct+2,(WORD)lpRect->top);
		PUTWORD(lpStruct+4,(WORD)lpRect->right);
		PUTWORD(lpStruct+6,(WORD)lpRect->bottom);
		PUTWORD(lpSP+6,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP+4,(WORD)((DWORD)lpStruct - (DWORD)ss));
	    }
	    else
		PUTDWORD(lpSP+4,0L);
	    PUTDWORD(lpSP,0L);
	    invoke_binary();
	    break;
	case NEXTBAND:
		PUTWORD(lpStruct,  (WORD)((LPRECT)lpOutData)->left);
		PUTWORD(lpStruct+2,(WORD)((LPRECT)lpOutData)->top);
		PUTWORD(lpStruct+4,(WORD)((LPRECT)lpOutData)->right);
		PUTWORD(lpStruct+6,(WORD)((LPRECT)lpOutData)->bottom);
		PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP  ,(WORD)((DWORD)lpStruct - (DWORD)ss));
		invoke_binary();
		((LPRECT)lpOutData)->left   = (int)GETWORD(lpStruct);
		((LPRECT)lpOutData)->top    = (int)GETWORD(lpStruct+2);
		((LPRECT)lpOutData)->right  = (int)GETWORD(lpStruct+4);
		((LPRECT)lpOutData)->bottom = (int)GETWORD(lpStruct+6);
		break;
	case SETCHARSET:
	    if (lpInData) {
		PUTWORD(lpStruct,(WORD)(*((int *)lpInData)));
		PUTWORD(lpSP+6,(WORD)envp_global->reg.ss);
		PUTWORD(lpSP+4,(WORD)((DWORD)lpStruct - (DWORD)ss));
		lpStruct += 4;
	    }
	    else
		PUTDWORD(lpSP+4,0L);

	    PUTDWORD(lpSP,0L);
	    invoke_binary();
	    break;

	case GETEXTENDEDTEXTMETRICS: {	/* MS Word 6.0 */
		LPEXTTEXTDATA	lpetd = (LPEXTTEXTDATA)lpInData;
		LPBYTE		lpExtTextData;
		LPBYTE		lpAppExtTextData;
		LPBYTE		lpDFontInfo;
		LPBYTE		lpXForm;
		LPBYTE		lpDrawMode;
		int		cbRFsize;
		WORD		wStackSeg;

		if ( !lpInData  ||  !lpOutData ) {
			envp_global->reg.ax = 0;
			break;
		}
		cbRFsize = SIZEOF_DFONTINFO_BIN +
                                        ((RFONT*)lpetd->lpFont)->iBinSize;

		/* Create separate frame for every structure */
		lpExtTextData     = lpStruct;
		lpStruct         += SIZEOF_EXTTEXTDATA_BIN;
		lpAppExtTextData  = lpStruct;
		lpStruct         += SIZEOF_APPEXTTEXTDATA_BIN;
		lpDFontInfo       = lpStruct;
		lpStruct         += cbRFsize;
		lpXForm           = lpStruct;
		lpStruct         += SIZEOF_TEXTXFORM_BIN;
		lpDrawMode        = lpStruct;
		lpStruct         += SIZEOF_DRAWMODE_BIN;
		/* lpStruct points to output binary ETM structure */

		wStackSeg = (WORD)envp_global->reg.ss;

		/* Prepare EXTTEXTDATA */
		PutEXTTEXTDATA(lpExtTextData, wStackSeg, (DWORD)ss, cbRFsize);

		/* Prepare APPEXTTEXTDATA (PutAPPEXTTEXTDATA() ?) */
		PUTWORD(lpAppExtTextData, (WORD)lpetd->lpInData->x);

		/* Prepare realized font */
		PutRFONT(lpDFontInfo, (RFONT*)lpetd->lpFont);

		/* Prepare TEXTXFORM */
		PutTEXTXFORM(lpXForm, lpetd->lpXForm);

		/* Prepare DRAWMODE */
		PutDRAWMODE(lpDrawMode, lpetd->lpDrawMode);

		/* Put address of input structure onto stack */
		PUTWORD(lpSP+6, wStackSeg);
		PUTWORD(lpSP+4, (WORD)((DWORD)lpExtTextData - (DWORD)ss));

		/* Put address of output structure onto stack */
		PUTWORD(lpSP+2, wStackSeg);
		PUTWORD(lpSP,   (WORD)((DWORD)lpStruct - (DWORD)ss));
		PUTWORD(lpStruct, SIZEOF_ETM_BIN);

		invoke_binary();

		GetETM((EXTTEXTMETRIC *)lpOutData, lpStruct);
		break;
	}

	default:
	    ERRSTR((LF_ERROR,"CONTROL: UNIMP escape %d (%#x)\n",wFunction));
	    if (wSel)
		FreeSelector(wSel);
	    return 0;
    }
    if (wSel)
	FreeSelector(wSel);
    if (wSel1)
	FreeSelector(wSel1);
    if (wSel2)
	FreeSelector(wSel2);
    if (wSel3)
	FreeSelector(wSel3);
    envp_global->reg.sp += STK_DATA_512;
    return LOWORD(envp_global->reg.ax);
}


DWORD
cnb_ExtTextOut(LPPDEVICE lpDestDev, WORD wDestXOrg, WORD wDestYOrg,
		LPRECT lpClipRect, LPSTR lpString, int wCount,
		LPDFONTINFO lpFontInfo, LPDRAWMODE lpDrawMode,
		TEXTXFORM *lpTextXForm, short int *lpCharWidths,
		LPRECT lpOpaqueRect, WORD wOptions)
{
    LPBYTE	lpSP;
    LPBYTE	lpStruct,ss;
    LPBYTE	lpOutBin=NULL;
    WORD	wSel=0,wSel1=0,wSel2=0,wSel3=0;

    envp_global->reg.ax = envp_global->reg.dx = 0;
    envp_global->reg.sp -= STK_DATA_512;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_EXTTEXTOUT;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = (LPBYTE)GetPhysicalAddress(envp_global->reg.ss);

    if (lpDestDev) {
	wSel = ASSIGNSEL(lpDestDev, DEVICEsize);
	PUTDWORD(lpSP+36,MAKELP(wSel,0));
    }
    else
	PUTDWORD(lpSP+36,0L);

    PUTWORD(lpSP+34,wDestXOrg);
    PUTWORD(lpSP+32,wDestYOrg);

    /* convert the clipping rect */
    if (lpClipRect) {
        PUTWORD(lpStruct,(WORD)lpClipRect->left);
        PUTWORD(lpStruct+2,(WORD)lpClipRect->top);
        PUTWORD(lpStruct+4,(WORD)lpClipRect->right);
        PUTWORD(lpStruct+6,(WORD)lpClipRect->bottom);
        PUTWORD(lpSP+30,(WORD)envp_global->reg.ss);
        PUTWORD(lpSP+28,(WORD)((DWORD)lpStruct - (DWORD)ss));
	lpStruct += 8;
    }
    else
        PUTDWORD(lpSP+28,0L);

    /* convert text string */
    if (lpString) {
	wSel1 = ASSIGNSEL(lpString, strlen(lpString)+1);
	PUTDWORD(lpSP+24,MAKELP(wSel1,0));
    }
    else
	PUTDWORD(lpSP+24,0L);

    PUTWORD(lpSP+22,(WORD)wCount);

    /* pass a pointer to DFONTINFO struct */
    if (lpFontInfo) {
	/* Should be in separate segment */
	int size = SIZEOF_DFONTINFO_BIN + ((RFONT*)lpFontInfo)->iBinSize;

	if ( !(lpOutBin = (LPBYTE)WinMalloc(size)) ) {
		envp_global->reg.sp += STK_DATA_512 + STK_EXTTEXTOUT;
		return 0;
	}
	PutRFONT(lpOutBin, (RFONT*)lpFontInfo);
	wSel2 = ASSIGNSEL(lpOutBin, size);
	PUTDWORD(lpSP+18,MAKELP(wSel2,0));
    }
    else
	PUTDWORD(lpSP+18,0L);

    /* convert the DRAWMODE struct */
    if (lpDrawMode) {
	PutDRAWMODE(lpStruct,lpDrawMode);
        PUTWORD(lpSP+16,(WORD)envp_global->reg.ss);
        PUTWORD(lpSP+14,(WORD)((DWORD)lpStruct - (DWORD)ss));
        lpStruct += 32;
    }
    else
        PUTDWORD(lpSP+14,0L);

    /* convert the TEXTXFORM struct */
    if (lpTextXForm) {
	PutTEXTXFORM(lpStruct,lpTextXForm);
        PUTWORD(lpSP+12,(WORD)envp_global->reg.ss);
        PUTWORD(lpSP+10,(WORD)((DWORD)lpStruct - (DWORD)ss));
        lpStruct += 24;
    }
    else
        PUTDWORD(lpSP+10,0L);
 
    /* convert array of char widths  */
    if (lpCharWidths && wCount) {
	wSel3 = ASSIGNSEL(hsw_ConvertArrayToWord((LPBYTE)lpCharWidths,
					abs(wCount)), abs(wCount) * WORD_86);
	PUTDWORD(lpSP+6,MAKELP(wSel3,0));
    }
    else
        PUTDWORD(lpSP+6,0L);

    /* convert the opaquing rectangle */
    if (lpOpaqueRect) {
        PUTWORD(lpStruct,(WORD)lpOpaqueRect->left);
        PUTWORD(lpStruct+2,(WORD)lpOpaqueRect->top);
        PUTWORD(lpStruct+4,(WORD)lpOpaqueRect->right);
        PUTWORD(lpStruct+6,(WORD)lpOpaqueRect->bottom);
        PUTWORD(lpSP+4,(WORD)envp_global->reg.ss);
        PUTWORD(lpSP+2,(WORD)((DWORD)lpStruct - (DWORD)ss));
	lpStruct += 8;
    }
    else
        PUTDWORD(lpSP+2,0L);

    PUTWORD(lpSP,wOptions);

    invoke_binary();

    if (wSel)
        FreeSelector(wSel);
    if (wSel1)
        FreeSelector(wSel1);
    if (wSel2)
        FreeSelector(wSel2);
    if (wSel3)
        FreeSelector(wSel3);
    if ( lpOutBin )
	WinFree((LPSTR)lpOutBin);

    envp_global->reg.sp += STK_DATA_512;
    return MAKELONG(LOWORD(envp_global->reg.ax),
		    LOWORD(envp_global->reg.dx));
}


WORD
cnb_GetCharWidth(LPPDEVICE lpDestDev, LPWORD lpBuffer,
		WORD wFirstChar, WORD wLastChar, 
		LPDFONTINFO lpFontInfo, LPDRAWMODE lpDrawMode,
		TEXTXFORM *lpTextXForm)
{
    LPBYTE	lpSP;
    LPBYTE	lpStruct,ss;
    LPBYTE	lpOutBin = (LPBYTE)0;
    int		nCount;
    WORD	wSel=0,wSel1=0,wSel2=0;

    envp_global->reg.ax = 0;
    envp_global->reg.sp -= STK_DATA_576;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_GETCHARWID;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = (LPBYTE)GetPhysicalAddress(envp_global->reg.ss);

    if (lpDestDev) {
	wSel = ASSIGNSEL(lpDestDev, DEVICEsize);
	PUTDWORD(lpSP+20,MAKELP(wSel,0));
    }
    else
	PUTDWORD(lpSP+20,0L);

    PUTWORD(lpSP+14,wFirstChar);
    PUTWORD(lpSP+12,wLastChar);

    /* convert the DFONTINFO struct */
    if (lpFontInfo) {
	/* Should be in separate segment */
	int size = SIZEOF_DFONTINFO_BIN + ((RFONT*)lpFontInfo)->iBinSize;

	if ( !(lpOutBin = (LPBYTE)WinMalloc(size)) ) {
		envp_global->reg.sp += STK_DATA_576 + STK_GETCHARWID;
		return 0;
	}
	PutRFONT(lpOutBin, (RFONT*)lpFontInfo);
	wSel1 = ASSIGNSEL(lpOutBin, size);
	PUTDWORD(lpSP+8,MAKELP(wSel1,0));
    }
    else
	PUTDWORD(lpSP+8,0L);

    /* convert the DRAWMODE struct */
    if (lpDrawMode) {
	PutDRAWMODE(lpStruct,lpDrawMode);
        PUTWORD(lpSP+6,(WORD)envp_global->reg.ss);
        PUTWORD(lpSP+4,(WORD)((DWORD)lpStruct - (DWORD)ss));
        lpStruct += 32;
    }
    else
        PUTDWORD(lpSP+4,0L);

    /* convert the TEXTXFORM struct */
    if (lpTextXForm) {
	PutTEXTXFORM(lpStruct,lpTextXForm);
        PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
        PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
        lpStruct += 24;
    }
    else
        PUTDWORD(lpSP,0L);

    /* determine the number of chars in given range */
    if ( (nCount = wLastChar - wFirstChar + 1) < 0 )
	nCount = 0;

    if (lpBuffer && nCount) {
        PUTWORD(lpSP+18,(WORD)envp_global->reg.ss);
        PUTWORD(lpSP+16,(WORD)((DWORD)lpStruct - (DWORD)ss));
    }
    else
	PUTDWORD(lpSP+16,0L);

    if (nCount) {				/* sanity check */
	invoke_binary();
	if (LOWORD(envp_global->reg.ax)) 	/* function was successful */
	    memcpy(lpBuffer,(LPBYTE)hsw_ConvertArrayToInt(lpStruct,nCount),
			sizeof(int)*nCount);
    }

    if (wSel)
        FreeSelector(wSel);
    if (wSel1)
        FreeSelector(wSel1);
    if (wSel2)
        FreeSelector(wSel2);
    if ( lpOutBin )
	WinFree((LPSTR)lpOutBin);

    envp_global->reg.sp += STK_DATA_576;
    return LOWORD(envp_global->reg.ax);
}

DWORD
cnb_DeviceCapabilities(LPSTR lpDeviceName, LPSTR lpPort, WORD wIndex,
		LPSTR lpOutput, LPDEVMODE lpDevMode)
{
    LPBYTE lpSP;
    LPBYTE lpStruct,ss;
    DWORD retcode;
    int i;
    WORD wSel=0,wSel1=0,wSel2=0;
    LPBYTE lpCaps=NULL;
    static int nNumPapers = 0;
    static int nNumBins = 0;
    static int nNumRes = 0;

    envp_global->reg.ax = envp_global->reg.dx = 0;
    envp_global->reg.sp -= STK_DATA_512;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_DEVICECAPS;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = (LPBYTE)GetPhysicalAddress(envp_global->reg.ss);

    if (lpDeviceName) {
	wSel = ASSIGNSEL(lpDeviceName, strlen(lpDeviceName)+1);
	PUTDWORD(lpSP+14,MAKELP(wSel,0));
    }
    else
	PUTDWORD(lpSP+14,0L);

    if (lpPort) {
	wSel1 = ASSIGNSEL(lpPort, strlen(lpPort)+1);
	PUTDWORD(lpSP+10,MAKELP(wSel1,0));
    }
    else
	PUTDWORD(lpSP+10,0L);

    PUTWORD(lpSP+8,wIndex);

    if (lpDevMode) {
	PutDEVMODE(lpStruct,lpDevMode);
	PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
	PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
    }
    else
	PUTDWORD(lpSP,0L);

    PUTDWORD(lpSP+4,0L);	/* default; overwritten when needed */

    switch (wIndex) {
	case DC_MAXEXTENT:
	case DC_MINEXTENT:	/* ??? -- not referred to in DDK source */

	case DC_PAPERSIZE:
	    printf("ALARM: unimp interface to func %x in DeviceCapabilities!\n",
				wIndex);
	    retcode = (DWORD)-1;
	    break;

	case DC_ENUMRESOLUTIONS:
	    if (nNumRes && lpOutput) {
		lpCaps = (LPBYTE)WinMalloc(2 * sizeof(LONG) * nNumRes);
		wSel2 = ASSIGNSEL(lpCaps, 2 * sizeof(LONG) * nNumRes);
		PUTDWORD(lpSP+4,MAKELP(wSel2,0));
	    }
	    invoke_binary();
	    retcode = MAKELONG(LOWORD(envp_global->reg.ax),
		    LOWORD(envp_global->reg.dx));
	    if (retcode != (DWORD)-1) {
		if (!lpOutput)
		    nNumRes = (int)retcode;
		else if (nNumRes) {
		    for (i=0; i<nNumRes; i++)
			*(LPDWORD)(lpOutput+4*i) = GETDWORD(lpCaps+4*i);
		    WinFree((LPSTR)lpCaps);
		}
	    }
	    break;

	case DC_PAPERS:
	    if (nNumPapers && lpOutput) {
		lpCaps = (LPBYTE)WinMalloc(2*nNumPapers);
		wSel2 = ASSIGNSEL(lpCaps, 2 * nNumPapers);
		PUTDWORD(lpSP+4,MAKELP(wSel2,0));
	    }
	    invoke_binary();
	    retcode = MAKELONG(LOWORD(envp_global->reg.ax),
		    LOWORD(envp_global->reg.dx));
	    if (retcode != (DWORD)-1) {
		if (!lpOutput)
		    nNumPapers = (int)retcode;
		else if (nNumPapers) {
		    memcpy(lpOutput,(LPSTR)hsw_ConvertWordArray
				(lpCaps,nNumPapers),sizeof(WORD)*nNumPapers);
		    WinFree((LPSTR)lpCaps);
		}
	    }
	    break;

	case DC_BINS:
	    if (nNumBins && lpOutput) {
		lpCaps = (LPBYTE)WinMalloc(2*nNumBins);
		wSel2 = ASSIGNSEL(lpCaps, 2*nNumBins);
		PUTDWORD(lpSP+4,MAKELP(wSel2,0));
	    }
	    invoke_binary();
	    retcode = MAKELONG(LOWORD(envp_global->reg.ax),
		    LOWORD(envp_global->reg.dx));
	    if (retcode != (DWORD)-1) {
		if (!lpOutput)
		    nNumBins = (int)retcode;
		else if (nNumBins) {
		    memcpy(lpOutput,(LPSTR)hsw_ConvertWordArray
				(lpCaps,nNumBins),sizeof(WORD)*nNumBins);
		    WinFree((LPSTR)lpCaps);
		}
	    }
	    break;

	case DC_BINNAMES:
	case DC_FILEDEPENDENCIES:
	case DC_PAPERNAMES:
	    if (lpOutput) {
		wSel2 = ASSIGNSEL(lpOutput, STK_DATA_256);
		PUTDWORD(lpSP+4,MAKELP(wSel2,0));
	    }
		/* fall through */
	default:
	    invoke_binary();
	    retcode = MAKELONG(LOWORD(envp_global->reg.ax),
		    LOWORD(envp_global->reg.dx));
	    break;
    }

    if (wSel)
        FreeSelector(wSel);
    if (wSel1)
        FreeSelector(wSel1);
    if (wSel2)
        FreeSelector(wSel2);
    envp_global->reg.sp += STK_DATA_512;
    return retcode;
}

int
cnb_StretchDIBits(LPPDEVICE lpPDevice, WORD fGet, 
			WORD DestX, WORD DestY, WORD DestXE, WORD DestYE,
			WORD SrcX,  WORD SrcY,  WORD SrcXE,  WORD SrcYE,
			LPSTR lpBits, LPBITMAPINFO lpBitmapInfo,
			LPINT lpTranslate, DWORD dwROP, LPBYTE lpPBrush, 
			LPDRAWMODE lpDrawMode, LPRECT lpClipRect)
{
    LPBYTE lpSP;
    LPBYTE lpStruct,ss;
    LPBYTE lpBinBMInfo = 0;
    WORD wSel=0,wSel1=0,wSel2=0,wSel3=0;
    int nNumSel = 0;
    WORD wIndex;
    DWORD dwSelSize;
    int i;

    envp_global->reg.ax = envp_global->reg.dx = 0;
    envp_global->reg.sp -= STK_DATA_512;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_STRETCHDIB;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = (LPBYTE)GetPhysicalAddress(envp_global->reg.ss);

    if (lpPDevice) {
	wSel = ASSIGNSEL(lpPDevice, DEVICEsize);
	PUTDWORD(lpSP+46,MAKELP(wSel,0));
    }
    else
	PUTDWORD(lpSP+46,0L);

    PUTWORD(lpSP+44,fGet);
    PUTWORD(lpSP+42,DestX);
    PUTWORD(lpSP+40,DestY);
    PUTWORD(lpSP+38,DestXE);
    PUTWORD(lpSP+36,DestYE);
    PUTWORD(lpSP+34,SrcX);
    PUTWORD(lpSP+32,SrcY);
    PUTWORD(lpSP+30,SrcXE);
    PUTWORD(lpSP+28,SrcYE);

    if (lpBits) {
	if (lpBitmapInfo->bmiHeader.biSizeImage <= 0xffff) {
	    wSel1 = ASSIGNSEL(lpBits, lpBitmapInfo->bmiHeader.biSizeImage);
	}
	else { /* huge bits */
	    nNumSel = (lpBitmapInfo->bmiHeader.biSizeImage + 0xffff) / 0x10000;
	    wIndex = AssignSelRange(nNumSel);
	    for (i = 0, dwSelSize = lpBitmapInfo->bmiHeader.biSizeImage;
		 i < nNumSel; dwSelSize -= 0x10000, i++) {
		wSel1 = (wIndex + i) << 3;
		AssignSelector((LPBYTE)(lpBits + (i * 0x10000)), wSel1,
				TRANSFER_DATA,dwSelSize);
	    }
	    wSel1 = wIndex << 3 | 7;
	}
	PUTDWORD(lpSP+24,MAKELP(wSel1,0));
    }
    else
	PUTDWORD(lpSP+24,0L);

    if (lpBitmapInfo) {
	int size = sizeof(BITMAPINFOHEADER);

	if ( lpBitmapInfo->bmiHeader.biBitCount == 1 )
	    size += 2 * sizeof(RGBQUAD);
	else if ( lpBitmapInfo->bmiHeader.biBitCount == 4 )
	    size += 16 * sizeof(RGBQUAD);
	else if ( lpBitmapInfo->bmiHeader.biBitCount == 8 )
	    size += 256 * sizeof(RGBQUAD);

	lpBinBMInfo = PutBITMAPINFO(lpBitmapInfo);
	wSel2 = ASSIGNSEL(lpBinBMInfo, size);
	PUTDWORD(lpSP+20,MAKELP(wSel2,0));
    }
    else
	PUTDWORD(lpSP+20,0L);

#ifdef LATER
	convert INT array of palette indices
#else
    PUTDWORD(lpSP+16,0L);
#endif

    PUTDWORD(lpSP+12,dwROP);

    if (lpPBrush) {
	wSel3 = ASSIGNSEL(lpPBrush, STK_DATA_256);
	PUTDWORD(lpSP+8,MAKELP(wSel3,0));
    }
    else
	PUTDWORD(lpSP+8,0L);

    /* convert the DRAWMODE struct */
    if (lpDrawMode) {
	PutDRAWMODE(lpStruct,lpDrawMode);
        PUTWORD(lpSP+6,(WORD)envp_global->reg.ss);
        PUTWORD(lpSP+4,(WORD)((DWORD)lpStruct - (DWORD)ss));
        lpStruct += 32;
    }
    else
        PUTDWORD(lpSP+4,0L);

    if (lpClipRect) {
        PUTWORD(lpStruct,(WORD)lpClipRect->left);
        PUTWORD(lpStruct+2,(WORD)lpClipRect->top);
        PUTWORD(lpStruct+4,(WORD)lpClipRect->right);
        PUTWORD(lpStruct+6,(WORD)lpClipRect->bottom);
        PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
        PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
	lpStruct += 8;
    }
    else
        PUTDWORD(lpSP,0L);

    invoke_binary();

    if (wSel)
        FreeSelector(wSel);
    if (wSel1)
        FreeSelector(wSel1);
    for (i = 1; i < nNumSel; i++)
	FreeSelector(wSel1+8*i);
    if (wSel2)
        FreeSelector(wSel2);
    if (wSel3)
        FreeSelector(wSel3);
    if (lpBinBMInfo)
	WinFree((LPSTR)lpBinBMInfo);
    envp_global->reg.sp += STK_DATA_512;
    return (int)(short)(LOWORD(envp_global->reg.ax));
}

int
cnb_BitBlt(LPPDEVICE lpDestDev, WORD wDestX, WORD wDestY,
		LPPDEVICE lpSrcDev, WORD wSrcX, WORD wSrcY,
		WORD wXext, WORD wYext, DWORD dwRop3, 
		LPVOID lpPhBrush, LPDRAWMODE lpDrawMode)
{
    LPBYTE	lpSP, lpStruct, ss, lpData;
    WORD	wSelDest = 0, wSelSrc = 0, wSelBr = 0;
    BYTE	bufSrc[64], bufDest[64];
    LPPSBITMAP lpPSBitmap;
    int nByteCount   = 0;
    WORD wSelSrcBits = 0, 
	 wSelSrcBmp  = 0, 
	 wSelDestBits = 0, 
 	 wSelDestBmp = 0;

    envp_global->reg.ax = 0;
    envp_global->reg.sp -= STK_DATA_512;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_BITBLT;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = GetPhysicalAddress(envp_global->reg.ss);

    if (lpDestDev) {
	if (lpDestDev->pdType) {		/* device DC */
	    wSelDest = ASSIGNSEL(lpDestDev, DEVICEsize);
	    PUTDWORD(lpSP+28, MAKELP(wSelDest,0));
	}
	else {				/* memory DC */
	    lpPSBitmap = (LPPSBITMAP)lpDestDev;
	    lpData = bufDest;
	    PutPSBITMAP(lpData,lpPSBitmap);
	    if (lpPSBitmap->bm.bmBits) {
		nByteCount = 
			lpPSBitmap->bm.bmWidthBytes*lpPSBitmap->bm.bmHeight;
		wSelDestBits = ASSIGNSEL(lpPSBitmap->bm.bmBits,nByteCount);
		PUTDWORD(lpData+10, MAKELP(wSelDestBits,0));
	    }
	    else
		PUTDWORD(lpData+10,0L);
	    if (nByteCount > 65535)
		FatalAppExit(0,"cnb_BitBlt: huge dest bitmap");
	    wSelDestBmp = ASSIGNSEL(lpData,32);
	    PUTDWORD(lpSP+28, MAKELP(wSelDestBmp,0));
	}
    }
    else
        PUTDWORD(lpSP+28, 0L);

    PUTWORD(lpSP+26, wDestX);
    PUTWORD(lpSP+24, wDestY);

    if (lpSrcDev) {
	if (lpSrcDev->pdType) {		/* device DC */
	    wSelSrc = ASSIGNSEL(lpSrcDev, DEVICEsize);
	    PUTDWORD(lpSP+20, MAKELP(wSelSrc,0));
	}
	else {				/* memory DC */
	    lpPSBitmap = (LPPSBITMAP)lpSrcDev;
	    lpData = bufSrc;
	    PutPSBITMAP(lpData,lpPSBitmap);
	    if (lpPSBitmap->bm.bmBits) {
		nByteCount = 
			lpPSBitmap->bm.bmWidthBytes*lpPSBitmap->bm.bmHeight;
		wSelSrcBits = ASSIGNSEL(lpPSBitmap->bm.bmBits,nByteCount);
		PUTDWORD(lpData+10, MAKELP(wSelSrcBits,0));
	    }
	    else
		PUTDWORD(lpData+10,0L);
	    if (nByteCount > 65535)
		FatalAppExit(0,"cnb_BitBlt: huge source bitmap");
	    wSelSrcBmp = ASSIGNSEL(lpData,32);
	    PUTDWORD(lpSP+20, MAKELP(wSelSrcBmp,0));
	}
    }
    else
        PUTDWORD(lpSP+20, 0L);

    PUTWORD(lpSP+18, wSrcX);
    PUTWORD(lpSP+16, wSrcY);
    PUTWORD(lpSP+14, wXext);
    PUTWORD(lpSP+12, wYext);
    PUTDWORD(lpSP+8, dwRop3);

    if (lpPhBrush) {
        wSelBr = ASSIGNSEL(lpPhBrush, STK_DATA_256);
        PUTDWORD(lpSP+4, MAKELP(wSelBr,0));
    }
    else
        PUTDWORD(lpSP+4, 0L);

    if (lpDrawMode) {
	PutDRAWMODE(lpStruct, lpDrawMode);
        PUTWORD(lpSP+2, (WORD)envp_global->reg.ss);
        PUTWORD(lpSP  , (WORD)((DWORD)lpStruct - (DWORD)ss));
    }
    else
        PUTDWORD(lpSP, 0L);

    invoke_binary();

    if (wSelDest)
        FreeSelector(wSelDest);
    if (wSelSrc)
        FreeSelector(wSelSrc);
    if (wSelBr)
        FreeSelector(wSelBr);
    if (wSelSrcBits)
        FreeSelector(wSelSrcBits);
    if (wSelSrcBmp)
        FreeSelector(wSelSrcBmp);
    if (wSelDestBits)
        FreeSelector(wSelDestBits);
    if (wSelDestBmp)
        FreeSelector(wSelDestBmp);

    envp_global->reg.sp += STK_DATA_512;
    return (int)(short)(LOWORD(envp_global->reg.ax));
}

int
cnb_StretchBlt(LPPDEVICE lpDestDev, WORD wDestX, WORD wDestY,
		WORD wDestXext, WORD wDestYext,
		LPPDEVICE lpSrcDev, WORD wSrcX, WORD wSrcY,
		WORD wSrcXext, WORD wSrcYext, DWORD dwRop3, 
		LPVOID lpPhBrush, LPDRAWMODE lpDrawMode, LPRECT lpClipRect)
{
    LPBYTE	lpSP, lpStruct, ss, lpData;
    WORD	wSelDest = 0, wSelSrc = 0, wSelBr = 0;
    WORD	wSelSrcBits = 0, wSelSrcBmp = 0;
    WORD	wSelDestBits = 0, wSelDestBmp = 0;
    LPPSBITMAP  lpPSBitmap;
    BYTE	bufSrc[64];
    BYTE	bufDest[64];
    int		nByteCount = 0;

    envp_global->reg.ax = 0;
    envp_global->reg.sp -= STK_DATA_512;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= STK_STRETCHBLT;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = GetPhysicalAddress(envp_global->reg.ss);

    if (lpDestDev) {
	if (lpDestDev->pdType) {		/* device DC */
	    wSelDest = ASSIGNSEL(lpDestDev, DEVICEsize);
	    PUTDWORD(lpSP+36, MAKELP(wSelDest,0));
	}
	else {				/* memory DC */
	    lpPSBitmap = (LPPSBITMAP)lpDestDev;
	    lpData = bufDest;
	    PutPSBITMAP(lpData,lpPSBitmap);
	    if (lpPSBitmap->bm.bmBits) {
		nByteCount = 
			lpPSBitmap->bm.bmWidthBytes*lpPSBitmap->bm.bmHeight;
		wSelDestBits = ASSIGNSEL(lpPSBitmap->bm.bmBits,nByteCount);
		PUTDWORD(lpData+10, MAKELP(wSelDestBits,0));
	    }
	    else
		PUTDWORD(lpData+10,0L);
	    if (nByteCount > 65535)
		FatalAppExit(0,"cnb_StretchBlt: huge dest bitmap");
	    wSelDestBmp = ASSIGNSEL(lpData,32);
	    PUTDWORD(lpSP+36, MAKELP(wSelDestBmp,0));
	}
    }
    else
        PUTDWORD(lpSP+36, 0L);

    PUTWORD(lpSP+34, wDestX);
    PUTWORD(lpSP+32, wDestY);
    PUTWORD(lpSP+30, wDestXext);
    PUTWORD(lpSP+28, wDestYext);

    if (lpSrcDev) {
	if (lpSrcDev->pdType) {		/* device DC */
	    wSelSrc = ASSIGNSEL(lpSrcDev, DEVICEsize);
	    PUTDWORD(lpSP+24, MAKELP(wSelSrc,0));
	}
	else {				/* memory DC */
	    lpPSBitmap = (LPPSBITMAP)lpSrcDev;
	    lpData = bufSrc;
	    PutPSBITMAP(lpData,lpPSBitmap);
	    if (lpPSBitmap->bm.bmBits) {
		nByteCount = 
			lpPSBitmap->bm.bmWidthBytes*lpPSBitmap->bm.bmHeight;
		wSelSrcBits = ASSIGNSEL(lpPSBitmap->bm.bmBits,nByteCount);
		PUTDWORD(lpData+10, MAKELP(wSelSrcBits,0));
	    }
	    else
		PUTDWORD(lpData+10,0L);
	    if (nByteCount > 65535)
		FatalAppExit(0,"cnb_StretchBlt: huge source bitmap");
	    wSelSrcBmp = ASSIGNSEL(lpData,32);
	    PUTDWORD(lpSP+24, MAKELP(wSelSrcBmp,0));
	}
    }
    else
        PUTDWORD(lpSP+24, 0L);

    PUTWORD(lpSP+22, wSrcX);
    PUTWORD(lpSP+20, wSrcY);
    PUTWORD(lpSP+18, wSrcXext);
    PUTWORD(lpSP+16, wSrcYext);
    PUTDWORD(lpSP+12, dwRop3);

    if (lpPhBrush) {
        wSelBr = ASSIGNSEL(lpPhBrush, STK_DATA_256);
        PUTDWORD(lpSP+8, MAKELP(wSelBr,0));
    }
    else
        PUTDWORD(lpSP+8, 0L);

    if (lpDrawMode) {
	PutDRAWMODE(lpStruct, lpDrawMode);
        PUTWORD(lpSP+6, (WORD)envp_global->reg.ss);
        PUTWORD(lpSP+4, (WORD)((DWORD)lpStruct - (DWORD)ss));
	lpStruct += 60;   /* ??? */
    }
    else
        PUTDWORD(lpSP+4, 0L);

    if ( lpClipRect ) {
        PUTWORD(lpStruct,   (WORD)lpClipRect->left);
        PUTWORD(lpStruct+2, (WORD)lpClipRect->top);
        PUTWORD(lpStruct+4, (WORD)lpClipRect->right);
        PUTWORD(lpStruct+6, (WORD)lpClipRect->bottom);
        PUTWORD(lpSP+2, (WORD)envp_global->reg.ss);
        PUTWORD(lpSP  , (WORD)((DWORD)lpStruct - (DWORD)ss));
    }
    else
	PUTDWORD(lpSP, 0L);

    invoke_binary();
    if (wSelDest)
        FreeSelector(wSelDest);
    if (wSelSrc)
        FreeSelector(wSelSrc);
    if (wSelBr)
        FreeSelector(wSelBr);
    if (wSelSrcBits)
        FreeSelector(wSelSrcBits);
    if (wSelSrcBmp)
        FreeSelector(wSelSrcBmp);
    if (wSelDestBits)
        FreeSelector(wSelDestBits);
    if (wSelDestBmp)
        FreeSelector(wSelDestBmp);

    envp_global->reg.sp += STK_DATA_512;
    return (int)(short)(LOWORD(envp_global->reg.ax));
}

COLORREF
cnb_ColorInfo(LPPDEVICE lpDestDev, DWORD dwRGB, LPVOID lpPhColor)
{
    LPBYTE	lpSP,lpData;
    WORD	wSelDest = 0, wSelCol = 0, wSelDestBits = 0, wSelDestBmp = 0;
    BOOL	retcode;
    int		nByteCount = 0;
    BYTE	bufDest[64];
    LPPSBITMAP lpPSBitmap;

    envp_global->reg.ax = 0;
    envp_global->reg.sp -= STK_COLORINFO;
    lpSP = (LPBYTE)envp_global->reg.sp;

    if (lpDestDev) {
	if (lpDestDev->pdType) {		/* device DC */
	    wSelDest = ASSIGNSEL(lpDestDev, DEVICEsize);
	    PUTDWORD(lpSP+8, MAKELP(wSelDest,0));
	}
	else {				/* memory DC */
	    lpPSBitmap = (LPPSBITMAP)lpDestDev;
	    lpData = bufDest;
	    PutPSBITMAP(lpData,lpPSBitmap);
	    if (lpPSBitmap->bm.bmBits) {
		nByteCount = 
			lpPSBitmap->bm.bmWidthBytes*lpPSBitmap->bm.bmHeight;
		wSelDestBits = ASSIGNSEL(lpPSBitmap->bm.bmBits,nByteCount);
		PUTDWORD(lpData+10, MAKELP(wSelDestBits,0));
	    }
	    else
		PUTDWORD(lpData+10,0L);
	    if (nByteCount > 65535)
		FatalAppExit(0,"cnb_ColorInfo: huge dest bitmap");
	    wSelDestBmp = ASSIGNSEL(lpData,32);
	    PUTDWORD(lpSP+8, MAKELP(wSelDestBmp,0));
	}
    }
    else
        PUTDWORD(lpSP+8, 0L);

    PUTDWORD(lpSP+4, dwRGB);

    if (lpPhColor) {
        wSelCol = ASSIGNSEL(lpPhColor, STK_DATA_256);
        PUTDWORD(lpSP, MAKELP(wSelCol,0));
    }
    else
        PUTDWORD(lpSP, 0L);

    invoke_binary();

    if (wSelDest)
        FreeSelector(wSelDest);
    if (wSelCol)
        FreeSelector(wSelCol);
    if (wSelDestBits)
        FreeSelector(wSelDestBits);
    if (wSelDestBmp)
        FreeSelector(wSelDestBmp);

    retcode = (COLORREF)MAKELONG(envp_global->reg.ax,envp_global->reg.dx);

    return retcode;
}

COLORREF
cnb_Pixel(LPPDEVICE lpDestDev, int X, int Y, COLORREF dwPhysColor,
	LPDRAWMODE lpDrawMode)
{
    LPBYTE	lpSP,lpData,lpStruct,ss;
    WORD	wSelDest = 0, wSelDestBits = 0, wSelDestBmp = 0;
    BOOL	retcode;
    int		nByteCount = 0;
    BYTE	bufDest[64];
    LPPSBITMAP lpPSBitmap;

    envp_global->reg.sp -= STK_DATA_256;
    lpStruct = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.ax = 0;
    envp_global->reg.sp -= STK_PIXEL;
    lpSP = (LPBYTE)envp_global->reg.sp;
    ss = (LPBYTE)GetPhysicalAddress(envp_global->reg.ss);

    if (lpDestDev) {
	if (lpDestDev->pdType) {		/* device DC */
	    wSelDest = ASSIGNSEL(lpDestDev, DEVICEsize);
	    PUTDWORD(lpSP+12, MAKELP(wSelDest,0));
	}
	else {				/* memory DC */
	    lpPSBitmap = (LPPSBITMAP)lpDestDev;
	    lpData = bufDest;
	    PutPSBITMAP(lpData,lpPSBitmap);
	    if (lpPSBitmap->bm.bmBits) {
		nByteCount = 
			lpPSBitmap->bm.bmWidthBytes*lpPSBitmap->bm.bmHeight;
		wSelDestBits = ASSIGNSEL(lpPSBitmap->bm.bmBits,nByteCount);
		PUTDWORD(lpData+10, MAKELP(wSelDestBits,0));
	    }
	    else
		PUTDWORD(lpData+10,0L);
	    if (nByteCount > 65535)
		FatalAppExit(0,"cnb_ColorInfo: huge dest bitmap");
	    wSelDestBmp = ASSIGNSEL(lpData,32);
	    PUTDWORD(lpSP+12, MAKELP(wSelDestBmp,0));
	}
    }
    else
        PUTDWORD(lpSP+12, 0L);

    PUTWORD(lpSP+10, (WORD)X);
    PUTWORD(lpSP+8, (WORD)Y);
    PUTDWORD(lpSP+4, dwPhysColor);

    invoke_binary();

    /* convert the DRAWMODE struct */
    if (lpDrawMode) {
	PutDRAWMODE(lpStruct,lpDrawMode);
	PUTWORD(lpSP+2,(WORD)envp_global->reg.ss);
	PUTWORD(lpSP,(WORD)((DWORD)lpStruct - (DWORD)ss));
    }
    else
	PUTDWORD(lpSP,0L);

    if (wSelDest)
        FreeSelector(wSelDest);
    if (wSelDestBits)
        FreeSelector(wSelDestBits);
    if (wSelDestBmp)
        FreeSelector(wSelDestBmp);

    retcode = (COLORREF)MAKELONG(envp_global->reg.ax,envp_global->reg.dx);

    envp_global->reg.sp += STK_DATA_256;

    return retcode;
}

/*********************************************************************/

BOOL
cnb_CTL3DV2_Ctl3dRegister(HINSTANCE hInstance)
{
    envp_global->reg.sp -= HANDLE_86;
    PUTWORD(envp_global->reg.sp,GetDataSelectorFromInstance(hInstance));
    invoke_binary();
    return (BOOL)envp_global->reg.ax;
}

BOOL
cnb_CTL3DV2_Ctl3dSubclassCtl(HWND hWnd)
{
    envp_global->reg.sp -= HANDLE_86;
    PUTWORD(envp_global->reg.sp,hWnd);
    invoke_binary();
    return (BOOL)envp_global->reg.ax;
}

BOOL
cnb_CTL3DV2_Ctl3dSubclassDlg(HWND hWnd, WORD wFlags)
{
    envp_global->reg.sp -= HANDLE_86 + WORD_86;
    PUTWORD(envp_global->reg.sp,wFlags);
    PUTWORD(envp_global->reg.sp+2,hWnd);
    invoke_binary();
    return (BOOL)envp_global->reg.ax;
}

BOOL
cnb_CTL3DV2_Ctl3dSubclassDlgEx(HWND hWnd, DWORD dwFlags)
{
    envp_global->reg.sp -= HANDLE_86 + DWORD_86;
    PUTDWORD(envp_global->reg.sp,dwFlags);
    PUTWORD(envp_global->reg.sp+4,hWnd);
    invoke_binary();
    return (BOOL)envp_global->reg.ax;
}

HBRUSH
cnb_CTL3DV2_Ctl3dCtlColor(HDC hDC, LONG l)
{
    envp_global->reg.sp -= HANDLE_86 + DWORD_86;
    PUTDWORD(envp_global->reg.sp,l);
    PUTWORD(envp_global->reg.sp+4,hDC);
    invoke_binary();
    return (HBRUSH)envp_global->reg.ax;
}

HBRUSH
cnb_CTL3DV2_Ctl3dCtlColorEx(UINT wm, WPARAM wParam, LPARAM lParam)
{
    envp_global->reg.sp -= UINT_86 + WORD_86 + DWORD_86;
    PUTDWORD(envp_global->reg.sp,lParam);
    PUTWORD(envp_global->reg.sp+4,wParam);
    PUTWORD(envp_global->reg.sp+6,wm);
    invoke_binary();
    return (HBRUSH)envp_global->reg.ax;
}

LONG
cnb_CTL3DV2_Ctl3dDlgFramePaint(HWND hWnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    return hsw_common_nat_to_bin(hWnd, wm, wParam, lParam);
}

BOOL
cnb_CTL3DV2_Ctl3dColorChange(void)
{
    invoke_binary();
    return (BOOL)envp_global->reg.ax;
}

void
cnb_CTL3DV2_Stub(void)
{
    MessageBox((HWND)NULL,"cnb_CTL3DV2_Stub called...",
		"Unimplemented interface",MB_OK|MB_ICONEXCLAMATION);
    FatalExit(0);
}
