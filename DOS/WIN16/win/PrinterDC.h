/*
	@(#)PrinterDC.h	2.11
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
 

typedef struct tagPDEVICE {
        short pdType;           /* Device type. 0, if bitmap */
} PDEVICE;
typedef PDEVICE FAR *LPPDEVICE;

typedef struct tagPRINTERDRIVERDC {
	HMODULE		hModuleDriver;	/* hModule for the driver DLL */

	/* These are things that GDI keeps for the printer driver */
        LPPDEVICE	pDevice;	/* Device dependent structure      */
        LPVOID          lpPhPen;	/* Driver specific pen structure   */
        LPVOID          lpPhBrush;	/* Driver specific brush structure */
        LPVOID		lpPhFont;	/* Driver specific font structure  */
					/* pointer to FONTINFO structure   */

	/* These are things that are not kept in GDI portion of DC */
	POINT		cpt;		/* current MoveTo point */
	RECT		rcClipping;	/* current clipping rectangle */
	BOOL		fClippingSet;

	/* these are DDK-specific structures */
        GDIINFO         GdiInfo;
        DRAWMODE        DrawMode;
	TEXTXFORM	txf;

	/* These are internal flags */
	BOOL		bDisableResetDC;
	ABORTPROC	lpfnAbortProc;
#ifdef	LATER
        BOOL            IsIC;
#endif
        char            szDrvName[9];	/* Driver filename w/o extension   */
	char		szDevice[CCHDEVICENAME];
	char		szPort[10];
} PRINTERDRIVERDC;

typedef PRINTERDRIVERDC *LPPRINTERDRIVERDC;

#define	ORD_BITBLT	MAKEINTRESOURCE(1)
#define	ORD_COLORINFO	MAKEINTRESOURCE(2)
#define	ORD_CONTROL	MAKEINTRESOURCE(3)
#define	ORD_ENABLE	MAKEINTRESOURCE(5)
#define	ORD_ENUMDFONTS	MAKEINTRESOURCE(6)
#define	ORD_ENUMOBJ	MAKEINTRESOURCE(7)
#define	ORD_OUTPUT	MAKEINTRESOURCE(8)
#define	ORD_PIXEL	MAKEINTRESOURCE(9)
#define	ORD_REALIZEOBJ	MAKEINTRESOURCE(10)
#define	ORD_DEVICEMODE	MAKEINTRESOURCE(13)
#define	ORD_EXTTEXTOUT	MAKEINTRESOURCE(14)
#define	ORD_CHARWIDTHS	MAKEINTRESOURCE(15)
#define	ORD_SETDIBTODEV	MAKEINTRESOURCE(21)
#define	ORD_STRETCHBLT	MAKEINTRESOURCE(27)
#define	ORD_STRETCHDIB	MAKEINTRESOURCE(28)
#define	ORD_EXTDEVMODE	MAKEINTRESOURCE(90)
#define	ORD_DEVICECAPS	MAKEINTRESOURCE(91)
#define	ORD_ADVSETUPDLG	MAKEINTRESOURCE(93)

#define DRV_OBJ_PEN	OBJ_PEN			/* 1 */
#define DRV_OBJ_BRUSH	(DRV_OBJ_PEN + 1)	/* 2 */
#define DRV_OBJ_FONT	(DRV_OBJ_PEN + 2)	/* 3 */
#define DRV_OBJ_PBITMAP	(DRV_OBJ_PEN + 4)	/* 5 */

typedef int (CALLBACK *PBITBLTPROC)(LPPDEVICE,int,int,
			LPPDEVICE,int,int,int,int,DWORD,LPVOID,
			LPDRAWMODE);
typedef DWORD (CALLBACK *PCOLORINFOPROC)(LPPDEVICE,DWORD,LPVOID);
typedef DWORD (CALLBACK *PCONTROLPROC)(LPPDEVICE,int,LPVOID,LPVOID);
typedef DWORD (CALLBACK *PENABLEPROC)(LPVOID,int,LPSTR,LPSTR,LPDEVMODE);
typedef DWORD (CALLBACK *PFONTENUMPROC)(LPPDEVICE,LPSTR,FONTENUMPROC,LPARAM);
typedef DWORD (CALLBACK *PENUMOBJPROC)(LPPDEVICE,int,GOBJENUMPROC,LPARAM);
typedef int (CALLBACK *POUTPUTPROC)(LPPDEVICE,int,int,LPPOINT,
			LPVOID,LPVOID,LPDRAWMODE,LPVOID);
typedef DWORD (CALLBACK *PPIXELPROC)(LPPDEVICE,int,int,COLORREF,LPDRAWMODE);
typedef int (CALLBACK *PREALIZEPROC)(LPPDEVICE,int,LPVOID,LPVOID,LPTEXTXFORM);
typedef DWORD (CALLBACK *PEXTTEXTOUTPROC)(LPPDEVICE,int,int,LPRECT,
			LPSTR,int,LPVOID,LPDRAWMODE,LPTEXTXFORM,
			LPINT,LPRECT,int);
typedef DWORD (CALLBACK *PGETCHARWIDTHPROC)(LPPDEVICE,LPINT,BYTE,BYTE,
			LPVOID,LPDRAWMODE,LPTEXTXFORM);
typedef int (CALLBACK *PSETDIBTODEVPROC)(LPPDEVICE,int,int,int,int,
			LPRECT,LPDRAWMODE,LPVOID,
			LPBITMAPINFO,LPINT);
typedef int (CALLBACK *PSTRETCHBLTPROC)(LPPDEVICE,int,int,int,int,
			LPPDEVICE,int,int,int,int,DWORD,LPVOID,
			LPDRAWMODE,LPRECT);
typedef int (CALLBACK *PSTRETCHDIBPROC)(LPPDEVICE,int,
			int,int,int,int,
			int,int,int,int,
			LPVOID,LPBITMAPINFO,LPINT,
			DWORD,LPVOID,LPDRAWMODE,LPRECT);
typedef DWORD (CALLBACK *PEXTDEVMODEPROC)(HWND,HINSTANCE,
			LPDEVMODE,LPSTR,LPSTR,
			LPDEVMODE,LPSTR,int);
typedef DWORD (CALLBACK *PDEVCAPSPROC)(LPSTR,LPSTR,
			int,LPVOID,LPDEVMODE);
typedef DWORD (CALLBACK *PADVSETUPDLGPROC)(HWND,HINSTANCE,
			LPDEVMODE,LPDEVMODE);
