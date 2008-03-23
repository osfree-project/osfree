/*    
	PrinterBin.c	2.13
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
#include <string.h>

#include "windows.h"

#include "kerndef.h"
#include "BinTypes.h"
#include "Endian.h"
#include "DPMI.h"
#include "PrinterBin.h"
#include "PrinterDC.h"
#include "Log.h"

#ifndef NOAPIPROTO
DWORD dmPixel(LPVOID, int, int, DWORD, LPDRAWMODE);
LPINT  hsw_ConvertArrayToInt(LPBYTE, int);
#endif /* !NOAPIPROTO */

void
GetDEVMODE(DEVMODE *lpdm, LPBYTE lpStruct)
{
	memcpy(lpdm->dmDeviceName, (LPSTR)lpStruct, CCHDEVICENAME);
	lpStruct += CCHDEVICENAME;
	lpdm->dmSpecVersion = (UINT)GETWORD(lpStruct);
	lpdm->dmDriverVersion = (UINT)GETWORD(lpStruct+2);
	lpdm->dmSize = sizeof(DEVMODE);
	lpdm->dmDriverExtra = (UINT)GETWORD(lpStruct+6);
	lpdm->dmFields = (DWORD)GETDWORD(lpStruct+8);
	lpdm->dmOrientation = (int)((short)GETWORD(lpStruct+12));
	lpdm->dmPaperSize = (int)((short)GETWORD(lpStruct+14));
	lpdm->dmPaperLength = (int)((short)GETWORD(lpStruct+16));
	lpdm->dmPaperWidth = (int)((short)GETWORD(lpStruct+18));
	lpdm->dmScale = (int)((short)GETWORD(lpStruct+20));
	lpdm->dmCopies = (int)((short)GETWORD(lpStruct+22));
	lpdm->dmDefaultSource = (int)((short)GETWORD(lpStruct+24));
	lpdm->dmPrintQuality = (int)((short)GETWORD(lpStruct+26));
	lpdm->dmColor = (int)((short)GETWORD(lpStruct+28));
	lpdm->dmDuplex = (int)((short)GETWORD(lpStruct+30));
	lpdm->dmYResolution = (int)((short)GETWORD(lpStruct+32));
	lpdm->dmTTOption = (int)((short)GETWORD(lpStruct+34));
	if (lpdm->dmDriverExtra)
		memcpy((LPSTR)lpdm+sizeof(DEVMODE),(LPSTR)lpStruct+36,
				lpdm->dmDriverExtra);
}

void
PutDEVMODE (LPBYTE lpData, DEVMODE *lpdm)
{
	memcpy((LPSTR)lpData,(LPSTR)&lpdm->dmDeviceName,CCHDEVICENAME);
	lpData += CCHDEVICENAME;
	PUTWORD(lpData,lpdm->dmSpecVersion);
	PUTWORD(lpData+2,lpdm->dmDriverVersion);
	PUTWORD(lpData+4,CCHDEVICENAME + 4*UINT_86 + DWORD_86 + 12*INT_86);
	PUTWORD(lpData+6,lpdm->dmDriverExtra);
	PUTDWORD(lpData+8,lpdm->dmFields);
	PUTWORD(lpData+12,(WORD)lpdm->dmOrientation);
	PUTWORD(lpData+14,(WORD)lpdm->dmPaperSize);
	PUTWORD(lpData+16,(WORD)lpdm->dmPaperLength);
	PUTWORD(lpData+18,(WORD)lpdm->dmPaperWidth);
	PUTWORD(lpData+20,(WORD)lpdm->dmScale);
	PUTWORD(lpData+22,(WORD)lpdm->dmCopies);
	PUTWORD(lpData+24,(WORD)lpdm->dmDefaultSource);
	PUTWORD(lpData+26,(WORD)lpdm->dmPrintQuality);
	PUTWORD(lpData+28,(WORD)lpdm->dmColor);
	PUTWORD(lpData+30,(WORD)lpdm->dmDuplex);
	PUTWORD(lpData+32,(WORD)lpdm->dmYResolution);
	PUTWORD(lpData+34,(WORD)lpdm->dmTTOption);
	if (lpdm->dmDriverExtra)
		memcpy((LPVOID)((LPBYTE)lpData+36),
			(LPVOID)((LPBYTE)lpdm+sizeof(DEVMODE)),
			lpdm->dmDriverExtra);
}

/*--------------------------------------------------------------------*/

void
PutDRAWMODE(LPBYTE lpData, DRAWMODE *lpDrawMode)
{
    PUTWORD(lpData,(WORD)lpDrawMode->Rop2);
    PUTWORD(lpData+2,(WORD)lpDrawMode->bkMode);
    PUTDWORD(lpData+4,(DWORD)lpDrawMode->bkColor);
    PUTDWORD(lpData+8,(DWORD)lpDrawMode->TextColor);
    PUTWORD(lpData+12,(WORD)lpDrawMode->TBreakExtra);
    PUTWORD(lpData+14,(WORD)lpDrawMode->BreakExtra);
    PUTWORD(lpData+16,(WORD)lpDrawMode->BreakErr);
    PUTWORD(lpData+18,(WORD)lpDrawMode->BreakRem);
    PUTWORD(lpData+20,(WORD)lpDrawMode->BreakCount);
    PUTWORD(lpData+22,(WORD)lpDrawMode->CharExtra);
    PUTDWORD(lpData+24,(DWORD)lpDrawMode->LbkColor);
    PUTDWORD(lpData+28,(DWORD)lpDrawMode->LTextColor);
}

void
GetDRAWMODE(DRAWMODE *lpDrawMode, LPBYTE lpData)
{
    lpDrawMode->Rop2 = (short)(int)GETWORD(lpData);
    lpDrawMode->bkMode = (short)(int)GETWORD(lpData+2);
    lpDrawMode->bkColor = GETDWORD(lpData+4);
    lpDrawMode->TextColor = GETDWORD(lpData+8);
    lpDrawMode->TBreakExtra = (short)(int)GETWORD(lpData+12);
    lpDrawMode->BreakExtra = (short)(int)GETWORD(lpData+14);
    lpDrawMode->BreakErr = (short)(int)GETWORD(lpData+16);
    lpDrawMode->BreakRem = (short)(int)GETWORD(lpData+18);
    lpDrawMode->BreakCount = (short)(int)GETWORD(lpData+20);
    lpDrawMode->CharExtra = (short)(int)GETWORD(lpData+22);
    lpDrawMode->LbkColor = GETDWORD(lpData+24);
    lpDrawMode->LTextColor = GETDWORD(lpData+28);
}

void
GetGDIINFO(GDIINFO *lpgdi,LPBYTE lpData)
{
    lpgdi->dpVersion = (int)(short)GETWORD(lpData);
    lpgdi->dpTechnology = (int)(short)GETWORD(lpData+2);
    lpgdi->dpHorzSize = (int)(short)GETWORD(lpData+4);
    lpgdi->dpVertSize = (int)(short)GETWORD(lpData+6);
    lpgdi->dpHorzRes = (int)(short)GETWORD(lpData+8);
    lpgdi->dpVertRes = (int)(short)GETWORD(lpData+10);
    lpgdi->dpBitsPixel = (int)(short)GETWORD(lpData+12);
    lpgdi->dpPlanes = (int)(short)GETWORD(lpData+14);
    lpgdi->dpNumBrushes = (int)(short)GETWORD(lpData+16);
    lpgdi->dpNumPens = (int)(short)GETWORD(lpData+18);
    lpgdi->dpNumMarkers = (int)(short)GETWORD(lpData+20);
    lpgdi->dpNumFonts = (int)(short)GETWORD(lpData+22);
    lpgdi->dpNumColors = (int)(short)GETWORD(lpData+24);
    lpgdi->dpDEVICEsize = (int)(short)GETWORD(lpData+26);
    lpgdi->dpCurves = (UINT)GETWORD(lpData+28);
    lpgdi->dpLines = (UINT)GETWORD(lpData+30);
    lpgdi->dpPolygonals = (UINT)GETWORD(lpData+32);
    lpgdi->dpText = (UINT)GETWORD(lpData+34);
    lpgdi->dpClip = (UINT)GETWORD(lpData+36);
    lpgdi->dpRaster = (int)(short)GETWORD(lpData+38);
    lpgdi->dpAspectX = (int)(short)GETWORD(lpData+40);
    lpgdi->dpAspectY = (int)(short)GETWORD(lpData+42);
    lpgdi->dpAspectXY = (int)(short)GETWORD(lpData+44);
    lpgdi->dpStyleLen = (int)(short)GETWORD(lpData+46);
    GetPOINT(lpgdi->dpMLoWin,lpData+48);
    GetPOINT(lpgdi->dpMLoVpt,lpData+52);
    GetPOINT(lpgdi->dpMHiWin,lpData+56);
    GetPOINT(lpgdi->dpMHiVpt,lpData+60);
    GetPOINT(lpgdi->dpELoWin,lpData+64);
    GetPOINT(lpgdi->dpELoVpt,lpData+68);
    GetPOINT(lpgdi->dpEHiWin,lpData+72);
    GetPOINT(lpgdi->dpEHiVpt,lpData+76);
    GetPOINT(lpgdi->dpTwpWin,lpData+80);
    GetPOINT(lpgdi->dpTwpVpt,lpData+84);
    lpgdi->dpLogPixelsX = (int)(short)GETWORD(lpData+88);
    lpgdi->dpLogPixelsY = (int)(short)GETWORD(lpData+90);
    lpgdi->dpDCManage = (int)(short)GETWORD(lpData+92);
    lpgdi->dpCaps1 = (int)(short)GETWORD(lpData+94);
    lpgdi->futureuse4 = (int)(short)GETWORD(lpData+96);
    lpgdi->futureuse5 = (int)(short)GETWORD(lpData+98);
    lpgdi->futureuse6 = (int)(short)GETWORD(lpData+100);
    lpgdi->futureuse7 = (int)(short)GETWORD(lpData+102);
    lpgdi->dpSizePallete = (int)(short)GETWORD(lpData+104);
    lpgdi->dpNumReserved = (int)(short)GETWORD(lpData+106);
    lpgdi->dpColorRes = (int)(short)GETWORD(lpData+108);
}

/*--------------------------------------------------------------------*/

int
PutDFONTINFO(LPBYTE lpData, DFONTINFO *lpfi)
{
    PUTWORD(lpData,(WORD)lpfi->dfType);
    PUTWORD(lpData+2,(WORD)lpfi->dfPoints);
    PUTWORD(lpData+4,(WORD)lpfi->dfVertRes);
    PUTWORD(lpData+6,(WORD)lpfi->dfHorizRes);
    PUTWORD(lpData+8,(WORD)lpfi->dfAscent);
    PUTWORD(lpData+10,(WORD)lpfi->dfInternalLeading);
    PUTWORD(lpData+12,(WORD)lpfi->dfExternalLeading);
    *(lpData+14) = (BYTE)lpfi->dfItalic;
    *(lpData+15) = (BYTE)lpfi->dfUnderline;
    *(lpData+16) = (BYTE)lpfi->dfStrikeOut;
    PUTWORD(lpData+17,(WORD)lpfi->dfWeight);
    *(lpData+19) = (BYTE)lpfi->dfCharSet;
    PUTWORD(lpData+20,(WORD)lpfi->dfPixWidth);
    PUTWORD(lpData+22,(WORD)lpfi->dfPixHeight);
    *(lpData+24) = (BYTE)lpfi->dfPitchAndFamily;
    PUTWORD(lpData+25,(WORD)lpfi->dfAvgWidth);
    PUTWORD(lpData+27,(WORD)lpfi->dfMaxWidth);
    *(lpData+29) = (BYTE)lpfi->dfFirstChar;
    *(lpData+30) = (BYTE)lpfi->dfLastChar;
    *(lpData+31) = (BYTE)lpfi->dfDefaultChar;
    *(lpData+32) = (BYTE)lpfi->dfBreakChar;
    PUTWORD(lpData+33,(WORD)lpfi->dfWidthBytes);
    PUTDWORD(lpData+35,lpfi->dfDevice);
    PUTDWORD(lpData+39,lpfi->dfFace);
    PUTDWORD(lpData+43,lpfi->dfBitsPointer);
    PUTDWORD(lpData+47,lpfi->dfBitsOffset);
    PUTWORD(lpData+49,lpfi->dfSizeFields);
    PUTDWORD(lpData+53,lpfi->dfExtMetricsOffset);
    PUTDWORD(lpData+57,lpfi->dfExtentTable);
    PUTDWORD(lpData+61,lpfi->dfOriginTable);
    PUTDWORD(lpData+65,lpfi->dfPairKernTable);
    PUTDWORD(lpData+69,lpfi->dfTrackKernTable);
    PUTDWORD(lpData+73,lpfi->dfDriverInfo);
    PUTDWORD(lpData+77,lpfi->dfReserved);
    return(1);
}


void
GetDFONTINFO(DFONTINFO *pfi, LPBYTE lpBin)
{
    int nDiff = sizeof(DFONTINFO) - SIZEOF_DFONTINFO_BIN + sizeof(int);

        pfi->dfType             =  GETWORD(lpBin);
        pfi->dfPoints           =  GETWORD(lpBin + 2);
        pfi->dfVertRes          =  GETWORD(lpBin + 4);
        pfi->dfHorizRes         =  GETWORD(lpBin + 6);
        pfi->dfAscent           =  GETWORD(lpBin + 8);
        pfi->dfInternalLeading  =  GETWORD(lpBin + 10);
        pfi->dfExternalLeading  =  GETWORD(lpBin + 12);
        pfi->dfItalic           =  *(lpBin + 14);
        pfi->dfUnderline        =  *(lpBin + 15);
        pfi->dfStrikeOut        =  *(lpBin + 16);
        pfi->dfWeight           =  GETWORD(lpBin + 17);
        pfi->dfCharSet          =  *(lpBin + 19);
        pfi->dfPixWidth         =  GETWORD(lpBin + 20);
        pfi->dfPixHeight        =  GETWORD(lpBin + 22);
        pfi->dfPitchAndFamily   =  *(lpBin + 24);
        pfi->dfAvgWidth         =  GETWORD(lpBin + 25);
        pfi->dfMaxWidth         =  GETWORD(lpBin + 27);
        pfi->dfFirstChar        =  *(lpBin + 29);
        pfi->dfLastChar         =  *(lpBin + 30);
        pfi->dfDefaultChar      =  *(lpBin + 31);
        pfi->dfBreakChar        =  *(lpBin + 32);
        pfi->dfWidthBytes       =  GETWORD(lpBin + 33);
        pfi->dfDevice           =  GETDWORD(lpBin + 35) + nDiff;
        pfi->dfFace             =  GETDWORD(lpBin + 39) + nDiff;
#ifdef	LATER
	we should adjust some other fields, too...
#endif
        pfi->dfBitsPointer      =  GETDWORD(lpBin + 43);
        pfi->dfBitsOffset       =  GETDWORD(lpBin + 47);
        pfi->dfSizeFields       =  GETWORD(lpBin + 51);
        pfi->dfExtMetricsOffset =  GETDWORD(lpBin + 53);
        pfi->dfExtentTable      =  GETDWORD(lpBin + 57);
        pfi->dfOriginTable      =  GETDWORD(lpBin + 61);
        pfi->dfPairKernTable    =  GETDWORD(lpBin + 65);
        pfi->dfTrackKernTable   =  GETDWORD(lpBin + 69);
        pfi->dfDriverInfo       =  GETDWORD(lpBin + 73);
        pfi->dfReserved         =  GETDWORD(lpBin + 77);
}

/*--------------------------------------------------------------------*/

int
GetRFONT(RFONT *prf, LPBYTE pBin, int iBinSize)
{
	GetDFONTINFO((DFONTINFO*)prf, pBin);

	if (iBinSize < SIZEOF_DFONTINFO_BIN)
	{
		return(0);
	}

	prf->iBinSize = iBinSize - SIZEOF_DFONTINFO_BIN;

	memcpy((LPSTR)prf->byBinPart, 
		(LPSTR)pBin + SIZEOF_DFONTINFO_BIN, prf->iBinSize);

	return(1);
}

void
PutRFONT(LPBYTE pBin, RFONT *prf)
{
	PutDFONTINFO(pBin, (DFONTINFO*)prf);
	memcpy((LPSTR)pBin + SIZEOF_DFONTINFO_BIN,
		(LPSTR)prf->byBinPart, prf->iBinSize);
}


/*--------------------------------------------------------------------*/
void
GetTEXTXFORM(TEXTXFORM *ptxf, LPBYTE pBin)
{
	ptxf->txfHeight        = GETWORD(pBin);
	ptxf->txfWidth         = GETWORD(pBin + 2);
	ptxf->txfEscapement    = GETWORD(pBin + 4);
	ptxf->txfOrientation   = GETWORD(pBin + 6);
	ptxf->txfWeight        = GETWORD(pBin + 8);
	ptxf->txfItalic        = *(pBin + 10);
	ptxf->txfUnderline     = *(pBin + 11);
	ptxf->txfStrikeOut     = *(pBin + 12);
	ptxf->txfOutPrecision  = *(pBin + 13);
	ptxf->txfClipPrecision = *(pBin + 14);
	ptxf->txfAccelerator   = GETWORD(pBin + 15);
	ptxf->txfOverhang      = GETWORD(pBin + 17);
}


void
PutTEXTXFORM(LPBYTE lpData, TEXTXFORM *lptf)
{
    PUTWORD(lpData,(WORD)lptf->txfHeight);
    PUTWORD(lpData+2,(WORD)lptf->txfWidth);
    PUTWORD(lpData+4,(WORD)lptf->txfEscapement);
    PUTWORD(lpData+6,(WORD)lptf->txfOrientation);
    PUTWORD(lpData+8,(WORD)lptf->txfWeight);
    *(lpData+10) = (BYTE)lptf->txfItalic;
    *(lpData+11) = (BYTE)lptf->txfUnderline;
    *(lpData+12) = (BYTE)lptf->txfStrikeOut;
    *(lpData+13) = (BYTE)lptf->txfOutPrecision;
    *(lpData+14) = (BYTE)lptf->txfClipPrecision;
    PUTWORD(lpData+15,(WORD)lptf->txfAccelerator);
    PUTWORD(lpData+17,(WORD)lptf->txfOverhang);
}

/*--------------------------------------------------------------------*/

void
GetETM(EXTTEXTMETRIC *pETM, LPBYTE pBin)
{
	pETM->etmSize                       = sizeof(EXTTEXTMETRIC);
	pETM->etmPointSize                  = (short)GETWORD(pBin + 2);
	pETM->etmOrientation                = (short)GETWORD(pBin + 4);
	pETM->etmMasterHeight               = (short)GETWORD(pBin + 6);
	pETM->etmMinScale                   = (short)GETWORD(pBin + 8);
	pETM->etmMaxScale                   = (short)GETWORD(pBin + 10);
	pETM->etmMasterUnits                = (short)GETWORD(pBin + 12);
	pETM->etmCapHeight                  = (short)GETWORD(pBin + 14);
	pETM->etmXHeight                    = (short)GETWORD(pBin + 16);
	pETM->etmLowerCaseAscent            = (short)GETWORD(pBin + 18);
	pETM->etmLowerCaseDescent           = (short)GETWORD(pBin + 20);
	pETM->etmSlant                      = (short)GETWORD(pBin + 22);
	pETM->etmSuperScript                = (short)GETWORD(pBin + 24);
	pETM->etmSubScript                  = (short)GETWORD(pBin + 26);
	pETM->etmSuperScriptSize            = (short)GETWORD(pBin + 28);
	pETM->etmSubScriptSize              = (short)GETWORD(pBin + 30);
	pETM->etmUnderlineOffset            = (short)GETWORD(pBin + 32);
	pETM->etmUnderlineWidth             = (short)GETWORD(pBin + 34);
	pETM->etmDoubleUpperUnderlineOffset = (short)GETWORD(pBin + 36);
	pETM->etmDoubleLowerUnderlineOffset = (short)GETWORD(pBin + 38);
	pETM->etmDoubleUpperUnderlineWidth  = (short)GETWORD(pBin + 40);
	pETM->etmDoubleLowerUnderlineWidth  = (short)GETWORD(pBin + 42);
	pETM->etmStrikeOutOffset            = (short)GETWORD(pBin + 44);
	pETM->etmStrikeOutWidth             = (short)GETWORD(pBin + 46);
	pETM->etmNKernPairs                 =        GETWORD(pBin + 48);
	pETM->etmNKernTracks                =        GETWORD(pBin + 50);
}

void
PutETM(LPBYTE lp, EXTTEXTMETRIC *pETM, int nLimit)
{
	PUTWORD(lp,(WORD)pETM->etmSize);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+2,(WORD)pETM->etmPointSize);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+4,(WORD)pETM->etmOrientation);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+6,(WORD)pETM->etmMasterHeight);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+8,(WORD)pETM->etmMinScale);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+10,(WORD)pETM->etmMaxScale);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+12,(WORD)pETM->etmMasterUnits);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+14,(WORD)pETM->etmCapHeight);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+16,(WORD)pETM->etmXHeight);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+18,(WORD)pETM->etmLowerCaseAscent);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+20,(WORD)pETM->etmLowerCaseDescent);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+22,(WORD)pETM->etmSlant);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+24,(WORD)pETM->etmSuperScript);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+26,(WORD)pETM->etmSubScript);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+28,(WORD)pETM->etmSuperScriptSize);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+30,(WORD)pETM->etmSubScriptSize);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+32,(WORD)pETM->etmUnderlineOffset);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+34,(WORD)pETM->etmUnderlineWidth);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+36,(WORD)pETM->etmDoubleUpperUnderlineOffset);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+38,(WORD)pETM->etmDoubleLowerUnderlineOffset);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+40,(WORD)pETM->etmDoubleUpperUnderlineWidth);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+42,(WORD)pETM->etmDoubleLowerUnderlineWidth);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+44,(WORD)pETM->etmStrikeOutOffset);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+46,(WORD)pETM->etmStrikeOutWidth);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+48,(WORD)pETM->etmNKernPairs);
	nLimit -= 2; if (nLimit < 2) return;
	PUTWORD(lp+50,(WORD)pETM->etmNKernTracks);
}

/*--------------------------------------------------------------------*/

/* This function implies the following layout:
 *	Frame for EXTTEXTDATA
 *	Frame for APPEXTTEXTDATA
 *	Frame for DFONTINFO (RFONT)
 *	Frame for TEXTXFORM
 *	Frame for DRAWMODE
 */
int
PutEXTTEXTDATA(LPBYTE lpEtdBin, WORD wStackSeg, DWORD ss, int cbRFsize)
{
	DWORD	dwCurOffset = SIZEOF_EXTTEXTDATA_BIN;

	PUTWORD(lpEtdBin, SIZEOF_EXTTEXTDATA_BIN);

	/* Address of APPEXTTEXTDATA structure */
	PUTWORD(lpEtdBin+4, wStackSeg);
	PUTWORD(lpEtdBin+2, (WORD)((DWORD)lpEtdBin + dwCurOffset - ss));
	dwCurOffset += SIZEOF_APPEXTTEXTDATA_BIN;

	/* Address of DFONTINFO (RFONT) structure */
	PUTWORD(lpEtdBin+8, wStackSeg);
	PUTWORD(lpEtdBin+6, (WORD)((DWORD)lpEtdBin + dwCurOffset - ss));
	dwCurOffset += cbRFsize;

	/* Address of TEXTXFORM structure */
	PUTWORD(lpEtdBin+12, wStackSeg);
	PUTWORD(lpEtdBin+10, (WORD)((DWORD)lpEtdBin + dwCurOffset - ss));
	dwCurOffset += SIZEOF_TEXTXFORM_BIN;

	/* Address of DRAWMODE structure */
	PUTWORD(lpEtdBin+16, wStackSeg);
	PUTWORD(lpEtdBin+14, (WORD)((DWORD)lpEtdBin + dwCurOffset - ss));
	dwCurOffset += SIZEOF_DRAWMODE_BIN;

	return (int)dwCurOffset;	/* The total size of all structures */
}

void
PutPSBITMAP(LPBYTE lpData, LPPSBITMAP lpPSBitmap)
{
    PUTWORD(lpData,lpPSBitmap->bm.bmType);
    PUTWORD(lpData+2,lpPSBitmap->bm.bmWidth);
    PUTWORD(lpData+4,lpPSBitmap->bm.bmHeight);
    PUTWORD(lpData+6,lpPSBitmap->bm.bmWidthBytes);
    *(lpData+8) = lpPSBitmap->bm.bmPlanes;
    *(lpData+9) = lpPSBitmap->bm.bmBitsPixel;
    PUTDWORD(lpData+10,0L);
    PUTDWORD(lpData+14,lpPSBitmap->bmWidthPlanes);
    PUTDWORD(lpData+18,lpPSBitmap->bmlpPDevice);
    PUTWORD(lpData+22,lpPSBitmap->bmSegmentIndex);
    PUTWORD(lpData+24,lpPSBitmap->bmScanSegment);
    PUTWORD(lpData+26,lpPSBitmap->bmFillBytes);
    PUTWORD(lpData+28,lpPSBitmap->futureUse4);
    PUTWORD(lpData+30,lpPSBitmap->futureUse5);
}

void
GetPSBITMAP(LPPSBITMAP lpPSBitmap, LPBYTE lpData)
{
    BINADDR binBits;

    lpPSBitmap->bm.bmType = GETWORD(lpData);
    lpPSBitmap->bm.bmWidth = GETWORD(lpData+2);
    lpPSBitmap->bm.bmHeight = GETWORD(lpData+4);
    lpPSBitmap->bm.bmWidthBytes = GETWORD(lpData+6);
    lpPSBitmap->bm.bmPlanes = *(lpData+8);
    lpPSBitmap->bm.bmBitsPixel = *(lpData+9);
    binBits = GETDWORD(lpData+10);
    if (binBits)
	lpPSBitmap->bm.bmBits = GetAddress(HIWORD(binBits),LOWORD(binBits));
    else
	lpPSBitmap->bm.bmBits = 0;
    lpPSBitmap->bmWidthPlanes = GETDWORD(lpData+14);
    lpPSBitmap->bmlpPDevice = (LPVOID)GETDWORD(lpData+18);
    lpPSBitmap->bmSegmentIndex = GETWORD(lpData+22);
    lpPSBitmap->bmScanSegment = GETWORD(lpData+24);
    lpPSBitmap->bmFillBytes = GETWORD(lpData+26);
    lpPSBitmap->futureUse4 = GETWORD(lpData+28);
    lpPSBitmap->futureUse5 = GETWORD(lpData+30);
}

void
GetDOCINFO(LPDOCINFO lpdi, LPBYTE lpBin)
{
    lpdi->cbSize = GETWORD(lpBin);
    lpdi->lpszDocName = GetAddress(GETWORD(lpBin + 4), GETWORD(lpBin + 2));
    lpdi->lpszOutput  = GetAddress(GETWORD(lpBin + 8), GETWORD(lpBin + 6));
}

/* interface methods for brute-force functions follow */
void
IT_DMREALIZE(ENV *envp, LONGPROC f)	/* dmRealizeObject */
{
    int iStyle;
    LOGBRUSH LogBrush;
    LOGPEN LogPen;
    LPBYTE lpStruct,lpStruct1;
    DWORD retcode = MAKELONG(0xffff,0xffff);

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+20),GETWORD(SP+18));

    iStyle = (int)(short)GETWORD(SP+16);

    lpStruct1 = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));

    switch (iStyle) {
	case DRV_OBJ_BRUSH:
	    lpStruct1 = (LPBYTE)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	    LogBrush.lbStyle = (UINT)GETWORD(lpStruct1);
	    LogBrush.lbColor = (COLORREF)GETDWORD(lpStruct1+2);
	    LogBrush.lbHatch = (int)(short)GETWORD(lpStruct1+6);
	    retcode = dmRealizeObject((LPVOID)lpStruct,iStyle,
					(LPSTR)&LogBrush,0,0);
	    break;

	case DRV_OBJ_PEN:
	    lpStruct1 = (LPBYTE)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	    LogPen.lopnStyle = (UINT)GETWORD(lpStruct1);
	    LogPen.lopnWidth.x = (short)(int)GETWORD(lpStruct1+2);
	    LogPen.lopnWidth.y = (short)(int)GETWORD(lpStruct1+4);
	    LogPen.lopnColor = (COLORREF)GETDWORD(lpStruct1+6);
	    retcode = dmRealizeObject((LPVOID)lpStruct,iStyle,
					(LPSTR)&LogPen,0,0);
	    break;

	case DRV_OBJ_FONT:
	case ~DRV_OBJ_BRUSH:
	case ~DRV_OBJ_PEN:
	case ~DRV_OBJ_FONT:
	default:
	    FatalAppExit(0,"IT_DMREALIZE: unknown object type");
    }

    envp->reg.sp += INT_86 + 4*LP_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
IT_DMBITBLT(ENV *envp, LONGPROC f)	/* dmBitBlt */
{
    LPBYTE lpStruct;
    PSBITMAP psbmpSrc,psbmpDest;
    LPPSBITMAP lpbmpSrc;
    DRAWMODE dm;
    DWORD retcode;

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+34),GETWORD(SP+32));
    GetPSBITMAP(&psbmpDest,lpStruct);

    if ((lpStruct = (LPBYTE)GetAddress(GETWORD(SP+26),GETWORD(SP+24)))) {
	GetPSBITMAP(&psbmpSrc,lpStruct);
	lpbmpSrc = &psbmpSrc;
    }
    else
	lpbmpSrc = 0;

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
    GetDRAWMODE(&dm,lpStruct);

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));

    retcode = dmBitBlt(&psbmpDest,
		(int)GETSHORT(SP+30),
		(int)GETSHORT(SP+28),
		(LPVOID)lpbmpSrc,
		(int)GETSHORT(SP+22),
		(int)GETSHORT(SP+20),
		(int)GETSHORT(SP+18),
		(int)GETSHORT(SP+16),
		GETDWORD(SP+12),
		(LPVOID)lpStruct,
		&dm);

    envp->reg.sp += 6*INT_86 + 4*LP_86 + LONG_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
IT_DMCOLORINFO(ENV *envp, LONGPROC f)	/* dmColorInfo */
{
    LPBYTE lpStruct;
    PSBITMAP psbmpDest;
    DWORD retcode;

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
    GetPSBITMAP(&psbmpDest,lpStruct);

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));

    retcode = dmColorInfo(&psbmpDest,
		GETDWORD(SP+8),
		(LPVOID)lpStruct);

    envp->reg.sp += 2*LP_86 + LONG_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
IT_DMPIXEL(ENV *envp, LONGPROC f)	/* dmPixel */
{
    LPBYTE lpStruct;
    PSBITMAP psbmpDest;
    DWORD retcode;
    DRAWMODE dm;

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+18),GETWORD(SP+16));
    GetPSBITMAP(&psbmpDest,lpStruct);

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
    GetDRAWMODE(&dm,lpStruct);

    retcode = dmPixel(&psbmpDest,
		(int)GETSHORT(SP+14),
		(int)GETSHORT(SP+12),
		GETDWORD(SP+8),
		(LPVOID)lpStruct);

    envp->reg.sp += 2*LP_86 + LONG_86 + 2*INT_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
IT_DMOUTPUT(ENV *envp, LONGPROC f)	/* dmOutput */
{
    DWORD retcode;
    LPBYTE lpStruct,lpBrush,lpPen,lpPoints;
    PSBITMAP psbmpDest;
    int nPointCount;
    RECT rc;
    LPRECT lprc;
    DRAWMODE dm;

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+30),GETWORD(SP+28));
    GetPSBITMAP(&psbmpDest,lpStruct);

    nPointCount = (int)GETSHORT(SP+24);

    lpPoints = (LPBYTE)GetAddress(GETWORD(SP+22),GETWORD(SP+20));
    lpPen = (LPBYTE)GetAddress(GETWORD(SP+18),GETWORD(SP+16));
    lpBrush = (LPBYTE)GetAddress(GETWORD(SP+14),GETWORD(SP+12));

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
    GetDRAWMODE(&dm,lpStruct);

    if ((lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4)))) {
	RECT_TO_C(rc,lpStruct);
	lprc = &rc;
    }
    else
	lprc = 0;

    retcode = dmOutput(&psbmpDest,
		(int)GETSHORT(SP+26),
		nPointCount,
		(LPPOINT)hsw_ConvertArrayToInt(lpPoints,nPointCount*2),
		(LPVOID)lpPen,
		(LPVOID)lpBrush,
		&dm,
		lprc);

    envp->reg.sp += 2*INT_86 + 6*LP_86  + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}
