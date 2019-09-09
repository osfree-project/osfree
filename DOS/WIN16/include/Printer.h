/*  Printer.h	2.5
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
 
#ifndef Printer__h
#define Printer__h

#include "print.h"

#define	DEVMODE_SIZE	68
#define PFM_CBLEADER	66	/* The length of Copyright string + ... */
				/* in binary PFM file                   */

/* Commands to InternalEnvironment */
#define	GET_ENV		1
#define	SET_ENV		2
#define	DELETE_ENV	3

/* The output styles */
#define	OS_ARC		3
#define	OS_SCANLINES	4
#define	OS_RECTANGLE	6
#define	OS_ELLIPSE	7
#define	OS_MARKER	8
#define	OS_POLYLINE 	18
#define	OS_ALTPOLYGON	22
#define	OS_WINDPOLYGON	20
#define	OS_PIE		23
#define	OS_POLYMARKER	24
#define	OS_CHORD	39
#define	OS_CIRCLE	55
#define	OS_ROUNDRECT	72

#define	RESETDC		128	/* Undocumented escape */


typedef struct {
    short int dpVersion;
    short int dpTechnology;
    short int dpHorzSize;
    short int dpVertSize;
    short int dpHorzRes;
    short int dpVertRes;
    short int dpBitsPixel;
    short int dpPlanes;
    short int dpNumBrushes;
    short int dpNumPens;
    short int futureuse;
    short int dpNumFonts;
    short int dpNumColors;
    short int dpDEVICEsize;
    unsigned short int	dpCurves;
    unsigned short int	dpLines;
    unsigned short int	dpPolygonals;
    unsigned short int	dpText;
    unsigned short int	dpClip;
    unsigned short int	dpRaster;
    short int dpAspectX;
    short int dpAspectY;
    short int dpAspectXY;
    short int dpStyleLen;
    POINT    dpMLoWin;
    POINT    dpMLoVpt;
    POINT    dpMHiWin;
    POINT    dpMHiVpt;
    POINT    dpELoWin;
    POINT    dpELoVpt;
    POINT    dpEHiWin;
    POINT    dpEHiVpt;
    POINT    dpTwpWin;
    POINT    dpTwpVpt;
    short int dpLogPixelsX;
    short int dpLogPixelsY;
    short int dpDCManage;
    short int dpCaps1;
    short int futureuse4;
    short int futureuse5;
    short int futureuse6;
    short int futureuse7;
    short     new1, new2, new3;		/* for version 0x300 devices */
} GDIINFO;


typedef struct {
    short int	      Rop2;
    short int	      bkMode;
    unsigned long int bkColor;
    unsigned long int TextColor;
    short int	      TBreakExtra;
    short int	      BreakExtra;
    short int	      BreakErr;
    short int	      BreakRem;
    short int	      BreakCount;
    short int	      CharExtra;
    unsigned long int LbkColor;
    unsigned long int LTextColor;
} DRAWMODE;
typedef DRAWMODE FAR *LPDRAWMODE;


typedef struct {
    short int dfType;
    short int dfPoints;
    short int dfVertRes;
    short int dfHorizRes;
    short int dfAscent;
    short int dfInternalLeading;
    short int dfExternalLeading;
    BYTE dfItalic;
    BYTE dfUnderline;
    BYTE dfStrikeOut;
    short int	dfWeight;
    BYTE dfCharSet;
    short int dfPixWidth;
    short int dfPixHeight;
    BYTE dfPitchAndFamily;
    short int dfAvgWidth;
    short int dfMaxWidth;
    BYTE dfFirstChar;
    BYTE dfLastChar;
    BYTE dfDefaultChar;
    BYTE dfBreakChar;
    short int	dfWidthBytes;
    unsigned long int	dfDevice;
    unsigned long int	dfFace;
    unsigned long int	dfBitsPointer;
    unsigned long int	dfBitsOffset;
    WORD dfSizeFields;
    DWORD dfExtMetricsOffset;
    DWORD dfExtentTable;
    DWORD dfOriginTable;
    DWORD dfPairKernTable;
    DWORD dfTrackKernTable;
    DWORD dfDriverInfo;
    DWORD dfReserved;
} DFONTINFO;
typedef DFONTINFO FAR *LPDFONTINFO;

typedef struct {
    short int		txfHeight;
    short int		txfWidth;
    short int		txfEscapement;
    short int		txfOrientation;
    short int		txfWeight;
    BYTE		txfItalic;
    BYTE		txfUnderline;
    BYTE		txfStrikeOut;
    BYTE		txfOutPrecision;
    BYTE		txfClipPrecision;
    unsigned short int	txfAccelerator;
    short int		txfOverhang;
} TEXTXFORM;
typedef TEXTXFORM *LPTEXTXFORM;


/*--------------------------------------------------------------*/
/*	One of the cases when GDI uses printer's private	*/
/*	structure. (ENABLEPAIRKERNING/SETKERNTRACK escapes).	*/
/*--------------------------------------------------------------*/
typedef struct {
        short   x;
        short   y;
        int     count;
        RECT    ClipRect;
        LPSTR   lpStr;
        short   far *lpWidths;
} APPEXTTEXTDATA;
typedef APPEXTTEXTDATA FAR *LPAPPEXTTEXTDATA;


typedef struct {
        short                   nSize;
        LPAPPEXTTEXTDATA        lpInData;
        LPDFONTINFO             lpFont;	    /* ptr to RFONT structure */
        LPTEXTXFORM             lpXForm;
        LPDRAWMODE              lpDrawMode;
} EXTTEXTDATA;
typedef EXTTEXTDATA FAR *LPEXTTEXTDATA;

typedef struct {
    short etmSize;
    short etmPointSize;
    short etmOrientation;
    short etmMasterHeight;
    short etmMinScale;
    short etmMaxScale;
    short etmMasterUnits;
    short etmCapHeight;
    short etmXHeight;
    short etmLowerCaseAscent;
    short etmLowerCaseDescent;	/* use to be: short etmUpperCaseDecent; */
    short etmSlant;
    short etmSuperScript;
    short etmSubScript;
    short etmSuperScriptSize;
    short etmSubScriptSize;
    short etmUnderlineOffset;
    short etmUnderlineWidth;
    short etmDoubleUpperUnderlineOffset;
    short etmDoubleLowerUnderlineOffset;
    short etmDoubleUpperUnderlineWidth;
    short etmDoubleLowerUnderlineWidth;
    short etmStrikeOutOffset;
    short etmStrikeOutWidth;
    WORD etmNKernPairs;
    WORD etmNKernTracks;
} EXTTEXTMETRIC;
typedef EXTTEXTMETRIC	ETM;


HDC OnResetDC(HDC, const DEVMODE *);
int OnSetAbortProc(HDC, ABORTPROC);
int OnAbortDoc(HDC);
int OnStartDoc(HDC, DOCINFO*);
int OnEndDoc(HDC);


#endif /* Printer__h */
