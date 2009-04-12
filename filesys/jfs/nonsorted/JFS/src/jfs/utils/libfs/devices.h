/* $Id: devices.h,v 1.1.1.1 2003/05/21 13:41:44 pasha Exp $ */

/* static char *SCCSID = "@(#)1.11  2/26/99 07:58:03 src/jfs/utils/libfs/devices.h, jfslib, w45.fs32, 990417.1";*/
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
 *   MODULE_NAME:		devices.h
 *
 *   COMPONENT_NAME: 	jfslib
 *
 *   FUNCTIONS: none
 *
*/
#ifndef H_DEVICES
#define H_DEVICES

#define GET	0
#define PUT	1
#define VRFY	2

/* Macros used for determining open mode */
#define READONLY	0
#define RDWR_EXCL	1

int32 ujfs_get_dev_size( HFILE, int64 *, int64 *);
int32 ujfs_open_device( char *, PHFILE, int32 *, int32);
int32 ujfs_beginformat(HFILE);
int32 ujfs_redeterminemedia(HFILE);
int32 ujfs_close( HFILE);
int32 ujfs_rw_diskblocks(HFILE, int64, int32, void *, int32);
int32 ujfs_stdout_redirected(void);
int32 ujfs_update_mbr(HFILE);
int32 ujfs_check_adapter(void);
int32 ujfs_verify_device_type( HFILE );

#endif /* H_DEVICES */
