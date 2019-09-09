/*    
	GdiDCAttr.c	2.25
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

#include <stdlib.h>

#include "windows.h"

#include "GdiDC.h"
#include "GdiDCAttr.h"
#include "Log.h"
#include "DeviceData.h"
#include "Driver.h"
#include <string.h>

#define	DC_MAPMODE_ON(hDC32) { \
	if ((hDC32)->Output[0] != (hDC32)->lpLSDEntry->MapModeProc) { \
	    (hDC32)->Output[1] = (hDC32)->Output[0]; \
	    (hDC32)->Output[0] = (hDC32)->lpLSDEntry->MapModeProc; \
	}}

#define	DC_MAPMODE_OFF(hDC32) { \
	if ((hDC32)->Output[0] == (hDC32)->lpLSDEntry->MapModeProc) { \
	    (hDC32)->Output[0] = (hDC32)->Output[1]; \
	    (hDC32)->Output[1] = (hDC32)->Output[2]; \
	}}

#define	DC_CAN_SWITCH_OFF_MAPPING(hDC32) ( \
	(hDC32)->eM11 == 1.0 && (hDC32)->eM12 == 0.0 && \
	(hDC32)->eM21 == 0.0 && (hDC32)->eM22 == 1.0 && \
	(hDC32)->eDx  == 0.0 && (hDC32)->eDx  == 0.0 && \
	(hDC32)->MapMode == MM_TEXT && \
	(hDC32)->FOx == 0 && \
	(hDC32)->FOy == 0 && \
	(hDC32)->DOx == 0 && \
	(hDC32)->DOy == 0 && \
	(hDC32)->WOx == 0 && \
	(hDC32)->WOy == 0 && \
	(hDC32)->VOx == 0 && \
	(hDC32)->VOy == 0 && \
	(hDC32)->WEx == 1 && \
	(hDC32)->WEy == 1 && \
	(hDC32)->VEx == 1 && \
	(hDC32)->VEy == 1)

static DWORD GdiDCAttributes(HDC32,DWORD,DWORD,LPLSDS_PARAMS);
static void GdiAdjustViewportExt(HDC32);
static void GdiSetMapMode(HDC32, int);
static void TextJustifyBresenham(int, int, LPINT);

/* (WIN32) GDI Graphics Mode *********************************************** */

int	WINAPI
GetGraphicsMode(HDC hDC)
{
	HDC32 hDC32;
	int nGraphicsMode;

	ASSERT_HDC(hDC32, hDC, 0);
	nGraphicsMode = hDC32->nGraphicsMode;
	RELEASEDCINFO(hDC32);

	return (nGraphicsMode);

}

int	WINAPI
SetGraphicsMode(HDC hDC, int nGraphicsMode)
{
	HDC32 hDC32;
	int nOldGraphicsMode;

	ASSERT_HDC(hDC32, hDC, 0);
	nOldGraphicsMode = hDC32->nGraphicsMode;
	switch (nGraphicsMode)
	{
	case GM_COMPATIBLE:
		/* prerequisite for GM_COMPATIBLE graphics mode
		 *	- identity world transform
		 */
		if ((hDC32->eM11 != 1.0) || (hDC32->eM12 != 0.0)
		 || (hDC32->eM21 != 0.0) || (hDC32->eM22 != 1.0)
		 || (hDC32->eDx  != 0.0) || (hDC32->eDy  != 0.0))
		{
			SetLastErrorEx(1, 0);
			nOldGraphicsMode = 0;
			break;
		}
		/* set GM_COMPATIBLE graphics mode */
		hDC32->nGraphicsMode = GM_COMPATIBLE;
		break;
	case GM_ADVANCED:
		/* set GM_ADVANCED graphics mode */
		hDC32->nGraphicsMode = GM_ADVANCED;
		break;
	default:
		SetLastErrorEx(1, 0);
		break;
	}
	RELEASEDCINFO(hDC32);

	return (nOldGraphicsMode);

}

/* (WIN32) GDI World Coordinate Transform ********************************** */

static void twin_MultiplyMatrix(double *eM11, double *eM12,
	double *eM21, double *eM22,
	double *eDx,  double *eDy,
	double eAM11, double eAM12,
	double eAM21, double eAM22,
	double eADx,  double eADy,
	double eBM11, double eBM12,
	double eBM21, double eBM22,
	double eBDx,  double eBDy)
{
	*eM11 = eAM11 * eBM11 + eAM12 * eBM21;
	*eM12 = eAM11 * eBM12 + eAM12 * eBM22;
	*eM21 = eAM21 * eBM11 + eAM22 * eBM21;
	*eM22 = eAM21 * eBM12 + eAM22 * eBM22;
	*eDx  = eADx  * eBM11 + eADy  * eBM21 + eBDx;
	*eDy  = eADx  * eBM12 + eADy  * eBM22 + eBDy;
}

BOOL	WINAPI
CombineTransform(LPXFORM lpXformResult, LPXFORM lpXform1, LPXFORM lpXform2)
{
	double eM11, eM12, eM21, eM22, eDx, eDy;

	if (!lpXformResult || !lpXform1 || !lpXform2)
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}

	twin_MultiplyMatrix(&eM11, &eM12, &eM21, &eM22, &eDx, &eDy,
		lpXform1->eM11, lpXform1->eM12,
		lpXform1->eM21, lpXform1->eM22,
		lpXform1->eDx,  lpXform1->eDy,
		lpXform2->eM11, lpXform2->eM12,
		lpXform2->eM21, lpXform2->eM22,
		lpXform2->eDx,  lpXform2->eDy);
	lpXformResult->eM11 = eM11; lpXformResult->eM12 = eM12;
	lpXformResult->eM21 = eM21; lpXformResult->eM22 = eM22;
	lpXformResult->eDx  = eDx;  lpXformResult->eDy  = eDy;

	return (TRUE);

}

BOOL	WINAPI
GetWorldTransform(HDC hDC, LPXFORM lpXform)
{
	HDC32 hDC32;

	if (lpXform == NULL)
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}

	ASSERT_HDC(hDC32, hDC, FALSE);
	lpXform->eM11 = hDC32->eM11; lpXform->eM12 = hDC32->eM12;
	lpXform->eM21 = hDC32->eM21; lpXform->eM22 = hDC32->eM22;
	lpXform->eDx  = hDC32->eDx;  lpXform->eDy  = hDC32->eDy;
	RELEASEDCINFO(hDC32);

	return (TRUE);

}

BOOL	WINAPI
ModifyWorldTransform(HDC hDC, LPXFORM lpXform, DWORD dwMode)
{
	HDC32 hDC32;
	BOOL bSuccess;

	if (lpXform == NULL)
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}

	ASSERT_HDC(hDC32, hDC, FALSE);
	bSuccess = FALSE;
	switch (hDC32->nGraphicsMode)
	{
	case GM_COMPATIBLE:
		SetLastErrorEx(1, 0);
		break;
	case GM_ADVANCED:
		switch (dwMode)
		{
		case MWT_IDENTITY:
			hDC32->eM11 = hDC32->eM22 = 1.0;
			hDC32->eM12 = hDC32->eM21 =
				hDC32->eDx = hDC32->eDy = 0.0;
			bSuccess = TRUE;
			break;
		case MWT_LEFTMULTIPLY:
			twin_MultiplyMatrix(&hDC32->eM11, &hDC32->eM12,
				&hDC32->eM21, &hDC32->eM22,
				&hDC32->eDx,  &hDC32->eDy,
				lpXform->eM11, lpXform->eM12,
				lpXform->eM21, lpXform->eM22,
				lpXform->eDx,  lpXform->eDy,
				hDC32->eM11, hDC32->eM12,
				hDC32->eM21, hDC32->eM22,
				hDC32->eDx,  hDC32->eDy);
			bSuccess = TRUE;
			break;
		case MWT_RIGHTMULTIPLY:
			twin_MultiplyMatrix(&hDC32->eM11, &hDC32->eM12,
				&hDC32->eM21, &hDC32->eM22,
				&hDC32->eDx,  &hDC32->eDy,
				hDC32->eM11, hDC32->eM12,
				hDC32->eM21, hDC32->eM22,
				hDC32->eDx,  hDC32->eDy,
				lpXform->eM11, lpXform->eM12,
				lpXform->eM21, lpXform->eM22,
				lpXform->eDx,  lpXform->eDy);
			bSuccess = TRUE;
			break;
		default:
			SetLastErrorEx(1, 0);
			break;
		}
		break;
	default:
		SetLastErrorEx(1, 0);
		break;
	}
	RELEASEDCINFO(hDC32);

	return (bSuccess);

}

BOOL	WINAPI
SetWorldTransform(HDC hDC, LPXFORM lpXform)
{
	HDC32 hDC32;
	BOOL bSuccess;

	if (lpXform == NULL)
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}

	ASSERT_HDC(hDC32, hDC, FALSE);
	bSuccess = FALSE;
	switch (hDC32->nGraphicsMode)
	{
	case GM_COMPATIBLE:
		SetLastErrorEx(1, 0);
		break;
	case GM_ADVANCED:
		hDC32->eM11 = lpXform->eM11; hDC32->eM12 = lpXform->eM12;
		hDC32->eM21 = lpXform->eM21; hDC32->eM22 = lpXform->eM22;
		hDC32->eDx  = lpXform->eDx;  hDC32->eDy  = lpXform->eDy;
		bSuccess = TRUE;
		break;
	default:
		SetLastErrorEx(1, 0);
		break;
	}
	RELEASEDCINFO(hDC32);

	return (bSuccess);

}

/* GDI Coordinate Transform ************************************************ */

int WINAPI
SetMapMode(HDC hDC, int mode)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_API,"SetMapMode: hDC %x mode %x\n",hDC,mode));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.mode = mode;

    nRet = (int)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_MAPMODE,&argptr);

    RELEASEDCINFO(hDC32);

    return nRet;
}

int WINAPI
GetMapMode(HDC hDC)
{
    HDC32 hDC32;
    int nRet;

    APISTR((LF_API,"GetMapMode: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    nRet = (int)DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_MAPMODE, 0);

    RELEASEDCINFO(hDC32);

    return nRet;
}

DWORD WINAPI
SetWindowOrg(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"SetWindowOrg: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_WINDOWORG,&argptr);

    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
SetWindowOrgEx(HDC hDC, int x, int y, LPPOINT lppt)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"SetWindowOrgEx: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_WINDOWORG,&argptr);

    if (lppt)
	*lppt = argptr.lsde.point;


    RELEASEDCINFO(hDC32);

    return TRUE;
}

DWORD WINAPI
GetWindowOrg(HDC hDC)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"GetWindowOrg: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_WINDOWORG,&argptr);

    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
GetWindowOrgEx(HDC hDC,LPPOINT lppt)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet = FALSE;

    APISTR((LF_API,"GetWindowOrgEx: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (lppt) {
	argptr.lsde_validate.dwInvalidMask = 0;
	DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_WINDOWORG,&argptr);
	*lppt = argptr.lsde.point;
	bRet = TRUE;
    }

    RELEASEDCINFO(hDC32);

    return bRet;
}

DWORD WINAPI
SetWindowExt(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"SetWindowExt: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.size.cx = x;
    argptr.lsde.size.cy = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_WINDOWEXT,&argptr);

    dwRet = MAKELONG(argptr.lsde.size.cx,argptr.lsde.size.cy);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
SetWindowExtEx(HDC hDC, int x, int y, LPSIZE lpsize)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"SetWindowExtEx: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.size.cx = x;
    argptr.lsde.size.cy = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_WINDOWEXT,&argptr);

    if (lpsize)
	*lpsize = argptr.lsde.size;

    RELEASEDCINFO(hDC32);

    return TRUE;
}

DWORD WINAPI
GetWindowExt(HDC hDC)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"GetWindowExt: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_WINDOWEXT,&argptr);

    dwRet = MAKELONG(argptr.lsde.size.cx,argptr.lsde.size.cy);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
GetWindowExtEx(HDC hDC,LPSIZE lpsize)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet = FALSE;

    APISTR((LF_API,"GetWindowExtEx: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (lpsize) {
	argptr.lsde_validate.dwInvalidMask = 0;
	DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_WINDOWEXT,&argptr);
	*lpsize = argptr.lsde.size;
	bRet = TRUE;
    }

    RELEASEDCINFO(hDC32);

    return bRet;
}

DWORD WINAPI
OffsetWindowOrg(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"OffsetWindowOrg: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_OFFSETWINDOWORG,&argptr);

    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
OffsetWindowOrgEx(HDC hDC, int x, int y, LPPOINT lpPoint)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"OffsetWindowOrgEx: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_OFFSETWINDOWORG,&argptr);

    if (lpPoint)
	*lpPoint = argptr.lsde.point;

    RELEASEDCINFO(hDC32);

    return TRUE;
}

DWORD WINAPI
ScaleWindowExt(HDC hDC, int Xnum, int Xdenom, int Ynum, int Ydenom)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"ScaleWindowExt: hDC %x x %d/%d y %d/%d\n",hDC,
		Xnum,Xdenom,Ynum,Ydenom));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.rect.left = Xnum;
    argptr.lsde.rect.top = Ynum;
    argptr.lsde.rect.right = Xdenom;
    argptr.lsde.rect.bottom = Ydenom;

    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_SCALEWINDOWEXT,&argptr);

    dwRet = MAKELONG(argptr.lsde.rect.left,argptr.lsde.rect.top);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
ScaleWindowExtEx(HDC hDC, int Xnum, int Xdenom, int Ynum, int Ydenom,
			LPSIZE lpSize)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"ScaleWindowExtEx: hDC %x x %d/%d y %d/%d\n",hDC,
		Xnum,Xdenom,Ynum,Ydenom));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.rect.left = Xnum;
    argptr.lsde.rect.top = Ynum;
    argptr.lsde.rect.right = Xdenom;
    argptr.lsde.rect.bottom = Ydenom;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_SCALEWINDOWEXT,&argptr);

    if (lpSize) {
	lpSize->cx = argptr.lsde.rect.left;
	lpSize->cy = argptr.lsde.rect.top;
    }

    RELEASEDCINFO(hDC32);

    return TRUE;
}

DWORD WINAPI
SetViewportOrg(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"SetViewportOrg: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_VIEWPORTORG,&argptr);

    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
SetViewportOrgEx(HDC hDC, int x, int y, LPPOINT lppt)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"SetViewportOrgEx: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_VIEWPORTORG,&argptr);

    if (lppt)
	*lppt = argptr.lsde.point;

    RELEASEDCINFO(hDC32);

    return TRUE;
}

DWORD WINAPI
GetViewportOrg(HDC hDC)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"GetViewportOrg: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_VIEWPORTORG,&argptr);

    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
GetViewportOrgEx(HDC hDC, LPPOINT lppt)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet = FALSE;

    APISTR((LF_API,"GetViewportOrgEx: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (lppt) {
	argptr.lsde_validate.dwInvalidMask = 0;
	DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_VIEWPORTORG,&argptr);
	*lppt = argptr.lsde.point;
	bRet = TRUE;
    }

    RELEASEDCINFO(hDC32);

    return bRet;
}

DWORD WINAPI
SetViewportExt(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"SetViewportExt: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.size.cx = x;
    argptr.lsde.size.cy = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_VIEWPORTEXT,&argptr);

    dwRet = MAKELONG(argptr.lsde.size.cx,argptr.lsde.size.cy);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
SetViewportExtEx(HDC hDC, int x, int y, LPSIZE lpsz)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"SetViewportExtEx: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.size.cx = x;
    argptr.lsde.size.cy = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_VIEWPORTEXT,&argptr);

    if (lpsz)
	*lpsz = argptr.lsde.size;

    RELEASEDCINFO(hDC32);

    return TRUE;
}

DWORD WINAPI
GetViewportExt(HDC hDC)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"GetViewportExt: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_VIEWPORTEXT,&argptr);

    dwRet = MAKELONG(argptr.lsde.size.cx,argptr.lsde.size.cy);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
GetViewportExtEx(HDC hDC, LPSIZE lpsz)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet = FALSE;

    APISTR((LF_API,"GetViewportExtEx: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,FALSE);

    if (lpsz) {
	argptr.lsde_validate.dwInvalidMask = 0;
	DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_VIEWPORTEXT,&argptr);
	*lpsz = argptr.lsde.size;
	bRet = TRUE;
    }

    RELEASEDCINFO(hDC32);

    return TRUE;
}

DWORD WINAPI
OffsetViewportOrg(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"OffsetViewportOrg: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_OFFSETVIEWPORTORG,&argptr);

    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
OffsetViewportOrgEx(HDC hDC, int x, int y, LPPOINT lpPoint)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"OffsetViewportOrgEx: hDC %x x,y %d,%d\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_OFFSETVIEWPORTORG,&argptr);

    if (lpPoint)
	*lpPoint = argptr.lsde.point;

    RELEASEDCINFO(hDC32);

    return TRUE;
}

DWORD WINAPI
ScaleViewportExt(HDC hDC, int Xnum, int Xdenom, int Ynum, int Ydenom)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"ScaleViewportExt: hDC %x x %d/%d y %d/%d\n",hDC,
		Xnum,Xdenom,Ynum,Ydenom));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.rect.left = Xnum;
    argptr.lsde.rect.top = Ynum;
    argptr.lsde.rect.right = Xdenom;
    argptr.lsde.rect.bottom = Ydenom;

    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_SCALEVIEWPORTEXT,&argptr);

    dwRet = MAKELONG(argptr.lsde.rect.left,argptr.lsde.rect.top);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
ScaleViewportExtEx(HDC hDC, int Xnum, int Xdenom, int Ynum, int Ydenom,
			LPSIZE lpSize)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"ScaleViewportExtEx: hDC %x x %d/%d y %d/%d\n",hDC,
		Xnum,Xdenom,Ynum,Ydenom));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.rect.left = Xnum;
    argptr.lsde.rect.top = Ynum;
    argptr.lsde.rect.right = Xdenom;
    argptr.lsde.rect.bottom = Ydenom;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_SCALEVIEWPORTEXT,&argptr);

    if (lpSize) {
	lpSize->cx = argptr.lsde.rect.left;
	lpSize->cy = argptr.lsde.rect.top;
    }

    RELEASEDCINFO(hDC32);

    return TRUE;
}

DWORD WINAPI
SetBrushOrg(HDC hDC, int nXOrg, int nYOrg)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"SetBrushOrg: hDC %x %x %x\n",hDC,nXOrg,nYOrg));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.point.x = nXOrg;
    argptr.lsde.point.y = nYOrg;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_BRUSHORG,&argptr);

    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

DWORD WINAPI
GetBrushOrg(HDC hDC)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"GetBrushOrg: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_BRUSHORG,&argptr);

    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
GetBrushOrgEx(HDC hDC, LPPOINT lpPoint)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"GetBrushOrg: hDC %x\n",hDC));

    if (!lpPoint)
	return FALSE;

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_BRUSHORG,&argptr);

    *lpPoint = argptr.lsde.point;

    RELEASEDCINFO(hDC32);

    return TRUE;
}

int WINAPI
SetPolyFillMode(HDC hDC,int nMode)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_API,"SetPolyFillMode: hDC %x mode %d\n",hDC,nMode));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.mode = nMode;
    nRet = (int)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_POLYFILLMODE,&argptr);

    RELEASEDCINFO(hDC32);

    return nRet;
}

int WINAPI
GetPolyFillMode(HDC hDC)
{
    HDC32 hDC32;
    int nRet;

    APISTR((LF_API,"GetPolyFillMode: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    nRet = (int)DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_POLYFILLMODE,0);

    RELEASEDCINFO(hDC32);

    return nRet;
}

DWORD WINAPI
SetTextColor(HDC hDC,COLORREF crColor)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"SetTextColor: hDC %x color %x\n",hDC,crColor));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.colorref = crColor;
    
    dwRet =(DWORD)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_TEXTCOLOR,&argptr);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

DWORD WINAPI
GetTextColor(HDC hDC)
{
    HDC32 hDC32;
    DWORD dwRet;

    APISTR((LF_API,"GetTextColor: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    dwRet = (DWORD)DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_TEXTCOLOR,0);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

DWORD WINAPI
SetBkColor(HDC hDC,COLORREF crColor)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"SetBkColor: hdc %x clr %x\n",hDC,crColor));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.colorref = crColor;

    dwRet = (DWORD)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_BKCOLOR,&argptr);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

DWORD WINAPI
GetBkColor(HDC hDC)
{
    HDC32 hDC32;
    DWORD dwRet;

    APISTR((LF_API,"GetBkColor: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    dwRet = (DWORD)DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_BKCOLOR,0);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

int WINAPI
SetBkMode(HDC hDC,int nBkMode)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_API,"SetBkMode: hDC %x %x\n",hDC,nBkMode));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.mode = nBkMode;
    
    nRet =(int)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_BKMODE,&argptr);

    RELEASEDCINFO(hDC32);

    return nRet;
}

int WINAPI
GetBkMode(HDC hDC)
{
    HDC32 hDC32;
    int nRet;

    APISTR((LF_API,"GetBkMode: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    nRet = (int)DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_BKMODE,0);

    RELEASEDCINFO(hDC32);

    return nRet;
}

UINT WINAPI
SetTextAlign(HDC hDC, UINT uiFlags)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    UINT uiRet;

    APISTR((LF_API,"SetTextAlign: hDC %x, flags %x\n",hDC,uiFlags));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.mode = uiFlags;
    
    uiRet =(UINT)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_TEXTALIGN,&argptr);

    RELEASEDCINFO(hDC32);

    return uiRet;
}

UINT WINAPI
GetTextAlign(HDC hDC)
{
    HDC32 hDC32;
    UINT uiRet;

    APISTR((LF_API,"GetTextAlign: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    uiRet = (UINT)DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_TEXTALIGN,0);

    RELEASEDCINFO(hDC32);

    return uiRet;
}

int WINAPI
SetTextCharacterExtra(HDC hDC, int nExtraSpace)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_API,"SetTextCharacterExtra: hDC %x extra %x\n",
			hDC,nExtraSpace));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.size.cx = nExtraSpace;

    nRet = (int)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_TEXTCHAREXTRA,&argptr);

    RELEASEDCINFO(hDC32);

    return nRet;
}

int WINAPI 
GetTextCharacterExtra(HDC hDC)
{
    HDC32 hDC32;
    int nRet;

    APISTR((LF_API,"GetTextCharacterExtra: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    nRet = (int)DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_TEXTCHAREXTRA,0);

    RELEASEDCINFO(hDC32);

    return nRet;
}

BOOL WINAPI
SetTextJustification(HDC hDC, int nBreakExtra, int nBreakCount)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_API,
	"SetTextJustification: hDC %x, breakextra=%d breakcount=%d\n",
	hDC,nBreakExtra,nBreakCount));

    ASSERT_HDC(hDC32,hDC,FALSE);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.size.cx = nBreakExtra;
    argptr.lsde.size.cy = nBreakCount;
    
    bRet = (BOOL)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_TEXTJUST,&argptr);

    RELEASEDCINFO(hDC32);

    return bRet;
}

DWORD
SetMapperFlags(HDC hDC, DWORD fdwMatch)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"SetMapperFlags: hDC %x flag %x\n",hDC,fdwMatch));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.mode = fdwMatch;
    
    dwRet = (DWORD)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_MAPPERFLAGS,&argptr);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

int WINAPI
SetStretchBltMode(HDC hDC,int nStretchMode)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_API,"SetStretchBltMode: hDC %x %x\n",hDC,nStretchMode));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.mode = nStretchMode;

    nRet = (int)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_STRETCHMODE,&argptr);

    RELEASEDCINFO(hDC32);

    return nRet;
}

int WINAPI
GetStretchBltMode(HDC hDC)
{
    HDC32 hDC32;
    int nRet;

    APISTR((LF_API,"GetStretchBltMode: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    nRet = (int)DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_STRETCHMODE,0);

    RELEASEDCINFO(hDC32);

    return nRet;
}

int WINAPI
SetROP2(HDC hDC,int nDrawMode)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_API,"SetROP2: hDC %x %x\n",hDC,nDrawMode));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.mode = nDrawMode;
    
    nRet = (int)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_ROP2,&argptr);

    RELEASEDCINFO(hDC32);

    return nRet;
}

int WINAPI
GetROP2(HDC hDC)
{
    HDC32 hDC32;
    int nRet;

    APISTR((LF_API,"GetROP2: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    nRet = (int)DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_ROP2,0);

    RELEASEDCINFO(hDC32);

    return nRet;
}

int WINAPI
SetRelAbs(HDC hDC,int nRelAbs)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    int nRet;

    APISTR((LF_API,"SetRelAbs: hDC %x %x\n",hDC,nRelAbs));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.mode = nRelAbs;
    
    nRet = (int)DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_RELABS,&argptr);

    RELEASEDCINFO(hDC32);

    return nRet;
}

int WINAPI
GetRelAbs(HDC hDC)
{
    HDC32 hDC32;
    int nRet;

    APISTR((LF_API,"GetRelAbs: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    nRet = (int)DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_RELABS,0);

    RELEASEDCINFO(hDC32);

    return nRet;
}

DWORD WINAPI
SetDCOrg(HDC hDC, int x, int y)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"SetDCOrg: hDC %x, x %x, y %x\n",hDC,x,y));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.point.x = x;
    argptr.lsde.point.y = y;
    DC_OUTPUT(LSD_SETDA,hDC32,LSD_DA_DCORG,&argptr);

    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

DWORD WINAPI
GetDCOrg(HDC hDC)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    DWORD dwRet;

    APISTR((LF_API,"GetDCOrg: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_DCORG,&argptr);

    dwRet = MAKELONG(argptr.lsde.point.x,argptr.lsde.point.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

BOOL WINAPI
GetDCOrgEx(HDC hDC, LPPOINT lpPt)
{
    HDC32 hDC32;
    LSDS_PARAMS argptr;
    BOOL bRet;

    APISTR((LF_API,"GetDCOrg: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    bRet = DC_OUTPUT(LSD_GETDA,hDC32,LSD_DA_DCORG,&argptr);

    lpPt->x = argptr.lsde.point.x;
    lpPt->y = argptr.lsde.point.y;

    RELEASEDCINFO(hDC32);

    return bRet;
}

/* this is called only for display DC and, therefore, is not in LSD layer */
DWORD
OffsetDCOrg(HDC hDC, int xOffset, int yOffset)
{
    HDC32 hDC32;
    POINT pt,ptClipOrg;
    DWORD dwRet;

    ASSERT_HDC(hDC32,hDC,0);

    pt.x = hDC32->DOx;
    pt.y = hDC32->DOy;

    hDC32->DOx += xOffset;
    hDC32->DOy += yOffset;

    ptClipOrg.x = hDC32->DOx;
    ptClipOrg.y = hDC32->DOy;
    DRVCALL_DC(PDCH_SETCLIPORG,&ptClipOrg,0,hDC32->lpDrvData);

    dwRet = MAKELONG(pt.x,pt.y);

    RELEASEDCINFO(hDC32);

    return dwRet;
}

DWORD
lsd_setda(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return GdiDCAttributes(hDC32,dwParam,LSDM_SET,lpStruct);
}

DWORD
lsd_getda(WORD msg, HDC32 hDC32, DWORD dwParam, LPLSDS_PARAMS lpStruct)
{
    return GdiDCAttributes(hDC32,dwParam,LSDM_GET,lpStruct);
}

static DWORD
GdiDCAttributes(HDC32 hDC32, DWORD dwFunc, DWORD dwSet, LPLSDS_PARAMS lpStruct)
{
    DWORD dwRet;

    switch(dwFunc) {
	case LSD_DA_MAPMODE:
	    dwRet = (DWORD)hDC32->MapMode;

	    if (dwSet == LSDM_SET)
		GdiSetMapMode(hDC32,lpStruct->lsde.mode);
	    return dwRet;

	case LSD_DA_WINDOWORG: {
	    POINT pt;
	    pt.x = hDC32->WOx;
	    pt.y = hDC32->WOy;
	    if (dwSet == LSDM_SET) {
		hDC32->WOx = lpStruct->lsde.point.x;
		hDC32->WOy = lpStruct->lsde.point.y;
	    }
	    lpStruct->lsde.point = pt;
	    break; }

	case LSD_DA_WINDOWEXT: {
	    SIZE sz;

	    sz.cx = hDC32->WEx;
	    sz.cy = hDC32->WEy;
	    /* extents cannot be changed unless the current mapmode is */
	    /* MM_ISOTROPIC or MM_ANISOTROPIC */
	    if (dwSet == LSDM_SET &&
		(hDC32->MapMode == MM_ISOTROPIC ||
		 hDC32->MapMode == MM_ANISOTROPIC)) {
		hDC32->WEx = lpStruct->lsde.size.cx;
		hDC32->WEy = lpStruct->lsde.size.cy;
		if (hDC32->MapMode == MM_ISOTROPIC)
		    GdiAdjustViewportExt(hDC32);

		if (hDC32->WEx != sz.cx)	/* horz extent changed */
		    hDC32->dwInvalid |= IM_PENWIDTHMASK;

		if (hDC32->WEy != sz.cy)	/* vert extent changed */
		    hDC32->dwInvalid |= IM_FONTSCALEMASK;
	    }
	    lpStruct->lsde.size = sz;
	    return 1L; }

	case LSD_DA_OFFSETWINDOWORG: {
	    POINT pt;
	    pt.x = hDC32->WOx;
	    pt.y = hDC32->WOy;
	    if (dwSet == LSDM_SET) {
		hDC32->WOx += lpStruct->lsde.point.x;
		hDC32->WOy += lpStruct->lsde.point.y;
	    }
	    lpStruct->lsde.point = pt;
	    break; }

	case LSD_DA_SCALEWINDOWEXT: {
	    SIZE sz;
	    sz.cx = hDC32->WEx;
	    sz.cy = hDC32->WEy;
	    if (dwSet == LSDM_SET) {
		hDC32->WEx *= lpStruct->lsde.rect.left;
		hDC32->WEx /= lpStruct->lsde.rect.right;
		hDC32->WEy *= lpStruct->lsde.rect.top;
		hDC32->WEy /= lpStruct->lsde.rect.bottom;
	    }
	    if (hDC32->WEx != sz.cx)	/* horz extent changed */
		hDC32->dwInvalid |= IM_PENWIDTHMASK;

	    if (hDC32->WEy != sz.cy)	/* vert extent changed */
		hDC32->dwInvalid |= IM_FONTSCALEMASK;

	    lpStruct->lsde.rect.left = sz.cx;
	    lpStruct->lsde.rect.top = sz.cy;
	    break; }

	case LSD_DA_VIEWPORTORG: {
	    POINT pt;

	    pt.x = hDC32->VOx;
	    pt.y = hDC32->VOy;
	    if (dwSet == LSDM_SET) {
		hDC32->VOx = lpStruct->lsde.point.x;
		hDC32->VOy = lpStruct->lsde.point.y;
	    }
	    lpStruct->lsde.point = pt;
	    break; }

	case LSD_DA_VIEWPORTEXT: {
	    SIZE sz;
	    sz.cx = hDC32->VEx;
	    sz.cy = hDC32->VEy;
	    /* extents cannot be changed unless the current mapmode is */
	    /* MM_ISOTROPIC or MM_ANISOTROPIC */
	    if (dwSet == LSDM_SET &&
			(hDC32->MapMode == MM_ISOTROPIC ||
		 	hDC32->MapMode == MM_ANISOTROPIC)) {
		hDC32->VEx = lpStruct->lsde.size.cx;
		hDC32->VEy = lpStruct->lsde.size.cy;
		if (hDC32->MapMode == MM_ISOTROPIC)
		    GdiAdjustViewportExt(hDC32);

		if (hDC32->VEx != sz.cx)	/* horz extent changed */
		    hDC32->dwInvalid |= IM_PENWIDTHMASK;

		if (hDC32->VEy != sz.cy)	/* vert extent changed */
		    hDC32->dwInvalid |= IM_FONTSCALEMASK;

	    }
	    lpStruct->lsde.size = sz;
	    return 1L; }

	case LSD_DA_OFFSETVIEWPORTORG: {
	    POINT pt;
	    pt.x = hDC32->VOx;
	    pt.y = hDC32->VOy;
	    if (dwSet == LSDM_SET) {
		hDC32->VOx += lpStruct->lsde.point.x;
		hDC32->VOy += lpStruct->lsde.point.y;
	    }
	    lpStruct->lsde.point = pt;
	    break; }

	case LSD_DA_SCALEVIEWPORTEXT: {
	    SIZE sz;
	    sz.cx = hDC32->VEx;
	    sz.cy = hDC32->VEy;
	    if (dwSet == LSDM_SET) {
		hDC32->VEx *= lpStruct->lsde.rect.left;
		hDC32->VEx /= lpStruct->lsde.rect.right;
		hDC32->VEy *= lpStruct->lsde.rect.top;
		hDC32->VEy /= lpStruct->lsde.rect.bottom;
	    }
	    if (hDC32->VEx != sz.cx)	/* horz extent changed */
		hDC32->dwInvalid |= IM_PENWIDTHMASK;

	    if (hDC32->VEy != sz.cy)	/* vert extent changed */
		hDC32->dwInvalid |= IM_FONTSCALEMASK;

	    lpStruct->lsde.rect.left = sz.cx;
	    lpStruct->lsde.rect.top = sz.cy;
	    break; }

	case LSD_DA_BRUSHORG: {
	    POINT pt = hDC32->ptBrushOrg;
	    if (dwSet == LSDM_SET)
		hDC32->ptBrushOrg = lpStruct->lsde.point;
	    lpStruct->lsde.point = pt;
	    return 1L; }

	case LSD_DA_POLYFILLMODE:
	    dwRet = (DWORD)hDC32->PolyFillMode;
	    if (dwSet == LSDM_SET) {
		hDC32->PolyFillMode = lpStruct->lsde.mode;
		hDC32->dwInvalid |= IM_POLYFILLMODEMASK;
	    }
	    return dwRet;

	case LSD_DA_TEXTCOLOR:
	    dwRet = (DWORD)hDC32->TextColor;
	    if (dwSet == LSDM_SET) {
		hDC32->TextColor = lpStruct->lsde.colorref;
		hDC32->dwInvalid |= IM_TEXTCOLORMASK;
	    }
	    return dwRet;

	case LSD_DA_BKCOLOR:
	    dwRet = (DWORD)hDC32->BackColor;
	    if (dwSet == LSDM_SET) {
		hDC32->BackColor = lpStruct->lsde.colorref;
		hDC32->dwInvalid |= IM_BKCOLORMASK;
	    }
	    return dwRet;

	case LSD_DA_BKMODE:
	    dwRet = (DWORD)hDC32->BackMode;
	    if (dwSet == LSDM_SET) {
		hDC32->BackMode = lpStruct->lsde.mode;
		hDC32->dwInvalid |= IM_BKMODEMASK;
		if (hDC32->BackMode == OPAQUE)
		    hDC32->dwInvalid |= IM_BKCOLORMASK;
	    }
	    return dwRet;

	case LSD_DA_TEXTALIGN:
	    dwRet = (DWORD)hDC32->TextAlign;
	    if (dwSet == LSDM_SET) 
		hDC32->TextAlign = lpStruct->lsde.mode;
	    return dwRet;

	case LSD_DA_TEXTCHAREXTRA: {
	    SIZE sz;
	    dwRet = (DWORD)hDC32->nCharExtra;
	    if (dwSet == LSDM_SET) {
		sz.cx = lpStruct->lsde.size.cx;
		sz.cy = 0;
		LEtoDE(hDC32,&sz);
		hDC32->nCharExtra = sz.cx;
	    }
	    sz.cx = dwRet;
	    sz.cy = 0;
	    DEtoLE(hDC32,&sz);
	    return (DWORD)sz.cx; }

	case LSD_DA_TEXTJUST: {
	    SIZE sz;
	    sz.cx = lpStruct->lsde.size.cx;
	    sz.cy = 0;
	    LEtoDE(hDC32,&sz);
	    if (dwSet == LSDM_SET) {
                /*
                **   MiD: free previous copy of lpExtraSpace (if any)
                **        If nBreakCount > 0 then allocate the new array
                **        for that number of entries and call Bresenham
                **        magic to fill in the array.
                */
                if (hDC32->lpExtraSpace != NULL)
                   {
                   WinFree((LPVOID)hDC32->lpExtraSpace);
                   hDC32->lpExtraSpace = NULL;
                   }
		hDC32->nBreakExtra = sz.cx;
		hDC32->nBreakCount = lpStruct->lsde.size.cy;
                if (hDC32->nBreakCount > 0)
                   {
                   hDC32->lpExtraSpace = (LPINT)WinMalloc(hDC32->nBreakCount * sizeof(int));
                   memset((LPVOID)hDC32->lpExtraSpace, '\0', hDC32->nBreakCount * sizeof(int));
                   TextJustifyBresenham(hDC32->nBreakExtra, hDC32->nBreakCount, hDC32->lpExtraSpace);
                   }
		if (hDC32->nBreakExtra == 0)
		    hDC32->nErrorTerm = 0;
	    }
	    return 1L; }

	case LSD_DA_MAPPERFLAGS:
	    dwRet = hDC32->dwMapperFlags;
	    if (dwSet == LSDM_SET) 
		hDC32->dwMapperFlags = lpStruct->lsde.mode;
	    return dwRet;

	case LSD_DA_STRETCHMODE:
	    dwRet = (DWORD)hDC32->StretchBltMode;
	    if (dwSet == LSDM_SET) {
		hDC32->StretchBltMode = lpStruct->lsde.mode;
		hDC32->dwInvalid |= IM_STRETCHMODEMASK;
	    }
	    return dwRet;

	case LSD_DA_ROP2:
	    dwRet = hDC32->ROP2;
	    if (dwSet == LSDM_SET) {
		hDC32->ROP2 = lpStruct->lsde.mode;
		hDC32->dwInvalid |= IM_ROP2MASK;
	    }
	    return dwRet;

	case LSD_DA_RELABS:
	    dwRet = (DWORD)hDC32->RelAbs;
	    if (dwSet == LSDM_SET) {
		hDC32->RelAbs = lpStruct->lsde.mode;
		/* no invalidation is done here, since RelAbs flag */
		/* is being passed to LineTo/Polyline/Polygon driver calls */
	    }
	    return dwRet;

	case LSD_DA_DCORG: {
	    POINT pt;
	    pt.x = hDC32->DOx;
	    pt.y = hDC32->DOy;
	    if (dwSet == LSDM_SET) {
		hDC32->DOx = lpStruct->lsde.point.x;
		hDC32->DOy = lpStruct->lsde.point.y;
		DRVCALL_DC(PDCH_SETCLIPORG,
			&lpStruct->lsde.point,0,hDC32->lpDrvData);
	    }
	    lpStruct->lsde.point = pt;
	    break; }

	default:
	    return 0L;
    }

    if (DC_CAN_SWITCH_OFF_MAPPING(hDC32)) {
	DC_MAPMODE_OFF(hDC32);
    }
    else {
	DC_MAPMODE_ON(hDC32);
    }

    return 1L;
}

static void
GdiAdjustViewportExt(HDC32 hDC32)
{
    int		iUnAdjustedVEx, iUnAdjustedVEy;
    int		absVEx, absVEy, absWEx, absWEy;
    int		iSignVEx, iSignVEy;
    float	flWinExtRatio, flWinExtRatio1;

    absVEx = abs(hDC32->VEx);
    absVEy = abs(hDC32->VEy);
    absWEx = abs(hDC32->WEx);
    absWEy = abs(hDC32->WEy);
    iUnAdjustedVEx = absVEx;
    iUnAdjustedVEy = absVEy;
    iSignVEx = hDC32->VEx / absVEx;
    iSignVEy = hDC32->VEy / absVEy;

    flWinExtRatio  = (float)max(absWEx, absWEy) / (float)min(absWEy, absWEx);
    flWinExtRatio1 = (float)min(absWEx, absWEy) / (float)max(absWEy, absWEx);

    /* Adjust the longest viewport extent */
    if ( absVEx > absVEy )
	/* --- Adjust x-extent */
	if ( absWEx > absWEy )
		hDC32->VEx = absVEy * flWinExtRatio * iSignVEx;
	else
		hDC32->VEx = absVEy * flWinExtRatio1 * iSignVEx;
    else
	/* --- Adjust y-extent */
	if ( absWEy > absWEx )
		hDC32->VEy = absVEx * flWinExtRatio * iSignVEy;
	else
		hDC32->VEy = absVEx * flWinExtRatio1 * iSignVEy;

    /* Make sure that the adjusted extent is not
       bigger than the previous one (unadjusted).
    */
    if ( abs(hDC32->VEx) > iUnAdjustedVEx ) {
	hDC32->VEx = iUnAdjustedVEx * iSignVEx;
	hDC32->VEy = iUnAdjustedVEx / flWinExtRatio * iSignVEy;
    }
    else
	if ( abs(hDC32->VEy) > iUnAdjustedVEy ) {
		hDC32->VEy = iUnAdjustedVEy * iSignVEy;
		hDC32->VEx = iUnAdjustedVEy / flWinExtRatio * iSignVEx;
	}
}

static void
GdiSetMapMode(HDC32 hDC32, int nMapMode)
{
    HDC hDC = GETHDC16(hDC32);
    int iHorzSize,iVertSize,iHorzRes,iVertRes,iLogPixelsX,iLogPixelsY;
    SIZE szVE, szWE;

    iHorzSize   = GetDeviceCaps(hDC, HORZSIZE);
    iVertSize   = GetDeviceCaps(hDC, VERTSIZE);
    iHorzRes    = GetDeviceCaps(hDC, HORZRES);
    iVertRes    = GetDeviceCaps(hDC, VERTRES);
    iLogPixelsX = GetDeviceCaps(hDC, LOGPIXELSX);
    iLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);

    szVE.cx = hDC32->VEx;
    szVE.cy = hDC32->VEy;
    szWE.cx = hDC32->WEx;
    szWE.cy = hDC32->WEy;

    switch(nMapMode) {
	case MM_ANISOTROPIC:/* unconstrained mode   */
	    if (hDC32->MapMode)
		break;		/* Inherits extents of previously set
				   mapping mode (and needs coordinates
				   conversion too). If DC is created
				   from scratch - pass through MM_TEXT. */
	case MM_TEXT:		/* constrained modes... */
	    hDC32->WEx = 1;
	    hDC32->WEy = 1;
	    hDC32->VEx = 1;
	    hDC32->VEy = 1;
	    break;

	case MM_ISOTROPIC:	/* partially constrained */
	case MM_LOMETRIC:
	    hDC32->WEx = iHorzSize * 10;
	    hDC32->WEy = iVertSize * 10;
	    hDC32->VEx = iHorzRes;
	    hDC32->VEy = -iVertRes;
	    break;

	case MM_HIMETRIC:
	    hDC32->WEx = iHorzSize * 100;
	    hDC32->WEy = iVertSize * 100;
	    hDC32->VEx = iHorzRes;
	    hDC32->VEy = -iVertRes;
	    break;

	case MM_LOENGLISH:
	    hDC32->WEx = iHorzSize * 100.0 / 25.4;
	    hDC32->WEy = iVertSize * 100.0 / 25.4;
	    hDC32->VEx = iHorzRes;
	    hDC32->VEy = -iVertRes;
	    break;

	case MM_HIENGLISH:
	    hDC32->WEx = iHorzSize * 1000.0 / 25.4;
	    hDC32->WEy = iVertSize * 1000.0 / 25.4;
	    hDC32->VEx = iHorzRes;
	    hDC32->VEy = -iVertRes;
	    break;

	case MM_TWIPS:
	    hDC32->WEx = iHorzSize * 1440.0 / 25.4;
	    hDC32->WEy = iVertSize * 1440.0 / 25.4;
	    hDC32->VEx = iHorzRes;
	    hDC32->VEy = -iVertRes;
	    break;
    }

    hDC32->MapMode = nMapMode;

    if (nMapMode != MM_TEXT) {
	DC_MAPMODE_ON(hDC32);
    }
    else
	if (hDC32->FOx == 0 && hDC32->FOy == 0 &&
	    hDC32->DOx == 0 && hDC32->DOy == 0)
	    DC_MAPMODE_OFF(hDC32);

    if (hDC32->VEx != szVE.cx || hDC32->WEx != szWE.cx)
	hDC32->dwInvalid |= IM_PENWIDTHMASK;

    if (hDC32->VEy != szVE.cy || hDC32->WEy != szWE.cy)
	hDC32->dwInvalid |= IM_FONTSCALEMASK;
}


/*
**   Distributes nBreakExtra amount filling in nBreakCount items
**   in lpExtraSpace array.
*/
static void
TextJustifyBresenham(int nBreakExtra, int nBreakCount, LPINT lpExtraSpace)
{
   int   i, rest, remaining = nBreakExtra;
   int   step = nBreakExtra / nBreakCount;

   if (nBreakExtra >= nBreakCount)
      for (i = 0;   i < nBreakCount;  i++)
          {
          lpExtraSpace[i] = step;
          /* printf("[%d] %d ", i, lpExtraSpace[i]); */
          remaining -= step;
          }

   if (remaining > 0)
      for (i = 0, rest = remaining;  i < nBreakCount;  i += nBreakCount / remaining)
          {
          lpExtraSpace[i]++;
          /* printf("[%d] %d ", i, lpExtraSpace[i]); */
          if (--rest == 0)
             break;
          }
}

/* (WIN32) GDI DC Attributes *********************************************** */

int	WINAPI
GetArcDirection(HDC hDC)
{
	HDC32 hDC32;
	int nArcDirection;

	ASSERT_HDC(hDC32, hDC, 0);
	nArcDirection = hDC32->nArcDirection;
	RELEASEDCINFO(hDC32);

	return (nArcDirection);

}

int WINAPI
SetArcDirection(HDC hDC, int nArcDirection)
{
	HDC32 hDC32;
	int nOldArcDirection;

	ASSERT_HDC(hDC32, hDC, 0);
	nOldArcDirection = hDC32->nArcDirection;
	switch (nArcDirection)
	{
	case AD_COUNTERCLOCKWISE:
		hDC32->nArcDirection = nArcDirection;
		break;
	case AD_CLOCKWISE:
		hDC32->nArcDirection = nArcDirection;
		break;
	default:
		SetLastErrorEx(1, 0);
		nOldArcDirection = 0;
		break;
	}
	RELEASEDCINFO(hDC32);

	return (nOldArcDirection);

}

BOOL	WINAPI
GetMiterLimit(HDC hDC, PFLOAT peMiterLimit)
{
	HDC32 hDC32;

	if (peMiterLimit == NULL)
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}

	ASSERT_HDC(hDC32, hDC, FALSE);
	*peMiterLimit = hDC32->eMiterLimit;
	RELEASEDCINFO(hDC32);

	return (TRUE);

}

BOOL	WINAPI
SetMiterLimit(HDC hDC, FLOAT eNewMiterLimit, PFLOAT peOldMiterLimit)
{
	HDC32 hDC32;

	ASSERT_HDC(hDC32, hDC, FALSE);
	if (peOldMiterLimit != NULL)
		*peOldMiterLimit = hDC32->eMiterLimit;
	hDC32->eMiterLimit = eNewMiterLimit;
	RELEASEDCINFO(hDC32);

	return (TRUE);

}

BOOL	WINAPI
GetColorAdjustment(HDC hDC, LPCOLORADJUSTMENT lpColorAdjustment)
{
	HDC32 hDC32;

	APISTR((LF_API, "GetColorAdjustment: hDC %x lpColorAdjustment %p\n",
		hDC, lpColorAdjustment));

	if (!lpColorAdjustment)
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}

	ASSERT_HDC(hDC32, hDC, FALSE);

	*lpColorAdjustment = hDC32->caHalftone;

	RELEASEDCINFO(hDC32);

	APISTR((LF_API,
		" ->caSize ............. %d\n"
		" ->caFlags ............ %d\n"
		" ->caIlluminantIndex .. %d\n"
		" ->caRedGamma ......... %d\n"
		" ->caGreenGamma ....... %d\n"
		" ->caBlueGamma ........ %d\n"
		" ->caReferenceBlack ... %d\n"
		" ->caReferenceWhite ... %d\n"
		" ->caContrast ......... %d\n"
		" ->caBrightness ....... %d\n"
		" ->caColorfulness ..... %d\n"
		" ->caRedGreenTint ..... %d\n",
		lpColorAdjustment->caSize,
		lpColorAdjustment->caFlags,
		lpColorAdjustment->caIlluminantIndex,
		lpColorAdjustment->caRedGamma,
		lpColorAdjustment->caGreenGamma,
		lpColorAdjustment->caBlueGamma,
		lpColorAdjustment->caReferenceBlack,
		lpColorAdjustment->caReferenceWhite,
		lpColorAdjustment->caContrast,
		lpColorAdjustment->caBrightness,
		lpColorAdjustment->caColorfulness,
		lpColorAdjustment->caRedGreenTint));

	return (TRUE);

}

#define ASSERT_COLORADJUSTMENT(val,min,max,err)\
	if (((val) < (min)) || ((val) > (max)))\
	{\
		ERRSTR((LF_ERROR, "ASSERT_COLORADJUSTMENT: failed\n"));\
		SetLastErrorEx(1, 0);\
		return (err);\
	}

BOOL	WINAPI
SetColorAdjustment(HDC hDC, CONST COLORADJUSTMENT *lpColorAdjustment)
{
	HDC32 hDC32;

	APISTR((LF_API, "SetColorAdjustment: hDC %x lpColorAdjustment %p\n",
		hDC, lpColorAdjustment));

	if (!lpColorAdjustment)
	{
		SetLastErrorEx(1, 0);
		return (FALSE);
	}

	APISTR((LF_API,
		" ->caSize ............. %d\n"
		" ->caFlags ............ %d\n"
		" ->caIlluminantIndex .. %d\n"
		" ->caRedGamma ......... %d\n"
		" ->caGreenGamma ....... %d\n"
		" ->caBlueGamma ........ %d\n"
		" ->caReferenceBlack ... %d\n"
		" ->caReferenceWhite ... %d\n"
		" ->caContrast ......... %d\n"
		" ->caBrightness ....... %d\n"
		" ->caColorfulness ..... %d\n"
		" ->caRedGreenTint ..... %d\n",
		lpColorAdjustment->caSize,
		lpColorAdjustment->caFlags,
		lpColorAdjustment->caIlluminantIndex,
		lpColorAdjustment->caRedGamma,
		lpColorAdjustment->caGreenGamma,
		lpColorAdjustment->caBlueGamma,
		lpColorAdjustment->caReferenceBlack,
		lpColorAdjustment->caReferenceWhite,
		lpColorAdjustment->caContrast,
		lpColorAdjustment->caBrightness,
		lpColorAdjustment->caColorfulness,
		lpColorAdjustment->caRedGreenTint));

	ASSERT_COLORADJUSTMENT((short) lpColorAdjustment->caSize,
		0, sizeof(COLORADJUSTMENT), FALSE);

	ASSERT_COLORADJUSTMENT((short)lpColorAdjustment->caFlags,
		0, CA_NEGATIVE | CA_LOG_FILTER, FALSE);

	ASSERT_COLORADJUSTMENT((short) lpColorAdjustment->caIlluminantIndex,
		0, ILLUMINANT_MAX_INDEX, FALSE);

	ASSERT_COLORADJUSTMENT((int) lpColorAdjustment->caRedGamma,
		2500, 65000, FALSE);

	ASSERT_COLORADJUSTMENT((int)lpColorAdjustment->caGreenGamma,
		2500, 65000, FALSE);

	ASSERT_COLORADJUSTMENT((int)lpColorAdjustment->caBlueGamma,
		2500, 65000, FALSE);

	ASSERT_COLORADJUSTMENT((short)lpColorAdjustment->caReferenceBlack,
		0, 4000, FALSE);

	ASSERT_COLORADJUSTMENT((short)lpColorAdjustment->caReferenceWhite,
		6000, 10000, FALSE);

	ASSERT_COLORADJUSTMENT(lpColorAdjustment->caContrast,
		-100, 100, FALSE);

	ASSERT_COLORADJUSTMENT(lpColorAdjustment->caBrightness,
		-100, 100, FALSE);

	ASSERT_COLORADJUSTMENT(lpColorAdjustment->caColorfulness,
		-100, 100, FALSE);

	ASSERT_COLORADJUSTMENT(lpColorAdjustment->caRedGreenTint,
		-100, 100, FALSE);

	ASSERT_HDC(hDC32, hDC, FALSE);

	hDC32->caHalftone = *lpColorAdjustment;

	RELEASEDCINFO(hDC32);

	return (TRUE);

}
