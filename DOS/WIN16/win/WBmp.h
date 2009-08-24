/*       
	WBmp.h	1.1
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

#ifndef  _BMP_H
#define  _BMP_H


/* Header signatutes for various resources*/
#define BFT_ICON   0x4349   /* 'IC' */
#define BFT_BITMAP 0x4d42   /* 'BM' */
#define BFT_CURSOR 0x5450   /* 'PT' */

/* This WIDTHBYTES macro determines the number of BYTES per scan line.*/
#define  WIDTHBYTES(i)   ((i+31)/32*4)
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))

/* flags for _lseek */
#define  SEEK_CUR 1
#define  SEEK_END 2
#define  SEEK_SET 0

/* macro to determine if resource is a DIB */
#define ISDIB(bft) ((bft) == BFT_BITMAP)

#define MAXREAD 32767

HBITMAP      W_IDIBToBitmap(HANDLE hDIB);
WORD         W_IDIBNumColors(LPSTR pv);
HANDLE       W_IReadDibBitmapInfo (int fh);
BOOL         W_IGetDIBInfo(HANDLE hbi, LPBITMAPINFOHEADER lpbi);
HANDLE       ReadDIB (LPSTR szFile);
DWORD PASCAL W_IReadDIBBits(int fh, VOID FAR *pv, DWORD ul);
WORD         W_IPaletteSize(LPSTR lpbi);

HICON 	     
W_IOverDrawIcon 
( 
  HICON				hDstIcon, 
  HICON				hSrcIcon, 
  int				dxOffset, 
  int				dyOffset,
  POINT				*ptNewSize
);


#endif


