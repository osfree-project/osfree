/* $Id: initmap.h,v 1.2 2004/03/21 02:43:21 pasha Exp $ */

/* static char *SCCSID = "@(#)1.3  1/8/97 16:51:29 src/jfs/utils/validfs/initmap.h, jfsutil, w45.fs32, 990417.1";*/
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
 *   COMPONENT_NAME: jfsutil
 *
 *   FUNCTIONS:
 *
  */
#ifndef H_INITMAP
#define H_INITMAP

#define BADBLOCK	0x4

int calc_map_size(uint64, uint32, uint32);
void markit(uint64, int32);
int write_block_map(int, uint64, uint32);
int32 compare_maps(int, uint64, uint32);

#endif	/* H_INITMAP */
