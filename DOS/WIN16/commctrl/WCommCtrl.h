
/* WCommCtrl.h	
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

#define NONAMELESSUNION
#ifndef WCOMMCTRL_H
#define WCOMMCTRL_H




#if ( WINVER < 0x0400 ) 
#undef WINVER
#define WINVER                          0x0400
#endif
#include "windows.h"
#include <stdio.h>
#include "commctrl.h"
#include "Willows.h"

#ifndef _WINDOWS
#define WInitCommonControls	InitCommonControls
#endif


#ifndef WCOMMCTRL


#   if defined ( __cplusplus )
        extern CWSharedLib                      *pWCommCtrl;
#   endif /* #if defined ( __cplusplus ) */

#   if defined ( __cplusplus )
    extern "C"
    {
#   endif /* #if defined ( __cplusplus ) */

extern char                             WCCDebugString[];
void EXPORT WINAPI WInitCommonControls ();

#   if defined ( __cplusplus )
    }
#   endif /* #if defined ( __cplusplus ) */

#endif


#endif /* #ifndef WCOMMCTRL_H */
