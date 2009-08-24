/*
	@(#)Driver.h	1.39   Entry point for platform specific sub systems.
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
 
#ifndef Driver__h
#define Driver__h

#include "windows.h"

typedef DWORD (CALLBACK* TWINDRVPROC)(WORD, LPARAM, LPARAM, LPVOID);
typedef DWORD (CALLBACK* TWINLIBCALLBACKPROC)(DWORD, LPARAM, LPARAM, LPVOID);

#ifdef DRVTAB
typedef DWORD (CALLBACK* TWINDRVSUBPROC)(LPARAM, LPARAM, LPVOID);
#else
extern TWINLIBCALLBACKPROC DrvEntry;
#endif /*DRVTAB*/

#ifdef DRVTAB
extern TWINDRVSUBPROC **DriverBootstrap(void);
#else
extern TWINLIBCALLBACKPROC DriverBootstrap(void);
#endif

#ifndef DRVTAB
extern DWORD PrivateSystemHook(DWORD, LPARAM, LPARAM, LPVOID);
#endif

extern DWORD DriverWaitEvent(BOOL);

#define	TWINLIBCALLBACK_BUTTON		0x0001
#define	TWINLIBCALLBACK_KEYBOARD	0x0002
#define	TWINLIBCALLBACK_TIMEOUT		0x0003
#define	TWINLIBCALLBACK_POSTMESSAGE	0x0004
#define	TWINLIBCALLBACK_EXPOSE		0x0005
#define	TWINLIBCALLBACK_CONFIGURE	0x0006
#define	TWINLIBCALLBACK_CLIPBOARD	0x0007
#define	TWINLIBCALLBACK_FOCUS		0x0008
#define	TWINLIBCALLBACK_MEMORY		0x0009
#define		TLC_MEMALLOC		0x0000
#define		TLC_MEMREALLOC		0x0001
#define		TLC_MEMFREE		0x0002
#define	TWINLIBCALLBACK_CHECKOBJ	0x000A
#define TWINLIBCALLBACK_VSLOGSTR	0x000B
#define	TWINLIBCALLBACK_TIMERWINDOW	0x000C

typedef struct tagTWINTIMERWIN
{
	WNDPROC		winProc;    /*  The window procedure  */
	LPSTR		winName;    /*  The class name  */
	UINT		timerID;    /*  timer ID for SetTimer() */
	UINT		timeout;    /*  number of milleseconds for the timer */
} TWINTIMERWIN;
typedef TWINTIMERWIN	*LPTWINTIMERWIN;


/* Sub System Definitions */
#define DSUB_SYSTEM	    0x0000
#define DSUB_REGIONS        0x0001
#define DSUB_COMM           0x0002
#define DSUB_IMAGES         0x0003
#define DSUB_GRAPHICS	    0x0004
#define DSUB_DC		    0x0005
#define DSUB_TEXT	    0x0006
#define DSUB_WINDOWS	    0x0007
#define DSUB_IPC	    0x0008
#define DSUB_CURSORS	    0x0009
#define DSUB_KEYBOARD	    0x000a
#define DSUB_COLORS	    0x000b
#define DSUB_EVENTS	    0x000c
#define DSUB_WINSOCK        0x000d
#define DSUB_CONFIG         0x000e
#define DSUB_TIME           0x000f
#define DSUB_LICENSE        0x0010
#define DSUB_PRINTER        0x0011
#define DSUB_FILES	    0x0012
#define DSUB_THREADS        0x0013
#define DSUB_MEMORY         0x0014
#define DSUB_LAST	    DSUB_MEMORY

#ifdef DRVTAB
extern TWINDRVSUBPROC **DrvEntryTab;
#endif /*DRVTAB*/

/* this initializes driver subsystems */
#define	DSUB_INITDRIVER	    0x0098
#define	DSUB_INITSYSTEM	    0x0099

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_INITDRIVER(f,dw1,dw2,lpv) PrivateInitDriver( \
		MAKELONG(f,DSUB_INITDRIVER), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))

#define	DRVCALL_INITSYSTEM(f,dw1,dw2,lpv) PrivateInitDriver( \
		MAKELONG(f,DSUB_INITSYSTEM), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define DRVCALL_INITDRIVER(f,dw1,dw2,lpv) (DrvEntry)( \
                MAKELONG(f,DSUB_INITDRIVER), \
                (DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
 
#define	DRVCALL_INITSYSTEM(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_INITSYSTEM), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Driver Native Object Capabilities Mask */ 
#define DNO_MENUS	0x00000001

typedef struct tagTWINDRVCONFIG
{
    int 	nScreenWidth;
    int		nScreenHeight;
    DWORD	dwDoubleClickTime;
    TWINLIBCALLBACKPROC lpDrvCallback;
} TWINDRVCONFIG;
typedef TWINDRVCONFIG *LPTWINDRVCONFIG;

/* Mandatory Sub-System Interface Definitions */
#define DSUBSYSTEM_INIT     0x0000
#define DSUBSYSTEM_GETCAPS  0x0001
#define DSUBSYSTEM_EVENTS   0x0002
#define	DSUBSYSTEM_LAST     DSUBSYSTEM_EVENTS

typedef struct tagTWINDRVMALLOC
{
	char *lparam;
	int wparam;
	int flag;
	int handle;
	void *theptr;
} TWINDRVMALLOC;
typedef TWINDRVMALLOC *LPTWINDRVMALLOC;

/* System hook definitions */
#define	PSSH_ALLOCMEM			DSUBSYSTEM_LAST+1
#define	PSSH_REALLOCMEM			DSUBSYSTEM_LAST+2
#define	PSSH_FREEMEM			DSUBSYSTEM_LAST+3
#define	PSSH_CREATETHREAD		DSUBSYSTEM_LAST+4
#define	PSSH_FREETHREAD			DSUBSYSTEM_LAST+5
#define	PSSH_YIELDTOTHREAD		DSUBSYSTEM_LAST+6
#define PSSH_CANDOTHREADS		DSUBSYSTEM_LAST+7
#define PSSH_GETMAINTHREAD		DSUBSYSTEM_LAST+8
#define PSSH_MAKEEXEDATA		DSUBSYSTEM_LAST+9
#define PSSH_GETFREEDISKSPACE		DSUBSYSTEM_LAST+10
#define PSSH_CREATEPSDKEY		DSUBSYSTEM_LAST+11
#define PSSH_DELETEPSDKEY		DSUBSYSTEM_LAST+12
#define PSSH_SETPSD			DSUBSYSTEM_LAST+13
#define PSSH_GETPSD			DSUBSYSTEM_LAST+14
#define PSSH_LOADLIBRARY		DSUBSYSTEM_LAST+15
#define PSSH_SLEEP			DSUBSYSTEM_LAST+16
#define PSSH_SIGWAIT			DSUBSYSTEM_LAST+17
#define PSSH_CREATEMUTEX		DSUBSYSTEM_LAST+18
#define PSSH_DELETEMUTEX		DSUBSYSTEM_LAST+19
#define PSSH_LOCKMUTEX			DSUBSYSTEM_LAST+20
#define PSSH_UNLOCKMUTEX		DSUBSYSTEM_LAST+21

typedef struct tagCREATETHREADDATA
{
    DWORD stacksize;
    int   priority;
    void *thread_main;
    void *thread_main_arg;
    DWORD flags;
} CREATETHREADDATA;

#define TWIN_THREADFLAGS_WIN16	0x00000001L /* Thread is a WIN16 task */
/* macro to call the driver */

#ifdef DRVTAB
#define	DRVCALL_SYSTEM(f,dw1,dw2,lpv) (DrvEntryTab[0][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define DRVCALL_SYSTEM(f,dw1,dw2,lpv) PrivateSystemHook( \
		(DWORD)(f), (DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Regions Sub-System Definitions */
#define	PRH_CREATEREGION		DSUBSYSTEM_LAST+1
#define	PRH_CREATEPOLYREGION		DSUBSYSTEM_LAST+2
#define	PRH_CREATERECTREGION		DSUBSYSTEM_LAST+3
#define	PRH_CREATEELLIPTICREGION	DSUBSYSTEM_LAST+4
#define	PRH_CREATEROUNDRECTREGION	DSUBSYSTEM_LAST+5
#define	PRH_DESTROYREGION		DSUBSYSTEM_LAST+6
#define	PRH_SETREGIONEMPTY		DSUBSYSTEM_LAST+7
#define	PRH_SETRECTREGION		DSUBSYSTEM_LAST+8
#define	PRH_PTINREGION			DSUBSYSTEM_LAST+9
#define	PRH_RECTINREGION		DSUBSYSTEM_LAST+10
#define	PRH_ISEMPTYREGION		DSUBSYSTEM_LAST+11
#define	PRH_EQUALREGION			DSUBSYSTEM_LAST+12
#define	PRH_OFFSETREGION		DSUBSYSTEM_LAST+13
#define	PRH_UNIONREGION			DSUBSYSTEM_LAST+14
#define	PRH_COPYREGION			DSUBSYSTEM_LAST+15
#define	PRH_INTERSECTREGION		DSUBSYSTEM_LAST+16
#define	PRH_DIFFREGION			DSUBSYSTEM_LAST+17
#define	PRH_XORREGION			DSUBSYSTEM_LAST+18
#define	PRH_UNIONRECTWITHREGION		DSUBSYSTEM_LAST+19
#define	PRH_REGIONBOX			DSUBSYSTEM_LAST+20
#define PRH_GETREGIONDATA		DSUBSYSTEM_LAST+21

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_REGIONS(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_REGIONS][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_REGIONS(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_REGIONS), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* capabilities masks */
#define	RSUB_CANDO_RECT			0x0001
#define	RSUB_CANDO_ELLIPTIC		0x0002
#define	RSUB_CANDO_ROUNDRECT		0x0004
#define	RSUB_CANDO_FLOODFILL		0x0008

/* Images Sub-System Definitions */
#define	PIH_CREATEIMAGE		DSUBSYSTEM_LAST+1
#define	PIH_DESTROYIMAGE	DSUBSYSTEM_LAST+2
#define	PIH_UPDATEPIXMAP	DSUBSYSTEM_LAST+3
#define	PIH_GETDEVCAPS		DSUBSYSTEM_LAST+4
#define	PIH_GETIMAGEDATA	DSUBSYSTEM_LAST+5
#define	PIH_FETCHBITS		DSUBSYSTEM_LAST+6
#define	PIH_CREATEBRUSH		DSUBSYSTEM_LAST+7
#define	PIH_DESTROYBRUSH	DSUBSYSTEM_LAST+8
#define	PIH_CREATECURSOR	DSUBSYSTEM_LAST+9
#define	PIH_DESTROYCURSOR	DSUBSYSTEM_LAST+10

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_IMAGES(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_IMAGES][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_IMAGES(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_IMAGES), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

typedef struct tagCREATEIMAGEDATA {
    int nWidth,nHeight;
    int nBitsPixel;
    LPSTR lpData;
    int nWidthBytes;
    int nLinePad;
} CREATEIMAGEDATA;

typedef struct tagCREATECURSOR {
    LPVOID lpXORData;
    LPVOID lpANDData;
    POINT  ptHot;
    COLORREF crFore;
    COLORREF crBack;
} CREATECURSOR;

#define DRV_BITORDER		1
#define DRV_BYTEORDER		2
#define DRV_QUERYDEPTH		3
#define DRV_QUERYBITSPIXEL	4
#define DRV_QUERYREDMASK	5
#define DRV_QUERYGREENMASK	6
#define DRV_QUERYBLUEMASK	7

#define MSBFIRST	0
#define LSBFIRST	1

/* Graphics Sub-System Definitions */
#define	PGH_GETDEVICECAPS	DSUBSYSTEM_LAST+1
#define	PGH_VALIDATE		DSUBSYSTEM_LAST+2
#define	PGH_GETASPECTRATIO	DSUBSYSTEM_LAST+3
#define	PGH_SETBRUSHORG		DSUBSYSTEM_LAST+4
#define	PGH_SELECTIMAGE		DSUBSYSTEM_LAST+5
#define	PGH_GETPIXEL		DSUBSYSTEM_LAST+6
#define	PGH_SETPIXEL		DSUBSYSTEM_LAST+7
#define	PGH_MOVETO		DSUBSYSTEM_LAST+8
#define	PGH_LINETO		DSUBSYSTEM_LAST+9
#define	PGH_POLYLINE		DSUBSYSTEM_LAST+10
#define	PGH_RECTANGLE		DSUBSYSTEM_LAST+11
#define	PGH_ROUNDRECT		DSUBSYSTEM_LAST+12
#define	PGH_ELLIPSE		DSUBSYSTEM_LAST+13
#define	PGH_ARC			DSUBSYSTEM_LAST+14
#define	PGH_POLYGON		DSUBSYSTEM_LAST+15
#define	PGH_POLYPOLYGON		DSUBSYSTEM_LAST+16
#define	PGH_FLOODFILL		DSUBSYSTEM_LAST+17
#define	PGH_SCROLLDC		DSUBSYSTEM_LAST+18
#define	PGH_PUTIMAGE		DSUBSYSTEM_LAST+19
#define PGH_PATBLT		DSUBSYSTEM_LAST+20
#define PGH_BITBLT		DSUBSYSTEM_LAST+21
#define	PGH_STRETCHBLT		DSUBSYSTEM_LAST+22
#define PGH_MASKBLT		DSUBSYSTEM_LAST+23
#define PGH_PLGBLT		DSUBSYSTEM_LAST+24

#define PGH_SUCCESS	1L
#define PGH_ERROR	(DWORD)-1L

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_GRAPHICS(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_GRAPHICS][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_GRAPHICS(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_GRAPHICS), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

typedef struct tagPUTIMAGEDATA {
    LPVOID lpid;
    int xSrc,ySrc;
    int xDest,yDest;
    int cx,cy;
} PUTIMAGEDATA;

/* DC Sub-System Definitions */
#define	PDCH_CREATE	DSUBSYSTEM_LAST+1
#define	PDCH_DELETE	DSUBSYSTEM_LAST+2
#define	PDCH_SAVE	DSUBSYSTEM_LAST+3
#define	PDCH_RESTORE	DSUBSYSTEM_LAST+4
#define	PDCH_GETCLIP	DSUBSYSTEM_LAST+5
#define	PDCH_SETCLIP	DSUBSYSTEM_LAST+6
#define	PDCH_SETCLIPORG	DSUBSYSTEM_LAST+7
#define	PDCH_SETUPDATECLIP DSUBSYSTEM_LAST+8
#define	PDCH_SETVISRGN	DSUBSYSTEM_LAST+9

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_DC(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_DC][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_DC(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_DC), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Text Sub-System definitions */
#define	PTH_REALIZE	DSUBSYSTEM_LAST+1
#define	PTH_RESCALE	DSUBSYSTEM_LAST+2
#define	PTH_GETMETRICS	DSUBSYSTEM_LAST+3
#define	PTH_GETCHARWIDTH DSUBSYSTEM_LAST+4
#define	PTH_EXTTEXTOUT	DSUBSYSTEM_LAST+5
#define	PTH_GETTEXTFACE	DSUBSYSTEM_LAST+6
#define	PTH_ENUMFONTS	DSUBSYSTEM_LAST+7
#define PTH_GETENUMCNT  DSUBSYSTEM_LAST+8 
#define PTH_DELETEFONT  DSUBSYSTEM_LAST+9 
#define PTH_GETGLYPHOUTLINE DSUBSYSTEM_LAST+10

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_TEXT(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_TEXT][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_TEXT(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_TEXT), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Windows Sub-System definitions */
#define	PWSH_CREATEWINDOW	DSUBSYSTEM_LAST+0x0001
#define	PWSH_DESTROYWINDOW	DSUBSYSTEM_LAST+0x0002
#define	PWSH_MOVEFRAME		DSUBSYSTEM_LAST+0x0003
#define	PWSH_MOVERESIZEFRAME	DSUBSYSTEM_LAST+0x0004
#define	PWSH_RESIZEFRAME	DSUBSYSTEM_LAST+0x0005
#define	PWSH_SHOWWINDOW		DSUBSYSTEM_LAST+0x0006
#define	PWSH_SETCAPTURE		DSUBSYSTEM_LAST+0x0007
#define	PWSH_RELEASECAPTURE	DSUBSYSTEM_LAST+0x0008
#define	PWSH_STACKINGORDER	DSUBSYSTEM_LAST+0x0009
#define	PWSH_SETTEXT		DSUBSYSTEM_LAST+0x000a
#define	PWSH_FLUSHEXPOSES	DSUBSYSTEM_LAST+0x000b
#define	PWSH_ICONIFY		DSUBSYSTEM_LAST+0x000c
#define	PWSH_RAISEWINDOW	DSUBSYSTEM_LAST+0x000d
#define PWSH_FIXUPSTACKINGORDER DSUBSYSTEM_LAST+0x000e

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_WINDOWS(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_WINDOWS][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_WINDOWS(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_WINDOWS), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* capabilities masks */
#define	WSUB_CANDO_RESTACK	0x0001
#define	WSUB_CANDO_TOPMOST	0x0002

/* Comm port Sub-System definitions */
#define PCH_OPENPORT		DSUBSYSTEM_LAST+0x0001
#define PCH_CLOSEPORT		DSUBSYSTEM_LAST+0x0002
#define PCH_ESCAPEFUNC		DSUBSYSTEM_LAST+0x0003
#define PCH_FLUSHCOMM		DSUBSYSTEM_LAST+0x0004
#define PCH_SETSTATE		DSUBSYSTEM_LAST+0x0005
#define PCH_GETSTATE		DSUBSYSTEM_LAST+0x0006

/* IPC Sub-System definitions */
#define	IPCH_INITATOMTABLE	DSUBSYSTEM_LAST+0x0001
#define	IPCH_GLOBALATOMDATA	DSUBSYSTEM_LAST+0x0002
        /* MiD - the following are clipboard driver hooks */
#define IPCH_CLOSECLIPBRD       DSUBSYSTEM_LAST+0x0003
#define IPCH_EMPTYCLIPBRD       DSUBSYSTEM_LAST+0x0004
#define IPCH_GETCLIPBRDDATA     DSUBSYSTEM_LAST+0x0005
#define IPCH_OPENCLIPBRD        DSUBSYSTEM_LAST+0x0006
#define IPCH_SETCLIPBRDDATA     DSUBSYSTEM_LAST+0x0007
#define IPCH_LOADFORMATS        DSUBSYSTEM_LAST+0x0008
#define IPCH_ENUMFORMATS        DSUBSYSTEM_LAST+0x0009
#define IPCH_ISFORMATAVAIL      DSUBSYSTEM_LAST+0x000A
#define IPCH_REGISTERFORMAT     DSUBSYSTEM_LAST+0x000B

#define	GAD_READONLY	0x0001	/* read data from server     */
#define	GAD_READWRITE	0x0002	/* read data from server     */
#define	GAD_UPDATE	0x0003	/* write data back to server */

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_IPC(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_IPC][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_IPC(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_IPC), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Clipboard Data exchange structure we poke between driver and library */
typedef struct tagGETSETDATA
{
   UINT    uiFormat;
   DWORD   dwSize;
   LPVOID  lpMem;
} GETSETDATA, *LPGETSETDATA;

/* Cursor Sub-System definitions */
#define	PCH_SETCURSOR		DSUBSYSTEM_LAST+0x0001
#define	PCH_GETCURSORPOS	DSUBSYSTEM_LAST+0x0002
#define	PCH_SETCURSORPOS	DSUBSYSTEM_LAST+0x0003
#define	PCH_SETCLIPCURSOR	DSUBSYSTEM_LAST+0x0004
#define	PCH_GETCLIPCURSOR	DSUBSYSTEM_LAST+0x0005
#define	PCH_HIDECURSOR		DSUBSYSTEM_LAST+0x0006
#define	PCH_SHOWCURSOR		DSUBSYSTEM_LAST+0x0007

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_CURSORS(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_CURSORS][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_CURSORS(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_CURSORS), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Keyboard Sub-System definitions */
#define	PKH_GETTYPE		DSUBSYSTEM_LAST+0x0001
#define	PKH_CODEPAGE		DSUBSYSTEM_LAST+0x0002
#define	PKH_KBSTATE		DSUBSYSTEM_LAST+0x0003
#define	PKH_KEYSTATE		DSUBSYSTEM_LAST+0x0004
#define	PKH_BTNSTATE		DSUBSYSTEM_LAST+0x0005
#define	PKH_KEYTEXT		DSUBSYSTEM_LAST+0x0006
#define	PKH_MAPKEY		DSUBSYSTEM_LAST+0x0007
#define	PKH_GETCHAR		DSUBSYSTEM_LAST+0x0008

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_KEYBOARD(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_KEYBOARD][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_KEYBOARD(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_KEYBOARD), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Color Sub-System definitions */
#define PLH_DEFAULTPALETTE	DSUBSYSTEM_LAST+0x0001
#define PLH_SYSTEMPALETTEUSE	DSUBSYSTEM_LAST+0x0002
#define PLH_GETSYSTEMPALETTE	DSUBSYSTEM_LAST+0x0003
#define PLH_SETSYSTEMPALETTE	DSUBSYSTEM_LAST+0x0004
#define PLH_ANIMATEPALETTE	DSUBSYSTEM_LAST+0x0005
#define PLH_UPDATECOLORS	DSUBSYSTEM_LAST+0x0006
#define	PLH_MAKEPIXEL		DSUBSYSTEM_LAST+0x0007
#define	PLH_GETNEARCOLOR	DSUBSYSTEM_LAST+0x0008
#define	PLH_GETCOLORREF		DSUBSYSTEM_LAST+0x0009
#define	PLH_FILLRGBTABLE	DSUBSYSTEM_LAST+0x000a
#define	PLH_MAKEPALENTRY	DSUBSYSTEM_LAST+0x000b
#define PLH_GETPIXELINFO	DSUBSYSTEM_LAST+0x000c

typedef struct tagTWINPIXELINFO
{
    int bytes_per_line;		/* # of bytes per line including padding */
    int bits_per_pixel;		/* # of bits per pixel including padding */
    int bit_order;		/* 0 LSB first, 1 MSB first              */
    int byte_order;		/* 0 LSB first, 1 MSB first              */
    unsigned long pixel_mask;	/* mask indicating valid pixel bits      */
    unsigned long red_mask;	/* mask indicating the red bits          */
    unsigned long green_mask;	/* mask indicating the green bits        */
    unsigned long blue_mask;	/* mask indicating the blue bits         */
} TWINPIXELINFO;

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_COLORS(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_COLORS][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_COLORS(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_COLORS), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/


/* Event Sub-System definitions */
#define	PEH_MOUSEBUTTON		DSUBSYSTEM_LAST+0x0001
#define	PEH_DBLCLKTIME		DSUBSYSTEM_LAST+0x0002
#define	PEH_BEEP		DSUBSYSTEM_LAST+0x0003
#define	PEH_SYSTIME		DSUBSYSTEM_LAST+0x0004
#define PEH_UNBLOCKSELECTWAIT	DSUBSYSTEM_LAST+0x0005

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_EVENTS(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_EVENTS][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_EVENTS(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_EVENTS), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/******     WINSOCK DEFINITIONS     *************/
/* RD */
/* WinSock Sub-system definitions */
#define PWS_ACCEPT			DSUBSYSTEM_LAST+0x0001
#define PWS_BIND			DSUBSYSTEM_LAST+0x0002
#define PWS_CLOSESOCKET			DSUBSYSTEM_LAST+0x0003
#define PWS_CONNECT			DSUBSYSTEM_LAST+0x0004
#define PWS_GETPEERNAME			DSUBSYSTEM_LAST+0x0005
#define PWS_GETSOCKNAME			DSUBSYSTEM_LAST+0x0006
#define PWS_GETSOCKOPT			DSUBSYSTEM_LAST+0x0007
#define PWS_HTONL			DSUBSYSTEM_LAST+0x0008
#define PWS_HTONS			DSUBSYSTEM_LAST+0x0009
#define PWS_INET_ADDR			DSUBSYSTEM_LAST+0x000a
#define PWS_INET_NTOA			DSUBSYSTEM_LAST+0x000b
#define PWS_IOCTLSOCKET			DSUBSYSTEM_LAST+0x000c
#define PWS_LISTEN			DSUBSYSTEM_LAST+0x000d
#define PWS_NTOHL			DSUBSYSTEM_LAST+0x000e
#define PWS_NTOHS			DSUBSYSTEM_LAST+0x000f
#define PWS_RECV			DSUBSYSTEM_LAST+0x0010
#define PWS_RECVFROM			DSUBSYSTEM_LAST+0x0011
#define PWS_SELECT			DSUBSYSTEM_LAST+0x0012
#define PWS_SEND			DSUBSYSTEM_LAST+0x0013
#define PWS_SENDTO			DSUBSYSTEM_LAST+0x0014
#define PWS_SETSOCKOPT			DSUBSYSTEM_LAST+0x0015
#define PWS_SHUTDOWN			DSUBSYSTEM_LAST+0x0016
#define PWS_SOCKET			DSUBSYSTEM_LAST+0x0017
#define PWS_GETHOSTBYADDR		DSUBSYSTEM_LAST+0x0018
#define PWS_GETHOSTBYNAME		DSUBSYSTEM_LAST+0x0019
#define PWS_GETPROTOBYNAME		DSUBSYSTEM_LAST+0x001a
#define PWS_GETPROTOBYNUMBER		DSUBSYSTEM_LAST+0x001b
#define PWS_GETSERVBYNAME		DSUBSYSTEM_LAST+0x001c
#define PWS_GETSERVBYPORT		DSUBSYSTEM_LAST+0x001d
#define PWS_GETHOSTNAME			DSUBSYSTEM_LAST+0x001e
#define PWS_WSAASYNCSELECT		DSUBSYSTEM_LAST+0x001f
#define PWS_WSAASYNCGETHOSTBYADDR 	DSUBSYSTEM_LAST+0x0020
#define PWS_WSAASYNCGETHOSTBYNAME 	DSUBSYSTEM_LAST+0x0021
#define PWS_WSAASYNCGETPROTOBYNUMBER 	DSUBSYSTEM_LAST+0x0022
#define PWS_WSAASYNCGETPROTOBYNAME 	DSUBSYSTEM_LAST+0x0023
#define PWS_WSAASYNCGETSERVBYPORT 	DSUBSYSTEM_LAST+0x0024
#define PWS_WSAASYNCGETSERVBYNAME 	DSUBSYSTEM_LAST+0x0025
#define PWS_WSACANCELASYNCREQUEST 	DSUBSYSTEM_LAST+0x0026
#define PWS_WSASETBLOCKINGHOOK	 	DSUBSYSTEM_LAST+0x0027
#define PWS_WSAUNHOOKBLOCKINGHOOK	DSUBSYSTEM_LAST+0x0028
#define PWS_WSAGETLASTERROR		DSUBSYSTEM_LAST+0x0029
#define PWS_WSASETLASTERROR		DSUBSYSTEM_LAST+0x002a
#define PWS_WSACANCELBLOCKINGHOOK	DSUBSYSTEM_LAST+0x002b
#define PWS_WSAISBLOCKING		DSUBSYSTEM_LAST+0x002c
#define PWS_WSASTARTUP			DSUBSYSTEM_LAST+0x002d
#define PWS_WSACLEANUP			DSUBSYSTEM_LAST+0x002e
#define PWS___WSAFDISSET		DSUBSYSTEM_LAST+0x002f
/*	CMA:	I added this one	*/
#define	PWS_GETSOCKSTATE		DSUBSYSTEM_LAST+0x0030

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_WINSOCK(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_WINSOCK][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_WINSOCK(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_WINSOCK), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Capabilities Mask */ 
/* None For WinSock at present */

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_TIME(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_TIME][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_TIME(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_TIME), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/


/* Event Sub-System definitions */
#define	PTMH_FT2ST		DSUBSYSTEM_LAST+0x0001
#define	PTMH_ST2FT		DSUBSYSTEM_LAST+0x0002
#define	PTMH_FT2LFT		DSUBSYSTEM_LAST+0x0003
#define	PTMH_LFT2FT		DSUBSYSTEM_LAST+0x0004
#define	PTMH_GETSYS		DSUBSYSTEM_LAST+0x0005
#define	PTMH_SETSYS		DSUBSYSTEM_LAST+0x0006
#define	PTMH_GETLOC		DSUBSYSTEM_LAST+0x0007
#define	PTMH_SETLOC		DSUBSYSTEM_LAST+0x0008
#define	PTMH_GETTZI		DSUBSYSTEM_LAST+0x0009
#define	PTMH_SETTZI		DSUBSYSTEM_LAST+0x000a
#define	PTMH_DOS2FT		DSUBSYSTEM_LAST+0x000b
#define	PTMH_FT2DOS		DSUBSYSTEM_LAST+0x000c
#define	PTMH_SYS2TZILOC		DSUBSYSTEM_LAST+0x000d


/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_LICENSE(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_LICENSE][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_LICENSE(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f,DSUB_LICENSE), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Event Sub-System definitions */
#define	PLH_SETDEFLICKEY	DSUBSYSTEM_LAST+0x0001
#define	PLH_GETLICID		DSUBSYSTEM_LAST+0x0002
#define	PLH_GETLICDATE		DSUBSYSTEM_LAST+0x0003
#define	PLH_GETLICKEY		DSUBSYSTEM_LAST+0x0004
#define	PLH_GETDEFLICKEY	DSUBSYSTEM_LAST+0x0005


/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_CONFIG(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_CONFIG][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define	DRVCALL_CONFIG(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f, DSUB_CONFIG), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Configuration Sub-System definitions */
#define	PCFGH_GETFILENAME		DSUBSYSTEM_LAST+0x0001
#define	PCFGH_GETDEFAULTS		DSUBSYSTEM_LAST+0x0002

/* Capabilities Mask */ 
/* None For Configuration at present */

/* macro to call the driver */
#ifdef DRVTAB
#define	DRVCALL_PRINTER(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_PRINTER][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define DRVCALL_PRINTER(f,dw1,dw2,lpv) (DrvEntry)( \
                MAKELONG(f, DSUB_PRINTER), \
                (DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Printer Sub-System definitions */
#define	PPH_CREATEDC		DSUBSYSTEM_LAST+0x0001
#define	PPH_STARTDOC		DSUBSYSTEM_LAST+0x0002
#define	PPH_STARTPAGE		DSUBSYSTEM_LAST+0x0003
#define	PPH_ENDPAGE			DSUBSYSTEM_LAST+0x0004
#define	PPH_ENDDOC			DSUBSYSTEM_LAST+0x0005
#define PPH_SENDDATA		DSUBSYSTEM_LAST+0x0006
#define	PPH_SETUPDLG		DSUBSYSTEM_LAST+0x0007
#define	PPH_GETDEVMODESIZE	DSUBSYSTEM_LAST+0x0008
#define	PPH_GETDEFAULT		DSUBSYSTEM_LAST+0x0009
#define	PPH_FILLPRINTDLG	DSUBSYSTEM_LAST+0x000A


/* macro to call the file sub-system driver */
#ifdef DRVTAB
#define	DRVCALL_FILES(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_FILES][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define DRVCALL_FILES(f,dw1,dw2,lpv) (DrvEntry)( \
                MAKELONG(f, DSUB_FILES), \
                (DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/


/* File Sub-System definitions */
#define	PFH_GETFILEINFO		DSUBSYSTEM_LAST+0x0001


/* macro to call the memory sub-system driver */
#ifdef DRVTAB
#define DRVCALL_MEMORY(f,dw1,dw2,lpv) (DrvEntryTab[DSUB_MEMORY][f])( \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#else
#define DRVCALL_MEMORY(f,dw1,dw2,lpv) (DrvEntry)( \
		MAKELONG(f, DSUB_MEMORY), \
		(DWORD)(dw1),(DWORD)(dw2),(LPVOID)(lpv))
#endif  /*DRVTAB*/

/* Memory Sub-System definitions */
#define PMH_ALLOCMEM            DSUBSYSTEM_LAST+0x0001
#define PMH_FREEMEM             DSUBSYSTEM_LAST+0x0002
#define PMH_REALLOCMEM          DSUBSYSTEM_LAST+0x0003
#define PMH_ALLOCMEMCHECK       DSUBSYSTEM_LAST+0x0004
#define PMH_FREEMEMCHECK        DSUBSYSTEM_LAST+0x0005
#define PMH_REALLOCMEMCHECK     DSUBSYSTEM_LAST+0x0006


#endif /* Driver__h */
