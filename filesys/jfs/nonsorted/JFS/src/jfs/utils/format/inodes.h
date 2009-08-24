/* $Id: inodes.h,v 1.1.1.1 2003/05/21 13:41:34 pasha Exp $ */

/* static char *SCCSID = "@(#)1.8  7/10/98 15:22:24 src/jfs/utils/format/inodes.h, jfsformat, w45.fs32, 990417.1";*/
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
 *   MODULE_NAME:		inodes.h
 *
 *   COMPONENT_NAME: 	jfsformat
 *
 *   FUNCTIONS: 	none
 *
 */
#ifndef H_INODES
#define H_INODES

typedef enum { inline_data, extent_data, max_extent_data,
		no_data } ino_data_type;

int32 init_aggr_inode_table(int32, HFILE, struct dinode *, int32, int64, int64,
		int32, uint32);
int32 init_fileset_inode_table(int32, HFILE, int64 *, int32 *, int64, int64,
	int32);
int32 init_fileset_inodes(int32, HFILE, int64, int32, int64, int32);
void init_inode(struct dinode *, int32, ino_t, int64, int64, int64, mode_t,
		ino_data_type, int64, int32, int32);

#endif
