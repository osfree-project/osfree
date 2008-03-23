/* 
       @(#)WIcon.h	1.1
	Icon header structures from "SDK Reference -- Volume 2", 
	page 9-2, "Icon Resource File Format"
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

struct tagICONFILEHEADER {
  WORD wReserved;         /* Always 0*/
  WORD wResourceType;     /* 1 = icon*/
  WORD wResourceCount;    /* Number of icons in the file*/
  };

typedef struct tagICONFILEHEADER ICONFILEHEADER;

struct tagICONFILERES {
    BYTE  bWidth;         /* Width of image*/
    BYTE  bHeight;        /* Height of image*/
    BYTE  bColorCount;    /* Number of colors in image (2, 8, or 16)*/
    BYTE  bReserved1;     /* Reserved*/
    WORD  wReserved2;
    WORD  wReserved3; 
    DWORD dwDIBSize;      /* Size of DIB for this image*/
    DWORD dwDIBOffset;    /* Offset to DIB for this image*/
  };

typedef struct tagICONFILERES ICONFILERES;

HANDLE ReadIcon(LPSTR szFileName);
HICON MakeIcon(HANDLE hDIB);


