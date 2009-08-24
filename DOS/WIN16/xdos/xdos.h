/*************************************************************************
*
*       @(#)xdos.h	1.3
*	x86 error and carry flags
*       Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.

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

**************************************************************************/

/*******************************************
**
**	dos error codes
**
********************************************/

#ifndef xdos__h
#define xdos__h

#include	"windows.h"

/* Simulated CPU flags */
#define CARRY_FLAG	0x00010000
#define ZERO_FLAG	0x00400000

#endif /* xdos__h */
