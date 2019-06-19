/********************************************************************
*
*	@(#)DrvImages.c	2.47 Image platform specific subsytem implementation.
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

********************************************************************/
 

#include "windows.h"
#include "Driver.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "ObjEngine.h"
#include "Log.h"
#include "GdiDC.h"
#include "GdiObjects.h"

#include "DrvCommon.h"
#include "DrvDP.h"
#include "DrvDC.h"
#include "DrvImages.h"
#include "DrvColors.h"

#include <stdio.h>
#include <string.h>

#define TWIN_PALETTE

DWORD DrvImagesInit(LPARAM, LPARAM, LPVOID);
DWORD DrvImagesTab(LPARAM,LPARAM,LPVOID);
static DWORD DrvInitGraphics(void);
static DWORD DrvCreateImage(LPARAM, LPARAM, LPVOID);
DWORD DrvDestroyImage(LPARAM, LPARAM, LPVOID);
static DWORD DrvUpdatePixmap(LPARAM, LPARAM, LPVOID);
static DWORD DrvGetImageCaps(LPARAM, LPARAM, LPVOID);
static DWORD DrvGetBitmapData(LPARAM, LPARAM, LPVOID);
static DWORD DrvFetchImageBits(LPARAM, LPARAM, LPVOID);
static DWORD DrvCreateBrush(LPARAM, LPARAM, LPVOID);
static DWORD DrvDestroyBrush(LPARAM, LPARAM, LPVOID);
static DWORD DrvCreateCursor(LPARAM, LPARAM, LPVOID);
static DWORD DrvDestroyCursor(LPARAM, LPARAM, LPVOID);
static DWORD DrvImagesDoNothing(LPARAM, LPARAM, LPVOID);

BOOL ConvertColorDDBToMono(LPDRVIMAGEDATA,LPRECT,DWORD);
void DrvCopyPixmap(LPDRVIMAGEDATA);
LPBYTE DrvFillImageInfo(LPDRVIMAGEDATA);

extern void DrvGetXColor(COLORREF, XColor *);

static TWINDRVSUBPROC DrvImagesEntryTab[] = {
	DrvImagesInit,
	DrvImagesDoNothing,
	DrvImagesDoNothing,
	DrvCreateImage,
	DrvDestroyImage,
	DrvUpdatePixmap,
	DrvGetImageCaps,
	DrvGetBitmapData,
	DrvFetchImageBits,
	DrvCreateBrush,
	DrvDestroyBrush,
	DrvCreateCursor,
	DrvDestroyCursor
};

#ifdef	DEBUG
void TWIN_DumpDC(HDC);
void TWIN_DumpXImage(XImage *,int,int,int,int);
void TWIN_DumpDrawable(Pixmap,int,int,int,int);
#endif

static WORD byte_order_test = (LSBFirst | (MSBFirst << 8));
#define TEST_BYTE_ORDER ((int) *((char *) &byte_order_test))

static DWORD 
DrvCreateImage(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    CREATEIMAGEDATA *lpcreate = (CREATEIMAGEDATA *)lpStruct;
    BOOL fMono = (BOOL)dwParam1; 
    DRVIMAGEDATA *lpid;
    XImage *image;
    PRIVATEDISPLAY *dp;

    dp = (PRIVATEDISPLAY *)GETDP();

    if (fMono) 
	image = (LPVOID)XCreateImage(dp->display,
			DefaultVisual(dp->display,dp->screen),
			1, XYBitmap,0,lpcreate->lpData,
			lpcreate->nWidth,lpcreate->nHeight,
			lpcreate->nLinePad,lpcreate->nWidthBytes);
    else  {
	if (lpcreate->nBitsPixel != dp->Depth) {
	    if (lpcreate->nBitsPixel == 1) /* depth 1 is always supported */
		image = (LPVOID)XCreateImage(dp->display,
			DefaultVisual(dp->display,dp->screen),
			1, XYBitmap,0,lpcreate->lpData,
			lpcreate->nWidth,lpcreate->nHeight,
			lpcreate->nLinePad,lpcreate->nWidthBytes);
	    else {
		/* We have to add support for this case */
		ERRSTR((LF_ERROR,"DrvCreateImage: wrong data format!\n"));
		return (DWORD)FALSE;
	    }
	}
	else {
	    image = (LPVOID)XCreateImage(dp->display,
			DefaultVisual(dp->display,dp->screen),
			dp->Depth,
			ZPixmap,0,lpcreate->lpData,
			lpcreate->nWidth,lpcreate->nHeight,
			lpcreate->nLinePad,lpcreate->nWidthBytes);
	    image->byte_order = TEST_BYTE_ORDER;
	}
    }
    if (image == 0) 
	return (DWORD)NULL;

    lpid = (DRVIMAGEDATA *)DRVMEM_MALLOC(sizeof(DRVIMAGEDATA));
    lpid->nWidth = lpcreate->nWidth;
    lpid->nHeight = lpcreate->nHeight;
    lpid->nLinePad = lpcreate->nLinePad;
    lpid->nBitsPixel = lpcreate->nBitsPixel;
    lpid->nWidthBytes = lpcreate->nWidthBytes;
    lpid->fMono = fMono;
    lpid->fInvalid = FALSE;
    lpid->image = image;
    lpid->pixmap = 0;
    return (DWORD)lpid;
}

DWORD
DrvDestroyImage(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    BOOL ynFree = TRUE;
    LPDRVIMAGEDATA lpid = (LPDRVIMAGEDATA)lpStruct;
    PRIVATEDISPLAY *dp;

    if (!lpid)
	return(0);
    if (lpid->pixmap) {
	dp = (PRIVATEDISPLAY *)GETDP();
	XFreePixmap(dp->display,lpid->pixmap);
    }
    if (lpid->image)
        {
        if(lpid->image->data)
		WinFree( lpid->image->data );
        lpid->image->data = 0;
	XDestroyImage(lpid->image);
        }
    /*
    **   Clipboard may ask not to free the memory as it does that itself.
    */
    if (ynFree)
       DRVMEM_FREE((LPSTR)lpid);

    return 1L;	
}

static DWORD 
DrvUpdatePixmap(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    LPDRVIMAGEDATA lpid = (LPDRVIMAGEDATA)lpStruct;
    PRIVATEDISPLAY *dp = (PRIVATEDISPLAY *)GETDP();
    GC gc;
    XGCValues values;

    if (lpid->pixmap) {
	if (lpid->fMono) {
#ifdef TWIN_PALETTE
	    values.foreground = DrvMakePixel(RGB(255,255,255), NULL);
	    values.background = DrvMakePixel(RGB(0,0,0), NULL);
#else
	    values.foreground = DrvMakePixel(RGB(255,255,255));
	    values.background = DrvMakePixel(RGB(0,0,0));
#endif
	    gc = XCreateGC(dp->display,
		DefaultRootWindow(dp->display),
		GCForeground|GCBackground,&values);
	}
	else
	    gc =  XCreateGC(dp->display,
		DefaultRootWindow(dp->display),
		0,(XGCValues *)0);
	XPutImage(dp->display,lpid->pixmap,gc,
		lpid->image,0,0,0,0,
		lpid->nWidth,lpid->nHeight);
	XFreeGC(dp->display,gc);
    }
    return 1L;
}

static DWORD 
DrvGetImageCaps(LPARAM dwParam1, LPARAM dwParam, LPVOID lpStruct)
{
    int nFunction = (int)dwParam1;
    PRIVATEDISPLAY    *dp;
    int nBitsPixel,nImageBitsPixel;
    DRVIMAGEDATA *lpimage;

    dp = (PRIVATEDISPLAY *)GETDP();

    switch (nFunction) {
	case DRV_QUERYDEPTH:

	/* Ask driver if it can convert given image (passed in lpStruct)*/
	/* to given depth (passed in dwParam); This X driver supports  */
	/* only mono to mono (1 bit deep) and default depth;  */
	/* Return value is the supported depth (may be different from	*/
	/* the one asked for);  passing 0 as format queries default	*/
	/* value of bits per pixel	*/

	    if ((lpimage = (DRVIMAGEDATA *)lpStruct))
		nImageBitsPixel = lpimage->nBitsPixel;
	    else
		nImageBitsPixel = 0;
	    nBitsPixel = (int)dwParam;

	    if ((nBitsPixel == 1) && (nImageBitsPixel == 1))
		return (DWORD)1;
	    else
		return (DWORD)dp->Depth;
	    break;

	case DRV_QUERYBITSPIXEL:
	    return (DWORD)dp->BitsPixel;

	case DRV_BITORDER:
	    return (DWORD)(dp->BitOrder == LSBFirst)?LSBFIRST:MSBFIRST;

	case DRV_BYTEORDER:
#if 1
	    return (DWORD)TEST_BYTE_ORDER;
#else
	    return (DWORD)(dp->ByteOrder == LSBFirst)?LSBFIRST:MSBFIRST;
#endif

	case DRV_QUERYREDMASK:
	    if ((lpimage = (DRVIMAGEDATA *)lpStruct))
		return (DWORD)lpimage->image->red_mask;
	    return (DWORD)0;

	case DRV_QUERYGREENMASK:
	    if ((lpimage = (DRVIMAGEDATA *)lpStruct))
		return (DWORD)lpimage->image->green_mask;
	    return (DWORD)0;

	case DRV_QUERYBLUEMASK:
	    if ((lpimage = (DRVIMAGEDATA *)lpStruct))
		return (DWORD)lpimage->image->blue_mask;
	    return (DWORD)0;

	default:
	    return (DWORD)0;
    }
}

static DWORD
DrvGetBitmapData(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct )
{
    BOOL fUpdate = (BOOL)dwParam1;
    LPDRVIMAGEDATA lpid = (LPDRVIMAGEDATA)lpStruct;
    PRIVATEDISPLAY *dp = (PRIVATEDISPLAY *)GETDP();

    if (fUpdate && lpid->pixmap) {
	/* if bitmap image has a drawable, destroy image */
	/* if it exists and create a fresh one in the same */
	/* format */

	if (lpid->image) {
	    if (lpid->image->data) {
		DRVMEM_FREE(lpid->image->data);
		lpid->image->data = (LPSTR)0;
	    }
	    XDestroyImage(lpid->image);
	}
	lpid->image = XGetImage(dp->display,
			lpid->pixmap,
			0,0,
			lpid->nWidth,lpid->nHeight,
			(lpid->nBitsPixel == 1) ? 1 : (DWORD)-1,
			(lpid->nBitsPixel == 1) ? XYPixmap : ZPixmap);	    
    }
    if (!lpid->image)
	return (DWORD)NULL;

    return (DWORD)lpid->image->data;
}

static DWORD 
DrvFetchImageBits(LPARAM dwParam1,LPARAM dwParam2, LPVOID lpStruct)
{
    LPSTR lpData = (LPSTR)dwParam1;
    int nBitCount = (int)dwParam2;
    LPDRVIMAGEDATA lpid = (LPDRVIMAGEDATA)lpStruct;
    PRIVATEDISPLAY    *dp;
    XImage    *im = 0;
    GC gc;
    Pixmap tmpPix = 0;
    int nWidthBytes;
    BOOL fNeedToCleanup = FALSE;

    dp = (PRIVATEDISPLAY *)GETDP();

    if (!lpid->pixmap) {
	if (!(im = (XImage *)lpid->image)) 
	    return (DWORD)0;
    }

    gc =  XCreateGC(dp->display,
		DefaultRootWindow(dp->display),
		0,(XGCValues *)0);

    /* Her we assume that server supports given nBitCount; */
    /* Everybody should call DRV_QUERYFORMAT before calling this function; */
    /* This way we make sure the buffer for the bits is big enough without */
    /* adding extra parameter of buffer size */

    /* if we need to get image or we need to convert */
    if (!im || (im->bits_per_pixel != nBitCount)) {
	if (lpid->pixmap == 0) {
	    /* if there is no drawable, create default and put the */
	    /* image we have into it*/
	    tmpPix = XCreatePixmap(dp->display,
				DefaultRootWindow(dp->display),
				lpid->nWidth,lpid->nHeight,
				dp->Depth);
	    if (tmpPix == 0) {
		XFreeGC(dp->display,gc);
		return (DWORD)0;
	    }
	    XSetForeground(dp->display,gc,0);
	    XSetBackground(dp->display,gc,0xff);
	    XPutImage(dp->display,(Drawable)tmpPix,
				gc,(XImage *)lpid->image,
				0,0,0,0,
				lpid->nWidth,lpid->nHeight);
	}
	if (nBitCount == 1)
	    im = XGetImage(dp->display,
			(Drawable)((tmpPix)?tmpPix:lpid->pixmap),
			0,0,
			lpid->nWidth,lpid->nHeight,
			(DWORD)1,
			XYPixmap);
	else
	    im = XGetImage(dp->display,
			(Drawable)((tmpPix)?tmpPix:lpid->pixmap),
			0,0,
			lpid->nWidth,lpid->nHeight,
			(DWORD)-1,ZPixmap);
	if (tmpPix)	    
	    XFreePixmap(dp->display,tmpPix);
	if (im == 0)
	    return (DWORD)FALSE;
	fNeedToCleanup = TRUE;
    }
    nWidthBytes = im->bytes_per_line;
    memcpy(lpData,im->data,nWidthBytes*im->height);

    if (fNeedToCleanup)
	XDestroyImage(im);
    XFreeGC(dp->display,gc);

    return (DWORD)nWidthBytes;
}

static DWORD
DrvCreateBrush( LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    DWORD dwStyle = (DWORD)dwParam1;
    LPDRVIMAGEDATA lpImageData = (LPDRVIMAGEDATA)lpStruct;
    PRIVATEDISPLAY    *dp;
    Pixmap pix;
    XGCValues values;
    GC	gc;
    LPSTR lpData;
    XImage *im;
    char data[8];
    LPSTR ptr;
    int i;

    if (!lpImageData)
	return (DWORD)NULL;

    dp = (PRIVATEDISPLAY *)GETDP();

    if (dwStyle == BFP_PIXMAP) { /* tile -- pixmap of default screen depth */
	pix = XCreatePixmap(dp->display,
		DefaultRootWindow(dp->display),
		8,8,dp->Depth);
	if (lpImageData->fMono) {
#ifdef TWIN_PALETTE
	    values.background = DrvMakePixel(RGB(0,0,0), NULL);
	    values.foreground = DrvMakePixel(RGB(255,255,255), NULL);
#else
	    values.background = DrvMakePixel(RGB(0,0,0));
	    values.foreground = DrvMakePixel(RGB(255,255,255));
#endif
	    gc = XCreateGC(dp->display,DefaultRootWindow(dp->display),
			GCForeground|GCBackground,&values);
	}
	else
	    gc = XCreateGC(dp->display,
			DefaultRootWindow(dp->display),
			0, (XGCValues *) 0);

	if (lpImageData->pixmap && !lpImageData->fMono)
	    XCopyArea(dp->display,lpImageData->pixmap,pix,gc,0,0,8,8,0,0);
	else
	    XPutImage(dp->display,pix,gc,lpImageData->image,0,0,0,0,8,8);

	XFreeGC(dp->display,gc);
    }
    else {   /* BFP_BITMAP -- for hatched brushes create stipple of depth 1 */
	im = lpImageData->image;
	if (im->bitmap_pad == 8) 
	    lpData = im->data;
	else {
	    ptr = im->data;
	    for (i=0; i<8; i++) {
		data[i] = ~(*ptr);
		ptr += im->bitmap_pad/8;
	    }
	    lpData = data;
	}
	pix = XCreateBitmapFromData(dp->display,
		DefaultRootWindow(dp->display),
		lpData,8,8);
    }

    return (DWORD)pix;
}

static DWORD
DrvDestroyBrush(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct) 
{
    Pixmap pix = (Pixmap)dwParam1;
    PRIVATEDISPLAY    *dp;

    if (!pix) 
	return 0L;

    dp = (PRIVATEDISPLAY *)GETDP();
    XFreePixmap(dp->display,pix);
    return 1L;
}

static DWORD
DrvCreateCursor(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    CREATECURSOR *lpcc = (CREATECURSOR *)lpStruct;
    Pixmap pixXOR, pixAND;
    GC  gc;
    XGCValues    Xgcval;
    PRIVATEDISPLAY    *dp;
    LPDRVIMAGEDATA lpXORData, lpANDData;
    XImage source_image, mask_image;
    int nWidth,nHeight;
    Cursor cur;
    XColor x1, x2;
    LPBYTE lpbp;
    LPBYTE lpAND, lpXOR;
    int nbytes;
    int byteidx;

    dp = GETDP();

    lpXORData = (LPDRVIMAGEDATA)lpcc->lpXORData;
    lpANDData = (LPDRVIMAGEDATA)lpcc->lpANDData;

    if (((nWidth = lpXORData->nWidth) != lpANDData->nWidth) ||
	((nHeight = lpXORData->nHeight) != lpANDData->nHeight))
	return 0L;

    /* ISSUE: cursor size should be supported by the server */

    if (lpXORData->nBitsPixel != 1) {
#ifdef LATER
	/* ??? -- can it ever happen???			 */
	/* If so, color image has to be converted to monochrome */
#endif
	return 0L;
    }

    /* 
     * X cursors don't work like MS Windows cursors.  Windows uses an AND and
     * an XOR mask.  A 1 in the AND mask says start from the screen pixel
     * color, and a 0 says start from the cursor background color.  A 1 in
     * the XOR mask says invert the starting color.
     *
     * X uses a source image and a mask image.  A 1 in the mask image says
     * use the pixel on the screen, and a 0 says use the pixel in the source
     * image.
     *
     * We use the XOR image bitwise-xor the AND image as the source image.  
     * The mask image will be the inverse of XOR image bitwise-and the 
     * AND image.  This isn't the same behavior as Windows, but it is a 
     * reasonable work-around.
     */
    if (nWidth % 8)
	return 0L;
	
    nbytes = (nWidth / 8) * nHeight;
    /* Create the mask image */
    lpAND = (LPBYTE) lpANDData->image->data;
    lpXOR = (LPBYTE) lpXORData->image->data;
    mask_image = *lpANDData->image;
    mask_image.data = (char *) WinMalloc(nbytes);
    lpbp = (LPBYTE) mask_image.data;
    for (byteidx = 0; byteidx < nbytes; byteidx++)
	*lpbp++ = ((*lpAND++) & ~(*lpXOR++));

    /* Create the source image */
    lpAND = (LPBYTE) lpANDData->image->data;
    lpXOR = (LPBYTE) lpXORData->image->data;
    source_image = *lpXORData->image;
    source_image.data = (char *) WinMalloc(nbytes);
    lpbp = (LPBYTE) source_image.data;
    for (byteidx = 0; byteidx < nbytes; byteidx++)
	*lpbp++ = ((*lpAND++) ^ (*lpXOR++));

    /* Create pixmap of depth 1 for XOR image */
    pixXOR = XCreatePixmap(dp->display,
			DefaultRootWindow(dp->display),
			nWidth,nHeight,1);

    /* Create pixmap of depth 1 for AND image */
    pixAND = XCreatePixmap(dp->display,
			DefaultRootWindow(dp->display),
			nWidth,nHeight,1);

    Xgcval.foreground = (unsigned long)0;
    Xgcval.background = (unsigned long)-1;
    gc = XCreateGC(dp->display, pixXOR,
		GCForeground | GCBackground, (XGCValues *)&Xgcval);
    XPutImage(dp->display, pixAND, gc, &mask_image,
		0,0,0,0,nWidth,nHeight);
    XPutImage(dp->display, pixXOR, gc, &source_image,
		0,0,0,0,nWidth,nHeight);

    WinFree((char *) mask_image.data);
    WinFree((char *) source_image.data);

    DrvGetXColor(lpcc->crFore,&x1);
    DrvGetXColor(lpcc->crBack,&x2);
    cur = XCreatePixmapCursor(dp->display,pixXOR,pixAND,
		&x1,&x2,lpcc->ptHot.x,lpcc->ptHot.y);

    XFreePixmap(dp->display,pixXOR);
    XFreePixmap(dp->display,pixAND);
    XFreeGC(dp->display,gc);

    return (DWORD)cur;
}

static DWORD
DrvDestroyCursor(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
  return(TRUE);
}

BOOL
ConvertColorDDBToMono(LPDRVIMAGEDATA lpid, LPRECT lprc, DWORD bg)
{
    PRIVATEDISPLAY    *dp;
    XImage *imSrc, imDest;
    GC gc;
    DWORD dwSize;
    DWORD dwPixels;
    LPBYTE lpSrc, lpDest, src, dest;
    LPWORD srcW, destW;
    BYTE r, g, b, r_bg, g_bg, b_bg;
    DWORD r_mask, g_mask, b_mask;
    int i, x, y, w, h;
    DWORD blackpixel,whitepixel;

    dp = (PRIVATEDISPLAY *)GETDP();
    gc = XCreateGC(dp->display,
		DefaultRootWindow(dp->display),
		0,(XGCValues *)0);
    imSrc = XGetImage(dp->display,(Drawable)lpid->pixmap,
		lprc->left,lprc->top,
		lprc->right,lprc->bottom,
		(DWORD)-1,ZPixmap);
    if (!imSrc)
	return FALSE;
    dwSize = imSrc->bytes_per_line * imSrc->height;
    dwPixels = imSrc->width * imSrc->height;
    lpSrc = (LPBYTE)imSrc->data;
    lpDest = (LPBYTE)DRVMEM_MALLOC(dwSize);
#ifdef TWIN_PALETTE
    blackpixel = DrvMakePixel(RGB(0,0,0), NULL);
    whitepixel = DrvMakePixel(RGB(255,255,255), NULL);
#else
    blackpixel = DrvMakePixel(RGB(0,0,0));
    whitepixel = DrvMakePixel(RGB(255,255,255));
#endif
    imDest = *imSrc;
    imDest.data = (LPSTR)lpDest;

    /*
     * Added code for 15, 16 bpp, HOWEVER, there are several cases which
     * are not handled properly. What if bitmap_pad requires us to skip
     * at the end of each line to get to the beginning of the next? Or,
     * what if we have a non-zero xoffset? These issues must be addressed
     * when 24-bit and other pixel depths are supported. LJW 1/15/96
     * Added code for 32 bpp, but did not address above. JJF 2/20/96 
     */
    switch (imSrc->bits_per_pixel) {
        case 32:
            {
            LPDWORD lpSrcW = (LPDWORD)lpSrc; 
            LPDWORD lpDestW = (LPDWORD)lpDest; 
	    for (i=0; i<dwPixels; i++)
		*lpDestW++ = (DWORD)((*lpSrcW++ == (DWORD)bg) ?
                                            whitepixel : blackpixel);
            }
	    break;
	case 15:
	case 16:
#if 1
		r_mask = lpid->image->red_mask;
		g_mask = lpid->image->green_mask;
		b_mask = lpid->image->blue_mask;

		r_bg = 255 * (bg & r_mask) / r_mask;
		g_bg = 255 * (bg & g_mask) / g_mask;
		b_bg = 255 * (bg & b_mask) / b_mask;

		w = lprc->right - lprc->left;
		h = lprc->bottom - lprc->top;
		for (y = 0; y < h; y++)
		{
			srcW = ((LPWORD) lpSrc) + imSrc->xoffset;
			destW = ((LPWORD) lpDest) + imDest.xoffset;
			for (x = 0; x < w; x++)
			{
				WORD word = *srcW++;
				r = 255 * (word & r_mask) / r_mask;
				g = 255 * (word & g_mask) / g_mask;
				b = 255 * (word & b_mask) / b_mask;
				if ((r == r_bg) && (g == g_bg) && (b == b_bg))
				{
					*destW++ = (WORD) whitepixel;
				}
				else
				{
					*destW++ = (WORD) blackpixel;
				}
			}
			lpSrc += imSrc->bytes_per_line;
			lpDest += imDest.bytes_per_line;
		}
		break;
#else
            {
		LPWORD lpSrcW = (LPWORD)lpSrc; 
		LPWORD lpDestW = (LPWORD)lpDest; 
		for (i=0; i<dwPixels; i++)
		{
			*lpDestW++ = (WORD) ((*lpSrcW++ == (WORD)bg)
				? whitepixel : blackpixel);
		}
            }
	    break;
#endif
	case 8:
	    for (i=0; i<dwPixels; i++)
		*lpDest++ = (BYTE)((*lpSrc++ == (BYTE)bg) ?
                                            whitepixel : blackpixel);
	    break;
	case 24:
		r_mask = lpid->image->red_mask;
		g_mask = lpid->image->green_mask;
		b_mask = lpid->image->blue_mask;

		r_bg = 255 * (bg & r_mask) / r_mask;
		g_bg = 255 * (bg & g_mask) / g_mask;
		b_bg = 255 * (bg & b_mask) / b_mask;

		w = lprc->right - lprc->left;
		h = lprc->bottom - lprc->top;
		for (y = 0; y < h; y++)
		{
			src = lpSrc + (imSrc->xoffset * 3);
			dest = lpDest + (imDest.xoffset * 3);
			for (x = 0; x < w; x++, src += 3, dest += 3)
			{
				r = src[0];
				g = src[1];
				b = src[2];
				if ((r == r_bg) && (g == g_bg) && (b == b_bg))
				{
					dest[0] = (whitepixel & 0x000000ffL);
					dest[1] = (whitepixel & 0x0000ff00L)
						>> 8;
					dest[2] = (whitepixel & 0x00ff0000L)
						>> 16;
				}
				else
				{
					dest[0] = (blackpixel & 0x000000ffL);
					dest[1] = (blackpixel & 0x0000ff00L)
						>> 8;
					dest[2] = (blackpixel & 0x00ff0000L)
						>> 16;
				}
			}
			lpSrc += imSrc->bytes_per_line;
			lpDest += imDest.bytes_per_line;
		}
		break;
	case 4:
	case 1:
	default:
	    break;
    }
    XPutImage(dp->display, (Drawable)lpid->pixmap, gc, &imDest,
		0,0,
		lprc->left,lprc->top,
		lprc->right,lprc->bottom);
    XDestroyImage(imSrc);
    DRVMEM_FREE((LPSTR)imDest.data);
    XFreeGC(dp->display,gc);
    return TRUE;
}

#ifdef DEBUG	/* [ */
void
TWIN_DumpDC(HDC hDC)
{
	HDC32		hDC32;
	GC		gc;
	XGCValues	values;
	PRIVATEDISPLAY  *dp;
	LPDRIVERDC	lpddc;
	int		nWidth, nHeight;
	XImage		*im;
	
	hDC32 = GETDCINFO(hDC);
	if ((lpddc = hDC32->lpDrvData) == (LPVOID)0) {
	    return;
	}
	if (lpddc->lpDrvImage == NULL) {
	    return;
	}
	nWidth = lpddc->lpDrvImage->nWidth;
	nHeight = lpddc->lpDrvImage->nHeight;

	dp = (PRIVATEDISPLAY *)GETDP();
	values.cap_style = 0;
	gc = XCreateGC(dp->display, RootWindow(dp->display,dp->screen),
		0, &values);

#ifdef LATER
	XCopyArea(dp->display,
		lpddc->Drawable,
		RootWindow(dp->display,dp->screen),
		gc,0,0,
		nWidth,nHeight,
		XDisplayWidth(dp->display,dp->screen) - nWidth,
		XDisplayHeight(dp->display,dp->screen) - nHeight);
#else
	im = XGetImage(dp->display,
			lpddc->Drawable,
			0,0,
			nWidth,nHeight,
			(DWORD)0xffffffff,ZPixmap);
	XPutImage(dp->display,
		(Drawable)RootWindow(dp->display,dp->screen),
		gc,
		im,
		0,0,
		XDisplayWidth(dp->display,dp->screen) - nWidth,
		XDisplayHeight(dp->display,dp->screen) - nHeight,
		nWidth,nHeight);
	XDestroyImage(im);
#endif

	XFreeGC(dp->display, gc);
}
#endif

#ifdef DEBUG
void
TWIN_DumpXImage(XImage *im,int x,int y,int Width, int Height)
{
	GC 			 gc;
	PRIVATEDISPLAY    	*dp;
	XGCValues 		 values;

	LOGSTR((LF_DEBUG, "DumpXImage: image %p x %d y %d w %d h %d\n",
		im, x, y, Width, Height));

        dp = (PRIVATEDISPLAY *)GETDP();

	gc = XCreateGC(dp->display, RootWindow(dp->display,dp->screen), 
		0, &values);

	XPutImage(dp->display, RootWindow(dp->display,dp->screen),gc,im,
		x,y,
		0,XDisplayHeight(dp->display,dp->screen)-Height,
		Width,Height);

	XFreeGC(dp->display, gc);
}
#endif

#ifdef DEBUG
void
TWIN_DumpDrawable(Pixmap d, int x, int y, int Width, int Height)
{
	GC 			 gc;
	PRIVATEDISPLAY    	*dp;
	XImage *im;

        dp = (PRIVATEDISPLAY *)GETDP();

	gc = XCreateGC(dp->display, RootWindow(dp->display,dp->screen), 
		0, (XGCValues *)0);

	im = XGetImage(dp->display,
			d,
			0,0,
			Width,Height,
			(DWORD)0xffffffff,ZPixmap);
	XPutImage(dp->display,
		(Drawable)RootWindow(dp->display,dp->screen),
		gc,
		im,
		0,0,
		XDisplayWidth(dp->display,dp->screen) - Width,
		XDisplayHeight(dp->display,dp->screen) - Height,
		Width,Height);
	XDestroyImage(im);
	XFreeGC(dp->display, gc);
}
#endif		/* DEBUG ] */

static DWORD
DrvInitGraphics(void)
{
    PRIVATEDISPLAY    *dp;
    XImage *image;

    dp = (PRIVATEDISPLAY *)GETDP();

    dp->Depth = DefaultDepth(dp->display,dp->screen);
    image = XCreateImage(dp->display,
		DefaultVisual(dp->display,dp->screen),
		dp->Depth,ZPixmap,0,0,1,1,32,0);
    if (image) {
	dp->BitsPixel = image->bits_per_pixel;
	dp->BitOrder = image->bitmap_bit_order;
	dp->ByteOrder = image->byte_order;
	XDestroyImage(image);
    }
    return 1;
}

/* This function replaces the pixmap contained in DRVIMAGEDATA 	*/
/* structure with a copy so that the original one may later be	*/
/* discarded.  The original pixmap handle is lost!!!		*/

void 
DrvCopyPixmap(LPDRVIMAGEDATA lpid)
{
    Pixmap	tmpPix;
    BOOL	bRet = TRUE;
    GC		gc;

    PRIVATEDISPLAY *dp = (PRIVATEDISPLAY *)GETDP();

    tmpPix = XCreatePixmap(dp->display,
			DefaultRootWindow(dp->display),
			lpid->nWidth,lpid->nHeight,
			(lpid->fMono)?1:dp->Depth);
    gc =  XCreateGC(dp->display,
		    DefaultRootWindow(dp->display),
		    0,(XGCValues *)0);

    if (lpid->pixmap == 0) {
	if (lpid->image)
	    XPutImage(dp->display,(Drawable)tmpPix,
		gc,(XImage *)lpid->image,
		0,0,0,0,
		lpid->nWidth,lpid->nHeight);
	else
	    bRet = FALSE;

    }
    else {
	XImage *im;

	im = XGetImage(dp->display,
			lpid->pixmap,
			0,0,
			lpid->nWidth,lpid->nHeight,
			(DWORD)0xffffffff,ZPixmap);
	XPutImage(dp->display,tmpPix,gc,im,
		0,0,0,0,
		lpid->nWidth,lpid->nHeight);
	XDestroyImage(im);
    }

    lpid->pixmap = (bRet)?tmpPix:0;
    lpid->image = NULL;
	
    XFreeGC(dp->display,gc);
}

/* DrvFillImageInfo function allocates an IMAGEINFO structure	*/
/* and fills it in based on the data contained in 	*/
/* DRVIMAGEDATA struct.	*/

LPBYTE
DrvFillImageInfo(LPDRVIMAGEDATA lpid)
{
    PRIVATEDISPLAY *dp = (PRIVATEDISPLAY *)GETDP();
    LPIMAGEINFO lpImageInfo;

    if (lpid->pixmap == 0)
	return NULL;

    lpImageInfo = (LPIMAGEINFO)WinMalloc(sizeof(IMAGEINFO));
    lpImageInfo->ImageWidth = lpid->nWidth;
    lpImageInfo->ImageHeight = lpid->nHeight;
    lpImageInfo->WidthBytes = lpid->nWidthBytes;
    lpImageInfo->LinePad = lpid->nLinePad;
    lpImageInfo->ImagePlanes = 1;
    lpImageInfo->ImageDepth = (lpid->fMono)?1:lpid->nBitsPixel;
    lpImageInfo->BitsPixel = lpid->nBitsPixel;
    lpImageInfo->fMono = lpid->fMono;
    lpImageInfo->rcsinfo = 0;
    lpImageInfo->dimension = 0;
    lpImageInfo->lpDrvData = (LPVOID)lpid;
    lpImageInfo->lpdsBmi = NULL;
    lpid->image = XGetImage(dp->display,
			lpid->pixmap,
			0,0,
			lpid->nWidth,lpid->nHeight,
			(DWORD)0xffffffff,ZPixmap);
    if (lpid->image == NULL)
       {
       WinFree((LPSTR)lpImageInfo);
       return NULL;
       }

    lpid->pixmap = 0;

    return (LPBYTE)lpImageInfo;
}

static DWORD
DrvImagesDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 1L;
}

DWORD
DrvImagesInit(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	if (dwParam2)
	{
		return (DWORD) DrvInitGraphics();
	}
	else
		return 0L;
}

DWORD
DrvImagesTab(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvImagesEntryTab[0] = DrvImagesInit;
        DrvImagesEntryTab[1] = DrvImagesDoNothing;
        DrvImagesEntryTab[2] = DrvImagesDoNothing;
        DrvImagesEntryTab[3] = DrvCreateImage;
        DrvImagesEntryTab[4] = DrvDestroyImage;
        DrvImagesEntryTab[5] = DrvUpdatePixmap;
        DrvImagesEntryTab[6] = DrvGetImageCaps;
        DrvImagesEntryTab[7] = DrvGetBitmapData;
        DrvImagesEntryTab[8] = DrvFetchImageBits;
        DrvImagesEntryTab[9] = DrvCreateBrush;
        DrvImagesEntryTab[10] = DrvDestroyBrush;
        DrvImagesEntryTab[11] = DrvCreateCursor;
        DrvImagesEntryTab[12] = DrvDestroyCursor;
#endif
	return (DWORD)DrvImagesEntryTab;
}
