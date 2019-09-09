/*
	@(#)DrvDC.c	2.25
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
#include "windowsx.h"
#include "Driver.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Log.h"
#include "DrvDP.h"
#include "GdiDC.h"
#include "DrvDC.h"

static DWORD DrvDCDoNothing(LPARAM,LPARAM,LPVOID);
static DWORD DrvCreateDC(LPARAM,LPARAM,LPVOID);
static DWORD DrvDeleteDC(LPARAM,LPARAM,LPVOID);
static DWORD DrvSaveDC(LPARAM,LPARAM,LPVOID);
static DWORD DrvRestoreDC(LPARAM,LPARAM,LPVOID);
static DWORD DrvGetDCClip(LPARAM,LPARAM,LPVOID);
static DWORD DrvSetDCClip(LPARAM,LPARAM,LPVOID);
static DWORD DrvSetClipOrg(LPARAM,LPARAM,LPVOID);
static DWORD DrvSetUpdateClip(LPARAM,LPARAM,LPVOID);
static DWORD DrvSetVisRgn(LPDRIVERDC,Region);
static DWORD DrvDCSetVisRgn(LPARAM,LPARAM,LPVOID);
static DWORD DrvCombineClipRgn(LPDRIVERDC);

DWORD DrvDCTab(void);

static TWINDRVSUBPROC DrvDCEntryTab[] = {
	DrvDCDoNothing,
	DrvDCDoNothing,
	DrvDCDoNothing,
	DrvCreateDC,
	DrvDeleteDC,
	DrvSaveDC,
	DrvRestoreDC,
	DrvGetDCClip,
	DrvSetDCClip,
	DrvSetClipOrg,
	DrvSetUpdateClip,
	DrvDCSetVisRgn};

DWORD
DrvDCTab(void)
{
#if defined TWIN_RUNTIME_DRVTAB
	DrvDCEntryTab[0] = DrvDCDoNothing;
	DrvDCEntryTab[1] = DrvDCDoNothing;
	DrvDCEntryTab[2] = DrvDCDoNothing;
	DrvDCEntryTab[3] = DrvCreateDC;
	DrvDCEntryTab[4] = DrvDeleteDC;
	DrvDCEntryTab[5] = DrvSaveDC;
	DrvDCEntryTab[6] = DrvRestoreDC;
	DrvDCEntryTab[7] = DrvGetDCClip;
	DrvDCEntryTab[8] = DrvSetDCClip;
	DrvDCEntryTab[9] = DrvSetClipOrg;
	DrvDCEntryTab[10] = DrvSetUpdateClip;
	DrvDCEntryTab[11] = DrvDCSetVisRgn;
#endif

	return (DWORD)DrvDCEntryTab;
}

DWORD
DrvDCDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 1L;
}

static DWORD
DrvCreateDC(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    DWORD dwDCXFlags = (DWORD)dwParam1;
    Window frame_win = (Window)lpStruct;
    LPDRIVERDC lpddc;
    PRIVATEDISPLAY *dp;
    XGCValues values;
    DWORD vmask;
    static Pixmap DefaultDCPixmap = (Pixmap)0;

    lpddc = (LPDRIVERDC)WinMalloc(sizeof(DRIVERDC));

    memset((LPSTR)lpddc,'\0',sizeof(DRIVERDC));
    dp = (PRIVATEDISPLAY *)GETDP();
    lpddc->dp = dp;
    lpddc->dwDCFlags = dwDCXFlags;

    /* set the drawable */
    if (dwDCXFlags & DCX_COMPATIBLE_DC) {
	/* for compatible DCs we stick 1x1 stock pixmap in it */
	if (DefaultDCPixmap == (Pixmap)0)
	    DefaultDCPixmap = XCreatePixmap(dp->display,
			RootWindow(dp->display,dp->screen),
			1,1, 
			DefaultDepth(dp->display,dp->screen));
	lpddc->Drawable = DefaultDCPixmap;
    }
    else
	lpddc->Drawable = (frame_win)?
		frame_win:RootWindow(dp->display, dp->screen);
    lpddc->BkFillMode = OPAQUE;
    lpddc->RopFunction = GXcopy;	/* default for new GC */
    lpddc->FillStyle = FillSolid;
    lpddc->ForeColorPixel =
		BlackPixel(lpddc->dp->display,lpddc->dp->screen);
    lpddc->BackColorPixel =
		WhitePixel(lpddc->dp->display,lpddc->dp->screen);

    /* now create graphic context */
    vmask = GCCapStyle|GCForeground|GCBackground;
    values.cap_style = CapNotLast;
    values.foreground = lpddc->ForeColorPixel;
    values.background = lpddc->BackColorPixel;
    lpddc->gc = XCreateGC(dp->display, lpddc->Drawable, vmask, &values);

    /* default attributes for X11 will be set by selecting stock */
    /* objects into the DC after this call returns */

    XSetSubwindowMode(dp->display,lpddc->gc,IncludeInferiors);

    /* init palette index translation table */
    lpddc->wPaletteSize = 0;
    lpddc->lpPaletteIndexTable = (LPWORD) 0;
    lpddc->lpPaletteEntryTable = (LPPALETTEENTRY) 0;

    return (DWORD)lpddc;
}

static DWORD
DrvDeleteDC(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    LPDRIVERDC lpddc = (LPDRIVERDC)lpStruct;
    if (lpddc->gc)
	XFreeGC(lpddc->dp->display, lpddc->gc);

    if (lpddc->ClipRegion)
	XDestroyRegion(lpddc->ClipRegion);
    if (lpddc->UpdateRegion)
	XDestroyRegion(lpddc->UpdateRegion);
    if (lpddc->VisibleRegion)
	XDestroyRegion(lpddc->VisibleRegion);

    /* delete palette index translation table */
    if (lpddc->lpPaletteIndexTable)
    {
	WinFree((LPSTR) lpddc->lpPaletteIndexTable);
	lpddc->lpPaletteIndexTable = (LPWORD) 0;
    }
    if (lpddc->lpPaletteEntryTable)
    {
	WinFree((LPSTR) lpddc->lpPaletteEntryTable);
	lpddc->lpPaletteEntryTable = (LPPALETTEENTRY) 0;
    }

    WinFree((LPSTR)lpddc);
    return 1L;
}

static DWORD
DrvSaveDC(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    LPDRIVERDC lpddc = (LPDRIVERDC)lpStruct;
    LPDRIVERDC lpNewddc;
    Region xReg;

    lpNewddc = (LPDRIVERDC)WinMalloc(sizeof(DRIVERDC));
    memcpy((LPSTR)lpNewddc,(LPSTR)lpddc,sizeof(DRIVERDC));

    if (lpNewddc->ClipRegion)
    {
	xReg = XCreateRegion();
	XUnionRegion(xReg,lpNewddc->ClipRegion,xReg);
	lpNewddc->ClipRegion = xReg;
    }

    if (lpNewddc->UpdateRegion)
    {
	xReg = XCreateRegion();
	XUnionRegion(xReg,lpNewddc->UpdateRegion,xReg);
	lpNewddc->UpdateRegion = xReg;
    }

    if (lpNewddc->VisibleRegion)
    {
	xReg = XCreateRegion();
	XUnionRegion(xReg,lpNewddc->VisibleRegion,xReg);
	lpNewddc->VisibleRegion = xReg;
    }

    /* copy palette index translation table */
    if (lpNewddc->lpPaletteIndexTable)
    {
	lpNewddc->lpPaletteIndexTable =
		(LPWORD) WinMalloc(sizeof(WORD)
			* (lpNewddc->wPaletteSize + 1));
	if (lpNewddc->lpPaletteIndexTable)
		memcpy(lpNewddc->lpPaletteIndexTable,
			lpddc->lpPaletteIndexTable,
			sizeof(WORD) * (lpNewddc->wPaletteSize + 1));
    }
    if (lpNewddc->lpPaletteEntryTable)
    {
	lpNewddc->lpPaletteEntryTable =
		(LPPALETTEENTRY) WinMalloc(sizeof(PALETTEENTRY)
			* (lpNewddc->wPaletteSize + 1));
	if (lpNewddc->lpPaletteEntryTable)
		memcpy(lpNewddc->lpPaletteEntryTable,
			lpddc->lpPaletteEntryTable,
			sizeof(WORD) * (lpNewddc->wPaletteSize + 1));
    }

    return (DWORD)lpNewddc;
}

/* If called with lpddc2==NULL, DrvRestoreDC() destroys clipping region
 * in lpddc1 and deallocates the structure; called with lpddc2, it saves
 * the drawable from lpddc1 to lpddc2, invalidates the fields in lpddc2 that
 * don't make sense anymore, and also frees the memory taken by lpddc1 */

static DWORD
DrvRestoreDC(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    LPDRIVERDC lpddc2 = (LPDRIVERDC)dwParam1;
    LPDRIVERDC lpddc1 = (LPDRIVERDC)lpStruct;

    if (lpddc1->ClipRegion)
	XDestroyRegion(lpddc1->ClipRegion);

    if (lpddc1->UpdateRegion)
	XDestroyRegion(lpddc1->UpdateRegion);

    if (lpddc1->VisibleRegion)
	XDestroyRegion(lpddc1->VisibleRegion);

    if (lpddc1->lpPaletteIndexTable)
	WinFree((LPSTR) lpddc1->lpPaletteIndexTable);

    if (lpddc1->lpPaletteEntryTable)
	WinFree((LPSTR) lpddc1->lpPaletteEntryTable);

    if (lpddc2) {
	lpddc2->Drawable = lpddc1->Drawable;
	DrvCombineClipRgn(lpddc2);
    }

    WinFree((LPSTR)lpddc1);

    return (DWORD)PGH_SUCCESS;
}

static DWORD
DrvGetDCClip(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    LPDRIVERDC lpddc = (LPDRIVERDC)lpStruct;
    return (DWORD)lpddc->ClipRegion;
}

static DWORD
DrvSetDCClip(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    Region RgnClip = (Region)dwParam1;
    LPDRIVERDC lpddc = (LPDRIVERDC)lpStruct;
    /* if there is an empty update region, get rid of it */
    if (lpddc->UpdateRegion && XEmptyRegion(lpddc->UpdateRegion)) {
	XDestroyRegion(lpddc->UpdateRegion);
	lpddc->UpdateRegion = (Region)0;
    }

    if (lpddc->ClipRegion) {
	XDestroyRegion(lpddc->ClipRegion);
	lpddc->ClipRegion = 0;
    }

    if (RgnClip && !XEmptyRegion(RgnClip)) {
	lpddc->ClipRegion = XCreateRegion();
	XUnionRegion(RgnClip,lpddc->ClipRegion,lpddc->ClipRegion);
    }

    return DrvCombineClipRgn(lpddc);
}

static DWORD
DrvSetClipOrg(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    LPPOINT lppt = (LPPOINT)dwParam1;
    LPDRIVERDC lpddc = (LPDRIVERDC)lpStruct;
    lpddc->cpClipOrigin = *lppt;
    return 1L;
}

static DWORD
DrvSetUpdateClip(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    Region RgnUpdate = (Region)dwParam1;
    Region RgnVis = (Region)dwParam2;
    LPDRIVERDC lpddc = (LPDRIVERDC)lpStruct;
    /* if there is a non-empty update region passed, remember it */
    if (RgnUpdate && !XEmptyRegion(RgnUpdate)) {
	if (lpddc->UpdateRegion)
	    XDestroyRegion(lpddc->UpdateRegion);
	lpddc->UpdateRegion = XCreateRegion();
        XUnionRegion(RgnUpdate,lpddc->UpdateRegion,lpddc->UpdateRegion);
    }
    else
	if (lpddc->UpdateRegion) {
	    XDestroyRegion(lpddc->UpdateRegion);
	    lpddc->UpdateRegion = 0;
	}

    /* no user-defined clipping at this point */
    if(lpddc->ClipRegion) {
	XDestroyRegion(lpddc->ClipRegion);
	lpddc->ClipRegion = 0;
    }

    /* this combines all three... */
    (void)DrvSetVisRgn(lpddc,RgnVis);

    return 1L;
}

static DWORD
DrvDCSetVisRgn(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	LPDRIVERDC lpddc = (LPDRIVERDC)lpStruct;
	Region VisRgn = (Region)dwParam2;
	return DrvSetVisRgn(lpddc, VisRgn);
}
 
static DWORD
DrvSetVisRgn(LPDRIVERDC lpddc, Region VisRgn)
{
    int nRet = NULLREGION;

    if (lpddc->VisibleRegion)
	XDestroyRegion(lpddc->VisibleRegion);

    if (VisRgn) {
	lpddc->VisibleRegion = XCreateRegion();
	XUnionRegion(VisRgn,lpddc->VisibleRegion,lpddc->VisibleRegion);
	nRet = COMPLEXREGION;
    }
    else
	lpddc->VisibleRegion = 0;

    (void)DrvCombineClipRgn(lpddc);

    return (DWORD)nRet;
}

static DWORD
DrvCombineClipRgn(LPDRIVERDC lpddc)
{
    Region Rgn;

    Rgn = XCreateRegion();

    if (lpddc->VisibleRegion) {
	XUnionRegion(lpddc->VisibleRegion,Rgn,Rgn);
	if (lpddc->ClipRegion)
	    XIntersectRegion(lpddc->ClipRegion,Rgn,Rgn);
	if (lpddc->UpdateRegion)
	    XIntersectRegion(lpddc->UpdateRegion,Rgn,Rgn);
    }
    else {
	if (lpddc->ClipRegion) {
	    XUnionRegion(lpddc->ClipRegion,Rgn,Rgn);
	    if (lpddc->UpdateRegion)
		XIntersectRegion(lpddc->UpdateRegion,Rgn,Rgn);
	}
	else {
	    if (lpddc->UpdateRegion)
		XUnionRegion(lpddc->UpdateRegion,Rgn,Rgn);
	    else {
		XSetClipMask(lpddc->dp->display,lpddc->gc,None);
		XDestroyRegion(Rgn);
		return (DWORD)NULLREGION;
	    }
	}
    }

    XSetRegion(lpddc->dp->display,lpddc->gc,Rgn);

    if (lpddc->cpClipOrigin.x || lpddc->cpClipOrigin.y)
	XSetClipOrigin(lpddc->dp->display,lpddc->gc,
	    lpddc->cpClipOrigin.x,lpddc->cpClipOrigin.y);

    XDestroyRegion(Rgn);

    return (DWORD)COMPLEXREGION;
}
