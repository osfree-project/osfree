
/*  Net.h	2.4
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/
#ifndef Net__h
#define Net__h

#define	WNNC_SPEC_VERSION			0x0001

#define	WNNC_NET_TYPE				0x0002

#define		WNNC_NET_None			0x0000
#define		WNNC_NET_MSNet			0x0100
#define		WNNC_NET_LanMan 		0x0200
#define		WNNC_NET_NetWare		0x0300
#define		WNNC_NET_Vines			0x0400

#define	WNNC_DRIVER_VERSION			0x0003

#define	WNNC_USER				0x0004

#define		WNNC_USR_GetUser		0x0001

#define	WNNC_CONNECTION 			0x0006

#define		WNNC_CON_AddConnection		0x0001
#define		WNNC_CON_CancelConnection	0x0002
#define		WNNC_CON_GetConnections 	0x0004

#define	WNNC_PRINTING				0x0007

#define		WNNC_PRT_OpenJob		0x0002
#define		WNNC_PRT_CloseJob		0x0004
#define		WNNC_PRT_GetQueue		0x0008
#define		WNNC_PRT_HoldJob		0x0010
#define		WNNC_PRT_ReleaseJob		0x0020
#define		WNNC_PRT_CancelJob		0x0040
#define		WNNC_PRT_SetJobCopies		0x0080
#define		WNNC_PRT_WatchQueue		0x0100
#define		WNNC_PRT_UnwatchQueue		0x0200
#define		WNNC_PRT_LockQueueData		0x0400
#define		WNNC_PRT_UnlockQueueData	0x0800
#define		WNNC_PRT_ChangeMsg		0x1000
#define		WNNC_PRT_AbortJob		0x2000
#define		WNNC_PRT_NoArbitraryLock	0x4000
#define		WNNC_PRT_WriteJob		0x8000

#define	WNNC_DEVMODE				0x0008

#define		WNNC_DEV_Devmode		0x0001

#define	WNNC_ERROR				0x000a

#define		WNNC_ERR_GetError		0x0001
#define		WNNC_ERR_GetErrorInfo		0x0002

#define	WNNC_BROWSE				0x000b

#define		WNNC_BRW_BrowseDialog		0x0001

/* prototypes */

WORD WINAPI WNetGetUser(LPSTR, LPWORD);

#endif /* Net__h */
