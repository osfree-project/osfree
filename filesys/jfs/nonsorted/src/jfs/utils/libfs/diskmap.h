/* $Id: diskmap.h,v 1.2 2004/03/21 02:43:17 pasha Exp $ */

/* static char *SCCSID = "@(#)1.2  10/4/96 10:14:56 src/jfs/utils/libfs/diskmap.h, jfslib, w45.fs32, 990417.1";*/
/* @(#)diskmap.h	1.2  @(#)diskmap.h	1.2 9/27/96 15:31:08  */
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
 *   FUNCTIONS: none
 *
 */
#ifndef H_DISKMAP
#define H_DISKMAP

int8 ujfs_maxbuddy(char *);
int8 ujfs_adjtree(int8 *, int32, int32);
void ujfs_complete_dmap(dmap_t *, int64, int8 *);
void ujfs_idmap_page(dmap_t *, uint32);
int32 ujfs_getagl2size(int64, int32);

#endif	/* H_DISKMAP */
