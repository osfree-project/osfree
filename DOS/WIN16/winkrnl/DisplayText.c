/*    
	DisplayText.c	2.40
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

#include "windows.h"

#include "Log.h"
#include "GdiDC.h"
#include "GdiText.h"
#include "DeviceData.h"
#include "Driver.h"

#include <string.h>

static BOOL DisplayValidateFont(HDC32);
static DWORD lsd_display_enumfonts_callback(LPVOID, LSDS_ENUMFONTSCALLBACK *);
static BOOL DisplayValidateFont(HDC32);

void DisplayValidate(HDC32,LPLSDS_PARAMS,DWORD);
void MM_RescaleNEWTEXTMETRIC(HDC32, LPNEWTEXTMETRIC, int);

DWORD
lsd_display_getglyphoutline(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    if (hDC32->dwInvalid & (IM_FONTMASK|IM_FONTSCALEMASK))
	if (!DisplayValidateFont(hDC32))
	    return 0L;
	
	return DRVCALL_TEXT(PTH_GETGLYPHOUTLINE,hDC32->lpDrvData,0,lpStruct);
}


DWORD
lsd_display_gettextmetrics(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LSDS_PARAMS argptr;

    if (hDC32->dwInvalid & (IM_FONTMASK|IM_FONTSCALEMASK))
	if (!DisplayValidateFont(hDC32))
	    return 0L;
    
    if (!hDC32->lpNTM) {
	hDC32->lpNTM = (LPNEWTEXTMETRIC)WinMalloc(sizeof(NEWTEXTMETRIC));
	hDC32->dwInvalid |= IM_TEXTMETRICMASK;
    }

    if (hDC32->dwInvalid & IM_TEXTMETRICMASK) {
        HFONT32 hFont32;
	ASSERT_HFONT(hFont32,hDC32->hFont,FALSE);
	if (!hFont32->lpNTM) {
	    hFont32->lpNTM = (LPNEWTEXTMETRIC)WinMalloc(sizeof(NEWTEXTMETRIC));
	    argptr.lsde_validate.dwInvalidMask = 0;
	    argptr.lsde.lpmetric = (LPTEXTMETRIC)hFont32->lpNTM;
	    if (!DRVCALL_TEXT(PTH_GETMETRICS,hDC32->lpDrvData,0,&argptr)) {
	        RELEASEFONTINFO(hFont32);
		return 0L;
	    }
	}
	*hDC32->lpNTM = *hFont32->lpNTM;
	RELEASEFONTINFO(hFont32);
    }

    if ( lpStruct && lpStruct->lsde.lpmetric )
	memcpy(lpStruct->lsde.lpmetric, hDC32->lpNTM, sizeof(TEXTMETRIC));

    hDC32->dwInvalid &= ~IM_TEXTMETRICMASK;

    return 1L;
}

DWORD
lsd_display_getcharwidth(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LSDE_TEXT *lptp = &lpStruct->lsde.text;
    int i,nCharRange,nElementSize;
    DWORD dwSize;
    LPBYTE lp;

    if (hDC32->dwInvalid & (IM_FONTMASK|IM_FONTSCALEMASK))
	if (!DisplayValidateFont(hDC32))
	    return 0L;
    
    if (dwParam && (hDC32->uiFontType != TRUETYPE_FONTTYPE))
	return 0L;

    if (!hDC32->lpNTM || (hDC32->dwInvalid & IM_TEXTMETRICMASK))
	lsd_display_gettextmetrics(0,hDC32,0,0);

    nCharRange = hDC32->lpNTM->tmLastChar-hDC32->lpNTM->tmFirstChar+1;
    nElementSize = (hDC32->uiFontType == TRUETYPE_FONTTYPE)?
			sizeof(ABC):sizeof(int);

    if (!hDC32->lpCharWidths) {
	LSDS_PARAMS argptr;
	HFONT32 hFont32;

	dwSize = (DWORD)(nCharRange*nElementSize);
	hDC32->lpCharWidths = (LPVOID)WinMalloc(dwSize);

    /* If there is a cached array of char widths in the font, use it */
    /* Otherwise, get a new one from the driver and cache it for future use */

	ASSERT_HFONT(hFont32,hDC32->hFont,0L);

	if (hFont32->lpCharWidths) {
	    memcpy(hDC32->lpCharWidths, hFont32->lpCharWidths, dwSize);
	}
	else {
	    argptr.lsde_validate.dwInvalidMask = 0;
	    argptr.lsde.text.lpDX = hDC32->lpCharWidths;

	    if (!DRVCALL_TEXT(PTH_GETCHARWIDTH,
			hDC32->lpDrvData, dwSize, &argptr)) {
		WinFree((LPSTR)hDC32->lpCharWidths);
		hDC32->lpCharWidths = (LPVOID)0;
		RELEASEFONTINFO(hFont32);
		return 0L;
	    }
	    hFont32->lpCharWidths = (LPVOID)WinMalloc(dwSize);
	    memcpy(hFont32->lpCharWidths, hDC32->lpCharWidths, dwSize);
	}
        RELEASEFONTINFO(hFont32);
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
		lptp->lpDX[i-lptp->x] = 
		    ((LPABC)lp)->abcA + ((LPABC)lp)->abcB + ((LPABC)lp)->abcC;
	}
    }
    return 1L;
}

DWORD
lsd_display_gettextface(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    if (hDC32->dwInvalid & (IM_FONTMASK|IM_FONTSCALEMASK))
	if (!DisplayValidateFont(hDC32))
	    return 0L;
    
    return DRVCALL_TEXT(PTH_GETTEXTFACE,hDC32->lpDrvData, dwParam, lpStruct);
}

DWORD
lsd_display_textout(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LSDE_TEXT *lptp = &lpStruct->lsde.text;

    if (hDC32->dwInvalid & (IM_FONTMASK|IM_FONTSCALEMASK))
	if (!DisplayValidateFont(hDC32))
	    return 0L;

/* Temporary fix - Bit should be invalided somewhere else. */
   hDC32->dwInvalid |= IM_ROP2MASK;
/* Temporary fix - Bit should be invalided somewhere else. */

    if (hDC32->dwInvalid & IM_TEXTMASK || hDC32->dwInvalid & IM_ROP2MASK )
	  DisplayValidate(hDC32, lpStruct,IM_TEXTMASK | IM_ROP2MASK);
    
    lptp->TextAlign = hDC32->TextAlign;
    lptp->nBreakExtra = hDC32->nBreakExtra;
    lptp->nBreakCount = hDC32->nBreakCount;
    lptp->nErrorTerm = hDC32->nErrorTerm;
    lptp->nCharExtra = hDC32->nCharExtra;
    lptp->lpExtraSpace = hDC32->lpExtraSpace;
    lptp->chBreak = ' ';

    return DRVCALL_TEXT(PTH_EXTTEXTOUT,hDC32->lpDrvData, 0L, lpStruct);
}

DWORD
lsd_display_exttextout(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LSDE_TEXT *lptp = &lpStruct->lsde.text;
    HDC hDC;
    HRGN hRgnText = 0,hRgnClip = 0;
    static HBRUSH hBrush = 0;
    static COLORREF crColor;
    RECT rcClip;
    DWORD dwRet;

    hDC = GETHDC16(hDC32);

    if (lptp->lpRect && (lptp->uiFlags & ETO_OPAQUE)) {
	if (crColor != hDC32->BackColor) {
	    crColor = hDC32->BackColor;
	    if (hBrush) {
		DeleteObject(hBrush);
		hBrush = 0;
	    }
	}
	if (hBrush == 0)
	    hBrush = CreateSolidBrush(crColor);
	FillRect(hDC,lptp->lpRect,hBrush);
    }

    if (lptp->nCnt == 0 || !lptp->lpStr)
	/* nothing else to do... */
	return 1L;

    if (lptp->lpRect && (lptp->uiFlags & ETO_CLIPPED)) {
	rcClip = *lptp->lpRect;
	LPtoDP(hDC,(LPPOINT)&rcClip,2);
	hRgnText = CreateRectRgnIndirect(&rcClip);
	hRgnClip = CreateRectRgn(0,0,0,0);

	/* get the clipping region from the DC */
	if (GetClipRgn(hDC,hRgnClip) == 1)
	    /* if we have one, then merge it with our rectangle */
	    CombineRgn(hRgnText,hRgnText,hRgnClip,RGN_AND);
	else {
	    DeleteObject(hRgnClip);
	    hRgnClip = (HRGN)0;
	}

	/* select the resulting clipping */
	lsd_display_selectcliprgn(LSD_SELECTCLIPRGN,hDC32, (DWORD)hRgnText,0);
    }

    if (hDC32->dwInvalid & (IM_FONTMASK|IM_FONTSCALEMASK))
	if (!DisplayValidateFont(hDC32))
	    return 0L;
    
/* Temporary fix - Bit should be invalided somewhere else. */
   hDC32->dwInvalid |= IM_ROP2MASK;
/* Temporary fix - Bit should be invalided somewhere else. */

    if (hDC32->dwInvalid & IM_TEXTMASK || hDC32->dwInvalid & IM_ROP2MASK )
	  DisplayValidate(hDC32, lpStruct,IM_TEXTMASK | IM_ROP2MASK);

    lptp->TextAlign = hDC32->TextAlign;
    lptp->nBreakExtra = hDC32->nBreakExtra;
    lptp->nBreakCount = hDC32->nBreakCount;
    lptp->nErrorTerm = hDC32->nErrorTerm;
    lptp->nCharExtra = hDC32->nCharExtra;
    lptp->lpExtraSpace = hDC32->lpExtraSpace;
    lptp->chBreak = ' ';

    dwRet = DRVCALL_TEXT(PTH_EXTTEXTOUT,hDC32->lpDrvData, 1L, lpStruct);

    if (lptp->lpRect && (lptp->uiFlags & ETO_CLIPPED)) {
	/* set back the original clipping region or NULL */
	lsd_display_selectcliprgn(LSD_SELECTCLIPRGN,hDC32, (DWORD)hRgnClip,0);
	if (hRgnClip)
	    DeleteObject(hRgnClip);
	DeleteObject(hRgnText);
    }

    return dwRet;
}

DWORD
lsd_display_enumfonts(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    /*   dwParam == 0   EnumFonts
    **   dwParam == 1   EnumFontFamilies
    **
    **   For TrueType and vector(?) fonts EnumFontFamProc returns NEWLOGFONT 
    **   instead of LOGFONT
    */
    LSDE_ENUMFONTS *lpenum = &lpStruct->lsde.enumfonts;
    LPCSTR lpcstr = lpenum->lpszFamily;
    FONTENUMPROC fenumproc = lpenum->fntenmprc;
    LPARAM lParam = (LPARAM)lpenum->lParam;

    int               i, status;
    DWORD             dwCount = 0;
    LPNEWLOGFONT      lplf, lplfCache;
    LPNEWTEXTMETRIC   lpntm, lpntmCache;
	

    dwCount = DRVCALL_TEXT(PTH_GETENUMCNT, hDC32->lpDrvData, dwParam, lpcstr);
    if (dwCount == 0L)
       return 0;

    if (!(lplfCache = (LPNEWLOGFONT)WinMalloc(dwCount * sizeof(NEWLOGFONT))))
       return 0L;

    memset((LPSTR)lplfCache,'\0',dwCount * sizeof(NEWLOGFONT));

    if (!(lpntmCache = (LPNEWTEXTMETRIC)WinMalloc(dwCount * sizeof(NEWTEXTMETRIC))))
       return 0L;
    memset((LPSTR)lpntmCache,'\0',dwCount * sizeof(NEWTEXTMETRIC));

    lpenum->lplfCache = lplfCache;
    lpenum->lpntmCache = lpntmCache;
    lpenum->dwCacheSize = dwCount;

    dwCount = DRVCALL_TEXT(PTH_ENUMFONTS, hDC32->lpDrvData, dwParam, lpStruct);

    for (i = 0, lplf = lplfCache,  lpntm = lpntmCache;  lplf && lpntm && i < dwCount;  i++, lplf++, lpntm++)
        {
        if (lpntm->tmHeight == 0xFFFF) /* unable to load font flag from driver */
           continue;        /* XLoadFont failed for some reason - just ignore the font... */
        if (dwParam == 0)
           status = (*fenumproc)((LPLOGFONT)lplf, lpntm, RASTER_FONTTYPE, lParam);
        else 
	   status = (*fenumproc)((LPNEWLOGFONT)lplf, lpntm, RASTER_FONTTYPE, lParam);

        if (status == 0)
           break;
        }

    WinFree((void *)lplfCache);
    WinFree((void *)lpntmCache);

    return dwCount;	
}

LSDENUMFONTSPROC lsd_display_enumfontsproc =
	(LSDENUMFONTSPROC)lsd_display_enumfonts_callback;

static DWORD
lsd_display_enumfonts_callback(LPVOID DCCookie, LSDS_ENUMFONTSCALLBACK *lpefc)
{
    if (((HDC32)DCCookie)->MapMode != MM_TEXT)
	MM_RescaleNEWTEXTMETRIC((HDC32)DCCookie,lpefc->lpntm,
		(lpefc->FontType == TRUETYPE_FONTTYPE)?1:0);

    return lpefc->fntenmprc(lpefc->lpLogFont,lpefc->lpntm,
			lpefc->FontType,lpefc->lParam);
}

static BOOL
DisplayValidateFont(HDC32 hDC32)
{
    SIZE sz;
    HFONT32 hFont32;
    LSDS_PARAMS argptr;

    ASSERT_HFONT(hFont32,hDC32->hFont,FALSE);

    sz.cx = hFont32->LogFont.lfWidth;
    sz.cy = hFont32->LogFont.lfHeight;

    LEtoDE(hDC32,&sz);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.realizefont.LogFont = hFont32->LogFont;
    argptr.lsde.realizefont.LogFont.lfHeight = sz.cy;
    argptr.lsde.realizefont.LogFont.lfWidth  = sz.cx;
    argptr.lsde.realizefont.lpMagic = (DWORD)hFont32->lpCachedFont;

    if (hDC32->dwInvalid & IM_FONTMASK) {
	/* call the driver to set the cached font into the DC */
	/* or to realize the font anew */

	if (!(hFont32->lpCachedFont = (LPVOID)DRVCALL_TEXT(PTH_REALIZE,
			hDC32->lpDrvData,
			hDC32->dwMapperFlags,
			&argptr))) {
	    RELEASEFONTINFO(hFont32);
	    return FALSE;
	}

	hDC32->uiFontType = *hFont32->lpCachedFont;
    }
    else if (hDC32->dwInvalid & IM_FONTSCALEMASK) {
	/* we only need to rescale the same font */

	if (sz.cy == hDC32->nDevHeight) {
	    hDC32->dwInvalid &= ~IM_FONTSCALEMASK;
	    RELEASEFONTINFO(hFont32);
	    return TRUE;
	}
	else {
	    if (!(hFont32->lpCachedFont = (LPVOID)DRVCALL_TEXT(PTH_RESCALE,
			hDC32->lpDrvData,
			hDC32->dwMapperFlags,
			&argptr))) {
	      RELEASEFONTINFO(hFont32);
	      return FALSE;
	    }
	    hDC32->uiFontType = *hFont32->lpCachedFont;
	}
    }

    hDC32->nDevHeight = sz.cy;
    if (hDC32->lpCharWidths) {
	WinFree((LPSTR)hDC32->lpCharWidths);
	hDC32->lpCharWidths = 0;
    }

    if (hFont32->nDevHeight != sz.cy) {
	if (hFont32->lpNTM) {
	    WinFree((LPSTR)hFont32->lpNTM);
	    hFont32->lpNTM = 0;
	}
	if (hFont32->lpCharWidths) {
	    WinFree((LPSTR)hFont32->lpCharWidths);
	    hFont32->lpCharWidths = 0;
	}
	/* save physical height in the font structure */
	hFont32->nDevHeight = sz.cy;
    }

    if (hFont32->lpNTM) {
	/* if there is a cached textmetrics info in the font, re-use it */
	/* otherwise force getting it from the driver next time it is used */

	if (!hDC32->lpNTM) 
	    hDC32->lpNTM = (LPNEWTEXTMETRIC)WinMalloc(sizeof(NEWTEXTMETRIC));
	*hDC32->lpNTM = *hFont32->lpNTM;
	hDC32->dwInvalid &= ~IM_TEXTMETRICMASK;
    }
    else
	hDC32->dwInvalid |= IM_TEXTMETRICMASK;

    hDC32->dwInvalid &= ~(IM_FONTMASK|IM_FONTSCALEMASK);

    RELEASEFONTINFO(hFont32);
    return TRUE;
}
