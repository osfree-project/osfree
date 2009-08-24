/*
	@(#)DrvInit.c	1.33
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
 
#include <string.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "windows.h"

#include "Log.h"
#include "kerndef.h"
#include "ObjEngine.h"		/* DUMPHTAB */
#include "DrvDP.h"
#include "compat.h"
#include "WinConfig.h"
#include "Driver.h"
#include "DrvInit.h"

	/* enable new palette code */
#define TWIN_PALETTE

/* internal static data */
static PRIVATEDISPLAY DP[1], *dp;
static char *SysErr = "SystemError";
#define CONNECTSTR "No Display Server\nMust set DISPLAY environment variable\n"

/* internal routines */
static void ErrorHook(Display *);
static int TrapHook(Display *,XErrorEvent *);

/* this is exported to other driver subsystems, but NOT to the library */
PRIVATEDISPLAY * GETDP();
void TWIN_SystemError(UINT,DWORD,DWORD,DWORD);
TWINLIBCALLBACKPROC lpLibCallback;

static TWINDRVSUBPROC *DrvEntryTab1[DSUB_LAST];

int XrmUniqueQuark();

#define SYSERR_DISPLAY		1	/* error connecting to server */
#define SYSERR_SERVER		2	/* fatal error w/ server      */
#define SYSERR_XIO		3	/* fatal i/o error w/ server  */
#define SYSERR_MEMORY		4	/* fatal error memory allocation */
#define SYSERR_CONNECT		5	/* error connecting to server */
#define SYSERR_COLORS		6	/* error initializing colors */

/* wFunc has to be DSUBSYSTEM_INIT */
/* dwParam1 - compatibility mask */
/* dwParam2 - init/exit flag */
/* lpStruct - points to the structure to fill */
DWORD
PrivateInitDriver(WORD wFunc, LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    LPSTR lpszDisplayName;
    UINT uiCompatibility = (UINT)dwParam1;
    LPTWINDRVCONFIG lpDrvConfig = (LPTWINDRVCONFIG)lpStruct;
    char buffer[256];

    if (wFunc != DSUBSYSTEM_INIT)
	return 0L;

    if (dwParam2 != 0) { /* init */
	dp = &DP[0];

	lpLibCallback = lpDrvConfig->lpDrvCallback;

	lpszDisplayName = GetTwinString(WCP_DISPLAY,buffer,256);

	if ((dp->display=XOpenDisplay(lpszDisplayName)) == NULL) {
	    ERRSTR((LF_ERROR,"PrivateInitDriver: connect failure on %s\n", 
			XDisplayName(lpszDisplayName)));
	    TWIN_SystemError(0,SYSERR_DISPLAY,0,
			(DWORD) XDisplayName(lpszDisplayName));
	}
	dp->bConnected = TRUE;


	if (uiCompatibility & WD_SYNCRONOUS) {
		XSynchronize(dp->display,1);
        }

	{
	    char	buffer[256];
	    logstr(LF_SYSTEM,"X11 Configuration\n--------------------\n");
	    logstr(LF_SYSTEM,"Display: \t%s\n", DisplayString(dp->display));
	    logstr(LF_SYSTEM,"Vendor:  \t%s\n", ServerVendor(dp->display));
	    if (uiCompatibility & WD_SYNCRONOUS)
	    	logstr(LF_SYSTEM,"Mode:    \tSynchronous\n");
	    else
	    	logstr(LF_SYSTEM,"Mode:    \tAsynchronous\n");

	    gethostname(buffer,256);
	    logstr(LF_SYSTEM,"Hostname:\t%s\n",buffer);
	}

	XSetIOErrorHandler((XIOErrorHandler)ErrorHook);
	XSetErrorHandler((XErrorHandler)TrapHook);

	/* get the unique context value for the associated hwnd */
	dp->client_hwnd  = XUniqueContext();
	dp->window_style  = XUniqueContext();
	dp->window_ex_style  = XUniqueContext();
	
	dp->CodePage     = 437;

	
	DrvIPCInit(dwParam1, 1, 0);

	dp->screen = DefaultScreen(dp->display);

	DrvImagesInit(dwParam1, 1, 0);

	DrvCursorSetClipCursor(0,0,NULL);

	/* setup hardware equivalence */
	dp->HardwareEnabled = TRUE;
	dp->SwappedButtons  = FALSE;
	dp->LastClickTime   = 0;
	dp->DoubleClickTime = lpDrvConfig->dwDoubleClickTime;

	/* init system palette */
	dp->uSystemPaletteUse = SYSPAL_STATIC;
	dp->uSystemPaletteUseCount = ((LPUINT) 0);
	dp->hSystemPalette = (HPALETTE)NULL;
	dp->hOldSystemPalette = (HPALETTE)NULL;

	if(uiCompatibility & WD_COLORMAP) {
	    dp->SystemPalette = XCreateColormap(dp->display,
		RootWindow(dp->display,dp->screen),
		DefaultVisual(dp->display,dp->screen),
		AllocNone);
	}
	else
	    dp->SystemPalette = XDefaultColormap(dp->display,dp->screen);

	if (!DrvColorsInit(dwParam1,1,0))
	    TWIN_SystemError(0,SYSERR_COLORS,0,0);

	XSelectInput(dp->display,RootWindow(dp->display,dp->screen),
			PropertyChangeMask);

	DrvKeyboardInit(dwParam1,1,0);

	DrvTextInit(dwParam1,1,0);

	if(uiCompatibility & WD_VGAMODE) {
	  lpDrvConfig->nScreenWidth  = 640;
	  lpDrvConfig->nScreenHeight = 480;
	} else {
	  lpDrvConfig->nScreenWidth = DisplayWidth(dp->display,dp->screen);
	  lpDrvConfig->nScreenHeight = DisplayHeight(dp->display,dp->screen);
	}

	return 1L;
    }
    else { /* exit */

	if (dp && dp->display && dp->bConnected) {
	    XCloseDisplay(dp->display);
	    dp->bConnected = FALSE;
	}
    }
    return 1L;
}

void
TWIN_SystemError(UINT action,DWORD errorcode,DWORD wParam,DWORD lParam)
{
    int	exitcode = -1;
	
    if (dp && dp->display)
	XUngrabPointer(dp->display,0);

    switch (errorcode) {
	case SYSERR_CONNECT:
	    ERRSTR((LF_ERROR,CONNECTSTR));
	    exitcode = -errorcode;
	    break;

	case SYSERR_DISPLAY:
	    ERRSTR((LF_ERROR,"%s: Cannot connect to server %s.\n", SysErr,lParam));
	    exitcode = -errorcode;
	    break;

	case SYSERR_SERVER:
	    ERRSTR((LF_ERROR,"%s: Fatal X I/O Error %x.\n", SysErr,lParam));
	    exitcode = -errorcode;
	    break;

	case SYSERR_XIO:
	    ERRSTR((LF_ERROR,"%s: Fatal X I/O Error on %s.\n", SysErr,lParam));
	    exitcode = -errorcode;
	    break;

	case SYSERR_MEMORY:
	    ERRSTR((LF_ERROR,"%s: Memory Allocation Failure %x %x.\n",
		SysErr,wParam,lParam));
	    exitcode = -errorcode;
	    break;

	case SYSERR_COLORS:
	    ERRSTR((LF_ERROR,"Error initializing colors.\n"));
	    exitcode = -errorcode;
	    break;

	default:
	    ERRSTR((LF_ERROR,"%s: %x %x %x %x\n",
		SysErr,action,errorcode,wParam,lParam));
	    exitcode = -errorcode;
	    break;
    }
    ExitWindows(exitcode,1);
}

static void
ErrorHook(Display *display)
{
    ERRSTR((LF_ERROR,"Fatal I/O Error on %s\n",XDisplayName(NULL)));

    TWIN_SystemError(0,SYSERR_XIO,0,(DWORD)XDisplayName(NULL));
}

static int
TrapHook(Display *display, XErrorEvent *err)
{
    char  errbuf[512];
    char *errmsg;
    int   errflg;
    static int errcnt;
    static int fatalerr;

    /* save the error in a globally accessable place */
    dp->ErrorCode = err->error_code;

    switch (err->error_code) {
	case BadWindow:
	    errmsg = "BadWindow";
	    errflg = 1;
	    break;

	case BadPixmap:
	    errmsg = "BadPixmap";
	    errflg = 0;
	    break;

	case BadMatch:	
	    errmsg = "BadMatch";
	    errflg = 0;
	    break;

	case BadFont:	
	    errmsg = "BadFont";
	    errflg = 0;
	    break;

	case BadName:	
	    errmsg = "BadName";
	    errflg = 0;
	    break;

	case BadDrawable:
	    errmsg = "BadDrawable";
	    errflg = 0;
	    break;

	case Success:
	    errmsg = "Success";
	    errflg = 0;
	    break;

	default:
	    errmsg = "other";
            errflg = 1;
	    break;
    }

    sprintf(errbuf,"TrapHook: error = %d %s (%s) #=%d\n",
		err->error_code,errmsg,errflg?"FATAL":"warning",errcnt);

    ERRSTR((LF_ERROR,"%s",errbuf));

    if (errflg) {
	if (fatalerr)
	    TWIN_SystemError(0,SYSERR_SERVER,0,(DWORD) err->error_code);
	fatalerr++;
	FatalAppExit(0,"fatal X error");
    }

    return 0;
}

PRIVATEDISPLAY *
GETDP()
{
    return dp;
}

void
TWIN_SyncGraphics(BOOL bSync)
{
    PRIVATEDISPLAY *dp = GETDP();

    XSynchronize(dp->display,(bSync)?1:0);
}

DWORD
DrvConfigEntryTab(void)
{

	DrvEntryTab1[DSUB_SYSTEM] = (TWINDRVSUBPROC *)DrvSystemTab();
	DrvEntryTab1[DSUB_REGIONS] = (TWINDRVSUBPROC *)DrvRegionsTab();
	DrvEntryTab1[DSUB_IMAGES] = (TWINDRVSUBPROC *)DrvImagesTab();
	DrvEntryTab1[DSUB_GRAPHICS] = (TWINDRVSUBPROC *)DrvGraphicsTab();
	DrvEntryTab1[DSUB_DC] = (TWINDRVSUBPROC *)DrvDCTab();
	DrvEntryTab1[DSUB_TEXT] = (TWINDRVSUBPROC *)DrvTextTab();
	DrvEntryTab1[DSUB_WINDOWS] = (TWINDRVSUBPROC *)DrvWindowsTab();
	DrvEntryTab1[DSUB_IPC] = (TWINDRVSUBPROC *)DrvIPCTab();
	DrvEntryTab1[DSUB_CURSORS] = (TWINDRVSUBPROC *)DrvCursorTab();
	DrvEntryTab1[DSUB_KEYBOARD] = (TWINDRVSUBPROC *)DrvKeyboardTab();
	DrvEntryTab1[DSUB_COLORS] = (TWINDRVSUBPROC *)DrvColorsTab();
	DrvEntryTab1[DSUB_EVENTS] = (TWINDRVSUBPROC *)DrvEventsTab();
	DrvEntryTab1[DSUB_WINSOCK] = (TWINDRVSUBPROC *)DrvWinsockTab();
	DrvEntryTab1[DSUB_CONFIG] = (TWINDRVSUBPROC *)DrvConfigTab();
	DrvEntryTab1[DSUB_TIME] = (TWINDRVSUBPROC *)DrvTimeTab();
	DrvEntryTab1[DSUB_LICENSE] = 0;
	DrvEntryTab1[DSUB_PRINTER] = (TWINDRVSUBPROC *)DrvPrintingTab();
	DrvEntryTab1[DSUB_FILES] = (TWINDRVSUBPROC *)DrvFilesTab();
	DrvEntryTab1[DSUB_MEMORY] = (TWINDRVSUBPROC *)DrvMemoryTab();
	return (DWORD)DrvEntryTab1;
}
