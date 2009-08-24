/* $Id: jfs_rele.c,v 1.1 2000/04/21 10:58:15 ktk Exp $ */

static char *SCCSID = "@(#)1.4  7/30/98 14:19:16 src/jfs/ifs/jfs_rele.c, sysjfs, w45.fs32, 990417.1";
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
 *
 * Module: jfs_rele.c
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
 * NAME:	jfs_rele(vp)
 *
 * FUNCTION:	release a reference on vnode <vp>.
 *
 * PARAMETER:	vp	- The doomed vnode pointer.
 *
 * RETURN:	Returns any errors that occur below this layer.
 *
 * serialization:
 */
jfs_rele(vp)
struct vnode *vp;
{
	struct inode	*ip = VP2IP(vp);
	struct vfs	*vfsp = vp->v_vfsp;

NOISE(1,("jfs_rele: ip:0x%08x\n", ip));

	ICACHE_LOCK();
	assert(vp->v_count == 1);

	iput(ip, vfsp);

	ICACHE_UNLOCK();

	return 0;
}
