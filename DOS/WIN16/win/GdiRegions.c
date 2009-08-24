/*    
	GdiRegions.c	2.29
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
#include "windowsx.h"

#include "WinDefs.h"		/* TestWF and such */
#include "Log.h"
#include "GdiDC.h"
#include "GdiRegions.h"
#include "Driver.h"

/* imported routines */
extern InternalPaintWindows(HWND,UINT);

/* internal routines */
static int DoRectAndRegion(HRGN, const LPRECT, int);

BOOL
IsRegion(HRGN hRgn)
{
    REGIONINFO *lpR = GETREGIONINFO(hRgn);
    BOOL b = (lpR != NULL)?TRUE:FALSE;
    if (b)
      RELEASEREGIONINFO(lpR);
    return  b;
}

REGION32
TWIN_InternalGetRegionData(HRGN hRgn)
{
    LPREGIONINFO lpRegion;

    if (!(lpRegion = GETREGIONINFO(hRgn)))
	return (REGION32)0;
    else {
        REGION32 lpr32 = (REGION32)lpRegion->Rgn32;
        RELEASEREGIONINFO(lpRegion);
	return lpr32;
    }
}

HRGN    WINAPI 
CreateRectRgn(int left, int top, int right, int bottom)
{
    RECT	rc;

    SetRect(&rc,left,top,right,bottom);
    return  CreateRectRgnIndirect(&rc);
}

HRGN    WINAPI
CreateRectRgnIndirect(const RECT *lpr)
{
    REGION32 Rgn32;
    HRGN     hrgn;

    APISTR((LF_APICALL,"CreateRectRgnIndirect(RET *=%p)\n", lpr));

    Rgn32 = (REGION32)DRVCALL_REGIONS(PRH_CREATERECTREGION,0,0,lpr);

    hrgn = GdiCreateRegion((LPSTR)Rgn32);

    APISTR((LF_APIRET,"CreateRectRgnIndirect: returns HRGN %x\n",hrgn));
    return hrgn;
}

void    WINAPI 
SetRectRgn(HRGN hRgn, int left, int top, int right, int bottom)
{
    REGION32 Rgn32;
    RECT rc;

    APISTR((LF_APICALL,"SetRectRgn(HRGN=%x,int=%d,int=%d,int=%d,int=%d)\n",
		hRgn,left,top,right,bottom));

    if (!(Rgn32 = TWIN_InternalGetRegionData(hRgn))) {
        APISTR((LF_APIFAIL,"CreateRectRgnIndirect: returns void\n"));
	return;
    }

    SetRect(&rc,left,top,right,bottom);
    (void)DRVCALL_REGIONS(PRH_SETRECTREGION,Rgn32,0,&rc);
    APISTR((LF_APIRET,"SetRectRgn: returns void\n"));
}

int
UnionRectWithRegion(HRGN hRgn, const LPRECT lprc)
{
    int rc;
    APISTR((LF_APICALL,
	"UnionRectWithRegion(HRGN=%x,LPRECT=%p)\n", hRgn,lprc));
    rc =  DoRectAndRegion(hRgn,lprc,RGN_OR);
    APISTR((LF_APIRET,"UnionRectWithRegionn: returns int %d\n",rc));
    return rc;
}

int
SubtractRectFromRegion(HRGN hRgn, const LPRECT lprc)
{
    int rc;
    APISTR((LF_APICALL,
	"SubtractRectWithRegion(HRGN=%x,LPRECT=%p)\n", hRgn,lprc));
    rc =  DoRectAndRegion(hRgn,lprc,RGN_DIFF);
    APISTR((LF_APIRET,"SubtractRectWithRegion: returns int %d\n",rc));
    return rc;
}

int
IntersectRectWithRegion(HRGN hRgn, const LPRECT lprc)
{
    int rc;
    APISTR((LF_APICALL,
	"IntersectRectWithRegion(HRGN=%x,LPRECT=%p)\n", hRgn,lprc));
    rc = DoRectAndRegion(hRgn,lprc,RGN_AND);
    APISTR((LF_APIRET,"IntersectRectWithRegion: returns int %d\n",rc));
    return rc;
}

int
XorRectWithRegion(HRGN hRgn, const LPRECT lprc)
{
    int rc;
    APISTR((LF_APICALL,
	"XorRectWithRegion(HRGN=%x,LPRECT=%p)\n", hRgn,lprc));
    rc = DoRectAndRegion(hRgn,lprc,RGN_XOR);
    APISTR((LF_APIRET,"XorRectWithRegion: returns int %d\n",rc));
    return rc;
}

static int
DoRectAndRegion(HRGN hRgn, const LPRECT lprc, int fCombineMode)
{
    REGION32 Rgn32;
    HRGN hRectRgn;
    int ret;

    if (!lprc)
	return ERROR;

    if (fCombineMode == RGN_OR) {
	if (!(Rgn32 = TWIN_InternalGetRegionData(hRgn)))
	    return ERROR;
	else
	    return (int)DRVCALL_REGIONS(PRH_UNIONRECTWITHREGION,
				Rgn32,0,lprc);
    }

    hRectRgn = CreateRectRgnIndirect(lprc);

    ret = CombineRgn(hRgn,hRgn,hRectRgn,fCombineMode);

    DeleteObject(hRectRgn);

    return ret;
}

HRGN    WINAPI
CreatePolygonRgn(const POINT *lpPt, int nPt, int mode)
{
    REGION32 Rgn32;
    HRGN hRgn = 0;

    APISTR((LF_APICALL,"CreatePolygonRgn(POINT=%p,int %d,int=%d)\n",
	lpPt,nPt,mode));

    Rgn32 = (REGION32)DRVCALL_REGIONS(PRH_CREATEPOLYREGION,nPt,mode,lpPt);
    if(Rgn32 == 0) {
        APISTR((LF_APIFAIL,"CreatePolygonRgn: returns HRGN %x\n",hRgn));
        return hRgn;
    }
    hRgn = GdiCreateRegion((LPSTR)Rgn32);
    APISTR((LF_APIRET,"CretePolygonRgn: returns HRGN %x\n",hRgn));
    return hRgn;
}

HRGN    WINAPI 
CreatePolyPolygonRgn(const POINT *lpPt, const int *nPt, int n, int mode)
{
    REGION32 Rgn32, Rgn32Poly;
    int	  	i,nsides,offset;
    HRGN     hRgn;

    APISTR((LF_APICALL,
	"CreatePolyPolygonRgn(POINT*=%p,int *=%p,int=%d,int=%d)\n",
	lpPt,nPt,n,mode));

    if (!(Rgn32 = (REGION32)DRVCALL_REGIONS(PRH_CREATEREGION,0,0,0))) {
  	APISTR((LF_APIFAIL,"CreatePolyPolygonRgn: returns HRGN 0\n"));
	return (HRGN)0;
    }

    for(i=0,offset=0;i<n;i++) {
	nsides = nPt[i];
	if (!(Rgn32Poly = (REGION32)DRVCALL_REGIONS(PRH_CREATEPOLYREGION,
			nsides,mode,&lpPt[offset]))) {
	    (void)DRVCALL_REGIONS(PRH_DESTROYREGION,Rgn32,0,0);
  	    APISTR((LF_APIFAIL,"CreatePolyPolygonRgn: returns HRGN 0\n"));
	    return (HRGN)0;
	}
	offset += nsides;
	(void)DRVCALL_REGIONS(PRH_UNIONREGION,Rgn32Poly,Rgn32,Rgn32);
	(void)DRVCALL_REGIONS(PRH_DESTROYREGION,Rgn32Poly,0,0);
    }

    hRgn = GdiCreateRegion((LPSTR)Rgn32);
    APISTR((LF_APIRET,"CreatePolyPolygonRgn: returns HRGN %x\n",hRgn));
    return hRgn;
}

BOOL
GdiDeleteRgn(HRGN hRgn)
{
    REGION32 Rgn32;

    if ( (Rgn32 = TWIN_InternalGetRegionData(hRgn))) {
	(void)DRVCALL_REGIONS(PRH_DESTROYREGION,Rgn32,0,0);
	return TRUE;
    }
    else
	return FALSE;
}

int     WINAPI 
OffsetRgn(HRGN hRgn, int x, int y)
{
    REGION32 Rgn32;
    POINT pt;
    int   rc = ERROR;

    APISTR((LF_APICALL,"OffsetRgn(HRGN=%x,int=%d,int=%d)\n",hRgn,x,y));

    if ((Rgn32 = TWIN_InternalGetRegionData(hRgn))) {
	pt.x = x; pt.y = y;
        rc = (int) DRVCALL_REGIONS(PRH_OFFSETREGION,Rgn32,0,&pt);
        APISTR((LF_APIRET,"OffestRgn: returns int %d\n",rc));
	return rc;
    }
    APISTR((LF_APIFAIL,"OffsetRgn: returns int 0\n"));
    return ERROR;
}

BOOL    WINAPI 
PtInRegion(HRGN hRgn, int x, int y)
{
    REGION32 Rgn32;
    POINT pt;
    BOOL  rc;

    APISTR((LF_APICALL,"PtInRegion(HRGN=%x,int=%d,int=%d0\n",hRgn,x,y));

    if ((Rgn32 = TWIN_InternalGetRegionData(hRgn))) {
	pt.x = x; pt.y = y;
	rc =  (BOOL)DRVCALL_REGIONS(PRH_PTINREGION,Rgn32,0,&pt);
        APISTR((LF_APIRET,"PtInRegion: returns BOOL %d\n",rc));
        return rc;
    }
    APISTR((LF_APIFAIL,"PtInRegion: returns int 0\n"));
    return FALSE;
}

BOOL    WINAPI 
RectInRegion(HRGN hRgn, const RECT FAR* lpr)
{
    REGION32 Rgn32;
    BOOL     rc;

    APISTR((LF_APICALL,"PtInRegion(HRGN=%x,RECT *=%p)\n",hRgn,lpr));

    if (!lpr) {
        APISTR((LF_APIFAIL,"RectInRegion: returns int 0\n"));
	return FALSE;
    }


    if ((Rgn32 = TWIN_InternalGetRegionData(hRgn))) {
	rc = (BOOL)DRVCALL_REGIONS(PRH_RECTINREGION,Rgn32,0,lpr);
        APISTR((LF_APIRET,"RectInRegion: returns BOOL %d\n",rc));
        return rc;
    }
    APISTR((LF_APIFAIL,"RectInRegion: returns int 0\n"));
    return FALSE;
}

BOOL    WINAPI 
EqualRgn(HRGN hRgn1, HRGN hRgn2)
{
    REGION32 Rgn32_1,Rgn32_2;
    BOOL rc;

    APISTR((LF_APICALL,"EqualRgn(HRGN=%x,HRGN=%x)\n",hRgn1,hRgn2));

    if (!(Rgn32_1 = TWIN_InternalGetRegionData(hRgn1)) ||
        !(Rgn32_2 = TWIN_InternalGetRegionData(hRgn2))) {
        APISTR((LF_APIFAIL,"PtInRegion: returns int 0\n"));
	return  FALSE;
    }

    rc =  (BOOL)DRVCALL_REGIONS(PRH_EQUALREGION,Rgn32_1,Rgn32_1,0);
    APISTR((LF_APIRET,"EqualRgn: returns BOOL %d\n",rc));
    return rc;
}

int     WINAPI 
CombineRgn(HRGN hDestRgn, HRGN hSrcRgn1, HRGN hSrcRgn2, int nCombineMode)
{
    REGION32 Rgn32Src1, Rgn32Src2, Rgn32Dest;
    int rc;
	
    APISTR((LF_APICALL,"CombineRgn(HRGN=%x,HRGN=%x,HRGN=%x,int=%d)\n",
	hDestRgn,hSrcRgn1,hSrcRgn2,nCombineMode));

    /* destination and source1 are always passed */
    if (!(Rgn32Dest = TWIN_InternalGetRegionData(hDestRgn)) ||
	!(Rgn32Src1 = TWIN_InternalGetRegionData(hSrcRgn1))) {
        APISTR((LF_APIFAIL,"CombineRgn: returns int %d\n",ERROR));
	return ERROR;
    }

    switch(nCombineMode) {
	case RGN_AND:
	    if (!(Rgn32Src2 = TWIN_InternalGetRegionData(hSrcRgn2))) {
        	APISTR((LF_APIFAIL,"CombineRgn: returns int %d\n",ERROR));
		return ERROR;
  	    }
	    (void)DRVCALL_REGIONS(PRH_INTERSECTREGION,
		Rgn32Src1,Rgn32Src2,Rgn32Dest);
	    break;
			
	case RGN_COPY:
	    (void)DRVCALL_REGIONS(PRH_COPYREGION,Rgn32Src1,Rgn32Dest,0);
	    break;

	case RGN_DIFF:
	    if (!(Rgn32Src2 = TWIN_InternalGetRegionData(hSrcRgn2))) {
        	APISTR((LF_APIFAIL,"CombineRgn: returns int %d\n",ERROR));
		return ERROR;
	    }
	    (void)DRVCALL_REGIONS(PRH_DIFFREGION,Rgn32Src1,Rgn32Src2,Rgn32Dest);
	    break;

	case RGN_OR:
	    if (!(Rgn32Src2 = TWIN_InternalGetRegionData(hSrcRgn2))) {
        	APISTR((LF_APIFAIL,"CombineRgn: returns int %d\n",ERROR));
		return ERROR;
 	    }
	    (void)DRVCALL_REGIONS(PRH_UNIONREGION,
		Rgn32Src1,Rgn32Src2,Rgn32Dest);
	    break;

	case RGN_XOR:
	    if (!(Rgn32Src2 = TWIN_InternalGetRegionData(hSrcRgn2))) {
        	APISTR((LF_APIFAIL,"CombineRgn: returns int %d\n",ERROR));
		return ERROR;
	    }
	    (void)DRVCALL_REGIONS(PRH_XORREGION,Rgn32Src1,Rgn32Src2,Rgn32Dest);
	    break;

	default:
            APISTR((LF_APIFAIL,"CombineRgn: returns int %d\n",ERROR));
	    return ERROR;
    }

    if ((BOOL)DRVCALL_REGIONS(PRH_ISEMPTYREGION,Rgn32Dest,0,0))
	rc =  NULLREGION;
    else
	rc =  COMPLEXREGION;

    APISTR((LF_APIRET,"CombineRgn: returns int %d\n",rc));
    return rc;
}

int WINAPI 
GetRgnBox(HRGN hRgn, RECT * lpr)
{
    REGION32 Rgn32;
    int rc;

    APISTR((LF_APICALL,"GetRgnBox(HRGN=%x)\n",hRgn));

    if (!(Rgn32 = TWIN_InternalGetRegionData(hRgn)) ||
	(BOOL)DRVCALL_REGIONS(PRH_ISEMPTYREGION,Rgn32,0,0)) {
	SetRectEmpty(lpr);
        APISTR((LF_APIFAIL,"GetRgnBox: returns int %d\n",NULLREGION));
        return NULLREGION;
    }

    rc = (int)DRVCALL_REGIONS(PRH_REGIONBOX,Rgn32,0,lpr);
    APISTR((LF_APIRET,"GetRgnBox: returns int %d\n",rc));
    return rc;
}

int WINAPI 
ExcludeUpdateRgn(HDC hDC,HWND hWnd)
{
    REGION32 Rgn32Clip, Rgn32Update;
    HDC32 hDC32;
    int rc;

    APISTR((LF_APICALL,"ExcludeUpdateRgn(HDC=%x,HWND=%x)\n", hDC,hWnd));

    ASSERT_HDC(hDC32,hDC,0);

    if (0 == (Rgn32Clip = (REGION32)
		DRVCALL_DC(PDCH_GETCLIP,0L,0L,hDC32->lpDrvData))) {
        RELEASEDCINFO(hDC32);
        APISTR((LF_APIFAIL,"ExcludeUpdateRgn: returns int %d\n",NULLREGION));
	return NULLREGION;
    }
    
    RELEASEDCINFO(hDC32);      /*  done with it by here, apparently */

    if ((Rgn32Update = (REGION32)GetWindowLong(hWnd,GWL_UPDATE))) {
	rc = (int)DRVCALL_REGIONS(PRH_DIFFREGION,
		Rgn32Clip,Rgn32Update,Rgn32Clip);
        APISTR((LF_APIRET,"ExcludeUpdateRgn: returns int %d\n",rc));
	return rc;
    }
    else
	if ((BOOL)DRVCALL_REGIONS(PRH_ISEMPTYREGION,Rgn32Clip,0,0))
	    rc =  NULLREGION;
	else
	    rc =  COMPLEXREGION;
    APISTR((LF_APIRET,"ExcludeUpdateRgn: returns int %d\n",rc));
    return rc;
}

int WINAPI 
GetUpdateRgn(HWND hWnd, HRGN hRgn, BOOL fErase)
{
    int rc;
    APISTR((LF_APICALL,"GetUpdateRgn(HWND=%x,HRGN=%x,BOOL=%d\n",
	hWnd,hRgn,fErase));
    rc = GetUpdateRgnEx(hWnd,(LPRECT)0,hRgn,fErase);
    APISTR((LF_APIRET,"GetUpdateRgn: returns int %d\n",rc));
    return rc;
}

BOOL    WINAPI 
GetUpdateRect(HWND hWnd,LPRECT lpr,BOOL fErase)
{
    WORD	wClassStyle;
    HDC	hDC;
    BOOL rc;

    APISTR((LF_APICALL,"GetUpdateRect(HWND=%x,LPRECT=%p,BOOL=%d\n",
	hWnd,lpr,fErase));

    if (!lpr) {
        APISTR((LF_APIFAIL,"GetUpdateRect: returns BOOL %d\n",FALSE));
	return FALSE;
    }

    (void) GetUpdateRgnEx(hWnd,lpr,(HRGN)0,fErase);
    if ((wClassStyle = GetClassStyle(hWnd)) & CS_OWNDC) {
	if ((hDC = GetOwnDC(hWnd)))
	    DPtoLP(hDC,(POINT *)lpr,2);
    }

    rc = !IsRectEmpty(lpr);
    APISTR((LF_APIRET,"GetUpdateRect: returns BOOL %d\n",rc));
    return rc;
}

int
GetUpdateRgnEx(HWND hWnd,LPRECT lpr,HRGN hRgn,BOOL bErase)
{
    REGION32 Rgn32, Rgn32Update;

    if (hRgn) {
	if (!(Rgn32 = TWIN_InternalGetRegionData(hRgn)))
	    return ERROR;
    }
    else
	Rgn32 = (REGION32)0;

    if (!(Rgn32Update = (REGION32) GetWindowLong(hWnd,GWL_UPDATE))) {
	if(lpr)
	    SetRectEmpty(lpr);
	if (Rgn32)
	    (void)DRVCALL_REGIONS(PRH_SETREGIONEMPTY,Rgn32,0,0);
	return NULLREGION;
    }

    if (!lpr && !Rgn32)
	return NULLREGION;

    if (lpr) {
	(void)DRVCALL_REGIONS(PRH_REGIONBOX,Rgn32Update,0,lpr);
    }
    else {
	if (Rgn32) {
	    (void)DRVCALL_REGIONS(PRH_COPYREGION,Rgn32Update,Rgn32,0);
	}
    }

    if (bErase)
	InternalPaintWindows(hWnd,RDW_ERASENOW|RDW_NOCHILDREN);

    return COMPLEXREGION;
}

/*****************************************************************************/
/* these are convinience functions to deal with update regions */

int
AddRgnToUpdate(HWND hWnd, HRGN hRgn)
{
    REGION32 Rgn32Update, Rgn32;
    HRGN hRgnClient;
    RECT   rcTemp;
    int rc;

    if ((Rgn32Update = (REGION32)GetWindowLong(hWnd,GWL_UPDATE))) {
	GetClientRect(hWnd, &rcTemp);
	if (hRgn) {
	    if (!(hRgnClient = CreateRectRgnIndirect(&rcTemp)))
		return ERROR;

	    IntersectRgn(hRgnClient,hRgnClient,hRgn);
	    Rgn32 = TWIN_InternalGetRegionData(hRgnClient);
	    rc = (int)DRVCALL_REGIONS(PRH_UNIONREGION,
			Rgn32,Rgn32Update,Rgn32Update);
	    DeleteObject(hRgnClient);
	}
	else
	    rc = (int)DRVCALL_REGIONS(PRH_UNIONRECTWITHREGION,
			Rgn32Update,0,&rcTemp);
	return rc;
    }

    return NULLREGION;
}

int
AddRectToUpdate(HWND hWnd, const RECT *lpRect)
{
    RECT rcClient;
    REGION32 Rgn32Update;

    if ((Rgn32Update = (REGION32)GetWindowLong(hWnd,GWL_UPDATE))) {
	GetClientRect(hWnd,&rcClient);
	IntersectRect(&rcClient,&rcClient,lpRect);
	return (int)DRVCALL_REGIONS(PRH_UNIONRECTWITHREGION,
			Rgn32Update,0,&rcClient);
    }

    return NULLREGION;
}

int
SubtractRgnFromUpdate(HWND hWnd, HRGN hRgn)
{
    REGION32 Rgn32Src,Rgn32Update;

    if ((Rgn32Update = (REGION32)GetWindowLong(hWnd,GWL_UPDATE))) {
	if ((BOOL)DRVCALL_REGIONS(PRH_ISEMPTYREGION,Rgn32Update,0,0)) {
	    return NULLREGION;
    	}
	if (hRgn) {
	    Rgn32Src = TWIN_InternalGetRegionData(hRgn);
	    if ((int)DRVCALL_REGIONS(PRH_DIFFREGION,
		Rgn32Update,Rgn32Src,Rgn32Update) == NULLREGION) {
		return NULLREGION;
	    }
	    else
		return COMPLEXREGION;
	}
	else {
	    (void)DRVCALL_REGIONS(PRH_SETREGIONEMPTY,Rgn32Update,0,0);
	    return NULLREGION;
	}
    }
    return NULLREGION;
}

int
SubtractRectFromUpdate(HWND hWnd, const RECT *lpRect)
{
    HRGN hRgn;
    int rc;

    if (!lpRect)
	return SubtractRgnFromUpdate(hWnd,(HRGN)0);
    if (!(hRgn = CreateRectRgnIndirect(lpRect)))
	return ERROR;
    rc = SubtractRgnFromUpdate(hWnd,hRgn);
    DeleteObject(hRgn);
    return rc;
}

/*****************************************************************************/

HRGN
GdiCreateRegion(LPSTR lpRgnData)
{
    HRGN hRegion;
    LPREGIONINFO lpRegInfo;

    lpRegInfo = CREATEHRGN(hRegion);

    if (!lpRegInfo) {
	return (HRGN)0;
    }
    lpRegInfo->Rgn32 = (REGION32)lpRgnData;
    RELEASEREGIONINFO(lpRegInfo);
    return hRegion;
}

/* (WIN32) GDI Region ****************************************************** */

HRGN	WINAPI
ExtCreateRegion(CONST XFORM *lpXform,
	DWORD dwRgnDataSize,
	CONST RGNDATA *lpRgnData)
{
	DWORD dwRectCount;
	CONST RECT *lpRect;
	HRGN hRgn, hRectRgn;
	int nRgnMode;

	APISTR((LF_APICALL,"ExtCreateRegion(XFORM=%p,DWORD=%x,RGNDATA*=%p)\n",
		lpXform,dwRgnDataSize,lpRgnData));

	if ((dwRgnDataSize < sizeof(RGNDATA))
	 || (lpRgnData == NULL)
	 || (lpRgnData->rdh.dwSize < sizeof(RGNDATAHEADER))
	 || (lpRgnData->rdh.iType != RDH_RECTANGLES)
	 || (lpRgnData->rdh.nCount <= 0)
	 || ((lpRgnData->rdh.nRgnSize != 0)
	  && (lpRgnData->rdh.nRgnSize < lpRgnData->rdh.nCount * sizeof(RECT))))
	{
		SetLastErrorEx(1, 0);
        	APISTR((LF_APIFAIL,"ExtCreateRegion: returns HRGN 0\n"));
		return (HRGN)0;
	}

	/* For dwRectCount > 0,
	 *	lpRect = region rectangle in RGNDATA->Buffer
	 *	nRgnMode = RGN_OR
	 *
	 * For dwRectCount == 0,
	 *	lpRect = bounding rectangle in RGNDATAHEADER
	 *	nRgnMode = RGN_AND
	 *
	 * This allows us to implement bounding rectangle without retyping
	 * entire code within for loop twice.
	 */
	hRgn = (HRGN)0;
	nRgnMode = RGN_OR;
	for (dwRectCount = lpRgnData->rdh.nCount,
	     lpRect = (CONST RECT *) (lpRgnData->Buffer);
	     ;
	     dwRectCount--, lpRect++)
	{
		if (dwRectCount == 0)
		{
			lpRect = &(lpRgnData->rdh.rcBound);
			nRgnMode = RGN_AND;
		}
		if (lpXform == NULL)
		{
			hRectRgn = CreateRectRgnIndirect(lpRect);
		}
		else
		{
			/* We cannot call CreateRectRgn*() because after
			 * appling a shear or rotate transform, the resulting
			 * shape may not have edges parallel to the xy axis
			 * (and hence we won't be able to fully specify the
			 * shape simply by giving a upper-left and lower-right
			 * coordinate in a RECT structure).
			 */
			int i, x, y;
			POINT pt[4];
			pt[0].x = lpRect->left;  pt[0].y = lpRect->top;
			pt[1].x = lpRect->right; pt[1].y = lpRect->top;
			pt[2].x = lpRect->right; pt[2].y = lpRect->bottom;
			pt[3].x = lpRect->left;  pt[3].y = lpRect->bottom;
			for (i = 0; i < sizeof(pt)/sizeof(pt[0]); i++)
			{
				x = pt[i].x * lpXform->eM11
					+ pt[i].y * lpXform->eM21
					+ lpXform->eDx;
				y = pt[i].x * lpXform->eM12
					+ pt[i].y * lpXform->eM22
					+ lpXform->eDy;
				pt[i].x = x;
				pt[i].y = y;
			}
			hRectRgn = CreatePolygonRgn(pt,
				sizeof(pt)/sizeof(pt[0]), WINDING);
		}
		if (!hRectRgn)
		{
			if (hRgn)
				DeleteObject(hRgn);
			SetLastErrorEx(1, 0);
        		APISTR((LF_APIFAIL,
				"ExtCreateRegion: returns HRGN 0\n"));
			return (HRGN)0;
		}
		if (!hRgn)
			hRgn = hRectRgn;
		else
		{
			if (CombineRgn(hRgn, hRgn, hRectRgn, nRgnMode)
				== ERROR)
			{
				DeleteObject(hRgn);
				DeleteObject(hRectRgn);
				SetLastErrorEx(1, 0);
        			APISTR((LF_APIFAIL,
					"ExtCreateRegion: returns HRGN 0\n"));
				return (HRGN)0;
			}
			DeleteObject(hRectRgn);
		}
		if (dwRectCount == 0)
			break;
	}

        APISTR((LF_APIRET,"ExtCreateRegion: returns HRGN %x\n",hRgn));
	return (hRgn);
}

DWORD	WINAPI
GetRegionData(HRGN hRgn,
	DWORD dwCount,
	LPRGNDATA lpRgnData)
{
	REGION32 Rgn32;
 	DWORD    rc;

	APISTR((LF_APICALL, "GetRegionData(HRGN=%x,DWORD=%x,LPRGNDATA=%p)\n", 
		hRgn, dwCount,lpRgnData));

	Rgn32 = TWIN_InternalGetRegionData(hRgn);
	if (!Rgn32)
	{
		SetLastErrorEx(1, 0);
        	APISTR((LF_APIFAIL, "GetRegionData: returns DWORD 0\n"));
		return (0);
	}

	rc = ((DWORD) DRVCALL_REGIONS(PRH_GETREGIONDATA, Rgn32, dwCount,
		lpRgnData));
	
        APISTR((LF_APIRET,"GetRegionData: returns DWORD %x\n",rc));
	return rc;
}

BOOL	WINAPI
SelectClipPath(HDC hDC,
	int nClipMode)
{
	HRGN hClipRgn;
	int nClipResult;
	BOOL rc;

	APISTR((LF_APICALL, "SelectClipPath(HDC=%x,int=%d)\n",
		hDC, nClipMode));

	hClipRgn = PathToRegion(hDC);
	if (!hClipRgn) {
        	APISTR((LF_APIFAIL,"SelectClipPath: returns BOOL 0\n"));
		return (FALSE);
	}

	nClipResult = ExtSelectClipRgn(hDC, hClipRgn, nClipMode);

	if (DeleteObject(hClipRgn) == FALSE) {
        	APISTR((LF_APIFAIL,"SelectClipPath: returns BOOL 0\n"));
		return (FALSE);
	}

	rc = (nClipResult == ERROR) ? FALSE : TRUE;
        APISTR((LF_APIRET,"SelectClipPath: returns BOOL %d\n",rc));
	return rc;
}

