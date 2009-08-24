/*    
	GdiBitmaps.c	2.45
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
#include "windowsx.h"

#include "GdiObjects.h"
#include "Kernel.h"
#include "ObjEngine.h"
#include "Resources.h"
#include "GdiDC.h"
#include "Log.h"
#include "Driver.h"

/* This file contains most of high-level entry points to */
/* bitmap-related APIs.					 */

extern BYTE bit_reverse[];

/* exported stuff */
LONG TWIN_GetImageBits(HBITMAP, LONG, LPVOID, int);
LONG TWIN_SetImageBits(HBITMAP, DWORD, const void *, int);
HICON LoadIconResource(LPNAMEINFO);
HCURSOR LoadCursorResource(LPNAMEINFO);
HCURSOR GetDragIconCursor(HWND);

extern HBITMAP CreateDIBitmapEx(LPBITMAPIMAGE,LPNAMEINFO);
extern BOOL CreateDIBImage(LPIMAGEINFO,BITMAPINFOHEADER *,
		RGBQUAD *,LPVOID);
extern BOOL CreateDDBImage(LPIMAGEINFO,BYTE,BYTE,const LPBYTE);
extern void ConvertMonoDDBToImage(LPBYTE,LPBYTE,int,int,int);
extern BOOL ConvertDIBFormat(LPIMAGEINFO,BITMAPINFOHEADER *,
		RGBQUAD *,LPVOID,UINT,UINT);
extern BOOL FetchDIBits(HDC,LPIMAGEINFO,UINT,UINT,
                BITMAPINFOHEADER *,RGBQUAD *,BYTE *);
extern int CalcByteWidth(int,int,int);
LPRGBQUAD TWIN_MapPaletteColors(HDC,LPBITMAPINFO);

DWORD WINAPI
SetBitmapDimension(HBITMAP hBitmap,int nWidth,int nHeight)
{
    DWORD	 dwSize;
    SIZE	 lpSize;

    if (SetBitmapDimensionEx(hBitmap,nWidth,nHeight,&lpSize))
	dwSize = MAKELONG(lpSize.cx,lpSize.cy);
    else
	dwSize = 0;
    return dwSize;    
}

BOOL WINAPI
SetBitmapDimensionEx(HBITMAP hBitmap,int nWidth,int nHeight,SIZE FAR *lpsize)
{
    LPIMAGEINFO lpimage;

    if (!(lpimage = GETBITMAPINFO(hBitmap)))
	return (FALSE);

    if(lpsize) {
	lpsize->cx = (int)(short)LOWORD(lpimage->dimension);
	lpsize->cy = (int)(short)HIWORD(lpimage->dimension);
    }
    lpimage->dimension = MAKELONG(nWidth, nHeight);

    RELEASEBITMAPINFO(lpimage);

    return TRUE;

}

DWORD WINAPI
GetBitmapDimension(HBITMAP hBitmap)
{
    SIZE	 lpSize;

    if (GetBitmapDimensionEx(hBitmap,&lpSize)) 
	return MAKELONG(lpSize.cx,lpSize.cy);
    else
	return 0;
}
    
BOOL WINAPI
GetBitmapDimensionEx(HBITMAP hBitmap,SIZE FAR *lpSize)
{
    LPIMAGEINFO lpimage;

    if (!(lpimage = GETBITMAPINFO(hBitmap))) 
	return FALSE;

    if(lpSize) {
	lpSize->cx = (int)(short)LOWORD(lpimage->dimension);
	lpSize->cy = (int)(short)HIWORD(lpimage->dimension);
    }

    RELEASEBITMAPINFO(lpimage);

    return TRUE;	

}

int     WINAPI 
SetDIBits(HDC hDC, HBITMAP hBitmap, UINT uStartScan, UINT cScanLines, 
    const void *lpvbits, BITMAPINFO *lpbmi, UINT fuColorUse)
{
    LPIMAGEINFO     lpimage;
    LPRGBQUAD	    lpRGB;

    if ((lpimage = GETBITMAPINFO(hBitmap)) == 0)
	return 0;

    APISTR((LF_APICALL,"SetDIBits(HDC=%x,HBITMAP=%x,UINT=%d,UINT=%d,VOID *%x,BITMAPINFO *%x,UINT=%s\n",
	hDC,hBitmap,uStartScan,cScanLines,lpvbits,lpbmi,
	fuColorUse==DIB_RGB_COLORS?"DIB_RGB_COLORS":"DIB_PAL_COLORS"));


    if (fuColorUse == DIB_PAL_COLORS) 
	lpRGB = TWIN_MapPaletteColors(hDC,lpbmi);
    else
	lpRGB = &lpbmi->bmiColors[0];

    ConvertDIBFormat(lpimage, &lpbmi->bmiHeader, lpRGB, 
			(LPBYTE)lpvbits,uStartScan,cScanLines);

    if (fuColorUse == DIB_PAL_COLORS) 
	WinFree((LPSTR)lpRGB);

    RELEASEBITMAPINFO(lpimage);

    APISTR((LF_APIRET,"SetDIBits: returns int %d\n",cScanLines));
    return cScanLines;
}

int WINAPI 
GetDIBits(HDC hDC, HBITMAP hBitmap, 
	UINT nStartScan, UINT cScanLines, 
	LPVOID lpvBits, BITMAPINFO *lpbmi, UINT fuColorUse)
{
    LPIMAGEINFO lpImage;
    int rc = 0;

    APISTR((LF_APICALL,"GetDIBits(HDC=%x,HBITMAP=%x,UINT=%d,UINT=%d,VOID *%x,BITMAPINFO *%x,UINT=%s\n",
	hDC,hBitmap,nStartScan,cScanLines,lpvBits,lpbmi,
	fuColorUse==DIB_RGB_COLORS?"DIB_RGB_COLORS":"DIB_PAL_COLORS"));

    if (!(lpImage = (LPIMAGEINFO)GETBITMAPINFO(hBitmap))) {
        APISTR((LF_APIRET,"SetDIBits: returns int %d\n",rc));
	return rc;
    }

    if (!(lpbmi->bmiHeader.biSizeImage = CalcByteWidth(
	    lpbmi->bmiHeader.biWidth,
	    lpbmi->bmiHeader.biBitCount,
	    lpImage->LinePad))) {
        APISTR((LF_APIRET,"GetDIBits: returns int %d\n",rc));
	return rc;
    }

    lpbmi->bmiHeader.biSizeImage *= lpbmi->bmiHeader.biHeight;

#ifdef    LATER
fill lpbmi->bmiHeader.biCompression,lpbmi->bmiHeader.biXPelsPerMeter,
    lpbmi->bmiHeader.biYPelsPerMeter,biClrUsed,biClrImportant
#else
    lpbmi->bmiHeader.biCompression = lpbmi->bmiHeader.biXPelsPerMeter =
    lpbmi->bmiHeader.biYPelsPerMeter = lpbmi->bmiHeader.biClrUsed = 
    lpbmi->bmiHeader.biClrImportant = 0;
#endif

    if (!lpvBits) {
	rc =  lpbmi->bmiHeader.biHeight;
        APISTR((LF_APIRET,"GetDIBits: returns int %d\n",rc));
	return rc;
    }

    if (FetchDIBits(hDC,lpImage,nStartScan,cScanLines,
	    &lpbmi->bmiHeader,
	    lpbmi->bmiColors,(LPBYTE)lpvBits)) {
	rc =  cScanLines;
    } 

    APISTR((LF_APIRET,"GetDIBits: returns int %d\n",cScanLines));
    return rc;
}

HBITMAP WINAPI 
#ifdef TWIN32
CreateDIBitmap(HDC hDC, const BITMAPINFOHEADER *lpbmih, DWORD dwInit,
	const void *initbits, const BITMAPINFO *lpbmi, UINT colors)
#else
CreateDIBitmap(HDC hDC, BITMAPINFOHEADER *lpbmih, DWORD dwInit,
	const void *initbits, BITMAPINFO *lpbmi, UINT colors)
#endif
{
    LPIMAGEINFO lpimage;
    HBITMAP	hBitmap = (HBITMAP) 0;
    int		nWidthBytes;
    LPRGBQUAD	lpRGB;
    BOOL	rc = 0;

    APISTR((LF_APICALL,
	"CreateDIBitmap(HDC=%x,BITMAPINFOHEADER *%x, DWORD %x, void *%x, BITMAPINFO *%x,UINT=%x\n",
		hDC, lpbmih, dwInit, initbits, lpbmi, colors));

    if (dwInit != CBM_INIT) {
	if (lpbmih->biPlanes == 1 &&
		(lpbmih->biBitCount == 1 ||
	 	lpbmih->biBitCount == 4 ||
	 	lpbmih->biBitCount == 8 ||
	 	lpbmih->biBitCount == 24)) {
	    hBitmap = CreateCompatibleBitmap(hDC,lpbmih->biWidth,lpbmih->biHeight);
    	    APISTR((LF_APIRET,"CreateDIBitmap: returns HBITMAP %x\n",hBitmap));
	    return hBitmap; 
	} else {
    	    APISTR((LF_APIRET,"CreateDIBitmap: returns HBITMAP %x\n",hBitmap));
	    return hBitmap;
	}
    }

    /* DIBs are always 32-bit aligned */
    if (!(nWidthBytes = CalcByteWidth(lpbmih->biWidth,
		lpbmih->biBitCount,
		32))) {
    	APISTR((LF_APIRET,"CreateDIBitmap: returns HBITMAP %x\n",hBitmap));
	return hBitmap;
    }

    if(!(lpbmih->biSize == 40 || lpbmih->biSize == 12)) {
    	APISTR((LF_APIFAIL,"CreateDIBitmap: returns HBITMAP %x\n",hBitmap));
	return (HBITMAP)hBitmap;
    }

    if (!(lpimage = (LPIMAGEINFO) CREATEHBITMAP(hBitmap))) {
    	APISTR((LF_APIFAIL,"CreateDIBitmap: returns HBITMAP %x\n",hBitmap));
	return hBitmap;
    }

    lpimage->ImageDepth = (lpbmih->biBitCount == 1) ? 1 :
			GetDeviceCaps(hDC,BITSPIXEL);
    lpimage->ImagePlanes = (lpbmih->biBitCount == 1) ? 1 :
			GetDeviceCaps(hDC,PLANES);
    lpimage->rcsinfo = NULL;
    lpimage->lpdsBmi = NULL;

    if (colors == DIB_PAL_COLORS) 
	lpRGB = TWIN_MapPaletteColors(hDC,(BITMAPINFO *)lpbmi);
    else
	lpRGB = (LPRGBQUAD) &lpbmi->bmiColors[0];

    rc = CreateDIBImage(lpimage,(BITMAPINFOHEADER *)lpbmih,lpRGB,(LPVOID)initbits);

    if (colors == DIB_PAL_COLORS)
	WinFree((LPSTR)lpRGB);

    if (rc) {
        APISTR((LF_APIRET,"CreateDIBitmap: returns HBITMAP %x\n",hBitmap));
	return hBitmap;
    }

    FREEGDI(hBitmap);

    APISTR((LF_APIFAIL,"CreateDIBitmap: returns HBITMAP %x\n",0));
    return (HBITMAP)0;
}

HBITMAP WINAPI
CreateBitmapIndirect(LPBITMAP lpbm)
{
    LPIMAGEINFO     lpimage;
    HBITMAP	hBitmap;

    APISTR((LF_APICALL,"CreateBitmapIndirect(LPBITMAP=%x)\n",lpbm));

    if (lpbm->bmWidth < 0 || lpbm->bmHeight < 0) {
        APISTR((LF_APIFAIL,"CreateBitmapIndirect: returns HBITMAP %x\n",0));
	return (HBITMAP)0;
    }

    if (!(lpimage = (LPIMAGEINFO) CREATEHBITMAP(hBitmap))) {
        APISTR((LF_APIFAIL,"CreateBitmapIndirect: returns HBITMAP %x\n",0));
	return (HBITMAP)0;
    }
    lpimage->ImageWidth     = lpbm->bmWidth;
    lpimage->ImageHeight    = lpbm->bmHeight;
    lpimage->rcsinfo	    = 0;
    lpimage->lpdsBmi	    = NULL;

    if (!CreateDDBImage(lpimage,lpbm->bmPlanes,lpbm->bmBitsPixel,
				(const LPBYTE)lpbm->bmBits)) {
	FREEGDI(hBitmap);
        APISTR((LF_APIFAIL,"CreateBitmapIndirect: returns HBITMAP %x\n",0));
	return (HBITMAP)0;
    }

    APISTR((LF_APIRET,"CreateBitmapIndirect: returns HBITMAP %x\n",hBitmap));
    return (HBITMAP)hBitmap;
}

HBITMAP WINAPI
CreateBitmap(int nWidth, int nHeight, UINT cPlanes, UINT cBitsPerPel,
	const void *lpvBits)
{
    BITMAP    bm;
    HBITMAP   hBitmap = 0;
 
    APISTR((LF_APICALL,"CreateBitmap(int=%d,int=%d,UINT=%x,UINT=%x,void *%x)\n",
	nWidth,nHeight,cPlanes,cBitsPerPel,lpvBits));

    bm.bmType     = 0;
    bm.bmWidth    = nWidth;
    bm.bmHeight   = nHeight;
    bm.bmPlanes   = (BYTE) cPlanes;
    bm.bmBitsPixel = (BYTE) cBitsPerPel;

    /* we have to set a bmWidthBytes field based on cBitsPerPel */
    if (!(bm.bmWidthBytes = CalcByteWidth(nWidth,cBitsPerPel,16))) {
        APISTR((LF_APIRET,"CreateBitmapIndirect: returns HBITMAP %x\n",hBitmap));
	return hBitmap;
    }

    bm.bmBits = (LPVOID) lpvBits;
    
    hBitmap =  CreateBitmapIndirect(&bm);

    APISTR((LF_APIRET,"CreateBitmap: returns HBITMAP %x\n",hBitmap));
    return hBitmap;
}

HBITMAP WINAPI 
CreateCompatibleBitmap(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    int nPlanes, nBitsPixel;
    LPIMAGEINFO ip;
    HBITMAP hBitmap;

    APISTR((LF_APICALL,"CreateCompatibleBitmap(HDC=%x,int=%x,int=%x)\n",hDC,x,y));
    ASSERT_HDC(hDC32,hDC,0);

    if ((hDC32->dwDCXFlags & DCX_COMPATIBLE_DC) &&
		(ip = GETBITMAPINFO(hDC32->hBitmap))) {
	nPlanes = ip->ImagePlanes;
	nBitsPixel = ip->ImageDepth;
	RELEASEBITMAPINFO(ip);
    }
    else {
	nPlanes = GetDeviceCaps(hDC,PLANES);
	nBitsPixel = GetDeviceCaps(hDC,BITSPIXEL);
    }

    hBitmap = CreateBitmap(x,y,nPlanes,nBitsPixel,NULL);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"CreateCompatibleBitmap: returns HBITMAP %x\n",hBitmap));
    return hBitmap;
}

HBITMAP WINAPI 
CreateDiscardableBitmap(HDC hDC, int x, int y)
{
    HBITMAP hBitmap;

    APISTR((LF_APICALL,"CreateDiscardableBitmap(HDC=%x,int=%x,int=%x)\n",hDC,x,y));
    hBitmap = CreateCompatibleBitmap(hDC,x,y);
    APISTR((LF_APIRET,"CreateDiscardableBitmap: returns HBITMAP %x\n",hBitmap));
    return hBitmap;
}

LONG
TWIN_SetImageBits(HBITMAP hBitmap, DWORD cBits,
		const void *lpvBits, int nAlignment)
{
    LPIMAGEINFO lpimage;
    LPBYTE	lpImageData;
    DWORD	dwDataSize;

    if (!lpvBits || !(lpimage = GETBITMAPINFO(hBitmap)))
	return 0;

    if (lpimage->lpDrvData == 0) {
	return 0;
    }

    dwDataSize = lpimage->WidthBytes * lpimage->ImageHeight;
    dwDataSize = min(dwDataSize,cBits);

    lpImageData = (LPBYTE)DRVCALL_IMAGES(PIH_GETIMAGEDATA,0L,0L,
		(LPVOID)lpimage->lpDrvData);
    if (lpImageData == 0) {
	return 0;
    }

    if (lpimage->ImageDepth != 1) {
	/* this is a color DDB, copy it without any changes */
	memcpy((LPSTR)lpImageData,lpvBits,dwDataSize);
    }
    else {
	/* mono DDB has to be converted to mono image */

	ConvertMonoDDBToImage(lpImageData, (LPBYTE)lpvBits,
		lpimage->WidthBytes,
		CalcByteWidth(lpimage->ImageWidth,1,nAlignment),
		lpimage->ImageHeight);
    }

    DRVCALL_IMAGES(PIH_UPDATEPIXMAP,0L,0L,(LPVOID)lpimage->lpDrvData);

    return dwDataSize;
}

LONG WINAPI
SetBitmapBits(HBITMAP hBitmap,DWORD cBits,const void FAR *lpvBits)
{
    LONG   lret;
    APISTR((LF_APICALL,"SetBitmapBits(HBITMAP=%x,DWORD=%x,void *%x)\n",
	hBitmap,cBits,lpvBits));
    lret = TWIN_SetImageBits(hBitmap, cBits, lpvBits, 16);
    APISTR((LF_APIRET,"SetBitmapBits: returns LONG %x\n",lret));
    return lret;
}

LONG WINAPI 
GetBitmapBits(HBITMAP hBitmap, LONG cSize, void * lpvBits)
{
    LONG lret;
    APISTR((LF_APICALL,"GetBitmapBits(HBITMAP=%x,DWORD=%x,void *%x)\n",
	hBitmap,cSize,lpvBits));
    lret =  TWIN_GetImageBits(hBitmap,cSize,lpvBits,16);
    APISTR((LF_APIRET,"GetBitmapBits: returns LONG %x\n",lret));
    return lret;
}

LONG
TWIN_GetImageBits(HBITMAP hBitmap, LONG cSize,
		void FAR* lpvBits, int nAlignment)
{
    LPIMAGEINFO     lpimage;
    DWORD	dwDataSize;
    int		x,y;
    LPBYTE	lpSrc,lpDest,lpTemp;
    LPSTR	lpImageData;
    int		nDestWidth;
    BOOL	fReverse;
    
    if (nAlignment != 16 && nAlignment != 32)
	return 0;

    if (!(lpimage = GETBITMAPINFO(hBitmap)))
	return 0;

    lpImageData = (LPSTR)DRVCALL_IMAGES(PIH_GETIMAGEDATA,TRUE,0L,
		(LPVOID)lpimage->lpDrvData);

    if (lpImageData == NULL)
	return FALSE;

    fReverse = (DRVCALL_IMAGES(PIH_GETDEVCAPS,DRV_BITORDER,0L,NULL) ==
			LSBFIRST)?TRUE:FALSE;

    nDestWidth = CalcByteWidth(lpimage->ImageWidth,lpimage->ImageDepth,
			nAlignment);
    dwDataSize = lpimage->WidthBytes * lpimage->ImageHeight;
    dwDataSize = min(dwDataSize,cSize);

    /* Line padding for mono DDB is 16; if we've got an image padded to 32, */
    /* we have to perform a per-line conversion; for color DDB, we take	    */
    /* whatever data we've got, and the app is not supposed to know about   */
    /* its format.							    */

    if (lpimage->ImageDepth == 1) {
	lpSrc = (LPBYTE)lpImageData;
	lpDest = (LPBYTE)lpvBits;
	for (y = 0; y < lpimage->ImageHeight; y++) {
	    lpTemp = lpSrc;
	    for (x=0; x<nDestWidth; x++)
		*lpDest++ = (fReverse)? bit_reverse[*lpTemp++]: *lpTemp++;
	    lpSrc += lpimage->WidthBytes;
	}
    }
    else 
	memcpy((LPSTR)lpvBits,lpImageData,dwDataSize);

    return dwDataSize;
}

HICON   WINAPI 
CreateIcon(HINSTANCE hinst, int cx, int cy, BYTE bplanes, BYTE bitspixel,
		const void FAR* lpandbits, const void FAR*lpxorbits)
{
    HANDLE		hIcon;
    LPTWIN_ICONINFO		lpIconInfo;

    APISTR((LF_APICALL,"CreateIcon(HINSTANCE=%x,int=%d,int=%d,BYTE=%x,BYTE=%x,void *%x,void *%x)\n",
			hinst,cx,cy,bplanes,bitspixel,lpandbits,lpxorbits));

    if ((lpIconInfo = CREATEHICON(hIcon)) == 0) {
        APISTR((LF_APIFAIL,"CreateIcon: returns HICON %x\n",0));
	return (HICON)0;
    }

    if (!(lpIconInfo->hXORImage = CreateBitmap(cx,cy,bplanes,bitspixel,
			lpxorbits))) {
	FREEGDI(hIcon);
        APISTR((LF_APIFAIL,"CreateIcon: returns HICON %x\n",0));
	return (HICON)0;
    }

    if (!(lpIconInfo->hANDMask = CreateBitmap(cx,cy,1,1,lpandbits))) {
	DeleteObject(lpIconInfo->hXORImage);
	FREEGDI(hIcon);
        APISTR((LF_APIFAIL,"CreateIcon: returns HICON %x\n",0));
	return (HICON)0;
    }

    APISTR((LF_APIRET,"CreateIcon: returns HICON %x\n",hIcon));
    return hIcon;
}

static HBITMAP copy_bitmap(HBITMAP hBitmapOld)
{
  BITMAP bitmap;
  HBITMAP hBitmapNew;
  HDC hdcOld, hdcNew;

  if (!hBitmapOld)
    return 0;

  GetObject(hBitmapOld, sizeof(bitmap), &bitmap);
  hBitmapNew = CreateBitmap(bitmap.bmWidth, bitmap.bmHeight, bitmap.bmPlanes, bitmap.bmBitsPixel, NULL);
  hdcOld = CreateCompatibleDC(0);
  hdcNew = CreateCompatibleDC(0);
  SelectObject(hdcOld, hBitmapOld);
  SelectObject(hdcNew, hBitmapNew);
  BitBlt(hdcNew, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcOld, 0, 0, SRCCOPY);
  DeleteDC(hdcNew);
  DeleteDC(hdcOld);
  return hBitmapNew;
}

HICON WINAPI
#ifdef TWIN32
CopyIcon(HICON hIcon)
#else
CopyIcon(HINSTANCE hInstance, HICON hIcon)
#endif
{
  	HICON hIconNew;
  	LPTWIN_ICONINFO lptiiOld, lptiiNew;

#ifdef TWIN32
    	APISTR((LF_APICALL,"CopyIcon(HICON=%x)\n",hIcon));
#else
    	APISTR((LF_APICALL,"CopyIcon(HINSTANCE=%x,HICON=%x)\n",hInstance,hIcon));
#endif
  	if (!(lptiiOld = GETICONINFO(hIcon))) {
    		APISTR((LF_APIFAIL,"CopyIcon: returns HBITMAP %x\n",0));
    		return 0;
	}

  	if (!(lptiiNew = CREATEHICON(hIconNew))) {
    		APISTR((LF_APIFAIL,"CopyIcon: returns HBITMAP %x\n",0));
    		return 0;
	}

  	lptiiNew->hXORImage = copy_bitmap(lptiiOld->hXORImage);
  	lptiiNew->hANDMask = copy_bitmap(lptiiOld->hANDMask);
  	lptiiNew->hMonoBmp = copy_bitmap(lptiiOld->hMonoBmp);

    	APISTR((LF_APIRET,"CopyIcon: returns HICON %x\n",hIconNew));
  	return hIconNew;
}

HICON CreateIconIndirect(PICONINFO pIconInfo)
{
  HICON hIcon;
  LPTWIN_ICONINFO lptii;

  if (!(lptii = CREATEHICON(hIcon)))
    return 0;

  lptii->hXORImage = copy_bitmap(pIconInfo->hbmColor);
  lptii->hANDMask = copy_bitmap(pIconInfo->hbmMask);

  return hIcon;
}

BOOL GetIconInfo(HICON hIcon, PICONINFO pIconInfo)
{
  LPTWIN_ICONINFO lptii;

  if (!(lptii = GETICONINFO(hIcon)))
    return FALSE;

  pIconInfo->fIcon = TRUE;
  pIconInfo->hbmMask = lptii->hANDMask;
  pIconInfo->hbmColor = lptii->hXORImage;

  return TRUE;
}

HICON
LoadIconResource(LPNAMEINFO rcsinfo)
{
    HICON	     hIcon;
    LPICONIMAGE	     lpIconImage;
    LPTWIN_ICONINFO       lpIconInfo;
    BITMAPINFOHEADER bmih;
    BITMAPIMAGE	     bimage;
    RGBQUAD	     rgb[2];
    LPIMAGEINFO	     lpImage;
    int 	     nIconWidth,nIconHeight;

    if ((lpIconImage = (LPICONIMAGE)rcsinfo->rcsdata) == 0)
	return (HICON)0;

    if ((lpIconInfo = CREATEHICON(hIcon)) == 0)
	return (HICON)0;

    /* Create XOR image color bitmap	*/

    bmih = *(lpIconImage->icon_bmp_hdr);
    bmih.biHeight /= 2;
    bimage.bmp_hdr = &bmih;
    bimage.rgb_quad = lpIconImage->icon_rgb_quad;
    bimage.bitmap_bits = lpIconImage->icon_xor_mask;

    if (!(lpIconInfo->hXORImage = CreateDIBitmapEx(&bimage,0))) {
	FREEGDI(hIcon);
	return (HICON)0;
    }

    /* Now I create a monochrome XOR bitmap with colors mapped to */
    /* black-and-white a la GetNearestColor; it will be used as a */
    /* WM icon for top-level windows and as a cursor for dragging */
    /* iconized child windows --- LK				  */

    lpImage = (LPIMAGEINFO)CREATEHBITMAP(lpIconInfo->hMonoBmp);
    lpImage->ImageDepth = 1;
    lpImage->ImagePlanes = 1;
    lpImage->rcsinfo = 0;
    lpImage->lpdsBmi = NULL;
    if (!CreateDIBImage(lpImage,&bmih,lpIconImage->icon_rgb_quad,
			lpIconImage->icon_xor_mask)) {
	DeleteObject(lpIconInfo->hMonoBmp);
	lpIconInfo->hMonoBmp = 0;
    }

    /* Create AND mask bitmap	*/

    bmih.biBitCount = bmih.biPlanes = 1;
    rgb[0].rgbBlue = rgb[0].rgbGreen = rgb[0].rgbRed = 0;
    rgb[1].rgbBlue = rgb[1].rgbGreen = rgb[1].rgbRed = 255;
    bimage.rgb_quad = &rgb[0];
    bimage.bitmap_bits = lpIconImage->icon_and_mask;
    if (!(lpIconInfo->hANDMask = CreateDIBitmapEx(&bimage,0))) {
	DeleteObject(lpIconInfo->hXORImage);
	FREEGDI(hIcon);
	return (HICON)0;
    }

    /* Now figure out if the size matches system metrics; if not -- */
    /* stretch or shrink it to fit				    */
    nIconWidth = GetSystemMetrics(SM_CXICON);
    nIconHeight = GetSystemMetrics(SM_CYICON);
    if (bmih.biWidth != nIconWidth || bmih.biHeight != nIconHeight) {
	HDC hDC,hCompatDC1,hCompatDC2;
	HBITMAP hCompatBmp,hOldBmp1,hOldBmp2;

	hDC = GetDC(0);
	hCompatDC1 = CreateCompatibleDC(hDC);
	hCompatDC2 = CreateCompatibleDC(hDC);

	hCompatBmp = CreateCompatibleBitmap(hDC,nIconWidth,nIconHeight);
	hOldBmp1 = SelectObject(hCompatDC1,lpIconInfo->hXORImage);
	hOldBmp2 = SelectObject(hCompatDC2,hCompatBmp);
	StretchBlt(hCompatDC2,0,0,nIconWidth,nIconHeight,
		   hCompatDC1,0,0,bmih.biWidth,bmih.biHeight,
		   SRCCOPY);
	SelectObject(hCompatDC1,hOldBmp1);
	DeleteObject(lpIconInfo->hXORImage);
	lpIconInfo->hXORImage = hCompatBmp;

	hCompatBmp = CreateCompatibleBitmap(hCompatDC1,nIconWidth,nIconHeight);
	SelectObject(hCompatDC1,lpIconInfo->hANDMask);
	SelectObject(hCompatDC2,hCompatBmp);
	StretchBlt(hCompatDC2,0,0,nIconWidth,nIconHeight,
		   hCompatDC1,0,0,bmih.biWidth,bmih.biHeight,
		   SRCCOPY);
	SelectObject(hCompatDC1,hOldBmp1);
	DeleteObject(lpIconInfo->hANDMask);
	lpIconInfo->hANDMask = hCompatBmp;

	hCompatBmp = CreateCompatibleBitmap(hCompatDC1,nIconWidth,nIconHeight);
	SelectObject(hCompatDC1,lpIconInfo->hMonoBmp);
	SelectObject(hCompatDC2,hCompatBmp);
	StretchBlt(hCompatDC2,0,0,nIconWidth,nIconHeight,
		   hCompatDC1,0,0,bmih.biWidth,bmih.biHeight,
		   SRCCOPY);
	SelectObject(hCompatDC1,hOldBmp1);
	DeleteObject(lpIconInfo->hMonoBmp);
	lpIconInfo->hMonoBmp = hCompatBmp;

	SelectObject(hCompatDC2,hOldBmp2);
	DeleteDC(hCompatDC1);
	DeleteDC(hCompatDC2);
	ReleaseDC(0,hDC);
    }

    lpIconInfo->rcsinfo = rcsinfo;
    return hIcon;
}

BOOL    WINAPI 
DestroyIcon(HICON hIcon)
{
    LPTWIN_ICONINFO	lpIconInfo;

    APISTR((LF_APICALL,"DestroyIcon(HICON=%x)\n",hIcon));
    if (!(lpIconInfo = GETICONINFO(hIcon))) {
    	APISTR((LF_APIFAIL,"DestroyIcon: returns BOOL %x\n",FALSE));
	return FALSE;
    }
    DeleteObject(lpIconInfo->hXORImage);
    DeleteObject(lpIconInfo->hANDMask);

    if (lpIconInfo->rcsinfo)
	lpIconInfo->rcsinfo->hObject = 0;

    FREEGDI(hIcon);

    APISTR((LF_APIRET,"DestroyIcon: returns BOOL %x\n",TRUE));
    return TRUE;
}

HCURSOR WINAPI
CreateCursor(HINSTANCE hInst, int xhot, int yhot, int width, int height,
		const void FAR* lpANDbits, const void FAR* lpXORbits)
{
    HCURSOR hCursor;
    LPCURSORINFO lpCursorInfo;
    HBITMAP hXORBmp,hANDBmp;
    LPIMAGEINFO lpANDImage,lpXORImage;
    CREATECURSOR cc;

    APISTR((LF_APICALL,"CreateCursor(HINSTANCE=%x,int=%d,int=%d,int=%d,int=%d,void *%x,void *%x)\n",
	hInst,xhot,yhot,width,height,lpANDbits,lpXORbits));

    if ((lpCursorInfo = CREATEHCURSOR(hCursor)) == 0) {
    	APISTR((LF_APIFAIL,"CreateCursor: returns HCURSOR %x\n",0));
	return (HCURSOR)0;
    }

    if (!(hXORBmp = CreateBitmap(width,height,1,1,lpXORbits))) {
	FREEGDI(hCursor);
    	APISTR((LF_APIFAIL,"CreateCursor: returns HCURSOR %x\n",0));
	return (HCURSOR)0;
    }

    if (!(hANDBmp = CreateBitmap(width,height,1,1,lpANDbits))) {
	DeleteObject(hXORBmp);
	FREEGDI(hCursor);
    	APISTR((LF_APIFAIL,"CreateCursor: returns HCURSOR %x\n",0));
	return (HCURSOR)0;
    }
    if (!(lpXORImage = GETBITMAPINFO(hXORBmp)) ||
			!(lpANDImage = GETBITMAPINFO(hANDBmp))) {
	DeleteObject(hXORBmp);
	DeleteObject(hANDBmp);
	FREEGDI(hCursor);
    	APISTR((LF_APIFAIL,"CreateCursor: returns HCURSOR %x\n",0));
	return (HCURSOR)0;
    }
    cc.lpXORData = lpXORImage->lpDrvData;
    cc.lpANDData = lpANDImage->lpDrvData;
    cc.ptHot.x = xhot;
    cc.ptHot.y = yhot;
    cc.crFore = RGB(0,  0,  0);
    cc.crBack = RGB(255,255,255);

    lpCursorInfo->lpCurData = (LPVOID)DRVCALL_IMAGES(PIH_CREATECURSOR,0L,0L,
		(LPVOID)&cc);
    if (lpCursorInfo->lpCurData == NULL) {
	FREEGDI(hCursor);
	hCursor = 0;
    }
    DeleteObject(hXORBmp);
    DeleteObject(hANDBmp);

    APISTR((LF_APIRET,"CreateCursor: returns HCURSOR %x\n",hCursor));
    return hCursor;
}

HCURSOR
LoadCursorResource(LPNAMEINFO rcsinfo)
{
    HCURSOR hCursor;
    CURSORIMAGE *lpCurImage;
    LPCURSORINFO lpCursorInfo;
    BITMAPINFOHEADER bmih;
    HBITMAP hANDBmp, hXORBmp;
    LPIMAGEINFO lpXORImage,lpANDImage;
    BITMAPIMAGE bimage;
    RGBQUAD	rgb[2];
    CREATECURSOR cc;

    if (!(lpCursorInfo = CREATEHCURSOR(hCursor))) {
	return (HCURSOR)0;
    }
    if (!(lpCurImage = (CURSORIMAGE *)rcsinfo->rcsdata)) {
	return (HCURSOR)0;
    }

    bmih = *(lpCurImage->bmp_hdr);
    bmih.biHeight /= 2;
    bimage.bmp_hdr = &bmih;
    bimage.rgb_quad = lpCurImage->rgb_quad;
    bimage.bitmap_bits = lpCurImage->xor_mask;

    hXORBmp = CreateDIBitmapEx(&bimage,0);
    if (!(lpXORImage = GETBITMAPINFO(hXORBmp))) {
	FREEGDI(hCursor);
	return (HCURSOR)0;
    }

    bmih.biBitCount = bmih.biPlanes = 1;
    rgb[0].rgbBlue = rgb[0].rgbGreen = rgb[0].rgbRed = 0;
    rgb[1].rgbBlue = rgb[1].rgbGreen = rgb[1].rgbRed = 255;
    bimage.rgb_quad = &rgb[0];
    bimage.bitmap_bits = lpCurImage->and_mask;

    hANDBmp = CreateDIBitmapEx(&bimage,0);
    if (!(lpANDImage = GETBITMAPINFO(hANDBmp))) {
	DeleteObject(hXORBmp);
        FREEGDI(hCursor);
        return (HCURSOR)0;
    }

    cc.lpXORData = lpXORImage->lpDrvData;
    cc.lpANDData = lpANDImage->lpDrvData;
    cc.ptHot.x = lpCurImage->wHotSpotX;
    cc.ptHot.y = lpCurImage->wHotSpotY;
    cc.crFore = RGB(lpCurImage->rgb_quad[0].rgbRed,
		     lpCurImage->rgb_quad[0].rgbGreen, 
		     lpCurImage->rgb_quad[0].rgbBlue);
    cc.crBack = RGB(lpCurImage->rgb_quad[1].rgbRed,
		     lpCurImage->rgb_quad[1].rgbGreen, 
		     lpCurImage->rgb_quad[1].rgbBlue);
    lpCursorInfo->lpCurData = (LPVOID)DRVCALL_IMAGES(PIH_CREATECURSOR,0L,0L,
		(LPVOID)&cc);

    if (lpCursorInfo->lpCurData == NULL) {
	FREEGDI(hCursor);
	hCursor = 0;
    }

    lpCursorInfo->rcsinfo = rcsinfo;

    DeleteObject(hANDBmp);
    DeleteObject(hXORBmp);

    return hCursor;
}  

BOOL WINAPI
DestroyCursor(HCURSOR hCursor)
{
    LPCURSORINFO lpCurInfo;

    APISTR((LF_APICALL,"DestroyCursor(HCURSOR=%x)\n",hCursor));

    if (!(lpCurInfo = GETCURSORINFO(hCursor))) {
    	APISTR((LF_APIRET,"DestroyCursor: returns BOOL %x\n",0));
	return FALSE;
    }

    DRVCALL_IMAGES(PIH_DESTROYCURSOR,0L,0L,lpCurInfo->lpCurData);

    if (lpCurInfo->rcsinfo)
	lpCurInfo->rcsinfo->hObject = 0;

    FREEGDI(hCursor);

    APISTR((LF_APIRET,"DestroyCursor: returns BOOL %x\n",TRUE));
    return TRUE;
}

HCURSOR
GetDragIconCursor(HWND hWnd)
{
    HICON hIcon;
    HCURSOR hCursor;
    LPCURSORINFO lpCursorInfo;
    HGDIOBJ hRetcode;
    LPTWIN_ICONINFO lpIconInfo;
    int nWidth,nHeight;
    CREATECURSOR cc;
    LPIMAGEINFO lpXORImage,lpANDImage;

    if (!(hIcon = GetClassIcon(hWnd)) ||
			!(lpIconInfo = GETICONINFO(hIcon))) {
	hRetcode = (HGDIOBJ)SendMessage(hWnd,WM_QUERYDRAGICON,0,0L);
	if (!hRetcode)
	    return LoadCursor(0,"DRAGCURSOR");

	if (GETCURSORINFO(hRetcode)) {
	    LOCKGDI(hRetcode);
	    return (HCURSOR)hRetcode;
	}
	else if (!(lpIconInfo = GETICONINFO(hRetcode)))
	    return (HCURSOR)0;
    }
    lpXORImage = GETBITMAPINFO(lpIconInfo->hMonoBmp);
    lpANDImage = GETBITMAPINFO(lpIconInfo->hANDMask);

    if (!lpXORImage || !lpANDImage)
	return (HCURSOR)0;

    nWidth = GetSystemMetrics(SM_CXCURSOR);
    nHeight = GetSystemMetrics(SM_CYCURSOR);

    lpCursorInfo = CREATEHCURSOR(hCursor);
    lpCursorInfo->rcsinfo = 0;

    cc.lpXORData = lpXORImage->lpDrvData;
    cc.lpANDData = lpANDImage->lpDrvData;
    cc.ptHot.x = nWidth/2;
    cc.ptHot.y = nHeight/2;
    cc.crFore = RGB(0,0,0);
    cc.crBack = RGB(255,255,255);
    lpCursorInfo->lpCurData = (LPVOID)DRVCALL_IMAGES(PIH_CREATECURSOR,0L,0L,
		(LPVOID)&cc);

    if (lpCursorInfo->lpCurData == 0) {
	FREEGDI(hCursor);
	return (HCURSOR)0;
    }
    else 
	return hCursor;
}

/* (WIN32) GDI Bitmap ****************************************************** */

static UINT NumDIBColors(CONST BITMAPINFO *pBmi)
{
	UINT uNumDIBColors;

	if (!pBmi)
		return (0);

	if ((uNumDIBColors = pBmi->bmiHeader.biClrUsed) == 0)
	{
		switch (pBmi->bmiHeader.biCompression)
		{
		case BI_RLE4:
			uNumDIBColors = 16;
			break;
		case BI_RLE8:
			uNumDIBColors = 256;
			break;
		default:
			uNumDIBColors = 0;
			break;
		}
	}

	return (uNumDIBColors);

}

static RGBQUAD *DIBColors(CONST BITMAPINFO *pBmi)
{
	return (RGBQUAD *)((LPSTR)pBmi + (WORD)(pBmi->bmiHeader.biSize));
}

#define WIDTHBYTES(bits)	((((bits) + 31) / 32) * 4)

static DWORD CalcDIBBitsLineBufferSize(CONST BITMAPINFO *pBmi)
{
	LONG biWidth = (pBmi->bmiHeader.biWidth > 0)
		? +pBmi->bmiHeader.biWidth
		: -pBmi->bmiHeader.biWidth;
	WORD biPlanes = pBmi->bmiHeader.biPlanes;
	WORD biBitCount = pBmi->bmiHeader.biBitCount;

	return (WIDTHBYTES(biWidth * biPlanes * biBitCount));
}

static DWORD CalcDIBBitsBufferSize(CONST BITMAPINFO *pBmi)
{
	LONG biHeight = (pBmi->bmiHeader.biHeight > 0)
		? +pBmi->bmiHeader.biHeight
		: -pBmi->bmiHeader.biHeight;

	return (biHeight * CalcDIBBitsLineBufferSize(pBmi));
}

static LPBITMAPINFO CopyDIB(CONST BITMAPINFO *pBmi)
{
	DWORD dwBmiSize;
	LPBITMAPINFO pDuplicateBmi;

	dwBmiSize = sizeof(BITMAPINFO) + NumDIBColors(pBmi) * sizeof(RGBQUAD);
	pDuplicateBmi = (LPBITMAPINFO) WinMalloc(dwBmiSize);

	if (pDuplicateBmi != NULL)
		memcpy(pDuplicateBmi, pBmi, dwBmiSize);

	return (pDuplicateBmi);
}

HBITMAP	WINAPI
CreateDIBSection(HDC hDC,
	CONST BITMAPINFO *pBmi,
	UINT uUsage,
	VOID *ppvBits,
	HANDLE hSection,
	DWORD dwOffset)
{
	HBITMAP hBitmap = 0;
	LPIMAGEINFO lpImageInfo;

	APISTR((LF_APICALL,
	       "CreateDIBSection(HDC=%x,BITMAPINFO *%p,UINT=%x,void *%x,HANDLE=%x,DWORD=%x\n",
		hDC, pBmi, uUsage, ppvBits, hSection, dwOffset));

	if (!ppvBits)
	{
		SetLastErrorEx(1, 0);
    		APISTR((LF_APIFAIL,"CreateDIBSection: returns HBITMAP %x\n",hBitmap));
		return hBitmap;
	}

	if (!pBmi)
	{
		SetLastErrorEx(1, 0);
		*((void **)ppvBits) = (void *)0;
    		APISTR((LF_APIFAIL,"CreateDIBSection: returns HBITMAP %x\n",hBitmap));
		return hBitmap;
	}

	if (hSection)
	{
#ifdef	LATER	/* implement file mapping */
		*((void **)ppvBits) = MapViewOfFile(hSection,
			FILE_MAP_ALL_ACCESS, 0UL, dwOffset,
			CalcDIBBitsBufferSize(pBmi));

		if (!*((void **)ppvBits))
		{
			SetLastErrorEx(1, 0);
			*((void **)ppvBits) = (void *)0;
    			APISTR((LF_APIFAIL,"CreateDIBSection: returns HBITMAP %x\n",hBitmap));
			return hBitmap;
		}
#endif
	}
	else
	{
		*((void **)ppvBits) = WinMalloc(CalcDIBBitsBufferSize(pBmi));
		if (!*((void **)ppvBits))
		{
			SetLastErrorEx(1, 0);
			*((void **)ppvBits) = (void *)0;
    			APISTR((LF_APIFAIL,"CreateDIBSection: returns HBITMAP %x\n",hBitmap));
			return hBitmap;
		}
	}

	if (!(hBitmap = CreateDIBitmap(hDC,
		(LPBITMAPINFOHEADER) &pBmi->bmiHeader, CBM_INIT,
		*((void **)ppvBits), (LPBITMAPINFO) pBmi, uUsage)))
	{
		SetLastErrorEx(1, 0);
		if (hSection) {
#ifdef LATER
			UnmapViewOfFile(*((void **)ppvBits));
#endif
		} else
			WinFree(*((void **)ppvBits));

		*((void **)ppvBits) = (void *)0;
    		APISTR((LF_APIFAIL,"CreateDIBSection: returns HBITMAP %x\n",hBitmap));
		return hBitmap;
	}

	if (!(lpImageInfo = GETBITMAPINFO(hBitmap)))
	{
		ERRSTR((LF_ERROR, "CreateDIBSection: (ERR) access DIB failed\n"));
		SetLastErrorEx(1, 0);
		DeleteObject(hBitmap);
		if (hSection) {
#ifdef LATER
			UnmapViewOfFile(*((void **)ppvBits));
#endif
		} else
			WinFree(*((void **)ppvBits));

		*((void **)ppvBits) = (void *)0;
    		APISTR((LF_APIFAIL,"CreateDIBSection: returns HBITMAP %x\n",hBitmap));
		return hBitmap;
	}

	lpImageInfo->ds.dsBm.bmType = 0;
	lpImageInfo->ds.dsBm.bmWidth =
		(pBmi->bmiHeader.biWidth > 0)
		? +pBmi->bmiHeader.biWidth
		: -pBmi->bmiHeader.biWidth;
	lpImageInfo->ds.dsBm.bmHeight =
		(pBmi->bmiHeader.biHeight > 0)
		? +pBmi->bmiHeader.biHeight
		: -pBmi->bmiHeader.biHeight;
	lpImageInfo->ds.dsBm.bmWidthBytes = CalcDIBBitsLineBufferSize(pBmi);
	lpImageInfo->ds.dsBm.bmPlanes = GetDeviceCaps(hDC, PLANES);
	lpImageInfo->ds.dsBm.bmBitsPixel = GetDeviceCaps(hDC, BITSPIXEL);
	lpImageInfo->ds.dsBm.bmBits = *((void **)ppvBits);
	lpImageInfo->ds.dsBmih = pBmi->bmiHeader;
	lpImageInfo->ds.dsBitfields[0] = *((DWORD *)&pBmi->bmiColors[0]);
	lpImageInfo->ds.dsBitfields[1] = *((DWORD *)&pBmi->bmiColors[1]);
	lpImageInfo->ds.dsBitfields[2] = *((DWORD *)&pBmi->bmiColors[2]);
	lpImageInfo->ds.dshSection = hSection;
	lpImageInfo->ds.dsOffset = dwOffset;
	lpImageInfo->dsUsage = uUsage;
	lpImageInfo->lpdsBmi = CopyDIB(pBmi);
	if (!lpImageInfo->lpdsBmi)
	{
		SetLastErrorEx(1, 0);
		RELEASEBITMAPINFO(lpImageInfo);
		DeleteObject(hBitmap);
		if (hSection) {
#ifdef LATER
			UnmapViewOfFile(*((void **)ppvBits));
#endif
		} else
			WinFree(*((void **)ppvBits));
		*((void **)ppvBits) = (void *)0;
    		APISTR((LF_APIFAIL,"CreateDIBSection: returns HBITMAP %x\n",hBitmap));
		return ((HBITMAP)0);
	}

	RELEASEBITMAPINFO(lpImageInfo);
    	APISTR((LF_APIRET,"CreateDIBSection: returns HBITMAP %x\n",hBitmap));
	return (hBitmap);
}

UINT	WINAPI
GetDIBColorTable(HDC hDC,
	UINT uColorIndex, UINT uNumColors,
	RGBQUAD *pColors)
{
	HDC32 hDC32;
	LPIMAGEINFO lpImageInfo;
	UINT uNumDIBColors;

	APISTR((LF_APICALL,"GetDIBColorTable(HDC=%x,UINT=%x,UINT=%x,RGBQUAD*=%p)\n",
		hDC, uColorIndex, uNumColors,pColors));

	ASSERT_HDC(hDC32, hDC, 0);

	if ((hDC32->hBitmap == (HBITMAP)NULL)
	 || ((lpImageInfo = GETBITMAPINFO(hDC32->hBitmap)) == NULL))
	{
		RELEASEDCINFO(hDC32);
		SetLastErrorEx(1, 0);
    		APISTR((LF_APIFAIL,"GetDIBColorTable: returns UINT %x\n",0));
		return (0);
	}

	if (lpImageInfo->lpdsBmi == NULL)
	{
		RELEASEBITMAPINFO(lpBitmapInfo);
		RELEASEDCINFO(hDC32);
		SetLastErrorEx(1, 0);
    		APISTR((LF_APIFAIL,"GetDIBColorTable: returns UINT %x\n",0));
		return (0);
	}

	uNumDIBColors = NumDIBColors(lpImageInfo->lpdsBmi);
	if ((uColorIndex >= uNumDIBColors)
	 || (uColorIndex + uNumColors > uNumDIBColors))
	{
		RELEASEBITMAPINFO(lpBitmapInfo);
		RELEASEDCINFO(hDC32);
		SetLastErrorEx(1, 0);
    		APISTR((LF_APIFAIL,"GetDIBColorTable: returns UINT %x\n",0));
		return (0);
	}

	uNumColors = min(uNumColors, uNumDIBColors - uColorIndex);
	if (uNumColors && pColors)
		memcpy(pColors, DIBColors(lpImageInfo->lpdsBmi) + uColorIndex,
			uNumColors * sizeof(RGBQUAD));

	RELEASEBITMAPINFO(lpImageInfo);
	RELEASEDCINFO(hDC32);

    	APISTR((LF_APIRET,"GetDIBColorTable: returns UINT %x\n",uNumColors));
	return (uNumColors);

}

UINT	WINAPI
SetDIBColorTable(HDC hDC,
	UINT uColorIndex, UINT uNumColors,
	CONST RGBQUAD *pColors)
{
	HDC32 hDC32;
	LPIMAGEINFO lpImageInfo;
	UINT uNumDIBColors;

	APISTR((LF_APICALL,"SetDIBColorTable(HDC=%x,UINT=%x,UINT=%x,RGBQUAD*=%p)\n",
		hDC, uColorIndex, uNumColors, pColors));

	ASSERT_HDC(hDC32, hDC, 0);

	if ((hDC32->hBitmap == (HBITMAP)NULL)
	 || ((lpImageInfo = GETBITMAPINFO(hDC32->hBitmap)) == NULL))
	{
		SetLastErrorEx(1, 0);
		RELEASEDCINFO(hDC32);
    		APISTR((LF_APIFAIL,"SetDIBColorTable: returns UINT %x\n",0));
		return (0);
	}

	if (lpImageInfo->lpdsBmi == NULL)
	{
		SetLastErrorEx(1, 0);
		RELEASEBITMAPINFO(lpImageInfo);
		RELEASEDCINFO(hDC32);
    		APISTR((LF_APIFAIL,"SetDIBColorTable: returns UINT %x\n",0));
		return (0);
	}

	uNumDIBColors = NumDIBColors(lpImageInfo->lpdsBmi);
	if ((uColorIndex >= uNumDIBColors)
	 || (uColorIndex + uNumColors > uNumDIBColors))
	{
		RELEASEBITMAPINFO(lpBitmapInfo);
		RELEASEDCINFO(hDC32);
		SetLastErrorEx(1, 0);
    		APISTR((LF_APIFAIL,"SetDIBColorTable: returns UINT %x\n",0));
		return (0);
	}

	uNumColors = min(uNumColors, uNumDIBColors - uColorIndex);
	if (uNumColors && pColors)
		memcpy(DIBColors(lpImageInfo->lpdsBmi) + uColorIndex, pColors,
			uNumColors * sizeof(RGBQUAD));

	RELEASEBITMAPINFO(lpImageInfo);
	RELEASEDCINFO(hDC32);

    	APISTR((LF_APIRET,"SetDIBColorTable: returns UINT %x\n",uNumColors));
	return (uNumColors);
}
