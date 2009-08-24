/* $Id: inode.h,v 1.1.1.1 2003/05/21 13:41:50 pasha Exp $ */

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
#ifndef H_INODE
#define H_INODE

int32 ujfs_rwinode(HFILE, struct dinode *, ino_t, int32, int32, ino_t);
int32 ujfs_rwdaddr(HFILE, int64 *, struct dinode *, int64, int32, int32);

#endif
