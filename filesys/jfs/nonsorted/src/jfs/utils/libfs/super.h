/* $Id: super.h,v 1.2 2004/03/21 02:43:17 pasha Exp $ */

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
 */
#ifndef H_SUPER
#define	H_SUPER

#include "utilsubs.h"

int32 ujfs_validate_super(struct superblock *);
int32 ujfs_put_superblk(HFILE, struct superblock *, int16);
int32 ujfs_init_superblock(struct superblock *, HFILE, char *, int64, uint32,
		int32, int32, uint32, int64, int32, int32, int64, int32, int32,
		int64, int32, int64, int32 );
int32 ujfs_get_superblk(HFILE, struct superblock *, int32);
int32 inrange(uint32, uint32, uint32);

#endif
