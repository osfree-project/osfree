/*    
	GdiGraphics.c	2.56
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
#include <stdlib.h>
#include <string.h>

#include "windows.h"

#include "kerndef.h"
#include "ObjEngine.h"
#include "Log.h"
#include "GdiObjects.h" /* LPIMAGEINFO */
#include "GdiDC.h"
#include "DeviceData.h"
#include "GdiRegions.h"

POINT MAKEPOINT(DWORD);

static BOOL InsertDeletePath(HDC32, int, int);

int WINAPI
GetDeviceCaps(HDC hDC,int nIndex)
{
    HDC32 hDC32;
    int nRet;

    APISTR((LF_APICALL,"GetDeviceCaps(HDC=%x,int=%x)\n",hDC,nIndex));

    ASSERT_HDC(hDC32,hDC,0);

    nRet = (int) DC_OUTPUT(LSD_GETDEVICECAPS,hDC32,nIndex,0);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"GetDeviceCaps: returns int %d\n",nRet));
    return (nRet);

}

COLORREF	WINAPI
GetNearestColor(HDC hDC, COLORREF crColor)
{
	HDC32 hDC32;
	LSDS_PARAMS lsdsParams;
	COLORREF crNearestColor;

	APISTR((LF_APICALL, "GetNearestColor(HDC=%x,COLORRET=%x)\n", 
		hDC, crColor));

	ASSERT_HDC(hDC32, hDC, CLR_INVALID);

	lsdsParams.lsde_validate.dwInvalidMask = 0;
	lsdsParams.lsde.colorref = crColor;
	crNearestColor = (COLORREF)
		DC_OUTPUT(LSD_GETNEARCLR, hDC32, 0L, &lsdsParams);

	RELEASEDCINFO(hDC32);

	APISTR((LF_APIRET, "GetNearestColor: returns COLORREF %x\n",
		crNearestColor));
	return (crNearestColor);

}

DWORD  WINAPI
MoveTo(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_APICALL,"MoveTo(HDC=%x,int=%d,int=%d)\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,0);

    if (hDC32->nPathState == 1)
    {
	if (!InsertDeletePath(hDC32, hDC32->nPathSize, 1))
	{
		SetLastErrorEx(1, 0);
		RELEASEDCINFO(hDC32);
		dwRet = GetCurrentPosition(hDC);
    		APISTR((LF_APIRET,"MoveTo: returns DWORD %x\n",dwRet));
		return dwRet;
	}
	hDC32->lpPathTypes[hDC32->nPathSize-1] = PT_MOVETO;
	hDC32->lpPathPoints[hDC32->nPathSize-1].x = x;
	hDC32->lpPathPoints[hDC32->nPathSize-1].y = y;
    }

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_MOVETO,hDC32,LSDM_SET,&argptr);
    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"MoveTo: returns DWORD %x\n",dwRet));
    return (dwRet);

}

BOOL WINAPI
MoveToEx(HDC hDC,int x, int y, LPPOINT lppt)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;

    APISTR((LF_APICALL,"MoveToEx(HDC=%x,int=%d,int=%d,LPPOINT=%p)\n",
	hDC,x,y,lppt));

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (hDC32->nPathState == 1)
    {
	if (!InsertDeletePath(hDC32, hDC32->nPathSize, 1))
	{
		SetLastErrorEx(1, 0);
		RELEASEDCINFO(hDC32);
    		APISTR((LF_APIRET,"MoveToEx: returns BOOL 0\n"));
		return (FALSE);
	}
	hDC32->lpPathTypes[hDC32->nPathSize-1] = PT_MOVETO;
	hDC32->lpPathPoints[hDC32->nPathSize-1].x = x;
	hDC32->lpPathPoints[hDC32->nPathSize-1].y = y;
    }

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_MOVETO,hDC32,LSDM_SET,&argptr);
    if (lppt)
	*lppt = argptr.lsde.point;

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"MoveToEx: returns BOOL 1\n"));
    return TRUE;

}

DWORD WINAPI
GetCurrentPosition(HDC hDC)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_APICALL,"GetCurrentPosition(HDC=%x)\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.point.x = argptr.lsde.point.y = 0;
    DC_OUTPUT(LSD_MOVETO,hDC32,LSDM_GET,&argptr);
    dwRet = (DWORD) MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"GetCurrentPosition: returns DWORD %x\n",dwRet));
    return (dwRet);

}

BOOL WINAPI
GetCurrentPositionEx(HDC hDC, LPPOINT lppt)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"GetCurrentPositionEx(HDC=%x,LPPOINT=%p)\n",
	hDC,lppt));

    ASSERT_HDC(hDC32,hDC,FALSE);

    bRet = FALSE;
    if (lppt) {
	argptr.lsde_validate.dwInvalidMask = 0;
	DC_OUTPUT(LSD_MOVETO,hDC32,LSDM_GET,&argptr);
	*lppt = argptr.lsde.point;
	bRet = TRUE;
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"GetCurrentPositionEx: returns BOOL %d\n",bRet));
    return (bRet);

}


BOOL WINAPI
LineTo(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"LineTo(HDC=%x,int=%x,int=%x)\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (hDC32->nPathState == 1)
    {
	if (!InsertDeletePath(hDC32, hDC32->nPathSize, 1))
	{
		SetLastErrorEx(1, 0);
		RELEASEDCINFO(hDC32);
    		APISTR((LF_APIRET,"LineTo: returns BOOL 0\n"));
		return (FALSE);
	}
	hDC32->lpPathTypes[hDC32->nPathSize-1] = PT_LINETO;
	hDC32->lpPathPoints[hDC32->nPathSize-1].x = x;
	hDC32->lpPathPoints[hDC32->nPathSize-1].y = y;
	RELEASEDCINFO(hDC32);
    	APISTR((LF_APIRET,"LineTo: returns BOOL 1\n"));
	return (TRUE);
    }

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    bRet = (BOOL) DC_OUTPUT(LSD_LINETO,hDC32,hDC32->RelAbs,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"LineTo: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
Polyline(HDC hDC, const POINT *lpPts, int nCount)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"Polyline(HDC=%x,POINT *=%p,int=%d)\n",
	hDC,lpPts,nCount));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.polypoly.lpPoints = (LPPOINT)lpPts;
    argptr.lsde.polypoly.lpCounts = (LPINT)0;
    argptr.lsde.polypoly.nCount = 1;
    argptr.lsde.polypoly.nTotalCount = nCount;
    bRet = (BOOL) DC_OUTPUT(LSD_POLYLINE,hDC32,hDC32->RelAbs,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"Polyline: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL   WINAPI
Rectangle(HDC hDC, int nLeft, int nTop, int nRight, int nBottom)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"Rectangle(HDC=%x,int=%d,int=%d,int=%d,int=%d\n",
		hDC,nLeft,nTop,nRight,nBottom));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.rect.left = nLeft;
    argptr.lsde.rect.top = nTop;
    argptr.lsde.rect.right = nRight;
    argptr.lsde.rect.bottom = nBottom;
    bRet = (BOOL) DC_OUTPUT(LSD_RECTANGLE,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"Rectangle: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL   WINAPI
RoundRect(HDC hDC, int nLeft, int nTop,
	int nRight, int nBottom, int nEllipseWidth, int nEllipseHeight)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,
	"RoundRect(HDC=%x,int=%d,int=%d,int=%d,int=%d,int=%d,int=%d\n",
	hDC,nLeft,nTop,nRight,nBottom,nEllipseWidth,nEllipseHeight));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.arc[0].x = nLeft;
    argptr.lsde.arc[0].y = nTop;
    argptr.lsde.arc[1].x = nRight;
    argptr.lsde.arc[1].y = nBottom;
    argptr.lsde.arc[2].x = nEllipseWidth;
    argptr.lsde.arc[2].y = nEllipseHeight;
    bRet = (BOOL) DC_OUTPUT(LSD_ROUNDRECT,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET, "RoundRect: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI 
Ellipse(HDC hDC, int nLeft, int nTop, int nRight, int nBottom)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"Ellipse(HDC=%x,int=%d,int=%d,int=%d,int=%d)\n",
		hDC,nLeft,nTop,nRight,nBottom));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.rect.left = nLeft;
    argptr.lsde.rect.top = nTop;
    argptr.lsde.rect.right = nRight;
    argptr.lsde.rect.bottom = nBottom;
    bRet = (BOOL) DC_OUTPUT(LSD_ELLIPSE,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"Ellipse: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI 
Arc(HDC hDC, int nLeft, int nTop, int nRight, int nBottom,
	int nXStart, int nYStart, int nXEnd, int nYEnd)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,
	"Arc(HDC=%x,int=%d,int=%d,int=%d,int=%d,int=%d,int=%d,int=%d,int=%d)\n",
	hDC,nLeft,nTop,nRight,nBottom,nXStart,nYStart,nXEnd,nYEnd));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.arc[0].x = nLeft;
    argptr.lsde.arc[0].y = nTop;
    argptr.lsde.arc[1].x = nRight;
    argptr.lsde.arc[1].y = nBottom;
    argptr.lsde.arc[2].x = nXStart;
    argptr.lsde.arc[2].y = nYStart;
    argptr.lsde.arc[3].x = nXEnd;
    argptr.lsde.arc[3].y = nYEnd;
    bRet = (BOOL) DC_OUTPUT(LSD_ARC,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"Arc: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
Chord(HDC hDC, int nLeft, int nTop, int nRight, int nBottom,
	int nXStart, int nYStart, int nXEnd, int nYEnd)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,
	"Chord(HDC=%x,int=%d,int=%d,int=%d,int=%d,int=%d,int=%d,int=%d,int=%d)\n",
	hDC,nLeft,nTop,nRight,nBottom,nXStart,nYStart,nXEnd,nYEnd));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.arc[0].x = nLeft;
    argptr.lsde.arc[0].y = nTop;
    argptr.lsde.arc[1].x = nRight;
    argptr.lsde.arc[1].y = nBottom;
    argptr.lsde.arc[2].x = nXStart;
    argptr.lsde.arc[2].y = nYStart;
    argptr.lsde.arc[3].x = nXEnd;
    argptr.lsde.arc[3].y = nYEnd;
    bRet = (BOOL) DC_OUTPUT(LSD_CHORD,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"Chord: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
Pie(HDC hDC, int nLeft, int nTop, int nRight, int nBottom,
	int nXStart, int nYStart, int nXEnd, int nYEnd)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,
	"Pie(HDC=%x,int=%d,int=%d,int=%d,int=%d,int=%d,int=%d,int=%d,int=%d\n",
	hDC,nLeft,nTop,nRight,nBottom,nXStart,nYStart,nXEnd,nYEnd));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.arc[0].x = nLeft;
    argptr.lsde.arc[0].y = nTop;
    argptr.lsde.arc[1].x = nRight;
    argptr.lsde.arc[1].y = nBottom;
    argptr.lsde.arc[2].x = nXStart;
    argptr.lsde.arc[2].y = nYStart;
    argptr.lsde.arc[3].x = nXEnd;
    argptr.lsde.arc[3].y = nYEnd;
    bRet = (BOOL) DC_OUTPUT(LSD_PIE,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"Pie: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
Polygon(HDC hDC, const POINT *lpPts, int nCount)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"Polygon(HDC=%x,POINT *=%p,int=%d\n",hDC,lpPts,nCount));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.polypoly.lpPoints = (LPPOINT)lpPts;
    argptr.lsde.polypoly.lpCounts = (LPINT)0;
    argptr.lsde.polypoly.nCount = 1;
    argptr.lsde.polypoly.nTotalCount = nCount;
    bRet = (BOOL) DC_OUTPUT(LSD_POLYGON,hDC32,hDC32->RelAbs,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"Polygon: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
PolyPolygon(HDC hDC, const POINT *lpPts, LPINT lpPoly,int nCount)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int i,n;
    BOOL bRet;

    APISTR((LF_APICALL,
	"PolyPolygon(HDC=%x,POINT *=%p,LPINT=%p,int=%d\n",
	hDC,lpPts,lpPoly,nCount));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    for(i=0, n=0; i < nCount; i++) 
	n += lpPoly[i];
    argptr.lsde.polypoly.lpPoints = (LPPOINT)lpPts;
    argptr.lsde.polypoly.lpCounts = lpPoly;
    argptr.lsde.polypoly.nCount = nCount;
    argptr.lsde.polypoly.nTotalCount = n;
    bRet = (BOOL) DC_OUTPUT(LSD_POLYPOLYGON,hDC32,hDC32->RelAbs,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"PolyPolygon: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
FloodFill(HDC hDC,int nXStart,int nYStart,COLORREF clrref)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"FloodFill(HDC=%x,int=%d,int=%d,COLORREF=%x)\n",
	hDC,nXStart,nYStart,clrref));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.floodfill.nXStart = nXStart;
    argptr.lsde.floodfill.nYStart = nYStart;
    argptr.lsde.floodfill.cr = clrref;
    argptr.lsde.floodfill.fuFillType = FLOODFILLBORDER;
    bRet = (BOOL) DC_OUTPUT(LSD_FLOODFILL,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"FloodFill: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
ExtFloodFill(HDC hDC,int nXStart,int nYStart,COLORREF clrref,UINT fuFillType)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,
	"ExtFloodFill(HDC=%x,int=%d,int=%d,COLORREF=%x,UINT=%x)\n",
	hDC,nXStart,nYStart,clrref,fuFillType));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.floodfill.nXStart = nXStart;
    argptr.lsde.floodfill.nYStart = nYStart;
    argptr.lsde.floodfill.cr = clrref;
    argptr.lsde.floodfill.fuFillType = fuFillType;
    bRet = (BOOL) DC_OUTPUT(LSD_EXTFLOODFILL,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"ExtFloodFill: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
FillRgn(HDC hDC,HRGN hRgn,HBRUSH hBrush)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"FillRgn(HDC=%x,HRGN=%x,HBRUSH=%x)\n",
		hDC,hRgn,hBrush));

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (!IsRegion(hRgn))
	bRet = FALSE;
    else {
	argptr.lsde_validate.dwInvalidMask = 0;
	argptr.lsde.paintrgn.hRgn = hRgn;
	argptr.lsde.paintrgn.hBrush = hBrush;
	argptr.lsde.paintrgn.dwROP = PATCOPY;
	bRet = (BOOL) DC_OUTPUT(LSD_FILLRGN,hDC32,0,&argptr);
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"FillRgn: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
FrameRgn(HDC hDC,HRGN hRgn,HBRUSH hBrush,int nWidth,int nHeight)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"FrameRgn(HDC=%x,HRGN=%x,HBRUSH=%x,int=%d,int=%d)\n",
		hDC,hRgn,hBrush,nWidth,nHeight));

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (!IsRegion(hRgn))
	bRet = FALSE;
    else {
	argptr.lsde_validate.dwInvalidMask = 0;
	argptr.lsde.paintrgn.hRgn = hRgn;
	argptr.lsde.paintrgn.hBrush = hBrush;
	argptr.lsde.paintrgn.nWidth = nWidth;
	argptr.lsde.paintrgn.nHeight = nHeight;
	argptr.lsde.paintrgn.dwROP = PATCOPY;
	bRet = (BOOL) DC_OUTPUT(LSD_FRAMERGN,hDC32,0,&argptr);
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"FrameRgn: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
InvertRgn(HDC hDC, HRGN hRgn)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"InvertRgn(HDC=%x,HRGN=%x)\n",hDC,hRgn));

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (!IsRegion(hRgn))
	bRet = FALSE;
    else {
	argptr.lsde_validate.dwInvalidMask = 0;
	argptr.lsde.paintrgn.hRgn = hRgn;
	argptr.lsde.paintrgn.hBrush = (HBRUSH)0;
	argptr.lsde.paintrgn.dwROP = DSTINVERT;
	bRet = (BOOL) DC_OUTPUT(LSD_INVERTRGN,hDC32,0,&argptr);
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"InvertRgn: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
PaintRgn(HDC hDC,HRGN hRgn)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_APICALL,"PaintRgn(HDC=%x,HRGN=%x)\n",hDC,hRgn));

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (!IsRegion(hRgn))
	bRet = FALSE;
    else {
	argptr.lsde_validate.dwInvalidMask = 0;
	argptr.lsde.paintrgn.hRgn = hRgn;
	argptr.lsde.paintrgn.hBrush = (HBRUSH)0;
	argptr.lsde.paintrgn.dwROP = PATCOPY;
	bRet = (BOOL) DC_OUTPUT(LSD_PAINTRGN,hDC32,0,&argptr);
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"PaintRgn: returns BOOL %d\n",bRet));
    return (bRet);

}

BOOL WINAPI
FillRect(HDC hDC, const RECT *lpRect, HBRUSH hBrush)
{
    HBRUSH hBrushOld;
    BOOL bRet = FALSE;
    int nWidth, nHeight;
    HDC32 hDC32;

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (!lpRect) {
    	APISTR((LF_APIFAIL,"FillRect: returns BOOL %d\n",bRet));
	return bRet;
    }

    APISTR((LF_APICALL,"FillRect(HDC=%x,RECT *%p,HBRUSH=%x)\n",
	hDC, lpRect, hBrush));

    if (!(hBrushOld = SelectObject(hDC,hBrush))) {

    	APISTR((LF_APIFAIL,"FillRect: returns BOOL %d\n",bRet));
	return bRet;
    }

    /*  Account for decreasing-to-the-right-or-bottom coord systems */
    nWidth = lpRect->right-lpRect->left;
    nHeight = lpRect->bottom-lpRect->top;

    /*  If the XOR of the extents shows negative, the product will be negative,
	thus a negative-going coord system.  Correct for it. */
    if ((hDC32->WEx ^ hDC32->VEx) < 0) 
    /* if (nWidth < 0) */
      nWidth = -nWidth;

    if ((hDC32->WEy ^ hDC32->VEy) < 0) 
    /* if (nHeight < 0) */
      nHeight = -nHeight;

    /* if height and width STILL show negative it's an error; don't draw. */
    if (nWidth < 0  || nHeight < 0) {
    	APISTR((LF_APIFAIL,"FillRect: returns BOOL %d\n",bRet));
        return bRet;  /* value is not used and has no meaning */
    }

    bRet = PatBlt(hDC,lpRect->left,lpRect->top,
		nWidth,nHeight,PATCOPY);

    SelectObject(hDC,hBrushOld);

    APISTR((LF_APIRET,"FillRect: returns BOOL %d\n",bRet));
    return bRet;
}

BOOL WINAPI
FrameRect(HDC hDC, const RECT *lpRect, HBRUSH hBrush)
{
    HBRUSH hBrushOld;
    BOOL bRet;

    if (!lpRect)
	return FALSE;

    APISTR((LF_API,"FrameRect: hDC=%x, rect %d,%d %d,%d, hBrush %x\n",
	hDC,
	lpRect->left,lpRect->top,lpRect->right,lpRect->bottom,
	hBrush));

    if (!(hBrushOld = SelectObject(hDC,hBrush)))
	return FALSE;

    /* do the top bar */	
    bRet = PatBlt(hDC, lpRect->left, lpRect->top, 
		lpRect->right - lpRect->left,
		1, PATCOPY);

    if (bRet) {
	/* do the bottom bar */	
	PatBlt(hDC, lpRect->left, lpRect->bottom-1, 
		lpRect->right - lpRect->left,
		1, PATCOPY);

	/* do the left bar */	
	PatBlt(hDC, lpRect->left, lpRect->top, 
		1,
		lpRect->bottom-lpRect->top, PATCOPY);

	/* do the right bar */	
	PatBlt(hDC, lpRect->right-1, lpRect->top, 
		1,
		lpRect->bottom-lpRect->top, PATCOPY);
    }

    SelectObject(hDC,hBrushOld);

    return bRet;
}

void WINAPI
InvertRect(HDC hDC, const RECT *lpRect)
{
    if (!lpRect)
	return;

    APISTR((LF_API,"InvertRect: hDC %x rect %d,%d %d,%d\n",hDC,
	lpRect->left,lpRect->top,lpRect->right,lpRect->bottom));

    PatBlt(hDC,lpRect->left,lpRect->top,
		lpRect->right-lpRect->left, 
		lpRect->bottom-lpRect->top, 
		DSTINVERT);
}


BOOL
FastWindowFrame(HDC hDC, LPRECT lpRect, int xWidth, int yWidth, DWORD dwROP3)
{
    BOOL bResult;
    int nWidth,nHeight;

    if (!lpRect || IsRectEmpty(lpRect))
	return FALSE;

    APISTR((LF_API,"FastWindowFrame: hDC %x rect %d,%d %d,%d\n",hDC,
	lpRect->left,lpRect->top,lpRect->right,lpRect->bottom));
    APISTR((LF_API,"\tpen x/y %d/%d rop %x",xWidth,yWidth,dwROP3));

    nWidth = lpRect->right - lpRect->left;
    nHeight = lpRect->bottom - lpRect->top;

    if ((bResult = PatBlt(hDC,lpRect->left,lpRect->top,nWidth,yWidth,dwROP3))){
	PatBlt(hDC,lpRect->right-xWidth,lpRect->top,
				xWidth,nHeight,dwROP3);
	PatBlt(hDC,lpRect->left,lpRect->bottom-yWidth,
				nWidth,yWidth,dwROP3);
	PatBlt(hDC,lpRect->left,lpRect->top,
				xWidth,nHeight,dwROP3);
    }

    return bResult;
}

void WINAPI
DrawFocusRect(HDC hDC, const RECT *lprc)
{
    static HPEN	hPen = (HPEN)NULL;
    int nOldROP,nOldBkMode;
    HPEN hOldPen;
    HDC32 hDC32;

    if (!lprc)
	return;

    APISTR((LF_APICALL,"DrawFocusRect(HDC=%x,RECT *%x)\n",
	hDC,lprc));

    if (!(hDC32 = GETDCINFO(hDC))) {
	APISTR((LF_APIFAIL,"DrawFocusRect: returns void\n"));
	return;
    }

    if (!hPen)
	hPen = CreatePen(PS_DOT,1,RGB(255,255,255));

    nOldROP = SetROP2(hDC, R2_NOTMASKPEN);
    nOldBkMode = SetBkMode(hDC,TRANSPARENT);
    hOldPen = SelectObject(hDC,hPen);

    MoveTo(hDC,lprc->left,lprc->top);
    LineTo(hDC,lprc->right-1,lprc->top);
    LineTo(hDC,lprc->right-1,lprc->bottom-1);
    LineTo(hDC,lprc->left,lprc->bottom-1);
    LineTo(hDC,lprc->left,lprc->top);

    SetROP2(hDC, nOldROP);
    if (nOldBkMode != TRANSPARENT)
	SetBkMode(hDC,nOldBkMode);
    SelectObject(hDC, hOldPen);

    APISTR((LF_APIRET,"DrawFocusRect: returns void\n"));
}

BOOL WINAPI
GrayString(HDC hDC, HBRUSH hBr, GRAYSTRINGPROC gsprc, LPARAM lParam,
		int cch, int x, int y, int cx, int cy)
{
    COLORREF	rgbSystemGray;
    COLORREF	crTextOld;
    BOOL	bRet;
    HDC32	hDC32;

    APISTR((LF_APICALL,
	"GrayString(HDC=%x,HBRUSH=%x,GRAYSTRINGPROC=%x,LPARAM=%x,int=%x,int=%x,int=%x,int=%x,int=%x)\n",
	hDC,hBr,gsprc,lParam,cch,x,y,cx,cy));

    ASSERT_HDC(hDC32,hDC,FALSE);

#ifdef LATER
    if (gsprc) {
	APISTR((LF_APIFAIL,"GrayString: returns BOOL FAIL\n"));
	return FALSE;
    }
#endif

    rgbSystemGray = GetSysColor(COLOR_GRAYTEXT);
    crTextOld = SetTextColor(hDC,rgbSystemGray);

    bRet = TextOut(hDC,x,y,(LPCSTR)lParam,
	(cch)?cch:strlen((LPSTR)lParam));

    SetTextColor(hDC,crTextOld);

    APISTR((LF_APIRET,"GrayString: returns BOOL %d\n",bRet));
    return bRet;
}

DWORD WINAPI
GetAspectRatioFilter(HDC hDC)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"GetAspectRatioFilter: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;
    dwRet = (DC_OUTPUT(LSD_GETASPECTRATIO,hDC32,0,&argptr)) ?
	MAKELONG(argptr.lsde.size.cx,argptr.lsde.size.cy) : 0;

    RELEASEDCINFO(hDC32);

    return (dwRet);

}

BOOL WINAPI
GetAspectRatioFilterEx(HDC hDC,LPSIZE lpAspectRatio)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_API,"GetAspectRatioFilterEx: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,FALSE);

    bRet = FALSE;
    argptr.lsde_validate.dwInvalidMask = 0;
    if (lpAspectRatio && DC_OUTPUT(LSD_GETASPECTRATIO,hDC32,0,&argptr)) {
	*lpAspectRatio = argptr.lsde.size;
	bRet = TRUE;
    }

    RELEASEDCINFO(hDC32);

    return (bRet);

}

#if	0
/* ********************* NEW PatBlt() ************************************** */
BOOL	WINAPI
PatBlt(HDC hDC, int nX, int nY, int nWidth, int nHeight, DWORD dwRop)
{
	LPDC lpDC;
	LSDS_PARAMS lsdsParam;
	BOOL bSuccess;

	APISTR((LF_API, "PatBlt: hDC %x\n"
		" nX %x nY %x\n"
		" nWidth %x nHeight %x\n"
		" dwRop %x\n",
		hDC, nX, nY, nWidth, nHeight, dwRop));

	ASSERT_DC(lpDC, hDC, FALSE);

	lsdsParam.lsde_validate.dwInvalidMask = 0;
	lsdsParam.lsde.worldblt.ptDestXY[0].x = nX;
	lsdsParam.lsde.worldblt.ptDestXY[0].y = nY;
	lsdsParam.lsde.worldblt.ptDestXY[1].x = nX + nWidth;
	lsdsParam.lsde.worldblt.ptDestXY[1].y = nY;
	lsdsParam.lsde.worldblt.ptDestXY[2].x = nX;
	lsdsParam.lsde.worldblt.ptDestXY[2].y = nY + nHeight;
	lsdsParam.lsde.worldblt.lpSrcDC = NULL;
	lsdsParam.lsde.worldblt.nSrcX = 0;
	lsdsParam.lsde.worldblt.nSrcY = 0;
	lsdsParam.lsde.worldblt.nSrcWidth = 0;
	lsdsParam.lsde.worldblt.nSrcHeight = 0;
	lsdsParam.lsde.worldblt.hMaskBitmap = NULL;
	lsdsParam.lsde.worldblt.nMaskX = 0;
	lsdsParam.lsde.worldblt.nMaskY = 0;
	lsdsParam.lsde.worldblt.dwRop = dwRop;
	bSuccess = (BOOL) DC_OUTPUT(LSD_PATBLT, lpDC, 0, &lsdsParam);

	UNLOCK_DC(hDC);

	return (bSuccess);

}
#endif	/* NEW */

#if	1
/* ********************* OLD PatBlt() ************************************** */
BOOL WINAPI
PatBlt(HDC hDC, int X, int Y, int nWidth,int nHeight, DWORD dwRop)
{
    HDC32 hDC32;
    LSDS_PARAMS	argptr;
    BOOL bRet;

    APISTR((LF_API,
	"PatBlt: hDC %x dest %d,%d size %d,%d rop %x\n",
	hDC,X,Y,nWidth,nHeight,dwRop));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;
    argptr.lsde.stretchdata.xDest = X;
    argptr.lsde.stretchdata.nWidthDest = nWidth;
    argptr.lsde.stretchdata.yDest = Y;
    argptr.lsde.stretchdata.nHeightDest = nHeight;
    argptr.lsde.stretchdata.hSrcDC32 = 0;
    argptr.lsde.stretchdata.xSrc = 0;
    argptr.lsde.stretchdata.ySrc = 0;
    argptr.lsde.stretchdata.nWidthSrc = 0;
    argptr.lsde.stretchdata.nHeightSrc = 0;
    argptr.lsde.stretchdata.dwRop = dwRop;
    bRet = (BOOL) DC_OUTPUT(LSD_PATBLT,hDC32,0,&argptr);

    RELEASEDCINFO(hDC32);

    return (bRet);

}
#endif	/* OLD */

#if	0
/* ******************** NEW BitBlt() *************************************** */
BOOL	WINAPI 
BitBlt(HDC hDestDC,
	int nDestX, int nDestY,
	int nWidth, int nHeight,
	HDC hSrcDC,
	int nSrcX, int nSrcY,
	DWORD dwRop)
{
	LPDC lpDestDC, lpSrcDC;
	LSDS_PARAMS lsdsParam;
	BOOL bSuccess;

	APISTR((LF_API, "BitBlt: hDestDC %x\n"
		" nDestX %x nDestY %x\n"
		" nWidth %x nHeight %x\n"
		" hSrcDC %x\n"
		" nSrcX %x nSrcY %x\n"
		" dwRop %x\n",
		hDestDC, nDestX, nDestY,
		nWidth, nHeight,
		hSrcDC, nSrcX, nSrcY,
		dwRop));

	ASSERT_DC(lpDestDC, hDestDC, FALSE);

	lpSrcDC = NULL;
	if ((hSrcDC != NULL) && ((lpSrcDC = LOCK_DC(hSrcDC)) == NULL))
	{
		UNLOCK_DC(hDestDC);
		return (FALSE);
	}

	lsdsParam.lsde_validate.dwInvalidMask = 0;
	lsdsParam.lsde.worldblt.ptDestXY[0].x = nDestX;
	lsdsParam.lsde.worldblt.ptDestXY[0].y = nDestY;
	lsdsParam.lsde.worldblt.ptDestXY[1].x = nDestX + nWidth;
	lsdsParam.lsde.worldblt.ptDestXY[1].y = nDestY;
	lsdsParam.lsde.worldblt.ptDestXY[2].x = nDestX;
	lsdsParam.lsde.worldblt.ptDestXY[2].y = nDestY + nHeight;
	lsdsParam.lsde.worldblt.lpSrcDC = lpSrcDC;
	lsdsParam.lsde.worldblt.nSrcX = nSrcX;
	lsdsParam.lsde.worldblt.nSrcY = nSrcY;
	lsdsParam.lsde.worldblt.nSrcWidth = nWidth;
	lsdsParam.lsde.worldblt.nSrcHeight = nHeight;
	lsdsParam.lsde.worldblt.hMaskBitmap = NULL;
	lsdsParam.lsde.worldblt.nMaskX = 0;
	lsdsParam.lsde.worldblt.nMaskY = 0;
	lsdsParam.lsde.worldblt.dwRop = dwRop;
	bSuccess = (BOOL) DC_OUTPUT(LSD_BITBLT, lpDestDC, 0, &lsdsParam);

	if (hSrcDC)
		UNLOCK_DC(hSrcDC);

	UNLOCK_DC(hDestDC);

	return (bSuccess);

}
#endif	/* NEW */

#if	1
/* ******************** OLD BitBlt() *************************************** */

static BOOL BitBltDIBSection(HDC hDestDC,
	int nX, int nY,
	int nWidth, int nHeight,
	HDC hSrcDC,
	int nSrcX, int nSrcY,
	DWORD dwROP)
{
	HDC32 hDestDC32, hSrcDC32;
	HBITMAP hDestBitmap, hSrcBitmap;
	LPIMAGEINFO lpSrcImageInfo;

	if (dwROP != SRCCOPY)
		return (FALSE);

	ASSERT_HDC(hDestDC32, hDestDC, FALSE);
	hDestBitmap = hDestDC32->hBitmap;
	RELEASEDCINFO(hDestDC32);

	ASSERT_HDC(hSrcDC32, hSrcDC, FALSE);
	hSrcBitmap = hSrcDC32->hBitmap;
	RELEASEDCINFO(hSrcDC32);

	if (!hSrcBitmap || !(lpSrcImageInfo = GETBITMAPINFO(hSrcBitmap)))
		return (FALSE);

	if (!lpSrcImageInfo->lpdsBmi)
	{
		RELEASEBITMAPINFO(lpSrcImageInfo);
		return (FALSE);
	}

	SetDIBitsToDevice(hDestDC, nX, nY, nWidth, nHeight, nSrcX, nSrcY,
		0, lpSrcImageInfo->ds.dsBmih.biHeight,
		lpSrcImageInfo->ds.dsBm.bmBits,
		lpSrcImageInfo->lpdsBmi,
		lpSrcImageInfo->dsUsage);

	RELEASEBITMAPINFO(lpSrcImageInfo);

	return (TRUE);

}

BOOL WINAPI 
BitBlt(HDC hDCDest, int nXOriginDest, int nYOriginDest,
			int nWidthDest, int nHeightDest,
       HDC hDCSrc,  int nXOriginSrc, int nYOriginSrc, DWORD dwRop)
{
    HDC32 hDC32Dest;
    LSDS_PARAMS	argptr;

    APISTR((LF_API, "BitBlt: hDCSrc %x src %d,%d,"
	" hDCDest %x dest %d,%d size %d,%d rop %x\n",
	hDCSrc,nXOriginSrc,nYOriginSrc,
	hDCDest,nXOriginDest,nYOriginDest,nWidthDest,nHeightDest,
	dwRop));

    /* (WIN32) check for DIBSECTION bitblt */
    if (BitBltDIBSection(hDCDest, nXOriginDest, nYOriginDest,
	nWidthDest, nHeightDest, hDCSrc, nXOriginSrc, nYOriginSrc, dwRop))
		return (TRUE);

    ASSERT_HDC(hDC32Dest,hDCDest,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    if (hDCSrc) {
	ASSERT_HDC(argptr.lsde.stretchdata.hSrcDC32,hDCSrc,FALSE);
    }
    else
	argptr.lsde.stretchdata.hSrcDC32 = 0;

    argptr.lsde.stretchdata.xDest   = nXOriginDest;
    argptr.lsde.stretchdata.yDest   = nYOriginDest;
    argptr.lsde.stretchdata.nWidthDest = nWidthDest;
    argptr.lsde.stretchdata.nHeightDest = nHeightDest;
    argptr.lsde.stretchdata.xSrc    = nXOriginSrc;
    argptr.lsde.stretchdata.ySrc    = nYOriginSrc;
    argptr.lsde.stretchdata.nWidthSrc = nWidthDest;
    argptr.lsde.stretchdata.nHeightSrc = nHeightDest;
    argptr.lsde.stretchdata.dwRop   = dwRop;

    return (BOOL)DC_OUTPUT(LSD_BITBLT,hDC32Dest,0,&argptr);
}
#endif	/* OLD */

#if	0
/* ********************* NEW StretchBlt() ****************************** */
BOOL	WINAPI
StretchBlt(HDC hDestDC,
	int nDestX, int nDestY,
	int nDestWidth, int nDestHeight,
	HDC hSrcDC,
	int nSrcX, int nSrcY,
	int nSrcWidth, int nSrcHeight,
	DWORD dwRop)
{
	LPDC lpDestDC, lpSrcDC;
	LSDS_PARAMS lsdsParam;
	BOOL bSuccess;

	APISTR((LF_API, "StretchBlt: hDestDC %x\n"
		" nDestX %x nDestY %x\n"
		" nDestWidth %x nDestHeight %x\n"
		" hSrcDC %x\n"
		" nSrcX %x nSrcY %x\n"
		" nSrcWidth %x nSrcHeight %x\n"
		" dwRop %x\n",
		hDestDC, nDestX, nDestY, nDestWidth, nDestHeight,
		hSrcDC, nSrcX, nSrcY, nSrcWidth, nSrcHeight,
		dwRop));

	ASSERT_DC(lpDestDC, hDestDC, FALSE);

	lpSrcDC = NULL;
	if ((hSrcDC != NULL) && ((lpSrcDC = LOCK_DC(hSrcDC)) == NULL))
	{
		UNLOCK_DC(hDestDC);
		return (FALSE);
	}

	lsdsParam.lsde_validate.dwInvalidMask = 0;
	lsdsParam.lsde.worldblt.ptDestXY[0].x = nDestX;
	lsdsParam.lsde.worldblt.ptDestXY[0].y = nDestY;
	lsdsParam.lsde.worldblt.ptDestXY[1].x = nDestX + nDestWidth;
	lsdsParam.lsde.worldblt.ptDestXY[1].y = nDestY;
	lsdsParam.lsde.worldblt.ptDestXY[2].x = nDestX;
	lsdsParam.lsde.worldblt.ptDestXY[2].y = nDestY + nDestHeight;
	lsdsParam.lsde.worldblt.lpSrcDC = lpSrcDC;
	lsdsParam.lsde.worldblt.nSrcX = nSrcX;
	lsdsParam.lsde.worldblt.nSrcY = nSrcY;
	lsdsParam.lsde.worldblt.nSrcWidth = nSrcWidth;
	lsdsParam.lsde.worldblt.nSrcHeight = nSrcHeight;
	lsdsParam.lsde.worldblt.hMaskBitmap = NULL;
	lsdsParam.lsde.worldblt.nMaskX = 0;
	lsdsParam.lsde.worldblt.nMaskY = 0;
	lsdsParam.lsde.worldblt.dwRop = dwRop;
	bSuccess = (BOOL) DC_OUTPUT(LSD_STRETCHBLT, lpDestDC, 0, &lsdsParam);

	if (hSrcDC)
		UNLOCK_DC(hSrcDC);

	UNLOCK_DC(hDestDC);

	return (bSuccess);

}
#endif	/* NEW */

#if	1
/* ********************* OLD StretchBlt() ****************************** */
BOOL	WINAPI
StretchBlt(HDC hDCDest, int nXOriginDest, int nYOriginDest,
			int nWidthDest, int nHeightDest,
	   HDC hDCSrc,  int nXOriginSrc, int nYOriginSrc,
			int nWidthSrc, int nHeightSrc, DWORD dwRop)
{
    HDC32 hDC32Dest;
    LSDS_PARAMS	argptr;

    APISTR((LF_API,
	"StretchBlt:"
	" hDCSrc %x src %d,%d size %d,%d"
	" hDCDest %x dest %d,%d size %d,%d"
	" rop %x\n",
	hDCSrc,nXOriginSrc,nYOriginSrc,nWidthSrc,nHeightSrc,
	hDCDest,nXOriginDest,nYOriginDest,nWidthDest,nHeightDest,
	dwRop));

    ASSERT_HDC(hDC32Dest,hDCDest,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    if (hDCSrc) {
	ASSERT_HDC(argptr.lsde.stretchdata.hSrcDC32,hDCSrc,FALSE);
    }
    else
	argptr.lsde.stretchdata.hSrcDC32 = 0;

    argptr.lsde.stretchdata.xDest = nXOriginDest;
    argptr.lsde.stretchdata.yDest = nYOriginDest;
    argptr.lsde.stretchdata.nWidthDest = nWidthDest;
    argptr.lsde.stretchdata.nHeightDest = nHeightDest;
    argptr.lsde.stretchdata.xSrc = nXOriginSrc;
    argptr.lsde.stretchdata.ySrc = nYOriginSrc;
    argptr.lsde.stretchdata.nWidthSrc = nWidthSrc;
    argptr.lsde.stretchdata.nHeightSrc = nHeightSrc;
    argptr.lsde.stretchdata.dwRop = dwRop;

    return (BOOL)DC_OUTPUT(LSD_STRETCHBLT,hDC32Dest,0,&argptr);

}
#endif	/* OLD */

int	WINAPI
StretchDIBits(HDC hDC, int XDest, int YDest, int cxDest, int cyDest,
		int XSrc, int YSrc, int cxSrc, int cySrc,
		const void *lpvBits, LPBITMAPINFO lpbmi, UINT fuColorUse,
		DWORD dwRop)
{
    HDC32 hDC32;
    LSDS_PARAMS	argptr;
    int nRet;

    APISTR((LF_API,
	"StretchDIBits: hDC %x"
	" src %d,%d size %d,%d"
	" dest %d,%d size %d,%d\n",
	hDC, XSrc, YSrc, cxSrc, cySrc, XDest, YDest, cxDest, cyDest));

    APISTR((LF_API,
	"\tbits %x, lpbmi %x, use %x, rop %x\n",
	lpvBits, lpbmi, fuColorUse, dwRop));

    ASSERT_HDC(hDC32,hDC,0);

    if (lpbmi == NULL)
	nRet = 0;
    else {
	argptr.lsde_validate.dwInvalidMask = 0;
	argptr.lsde.stretchdata.xDest = XDest;
	argptr.lsde.stretchdata.yDest = YDest;
	argptr.lsde.stretchdata.nWidthDest = cxDest;
	argptr.lsde.stretchdata.nHeightDest = cyDest;
	argptr.lsde.stretchdata.hSrcDC32 = 0;
	argptr.lsde.stretchdata.xSrc = XSrc;

  /* Changed for smosaic port, needs more testing. */
  #if 0
  argptr.lsde.stretchdata.ySrc = YSrc;
  #endif
  argptr.lsde.stretchdata.ySrc = lpbmi->bmiHeader.biHeight - cySrc - YSrc;
  /* Changed for smosaic port, needs more testing. */

	argptr.lsde.stretchdata.nWidthSrc = cxSrc;
	argptr.lsde.stretchdata.nHeightSrc = cySrc;
	argptr.lsde.stretchdata.lpvBits = lpvBits;
	argptr.lsde.stretchdata.lpbmi = lpbmi;
	argptr.lsde.stretchdata.fuColorUse = fuColorUse;
	argptr.lsde.stretchdata.dwRop = dwRop;
	nRet = (int) DC_OUTPUT(LSD_STRETCHDIBITS,hDC32,0,&argptr);
    }

    RELEASEDCINFO(hDC32);

    return (nRet);

}

COLORREF WINAPI 
SetPixel(HDC hDC, int x, int y, COLORREF cr)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    COLORREF crRet;

    APISTR((LF_API,"SetPixel: hDC %x pt %d,%d color %x\n",
		hDC,x,y,cr));

    ASSERT_HDC(hDC32,hDC,-1);

    if (!PtVisible(hDC,x,y))
	crRet = (COLORREF) -1;
    else {
	argptr.lsde_validate.dwInvalidMask = 0;
	argptr.lsde.point.x = x;
	argptr.lsde.point.y = y;
	crRet = (COLORREF) DC_OUTPUT(LSD_SETPIXEL,hDC32,cr,&argptr);
    }

    RELEASEDCINFO(hDC32);

    return (crRet);

}

COLORREF WINAPI 
GetPixel(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    COLORREF crRet;

    APISTR((LF_API,"GetPixel: hDC %x pt %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,-1);

    if (!PtVisible(hDC,x,y))
	crRet = (COLORREF) -1;
    else {
	argptr.lsde_validate.dwInvalidMask = 0;
	argptr.lsde.point.x = x;
	argptr.lsde.point.y = y;
	crRet = (COLORREF) DC_OUTPUT(LSD_GETPIXEL,hDC32,0,&argptr);
    }

    RELEASEDCINFO(hDC32);

    return (crRet);

}

int     WINAPI 
SetDIBitsToDevice(HDC hDC, 
	int XDest, int YDest, int cxDest, int cyDest, int XSrc, int YSrc, 
	UINT startline, UINT numlines,
	LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT fuColorUse)
{
    HDC32 hDC32;
    LSDS_PARAMS	argptr;
    int nRet;

    APISTR((LF_API,
	"SetDIBitsToDevice: hDC %x, src %d,%d, dest %d,%d size %d,%d\n",
	hDC,XSrc,YSrc,
	XDest,YDest,cxDest,cyDest));
    APISTR((LF_API,
	"\tstartline %d numlines %d, bits %x, lpbmi %x, use %x\n",
	startline,numlines,lpvBits,lpbmi,fuColorUse));

    ASSERT_HDC(hDC32,hDC,0);

    if (lpbmi == 0)
	nRet = 0;
    else {
	argptr.lsde_validate.dwInvalidMask = 0;

#ifdef LATER
	snafu with cxDest, cyDest
#endif

	argptr.lsde.stretchdata.xDest = XDest;
	argptr.lsde.stretchdata.yDest = YDest;
	argptr.lsde.stretchdata.nWidthDest = cxDest;
	argptr.lsde.stretchdata.nHeightDest = cyDest;
	argptr.lsde.stretchdata.hSrcDC32 = 0;
	argptr.lsde.stretchdata.xSrc = XSrc;
	argptr.lsde.stretchdata.ySrc = YSrc;
	argptr.lsde.stretchdata.nWidthSrc = 0;
	argptr.lsde.stretchdata.nHeightSrc = 0;
	argptr.lsde.stretchdata.startline = startline;
	argptr.lsde.stretchdata.numlines = numlines;
	argptr.lsde.stretchdata.lpvBits = lpvBits;
	argptr.lsde.stretchdata.lpbmi = lpbmi;
	argptr.lsde.stretchdata.fuColorUse = fuColorUse;
	nRet = (int) DC_OUTPUT(LSD_SETDIBTODEVICE,hDC32,0,&argptr);
    }

    RELEASEDCINFO(hDC32);

    return (nRet);

}

BOOL  WINAPI
DrawIcon(HDC hDC, int x, int y, HICON hIcon)
{
    LPTWIN_ICONINFO lpIconInfo;
    HDC hCompatDC;
    int cxIcon,cyIcon;
    HBITMAP hOldBitmap;
    COLORREF bg,fg;
    BOOL bRet;

    APISTR((LF_API,"DrawIcon: hDC=%x %d,%d hIcon %x\n",hDC,x,y,hIcon));

    if (!hIcon || !(lpIconInfo = GETICONINFO(hIcon)))
	return (DWORD)FALSE;

    if (!lpIconInfo->hXORImage || !lpIconInfo->hANDMask)
	return (DWORD)FALSE;

    if (!(hCompatDC = CreateCompatibleDC(hDC)))
	return (DWORD)FALSE;

    cxIcon = GetSystemMetrics(SM_CXICON);
    cyIcon = GetSystemMetrics(SM_CYICON);

    bg = SetBkColor(hDC,RGB(255,255,255));
    fg = SetTextColor(hDC,RGB(0,0,0));

    hOldBitmap = SelectObject(hCompatDC,lpIconInfo->hANDMask);
    bRet = BitBlt(hDC,x,y,cxIcon,cyIcon,hCompatDC,0,0,SRCAND);
    if (bRet) {
	SelectObject(hCompatDC,lpIconInfo->hXORImage);
	bRet = BitBlt(hDC,x,y,cxIcon,cyIcon,hCompatDC,0,0,SRCINVERT);
    }

    SelectObject(hCompatDC,hOldBitmap);
    DeleteDC(hCompatDC);

    SetBkColor(hDC,bg);
    SetTextColor(hDC,fg);

    return bRet;
}

HGDIOBJ WINAPI
SelectObject(HDC hDC, HGDIOBJ hObject)
{
    HDC32 hDC32;
    HGDIOBJ hOldObject;

    APISTR((LF_APICALL,"SelectObject(HDC=%x,HGDIOBJ=%x)\n", hDC, hObject));

    ASSERT_HDC(hDC32, hDC, 0);

    /* Some people try to select value previously returned from */
    /* SelectObject on a region handle; allow this to fail quietly */
    if ((hObject >= (HGDIOBJ)NULLREGION)
     && (hObject <= (HGDIOBJ)COMPLEXREGION))
	hOldObject = (HGDIOBJ) 0;
    else {
	hOldObject = (HGDIOBJ) DC_OUTPUT(LSD_SELECTOBJECT,hDC32,hObject,0L);
    }

    RELEASEDCINFO(hDC32);

    APISTR((LF_APIRET,"SelectObject: returns HGDIOBJ %x\n", hOldObject));
    return (hOldObject);

}

HGDIOBJ WINAPI
GetCurrentObject(HDC hDC, UINT nType)
{
    HDC32 hDC32;
    HGDIOBJ hObject = (HGDIOBJ)NULL;

    APISTR((LF_API,"GetCurrentObject: hDC %x nType %x\n", hDC, nType));

    ASSERT_HDC(hDC32, hDC, 0);

    switch (nType) {
	case OBJ_PEN:
	    hObject = (HGDIOBJ)hDC32->hPen;
	    break;

	case OBJ_BRUSH:
	    hObject = (HGDIOBJ)hDC32->hBrush;
	    break;

#ifdef TWIN32

	case OBJ_PAL:
	    hObject = (HGDIOBJ)hDC32->hPalette;
	    break;

	case OBJ_FONT:
	    hObject = (HGDIOBJ)hDC32->hFont;
	    break;

	case OBJ_BITMAP:
	    hObject = (HGDIOBJ)hDC32->hBitmap;
	    break;

#endif /* TWIN32 */

	default:
	    break;
    }

    RELEASEDCINFO(hDC32);

    return hObject;
}

BOOL WINAPI
ScrollDC(HDC hDC, int dx, int dy, const RECT *lprcScroll,
	const RECT *lprcClip, HRGN hRgnUpdate, LPRECT lprcUpdate)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    RECT  rc,rcClientLog,rcClientPh;
    HRGN  hRgnDest,hRgnSrc;
    DWORD dwRet = FALSE;
    SIZE sz;

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    GetClientRect(hDC32->hWnd,&rcClientPh);
    sz.cx = rcClientPh.right;
    sz.cy = rcClientPh.bottom;
    DEtoLE(hDC32,&sz);
    rcClientLog.right = sz.cx;
    rcClientLog.bottom = sz.cy;
    rcClientLog.left = rcClientLog.top = 0;
    
/* All the rectangles involved are in logical coordinates */

    if (lprcScroll == 0) 
	lprcScroll = &rcClientLog;

    CopyRect(&rc,lprcScroll);

    OffsetRect(&rc,dx,dy);

    if(lprcClip == 0) 
	lprcClip = &rcClientLog;

    IntersectRect(&rc,&rc,lprcClip);

    if(IsRectEmpty(&rc))
	SetRect(&rc,0,0,0,0);
    else {
	argptr.lsde.scrolldc.xSrc = rc.left - dx;
	argptr.lsde.scrolldc.ySrc = rc.top  - dy;
	argptr.lsde.scrolldc.xDest = rc.left;
	argptr.lsde.scrolldc.yDest = rc.top;
	argptr.lsde.scrolldc.nWidth  = rc.right - rc.left;
	argptr.lsde.scrolldc.nHeight = rc.bottom - rc.top;
	dwRet = DC_OUTPUT(LSD_SCROLLDC,hDC32,0L,&argptr);
    }

/* All the regions involved are in physical coordinates */
    LPtoDP(hDC,(LPPOINT)&rc,2);
    LPtoDP(hDC,(LPPOINT)lprcScroll,2);
    hRgnDest = CreateRectRgnIndirect(&rc);
    hRgnSrc  = CreateRectRgnIndirect(lprcScroll);

    CombineRgn(hRgnSrc,hRgnSrc,hRgnDest,RGN_DIFF);

    if(hRgnUpdate) 
	CombineRgn(hRgnUpdate,hRgnSrc,0,RGN_COPY);
	
    if(lprcUpdate) 
	GetRgnBox(hRgnSrc,lprcUpdate);

    DeleteObject(hRgnSrc);
    DeleteObject(hRgnDest);
    return dwRet;	
}

/**********************************************************************/

static DWORD GdiPaintRopRgn(HDC32,LSDE_PAINTRGN *);
static DWORD GdiFrameRgn(HDC32,LSDE_PAINTRGN *);

/* LSD_FILLRGN, LSD_INVERTRGN, LSD_PAINTRGN come here */
DWORD
lsd_paintrgn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return GdiPaintRopRgn(hDC32,&lpStruct->lsde.paintrgn);
}

/* LSD_FRAMERGN comes here */
DWORD
lsd_framergn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return GdiFrameRgn(hDC32,&lpStruct->lsde.paintrgn);
}

/*******************************************************************/

#define	IsFunnyMapMode(hDC32) ((hDC32)->Output[0] == \
			       (hDC32)->lpLSDEntry->MapModeProc)


static DWORD
GdiFrameRgn(HDC32 hDC32, LSDE_PAINTRGN *lppr)
{
    HDC	    hDC;
    RECT    rcRect;
    HBRUSH  hOldBrush;

    hDC = GETHDC16(hDC32);

    /* get the bounding rect of the region for PATBLT */
    GetRgnBox(lppr->hRgn, &rcRect);

    hOldBrush = SelectObject(hDC,lppr->hBrush);	

    /* do the top bar */	
    PatBlt(hDC, rcRect.left - lppr->nWidth, rcRect.top - lppr->nHeight, 
		rcRect.right - rcRect.left + 2*lppr->nWidth, lppr->nHeight,
		lppr->dwROP);

    /* do the bottom bar */	
    PatBlt(hDC, rcRect.left-lppr->nWidth, rcRect.bottom, 
		rcRect.right - rcRect.left + 2*lppr->nWidth,
		lppr->nHeight, lppr->dwROP);

    /* do the left bar */	
    PatBlt(hDC, rcRect.left-lppr->nWidth, rcRect.top, 
		lppr->nWidth,
		rcRect.bottom-rcRect.top, lppr->dwROP);

    /* do the right bar */	
    PatBlt(hDC, rcRect.right, rcRect.top, 
		lppr->nWidth,
		rcRect.bottom-rcRect.top, lppr->dwROP);

    SelectObject(hDC,hOldBrush);
    return 1L;
}

static DWORD
GdiPaintRopRgn(HDC32 hDC32, LSDE_PAINTRGN *lppr)
{
    RECT 	rcRect;
    int		ret;
    HRGN	hRgn,hRgnClip;
    HRGN	hRgnMerged = 0;
    HBRUSH 	hOldBrush = 0;
    HDC		hDC,hMemoryDC;
    HBITMAP	hBmp,hOldBmp;

    hDC = GETHDC16(hDC32);
    hRgn = lppr->hRgn;

    /* get the bounding rect of the region for PATBLT */
    GetRgnBox(hRgn, &rcRect);

    if (IsRectEmpty(&rcRect))
	return 1L;

    /* get existing clipping region */
    /* if we have one, then AND it with our region */
    hRgnClip = CreateRectRgn(0,0,0,0);
    if (GetClipRgn(hDC,hRgnClip) == 1) {
	hRgnMerged = CreateRectRgn(0,0,0,0);
	/* combine the clipping rect with our region */
	ret = CombineRgn(hRgnMerged,hRgn,hRgnClip,RGN_AND);

	if (ret == NULLREGION || ret == ERROR) {
	    DeleteObject(hRgnClip);
	    DeleteObject(hRgnMerged);
	    return 0;
	}
	hRgn = hRgnMerged;
    }

    if (lppr->hBrush)
	hOldBrush = SelectObject(hDC,lppr->hBrush);

    /* if there is no mapping mode transform, select rgn as a clip */
    /* and do a PatBlt */
    if (!IsFunnyMapMode(hDC32)) {
	SelectClipRgn(hDC,hRgn);
	PatBlt(hDC, rcRect.left, rcRect.top, 
		rcRect.right - rcRect.left,
		rcRect.bottom - rcRect.top,
		lppr->dwROP);

	/* return old clipping back */
	SelectObject(hDC,hRgnClip);
    }
    else {
	hMemoryDC = CreateCompatibleDC(hDC);
	hBmp = CreateBitmap(rcRect.right-rcRect.left,
			rcRect.bottom-rcRect.top,
			1,8,(const void *)0);
	hOldBmp = SelectObject(hMemoryDC,hBmp);
	/* this fills the whole thing with zeros */
	PatBlt(hMemoryDC,
		0,0,
		rcRect.right-rcRect.left,
		rcRect.bottom-rcRect.top,
		BLACKNESS);
	SelectClipRgn(hMemoryDC,hRgn);
	/* make clip region zero-based */
	OffsetClipRgn(hMemoryDC,-rcRect.left,-rcRect.top);
	/* set bits inside the clipregion to 1's */
	PatBlt(hMemoryDC,
		0,0,
		rcRect.right-rcRect.left,
		rcRect.bottom-rcRect.top,
		WHITENESS);
	/* blit will perform mapmode conversions and stretch, if needed */
	BitBlt(hDC,
		rcRect.left,rcRect.top,
		rcRect.right-rcRect.left,
		rcRect.bottom-rcRect.top,
		hMemoryDC,
		0,0,
		(lppr->dwROP == PATCOPY)?
			0xe20746:	/* DSPDxax */
			SRCINVERT);
	SelectObject(hMemoryDC,hOldBmp);
	DeleteObject(hBmp);
	DeleteDC(hMemoryDC);
    }

    DeleteObject(hRgnClip);
    if (hRgnMerged)
	DeleteObject(hRgnMerged);

    if (lppr->hBrush)
	SelectObject(hDC,hOldBrush);

    return 1L;
}

/*
 *	pass the points through an array to the driver
 *	could use HIWORD/LOWORD, by building and tearing
 *	down a long to pack the data. This method allows
 *	you to extend to world coordinates later.
 */

POINT
MAKEPOINT(DWORD dw)
{
	POINT pt;

	pt.x = (int)(signed short)LOWORD(dw);
	pt.y = (int)(signed short)HIWORD(dw);
	return pt;
}

DWORD
lsd_setboundsrect(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    FatalAppExit(0,"SetBoundsRect!!!");
    return(0);  /* eliminates compiler warning */
}

DWORD
lsd_getboundsrect(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    FatalAppExit(0,"GetBoundsRect!!!");
    return(0);  /* eliminates compiler warning */
}

DWORD
lsd_noop(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return 0;
}

/* (WIN32) GDI Pixel Graphics ********************************************** */

BOOL	WINAPI
SetPixelV(HDC hDC,
	int nX, int nY,
	COLORREF crPixel)
{
	if (SetPixel(hDC, nX, nY, crPixel) == ((COLORREF)-1))
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}
	return (TRUE);
}

/* (WIN32) GDI Path Graphics *********************************************** */

static void
EllipseRadialAngle(int nRectLeft, int nRectTop,
	int nRectRight, int nRectBottom,
	double fAngle, int *nX, int *nY)
{
	/* compute intersection point between
	 *	- ellipse (bounded by rectangle)
	 * and
	 *	- radial line (at an angle measured CCW from +x axis)
	 */

	/* solving ellipse equation
	 *	(x/a)^2 + (y/b)^2 = 1
	 * and
	 *	x = r * cos(angle)
	 *	y = r * sin(angle)
	 * yields
	 *	r^2 = (a^2 * b^2) / (b^2 * cos(angle)^2 + a^2 * sin(angle)^2)
	 */

	double x0 = (nRectLeft + nRectRight) / 2.0,
	       y0 = (nRectTop + nRectBottom) / 2.0,
	       a = abs(nRectLeft - nRectRight) / 2.0,
	       b = abs(nRectTop - nRectBottom) / 2.0,
	       a_2 = a * a,
	       b_2 = b * b,
	       cos_angle = cos(fAngle),
	       sin_angle = sin(fAngle),
	       cos_angle_2 = cos_angle * cos_angle,
	       sin_angle_2 = sin_angle * sin_angle,
	       r = sqrt((a_2 * b_2) / (b_2 * cos_angle_2 + a_2 * sin_angle_2)),
	       x = r * cos_angle,
	       y = r * sin_angle;

	/* translate solution to (x0, y0) */
	*nX = (int) (x + x0);
	*nY = (int) (y0 - y);

}

static void
EllipseRadialPoint(int nRectLeft, int nRectTop,
	int nRectRight, int nRectBottom,
	int nPointX, int nPointY, int *nX, int *nY)
{
	/* compute intersection point between
	 *	- ellipse (bounded by rectangle)
	 * and
	 *	- radial line (from ellipse center to another point)
	 */
	double x0 = (nRectLeft + nRectRight) / 2.0,
	       y0 = (nRectTop + nRectBottom) / 2.0,
	       dx = ((double) nPointX) - x0,
	       dy = ((double) nPointY) - y0;

	EllipseRadialAngle(nRectLeft, nRectTop, nRectRight, nRectBottom,
		atan2(dy, dx), nX, nY);

}

#ifndef M_PI           /* Some math.h's do not define M_PI (Centerline, Mac) */
#ifdef _PI
#define M_PI _PI       /* In that case, they usually do define _PI, however. */
#else
#define M_PI        3.14159265358979323846      /* pi */
#endif
#endif

BOOL WINAPI
AngleArc(HDC hDC,
	int X, int Y,
	DWORD dwRadius,
	FLOAT eStartAngle, FLOAT eSweepAngle)
{
	int nArcStartX, nArcStartY, nArcEndX, nArcEndY;

	/* compute start point of elliptical arc */
	EllipseRadialAngle(X - ((int) dwRadius), Y - ((int) dwRadius),
		X + ((int) dwRadius), Y + ((int) dwRadius),
		M_PI*eStartAngle/180.0, &nArcStartX, &nArcStartY);

	/* compute end point of elliptical arc */
	EllipseRadialAngle(X - ((int) dwRadius), Y - ((int) dwRadius),
		X + ((int) dwRadius), Y + ((int) dwRadius),
		M_PI*(eStartAngle+eSweepAngle)/180.0, &nArcEndX, &nArcEndY);

	if (!LineTo(hDC, nArcStartX, nArcStartY))
		return (FALSE);
	if (!Arc(hDC, X-dwRadius, Y-dwRadius, X+dwRadius, Y+dwRadius,
		X + (int)(dwRadius*cos(M_PI*eStartAngle/180.0)),
		Y - (int)(dwRadius*sin(M_PI*eStartAngle/180.0)),
		X + (int)(dwRadius*cos(M_PI*(eStartAngle+eSweepAngle)/180.0)),
		Y - (int)(dwRadius*sin(M_PI*(eStartAngle+eSweepAngle)/180.0))))
		return (FALSE);
	if (!MoveToEx(hDC, nArcEndX, nArcEndY, NULL))
		return (FALSE);

	return (TRUE);

}

BOOL WINAPI
ArcTo(HDC hDC,
	int nRectLeft, int nRectTop, int nRectRight, int nRectBottom,
	int nXRadial1, int nYRadial1, int nXRadial2, int nYRadial2)
{
	HDC32 hDC32;
	int nArcStartX, nArcStartY, nArcEndX, nArcEndY;
	BOOL bRet;

	ASSERT_HDC(hDC32, hDC, FALSE);

	/* compute start point of elliptical arc */
	EllipseRadialPoint(nRectLeft, nRectTop, nRectRight, nRectBottom,
		nXRadial1, nYRadial1, &nArcStartX, &nArcStartY);

	/* compute end point of elliptical arc */
	EllipseRadialPoint(nRectLeft, nRectTop, nRectRight, nRectBottom,
		nXRadial2, nYRadial2, &nArcEndX, &nArcEndY);

	bRet = TRUE;
	if (!LineTo(hDC, nArcStartX, nArcStartY))
		bRet = FALSE;
	if (!Arc(hDC, nRectLeft, nRectTop, nRectRight, nRectBottom,
		nXRadial1, nYRadial1, nXRadial2, nYRadial2))
		bRet = FALSE;
	if (!MoveToEx(hDC, nArcEndX, nArcEndY, NULL))
		bRet = FALSE;

	RELEASEDCINFO(hDC32);

	return (bRet);

}

static int BezierPts(CONST POINT *pt, CONST POINT *pts, LPPOINT lpBezierPts)
{
	int dx, dy, x, y, n;
	double dt, t, t_2, t_3, f1, f2, f3, f4;

	dx = abs(pts[2].x - pt->x);
	dy = abs(pts[2].y - pt->y);
	dt = 1.0 / max(dx + 1, dy + 1);
	for (n = 0, t = dt; t <= 1.0; n++, t += dt)
	{
		t_2 = t * t;
		t_3 = t_2 * t;
		f1 = (1 - 3 * t + 3 * t_2 -     t_3);
		f2 = (    3 * t - 6 * t_2 + 3 * t_3);
		f3 = (            3 * t_2 - 3 * t_3);
		f4 = (                          t_3);
		x = f1 * ((double) pt->x)
			+ f2 * ((double) pts[0].x)
			+ f3 * ((double) pts[1].x)
			+ f4 * ((double) pts[2].x);
		y = f1 * ((double) pt->y)
			+ f2 * ((double) pts[0].y)
			+ f3 * ((double) pts[1].y)
			+ f4 * ((double) pts[2].y);
		if (lpBezierPts)
		{
			lpBezierPts[n].x = x;
			lpBezierPts[n].y = y;
		}
	}

	return (n);

}

BOOL WINAPI
PolyBezier(HDC hDC,
	CONST POINT *lpPoints,
	DWORD cCount)
{
	HDC32 hDC32;
	POINT pt;
	BOOL bRet;

	ASSERT_HDC(hDC32, hDC, FALSE);

	bRet = TRUE;
	if ((lpPoints == NULL)
	 || (cCount < 1)
	 || !MoveToEx(hDC, lpPoints->x, lpPoints->y, &pt)
	 || !PolyBezierTo(hDC, lpPoints + 1, cCount - 1)
	 || !MoveToEx(hDC, pt.x, pt.y, NULL)
	) bRet = FALSE;

	RELEASEDCINFO(hDC32);

	return (bRet);
	
}

BOOL WINAPI
PolyBezierTo(HDC hDC,
	CONST POINT *lpPoints,
	DWORD cCount)
{
	HDC32 hDC32;
	POINT pt;
	int i, n;
	LPPOINT lpBezierPts;

 	if (lpPoints == NULL)
 	{
 		SetLastErrorEx(1, 0);
 		return (FALSE);
 	}
 
	ASSERT_HDC(hDC32, hDC, FALSE);

 	if (hDC32->nPathState == 1)
 	{
 		if ((cCount % 3)
 		 || !InsertDeletePath(hDC32, hDC32->nPathSize, cCount))
 		{
 			SetLastErrorEx(1, 0);
 			RELEASEDCINFO(hDC32);
 			return (FALSE);
 		}
 		for (i = 0; i < cCount; i++)
 		{
 			hDC32->lpPathTypes[hDC32->nPathSize-cCount+i]
 				= PT_BEZIERTO;
 			hDC32->lpPathPoints[hDC32->nPathSize-cCount+i]
 				= lpPoints[i];
 		}
 		RELEASEDCINFO(hDC32);
 		return (TRUE);
 	}

	if (!GetCurrentPositionEx(hDC, &pt))
	{
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	for ( ; cCount >= 3; pt = lpPoints[2], lpPoints += 3, cCount -= 3)
	{
		n = BezierPts(&pt, lpPoints, NULL);
		if (!(lpBezierPts = (LPPOINT) WinMalloc(n * sizeof(POINT))))
			break;
		if (BezierPts(&pt, lpPoints, lpBezierPts) != n)
			break;
		for (i = 0; i < n; i++)
			LineTo(hDC, lpBezierPts[i].x, lpBezierPts[i].y);
		WinFree((LPSTR) lpBezierPts);
	}

	RELEASEDCINFO(hDC32);

	return (cCount == 0);

}

BOOL WINAPI
PolyDraw(HDC hDC,
	CONST POINT *lpPoints,
	CONST BYTE *lpbTypes,
	int nSize)
{
	HDC32 hDC32;
	BOOL bResult;
	POINT ptCloseFigure;

	ASSERT_HDC(hDC32, hDC, FALSE);

	/* save current point for PT_CLOSEFIGURE (if any) */
	if (!GetCurrentPositionEx(hDC, &ptCloseFigure))
	{
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	for (bResult = FALSE; nSize > 0; lpPoints++, lpbTypes++, nSize--)
	{
		switch (*lpbTypes & ~PT_CLOSEFIGURE)
		{
		case PT_MOVETO:
			bResult = MoveToEx(hDC, lpPoints->x, lpPoints->y, NULL);
			ptCloseFigure = *lpPoints;
			break;
		case PT_LINETO:
			bResult = LineTo(hDC, lpPoints->x, lpPoints->y);
			if (bResult && (*lpbTypes & PT_CLOSEFIGURE))
				LineTo(hDC, ptCloseFigure.x, ptCloseFigure.y);
			break;
		case PT_BEZIERTO:
			if ((nSize < 3)
			 || (lpbTypes[1] != PT_BEZIERTO)
			 || ((lpbTypes[2] & ~PT_CLOSEFIGURE) != PT_BEZIERTO))
			{
				bResult = FALSE;
				break;
			}
			bResult = PolyBezierTo(hDC, lpPoints, 3);
			if (bResult && (lpbTypes[2] & PT_CLOSEFIGURE))
				LineTo(hDC, ptCloseFigure.x, ptCloseFigure.y);
			lpPoints += 2;
			lpbTypes += 2;
			nSize -= 2;
			break;
		default:
			bResult = FALSE;
			break;
		}
		if (bResult == FALSE) break;
	}

	RELEASEDCINFO(hDC32);

	return (bResult);

}

BOOL WINAPI
PolylineTo(HDC hDC,
	CONST POINT *lpPoints,
	DWORD cCount)
{
	HDC32 hDC32;

	ASSERT_HDC(hDC32, hDC, FALSE);

	for ( ; cCount > 0; lpPoints++, cCount--)
		if (LineTo(hDC, lpPoints->x, lpPoints->y) == FALSE)
			break;

	RELEASEDCINFO(hDC32);

	return (cCount == 0);

}

BOOL WINAPI
PolyPolyline(HDC hDC,
	CONST POINT *lpPoints,
	CONST DWORD *lpNumPoints,
	DWORD dwNumPolylines)
{
	HDC32 hDC32;

	APISTR((LF_API, "PolyPolyline: hDC %x dwNumPolylines %d\n",
		hDC, dwNumPolylines));

	ASSERT_HDC(hDC32, hDC, FALSE);

	for ( ;
	     dwNumPolylines > 0;
	     lpPoints += *lpNumPoints++, dwNumPolylines--)
		if (Polyline(hDC, lpPoints, *lpNumPoints) == FALSE)
			break;

	RELEASEDCINFO(hDC32);

	return (dwNumPolylines == 0);

}

/* (WIN32) GDI BitBlt ****************************************************** */

BOOL	WINAPI
MaskBlt(HDC hDestDC,
	int nDestX, int nDestY,
	int nWidth, int nHeight,
	HDC hSrcDC,
	int nSrcX, int nSrcY,
	HBITMAP hMaskBitmap,
	int nMaskX, int nMaskY,
	DWORD dwRop)
{
	LPDC lpDestDC, lpSrcDC;
	LSDS_PARAMS lsdsParam;
	BOOL bSuccess;

	APISTR((LF_API, "MaskBlt: hDestDC %x\n"
		" nDestX %x nDestY %x\n"
		" nWidth %x nHeight %x\n"
		" hSrcDC %x\n"
		" nSrcX %x nSrcY %x\n"
		" hMaskBitmap %x\n"
		" nMaskX %x nMaskY %x\n"
		" dwRop %x\n",
		hDestDC, nDestX, nDestY,
		nWidth, nHeight,
		hSrcDC, nSrcX, nSrcY,
		hMaskBitmap, nMaskX, nMaskY,
		dwRop));

	ASSERT_DC(lpDestDC, hDestDC, FALSE);

	lpSrcDC = NULL;
	if ((hSrcDC != (HDC)NULL) && ((lpSrcDC = LOCK_DC(hSrcDC)) == NULL))
	{
		UNLOCK_DC(hDestDC);
		return (FALSE);
	}

	lsdsParam.lsde_validate.dwInvalidMask = 0;
	lsdsParam.lsde.worldblt.ptDestXY[0].x = nDestX;
	lsdsParam.lsde.worldblt.ptDestXY[0].y = nDestY;
	lsdsParam.lsde.worldblt.ptDestXY[1].x = nDestX + nWidth;
	lsdsParam.lsde.worldblt.ptDestXY[1].y = nDestY;
	lsdsParam.lsde.worldblt.ptDestXY[2].x = nDestX;
	lsdsParam.lsde.worldblt.ptDestXY[2].y = nDestY + nHeight;
	lsdsParam.lsde.worldblt.lpSrcDC = lpSrcDC;
	lsdsParam.lsde.worldblt.nSrcX = nSrcX;
	lsdsParam.lsde.worldblt.nSrcY = nSrcY;
	lsdsParam.lsde.worldblt.nSrcWidth = nWidth;
	lsdsParam.lsde.worldblt.nSrcHeight = nHeight;
	lsdsParam.lsde.worldblt.hMaskBitmap = hMaskBitmap;
	lsdsParam.lsde.worldblt.nMaskX = nMaskX;
	lsdsParam.lsde.worldblt.nMaskY = nMaskY;
	lsdsParam.lsde.worldblt.dwRop = dwRop;
	bSuccess = (BOOL) DC_OUTPUT(LSD_MASKBLT, lpDestDC, 0, &lsdsParam);

	if (hSrcDC)
		UNLOCK_DC(hSrcDC);

	UNLOCK_DC(hDestDC);

	return (bSuccess);

#if	0
	/* ***************** OLD MaskBlt() ********************************* */
	BITMAP bitmap;
	int nMaskWidth, nMaskHeight;
	HDC hAndDC, hXorDC;
	HBITMAP hOldAndBitmap, hOldXorBitmap, hXorBitmap;
	BOOL bSuccess;

	/* MaskBlt() functions exactly like BitBlt() with the foreground ROP
	 * if no mask bitmap is supplied.
	 */
	if (hMaskBitmap == NULL)
	{
		return BitBlt(hDestDC, nDestX, nDestY, nWidth, nHeight,
			hSrcDC, nSrcX, nSrcY, dwRop & 0x00ffffffL);
	}

	/* mask bitmap must be monochrome */
	if ((GetObject(hMaskBitmap, sizeof(BITMAP), &bitmap) == 0)
	 || (bitmap.bmPlanes != 1)
	 || (bitmap.bmBitsPixel != 1)
	 || (nMaskX < 0) || (nMaskX >= bitmap.bmWidth)
	 || (nMaskY < 0) || (nMaskY >= bitmap.bmHeight))
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}
	nMaskWidth = bitmap.bmWidth - nMaskX;
	nMaskHeight = bitmap.bmHeight - nMaskY;

	/* create memory DC for AND ROP */
	if ((hAndDC = CreateCompatibleDC(hDestDC)) == NULL)
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}
	if ((hOldAndBitmap = SelectObject(hAndDC, hMaskBitmap)) == NULL)
	{
		DeleteDC(hAndDC);
		SetLastErrorEx(1, 0);
		return (FALSE);
	}

	/* create memory DC for XOR ROP */
	if ((hXorDC = CreateCompatibleDC(hDestDC)) == NULL)
	{
		SelectObject(hAndDC, hOldAndBitmap);
		DeleteDC(hAndDC);
		SetLastErrorEx(1, 0);
		return (FALSE);
	}
	if ((hXorBitmap = CreateCompatibleBitmap(hDestDC,
		nMaskWidth, nMaskHeight)) == NULL)
	{
		DeleteDC(hXorDC);
		SelectObject(hAndDC, hOldAndBitmap);
		DeleteDC(hAndDC);
		SetLastErrorEx(1, 0);
		return (FALSE);
	}
	if ((hOldXorBitmap = SelectObject(hXorDC, hXorBitmap)) == NULL)
	{
		DeleteObject(hXorBitmap);
		DeleteDC(hXorDC);
		SelectObject(hAndDC, hOldAndBitmap);
		DeleteDC(hAndDC);
		SetLastErrorEx(1, 0);
		return (FALSE);
	}

	/* perform mask bitblt */

	/* destroy memory DC used for XOR ROP */
	SelectObject(hXorDC, hOldXorBitmap);
	DeleteObject(hXorBitmap);
	DeleteDC(hXorDC);

	/* destroy memory DC used for AND ROP */
	SelectObject(hAndDC, hOldAndBitmap);
	DeleteDC(hAndDC);

	/* return BitBlt() success code */
	return (bSuccess);

#endif	/* 0 */
}

BOOL	WINAPI
PlgBlt(HDC hDestDC,
	LPPOINT lpDestPoints,
	HDC hSrcDC,
	int nSrcX, int nSrcY,
	int nSrcWidth, int nSrcHeight,
	HBITMAP hMaskBitmap,
	int nMaskX, int nMaskY)
{
	LPDC lpDestDC, lpSrcDC;
	LSDS_PARAMS lsdsParam;
	BOOL bSuccess;

	APISTR((LF_API, "PlgBlt: hDestDC %x\n"
		" A.x %x A.y %x\n"
		" B.x %x B.y %x\n"
		" C.x %x C.y %x\n"
		" hSrcDC %x\n"
		" nSrcX %x nSrcY %x\n"
		" nSrcWidth %x nSrcHeight %x\n"
		" hMaskBitmap %x\n"
		" nMaskX %x nMaskY %x\n",
		hDestDC,
		lpDestPoints ? lpDestPoints[0].x : 0,
		lpDestPoints ? lpDestPoints[0].y : 0,
		lpDestPoints ? lpDestPoints[1].x : 0,
		lpDestPoints ? lpDestPoints[1].y : 0,
		lpDestPoints ? lpDestPoints[2].x : 0,
		lpDestPoints ? lpDestPoints[2].y : 0,
		hSrcDC, nSrcX, nSrcY,
		nSrcWidth, nSrcHeight,
		hMaskBitmap, nMaskX, nMaskY));

	if (lpDestPoints == NULL)
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}

	ASSERT_DC(lpDestDC, hDestDC, FALSE);

	lpSrcDC = NULL;
	if ((hSrcDC != (HDC)NULL) && ((lpSrcDC = LOCK_DC(hSrcDC)) == NULL))
	{
		UNLOCK_DC(hDestDC);
		return (FALSE);
	}

	lsdsParam.lsde_validate.dwInvalidMask = 0;
	lsdsParam.lsde.worldblt.ptDestXY[0] = lpDestPoints[0];
	lsdsParam.lsde.worldblt.ptDestXY[1] = lpDestPoints[1];
	lsdsParam.lsde.worldblt.ptDestXY[2] = lpDestPoints[2];
	lsdsParam.lsde.worldblt.lpSrcDC = lpSrcDC;
	lsdsParam.lsde.worldblt.nSrcX = nSrcX;
	lsdsParam.lsde.worldblt.nSrcY = nSrcY;
	lsdsParam.lsde.worldblt.nSrcWidth = nSrcWidth;
	lsdsParam.lsde.worldblt.nSrcHeight = nSrcHeight;
	lsdsParam.lsde.worldblt.hMaskBitmap = hMaskBitmap;
	lsdsParam.lsde.worldblt.nMaskX = nMaskX;
	lsdsParam.lsde.worldblt.nMaskY = nMaskY;
	lsdsParam.lsde.worldblt.dwRop = MAKEROP4(SRCCOPY, 0x00AA0029L);
	bSuccess = (BOOL) DC_OUTPUT(LSD_PLGBLT, lpDestDC, 0, &lsdsParam);

	if (hSrcDC)
		UNLOCK_DC(hSrcDC);

	UNLOCK_DC(hDestDC);

	return (bSuccess);

#if 0
	/* ******************* OLD PlgBlt() ******************************** */
	/* The parallelogram specified by the LPPOINT array can be simulated
	 * by using a sequence of translate/scale/shear/translate world
	 * transform concatenations.  Of course, you should save/restore the
	 * original world transform.
	 *
	 * Otherwise, this call is similar to MaskBlt().
	 *
	 */

	XFORM xform, xform0, xform1, xform2, xform3;
	double sx, sy, shx, shy;
	BOOL bSuccess;
	BITMAP bitmap;
	int i, j, nMaskWidth, nMaskHeight;

#ifdef DEBUG
	POINT save;
#endif

	/* save original world transform */
	if (!GetWorldTransform(hDestDC, &xform))
		return (FALSE);
	xform0 = xform;

	/* concatenate translate transform with world transform */
	xform1.eM11 = 1.0;
	xform1.eM21 = 0.0;
	xform1.eDx  = -lpDestPoints[0].x;
	xform1.eM12 = 0.0;
	xform1.eM22 = 1.0;
	xform1.eDy  = -lpDestPoints[0].y;
	if (!CombineTransform(&xform0, &xform0, &xform1))
		return (FALSE);

	/* concatenate scale/shear transform with world transform */
	sx = (lpDestPoints[1].x - lpDestPoints[0].x) / ((FLOAT) nWidth);
	sy = (lpDestPoints[2].y - lpDestPoints[0].y) / ((FLOAT) nHeight);
	shx = (lpDestPoints[2].x - lpDestPoints[0].x) / ((FLOAT) nHeight);
	shy = (lpDestPoints[1].y - lpDestPoints[0].y) / ((FLOAT) nWidth);
	xform2.eM11 = sx;
	xform2.eM21 = shx;
	xform2.eDx  = 0.0;
	xform2.eM12 = shy;
	xform2.eM22 = sy;
	xform2.eDy  = 0.0;
	if (!CombineTransform(&xform0, &xform0, &xform2))
		return (FALSE);

	/* concatenate translate transform with world transform */
	xform3.eM11 = 1.0;
	xform3.eM21 = 0.0;
	xform3.eDx  = +lpDestPoints[0].x;
	xform3.eM12 = 0.0;
	xform3.eM22 = 1.0;
	xform3.eDy  = +lpDestPoints[0].y;
	if (!CombineTransform(&xform0, &xform0, &xform3))
		return (FALSE);

	/* modify world transform */
	if (!SetWorldTransform(hDestDC, &xform0))
		return (FALSE);

#ifdef DEBUG
	/* DEBUG: draw top and left edges of parallelogram */
	if (!MoveToEx(hDestDC, lpDestPoints[0].x, lpDestPoints[0].y, &save)
	 || !LineTo(hDestDC, lpDestPoints[0].x + nWidth, lpDestPoints[0].y)
	 || !MoveToEx(hDestDC, lpDestPoints[0].x, lpDestPoints[0].y, NULL)
	 || !LineTo(hDestDC, lpDestPoints[0].x, lpDestPoints[0].y + nHeight)
	 || !MoveToEx(hDestDC, save.x, save.y, NULL))
		return (FALSE);
#endif

	/* use MaskBlt() to implement masking function
	 *
	 * MaskBlt() will not tile mask bitmap to fill src/dst rectangle,
	 * but PlgBlt() will tile mask bitmap if necessary.  (Hence the
	 * for loop instead of just one call to MaskBlt().)
	 */
	bSuccess = TRUE;
	nMaskWidth = nWidth;
	nMaskHeight = nHeight;
	if ((hMaskBitmap != NULL)
	 && (nMaskX > 0)
	 && (nMaskY > 0)
	 && (GetObject(hMaskBitmap, sizeof(BITMAP), &bitmap) != 0)
	 && (bitmap.bmPlanes == 1)
	 && (bitmap.bmBitsPixel == 1))
	{
		nMaskWidth = bitmap.bmWidth - nMaskX;
		nMaskHeight = bitmap.bmHeight - nMaskY;
		nMaskWidth = max(0, nMaskWidth);
		nMaskHeight = max(0, nMaskHeight);
		nMaskWidth = min(nMaskWidth, nWidth);
		nMaskHeight = min(nMaskHeight, nHeight);
	}
	for (i = 0; bSuccess && (i < nWidth); i += nMaskWidth)
		for (j = 0; bSuccess && (j < nHeight); j += nMaskHeight)
			bSuccess = MaskBlt(hDestDC,
				lpDestPoints[0].x + i,
				lpDestPoints[0].y + j,
				min(nMaskWidth, nWidth - i),
				min(nMaskHeight, nHeight - j),
				hSrcDC, nSrcX + i, nSrcY + j,
				hMaskBitmap, nMaskX, nMaskY,
				MAKEROP4(SRCCOPY, 0x00AA0029L));

	/* restore original world transform */
	if (!SetWorldTransform(hDestDC, &xform))
		return (FALSE);

	/* return MaskBlt() success code */
	return (bSuccess);

#endif	/* 0 */
}



/* (WIN32) GDI Path ******************************************************** */

BOOL	WINAPI
BeginPath(HDC hDC)
{
	HDC32 hDC32;
	BOOL bSuccess;

	APISTR((LF_API, "BeginPath: hDC %x\n", hDC));

	ASSERT_HDC(hDC32, hDC, FALSE);

	if (hDC32->nPathState != 0)
	{
		SetLastErrorEx(1, 0);
		bSuccess = FALSE;
	}
	else
	{
		hDC32->nPathState = 1;
		hDC32->nPathSize = 0;
		hDC32->lpPathTypes = (LPBYTE) 0;
		hDC32->lpPathPoints = (LPPOINT) 0;
		bSuccess = TRUE;
	}

	RELEASEDCINFO(hDC32);

	return (bSuccess);

}

BOOL	WINAPI
EndPath(HDC hDC)
{
	HDC32 hDC32;
	BOOL bSuccess;

	APISTR((LF_API, "EndPath: hDC %x\n", hDC));

	ASSERT_HDC(hDC32, hDC, FALSE);

	if (hDC32->nPathState != 1)
	{
		SetLastErrorEx(1, 0);
		bSuccess = FALSE;
	}
	else
	{
		hDC32->nPathState = 2;
		bSuccess = TRUE;
	}

	RELEASEDCINFO(hDC32);

	return (bSuccess);

}

BOOL	WINAPI
AbortPath(HDC hDC)
{
	HDC32 hDC32;
	BOOL bSuccess;

	APISTR((LF_API, "AbortPath: hDC %x\n", hDC));

	ASSERT_HDC(hDC32, hDC, FALSE);

	if ((hDC32->nPathState != 1) && (hDC32->nPathState != 2))
	{
		SetLastErrorEx(1, 0);
		bSuccess = FALSE;
	}
	else
	{
		hDC32->nPathState = 0;
		hDC32->nPathSize = 0;
		if (hDC32->lpPathTypes)
		{
			WinFree((LPSTR) hDC32->lpPathTypes);
			hDC32->lpPathTypes = (LPBYTE) 0;
		}
		if (hDC32->lpPathPoints)
		{
			WinFree((LPSTR) hDC32->lpPathPoints);
			hDC32->lpPathPoints = (LPPOINT) 0;
		}
		bSuccess = TRUE;
	}

	RELEASEDCINFO(hDC32);

	return (bSuccess);

}

int	WINAPI
GetPath(HDC hDC, LPPOINT lpPathPoints, LPBYTE lpPathTypes, int nPathSize)
{
	HDC32 hDC32;
	int n;

	APISTR((LF_API, "GetPath: hDC %x nPathSize %d\n", hDC));

	ASSERT_HDC(hDC32, hDC, -1);

	if (hDC32->nPathState != 2)
	{
		SetLastErrorEx(1, 0);
		n = -1;
	}
	else
	{
		if (nPathSize == 0)
			n = hDC32->nPathSize;
		else if (nPathSize < hDC32->nPathSize)
		{
			SetLastErrorEx(ERROR_BUFFER_OVERFLOW, 0);
			n = -1;
		}
		else
		{
			memcpy(lpPathTypes, hDC32->lpPathTypes,
				hDC32->nPathSize * sizeof(BYTE));
			memcpy(lpPathPoints, hDC32->lpPathPoints,
				hDC32->nPathSize * sizeof(POINT));
			n = hDC32->nPathSize;
		}
	}

	if ((nPathSize > 0) && (n > 0))
		if (DPtoLP(hDC, lpPathPoints, n) == FALSE)
			n = -1;

	RELEASEDCINFO(hDC32);

	return (n);

}

BOOL	WINAPI
CloseFigure(HDC hDC)
{
	HDC32 hDC32;
	BOOL bSuccess;
	int i;

	APISTR((LF_APICALL, "CloseFigure(HDC=%x)\n", hDC));

	ASSERT_HDC(hDC32, hDC, FALSE);

	if (hDC32->nPathState != 1)
	{
		SetLastErrorEx(1, 0);
		bSuccess = FALSE;
	}
	else
	{
		bSuccess = TRUE;
		for (i = hDC32->nPathSize; --i >= 0; )
		{
			if (hDC32->lpPathTypes[i] & PT_CLOSEFIGURE)
			{
				SetLastErrorEx(1, 0);
				bSuccess = FALSE;
				break;
			}
			else if (hDC32->lpPathTypes[i] == PT_LINETO)
			{
				hDC32->lpPathTypes[i] |= PT_CLOSEFIGURE;
				break;
			}
			else if (hDC32->lpPathTypes[i] == PT_BEZIERTO)
			{
				hDC32->lpPathTypes[i] |= PT_CLOSEFIGURE;
				break;
			}
		}
	}

	RELEASEDCINFO(hDC32);

	APISTR((LF_APIRET, "CloseFigure: returns BOOL %d\n",bSuccess));
	return (bSuccess);

}

static int FlattenBezier(HDC32 hDC32, int i, LPPOINT pt, LPPOINT pt0)
{
	/* Flatten bezier spline within current path.
	 *
	 * i   = starting index
	 * pt  = current point
	 * pt0 = close figure point
	 */

	BYTE closefigure;
	POINT bezierpts[3];
	int j, n;

	/* save bezier spline before it gets overwritten */
	closefigure = hDC32->lpPathTypes[i+2] & PT_CLOSEFIGURE;
	bezierpts[0] = hDC32->lpPathPoints[i];
	bezierpts[1] = hDC32->lpPathPoints[i+1];
	bezierpts[2] = hDC32->lpPathPoints[i+2];

	/* compute number of points in flattened bezier spline */
	n = BezierPts(pt, bezierpts, NULL);
	if (n < 1)
		return (-1);

	/* insert (delete) extra points in current path */
	if (InsertDeletePath(hDC32, i, n - 3) == FALSE)
		return (-1);

	/* replace bezier spline with lines */
	for (j = i; j < i + n; j++)
		hDC32->lpPathTypes[j] = PT_LINETO;
	if (BezierPts(pt, bezierpts, hDC32->lpPathPoints + i) != n)
		return (-1);
	*pt = hDC32->lpPathPoints[i+n-1];
	if ((j == i + n) && closefigure)
	{
		hDC32->lpPathTypes[j-1] |= PT_CLOSEFIGURE;
		*pt = *pt0;
	}

	/* return number of points in flattened bezier spline */
	return (n);

}

BOOL	WINAPI
FlattenPath(HDC hDC)
{
	HDC32 hDC32;
	BOOL bSuccess;
	int i, n;
	POINT pt, pt0;

	APISTR((LF_APICALL, "FlattenPath(HDC=%x)\n", hDC));

	ASSERT_HDC(hDC32, hDC, FALSE);

	if (hDC32->nPathState != 2)
	{
		SetLastErrorEx(1, 0);
		bSuccess = FALSE;
	}
	else
	{
		bSuccess = TRUE;
		for (i = 0; i < hDC32->nPathSize; i += n)
		{
			n = 1;
			switch (hDC32->lpPathTypes[i] & ~PT_CLOSEFIGURE)
			{
			case PT_MOVETO:
				/* PT_MOVETO updates current point */
				pt = hDC32->lpPathPoints[i];
				/* PT_MOVETO updates close figure point */
				pt0 = hDC32->lpPathPoints[i];
				break;
			case PT_LINETO:
				/* PT_LINETO updates current point */
				pt = hDC32->lpPathPoints[i];
				break;
			case PT_BEZIERTO:
				/* PT_BEZIERTO's replaced with PT_LINETO's */ 
				n = FlattenBezier(hDC32, i, &pt, &pt0);
				if (n < 0)
				{
					n = hDC32->nPathSize - i;
					SetLastErrorEx(1, 0);
					bSuccess = FALSE;
				}
				break;
			default:
				n = hDC32->nPathSize - i;
				SetLastErrorEx(1, 0);
				bSuccess = FALSE;
				break;
			}
		}
	}

	RELEASEDCINFO(hDC32);

	APISTR((LF_APIRET, "FlattenPath: returns BOOL %d\n",bSuccess));
	return (bSuccess);

}

/* ******* THESE INTERNAL PEN FUNCTIONS MAY BE USEFUL FOR OTHER API's ****** */

static int PenWidth(HDC32 hDC32)
{
	LPPENINFO lpPenInfo;
	int nPenWidth;

	if ((hDC32->hPen == (HPEN)NULL)
	 || ((lpPenInfo = GETPENINFO(hDC32->hPen)) == NULL))
	{
		SetLastErrorEx(1, 0);
		return (1);
	}

	if ((lpPenInfo->lpExtPen == NULL)
	 || ((lpPenInfo->lpExtPen->elpPenStyle & PS_TYPE_MASK) == PS_COSMETIC))
	{
		nPenWidth = lpPenInfo->lpPen.lopnWidth.x;
	}
	else
	{
		nPenWidth = lpPenInfo->lpExtPen->elpWidth;
		nPenWidth = MM_LEtoDE_X(hDC32, nPenWidth);
	}

	RELEASEPENINFO(lpPenInfo);

	return (nPenWidth);

}

static int PenEndCapStyle(HDC32 hDC32)
{
	LPPENINFO lpPenInfo;
	int nPenEndCapStyle;

	if ((hDC32->hPen == (HPEN)NULL)
	 || ((lpPenInfo = GETPENINFO(hDC32->hPen)) == NULL))
	{
		SetLastErrorEx(1, 0);
		return 0;
	}

	nPenEndCapStyle = (lpPenInfo->lpExtPen != NULL)
		? lpPenInfo->lpExtPen->elpPenStyle & PS_ENDCAP_MASK : 0;

	RELEASEPENINFO(lpPenInfo);

	return (nPenEndCapStyle);

}

static int PenJoinStyle(HDC32 hDC32)
{
	LPPENINFO lpPenInfo;
	int nPenJoinStyle;

	if ((hDC32->hPen == (HPEN)NULL)
	 || ((lpPenInfo = GETPENINFO(hDC32->hPen)) == NULL))
	{
		SetLastErrorEx(1, 0);
		return 0;
	}

	nPenJoinStyle = (lpPenInfo->lpExtPen != NULL)
		? lpPenInfo->lpExtPen->elpPenStyle & PS_JOIN_MASK : 0;

	RELEASEPENINFO(lpPenInfo);

	return (nPenJoinStyle);

}

static int PenEndCapPoints_Round(int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1,
	LPPOINT lpEndCapPoints)
{

	return (0);

}

static int PenEndCapPoints_Square(int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1,
	LPPOINT lpEndCapPoints)
{
	if (pt0 && pt1 && lpEndCapPoints)
	{
		lpEndCapPoints[0] = *pt1;
#if 0	/* LATER */
		lpEndCapPoints[1].x = /* */ 0;
		lpEndCapPoints[1].y = /* */ 0;
		lpEndCapPoints[2].x = /* */ 0;
		lpEndCapPoints[2].y = /* */ 0;
		lpEndCapPoints[3].x = /* */ 0;
		lpEndCapPoints[3].y = /* */ 0;
		lpEndCapPoints[4].x = /* */ 0;
		lpEndCapPoints[4].y = /* */ 0;
#endif
	}

	return (0);

}

static int PenEndCapPoints_Flat(int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1,
	LPPOINT lpEndCapPoints)
{

	/* by definition, flat end caps create no extra area */
	return (0);

}

static int PenEndCapPoints(int nPenWidth, int nPenEndCapStyle,
	CONST POINT *pt0, CONST POINT *pt1,
	LPPOINT lpEndCapPoints)
{

	switch (nPenEndCapStyle)
	{
	case PS_ENDCAP_ROUND:
		return PenEndCapPoints_Round(nPenWidth,
			pt0, pt1, lpEndCapPoints);
	case PS_ENDCAP_SQUARE:
		return PenEndCapPoints_Square(nPenWidth,
			pt0, pt1, lpEndCapPoints);
	case PS_ENDCAP_FLAT:
		return PenEndCapPoints_Flat(nPenWidth,
			pt0, pt1, lpEndCapPoints);
	default:
		break;
	}

	return (0);

}

static int PenJoinPoints_Round(int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1, CONST POINT *pt2,
	LPPOINT lpJoinPoints)
{
	return (0);

}

static int PenJoinPoints_Bevel(int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1, CONST POINT *pt2,
	LPPOINT lpJoinPoints)
{
	if (lpJoinPoints)
	{
		lpJoinPoints[0] = *pt1;
#if 0	/* LATER */
		lpJoinPoints[1].x = /* */ 0;
		lpJoinPoints[1].y = /* */ 0;
		lpJoinPoints[2].x = /* */ 0;
		lpJoinPoints[2].y = /* */ 0;
#endif
	}

	return (1);

}

static int PenJoinPoints_Miter(int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1, CONST POINT *pt2,
	LPPOINT lpJoinPoints)
{
	if (pt0 && pt1 && pt2 && lpJoinPoints)
	{
		lpJoinPoints[0] = *pt1;
#if 0	/* LATER */
		lpJoinPoints[1].x = /* */ 0;
		lpJoinPoints[1].y = /* */ 0;
		lpJoinPoints[2].x = /* */ 0;
		lpJoinPoints[2].y = /* */ 0;
		lpJoinPoints[3].x = /* */ 0;
		lpJoinPoints[3].y = /* */ 0;
#endif
	}

	return (1);

}

static int PenJoinPoints(int nPenWidth, int nPenJoinStyle,
	CONST POINT *pt0, CONST POINT *pt1, CONST POINT *pt2,
	LPPOINT lpJoinPoints)
{
	switch (nPenJoinStyle)
	{
	case PS_JOIN_ROUND:
		return PenJoinPoints_Round(nPenWidth,
			pt0, pt1, pt2, lpJoinPoints);
	case PS_JOIN_BEVEL:
		return PenJoinPoints_Bevel(nPenWidth,
			pt0, pt1, pt2, lpJoinPoints);
	case PS_JOIN_MITER:
		return PenJoinPoints_Miter(nPenWidth,
			pt0, pt1, pt2, lpJoinPoints);
	default:
		break;
	}

	return (0);

}

#ifdef LATER
static int PenStartCapPoints_Flat(int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1,
	LPPOINT lpStartCapPoints)
{
	/* Start cap is identical to end cap except with the starting and
	 * ending points reversed.
	 */

	APISTR((LF_API, "PenStartCapPoints_Flat: calling end cap code\n"));

	return PenEndCapPoints_Flat(nPenWidth,
		pt1, pt0, lpStartCapPoints);

}

static int PenStartCapPoints_Square(int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1,
	LPPOINT lpStartCapPoints)
{
	/* Start cap is identical to end cap except with the starting and
	 * ending points reversed.
	 */

	APISTR((LF_API, "PenStartCapPoints_Square: calling end cap code\n"));

	return PenEndCapPoints_Square(nPenWidth,
		pt1, pt0, lpStartCapPoints);

}

static int PenStartCapPoints_Round(int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1,
	LPPOINT lpStartCapPoints)
{
	/* Start cap is identical to end cap except with the starting and
	 * ending points reversed.
	 */

	APISTR((LF_API, "PenStartCapPoints_Round: calling end cap code\n"));

	return PenEndCapPoints_Round(nPenWidth,
		pt1, pt0, lpStartCapPoints);

}
#endif

static int PenStartCapPoints(int nPenWidth, int nPenStartCapStyle,
	CONST POINT *pt0, CONST POINT *pt1,
	LPPOINT lpStartCapPoints)
{
	/* Start cap is identical to end cap except with the starting and
	 * ending points reversed.
	 */

	APISTR((LF_API, "PenStartCapPoints: calling end cap code\n"));

	return PenEndCapPoints(nPenWidth, nPenStartCapStyle,
		pt1, pt0, lpStartCapPoints);

}

static int PenWideLinePoints(int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1,
	LPPOINT lpWideLinePoints)
{
	if (pt0 && pt1 && lpWideLinePoints)
	{
		lpWideLinePoints[0] = *pt0;
		lpWideLinePoints[1] = *pt1;
#if 0	/* LATER */
		lpWideLinePoints[1].x = /* */ 0;
		lpWideLinePoints[1].y = /* */ 0;
		lpWideLinePoints[2].x = /* */ 0;
		lpWideLinePoints[2].y = /* */ 0;
		lpWideLinePoints[3].x = /* */ 0;
		lpWideLinePoints[3].y = /* */ 0;
		lpWideLinePoints[4].x = /* */ 0;
		lpWideLinePoints[4].y = /* */ 0;
#endif
	}

	return (2);

}

/* ************* END OF INTERNAL PEN FUNCTIONS ***************************** */

static void InitPathWideLineTypes(HDC32 hDC32, int i, int n)
{
	/* All the Pen*Points*() return an array of points:
	 * (1) 1st point is PT_MOVETO
	 * (2) 2nd & later points are PT_LINETO
	 * (3) last point is PT_LINETO | PT_CLOSEFIGURE
	 */

	if (n > 0)
	{
		int j;
		hDC32->lpPathTypes[i] = PT_MOVETO;
		for (j = i + 1; j < i + n; j++)
		{
			hDC32->lpPathTypes[j] = PT_LINETO;
		}
		hDC32->lpPathTypes[j-1] |= PT_CLOSEFIGURE;
	}

}

static int InsertPath_EndCap(HDC32 hDC32, int i,
	int nPenWidth, int nPenEndCapStyle,
	CONST POINT *pt0, CONST POINT *pt1)
{
	int n;

	/* insert end cap points */
	if (((n = PenEndCapPoints(nPenWidth, nPenEndCapStyle, pt0, pt1,
		(LPPOINT) 0)) < 0)
	 || (InsertDeletePath(hDC32, i, n) == FALSE)
	 || (PenEndCapPoints(nPenWidth, nPenEndCapStyle, pt0, pt1,
		hDC32->lpPathPoints + i) != n))
	{
		return (-1);
	}

	/* set end cap types */
	InitPathWideLineTypes(hDC32, i, n);

	/* return number of points in end cap */
	return (n);

}

static int InsertPath_Join(HDC32 hDC32, int i,
	int nPenWidth, int nPenJoinStyle,
	CONST POINT *pt0, CONST POINT *pt1, CONST POINT *pt2)
{
	int n;

	/* insert join points */
	if (((n = PenJoinPoints(nPenWidth, nPenJoinStyle, pt0, pt1, pt2,
		(LPPOINT) 0)) < 0)
	 || (InsertDeletePath(hDC32, i, n) == FALSE)
	 || (PenJoinPoints(nPenWidth, nPenJoinStyle, pt0, pt1, pt2,
		hDC32->lpPathPoints + i) != n))
	{
		return (-1);
	}

	/* set join types */
	InitPathWideLineTypes(hDC32, i, n);

	/* return number of points in join */
	return (n);

}

static int InsertPath_StartCap(HDC32 hDC32, int i,
	int nPenWidth, int nPenStartCapStyle,
	CONST POINT *pt0, CONST POINT *pt1)
{
	int n;

	/* insert start cap points */
	if (((n = PenStartCapPoints(nPenWidth, nPenStartCapStyle, pt0, pt1,
		(LPPOINT) 0)) < 0)
	 || (InsertDeletePath(hDC32, i, n) == FALSE)
	 || (PenStartCapPoints(nPenWidth, nPenStartCapStyle, pt0, pt1,
		hDC32->lpPathPoints + i) != n))
	{
		return (-1);
	}

	/* set start cap types */
	InitPathWideLineTypes(hDC32, i, n);

	/* return number of points in start cap */
	return (n);

}

static int InsertPath_WideLine(HDC32 hDC32, int i,
	int nPenWidth,
	CONST POINT *pt0, CONST POINT *pt1)
{
	int n;

	/* insert wide line points */
	if (((n = PenWideLinePoints(nPenWidth, pt0, pt1,
		(LPPOINT) 0)) < 0)
	 || (InsertDeletePath(hDC32, i, n) == FALSE)
	 || (PenWideLinePoints(nPenWidth, pt0, pt1,
		hDC32->lpPathPoints + i) != n))
	{
		return (-1);
	}

	/* set wide line types */
	InitPathWideLineTypes(hDC32, i, n);

	/* return number of points in wide line */
	return (n);

}

#ifdef LATER
static int InsertPath_WideLine_EndCap(HDC32 hDC32, int i,
	int nPenWidth, int nPenEndCapStyle,
	CONST POINT *pt0, CONST POINT *pt1)
{
	int n1, n2;

	/* insert wide line points */
	if ((n1 = InsertPath_WideLine(hDC32, i, nPenWidth,
		pt0, pt1)) < 0)
	{
		return (-1);
	}
	i += n1;

	/* insert end cap points */
	if ((n2 = InsertPath_EndCap(hDC32, i, nPenWidth, nPenEndCapStyle,
		pt0, pt1)) < 0)
	{
		return (-1);
	}
	i += n2;

	/* return total points */
	return (n1 + n2);

}

static int InsertPath_WideLine_Join(HDC32 hDC32, int i,
	int nPenWidth, int nPenJoinStyle,
	CONST POINT *pt0, CONST POINT *pt1, CONST POINT *pt2)
{
	int n1, n2;

	/* insert wide line points */
	if ((n1 = InsertPath_WideLine(hDC32, i, nPenWidth,
		pt0, pt1)) < 0)
	{
		return (-1);
	}
	i += n1;

	/* insert join points */
	if ((n2 = InsertPath_Join(hDC32, i, nPenWidth, nPenJoinStyle,
		pt0, pt1, pt2)) < 0)
	{
		return (-1);
	}
	i += n2;

	/* return total points */
	return (n1 + n2);

}
#endif

static int WidenPathLine(HDC32 hDC32, int i, int n, LPPOINT pt, LPPOINT pt0)
{
	/* Replace path line with wide path figure.
	 *
	 * i   = starting index
	 * n   = number of points
	 * pt  = current point
	 * pt0 = close figure point
	 */

	int nPenWidth = PenWidth(hDC32),
	    nPenEndCapStyle = PenEndCapStyle(hDC32),
	    nPenJoinStyle = PenJoinStyle(hDC32),
	    j = i,
	    ii = i + n,
	    nn1, nn2, nn3;
	LPPOINT nextpt, nextnextpt;

	/* insert start cap */
	if ((nn1 = InsertPath_StartCap(hDC32, ii,
		nPenWidth, nPenEndCapStyle,
		pt, hDC32->lpPathPoints + j)) < 0)
	{
		return (-1);
	}
	ii += nn1;

	/* insert (wide line, join) pair(s) */
	nn2 = 0;
	for ( ; (nextpt = hDC32->lpPathPoints + j); j++, *pt = *nextpt)
	{
		if ((nn2 = InsertPath_WideLine(hDC32, ii,
			nPenWidth,
			pt, nextpt)) < 0)
		{
			return (-1);
		}
		ii += nn2;
		if (j >= i + n - 1)
			break;
		nextnextpt = nextpt + 1;
		if ((nn2 = InsertPath_Join(hDC32, ii,
			nPenWidth, nPenJoinStyle,
			pt, nextpt, nextnextpt)) < 0)
		{
			return (-1);
		}
		ii += nn2;
	}

	/* insert end cap */
	if ((nn3 = InsertPath_EndCap(hDC32, ii,
		nPenWidth, nPenEndCapStyle,
		pt, nextpt)) < 0)
	{
		return (-1);
	}
	ii += nn3;

	/* delete thin line */
	if (!InsertDeletePath(hDC32, i, -n))
	{
		return (-1);
	}

	/* return total points */
	return (nn1 + nn2 + nn3);

}

static int WidenPathFigure(HDC32 hDC32, int i, int n, LPPOINT pt, LPPOINT pt0)
{
	/* Replace path figure with wide path figure.
	 *
	 * i   = starting index
	 * n   = number of points
	 * pt  = current point
	 * pt0 = close figure point
	 */

	int nPenWidth = PenWidth(hDC32),
	    nPenEndCapStyle = PenEndCapStyle(hDC32),
	    nPenJoinStyle = PenJoinStyle(hDC32),
	    j = i,
	    ii = i + n,
	    nn1, nn2;
	LPPOINT nextpt, nextnextpt;

	nn1 = 0;
	/* insert (wide line, join) pair(s) */
	for ( ; (nextpt = hDC32->lpPathPoints + j); j++, *pt = *nextpt)
	{
		if ((nn1 = InsertPath_WideLine(hDC32, ii,
			nPenWidth,
			pt, nextpt)) < 0)
		{
			return (-1);
		}
		ii += nn1;
		if (j >= i + n)
			break;
		nextnextpt = nextpt + 1;
		if ((nn1 = InsertPath_Join(hDC32, ii,
			nPenWidth, nPenJoinStyle,
			pt, nextpt, nextnextpt)) < 0)
		{
			return (-1);
		}
		ii += nn1;
	}

	/* insert end cap */
	nextpt = pt0;
	if ((nn2 = InsertPath_EndCap(hDC32, ii,
		nPenWidth, nPenEndCapStyle,
		pt, nextpt)) < 0)
	{
		return (-1);
	}
	ii += nn2;

	/* delete thin line */
	if (!InsertDeletePath(hDC32, i, -n))
	{
		return (-1);
	}

	/* return total points */
	return (nn1 + nn2);

}
/* ^^^ check for BUGS */

BOOL	WINAPI
WidenPath(HDC hDC)
{
	HDC32 hDC32;
	BOOL bSuccess;
	int i, n;
	POINT pt, pt0;

	APISTR((LF_API, "WidenPath: hDC %x\n", hDC));

	if (FlattenPath(hDC) == FALSE)
		return (FALSE);

	ASSERT_HDC(hDC32, hDC, FALSE);

	if (hDC32->nPathState != 2)
	{
		SetLastErrorEx(1, 0);
		bSuccess = FALSE;
	}
	else
	{
		bSuccess = TRUE;
		for (i = 0; i < hDC32->nPathSize; i += n)
		{
			n = 1;
			switch (hDC32->lpPathTypes[i] & ~PT_CLOSEFIGURE)
			{
			case PT_MOVETO:
				/* PT_MOVETO updates current point */
				pt = hDC32->lpPathPoints[i];
				/* PT_MOVETO updates close figure point */
				pt0 = hDC32->lpPathPoints[i];
				break;
			case PT_LINETO:
				/* PT_LINETO's replaced with PT_LINETO's */ 
				for (n = 0; i + n < hDC32->nPathSize; n++)
				{
					if (hDC32->lpPathTypes[i + n] !=
						PT_LINETO)
						break;
				}
				if (i + n >= hDC32->nPathSize)
				{
					n = WidenPathLine(hDC32, i, n,
						&pt, &pt0);
				}
				else if (hDC32->lpPathTypes[i + n] !=
					(PT_LINETO|PT_CLOSEFIGURE))
				{
					n = WidenPathLine(hDC32, i, n,
						&pt, &pt0);
				}
				else
				{
					n = WidenPathFigure(hDC32, i, n + 1,
						&pt, &pt0);
				}
				if (n < 0)
				{
					n = hDC32->nPathSize - i;
					SetLastErrorEx(1, 0);
					bSuccess = FALSE;
				}
				break;
			case PT_BEZIERTO:
				/* PT_BEZIERTO illegal in flat path */
				n = hDC32->nPathSize - i;
				SetLastErrorEx(1, 0);
				bSuccess = FALSE;
				break;
			default:
				n = hDC32->nPathSize - i;
				SetLastErrorEx(1, 0);
				bSuccess = FALSE;
				break;
			}
		}
	}

	RELEASEDCINFO(hDC32);

	return (bSuccess);

}

static HRGN PathFigureToRegion(HDC32 hDC32, int i, int n,
	LPPOINT pt, LPPOINT pt0)
{
	/* Create region from path figure.
	 *
	 * i   = starting index
	 * n   = number of points
	 * pt  = current point
	 * pt0 = close figure point
	 */

	/* BUG: assumes [i-1] == *pt */

	*pt = *pt0;

	return CreatePolygonRgn(hDC32->lpPathPoints + i - 1, n + 1,
		hDC32->PolyFillMode);

}

static HRGN PathToRegionExt(HDC hDC, BOOL bDiscardPath)
{
	HDC32 hDC32;
	HRGN hPathRgn, hPathFigureRgn;
	int i, n;
	POINT pt, pt0;

	APISTR((LF_API, "PathToRegionExt: hDC %x bDiscardPath %d\n",
		hDC, bDiscardPath));

	ASSERT_HDC(hDC32, hDC, (HRGN)NULL);

	if (FlattenPath(hDC) == FALSE)
	{
		RELEASEDCINFO(hDC32);
		return (HRGN)NULL;
	}

	if (hDC32->nPathState != 2)
	{
		SetLastErrorEx(1, 0);
		hPathRgn = (HRGN)NULL;
	}
	else
	{
		hPathRgn = hPathFigureRgn = (HRGN)NULL;
		for (i = 0; i < hDC32->nPathSize; i += n)
		{
			n = 1;
			switch (hDC32->lpPathTypes[i] & ~PT_CLOSEFIGURE)
			{
			case PT_MOVETO:
				/* PT_MOVETO updates current point */
				pt = hDC32->lpPathPoints[i];
				/* PT_MOVETO updates close figure point */
				pt0 = hDC32->lpPathPoints[i];
				break;
			case PT_LINETO:
				/* One or more PT_LINETO's terminated by
				 * PT_CLOSEFIGURE flag creates a path figure
				 * region.
				 */
				for (n = 0; i + n < hDC32->nPathSize; n++)
				{
					if (hDC32->lpPathTypes[i + n] !=
						PT_LINETO)
						break;
				}
				if (i + n >= hDC32->nPathSize)
				{
					pt = hDC32->lpPathPoints[i + n - 1];
					break;
				}
				else if (hDC32->lpPathTypes[i + n] !=
					(PT_LINETO|PT_CLOSEFIGURE))
				{
					pt = hDC32->lpPathPoints[i + n - 1];
					break;
				}
				else
				{
					n++;
					hPathFigureRgn = PathFigureToRegion(
						hDC32, i, n, &pt, &pt0);
				}
				if (hPathFigureRgn == (HRGN)NULL)
				{
					n = hDC32->nPathSize - i;
					SetLastErrorEx(1, 0);
					if (hPathRgn != (HRGN)NULL)
					{
						DeleteObject(hPathRgn);
						hPathRgn = (HRGN)NULL;
					}
				}
				break;
			case PT_BEZIERTO:
				/* PT_BEZIERTO illegal in flat path */
				n = hDC32->nPathSize - i;
				SetLastErrorEx(1, 0);
				if (hPathRgn != (HRGN)NULL)
				{
					DeleteObject(hPathRgn);
					hPathRgn = (HRGN)NULL;
				}
				break;
			default:
				n = hDC32->nPathSize - i;
				if (hPathRgn != (HRGN)NULL)
				{
					DeleteObject(hPathRgn);
					hPathRgn = (HRGN)NULL;
				}
				break;
			}
			/* combine path figure region with path region */
			if (hPathFigureRgn == (HRGN)NULL)
				continue;
			if (hPathRgn == (HRGN)NULL)
			{
				hPathRgn = hPathFigureRgn;
				hPathFigureRgn = (HRGN)NULL;
				continue;
			}
			if (CombineRgn(hPathRgn, hPathRgn, hPathFigureRgn,
				RGN_OR) == ERROR)
			{
				DeleteObject(hPathFigureRgn);
				hPathFigureRgn = (HRGN)NULL;
				DeleteObject(hPathRgn);
				hPathRgn = (HRGN)NULL;
				SetLastErrorEx(1, 0);
				break;
			}
			else
			{
				DeleteObject(hPathFigureRgn);
				hPathFigureRgn = (HRGN)NULL;
			}
		}
		/* (optional) discard path */
		if ((bDiscardPath == TRUE) && (hPathRgn != (HRGN)NULL))
		{
			hDC32->nPathState = 0;
			hDC32->nPathSize = 0;
			WinFree((LPSTR) hDC32->lpPathTypes);
			hDC32->lpPathTypes = (LPBYTE) 0;
			WinFree((LPSTR) hDC32->lpPathPoints);
			hDC32->lpPathPoints = (LPPOINT) 0;
		}
	}

	RELEASEDCINFO(hDC32);

	return (hPathRgn);

}

HRGN	WINAPI
PathToRegion(HDC hDC)
{
	APISTR((LF_API, "PathToRegion: hDC %x\n", hDC));

	return PathToRegionExt(hDC, TRUE);

}

BOOL	WINAPI
FillPath(HDC hDC)
{
	HDC32 hDC32;
	HRGN hPathRgn;
	BOOL bSuccess;

	APISTR((LF_API, "FillPath: hDC %x\n", hDC));

	ASSERT_HDC(hDC32, hDC, FALSE);

	if ((hPathRgn = PathToRegion(hDC)) == (HRGN)NULL)
	{
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	bSuccess = PaintRgn(hDC, hPathRgn);

	if (DeleteObject(hPathRgn) == FALSE)
	{
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	RELEASEDCINFO(hDC32);

	return (bSuccess);

}

BOOL	WINAPI
StrokePath(HDC hDC)
{
	HDC32 hDC32;
	int nPathSize;
	LPBYTE lpPathTypes;
	LPPOINT lpPathPoints;
	BOOL bSuccess;

	APISTR((LF_API, "StrokePath: hDC %x\n", hDC));

	ASSERT_HDC(hDC32, hDC, FALSE);

	if ((nPathSize = GetPath(hDC, NULL, NULL, 0)) <= 0)
	{
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	if ((lpPathTypes = (LPBYTE) WinMalloc(nPathSize * sizeof(BYTE)))
		== NULL)
	{
		SetLastErrorEx(ERROR_NOT_ENOUGH_MEMORY, 0);
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	if ((lpPathPoints = (LPPOINT) WinMalloc(nPathSize * sizeof(POINT)))
		== NULL)
	{
		WinFree((LPSTR)lpPathTypes);
		SetLastErrorEx(ERROR_NOT_ENOUGH_MEMORY, 0);
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	if ((GetPath(hDC, lpPathPoints, lpPathTypes, nPathSize) == nPathSize)
	 && (PolyDraw(hDC, lpPathPoints, lpPathTypes, nPathSize) == TRUE))
		bSuccess = TRUE;
	else
		bSuccess = FALSE;

	WinFree((LPSTR)lpPathTypes);
	WinFree((LPSTR)lpPathPoints);

	RELEASEDCINFO(hDC32);

	return (bSuccess);

}

BOOL	WINAPI
StrokeAndFillPath(HDC hDC)
{
	HDC32 hDC32;
	HRGN hBrushRgn, hPenRgn;
	BOOL bSuccess;

	APISTR((LF_API, "StrokeAndFillPath: hDC %x\n", hDC));

	ASSERT_HDC(hDC32, hDC, FALSE);

	/* stroke path */
	if (StrokePath(hDC) == FALSE)
	{
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	/* get interior (brush) region (but don't delete path) */
	if ((hBrushRgn = PathToRegionExt(hDC, FALSE)) == (HRGN)NULL)
	{
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	/* get outline (pen) region (also delete path) */
	if ((WidenPath(hDC) == FALSE)
	 || ((hPenRgn = PathToRegion(hDC)) == (HRGN)NULL))
	{
		DeleteObject(hBrushRgn);
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	/* exclude outline (pen) region from interior (brush) region */
	if (CombineRgn(hBrushRgn, hBrushRgn, hPenRgn, RGN_DIFF) == ERROR)
	{
		DeleteObject(hBrushRgn);
		DeleteObject(hPenRgn);
		RELEASEDCINFO(hDC32);
		return (FALSE);
	}

	/* fill interior (brush) region */
	bSuccess = PaintRgn(hDC, hBrushRgn);

	DeleteObject(hBrushRgn);
	DeleteObject(hPenRgn);

	RELEASEDCINFO(hDC32);

	return (bSuccess);

}

#ifdef LATER
static LPBYTE AllocPathTypes(HDC32 hDC32, int i, int n)
{
	/* Allocate path type array and initialize with path types.
	 *
	 * i = starting index
	 * n = number of points
	 */

	LPBYTE lpPathTypes = (LPBYTE) WinMalloc(n * sizeof(BYTE));
	if (lpPathTypes)
	{
		memcpy(lpPathTypes, hDC32->lpPathTypes + i,
			n * sizeof(BYTE));
	}
	return (lpPathTypes);

}

static BOOL FreePathTypes(LPBYTE lpPathTypes)
{
	/* Free path type array allocated by AllocPathTypes().
	 */

	if (lpPathTypes == NULL)
		return (FALSE);
	WinFree((LPSTR) lpPathTypes);
	return (TRUE);

}

static LPPOINT AllocPathPoints(HDC32 hDC32, int i, int n)
{
	/* Allocate path point array and initialize with path points.
	 *
	 * i = starting index
	 * n = number of points
	 */

	LPPOINT lpPathPoints = (LPPOINT) WinMalloc(n * sizeof(POINT));
	if (lpPathPoints)
	{
		memcpy(lpPathPoints, hDC32->lpPathPoints + i,
			n * sizeof(POINT));
	}
	return (lpPathPoints);

}

static BOOL FreePathPoints(LPPOINT lpPathPoints)
{
	/* Free path point array allocated by AllocPathPoints().
	 */

	if (lpPathPoints == NULL)
		return (FALSE);
	WinFree((LPSTR) lpPathPoints);
	return (TRUE);

}
#endif

static BOOL InsertDeletePath(HDC32 hDC32, int i, int n)
{
	/* Insert (delete) path entries.
	 *
	 *  i = starting index
	 *  n = number of entries to insert (if n > 0)
	 * -n = number of entries to delete (if n < 0)
	 */

	int nNewPathSize;
	LPBYTE lpNewPathTypes;
	LPPOINT lpNewPathPoints;

	APISTR((LF_API, "InsertDeletePath: hDC32 %x i %d n %d\n",
		hDC32, i, n));

	/* nop */
	if (n == 0)
		return (TRUE);

	/* delete -n entries */
	if (n < 0)
	{
		n = -n;
		if ((i < 0) || (i >= hDC32->nPathSize))
			return (FALSE);
		if (i + n > hDC32->nPathSize)
			n = hDC32->nPathSize - i;
		if (hDC32->nPathSize - i - n > 0)
		{
			memmove(hDC32->lpPathTypes + i,
				hDC32->lpPathTypes + i + n,
				(hDC32->nPathSize - i - n) * sizeof(BYTE));
			memmove(hDC32->lpPathPoints + i,
				hDC32->lpPathPoints + i + n,
				(hDC32->nPathSize - i - n) * sizeof(POINT));
		}
		hDC32->nPathSize -= n;
		return (TRUE);
	}

	/* insert n entries */
	if ((i < 0) || (i > hDC32->nPathSize))
		return (FALSE);
	nNewPathSize = hDC32->nPathSize + n;
	if ((lpNewPathTypes = (LPBYTE) WinRealloc((LPSTR)hDC32->lpPathTypes,
		nNewPathSize * sizeof(BYTE))) == NULL)
	{
		SetLastErrorEx(ERROR_NOT_ENOUGH_MEMORY, 0);
		return (FALSE);
	}
	hDC32->lpPathTypes = lpNewPathTypes;
	if ((lpNewPathPoints = (LPPOINT) WinRealloc((LPSTR)hDC32->lpPathPoints,
		nNewPathSize * sizeof(POINT))) == NULL)
	{
		SetLastErrorEx(ERROR_NOT_ENOUGH_MEMORY, 0);
		return (FALSE);
	}
	hDC32->lpPathPoints = lpNewPathPoints;
	hDC32->nPathSize = nNewPathSize;
	if (nNewPathSize - i - n > 0)
	{
		memmove(lpNewPathTypes + i + n, lpNewPathTypes + i,
			(nNewPathSize - i - n) * sizeof(BYTE));
		memmove(lpNewPathPoints + i + n, lpNewPathPoints + i,
			(nNewPathSize - i - n) * sizeof(POINT));
	}
	return (TRUE);

}

/* (WIN32) GDI Brush ******************************************************* */

HBRUSH	WINAPI
CreateDIBPatternBrushPt(CONST VOID *lpPackedDIB, UINT uUsage)
{
	LOGBRUSH lb;

	APISTR((LF_API, "CreateDIBPatternBrushPt: usage %x\n", uUsage));

	lb.lbStyle = BS_DIBPATTERNPT;
	lb.lbColor = (COLORREF) uUsage;
	lb.lbHatch = (LONG) lpPackedDIB;
	return CreateBrushIndirect(&lb);

}

BOOL	WINAPI
FixBrushOrgEx()
{
	APISTR((LF_API, "FixBrushOrgEx() not part of WIN32 (only WIN32S)\n"));
	return (FALSE);
}

BOOL	WINAPI
SetBrushOrgEx(HDC hDC, int nXOrg, int nYOrg, LPPOINT lpOldOrg)
{
	HDC32 hDC32;
	BOOL bSuccess;

	APISTR((LF_API, "SetBrushOrgEx: hDC %x nXOrg %d nYOrg %d\n",
		hDC, nXOrg, nYOrg));

	ASSERT_HDC(hDC32, hDC, FALSE);

	bSuccess = TRUE;
	if (lpOldOrg)
		bSuccess = GetBrushOrgEx(hDC, lpOldOrg);
	SetBrushOrg(hDC, nXOrg, nYOrg);

	RELEASEDCINFO(hDC32);

	return (bSuccess);

}
