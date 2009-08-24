/* $Id: jfs_defragfs.c,v 1.1 2000/04/21 10:58:06 ktk Exp $ */

static char *SCCSID = "@(#)1.5  3/12/99 10:47:23 src/jfs/ifs/jfs_defragfs.c, sysjfs, w45.fs32, 990417.1";
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
 * Module: jfs_defragfs.c
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
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_txnmgr.h"
#include "jfs_defragfs.h"
#include "jfs_proto.h"
#include "jfs_debug.h"

#ifdef	_JFS_OS2
extern struct vfs *vfs_anchor;     /* used to find log VPB      */
#endif	/* _JFS_OS2 */

/*
 * NAME:        jfs_defragfs()
 *
 * FUNCTION:    relocate specified extent for defragfs()
 *              optionally commiting the operation.
 */
int32 jfs_defragfs(
	char	*pData,		/* pointer to buffer containing plist */ 
	uint32	lenData,	/* length of buffer */
	uint16	*pbufsize)	/* pointer of buffer length */ 
{
	int32	rc = 0;
	defragfs_t	pList, *p = &pList;
	uint32	xtype;
	int64	offset, xoff, oxaddr, nxaddr;
	int32	xlen;
	inode_t	*ipmnt, *ipimap, *ipbmap;
	inode_t	*ip = NULL;
	xad_t	oxad;
	pxd_t	opxd;
        mode_t	imode;
        int32	tid;
        inode_t	*iplist[1];
	struct vfs *vfsp;

	if (rc = KernCopyIn(&pList, pData, lenData))
		return rc;

	/* get the 'mount' inode */
        for (vfsp = vfs_anchor; vfsp != NULL; vfsp = vfsp->vfs_next)
		if (vfsp->vfs_vpfsi->vpi_drive == pList.dev)
			break;
	if (vfsp == NULL)
		return EINVAL;

	xtype = pList.flag;

	/* sync at start of defragfs ? */
	if (xtype & DEFRAGFS_SYNC)
	{
jEVENT(0,("jfs_defragfs: DEFRAGFS_SYNC\n"));

		if ((vfsp->vfs_flag & VFS_READONLY) ||
		    (vfsp->vfs_flag & VFS_ACCEPT))
			return 0;

		ipimap = (inode_t *)vfsp->vfs_data;
		ipmnt = ipimap->i_ipmnt;
		ipbmap = ipmnt->i_ipbmap;

		/* sync the file system */
		iSyncFS(vfsp);

		/* write dirty pages of imap */
		diSync(ipimap);

		/* write dirty pages of bmap */
		dbSync(ipbmap);

		return 0;
	}
	else if (!(xtype & DEFRAGFS_RELOCATE))
		return EINVAL;

	if (vfsp->vfs_flag & VFS_READONLY)
		return EROFS;

	if (vfsp->vfs_flag & VFS_ACCEPT)
		return EINVAL;

	/* get the relocation parameter */
        xoff = pList.xoff;
        oxaddr = pList.old_xaddr;
        nxaddr = pList.new_xaddr;
        xlen = pList.xlen;

jEVENT(0,("jfs_defragfs: i:%d xtype:0x%08x xoff:%lld xlen:%d xaddr:%lld:%lld\n",
	pList.ino, xtype, xoff, xlen, oxaddr, nxaddr));

	/* get the object inode if it exist */
	ICACHE_LOCK();
	rc = iget(vfsp, pList.ino, &ip, 0);
	ICACHE_UNLOCK();
	if(rc)
        {
jEVENT(0,("jfs_defragfs: stale target object.\n"));
		rc = ESTALE;  /* stale object ENOENT */
		goto out1;
        }

	IWRITE_LOCK(ip);
	
	/* validate inode */
	if (ip->i_nlink == 0  
	|| ip->i_gen != pList.gen
	|| ip->i_fileset != pList.fileset
	|| ip->i_inostamp != pList.inostamp)
	{
jEVENT(0,("jfs_defragfs: stale target object.\n"));
		rc = ESTALE;  /* stale object ENOENT */
		goto out1;
	}
	
	/* validate object type: regular file or directory */
	imode = ip->i_mode & IFMT;
	switch(imode)
	{
	case IFREG:
	case IFDIR:
		break;
	default:
		rc = ESTALE;	/* invalid object type ENOENT */
		goto out1;
	}

	/*
	 * try to allocate new destination extent
	 */
	if (rc = dbAllocExact(ip, nxaddr, xlen))
        {
jEVENT(0,("jfs_defragfs: stale destination extent.\n"));
		rc = ENOSPC;
		goto out1;
        }

	iBindCache(ip);

	/*
	 *	regular file: 
         */
	if (imode == IFREG)
	{
		/*
		 * automatic commit before and after each relocation 
		 * may be skipped after more experience;
		 */
		/*
		 * commit any update before relocation
		 */
		if (ip->i_flag & IUPD)
		{
			ip->i_flag |= IFSYNC;

			txBegin(ip->i_ipmnt, &tid, 0);
			iplist[0] = ip;
			rc = txCommit(tid, 1, &iplist[0], 0);
			if (rc)
				goto out2;
			txEnd(tid);
		}

		/*
		 *	relocate either xtpage or data extent
		 */
		txBegin(ip->i_ipmnt, &tid, 0);

		/* source extent xad */
		XADoffset(&oxad, xoff);
		XADaddress(&oxad, oxaddr);
		XADlength(&oxad, xlen);

		rc = xtRelocate(tid, ip, &oxad, nxaddr, xtype);
		if (rc)
			goto out2;

		iplist[0] = ip;
		rc = txCommit(tid, 1, &iplist[0], 0);
		if (rc)
			goto out2;

		txEnd(tid);
		goto out1;
	}
	/*
	 *	directory:
	 */
	else  /* IFDIR */
	{
		/*
		 *	relocate dtpage
		 */
		txBegin(ip->i_ipmnt, &tid, 0);

		/* source extent pxd */
		PXDaddress(&opxd, oxaddr);
		PXDlength(&opxd, xlen);
	
		rc = dtRelocate(tid, ip, xoff, &opxd, nxaddr); 
		if (rc)
			goto out2;

		iplist[0] = ip;
		rc = txCommit(tid, 1, &iplist[0], 0);
		if (rc)
			goto out2;

		txEnd(tid);
		goto out1;
	}

out2:
	dbFree(ip, nxaddr, xlen) ;

out1:
	if (ip)
	{
		IWRITE_UNLOCK(ip);

		ICACHE_LOCK();
		iput(ip, NULL);
		ICACHE_UNLOCK();
	}

jEVENT(0,("jfs_defragfs: rc=%d\n", rc));
	return (rc);
}
