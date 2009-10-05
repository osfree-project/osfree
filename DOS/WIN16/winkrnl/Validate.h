/*
	@(#)Validate.h	1.4
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


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*/
 
#include "windows.h"

static BYTE AllowedModules[] = {
    0x32, 0xf2, 0xca, 0,				/* SOL */
    0x12, 0xc2, 0x82, 0x2a, 0x72, 0x92, 0xea, 0,	/* WINTACH */
    0x32, 0xa2, 0xc2, 0x1a, 0xa2, 0,			/* EXCEL */
    0x62, 0x32, 0xf2, 0xe2, 0,				/* GOLF */
    0xa,  0x2a, 0,					/* TP */
    0xa,  0x32, 0xa2, 0x12, 0x72, 0x92, 0xea, 0,	/* WINHELP */
    0x4a, 0xa2, 0xa,  0xa2, 0xa2, 0xea, 0xca, 0xa2, 0x72, 0x92, 0xb2, 0,
						/* MINESWEEPER (WINMINE) */
    0
};

