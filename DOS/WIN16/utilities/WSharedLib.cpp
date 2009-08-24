//==============================================================================
//
//
//  @(#)WSharedLib.cpp	1.4
//  Shared Library
//
//  Copyright (c) 1996 Willows Software Inc. All Rights Reserved
//
//==============================================================================


#include "WSharedLib.h"



int CWSharedLib::Error ( int    NewError )
{

    int                         ReturnError = _Error;

    _Error = NewError;

    return ( ReturnError );

}

CWSharedLib::CWSharedLib ( HINSTANCE	hInstance )

{ 
	_hInstance = hInstance;
	_CatchBufIndex = 0;

}

