/*
	@(#)DrvUtils.c	2.5
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
#include <X11/Xlib.h>

#include "windows.h"

/* Bresenham line drawing algorithm */
/* x1, y1 and x2, y2 are end points */
/* calculate two lines, from 0,0 to x1,x2 and y1,y2 */
/* these will be the compression/expansion arrays   */

int
stretch(int sx,int dx, LPINT dp)
{
	int x1,y1,x2,y2;
 	int w, h, d, dxd, dyd, dxn, dyn, dinc, ndinc, p;
 	int x, y;
 
	if (sx == dx) {
	    for (x=0; x<sx; x++)
		dp[x] = x;
	    return x;
	}
	x1 = 0;
	y1 = 0;
	x2 = dx;
	y2 = sx;

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
	ndinc = h * 2 - 1;            /* Non-diagonal increment */
	d = ndinc - w;                /* pixel selection variable */
	dinc = d - w;                 /* Diagonal increment */
   
	/* Loop to draw the line */
	for (p = 0; p < w; p++) {
		/* x, y */
		dp[x] = y;

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
	
	return dx;
}

void
flip(LPINT data,int len)
{
    int i;
    BYTE tmp;

    for (i=0; i<len/2; i++) {
	tmp = data[i];
	data[i] = data[len-i-1];
	data[len-i-1] = tmp;
    }
}


XPoint *
DrvConvertPoints(LPPOINT lpPts, int nNum)
{
    static int nNumPoints = 0;
    static XPoint *lpXPoints = 0;
    LPPOINT sp = lpPts;
    XPoint *tp;
    int i;

    if (lpXPoints == 0) {
	nNumPoints = nNum;
	lpXPoints = (XPoint *)WinMalloc(sizeof(XPoint) * nNumPoints);
    }
    else if (nNumPoints < nNum) {
	nNumPoints = nNum;
	lpXPoints = (XPoint *)WinRealloc(lpXPoints,
				sizeof(XPoint)*nNumPoints);
    }
    for (i=0,tp = lpXPoints; i<nNum; i++,sp++,tp++) {
	tp->x = (short)sp->x;
	tp->y = (short)sp->y;
    }
    return lpXPoints;
}
