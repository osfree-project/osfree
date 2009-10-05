/*    
	PrinterGraphics.c	2.24
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
#include "print.h"

#include "Driver.h"
#include "GdiDC.h"
#include "DeviceData.h"
#include "GdiDDK.h"
#include "PrinterDC.h"
#include "Log.h"

static void GetPrinterTextMetrics(HDC32, LPTEXTMETRIC);
static void PrinterValidate(HDC32, DWORD);

/* this is exported to PrinterDC.c */
int TWIN_PrinterEscape(HDC32, int, const LPVOID, LPVOID);

extern int CalcByteWidth(int,int,int);
extern LONG TWIN_GetImageBits(HBITMAP, LONG, LPVOID, int);

DWORD
lsd_printer_getdevcaps(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    GDIINFO *ginfo;
    int rval;

    ginfo = &(lppddc->GdiInfo);

    switch (dwParam) {
	case DRIVERVERSION:
	    rval = ginfo->dpVersion;
	    break;

	case TECHNOLOGY:
	    rval = ginfo->dpTechnology;
	    break;

	case HORZSIZE:
	    rval = ginfo->dpHorzSize;
	    break;

	case VERTSIZE:
	    rval = ginfo->dpVertSize;
	    break;

	case HORZRES:
	    rval = ginfo->dpHorzRes;
	    break;

	case VERTRES:
	    rval = ginfo->dpVertRes;
	    break;

	case LOGPIXELSX:
	    rval = ginfo->dpLogPixelsX;
	    break;

	case LOGPIXELSY:
	    rval = ginfo->dpLogPixelsY;
	    break;

	case BITSPIXEL:
	    rval = ginfo->dpBitsPixel;
	    break;

	case PLANES:
	    rval = ginfo->dpPlanes;
	    break;

	case NUMBRUSHES:
	    rval = ginfo->dpNumBrushes;
	    break;

	case NUMPENS:
	    rval = ginfo->dpNumPens;
	    break;

	case NUMMARKERS:
	    rval = ginfo->dpNumMarkers;
	    break;

	case NUMFONTS:
	    rval = ginfo->dpNumFonts;
	    break;

	case NUMCOLORS:
	    rval = ginfo->dpNumColors;
	    break;

	case ASPECTX:
	    rval = ginfo->dpAspectX;
	    break;

	case ASPECTY:
	    rval = ginfo->dpAspectY;
	    break;

	case ASPECTXY:
	    rval = ginfo->dpAspectXY;
	    break;

	case PDEVICESIZE:
	    rval = ginfo->dpDEVICEsize;
	    break;

	case CLIPCAPS:
	    rval = ginfo->dpClip;
	    break;

	case SIZEPALETTE:
	    rval = ginfo->dpSizePallete;
	    break;

	case NUMRESERVED:
	    rval = ginfo->dpNumReserved;
	    break;

	case COLORRES:
	    rval = ginfo->dpColorRes;
	    break;

	case RASTERCAPS:
	    rval = ginfo->dpRaster;
	    break;

	case CURVECAPS:
	    rval = ginfo->dpCurves;
	    break;

	case LINECAPS:
	    rval = ginfo->dpLines;
	    break;

	case POLYGONALCAPS:
	    rval = ginfo->dpPolygonals;
	    break;

	case TEXTCAPS:
	    rval = ginfo->dpText;
	    break;

	default:
	    return 0L;
	}
    return (DWORD)rval;
}

DWORD
lsd_printer_getnearclr(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    COLORREF cr = lpStruct->lsde.colorref;
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    PCOLORINFOPROC lpfnColorInfo;

    if (!(lpfnColorInfo = (PCOLORINFOPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_COLORINFO)))
	FatalAppExit(0,"Printer driver does not have COLORINFO caps...\n");

    return lpfnColorInfo(lppddc->pDevice,cr,0);
}

DWORD
lsd_printer_enumobjects(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LSDE_ENUMOBJ *lpeo = &lpStruct->lsde.enumobj;
    PENUMOBJPROC lpfnEnumObj;

    if ((lpfnEnumObj = (PENUMOBJPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_ENUMOBJ)))
	return lpfnEnumObj(lppddc->pDevice, lpeo->fnObjectType,
		lpeo->goenmprc, lpeo->lParam);
    else
	FatalAppExit(0,"Printer driver does not have ENUMOBJ caps...\n");
    return 0;
}

DWORD
lsd_printer_moveto(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    POINT oldPt;

    /* dwParam -- get/set flag  */
    oldPt = lppddc->cpt;
    if (dwParam == LSDM_SET)
	lppddc->cpt = lpStruct->lsde.point;
    lpStruct->lsde.point = oldPt;
    return 1L;
}

DWORD
lsd_printer_output(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    POUTPUTPROC lpfnOutput;

    /* getting driver's OUTPUT proc address by ordinal is much faster */
    if (!(lpfnOutput = (POUTPUTPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_OUTPUT)))
	FatalAppExit(0,"Printer driver does not have OUTPUT caps...\n");

    switch (msg) {
	case LSD_LINETO: {
	    POINT oldPt, Points[2];

	    /* dwParam -- RelAbs flag */
	    oldPt = lppddc->cpt;
	    if (dwParam == RELATIVE) {
		lppddc->cpt.x = oldPt.x + lpStruct->lsde.point.x;
		lppddc->cpt.y = oldPt.y + lpStruct->lsde.point.y;
	    } else {	/* ABSOLUTE mode */
		lppddc->cpt.x = lpStruct->lsde.point.x;
		lppddc->cpt.y = lpStruct->lsde.point.y;
	    }

	    Points[0] = oldPt;
	    Points[1] = lppddc->cpt;

	    if ((hDC32->dwInvalid & (IM_PENMASK|IM_ROP2MASK)))
		PrinterValidate(hDC32, IM_PENMASK|IM_ROP2MASK);

	    return lpfnOutput(lppddc->pDevice, OS_POLYLINE, 2, Points,
			lppddc->lpPhPen, lppddc->lpPhBrush, &lppddc->DrawMode,
			NULL);
	    }

	case LSD_POLYLINE:
	    /* dwParam -- RelAbs flag */
	    if ((hDC32->dwInvalid & (IM_PENMASK|IM_ROP2MASK)))
		PrinterValidate(hDC32, IM_PENMASK|IM_ROP2MASK);
	    /* we do have the array of points ready here */
	    return lpfnOutput(lppddc->pDevice, OS_POLYLINE,
			lpStruct->lsde.polypoly.nTotalCount,
			lpStruct->lsde.polypoly.lpPoints,
			lppddc->lpPhPen, lppddc->lpPhBrush,
			&lppddc->DrawMode, NULL);

	case LSD_RECTANGLE:
	case LSD_ELLIPSE: {
	    RECT Rect;

	    Rect.left = lpStruct->lsde.rect.left;
	    Rect.right = lpStruct->lsde.rect.right;
	    Rect.top = lpStruct->lsde.rect.top;
	    Rect.bottom = lpStruct->lsde.rect.bottom;

	    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK))
		PrinterValidate(hDC32, IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK);

	    return lpfnOutput(lppddc->pDevice,
			(msg == LSD_RECTANGLE)?OS_RECTANGLE:OS_ELLIPSE,
			2, (LPPOINT)&Rect, lppddc->lpPhPen, lppddc->lpPhBrush, 
			&lppddc->DrawMode, NULL);
	    }

	case LSD_ROUNDRECT:
	    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK))
		PrinterValidate(hDC32, IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK);
	    return lpfnOutput(lppddc->pDevice, OS_ROUNDRECT,
			3, lpStruct->lsde.arc, lppddc->lpPhPen,
			lppddc->lpPhBrush, 
			&lppddc->DrawMode, NULL);

	case LSD_ARC:
	case LSD_CHORD:
	case LSD_PIE:
	    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK))
		PrinterValidate(hDC32, IM_PENMASK|IM_BRUSHMASK|IM_ROP2MASK);
	    return lpfnOutput(lppddc->pDevice,
			(msg == LSD_ARC) ? OS_ARC :
		        (msg == LSD_PIE) ? OS_PIE :
					   OS_CHORD,
			4, lpStruct->lsde.arc, lppddc->lpPhPen,
			lppddc->lpPhBrush, 
			&lppddc->DrawMode, NULL);

	case LSD_POLYGON:
	    /* dwParam -- RelAbs flag */
	    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|
				IM_POLYFILLMODEMASK|IM_ROP2MASK))
		PrinterValidate(hDC32, IM_PENMASK|IM_BRUSHMASK|
				IM_POLYFILLMODEMASK|IM_ROP2MASK);
	    /* we do have the array of points ready here */
	    return lpfnOutput(lppddc->pDevice, 
			(hDC32->PolyFillMode == ALTERNATE)?
				OS_ALTPOLYGON:OS_WINDPOLYGON,
			lpStruct->lsde.polypoly.nTotalCount,
			lpStruct->lsde.polypoly.lpPoints,
			lppddc->lpPhPen, lppddc->lpPhBrush,
			&lppddc->DrawMode, NULL);

	case LSD_POLYPOLYGON: {
	    int i, nCount;
	    DWORD dwRet=0;

	    /* dwParam -- RelAbs flag */
	    if (hDC32->dwInvalid & (IM_PENMASK|IM_BRUSHMASK|
				IM_POLYFILLMODEMASK|IM_ROP2MASK))
		PrinterValidate(hDC32, IM_PENMASK|IM_BRUSHMASK|
				IM_POLYFILLMODEMASK|IM_ROP2MASK);
	    for (i=0,nCount=0; i<lpStruct->lsde.polypoly.nCount;i++) {
		if (!(dwRet = lpfnOutput(lppddc->pDevice,
			(hDC32->PolyFillMode == ALTERNATE)?
				OS_ALTPOLYGON:OS_WINDPOLYGON,
			lpStruct->lsde.polypoly.lpCounts[i],
			&lpStruct->lsde.polypoly.lpPoints[nCount],
			lppddc->lpPhPen, lppddc->lpPhBrush,
			&lppddc->DrawMode, NULL)))
		    break;
		nCount += lpStruct->lsde.polypoly.lpCounts[i];
	    }
	    return dwRet;
	    }

	default:
	    return 0L;
    }
}

DWORD
lsd_printer_extfloodfill(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return(0);
}

DWORD
lsd_printer_textout(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LSDE_TEXT *lpText = &lpStruct->lsde.text;
    LPDFONTINFO lpfi;
    PEXTTEXTOUTPROC lpfnExtTextOut;
    int x, y;
    WORD xext = 0;

    if (!(lpfnExtTextOut=(PEXTTEXTOUTPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_EXTTEXTOUT)))
	FatalAppExit(0,"Printer driver does not have EXTTEXTOUT caps...\n");

    if (hDC32->dwInvalid & IM_TEXTMASK)
	PrinterValidate(hDC32, IM_TEXTMASK);

    if ( hDC32->TextAlign & TA_UPDATECP ) {
	x = lppddc->cpt.x;
	y = lppddc->cpt.y;
    } else {
	x = lpText->x;
	y = lpText->y;
    }

    /* TA_LEFT == 0, so check for others */
    /* also if TA_UPDATECP get extents now */
    if ( hDC32->TextAlign & (TA_CENTER | TA_RIGHT) ) {
	if ( (xext = LOWORD(lpfnExtTextOut(lppddc->pDevice, x, y,
		      (lppddc->fClippingSet)?&lppddc->rcClipping:0,
		      lpText->lpStr, -lpText->nCnt, lppddc->lpPhFont,
		      &lppddc->DrawMode, &lppddc->txf, NULL, NULL, 0))) )
	    if ( hDC32->TextAlign & TA_CENTER)
		x -= xext/2;
	    else		/* TA_RIGHT */
		x -= xext;
    }

    lpfi  = (LPDFONTINFO)lppddc->lpPhFont;

    /* TA_TOP is 0 and TA_BASELINE includes TA_BOTTOM */
    if ( (hDC32->TextAlign & TA_BASELINE) == TA_BASELINE )
	    y -= lpfi->dfAscent;
    if ( (hDC32->TextAlign & TA_BASELINE) == TA_BOTTOM )
	    y -= lpfi->dfPixHeight;

    if ( hDC32->TextAlign & TA_UPDATECP )
	lppddc->cpt.x += xext;

    return lpfnExtTextOut(lppddc->pDevice, x, y,
		(lppddc->fClippingSet)?&lppddc->rcClipping:0,
		lpText->lpStr, lpText->nCnt, lppddc->lpPhFont,
		&lppddc->DrawMode, &lppddc->txf, NULL, NULL, 0);
}

DWORD
lsd_printer_exttextout(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LSDE_TEXT *lpText = &lpStruct->lsde.text;
    LPDFONTINFO lpfi;
    PEXTTEXTOUTPROC lpfnExtTextOut;

    if (!(lpfnExtTextOut=(PEXTTEXTOUTPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_EXTTEXTOUT)))
	FatalAppExit(0,"Printer driver does not have EXTTEXTOUT caps...\n");

    if (hDC32->dwInvalid & IM_TEXTMASK)
	PrinterValidate(hDC32, IM_TEXTMASK);

    lpfi  = (LPDFONTINFO)lppddc->lpPhFont;

    return lpfnExtTextOut(lppddc->pDevice,
		lpText->x, lpText->y,
		(lppddc->fClippingSet)?&lppddc->rcClipping:0,
		lpText->lpStr, lpText->nCnt, lppddc->lpPhFont,
		&lppddc->DrawMode, &lppddc->txf, lpText->lpDX,
		lpText->lpRect, lpText->uiFlags);
}

DWORD
lsd_printer_getcharwidth(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LSDE_TEXT *lptp = &lpStruct->lsde.text;
    PGETCHARWIDTHPROC lpfnCharWidth;
    int i,nCharRange,nElementSize;
    LPBYTE lp;

    if (!hDC32->lpNTM || (hDC32->dwInvalid & IM_TEXTMETRICMASK))
	lsd_printer_gettextmetrics(0,hDC32,0,0);

    nCharRange = hDC32->lpNTM->tmLastChar - hDC32->lpNTM->tmFirstChar + 1;
    nElementSize = (hDC32->uiFontType == TRUETYPE_FONTTYPE)?
			sizeof(ABC):sizeof(int);

    if (!hDC32->lpCharWidths) {
	LSDS_PARAMS argptr;

	if (!(lpfnCharWidth = (PGETCHARWIDTHPROC)
			GetProcAddress(lppddc->hModuleDriver,ORD_CHARWIDTHS)))
	    FatalAppExit(0,"Printer driver missing GETCHARWIDTH caps...\n");

	hDC32->lpCharWidths = (LPVOID)WinMalloc(nCharRange*nElementSize);
	LOGSTR((LF_LOG,
		"lsd_printer_getcharwidth: hDC32 %p malloc lpCharWidths %p\n",
		hDC32, hDC32->lpCharWidths));

	argptr.lsde_validate.dwInvalidMask = 0;

	argptr.lsde.text.lpDX = hDC32->lpCharWidths;

	if ( !lpfnCharWidth(lppddc->pDevice,
		hDC32->lpCharWidths,
		hDC32->lpNTM->tmFirstChar,
		hDC32->lpNTM->tmLastChar,
		lppddc->lpPhFont,
		&lppddc->DrawMode,
		&lppddc->txf) ) {
		LOGSTR((LF_LOG,
			"lsd_printer_getcharwidth: hDC32 %p free lpCharWidths %p\n",
			hDC32, hDC32->lpCharWidths));
	    WinFree((LPSTR)hDC32->lpCharWidths);
	    hDC32->lpCharWidths = (LPVOID)0;
	    return (DWORD)FALSE;
	}
    }

    for (i = lptp->x; i <= lptp->y; i++) {
	if (i < (int)hDC32->lpNTM->tmFirstChar ||
			i > (int)hDC32->lpNTM->tmLastChar)
	    lp = (LPBYTE)((LPSTR)hDC32->lpCharWidths +
		nElementSize*(hDC32->lpNTM->tmDefaultChar -
		hDC32->lpNTM->tmFirstChar));
	else
	    lp = (LPBYTE)((LPSTR)hDC32->lpCharWidths +
	 	nElementSize*(i - hDC32->lpNTM->tmFirstChar));
	if (hDC32->uiFontType != TRUETYPE_FONTTYPE)
	    lptp->lpDX[i-lptp->x] = *(LPINT)lp;
	else {
	    if (dwParam)
		((LPABC)lptp->lpDX)[i-lptp->x] = *(LPABC)lp;
	    else
		lptp->lpDX[i-lptp->x] = ((LPABC)lp)->abcB;
	}
    }

    return (DWORD)TRUE;
}

DWORD
lsd_printer_getaspectratio(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return(0);
}

static void
GetPrinterTextMetrics(HDC32 hDC32, LPTEXTMETRIC lpTM)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LPDFONTINFO	lpfi;
    LPTEXTXFORM	lptxf;
    int iSC1, iSC2, iExtraWidth;

    /* The data for TEXTMETRIC is taken from */
    /* FONTINFO and TEXTXFORM structures.    */
	
    lpfi  = (LPDFONTINFO)lppddc->lpPhFont;
    lptxf = &lppddc->txf;

    iSC1 = lptxf->txfHeight;
    iSC2 = lpfi->dfPixHeight;

#define SCALEVALUE(x) MulDiv((x), iSC1, iSC2)

    lpTM->tmHeight           = lptxf->txfHeight;
    lpTM->tmAscent           = SCALEVALUE(lpfi->dfAscent);
    lpTM->tmDescent          = SCALEVALUE(lpfi->dfPixHeight - lpTM->tmAscent);
    lpTM->tmInternalLeading  = SCALEVALUE(lpfi->dfInternalLeading);
    lpTM->tmExternalLeading  = SCALEVALUE(lpfi->dfExternalLeading);

    iExtraWidth = (lptxf->txfAccelerator & TC_EA_DOUBLE) ? 1 : 0;
    lpTM->tmAveCharWidth     = lptxf->txfWidth + iExtraWidth;
    lpTM->tmMaxCharWidth     = MulDiv(lpfi->dfMaxWidth, lptxf->txfWidth,
					lpfi->dfAvgWidth) + iExtraWidth;

    lpTM->tmWeight           = lptxf->txfWeight;
    lpTM->tmItalic           = lptxf->txfItalic;
    lpTM->tmUnderlined       = lptxf->txfUnderline;
    lpTM->tmStruckOut        = lptxf->txfStrikeOut;
    lpTM->tmFirstChar        = lpfi->dfFirstChar;
    lpTM->tmLastChar         = lpfi->dfLastChar;
    lpTM->tmDefaultChar      = lpfi->dfDefaultChar;
    lpTM->tmBreakChar        = lpfi->dfBreakChar;

    /* The algorithm for calculating tmPitchaAndFamily is not completely   */
    /* obvious. It depends on the values in [BP-9] and [BP-A] which in     */
    /* turn depend on the flags in internal GDI's DC structure. The nature */
    /* of these flags is not clear (it might be indication of IC or        */
    /* whatever). In my case [BP-9] and [BP-A] were 1 and 0 respectively.  */
    {
    	int bp_9, bp_a, al, cx=0;

    	bp_9 = 1;
    	bp_a = 0;
    	al = lpfi->dfPitchAndFamily & 0xF1;
    	if ( bp_9 ) {
	    al |= 0x08;
	    cx = 3;
    	}

    	if ( !bp_a ) {
	    cx = lpfi->dfType & 0x03;
    	}

    	lpTM->tmPitchAndFamily = al | (cx << 1);
    }

    lpTM->tmCharSet          = lpfi->dfCharSet;
    lpTM->tmOverhang         = lptxf->txfOverhang;
    lpTM->tmDigitizedAspectX = lpfi->dfHorizRes;
    lpTM->tmDigitizedAspectY = lpfi->dfVertRes;
}

DWORD
lsd_printer_gettextmetrics(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    if (!hDC32->lpNTM) {
	hDC32->lpNTM = (LPNEWTEXTMETRIC)WinMalloc(sizeof(NEWTEXTMETRIC));
	hDC32->dwInvalid |= IM_TEXTMETRICMASK;
    }

    if (hDC32->dwInvalid & IM_TEXTMETRICMASK)
	GetPrinterTextMetrics(hDC32, (LPTEXTMETRIC)hDC32->lpNTM);

    if ( lpStruct && lpStruct->lsde.lpmetric )
	memcpy((LPSTR)lpStruct->lsde.lpmetric, (LPSTR)hDC32->lpNTM, sizeof(TEXTMETRIC));

    hDC32->dwInvalid &= ~IM_TEXTMETRICMASK;

    return 1L;
}

DWORD
lsd_printer_enumfonts(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LSDE_ENUMFONTS *lpef = &lpStruct->lsde.enumfonts;
    PFONTENUMPROC lpfnEnumObj;

    if ((lpfnEnumObj = (PFONTENUMPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_ENUMDFONTS)))
	return lpfnEnumObj(lppddc->pDevice, lpef->lpszFamily,
		lpef->fntenmprc, lpef->lParam);
    else
	FatalAppExit(0,"Printer driver does not have ENUMFONTS caps...\n");
    return 0;
}

DWORD
lsd_printer_gettextface(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LPDFONTINFO lpfi;
    LPSTR lpFace;
    int nLen;

    /* dwParam -- length of the buffer in lpStruct */
    if (!(lpfi = (LPDFONTINFO)lppddc->lpPhFont))
	return 0L;

    lpFace = (LPSTR)lpfi + lpfi->dfFace;
    nLen = min((int)strlen(lpFace),(int)(dwParam-1));
    strncpy(lpStruct->lsde.text.lpStr,lpFace,nLen);
    lpStruct->lsde.text.lpStr[nLen] = '\0';

    return (DWORD)nLen;
}

DWORD
lsd_printer_stretchblt(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LSDE_STRETCHDATA *lpsd = &lpStruct->lsde.stretchdata;
    PBITBLTPROC lpfnBitBlt;
    PSTRETCHBLTPROC lpfnStretchBlt=NULL;
    RECT rcClip;
    POINT pt;
    SIZE sz;
    DWORD dwRet,dwSize;
    LPVOID lpSrc = 0;
    PSBITMAP psbmp;

    /* lpStruct -- points to LSDS_STRETCHDATA */
    if (hDC32->dwInvalid & (IM_BRUSHMASK|IM_TEXTCOLORMASK|IM_BKCOLORMASK))
	PrinterValidate(hDC32, IM_BRUSHMASK|IM_TEXTCOLORMASK|IM_BKCOLORMASK);

    if (msg == LSD_PATBLT) {
	lpsd->nWidthSrc = lpsd->nWidthDest;
	lpsd->nHeightSrc = lpsd->nHeightDest;
    }
    else
	if (lpsd->hSrcDC32) {
	    /* true BitBlt or StretchBlt with source DC */
	    pt.x = lpsd->xSrc;
	    pt.y = lpsd->ySrc;
	    LPtoDP(GETHDC16(lpsd->hSrcDC32),&pt,1);
	    lpsd->xSrc = pt.x;
	    lpsd->ySrc = pt.y;

	    sz.cx = lpsd->nWidthSrc;
	    sz.cy = lpsd->nHeightSrc;
	    LEtoDE(lpsd->hSrcDC32, &sz);
	    lpsd->nWidthSrc = sz.cx;
	    lpsd->nHeightSrc = sz.cy;
	}

    memset((LPSTR)&psbmp,'\0',sizeof(PSBITMAP));
    if (lpsd->hSrcDC32) {
	if (lpsd->hSrcDC32->dwDCXFlags & DCX_COMPATIBLE_DC) {
	    if (lpsd->hSrcDC32->dwDCXFlags & DCX_PRINTER_DC)
		lpSrc = (LPVOID)
		    ((LPPRINTERDRIVERDC)lpsd->hSrcDC32->lpDrvData)->pDevice;
	    else {
		if (lpsd->hSrcDC32->dwDCXFlags & DCX_DISPLAY_DC) {
		    GetObject((HGDIOBJ)lpsd->hSrcDC32->hBitmap,
			sizeof(BITMAP),(LPBITMAP)&psbmp);
		    psbmp.bm.bmWidthBytes =
			CalcByteWidth(psbmp.bm.bmWidth,psbmp.bm.bmBitsPixel,32);
		    dwSize = psbmp.bm.bmWidthBytes*psbmp.bm.bmHeight;
		    psbmp.bm.bmBits = (LPVOID)WinMalloc(dwSize);
		    TWIN_GetImageBits((HBITMAP)lpsd->hSrcDC32->hBitmap,
				dwSize,psbmp.bm.bmBits,32);
		    lpSrc = (LPVOID)&psbmp;
		}
		else
		    FatalAppExit(0,
			"lsd_printer_stretchblt: unknown DC type");
	    }
	}
	else {
	    psbmp.bm.bmType = 1;
	    lpSrc = (LPVOID)&psbmp;
	}
    }
    else
	lpSrc = 0;

    /* if driver has a bitblt capability and bitblt has not been */
    /* transformed to stretchblt by mapping mode, try BITBLT */
    if ((lppddc->GdiInfo.dpRaster & RC_BITBLT) &&
	((msg == LSD_PATBLT) ||
	 ((lpsd->nWidthSrc == lpsd->nWidthDest) &&
	 (lpsd->nHeightSrc == lpsd->nHeightDest)))) {
	if ((lpfnBitBlt = (PBITBLTPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_BITBLT))) {
	    dwRet = lpfnBitBlt(lppddc->pDevice,lpsd->xDest,lpsd->yDest,
		lpSrc,
		lpsd->xSrc,lpsd->ySrc,
		lpsd->nWidthDest,lpsd->nHeightDest,
		lpsd->dwRop,
		lppddc->lpPhBrush,&lppddc->DrawMode);
	    if (psbmp.bm.bmBits)
		WinFree((LPSTR)psbmp.bm.bmBits);
	    return dwRet;
	}
    }

    /* BitBlt() does not cut it. Try StretchBlt(). */
    if (!(lppddc->GdiInfo.dpRaster & RC_STRETCHBLT) ||
	!(lpfnStretchBlt = (PSTRETCHBLTPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_STRETCHBLT)))
	/* driver does not has STRETCHBLT caps. What do we do??? */
	FatalAppExit(0,"Printer driver does not have STRETCHBLT caps...\n");

    SetRect(&rcClip,lpsd->xDest,lpsd->yDest,
		lpsd->xDest+lpsd->nWidthDest,
		lpsd->yDest+lpsd->nHeightDest);

    if (lppddc->fClippingSet)
	IntersectRect(&rcClip,&rcClip,&lppddc->rcClipping);

    dwRet = (DWORD)lpfnStretchBlt(lppddc->pDevice,lpsd->xDest,lpsd->yDest,
		lpsd->nWidthDest,lpsd->nHeightDest,
		lpSrc,
		lpsd->xSrc,lpsd->ySrc,
		lpsd->nWidthSrc,lpsd->nHeightSrc,
		lpsd->dwRop,
		lppddc->lpPhBrush,
		&lppddc->DrawMode,
		&rcClip);

    if (psbmp.bm.bmBits)
	WinFree((LPSTR)psbmp.bm.bmBits);

    if (dwRet != (DWORD)-1)
	return dwRet;
    else
	return (lppddc->pDevice->pdType == 0)?
		(DWORD)dmStretchBlt((LPVOID)lppddc->pDevice,
			lpsd->xDest,lpsd->yDest,
			lpsd->nWidthDest,lpsd->nHeightDest,
			lpSrc,
			lpsd->xSrc,lpsd->ySrc,
			lpsd->nWidthSrc,lpsd->nHeightSrc,
			lpsd->dwRop,
			lppddc->lpPhBrush,
			&lppddc->DrawMode):
		0L;
}

DWORD
lsd_printer_stretchdibits(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LSDE_STRETCHDATA *lpsd = &lpStruct->lsde.stretchdata;
    PSTRETCHDIBPROC lpfnStretchDIB;
    RECT rcClip;
    DWORD dwRet;

    if (hDC32->dwInvalid & IM_BRUSHMASK)
	PrinterValidate(hDC32, IM_BRUSHMASK);

    if (lppddc->GdiInfo.dpRaster & RC_STRETCHDIB) {
	if (!(lpfnStretchDIB = (PSTRETCHDIBPROC)GetProcAddress(
		lppddc->hModuleDriver,ORD_STRETCHDIB)))
	    dwRet = (DWORD)-1;
	else {
	    SetRect(&rcClip,lpsd->xDest,lpsd->yDest,
		lpsd->xDest+lpsd->nWidthDest,
		lpsd->yDest+lpsd->nHeightDest);

	    if (lppddc->fClippingSet)
		IntersectRect(&rcClip,&rcClip,&lppddc->rcClipping);

	    dwRet = lpfnStretchDIB(lppddc->pDevice,
			0,	/* always 0 in 3.x */
			lpsd->xDest, lpsd->yDest,
			lpsd->nWidthDest,lpsd->nHeightDest,
			lpsd->xSrc,lpsd->ySrc,
			lpsd->nWidthSrc,lpsd->nHeightSrc,
			(LPVOID)lpsd->lpvBits,
			lpsd->lpbmi,
			0,	/* drivers using color palettes??? */
			lpsd->dwRop,
			lppddc->lpPhBrush,
			&lppddc->DrawMode,
			&rcClip);
	}
    }
    else
	dwRet = (DWORD)-1;

    if (dwRet == (DWORD)-1) {
	/* GDI has to simulate the call */
	FatalAppExit(0,"Printer driver does not have STRETCHDIB caps...\n");
    } 
    return dwRet;
}

DWORD
lsd_printer_pixel(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LPPOINT lppt = &lpStruct->lsde.point;
    PPIXELPROC lpfnPixel;
    PCOLORINFOPROC lpfnColorInfo;
    COLORREF cr;
    DWORD dwRet;

    if (!(lpfnPixel = (PPIXELPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_PIXEL)))
	FatalAppExit(0,"Printer driver does not have PIXEL caps...\n");

    if (msg == LSD_SETPIXEL) {
	if (hDC32->dwInvalid & IM_ROP2MASK)
	    PrinterValidate(hDC32, IM_ROP2MASK);
	if (!(lpfnColorInfo = (PCOLORINFOPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_COLORINFO)))
	    FatalAppExit(0,"Printer driver does not have COLORINFO caps...\n");

	cr = lpfnColorInfo(lppddc->pDevice,dwParam,0);
	dwRet = lpfnPixel(lppddc->pDevice,lppt->x,lppt->y,
			cr,&lppddc->DrawMode);
    }
    else
	dwRet = lpfnPixel(lppddc->pDevice,lppt->x,lppt->y,
			0L, 0L);

    return (dwRet == 0x80000000)?(DWORD)-1:dwRet;
}

DWORD
lsd_printer_setdibtodevice(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LSDE_STRETCHDATA *lpsd = &lpStruct->lsde.stretchdata;
    PSETDIBTODEVPROC lpfnSetDIBToDev;
    RECT rcClip;
    DWORD dwRet;

    if (lppddc->GdiInfo.dpRaster & RC_DIBTODEV) {
	if (!(lpfnSetDIBToDev = (PSETDIBTODEVPROC)GetProcAddress(
		lppddc->hModuleDriver,ORD_SETDIBTODEV)))
	    dwRet = (DWORD)-1;
	else {
	    SetRect(&rcClip,lpsd->xDest,lpsd->yDest,
		lpsd->xDest+lpsd->nWidthDest,
		lpsd->yDest+lpsd->nHeightDest);

	    if (lppddc->fClippingSet)
		IntersectRect(&rcClip,&rcClip,&lppddc->rcClipping);

#ifdef LATER
	    snafu with source rectangle size
#endif

	    dwRet = lpfnSetDIBToDev(lppddc->pDevice,
			lpsd->xDest, lpsd->yDest,
			lpsd->startline,lpsd->numlines,
			&rcClip,
			&lppddc->DrawMode,
			(LPVOID)lpsd->lpvBits,
			lpsd->lpbmi,
			0);	/* drivers using color palettes??? */
	}
    }
    else
	dwRet = (DWORD)-1;

    if (dwRet == (DWORD)-1) {
	/* GDI has to simulate the call */
	FatalAppExit(0,"Printer driver does not have DIBTODEV caps...\n");
    } 
    return dwRet;
}

DWORD
lsd_printer_startdoc(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
#ifdef	macintosh

/*	TODO:	Do something with the DOCINFO that is passed to StartDoc() */
	if ( DRVCALL_PRINTER( PPH_STARTDOC, NULL, NULL, hDC32->lpDrvData ) != 0 )
		return( SP_ERROR );
	return( 1 );
#else	/* macintosh */
    LPDOCINFO lpdi = lpStruct->lsde.lpdocinfo;

    return (DWORD)TWIN_PrinterEscape(hDC32, STARTDOC,
		(const LPVOID)lpdi->lpszDocName,
		(LPVOID)lpdi);
#endif	/* else macintosh */
}

DWORD
lsd_printer_startpage(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
#ifdef	macintosh

/*	TODO:	Do something with the DOCINFO that is passed to StartDoc() */
	if ( DRVCALL_PRINTER( PPH_STARTPAGE, NULL, NULL, hDC32->lpDrvData ) != 0 )
		return( SP_ERROR );
	return( 1 );
#else	/* macintosh */
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;

    if ( lppddc == NULL )
	return (DWORD)0;

    lppddc->bDisableResetDC = TRUE;

    return (DWORD)1;
#endif	/* else macintosh */
}

DWORD
lsd_printer_endpage(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
#ifdef	macintosh
/*	TODO:	Do something with the DOCINFO that is passed to StartDoc() */
	if ( DRVCALL_PRINTER( PPH_ENDPAGE, NULL, NULL, hDC32->lpDrvData ) != 0 )
		return( SP_ERROR );
	return( 1 );
#else	/* macintosh */
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;

    if ( lppddc == NULL )
	return (DWORD)0;

    lppddc->bDisableResetDC = FALSE;

    return (DWORD)TWIN_PrinterEscape(hDC32, NEWFRAME, 0, 0);
#endif	/* else macintosh */
}

DWORD
lsd_printer_enddoc(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
#ifdef	macintosh
	if ( DRVCALL_PRINTER( PPH_ENDDOC, NULL, NULL, hDC32->lpDrvData ) != 0 )
		return( SP_ERROR );
	return( 1 );
#else	/* macintosh */
    return (DWORD)TWIN_PrinterEscape(hDC32, ENDDOC, 0, 0);
#endif	/* else macintosh */
}

DWORD
lsd_printer_abortdoc(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return(0);
}

DWORD
lsd_printer_setabortproc(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return(0);
}

DWORD
lsd_printer_queryabort(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return(0);
}

DWORD
lsd_printer_escape(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LSDE_ESCAPEDATA *lpesc = &lpStruct->lsde.escape;


    return (DWORD)TWIN_PrinterEscape(hDC32, (int)dwParam,
			(const LPVOID)lpesc->lpszInData, lpesc->lpvOutData);
}

DWORD
lsd_printer_scrolldc(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return(0);
}

static void
PrinterValidate(HDC32 hDC32, DWORD dwInvalidMask)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    DWORD dwMask = 0;

    if ( (dwMask = hDC32->dwInvalid & dwInvalidMask) ) {
	lppddc->DrawMode.Rop2 = hDC32->ROP2;
	lppddc->DrawMode.bkMode = hDC32->BackMode;
	lppddc->DrawMode.bkColor = hDC32->BackColor;
	lppddc->DrawMode.TextColor = hDC32->TextColor;
	lppddc->DrawMode.TBreakExtra = hDC32->nBreakExtra;
	lppddc->DrawMode.BreakErr = hDC32->nErrorTerm;
	if ( hDC32->nBreakCount ) {
	    lppddc->DrawMode.BreakExtra = hDC32->nBreakExtra/hDC32->nBreakCount;
	    lppddc->DrawMode.BreakRem = hDC32->nBreakExtra%hDC32->nBreakCount;
	} else {
	    lppddc->DrawMode.BreakExtra = 0;
	    lppddc->DrawMode.BreakRem = 0;
	}
	lppddc->DrawMode.BreakCount = hDC32->nBreakCount;
	lppddc->DrawMode.CharExtra = hDC32->nCharExtra;
	lppddc->DrawMode.LbkColor = hDC32->BackColor;
	lppddc->DrawMode.LTextColor = hDC32->TextColor;

	hDC32->dwInvalid &= ~dwMask;
    }
/*    FatalAppExit(0,"PrinterValidate called!!!"); */
}

int
TWIN_PrinterEscape(HDC32 hDC32, int nEsc,
		const LPVOID lpInData, LPVOID lpOutData)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    PCONTROLPROC lpfnControl;
    LPSTR lpIn = NULL;			/* or lpIn = lpInData ljw 02/10/95 */
    EXTTEXTDATA	extTxtData;
    APPEXTTEXTDATA appExtTextData;
    int rval;

    if (!(lpfnControl = (PCONTROLPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_CONTROL)))
	FatalAppExit(0,"Printer driver does not have CONTROL caps...\n");

    switch ( nEsc ) {
	case MFCOMMENT:
	    return 0;

	case SETABORTPROC:
	    lppddc->lpfnAbortProc = (ABORTPROC)lpInData;
	    lpIn = (LPSTR)&hDC32->ObjHead.hObj;		/* (GETHDC16(hDC32)) */
	    break;

	case NEWFRAME:
	    if ( lppddc->lpfnAbortProc )
		(lppddc->lpfnAbortProc)(GETHDC16(hDC32), 0);
	    break;

	case ENABLEPAIRKERNING:
	case SETKERNTRACK:
	    /* Special case (see DDK source control.c) */
	    /* AmiPro                                  */
	    extTxtData.nSize      = sizeof(EXTTEXTDATA);
	    extTxtData.lpInData   = (LPAPPEXTTEXTDATA)lpInData;
	    lpIn = (LPSTR)&extTxtData;
	    break;

	case GETEXTENDEDTEXTMETRICS:
	    extTxtData.nSize      = sizeof(EXTTEXTDATA);
	    extTxtData.lpInData   = &appExtTextData;
	    extTxtData.lpFont     = lppddc->lpPhFont;
	    extTxtData.lpXForm    = &lppddc->txf;
	    extTxtData.lpDrawMode = &lppddc->DrawMode;

	    /* Put the size of the output structure (ETM) */
	    appExtTextData.x = *(short*)lpInData;
	    lpIn = (LPSTR)&extTxtData;
	    break;

	default:
	    lpIn = (LPSTR)lpInData;
    }

    rval = lpfnControl(lppddc->pDevice, nEsc, lpIn, lpOutData);

    if ( nEsc == NEWFRAME )		/* Call it one more time */
	if ( lppddc->lpfnAbortProc )
    	    (lppddc->lpfnAbortProc)(GETHDC16(hDC32), 0);

    return rval;
}

DWORD
lsd_printer_worldblt(WORD wMsg,
	HDC32 hDC32,
	DWORD dwParam,
	LPLSDS_PARAMS lpParam)
{
	return(0);
}

DWORD
lsd_printer_systempaletteuse(WORD wMsg,
	LPDC lpDC,
	DWORD dwParam,
	LPLSDS_PARAMS lpParam)
{
	/* Get/Set system palette use.
	 *
	 * STUB: We simulate proper behavior by saving the value for a later
	 * get request.  Of course, the rest of the printer driver ignores
	 * the saved value.
	 */

	static UINT uPrinterSystemPaletteUse = SYSPAL_STATIC;
	UINT uSystemPaletteUse = (UINT) dwParam;
	BOOL bSetSystemPaletteUse = (BOOL) lpParam;
	UINT uOldSystemPaletteUse;

	uOldSystemPaletteUse = uPrinterSystemPaletteUse;

	if (bSetSystemPaletteUse)
	{
		uPrinterSystemPaletteUse = uSystemPaletteUse;
	}

	return(uOldSystemPaletteUse);

}

DWORD
lsd_printer_getsystempalette(WORD wMsg,
	LPDC lpDC,
	DWORD dwParam,
	LPLSDS_PARAMS lpParam)
{
	return ((HPALETTE) NULL);
}

DWORD
lsd_printer_updatecolors(WORD wMsg,
	LPDC lpDC,
	DWORD dwParam,
	LPLSDS_PARAMS lpParam)
{
	return ((BOOL) FALSE);
}
