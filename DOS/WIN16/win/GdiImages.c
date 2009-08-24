/*    
	GdiImages.c	2.63
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

#include "GdiObjects.h"
#include "Kernel.h"
#include "Log.h"
#include "GdiDC.h"
#include "Driver.h"

/* This file contains internal entry points for GDI */
/* bitmap support				    */

extern DWORD InternalDitherColor(UINT,DWORD,UINT *);
extern LPRGBQUAD TWIN_MapPaletteColors(HDC,LPBITMAPINFO);

extern RGBQUAD VGAColors[];
extern BYTE bit_reverse[];

/* internal functions */
BOOL CreateDIBImage(LPIMAGEINFO,BITMAPINFOHEADER *,RGBQUAD *,LPVOID);
BOOL CreateDDBImage(LPIMAGEINFO,BYTE,BYTE,const LPBYTE);
HICON LoadIconResource(LPNAMEINFO);
void ConvertMonoDDBToImage(LPBYTE,LPBYTE,int,int,int);
BOOL ConvertDIBFormat(LPIMAGEINFO,BITMAPINFOHEADER *,LPRGBQUAD,
				LPVOID,UINT,UINT);
BOOL FetchDIBits(HDC,LPIMAGEINFO,UINT,UINT,BITMAPINFOHEADER *,
				LPRGBQUAD,LPBYTE);

HBITMAP CreateDIBitmapEx(LPBITMAPIMAGE,LPNAMEINFO);
int CalcByteWidth(int,int,int);

static LPDWORD MapColorTable(LPRGBQUAD,UINT,UINT);
static LPBYTE  DecodeRLE(LPBITMAPINFOHEADER,LPBYTE);
static BYTE GetColorIndex(BYTE,BOOL,BOOL);
static int MapColorIndex(BYTE,LPRGBQUAD,LPBYTE,int);
static void FillRGBTable(HDC,LPRGBQUAD,int);
static HBITMAP TWIN_LoadHatchBitmap(int);

#define MAPRGB(r,g,b)	(((WORD)(b)&0x0F0)<<4) | ((WORD)(g)&0x0F0) | (((WORD)(r)&0x0F0)>>4)

/* This routine is called from LoadBitmap  */

HBITMAP
CreateDIBitmapEx(LPBITMAPIMAGE ip, LPNAMEINFO  lpTemplate)
{
    LPIMAGEINFO	lpimage;
    HDC hDC;
    HBITMAP hBitmap;
    BOOL fMono = FALSE;
    COLORREF cr1,cr2;
    static int imagedepth,imageplanes;

    if (!(lpimage = (LPIMAGEINFO) CREATEHBITMAP(hBitmap))) {
	return (HBITMAP)0;
    }

/* When loading bitmaps from resources, colors are treated as follows:
   - for 16- and 256-color bitmaps, resulting DDB is 16-color;
   - for monochrome bitmaps, if color table is other than black and white,
     a 16-color DDB is created, otherwise, loaded DDB is mono */

    if (ip->bmp_hdr->biBitCount == 1) {
	cr1 = RGB(ip->rgb_quad[0].rgbRed,
		     ip->rgb_quad[0].rgbGreen,
		     ip->rgb_quad[0].rgbBlue);
	cr2 = RGB(ip->rgb_quad[1].rgbRed,
		     ip->rgb_quad[1].rgbGreen,
		     ip->rgb_quad[1].rgbBlue);
	if ((cr1 == 0 || cr1 == 0xffffff) && (cr2 == 0 || cr2 == 0xffffff))
	    fMono = TRUE;
    }
    if (fMono) {
	lpimage->ImageDepth = 1;
	lpimage->ImagePlanes = 1;
    }
    else {
	/* we only need to retrieve default depth/planes once */
	if(imagedepth == 0) {
		hDC = CreateCompatibleDC(0);
		imagedepth = GetDeviceCaps(hDC,BITSPIXEL);
		imageplanes = GetDeviceCaps(hDC,PLANES);
		DeleteDC(hDC);
	}
	lpimage->ImageDepth = imagedepth;
	lpimage->ImagePlanes = imageplanes;
    }
    lpimage->rcsinfo = lpTemplate;
    lpimage->lpdsBmi = NULL;		/* (WIN32) DIBSECTION */
    if (!CreateDIBImage(lpimage,ip->bmp_hdr,ip->rgb_quad,ip->bitmap_bits)) {
	DeleteObject(hBitmap);
	hBitmap = (HBITMAP)0;
    }
    RELEASEBITMAPINFO(lpimage);
    return hBitmap;
}

int
CalcByteWidth(int nWidth, int nBitsPixel, int nAlignment)
{
    int nResult = 0;

    if (nBitsPixel == 1) { /* special case mono, can be aligned 16/32 */
	if (nAlignment == 16)
	    nResult = ((nWidth + 15)/16) << 1;
	else if (nAlignment == 32)
	    nResult = ((nWidth + 31)/32) << 2;
    } else { /* alignment is always 32 here */

#ifdef LATER
	for 4 bit servers, the second line needs to be used, but for
	256 color servers, the first line needs to be used...
	This probably needs an additional parameter... ljw
#endif
    if (nBitsPixel == 4)
	nResult = ((nWidth + 7)/8) << 2;
/*	nResult = ((nWidth + 3)/4) << 2;	    */

    else if (nBitsPixel == 8)
	nResult = ((nWidth + 3)/4) << 2;

    else if (nBitsPixel == 15 || nBitsPixel == 16)
	nResult = ((nWidth + 1) & ~1) * 2;

    else if (nBitsPixel == 24)
	nResult = ((nWidth * 3) + 3) & ~3;

    else if (nBitsPixel == 32)
        nResult = nWidth * 4;
    }
    return nResult;
}

/*	CreateDIBImage function takes LPIMAGEINFO structure with
	ImageDepth and ImagePlanes fields already set
	(those can differ from the ones specified in BITMAPINFOHEADER) */

BOOL
CreateDIBImage(LPIMAGEINFO lpimage, BITMAPINFOHEADER *lpbmih,
	  RGBQUAD *lpRGB, LPVOID lpInitBits)
{
    int nDestWidth;
    DWORD dwImageSize;
    LPBYTE lpDest;
    CREATEIMAGEDATA cid;

    /* This function handles two cases:
	1. Color DIB: gets converted into color image;
	2. Mono DIB: gets converted into mono image; fMono flag
	   in lpImage structure is set.  RGB table gets mapped to black
	   and white a la GetNearestColor(), and bits in the image are
	   set depending on the resulting table.  */

    lpimage->ImageWidth = lpbmih->biWidth;
    lpimage->ImageHeight = lpbmih->biHeight;
    if ((lpbmih->biBitCount == 1) && (lpimage->ImageDepth == 1))  {
					/* MONO case */
	lpimage->fMono = TRUE;
	lpimage->BitsPixel = 1;
    }
    else  {
	lpimage->fMono = FALSE;
	lpimage->BitsPixel = (lpimage->ImageDepth == 1) ? 1 :
		DRVCALL_IMAGES(PIH_GETDEVCAPS,DRV_QUERYBITSPIXEL,0,0);
    }

    nDestWidth = CalcByteWidth(lpimage->ImageWidth,lpimage->BitsPixel,32);
							/* 32-bit aligned */
    dwImageSize = nDestWidth * lpimage->ImageHeight;
    lpDest = (LPBYTE)WinMalloc(dwImageSize);

    if (!lpDest)
        return FALSE;

    lpimage->WidthBytes = nDestWidth;
    lpimage->LinePad = 32;
    lpimage->lpdsBmi = NULL;		/* (WIN32) DIBSECTION */

    cid.nWidth = lpimage->ImageWidth;
    cid.nHeight = lpimage->ImageHeight;
    cid.nBitsPixel = lpimage->ImageDepth;
    cid.lpData = (LPSTR)lpDest;
    cid.nWidthBytes = lpimage->WidthBytes;
    cid.nLinePad = lpimage->LinePad;
    if (!(lpimage->lpDrvData = (LPVOID)DRVCALL_IMAGES(PIH_CREATEIMAGE,
		lpimage->fMono,0L,&cid)))
	return FALSE;

    if (!ConvertDIBFormat(lpimage,lpbmih,lpRGB,lpInitBits,0,
				lpimage->ImageHeight)) {
	DRVCALL_IMAGES(PIH_DESTROYIMAGE,0L,0L,lpimage->lpDrvData);
	return FALSE;
    }
    return TRUE;
}

BOOL
CreateDDBImage(LPIMAGEINFO lpimage, BYTE bPlanes, BYTE bBitsPixel,
			const LPBYTE lpBits)
{
    LPSTR lpData;
    HDC hDC;
    CREATEIMAGEDATA cid;
    int nLength;

    if ((bBitsPixel == 1) && (bPlanes == 1)) {	/* MONO */
	lpimage->fMono = TRUE;
#if 0
	lpimage->LinePad    = 32;
#else
	lpimage->LinePad    = 16;
#endif
	lpimage->ImagePlanes = 1;
	lpimage->ImageDepth = 1;
	lpimage->BitsPixel = 1;
	lpimage->lpdsBmi = NULL;		/* (WIN32) DIBSECTION */
    }
    else {
	lpimage->fMono = FALSE;
#if 0
	lpimage->LinePad    = 32;
#else
	lpimage->LinePad    = 16;
#endif
	hDC = CreateCompatibleDC(0);
	lpimage->ImagePlanes = GetDeviceCaps(hDC,PLANES);
	lpimage->ImageDepth = GetDeviceCaps(hDC,BITSPIXEL);
	DeleteDC(hDC);
	lpimage->BitsPixel = DRVCALL_IMAGES(PIH_GETDEVCAPS,
				DRV_QUERYBITSPIXEL,0,0);
	lpimage->lpdsBmi = NULL;		/* (WIN32) DIBSECTION */
    }
    lpimage->WidthBytes = CalcByteWidth(lpimage->ImageWidth,
			lpimage->BitsPixel,lpimage->LinePad);
    nLength = lpimage->WidthBytes * lpimage->ImageHeight;
    lpData = WinMalloc(nLength);

    if (!lpData)
        return FALSE;

    if (lpBits) { 
	if (lpimage->fMono)
		    /* mono DDB has to be converted to mono image */
	    ConvertMonoDDBToImage((LPBYTE)lpData,lpBits,
			lpimage->WidthBytes,
			CalcByteWidth(lpimage->ImageWidth,
				lpimage->BitsPixel,
				lpimage->LinePad),
			lpimage->ImageHeight);
	else {
	    if (lpimage->ImageDepth == bBitsPixel)
		memcpy(lpData,(LPSTR)lpBits,nLength);
	    else {
		ERRSTR((LF_ERROR,
		    "CreateDDBImage: unsupported DDB format conversion!\n"));
		return FALSE;
	    }
	}
    }
    else
	memset((LPSTR)lpData,'\0',nLength);

    cid.nWidth = lpimage->ImageWidth;
    cid.nHeight = lpimage->ImageHeight;
    cid.nBitsPixel = lpimage->ImageDepth;
    cid.lpData = lpData;
    cid.nWidthBytes = lpimage->WidthBytes;
    cid.nLinePad = lpimage->LinePad;

    if (!(lpimage->lpDrvData = (LPVOID)DRVCALL_IMAGES(PIH_CREATEIMAGE,
		lpimage->fMono,0L,&cid))) {
	ERRSTR((LF_ERROR,
		"CreateDDBImage: failed to create image\n"));
	WinFree(lpData);
	return FALSE;
    }

    return TRUE;
}

void
ConvertMonoDDBToImage(LPBYTE lpDest, LPBYTE lpSrc,
			int widthDest, int widthSrc, int height)
{
    int x,y;
    BOOL fReverse;
    LPBYTE ptrSrc,ptrDest;

    fReverse = (DRVCALL_IMAGES(PIH_GETDEVCAPS,DRV_BITORDER,0L,NULL) ==
			LSBFIRST)?TRUE:FALSE;

    ptrSrc = lpSrc;
    ptrDest = lpDest;
    memset((LPSTR)lpDest,'\0',widthDest*height);
    for(y=0;y<height;y++) {
	for(x=0;x<min(widthDest,widthSrc);x++)
	    ptrDest[x] = (fReverse) ? bit_reverse[ptrSrc[x]] : ptrSrc[x];
	ptrSrc += widthSrc;
	ptrDest += widthDest;
    }
}

static int mask_to_shift(DWORD mask)
{
	int i;

	for (i = 0; i < 32; i++, mask >>= 1)
	{
		if (mask & 1)
			return (i);
	}
	return (0);

}

BOOL
ConvertDIBFormat(LPIMAGEINFO lpimage, BITMAPINFOHEADER *lpbmih,
		  RGBQUAD *lpRGB, LPVOID lpInitBits,
		  UINT uStartScan, UINT uScanLines)
{
    int nSrcWidth,nDestWidth;
    LPBYTE lpBitmapBits = 0;
    LPBYTE src, dest, s, d, ptr;
    BYTE byte;
    LPDWORD lpPixCache, lpDW;
    LPWORD lpW;
    LPBYTE lpB;
    DWORD dword;
    DWORD red_mask, green_mask, blue_mask;
    int red_shift, green_shift, blue_shift;
    int nByteOrder;
    DWORD dwColorIndex;
    BOOL fReverse;
    int i,j,x,y;

    nSrcWidth = CalcByteWidth(lpbmih->biWidth,lpbmih->biBitCount,32);
							/* 32-bit aligned */
    if (!nSrcWidth)
	return FALSE;		/* unsupported format */
    nDestWidth = lpimage->WidthBytes;
    src = lpInitBits;
    dest = (LPBYTE)DRVCALL_IMAGES(PIH_GETIMAGEDATA,0L,0L,lpimage->lpDrvData);
    if (dest == NULL)
	return FALSE;	
    dest += nDestWidth * lpimage->ImageHeight;
    if (uStartScan)
	dest -= nDestWidth * uStartScan;
    lpPixCache = MapColorTable(lpRGB,lpbmih->biBitCount,
				lpimage->ImageDepth);

    if (lpimage->fMono && (lpbmih->biBitCount == 1)) {	/* MONO case */
	fReverse = (DRVCALL_IMAGES(PIH_GETDEVCAPS,DRV_BITORDER,0L,NULL) ==
			LSBFIRST)?TRUE:FALSE;
	for (y=0; y<(int)uScanLines; y++) {
	    dest -= nDestWidth;
	    for (x=0; x<nSrcWidth; x++) {
		byte = *src++;
		if (x < nDestWidth) {
		    if (lpPixCache[0] == 1)
			byte = ~byte;
		    dest[x] = (fReverse)?bit_reverse[byte]:byte;
		}
	    }
	}
    }
    else {
	if (lpbmih->biCompression) {
            lpBitmapBits = (LPBYTE)DecodeRLE(lpbmih,lpInitBits);
	    src = lpBitmapBits;
        }

	red_mask = DRVCALL_IMAGES(PIH_GETDEVCAPS, DRV_QUERYREDMASK, 0L,
		lpimage->lpDrvData);
	green_mask = DRVCALL_IMAGES(PIH_GETDEVCAPS, DRV_QUERYGREENMASK, 0L,
		lpimage->lpDrvData);
	blue_mask = DRVCALL_IMAGES(PIH_GETDEVCAPS, DRV_QUERYBLUEMASK, 0L,
		lpimage->lpDrvData);
	red_shift = mask_to_shift(red_mask);
	green_shift = mask_to_shift(green_mask);
	blue_shift = mask_to_shift(blue_mask);

	dwColorIndex = (lpimage->BitsPixel << 16) | 
			(lpimage->ImageDepth << 8) | 
			(BYTE)lpbmih->biBitCount;

	switch (dwColorIndex) {
	    case 0x10101:
		fReverse = (DRVCALL_IMAGES(PIH_GETDEVCAPS,
			DRV_BITORDER,0L,NULL) == LSBFIRST)?TRUE:FALSE;
		for (y=0; y<(int)uScanLines; y++) {
		    dest -= nDestWidth;
		    for (x=0; x<nSrcWidth; x++) {
			byte = *src++;
			if (x < nDestWidth) {
			    if (lpPixCache[0] == 1)
				byte = ~byte;
			    dest[x] = (fReverse)?bit_reverse[byte]:byte;
			}
		    }
		}
		break;
	    case 0x10104:
		fReverse = (DRVCALL_IMAGES(PIH_GETDEVCAPS,
			DRV_BITORDER,0L,NULL) == LSBFIRST)?TRUE:FALSE;
		for (y=0; y<(int)uScanLines; y++) {
		    dest -= nDestWidth;
		    ptr = dest;
		    for (x=0,j=0; x<nDestWidth; x++,ptr++) {
			*ptr = 0;
			for (i=0; i<8; i++) {
			    byte = src[j++];
			    if ((x*8 + i) < lpbmih->biWidth) {
				*ptr |= (fReverse) ? 
					((lpPixCache[byte >> 4] & 1) << i) :
					((lpPixCache[byte >> 4] & 1) << (7-i));
				if ((x*8 + ++i) < lpbmih->biWidth)
				    *ptr |= (fReverse) ?
					((lpPixCache[byte & 0xf] &1) << i) :
					((lpPixCache[byte & 0xf] &1) << (7-i));
			    }
			}
		    }
		    src += nSrcWidth;
		}
		break;
	    case 0x10108:
		fReverse = (DRVCALL_IMAGES(PIH_GETDEVCAPS,
			DRV_BITORDER,0L,NULL) == LSBFIRST)?TRUE:FALSE;
		for (y=0; y<(int)uScanLines; y++) {
		    dest -= nDestWidth;
		    ptr = dest;
		    for (x=0,j=0; x<nDestWidth; x++,ptr++) {
			*ptr = 0;
			for (i=0; i<8; i++) {
			    byte = src[j];
			    if (j++ < lpbmih->biWidth)
				*ptr |= (fReverse) ? 
					((lpPixCache[byte] & 1) << i) :
					((lpPixCache[byte] & 1) << (7-i));
			}
		    }
		    src += nSrcWidth;
		}
		break;
	    case 0x40401:
		for (y=0; y<(int)uScanLines; y++) {
		    dest -= nDestWidth;
		    ptr = dest;
		    for (x=0; x<nSrcWidth; x++) {
			byte = bit_reverse[*src++];
			for (i=0,j=0; i<8; i++,j++) {
			    if ((x*8 + i) < lpbmih->biWidth)
				ptr[j] = lpPixCache[(byte >> i) & 1] << 4;
			    if ((x*8 + ++i) < lpbmih->biWidth)
				ptr[j] |= lpPixCache[(byte >> i) & 1];
			}
			if ((x*8 + i) < lpbmih->biWidth)
			    ptr += 4;
		    }
		}
		break;
	    case 0x80401:
		for (y=0; y<(int)uScanLines; y++) {
		    dest -= nDestWidth;
		    ptr = dest;
		    for (x=0; x<nSrcWidth; x++) {
			byte = bit_reverse[*src++];
			for (i=0,j=0; i<8; i++,j++) {
			    if ((x*8 + i) < lpbmih->biWidth)
				ptr[j] = lpPixCache[(byte >> i) & 1];
			    if ((x*8 + ++i) < lpbmih->biWidth)
				ptr[++j] = lpPixCache[(byte >> i) & 1];
			}
			if ((x*8 + i) < lpbmih->biWidth)
			    ptr += 8;
		    }
		}
		break;
	    case 0x101001:
	    case 0x100F01:
		for (y=0; y<(int)uScanLines; y++) {
		    dest -= nDestWidth;
		    ptr = dest;
		    for (x=0; x<nSrcWidth; x++) {
			byte = bit_reverse[*src++];
			for (i=0,j=0; i<8; i++,j+=2) {
			    if ((x*8 + i) < lpbmih->biWidth) 
			    {
				*((LPWORD)&ptr[j]) =
					lpPixCache[(byte >> i) & 1];
			    }
			}
			if ((x*8 + i) < lpbmih->biWidth)
			    ptr += 0x10;
		    }
		}
		break;
	    case 0x201801: /* XFree86 24bpp -- JJF 2/20/96 */ 
		for (y=0; y<(int)uScanLines; y++) {
		  dest -= nDestWidth;
		  ptr = dest;
		  for (x=0; x<nSrcWidth; x++) {
		    byte = bit_reverse[*src++];
		    for (i=0,j=0; i<8; i++, j+=4) {
		      if ((x*8 + i) < lpbmih->biWidth) 
			*((LPDWORD)&ptr[j]) =
			       	lpPixCache[(byte >> i) & 1];
		    }
		    if ((x*8 + i) < lpbmih->biWidth)
		      ptr += 0x20;
		  }
		} 
		break;
	    case 0x40404:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		    ptr = src;
		    lpB = (dest -= nDestWidth);
 		    for (i=0; i<lpbmih->biWidth; i+=2) {
			byte = *ptr++;
			*lpB = lpPixCache[(byte >> 4) & 0xf];
			*lpB++ |= lpPixCache[byte & 0xf];
		    }
		}
		break;
	    case 0x80404:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		    ptr = src;
		    lpB = (dest -= nDestWidth);
 		    for (i=0; i<lpbmih->biWidth; i+=2) {
			byte = *ptr++;
			*lpB++ = lpPixCache[(byte >> 4) & 0xf];
			*lpB++ = lpPixCache[byte & 0xf];
		    } 
		}
		break;
	    case 0x80801:
		for (y=0; y<(int)uScanLines; y++) {
		    dest -= nDestWidth;
		    ptr = dest;
		    for (x=0; x<nSrcWidth; x++) {
			byte = bit_reverse[*src++];
			for (i=0; i<8; i++) 
			    if ((x*8 + i) < lpbmih->biWidth)
				ptr[i] = lpPixCache[(byte >> i) & 1];
			if ((x*8 + i) < lpbmih->biWidth)
			    ptr += 8;
		    }
		}
		break;
	    case 0x80804:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		    ptr = src;
		    lpB = (dest -= nDestWidth);
 		    for (i=0; i<lpbmih->biWidth; i+=2) {
			byte = *ptr++;
			*lpB++ = lpPixCache[(byte >> 4) & 0xf];
			*lpB++ = lpPixCache[byte & 0xf];
		    } 
		}
		break;
	    case 0x80408:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		    ptr = src;
		    dest -= nDestWidth;
		    for (i=0; i<lpbmih->biWidth; i++,ptr++) 
			dest[i] = lpPixCache[*ptr];
		}
		break;
	    case 0x80808:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		    ptr = src;
		    dest -= nDestWidth;
		    for (i=0; i<lpbmih->biWidth; i++,ptr++) 
			dest[i] = lpPixCache[*ptr];
		}
		break;
	    case 0x80418:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		    ptr = src;
		    dest -= nDestWidth;
		    for (i=0; i<lpbmih->biWidth; i++,ptr+=3)
			dest[i] = lpPixCache[MAPRGB(*ptr, *(ptr+1), *(ptr+2))];
		}
		break;
	    case 0x80818:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		    ptr = src;
		    dest -= nDestWidth;
		    for (i=0; i<lpbmih->biWidth; i++,ptr+=3)
			dest[i] = lpPixCache[MAPRGB(*ptr, *(ptr+1), *(ptr+2))];
		}
		break;
	    case 0x101004:
	    case 0x100F04:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		    ptr = src;
		    lpW = (LPWORD)(dest -= nDestWidth);
 		    for (i=0; i<lpbmih->biWidth; i+=2) {
			byte = *ptr++;
			*lpW++ = lpPixCache[(byte >> 4) & 0xf];
			*lpW++ = lpPixCache[byte & 0xf];
		    } 
		}		
		break;
	    case 0x101008:
	    case 0x100F08:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		    ptr = src;
		    lpW = (LPWORD)(dest -= nDestWidth);
		    for (i=0; i<lpbmih->biWidth; i++,ptr++) 
			*lpW++ = lpPixCache[*ptr];
		}
		break;
	    case 0x101018:
	    case 0x100F18:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		  ptr = src;
		  lpW = (LPWORD)(dest -= nDestWidth);
		  for (i=0; i<lpbmih->biWidth; i++,ptr+=3)
		      *lpW++ = lpPixCache[MAPRGB(*ptr, *(ptr+1), *(ptr+2))];
		}
		break;
/* Changes to support XFree86 24bpp originally submitted by JJF 2/20/96 */
/* 0x201804: change to prevent from writing over the end of Dest */ 
	    case 0x201804:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		  ptr = src;
		  lpDW = (LPDWORD)(dest -= nDestWidth);
 		  for (i=0; i<lpbmih->biWidth-1; i+=2) {
		    byte = *ptr++;
		    dword = lpPixCache[(byte >> 4) & 0xf];
		    *lpDW++ = dword;
		    dword = lpPixCache[byte & 0xf];
		    *lpDW++ = dword;
		  } 
		  if(lpbmih->biWidth % 2)
		  {
		    byte = *ptr++;
		    dword = lpPixCache[(byte >> 4) & 0xf];
		    *lpDW++ = dword;
		  } 
		}		
		break;
	    case 0x201808:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		  ptr = src;
		  lpDW = (LPDWORD)(dest -= nDestWidth);
		  for (i=0; i<lpbmih->biWidth; i++)
		  {
		      *lpDW++ = lpPixCache[*ptr++];
		  }
		}
		break;
	    case 0x201818:
		for (y=0; y<(int)uScanLines; y++, src += nSrcWidth) {
		  ptr = src;
		  lpDW = (LPDWORD)(dest -= nDestWidth);
		  for (i=0; i<lpbmih->biWidth; i++,ptr+=3)
		  {
			dword = ((*ptr << red_shift) & red_mask)
				| ((ptr[1] << green_shift) & green_mask)
				| ((ptr[2] << blue_shift) & blue_mask);
			*lpDW++ = dword;
		  }
		}
		break;
	    case 0x181801:
		nByteOrder = (DRVCALL_IMAGES(PIH_GETDEVCAPS, DRV_BYTEORDER,
			0, 0));
		for (y = 0; y < (int) uScanLines; y++, src += nSrcWidth)
		{
			s = src;
			d = (dest -= nDestWidth);
			for (x = 0; x < lpbmih->biWidth; x += 8)
			{
				byte = *s++;
				for (i = 0; i < 8; i++)
				{
					if (x + i >= lpbmih->biWidth)
						break;
					dword = lpPixCache[(byte>>(7-i))&1];
					if (nByteOrder == MSBFIRST)
					{
						*d++ = (dword & 0x0000ffL);
						*d++ = (dword & 0x00ff00L)>>8;
						*d++ = (dword & 0xff0000L)>>16;
					}
					else
					{
						*d++ = (dword & 0xff0000L)>>16;
						*d++ = (dword & 0x00ff00L)>>8;
						*d++ = (dword & 0x0000ffL);
					}
				}
			}
		}
		break;
	    case 0x181804:
		nByteOrder = (DRVCALL_IMAGES(PIH_GETDEVCAPS, DRV_BYTEORDER,
			0, 0));
		for (y = 0; y < (int) uScanLines; y++, src += nSrcWidth)
		{
			s = src;
			d = (dest -= nDestWidth);
			for (x = 0; x < lpbmih->biWidth; x += 2)
			{
				byte = *s++;
				if (nByteOrder == MSBFIRST)
				{
					dword = lpPixCache[(byte>>4)&0x0f];
					*d++ = (dword & 0x0000ffL);
					*d++ = (dword & 0x00ff00L) >> 8;
					*d++ = (dword & 0xff0000L) >> 16;
					dword = lpPixCache[byte&0x0f];
					*d++ = (dword & 0x0000ffL);
					*d++ = (dword & 0x00ff00L) >> 8;
					*d++ = (dword & 0xff0000L) >> 16;
				}
				else
				{
					dword = lpPixCache[(byte>>4)&0x0f];
					*d++ = (dword & 0xff0000L) >> 16;
					*d++ = (dword & 0x00ff00L) >> 8;
					*d++ = (dword & 0x0000ffL);
					dword = lpPixCache[byte&0x0f];
					*d++ = (dword & 0xff0000L) >> 16;
					*d++ = (dword & 0x00ff00L) >> 8;
					*d++ = (dword & 0x0000ffL);
				}
			}
		}
		break;
	    case 0x181808:
		nByteOrder = (DRVCALL_IMAGES(PIH_GETDEVCAPS, DRV_BYTEORDER,
			0, 0));
		for (y = 0; y < (int) uScanLines; y++, src += nSrcWidth)
		{
			s = src;
			d = (dest -= nDestWidth);
			for (x = 0; x < lpbmih->biWidth; x++)
			{
				dword = lpPixCache[*s++];
				if (nByteOrder == MSBFIRST)
				{
					*d++ = (dword & 0x0000ffL);
					*d++ = (dword & 0x00ff00L) >> 8;
					*d++ = (dword & 0xff0000L) >> 16;
				}
				else
				{
					*d++ = (dword & 0xff0000L) >> 16;
					*d++ = (dword & 0x00ff00L) >> 8;
					*d++ = (dword & 0x0000ffL);
				}
			}
		}
		break;
	    default:
		ERRSTR((LF_ERROR, "ConvertDIB: (ERR) format not supported: %x\n",
			dwColorIndex));
		break;
	}
	if (lpBitmapBits)
	    WinFree((LPSTR)lpBitmapBits);
    }
    /* If there is a pixmap created from the image, bring it in sync */
    DRVCALL_IMAGES(PIH_UPDATEPIXMAP,0L,0L,lpimage->lpDrvData);

    return TRUE;
}

static LPBYTE    
DecodeRLE(LPBITMAPINFOHEADER lpInfoHeader, LPBYTE lpSource)
{
    DWORD    nDestSize, nLineSize, nByteCount = 0, nLineCount = 0;
    DWORD    nBufSize;
    int    i;
    LPBYTE    lpDest, lpDestSave, lpTemp;
    BOOL    bRLE4 = FALSE;
    BYTE    ColorIndex, count;
    
    if (lpInfoHeader->biCompression == BI_RLE4)
	bRLE4 = TRUE;

    /* this is a byte width of the destination bitmap */
    nDestSize = CalcByteWidth(lpInfoHeader->biWidth,
		lpInfoHeader->biBitCount,
		32);
    /* this is an image size of the destination bitmap */
    nDestSize *= lpInfoHeader->biHeight;

    if (bRLE4)
	nBufSize = nDestSize * 2;
    else
	nBufSize = nDestSize;

    if (!(lpDest = (LPBYTE) WinMalloc(nBufSize)))
	return (LPBYTE)NULL;

    lpDestSave = lpDest;
    memset((LPSTR)lpDest, 0, nBufSize);

    /* this is a line size in the destination bitmap */
    nLineSize = nBufSize / lpInfoHeader->biHeight;

    while (nByteCount < nBufSize) {
	if ( lpSource[0] == 0x00) { /* escape or absolute mode */
	    switch (lpSource[1]) {
	    case 0:	/* escape end of line */
		/* advance destination pointer to the next line */
		lpDest += (nLineSize - nLineCount);
		nByteCount += (nLineSize - nLineCount);
		nLineCount = 0;
		break;
	    case 1:	/* escape end of bitmap */
		if (bRLE4) {
		    /* combine nibbles */
		    lpTemp = lpDestSave;
		    for (i = 0; i < nDestSize * 2; i += 2  ) {
			 lpTemp[0] = lpDestSave[i] | lpDestSave[i+ 1];
			 lpTemp++;
		    }
		}
		return lpDestSave;
	    case 2:	/* escape delta */
		lpDest += lpSource[2];
		nByteCount += lpSource[2];
		nLineCount += lpSource[2];
		lpDest += lpSource[3] * nLineSize;
		nByteCount += lpSource[3] * nLineSize;
		lpSource += 2;
		break;
	    default:	/* absolute mode */
		for (i = 0; (BYTE)i < lpSource[1]; i++) {
		    if (nByteCount > nBufSize)
			break;
		    if (bRLE4)
			GetColorIndex(lpSource[i/2+2], bRLE4, TRUE);
		    else
			GetColorIndex(lpSource[i+2], bRLE4, TRUE);
		    ColorIndex = GetColorIndex(0, bRLE4, FALSE);
		    *lpDest = ColorIndex;
		    nByteCount++;
		    nLineCount++;
		    lpDest++;
		    if (bRLE4) {
			ColorIndex = GetColorIndex(0, bRLE4, FALSE);
			*lpDest = ColorIndex;
			nByteCount++;
			nLineCount++;
			lpDest++;
			i++;
		    }
		}
		count = lpSource[1];
		if (bRLE4)
		    lpSource += i/2;
		else
		    lpSource += i;
		if (!bRLE4 && (count % 2))
		    lpSource++;
		else if(bRLE4) {  
		    if(((count % 4) == 1) || ((count % 4) == 2))
			lpSource++;
		}
	    }    
	} else {
	    GetColorIndex(lpSource[1], bRLE4, TRUE);
	    for(i = 0; (BYTE)i < lpSource[0]; i++) {
		    ColorIndex = GetColorIndex(0, bRLE4, FALSE);
		    *lpDest = ColorIndex;
		    nByteCount++;
		    nLineCount++;
		    lpDest++;
	    }
	}
	lpSource += 2;
    }
    if (bRLE4) {
	lpTemp = lpDestSave;
	for (i = 0; i < nDestSize * 2; i += 2  ) {
	     lpTemp[0] = (lpDestSave[i]) | (lpDestSave[i+ 1]);
	     lpTemp++;
	}
    }
    return lpDestSave;
}

static BYTE
GetColorIndex(BYTE nVal, BOOL bRLE4, BOOL bReset)
{
    static    BYTE    Value;
    static BOOL    bLowNibble = FALSE;
    
    if (bReset) {
	Value = nVal;
	bLowNibble = FALSE;
	return (0);
    }
    if (bRLE4) {
	if (bLowNibble) {
	    bLowNibble = FALSE;
	    return (Value & 0x0f);
	} else {
	    bLowNibble = TRUE;
	    return (Value & 0xf0);
	}    
    } else
	return (Value);
}
 
/*	MapColorTable fills in static array with pixel values
	from given array of RGBQUAD values	*/

static LPDWORD
MapColorTable(RGBQUAD *lprgb, UINT uSrcBitsPixel, UINT uDestBitPixel)
{
    static DWORD PixelCache[4096];
    UINT i;
    UINT uNumColors;
    UINT r,g,b,r1,g1,b1;

    /*  If no palette (i.e., real colors), create translation table.
     *  The table is generated by producing a 'fake' palette in the
     *  PixelCache structure and then performing normal algorithms.
     */
    if ( uSrcBitsPixel > 8 ) {
	uSrcBitsPixel = 12;
	lprgb = (LPRGBQUAD)PixelCache;
	for (r=0; r<16; r++)
	    for (g=0; g<16; g++)
		for (b=0; b<16; b++) {
		    lprgb->rgbRed = r + (r<<4);
		    lprgb->rgbGreen = g + (g<<4);
		    lprgb->rgbBlue = b + (b<<4);
		    lprgb++;
		}
	lprgb = (LPRGBQUAD)PixelCache;	/* reset 'palette' pointer */
    }

    uNumColors = 1 << uSrcBitsPixel;

    if (uDestBitPixel == 1)
	for (i=0; i<uNumColors; i++,lprgb++) {
	    r = lprgb->rgbRed;
	    g = lprgb->rgbGreen;
	    b = lprgb->rgbBlue;
	    r1 = 255 - r;
	    g1 = 255 - g;
	    b1 = 255 - b;
	    PixelCache[i] = ((r*r + g*g + b*b) > (r1*r1 + g1*g1 + b1*b1))?1:0;
	}
    else
	for (i=0;i<uNumColors;i++,lprgb++) {
	    PixelCache[i] = DRVCALL_COLORS(PLH_MAKEPIXEL,
				RGB((BYTE)lprgb->rgbRed,
				    (BYTE)lprgb->rgbGreen,
				    (BYTE) lprgb->rgbBlue),
				0L,0L);
	}
    return PixelCache;
}

BOOL
FetchDIBits(HDC hDC, LPIMAGEINFO lpimage, UINT nStartScan, UINT cScanLines,
	BITMAPINFOHEADER *lpbmih, RGBQUAD *lprgb, LPBYTE bmbits)
{
    int    x,y,i;
    DWORD    pixel;
    int nSrcWidth, nDestWidth;
    DWORD dwColorIndex;
    LPBYTE src,dest;
    UINT ColorArray[256];
    COLORREF cr;
    int nImageDepth,nImageBitCount;
    LPBYTE lpData;
    BOOL fReverse;

    nImageDepth = DRVCALL_IMAGES(PIH_GETDEVCAPS,
			DRV_QUERYDEPTH,lpbmih->biBitCount,lpimage->lpDrvData);
    if (nImageDepth == 1)
	nImageBitCount = 1;
    else
	nImageBitCount = DRVCALL_IMAGES(PIH_GETDEVCAPS,DRV_QUERYBITSPIXEL,0,0);

    /* take the max possible padding; we'll reassign the value with */
    /* whatever the driver returns */
    nSrcWidth = CalcByteWidth(lpimage->ImageWidth,nImageBitCount,32);
    lpData = (LPBYTE)WinMalloc(nSrcWidth*lpimage->ImageHeight);

    if (!lpData)
        return FALSE;

    nSrcWidth = (int)DRVCALL_IMAGES(PIH_FETCHBITS,
			lpData,nImageBitCount,lpimage->lpDrvData);
    if (!nSrcWidth)
	return FALSE;

    src = lpData;
    dest = bmbits;
    if (!(src && dest))
	return FALSE;
    nDestWidth = CalcByteWidth(lpimage->ImageWidth,lpbmih->biBitCount,32);

    src += (nSrcWidth * nStartScan);
    dest += (nDestWidth * cScanLines);

    dwColorIndex = (((BYTE)lpbmih->biBitCount) << 16) |
			(BYTE)nImageDepth << 8 | (BYTE)nImageBitCount;

    switch (dwColorIndex) {
	case 0x010101:
	    fReverse = (DRVCALL_IMAGES(PIH_GETDEVCAPS,
		DRV_BITORDER,0L,NULL) == LSBFIRST)?TRUE:FALSE;
	    FillRGBTable(hDC,lprgb,2);
	    for (y=0; y<cScanLines; y++) {
		dest -= nDestWidth;
		for (x=0; x<min(nSrcWidth,nDestWidth); x++) 
		    dest[x] = (fReverse)?bit_reverse[*src++]:*src++;
	    }
	    break;
	case 0x010808:
	{
	    BYTE p,b;
	    BYTE Cache[256];
	    LPBYTE ptr;

	    FillRGBTable(hDC,lprgb,2);
	    for (i=0; i<256; i++)
		Cache[i] = (BYTE)-1;
	    for (y=0; y<cScanLines; y++) {
		dest -= nDestWidth;
		ptr = dest;
		for (x=0; x<nDestWidth; x++,ptr++) {
		    *ptr = 0;
		    for (i=0; i<8; i++) {
			if ((x*8 + i) < nSrcWidth) {
			    p = *src++;
			    b = MapColorIndex(p,lprgb,&Cache[0],2);
			    *ptr |= (b & 1) << (7-i);
			}
		    }
		}
	    }
	    break;
	}
	case 0x040808:
	{
	    BYTE p;
	    BYTE Cache[256];

	    FillRGBTable(hDC,lprgb,16);
	    for (i=0; i<256; i++)
		Cache[i] = (BYTE)-1;
	    for (y=0; y<cScanLines; y++) {
		dest -= nDestWidth;
		for (x=0; x<nDestWidth; x++) {
		    p = *src++;
		    i = MapColorIndex(p,lprgb,&Cache[0],16);
		    dest[x] = (i << 4) & 0xf0;
		    p = *src++;
		    i = MapColorIndex(p,lprgb,&Cache[0],16);
		    dest[x] |= i & 0xf;
		}
	    }
	    break;
	}
	case 0x080808:
	    i = 0;
	    memset((LPSTR)&ColorArray[0],0xff,sizeof(ColorArray));
	    for (y=0; y<cScanLines; y++) {
		dest -= nDestWidth;
		for (x=0; x<min(nSrcWidth,nDestWidth); x++) { 
		    pixel = *src++;
		    if (ColorArray[(BYTE)pixel] == (UINT)-1) {
			cr = (COLORREF)DRVCALL_COLORS(PLH_GETCOLORREF,
					pixel,0L,0L);
			lprgb[i].rgbRed = GetRValue(cr);
			lprgb[i].rgbGreen = GetGValue(cr);
			lprgb[i].rgbBlue = GetBValue(cr);
			ColorArray[(BYTE)pixel] = i++;
		    }
		    dest[x] = ColorArray[pixel];
		}
	    }
	    lpbmih->biClrUsed = i;
	    break;
	case 0x040101:
	case 0x040404:
	case 0x041818:
	case 0x080101:
	case 0x080404:
	case 0x010404:
	case 0x081818:
	case 0x180101:
	case 0x180404:
	case 0x180808:
	    printf("****ERROR**** FetchDIB: unsupported conversion %x\n",
		(int) dwColorIndex);
            break;

        
	// KimSG : add for preview image save, 16bit display
 	// Thanks quick_quant function in xv program
/* up to 256 colors:     3 bits R, 3 bits G, 2 bits B  (RRRGGGBB) */
#define RMASK      0xe0
#define RSHIFT        0
#define GMASK      0xe0
#define GSHIFT        3
#define BMASK      0xc0
#define BSHIFT        6
/* RANGE forces a to be in the range b..c (inclusive) */
#define RANGE(a,b,c) { if (a < b) a = b;  if (a > c) a = c; }
	case 0x81010:
	{
	    BYTE rmap[256], gmap[256], bmap[256];
	    LPBYTE s, d;
	    LPBYTE thisptr, nextptr;
    	    WORD word;
  	    int r1, g1, b1;
  	    int  imax, jmax;
	    int val;

	    imax = lpimage->ImageHeight-1; jmax = lpimage->ImageWidth-1;

  	   /* load up colormap:
   	    *   note that 0 and 255 of each color are always in the map;
  	    *   intermediate values are evenly spaced.
   	    */

  	    for (i=0; i<256; i++) {
    		rmap[i] = (((i<<RSHIFT) & RMASK) * 255 + RMASK/2) / RMASK;
    		gmap[i] = (((i<<GSHIFT) & GMASK) * 255 + GMASK/2) / GMASK;
    		bmap[i] = (((i<<BSHIFT) & BMASK) * 255 + BMASK/2) / BMASK;
		lprgb[i].rgbRed = rmap[i];
		lprgb[i].rgbGreen = gmap[i];
		lprgb[i].rgbBlue = bmap[i];
  	    }

	    for (y=0; y<cScanLines; y++, src += nSrcWidth) 
	    {
		s = src;
		d = (dest -= nDestWidth);
		thisptr = s;
		nextptr = s + nSrcWidth;

		for (x=0; x<nSrcWidth/2; x++, s += 2)
	 	{ 
		    thisptr += 2;
		    word = (*(s+1) << 8) | *(s);
		    cr = (COLORREF)DRVCALL_COLORS(PLH_GETCOLORREF,word,0L,0L);
		    b1 = GetRValue(cr);
		    g1 = GetGValue(cr);
                    r1 = GetBValue(cr);
      		    RANGE(r1,0,255);  RANGE(g1,0,255);  RANGE(b1,0,255);  
      
		    /* choose actual pixel value */
      		    val = (((r1&RMASK)>>RSHIFT) | ((g1&GMASK)>>GSHIFT) | 
	     		((b1&BMASK)>>BSHIFT));
      		    *d++ = val;

#if LATER // rewrite 2byte color index format; it's complex

		    /* compute color errors */
      		    r1 -= rmap[val];
      		    g1 -= gmap[val];
      		    b1 -= bmap[val];

      		    /* Add fractions of errors to adjacent pixels */
      		    if (x!=jmax) {  /* adjust RIGHT pixel */
		    	thisptr[0] += (r1*7) / 16;
		    	thisptr[1] += (g1*7) / 16;
		    	thisptr[2] += (b1*7) / 16;
      		    }
      
     		    if (y!=imax) {	/* do BOTTOM pixel */
		        nextptr[0] += (r1*5) / 16;
		        nextptr[1] += (g1*5) / 16;
		        nextptr[2] += (b1*5) / 16;

		        if (x>0) {  /* do BOTTOM LEFT pixel */
	  	          nextptr[-3] += (r1*3) / 16;
	  	          nextptr[-2] += (g1*3) / 16;
	  	          nextptr[-1] += (b1*3) / 16;
		        }

		    	if (x!=jmax) {  /* do BOTTOM RIGHT pixel */
		    	  nextptr[3] += (r1)/16;
		    	  nextptr[4] += (g1)/16;
		    	  nextptr[5] += (b1)/16;
		    	}
      		    }
#endif
		}
	    }
	    lpbmih->biClrUsed = i;
	
	    break;
	}

	// KimSG : add for preview image save, 32bit display
	case 0x81820:
	{
	    BYTE rmap[256], gmap[256], bmap[256];
	    LPBYTE s, d;
    	    DWORD dword;
  	    int r1, g1, b1;
	    int val;


  	   /* load up colormap:
   	    *   note that 0 and 255 of each color are always in the map;
  	    *   intermediate values are evenly spaced.
   	    */

  	    for (i=0; i<256; i++) {
    		rmap[i] = (((i<<RSHIFT) & RMASK) * 255 + RMASK/2) / RMASK;
    		gmap[i] = (((i<<GSHIFT) & GMASK) * 255 + GMASK/2) / GMASK;
    		bmap[i] = (((i<<BSHIFT) & BMASK) * 255 + BMASK/2) / BMASK;
		lprgb[i].rgbRed = rmap[i];
		lprgb[i].rgbGreen = gmap[i];
		lprgb[i].rgbBlue = bmap[i];
  	    }

	    for (y=0; y<cScanLines; y++, src += nSrcWidth) 
	    {
		s = src;
		d = (dest -= nDestWidth);

		for (x=0; x<nSrcWidth/4; x++, s += 4)
	 	{ 
		    dword = (*(s+2) << 16) | (*(s+1) << 8) | *(s);
		    cr = (COLORREF)DRVCALL_COLORS(PLH_GETCOLORREF,dword,0L,0L);
		    b1 = GetRValue(cr);
		    g1 = GetGValue(cr);
                    r1 = GetBValue(cr);
      		    RANGE(r1,0,255);  RANGE(g1,0,255);  RANGE(b1,0,255);  
      
		    /* choose actual pixel value */
      		    val = (((r1&RMASK)>>RSHIFT) | ((g1&GMASK)>>GSHIFT) | 
	     		((b1&BMASK)>>BSHIFT));
      		    *d++ = val;
		}
	    }
	    lpbmih->biClrUsed = i;

	    break;
	}

        case 0x11010:
	    printf("****ERROR**** FetchDIB: unsupported conversion %x\n",
		(int) dwColorIndex);
             break;

	case 0x181010:
	{
	    LPBYTE s, d;
    	    WORD word;

	    for (y=0; y<cScanLines; y++, src += nSrcWidth) 
	    {
		s = src;
		d = (dest -= nDestWidth);

		for (x=0; x<nSrcWidth/2; x++, s += 2)
	 	{ 
		    word = (*(s+1) << 8) | *(s);
		    cr = (COLORREF)DRVCALL_COLORS(PLH_GETCOLORREF,word,0L,0L);
		    *d++ = GetRValue(cr);
		    *d++ = GetGValue(cr);
                    *d++ = GetBValue(cr);
		}
	    }
	    //lpbmih->biClrUsed = i;

            break;
	}
	case 0x181818:
	    printf("****ERROR**** FetchDIB: unsupported conversion %x\n",
		(int) dwColorIndex);
	    break;

        case 0x181820:
        {
	    LPBYTE s, d;
    	    DWORD dword;

	    for (y=0; y<cScanLines; y++, src += nSrcWidth) 
	    {
		s = src;
		d = (dest -= nDestWidth);

		for (x=0; x<nSrcWidth/4; x++, s += 4)
	 	{ 
		    dword = (*(s+2)<<16) | (*(s+1) << 8) | *(s);
		    cr = (COLORREF)DRVCALL_COLORS(PLH_GETCOLORREF,dword,0L,0L);
		    *d++ = GetBValue(cr);
		    *d++ = GetGValue(cr);
                    *d++ = GetRValue(cr);
		}
	    }
            break;
        }

	default:
	    printf("****ERROR**** FetchDIB: unsupported conversion %x\n",
		(int) dwColorIndex);
	    break;
    }
    WinFree((LPSTR)lpData);
    return TRUE;
}

#define HATCH_BASE 0x4000 /* needs to be defined in rc file, too */

static HBITMAP
TWIN_LoadHatchBitmap(int nHatchIndex)
{
    static HBITMAP HatchBitmaps[6];

    if (HatchBitmaps[nHatchIndex] == 0)
	HatchBitmaps[nHatchIndex] =
			LoadBitmap(0,(LPSTR)(HATCH_BASE+nHatchIndex));

    return HatchBitmaps[nHatchIndex];
}

HBRUSH
GdiCreateBrush(LPLOGBRUSH lpBrush)
{
    HBRUSH hBrush;
    LPBRUSHINFO lpBrushInfo;
    HBITMAP hBitmap = 0;
    HGLOBAL hPackedDIB = 0;
    LPBYTE lpStruct;
    LPBITMAPINFO lpbmi;
    WORD wRGBCount;
    BITMAPIMAGE bimage;
    LPIMAGEINFO lpImageInfo;

    if (!(lpBrushInfo = CREATEHBRUSH(hBrush))) {
	return (HBRUSH)0;
    }
    lpBrushInfo->lpBrush.lbStyle = lpBrush->lbStyle;
    lpBrushInfo->lpBrush.lbColor = lpBrush->lbColor;
    lpBrushInfo->lpBrush.lbHatch = lpBrush->lbHatch;

    /* say the object is not realized */
    lpBrushInfo->fIsRealized = FALSE;

    switch (lpBrush->lbStyle) {
	case BS_NULL:
	    lpBrushInfo->dwFlags = BFP_NULL;
	    RELEASEBRUSHINFO(lpBrushInfo);
	    return hBrush;

	case BS_SOLID:
	    lpBrushInfo->dwFlags = BFP_UNASSIGNED;
	    lpBrushInfo->lpPrivate = 0;
	    RELEASEBRUSHINFO(lpBrushInfo);
	    return hBrush;

	case BS_INDEXED:
	    /* no info on this one ... */
	    RELEASEBRUSHINFO(lpBrushInfo);
	    return hBrush;

	case BS_PATTERN:
	    lpBrushInfo->dwFlags = BFP_PIXMAP;
	    hBitmap = (HBITMAP)(lpBrush->lbHatch);
	    LOCKGDI(hBitmap);
	    break;

	case BS_DIBPATTERNPT:
	    lpBrushInfo->dwFlags = BFP_PIXMAP;
	    lpStruct = (LPBYTE)(lpBrush->lbHatch);
	    goto DIBBrushCommonCode;

	case BS_DIBPATTERN:
	    lpBrushInfo->dwFlags = BFP_PIXMAP;
	    hPackedDIB = (HGLOBAL)lpBrush->lbHatch;
	    if (!(lpStruct = (LPBYTE)GlobalLock(hPackedDIB))) {
	        RELEASEBRUSHINFO(lpBrushInfo);
		FREEGDI(hBrush);
		return (HBRUSH)0;
	    }
	    /* BEGIN BS_DIBPATTERN AND BS_DIBPATTERNPT COMMON CODE */
DIBBrushCommonCode:
	    lpbmi = (LPBITMAPINFO)lpStruct;
	    wRGBCount = (lpbmi->bmiHeader.biClrUsed)?
				lpbmi->bmiHeader.biClrUsed:
				1 << lpbmi->bmiHeader.biBitCount;

	/* Here we can have a monochrome bitmap with colors other than */
	/* black-and-white; if we call CreateDIBitmap, it will map those */
	/* to black and white, and color information will be lost;      */
	/* therefore I'm calling my own wrapper called CreateDIBitmapEx, */
	/* the same one that gets called from LoadBitmap for the same   */
	/* reason       -- LK */

	    bimage.bmp_hdr = &lpbmi->bmiHeader;
	    if (lpBrush->lbColor == DIB_PAL_COLORS)
		bimage.rgb_quad = TWIN_MapPaletteColors(0,lpbmi);
	    else
		bimage.rgb_quad = &lpbmi->bmiColors[0];
	    bimage.bitmap_bits = (LPVOID)(lpStruct +
			sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*wRGBCount);
	    hBitmap = CreateDIBitmapEx(&bimage,NULL);
	    if (lpBrush->lbColor == DIB_PAL_COLORS)
		WinFree((LPSTR)bimage.rgb_quad);
	    /* END OF BS_DIBPATTERN and BS_DIBPATTERNPT COMMON CODE */
	    if (lpBrush->lbStyle == BS_DIBPATTERN)
		GlobalUnlock(hPackedDIB);
	    break;

	case BS_HATCHED:
	    lpBrushInfo->dwFlags = BFP_BITMAP;
	    hBitmap = TWIN_LoadHatchBitmap(lpBrush->lbHatch);
	    break;

	default:
	    RELEASEBRUSHINFO(lpBrushInfo);
	    FREEGDI(hBrush);
	    return (HBRUSH)0;
    }
    /* Now the only styles left are those with hBitmap assigned */

    lpImageInfo = (LPIMAGEINFO)GETBITMAPINFO(hBitmap);
    if (!lpImageInfo) {
	RELEASEBRUSHINFO(lpBrushInfo);
	RELEASEBITMAPINFO(lpImageInfo);
	FREEGDI(hBitmap);
	return (HBRUSH)0;
    }
    lpBrushInfo->lpBrush.lbHatch = (int)hBitmap;
    lpBrushInfo->lpPrivate = (LPVOID)DRVCALL_IMAGES(PIH_CREATEBRUSH,
			lpBrushInfo->dwFlags,0L,
			lpImageInfo->lpDrvData);
    RELEASEBRUSHINFO(lpBrushInfo);
    RELEASEBITMAPINFO(lpImageInfo);
    return hBrush;
}

BOOL
GdiDeleteBitmap(HBITMAP hBitmap)
{
    LPIMAGEINFO lpImageInfo;

    if (!(lpImageInfo = GETBITMAPINFO(hBitmap))) {
	return FALSE;
    }
    if (lpImageInfo->rcsinfo) {
        /* Expanded by Weav 25 Apr 96 to free all the allocations... */
        LPNAMEINFO lpri = lpImageInfo->rcsinfo;
	LPBITMAPIMAGE lpbi = (LPBITMAPIMAGE)lpri->rcsdata;
	/* remove the cached handle */
	lpri->hObject = 0;
	/* if rcsoffset is zero, it's not from a file so don't free... */
	if (lpri->rcsoffset != 0) {  
	  WinFree((LPSTR)lpbi->bmp_hdr);
	  WinFree((LPSTR)lpbi->rgb_quad);
	  WinFree((LPSTR)lpbi->bitmap_bits);
	  WinFree((LPSTR)lpbi);
	  lpri->rcsdata = 0;
	  lpbi = 0;
	  GlobalUnlock(lpri->hGlobal);
	  GlobalFree(lpri->hGlobal);
	  lpri->hGlobal = 0;
	  RELEASEHBITMAP(hBitmap);
	  FREEBITMAP(hBitmap); 
	}
    }

    DRVCALL_IMAGES(PIH_DESTROYIMAGE,0L,0L,lpImageInfo->lpDrvData);
    lpImageInfo->lpDrvData = 0;

    /* (WIN32) release DIBSECTION bitmap info */
    if (lpImageInfo->lpdsBmi)
    {
	if (lpImageInfo->ds.dshSection)
	{
	}
	else
	{
		WinFree(lpImageInfo->ds.dsBm.bmBits);
	}
	WinFree((LPSTR) lpImageInfo->lpdsBmi);
    }

    RELEASEBITMAPINFO(lpImageInfo);
    return TRUE;
}

BOOL
GdiDeleteBrush(HBRUSH hBrush)
{
    LPBRUSHINFO lpBrushInfo;
    HBITMAP hBitmap;

    lpBrushInfo = GETBRUSHINFO(hBrush);
    if (!lpBrushInfo) {
	return FALSE;
    }
    hBitmap = (HBITMAP)lpBrushInfo->lpBrush.lbHatch;

    switch (lpBrushInfo->lpBrush.lbStyle) {
	case BS_SOLID:
	    if (LOWORD(lpBrushInfo->dwFlags) == BFP_PIXMAP) {
		UNLOCKGDI(hBitmap);
		DeleteObject(hBitmap);
	    }
	    break;
	case BS_PATTERN:
	case BS_DIBPATTERN:
	case BS_DIBPATTERNPT:
	    UNLOCKGDI(hBitmap);
	    break;
	default:
	    break;
    }
    if (LOWORD(lpBrushInfo->dwFlags) == BFP_PIXMAP ||
		LOWORD(lpBrushInfo->dwFlags) == BFP_BITMAP)
	DRVCALL_IMAGES(PIH_DESTROYBRUSH,0L,0L,lpBrushInfo->lpPrivate);

    RELEASEBRUSHINFO(lpBrushInfo);
    return TRUE;
}

HBITMAP
GdiDitherColor(HDC hDC, COLORREF cr, int nBitsPerPixel)
{
    char DIBBuffer[sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*16 + 4];
    UINT DitherPattern[16];
    LPBITMAPINFO lpbmi;
    HBITMAP hBitmap;
    DWORD dwRGB = 0;
    LPPALETTEENTRY lprgb;

    lpbmi = (LPBITMAPINFO)DIBBuffer;
    lpbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbmi->bmiHeader.biWidth = lpbmi->bmiHeader.biHeight = 8;
    lpbmi->bmiHeader.biPlanes = 1;
    lpbmi->bmiHeader.biBitCount = (nBitsPerPixel == 1)?1:4;
    lpbmi->bmiHeader.biCompression = BI_RGB;
    lpbmi->bmiHeader.biSizeImage = 0;
    lpbmi->bmiHeader.biXPelsPerMeter = lpbmi->bmiHeader.biYPelsPerMeter =
		lpbmi->bmiHeader.biClrUsed =
		lpbmi->bmiHeader.biClrImportant = 0;
    if (nBitsPerPixel == 1) {
	lpbmi->bmiColors[0].rgbRed = lpbmi->bmiColors[0].rgbGreen =
			lpbmi->bmiColors[0].rgbBlue = 0;
	lpbmi->bmiColors[1].rgbRed = lpbmi->bmiColors[1].rgbGreen =
			lpbmi->bmiColors[1].rgbBlue = 255;
    }
    else
	memcpy((LPSTR)lpbmi->bmiColors,(LPSTR)VGAColors,sizeof(RGBQUAD)*16);
    lprgb = (LPPALETTEENTRY)&dwRGB;
    lprgb->peRed = GetRValue(cr);
    lprgb->peGreen = GetGValue(cr);
    lprgb->peBlue = GetBValue(cr);
    InternalDitherColor((nBitsPerPixel == 1)?DM_MONOCHROME:DM_DEFAULT,
			dwRGB,DitherPattern);
    if (!(hBitmap = CreateDIBitmap(hDC,&lpbmi->bmiHeader,CBM_INIT,
		(const void *)DitherPattern,lpbmi,DIB_RGB_COLORS))) {
	return 0;
    }
    return hBitmap;
}

static int
MapColorIndex(BYTE pixel, RGBQUAD *rgb, LPBYTE lpCache, int nNumColors)
{
    COLORREF cr;
    unsigned int r,g,b,r1,g1,b1;
    unsigned int best = (unsigned int)-1;
    unsigned int i,index=0,delta;

    if (lpCache[pixel] != (BYTE)-1)
	return lpCache[pixel];

    cr = (COLORREF)DRVCALL_COLORS(PLH_GETCOLORREF,pixel,0L,0L);
    r = GetRValue(cr);
    g = GetGValue(cr);
    b = GetBValue(cr);
    for (i=0; i<nNumColors; i++) {
	r1 = rgb[i].rgbRed;
	g1 = rgb[i].rgbGreen;
	b1 = rgb[i].rgbBlue;
	delta = (r-r1)*(r-r1) + (g-g1)*(g-g1) + (b-b1)*(b-b1);
	if (delta < best) {
	    best = delta;
	    index = i;
	}
    }
    lpCache[pixel] = index;
    return index;
}

static void
FillRGBTable(HDC hDC, RGBQUAD *lprgb, int nNumColors)
{
    switch(nNumColors) {
	case 2:
	    lprgb[0].rgbRed = lprgb[0].rgbGreen = lprgb[0].rgbBlue = 0;
	    lprgb[1].rgbRed = lprgb[1].rgbGreen = lprgb[1].rgbBlue = 0xff;
	    break;
	case 16:
	    memcpy((LPSTR)lprgb,(LPSTR)&VGAColors[0],16*sizeof(RGBQUAD));
	    break;
	case 256:
	    DRVCALL_COLORS(PLH_FILLRGBTABLE,(DWORD)nNumColors,0L,
				(LPVOID)lprgb);
	    break;
	default:
	    break;
    }
}
