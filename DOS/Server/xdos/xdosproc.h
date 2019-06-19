/*************************************************************************
*
*
*       @(#)xdosproc.h	1.6
*	definitions of supported x86 interrupts
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

#ifndef xdosproc__h
#define xdosproc__h

#include "BinTypes.h"

void INTTrap    ( int, ENV * );
void INT_handler( int, ENV * );
void int_10(int, ENV *);
void int_11(int, ENV *);
void int_12(int, ENV *);
void int_13(int, ENV *);
void int_15(int, ENV *);
void int_1a(int, ENV *);
void int_21(int, ENV *);
void int_25(int, ENV *);
void int_2a(int, ENV *);
void int_2f(int, ENV *);
void int_31(int, ENV *);
void int_3d(int, ENV *);
void int_5c(int, ENV *);

DWORD GetVector( int );
void SetVector  ( DWORD, ENV * );

#endif /* xdosproc__h */
