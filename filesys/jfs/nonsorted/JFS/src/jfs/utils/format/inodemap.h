/* $Id: inodemap.h,v 1.1.1.1 2003/05/21 13:41:32 pasha Exp $ */

/* static char *SCCSID = "@(#)1.3  7/10/98 15:21:20 src/jfs/utils/format/inodemap.h, jfsformat, w45.fs32, 990417.1";*/
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
 *   MODULE_NAME:		inodemap.h
 *
 *   COMPONENT_NAME: 	jfsformat
 *
 *   FUNCTIONS: 	none
 *
 */

#ifndef H_INODEMAP
#define H_INODEMAP

int32 init_inode_map(int32, HFILE, int64, int32, int64, int32, uint16, int32,
	uint32);
#endif
