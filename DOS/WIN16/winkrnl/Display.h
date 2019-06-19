/*
	@(#)Display.h	2.3
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
 
/* messages to DriverPrivateDC	*/
#define	DC_INIT		0
#define DC_GETDC	1
#define DC_RELEASE	2
#define DC_GETPTR	3
#define DC_LOCK		4
#define DC_GETDP	5
#define DC_GETDEVICE	6
#define DC_RELEASEDEV	7
#define DC_GETHDC	8

/* DC types */
#define DCT_COMPATIBLE  0
#define DCT_DISPLAY     1
