/*
	@(#)DrvEvents.c	1.20
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

 
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/times.h>

#include "windows.h"
#include "windowsx.h"
#include "Driver.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "Log.h"
#include "DrvDP.h"		/* PRIVATEDISPLAY */

static DWORD DrvSwapMouseButton(LPARAM, LPARAM, LPVOID);
static DWORD DrvDblClkTime(LPARAM, LPARAM, LPVOID);
static DWORD DrvBeep(LPARAM, LPARAM, LPVOID);
static DWORD DrvGetSystemTime(LPARAM, LPARAM, LPVOID);
static DWORD DrvEventsUnblockSelectWait(LPARAM, LPARAM, LPVOID);
static DWORD DrvEventsDoNothing(LPARAM,LPARAM,LPVOID);

DWORD DrvEventsTab(void);

extern BOOL DriverInSelectWait(void);

static TWINDRVSUBPROC DrvEventsEntryTab[] = {
	DrvEventsDoNothing,
	DrvEventsDoNothing,
	DrvEventsDoNothing,
	DrvSwapMouseButton,
	DrvDblClkTime,
	DrvBeep,
	DrvGetSystemTime,
	DrvEventsUnblockSelectWait};

DWORD
DrvEventsTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvEventsEntryTab[0] = DrvEventsDoNothing;
        DrvEventsEntryTab[1] = DrvEventsDoNothing;
        DrvEventsEntryTab[2] = DrvEventsDoNothing;
        DrvEventsEntryTab[3] = DrvSwapMouseButton;
        DrvEventsEntryTab[4] = DrvDblClkTime;
        DrvEventsEntryTab[5] = DrvBeep;
        DrvEventsEntryTab[6] = DrvGetSystemTime;
        DrvEventsEntryTab[7] = DrvEventsUnblockSelectWait;
#endif
	return (DWORD)DrvEventsEntryTab;
}

static DWORD
DrvEventsDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{	
	return 0L;
}
static DWORD
DrvSwapMouseButton(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    PRIVATEDISPLAY *dp = GETDP();
    static LPBYTE map = 0;
    static int nCount = 0;
    BYTE bMap;
    BOOL fState;

    if (map == 0) {
	nCount = XGetPointerMapping(dp->display,&bMap,1);
	if (nCount == 0)
	    return 0L;
	map = (LPBYTE)WinMalloc(nCount);
    }

    nCount = XGetPointerMapping(dp->display,map,nCount);

    if (nCount == 0)
	return 0L;

    if (nCount == 1) { /* one-button job */
	fState = FALSE;
    }
    else {
	if (nCount == 2) {
	    fState = (map[0] == 1);
	    bMap = map[0];
	    map[0] = map[1];
	    map[1] = bMap;
	}
	else { /* more than 2 buttons */
	    fState = (map[0] == 1 && map[2] == 3);
	    bMap = map[0];
	    map[0] = map[2];
	    map[2] = bMap;
	}
    }

    XSetPointerMapping(dp->display,map,nCount);

    return (DWORD)fState;
}

static DWORD
DrvDblClkTime(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    BOOL fSet = (BOOL)dwParam1;
    UINT uInterval = (UINT)dwParam2;
    PRIVATEDISPLAY *dp = GETDP();
    DWORD dwOldTime = dp->DoubleClickTime;

    if (fSet)
	dp->DoubleClickTime = (DWORD)uInterval;

    return dwOldTime;
}

static DWORD
DrvBeep(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    DWORD dwBeep = (DWORD)dwParam1;
    PRIVATEDISPLAY *dp = GETDP();

    XBell(dp->display,dwBeep==0?100:LOWORD(dwBeep));
    return 0L;
}

#ifdef TWIN_USESYSCONF

static DWORD
DrvGetSystemTime(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    DWORD dwTime;
    struct tms t_buffer;
    long clk_tick;

    /* if times fails, this will return 0   */
    /* we do not need to return -1 as no one */
    /* examines it anyway                   */
    t_buffer.tms_utime = 0;
    t_buffer.tms_stime = 0;
    times(&t_buffer);

    dwTime = t_buffer.tms_utime + t_buffer.tms_stime;
    dwTime *= 1000;
    clk_tick = sysconf(_SC_CLK_TCK);
    dwTime /= (DWORD)clk_tick;

    return dwTime;
}
#endif

#ifdef TWIN_USETIMES

#include <times.h>
static DWORD
DrvGetSystemTime(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    DWORD dwTime;
    tbuffer_t t_buffer;

    /* if times fails, this will return 0   */
    /* we do not need to return -1 as no one */
    /* examines it anyway                   */
    t_buffer.proc_user_time = 0;
    t_buffer.proc_system_time = 0;
    times(&t_buffer);

    dwTime = t_buffer.proc_user_time + t_buffer.proc_system_time;
    dwTime *= 10;  /* x1000 then /100 = 10 millisecond clock ticks */
    return dwTime;

}
#endif







static DWORD
DrvEventsUnblockSelectWait(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	if (DriverInSelectWait())
	{
		PRIVATEDISPLAY *dp = GETDP();
		/* send a fake X event to force driver out of select() wait */
		XChangeProperty(dp->display,
			XRootWindow(dp->display, dp->screen),
			dp->XFormatAtom, XA_STRING, 
			8, PropModeAppend, "", 0);
		XFlush(dp->display);
	}
	return 0L;
}

