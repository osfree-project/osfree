/*    
	LSDevices.c	2.15
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

#include "windows.h"
#include "windowsx.h"

#include "Log.h"
#include "GdiDC.h"
#include "kerndef.h"

extern DWORD DCDisplayProc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
extern DWORD DCPrinterProc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
extern DWORD DCMapModeProc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
extern DWORD DCBoundsRectProc(WORD,HDC32,DWORD,LPLSDS_PARAMS);
extern DWORD DCMetaProc(WORD,HDC32,DWORD,LPLSDS_PARAMS);

#define INIT_DRIVER_SLOTS	3	/* Number of slots to start with. */
#define EXPAND_DRIVER_SLOTS	3	/* Number to add when we need more. */

/* exported functions */
BOOL AddDriverEntry(LPSTR,GDIPROC,GDIPROC,GDIPROC);
LSDENTRY *GetDeviceEntry(LPCSTR lpszDriver);
void TWIN_InitLSD();

/* static functions */
static BOOL ExpandDriverTable(void);

/* Pointer to an expandable array of LSDENTRIES */
static LSDENTRY *DriverTable = NULL;
static int DriverTableSlots = 0;		/* Slots in allocated array. */
static int DriverTableUsed = 0;			/* Number of slots used. */

/* this is used for speedy referencing of DISPLAY tables in GetDC */
LSDENTRY *DisplayDeviceEntry = 0;

/* Increase the size of the driver table array. */
static BOOL
ExpandDriverTable(void)
{
	/* Initial allocation? */
	if(DriverTableSlots == 0) {
	    DriverTable = (LSDENTRY *)
		WinMalloc(INIT_DRIVER_SLOTS * sizeof(LSDENTRY));
	    DriverTableSlots = INIT_DRIVER_SLOTS;
	    return TRUE;
	}
	else {	/* Expand the existing array. */
	    LSDENTRY *NewDriverTable;
	    int NewSlots;

	    NewSlots = DriverTableSlots + EXPAND_DRIVER_SLOTS;
	    NewDriverTable = (LSDENTRY *)
		WinRealloc((char *)DriverTable,NewSlots * sizeof(LSDENTRY));

	    DriverTable = NewDriverTable;
	    DriverTableSlots = NewSlots;

	    return TRUE;
	}
}

BOOL
AddDriverEntry(LPSTR lpszName, GDIPROC lpfnOutput,
		GDIPROC lpfnMapMode, GDIPROC lpfnBoundsRect)
{
	LSDENTRY *NewDriver;

	/* Do we need to expand the driver table array? */
	if (DriverTableUsed == DriverTableSlots) {
	    if(!ExpandDriverTable())
		return FALSE;		/* Sorry! */
	}

	NewDriver = &DriverTable[DriverTableUsed++];

	strncpy(NewDriver->Name, lpszName, MAX_DEVICE_NAME - 1);
	NewDriver->Name[MAX_DEVICE_NAME - 1] = '\0';

	NewDriver->OutputProc = lpfnOutput;
	NewDriver->MapModeProc = lpfnMapMode;
	NewDriver->BoundsRectProc = lpfnBoundsRect;

	if (!(strcasecmp(lpszName,"DISPLAY")))
	    DisplayDeviceEntry = NewDriver;

	return TRUE;
}

LSDENTRY *
GetDeviceEntry(LPCSTR lpszDriver)
{
    int i;

    for(i = 0; i < DriverTableUsed; ++i)
	if (!strncasecmp(lpszDriver, DriverTable[i].Name,MAX_DEVICE_NAME-1))
	    return &DriverTable[i];
    return (LSDENTRY *)0;
}

void
TWIN_InitLSD()
{
    AddDriverEntry("DISPLAY", &DCDisplayProc, &DCMapModeProc, &DCBoundsRectProc);
    AddDriverEntry("PSCRIPT", &DCPrinterProc, &DCMapModeProc, &DCBoundsRectProc);
    AddDriverEntry("META", &DCMetaProc, &DCMetaProc, &DCBoundsRectProc);
}

