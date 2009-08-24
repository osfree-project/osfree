/* $Id: dosstubs.c,v 1.1.1.1 2003/05/21 13:41:46 pasha Exp $ */

static char *SCCSID = "@(#)1.3  10/22/96 14:49:06 src/jfs/utils/libfs/dosstubs.c, jfslib, w45.fs32, 990417.1";
/* @(#)dosstubs.c	1.3  @(#)dosstubs.c	1.3 10/21/96 14:14:39 */
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
 *   FUNCTIONS: DosClose
 *		DosDevIOCtl
 *		DosOpen
 *		DosRead
 *		DosSetFilePtr
 *		DosWrite
 *
 */
#include "dosstubs.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>

/*
 * Stub for DosOpen
 */

APIRET DosOpen(
PSZ	FileName,
PHFILE	FileHandle,
PULONG	Action,
ULONG	FileSize,
ULONG	Attribute,
ULONG	OpenFlags,
ULONG	OpenMode,
PEAOP2	peaop2)
{
	/* Real simple now, because we know where it's being called */

	if( OpenMode & OPEN_ACCESS_READWRITE ) {
		*FileHandle = open(FileName, O_RDWR, 0);
	} else {
		*FileHandle = open(FileName, O_RDONLY, 0);
	}

	/* We could get fancy and map errno to a DOS error, but let's not */
	if (*FileHandle == -1)
		return ERROR_FILE_NOT_FOUND;
	return NO_ERROR;
}

APIRET DosClose(
HFILE	FileName)
{
	return close(FileName);
}

APIRET DosSetFilePtr(
HFILE	FileHandle,
LONG	ib,
ULONG	method,
PULONG	ibActual)
{
	offset_t	whence;

	switch (method)
	{
	case FILE_BEGIN:
		whence = SEEK_SET;
		break;
	case FILE_CURRENT:
		whence = SEEK_CUR;
		break;
	case FILE_END:
		whence = SEEK_END;
		break;
	default:
		fprintf(stderr, "DosSetFilePtr: invalid method %d\n", method);
		return ERROR_INVALID_FUNCTION;
	}
	*ibActual = lseek(FileHandle, ib, whence);
	if ((long)(*ibActual) == -1)
		return ERROR_SEEK;

	return NO_ERROR;
}

APIRET DosRead(
HFILE	FileHandle,
PVOID	Buffer,
ULONG	BytesToRead,
PULONG	BytesRead)
{
	*BytesRead = read(FileHandle, Buffer, BytesToRead);
	if ((long)(*BytesRead) == -1)
		return ERROR_READ_FAULT;
	return NO_ERROR;
}

APIRET DosWrite(
HFILE	FileHandle,
PVOID	Buffer,
ULONG	BytesToWrite,
PULONG	BytesWritten)
{
	*BytesWritten = write(FileHandle, Buffer, BytesToWrite);
	if ((long)(*BytesWritten) == -1)
		return ERROR_WRITE_FAULT;
	return NO_ERROR;
}

/* Note:  This is the 16-bit interface.  The 32-bit interface is different */
APIRET DosDevIOCtl(
HFILE	Device,
ULONG	Category,
ULONG	Function,
PVOID	Parms,
ULONG	ParmLenMax,
PULONG	pParmLen,
PVOID	Data,
ULONG	DataLenMax,
PULONG	pDataLen)
{
	struct DPB	*dpb;
	struct devinfo	devbuf;

	if (Category != IOCTL_DISK)
		return ERROR_INVALID_FUNCTION;

	if (Function == DSK_GETDEVICEPARAMS) {
		if (ioctl(Device, IOCINFO, &devbuf) < 0)
			return ERROR_GEN_FAILURE;
		dpb = (struct DPB *) Data;
		dpb->dev_bpb.bytes_per_sector = devbuf.un.dk.bytpsec;
		dpb->dev_bpb.reserved_sectors = 0;
		dpb->dev_bpb.nbr_fats = 0;
		dpb->dev_bpb.root_entries = 0;
		dpb->dev_bpb.total_sectors = 0;
		dpb->dev_bpb.media_type = DT_HARD_DISK;
		dpb->dev_bpb.sectors_per_track = devbuf.un.dk.secptrk;
		dpb->dev_bpb.number_of_heads = devbuf.un.dk.trkpcyl;
		dpb->dev_bpb.hidden_sectors = 0;
		dpb->dev_bpb.large_total_sectors = devbuf.un.dk.numblks;
	
		return NO_ERROR;
	}
	else
		return ERROR_INVALID_FUNCTION;
}
