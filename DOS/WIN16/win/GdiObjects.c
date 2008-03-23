/*    
	GdiObjects.c	2.42
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

#include "GdiObjects.h"
#include "Log.h"
#include "Objects.h"
#include "Kernel.h"
#include "GdiDC.h"
#include "GdiText.h" /* FONTINFO */
#include "DeviceData.h"
#include "Driver.h"
#include "WinConfig.h"

/* imported routines */
extern HFONT DriverCreateFont(LPLOGFONT);
extern BOOL GdiDeleteFont(HFONT);
extern HBRUSH GdiCreateBrush(LPLOGBRUSH);
extern BOOL GdiDeleteBrush(HBRUSH);
extern BOOL GdiDeleteBitmap(HBITMAP);
extern HFONT GdiCreateFont(LPLOGFONT lpLogFont);
extern BOOL GdiDeleteRgn(HRGN hRgn);
extern VOID TWIN_MetaNotify(LPOBJHEAD);

/* internal routines */
static HPEN GdiCreatePen(LPLOGPEN);
static BOOL GdiDeletePen(HPEN);
static BOOL GdiDeletePalette(HPALETTE);

HBITMAP TWIN_LoadHatchBitmap(int);
BOOL TWIN_DeleteObject(HGDIOBJ, BOOL);

/* API functions */

int WINAPI
EnumObjects(HDC hDC, int fnObjectType, GOBJENUMPROC goEnumProc,
#ifdef	STRICT
		LPARAM lParam)
#else
		LPSTR lParam)
#endif
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_APICALL,"EnumObjects( HDC=%x,int=%x,ENUMPROC=%x,LPARAM=%x)\n",
		hDC,fnObjectType,goEnumProc,lParam));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.enumobj.fnObjectType = fnObjectType;
    argptr.lsde.enumobj.goenmprc = goEnumProc;
    argptr.lsde.enumobj.lParam = (LPARAM)lParam;

    nRet = (int)DC_OUTPUT(LSD_ENUMOBJECTS,hDC32,0L,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"EnumObjects: return int %d\n", nRet));
    return nRet;
}

HGDIOBJ WINAPI
GetStockObject(int nIndex)
{
    int	nObjType = -1;
    LPOBJHEAD	lpObjHead;
    HANDLE	hObj = 0;
    LOGBRUSH	lBrush;
    LOGPEN	lPen;
    LOGFONT	lFont;
    LPLOGPALETTE lpLogPal;
    int		count;
    HGDIOBJ	ret;

    APISTR((LF_APICALL,"GetStockObject(int=%x)\n",nIndex));

    /* StockObjects is a cache of handles to objects */
    /* they do NOT get removed */

    if(StockObjects[nIndex]) {
	ret = StockObjects[nIndex];
    	APISTR((LF_APIRET,"GetStockObject: return HGDIOBJ %x\n",ret));
	return ret;
    }

    switch (nIndex) {
	case WHITE_BRUSH:
	    nObjType = OT_BRUSH;
	    lBrush.lbStyle = BS_SOLID;
	    lBrush.lbHatch = 0;
    	    lBrush.lbColor = RGB(255,255,255);
	    break;
	case LTGRAY_BRUSH:
	    nObjType = OT_BRUSH;
	    lBrush.lbStyle = BS_SOLID;
	    lBrush.lbHatch = 0;
    	    lBrush.lbColor = RGB(192,192,192);
	    break;
	case GRAY_BRUSH:
	    nObjType = OT_BRUSH;
	    lBrush.lbStyle = BS_SOLID;
	    lBrush.lbHatch = 0;
    	    lBrush.lbColor = RGB(128,128,128);
	    break;
	case DKGRAY_BRUSH:
	    nObjType = OT_BRUSH;
	    lBrush.lbStyle = BS_SOLID;
	    lBrush.lbHatch = 0;
    	    lBrush.lbColor = RGB(64,64,64);
	    break;
	case BLACK_BRUSH:
	    nObjType = OT_BRUSH;
	    lBrush.lbStyle = BS_SOLID;
	    lBrush.lbHatch = 0;
    	    lBrush.lbColor = RGB(0,0,0);
	    break;
	case NULL_BRUSH:
	    nObjType = OT_BRUSH;
	    lBrush.lbStyle = BS_NULL;
	    lBrush.lbHatch = 0;
    	    lBrush.lbColor = 0;
	    break;
	case WHITE_PEN:
	    nObjType = OT_PEN;
	    lPen.lopnColor = RGB(255,255,255);
	    lPen.lopnStyle = PS_SOLID;
	    break;
	case BLACK_PEN:
	    nObjType = OT_PEN;
	    lPen.lopnColor = RGB(0,0,0);
	    lPen.lopnStyle = PS_SOLID;
	    break;
	case NULL_PEN:
	    nObjType = OT_PEN;
	    lPen.lopnColor = 0;
	    lPen.lopnStyle = PS_NULL;
	    break;
	
    	case SYSTEM_FONT:
    	case SYSTEM_FIXED_FONT:
	    nObjType = OT_FONT;
	    memset((LPVOID)&lFont, 0, sizeof(LOGFONT));

	    lFont.lfHeight = GetTwinInt(WCP_FONTSIZE);
            lFont.lfWeight = GetTwinInt(WCP_FONTBOLD);

	    if (nIndex == SYSTEM_FIXED_FONT)
		strcpy(lFont.lfFaceName,"fixed");
	    else
		GetTwinString(WCP_FONTFACE,lFont.lfFaceName,LF_FACESIZE);
	    break;

	case ANSI_FIXED_FONT:
	    nObjType = OT_FONT;
	    memset((LPVOID)&lFont, 0, sizeof(LOGFONT));
	    lFont.lfHeight = 12;
	    lFont.lfWidth  = 9;
	    lFont.lfClipPrecision = CLIP_STROKE_PRECIS;
	    lFont.lfPitchAndFamily = FIXED_PITCH;
	    strcpy(lFont.lfFaceName, "Courier");
	    break;
    	case ANSI_VAR_FONT:
	    nObjType = OT_FONT;
	    memset((LPVOID)&lFont, 0, sizeof(LOGFONT));
	    lFont.lfHeight = 12;
	    lFont.lfWidth  = 9;
	    lFont.lfClipPrecision = CLIP_STROKE_PRECIS;
	    lFont.lfPitchAndFamily = VARIABLE_PITCH;
	    strcpy(lFont.lfFaceName, "Helv");
	    break;
	case DEVICE_DEFAULT_FONT:
	    nObjType = OT_FONT;
	    memset((LPVOID)&lFont, 0, sizeof(LOGFONT));
	    lFont.lfPitchAndFamily = FIXED_PITCH;
	    break;
	case OEM_FIXED_FONT:
	    nObjType = OT_FONT;
	    memset((LPVOID)&lFont, 0, sizeof(LOGFONT));
	    lFont.lfHeight = 12;
	    lFont.lfWidth  = 8;
	    lFont.lfCharSet = 0xff;
	    lFont.lfClipPrecision = CLIP_STROKE_PRECIS;
	    lFont.lfPitchAndFamily = FIXED_PITCH;
	    strcpy(lFont.lfFaceName, "Terminal");
	    break;

	case DEFAULT_PALETTE:
	    nObjType = OT_PALETTE;
	    lpLogPal = (LPLOGPALETTE)WinMalloc(sizeof(LOGPALETTE)+
				sizeof(PALETTEENTRY)*20);
	    lpLogPal->palVersion = 0x300;
	    lpLogPal->palNumEntries = 20;
	    for (count = 0; count < 8; count ++) {
		    lpLogPal->palPalEntry[count].peRed =
				VGAColors[count].rgbRed;
		    lpLogPal->palPalEntry[count].peGreen =
				VGAColors[count].rgbGreen;
		    lpLogPal->palPalEntry[count].peBlue =
				VGAColors[count].rgbBlue;
		    lpLogPal->palPalEntry[count].peFlags = 0;
	    }
	    for (count = 8; count < 12; count ++) {
		    lpLogPal->palPalEntry[count].peRed =
				NiceColors[count-8].rgbRed;
		    lpLogPal->palPalEntry[count].peGreen =
				NiceColors[count-8].rgbGreen;
		    lpLogPal->palPalEntry[count].peBlue =
				NiceColors[count-8].rgbBlue;
		    lpLogPal->palPalEntry[count].peFlags = 0;
	    }
	    for (count = 12; count < 20; count ++) {
		    lpLogPal->palPalEntry[count].peRed =
				VGAColors[count-4].rgbRed;
		    lpLogPal->palPalEntry[count].peGreen =
				VGAColors[count-4].rgbGreen;
		    lpLogPal->palPalEntry[count].peBlue =
				VGAColors[count-4].rgbBlue;
		    lpLogPal->palPalEntry[count].peFlags = 0;
	    }
	    hObj = (HANDLE)CreatePalette(lpLogPal);
	    WinFree((LPSTR)lpLogPal);
	    break;

	case SYSTEM_BITMAP:
	    hObj = (HANDLE)CreateBitmap(1,1,1,1,NULL);
	    break;

	default:
    	    APISTR((LF_APIRET,"GetStockObject: return HGDIOBJ %x\n",hObj));
	    return hObj;
    }

    if (!hObj) {
	switch(nObjType) {
	    case OT_BRUSH:
    		hObj = CreateBrushIndirect(&lBrush);
		break;
	    case OT_PEN:
		lPen.lopnWidth.x = 1;
		lPen.lopnWidth.y = 1;
    		hObj = CreatePenIndirect(&lPen);
		break;
	    case OT_FONT:
		hObj = (HANDLE)CreateFontIndirect(&lFont);
		break;
	    default:
		break;
	}
    }

    if (hObj) {
	StockObjects[nIndex] = hObj;
	lpObjHead = GETGDIINFO(hObj);
	lpObjHead->wRefCount = (WORD)-1;
    }

    APISTR((LF_APIRET,"GetStockObject: return HGDIOBJ %x\n",hObj));
    return hObj;
}

HBRUSH WINAPI
CreateSolidBrush(COLORREF crColor)
{
    LOGBRUSH lBrush;
    HBRUSH   hBrush;

    APISTR((LF_APICALL,"CreateSolidBrush(COLORREF==%x)\n",crColor));

    lBrush.lbStyle = BS_SOLID;
    lBrush.lbColor = crColor;
    lBrush.lbHatch = 0;

    hBrush = CreateBrushIndirect(&lBrush);
    APISTR((LF_APIRET,"CreateSolidBrush: return HBRUSH %x\n",hBrush));
    return hBrush;
}

HBRUSH WINAPI
CreateHatchBrush(int nIndex,COLORREF crColor)
{
    LOGBRUSH lBrush;
    HBRUSH   hBrush;

    APISTR((LF_APICALL,"CreateHatchBrush(int=%x,COLORREF=%x)\n",
	nIndex,crColor));

    lBrush.lbStyle = BS_HATCHED;
    lBrush.lbColor = crColor;
    lBrush.lbHatch = nIndex;

    hBrush = CreateBrushIndirect(&lBrush);
    APISTR((LF_APIRET,"CreateHatchBrush: return HBRUSH %x\n",hBrush));
    return hBrush;
}

HBRUSH WINAPI
CreatePatternBrush(HBITMAP hBitmap)
{
    LOGBRUSH lBrush;
    HBRUSH   hBrush;

    APISTR((LF_APICALL,"CreatePatternBrush(HBITMAP=%x)\n",hBitmap));

    lBrush.lbStyle = BS_PATTERN;
    lBrush.lbColor = 0;
    lBrush.lbHatch = (int)hBitmap;

    hBrush = CreateBrushIndirect(&lBrush);
    APISTR((LF_APIRET,"CreatePatternBrush: return HBRUSH %x\n",hBrush));
    return hBrush;
}

HBRUSH WINAPI
CreateDIBPatternBrush(HGLOBAL hPacked, UINT fnColorSpec)
{
    LOGBRUSH lBrush;
    HBRUSH   hBrush;

    APISTR((LF_APICALL,"CreateDIBPatternBrush(HGLOBAL=%x,UINT=%x)\n",
	hPacked,fnColorSpec));

    lBrush.lbStyle = BS_DIBPATTERN;
    lBrush.lbColor = (COLORREF)fnColorSpec;
    lBrush.lbHatch = (int)hPacked;
    hBrush = CreateBrushIndirect(&lBrush);
    APISTR((LF_APIRET,"CreateDIBPatternBrush: return HBRUSH %x\n",hBrush));
    return hBrush;
}

HBRUSH WINAPI
CreateBrushIndirect(LPLOGBRUSH lpLogBrush)
{
    HBRUSH hBrush;
    APISTR((LF_APICALL,"CreateDIBPatternBrush(LPLOGBRUSH=%x)\n",lpLogBrush));

    hBrush = (HBRUSH)GdiCreateBrush(lpLogBrush);
    APISTR((LF_APIRET,"CreateBrushIndirect: return HBRUSH %x\n",hBrush));
    return hBrush;
}

HPEN WINAPI
CreatePen(int nPenStyle, int nWidth, COLORREF crColor)
{
    LOGPEN lPenInfo;
    HPEN   hPen;

    APISTR((LF_APICALL,"CreatePen(int=%d,int=%d,COLORREF=%x)\n",
		nPenStyle,nWidth,crColor));

    lPenInfo.lopnStyle = nPenStyle;
    lPenInfo.lopnWidth.x = nWidth;
    lPenInfo.lopnWidth.y = nWidth;
    lPenInfo.lopnColor = crColor;

    hPen = CreatePenIndirect(&lPenInfo);
    APISTR((LF_APIRET,"CreatePen: return HPEN %x\n",hPen));
    return hPen;
}

HPEN WINAPI
CreatePenIndirect(LPLOGPEN lpLogPen)
{
    HPEN hPen;
    APISTR((LF_APICALL,"CreatePenIndirect(LPLOGPEN=%x)\n",lpLogPen));

    hPen = (HPEN)GdiCreatePen(lpLogPen);
    APISTR((LF_APIRET,"CreatePenIndirect: return HPEN %x\n",hPen));
    return hPen;
}

int WINAPI 
AddFontResource(LPCSTR lpcstr)
{
    APISTR((LF_APISTUB,"AddFontResource(LPCSTR=%s) return int 1\n",lpcstr));
    return 1;
}

BOOL WINAPI
RemoveFontResource(LPCSTR lpcstr)
{
    APISTR((LF_APISTUB,"RemoveFontResource(LPCSTR=%s) return BOOL TRUE\n",lpcstr));
    return TRUE;
}

HFONT WINAPI
CreateFont(int nHeight, int nWidth, int nEsc, int nOrient,
	   int nWeight, BYTE cItalic, BYTE cUnder, BYTE cStrike,
	   BYTE cCharSet, BYTE cOutPrec, BYTE cClipPrec, BYTE cQuality,
	   BYTE cPitchFamily, LPCSTR FaceName)
{
    LOGFONT LogFont;
    HFONT   hFont;

    APISTR((LF_APICALL,"CreateFont(int=%d,int=%d,int=%d,int=%d,int=%d,BYTE=%d,BYTE=%d,BYTE=%d,BYTE=%d\
	BYTE=%d,BYTE=%d,BYTE=%d,BYTE=%d,LPCSTR=%s )\n",
		nHeight,nWidth,nEsc,nOrient,nWeight,cItalic,cUnder,cStrike,
		cCharSet,cOutPrec,cClipPrec,cQuality,cPitchFamily,
		FaceName?FaceName:"NULL"));

    memset((LPVOID)&LogFont,'\0',sizeof(LOGFONT));
    LogFont.lfHeight        = nHeight;
    LogFont.lfWidth         = nWidth;
    LogFont.lfEscapement    = nEsc;
    LogFont.lfOrientation   = nOrient;
    LogFont.lfWeight        = nWeight;
    LogFont.lfItalic        = cItalic;
    LogFont.lfUnderline     = cUnder;
    LogFont.lfStrikeOut     = cStrike;
    LogFont.lfCharSet	    = cCharSet;
    LogFont.lfOutPrecision  = cOutPrec;
    LogFont.lfClipPrecision = cClipPrec;
    LogFont.lfQuality       = cQuality;
    LogFont.lfPitchAndFamily = cPitchFamily;
    if (FaceName && FaceName[0])
	strncpy(LogFont.lfFaceName,FaceName,
			min(strlen(FaceName),LF_FACESIZE-1));
    else
	strcpy(LogFont.lfFaceName,"Helv");

    hFont = GdiCreateFont(&LogFont);
    APISTR((LF_APIRET,"CreateFont: return HFONT %x\n",hFont));
    return hFont;
}

HFONT WINAPI
CreateFontIndirect(const LOGFONT FAR *lpLogFont)
{
    HFONT hFont;
    APISTR((LF_APICALL,"CreateFontIndirect(LOGFONT *%x)\n",lpLogFont));

    hFont = GdiCreateFont((LPLOGFONT)lpLogFont);
    APISTR((LF_APIRET,"CreateFontIndirect: return HFONT %x\n",hFont));
    return hFont;
}

int WINAPI
EnumFonts(HDC hDC, LPCSTR lpcstr, OLDFONTENUMPROC fenumproc, LPARAM lParam)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_APICALL,"EnumFonts(HDC=%x,LPCSTR=%s, ENUMPROC=%x,LPARAM=%x)\n",
	hDC,lpcstr?lpcstr:"NULL",fenumproc,lParam));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.enumfonts.hDC32 = hDC32;
    argptr.lsde.enumfonts.lpszFamily = (LPSTR)lpcstr;
    argptr.lsde.enumfonts.fntenmprc = (FONTENUMPROC)fenumproc;
    argptr.lsde.enumfonts.lParam = (LPARAM)lParam;
    nRet = (int)DC_OUTPUT(LSD_ENUMFONTS,hDC32,0L,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"EnumFonts: return int %x\n",nRet));
    return nRet;
}

int     WINAPI 
EnumFontFamilies(HDC hDC, LPCSTR lpcstr, FONTENUMPROC fenumproc, LPARAM lParam)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_APICALL,"EnumFontFamilies(HDC=%x,LPCSTR=%s, ENUMPROC=%x,LPARAM=%x)\n",
	hDC,lpcstr?lpcstr:"NULL",fenumproc,lParam));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.enumfonts.hDC32 = hDC32;
    argptr.lsde.enumfonts.lpszFamily = (LPSTR)lpcstr;
    argptr.lsde.enumfonts.fntenmprc = (FONTENUMPROC)fenumproc;
    argptr.lsde.enumfonts.lParam = (LPARAM)lParam;
    nRet = (int)DC_OUTPUT(LSD_ENUMFONTS,hDC32,1L,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"EnumFontFamilies: return int %x\n",nRet));
    return nRet;
}

BOOL WINAPI 
IsGDIObject(HGDIOBJ hgdi)
{
    BOOL bRet;
    APISTR((LF_APICALL,"IsGDIObject(HGDIOBJ=%x)\n",hgdi));

    bRet = (CHECKGDIINFO(hgdi))?TRUE:FALSE;
    RELEASEGDIINFO(hgdi);

    APISTR((LF_APIRET,"IsGDIObject: return BOOL %x\n",bRet));
    return bRet;
}

int WINAPI
GetObject(HGDIOBJ hObject, int nObjectSize, LPVOID lpObject)
{
    WORD        	objType;
    LPOBJHEAD   	lpObjInfo;
    BITMAP		Bitmap;
    LPLOGBRUSH		lpLogBrush;
    LPLOGPEN		lpLogPen;
    LPEXTLOGPEN		lpExtLogPen;
    LPLOGFONT   	lpf;
    LPIMAGEINFO   	lpimage;
    LPPALETTE		lpPalette;

    APISTR((LF_APICALL,"GetObject(HGDIOBJ=%x,int=%d,LPVOID %p)\n",
	hObject, nObjectSize, lpObject));

    if (!(lpObjInfo = GETGDIINFO(hObject)))
    {
	SetLastErrorEx(1, 0);
        APISTR((LF_APIFAIL,"GetObject: return int 0\n"));
	return 0;
    }
	
    objType = GET_OBJECT_TYPE(lpObjInfo);

    switch (objType)
    {
	case OT_BRUSH:
		if (!lpObject)
		{
			nObjectSize = sizeof(LOGBRUSH);
			break;
		}
		if (nObjectSize < sizeof(LOGBRUSH))
		{
			nObjectSize = 0;
			break;
		}
		lpLogBrush = (LPLOGBRUSH)lpObject;
		*lpLogBrush = ((LPBRUSHINFO)lpObjInfo)->lpBrush; 
		break;

	case OT_PEN:
		if (!lpObject)
		{
			nObjectSize = sizeof(LOGPEN);
			break;
		}
		if (nObjectSize < sizeof(LOGPEN))
		{
			nObjectSize = 0;
			break;
		}
		if (((LPPENINFO)lpObjInfo)->lpExtPen == NULL)
		{
			/* LOGPEN */
			lpLogPen = (LPLOGPEN)lpObject;
			*lpLogPen = ((LPPENINFO)lpObjInfo)->lpPen;
		}
		else
		{
			/* EXTLOGPEN */
			lpExtLogPen = (LPEXTLOGPEN)lpObject;
			*lpExtLogPen = *(((LPPENINFO)lpObjInfo)->lpExtPen);
		}
		break;

	case OT_FONT:
		if (!lpObject)
		{
			nObjectSize = sizeof(LOGFONT);
			break;
		}
		if (nObjectSize < sizeof(LOGFONT))
		{
			nObjectSize = 0;
			break;
		}
                lpf = (LPLOGFONT) lpObject;
                *lpf = ((LPFONTINFO)lpObjInfo)->LogFont;
		lpf->lfCharSet = ANSI_CHARSET;
		lpf->lfPitchAndFamily = VARIABLE_PITCH|FF_SWISS;
                break;

	case OT_BITMAP:
		lpimage = (LPIMAGEINFO) lpObjInfo;

		Bitmap.bmType       = 0;
		Bitmap.bmWidth      = lpimage->ImageWidth;
		Bitmap.bmHeight     = lpimage->ImageHeight;
		Bitmap.bmPlanes     = lpimage->ImagePlanes;
		Bitmap.bmBitsPixel  = lpimage->ImageDepth;
		Bitmap.bmWidthBytes = lpimage->WidthBytes; 
		Bitmap.bmBits       = 0;

		memcpy(lpObject,(LPSTR)&Bitmap,min(nObjectSize,sizeof(BITMAP)));

		/* (WIN32) DIBSECTION */
		if ((lpimage->lpdsBmi != NULL)
		 && (nObjectSize >= sizeof(DIBSECTION)))
		{
			/* Copy everything to DIBSECTION except initial
			 * BITMAP structure, which was already copied by
			 * the above code.
			 */
			((DIBSECTION FAR *)lpObject)->dsBm.bmBits
				= lpimage->ds.dsBm.bmBits;
			((DIBSECTION FAR *)lpObject)->dsBmih
				= lpimage->ds.dsBmih;
			((DIBSECTION FAR *)lpObject)->dsBitfields[0]
				= lpimage->ds.dsBitfields[0];
			((DIBSECTION FAR *)lpObject)->dsBitfields[1]
				= lpimage->ds.dsBitfields[1];
			((DIBSECTION FAR *)lpObject)->dsBitfields[2]
				= lpimage->ds.dsBitfields[2];
			((DIBSECTION FAR *)lpObject)->dshSection
				= lpimage->ds.dshSection;
			((DIBSECTION FAR *)lpObject)->dsOffset
				= lpimage->ds.dsOffset;
		}

		break;

	case OT_PALETTE:
		if (!lpObject)
		{
			nObjectSize = sizeof(UINT);
			break;
		}
		if (nObjectSize < sizeof(UINT))
		{
			nObjectSize = 0;
			break;
		}
		lpPalette = (LPPALETTE) lpObjInfo;
		*((UINT *) lpObject) = lpPalette->lpLogPalette->palNumEntries;
		break;

	default:
		SetLastErrorEx(1, 0);
		nObjectSize = 0;
		break;
    }

    RELEASEGDIINFO(lpObjInfo);
    APISTR((LF_APIRET,"GetObject: return int %x\n",nObjectSize));

    return nObjectSize;
}

BOOL WINAPI
UnrealizeObject(HGDIOBJ hGDIObj)
{
    LPOBJHEAD   lpObjInfo;
    WORD        wObjType;
    BOOL bRet;

    APISTR((LF_APICALL,"UnrealizeObject(HGDIOBJ=%x)\n",hGDIObj));

    if (!(lpObjInfo = GETGDIINFO(hGDIObj))) {
    	APISTR((LF_APIFAIL,"UnrealizeObject: return BOOL %x\n",FALSE));
       	return FALSE;
    }
	
    wObjType= GET_OBJECT_TYPE(lpObjInfo);

    if (wObjType == OT_BRUSH || wObjType == OT_PALETTE) {
	/* say the object is not realized */
	((LPGDIOBJ)lpObjInfo)->fIsRealized = FALSE;
	bRet = TRUE;
    }
    else
	bRet = FALSE;

    RELEASEGDIINFO(hGDIObj);

    APISTR((LF_APIRET,"UnrealizeObject: return BOOL %x\n",bRet));
    return bRet;
}

BOOL WINAPI
DeleteObject(HGDIOBJ hObject)
{
    BOOL bRet;
    APISTR((LF_APICALL,"DeleteObject(HGDIOBJ=%x)\n",hObject));
    bRet = TWIN_DeleteObject(hObject, TRUE);
    APISTR((LF_APIRET,"DeleteObject: return BOOL %x\n",bRet));
    return bRet;
}

BOOL
TWIN_DeleteObject(HGDIOBJ hObject, BOOL fNotifyMetafile)
{
    LPOBJHEAD   lpObjInfo;
    WORD	wObjType;
    BOOL	bRet = FALSE;

    if (hObject == 0)
	return FALSE;

    if (!(lpObjInfo = (LPOBJHEAD)GETGDIINFO(hObject))) {
	return FALSE;
    }

    wObjType = GET_OBJECT_TYPE(lpObjInfo);

    switch (wObjType) {
	case OT_FONT:
	    if (lpObjInfo->wRefCount == 0) {
		if (fNotifyMetafile)
		    TWIN_MetaNotify(lpObjInfo);
		bRet = GdiDeleteFont((HFONT)hObject);
	    }
	    break;

	case OT_REGION:
	    if (lpObjInfo->wRefCount == 0) {
		bRet = GdiDeleteRgn((HRGN)hObject);
	    }
	    break;

	case OT_PEN:
	    if (lpObjInfo->wRefCount == 0) {
		if (fNotifyMetafile)
		    TWIN_MetaNotify(lpObjInfo);
		bRet = GdiDeletePen((HPEN)hObject);
	    }
	    break;

	case OT_BRUSH:
	    if (lpObjInfo->wRefCount == 0) {
		if (fNotifyMetafile)
		    TWIN_MetaNotify(lpObjInfo);
		bRet = GdiDeleteBrush((HBRUSH)hObject);
	    }
	    break;

	case OT_PALETTE:
	    if (lpObjInfo->wRefCount == 0) {
		bRet = GdiDeletePalette((HPALETTE)hObject);
	    }
	    break;

	case OT_BITMAP:
	    RELEASEGDIINFO(lpObjInfo);
	    if (lpObjInfo->wRefCount == 0) {
		bRet = GdiDeleteBitmap((HBITMAP)hObject);
	    }
	    else {
		if (((LPGDIOBJ)lpObjInfo)->fIsRealized)
		    ((LPGDIOBJ)lpObjInfo)->fIsRealized = FALSE;
		bRet = TRUE;
	    }
	    if (bRet)
	        FREEGDI(hObject);
	    return bRet;

	case OT_DC:
	    RELEASEGDIINFO(lpObjInfo);
	    if (lpObjInfo->wRefCount == 0) {
		return DeleteDC((HDC)hObject);
	    }
	    return FALSE;

	default:
	    break;
    }

    RELEASEGDIINFO(lpObjInfo);

    if (bRet)
	FREEGDI(hObject);

    return bRet;
}

/********************************************************************/
/*		Internal routines				    */
/********************************************************************/

static HPEN
GdiCreatePen(LPLOGPEN lpPen)
{
    LPPENINFO lpPenInfo;
    HPEN hPen;

    if (!(lpPenInfo = CREATEHPEN(hPen))) {
	return (HPEN)0;
    }
    lpPenInfo->lpPen.lopnStyle   = lpPen->lopnStyle;
    lpPenInfo->lpPen.lopnWidth.x = lpPen->lopnWidth.x;
    lpPenInfo->lpPen.lopnWidth.y = lpPen->lopnWidth.y;
    lpPenInfo->lpPen.lopnColor   = lpPen->lopnColor;
    lpPenInfo->lpExtPen = NULL;

    RELEASEPENINFO(lpPenInfo);

    return hPen;
}

static HPEN
GdiExtCreatePen(LPEXTLOGPEN lpExtPen, DWORD dwNumEntries, LPDWORD lpStyleEntry)
{
	LPPENINFO lpPenInfo;
	HPEN hPen;
	DWORD i;

	if ((lpPenInfo = CREATEHPEN(hPen)) == NULL)
	{
		SetLastErrorEx(1, 0);
		return ((HPEN) 0);
	}
	lpPenInfo->lpPen.lopnStyle = 0;
	lpPenInfo->lpPen.lopnWidth.x = 0;
	lpPenInfo->lpPen.lopnWidth.y = 0;
	lpPenInfo->lpPen.lopnColor = 0;
	lpPenInfo->lpExtPen = (LPEXTLOGPEN) WinMalloc(sizeof(EXTLOGPEN)
		+ dwNumEntries * sizeof(DWORD));
	if (lpPenInfo->lpExtPen == NULL)
	{
		SetLastErrorEx(1, 0);
		DeleteObject(hPen);
		RELEASEPENINFO(lpPenInfo);
		return ((HPEN) 0);
	}
	lpPenInfo->lpExtPen->elpPenStyle   = lpExtPen->elpPenStyle;
	lpPenInfo->lpExtPen->elpWidth      = lpExtPen->elpWidth;
	lpPenInfo->lpExtPen->elpBrushStyle = lpExtPen->elpBrushStyle;
	lpPenInfo->lpExtPen->elpColor      = lpExtPen->elpColor;
	lpPenInfo->lpExtPen->elpHatch      = lpExtPen->elpHatch;
	lpPenInfo->lpExtPen->elpNumEntries = dwNumEntries;
	for (i = 0; i < dwNumEntries; i++)
		lpPenInfo->lpExtPen->elpStyleEntry[i] = lpStyleEntry[i];

	RELEASEPENINFO(lpPenInfo);

	return (hPen);

}

static BOOL
GdiDeletePen(HPEN hPen)
{
	LPPENINFO lpPenInfo;

	if ((lpPenInfo = GETPENINFO(hPen)) == NULL)
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}
	if (lpPenInfo->lpExtPen)
		WinFree((LPSTR) lpPenInfo->lpExtPen);

	RELEASEPENINFO(lpPenInfo);

	return (TRUE);
}

static BOOL
GdiDeletePalette(HPALETTE hPalette)
{
	LPPALETTE lpPalette;

	ASSERT_PALETTE(lpPalette, hPalette, FALSE);

	if (lpPalette->lpLogPalette)
	{
		WinFree(lpPalette->lpLogPalette);
	}

	UNLOCK_PALETTE(hPalette);

	return (TRUE);
}

/* (WIN32) GDI Pen ********************************************************* */

HPEN	WINAPI
ExtCreatePen(DWORD dwPenStyle, DWORD dwPenWidth,
	LPLOGBRUSH lpLogBrush,
	DWORD dwNumEntries,
	LPDWORD lpStyleEntry)
{
	BOOL bError;
	EXTLOGPEN elp;
	HPEN	  hPen = 0;

        APISTR((LF_APICALL,"ExtCreatePen(DWORD=%x,DWORD=%x,LPLOGBRUSH=%x,DWORD=%x,LPDWORD=%x)\n",
		dwPenStyle,dwPenWidth,lpLogBrush,dwNumEntries,lpStyleEntry));
	/* error checking */
	bError = FALSE;
	switch (dwPenStyle & PS_TYPE_MASK)
	{
	case PS_GEOMETRIC:
		if (lpLogBrush == NULL)
			bError = TRUE;
		break;
	case PS_COSMETIC:
		if (((dwPenStyle & PS_STYLE_MASK) == PS_INSIDEFRAME)
		 || ((dwPenStyle & PS_ENDCAP_MASK) != 0)
		 || ((dwPenStyle & PS_JOIN_MASK) != 0)
		 || (dwPenWidth != 1)
		 || (lpLogBrush == NULL)
		 || (lpLogBrush->lbStyle != BS_SOLID))
			bError = TRUE;
		break;
	default:
		bError = TRUE;
		break;
	}
	if (((dwPenStyle & PS_STYLE_MASK) != PS_USERSTYLE)
	 && ((dwNumEntries != 0) || (lpStyleEntry != NULL)))
		bError = TRUE;
	if (bError)
	{
		SetLastErrorEx(1, 0);
    		APISTR((LF_APIFAIL,"ExtCreatePen: return HPEN %x\n",hPen));
		return ((HPEN) hPen);
	}

	/* create extended logical pen */
	elp.elpPenStyle = dwPenStyle;
	elp.elpWidth = dwPenWidth;
	switch (elp.elpBrushStyle = lpLogBrush->lbStyle)
	{
	case BS_SOLID:
		elp.elpColor = lpLogBrush->lbColor;
		elp.elpHatch = 0;		/* ignored */
		break;
	case BS_HATCHED:
		elp.elpColor = lpLogBrush->lbColor;
		elp.elpHatch = lpLogBrush->lbHatch;
		break;
	case BS_HOLLOW:
		elp.elpColor = 0;		/* ignored */
		elp.elpHatch = 0;		/* ignored */
		break;
	case BS_PATTERN:
		elp.elpColor = 0;		/* ignored */
		elp.elpHatch = lpLogBrush->lbHatch;
		break;
	case BS_DIBPATTERN:
		elp.elpColor = lpLogBrush->lbColor;
		elp.elpHatch = lpLogBrush->lbHatch;
		break;
	case BS_DIBPATTERNPT:
		elp.elpColor = lpLogBrush->lbColor;
		elp.elpHatch = lpLogBrush->lbHatch;
		break;
	default:
		SetLastErrorEx(1, 0);
    		APISTR((LF_APIFAIL,"ExtCreatePen: return HPEN %x\n",hPen));
		return ((HPEN) hPen);
	}
	hPen = GdiExtCreatePen(&elp, dwNumEntries, lpStyleEntry);
    	APISTR((LF_APIRET,"ExtCreatePen: return HPEN %x\n",hPen));
	return hPen;
}


void
TWIN_DeleteStockObjects()
{
	int i;
	LPOBJHEAD     lpObjHead;

	for(i=0;i<20;i++) {
		if(StockObjects[i]) {
			lpObjHead = GETGDIINFO(StockObjects[i]);
			lpObjHead->wRefCount = 0;
			DeleteObject(StockObjects[i]);
		}
	}
}
