/*  Win_Reg.h	1.6
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

#ifndef Win_Reg__h
#define Win_Reg__h

/*
 *  This file is only valid for (T)WIN32
 */
#if defined(TWIN32)


DECLARE_HANDLE(HKEY);
typedef HKEY *PHKEY;


#endif /* TWIN32 */

#if 1
/*
** Windows 95 version of Win_Reg.h contains
** much of what we have presently in shellapi.h
** and so we will include that here.
*/
#include "shellapi.h"
#endif

#endif      /* Win_Reg__h */

