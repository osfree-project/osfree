/*    
	WBmp.c	1.5
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

 * Common Controls Implementation source - helper file
 * Comments : The goal of the routines in this file is to be simple to read  
 *            and understand, rather than to be computationally efficient.   
 * Purpose  : Routines that read resources from files and return bitmap       
 *            handles reside here.                                            
 */

#include <string.h>
#include "windows.h"
#include "WBmp.h"
/*****************************************************************************
 * Function : W_IReadDibBitmapInfo()                                         *
 * Purpose  : Reads a file in DIB format and returns a global HANDLE to its  *
 *            BITMAPINFO.  This function will work with both                 *
 *            (BITMAPCOREHEADER, OS/2 style) and  (BITMAPINFOHEADER,         *
 *            Win 3.0 style) bitmap formats, but will always return a        *
 *            BITMAPINFO (Win 3.0 style).                                    *
 * Parameters : int fh - File handle to the DIB file.                        *
 * Returns :  A handle to BITMAPINFO in the DIB file. The handle will be NULL*
 *            if the BITMAPINFO cannot be created and filled for any reason. *
 * Comments :                                                                *
 *****************************************************************************/

HANDLE W_IReadDibBitmapInfo (int fh)
{
   DWORD off;
   HANDLE hbi = ( HANDLE )NULL;
   int size;
   int i;
   WORD nNumColors;

   RGBQUAD FAR *pRgb;
   BITMAPINFOHEADER bi;
   BITMAPCOREHEADER bc;
   LPBITMAPINFOHEADER lpbi;
   BITMAPFILEHEADER bf;
   DWORD dwWidth = 0;
   DWORD dwHeight = 0;
   WORD wPlanes, wBitCount;

   /* Reset file pointer and read file header*/

   off = _llseek(fh, 0L, SEEK_CUR);
   if (sizeof(bf) != _lread(fh, (LPSTR)&bf, sizeof(bf)))
   {
      return ( HANDLE )NULL;
   }

   /* Do we have a RC HEADER?*/
   if (!ISDIB (bf.bfType))
   {
      bf.bfOffBits = 0L;
      _llseek(fh, off, SEEK_SET);
   }
   if (sizeof(bi) != _lread(fh, (LPSTR)&bi, sizeof(bi)))
   {
      return ( HANDLE )NULL;
   }

   nNumColors = W_IDIBNumColors((LPSTR)&bi);

   /* Check the nature (BITMAPINFO or BITMAPCORE) of the info. block
      and extract the field information accordingly. If a BITMAPCOREHEADER,
      transfer it's field information to a BITMAPINFOHEADER-style block*/

   switch (size = (int)bi.biSize)
   {
      case sizeof(BITMAPINFOHEADER):
         break;

      case sizeof(BITMAPCOREHEADER):
         bc = *(BITMAPCOREHEADER *)&bi;

         dwWidth = (DWORD)bc.bcWidth;             /* width in pixels*/
         dwHeight = (DWORD)bc.bcHeight;           /* height in pixels*/
         wPlanes = bc.bcPlanes;                   /* # of planes. Always 1 for DIB*/
         wBitCount = bc.bcBitCount;               /* color bits per pixel.*/
         bi.biSize = sizeof(BITMAPINFOHEADER);    /* size of this structure*/
         bi.biWidth = dwWidth;                    
         bi.biHeight = dwHeight;
         bi.biPlanes = wPlanes;
         bi.biBitCount = wBitCount;
         bi.biCompression = BI_RGB;               /* no compression*/
         bi.biSizeImage = 0;                      /* 0 == default size*/
         bi.biXPelsPerMeter = 0;                  /* not used for this app*/
         bi.biYPelsPerMeter = 0;                  /* not used for this app*/
         bi.biClrUsed = nNumColors;               /* # of colors used.*/
         bi.biClrImportant = nNumColors;          /* # of important colors*/

         _llseek(fh, (LONG)sizeof(BITMAPCOREHEADER) - sizeof(BITMAPINFOHEADER), SEEK_CUR);
         break;

      default:
      {
         return ( HANDLE )NULL;
      }
   }

   /* Fill in some default values if they are zero*/
   if (bi.biSizeImage == 0)                       /* compute size of the image*/
      bi.biSizeImage = WIDTHBYTES ((DWORD)bi.biWidth * bi.biBitCount) * bi.biHeight;
   if (bi.biClrUsed == 0)                         /* compute the # of colors used*/
      bi.biClrUsed = W_IDIBNumColors((LPSTR)&bi);

   /* Allocate for the BITMAPINFO structure and the color table.*/
   hbi = GlobalAlloc(GHND, (LONG)bi.biSize + nNumColors * sizeof(RGBQUAD));
   if (hbi == ( HANDLE )NULL)
   {
      return ( HANDLE )NULL;
   }
   lpbi = (VOID FAR *)GlobalLock(hbi);
   *lpbi = bi;

   /* Get a pointer to the color table*/
   pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + bi.biSize);
   if (nNumColors)
   {
      if (size == sizeof(BITMAPCOREHEADER))
      {
      /* Convert a old color table (3 byte RGBTRIPLEs) to a new*/
      /* color table (4 byte RGBQUADs)*/
         _lread(fh, (LPSTR)pRgb, nNumColors * sizeof(RGBTRIPLE));

         for (i = nNumColors - 1; i >= 0; i--)
         {
            RGBQUAD rgb;

            rgb.rgbRed = ((RGBTRIPLE FAR *)pRgb)[i].rgbtRed;
            rgb.rgbBlue = ((RGBTRIPLE FAR *)pRgb)[i].rgbtBlue;
            rgb.rgbGreen = ((RGBTRIPLE FAR *)pRgb)[i].rgbtGreen;
            rgb.rgbReserved = (BYTE)0;

            pRgb[i] = rgb;
         }
      }
      else
         _lread(fh, (LPSTR)pRgb, nNumColors * sizeof(RGBQUAD));
   }

   if (bf.bfOffBits != 0L)
      _llseek(fh, off + bf.bfOffBits, SEEK_SET);

   GlobalUnlock(hbi);
   return hbi;
}

/*****************************************************************************
 * Function : W_IDIBToBitmap()                                               *
 * Purpose  : Creates a bitmap from a DIB.                                   *
 * Parameters : HANDLE hDIB - specifies the DIB to convert.                  *
 * Returns :  A handle to a device dependent bitmap. The handle will be NULL *
 *            if the handle cannot be created for any reason.                *
 * Comments : To keep code simple, palettes have not been used/created. If   *
 *            want to know how to preserve the colors of a 256 bitmap (by    *
 *            using a Palatte) please see the samples DIBView and WINCAP.    *
 *****************************************************************************/ 

HBITMAP W_IDIBToBitmap (HANDLE hDIB)
{
   LPSTR lpDIBHdr, lpDIBBits;     /* pointer to DIB header, pointer to DIB bits*/
   HBITMAP hBitmap;
   HDC hDC;

   lpDIBHdr = GlobalLock(hDIB);

   /* Get a pointer to the DIB bits*/
   lpDIBBits = lpDIBHdr + *(LPDWORD)lpDIBHdr + W_IPaletteSize((LPSTR)lpDIBHdr);

   hDC = GetDC( ( HWND )NULL);
   if (!hDC)
   {
      GlobalUnlock(hDIB);
      return ( HBITMAP )NULL;
   }

   /* create bitmap from DIB info. and bits */
   hBitmap = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER)lpDIBHdr, CBM_INIT,
                            lpDIBBits, (LPBITMAPINFO)lpDIBHdr, DIB_RGB_COLORS);

    ReleaseDC( ( HWND )NULL, hDC);
   GlobalUnlock(hDIB);

   return hBitmap;
}

/*****************************************************************************
 * Function : W_IPaletteSize()                                               *
 * Purpose  : Calculates the palette size in bytes. If the info. block is of *
 *            the BITMAPCOREHEADER type, the number of colors is multiplied  *
 *            by sizeof(RGBTRIPLE) to give the palette size, otherwise the   *
 *            number of colors is multiplied by sizeof(RGBQUAD).             *
 * Parameters : LPSTR pv - pointer to the BITMAPINFOHEADER                   *
 * Returns :  The size of the palette.                                       *
 * Comments :                                                                *
 ***************************************************************************** */

WORD W_IPaletteSize (LPSTR pv)
{
   LPBITMAPINFOHEADER lpbi;
   WORD NumColors;

   lpbi = (LPBITMAPINFOHEADER)pv;
   NumColors = W_IDIBNumColors((LPSTR)lpbi);

   if (lpbi->biSize == sizeof(BITMAPCOREHEADER))  /* OS/2 style DIBs*/
      return NumColors * sizeof(RGBTRIPLE);
   else
      return NumColors * sizeof(RGBQUAD);
}

/*****************************************************************************
 * Function : W_IDIBNumColors()                                              *
 * Purpose  : This function calculates the number of colors in the DIB's     *
 *            color table by finding the bits per pixel for the DIB (whether *
 *            Win3.0 or OS/2-style DIB). If bits per pixel is 1: colors=2,   *
 *            if 4: colors=16, if 8: colors=256, if 24, no colors in color   *
 *            table.                                                         *
 * Parameters : LPSTR lpbi - pointer to packed-DIB memory block.             *
 * Returns :  The number of colors in the color table.                       *
 * Comments :                                                                *
 *****************************************************************************/

WORD W_IDIBNumColors (LPSTR pv)
{
   int bits;
   LPBITMAPINFOHEADER lpbi;
   LPBITMAPCOREHEADER lpbc;

   lpbi = ((LPBITMAPINFOHEADER)pv);     /* assume win 3.0 style DIBs*/
   lpbc = ((LPBITMAPCOREHEADER)pv);     /* assume OS/2 style DIBs*/

   /* With the BITMAPINFO format headers, the size of the palette
      is in biClrUsed, whereas in the BITMAPCORE - style headers, it
      is dependent on the bits per pixel ( = 2 raised to the power of
      bits/pixel).*/

   if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
   {
      if (lpbi->biClrUsed != 0)
         return (WORD)lpbi->biClrUsed;
      bits = lpbi->biBitCount;
   }
   else
      bits = lpbc->bcBitCount;

   switch (bits)
   {
      case 1:
         return 2;

      case 4:
         return 16;

      case 8:
         return 256;

      default:
         /* A 24 bitcount DIB has no color table*/
         return 0;
   }
}

/*****************************************************************************
 * Function : W_IGetDIBInfo()                                                *
 * Purpose  : This function fills in the DIB info header's default fields.   *
 *            table.                                                         *
 * Parameters : LPSTR lpbi - pointer to packed-DIB memory block.             *
 *              HANDLE hbi - handle to packed-DIB memory block.              *
 * Returns :  The number of colors in the color table.                       *
 * Comments :                                                                *
 * History :  Date          Author            Reason                         *
 *            ???           ???               Created (ShowDIB SDK sample)   *
 *            2/25/92       Krishna           Added header.                  *
 *****************************************************************************/

BOOL W_IGetDIBInfo (HANDLE hbi, LPBITMAPINFOHEADER lpbi)
{
   if (hbi)
   {
      *lpbi = *(LPBITMAPINFOHEADER)GlobalLock(hbi);

      /* fill in the default fields*/
      if (lpbi->biSize != sizeof(BITMAPCOREHEADER))   /* If not OS/2 style */
      {
         if (lpbi->biSizeImage == 0L)
            lpbi->biSizeImage =            /* determine image size in bytes*/
                  WIDTHBYTES(lpbi->biWidth*lpbi->biBitCount) * lpbi->biHeight;

         if (lpbi->biClrUsed == 0L)        /* determine number of colors*/
            lpbi->biClrUsed = W_IDIBNumColors((LPSTR)lpbi);
      }
      GlobalUnlock(hbi);
      return TRUE;
   }
   return FALSE;
}

/*****************************************************************************
 * Function : W_IReadDIBBits()                                               *
 * Purpose  : Reads data in steps of MAXREAD bytes till all the data has been*
 *            read                                                           *
 * Parameters : int fh - Handle of the file to be read in.                   *
 *              void far *pv - structure to be filled with the data read.    *
 *              DWORD ul - Number of bytes to read.                          * 
 * Returns :  The number of bytes read. 0, if read did not proceed properly. *                       *
 *****************************************************************************/

DWORD PASCAL W_IReadDIBBits (int fh, VOID FAR *pv, DWORD ul)
{
   DWORD ulT = ul;
   BYTE *hp = pv;

   while (ul > (DWORD)MAXREAD)
   {
      if (_lread(fh, (LPSTR)hp, (WORD)MAXREAD) != MAXREAD)
         return 0;
      ul -= MAXREAD;
      hp += MAXREAD;
   }
   if (_lread(fh, (LPSTR)hp, (WORD)ul) != (WORD)ul)
      return 0;    /* failure. return 0*/

   /* successfully completed reading the file. Return # of bytes read.*/
   return ulT;
}

/*******************************************************************************
  Draw icon hSrcIcon over hDstIcon trnasparently
 *******************************************************************************/

HICON 
W_IOverDrawIcon ( HICON hDstIcon, HICON hSrcIcon, int dxOffset, int dyOffset, POINT *pNewSize )
{
	ICONINFO		DstIconInfo;
	ICONINFO		SrcIconInfo;
	ICONINFO		CombinedIconInfo;
	POINT			ptSize;
	POINT			ptMaxSize;
	BITMAP			Bitmap;
	HDC				hDeskDC     = GetDC ( GetDesktopWindow () );
	HDC				hTempDC	    = ( HDC )NULL;
	HDC				hSrcMaskDC  = ( HDC )NULL;
	HDC				hDstMaskDC  = ( HDC )NULL;
	
	HDC				hSrcDC	    = ( HDC )NULL;

	HBITMAP			hBitmap     = ( HBITMAP )NULL;
	HBITMAP			hbmMask     = ( HBITMAP )NULL;
	HICON			hIcon       = ( HICON )NULL;
	HBITMAP			hbmpOld	    = ( HBITMAP )NULL;
	HBITMAP			hmskOld	    = ( HBITMAP )NULL;


	memset ( &Bitmap, 0, sizeof (BITMAP) );
	memset ( &DstIconInfo, 0, sizeof ( ICONINFO ) );
	memset ( &SrcIconInfo, 0, sizeof ( ICONINFO ) );
	memset ( &CombinedIconInfo, 0, sizeof ( ICONINFO ) );

	if ( ( ! ( hTempDC = CreateCompatibleDC ( hDeskDC ) ) ) || 
	     ( ! ( hSrcDC  = CreateCompatibleDC ( hDeskDC ) ) ) ||
	     ( ! ( hSrcMaskDC  = CreateCompatibleDC ( hDeskDC ) ) ) ||
	     ( ! ( hDstMaskDC  = CreateCompatibleDC ( hDeskDC ) ) ) 
	   )
		return ( HICON )NULL;
		
	/*get size of the first icon*/
	GetIconInfo ( hDstIcon, &DstIconInfo );
	GetObject ( DstIconInfo.hbmColor, sizeof ( BITMAP ), &Bitmap ); 
	ptSize.x = Bitmap.bmWidth;
	ptSize.y = Bitmap.bmHeight;
	DPtoLP ( hTempDC, &ptSize, 1 );
	ptMaxSize.x = ptSize.x;
	ptMaxSize.y = ptSize.y;	

	/*get size of the second icon*/
	memset ( &Bitmap, 0, sizeof (BITMAP) );
	GetIconInfo ( hSrcIcon, &SrcIconInfo );
	GetObject ( SrcIconInfo.hbmColor, sizeof ( BITMAP ), &Bitmap ); 
	ptSize.x = Bitmap.bmWidth;
	ptSize.y = Bitmap.bmHeight;
	DPtoLP ( hSrcDC, &ptSize, 1 );
	ptMaxSize.x = ptMaxSize.x < ptSize.x ? ptSize.x : ptMaxSize.x;
	ptMaxSize.y = ptMaxSize.y < ptSize.y ? ptSize.y : ptMaxSize.y;
	ptMaxSize.x += dxOffset;
	ptMaxSize.y += dyOffset;
	pNewSize->x = ptMaxSize.x;
	pNewSize->y = ptMaxSize.y;

	/*this would be the one holding both icon images drawn over each other*/
	hBitmap = CreateCompatibleBitmap ( hDeskDC, ptMaxSize.x, ptMaxSize.y );
	hbmMask = CreateCompatibleBitmap ( hDstMaskDC, ptMaxSize.x, ptMaxSize.y );

	/*prepare DCs for both the images and masks*/
	hbmpOld = SelectObject ( hTempDC, hBitmap );
	PatBlt ( hTempDC, 0, 0, ptMaxSize.x, ptMaxSize.y, WHITENESS );
	hmskOld = SelectObject ( hDstMaskDC, hbmMask );
	PatBlt ( hDstMaskDC, 0, 0, ptMaxSize.x, ptMaxSize.y, WHITENESS );

	/*draw transparently the first image*/
	if ( DstIconInfo.hbmColor )
		SelectObject ( hSrcDC, DstIconInfo.hbmColor );
	else
		PatBlt ( hSrcDC, 0, 0, ptMaxSize.x, ptMaxSize.y, BLACKNESS );
	SelectObject ( hSrcMaskDC, DstIconInfo.hbmMask );
	BitBlt ( hTempDC, 0, 0, ptMaxSize.x, ptMaxSize.y, hSrcDC, 0, 0, SRCINVERT );
	BitBlt ( hTempDC, 0, 0, ptMaxSize.x, ptMaxSize.y, hSrcMaskDC, 0, 0, SRCAND );
	BitBlt ( hTempDC, 0, 0, ptMaxSize.x, ptMaxSize.y, hSrcDC, 0, 0, SRCINVERT );

	/*get the first mask*/
	BitBlt ( hDstMaskDC, 0, 0, ptMaxSize.x, ptMaxSize.y, hSrcMaskDC, 0, 0, SRCCOPY );

	/*draw transparently the second image over the first one*/
	PatBlt ( hSrcDC, 0, 0, ptMaxSize.x, ptMaxSize.y, BLACKNESS );
	SelectObject ( hSrcDC, SrcIconInfo.hbmColor );
	SelectObject ( hSrcMaskDC, SrcIconInfo.hbmMask );
	BitBlt ( hTempDC, dxOffset, dyOffset, ptMaxSize.x, ptMaxSize.y, hSrcDC, 0, 0, SRCINVERT );
	BitBlt ( hTempDC, dxOffset, dyOffset, ptMaxSize.x, ptMaxSize.y, hSrcMaskDC, 0, 0, SRCAND );
	BitBlt ( hTempDC, dxOffset, dyOffset, ptMaxSize.x, ptMaxSize.y, hSrcDC, 0, 0, SRCINVERT );

	BitBlt ( hDstMaskDC, dxOffset, dyOffset, ptMaxSize.x, ptMaxSize.y, hSrcMaskDC, 0, 0, SRCAND );

#if  0 /*def _DEBUG*/
	BitBlt ( hTestDC, 0, 0, ptMaxSize.x, ptMaxSize.y, hTempDC, 0, 0, SRCCOPY );
	BitBlt ( hTestDC, 45, 0, ptMaxSize.x, ptMaxSize.y, hDstMaskDC, 0, 0, SRCCOPY );

	BitBlt ( hTestDC, 75, 0, ptMaxSize.x, ptMaxSize.y, hTempDC, 0, 0, SRCINVERT );
	BitBlt ( hTestDC, 75, 0, ptMaxSize.x, ptMaxSize.y, hDstMaskDC, 0, 0, SRCAND );
	BitBlt ( hTestDC, 75, 0, ptMaxSize.x, ptMaxSize.y, hTempDC, 0, 0, SRCINVERT );
#endif

	SelectObject ( hDstMaskDC, hmskOld );
	SelectObject ( hTempDC, hbmpOld );

	if ( DstIconInfo.hbmColor )
		DeleteObject ( DstIconInfo.hbmColor );
	if ( DstIconInfo.hbmMask )
		DeleteObject ( DstIconInfo.hbmMask );


	CombinedIconInfo.fIcon = TRUE;
	CombinedIconInfo.xHotspot = 0;
	CombinedIconInfo.yHotspot = 0;
	CombinedIconInfo.hbmColor = hBitmap;
	CombinedIconInfo.hbmMask =  hbmMask;
	hIcon = CreateIconIndirect ( &CombinedIconInfo );


	if ( hBitmap )
		DeleteObject ( hBitmap );
	if ( hbmMask )
		DeleteObject ( hbmMask );
	if ( hTempDC )
		DeleteDC ( hTempDC );
	if ( hDeskDC )
		ReleaseDC ( GetDesktopWindow (), hDeskDC );
	return ( hIcon );
	
}


