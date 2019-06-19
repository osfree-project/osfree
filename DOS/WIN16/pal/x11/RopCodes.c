/*
	@(#)RopCodes.c	2.5
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
 
#include <X11/X.h>

#include "windows.h"
#include "DrvROPCodes.h"

int ROPS[] = {
	0,
	/* R2_BLACK */ 		GXclear,       	/*  0	    */
	/* R2_NOTMERGEPEN */ 	GXnor,  	/* DPon	    */
	/* R2_MASKNOTPEN  */	GXandInverted,  /* DPna	    */
	/* R2_NOTCOPYPEN  */	GXcopyInverted, /* PN	    */
	/* R2_MASKPENNOT*/	GXandReverse,   /* PDna	    */
	/* R2_NOT*/		GXinvert,	/* Dn	    */
	/* R2_XORPEN*/		GXxor,	      	/* DPx	    */
	/* R2_NOTMASKPEN*/	GXnand,		/* DPan	    */
	/* R2_MASKPEN*/		GXand,		/* DPa	    */
	/* R2_NOTXORPEN*/	GXequiv,  	/* DPxn     */
	/* R2_NOP*/		GXnoop,	 	/* D	    */
	/* R2_MERGENOTPEN*/	GXorInverted,   /* DPno     */
	/* R2_COPYPEN*/		GXcopy,	      	/* P	    */
	/* R2_MERGEPENNOT*/	GXorReverse, 	/* PDno     */
	/* R2_MERGEPEN*/	GXor,      	/* DPo	    */
	/* R2_WHITE*/		GXset	      	/*  1	    */
};

ROPtable       ROPTab[256] = {
  { BLACKNESS,{ ROP_FD|RI_1,  0,     	    0,            0   }},/* 0        */
  { 0x010289L,{ ROP_SB|RI_15, ROP_PD|RI_2,  0,		  0   }},/* DPSoon   */
  { 0x020C89L,{ ROP_SB|RI_15, ROP_PD|RI_3,  0,		  0   }},/* DPSona   */
  { 0x0300AAL,{ ROP_BD|RI_13, ROP_SD|RI_2,  0,		  0   }},/* PSon     */
  { 0x040C88L,{ ROP_BD|RI_2,  ROP_SD|RI_9,  0,		  0   }},/* SDPona   */
  { 0x0500A9L,{ ROP_BD|RI_2,  0,	    0,		  0   }},/* DPon     */
  { 0x060865L,{ ROP_SD|RI_10, ROP_BD|RI_2,  0,		  0   }},/* PDSxnon  */
  { 0x0702C5L,{ ROP_SD|RI_9,  ROP_BD|RI_2,  0,		  0   }},/* PDSaon   */
  { 0x080f08L,{ ROP_BD|RI_3,  ROP_SD|RI_9,  0,		  0   }},/* SDPnaa   */
  { 0x090245L,{ ROP_SD|RI_7,  ROP_BD|RI_2,  0,		  0   }},/* PDSxon   */
  { 0x0A0329L,{ ROP_BD|RI_3,  0,	    0,		  0   }},/* DPna     */
  { 0x0B0B2AL,{ ROP_SD|RI_5,  ROP_BD|RI_2,  0,		  0   }},/* PSDnaon  */
  { 0x0C0324L,{ ROP_BD|RI_4,  ROP_SD|RI_9,  0,		  0   }},/* SPna     */
  { 0x0D0B25L,{ ROP_SD|RI_3,  ROP_BD|RI_2,  0,		  0   }},/* PDSnaon  */
  { 0x0E08A5L,{ ROP_SD|RI_2,  ROP_BD|RI_2,  0,		  0   }},/* PDSonon  */
  { 0x0F0001L,{ ROP_BD|RI_4,  0,	    0,		  0   }},/* Pn       */

  { 0x100C85L,{ ROP_SD|RI_2,  ROP_BD|RI_9,  0,		  0   }},/* PDSona   */
  { NOTSRCERASE,{ROP_SD|RI_2, 0,	    0,		  0   }},/* DSon     */
  { 0x120868L,{ ROP_BD|RI_10, ROP_SD|RI_2,  0,		  0   }},/* SDPxnon  */
  { 0x1302C8L,{ ROP_BD|RI_9,  ROP_SD|RI_2,  0,		  0   }},/* SDPaon   */
  { 0x140869L,{ ROP_SB|RI_10, ROP_PD|RI_2,  0,		  0   }},/* DPSxnon  */
  { 0x1502C9L,{ ROP_SB|RI_9,  ROP_PD|RI_2,  0,		  0   }},/* DPSaon   */
  { 0x165CCAL,{ ROP_SB|RI_8,  ROP_PD|RI_9,  ROP_SD|RI_7,  ROP_BD|RI_7 }},
							 	 /* PSDPSanaxx*/
  { 0x171D54L,{ ROP_SD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_9,  ROP_SD|RI_10}},
								 /* SSPxDSxaxn*/
  { 0x180D59L,{ ROP_BD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_9,  0   }},/* SPxPDxa  */
  { 0x191CC8L,{ ROP_SB|RI_8,  ROP_PD|RI_9,  ROP_SD|RI_10, 0   }},/* SDPSanaxn */
  { 0x1A06C5L,{ ROP_SB|RI_9,  ROP_PD|RI_15, ROP_BD|RI_7,  0   }},/* PDSPaox  */
  { 0x1B0768L,{ ROP_SB|RI_7,  ROP_PD|RI_9,  ROP_SD|RI_10, 0   }},/* SDPSxaxn */
  { 0x1C06CAL,{ ROP_BD|RI_9,  ROP_SD|RI_15, ROP_BD|RI_7,  0   }},/* PSDPaox  */
  { 0x1D0766L,{ ROP_DB|RI_7,  ROP_SP|RI_9,  ROP_PD|RI_10, 0   }},/* DSPDxaxn */
  { 0x1E01A5L,{ ROP_SD|RI_15, ROP_BD|RI_7,  0,		  0   }},/* PDSox    */
  { 0x1F0385L,{ ROP_SD|RI_15, ROP_BD|RI_8,  0,            0   }},/* PDSoan   */

  { 0x200F09L,{ ROP_SB|RI_3,  ROP_PD|RI_9,  0,		  0   }},/* DPSnaa   */
  { 0x210248L,{ ROP_BD|RI_7,  ROP_SD|RI_2,  0,		  0   }},/* SDPxon   */
  { 0x220326L,{ ROP_SD|RI_3,  0,	    0,		  0   }},/* DSna     */
  { 0x230B24L,{ ROP_BD|RI_5,  ROP_SD|RI_2,  0,		  0   }},/* SPDnaon  */
  { 0x240D55L,{ ROP_SD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_9,  0   }},/* SPxDSxa  */
  { 0x251CC5L,{ ROP_SB|RI_8,  ROP_PD|RI_9,  ROP_BD|RI_10, 0   }},/* PDSPanaxn */
  { 0x2606C8L,{ ROP_SB|RI_9,  ROP_PD|RI_15, ROP_SD|RI_7,  0   }},/* SDPSaox  */
  { 0x271868L,{ ROP_SB|RI_10, ROP_PD|RI_15, ROP_SD|RI_7,  0   }},/* SDPSxnox */
  { 0x280369L,{ ROP_SB|RI_7,  ROP_PD|RI_9,  0,		  0   }},/* DPSxa    */
  { 0x2916CAL,{ ROP_SB|RI_9,  ROP_PD|RI_15, ROP_SD|RI_7,  ROP_BD|RI_10 }},
								 /* PSDPSaoxxn*/
  { 0x2A0CC9L,{ ROP_SB|RI_8,  ROP_PD|RI_9,  0,		  0   }},/* DPSana   */
  { 0x2B1D58L,{ ROP_BD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_9,  ROP_SD|RI_10 }},
								 /* SSPxPDxaxn*/
  { 0x2C0784L,{ ROP_SD|RI_15, ROP_BD|RI_9,  ROP_SD|RI_7,  0   }},/* SPDSoax  */
  { 0x2D060AL,{ ROP_SD|RI_14, ROP_BD|RI_7,  0,		  0   }},/* PSDnox   */
  { 0x2E064AL,{ ROP_BD|RI_7,  ROP_SD|RI_15, ROP_BD|RI_7,  0   }},/* PSDPxox  */
  { 0x2F0E2AL,{ ROP_SD|RI_14, ROP_BD|RI_8,  0,		  0   }},/* PSDnoan  */

  { 0x30032AL,{ ROP_BD|RI_13, ROP_SD|RI_3,  0,		  0   }},/* PSna     */
  { 0x310B28L,{ ROP_BD|RI_3,  ROP_SD|RI_2 , 0,		  0   }},/* SDPnaon  */
  { 0x320688L,{ ROP_SB|RI_15, ROP_PD|RI_15, ROP_SD|RI_7,  0   }},/* SDPSoox  */
  { NOTSRCCOPY,{ROP_SD|RI_4,  0,	    0,		  0   }},/* Sn	     */
  { 0x3406C4L,{ ROP_SD|RI_9,  ROP_BD|RI_15, ROP_SD|RI_7,  0   }},/* SPDSaox  */
  { 0x351864L,{ ROP_SD|RI_10, ROP_BD|RI_15, ROP_SD|RI_7,  0   }},/* SPDSxnox */
  { 0x3601A8L,{ ROP_BD|RI_15, ROP_SD|RI_7,  0,		  0   }},/* SDPox    */
  { 0x370388L,{ ROP_BD|RI_15, ROP_SD|RI_8,  0,		  0   }},/* SDPoan   */
  { 0x38078AL,{ ROP_BD|RI_15, ROP_SD|RI_9,  ROP_BD|RI_7,  0   }},/* PSDPoax  */
  { 0x390604L,{ ROP_BD|RI_14, ROP_SD|RI_7,  0,		  0   }},/* SPDnox   */
  { 0x3A0644L,{ ROP_SD|RI_7,  ROP_BD|RI_15, ROP_SD|RI_7,  0   }},/* SPDSxox  */
  { 0x3B0E24L,{ ROP_BD|RI_14, ROP_SD|RI_8,  0,		  0   }},/* SPDnoan  */
  { 0x3C004AL,{ ROP_BD|RI_13, ROP_SD|RI_7,  0,		  0   }},/* PSx      */
  { 0x3D18A4L,{ ROP_SD|RI_2,  ROP_BD|RI_15, ROP_SD|RI_7,  0   }},/* SPDSonox */
  { 0x3E1B24L,{ ROP_SD|RI_3,  ROP_BD|RI_15, ROP_SD|RI_7,  0   }},/* SPDSnaox */
  { 0x3F00EAL,{ ROP_BD|RI_13, ROP_SD|RI_8,  0,		  0   }},/* PSan     */

  { 0x400F0AL,{ ROP_SD|RI_5,  ROP_BD|RI_9,  0,		  0   }},/* PSDnaa   */
  { 0x410249L,{ ROP_SB|RI_7,  ROP_PD|RI_2,  0,		  0   }},/* DPSxon   */
  { 0x420D5DL,{ ROP_DB|RI_7,  ROP_SD|RI_7,  ROP_PD|RI_9,  0   }},/* SDxPDxa  */
  { 0x431CC4L,{ ROP_SD|RI_8,  ROP_BD|RI_9,  ROP_SD|RI_10, 0   }},/* SPDSanaxn */
  { SRCERASE, { ROP_SD|RI_5,  0,	    0,		  0   }},/* SDna     */
  { 0x450B29L,{ ROP_SB|RI_3,  ROP_PD|RI_2,  0,		  0   }},/* DPSnaon  */
  { 0x4606C6L,{ ROP_DB|RI_9,  ROP_SP|RI_15, ROP_PD|RI_7,  0   }},/* DSPDaox  */
  { 0x47076AL,{ ROP_BD|RI_7,  ROP_SD|RI_9,  ROP_BD|RI_10, 0   }},/* PSDPxaxn */
  { 0x480368L,{ ROP_BD|RI_7,  ROP_SD|RI_9,  0,		  0   }},/* SDPxa    */
  { 0x4916C5L,{ ROP_DB|RI_9,  ROP_SP|RI_15, ROP_PD|RI_7,  ROP_BD|RI_10 }},
								 /* PDSPDaoxxn*/
  { 0x4A0789L,{ ROP_DS|RI_15, ROP_BP|RI_9,  ROP_PD|RI_7,  0   }},/* DPSDoax  */
  { 0x4B0605L,{ ROP_SD|RI_12, ROP_BD|RI_7,  0,		  0   }},/* PDSnox   */
  { 0x4C0CC8L,{ ROP_BD|RI_8,  ROP_SD|RI_9,  0,		  0   }},/* SDPana   */
  { 0x4D1954L,{ ROP_SD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_15, ROP_SD|RI_10 }},
								 /* SSPxDSxoxn*/
  { 0x4E0645L,{ ROP_SB|RI_7,  ROP_PD|RI_15, ROP_BD|RI_7,  0   }},/* PDSPxox  */
  { 0x4F0E25L,{ ROP_SD|RI_12, ROP_BD|RI_8,  0,		  0   }},/* PDSnoan  */

  { 0x500325L,{ ROP_BD|RI_5,  0,	    0,		  0   }},/* PDna     */
  { 0x510B26L,{ ROP_SB|RI_5,  ROP_PD|RI_2,  0,		  0   }},/* DSPnaon  */
  { 0x5206C9L,{ ROP_DS|RI_9,  ROP_BP|RI_15, ROP_PD|RI_7,  0   }},/* DPSDaox  */
  { 0x530764L,{ ROP_SD|RI_7,  ROP_BD|RI_9,  ROP_SD|RI_10, 0   }},/* SPDSxaxn */
  { 0x5408A9L,{ ROP_SB|RI_2,  ROP_PD|RI_2,  0,		  0   }},/* DPSonon  */
  { DSTINVERT,{ ROP_FD|RI_6,  0,	    0,		  0   }},/* Dn       */
  { 0x5601A9L,{ ROP_SB|RI_15, ROP_PD|RI_7,  0,		  0   }},/* DPSox    */
  { 0x570389L,{ ROP_SB|RI_15, ROP_PD|RI_8,  0,		  0   }},/* DPSoan   */
  { 0x580785L,{ ROP_SB|RI_15, ROP_PD|RI_9,  ROP_BD|RI_7,  0   }},/* PDSPoax  */
  { 0x590609L,{ ROP_SB|RI_12, ROP_PD|RI_7,  0,		  0   }},/* DPSnox   */
  { PATINVERT,{ ROP_BD|RI_7,  0,	    0,		  0   }},/* DPx      */
  { 0x5B18A9L,{ ROP_DS|RI_2,  ROP_BP|RI_15, ROP_PD|RI_7,  0   }},/* DPSDonox */
  { 0x5C0649L,{ ROP_DS|RI_7,  ROP_BP|RI_15, ROP_PD|RI_7,  0   }},/* DPSDxox  */
  { 0x5D0E29L,{ ROP_SB|RI_12, ROP_PD|RI_8,  0,		  0   }},/* DPSnoan  */
  { 0x5E1B29L,{ ROP_DS|RI_5,  ROP_BP|RI_15, ROP_PD|RI_7,  0   }},/* DPSDnaox */
  { 0x5F00E9L,{ ROP_BD|RI_8,  0,	    0,		  0   }},/* DPan     */

  { 0x600365L,{ ROP_SD|RI_7,  ROP_BD|RI_9,  0,		  0   }},/* PDSxa    */
  { 0x6116C6L,{ ROP_DS|RI_9,  ROP_BP|RI_15, ROP_SP|RI_7,  ROP_PD|RI_10 }},
								 /* DSPDSaoxxn*/
  { 0x620786L,{ ROP_DB|RI_15, ROP_SP|RI_9,  ROP_PD|RI_7,  0   }},/* DSPDoax  */
  { 0x630608L,{ ROP_BD|RI_12, ROP_SD|RI_7,  0,		  0   }},/* SDPnox   */
  { 0x640788L,{ ROP_SB|RI_15, ROP_PD|RI_9,  ROP_SD|RI_7,  0   }},/* SDPSoax  */
  { 0x650606L,{ ROP_SB|RI_14, ROP_PD|RI_7,  0,		  0   }},/* DSPnox   */
  { SRCINVERT,{ ROP_SD|RI_7,  0,	    0,		  0   }},/* DSx      */
  { 0x6718A8L,{ ROP_SB|RI_2,  ROP_PD|RI_15, ROP_SD|RI_7,  0   }},/* SDPSonox */
  { 0x6858A6L,{ ROP_DS|RI_2,  ROP_BP|RI_15, ROP_SP|RI_7,  ROP_PD|RI_10 }},
								/* DSPDSonoxxn*/
  { 0x690145L,{ ROP_SD|RI_7,  ROP_BD|RI_10, 0,		  0   }},/* PDSxxn   */
  { 0x6A01E9L,{ ROP_SB|RI_9,  ROP_PD|RI_7,  0,		  0   }},/* DPSax    */
  { 0x6B178AL,{ ROP_SB|RI_15, ROP_PD|RI_9,  ROP_SD|RI_7,  ROP_BD|RI_10 }},
								 /* PSDPSoaxxn*/
  { 0x6C01E8L,{ ROP_BD|RI_9,  ROP_SD|RI_7,  0,		  0   }},/* SDPax    */
  { 0x6D1785L,{ ROP_DB|RI_15, ROP_SP|RI_9,  ROP_PD|RI_7,  ROP_BD|RI_10 }},
								 /* PDSPDoaxxn*/
  { 0x6E1E28L,{ ROP_SB|RI_12, ROP_PD|RI_9,  ROP_SD|RI_7,  0   }},/* SDPSnoax */
  { 0x6F0C65L,{ ROP_SD|RI_10, ROP_BD|RI_8,  0,		  0   }},/* PDSxnan  */

  { 0x700CC5L,{ ROP_SD|RI_8,  ROP_BD|RI_9,  0,		  0   }},/* PDSana   */
  { 0x711D5CL,{ ROP_DB|RI_7,  ROP_SD|RI_7,  ROP_PD|RI_9,  ROP_SD|RI_10 }},
								 /* SSDxPDxaxn*/
  { 0x720648L,{ ROP_SB|RI_7,  ROP_PD|RI_15, ROP_SD|RI_7,  0   }},/* SDPSxox  */
  { 0x730E28L,{ ROP_BD|RI_12, ROP_SD|RI_8,  0,		  0   }},/* SDPnoan  */
  { 0x740646L,{ ROP_DB|RI_7,  ROP_SP|RI_15, ROP_PD|RI_7,  0   }},/* DSPDxox  */
  { 0x750E26L,{ ROP_SB|RI_14, ROP_PD|RI_8,  0,		  0   }},/* DSPnoan  */
  { 0x761B28L,{ ROP_SB|RI_3,  ROP_PD|RI_15, ROP_SD|RI_7,  0   }},/* SDPSnaox */
  { 0x7700E6L,{ ROP_SD|RI_8,  0,	    0,		  0   }},/* DSan     */
  { 0x7801E5L,{ ROP_SD|RI_9,  ROP_BD|RI_7,  0,		  0   }},/* PDSax    */
  { 0x791786L,{ ROP_DS|RI_15, ROP_BP|RI_9,  ROP_SP|RI_7,  ROP_PD|RI_10 }},
				 				 /* DSPDSoaxxn*/
  { 0x7A1E29L,{ ROP_DS|RI_14, ROP_BP|RI_9,  ROP_PD|RI_7,  0   }},/* DPSDnoax */
  { 0x7B0C68L,{ ROP_BD|RI_10, ROP_SD|RI_8,  0,		  0   }},/* SDPxnan  */
  { 0x7C1E24L,{ ROP_SD|RI_12, ROP_BD|RI_9,  ROP_SD|RI_7,  0   }},/* SPDSnoax */
  { 0x7D0C69L,{ ROP_SB|RI_10, ROP_PD|RI_8,  0,		  0   }},/* DPSxnan  */
  { 0x7E0955L,{ ROP_SD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_15, 0   }},/* SPxDSxo  */
  { 0x7F03C9L,{ ROP_SB|RI_9,  ROP_PD|RI_8,  0,		  0   }},/* DPSaan   */

  { 0x8003E9L,{ ROP_SB|RI_9,  ROP_PD|RI_9,  0,		  0   }},/* DPSaa    */
  { 0x810975L,{ ROP_SD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_2,  0   }},/* SPxDSxon */
  { 0x820C49L,{ ROP_SB|RI_10, ROP_PD|RI_9,  0,		  0   }},/* DPSxna   */
  { 0x831E04L,{ ROP_SD|RI_12, ROP_BD|RI_9,  ROP_SD|RI_10, 0   }},/* SPDSnoaxn */
  { 0x840C48L,{ ROP_BD|RI_10, ROP_SD|RI_9,  0,		  0   }},/* SDPxna   */
  { 0x851E05L,{ ROP_SB|RI_14, ROP_PD|RI_9,  ROP_BD|RI_10, 0   }},/* PDSPnoaxn */
  { 0x8617A6L,{ ROP_DS|RI_15, ROP_BP|RI_9,  ROP_SP|RI_7,  ROP_PD|RI_7  }},
								 /* DSPDSoaxx */
  { 0x8701C5L,{ ROP_SD|RI_9,  ROP_BD|RI_10, 0,		  0   }},/* PDSaxn   */
  { SRCAND,   { ROP_SD|RI_9,  0,	    0,		  0   }},/* DSa      */
  { 0x891B08L,{ ROP_SB|RI_3,  ROP_PD|RI_15, ROP_SD|RI_10, 0   }},/* SDPSnaoxn */
  { 0x8A0E06L,{ ROP_SB|RI_14, ROP_PD|RI_9,  0,		  0   }},/* DSPnoa   */
  { 0x8B0666L,{ ROP_DB|RI_7,  ROP_SP|RI_15, ROP_PD|RI_10, 0   }},/* DSPDxoxn */
  { 0x8C0E08L,{ ROP_BD|RI_12, ROP_SD|RI_9,  0,		  0   }},/* SDPnoa   */
  { 0x8D0668L,{ ROP_SB|RI_7,  ROP_PD|RI_15, ROP_SD|RI_10, 0   }},/* SDPSxoxn */
  { 0x8E1D7CL,{ ROP_DB|RI_7,  ROP_SD|RI_7,  ROP_PD|RI_9,  ROP_SD|RI_7  }},
								 /* SSDxPDxax */
  { 0x8F0CE5L,{ ROP_SD|RI_8,  ROP_BD|RI_8,  0,		  0   }},/* PDSanan  */

  { 0x900C45L,{ ROP_SD|RI_10, ROP_BD|RI_9,  0,		  0   }},/* PDSxna   */
  { 0x911E08L,{ ROP_SB|RI_12, ROP_PD|RI_9,  ROP_SD|RI_10, 0   }},/* SDPSnoaxn */
  { 0x9217A9L,{ ROP_DB|RI_15, ROP_SP|RI_9,  ROP_BP|RI_7,  ROP_PD|RI_7  }},
								 /* DPSDPoaxx */
  { 0x9301C4L,{ ROP_BD|RI_9,  ROP_SD|RI_10, 0,		  0   }},/* SPDaxn   */
  { 0x9417AAL,{ ROP_SB|RI_15, ROP_PD|RI_9,  ROP_SD|RI_7,  ROP_BD|RI_7  }},
								 /* PSDPSoaxx */
  { 0x9501C9L,{ ROP_SB|RI_9,  ROP_PD|RI_10, 0,		  0   }},/* DPSaxn   */
  { 0x960169L,{ ROP_SB|RI_7,  ROP_PD|RI_7,  0,		  0   }},/* DPSxx    */
  { 0x97588AL,{ ROP_SB|RI_2,  ROP_PD|RI_15, ROP_SD|RI_7,  ROP_BD|RI_7  }},
								 /* PSDPSonoxx*/
  { 0x981888L,{ ROP_SB|RI_2,  ROP_PD|RI_15, ROP_SD|RI_10, 0   }},/* SDPSonoxn */
  { 0x990066L,{ ROP_SD|RI_10, 0,	    0,		  0   }},/* DSxn     */
  { 0x9A0709L,{ ROP_SB|RI_3,  ROP_PD|RI_7,  0,		  0   }},/* DPSnax   */
  { 0x9B07A8L,{ ROP_SB|RI_15, ROP_PD|RI_9,  ROP_SD|RI_10, 0   }},/* SDPSoaxn */
  { 0x9C0704L,{ ROP_BD|RI_5,  ROP_SD|RI_7,  0,		  0   }},/* SPDnax   */
  { 0x9D07A6L,{ ROP_DB|RI_7,  ROP_SP|RI_9,  ROP_PD|RI_10, 0   }},/* DSPDoaxn */
  { 0x9E16E6L,{ ROP_DS|RI_9,  ROP_BP|RI_15, ROP_SP|RI_7,  ROP_PD|RI_7  }},
								 /* DSPDSaoxx */
  { 0x9F0345L,{ ROP_SD|RI_7,  ROP_BD|RI_8,  0,		  0   }},/* PDSxan   */

  { 0xA000C9L,{ ROP_BD|RI_9,  0,	    0,		  0   }},/* DPa      */
  { 0xA11B05L,{ ROP_SB|RI_5,  ROP_PD|RI_15, ROP_BD|RI_10, 0   }},/* PDSPnaoxn */
  { 0xA20E09L,{ ROP_SB|RI_12, ROP_PD|RI_9,  0,		  0   }},/* DPSnoa   */
  { 0xA30669L,{ ROP_DS|RI_7,  ROP_BP|RI_15, ROP_PD|RI_10, 0   }},/* DPSDxoxn */
  { 0xA41885L,{ ROP_SB|RI_2,  ROP_PD|RI_15, ROP_BD|RI_10, 0   }},/* PDSPonoxn */
  { 0xA50065L,{ ROP_BD|RI_10, 0,	    0,		  0   }},/* PDxn     */
  { 0xA60706L,{ ROP_SB|RI_5,  ROP_PD|RI_7,  0,		  0   }},/* DSPnax   */
  { 0xA707A5L,{ ROP_SB|RI_15, ROP_PD|RI_9,  ROP_BD|RI_10, 0   }},/* PDSPoaxn */
  { 0xA803A9L,{ ROP_SB|RI_15, ROP_PD|RI_9,  0,		  0   }},/* DPSoa    */
  { 0xA90189L,{ ROP_SB|RI_15, ROP_PD|RI_10, 0,		  0   }},/* DPSoxn   */
  { 0xAA0029L,{ ROP_FD|RI_11, 0,	    0,		  0   }},/* D        */
  { 0xAB0889L,{ ROP_SB|RI_2,  ROP_PD|RI_15, 0,		  0   }},/* DPSono   */
  { 0xAC0744L,{ ROP_SD|RI_7,  ROP_BD|RI_9,  ROP_SD|RI_7,  0   }},/* SPDSxax  */
  { 0xAD06E9L,{ ROP_DS|RI_9,  ROP_BP|RI_7,  ROP_PD|RI_10, 0   }},/* DPSDaoxn */
  { 0xAE0B06L,{ ROP_SB|RI_5,  ROP_PD|RI_15, 0,		  0   }},/* DSPnao   */
  { 0xAF0229L,{ ROP_BD|RI_12, 0,	    0,		  0   }},/* DPno     */

  { 0xB00E05L,{ ROP_SD|RI_12, ROP_BD|RI_9,  0,		  0   }},/* PDSnoa   */
  { 0xB10665L,{ ROP_SB|RI_7,  ROP_PD|RI_15, ROP_BD|RI_10, 0   }},/* PDSPxoxn */
  { 0xB21974L,{ ROP_SD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_15, ROP_SD|RI_7,  }},
								 /* SSPxDSxox */
  { 0xB30CE8L,{ ROP_BD|RI_8,  ROP_SD|RI_8,  0,		  0   }},/* SDPanan  */
  { 0xB4070AL,{ ROP_SD|RI_5,  ROP_BD|RI_7,  0,		  0   }},/* PSDnax   */
  { 0xB507A9L,{ ROP_DS|RI_15, ROP_BP|RI_9,  ROP_PD|RI_10, 0   }},/* DPSDoaxn */
  { 0xB616E9L,{ ROP_DB|RI_9,  ROP_SP|RI_15, ROP_BP|RI_7,  ROP_PD|RI_7  }},
								 /* DPSDPaoxx */
  { 0xB70348L,{ ROP_BD|RI_7,  ROP_SD|RI_8,  0,		  0   }},/* SDPxan   */
  { 0xB8074AL,{ ROP_BD|RI_7,  ROP_SD|RI_9,  ROP_BD|RI_7,  0   }},/* PSDPxax  */
  { 0xB906E6L,{ ROP_DB|RI_9,  ROP_SP|RI_15, ROP_PD|RI_10, 0   }},/* DSPDaoxn */
  { 0xBA0B09L,{ ROP_SB|RI_3,  ROP_PD|RI_15, 0,		  0   }},/* DPSnao   */
  { MERGEPAINT,{ROP_SD|RI_12, 0,	    0,		  0   }},/* DSno     */
  { 0xBC1CE4L,{ ROP_SD|RI_8,  ROP_BD|RI_9,  ROP_SD|RI_7,  0   }},/* SPDSanax */
  { 0xBD0D7DL,{ ROP_DB|RI_7,  ROP_SD|RI_7,  ROP_PD|RI_8,  0   }},/* SDxPDxan */
  { 0xBE0269L,{ ROP_SB|RI_7,  ROP_PD|RI_15, 0,		  0   }},/* DPSxo    */
  { 0xBF08C9L,{ ROP_SB|RI_8,  ROP_PD|RI_15, 0,		  0   }},/* DPSano   */

  { MERGECOPY,{ ROP_BD|RI_13, ROP_SD|RI_9,  0,		  0   }},/* PSa      */
  { 0xC11B04L,{ ROP_SD|RI_3,  ROP_BD|RI_15, ROP_SD|RI_10, 0   }},/* SPDSnaoxn */
  { 0xC21884L,{ ROP_SD|RI_2,  ROP_BD|RI_15, ROP_SD|RI_10, 0   }},/* SPDSonoxn */
  { 0xC3006AL,{ ROP_BD|RI_13, ROP_SD|RI_10, 0,		  0   }},/* PSxn     */
  { 0xC40E04L,{ ROP_BD|RI_14, ROP_SD|RI_9,  0,		  0   }},/* SPDnoa   */
  { 0xC50664L,{ ROP_SD|RI_7,  ROP_BD|RI_15, ROP_SD|RI_10, 0   }},/* SPDSxoxn */
  { 0xC60708L,{ ROP_BD|RI_3,  ROP_SD|RI_7,  0,		  0   }},/* SDPnax   */
  { 0xC707AAL,{ ROP_BD|RI_15, ROP_SD|RI_9,  ROP_BD|RI_10, 0   }},/* PSDPoaxn */
  { 0xC803A8L,{ ROP_BD|RI_15, ROP_SD|RI_9,  0,		  0   }},/* SDPoa    */
  { 0xC90184L,{ ROP_BD|RI_15, ROP_SD|RI_10, 0,		  0   }},/* SPDoxn   */
  { 0xCA0749L,{ ROP_DS|RI_7,  ROP_BP|RI_9,  ROP_PD|RI_7,  0   }},/* DPSDxax  */
  { 0xCB06E4L,{ ROP_SD|RI_9,  ROP_BD|RI_15, ROP_SD|RI_10, 0   }},/* SPDSaoxn */
  { SRCCOPY,  { ROP_SD|RI_13, 0,	    0,		  0   }},/* S        */
  { 0xCD0888L,{ ROP_BD|RI_2,  ROP_SD|RI_15, 0,		  0   }},/* SDPono   */
  { 0xCE0B08L,{ ROP_BD|RI_3,  ROP_SD|RI_15, 0,		  0   }},/* SDPnao   */
  { 0xCF0224L,{ ROP_BD|RI_13, ROP_SD|RI_14, 0,		  0   }},/* SPno     */

  { 0xD00D0AL,{ ROP_SD|RI_14, ROP_BD|RI_9,  0,		  0   }},/* PSDnoa   */
  { 0xD11066L,{ ROP_BD|RI_7,  ROP_SD|RI_15, ROP_BD|RI_10, 0   }},/* PSDPxoxn */
  { 0xD20705L,{ ROP_SD|RI_3,  ROP_BD|RI_7,  0,		  0   }},/* PDSnax   */
  { 0xD307A4L,{ ROP_SD|RI_15, ROP_BD|RI_9,  ROP_SD|RI_10, 0   }},/* SPDSoaxn */
  { 0xD41D78L,{ ROP_BD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_9,  ROP_SD|RI_7  }},
								 /* SSPxPDxax */
  { 0xD50CE9L,{ ROP_SB|RI_8,  ROP_PD|RI_8,  0,		  0   }},/* DPSanan  */
  { 0xD616EAL,{ ROP_SB|RI_9,  ROP_PD|RI_15, ROP_SD|RI_7,  ROP_BD|RI_7  }},
								 /* PSDPSaoxx */
  { 0xD70349L,{ ROP_SB|RI_7,  ROP_PD|RI_8,  0,		  0   }},/* DPSxan   */
  { 0xD80745L,{ ROP_SB|RI_7,  ROP_PD|RI_9,  ROP_BD|RI_7,  0   }},/* PDSPxax  */
  { 0xD906E8L,{ ROP_SB|RI_9,  ROP_PD|RI_15, ROP_SD|RI_10, 0   }},/* SDPSaoxn */
  { 0xDA1CE9L,{ ROP_DS|RI_8,  ROP_BP|RI_9,  ROP_PD|RI_7,  0   }},/* DPSDanax */
  { 0xDB0D75L,{ ROP_SD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_8,  0   }},/* SPxDSxan */
  { 0xDC0B04L,{ ROP_BD|RI_5,  ROP_SD|RI_15, 0,		  0   }},/* SPDnao   */
  { 0xDD0228L,{ ROP_SD|RI_14, 0,	    0,		  0   }},/* SDno     */
  { 0xDE0268L,{ ROP_BD|RI_7,  ROP_SD|RI_15, 0,		  0   }},/* SDPxo    */
  { 0xDF08C8L,{ ROP_BD|RI_8,  ROP_SD|RI_15, 0,		  0   }},/* SDPano   */

  { 0xE003A5L,{ ROP_SD|RI_15, ROP_BD|RI_9,  0,		  0   }},/* PDSoa    */
  { 0xE10185L,{ ROP_SD|RI_15, ROP_BD|RI_10, 0,		  0   }},/* PDSoxn   */
  { 0xE20746L,{ ROP_DB|RI_7,  ROP_SP|RI_9,  ROP_PD|RI_7,  0   }},/* DSPDxax  */
  { 0xE306EAL,{ ROP_BD|RI_9,  ROP_SD|RI_15, ROP_BD|RI_10, 0   }},/* PSDPaoxn */
  { 0xE40748L,{ ROP_SB|RI_7,  ROP_PD|RI_9,  ROP_SD|RI_7,  0   }},/* SDPSxax  */
  { 0xE506E5L,{ ROP_SB|RI_9,  ROP_PD|RI_15, ROP_BD|RI_10, 0   }},/* PDSPaoxn */
  { 0xE61CE8L,{ ROP_SB|RI_8,  ROP_PD|RI_9,  ROP_SD|RI_7,  0   }},/* SDPSanax */
  { 0xE70D79L,{ ROP_BD|RI_7,  ROP_SB|RI_7,  ROP_PD|RI_8,  0   }},/* SPxPDxan */
  { 0xE81D74L,{ ROP_SB|RI_7,  ROP_SD|RI_7,  ROP_PD|RI_9,  ROP_SD|RI_7  }},
								 /* SSPxDSxax */
  { 0xE95CE6L,{ ROP_DS|RI_8,  ROP_BP|RI_9,  ROP_SP|RI_7,  ROP_PD|RI_10 }},
								/* DSPDSanaxxn*/
  { 0xEA02E9L,{ ROP_SB|RI_9,  ROP_PD|RI_15, 0,		  0   }},/* DPSao    */
  { 0xEB0849L,{ ROP_SB|RI_10, ROP_PD|RI_15, 0,		  0   }},/* DPSxno   */
  { 0xEC02E8L,{ ROP_BD|RI_9,  ROP_SD|RI_15, 0,		  0   }},/* SDPao    */
  { 0xED0848L,{ ROP_BD|RI_10, ROP_SD|RI_15, 0,		  0   }},/* SDPxno   */
  { SRCPAINT, { ROP_SD|RI_15, 0,	    0,		  0   }},/* DSo      */
  { 0xEF0A08L,{ ROP_BD|RI_12, ROP_SD|RI_15, 0,		  0   }},/* SDPnoo   */

  { PATCOPY,  { ROP_BD|RI_13, 0,	    0,		  0   }},/* P        */
  { 0xF10885L,{ ROP_SD|RI_2,  ROP_BD|RI_15, 0,		  0   }},/* PDSono   */
  { 0xF20B05L,{ ROP_SD|RI_3,  ROP_BD|RI_15, 0,		  0   }},/* PDSnao   */
  { 0xF3022AL,{ ROP_BD|RI_13, ROP_SD|RI_12, 0,		  0   }},/* PSno     */
  { 0xF40B0AL,{ ROP_SD|RI_5,  ROP_BD|RI_15, 0,		  0   }},/* PSDnao   */
  { 0xF50225L,{ ROP_BD|RI_14, 0,	    0,		  0   }},/* PDno     */
  { 0xF60265L,{ ROP_SD|RI_7,  ROP_BD|RI_15, 0,		  0   }},/* PDSxo    */
  { 0xF708C5L,{ ROP_SD|RI_8,  ROP_BD|RI_15, 0,		  0   }},/* PDSano   */
  { 0xF802E5L,{ ROP_SD|RI_9,  ROP_BD|RI_15, 0,		  0   }},/* PDSao    */
  { 0xF90845L,{ ROP_SD|RI_10, ROP_BD|RI_15, 0,		  0   }},/* PDSxno   */
  { 0xFA0089L,{ ROP_BD|RI_15, 0,	    0,		  0   }},/* DPo      */
  { PATPAINT, { ROP_SB|RI_12, ROP_PD|RI_15, 0,		  0   }},/* DPSnoo   */
  { 0xFC008AL,{ ROP_BD|RI_13, ROP_SD|RI_15, 0,		  0   }},/* PSo      */
  { 0xFD0A0AL,{ ROP_SD|RI_14, ROP_BD|RI_15, 0,		  0   }},/* PSDnoo   */
  { 0xFE02A9L,{ ROP_SB|RI_15, ROP_PD|RI_15, 0,		  0   }},/* DPSoo    */
  { WHITENESS,{ ROP_FD|RI_16, 0,	    0,		  0   }} /* 1        */
};
