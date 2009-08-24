/* $Id: dumpfs.h,v 1.1.1.1 2003/05/21 13:42:48 pasha Exp $ */

/* static char *SCCSID = "@(#)1.2  9/16/96 10:52:51 src/jfs/utils/validfs/dumpfs.h, jfsutil, w45.fs32, 990417.1";*/
/* @(#)dumpfs.h	1.2  @(#)dumpfs.h	1.2 9/13/96 12:04:46  */
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
 *   COMPONENT_NAME: cmdufs
 *
 *   FUNCTIONS: none
 *
  */
#ifndef H_DUMPFS
#define H_DUMPFS

int32 display_dmap(int32, dmap_t *);
int32 display_level(int32, int32, dmapctl_t *);
int32 display_cpage( dbmap_t *);
void print_uint_array(char *, uint32 *, uint32);
#endif	/* H_DUMPFS */
