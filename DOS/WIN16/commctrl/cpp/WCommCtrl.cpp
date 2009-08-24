//==============================================================================
//
//  Utilities Library
//
//  @(#)WCommCtrl.cpp	1.9
//
//  Copyright (c) 1996 Willows Software Inc. All Rights Reserved
//
//==============================================================================



#define WCOMMCTRL
#ifndef MAC
#include "commctrl/WCommCtrl.h"
#include "commctrl/WHeader.h"
#include "commctrl/WListView.h"
#else
#include "WCommCtrl.h"
#include "WHeader.h"
#include "WListView.h"
#endif


CWSharedLib                             *pWCommCtrl  = NULL;

extern "C"
{
    HINSTANCE                           hLibInstance = ( HINSTANCE )NULL;
    char                                WCCDebugString [ 128 ];
}

extern "C" int WINAPI  TabCtl_Initialize ( HINSTANCE hInstance );
extern "C" int WINAPI  Tooltip_Initialize ( HINSTANCE hLibInstance );
extern "C" int WINAPI  Trackbar_Initialize ( HINSTANCE hLibInstance );
extern "C" int WINAPI  SpinBtn_Initialize ( HINSTANCE hLibInstance );
extern "C" int WINAPI  Toolbr_Initialize ( HINSTANCE hLibInstance );
extern "C" int WINAPI  Statbr_Initialize ( HINSTANCE hLibInstance );
extern "C" int WINAPI  TView_Initialize ( HINSTANCE hLibInstance );

extern "C" void WINAPI Trackbar_Terminate ( HINSTANCE hLibInstance );
extern "C" void WINAPI Tooltip_Terminate ( HINSTANCE hLibInstance );
extern "C" void WINAPI SpinBtn_Terminate ( HINSTANCE hLibInstance );
extern "C" void WINAPI TabCtl_Terminate ( HINSTANCE hLibInstance );
extern "C" void WINAPI Toolbr_Terminate ( HINSTANCE hLibInstance );
extern "C" void WINAPI Statbr_Terminate ( HINSTANCE hLibInstance );
extern "C" void WINAPI TView_Terminate ( HINSTANCE hLibInstance );

extern "C" void EXPORT WINAPI WInitCommonControls ()
{
    WInitHeader();
    WInitListView();
    Trackbar_Initialize ( hLibInstance );
    Tooltip_Initialize ( hLibInstance );
    SpinBtn_Initialize ( hLibInstance );
    TabCtl_Initialize ( hLibInstance );
    Toolbr_Initialize ( hLibInstance );
    Statbr_Initialize ( hLibInstance );
    TView_Initialize ( hLibInstance );
}


static void WINAPI WTerminateCommonControls()
{
        Trackbar_Terminate ( hLibInstance );
        Tooltip_Terminate ( hLibInstance );
        SpinBtn_Terminate ( hLibInstance );
	TabCtl_Terminate ( hLibInstance );
	Toolbr_Terminate ( hLibInstance );
	Statbr_Terminate ( hLibInstance );
	TView_Terminate ( hLibInstance );
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
            if ( ! pWCommCtrl )
                if ( ! ( pWCommCtrl = new CWSharedLib ( hInstance ) ) )
                    ReturnValue = FALSE;
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            if ( pWCommCtrl )
            {
                delete ( pWCommCtrl );
                pWCommCtrl = NULL;
            }
	    WTerminateCommonControls();
            break;
    }

    return ( ReturnValue );

}

extern "C" BOOL WINAPI EXPORT Commctrl_LibMain (
    HINSTANCE                           hInstance,
    LPSTR				lpCmdLine,
    int					nCmdShow )

{

    BOOL                                ReturnValue = TRUE;

    hLibInstance = hInstance;
    if ( ! pWCommCtrl )
         if ( ! ( pWCommCtrl = new CWSharedLib ( hInstance ) ) )
              ReturnValue = FALSE;

    return ( ReturnValue );
}

extern "C" int WINAPI EXPORT WEP ( )
{
   if ( pWCommCtrl )
   {
       delete ( pWCommCtrl );
       pWCommCtrl = NULL;
    }
    WTerminateCommonControls();

}
