/* $Id: jfs_chkdsk.c,v 1.1.1.1 2003/05/21 13:36:26 pasha Exp $ */

static char *SCCSID = "@(#)1.2  3/12/99 10:46:57 src/jfs/ifs/jfs_chkdsk.c, sysjfs, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		jfs_chkdsk.c
 *
 *   COMPONENT_NAME: 	sysjfs
 *
 *   FUNCTIONS:		jfs_chkdsk
 *
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include <string.h>
#include "jfs_os2.h"

#include "jfs_inode.h"
#include "jfs_debug.h"
#include "jfs_proto.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
/*
#include "jfs_types.h"
#include "jfs_lock.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_txnmgr.h"
#include "jfs_ea.h"
*/
#include "jfs_chkdsk.h"

extern struct vfs *vfs_anchor;     /* used to find log VPB      */

/****************************************************************************
 * NAME:        jfs_chkdsk()
 *
 * FUNCTION:    synch a mounted file system
 *
 */
int32 jfs_chkdsk( char	*pData,		/* ptr to plist buffer */ 
		    uint32	lenData,	/* length of buffer */
		    uint16	*pbufsize)	/* ptr to buffer length */ 
{
	int32	rc = 0;
	chkdsk_t	pList, *p = &pList;
	uint32	xtype;
	inode_t	*ipmnt, *ipimap, *ipbmap;
	struct vfs *vfsp;

	if (rc = KernCopyIn(&pList, pData, lenData))  {
jFYI(1,("\njfs_chkdsk: KernCopyIn(&pList, pData, lenData) returned rc = %d\n", rc));
		return rc;
		}

	/* 
	 * get the 'mount' inode 
	 */
        for (vfsp = vfs_anchor; vfsp != NULL; vfsp = vfsp->vfs_next)  {
jFYI(1,("jfs_chkdsk: vpi_drive=     pList.dev=\n",vfsp->vfs_vpfsi->vpi_drive,pList.dev));
		if (vfsp->vfs_vpfsi->vpi_drive == pList.dev)  {
			break;
			}
		}
	if (vfsp == NULL)  {
		return EINVAL;
		}

	xtype = pList.flag;

	/* - - - - - - - - - - - - - - - - - - - - -
	 *
	 * synchronization request?   
	 */
	if (xtype & CHKDSK_SYNC) {

jFYI(1,("\njfs_chkdsk: entering for CHKDSK_SYNC\n"));

		if ((vfsp->vfs_flag & VFS_READONLY) ||
		    (vfsp->vfs_flag & VFS_ACCEPT))
			return 0;

		ipimap = (inode_t *)vfsp->vfs_data;
		ipmnt = ipimap->i_ipmnt;
		ipbmap = ipmnt->i_ipbmap;

jFYI(1,("jfs_chkdsk: ipimap:0x%08x ipmnt:0x%08x ipbmap:0x%08x\n", 
	ipimap, ipmnt, ipbmap));

		/* 
		 * sync the file system 
		 */
		iSyncFS(vfsp);

		/* 
		 * write dirty pages of imap 
		 */
		diSync(ipimap);

		/* 
		 * write dirty pages of bmap 
		 **/
		dbSync(ipbmap);

		return 0;
		}  /* end synchronization request */


	/* - - - - - - - - - - - - - - - - - - - - -
	 *
	 *  it's an error 	 
	 */
	else  {

jFYI(1,("\njfs_chkdsk: entered with invalid request\n\n"));

		return EINVAL;
		}

jFYI(1,("jfs_chkdsk: rc=%d\n\n", rc));

	return (rc);
}  							/* end jfs_chkdsk() */
