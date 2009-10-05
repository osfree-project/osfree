/*    
	WImageIO.c	1.6 Common Controls Implementation source - helper file
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
#include <math.h>
#ifdef _WINDOWS
#include <io.h>
#include <direct.h>
#endif
#include <stdlib.h>
#include "WImageIO.h"
#include "WBmp.h"
#include "WCursor.h"
#include "WIcon.h"

/*
 * Dib Header Marker - used in writing DIBs to files
 */
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
#define ALIGNLONG(i)	( ( i+3 )/ 4 * 4 )


/*************************************************************************
 *
 * Function:  W_IReadBmpFile ( LPCSTR )
 *
 *  Purpose:  Reads in the specified DIB file into a global chunk of
 *            memory.
 *
 *  Returns:  A handle to a dib (hDIB) if successful.
 *            NULL if an error occurs.
 *
 * Comments:  BITMAPFILEHEADER is stripped off of the DIB.  Everything
 *            from the end of the BITMAPFILEHEADER structure on is
 *            returned in the global memory handle.
 *
 *************************************************************************/

HBITMAP W_IReadBmpFile ( LPCSTR szFileName )
{
   HANDLE hDIB;
   HBITMAP hBitmap;
   unsigned fh;
   BITMAPINFOHEADER bi;
   LPBITMAPINFOHEADER lpbi;
   DWORD dwLen = 0;
   DWORD dwBits;
   HANDLE h;
   OFSTRUCT of;

   {


   /* Open the file and read the DIB information */

   fh = OpenFile(szFileName, &of, OF_READ);
   if (fh == -1)
      return ( HBITMAP )NULL;

   hDIB = W_IReadDibBitmapInfo(fh);
   if (!hDIB)
      return ( HBITMAP )NULL;

   W_IGetDIBInfo(hDIB, &bi);

   /* Calculate the memory needed to hold the DIB*/
   dwBits = bi.biSizeImage;
   dwLen = bi.biSize + (DWORD)W_IPaletteSize((LPSTR)&bi) + dwBits;

   /* Try to increase the size of the bitmap info. buffer to hold the DIB*/
   h = GlobalReAlloc(hDIB, dwLen, GHND);
   if (!h)
   {
      GlobalFree(hDIB);
      hDIB = ( HANDLE )NULL;
   }
   else
      hDIB = h;

   /* Read in the bits*/
   if (hDIB)
   {
      lpbi = (VOID FAR *)GlobalLock(hDIB);
      W_IReadDIBBits(fh, (LPSTR)lpbi + (WORD)lpbi->biSize + W_IPaletteSize((LPSTR)lpbi), dwBits);
      GlobalUnlock(hDIB);
   }

   _lclose(fh);


   }
   
   if (hDIB == ( HANDLE )NULL)
      return ( HBITMAP )NULL;

   hBitmap = W_IDIBToBitmap(hDIB);   /* Create a DD bitmap out of the DIB info*/

   GlobalFree(hDIB);
   return (hBitmap);
}

#if 1
HBITMAP W_IReadIconFile ( LPCSTR szFileName )
{
   HANDLE hDIB;    /* Handle to DIB memory*/
   HBITMAP hBitmap;
   LPBITMAPINFO lpDIB;
   ICONFILEHEADER iconFileHead;   /* ICON file header structure*/
   ICONFILERES iconFileRes;  /* ICON file resource*/
   WORD cbHead, cbRes, cbBits;    /* Used for reading in file*/
   int hFile; /* File handle*/

   /* Open and read the .ICO file header and the first ICONFILERES*/

   hFile = _lopen(szFileName, OF_READ);
   cbHead = _lread(hFile, (LPSTR)&iconFileHead, sizeof(ICONFILEHEADER));
   cbRes = _lread(hFile, (LPSTR)&iconFileRes, sizeof(ICONFILERES));

   if ((cbHead != sizeof(ICONFILEHEADER)) || (cbRes != sizeof(ICONFILERES)))
   {
      return ( HBITMAP )NULL;
   }

   /* Verify that it's an .ICON file*/

   if (iconFileHead.wResourceType != 1)
   {
      return ( HBITMAP )NULL;
   }

   /* Allocate and lock memory to read in the DIB*/
   hDIB = GlobalAlloc(GHND, iconFileRes.dwDIBSize);
   if (hDIB == ( HANDLE )NULL)
   {
      return ( HBITMAP )NULL;
   }
   lpDIB = (LPBITMAPINFO)GlobalLock(hDIB);

   /* Now read the DIB portion of the file, which follows the 
      end of icon resource table*/
   _llseek(hFile, iconFileRes.dwDIBOffset, 0);
   cbBits = _lread(hFile, (LPSTR)lpDIB, (WORD)iconFileRes.dwDIBSize);

   /* Done reading file*/
   _lclose(hFile);

   if ((DWORD)cbBits != iconFileRes.dwDIBSize)
   {
      GlobalUnlock(hDIB);
      GlobalFree(hDIB);
      return ( HBITMAP )NULL;
   }
   
   /* We need only the XOR part of the Icon DIB. So use just the top half of
      the DIB*/
   lpDIB->bmiHeader.biHeight /= 2;

   GlobalUnlock(hDIB);
   hBitmap = W_IDIBToBitmap(hDIB);      /* Convert the DIB to DD bitmap*/

   GlobalUnlock(hDIB);
   GlobalFree(hDIB);
   return (hBitmap);
}
#else
HICON 
W_IReadIconFile ( LPCSTR szFileName )
{
   HANDLE hDIB;    /* Handle to DIB memory*/
   HBITMAP hBitmap;
   LPBITMAPINFO lpDIB;
   ICONFILEHEADER iconFileHead;   /* ICON file header structure*/
   ICONFILERES iconFileRes;  /* ICON file resource*/
   WORD cbHead, cbRes, cbBits;    /* Used for reading in file*/
   int hFile; /* File handle*/

   /* Open and read the .ICO file header and the first ICONFILERES*/

   hFile = _lopen(szFileName, OF_READ);
   cbHead = _lread(hFile, (LPSTR)&iconFileHead, sizeof(ICONFILEHEADER));
   cbRes = _lread(hFile, (LPSTR)&iconFileRes, sizeof(ICONFILERES));

   if ((cbHead != sizeof(ICONFILEHEADER)) || (cbRes != sizeof(ICONFILERES)))
   {
      return ( HICON )NULL;
   }

   /* Verify that it's an .ICON file*/

   if (iconFileHead.wResourceType != 1)
      return ( HICON )NULL;

   /* Allocate and lock memory to read in the DIB*/
   hDIB = GlobalAlloc(GHND, iconFileRes.dwDIBSize);
   if (hDIB == ( HANDLE )NULL)
      return ( HICON )NULL;

   lpDIB = (LPBITMAPINFO)GlobalLock(hDIB);

   /* Now read the DIB portion of the file, which follows the 
      end of icon resource table*/
   _llseek(hFile, iconFileRes.dwDIBOffset, 0);
   cbBits = _lread(hFile, (LPSTR)lpDIB, (WORD)iconFileRes.dwDIBSize);

   if ((DWORD)cbBits != iconFileRes.dwDIBSize)
   {
      GlobalUnlock(hDIB);
      GlobalFree(hDIB);
      return ( HICON )NULL;
   }
   
   /* We need only the XOR part of the Icon DIB. So use just the top half of
      the DIB*/
   lpDIB->bmiHeader.biHeight /= 2;

   GlobalUnlock(hDIB);
   hBitmap = W_IDIBToBitmap(hDIB);      /* Convert the DIB to DD bitmap*/

   /* Done reading file*/
   _lclose(hFile);

   GlobalUnlock(hDIB);
   GlobalFree(hDIB);
   return (hBitmap);
}
#endif

HBITMAP W_IReadCursorFile ( LPCSTR szFileName )
{
   HANDLE hDIB;    /* Handle to DIB memory*/
   HBITMAP hBitmap;
   LPBITMAPINFO lpDIB;

   CURFILEHEADER curFileHead;     /* CURSOR file header structure*/
   CURFILERES curFileRes;    /* CURSOR file resource*/
   WORD cbHead, cbRes, cbBits;    /* Used for reading in file*/
   int hFile; /* Handle to File*/

   {


   /* Open and read the .ICO file header and the first ICONFILERES*/

   hFile = _lopen(szFileName, OF_READ);
   cbHead = _lread(hFile, (LPSTR)&curFileHead, sizeof(CURFILEHEADER));
   cbRes = _lread(hFile, (LPSTR)&curFileRes, sizeof(CURFILERES));

   if ((cbHead != sizeof(CURFILEHEADER)) || (cbRes != sizeof(CURFILERES)))
   {
      return ( HBITMAP )NULL;
   }

   /* Verify that it's an .CUR file*/
   if ((curFileRes.bReserved1 != 0) || (curFileHead.wResourceType != 2))
   {
      return ( HBITMAP )NULL;
   }

   /* Allocate & lock memory to read in the DIB*/
   hDIB = GlobalAlloc(GHND, curFileRes.dwDIBSize);
   if (hDIB == ( HANDLE )NULL)
   {
      return ( HBITMAP )NULL;
   }
   lpDIB = (LPBITMAPINFO)GlobalLock(hDIB);

   /* Now read the DIB portion of the file, which follows the 
      end of icon resource table*/
   _llseek(hFile, curFileRes.dwDIBOffset, 0);
   cbBits = _lread(hFile, (LPSTR)lpDIB, (WORD)curFileRes.dwDIBSize);

   /* Done reading file*/
   _lclose(hFile);

   if ((DWORD)cbBits != curFileRes.dwDIBSize)
   {
      GlobalUnlock(hDIB);
      GlobalFree(hDIB);
      return ( HBITMAP )NULL;
   }

   }
   
   lpDIB = (LPBITMAPINFO)GlobalLock(hDIB);  /* obtain a pointer to the DIB*/

   /* We need only the XOR part of the Cursor DIB. So use just the top half of*/
   /* the DIB*/
   lpDIB->bmiHeader.biHeight /= 2;

   GlobalUnlock(hDIB);
   hBitmap = W_IDIBToBitmap(hDIB);         /* Convert the DIB to DD bitmap.*/

   GlobalUnlock(hDIB);
   GlobalFree(hDIB);
   return (hBitmap);
}


#if 0
BOOL W_IReadBits(int hFile, LPSTR lpBuffer, DWORD dwSize)
{
   char *lpInBuf = (char *)lpBuffer;
   int nBytes;

   /*
    * Read in the data in 32767 byte chunks (or a smaller amount if it's
    * the last chunk of data read)
    */

   while (dwSize)
   {
      nBytes = (int)(dwSize > (DWORD)32767 ? 32767 : LOWORD (dwSize));
      if (_lread(hFile, (LPSTR)lpInBuf, nBytes) != (WORD)nBytes)
         return FALSE;
      dwSize -= nBytes;
      lpInBuf += nBytes;
   }
   return TRUE;
}

#endif

/*****************************************************************************
 * Function : W_ILoadImage()                                                 *
 * Returns :  always an HBITMAP.											  *
 * Comments :                                                                *
 *****************************************************************************/


HBITMAP WINAPI
W_ILoadImage
(
	HINSTANCE			hInstance,
	LPCSTR				pstrImage,
	COLORREF			MaskColor,
	UINT				ImageType,
	UINT				LoadFlags
)

{
	HBITMAP				hImage		  = ( HBITMAP )NULL;
	HCURSOR				hCursor		  = ( HCURSOR )NULL;
	HICON				hIcon		  = ( HICON )NULL;

	BITMAP				bmColor;
	ICONINFO			IconInfo;
	int				ErrorCode = 0;


	memset ( &bmColor, 0, sizeof ( BITMAP ) );
	memset ( &IconInfo, 0, sizeof ( ICONINFO ) );

	if ( LoadFlags & LR_LOADFROMFILE )
	{
		if ( ImageType == IMAGE_BITMAP )
			hImage = W_IReadBmpFile ( pstrImage );
		else
		if ( ImageType == IMAGE_ICON )
			hImage = W_IReadIconFile ( pstrImage );
		else
			hImage = W_IReadCursorFile ( pstrImage );

	}
	else
	{
		switch ( ImageType )
		{
			
			case IMAGE_BITMAP   :
				if ( ! ( hImage = LoadBitmap ( hInstance, pstrImage ) ) )
					ErrorCode =  WIM_ERR_LOADIMAGE;
					
				break;

			case IMAGE_ICON     :
				if ( ! ( hIcon = LoadIcon ( hInstance, pstrImage ) ) )
					break;
				if ( GetIconInfo ( hIcon, &IconInfo ) )
					hImage = IconInfo.hbmColor;
				break;
			
			case IMAGE_CURSOR   :
				if ( ! ( hCursor = LoadCursor ( hInstance, pstrImage ) ) )
					break;
				if ( GetIconInfo ( hCursor, &IconInfo ) )
					hImage = IconInfo.hbmColor;
				break;
		
			default		    :
				ErrorCode =  WIM_ERR_LOADIMAGE;
				break;
		}

	}

	return hImage;
}

#if 1
/*
 * LoadImage() Win32 API
 */
HANDLE 
WLoadImage
(
	HINSTANCE			hInstance,
	LPCSTR				pstrImage,
	UINT				ImageType,
	int				cxDesired,
	int				cyDesired,
	UINT				LoadFlags
)

{
	POINT			    	ptSize;
	HBITMAP				hImage		  = ( HBITMAP )NULL;
	HBITMAP				hImageMask	  = ( HBITMAP )NULL;
	HCURSOR				hCursor		  = ( HCURSOR )NULL;
	HICON				hIcon		  = ( HICON )NULL;
	HDC				hDeskDC		  = GetDC ( GetDesktopWindow () );
	HBITMAP				hBitmap		  = ( HBITMAP )NULL;
	HBITMAP				hbmpMask	  = ( HBITMAP )NULL;
	HDC				hSrcDC		  = ( HDC )NULL;
	HDC				hDstDC		  = ( HDC )NULL;
	HANDLE				hDIB		  = ( HANDLE )NULL;
	HANDLE				hmDIB		  = ( HANDLE )NULL;

	BITMAPINFOHEADER		bi;
	HBITMAP			    	hbmpOld;

	BITMAP			    	bmColor;
	BITMAP			    	Bitmap;
	ICONINFO		    	IconInfo;
	int			        ErrorCode = 0;


	memset ( &bi, 0, sizeof ( BITMAPINFOHEADER ) );
	memset ( &bmColor, 0, sizeof ( BITMAP ) );
	memset ( &IconInfo, 0, sizeof ( ICONINFO ) );

	if ( LoadFlags & LR_LOADFROMFILE )
	{
		if ( ImageType == IMAGE_BITMAP )
			hImage = W_IReadBmpFile ( pstrImage );
		else
		if ( ImageType == IMAGE_ICON )
			hImage = W_IReadIconFile ( pstrImage );
		else
			hImage = W_IReadCursorFile ( pstrImage );

	}
	else
	{
		switch ( ImageType )
		{
			
			case IMAGE_BITMAP   :
				if ( ! ( hImage = LoadBitmap ( hInstance, pstrImage ) ) )
					ErrorCode =  WIM_ERR_LOADIMAGE;
					
				break;

			case IMAGE_ICON     :
				if ( ! ( hIcon = LoadIcon ( hInstance, pstrImage ) ) )
					break;
				if ( GetIconInfo ( hIcon, &IconInfo ) )
				{
					hImage = IconInfo.hbmColor;
					hImageMask = IconInfo.hbmMask;
				}
				break;
			
			case IMAGE_CURSOR   :
				if ( ! ( hCursor = LoadCursor ( hInstance, pstrImage ) ) )
					break;
				if ( GetIconInfo ( hCursor, &IconInfo ) )
				{
					hImage = IconInfo.hbmColor;
					hImageMask = IconInfo.hbmMask;
				}
				break;
		
			default		    :
				ErrorCode =  WIM_ERR_LOADIMAGE;
				break;
		}

	}

	GetObject ( hImage, sizeof ( BITMAP ), &Bitmap ); 
	ptSize.x = Bitmap.bmWidth;
	ptSize.y = Bitmap.bmHeight;
	DPtoLP ( hDeskDC, &ptSize, 1 );

	if ( ( hSrcDC = CreateCompatibleDC ( hDeskDC ) ) &&
	     ( hDstDC = CreateCompatibleDC ( hDeskDC ) )
	   ) 
	{
		hBitmap = CreateCompatibleBitmap ( hDeskDC, cxDesired, cyDesired );
		hbmpOld = SelectObject ( hDstDC, hBitmap );
		SelectObject ( hSrcDC, hImage );
		StretchBlt ( hDstDC , 0, 0, cxDesired, cyDesired, hSrcDC,
			0, 0, ptSize.x, ptSize.y, SRCCOPY );
		SelectObject ( hDstDC, hbmpOld );
		GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap ); 

		if ( hImageMask )
		{
			hbmpMask = CreateCompatibleBitmap ( hDeskDC, cxDesired, cyDesired );
			hbmpOld = SelectObject ( hDstDC, hbmpMask );
			SelectObject ( hSrcDC, hImageMask );
			StretchBlt ( hDstDC, 0, 0, cxDesired, cyDesired, hSrcDC,
				0, 0, ptSize.x, ptSize.y, SRCCOPY );
			SelectObject ( hDstDC, hbmpOld );
		}
	}

#if 0
	if ( ImageType	== IMAGE_ICON || ImageType == IMAGE_CURSOR )
		return CreateIcon ( hInstance, Bitmap.bmWidth, Bitmap.bmHeight, ( BYTE )Bitmap.bmPlanes,
				( BYTE )Bitmap.bmBitsPixel, lpstBitmap, lpstmBitmap );
#else
	if ( ImageType	== IMAGE_ICON || ImageType == IMAGE_CURSOR )
	{
		IconInfo.hbmColor = hBitmap;
		IconInfo.hbmMask = hbmpMask;
		return CreateIconIndirect ( &IconInfo );
	}
#endif
	GlobalUnlock ( hDIB );
	GlobalFree ( hDIB );
	GlobalUnlock ( hmDIB );
	GlobalFree ( hmDIB );
	DeleteObject ( hBitmap );
	DeleteObject ( hbmpMask );
	if ( hDstDC )
		DeleteDC ( hDstDC );
	if ( hSrcDC )
		DeleteDC ( hSrcDC );
	ReleaseDC ( GetDesktopWindow (), hDeskDC );
	return hImage;

}
#endif
