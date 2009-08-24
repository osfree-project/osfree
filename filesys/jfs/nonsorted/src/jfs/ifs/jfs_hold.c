/* $Id: jfs_hold.c,v 1.1 2000/04/21 10:58:09 ktk Exp $ */

static char *SCCSID = "@(#)1.5  7/30/98 14:15:53 src/jfs/ifs/jfs_hold.c, sysjfs, w45.fs32, 990417.1";
/*
 *
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
 *
 * Module: jfs_hold.c
 *
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_proto.h"
#include "jfs_debug.h"

/*
 * NAME:	jfs_hold(vp)
 *
 * FUNCTION:	acquire a reference on vnode <vp>
 *
 * PARAMETER:	vp	- vnode to hold.
 *
 * RETURN:	always returns 0.
 *
 * serialization:
 *	ICACHE_LOCK serializes v_count vnodes.
 */
jfs_hold(vp)
struct vnode *vp;
{
	struct inode *ip = VP2IP(vp);

NOISE(1,("jfs_hold: ip:0x%08x\n", ip));

	ICACHE_LOCK();
	ip->i_count++;
	ICACHE_UNLOCK();

	return 0;
}
