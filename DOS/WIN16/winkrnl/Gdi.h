/*
	@(#)Gdi.h	2.4
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
 
#ifndef Gdi__h
#define Gdi__h

typedef struct tagPDEVICE {
        short pdType;           /* Device type. 0, if bitmap */
} PDEVICE;
typedef PDEVICE FAR *LPPDEVICE;


/* PBITMAP is used in printer-compatible DC */
typedef struct {
        short           bmType;
        short           bmWidth;
        short           bmHeight;
        short           bmWidthBytes;
        BYTE            bmPlanes;
        BYTE            bmBitsPixel;
        LPBYTE          bmBits;
        LONG            bmWidthPlanes;
        LPPDEVICE       bmlpPDevice;
        short           bmSegmentIndex;
        short           bmScanSegment;
        short           bmFillBytes;
        short           futureUse1;
        short           futureUse2;
} DCBITMAP;
typedef DCBITMAP *LPDCBITMAP;

/*--------------------------------------------------------------*/

typedef struct tagPRINTEREXTRA {
        LPPDEVICE	pDevice;	/* Device dependent structure      */
        LPVOID          lpPhPen;	/* Driver specific pen structure   */
        LPVOID          lpPhBrush;	/* Driver specific brush structure */
        LPVOID		lpPhFont;	/* Driver specific font structure  */
					/* pointer to FONTINFO structure   */
	ABORTPROC	lpfnAbortProc;
	TEXTXFORM	txf;
        BOOL            IsIC;
	BOOL		bDisableResetDC;
        GDIINFO         GdiInfo;
        DRAWMODE        DrawMode;
        char            szDrvName[9];	/* Driver filename w/o extension   */
	char		szDevice[CCHDEVICENAME];
	char		szPort[1];
} PRINTEREXTRA;


typedef struct tagINTERNENVTBL {
	LPVOID	lpEnv;
	int     iEnvLen;
	char	szPort[128];
} INTERNENVTBL;

typedef INTERNENVTBL *LPINTERNENVTBL;

/* Get pointer to PRINTEREXTRA structure */
#define	GETPRINTEREXTRA(dc)	(PRINTEREXTRA*)dc->PrivateHook

int InternalEnvironment(int, LPCSTR, LPINTERNENVTBL *, int);
WORD GetTextLength(PRINTEREXTRA *, LPSTR, TEXTXFORM *);
FARPROC GetFuncAddress(LPCSTR, LPCSTR);
BOOL GDIRealizeObject(LPCSTR, LPPDEVICE, int, LPVOID, LPVOID*, LPTEXTXFORM);

#endif /* Gdi__h */
