/*
	@(#)Objects.h	2.4
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
 
#ifndef Objects__h
#define Objects__h

static HGDIOBJ StockObjects[] =
  {
    0,                          /* WHITE_BRUSH */
    0,                          /* LTGRAY_BRUSH */
    0,                          /* GRAY_BRUSH */
    0,                          /* DKGRAY_BRUSH */
    0,                          /* BLACK_BRUSH */
    0,                          /* NULL_BRUSH */
    0,                          /* WHITE_PEN */
    0,                          /* BLACK_PEN */
    0,                          /* NULL_PEN */
    0,
    0,                          /* OEM_FIXED_FONT */
    0,                          /* ANSI_FIXED_FONT */
    0,                          /* ANSI_VAR_FONT */
    0,                          /* SYSTEM_FONT */
    0,                          /* DEVICE_DEFAULT_FONT */
    0,                          /* DEFAULT_PALETTE */
    0,                          /* SYSTEM_FIXED_FONT */
    0,
    0,
    0				/* SYSTEM_BITMAP */
  };

/* blue,green,red */
RGBQUAD VGAColors[16]  = {
	{ 0,0,0,0 },			/* black */
	{ 0,0,128,0 },			/* dark red */
	{ 0,128,0,0 },			/* dark green */
	{ 0,128,128,0 },		/* dark yellow */
	{ 128,0,0,0 },			/* dark blue */
	{ 128,0,128,0 },		/* dark magenta */
	{ 128,128,0,0 },		/* dark cyan */
	{ 192,192,192,0 },		/* light gray */
	{ 128,128,128,0 },		/* medium gray */
	{ 0,0,255,0 },			/* red */
	{ 0,255,0,0 },			/* green */
	{ 0,255,255,0 },		/* yellow */
	{ 255,0,0,0 },			/* blue */
	{ 255,0,255,0 },		/* magenta */
	{ 255,255,0,0 },		/* cyan */
	{ 255,255,255,0 },		/* white */
};

static RGBQUAD NiceColors[4]  = {
	{ 192,220,192,0 },		/* money green */
	{ 240,202,166,0 },		/* sky blue */
	{ 240,251,255,0 },		/* cream */
	{ 164,160,160,0 },		/* light gray */
};

#endif /* Objects__h */
