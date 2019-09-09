/*    
	MapMode.c	2.28
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

#include "stdlib.h"

#include "kerndef.h"
#include "Log.h"
#include "GdiDC.h"
#include "DeviceData.h"

static LPPOINT ArrayPoints;
static int nNumPoints = 0;

void MM_RescaleNEWTEXTMETRIC(HDC32, LPNEWTEXTMETRIC, int);

/* ************************************************************************* */

/* Mapping mode conversion macros */

#define MULDIV(a,b,c)\
	(((b) == (c)) ? (a) : ((a)*(b))/(c))


#define MULDIVDIV(a,b,c,f)\
	(((f) == 1.0) ? MULDIV(a,b,c) : (int)(MULDIV(a,b,c) / f))

#define MULMULDIV(f,a,b,c)\
	(((f) == 1.0) ? MULDIV(a,b,c) : (int)(f * MULDIV(a,b,c)))

/* logical extent <---> device extent
 *	MM_LEtoDE_X		(assumes eM12 == eM21 == 0.0)
 *	MM_LEtoDE_Y		(assumes eM12 == eM21 == 0.0)
 *	MM_DEtoLE_X		(assumes eM12 == eM21 == 0.0)
 *	MM_DEtoLE_Y		(assumes eM12 == eM21 == 0.0)
 *
 * Note that eM12 == eM21 == 0.0 is equivalent to stating that the device
 * context has no rotation/shear transformations.
 */

#define MM_LEtoDE_X(hDC32,x)\
	MULMULDIV((hDC32)->eM11, (int)(x), (hDC32)->VEx, (hDC32)->WEx)

#define MM_LEtoDE_Y(hDC32,y)\
	MULMULDIV((hDC32)->eM22, (int)(y), (hDC32)->VEy, (hDC32)->WEy)

#define MM_DEtoLE_X(hDC32,x)\
	MULDIVDIV((int)(x), (hDC32)->WEx, (hDC32)->VEx, (hDC32)->eM11)

#define MM_DEtoLE_Y(hDC32,y)\
	MULDIVDIV((int)(y), (hDC32)->WEy, (hDC32)->VEy, (hDC32)->eM22)

/* (original version) logical extent to device extent */

#define MM_SCALE_X(hDC32,x)	MULDIV((int)(short)x, abs((hDC32)->VEx), \
						      abs((hDC32)->WEx))
#define MM_SCALE_Y(hDC32,y)	MULDIV((int)(short)y, abs((hDC32)->VEy), \
						      abs((hDC32)->WEy))

/* (original version) device extent to logical extent */

#define MM_RESCALE_X(hDC32,x)	MULDIV((int)(short)x, abs((hDC32)->WEx), \
						      abs((hDC32)->VEx))
#define MM_RESCALE_Y(hDC32,y)	MULDIV((int)(short)y, abs((hDC32)->WEy), \
						      abs((hDC32)->VEy))

/* logical coordinates <---> device coordinates
 *	MM0_LPtoDP_X		(assumes DOx, DOy = (0,0))
 *	MM0_LPtoDP_Y		(assumes DOx, DOy = (0,0))
 *	MM0_DPtoLP_X		(assumes DOx, DOy = (0,0))
 *	MM0_DPtoLP_Y		(assumes DOx, DOy = (0,0))
 *	MM_LPtoDP_X
 *	MM_LPtoDP_Y
 *	MM_DPtoLP_X
 *	MM_DPtoLP_Y
 */

#define MM0_LPtoDP_X(hDC32,x,y)\
	MM_PGtoDEV_X(hDC32, MM_WRLDtoPG_X(hDC32,x,y), MM_WRLDtoPG_Y(hDC32,x,y))

#define MM0_LPtoDP_Y(hDC32,x,y)\
	MM_PGtoDEV_Y(hDC32, MM_WRLDtoPG_X(hDC32,x,y), MM_WRLDtoPG_Y(hDC32,x,y))

#define MM0_DPtoLP_X(hDC32,x,y)\
	MM_PGtoWRLD_X(hDC32, MM_DEVtoPG_X(hDC32,x,y), MM_DEVtoPG_Y(hDC32,x,y))

#define MM0_DPtoLP_Y(hDC32,x,y)\
	MM_PGtoWRLD_Y(hDC32, MM_DEVtoPG_X(hDC32,x,y), MM_DEVtoPG_Y(hDC32,x,y))

#define MM_LPtoDP_X(hDC32,x,y)\
	(MM0_LPtoDP_X(hDC32,x,y) + (hDC32)->DOx)

#define MM_LPtoDP_Y(hDC32,x,y)\
	(MM0_LPtoDP_Y(hDC32,x,y) + (hDC32)->DOy)

#define MM_DPtoLP_X(hDC32,x,y)\
	MM0_DPtoLP_X(hDC32, x - (hDC32)->DOx, y - (hDC32)->DOy)

#define MM_DPtoLP_Y(hDC32,x,y)\
	MM0_DPtoLP_Y(hDC32, x - (hDC32)->DOx, y - (hDC32)->DOy)

/* world coordinates <---> page coordinates
 *	MM_WRLDtoPG_X
 *	MM_WRLDtoPG_Y
 *	MM_PGtoWRLD_X
 *	MM_PGtoWRLD_Y
 */


#define MM_WRLDtoPG_X(hDC32,x,y)\
	((int)((x) * (hDC32)->eM11 + (y) * (hDC32)->eM21 + (hDC32)->eDx))

#define MM_WRLDtoPG_Y(hDC32,x,y)\
	((int)((x) * (hDC32)->eM12 + (y) * (hDC32)->eM22 + (hDC32)->eDy))

#define MM_PGtoWRLD_X(hDC32,x,y)\
	((int)((x) * A11(hDC32) + (y) * A21(hDC32) + A31(hDC32)))

#define MM_PGtoWRLD_Y(hDC32,x,y)\
	((int)((x) * A12(hDC32) + (y) * A22(hDC32) + A32(hDC32)))

#define A11(hDC32)	(M11(hDC32) / DET_MATRIX(hDC32))
#define A21(hDC32)	(M12(hDC32) / DET_MATRIX(hDC32))
#define A31(hDC32)	(M13(hDC32) / DET_MATRIX(hDC32))

#define A12(hDC32)	(M21(hDC32) / DET_MATRIX(hDC32))
#define A22(hDC32)	(M22(hDC32) / DET_MATRIX(hDC32))
#define A32(hDC32)	(M23(hDC32) / DET_MATRIX(hDC32))

#define M11(hDC32)	+((hDC32)->eM22)
#define M12(hDC32)	-((hDC32)->eM21)
#define M13(hDC32)\
	+((hDC32)->eM21 * (hDC32)->eDy - (hDC32)->eM22 * (hDC32)->eDx)

#define M21(hDC32)	-((hDC32)->eM12)
#define M22(hDC32)	+((hDC32)->eM11)
#define M23(hDC32)\
	-((hDC32)->eM11 * (hDC32)->eDy - (hDC32)->eM12 * (hDC32)->eDx)

#define DET_MATRIX(hDC32)\
	((hDC32)->eM11 * (hDC32)->eM22 - (hDC32)->eM12 * (hDC32)->eM21)

/* page coordinates <---> device coordinates
 *	MM_PGtoDEV_X
 *	MM_PGtoDEV_Y
 *	MM_DEVtoPG_X
 *	MM_DEVtoPG_Y
 */

#define MM_PGtoDEV_X(hDC32,x,y)\
	(MULDIV((x) - (hDC32)->WOx, (hDC32)->VEx, (hDC32)->WEx) + (hDC32)->VOx)

#define MM_PGtoDEV_Y(hDC32,x,y)\
	(MULDIV((y) - (hDC32)->WOy, (hDC32)->VEy, (hDC32)->WEy) + (hDC32)->VOy)

#define MM_DEVtoPG_X(hDC32,x,y)\
	(MULDIV((x) - (hDC32)->VOx, (hDC32)->WEx, (hDC32)->VEx) + (hDC32)->WOx)

#define MM_DEVtoPG_Y(hDC32,x,y)\
	(MULDIV((y) - (hDC32)->VOy, (hDC32)->WEy, (hDC32)->VEy) + (hDC32)->WOy)

/****************************************************************************/

int (MM_LEtoDE_X)(HDC32 hDC32, int x)
{
	return MM_LEtoDE_X(hDC32, x);
}

int (MM_LEtoDE_Y)(HDC32 hDC32, int y)
{
	return MM_LEtoDE_Y(hDC32, y);
}

int (MM_DEtoLE_X)(HDC32 hDC32, int x)
{
	return MM_DEtoLE_X(hDC32, x);
}

int (MM_DEtoLE_Y)(HDC32 hDC32, int y)
{
	return MM_DEtoLE_Y(hDC32, y);
}

int (MM0_LPtoDP_X)(HDC32 hDC32, int x, int y)
{
	return MM0_LPtoDP_X(hDC32, x, y);
}

int (MM0_LPtoDP_Y)(HDC32 hDC32, int x, int y)
{
	return MM0_LPtoDP_Y(hDC32, x, y);
}

int (MM0_DPtoLP_X)(HDC32 hDC32, int x, int y)
{
	return MM0_DPtoLP_X(hDC32, x, y);
}

int (MM0_DPtoLP_Y)(HDC32 hDC32, int x, int y)
{
	return MM0_DPtoLP_Y(hDC32, x, y);
}

int (MM_LPtoDP_X)(HDC32 hDC32, int x, int y)
{
	return MM_LPtoDP_X(hDC32, x, y);
}

int (MM_LPtoDP_Y)(HDC32 hDC32, int x, int y)
{
	return MM_LPtoDP_Y(hDC32, x, y);
}

int (MM_DPtoLP_X)(HDC32 hDC32, int x, int y)
{
	return MM_DPtoLP_X(hDC32, x, y);
}

int (MM_DPtoLP_Y)(HDC32 hDC32, int x, int y)
{
	return MM_DPtoLP_Y(hDC32, x, y);
}

/****************************************************************************/

DWORD
lsd_mm_passthru(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_point(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	if (lpStruct)
	{
		LPPOINT lppt = &lpStruct->lsde.point;
		int x = MM_LPtoDP_X(hDC32,lppt->x,lppt->y),
		    y = MM_LPtoDP_Y(hDC32,lppt->x,lppt->y);
		lppt->x = x;
		lppt->y = y;
	}
	return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_moveto(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	if (lpStruct)
	{
		LPPOINT lppt = &lpStruct->lsde.point;
		DWORD dwRet;
		int x,y;

		x = MM_LPtoDP_X(hDC32,lppt->x,lppt->y),
		y = MM_LPtoDP_Y(hDC32,lppt->x,lppt->y);

		lppt->x = x;
		lppt->y = y;
		if ((dwRet = DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct)))
		{
		    x = MM_DPtoLP_X(hDC32,lppt->x,lppt->y);
		    y = MM_DPtoLP_Y(hDC32,lppt->x,lppt->y);
		    lppt->x = x;
		    lppt->y = y;
		}
		return dwRet;
	}
	return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_lineto(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	if (lpStruct)
	{
		LPPOINT lppt = &lpStruct->lsde.point;
		if (dwParam == ABSOLUTE)
		{
			int x = MM_LPtoDP_X(hDC32,lppt->x,lppt->y),
			    y = MM_LPtoDP_Y(hDC32,lppt->x,lppt->y);
			lppt->x = x;
			lppt->y = y;
		}
		else
		{	/* RELATIVE */
			lppt->x = MM_SCALE_X(hDC32,lppt->x);
			lppt->y = MM_SCALE_Y(hDC32,lppt->y);
		}
	}
	return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_rectangle(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	if (lpStruct)
	{
		LPRECT lprc = &lpStruct->lsde.rect;
		int left = MM_LPtoDP_X(hDC32,lprc->left,lprc->top),
		    top = MM_LPtoDP_Y(hDC32,lprc->left,lprc->top),
		    right = MM_LPtoDP_X(hDC32,lprc->right,lprc->bottom),
		    bottom = MM_LPtoDP_Y(hDC32,lprc->right,lprc->bottom);
		lprc->left = left;
		lprc->top = top;
		lprc->right = right;
		lprc->bottom = bottom;
	}
	return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_roundrect(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
	if (lpStruct)
	{
		LPPOINT lprr = lpStruct->lsde.arc;
		int i;
		for ( i = 0; i < 2; i++, lprr++)
		{
			int x = MM_LPtoDP_X(hDC32,lprr->x,lprr->y),
			    y = MM_LPtoDP_Y(hDC32,lprr->x,lprr->y);
			lprr->x = x;
			lprr->y = y;
		}
		/* last point is ellipse width/height */
		lprr->x = MM_SCALE_X(hDC32,lprr->x);
		lprr->y = MM_SCALE_Y(hDC32,lprr->y);
	}
	return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

/* 
 * this is passed an array of POINTs 
 * and passes the modified array down...
 */
DWORD
lsd_mm_poly(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    LSDE_POLYPOLY *lppp = &lpStruct->lsde.polypoly;
    int i;
    LPPOINT sp,tp;

    if (nNumPoints < lppp->nTotalCount * 2) {
	nNumPoints = lppp->nTotalCount * 2;
	if (!ArrayPoints)
	    ArrayPoints = (LPPOINT)WinMalloc(nNumPoints * 2 * sizeof(POINT));
	else
	    ArrayPoints = (LPPOINT)WinRealloc((LPSTR)ArrayPoints,
					nNumPoints * 2 * sizeof(POINT));
    }
    tp = ArrayPoints;
    sp = (LPPOINT)lppp->lpPoints;

#ifdef LATER
    /* Someday someone has to check how REL/ABS really works; */
    /* for now I pretend it doesn't exist, though the code is here... */
    if ((dwParam == RELATIVE) && (msg == LSD_POLYLINE)) {

	int x = MM_LPtoDP_X(hDC32,sp->x,sp->y),
	    y = MM_LPtoDP_Y(hDC32,sp->x,sp->y);
	tp->x = x;
	tp->y = y;
	tp++; sp++;

	for(i=0; i<lppp->nTotalCount; i++,sp++,tp++) {
	    tp->x = MM_SCALE_X(hDC32,sp->x);
	    tp->y = MM_SCALE_Y(hDC32,sp->y);
	}
    else 	/* ABSOLUTE and/or LSD_POLYGON */
#endif
    for(i=0; i<lppp->nTotalCount; i++,sp++,tp++) {
	int x = MM_LPtoDP_X(hDC32,sp->x,sp->y),
	    y = MM_LPtoDP_Y(hDC32,sp->x,sp->y);
	tp->x = x;
	tp->y = y;
    }

    lppp->lpPoints = ArrayPoints;
    return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_arc(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    LPPOINT lprr = lpStruct->lsde.arc;
    int i;

    for ( i = 0; i < 4; i++, lprr++ ) {
	int x = MM_LPtoDP_X(hDC32,lprr->x,lprr->y),
	    y = MM_LPtoDP_Y(hDC32,lprr->x,lprr->y);
	lprr->x = x;
	lprr->y = y;
    }

    return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_extfloodfill(WORD msg,HDC32 hDC32,DWORD dwParam,LPLSDS_PARAMS lpStruct)
{
    LSDE_FLOODFILL *lpff = &lpStruct->lsde.floodfill;
    int x = MM_LPtoDP_X(hDC32,lpff->nXStart,lpff->nYStart),
	y = MM_LPtoDP_Y(hDC32,lpff->nXStart,lpff->nYStart);
    lpff->nXStart = x;
    lpff->nYStart = y;

    return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_stretchblt(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    LSDE_STRETCHDATA *lpsd = &lpStruct->lsde.stretchdata;
    int x = MM_LPtoDP_X(hDC32, lpsd->xDest, lpsd->yDest),
	y = MM_LPtoDP_Y(hDC32, lpsd->xDest, lpsd->yDest);
    lpsd->xDest = x;
    lpsd->yDest = y;
    lpsd->nWidthDest = MM_SCALE_X(hDC32,lpsd->nWidthDest);
    lpsd->nHeightDest = MM_SCALE_Y(hDC32,lpsd->nHeightDest);
    
    /*  This is a bandaid to somewhat counter the missing effects of */
    /*  MM_SCALE_{X,Y}.  If either scale factor is negative and the */
    /*  extent parameter is negative, take the absolute value. */
    /*    Weav  15 May 1996 */
    if ((hDC32->VEy ^ hDC32->WEy) < 0  && lpsd->nHeightDest < 0)
      lpsd->nHeightDest = abs(lpsd->nHeightDest);
    if ((hDC32->VEx ^ hDC32->WEx) < 0  && lpsd->nWidthDest < 0)
      lpsd->nWidthDest = abs(lpsd->nWidthDest);

    /* We cannot convert source coordinates here, because there might */
    /* be a case when the source DC has a mapping mode, whereas the   */
    /* destination DC does not, so we never get here. From now on,    */
    /* the output routine will have to take care of it.		      */

    return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_setdibtodevice(WORD msg, HDC32 hDC32, DWORD dwParam,
				LPLSDS_PARAMS lpStruct)
{
    LSDE_STRETCHDATA *lpsd = &lpStruct->lsde.stretchdata;
    int x = MM_LPtoDP_X(hDC32, lpsd->xDest, lpsd->yDest),
	y = MM_LPtoDP_Y(hDC32, lpsd->xDest, lpsd->yDest);

    lpsd->xDest = x;
    lpsd->yDest = y;

    /* extents are passed in pixels, not in logical values */

    return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_scrolldc(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    LSDE_SCROLLDC *lpsd = &lpStruct->lsde.scrolldc;
    int x, y;

    x = MM_LPtoDP_X(hDC32, lpsd->xSrc, lpsd->ySrc),
    y = MM_LPtoDP_Y(hDC32, lpsd->xSrc, lpsd->ySrc);
    lpsd->xSrc = x;
    lpsd->ySrc = y;
    x = MM_LPtoDP_X(hDC32, lpsd->xDest, lpsd->yDest);
    y = MM_LPtoDP_Y(hDC32, lpsd->xDest, lpsd->yDest);
    lpsd->xDest = x;
    lpsd->yDest = y;
    lpsd->nWidth = MM_SCALE_X(hDC32,lpsd->nWidth);
    lpsd->nHeight = MM_SCALE_Y(hDC32,lpsd->nHeight);

    return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

/* LPtoDP and DPtoLP do translation without DOx/DOy, because user is
   not supposed to see these things. The final translation will be done
   during the actual operation.
*/
BOOL WINAPI 
LPtoDP(HDC hDC, LPPOINT lpPoint, int n)
{
    HDC32 hDC32;
    int i;

    APISTR((LF_APICALL,"LPtoDP(HDC=%x,LPPOINT=%p,int=%d)\n",
		hDC,lpPoint,n));

    ASSERT_HDC(hDC32,hDC,FALSE);

    for (i = 0; i < n; i++,lpPoint++) {
	int x = MM0_LPtoDP_X(hDC32, lpPoint->x, lpPoint->y),
	    y = MM0_LPtoDP_Y(hDC32, lpPoint->x, lpPoint->y);
	lpPoint->x = x;
	lpPoint->y = y;
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"LPtoDP: returns BOOL 1\n"));
    return TRUE;
}

BOOL WINAPI 
DPtoLP(HDC hDC, LPPOINT lpPoint, int n)
{
    HDC32 hDC32;
    int i;

    APISTR((LF_APICALL,"DPtoLP(HDC=%x,LPPOINT=%p,int=%d)\n",
		hDC,lpPoint,n));

    ASSERT_HDC(hDC32,hDC,FALSE);

    for (i = 0; i < n; i++,lpPoint++) {
	int x = MM0_DPtoLP_X(hDC32, lpPoint->x, lpPoint->y),
	    y = MM0_DPtoLP_Y(hDC32, lpPoint->x, lpPoint->y);
	lpPoint->x = x;
	lpPoint->y = y;
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"DPtoLP: returns BOOL 1\n"));
    return TRUE;

}

BOOL
LEtoDE(HDC32 hDC32, LPSIZE lpSize)
{
    APISTR((LF_APICALL,"LEtoDE(HDC=%x,LPSIZE=%p)\n",hDC32,lpSize));

    lpSize->cx = MM_SCALE_X(hDC32, lpSize->cx);
    lpSize->cy = MM_SCALE_Y(hDC32, lpSize->cy);

    APISTR((LF_APIRET,"LEtoDE: returns BOOL 1\n"));
    return TRUE;
}

BOOL
DEtoLE(HDC32 hDC32, LPSIZE lpSize)
{
    APISTR((LF_APICALL,"DEtoLE(HDC=%x,LPSIZE=%p)\n",hDC32,lpSize));

    lpSize->cx = MM_RESCALE_X(hDC32, lpSize->cx);
    lpSize->cy = MM_RESCALE_Y(hDC32, lpSize->cy);

    APISTR((LF_APIRET,"DEtoLE: returns BOOL 1\n"));
    return TRUE;
}

/* lsd_mm_exttextout CANNOT TOUCH LSDS_TEXT->lpRect!!! */
/* lsd_display_exttextout assumes logical coordinates!!! */
DWORD
lsd_mm_exttextout(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    LSDE_TEXT *lptext = &lpStruct->lsde.text;
    int x,y;

    /* modify the x,y coordinates... */
    x = MM_LPtoDP_X(hDC32, lptext->x, lptext->y);
    y = MM_LPtoDP_Y(hDC32, lptext->x, lptext->y);
    lptext->x = x;
    lptext->y = y;

    if (msg == LSD_EXTTEXTOUT && hDC32->MapMode != MM_TEXT && lptext->lpDX) {
	LPINT lpDX = lptext->lpDX;
	int i;
	for (i = 0; i < lptext->nCnt; i++,lpDX++)
	    *lpDX = MM_SCALE_X(hDC32, *lpDX);
    }

    return DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_mm_gettextmetrics(WORD msg,HDC32 hDC32,DWORD dwParam,LPLSDS_PARAMS lpStruct)
{
    DWORD dwRet;

    dwRet = DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);

    if (dwRet && hDC32->MapMode != MM_TEXT)
	MM_RescaleNEWTEXTMETRIC(hDC32,
			(LPNEWTEXTMETRIC)lpStruct->lsde.lpmetric,0);

    return dwRet;
}

void
MM_RescaleNEWTEXTMETRIC(HDC32 hDC32,LPNEWTEXTMETRIC lpntm, int ttFlag)
{
    lpntm->tmHeight          = MM_RESCALE_Y(hDC32,lpntm->tmHeight);
    lpntm->tmAscent          = MM_RESCALE_Y(hDC32,lpntm->tmAscent);
    lpntm->tmDescent         = MM_RESCALE_Y(hDC32,lpntm->tmDescent);
    lpntm->tmInternalLeading = MM_RESCALE_Y(hDC32,lpntm->tmInternalLeading);
    lpntm->tmExternalLeading = MM_RESCALE_Y(hDC32,lpntm->tmExternalLeading);
    lpntm->tmAveCharWidth    = MM_RESCALE_X(hDC32,lpntm->tmAveCharWidth);
    lpntm->tmMaxCharWidth    = MM_RESCALE_X(hDC32,lpntm->tmMaxCharWidth);
    lpntm->tmOverhang        = MM_RESCALE_X(hDC32,lpntm->tmOverhang);
    if (ttFlag) {
    }
}

DWORD
lsd_mm_getcharwidth(WORD msg,HDC32 hDC32,DWORD dwParam,LPLSDS_PARAMS lpStruct)
{
    DWORD dwRet;
    LSDE_TEXT *lptext = &lpStruct->lsde.text;

    dwRet = DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);

    if (dwRet && hDC32->MapMode != MM_TEXT) {
	if (dwParam) {				/* GetCharABCWidths */
	    LPABC lpABC = (LPABC)lptext->lpDX;
	    int i;
	    for (i = (int)lptext->x; i <= (int)lptext->y; i++,lpABC++) {
		lpABC->abcA = MM_RESCALE_X(hDC32,lpABC->abcA);
		lpABC->abcB = MM_RESCALE_X(hDC32,lpABC->abcB);
		lpABC->abcC = MM_RESCALE_X(hDC32,lpABC->abcC);
	    }
	}
	else {					/* GetCharWidth */
	    LPINT lpWidths = lptext->lpDX;
	    int i;
	    for (i = (int)lptext->x; i <= (int)lptext->y; i++,lpWidths++)
		*lpWidths = MM_RESCALE_X(hDC32,*lpWidths);
	}
    }

    return dwRet;
}

DWORD
lsd_mm_getclipbox(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    DWORD dwRet;
    LPRECT lprc = &lpStruct->lsde.rect;

    dwRet = DC_MM_OUTPUT(msg,hDC32,dwParam,lpStruct);

    if ((int)dwRet > NULLREGION) {
	int left	= MM0_DPtoLP_X(hDC32, lprc->left, lprc->top),
	    top		= MM0_DPtoLP_Y(hDC32, lprc->left, lprc->top),
	    right	= MM0_DPtoLP_X(hDC32, lprc->right, lprc->bottom),
	    bottom	= MM0_DPtoLP_Y(hDC32, lprc->right, lprc->bottom);
	lprc->left	= left;
	lprc->top	= top;
	lprc->right	= right;
	lprc->bottom	= bottom;
    }

    return dwRet;
}

DWORD
lsd_mm_setboundsrect(WORD msg,HDC32 hDC32,DWORD dwParam,LPLSDS_PARAMS lpStruct)
{
	FatalAppExit(0,"lsd_mm_setboundsrect!!!");
	return(0);   /* Eliminates compiler warning */
}

DWORD
lsd_mm_getboundsrect(WORD msg,HDC32 hDC32,DWORD dwParam,LPLSDS_PARAMS lpStruct)
{
	FatalAppExit(0,"lsd_mm_getboundsrect!!!");
	return(0);   /* Eliminates compiler warning */
}

DWORD
lsd_mm_worldblt(WORD wMsg,
	HDC32 hDC32,
	DWORD dwParam,
	LPLSDS_PARAMS lpParam)
{
	LPLSDE_WORLDBLT lpWorldBltParam = &(lpParam->lsde.worldblt);
	int i, x, y;

	/* map destination parallelogram coordinates */
	for (i = 0; i < 3; i++)
	{
		x = MM_LPtoDP_X(hDC32,
			lpWorldBltParam->ptDestXY[i].x,
			lpWorldBltParam->ptDestXY[i].y);
		y = MM_LPtoDP_Y(hDC32,
			lpWorldBltParam->ptDestXY[i].x,
			lpWorldBltParam->ptDestXY[i].y);
		lpWorldBltParam->ptDestXY[i].x = x;
		lpWorldBltParam->ptDestXY[i].y = y;
	}

	/* We cannot map source rectangle coordinates here because
	 * this code only gets called when the destination DC has a
	 * mapping mode.  The solution is to let the output routine
	 * perform source rectangle mapping.
	 */

	return DC_MM_OUTPUT(wMsg, hDC32, dwParam, lpParam);

}
