/*    
	PrintIF.c	1.5
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
#include <string.h>

#include "windows.h"
#include "print.h"

#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "BinTypes.h"
#include "PrinterBin.h"
#include "DPMI.h"
#include "dos.h"

/* exported routines */
void IT_DEVICEMODE(ENV *,LONGPROC);
void IT_EXTDEVICEMODE(ENV *,LONGPROC);
void IT_DEVICECAPS(ENV *,LONGPROC);
void IT_ESCAPE(ENV *,LONGPROC);
void IT_GETDRVINFO(ENV *,LONGPROC);	/* GetDriverInfo */
void IT_RESETDC(ENV *,LONGPROC);
void IT_CREATEDC(ENV *,LONGPROC);	/* CreateDC, CreateIC */

/* imported routines */
extern DWORD make_native_thunk(DWORD, DWORD);
extern void GetDOCINFO(DOCINFO *, LPBYTE);

void
IT_DEVICEMODE (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	HWND hWnd;
	HANDLE hInstance;
	LPSTR lpDestDevType;
	LPSTR lpOutputFile;

	lpOutputFile = (LPSTR)GetAddress(GETWORD(SP+6), GETWORD(SP+4));
	lpDestDevType = (LPSTR)GetAddress(GETWORD(SP+10), GETWORD(SP+8));
	hInstance = (HANDLE)GetSelectorHandle(GETWORD(SP+12));
	hWnd = (HWND)GETWORD(SP+14);
	retcode = (f)(hWnd, hInstance, lpDestDevType, lpOutputFile);

	envp->reg.sp += HANDLE_86 + HANDLE_86 + LP_86 + LP_86+ RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_EXTDEVICEMODE (ENV *envp,LONGPROC f)
{
	static int	iExtSize;
	DWORD		retcode;
	DEVMODE		*lpdmOutput = NULL;
	DEVMODE		*lpdmInput = NULL;
	LPBYTE		lpStructIn, lpStructOut = 0;
	WORD		wMode;

	wMode = GETWORD(SP+4);
	if (wMode) {
	    lpStructIn = (LPBYTE)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	    if (lpStructIn) {
		lpdmInput = (DEVMODE *)WinMalloc(sizeof(DEVMODE) +
			GETWORD(lpStructIn+CCHDEVICENAME+6));
		GetDEVMODE(lpdmInput,lpStructIn);
	    }
	    lpStructOut = (LPBYTE)GetAddress(GETWORD(SP+24),GETWORD(SP+22));
	    if (lpStructOut) {
		lpdmOutput = (DEVMODE*)WinMalloc(sizeof(DEVMODE)+iExtSize);
		GetDEVMODE(lpdmOutput,lpStructOut);
	    }
	}
 	retcode = (DWORD)(f)((HWND)GETWORD(SP+28),
			(HANDLE)GETWORD(SP+26),
			lpdmOutput,
			(LPSTR)GetAddress(GETWORD(SP+20),GETWORD(SP+18)),
			(LPSTR)GetAddress(GETWORD(SP+16),GETWORD(SP+14)),
			lpdmInput,
			(LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6)),
			wMode);

	if (lpdmInput)
		WinFree((LPSTR)lpdmInput);
	if (lpdmOutput) {
		PutDEVMODE(lpStructOut,lpdmOutput);
		WinFree((LPSTR)lpdmOutput);
	}

	if ( !wMode ) { /* returns the size of buffer for DEVMODE struct */
		/* The 1st call */
		iExtSize = retcode - sizeof(DEVMODE);
		retcode = CCHDEVICENAME + 4*UINT_86 + DWORD_86 + 12*INT_86 +
			iExtSize;
	}

	envp->reg.sp += WORD_86 + 5*LP_86 + 2*HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_DEVICECAPS (ENV *envp,LONGPROC f)
{
	DWORD		retcode;
	LPSTR		lpDevice;
	LPSTR		lpPort;
	WORD		nIndex;
	LPSTR		lpOutput;
	LPBYTE		lpdmBin;
	LPDEVMODE	lpdm = NULL;
	LPPOINT		lpp;
	int		i;
	char		tmp_output[512];

	lpdmBin = (LPBYTE)GetAddress(GETWORD(SP+6), GETWORD(SP+4));
	if ( lpdmBin ) {
		/* sizeof(DEVMODE) + dmDriverExtra */
		lpdm = (LPDEVMODE)WinMalloc(sizeof(DEVMODE) + 
					GETWORD(lpdmBin+CCHDEVICENAME+6));
		GetDEVMODE(lpdm,lpdmBin);
	}
	lpOutput = (LPSTR)GetAddress(GETWORD(SP+10), GETWORD(SP+8));
	nIndex = GETWORD(SP+12);
	lpPort = (LPSTR)GetAddress(GETWORD(SP+16), GETWORD(SP+14));
	lpDevice = (LPSTR)GetAddress(GETWORD(SP+20), GETWORD(SP+18));

	retcode = (f)(lpDevice, lpPort, nIndex,
				(lpOutput) ? tmp_output : (LPSTR)NULL, lpdm);

	envp->reg.sp += LP_86 * 2 + WORD_86 + LP_86 * 2 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);

	if ( !lpOutput )
		return;

	/* Fixup and copy output buffer to *lpOutput. */
	switch(nIndex) {

	case DC_PAPERS:   	/* return value is # supported paper */
		for ( i = 0; i < retcode; i++ )
			PUTWORD(lpOutput + i, *(tmp_output + i));
		break;

	case DC_PAPERSIZE:	/* sizes, and (maybe) list of papers */
				/* or sizes in 10ths of a mm	     */
		lpp = (LPPOINT)tmp_output;
		/* retcode = number of points */
		for ( i = 0; i < retcode; i++ ) {
			lpOutput = (LPSTR)((LPPOINT)lpOutput + i);
			PUTWORD(lpOutput,     lpp->x);
			PUTWORD(lpOutput + 2, lpp->y);
			lpp++;
		}
		break;

	case DC_PAPERNAMES:	/*  return array of papername strings  */
		break;

	case DC_ORIENTATION:
		break;		/* DC_ORIENTATION doesn't require conversion */
	
	default:
		ERRSTR((LF_ERROR,
			"DeviceCapabilities: Conversion required!!!\n"));
		break;
	}
}

void
IT_RESETDC (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	DEVMODE *lpdm = (DEVMODE *)NULL;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
	    lpdm = (DEVMODE *)WinMalloc(sizeof(DEVMODE) + 
			(GETWORD(lpStruct+CCHDEVICENAME+6)));
	    GetDEVMODE(lpdm,lpStruct);
	}
	retcode = (f)((HANDLE)GETWORD(SP+8),lpdm);
	if (lpdm)
	    WinFree((LPSTR)lpdm);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETDRVINFO (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	DRIVERINFOSTRUCT dr;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((HANDLE)GETWORD(SP+8),&dr);
	PUTWORD(lpStruct,(UINT)dr.length);
	PUTWORD(lpStruct+2,(HDRVR)dr.hDriver);
	PUTWORD(lpStruct+4,(HANDLE)dr.hModule);
	memcpy((LPSTR)lpStruct+6,(LPSTR)&dr.szAliasName,128);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEDC (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	DEVMODE *lpdm = NULL;
	LPSTR lpString1,lpString2,lpString3;
	char FileName[_MAX_PATH];

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
	    lpdm = (DEVMODE *)WinMalloc(sizeof(DEVMODE) + 
			(GETWORD(lpStruct+CCHDEVICENAME+6)));
	    GetDEVMODE(lpdm,lpStruct);
	}
	lpString1 = (LPSTR)GetAddress(GETWORD(SP+18),GETWORD(SP+16));
	if (lpString1 && 
		strcasecmp(lpString1,"DISPLAY")) { /* For now -- special case */
    	    xdoscall(XDOS_GETALTNAME,0,(void *) FileName,(void *) lpString1);
	    strcat(FileName,".drv");
	}
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	lpString3 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
#ifdef LATER
	This also can be a DOS filename or device
#endif
	retcode = (f)(lpString1,
			lpString2,
			lpString3,
			lpdm);
	if (lpdm)
	    WinFree((LPSTR)lpdm);
	envp->reg.sp += 4*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}
	
void
IT_ESCAPE (ENV *envp,LONGPROC f)
{
	DWORD retcode = 0;
	LPBYTE lpStruct;
	LPSTR lpString;
	int nEscape,int1,int2;
	WORD word1;
	DWORD dword1;
	POINT pt;
	RECT rcRect1,rcRect2;
	EXTTEXTMETRIC etm;
	DOCINFO di;

	nEscape = (int)((short)GETWORD(SP+14));
	switch (nEscape) {
		case SETABORTPROC:
			retcode = 1;
			break;
		case ABORTDOC:
		case BEGIN_PATH:
		case ENDDOC:
		case FLUSHOUTPUT:
		case NEWFRAME:
		case RESTORE_CTM:
		case SAVE_CTM:
			retcode = Escape((HANDLE)GETWORD(SP+16),
				nEscape,(int)NULL,(LPCSTR)NULL,(LPVOID)NULL);
			break;
		case CLIP_TO_PATH:
		case DRAFTMODE:
		case EPSPRINTING:
		case SETCHARSET:
			lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
			int1 = (int)((short)GETWORD(lpStruct));
			retcode = Escape((HANDLE)GETWORD(SP+16),
				nEscape,sizeof(int),(LPCSTR)&int1,(LPVOID)NULL);
			break;
		case ENABLEDUPLEX:
			lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
			word1 = GETWORD(lpStruct);
			retcode = Escape((HANDLE)GETWORD(SP+16),
				nEscape,sizeof(WORD),(LPCSTR)&word1,(LPVOID)NULL);
			break;
		case ENABLEPAIRKERNING:
		case ENABLERELATIVEWIDTHS:
		case SETLINECAP:
		case SETCOPYCOUNT:
			lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
			int1 = (int)((short)GETWORD(lpStruct));
			lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+6),GETWORD(SP+4));
			if (lpStruct) {
				retcode = Escape((HANDLE)GETWORD(SP+16),
			    	nEscape,sizeof(int),(LPCSTR)&int1,(LPVOID)&int2);
				PUTWORD(lpStruct,(WORD)int2);
			}
			else
				retcode = Escape((HANDLE)GETWORD(SP+16),
			    	nEscape,sizeof(int),(LPCSTR)&int1,(LPVOID)NULL);
			break;
		case EXT_DEVICE_CAPS:
		case GETCOLORTABLE:
			lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
			int1 = (int)((short)GETWORD(lpStruct));
			lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+6),GETWORD(SP+4));
			if (lpStruct) {
				retcode = Escape((HANDLE)GETWORD(SP+16),
			    	nEscape,sizeof(int),(LPCSTR)&int1,(LPVOID)&dword1);
				PUTWORD(lpStruct,dword1);
			}
			else
				retcode = Escape((HANDLE)GETWORD(SP+16),
			    	nEscape,sizeof(int),(LPCSTR)&int1,(LPVOID)NULL);
			break;
		case GETFACENAME:
		case GETTECHNOLOGY:
			lpString = (LPSTR)GetAddress
				(GETWORD(SP+6),GETWORD(SP+4));
			retcode = Escape((HANDLE)GETWORD(SP+16),
			nEscape,(int)NULL,(LPCSTR)NULL,(LPVOID)lpString);
			break;
		case GETPHYSPAGESIZE:
		case GETPRINTINGOFFSET:
		case GETSCALINGFACTOR:
			lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+6),GETWORD(SP+4));
			if (lpStruct) {
				retcode = Escape((HANDLE)GETWORD(SP+16),
					nEscape,(int)NULL,(LPCSTR)NULL,(LPVOID)&pt);
				PutPOINT(lpStruct,pt);
			}
			else
				retcode = Escape((HANDLE)GETWORD(SP+16),
					nEscape,(int)NULL,(LPCSTR)NULL,(LPVOID)NULL);
			break;
		case GETSETPAPERMETRICS:
			lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+10),GETWORD(SP+8));
			RECT_TO_C(rcRect1,lpStruct);
			lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+6),GETWORD(SP+4));
			if (lpStruct) {
				retcode = Escape((HANDLE)GETWORD(SP+16),
					nEscape,sizeof(RECT),(LPCSTR)&rcRect1,
					(LPVOID)&rcRect2);
				RECT_TO_86(lpStruct,rcRect2);
			}
			else
				retcode = Escape((HANDLE)GETWORD(SP+16),
					nEscape,sizeof(RECT),(LPCSTR)&rcRect1,
					(LPVOID)NULL);
			break;
		case GETEXTENDEDTEXTMETRICS:
			word1 = sizeof(EXTTEXTMETRIC);
			retcode = Escape((HANDLE)GETWORD(SP+16),
					nEscape,sizeof(WORD),
					(LPCSTR)&word1,(LPVOID)&etm);
			lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+10),GETWORD(SP+8));
			word1 = GETWORD(lpStruct);
			lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+6),GETWORD(SP+4));
			if (lpStruct && word1)
			    PutETM(lpStruct,&etm,(int)word1);
			break;
		case SET_BOUNDS:
		case SET_CLIP_BOX:
			lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+10),GETWORD(SP+8));
			if (lpStruct) {
				RECT_TO_C(rcRect1,lpStruct);
				retcode = Escape((HANDLE)GETWORD(SP+16),
					nEscape,sizeof(RECT),(LPCSTR)&rcRect1,
					(LPVOID)NULL);
			}
			else
				retcode = Escape((HANDLE)GETWORD(SP+16),
					nEscape,sizeof(RECT),(LPCSTR)NULL,
					(LPVOID)NULL);
			break;
		case MFCOMMENT:
			lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),
					GETWORD(SP+8));
			int1 = GETWORD(SP+12);
			retcode = Escape((HDC)GETWORD(SP+16), nEscape, int1,
					(LPCSTR)lpStruct, (LPVOID)NULL);
			break;
		case NEXTBAND:
			lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),
							GETWORD(SP+4));
			if ( lpStruct )
				retcode = Escape((HDC)GETWORD(SP+16), nEscape,
					0, (LPSTR)NULL, (LPVOID)&rcRect2);
				RECT_TO_86(lpStruct,rcRect2);
			break;
#ifdef NOTYET
		case BANDINFO:
			break;
		case ENUMPAPERBINS:
		case ENUMPAPERMETRICS:
			break;
		case DEVICEDATA:
		case PASSTHROUGH:
		case POSTSCRIPT_DATA:
			break;
		case DRAWPATTERNRECT:
			break;
		case END_PATH:
			break;
		case EXTTEXTOUT:
			break;
		case GETEXTENTTABLE:
			break;
		case GETPAIRKERNTABLE:
			break;
		case GETSETPAPERBINS:
			break;
		case GETSETPRINTORIENT:
			break;
		case GETSETSCREENPARAMS:
			break;
		case GETTRACKKERNTABLE:
			break;
		case GETVECTORBRUSHSIZE:
			break;
		case GETVECTORPENSIZE:
			break;
		case MOUSETRAILS:
			break;
		case POSTSCRIPT_IGNORE:
			break;
#endif
		case QUERYESCSUPPORT:
			lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
			int1 = (int)((short)GETWORD(lpStruct));
			retcode = Escape((HANDLE)GETWORD(SP+16),
				nEscape,sizeof(int),(LPCSTR)&int1,NULL);
			break;
#ifdef NOTYET
		case SETALLJUSTVALUES:
			break;
		case SET_ARC_DIRECTION:
			break;
		case SET_BACKGROUND_COLOR:
			break;
		case SETCOLORTABLE:
			break;
		case SETKERNINGTRACK:
			break;
		case SETLINEJOIN:
		case SETMITERLIMIT:
			break;
		case SET_POLY_MODE:
			break;
		case SET_SCREEN_ANGLE:
		case SET_SPREAD:
			break;
		case TRANSFORM_CTM:
			break;
#endif
		case STARTDOC:
		    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
		    if (lpStruct)
			GetDOCINFO(&di, lpStruct);
		    retcode = Escape((HDC)GETWORD(SP+16),
				nEscape,(int)GETWORD(SP+12),
			    (LPCSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8)),
				(lpStruct)?(LPVOID)&di:(LPVOID)NULL);
		    break;
		default:
			printf("Unimplemented ESCAPE: %d.\n", nEscape);
			break;
	}
	envp->reg.sp += HANDLE_86 + 2*INT_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SETABORTPROC(ENV *envp, LONGPROC f)
{
	DWORD	dwProc, retcode = 0;

	dwProc = GETDWORD(SP + 6);
	if ( dwProc ) {
		dwProc = (DWORD)make_native_thunk(dwProc, (DWORD)hsw_abortproc);
		retcode = SetAbortProc((HDC)GETWORD(SP + 8), (ABORTPROC)dwProc);
	}
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_STARTDOC(ENV *envp, LONGPROC f)
{
	LPBYTE	lpdiBin;
	DWORD	retcode = (DWORD)SP_ERROR;
	DOCINFO	di;

	lpdiBin = (LPBYTE)GetAddress(GETWORD(SP + 6), GETWORD(SP + 4));
	if ( lpdiBin ) {
		GetDOCINFO(&di, lpdiBin);
		retcode = StartDoc((HDC)GETWORD(SP + 8), &di);
	}
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

