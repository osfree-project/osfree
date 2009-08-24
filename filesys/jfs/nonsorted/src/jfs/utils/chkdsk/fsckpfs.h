/* $Id: fsckpfs.h,v 1.2 2004/03/21 02:43:26 pasha Exp $ */

/* static char *SCCSID = "@(#)1.6  7/30/97 10:54:30 src/jfs/utils/chkdsk/fsckpfs.h, jfschk, w45.fs32, 990417.1";*/
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
 *   COMPONENT_NAME: jfschk
 *
 *   FUNCTIONS: none
 *
*/ 
#ifndef H_FSCKPFS
#define H_FSCKPFS

#define fsck_READ  1
#define fsck_WRITE 2

/*
 * the following structures are needed for CHKDSK.SYS
 * which is used to obtain high memory in the autocheck
 * environment.
 */

struct parm_s {
  ULONG size;
  };

struct data_s {
  ULONG rc;
  ULONG addr;
  ULONG cb;
  };

#endif









