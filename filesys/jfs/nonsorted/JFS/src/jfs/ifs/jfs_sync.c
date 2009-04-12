/* $Id: jfs_sync.c,v 1.1.1.1 2003/05/21 13:38:16 pasha Exp $ */

static char *SCCSID = "@(#)1.10  3/12/99 10:49:37 src/jfs/ifs/jfs_sync.c, sysjfs, w45.fs32, 990417.1";
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
 * Module: jfs_sync.c
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */

#include  "jfs_types.h"
#include  "jfs_filsys.h"
#include  "jfs_inode.h"
#include  "jfs_logmgr.h"
#include  "jfs_debug.h"


/*
 * external references
 */
int	iSyncFS(struct vfs *);		/* inode sync routine */
void	lmSync(log_t *);		/* log sync routine */

/*
 * NAME:	jfs_sync(vfsp)
 *
 * FUNCTION:	commit all regular files in the vfs which have not been
 *		committed since the last time jfs_sync() was invoked. 
 *
 *		initiates i/o for all modified journalled pages which 
 *		can be written to their home address or marks those which 
 *		can not so that they will be written when they are committed. 
 *		a new logsync value is computed for the log.
 *
 * PARAMETER:	none
 *
 * RETURN:	always 0
 */
int32
jfs_sync(struct vfs *vfsp)
{
	inode_t	*ipmnt;

	if (vfsp->vfs_flag & VFS_READONLY)
		return 0;

	ipmnt = ((inode_t *)vfsp->vfs_data)->i_ipmnt;

	if (!(WRITE_LOCK_TRY(&ipmnt->i_rdwrlock)))
		return 0;

	/* Avoid deadlock with hard quiesce */
	if (ipmnt->i_cachedev->cd_flag & CD_QUIESCE)
	{
		WRITE_UNLOCK(&ipmnt->i_rdwrlock);
		return 0;
	}
	/* commit all regular files in this vfs */
	iSyncFS(vfsp);

	/* sync the log associated with the vfs */
	lmSync((log_t *)(((inode_t *)vfsp->vfs_data)->i_ipmnt->i_iplog));

	WRITE_UNLOCK(&ipmnt->i_rdwrlock);

	return 0;
}
