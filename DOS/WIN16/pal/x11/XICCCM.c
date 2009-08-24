/*************************************************************************

	@(#)XICCCM.c	2.11
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

**************************************************************************/
 
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "windows.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "Log.h"
#include "DrvDP.h"
#include "Driver.h"
#include "DrvHook.h"

extern TWINLIBCALLBACKPROC lpLibCallback;
extern DWORD InternalClipboard(PRIVATEDISPLAY *, Window , Atom , Time , int );

/* 
 * dump the contents of a property in hex
 */
void
GetICCCMData(PRIVATEDISPLAY *dp,Atom atom,int state)
{
	unsigned char      *propertydata = 0;
	Atom		   actual_type; 
	int		   actual_format;
	unsigned long	   nitems;
	unsigned long	   remainder;

	XGetWindowProperty(dp->display,
			RootWindow(dp->display,dp->screen),
			atom,0,8192,
			FALSE,XA_STRING,&actual_type,
			&actual_format,&nitems,&remainder,
			(unsigned char **) &propertydata);	

	if(propertydata)
		XFree((char *)propertydata);
}

/* 
 * 	Handler called from DriverEvents.
 *	dispatch to handler for each type of atom
 *	(can be extended to user atoms... )
 */
int
InternalICCCM(int event,PRIVATEDISPLAY *dp,XEvent *report)
{
	char *lpstr;
	DWORD dwClientWin;

	lpstr = 0;

	switch(event) {
	case PropertyNotify:
		lpstr = XGetAtomName(dp->display,report->xproperty.atom);

		/* call our first and only handler for property change... */
		InternalClipboard( dp,
			report->xproperty.window,
			report->xproperty.atom,
			report->xproperty.time,
			report->xproperty.state);
#ifdef LATER
		if(state != PropertyDelete)
			GetICCCMData(dp,
				report->xproperty.atom,
				report->xproperty.state);
#endif
		break;

	case ClientMessage:
		lpstr = XGetAtomName(dp->display,report->xclient.message_type);

		/* what hWnd did this occur in... */
		if (XFindContext(dp->display,report->xclient.window,
			dp->client_hwnd,(caddr_t *)&dwClientWin))
			break;

		if (report->xclient.message_type == dp->wm_protocols) {
			if (report->xclient.data.l[0] == dp->wm_delete_window) {
			    MSG msg;
			    memset((LPVOID)&msg,'\0',sizeof(MSG));
			    msg.hwnd = (HWND)LOWORD(dwClientWin);
			    msg.message = WM_CLOSE;
			    (void)lpLibCallback(TWINLIBCALLBACK_POSTMESSAGE,
					0,0,(LPVOID)&msg);
			    return 1L;
			}
		}
		break;
	}

	if(lpstr) XFree(lpstr);
	return TRUE;
}
