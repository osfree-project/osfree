/* $Id: jfs_debug.c,v 1.1.1.1 2003/05/21 13:36:32 pasha Exp $ */

static char *SCCSID = "@(#)1.7  10/9/98 07:46:23 src/jfs/ifs/jfs_debug.c, sysjfs, w45.fs32, 990417.1";
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
 *
 *
*/
/*
 *	jfs_debug.c: debug utilities
 */
#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif

#include "jfs_types.h"
#include "jfs_debug.h"
#include "jfs_inode.h"
#include "jfs_bufmgr.h"
#include "jfs_superblock.h"
#include "jfs_filsys.h"

extern struct vfs *vfs_anchor;
int32	readSuper(inode_t *, cbuf_t **);

#ifdef	_JFS_DEBUG
int32	jfsFYI = 1;	/* global enable FYI message */
int32	jfsALERT = 1;	/* global enable ALERT message */
int32	jfsERROR = 1;	/* global enable ERROR message */

#endif /* _JFS_DEBUG */

/* Marks all JFS filesystems dirty */
void	all_dirty()
{
	cbuf_t			*bp;
	inode_t			*ipmnt;
	struct vfs		*vfsp;
	int32			rc;
	struct superblock	*sb;

	for (vfsp = vfs_anchor; vfsp; vfsp = vfsp->vfs_next)
	{
		ipmnt = ((inode_t *)vfsp->vfs_data)->i_ipmnt;
		if (rc = readSuper(ipmnt, &bp))
			return;
		sb = (struct superblock *)(bp->b_bdata);
		sb->s_state = FM_DIRTY;
		(void) rawWrite(ipmnt, bp, 1);
	}
}
