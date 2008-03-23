//==============================================================================
//
// @(#)WUtilities.cpp	1.5
//  Utilities Library
//
//  Copyright (c) 1996 Willows Software Inc. All Rights Reserved
//
//==============================================================================


#define WUTILITIES
#include "WUtilities.h"

CWSharedLib                             *pWUtilities = NULL;
char                                    WUDebugString [ 128 ];
int * CatchBuffer;



extern "C" BOOL WINAPI WUtilities (
    HINSTANCE                           hInstance,
    DWORD                               dReason,
    LPVOID                              pReserved )

{

    BOOL                                ReturnValue = TRUE;

    switch ( dReason )
    {
    case DLL_PROCESS_ATTACH:
        if ( ! pWUtilities )
            if ( ! ( pWUtilities = new CWSharedLib ( hInstance ) ) )
                ReturnValue = FALSE;
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        if ( pWUtilities )
        {
            delete ( pWUtilities );
            pWUtilities = NULL;
        }
        break;
    }

    return ( ReturnValue );

}

extern "C" BOOL WINAPI EXPORT Utilities_LibMain (
    HINSTANCE                           hInstance,
    LPSTR				lpCmdLine,
    int					nCmdShow )

{
    return WUtilities(hInstance, DLL_PROCESS_ATTACH, NULL);
}

extern "C" int WINAPI EXPORT WEP ( )
{
    return WUtilities(0, DLL_PROCESS_DETACH, NULL);
}
