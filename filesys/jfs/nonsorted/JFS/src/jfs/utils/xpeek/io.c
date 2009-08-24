/* $Id: io.c,v 1.1.1.1 2003/05/21 13:43:10 pasha Exp $ */

static char *SCCSID = "@(#)1.5  7/31/98 08:22:40 src/jfs/utils/xpeek/io.c, jfsutil, w45.fs32, 990417.1";
/*
 *
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
*/
/*
	io.c - I/O routines
*/

#include "xpeek.h"

/* libfs includes */
#include <devices.h>

/* This is a really ugly front-end to ujfs_rw_diskblocks.  I don't like the
fact that data read must start at the beginning of the block.  I assume that
this interface will change anyway
*/

int32 xRead(
int64	address,
uint32	count,
char	*buffer)
{
	int64	block_address;
	char *	block_buffer;
	int64	length;
	uint32	offset;

	offset = address & (bsize-1);
	length = (offset + count + bsize-1) & ~(bsize - 1);

	if ((offset == 0) & (length == count))
		return ujfs_rw_diskblocks(fd, address, count, buffer, GET);

	block_address = address - offset;
	block_buffer = (char *)malloc(length);
	if (block_buffer == 0)
		return 1;

	if (ujfs_rw_diskblocks(fd, block_address, length, block_buffer,
	    GET)) {
		free(block_buffer);
		return 1;
	}
	memcpy(buffer, block_buffer+offset, count);
	free(block_buffer);
	return 0;
}

int32 xWrite(
int64	address,
uint32	count,
char	*buffer)
{
	int64	block_address;
	char *	block_buffer;
	int64	length;
	uint32	offset;

	offset = address & (bsize-1);
	length = (offset + count + bsize-1) & ~(bsize - 1);

	if ((offset == 0) & (length == count))
		return ujfs_rw_diskblocks(fd, address, count, buffer, PUT);

	block_address = address - offset;
	block_buffer = (char *)malloc(length);
	if (block_buffer == 0)
		return 1;

	if (ujfs_rw_diskblocks(fd, block_address, length, block_buffer,
	    GET)) {
		free(block_buffer);
		return 1;
	}
	memcpy(block_buffer+offset, buffer, count);
	if (ujfs_rw_diskblocks(fd, block_address, length, block_buffer,
	    PUT)) {
		free(block_buffer);
		return 1;
	}
	free(block_buffer);
	return 0;
}
