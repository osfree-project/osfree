/********************************************************************
	@(#)PrinterBin.h	2.6
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

********************************************************************/
 
#ifndef PrinterBin__h
#define PrinterBin__h



#include "print.h"
#include "GdiDDK.h"


#define	SIZEOF_DFONTINFO_BIN		81
#define	SIZEOF_ETM_BIN			52
#define	SIZEOF_EXTTEXTDATA_BIN		18
#define	SIZEOF_APPEXTTEXTDATA_BIN	20
#define	SIZEOF_TEXTXFORM_BIN		19
#define	SIZEOF_DRAWMODE_BIN		32
#define	SIZEOF_BITMAPINFOHEADER_BIN	40
#define	SIZEOF_RGBQUAD_BIN		4


#define	ALIGN4(len)		((sizeof(int) - (len % sizeof(int))) & 0x03)


/* In this sturcture GDI keeps the font realized by printer driver */

typedef struct {
        DFONTINFO       df;		/* The part accessible by GDI.       */
        int             iBinSize;	/* Size of binary part.              */
        BYTE            byBinPart[1];	/* Binary part (ETM, KT, KP,         */
					/* ExtTable, FaceName, DeviceName,...*/
} RFONT;


void   GetDEVMODE(DEVMODE *, LPBYTE);
void   PutDEVMODE(LPBYTE, DEVMODE *);

int    GetRFONT(RFONT *, LPBYTE, int);
void   PutRFONT(LPBYTE, RFONT *);

void   GetGDIINFO(GDIINFO *,LPBYTE);

void   GetTEXTXFORM(TEXTXFORM *, LPBYTE);
void   PutTEXTXFORM(LPBYTE,TEXTXFORM *);

void   PutDRAWMODE(LPBYTE,DRAWMODE *);
WORD   PutBITMAP(LPBYTE, LPBITMAP);
LPBYTE PutBITMAPINFO(LPBITMAPINFO);
void   GetBITMAPINFO(BITMAPINFO *, LPBYTE);

void   GetDFONTINFO(DFONTINFO *, LPBYTE);
int    PutDFONTINFO(LPBYTE, DFONTINFO *);

void   GetETM(EXTTEXTMETRIC *, LPBYTE);
void   PutETM(LPBYTE, EXTTEXTMETRIC *, int);

int    PutEXTTEXTDATA(LPBYTE, WORD, DWORD, int cbRFsize);

void PutPSBITMAP(LPBYTE, LPPSBITMAP);

#endif /* PrinterBin__h */
