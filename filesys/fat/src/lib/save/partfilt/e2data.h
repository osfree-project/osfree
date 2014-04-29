/*
 * $Header$
 */

/************************************************************************/
/*                       Linux partition filter.                        */
/*          (C) Copyright Deon van der Westhuysen, July 1995.           */
/*                                                                      */
/*  Dedicated to Jesus Christ, my Lord and Saviour.                     */
/*                                                                      */
/* This program is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 2, or (at your option)  */
/* any later version.                                                   */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this program; if not, write to the Free Software          */
/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            */
/*                                                                      */
/*  This code is still under development; expect some rough edges.      */
/*                                                                      */
/************************************************************************/

#ifndef _E2DATA_H_
#define _E2DATA_H_

#include "os2head.h"

#define FILTER_ADD_NAME		"LinuxPartitions"	/* Max 15 chars. */

#define MAX_LINUX_PARTITIONS	26 	/* Max number of virtual partitions */

#define PARTITION_LINUX		0x83	 /* Type of partition to convert */
#define PARTITION_DOSSMALL	0x01	 /* Dos partition <16MB */
#define PARTITION_DOSMED	0x04	 /* Dos partition >16MB <32MB */
#define PARTITION_DOSLARGE	0x06	 /* Dos partition >32MB */
#define PARTITION_IFS		0x07	 /* Type of virtual partitions */
#define PARTITION_EXTENDED	0x05	 /* Type for extended partition */
#define PARTITION_FAT32    0x0B   /* FAT32 Partition */   
#define PARTITION_EXTENDED_INT13 0x0F /* LBA Mode extended partition */
#define PARTITION_HIDDEN	0x10	  /* Hidden primary partition */
#define PARTITION_BOOTMANAGER 0x0A /* Bootmanager */

#define LINUX_VIRTUAL_SECS	1	/* Override boot sector */

#define MAX_DEVTABLE_SIZE	1024	/* Maximum size of device table */
#define SECTOR_SIZE		512	/* Number of bytes in a sector */

#define	F_SERVER_ACTIVE		1	/* Flag: IORB queue being served */
#define F_REQUEST_BUSY		2	/* Flag: Wait for request to finish */
					/* (Unit record is being used) */
typedef struct
{
 PIORB	pHead;				/* Head of queue of IORBs */
 PIORB	pTail; 				/* Tail of queue of IORBs */
 int	Flags;				/* Flags for this queue */
} TIORBQueue;				/* Structure to queue IORBs */
typedef TIORBQueue *NPIORBQueue;

#define	F_ALLOCATED		1	/* Flag: unit is allocated */
#define F_ALLOW_WRITE		2	/* Flag: unit is writeable */

typedef struct
{
 int		Flags;			/* Flags for unit (Alloc, Write) */
 TIORBQueue	IORBQueue;		/* Queue to store waiting IORBs */
} TRecHeader;				/* Header common to all unit records */
typedef TRecHeader	*NPRecHeader;	/* Pointer type for record */


/* The 'base unit' is the source of the data used to construct the virtual */
/* units from. One virtual unit is constructed for each virtual partition. */
typedef struct
{
 TRecHeader	Hdr;			/* Header for the unit record */
 USHORT		UnitHandle;		/* Original UnitHandle for unit */
 PADDEntryPoint	pADDEntry;		/* Original ADD entry point */
 ADAPTERINFO	AdapterInfo;		/* Adapter information for unit */
					/* Also contains modified UnitInfo. */
 USHORT		GeoNumHeads;		/* Number of heads for this unit */
 USHORT		GeoTrackSec;		/* Number of sectors per track */
 /* Fields that is saved to later restore the current IORB... */
 USHORT		SaveReqCtrl;		/* Saved request control */
 USHORT		SaveReserved;		/* Saved DM reserved field */
 PNotifyAddr	SaveNotify;		/* Saved Notification address */
} TBaseUnitRec;				/* Unit record for base units */
typedef TBaseUnitRec	*NPBaseUnitRec;	/* Pointe type for record */

typedef struct
{
 TRecHeader	Hdr;			/* Header for the unit record */
 NPBaseUnitRec	pSourceUnitRec;		/* Pointer to base unit record */
					/* This is also the UnitHandle */
 PUNITINFO	pUnitInfo;		/* Pointer to modified UnitInfo */
 USHORT		UnitInfoLen;		/* Length of the unit information */
 UCHAR		PartSysIndicator;	/* System indicator in part table */
 UCHAR		FSType;			/* What type of FS for this unit */
 ULONG		StartRBA;		/* Start RBA of partiton source */
 ULONG		NumSectors;		/* Number of sectors in partition */
 ULONG		NumVirtualSectors;	/* Number of sectors to virtualize */
 ULONG		NumExtraSectors;	/* Number of sectors we add to part */
 /* Fields that is saved to later restore the current IORB... */
 USHORT		SaveReqCtrl;		/* Saved request control */
 USHORT		SaveReserved;		/* Saved DM reserved field */
 PNotifyAddr	SaveNotify;		/* Saved Notification address */
 USHORT		SavecSGList;		/* Saved cSGList field */
 SCATGATENTRY	SaveSGEntry;		/* Saved copy of modified SG entry */
 PSCATGATENTRY	SavepSGList;		/* Saved pSGList field */
 ULONG		SaveppSGList;		/* Saved ppSGList field */
 ULONG		SaveRBA;		/* Saved RBA field */
 USHORT		SaveBlockCount;		/* Saved BlockCount field */
 /* Fields to save informtion about virtual sectors allready transferred */
 USHORT		SectorsDone;		/* Number virtual sectors transfered */
 ULONG		SGOffset;		/* Offset to add to SG list */
 /* Field to restore the hidden sector field in a FAT bootsector. */
 ULONG		HiddenSectors;		/* Original number of hidden sectors */
} TVirtUnitRec;				/* Unit record for virtual units */
typedef TVirtUnitRec	*NPVirtUnitRec;	/* Pointe type for record */

#define FI_QUIET	1
#define FI_VERBOSE	2
#define FI_ALLPART	4
#define FI_ALLOWWRITE	8

#define FS_UNKNOWN	0		/* FS type not determined */
#define FS_DOSFAT	1		/* FS is FAT */
#define FS_EXT2		2		/* FS is ext2 linux filesystem */

/* External references to data in e2data.h. */

extern int		   InstallFlags;
extern PFN		   DevHelp;
extern USHORT		ADDHandle;
extern void far *	pDataSeg;
extern ULONG		ppDataSeg;
extern TBaseUnitRec	BaseUnits[MAX_LINUX_PARTITIONS];
extern int		   NumBaseUnits;
extern TVirtUnitRec	VirtUnits[MAX_LINUX_PARTITIONS];
extern int		   NumVirtUnits;
extern USHORT	   MountTable[MAX_LINUX_PARTITIONS];
extern int		   MountCount;
extern BYTE       rgfFakePart[256];
#endif
