/*    
	PrinterDC.c	2.21
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
#include "print.h"

#include "Driver.h"
#include "GdiObjects.h"
#include "GdiDC.h"
#include "DeviceData.h"
#include "GdiDDK.h"
#include "GdiText.h"
#include "PrinterDC.h"
#include "Log.h"

#include <string.h>

extern HINSTANCE LoadDriver(LPSTR);

extern int TWIN_PrinterEscape(HDC32, int, const LPVOID, LPVOID);
extern int CalcByteWidth(int,int,int);
extern LONG TWIN_GetImageBits(HBITMAP, LONG, LPVOID, int);

HDC PrinterResetDC(HDC32,const DEVMODE *);

static BOOL PrinterInitContext(LPPRINTERDRIVERDC, LPDEVMODE);
static BOOL PrinterInitCompatDC(LPPRINTERDRIVERDC,LPPRINTERDRIVERDC);
static BOOL GDIRealizeObject(HINSTANCE,LPPDEVICE,int,
				LPVOID,LPVOID *,LPTEXTXFORM);
static BOOL BruteRealize(HINSTANCE, LPPDEVICE, int, LPVOID);
static BOOL OurSpecialForPattern(LPLOGBRUSH, LPBITMAP);

DWORD
lsd_printer_init(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return 0L;
}

DWORD
lsd_printer_createdc(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LSDE_CREATEDATA *lpcd = &lpStruct->lsde.create;
#ifndef	macintosh
    LPPRINTERDRIVERDC	lppddc = NULL,lppddcOrig;
#endif	/* not macintosh */

    hDC32->dwDCXFlags |= DCX_PRINTER_DC;	/* req'd if not compatible */

#ifdef	macintosh

	hDC32->lpDrvData = (LPVOID )DRVCALL_PRINTER( PPH_CREATEDC, hDC32->dwDCXFlags,
													NULL, lpcd->lpvInitData );
	if ( hDC32->lpDrvData == NULL )
		return( 0 );

#else	/* macintosh */
    /* Get a pointer to the printer private data. */
    lppddc = (LPPRINTERDRIVERDC)WinMalloc(sizeof(PRINTERDRIVERDC));
    if ( lppddc == NULL )
	FatalAppExit(0,"lsd_printer_createdc() could not allocate memory!\n");

    memset((LPVOID)lppddc, 0, sizeof(PRINTERDRIVERDC));

    if ( hDC32->dwDCXFlags & DCX_COMPATIBLE_DC ) {
	if (dwParam == 0) {
	    WinFree((LPVOID)lppddc);
	    return 0;
	}
	lppddcOrig = (LPPRINTERDRIVERDC)((HDC32)dwParam)->lpDrvData;
	strcpy(lppddc->szDrvName, lppddcOrig->szDrvName);
	strcpy(lppddc->szDevice,  lppddcOrig->szDevice);
	strcpy(lppddc->szPort,    lppddcOrig->szPort);
	if ( !PrinterInitCompatDC(lppddc,lppddcOrig) ) {
	    WinFree((LPVOID)lppddc);		/* Free PrinterDeviceDC */
	    return 0;
	}
    }
    else {
	/* Store driver filename w/o extension (module name), */
	/* output device name and port/file name.             */
	strcpy(lppddc->szDrvName, lpcd->lpszDriver);
	strcpy(lppddc->szDevice,  lpcd->lpszDevice);
	strcpy(lppddc->szPort,    lpcd->lpszOutput);

	if ( !PrinterInitContext(lppddc, (LPDEVMODE)lpcd->lpvInitData)) {
	    WinFree((LPVOID)lppddc);		/* Free PrinterDeviceDC */
	    return 0;
	}

    	hDC32->lpDrvData = (LPVOID)lppddc;
    }
#endif	/* else macintosh */
    return 1;
}

DWORD
lsd_printer_deletedc(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    LPPSBITMAP lpPSBitmap;

/*	TODO:	Cleanup the driver's DC */

    if ( lppddc != NULL ) {			/* Clean up. */
	if ( lppddc->pDevice->pdType )
	    WinFree((LPSTR)lppddc->pDevice);
	else {
	    lpPSBitmap = (LPPSBITMAP)(lppddc->pDevice);
	    if (lpPSBitmap->bm.bmBits)
		WinFree((LPSTR)lpPSBitmap->bm.bmBits);
	    WinFree((LPSTR)lppddc->pDevice);
	}

	WinFree((LPSTR)lppddc);
	hDC32->lpDrvData = (LPVOID)NULL;

	return (DWORD)TRUE;
    }

    return (DWORD)FALSE;
}

DWORD
lsd_printer_savedc(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return 0L;
}

DWORD
lsd_printer_restoredc(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return 0L;
}

DWORD
lsd_printer_selectobject(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    HGDIOBJ	hObject = (HGDIOBJ)dwParam;
    HGDIOBJ	hOldObj = 0;
    LPOBJHEAD   lpObjInfo = 0;
    LPLOGPEN lpLogPen;
    LPLOGBRUSH lpLogBrush;
    LPLOGFONT lpLogFont;
    LPBITMAP   lpBmp;
    BITMAP bmp;
    DWORD dwSize;

    if ( hObject == 0 ) {
	return 0L;
    }

    if (!(lpObjInfo = GETGDIINFO(hObject))) {
	return 0L;
    }

    switch (GET_OBJECT_TYPE(lpObjInfo)) {
	case OT_BITMAP:
	    if ( lppddc->pDevice->pdType 	/* Doesn't make sense */
	         ||  !GetObject(hObject, sizeof(BITMAP), (LPVOID)&bmp) ) {
	        RELEASEGDIINFO(lpObjInfo);  
		return FALSE;
	    }

	    bmp.bmWidthBytes = CalcByteWidth(bmp.bmWidth,bmp.bmBitsPixel,32);
	    dwSize = bmp.bmWidthBytes*bmp.bmHeight;
	    bmp.bmBits = (LPVOID)WinMalloc(dwSize);
	    TWIN_GetImageBits((HBITMAP)hObject,dwSize,bmp.bmBits,32);

	    if ( lppddc->GdiInfo.dpRaster & RC_DEVBITS )
		FatalAppExit(0,
			"lsd_printer_selectobject: cannot do RC_DEVBITS");
	    else
		dmRealizeObject(lppddc->pDevice, DRV_OBJ_PBITMAP,
			(LPSTR)&bmp, 0, 0);
	    hOldObj = hDC32->hBitmap;
	    hDC32->hBitmap = (HBITMAP)hObject;
	    break;

	case OT_BRUSH:

	    lpLogBrush = &((LPBRUSHINFO)lpObjInfo)->lpBrush;
	    if (lppddc->pDevice->pdType) {	/* device */
		if ( lpLogBrush->lbStyle == BS_PATTERN ) {
		    lpBmp = (LPBITMAP)WinMalloc(sizeof(BITMAP));
		    if ( !OurSpecialForPattern(lpLogBrush, lpBmp) ) {
		        RELEASEGDIINFO(lpObjInfo);
			return FALSE;
		    }
		}

		if ( !GDIRealizeObject(lppddc->hModuleDriver, lppddc->pDevice,
			DRV_OBJ_BRUSH, (LPVOID)lpLogBrush, &lppddc->lpPhBrush, 0) ) {
		    RELEASEGDIINFO(lpObjInfo);
		    return FALSE;
		}
	    }
	    else {
		if (!BruteRealize(lppddc->hModuleDriver, lppddc->pDevice,
			DRV_OBJ_BRUSH, (LPVOID)lpLogBrush)) {
		    RELEASEGDIINFO(lpObjInfo);
		    return FALSE;
		}
		lppddc->lpPhBrush = (LPVOID)lpObjInfo;
	    }

	    hOldObj = hDC32->hBrush;
	    hDC32->hBrush = hObject;
	    break;

	case OT_PEN:

	    lpLogPen = &((LPPENINFO)lpObjInfo)->lpPen;
	    if (lppddc->pDevice->pdType) {	/* device */
		if ( !GDIRealizeObject(lppddc->hModuleDriver, lppddc->pDevice,
			DRV_OBJ_PEN, (LPVOID)lpLogPen, &lppddc->lpPhPen, 0) ) {
		    RELEASEGDIINFO(lpObjInfo);
		    return FALSE;
		}
	    }
	    else {
		if (!BruteRealize(lppddc->hModuleDriver, lppddc->pDevice,
			DRV_OBJ_PEN, (LPVOID)lpLogPen)) {
		    RELEASEGDIINFO(lpObjInfo);
		    return FALSE;
		}
		lppddc->lpPhPen = (LPVOID)lpObjInfo;
	    }

	    hOldObj = hDC32->hPen;
	    hDC32->hPen = hObject;
	    break;

	case OT_FONT:

	    lpLogFont = &((LPFONTINFO)lpObjInfo)->LogFont;
	    if (lppddc->pDevice->pdType) {	/* device */
		if ( !GDIRealizeObject(lppddc->hModuleDriver, lppddc->pDevice,
			DRV_OBJ_FONT, (LPVOID)lpLogFont,
			&lppddc->lpPhFont, &lppddc->txf) ) {
		    RELEASEGDIINFO(lpObjInfo);
		    return FALSE;
		}
	    }
	    else {
		if (!BruteRealize(lppddc->hModuleDriver, lppddc->pDevice,
			DRV_OBJ_FONT, (LPVOID)lpLogFont)) {
		    RELEASEGDIINFO(lpObjInfo);
		    return FALSE;
		}
		lppddc->lpPhFont = (LPVOID)lpObjInfo;
	    }

	    hOldObj = hDC32->hFont;
	    hDC32->hFont = hObject;
	    break;

	default:
	    RELEASEGDIINFO(lpObjInfo);
	    return FALSE;
    }

    RELEASEGDIINFO(lpObjInfo);
    return (DWORD)hOldObj;
}

DWORD
lsd_printer_selectpal(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return(0);
}

DWORD
lsd_printer_realizepal(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    return(0);
}

DWORD
lsd_printer_selectcliprgn(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    int nQuery = SET_CLIP_BOX;
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;
    HRGN hRgn;
    

    if (!(hRgn = (HRGN)dwParam)) {
	SetRectEmpty(&lppddc->rcClipping);
	lppddc->fClippingSet = FALSE;
    }
    else {
	GetRgnBox(hRgn,&lppddc->rcClipping);
	lppddc->fClippingSet = TRUE;
    }

    if (!TWIN_PrinterEscape(hDC32, QUERYESCSUPPORT, (const LPVOID)&nQuery,0))
	return (DWORD)(IsRectEmpty(&lppddc->rcClipping))?
			NULLREGION:SIMPLEREGION;
    else
	return (DWORD)TWIN_PrinterEscape(hDC32,nQuery,
		(lppddc->fClippingSet)?(const LPVOID)&lppddc->rcClipping:0,0);
}

DWORD
lsd_printer_getclipbox(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    LPRECT lprc = (LPRECT)&lpStruct->lsde.rect;
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;


    if (!lppddc->fClippingSet) {
	SetRect(lprc,0,0,lppddc->GdiInfo.dpHorzRes,lppddc->GdiInfo.dpVertRes);
	return SIMPLEREGION;
    }
    else {
	*lprc = lppddc->rcClipping;
	return (DWORD)(IsRectEmpty(lprc))?NULLREGION:SIMPLEREGION;
    }
}

DWORD
lsd_printer_getcliprgn(WORD msg, HDC32 hDC32, DWORD dwParam,
			LPLSDS_PARAMS lpStruct)
{
    HRGN hRgn = lpStruct->lsde.region;
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;


    SetRectRgn(hRgn,lppddc->rcClipping.left,lppddc->rcClipping.top,
		lppddc->rcClipping.right,lppddc->rcClipping.bottom);

    return (DWORD)(IsRectEmpty(&lppddc->rcClipping))?0:1;
}

HDC
PrinterResetDC(HDC32 hDC32 ,const DEVMODE *lpdm)
{
    LPPRINTERDRIVERDC lppddc = (LPPRINTERDRIVERDC)hDC32->lpDrvData;

    if (lppddc->bDisableResetDC)
	/* resetDC is disabled between the StartPage and EndPage */
	return (HDC)0;
    else {
#ifdef	LATER
	Really do reset: query escape support for RESETDC escape;
	    call PrinterInitContext
	    reselect printer objects
#endif
    }
    return GETHDC16(hDC32);
}

/*
    PrinterInitContext -- call printer driver to initialize GDIINFO
*/
static BOOL
PrinterInitContext(LPPRINTERDRIVERDC lppddc, DEVMODE *lpdm)
{
    PENABLEPROC lpfnEnable;


    if ( lppddc->hModuleDriver == (HMODULE)0 )
	if ( !(lppddc->hModuleDriver = GetModuleHandle(lppddc->szDrvName)) )
	    if ( !(lppddc->hModuleDriver = LoadDriver(lppddc->szDrvName)) )
		return FALSE;

    if (!(lpfnEnable = (PENABLEPROC)
		GetProcAddress(lppddc->hModuleDriver,ORD_ENABLE)))
	FatalAppExit(0,"Printer driver does not export ENABLE\n");

    /* Call Enable() in driver to copy GDIINFO structure */
    if ( !lpfnEnable(&lppddc->GdiInfo, 1, lppddc->szDevice,
			lppddc->szPort, lpdm) ) {
	return FALSE;
    }

    lppddc->pDevice = (PDEVICE *)WinMalloc(lppddc->GdiInfo.dpDEVICEsize);
    if ( !lppddc->pDevice ) {
	return FALSE;
    }

    memset((LPVOID)lppddc->pDevice, 0, lppddc->GdiInfo.dpDEVICEsize);

    /* Call Enable() in driver to initialize PDEVICE structure */
    if ( !lpfnEnable(lppddc->pDevice, 0, lppddc->szDevice,
			lppddc->szPort, lpdm) ) {
        WinFree((LPSTR)lppddc->pDevice);
	return FALSE;
    }

    return TRUE;
}

static BOOL
PrinterInitCompatDC(LPPRINTERDRIVERDC lppddc, LPPRINTERDRIVERDC lppddcOrig)
{
    LPPSBITMAP lpPSBitmap;

    lppddc->hModuleDriver = lppddcOrig->hModuleDriver;

    lppddc->pDevice = (PDEVICE *)WinMalloc(sizeof(PSBITMAP));
    lpPSBitmap = (LPPSBITMAP)lppddc->pDevice;
    memset((LPSTR)lpPSBitmap,'\0',sizeof(PSBITMAP));

    lppddc->GdiInfo = lppddcOrig->GdiInfo;

    return TRUE;
}

static BOOL 
BruteRealize(HINSTANCE hModule, LPPDEVICE lpDestDev, int iStyle,
		 LPVOID lpLogObj)
{
    PREALIZEPROC lpfnRealize;

    if (!(lpfnRealize = (PREALIZEPROC)
		GetProcAddress(hModule, ORD_REALIZEOBJ)))
	FatalAppExit(0,"Printer driver does not have REALIZEOBJ caps...\n");

    return (lpfnRealize(lpDestDev,iStyle,lpLogObj, NULL,NULL))?TRUE:FALSE;
}

static BOOL 
GDIRealizeObject(HINSTANCE hModule, LPPDEVICE lpDestDev, int iStyle,
		 LPVOID lpLogObj, LPVOID *lpPhObj, LPTEXTXFORM lpTxtXForm)
{
    PREALIZEPROC lpfnRealize;
    DWORD	dwPhSize;
    LPVOID	lpPhObjNew;

    if (!(lpfnRealize = (PREALIZEPROC)
		GetProcAddress(hModule, ORD_REALIZEOBJ)))
	FatalAppExit(0,"Printer driver does not have REALIZEOBJ caps...\n");

    /* The first call to driver's RealizeObject()         */
    /* Get the size required for the private object data. */
    dwPhSize = lpfnRealize(lpDestDev, iStyle, lpLogObj, NULL, NULL);

    if ( dwPhSize == 0x80000000  &&  iStyle == DRV_OBJ_PBITMAP ) {
	return FALSE;
    }

    if ( dwPhSize ) {
	/* Allocate space for the new object data. */
	if (!(lpPhObjNew = (LPVOID)WinMalloc(dwPhSize)))
	    return FALSE;

	/* Realize the object! */
	dwPhSize = lpfnRealize(lpDestDev, iStyle, lpLogObj,
			lpPhObjNew, lpTxtXForm);
	if ( dwPhSize ) {
	    if ( *lpPhObj ) {
		/* Success. Now, delete the previously selected object. */
		lpfnRealize(lpDestDev, -iStyle, *lpPhObj, *lpPhObj, lpTxtXForm);
		WinFree(*lpPhObj);
	    }
	    *lpPhObj = lpPhObjNew;
	    return TRUE;
	}
    }
#ifdef LATER
    else {
	if ( iStyle == DRV_OBJ_PBITMAP )
	    printf("GDIRealizeObject: DRV_OBJ_PBITMAP returns 0\n");
    }
#endif
    return FALSE;
}


static BOOL
OurSpecialForPattern(LPLOGBRUSH lplb, LPBITMAP lpBmp)
{
    LPVOID	lpvBits;
    LONG	cbBuffer;

    if ( !IsGDIObject((HANDLE)lplb->lbHatch) )
	return FALSE;

    GetObject((HBITMAP)lplb->lbHatch, sizeof(BITMAP), (LPBYTE)lpBmp);
    cbBuffer = 2 * ((int)(lpBmp->bmWidth * lpBmp->bmBitsPixel + 15)/16) *
			lpBmp->bmHeight;

    if (!(lpvBits = (LPVOID)WinMalloc(cbBuffer)))
	return FALSE;

    if (!GetBitmapBits((HBITMAP)lplb->lbHatch, cbBuffer, lpvBits)) {
	WinFree(lpvBits);
	return FALSE;
    }

    lpBmp->bmBits = lpvBits;
    lplb->lbColor = (COLORREF)lpBmp;
    return TRUE;
}
