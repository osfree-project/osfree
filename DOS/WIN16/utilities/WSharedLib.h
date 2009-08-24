/*==============================================================================
*
*   Shared Library
*
*   @(#)WSharedLib.h	1.4 11:01:45 9/23/96 /users/sccs/src/win/utilities/s.WSharedLib.h 
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/



#ifndef __WSHAREDLIB_H__
#define __WSHAREDLIB_H__



#include "windows.h"
#include "Willows.h"
#include "WErr.h"


#ifndef WEXCEPTIONS
#define MAX_CATCHBUF			5
#endif


#if defined ( __cplusplus )

class EXPORT CWSharedLib
{

    public:

        CWSharedLib ( HINSTANCE hInstance );

        ~CWSharedLib ( void ) { };

        int Error ( int NewError );

        int Error ( void ) { return ( _Error ); }

        HINSTANCE hInstance ( void ) { return ( _hInstance ); }

#ifdef WEXCEPTIONS
#define THROW	throw
#else
		int CATCH ();
		void THROW ( int	Exception );
#endif    
    private:

        int                     _Error;
        HINSTANCE               _hInstance;
		CATCHBUF				_CatchBuf [ MAX_CATCHBUF ];
		int						_CatchBufIndex;

};

#endif /* __cplusplus */

#endif /* __WSHAREDLIB_H__ */


