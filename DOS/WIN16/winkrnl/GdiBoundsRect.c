/*    
	GdiBoundsRect.c	1.7
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

/*
lsd_br_passthru     DONE
lsd_br_lineto       DONE
lsd_br_polyline     DONE
lsd_br_rectangle    DONE
lsd_br_polypolygon  DONE
lsd_br_floodfill      TODO
lsd_br_extfloodfill   TODO
lsd_br_framergn     DONE
lsd_br_paintrgn     DONE
lsd_br_textout      DONE
lsd_br_exttextout   DONE
lsd_br_stretchblt   DONE
lsd_br_setpixel     DONE
lsd_br_scrolldc     DONE
*/

#include "stdlib.h"
#include "windows.h"

#include "kerndef.h"
#include "Log.h"
#include "GdiDC.h"
#include "DeviceData.h"

#define DC_BOUNDS_ON(hDC32) { \
	if ((hDC32)->Output[0] == (hDC32)->Output[2]) \
	    (hDC32)->Output[0] = (hDC32)->lpLSDEntry->BoundsRectProc; \
	else if ((hDC32)->Output[0] != (hDC32)->lpLSDEntry->BoundsRectProc) \
	    (hDC32)->Output[1] = (hDC32)->lpLSDEntry->BoundsRectProc; }

#define DC_BOUNDS_OFF(hDC32) { \
	if ((hDC32)->Output[0] == (hDC32)->lpLSDEntry->BoundsRectProc) \
	    (hDC32)->Output[0] = (hDC32)->Output[2]; \
	else if ((hDC32)->Output[1] == (hDC32)->lpLSDEntry->BoundsRectProc) \
	    (hDC32)->Output[1] = (hDC32)->Output[2]; }

/********************************************************************
********************************************************************/
UINT WINAPI
SetBoundsRect(HDC hdc, const RECT *lprcBounds, UINT uiFlags)
{
   HDC32	hDC32;
   RECT 	rcTmp;
   UINT         uiPrev; /* previous bounds rect flag */

   APISTR((LF_API,
	"SetBoundsRect: hdc, '%04X', left '%d', top '%d', right '%d', bottom '%d', flags '%04X'\n",
	hdc, lprcBounds->left, lprcBounds->top, lprcBounds->right, lprcBounds->bottom, uiFlags));


   ASSERT_HDC(hDC32, hdc, 0);

   if ((uiFlags & DCB_ACCUMULATE) || (uiFlags & DCB_ENABLE))
      DC_BOUNDS_ON(hDC32)
   else DC_BOUNDS_OFF(hDC32)

   uiPrev = hDC32->BoundsFlag;
   hDC32->BoundsFlag = uiFlags;

   if (hDC32->BoundsFlag & DCB_ACCUMULATE)
      {
      /*   make a union of old and new bounds rectangles
      **   and store the result in hdc.
      */
      UnionRect(&rcTmp, lprcBounds, &hDC32->BoundsRect);
      CopyRect(&hDC32->BoundsRect, &rcTmp);
      }
   else if (hDC32->BoundsFlag & DCB_ENABLE)
	   /*
	   **	store the new bounds rectangle in hdc
	   */
           CopyRect(&hDC32->BoundsRect, lprcBounds);
	else if (hDC32->BoundsFlag & DCB_DISABLE)
		/*
		**   empty bounds rectangle in hdc
		*/
                SetRectEmpty(&hDC32->BoundsRect);

   RELEASEDCINFO(hDC32);

   return uiPrev;
}

/********************************************************************
********************************************************************/
UINT WINAPI
GetBoundsRect(HDC hdc, LPRECT lprcBounds, UINT uiFlags)
{
   HDC32	hDC32;
   UINT		uiRet;

   APISTR((LF_API, "GetBoundsRect: hdc, '%04X', flags '%04X'\n", hdc, uiFlags)); 
   ASSERT_HDC(hDC32, hdc, 0);

   CopyRect(lprcBounds, &hDC32->BoundsRect);
   /*
   **   TODO: GetBoundsRect can reset the bounding rectangle...
   */
   uiRet = (IsRectEmpty(&hDC32->BoundsRect) ? DCB_RESET : DCB_SET);

   RELEASEDCINFO(hDC32);

   return uiRet;
}


/********************************************************************
**   Helper functions
********************************************************************/
DWORD   /* LSD_CREATEDC/IC      1 */
lsd_br_passthru(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_LINETO           20 */
lsd_br_lineto(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    POINT    pt;
    LPPOINT  lppt = &lpStruct->lsde.point;
    /*
    **   Adjust hDC32->BoundsRect if given point lies
    **   outside the current bounds rect.
    */
    GetCurrentPositionEx(GETHDC16(hDC32), &pt);
    /*
    ** TODO: move the code below to a separate static
    **       function.
    */
    if (!PtInRect(&hDC32->BoundsRect, pt))
       {
       /*   fix up horizontal coordinate
       */
       if (pt.x < hDC32->BoundsRect.left)
          hDC32->BoundsRect.left = pt.x;
       else if (pt.x > hDC32->BoundsRect.right)
               hDC32->BoundsRect.right = pt.x;
       /*
       **   fix up vertical coordinate
       */
       if (pt.y < hDC32->BoundsRect.top)
          hDC32->BoundsRect.top = pt.y;
       else if (pt.y > hDC32->BoundsRect.bottom)
               hDC32->BoundsRect.bottom = pt.y;
       }

    if (!PtInRect(&hDC32->BoundsRect, *lppt))
       {
       /*   fix up horizontal coordinate
       */
       if (lppt->x < hDC32->BoundsRect.left)
          hDC32->BoundsRect.left = lppt->x;
       else if (lppt->x > hDC32->BoundsRect.right)
               hDC32->BoundsRect.right = lppt->x;
       /*
       **   fix up vertival coordinate
       */
       if (lppt->y < hDC32->BoundsRect.top)
          hDC32->BoundsRect.top = lppt->y;
       else if (lppt->y > hDC32->BoundsRect.bottom)
               hDC32->BoundsRect.bottom = lppt->y;
       }
    return DC_BR_OUTPUT(msg, hDC32, dwParam, lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_POLYLINE         21 */
lsd_br_polyline(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    register i;
    LPPOINT  lppt;
    LSDE_POLYPOLY *poly = &lpStruct->lsde.polypoly;
    /*
    **  poly->lpPoints = lpPoints;
    **  poly->lpCounts = 0;
    **  poly->nCount   = 1;
    **  poly->nTotalCount = nCount; (see GdiGraphics.c)
    */
    for (i = 0, lppt = poly->lpPoints;  i < poly->nTotalCount;  i++, lppt++)
        {
	if (!PtInRect(&hDC32->BoundsRect, *lppt))
           {
           /*   fix up horizontal coordinate
           */
           if (lppt->x < hDC32->BoundsRect.left)
              hDC32->BoundsRect.left = lppt->x;
           else if (lppt->x > hDC32->BoundsRect.right)
                   hDC32->BoundsRect.right = lppt->x;
           /*
           **   fix up vertical coordinate
           */
           if (lppt->y < hDC32->BoundsRect.top)
              hDC32->BoundsRect.top = lppt->y;
           else if (lppt->y > hDC32->BoundsRect.bottom)
                   hDC32->BoundsRect.bottom = lppt->y;
	   }
        }
    return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_RECTANGLE        22 */
lsd_br_rectangle(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    LPRECT lprc = &lpStruct->lsde.rect;
    /*
    **   Adjust hDC32->BoundsRect if given rectangle intersects
    **   with the current bounds rect.
    */
    if (lprc->left < hDC32->BoundsRect.left)
       hDC32->BoundsRect.left = lprc->left;
    if (lprc->top < hDC32->BoundsRect.top)
       hDC32->BoundsRect.top = lprc->top;
    if (lprc->right > hDC32->BoundsRect.right)
       hDC32->BoundsRect.right = lprc->right;
    if (lprc->bottom > hDC32->BoundsRect.bottom)
       hDC32->BoundsRect.bottom = lprc->bottom;

    return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_POLYPOLYGON      29 */
lsd_br_polypolygon(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    register i;
    LPPOINT  lppt;
    LSDE_POLYPOLY *poly = &lpStruct->lsde.polypoly;
    /*
    **  poly->lpPoints = lpPoints; -- array of points
    **  poly->lpCounts = lpPoly;   -- array of # of points in each poligon
    **  poly->nCount   = nCount;   -- # of poligons
    **  poly->nTotalCount = n;     -- total number of points (see GdiGraphics.c)
    */
    for (i = 0, lppt = poly->lpPoints;  i < poly->nTotalCount;  i++, lppt++)
        {
        if (!PtInRect(&hDC32->BoundsRect, *lppt))
           {
           /*   fix up horizontal coordinate
           */
           if (lppt->x < hDC32->BoundsRect.left)
              hDC32->BoundsRect.left = lppt->x;
           else if (lppt->x > hDC32->BoundsRect.right)
                   hDC32->BoundsRect.right = lppt->x;
           /*
           **   fix up vertical coordinate
           */
           if (lppt->y < hDC32->BoundsRect.top)
              hDC32->BoundsRect.top = lppt->y;
           else if (lppt->y > hDC32->BoundsRect.bottom)
                   hDC32->BoundsRect.bottom = lppt->y;
           }
        }

    return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_FLOODFILL        30 */
lsd_br_floodfill(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_EXTFLOODFILL     31 */
lsd_br_extfloodfill(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_FRAMERGN         33 */
lsd_br_framergn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
   RECT rcRgn, rcTmp;
   LSDE_PAINTRGN *lprgn = &lpStruct->lsde.paintrgn;
   /*
   **   lsde.paintrgn.hRgn    = hRgn;
   **   lsde.paintrgn.nWidth  = nWidth;
   **   lsde.paintrgn.nHeight = nHeight;
   */
   GetRgnBox(lprgn->hRgn, &rcRgn);
   rcRgn.left	-= lprgn->nWidth;  /* add brush width on both horizontal sides */
   rcRgn.right	+= lprgn->nWidth;
   rcRgn.top	-= lprgn->nHeight; /* add brush height on both vertical sides */
   rcRgn.bottom += lprgn->nHeight;
   UnionRect(&rcTmp, &rcRgn, &hDC32->BoundsRect);
   CopyRect(&hDC32->BoundsRect, &rcTmp);

   return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_PAINTRGN         35 */
lsd_br_paintrgn(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
   RECT rcRgn, rcTmp;
   LSDE_PAINTRGN *lprgn = &lpStruct->lsde.paintrgn;
   /*
   **   lsde.paintrgn.hRgn = hRgn;
   */
   GetRgnBox(lprgn->hRgn, &rcRgn);
   UnionRect(&rcTmp, &rcRgn, &hDC32->BoundsRect);
   CopyRect(&hDC32->BoundsRect, &rcTmp);

   return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_TEXTOUT          36 */
lsd_br_textout(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    DWORD    ext;
    RECT     rc, rcTmp;
    LSDE_TEXT *lptext = &lpStruct->lsde.text;
    /*
    **   lptext->x     = x;
    **   lptext->y     = y;
    **   lptext->lpStr = lpStr;
    **   lptext->nCnt  = nCnt;     (see GdiText.c)
    */
    ext = GetTextExtent(GETHDC16(hDC32), lptext->lpStr, lptext->nCnt);
    rc.left    = lptext->x;
    rc.top     = lptext->y;
    rc.right   = lptext->x + (int)(short)LOWORD(ext);
    rc.bottom  = lptext->y + (int)(short)HIWORD(ext);

    UnionRect(&rcTmp, &rc, &hDC32->BoundsRect);
    CopyRect(&hDC32->BoundsRect, &rcTmp);

    return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_EXTTEXTOUT       37 */
lsd_br_exttextout(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    register i;
    POINT    pt;
    SIZE     size;
    RECT     rc, rcTmp;
    LPINT    lpint;
    LSDE_TEXT *lptext = &lpStruct->lsde.text;
    /*
    **   lptext->x       = x;
    **   lptext->y       = y;
    **   lptext->lpStr   = lpStr;
    **   lptext->nCnt    = nCnt;
    **   lptext->uiFlags = uiFlags;
    **   lptext->lpRect  = lpRect;
    **   lptext->lpDX    = lpDX;         (see GdiText.c)
    */
    if (lptext->TextAlign & TA_UPDATECP)
       GetCurrentPositionEx(GETHDC16(hDC32), (LPPOINT)&pt);
    else {
         pt.x = lptext->x;
         pt.y = lptext->y;
         }
    /*
    **   Adjust coordinates for clipped text
    */
    if (lptext->uiFlags & ETO_CLIPPED)
       {
       /*   If the x,y of the text is outside clipping
       **   rectangle, do nothing, don't even draw a rectangle...
       */
       if (!PtInRect(lptext->lpRect, pt))
          return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
       /*
       **   Use the rectangle which comes with ExtTextOut, since
       **   the text is to be clipped inside it anyway...
       */
       if (lptext->lpRect->left < hDC32->BoundsRect.left)
          hDC32->BoundsRect.left = lptext->lpRect->left;
       if (lptext->lpRect->top < hDC32->BoundsRect.top)
          hDC32->BoundsRect.top = lptext->lpRect->top;
       if (lptext->lpRect->right > hDC32->BoundsRect.right)
          hDC32->BoundsRect.right = lptext->lpRect->right;
       if (lptext->lpRect->bottom > hDC32->BoundsRect.bottom)
          hDC32->BoundsRect.bottom = lptext->lpRect->bottom;
       }
    else {
         /*   Manually calculate width and height of the given string...
         */
         GetTextExtentPoint(GETHDC16(hDC32), lptext->lpStr, lptext->nCnt, &size);
         if (lptext->lpDX != NULL)
            {
            /*   Since GetTextExtentPoint() takes into account current
            **   intercharacter spacing set by SetTextCharacterExtra()
            **   we should substract it from the width. Then just
            **   go througt lpDX array and add specific spacing value.
            */
	    size.cx -= GetTextCharacterExtra(GETHDC16(hDC32)) * (lptext->nCnt - 1);
            for (i = 0, lpint = lptext->lpDX;
                    i < lptext->nCnt && lpint;
                        i++, lpint++)
                size.cx += *lpint;
            }
         /*
         **   Make a union rectangle of x,y an lpRect,
         **   since in ETO_OPAQUE mode ExtTextOut draws
         **   both string and rectangle regardless of
         **   their position.
         */
	 rcTmp.left   = pt.x;
	 rcTmp.top    = pt.y;
	 rcTmp.right  = pt.x + size.cx;
	 rcTmp.bottom = pt.y + size.cy;
	 UnionRect(&rc, &rcTmp, lptext->lpRect);

         if (rc.left < hDC32->BoundsRect.left)
            hDC32->BoundsRect.left = rc.left;
         if (rc.top < hDC32->BoundsRect.top)
            hDC32->BoundsRect.top = rc.top;
         if (rc.right > hDC32->BoundsRect.right)
            hDC32->BoundsRect.right = rc.right;
         if (rc.bottom > hDC32->BoundsRect.bottom)
            hDC32->BoundsRect.bottom = rc.bottom;
         }

    return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

/********************************************************************
********************************************************************/
DWORD   /* LSD_STRETCHBLT       45 */
lsd_br_stretchblt(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
   RECT rcBmp, rcTmp;
   LSDE_STRETCHDATA *lpsd = &lpStruct->lsde.stretchdata;
   /*
   **	lpsd->xDest	  = nXOriginDest;
   **	lpsd->yDest	  = nYOriginDest;
   **	lpsd->nWidthDest  = nWidthDest;
   **	lpsd->nHeightDest = nHeightDest; (see GdiGraphics.c)
   */
   rcBmp.left	= lpsd->xDest;
   rcBmp.top	= lpsd->yDest;
   rcBmp.right	= lpsd->xDest + lpsd->nWidthDest;
   rcBmp.bottom = lpsd->yDest + lpsd->nHeightDest;

   UnionRect(&rcTmp, &rcBmp, &hDC32->BoundsRect);
   CopyRect(&hDC32->BoundsRect, &rcTmp);

   return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_SETPIXEL         47 */
lsd_br_setpixel(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    LPPOINT  lppt = &lpStruct->lsde.point;
    /*
    **   Adjust hDC32->BoundsRect if given point lies
    **   outside the current bounds rect.
    */
    if (!PtInRect(&hDC32->BoundsRect, *lppt))
       {
       /*   fix up horizontal coordinate
       */
       if (lppt->x < hDC32->BoundsRect.left)
          hDC32->BoundsRect.left = lppt->x;
       else if (lppt->x > hDC32->BoundsRect.right)
               hDC32->BoundsRect.right = lppt->x;
       /*
       **   fix up vertical coordinate
       */
       if (lppt->y < hDC32->BoundsRect.top)
          hDC32->BoundsRect.top = lppt->y;
       else if (lppt->y > hDC32->BoundsRect.bottom)
               hDC32->BoundsRect.bottom = lppt->y;
       }
    return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}


/********************************************************************
********************************************************************/
DWORD   /* LSD_SCROLLDC         50 */
lsd_br_scrolldc(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
   RECT  rcBmp, rcTmp;
   LSDE_SCROLLDC  *lpsdc = &lpStruct->lsde.scrolldc;
   /*
   **	scrolldc.xSrc	 = rc.left - dx;
   **	scrolldc.ySrc	 = rc.top  - dy;
   **	scrolldc.xDest	 = rc.left;
   **	scrolldc.yDest	 = rc.top;
   **	scrolldc.nWidth  = rc.right - rc.left;
   **	scrolldc.nHeight = rc.bottom - rc.top;
   */
   rcBmp.left	= lpsdc->xSrc;
   rcBmp.top	= lpsdc->ySrc;
   rcBmp.right	= lpsdc->xSrc + lpsdc->nWidth;
   rcBmp.bottom = lpsdc->ySrc + lpsdc->nHeight;

   UnionRect(&rcTmp, &rcBmp, &hDC32->BoundsRect);
   CopyRect(&hDC32->BoundsRect, &rcTmp);

   return DC_BR_OUTPUT(msg,hDC32,dwParam,lpStruct);
}

DWORD
lsd_br_worldblt(WORD wMsg,
	HDC32 hDC32,
	DWORD dwParam,
	LPLSDS_PARAMS lpParam)
{
	if (lpParam)
	{
		LPLSDE_WORLDBLT lpWorldBltParam = &(lpParam->lsde.worldblt);
		POINT pt[4];
		int i;
		/* compute four corners of destination parallelogram */
		pt[0] = lpWorldBltParam->ptDestXY[0];
		pt[1] = lpWorldBltParam->ptDestXY[1];
		pt[2] = lpWorldBltParam->ptDestXY[2];
		pt[3].x = pt[1].x + pt[2].x - pt[0].x;
		pt[3].y = pt[1].y + pt[2].y - pt[0].y;
		/* adjust bounding rectangle to include all four corners */
		for (i = 0; i < 4; i++)
		{
			if (pt[i].x < hDC32->BoundsRect.left)
				hDC32->BoundsRect.left = pt[i].x;
			else if (pt[i].x > hDC32->BoundsRect.right)
				hDC32->BoundsRect.right = pt[i].x;
			if (pt[i].y < hDC32->BoundsRect.top)
				hDC32->BoundsRect.top = pt[i].y;
			else if (pt[i].y > hDC32->BoundsRect.bottom)
				hDC32->BoundsRect.bottom = pt[i].y;
		}
	}
	return DC_BR_OUTPUT(wMsg, hDC32, dwParam, lpParam);
}

