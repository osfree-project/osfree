/*
	@(#)DrvRegions.h	1.6
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
 
DECLARE_HANDLE32(REGION32);

#define	PRH_CREATEREGION		0x0003
#define	PRH_CREATEPOLYREGION		0x0004
#define	PRH_CREATERECTREGION		0x0005
#define	PRH_CREATEELLIPTICREGION	0x0006
#define	PRH_CREATEROUNDRECTREGION	0x0007
#define	PRH_DESTROYREGION		0x0008

#define	PRH_SETREGIONEMPTY		0x0009
#define	PRH_SETRECTREGION		0x000a

#define	PRH_PTINREGION			0x000b
#define	PRH_RECTINREGION		0x000c
#define	PRH_ISEMPTYREGION		0x000d
#define	PRH_EQUALREGION			0x000e

#define	PRH_OFFSETREGION		0x000f
#define	PRH_UNIONREGION			0x0010
#define	PRH_COPYREGION			0x0011
#define	PRH_INTERSECTREGION		0x0012
#define	PRH_DIFFREGION			0x0013
#define	PRH_XORREGION			0x0014
#define	PRH_UNIONRECTWITHREGION		0x0015

#define	PRH_REGIONBOX			0x0016

/* capabilities masks */
#define	RSUB_CANDO_RECT			0x0001
#define	RSUB_CANDO_ELLIPTIC		0x0002
#define	RSUB_CANDO_ROUNDRECT		0x0004
#define	RSUB_CANDO_FLOODFILL		0x0008
