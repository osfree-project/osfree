/*    
	MetaGraphics.c	2.19
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
#include <string.h>

#include "kerndef.h"
#include "Endian.h" /* PUTWORD, PUTDWORD */
#include "GdiDC.h"
#include "DeviceData.h"
#include "Log.h"

#include "Meta.h"
#include "MetaGraphics.h"

/* Unimplemented features

lsd_meta_bitblt
lsd_meta_stretchblt
lsd_meta_stretchdibits
lsd_meta_setdibtodevice

lsd_meta_selectcliprgn
lsd_meta_intersectcliprgn
lsd_meta_offsetcliprgn

lsd_meta_fillrgn
lsd_meta_framergn
lsd_meta_invertrgn
lsd_meta_paintrgn

lsd_meta_startdoc
lsd_meta_startpage
lsd_meta_endpage
lsd_meta_enddoc
lsd_meta_abortdoc

*/

static HDC32 MetaDCHead = 0, MetaDCTail = 0;

static void NotifyMetafileDC(HDC32, LPOBJHEAD);
static void GenerateDeleteRecord(HDC32, int);

void
META_PUTWORD(LPVOID lpData, WORD wWord)
{
    PUTWORD(lpData, wWord);
}

void
META_PUTDWORD(LPVOID lpData, DWORD dwWord)
{
    PUTDWORD(lpData, dwWord);
}

#undef	PUTWORD
#define	PUTWORD	META_PUTWORD
#undef	PUTDWORD
#define	PUTDWORD	META_PUTDWORD

DWORD
lsd_meta_init(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_createdc(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    hDC32->dwDCXFlags |= DCX_METAFILE_DC;

    if (MetaDCTail) {
	MetaDCTail->ObjHead.lpObjNext = (LPOBJHEAD)hDC32;
	MetaDCTail = hDC32;
    }
    else
	MetaDCHead = MetaDCTail = hDC32;

    return TRUE;
}

DWORD
lsd_meta_deletedc(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    HDC32 hDC32Temp,hDC32Prev;

    for (hDC32Temp = MetaDCHead,hDC32Prev = 0;
	 hDC32Temp && hDC32Temp != hDC32;
	 hDC32Prev = hDC32Temp,hDC32Temp = (HDC32)hDC32Temp->ObjHead.lpObjNext);

    if (!hDC32Temp)
	return FALSE;

    if (hDC32Temp == MetaDCHead) {
	MetaDCHead = (HDC32)hDC32Temp->ObjHead.lpObjNext;
	if (hDC32Temp == MetaDCTail)
	    MetaDCTail = 0;
	return TRUE;
    }

    hDC32Prev->ObjHead.lpObjNext = (LPOBJHEAD)hDC32Temp->ObjHead.lpObjNext;
    if (hDC32Temp == MetaDCTail)
	MetaDCTail = hDC32Prev;

    return TRUE;
}

void
TWIN_MetaNotify(LPOBJHEAD lpObjInfo)
{
    HDC32 hDC32Temp;

    for (hDC32Temp = MetaDCHead; hDC32Temp;
	 hDC32Temp = (HDC32)hDC32Temp->ObjHead.lpObjNext)
	NotifyMetafileDC(hDC32Temp,lpObjInfo);
}

static void
NotifyMetafileDC(HDC32 hDC32, LPOBJHEAD lpObjInfo)
{
    LPINTERNALMETAFILE lpIntMeta = (LPINTERNALMETAFILE)hDC32->lpDrvData;
    int i;

    if (!lpIntMeta)
	return;

    for (i = 0; i < HANDLE_TABLE_SIZE; i++) {
	if (lpIntMeta->lpObjTable[i].hObj == (HGDIOBJ)lpObjInfo->hObj) {
	    GenerateDeleteRecord(hDC32, i);
	    lpIntMeta->lpObjTable[i].hObj = 0;
	    break;
	}
    }
}

static void
GenerateDeleteRecord(HDC32 hDC32, int slot)
{
    WORD Record[MFRS_DELETEOBJECT];
    int RDWord = RDPARAM;

    PUTDWORD(&(Record[RDSIZE]), MFRS_DELETEOBJECT);
    PUTWORD(&(Record[RDFUNC]), META_DELETEOBJECT);
    PUTWORD(&(Record[RDWord++]), slot);
    META_RECORD(Record);
    return;
}

DWORD
lsd_meta_savedc(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    WORD Record[MFRS_SAVEDC];

    PUTDWORD(&(Record[RDSIZE]), MFRS_SAVEDC);
    PUTWORD(&(Record[RDFUNC]), META_SAVEDC);
    return META_RECORD(Record);
}

DWORD
lsd_meta_restoredc(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    int	RDWord = RDPARAM;
    WORD Record[MFRS_RESTOREDC];
    HDC32 hSavedDC32 = (HDC32)dwParam;

    PUTDWORD(&(Record[RDSIZE]), MFRS_RESTOREDC);
    PUTWORD(&(Record[RDFUNC]), META_RESTOREDC);
    PUTWORD(&(Record[RDWord]), hSavedDC32->nSavedID+1);
    return META_RECORD(Record);
}

DWORD
lsd_meta_setda(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    int	RDWord = RDPARAM;
    WORD Record[5];

    switch (dwParam) {
	case LSD_DA_MAPMODE:
	case LSD_DA_POLYFILLMODE:
	case LSD_DA_BKMODE:
	case LSD_DA_TEXTALIGN:
	case LSD_DA_MAPPERFLAGS:
	case LSD_DA_STRETCHMODE:
	case LSD_DA_ROP2:
	case LSD_DA_RELABS:
	    PUTDWORD(&(Record[RDSIZE]), 4);
	    PUTWORD(&(Record[RDFUNC]), (dwParam == LSD_DA_MAPMODE)?
			META_SETMAPMODE:
			((dwParam == LSD_DA_POLYFILLMODE)?
			META_SETPOLYFILLMODE:
			((dwParam == LSD_DA_BKMODE)?
			META_SETBKMODE:
			((dwParam == LSD_DA_TEXTALIGN)?
			META_SETTEXTALIGN:
			((dwParam == LSD_DA_MAPPERFLAGS)?
			META_SETMAPPERFLAGS:
			((dwParam == LSD_DA_STRETCHMODE)?
			META_SETSTRETCHBLTMODE:
			((dwParam == LSD_DA_ROP2)?
			META_SETROP2:META_SETRELABS)))))));
	    PUTWORD(&(Record[RDWord]), lpStruct->lsde.mode);
	    break;

	case LSD_DA_BKCOLOR:
	case LSD_DA_TEXTCOLOR:
	    PUTDWORD(&(Record[RDSIZE]), 5);
	    PUTWORD(&(Record[RDFUNC]), (dwParam == LSD_DA_BKCOLOR)?
			META_SETBKCOLOR:META_SETTEXTCOLOR);
	    PUTDWORD(&(Record[RDWord]), lpStruct->lsde.colorref);
	    break;

	case LSD_DA_WINDOWORG:
	case LSD_DA_OFFSETWINDOWORG:
	case LSD_DA_VIEWPORTORG:
	case LSD_DA_OFFSETVIEWPORTORG:
	    PUTDWORD(&(Record[RDSIZE]), 5);
	    PUTWORD(&(Record[RDFUNC]), (dwParam == LSD_DA_WINDOWORG)?
		META_SETWINDOWORG:
		((dwParam == LSD_DA_OFFSETWINDOWORG)?
		META_OFFSETWINDOWORG:
		((dwParam == LSD_DA_VIEWPORTORG)?
		META_SETVIEWPORTORG:META_OFFSETVIEWPORTORG)));
	    PUTWORD(&(Record[RDWord]), lpStruct->lsde.point.y);
	    PUTWORD(&(Record[RDWord+1]), lpStruct->lsde.point.x);
	    break;

	case LSD_DA_WINDOWEXT:
	case LSD_DA_SCALEWINDOWEXT:
	case LSD_DA_VIEWPORTEXT:
	case LSD_DA_SCALEVIEWPORTEXT:
	case LSD_DA_TEXTJUST:
	    PUTDWORD(&(Record[RDSIZE]), 5);
	    PUTWORD(&(Record[RDFUNC]), (dwParam == LSD_DA_WINDOWEXT)?
		META_SETWINDOWEXT:
		((dwParam == LSD_DA_SCALEWINDOWEXT)?
		META_SCALEWINDOWEXT:
		((dwParam == LSD_DA_TEXTJUST)?
		META_SETTEXTJUSTIFICATION:
		((dwParam == LSD_DA_VIEWPORTEXT)?
		META_SETVIEWPORTEXT:META_SCALEVIEWPORTEXT))));
	    PUTWORD(&(Record[RDWord]), lpStruct->lsde.size.cy);
	    PUTWORD(&(Record[RDWord+1]), lpStruct->lsde.size.cx);
	    break;

	case LSD_DA_TEXTCHAREXTRA:
	    PUTDWORD(&(Record[RDSIZE]), 4);
	    PUTWORD(&(Record[RDFUNC]), META_SETTEXTCHAREXTRA);
	    PUTWORD(&(Record[RDWord]), lpStruct->lsde.size.cx);
	    break;

	case LSD_DA_BRUSHORG:
	case LSD_DA_DCORG:
	default:
	    return FALSE;
    }
    return META_RECORD(Record);
}

DWORD
lsd_meta_selectobject(WORD msg, HDC32 hDC32,
			DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    int	RDWord = RDPARAM;
    WORD Record[MFRS_SELECTOBJECT];
    HGDIOBJ hObject = (HGDIOBJ)dwParam;
    int slot;
    WORD wType;
    METACREATEOBJPROC lpfnCreate;
    LPVOID lpObject;
    LPOBJHEAD	lpObjInfo;
    LPINTERNALMETAFILE lpIntMeta = (LPINTERNALMETAFILE)hDC32->lpDrvData;

    if (!lpIntMeta)
	return 0L;

    lpObjInfo = GETGDIINFO(hObject);
    wType = GET_OBJECT_TYPE(lpObjInfo);

    switch (wType) {
	case OT_BRUSH:
	    hDC32->hBrush = (HBRUSH)hObject;
	    break;
	case OT_PEN:
	    hDC32->hPen = (HPEN)hObject;
	    break;
	case OT_FONT:
	    hDC32->hFont = (HFONT)hObject;
	    break;
	case OT_PALETTE:
	    hDC32->hPalette = (HPALETTE)hObject;
	    break;
	default:
	    RELEASEGDIINFO(lpObjInfo);
	    return 0L;
    }

    slot = TWIN_ControlObjTable(lpIntMeta,CHECK_HANDLE, hObject);
    if (slot < 0) {
	if (slot == NEED_TO_DELETE)
	    TWIN_ControlObjTable(lpIntMeta,FREE_OBJ,hObject);
	TWIN_GetObjInfo(hObject,&lpfnCreate,&lpObject);
	lpfnCreate(GETHDC16(hDC32),lpObject);
	slot = TWIN_ControlObjTable(lpIntMeta,STORE_OBJ,hObject);
    }

    PUTDWORD(&(Record[RDSIZE]), MFRS_SELECTOBJECT);
    PUTWORD(&(Record[RDFUNC]),
		(wType == OT_PALETTE)?META_SELECTPALETTE:META_SELECTOBJECT);
    PUTWORD(&(Record[RDWord++]), slot);
    RELEASEGDIINFO(lpObjInfo);
    return META_RECORD(Record);
}

DWORD
lsd_meta_realizepal(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    WORD Record[MFRS_REALIZEPALETTE];

    PUTDWORD(&(Record[RDSIZE]), MFRS_REALIZEPALETTE);
    PUTWORD(&(Record[RDFUNC]), META_REALIZEPALETTE);
    return META_RECORD(Record);
}

DWORD
lsd_meta_moveto(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LPPOINT	lpPoint = &lpStruct->lsde.point;
	int	RDWord = RDPARAM;
	WORD	Record[MFRS_MOVETO];

	PUTDWORD(&(Record[RDSIZE]),MFRS_MOVETO);
	PUTWORD(&(Record[RDFUNC]), (msg == LSD_MOVETO)?META_MOVETO:META_LINETO);
	PUTWORD(&(Record[RDWord++]), lpPoint->y);	/* y before x */
	PUTWORD(&(Record[RDWord++]), lpPoint->x);
	return META_RECORD(Record);
}

DWORD
lsd_meta_poly(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LSDE_POLYPOLY *lpPoly = &lpStruct->lsde.polypoly;
	LPPOINT lpPoint = lpPoly->lpPoints;
	WORD	wCount = lpPoly->nTotalCount;
	int	RDWord = RDPARAM;
	DWORD	dwRecSize;		/* in bytes for WinMalloc() */
	LPWORD	Record;
	BOOL	Return;

	dwRecSize = (MFRS_POLY * sizeof(WORD)) +
			(wCount * sizeof(DWORD));
	if ( !(Record = (LPWORD)WinMalloc(dwRecSize)) )
		return FALSE;
	PUTDWORD(&(Record[RDSIZE]), dwRecSize / sizeof(WORD));
	PUTWORD(&(Record[RDFUNC]), (msg == LSD_POLYLINE) ? 
			META_POLYLINE : META_POLYGON);
	PUTWORD(&(Record[RDWord++]), wCount);
	while (wCount--)
	{
		PUTWORD(&(Record[RDWord++]), lpPoint->x);	/* x before y */
		PUTWORD(&(Record[RDWord++]), lpPoint->y);
		lpPoint++;
	}
	Return = META_RECORD(Record);
	WinFree((LPSTR)Record);
	return Return;
}


DWORD
lsd_meta_rectangle(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LPRECT	lpRect = &lpStruct->lsde.rect;
	int	RDWord = RDPARAM;
	WORD	Record[MFRS_RECTANGLE];
	WORD	wFunc;

	switch (msg) {
	    case LSD_RECTANGLE:
		wFunc = META_RECTANGLE;
		break;
	    case LSD_ELLIPSE:
		wFunc = META_ELLIPSE;
		break;
	    case LSD_INTERSECTCLIPRECT:
		wFunc = META_INTERSECTCLIPRECT;
		break;
	    case LSD_EXCLUDECLIPRECT:
		wFunc = META_EXCLUDECLIPRECT;
		break;
	    default:
		return 0L;
	}
	PUTDWORD(&(Record[RDSIZE]), MFRS_RECTANGLE);
	PUTWORD(&(Record[RDFUNC]), wFunc);
	PUTWORD(&(Record[RDWord++]), lpRect->bottom);
	PUTWORD(&(Record[RDWord++]), lpRect->right);
	PUTWORD(&(Record[RDWord++]), lpRect->top);
	PUTWORD(&(Record[RDWord++]), lpRect->left);
	return META_RECORD(Record);
}

DWORD
lsd_meta_roundrect(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LPPOINT lpArgs = lpStruct->lsde.arc;
	int	RDWord = RDPARAM;
	WORD	Record[MFRS_ROUNDRECT];

	PUTDWORD(&(Record[RDSIZE]), MFRS_ROUNDRECT);
	PUTWORD(&(Record[RDFUNC]), META_ROUNDRECT);
	PUTWORD(&(Record[RDWord++]), lpArgs[2].y);
	PUTWORD(&(Record[RDWord++]), lpArgs[2].x);
	PUTWORD(&(Record[RDWord++]), lpArgs[1].y);
	PUTWORD(&(Record[RDWord++]), lpArgs[1].x);
	PUTWORD(&(Record[RDWord++]), lpArgs[0].y);
	PUTWORD(&(Record[RDWord++]), lpArgs[0].x);
	return META_RECORD(Record);
}

DWORD
lsd_meta_arc(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LPPOINT lpArgs = lpStruct->lsde.arc;
	int	RDWord = RDPARAM;
	WORD	Record[MFRS_ARC];

	PUTDWORD(&(Record[RDSIZE]), MFRS_ARC);
	PUTWORD(&(Record[RDFUNC]),(msg == LSD_ARC)   ? META_ARC :
				  (msg == LSD_CHORD) ? META_CHORD :
						       META_PIE );
	PUTWORD(&(Record[RDWord++]), lpArgs[3].y);
	PUTWORD(&(Record[RDWord++]), lpArgs[3].x);
	PUTWORD(&(Record[RDWord++]), lpArgs[2].y);
	PUTWORD(&(Record[RDWord++]), lpArgs[2].x);
	PUTWORD(&(Record[RDWord++]), lpArgs[1].y);
	PUTWORD(&(Record[RDWord++]), lpArgs[1].x);
	PUTWORD(&(Record[RDWord++]), lpArgs[0].y);
	PUTWORD(&(Record[RDWord++]), lpArgs[0].x);
	return META_RECORD(Record);
}

DWORD
lsd_meta_polypolygon(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LSDE_POLYPOLY *lpPoly = &lpStruct->lsde.polypoly;
	int	RDWord = RDPARAM;
	DWORD	dwRecSize;		/* in bytes for WinMalloc() */
	LPWORD	Record;
	BOOL	Return;

	dwRecSize = (MFRS_POLYPOLY * sizeof(WORD)) +
			(lpPoly->nCount * sizeof(WORD)) +
			(lpPoly->nTotalCount * sizeof(DWORD));
	if ( !(Record = (LPWORD)WinMalloc(dwRecSize)) )
		return FALSE;
	PUTDWORD(&(Record[RDSIZE]), dwRecSize / sizeof(WORD));
	PUTWORD(&(Record[RDFUNC]), META_POLYPOLYGON);
	PUTWORD(&(Record[RDWord++]), lpPoly->nTotalCount);
	while (lpPoly->nCount--)
		PUTWORD(&(Record[RDWord++]), *lpPoly->lpCounts++);
	while (lpPoly->nTotalCount--)
	{
	    PUTWORD(&(Record[RDWord++]), lpPoly->lpPoints->x);
	    PUTWORD(&(Record[RDWord++]), lpPoly->lpPoints->y);
	    lpPoly->lpPoints++;
	}
	Return = META_RECORD(Record);
	WinFree((LPSTR)Record);
	return Return;
}

DWORD
lsd_meta_floodfill(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LSDE_FLOODFILL *lpArgs = &lpStruct->lsde.floodfill;
	int	RDWord = RDPARAM;
	WORD	Record[MFRS_FLOODFILL];

	PUTDWORD(&(Record[RDSIZE]), MFRS_FLOODFILL);
	PUTWORD(&(Record[RDFUNC]), META_FLOODFILL);
	PUTDWORD(&(Record[RDWord++]), lpArgs->cr);
	RDWord++;
	PUTWORD(&(Record[RDWord++]), lpArgs->nYStart);
	PUTWORD(&(Record[RDWord++]), lpArgs->nXStart);
	return META_RECORD(Record);
}

DWORD
lsd_meta_extfloodfill(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LSDE_FLOODFILL *lpArgs = &lpStruct->lsde.floodfill;
	int	RDWord = RDPARAM;
	WORD	Record[MFRS_EXTFLOODFILL];

	PUTDWORD(&(Record[RDSIZE]), MFRS_EXTFLOODFILL);
	PUTWORD(&(Record[RDFUNC]), META_EXTFLOODFILL);
	PUTWORD(&(Record[RDWord++]), lpArgs->fuFillType);
	PUTDWORD(&(Record[RDWord++]), lpArgs->cr);
	RDWord++;
	PUTWORD(&(Record[RDWord++]), lpArgs->nYStart);
	PUTWORD(&(Record[RDWord++]), lpArgs->nXStart);
	return META_RECORD(Record);
}

DWORD
lsd_meta_textout(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LSDE_TEXT *lpText = &lpStruct->lsde.text;
	int	RDWord = RDPARAM;
	DWORD	dwRecSize;		/* in bytes for WinMalloc() */
	LPWORD	Record;
	LPSTR	String;
	BOOL	Return;

	dwRecSize = (MFRS_TEXTOUT * sizeof(WORD)) + (lpText->nCnt + 1);
	if ( !(Record = (LPWORD)WinMalloc(dwRecSize)) )
		return FALSE;
	PUTDWORD(&(Record[RDSIZE]), dwRecSize / sizeof(WORD));
	PUTWORD(&(Record[RDFUNC]), META_TEXTOUT );
	PUTWORD(&(Record[RDWord++]), lpText->nCnt);
	String = (LPSTR)(&(Record[RDWord]));
	if ( lpText->nCnt & 1 )		/* Force null char into extra byte */
		String[lpText->nCnt] = '\0';
	RDWord += (lpText->nCnt+1) / sizeof(WORD);	/* move pointer */
	while (lpText->nCnt--)				/* transfer chars */
		*String++ = *lpText->lpStr++;
	PUTWORD(&(Record[RDWord++]), lpText->y);
	PUTWORD(&(Record[RDWord++]), lpText->x);
	Return = META_RECORD(Record);
	WinFree((LPSTR)Record);
	return Return;
}

DWORD
lsd_meta_exttextout(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LSDE_TEXT *lpText = &lpStruct->lsde.text;
	int	RDWord = RDPARAM;
	DWORD	dwRecSize;		/* in bytes for WinMalloc() */
	LPWORD	Record;
	LPSTR	String;
	int	Count;
	BOOL	Return;

	dwRecSize = (MFRS_EXTTEXTOUT * sizeof(WORD)) +
		    (lpText->nCnt + 1) +
		    (lpText->uiFlags != 0 ? sizeof(RECT) : 0) +
		    (lpText->lpDX != NULL ? (lpText->nCnt * sizeof(WORD)) : 0);
	if ( !(Record = (LPWORD)WinMalloc(dwRecSize)) )
		return FALSE;
	PUTDWORD(&(Record[RDSIZE]), dwRecSize / sizeof(WORD));
	PUTWORD(&(Record[RDFUNC]), META_EXTTEXTOUT );
	PUTWORD(&(Record[RDWord++]), lpText->y);
	PUTWORD(&(Record[RDWord++]), lpText->x);
	PUTWORD(&(Record[RDWord++]), lpText->nCnt);
	PUTWORD(&(Record[RDWord++]), lpText->uiFlags);
	if ( lpText->uiFlags )
	{
		PUTWORD(&(Record[RDWord++]), lpText->lpRect->left);
		PUTWORD(&(Record[RDWord++]), lpText->lpRect->top);
		PUTWORD(&(Record[RDWord++]), lpText->lpRect->right);
		PUTWORD(&(Record[RDWord++]), lpText->lpRect->bottom);
	}
	String = (LPSTR)(&(Record[RDWord]));
	if ( lpText->nCnt & 1 )		/* Force null char into extra byte */
		String[lpText->nCnt] = '\0';
	RDWord += (lpText->nCnt+1) / sizeof(WORD);	/* move pointer */
	for (Count = 0; Count < lpText->nCnt; Count++)	/* transfer chars */
		*String++ = *lpText->lpStr++;
	if ( lpText->lpDX != NULL )			/* transfer spaces */
		for (Count = 0; Count < lpText->nCnt; Count++)
			PUTWORD(&(Record[RDWord++]), *lpText->lpDX++);
	Return = META_RECORD(Record);
	WinFree((LPSTR)Record);
	return Return;
}

/*
	Field		Size (words)
	-------------	------------
	HIWORD(ROP)		1
	ySrc			1
	xSrc			1
	nWidthDest		1
	nHeightDest		1
	yDest			1
	xDest			1
	BITMAPINFO
	  BITMAPINFOHEADER
	    biSize		2
	    biWidth		2
	    biHeight		2
	    biPlanes		1
	    biBitCount		1
	    biCompression	2
	    biSizeImage		2
	    biXPelsPerMeter	2
	    biYPelsPerMeter	2
	    biClrUsed		2
	    biClrImportant	2
	  RGBQUAD[palette-size]	2n
	bits			x

	BITMAP
	  bmType
*/
DWORD
lsd_meta_bitblt(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LSDE_STRETCHDATA *lpStretch = &lpStruct->lsde.stretchdata;
	int	RDWord = RDPARAM;
	DWORD	dwRecSize;		/* in bytes for WinMalloc() */
	LPWORD	Record;
	WORD	Colors     = 256;
	BOOL	Return;
	BITMAP  Bitmap;
	DWORD   BitmapSize = 0;

	GetObject(lpStretch->hSrcDC32->hBitmap, sizeof(BITMAP), &Bitmap);

	dwRecSize = (MFRS_BITBLT * sizeof(WORD)) +
		    (sizeof(BITMAPINFOHEADER)) +
		    (sizeof(RGBQUAD) * Colors) +
		    (BitmapSize);
	if ( !(Record = (LPWORD)WinMalloc(dwRecSize)) )
		return FALSE;
	PUTDWORD(&(Record[RDSIZE]), dwRecSize / sizeof(WORD));
	PUTWORD(&(Record[RDFUNC]), META_BITBLT );
	PUTWORD(&(Record[RDWord++]), HIWORD(lpStretch->dwRop) );
	PUTWORD(&(Record[RDWord++]), lpStretch->ySrc );
	PUTWORD(&(Record[RDWord++]), lpStretch->xSrc );
	PUTWORD(&(Record[RDWord++]), lpStretch->nHeightDest );
	PUTWORD(&(Record[RDWord++]), lpStretch->nWidthDest );
	PUTWORD(&(Record[RDWord++]), lpStretch->yDest );
	PUTWORD(&(Record[RDWord++]), lpStretch->xDest );
/*
 *	TO BE DONE ...
 *		Create and PUT BITMAPINFOHEADER
 *		Get and PUT Palette
 *		Get and PUT Bitmap
 */
	Return = META_RECORD(Record);
	WinFree((LPSTR)Record);
	return Return;
}

DWORD
lsd_meta_setpixel(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	LPPOINT	lpPoint = &lpStruct->lsde.point;
	COLORREF cr = (COLORREF)dwParam;
	int	RDWord = RDPARAM;
	WORD	Record[MFRS_SETPIXEL];

	PUTDWORD(&(Record[RDSIZE]), MFRS_SETPIXEL);
	PUTWORD(&(Record[RDFUNC]), META_SETPIXEL);
	PUTDWORD(&(Record[RDWord++]), cr);
	RDWord++;
	PUTWORD(&(Record[RDWord++]), lpPoint->y);	/* y before x */
	PUTWORD(&(Record[RDWord++]), lpPoint->x);
	return META_RECORD(Record);
}

DWORD
lsd_meta_escape(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    int	RDWord = RDPARAM;
    LPWORD Record;
    int nMemLen;
    BOOL Return;

    nMemLen = 5*WORD_86 + lpStruct->lsde.escape.cbInput;
    Record = (LPWORD)WinMalloc(nMemLen);
    PUTDWORD(&(Record[RDSIZE]), nMemLen / sizeof(WORD));
    PUTWORD(&(Record[RDFUNC]), META_ESCAPE);
    PUTWORD(&(Record[RDWord++]), (int)dwParam);
    PUTWORD(&(Record[RDWord++]), lpStruct->lsde.escape.cbInput);
    memcpy((LPSTR)&(Record[RDWord]),lpStruct->lsde.escape.lpszInData,
				lpStruct->lsde.escape.cbInput);
    Return = META_RECORD(Record);
    WinFree((LPSTR)Record);
    return Return;
}

DWORD
lsd_meta_stretchblt(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return (DWORD)TRUE;
}

DWORD
lsd_meta_selectcliprgn(WORD msg, HDC32 hDC32,
			DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_intersectcliprgn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_offsetcliprgn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_fillrgn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_framergn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_invertrgn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_paintrgn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_patblt(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    LSDE_STRETCHDATA *lpSD = &lpStruct->lsde.stretchdata;
    int	RDWord = RDPARAM;
    WORD Record[MFRS_PATBLT];

    PUTDWORD(&(Record[RDSIZE]), MFRS_PATBLT);
    PUTWORD(&(Record[RDFUNC]), META_PATBLT);
    PUTDWORD(&(Record[RDWord++]), lpSD->dwRop);
    RDWord++;
    PUTWORD(&(Record[RDWord++]), lpSD->nHeightDest);
    PUTWORD(&(Record[RDWord++]), lpSD->nWidthDest);
    PUTWORD(&(Record[RDWord++]), lpSD->yDest);
    PUTWORD(&(Record[RDWord++]), lpSD->xDest);
    return META_RECORD(Record);
}

DWORD
lsd_meta_stretchdibits(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_setdibtodevice(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_startdoc(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_startpage(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_endpage(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_enddoc(WORD msg, HDC32 hDC32,
		DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}

DWORD
lsd_meta_abortdoc(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return TRUE;
}
