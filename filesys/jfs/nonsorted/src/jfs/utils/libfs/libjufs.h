/* $Id: libjufs.h,v 1.2 2004/03/21 02:43:18 pasha Exp $ */

/* static char *SCCSID = "@(#)1.1  7/30/96 15:57:58 src/jfs/utils/libfs/libjufs.h, jfslib, w45.fs32, 990417.1";*/
/* @(#)libjufs.h	1.3  @(#)libjufs.h	1.3 5/15/95 13:20:52 */
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
 * FUNCTIONS:
 *
 */
#ifndef H_LIBFS
#define H_LIBFS

#define LIBFS_BADMAGIC		-5	/* magic number not recognized	    */
#define LIBFS_BADVERSION	-6	/* magic num ok, incompatible vers  */
#define LIBFS_CORRUPTSUPER	-10  	/* fragsize, agsize, or iagsize bad */

#endif /* H_LIBFS */
