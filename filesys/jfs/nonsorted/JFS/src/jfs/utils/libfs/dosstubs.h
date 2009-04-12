/* $Id: dosstubs.h,v 1.1.1.1 2003/05/21 13:41:48 pasha Exp $ */

/* static char *SCCSID = "@(#)1.2  9/4/96 14:58:31 src/jfs/utils/libfs/dosstubs.h, jfslib, w45.fs32, 990417.1";*/
/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   COMPONENT_NAME: cmdufs
 *
 *   FUNCTIONS: APIRET
 *		EAOP2
 *		HFILE
 *		LHANDLE
 *		PEAOP2
 *		PHFILE
 *		PSZ
 *		PULONG
 *		PVOID
 *		SSToDS
 *		ULONG
 *		USHORT
 *
 */
#ifndef H_DOS
#define H_DOS

/* This file declares the Dos routines for building on AIX */

/* Types */

typedef unsigned char * PSZ;
typedef unsigned long APIRET;
typedef unsigned long ULONG;
typedef unsigned long USHORT;
#define LONG long
typedef ULONG * PULONG;
typedef void * PVOID;
typedef unsigned long LHANDLE;
typedef LHANDLE HFILE;
typedef HFILE * PHFILE;
typedef long EAOP2;	/* Not really, but why define a structure we aren't
			   using */
typedef EAOP2 * PEAOP2;

#define SSToDS(x) x
#define APIENTRY16

/* I got these from sysbloks.h */

struct BPB {
	unsigned short	bytes_per_sector;
	unsigned char	sectors_per_cluster;
	unsigned short	reserved_sectors;
	unsigned char	nbr_fats;
	unsigned short	root_entries;
	unsigned short	total_sectors;
	char		media_type;
	unsigned short	sectors_per_fat;
	unsigned short	sectors_per_track;
	unsigned short	number_of_heads;
	unsigned long	hidden_sectors;
	unsigned long	large_total_sectors;
	char		reserved_3[6];
};

struct DPB {
	struct BPB	dev_bpb;
	unsigned short	number_of_tracks;
	char		device_type;
	unsigned short	device_attributes;
};

#define DT_HARD_DISK	5

/* Dos* functions	*/

APIRET	DosOpen(PSZ pszFileName, PHFILE phf, PULONG pulAction, ULONG cbFile,
		ULONG ulAttribute, ULONG fsOpenFlags, ULONG fsOpenMode,
		PEAOP2 peaop2);
APIRET	DosClose(HFILE hFile);
APIRET	DosSetFilePtr(HFILE hfile, LONG ib, ULONG method, PULONG ibActual);
APIRET	DosRead(HFILE hFile, PVOID pBuffer, ULONG cbRead, PULONG pcbActual);
APIRET	DosWrite(HFILE hFile, PVOID pBuffer, ULONG cbWrite, PULONG pcbActual);
APIRET	DosDevIOCtl(HFILE hDev, ULONG Category, ULONG Funtion, PVOID pParams,
		    ULONG ParmLenMax, PULONG pParmLen, PVOID pData,
		    ULONG DataLenMax, PULONG pDataLen);
/* APIRET	DosDevIOCtl(PVOID pData, PVOID pParms, USHORT usFun, USHORT usCategory,
		    HFILE hDev);
*/

/* DosOpen() attributes */

#define FILE_NORMAL	0x0000
#define FILE_READONLY	0x0001
#define FILE_HIDDEN	0x0002
#define FILE_SYSTEM	0x0004
#define FILE_DIRECTORY	0x0010
#define FILE_ARCHIVED	0x0020

/* DosOpen() actions */

#define FILE_EXISTED	0x0001
#define FILE_CREATED	0x0002
#define FILE_TRUNCATED	0x0003

/* DosOpen() open flags */

#define FILE_OPEN	0x0001
#define FILE_TRUNCATE	0x0002
#define FILE_CREATE	0x0010

/*	this nibble applies if file already exists		  xxxx */

#define OPEN_ACTION_FAIL_IF_EXISTS	0x0000	/* ---- ---- ---- 0000 */
#define OPEN_ACTION_OPEN_IF_EXISTS	0x0001	/* ---- ---- ---- 0001 */
#define OPEN_ACTION_REPLACE_IF_EXISTS	0x0002	/* ---- ---- ---- 0010 */

/*	this nibble applies if file does not exist	     xxxx      */

#define OPEN_ACTION_FAIL_IF_NEW		0x0000	/* ---- ---- 0000 ---- */
#define OPEN_ACTION_CREATE_IF_NEW	0x0010	/* ---- ---- 0001 ---- */

/* DosOpen/DosSetFHandState flags */

#define OPEN_ACCESS_READONLY		0x0000	/* ---- ---- ---- -000 */
#define OPEN_ACCESS_WRITEONLY		0x0001	/* ---- ---- ---- -001 */
#define OPEN_ACCESS_READWRITE		0x0002	/* ---- ---- ---- -010 */
#define OPEN_SHARE_DENYREADWRITE	0x0010	/* ---- ---- -001 ---- */
#define OPEN_SHARE_DENYWRITE		0x0020	/* ---- ---- -010 ---- */
#define OPEN_SHARE_DENYREAD		0x0030	/* ---- ---- -011 ---- */
#define OPEN_SHARE_DENYNONE		0x0040	/* ---- ---- -100 ---- */
#define OPEN_FLAGS_NOINHERIT		0x0080	/* ---- ---- 1--- ---- */
#define OPEN_FLAGS_NO_LOCALITY		0x0000	/* ---- -000 ---- ---- */
#define OPEN_FLAGS_SEQUENTIAL		0x0100	/* ---- -001 ---- ---- */
#define OPEN_FLAGS_RANDOM		0x0200	/* ---- -010 ---- ---- */
#define OPEN_FLAGS_RANDOMSEQUENTIAL	0x0300	/* ---- -011 ---- ---- */
#define OPEN_FLAGS_NO_CACHE		0x1000	/* ---1 ---- ---- ---- */
#define OPEN_FLAGS_FAIL_ON_ERROR	0x2000	/* --1- ---- ---- ---- */
#define OPEN_FLAGS_WRITE_THROUGH	0x4000	/* -1-- ---- ---- ---- */
#define OPEN_FLAGS_DASD			0x8000	/* 1--- ---- ---- ---- */
#define OPEN_FLAGS_NONSPOOLED		0x00040000
#define OPEN_FLAGS_PROTECTED_HANDLE	0x40000000

/* DosSetFilePtr() file position codes */

#define FILE_BEGIN	0x0000	/* Move relative to beginning of file */
#define FILE_CURRENT	0x0001	/* Move relative to current fptr position */
#define FILE_END	0x0002	/* Move relative to end of file */


/* DosDevIOCtl defines */

#define IOCTL_DISK		0x0008
#define DSK_LOCKDRIVE		0x0000
#define DSK_UNLOCKDRIVE		0x0001
#define DSK_REDETERMINEMEDIA	0x0002
#define DSK_SETLOGICALMAP	0x0003
#define DSK_BEGINFORMAT		0X0004
#define DSK_BLOCKREMOVABLE	0x0020
#define DSK_GETLOGICALMAP	0x0021
#define DSK_UNLOCKEJECTMEDIA	0X0040
#define DSK_SETDEVICEPARAMS	0x0043
#define DSK_WRITETRACK		0x0044
#define DSK_FORMATVERIFY	0x0045
#define DSK_DISKETTECONTROL	0X005D
#define DSK_QUERYMEDIASENSE	0X0060
#define DSK_GETDEVICEPARAMS	0x0063
#define DSK_READTRACK		0x0064
#define DSK_VERIFYTRACK		0x0065
#define DSK_GETLOCKSTATUS	0X0066

/* Error codes that we care about */
#define NO_ERROR		0
#define ERROR_INVALID_FUNCTION	1
#define ERROR_FILE_NOT_FOUND	2
#define ERROR_INVALID_HANDLE	6
#define ERROR_INVALID_ACCESS	12
#define ERROR_SEEK		25
#define ERROR_WRITE_FAULT	29
#define ERROR_READ_FAULT	30
#define ERROR_GEN_FAILURE	31

#endif /* H_DOS */
