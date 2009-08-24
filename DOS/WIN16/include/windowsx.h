/*  windowsx.h	2.11
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
 
#ifndef windowsx__h
#define windowsx__h

/****** Data type Macros *******************************/

#define LOSHORT(x)	(short int)LOWORD(x)
#define HISHORT(x)	(short int)HIWORD(x)
#define LOINT(x)	(int)LOSHORT(x)
#define HIINT(x)	(int)HISHORT(x)

#ifndef	TWIN32
#include "windowsx16.h"
#else
#include "windowsx32.h"
#endif

#endif /* windowsx__h */
