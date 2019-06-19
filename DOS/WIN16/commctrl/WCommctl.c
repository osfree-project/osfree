/*  
	WCommctl.c 	Utilities Library
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/

#define WCOMMCTRL
#ifndef macintosh
#include "WCommCtrl.h"
#include "WHeader.h"
#include "WListView.h"
#else
#include "WCommctl.h"
#endif


HINSTANCE                           hLibInstance = ( HINSTANCE )NULL;

extern int WINAPI  TabCtl_Initialize ( HINSTANCE hInstance );
extern int WINAPI  Tooltip_Initialize ( HINSTANCE hLibInstance );
extern int WINAPI  Trackbar_Initialize ( HINSTANCE hLibInstance );
extern int WINAPI  SpinBtn_Initialize ( HINSTANCE hLibInstance );
extern int WINAPI  Toolbr_Initialize ( HINSTANCE hLibInstance );
extern int WINAPI  Statbr_Initialize ( HINSTANCE hLibInstance );
extern int WINAPI  TView_Initialize ( HINSTANCE hLibInstance );
extern int WINAPI  ProgBar_Initialize ( HINSTANCE hLibInstance );
extern int WINAPI  LView_Initialize ( HINSTANCE hLibInstance );
extern int WINAPI  Headr_Initialize ( HINSTANCE hLibInstance );

extern void WINAPI Trackbar_Terminate ( HINSTANCE hLibInstance );
extern void WINAPI Tooltip_Terminate ( HINSTANCE hLibInstance );
extern void WINAPI SpinBtn_Terminate ( HINSTANCE hLibInstance );
extern void WINAPI TabCtl_Terminate ( HINSTANCE hLibInstance );
extern void WINAPI Toolbr_Terminate ( HINSTANCE hLibInstance );
extern void WINAPI Statbr_Terminate ( HINSTANCE hLibInstance );
extern void WINAPI TView_Terminate ( HINSTANCE hLibInstance );
extern void WINAPI ProgBar_Terminate ( HINSTANCE hLibInstance );
extern void WINAPI LView_Terminate ( HINSTANCE hLibInstance );
extern void WINAPI Headr_Terminate ( HINSTANCE hLibInstance );

void EXPORT WINAPI WInitCommonControls ()
{
    LView_Initialize ( hLibInstance );
    Headr_Initialize ( hLibInstance );
    Trackbar_Initialize ( hLibInstance );
    Tooltip_Initialize ( hLibInstance );
    SpinBtn_Initialize ( hLibInstance );
    TabCtl_Initialize ( hLibInstance );
    Toolbr_Initialize ( hLibInstance );
    Statbr_Initialize ( hLibInstance );
    TView_Initialize ( hLibInstance );
    ProgBar_Initialize ( hLibInstance );
}


static void WINAPI WTerminateCommonControls()
{
        LView_Terminate ( hLibInstance );
        Headr_Terminate ( hLibInstance );
        Trackbar_Terminate ( hLibInstance );
        Tooltip_Terminate ( hLibInstance );
        SpinBtn_Terminate ( hLibInstance );
	TabCtl_Terminate ( hLibInstance );
	Toolbr_Terminate ( hLibInstance );
	Statbr_Terminate ( hLibInstance );
	TView_Terminate ( hLibInstance );
	ProgBar_Terminate ( hLibInstance );
}

BOOL WINAPI DllMain (
    HINSTANCE                           hInstance,
    DWORD                               dReason,
    LPVOID                              pReserved )

{

    BOOL                                ReturnValue = TRUE;

    switch ( dReason )
    {
        case DLL_PROCESS_ATTACH:
            hLibInstance = hInstance;
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
	    WTerminateCommonControls();
            break;
    }

    return ( ReturnValue );

}

BOOL WINAPI EXPORT libcommctrl32_LibMain (
    HINSTANCE                           hInstance,
    LPSTR				lpCmdLine,
    int					nCmdShow )

{

    BOOL                                ReturnValue = TRUE;

    hLibInstance = hInstance;
    return ( ReturnValue );
}

int WINAPI EXPORT WEP ( )
{
    WTerminateCommonControls();
    return 0;
}
