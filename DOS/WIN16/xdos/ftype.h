/*************************************************************************
*
*       @(#)ftype.h	1.2
*	dos file type definitions
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

#ifndef ftype__h
#define ftype__h

/* Defines for file access utility routine return codes */

#define		FT_ERROR	(-1)
#define		FT_NOTEXIST	0
#define		FT_NOPATH	1
#define		FT_PLAIN	2
#define		FT_DIR		3
#define		FT_DEV		4
#define		FT_OTHER	5

int do_ftype(char *);
#endif /* ftype__h */
