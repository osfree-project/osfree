/* $Id: initmap.h,v 1.1.1.1 2003/05/21 13:41:30 pasha Exp $ */

/* static char *SCCSID = "@(#)1.8  9/12/98 12:54:27 src/jfs/utils/format/initmap.h, jfsformat, w45.fs32, 990417.1";*/
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
 *   MODULE_NAME:		initmap.h
 *
 *   COMPONENT_NAME: 	jfsformat
 *
 *   FUNCTIONS: 	none
 *
*/

#ifndef H_INITMAP
#define H_INITMAP

#define ALLOC		0x1
#define FREE		0x2
#define BADBLOCK	0x4

int32 calc_map_size(int64, struct dinode *, int32, int32 *, uint32);
int32 markit(int32, uint32);
int32 record_LVM_BadBlks( int32, int32, int32, struct dinode *, int64 );
int32 verify_last_blocks( int32, int32, struct dinode * );
int32 write_block_map(int32, int64, int32);

#endif	/* H_INITMAP */
