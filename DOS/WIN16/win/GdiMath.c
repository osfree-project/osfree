/*    
	GdiMath.c	2.8
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

#include <math.h>

#include "windows.h"

#include "Log.h"

int WINAPI
MulDiv(int nMultiplicand, int nMultiplier, int nDivisor)
{
    int		nResult;
    long	lMulti;
#ifdef OLDWAY
    double	div1,fraction;

    lMulti = nMultiplicand * nMultiplier;
    if (!nDivisor)
	nResult = -1;
    else {
	div1 = (double)(lMulti) / (double)(nDivisor);
	fraction = div1 - floor(div1);
	if (fraction < 0.5)
	    nResult = (int)((div1 > 0.0) ? floor(div1) : ceil(div1));
	else
	    nResult = (int)((div1 > 0.0) ? ceil(div1) : floor(div1));
    }
#else
    APISTR((LF_APICALL,"MulDiv(int=%d,int=%d,int=%x)\n",
	nMultiplicand, nMultiplier, nDivisor));
	
    lMulti = nMultiplicand * nMultiplier;
    if ( lMulti < 0 )
	lMulti -= nDivisor >> 1;
    else
	lMulti += nDivisor >> 1;
    nResult = lMulti / nDivisor;
#endif
    APISTR((LF_APIRET,"MulDiv: returns int %d\n",nResult));
    return nResult;
}

/* Bresenham line drawing algorithm */
/* x1, y1 and x2, y2 are end points */
void WINAPI
LineDDA(int x1, int y1, int x2, int y2,LINEDDAPROC f,LPARAM lParam)
{
     int  w, h, d, dxd, dyd, dxn, dyn, dinc, ndinc, p;
     register x, y;
 
    /* Set up */
    x = x1; y = y1;                   /* start of line */
    w = x2 - x1;                      /* width domain of line */
    h = y2 - y1;                      /* height domain of line */
 
    /* Determine drawing direction */
    if (w < 0) {                       /* drawing right to left */
	w = -w;                    /* absolute width */
	dxd = -1;                  /* x increment is negative */
    } else                             /* drawing left to right */
	dxd = 1;                   /* so x incr is positive */
    if (h < 0) {                       /* drawing bottom to top */
	h = -h;                    /* so get absolute height */
	dyd = -1;                  /* y incr is negative */
    } else                             /* drawing top to bottom */
	dyd = 1;                   /* so y incr is positive */
 
    /* Determine major axis of motion */
    if (w < h) {                       /* major axis is Y */
	p = h, h = w, w = p;       /* exchange height and width */
	dxn = 0;
	dyn = dyd;
    } else {                           /* major axis is X */
	dxn = dxd;
	dyn = 0;
    }
 
    /* Set control variables */
    ndinc = h * 2;                /* Non-diagonal increment */
    d = ndinc - w;                /* pixel selection variable */
    dinc = d - w;                 /* Diagonal increment */
   
    /* Loop to draw the line */
    for (p = 0; p <= w; p++) {
	f (x, y,lParam);
	if (d < 0) {                     /* step non-diagonally */
	    x += dxn;
	    y += dyn;
		d += ndinc;
	} else {                         /* step diagonally */
	    x += dxd;
	    y += dyd;
	    d += dinc;
	}
    }
}

BYTE bit_reverse[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

/* (WIN32) GDI Math ******************************************************** */

LONGLONG WINAPI
Int32x32To64(LONG a, LONG b)
{
	return (a * b);
}

DWORDLONG WINAPI
Int64ShllMod32(DWORDLONG a, DWORD b)
{
	return (a << b);
}

LONGLONG WINAPI
Int64ShraMod32(LONGLONG a, DWORD b)
{
	return (a >> b);
}

DWORDLONG WINAPI
Int64ShrlMod32(DWORDLONG a, DWORD b)
{
	return (a >> b);
}

DWORDLONG WINAPI
UInt32x32To64(DWORD a, DWORD b)
{
	return (a * b);
}

