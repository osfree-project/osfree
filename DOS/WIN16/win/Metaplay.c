/*    
	Metaplay.c	2.18
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

#include "kerndef.h"
#include "Endian.h"
#include "BinTypes.h"
#include "Resources.h"
#include "Log.h"
#include "PrinterBin.h"
#include "GdiObjects.h"
#include "Meta.h"

/* Unimplemented features:

META_ANIMATEPALETTE
META_RESIZEPALETTE
META_SETPALENTRIES

META_CREATEBITMAP
META_CREATEBITMAPINDIRECT
META_CREATEBRUSH

META_CREATEREGION
META_FRAMEREGION

META_DRAWTEXT

META_SETRELABS

META_ABORTDOC
META_ENDDOC
META_ENDPAGE
META_RESETDC
META_STARTDOC
META_STARTPAGE

*/

extern void GetLOGFONT(LPLOGFONT, LPBYTE);
extern void GetBITMAPINFO(LPBITMAPINFO, LPBYTE);
extern LPINT hsw_ConvertArrayToInt(LPBYTE, int);
extern BOOL IsMetafileDC(HDC);
extern BOOL TWIN_DeleteObject(HGDIOBJ, BOOL);
extern HBITMAP CreateDIBitmapEx(LPBITMAPIMAGE,LPNAMEINFO);
extern int CalcByteWidth(int,int,int);
extern BOOL IsValidMetaFile(HMETAFILE );
void TWIN_InitHandleTable(LPHANDLETABLE , int );

static BOOL InternalEnumMF(HDC,HMETAFILE,MFENUMPROC,LPARAM);
static int EnumMFRecords(HDC,LPHANDLETABLE,LPMETARECORD,int,LPARAM);
static BOOL MetaDeleteObject(LPHANDLETABLE,int,BOOL);

void WINAPI
PlayMetaFileRecord(HDC hDC, HANDLETABLE *lpht, 
			METARECORD *lpMetaRecord, UINT nHandles)
{
	LPBYTE		lpEscapeData;
	LPMFRECORD	lpBRec;
	LPWORD		buffer;
	LOGBRUSH	brush;
	LOGPEN		pen;
	LPLOGPALETTE	lpLogPalette;
	LPRGBQUAD	lpRGB;
	HGDIOBJ		hObject = 0;
	HGLOBAL		hDIB;
	LPBYTE		lpDIB;
	int		nPaletteLength;
	LPSTR		lpStr;
	LPINT		lpInt,lpPolyCount;
	DWORD		dw;
	int		i;
	int		slot;
	int		iInt1, iInt2, ix, iy, nWidth;
	int		nSavedDC;
	int		nPatternType,nNumColors;
	UINT		uColorUse;
	unsigned	npoints;
	DWORD		dwSize;
	WORD		wFunc;
	int		x1, y1, x2, y2, x3, y3, x4, y4, x, y;
	WORD		w;
	WORD		nCount, nEscape, nBkMode, nMapMode;
	WORD		nPolyFillMode, nDrawMode, nStretchMode;
	RECT		rect;
	LOGFONT		lf;
	BITMAPINFOHEADER bmi;
	BITMAPIMAGE	bi;
	BITMAP		bm;
	HBITMAP		hBitmap;

	/* Casting from structure containing UINT, which */
	/* cannot be used, to structure containing WORD. */
	lpBRec = (LPMFRECORD)lpMetaRecord;
	buffer = &(lpBRec->rdParm[0]);
	dwSize = GETDWORD(lpBRec);
	wFunc = GETWORD(((LPWORD)lpBRec)+2);

	switch ( wFunc ) {
	case META_ABORTDOC:
		break;

	case META_ANIMATEPALETTE:
		break;

	case META_ARC: 
		y4 = GETSHORT(buffer);
		x4 = GETSHORT(buffer+1);
		y3 = GETSHORT(buffer+2);
		x3 = GETSHORT(buffer+3);
		y2 = GETSHORT(buffer+4);
		x2 = GETSHORT(buffer+5);
		y1 = GETSHORT(buffer+6);
		x1 = GETSHORT(buffer+7);
		Arc(hDC, x1, y1, x2, y2, x3, y3, x4, y4);
		break;

	case META_DIBBITBLT: {
		int		nColors;
		LPBITMAPINFO	lpbi;
		LPVOID		lpvBits;
		HDC		hDCCompat;
		HBITMAP		hDIB, hBmpOld;
		DWORD		dwRop;
		int		XSrc,YSrc,XDest,YDest,cxDest,cyDest;

		dwRop  = GETDWORD(buffer);
		YSrc   = GETSHORT(buffer+2);
		XSrc   = GETSHORT(buffer+3);
		cyDest = GETSHORT(buffer+4);
		cxDest = GETSHORT(buffer+5);
		YDest  = GETSHORT(buffer+6);
		XDest  = GETSHORT(buffer+7);

		buffer += 8;	/* buffer points to BITMAPINFO */
		nColors = 1 << (int)(short)GETWORD((LPBYTE)buffer+14);
		lpbi = (LPBITMAPINFO)WinMalloc(sizeof(BITMAPINFO) +
				sizeof(RGBQUAD) * (nColors - 1));
		GetBITMAPINFO(lpbi, (LPBYTE)buffer);
		lpvBits = (LPBYTE)buffer + SIZEOF_BITMAPINFOHEADER_BIN +
				SIZEOF_RGBQUAD_BIN * nColors;

		hDIB = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER)lpbi, CBM_INIT,
				lpvBits, lpbi, DIB_RGB_COLORS);
		hDCCompat = CreateCompatibleDC(hDC);
		hBmpOld = SelectObject(hDCCompat, hDIB);
		BitBlt(hDC, XDest, YDest, cxDest, cyDest,
			hDCCompat, XSrc, YSrc, dwRop);

		SelectObject(hDCCompat, hBmpOld);
		TWIN_DeleteObject(hDIB,FALSE);
		DeleteDC(hDCCompat);

		WinFree((LPSTR)lpbi);
	    }
	    break;

	case META_BITBLT: {
		BITMAP		bm;
		HDC		hDCCompat;
		HBITMAP		hDDB, hBmpOld;
		DWORD		dwRop;
		int		XSrc,YSrc,XDest,YDest,cxDest,cyDest;

		dwRop  = GETDWORD(buffer);
		YSrc   = GETSHORT(buffer+2);
		XSrc   = GETSHORT(buffer+3);
		cyDest = GETSHORT(buffer+4);
		cxDest = GETSHORT(buffer+5);
		YDest  = GETSHORT(buffer+6);
		XDest  = GETSHORT(buffer+7);

		buffer += 8;

		bm.bmType = 0;
		bm.bmWidth = GETSHORT(buffer);
		bm.bmHeight = GETSHORT(buffer+1);
		bm.bmWidthBytes = GETSHORT(buffer+2);
		bm.bmPlanes = GETSHORT(buffer+3);
		bm.bmBitsPixel = GETSHORT(buffer+4);
		bm.bmBits = (LPVOID)(buffer+5);

		hDDB = CreateBitmapIndirect(&bm);

		hDCCompat = CreateCompatibleDC(hDC);
		hBmpOld = SelectObject(hDCCompat, hDDB);
		BitBlt(hDC, XDest, YDest, cxDest, cyDest,
			hDCCompat, XSrc, YSrc, dwRop);

		SelectObject(hDCCompat, hBmpOld);
		TWIN_DeleteObject(hDDB,FALSE);
		DeleteDC(hDCCompat);
	    }
	    break;

	case META_DIBSTRETCHBLT: {
		int		nColors;
		LPBITMAPINFO	lpbi;
		LPVOID		lpvBits;
		HDC		hDCCompat;
		HBITMAP		hDIB, hBmpOld;
		DWORD		dwRop;
		int		XSrc,YSrc,XDest,YDest,cxDest,cyDest,cxSrc,cySrc;

		dwRop  = GETDWORD(buffer);
		cySrc = GETSHORT(buffer+2);
		cxSrc = GETSHORT(buffer+3);
		YSrc   = GETSHORT(buffer+4);
		XSrc   = GETSHORT(buffer+5);
		cyDest = GETSHORT(buffer+6);
		cxDest = GETSHORT(buffer+7);
		YDest  = GETSHORT(buffer+8);
		XDest  = GETSHORT(buffer+9);

		buffer += 10;	/* buffer points to BITMAPINFO */
		nColors = 1 << (int)(short)GETWORD((LPBYTE)buffer+14);
		lpbi = (LPBITMAPINFO)WinMalloc(sizeof(BITMAPINFO) +
				sizeof(RGBQUAD) * (nColors - 1));
		GetBITMAPINFO(lpbi, (LPBYTE)buffer);
		lpvBits = (LPBYTE)buffer + SIZEOF_BITMAPINFOHEADER_BIN +
				SIZEOF_RGBQUAD_BIN * nColors;

		hDIB = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER)lpbi, CBM_INIT,
				lpvBits, lpbi, DIB_RGB_COLORS);
		hDCCompat = CreateCompatibleDC(hDC);
		hBmpOld = SelectObject(hDCCompat, hDIB);
		StretchBlt(hDC, XDest, YDest, cxDest, cyDest,
			hDCCompat, XSrc, YSrc, cxSrc, cySrc, dwRop);

		SelectObject(hDCCompat, hBmpOld);
		TWIN_DeleteObject(hDIB,FALSE);
		DeleteDC(hDCCompat);

		WinFree((LPSTR)lpbi);
	    }
	    break;

	case META_CHORD: 
		y4 = GETSHORT(buffer);
		x4 = GETSHORT(buffer+1);
		y3 = GETSHORT(buffer+2);
		x3 = GETSHORT(buffer+3);
		y2 = GETSHORT(buffer+4);
		x2 = GETSHORT(buffer+5);
		y1 = GETSHORT(buffer+6);
		x1 = GETSHORT(buffer+7);
		Chord(hDC, x1, y1, x2, y2, x3, y3, x4, y4);
		break;

	case META_CREATEBITMAP:
		break;

	case META_CREATEBITMAPINDIRECT:
		break;

	case META_CREATEBRUSH:
		break;

	case META_CREATEBRUSHINDIRECT:
		if ((slot = TWIN_MetaFindFreeSlot(lpht, nHandles)) == -1)
			return;
		brush.lbStyle = GETWORD(buffer);
		buffer++;
		brush.lbColor = GETDWORD(buffer);
		brush.lbHatch = GETSHORT(buffer+2);
		if (0 == (hObject = (HGDIOBJ)CreateBrushIndirect(&brush)))
		    break;
		PUTWORD(&lpht->objectHandle[slot],hObject);
		break;

	case META_CREATEFONTINDIRECT:
		if ((slot = TWIN_MetaFindFreeSlot(lpht, nHandles)) == -1)
			return;
		GetLOGFONT(&lf, (LPBYTE)buffer);
		if (0 == (hObject = (HGDIOBJ)CreateFontIndirect(&lf)))
		    break;
		PUTWORD(&lpht->objectHandle[slot],hObject);
		break;

	case META_CREATEPALETTE:
		if ((slot = TWIN_MetaFindFreeSlot(lpht, nHandles)) == -1)
			return;
		nPaletteLength = GETSHORT(buffer+1);
		lpLogPalette = (LPLOGPALETTE)WinMalloc(
			nPaletteLength*sizeof(PALETTEENTRY)+2*sizeof(WORD));
		lpLogPalette->palVersion = GETSHORT(buffer);
		lpLogPalette->palNumEntries = nPaletteLength;
		memcpy((LPSTR)&lpLogPalette->palPalEntry[0],
			(LPSTR)(buffer+2),
			nPaletteLength*sizeof(PALETTEENTRY));
		if (0 == (hObject = (HGDIOBJ)CreatePalette(lpLogPalette))) {
		    WinFree((LPSTR)lpLogPalette);
		    break;
		}
		PUTWORD(&lpht->objectHandle[slot],hObject);
		break;

	case META_CREATEPATTERNBRUSH:
		if ( (slot = TWIN_MetaFindFreeSlot(lpht, nHandles)) == -1 )
			return;
		bm.bmType = 0;
		bm.bmWidth = GETWORD(buffer);
		bm.bmHeight = GETWORD(buffer+1);
		bm.bmWidthBytes = GETWORD(buffer+2);
		bm.bmPlanes = *((LPBYTE)(buffer+3));
		bm.bmBitsPixel = *((LPBYTE)(buffer+3)+1);
		bm.bmBits = (LPVOID)(buffer+5);
		hBitmap = CreateBitmapIndirect(&bm);
		if (0 == (hObject = CreatePatternBrush(hBitmap))) {
		    TWIN_DeleteObject(hBitmap,FALSE);
		    break;
		}
		PUTWORD(&lpht->objectHandle[slot],hObject);
		break;

	case META_DIBCREATEPATTERNBRUSH:
		nPatternType = GETSHORT(buffer);
		if (nPatternType != BS_PATTERN &&
		    nPatternType != BS_DIBPATTERN) {
		    break;
		}
		if ( (slot = TWIN_MetaFindFreeSlot(lpht, nHandles)) == -1 )
			return;
		uColorUse = GETWORD(buffer+1);
		bmi.biSize = sizeof(BITMAPINFOHEADER);
		bmi.biWidth = GETDWORD(buffer+4);
		bmi.biHeight = GETDWORD(buffer+6);
		bmi.biPlanes = GETWORD(buffer+8);
		bmi.biBitCount = GETWORD(buffer+9);
		bmi.biCompression = GETDWORD(buffer+10);
		bmi.biSizeImage = GETDWORD(buffer+12);
		bmi.biXPelsPerMeter = GETDWORD(buffer+14);
		bmi.biYPelsPerMeter = GETDWORD(buffer+16);
		bmi.biClrUsed = GETDWORD(buffer+18);
		bmi.biClrImportant = GETDWORD(buffer+20);
		nNumColors = 1 << bmi.biBitCount;
		lpRGB = (LPRGBQUAD)(buffer+22);
		if (nPatternType == BS_PATTERN) {
		    bi.bmp_hdr = &bmi;
		    bi.rgb_quad = lpRGB;
		    bi.bitmap_bits = (LPBYTE)(buffer+22) +
				nNumColors*sizeof(RGBQUAD);
		    hBitmap = CreateDIBitmapEx(&bi,0);
		    if (0 == (hObject = CreatePatternBrush(hBitmap))) {
			TWIN_DeleteObject(hBitmap,FALSE);
			break;
		    }
		}
		if (nPatternType == BS_DIBPATTERN) {
		    nWidth = CalcByteWidth(bmi.biWidth,bmi.biBitCount,32)*
				bmi.biHeight;
		    hDIB = GlobalAlloc(GHND,sizeof(BITMAPINFOHEADER)+
				nNumColors*sizeof(RGBQUAD)+ nWidth);
		    lpDIB = (LPBYTE)GlobalLock(hDIB);
		    *((LPBITMAPINFOHEADER)lpDIB) = bmi;
		    memcpy((LPSTR)lpDIB+sizeof(BITMAPINFOHEADER),(LPSTR)lpRGB,
				nNumColors*sizeof(RGBQUAD));
		    memcpy((LPSTR)lpDIB+sizeof(BITMAPINFOHEADER)+
				nNumColors*sizeof(RGBQUAD),
				(LPSTR)(buffer+22) +
					nNumColors*sizeof(RGBQUAD), nWidth);
		    GlobalUnlock(hDIB);
		    hObject = CreateDIBPatternBrush(hDIB,uColorUse);
		    GlobalFree(hDIB);
		    if (hObject == 0)
			break;
		}
		PUTWORD(&lpht->objectHandle[slot],hObject);
		break;

	case META_CREATEPENINDIRECT:
		if ( (slot = TWIN_MetaFindFreeSlot(lpht, nHandles)) == -1 )
			return;
		pen.lopnStyle   = GETWORD(buffer);
		pen.lopnWidth.x = GETSHORT(buffer+1);
		pen.lopnWidth.y = GETSHORT(buffer+2);
		buffer += 3;
		pen.lopnColor   = GETDWORD(buffer);
		if (0 == (hObject = (HGDIOBJ)CreatePenIndirect(&pen)))
		    break;
		PUTWORD(&lpht->objectHandle[slot],hObject);
		break;

	case META_CREATEREGION:
		break;

	case META_DELETEOBJECT:
		slot = GETWORD(buffer);
		MetaDeleteObject(lpht,slot,TRUE);
		break;

	case META_DRAWTEXT:
		break;

	case META_ELLIPSE: 
		y2 = GETSHORT(buffer);
		x2 = GETSHORT(buffer+1);
		y1 = GETSHORT(buffer+2);
		x1 = GETSHORT(buffer+3);
		Ellipse(hDC, x1, y1, x2, y2);
		break;

	case META_ENDDOC:
		break;

	case META_ENDPAGE:
		break;

	case META_ESCAPE: 
		nEscape = GETSHORT(buffer);
		nCount  = GETSHORT(buffer+1);
		lpEscapeData = (LPBYTE)WinMalloc(nCount);
		memcpy((LPSTR)lpEscapeData,(LPSTR)(buffer+2),nCount);
		Escape(hDC, nEscape, nCount, (LPCSTR)lpEscapeData, NULL);
		WinFree((LPSTR)lpEscapeData);
		break;

	case META_EXCLUDECLIPRECT: 
		y2 = GETSHORT(buffer);
		x2 = GETSHORT(buffer + 1);
		y1 = GETSHORT(buffer + 2);
		x1 = GETSHORT(buffer + 3);
		ExcludeClipRect(hDC, x1, y1, x2, y2);
		break;

	case META_EXTFLOODFILL:
		w = GETWORD(buffer);
		dw = GETDWORD(buffer+1);
		y1 = GETSHORT(buffer+3);
		x1 = GETSHORT(buffer+4);
		ExtFloodFill(hDC, x1, y1, dw, (UINT)w);
		break;

	case META_EXTTEXTOUT: 
		iy     = GETSHORT(buffer);
		ix     = GETSHORT(buffer+1);
		nCount = GETWORD(buffer+2);
		w      = GETWORD(buffer+3);
		buffer += 4;
		if (w & ETO_CLIPPED) {
		    RECT_TO_C(rect, (LPBYTE)buffer);
		    buffer += 4;
		}
		else
		    SetRectEmpty(&rect);
		lpStr = (LPSTR)buffer;
		/* check for character spacing array */
		buffer += (nCount+1) >> 1;
		if ( (LPBYTE)buffer < (LPBYTE)lpBRec+2*dwSize )
		    lpInt = hsw_ConvertArrayToInt((LPBYTE)buffer, nCount);
		else
		    lpInt = 0;
		ExtTextOut(hDC, ix, iy, w, &rect, lpStr, nCount, lpInt);
		break;

	case META_FILLREGION: {
		HRGN hRgn;
		HBRUSH hBrush;

		slot = GETWORD(buffer);
		hRgn = (HRGN)GETWORD(&lpht->objectHandle[slot]);
		slot = GETWORD(buffer+1);
		hBrush = (HBRUSH)GETWORD(&lpht->objectHandle[slot]);
		FillRgn(hDC,hRgn,hBrush);
		break;
		}

	case META_FLOODFILL: 
		dw = GETDWORD(buffer);
		y1 = GETSHORT(buffer+2);
		x1 = GETSHORT(buffer+3);
		FloodFill(hDC, x1, y1, dw);
		break;

	case META_FRAMEREGION:
		break;

	case META_INTERSECTCLIPRECT: 
		y2 = GETSHORT(buffer);
		x2 = GETSHORT(buffer+1);
		y1 = GETSHORT(buffer+2);
		x1 = GETSHORT(buffer+3);
		IntersectClipRect(hDC, x1, y1, x2, y2);
		break;

	case META_INVERTREGION: {
		HRGN hRgn;

		slot = GETWORD(buffer);
		hRgn = (HRGN)GETWORD(&lpht->objectHandle[slot]);
		InvertRgn(hDC,hRgn);
		break;
		}

	case META_LINETO: 
		y1 = GETSHORT(buffer);
		x1 = GETSHORT(buffer+1);
		LineTo(hDC, x1, y1);
		break;

	case META_MOVETO: 
		y1 = GETSHORT(buffer);
		x1 = GETSHORT(buffer+1);
		MoveTo(hDC, x1, y1);
		break;

	case META_OFFSETCLIPRGN: 
		y1 = GETSHORT(buffer);
		x1 = GETSHORT(buffer+1);
		OffsetClipRgn(hDC, x1, y1);
		break;

	case META_OFFSETVIEWPORTORG: 
		y1 = GETSHORT(buffer);
		x1 = GETSHORT(buffer+1);
		OffsetViewportOrg(hDC, x1, y1);
		break;

	case META_OFFSETWINDOWORG: 
		y1 = GETSHORT(buffer);
		x1 = GETSHORT(buffer+1);
		OffsetWindowOrg(hDC, x1, y1);
		break;

	case META_PAINTREGION: {
		HRGN hRgn;

		slot = GETWORD(buffer);
		hRgn = (HRGN)GETWORD(&lpht->objectHandle[slot]);
		PaintRgn(hDC,hRgn);
		break;
		}

	case META_PATBLT: 
		dw = GETDWORD(buffer);
		iy = GETSHORT(buffer + 2);
		ix = GETSHORT(buffer + 3);
		y1 = GETSHORT(buffer + 4);
		x1 = GETSHORT(buffer + 5);
		PatBlt(hDC, x1, y1, ix, iy, dw);
		break;

	case META_PIE: 
		y4 = GETSHORT(buffer);
		x4 = GETSHORT(buffer+1);
		y3 = GETSHORT(buffer+2);
		x3 = GETSHORT(buffer+3);
		y2 = GETSHORT(buffer+4);
		x2 = GETSHORT(buffer+5);
		y1 = GETSHORT(buffer+6);
		x1 = GETSHORT(buffer+7);
		Pie(hDC, x1, y1, x2, y2, x3, y3, x4, y4);
		break;

	case META_POLYGON: 
		nCount = GETWORD(buffer);
		Polygon(hDC,
		    (LPPOINT)hsw_ConvertArrayToInt((LPBYTE)(buffer+1),nCount*2),
		    nCount);
		break;

	case META_POLYLINE: 
		nCount = GETWORD(buffer);
		Polyline(hDC,
		    (LPPOINT)hsw_ConvertArrayToInt((LPBYTE)(buffer+1),nCount*2),
		    nCount);
		break;

	case META_POLYPOLYGON: 
		nCount = GETWORD(buffer);
		lpInt = hsw_ConvertArrayToInt((LPBYTE)(buffer+1),nCount);
		for ( i = 0,npoints = 0; i < (int)nCount; i++ )
			npoints += lpInt[i];
		buffer += nCount + 1;
		lpPolyCount = (LPINT)WinMalloc(nCount*sizeof(int));
		memcpy((LPSTR)lpPolyCount,(LPSTR)lpInt,nCount*sizeof(int));
		PolyPolygon(hDC,
		    (LPPOINT)hsw_ConvertArrayToInt((LPBYTE)buffer,npoints*2),
		    lpPolyCount,nCount);
		WinFree((LPSTR)lpPolyCount);
		break;

	case META_REALIZEPALETTE: 
		RealizePalette(hDC);
		break;

	case META_RECTANGLE: 
		y2 = GETSHORT(buffer);
		x2 = GETSHORT(buffer+1);
		y1 = GETSHORT(buffer+2);
		x1 = GETSHORT(buffer+3);
		Rectangle(hDC, x1, y1, x2, y2);
		break;

	case META_RESETDC:
		break;

	case META_RESIZEPALETTE: 
		break;

	case META_RESTOREDC: 
		nSavedDC = (int)GETSHORT(buffer);
		RestoreDC(hDC, nSavedDC);
		break;

	case META_ROUNDRECT: 
		y3 = GETSHORT(buffer);
		x3 = GETSHORT(buffer+1);
		y2 = GETSHORT(buffer+2);
		x2 = GETSHORT(buffer+3);
		y1 = GETSHORT(buffer+4);
		x1 = GETSHORT(buffer+5);
		RoundRect(hDC, x1, y1, x2, y2, x3, y3);
		break;

	case META_SAVEDC: 
		SaveDC(hDC);
		break;

	case META_SCALEVIEWPORTEXT: 
		y2 = GETSHORT(buffer);
		y1 = GETSHORT(buffer+1);
		x2 = GETSHORT(buffer+2);
		x1 = GETSHORT(buffer+3);
		ScaleViewportExt(hDC, x1, x2, y1, y2);
		break;

	case META_SCALEWINDOWEXT: 
		y2 = GETSHORT(buffer);
		y1 = GETSHORT(buffer+1);
		x2 = GETSHORT(buffer+2);
		x1 = GETSHORT(buffer+3);
		ScaleWindowExt(hDC, x1, x2, y1, y2);
		break;

	case META_SELECTCLIPREGION:
		slot = GETWORD(buffer);
		SelectObject(hDC, GETWORD(&lpht->objectHandle[slot]));
		break;

	case META_SELECTOBJECT: 
		slot = GETWORD(buffer);
		SelectObject(hDC, GETWORD(&lpht->objectHandle[slot]));
		break;

	case META_SELECTPALETTE: 
		slot = GETWORD(buffer);
		SelectPalette(hDC, GETWORD(&lpht->objectHandle[slot]),FALSE);
		break;

	case META_SETBKCOLOR: 
		dw = GETDWORD(buffer);
		SetBkColor(hDC, dw);
		break;

	case META_SETBKMODE: 
		nBkMode = GETSHORT(buffer);
		SetBkMode(hDC, nBkMode);
		break;

	case META_SETDIBTODEV: {
		UINT fuColorUse;
		UINT numlines,startline;
		int XSrc, YSrc, XDest, YDest, cxDest, cyDest;
		int nColors;
		LPBITMAPINFO lpbmi;
		LPVOID lpvBits;

		fuColorUse = GETWORD(buffer);
		numlines = GETWORD(buffer+1);
		startline = GETWORD(buffer+2);
		YSrc = GETSHORT(buffer+3);
		XSrc = GETSHORT(buffer+4);
		cyDest = GETSHORT(buffer+5);
		cxDest = GETSHORT(buffer+6);
		YDest = GETSHORT(buffer+7);
		XDest = GETSHORT(buffer+8);
		buffer += 9;

		nColors = 1 << (int)(short)GETWORD((LPBYTE)buffer+14);
		lpbmi = (LPBITMAPINFO)WinMalloc(sizeof(BITMAPINFO) +
			sizeof(RGBQUAD) * (nColors-1));
		GetBITMAPINFO(lpbmi,(LPBYTE)buffer);
		lpvBits = (LPBYTE)buffer + SIZEOF_BITMAPINFOHEADER_BIN +
				SIZEOF_RGBQUAD_BIN * nColors;
		SetDIBitsToDevice(hDC, XDest, YDest, cxDest, cyDest,
			XSrc, YSrc, startline, numlines, lpvBits, lpbmi,
			fuColorUse);

		WinFree((LPSTR)lpbmi);

		break;
		}

	case META_SETMAPMODE: 
		nMapMode = GETWORD(buffer);
		SetMapMode(hDC, nMapMode);
		break;

	case META_SETMAPPERFLAGS: 
		dw = GETDWORD(buffer);
		SetMapperFlags(hDC, dw);
		break;

	case META_SETPALENTRIES: {
		break;
		}

	case META_SETPIXEL: 
		dw = GETDWORD(buffer);
		y1 = GETSHORT(buffer+2);
		x1 = GETSHORT(buffer+3);
		SetPixel(hDC, x1, y1, dw);
		break;

	case META_SETPOLYFILLMODE: 
		nPolyFillMode = GETSHORT(buffer);
		SetPolyFillMode(hDC, nPolyFillMode);
		break;

	case META_SETRELABS:
		break;

	case META_SETROP2: 
		nDrawMode = GETSHORT(buffer);
		SetROP2(hDC, nDrawMode);
		break;

	case META_SETSTRETCHBLTMODE: 
		nStretchMode = GETSHORT(buffer);
		SetStretchBltMode(hDC, nStretchMode);
		break;

	case META_SETTEXTALIGN: 
		w = GETWORD(buffer);
		SetTextAlign(hDC, w);
		break;

	case META_SETTEXTCHAREXTRA: 
		x1 = GETSHORT(buffer);
		SetTextCharacterExtra(hDC, x1);
		break;

	case META_SETTEXTCOLOR: 
		dw = GETDWORD(buffer);
		SetTextColor(hDC, dw);
		break;

	case META_SETTEXTJUSTIFICATION: 
		iInt2 = GETSHORT(buffer);
		iInt1 = GETSHORT(buffer + 1);
		SetTextJustification(hDC, iInt1, iInt2);
		break;

	case META_SETVIEWPORTEXT: 
		iInt2 = GETSHORT(buffer);
		iInt1 = GETSHORT(buffer + 1);
		SetViewportExt(hDC, iInt1, iInt2);
		break;

	case META_SETVIEWPORTORG: 
		iInt2 = GETSHORT(buffer);
		iInt1 = GETSHORT(buffer + 1);
		SetViewportOrg(hDC, iInt1, iInt2);
		break;

	case META_SETWINDOWEXT: 
		y = GETSHORT(buffer);
		x = GETSHORT(buffer+1);
		SetWindowExt(hDC, x, y);
		break;

	case META_SETWINDOWORG: 
		y = GETSHORT(buffer);
		x = GETSHORT(buffer+1);
		SetWindowOrg(hDC, x, y);
		break;

	case META_STARTDOC:
		break;

	case META_STARTPAGE:
		break;

	case META_STRETCHBLT: {
		BITMAP		bm;
		HDC		hDCCompat;
		HBITMAP		hDDB, hBmpOld;
		DWORD		dwRop;
		int		XSrc,YSrc,XDest,YDest,cxDest,cyDest,cxSrc,cySrc;

		dwRop  = GETDWORD(buffer);
		cySrc = GETSHORT(buffer+2);
		cxSrc = GETSHORT(buffer+3);
		YSrc   = GETSHORT(buffer+4);
		XSrc   = GETSHORT(buffer+5);
		cyDest = GETSHORT(buffer+6);
		cxDest = GETSHORT(buffer+7);
		YDest  = GETSHORT(buffer+8);
		XDest  = GETSHORT(buffer+9);
		buffer += 10;

		bm.bmType = 0;
		bm.bmWidth = GETSHORT(buffer);
		bm.bmHeight = GETSHORT(buffer+1);
		bm.bmWidthBytes = GETSHORT(buffer+2);
		bm.bmPlanes = GETSHORT(buffer+3);
		bm.bmBitsPixel = GETSHORT(buffer+4);
		bm.bmBits = (LPVOID)(buffer+5);

		hDDB = CreateBitmapIndirect(&bm);

		hDCCompat = CreateCompatibleDC(hDC);
		hBmpOld = SelectObject(hDCCompat, hDDB);
		StretchBlt(hDC, XDest, YDest, cxDest, cyDest,
			hDCCompat, XSrc, YSrc, cxSrc, cySrc, dwRop);

		SelectObject(hDCCompat, hBmpOld);
		TWIN_DeleteObject(hDDB,FALSE);
		DeleteDC(hDCCompat);
	    }
	    break;

	case META_STRETCHDIB: {
		DWORD dwRop;
		UINT fuColorUse;
		int cySrc,cxSrc,YSrc,XSrc,cyDest,cxDest,YDest,XDest;
		int nColors;
		LPBITMAPINFO lpbmi;
		LPVOID lpvBits;

		dwRop = GETDWORD(buffer);
		buffer += 2;
		fuColorUse = (UINT)GETSHORT(buffer);
		buffer++;
		cySrc = GETSHORT(buffer);
		cxSrc = GETSHORT(buffer+1);
		YSrc = GETSHORT(buffer+2);
		XSrc = GETSHORT(buffer+3);
		cyDest = GETSHORT(buffer+4);
		cxDest = GETSHORT(buffer+5);
		YDest = GETSHORT(buffer+6);
		XDest = GETSHORT(buffer+7);
		buffer += 8;
		nColors = 1 << (int)(short)GETWORD((LPBYTE)buffer+14);
		lpbmi = (LPBITMAPINFO)WinMalloc(sizeof(BITMAPINFO) +
			sizeof(RGBQUAD) * (nColors-1));
		GetBITMAPINFO(lpbmi,(LPBYTE)buffer);
		lpvBits = (LPBYTE)buffer + SIZEOF_BITMAPINFOHEADER_BIN +
				SIZEOF_RGBQUAD_BIN * nColors;

		StretchDIBits(hDC,XDest,YDest,cxDest,cyDest,
			XSrc,YSrc,cxSrc,cySrc,
			lpvBits,lpbmi,fuColorUse,dwRop);

		WinFree((LPSTR)lpbmi);

		}
		break;

	case META_TEXTOUT: 
		nCount = GETWORD(buffer);
		lpStr = (LPSTR)(buffer + 1);
		buffer += (nCount / 2) + (nCount & 1) + 1;
		y1 = GETSHORT(buffer);
		x1 = GETSHORT(buffer+1);
		TextOut(hDC, x1, y1, (LPCSTR)lpStr, nCount);
		break;

	default:
		break;
	}
	return;
}


/* -------------------------------------------------------------------- */
/* Play metafile							*/
/* -------------------------------------------------------------------- */
static BOOL
InternalEnumMF(HDC hDC, HMETAFILE hMF, MFENUMPROC lpfnCallback, LPARAM lpData)
{
	DWORD		dwSize;
	LPWORD		lpMF, lpRec;
	LPHANDLETABLE	lpht;
	DWORD		mtSize, mtMaxRecord;
	WORD		mtNoObjects, rdFunc;
	int		dcID, i, rc = 0, nRecCount = 0;

	if ( !IsValidMetaFile(hMF)  ||  !(lpMF = (LPWORD)GlobalLock(hMF)) )
		return FALSE;

	/* Get some information from the metafile header */
	mtSize      = GETDWORD(&(lpMF[MF_SIZE]));
	mtNoObjects = GETWORD(&(lpMF[MF_NOOBJECTS]));
	mtMaxRecord = GETDWORD(&(lpMF[MF_MAXRECORD]));

	/* Allocate and initialize objects handle table */
	lpht = (LPHANDLETABLE)WinMalloc(sizeof(HANDLETABLE) * mtNoObjects);
	if ( lpht == NULL ) {
		GlobalUnlock(hMF);
		return FALSE;
	}

	TWIN_InitHandleTable(lpht, sizeof(HANDLETABLE) * mtNoObjects);
	dwSize = mtMaxRecord * sizeof(WORD);
	if (!IsMetafileDC(hDC))
	    dcID = SaveDC(hDC);	/* Save DC state before playing metafile */
	else
	    dcID = 0;

	/* lpRec points to binary metafile in METAFILE structure */
	lpRec = (LPWORD)(&(lpMF[MF_RECORD]));
	rdFunc = GETWORD(&lpRec[RDFUNC]);
	dwSize = GETDWORD(&lpRec[RDSIZE]);
	if ((lpRec < lpMF+mtSize) && rdFunc) {
	    do {
		/* Invoke intermediate callback function, which in turn
		   calls either PlayMetaFile(), or application callback
		   function.
		*/
		rc = lpfnCallback(hDC, lpht, (LPMETARECORD)lpRec, mtNoObjects,
				lpData);
		if ( !rc )
			break;	/* Stop enumeration */

		lpRec += dwSize;
		nRecCount++;

		/* Get size of next record */
		dwSize = GETDWORD(&lpRec[RDSIZE]);
		rdFunc = GETWORD(&lpRec[RDFUNC]);
	    } while (rdFunc && (lpRec < lpMF+mtSize));
	}
	if (!IsMetafileDC(hDC))
	    RestoreDC(hDC, dcID);  /* Restore DC state after PlayMetaFile */

	for (i = 0; i < (int)mtNoObjects; i++)
	    MetaDeleteObject(lpht,i,FALSE);

	WinFree((char*)lpht);

	GlobalUnlock(hMF);
	return rc;
}


/* -------------------------------------------------------------------- */
/* This function is called as a result of PlayMetaFile:			*/
/*	PlayMetaFile() -> InternalEnumMF() ->				*/
/*			-> this function -> PlayMetaFileRecord		*/
/* -------------------------------------------------------------------- */
static int
EnumMFRecords(HDC hDC, LPHANDLETABLE lpht, LPMETARECORD lpRec,
              int mtNoObjects, LPARAM lParam)
{
	PlayMetaFileRecord(hDC, lpht, lpRec, mtNoObjects);
	return 1;
}

/* -------------------------------------------------------------------- */
/* Play metafile							*/
/* -------------------------------------------------------------------- */
BOOL WINAPI
PlayMetaFile(HDC hDC, HMETAFILE hMF)
{
	return InternalEnumMF(hDC, hMF, (MFENUMPROC)EnumMFRecords, 0);
}

/* -------------------------------------------------------------------- */
/* Enumerate metafile							*/
/* -------------------------------------------------------------------- */
BOOL WINAPI
EnumMetaFile(HDC hDC, HMETAFILE hMF, MFENUMPROC mfenmprc, LPARAM lParam)
{
	return InternalEnumMF(hDC, hMF, mfenmprc, lParam);
}

static BOOL
MetaDeleteObject(LPHANDLETABLE lpht, int slot, BOOL fNotifyMetafile)
{
    HGDIOBJ hObject;
    HBITMAP hBitmap;
    LPOBJHEAD	lpObjHead;

    hObject = (HGDIOBJ)GETWORD(&lpht->objectHandle[slot]);
    if (hObject == 0 || 0 == (lpObjHead = GETGDIINFO(hObject)))
	return FALSE;

    if (GET_OBJECT_TYPE(lpObjHead) == OT_BRUSH &&
	((LPBRUSHINFO)lpObjHead)->lpBrush.lbStyle == BS_PATTERN)
	hBitmap = (HBITMAP)((LPBRUSHINFO)lpObjHead)->lpBrush.lbHatch;
    else
	hBitmap = 0;

    TWIN_DeleteObject(hObject,fNotifyMetafile);
    if (hBitmap)
	TWIN_DeleteObject(hBitmap,fNotifyMetafile);

    lpht->objectHandle[slot] = 0;

    RELEASEGDIINFO(lpObjHead);
    return TRUE;
}
