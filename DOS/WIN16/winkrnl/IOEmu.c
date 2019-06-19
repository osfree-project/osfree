/*    
	IOEmu.c	2.4
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

#include "kerndef.h"
#include "BinTypes.h"		/* ENV structure */
#include "Log.h"		/* logging/tracing */

static DWORD GetTicks(void);

REGISTER PortIO(DWORD dwPort, DWORD dwData, UINT wOpSize, BOOL bWrite);

REGISTER
PortIO(DWORD dwPort, DWORD dwData, UINT wOpSize, BOOL bWrite)
{
    static BYTE bPITMode;
    static BYTE bCounter0Mode, bCounter2Mode;
    static BOOL bCounter0Toggle,bCounter2Toggle;
    static WORD wCounter0,wCounter2;
    static DWORD tCounter0Enabled;
    static DWORD tCounter2Enabled,tCounter2Disabled;
    static BOOL bCounter2Enabled = FALSE;
    BOOL bNewCounter2;
    DWORD tCurrent;
    REGISTER ret=0;

    switch (dwPort) {
	case 0x40:	/* PIT counter 0 */
	    if (wOpSize != 8) {
		LOGSTR((LF_LOG,"PortIO: port 0x40 supports only 8-bit ops\n"));
		return 0;
	    }
	    if (bWrite) {
		LOGSTR((LF_LOG,"PortIO: PIT counter0 writing %x\n",dwData));
		switch (bCounter0Mode >> 4) {
		    case 0:	/* counter latch not supported */
			break;
		    case 1:	/* bits 0-7 only */
			wCounter0 &= 0xff00;
			wCounter0 |= (dwData & 0xff);
			break;
		    case 2:	/* bits 8-15 only */
			wCounter0 &= 0xff;
			wCounter0 |= (dwData & 0xff) << 8;
			break;
		    case 3: 	/* bits 0-7 first, then 8-15 */
			if (bCounter0Toggle) {
			    wCounter0 &= 0xff;
			    wCounter0 |= (dwData & 0xff) << 8;
			}
			else {
			    wCounter0 &= 0xff00;
			    wCounter0 |= (dwData & 0xff);
			}
			bCounter0Toggle = (bCounter0Toggle)?FALSE:TRUE;
			break;
		}
		tCounter0Enabled = GetTicks();
	    }
	    else {
		tCurrent = GetTicks();
		ret = wCounter0 + (tCurrent - tCounter0Enabled);
		switch (bCounter0Mode >> 4) {
		    case 0:	/* counter latch not supported */
			ret = 0;
			break;
		    case 1:	/* bits 0-7 only */
			ret = ret & 0xff;
			break;
		    case 2:	/* bits 8-15 only */
			ret = (ret & 0xff00) >> 8;
			break;
		    case 3: 	/* bits 0-7 first, then 8-15 */
			ret = (bCounter0Toggle)?
				(ret & 0xff00) >> 8:ret & 0xff;
			bCounter0Toggle = (bCounter0Toggle)?FALSE:TRUE;
			break;
		}
		LOGSTR((LF_LOG,"PortIO: PIT counter0 read %x\n",ret));
	    }
	    break;
	case 0x42:	/* PIT counter 2 */
	    if (wOpSize != 8) {
		LOGSTR((LF_LOG,"PortIO: port 0x42 supports only 8-bit ops\n"));
		return 0;
	    }
	    if (bWrite) {
		LOGSTR((LF_LOG,"PortIO: PIT counter2 writing %x\n",dwData));
		switch (bCounter2Mode >> 4) {
		    case 0:	/* counter latch not supported */
			break;
		    case 1:	/* bits 0-7 only */
			wCounter2 &= 0xff00;
			wCounter2 |= (dwData & 0xff);
			break;
		    case 2:	/* bits 8-15 only */
			wCounter2 &= 0xff;
			wCounter2 |= (dwData & 0xff) << 8;
			break;
		    case 3: 	/* bits 0-7 first, then 8-15 */
			if (bCounter2Toggle) {
			    wCounter2 &= 0xff;
			    wCounter2 |= (dwData & 0xff) << 8;
			}
			else {
			    wCounter2 &= 0xff00;
			    wCounter2 |= (dwData & 0xff);
			}
			bCounter2Toggle = (bCounter2Toggle)?FALSE:TRUE;
			break;
		}
		if (bCounter2Enabled)
		    tCounter0Enabled = GetTicks();
	    }
	    else {
		if (!bCounter2Enabled)
		    tCurrent = tCounter2Disabled;
		else
		    tCurrent = GetTicks();
		ret = wCounter2 + (tCurrent - tCounter2Enabled);
		switch (bCounter0Mode >> 4) {
		    case 0:	/* counter latch not supported */
			ret = 0;
			break;
		    case 1:	/* bits 0-7 only */
			ret = ret & 0xff;
			break;
		    case 2:	/* bits 8-15 only */
			ret = (ret & 0xff00) >> 8;
			break;
		    case 3: 	/* bits 0-7 first, then 8-15 */
			ret = (bCounter2Toggle)?
				(ret & 0xff00) >> 8:ret & 0xff;
			bCounter2Toggle = (bCounter2Toggle)?FALSE:TRUE;
			break;
		}
		LOGSTR((LF_LOG,"PortIO: PIT counter2 read %x\n",ret));
	    }
	    break;
	case 0x43:	/* PIT mode port, control register counters 0-2 */
	    if (wOpSize != 8) {
		LOGSTR((LF_LOG,"PortIO: port 0x43 supports only 8-bit ops\n"));
		return 0;
	    }
	    if (bWrite) {
		LOGSTR((LF_LOG,"PortIO: PIT mode writing %x\n",dwData));
		bPITMode = dwData & 0xff;
		switch (bPITMode >> 6) {
		    case 0:	/* counter0 */
			bCounter0Mode = bPITMode & 0x3f;
			bCounter0Toggle = FALSE;
			/* counter latch not supported */
			/* BCD counter format not supported */
			/* only rate generator mode supported */
			if (!(bCounter0Mode & 0x30) ||
			    ((bCounter0Mode & 0x07) != 4))
			    LOGSTR((LF_LOG,
				"PortIO: counter0 mode %x not supported\n",
				bCounter0Mode));
			break;
		    case 1:	/* counter1 - not supported */
			LOGSTR((LF_LOG,"PortIO: counter1 not supported\n"));
			break;
		    case 2:
			bCounter2Mode = bPITMode & 0x3f;
			bCounter2Toggle = FALSE;
			/* counter latch not supported */
			/* BCD counter format not supported */
			/* only rate generator mode supported */
			if (!(bCounter2Mode & 0x30) ||
			    ((bCounter2Mode & 0x07) != 4))
			    LOGSTR((LF_LOG,
				"PortIO: counter2 mode %x not supported\n",
				bCounter2Mode));
			break;
		    case 3:	/* invalid */
			LOGSTR((LF_LOG,"PortIO: invalid counter 3\n"));
			break;
		}
	    }
	    else {
		ret = bPITMode;
		LOGSTR((LF_LOG,"PortIO: PIT mode read %x\n",ret));
	    }
	    break;
	case 0x61:	/* KB controller port B control register */
	    if (wOpSize != 8) {
		LOGSTR((LF_LOG,"PortIO: port 0x61 supports only 8-bit ops\n"));
		return 0;
	    }
	    if (bWrite) {
		bNewCounter2 = (dwData & 1)?TRUE:FALSE;
		if (bNewCounter2) {
		    if (!bCounter2Enabled) {
			tCounter2Enabled = GetTicks();
			bCounter2Enabled = TRUE;
		    }
		}
		else {
		    if (bCounter2Enabled) {
			tCounter2Disabled = GetTicks();
			bCounter2Enabled = FALSE;
		    }
		}
		LOGSTR((LF_LOG,"PortIO: KB controller writing %x\n",dwData));
	    }
	    else {
		ret = bCounter2Enabled;
		LOGSTR((LF_LOG,"PortIO: KB controller read %x\n",ret));
	    }
	    break;
	default:
	    ret = 0;
	    LOGSTR((LF_LOG,"PortIO: port %x size %d func %s data %x\n",
		dwPort,wOpSize,(bWrite)?"OUTPUT":"INPUT",dwData));
	    break;
    }
    return ret;
}

static DWORD
GetTicks(void)
{
    DWORD dwTicks;

    dwTicks = GetTickCount();
    dwTicks /= 55;		/* 1000/18.2 */
    return dwTicks;
}
