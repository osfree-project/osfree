/*
	@(#)DrvROPCodes.h	2.3
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
 
#define RI_1	R2_BLACK	/* 0		GXclear	*/
#define RI_2 	R2_NOTMERGEPEN	/* DPon		GXnor	*/  
#define RI_3 	R2_MASKNOTPEN	/* DPna		GXandInverted	*/  
#define RI_4 	R2_NOTCOPYPEN	/* Pn  		GXcopyInverted	*/ 
#define RI_5 	R2_MASKPENNOT	/* PDna		GXandReverse	*/
#define RI_6 	R2_NOT		/* Dn  		GXinvert	*/
#define RI_7 	R2_XORPEN	/* DPx 		GXxor	*/
#define RI_8 	R2_NOTMASKPEN	/* DPan		GXnand	*/
#define RI_9 	R2_MASKPEN	/* DPa 		GXand	*/
#define RI_10	R2_NOTXORPEN	/* DPxn		GXequiv	*/
#define RI_11	R2_NOP		/* D   		GXnoop	*/
#define RI_12	R2_MERGENOTPEN	/* DPno		GXorInverted	*/
#define RI_13	R2_COPYPEN	/* P   		GXcopy	*/
#define RI_14	R2_MERGEPENNOT	/* PDno		GXorReverse	*/
#define RI_15	R2_MERGEPEN	/* DPo 		GXor	*/
#define RI_16	R2_WHITE	/* 1		GXset	*/

#define ROP_BD	0x0100	/* fill dest with brush */
#define ROP_FD	0x0200  /* fill dest w/o brush */
#define ROP_SD	0x0300	/* copy src into dest	*/
#define ROP_DB	0x8400	/* create pixmap; combine dest with brush into pixmap */
#define ROP_DS	0x8500	/* create pixmap; combine dest with src into pixmap */
#define ROP_SB	0x8600	/* create pixmap; combine src with brush into pixmap */
#define ROP_SP	0x0700  /* copy src into pixmap */
#define ROP_BP	0x0800  /* fill pixmap with brush */
#define ROP_PD	0x0900  /* copy pixmap into dest */

typedef struct {
	DWORD   dwROP;
	WORD	bRops[4];
} ROPtable;

