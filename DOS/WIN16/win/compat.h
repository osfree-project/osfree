/*************************************************************************
	@(#)compat.h	2.13
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

**************************************************************************/
 


/***************************************************************
 * Compatibility flag is the bitwize OR of the following...
 * it is defined as the control keyword in the [boot] section
 ***************************************************************/

/* server related functions */
#define WD_CONSOLE	1
#define WD_VERBOSE	2
#define WD_SYSTEM	4
#define WD_LOGGING	8

/* server related functions */
#define WD_VGAMODE	0x10    /* default to 640x480x16colors  */
#define WD_COLORMAP	0x20	/* allocate private colormap    */
#define WD_NODITHERPURE 0x40	/* how to handle dithering      */
#define WD_SYNCRONOUS	0x80	/* syncronous server connection */

/* kernel related functions */
#define WD_NOPAGING	0x100   /* disable demand page loading  */
#define WD_MAPCURDIR	0x200   /* xdos map current directory   */
#define WD_HANDLERR	0x400   /* break on handleobj errors    */
#define WD_APISTUBS     0x800   /* msgbox for api stubs		*/

/* file related functions */
#define WD_FILENOTFOUND 0x2000  /* print when file not found */
#define WD_NOCHANGECASE 0x4000  /* don't change case of filenames */
