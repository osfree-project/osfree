/* $Id: jfs_clrbblks.c,v 1.1 2000/04/21 10:58:05 ktk Exp $ */

static char *SCCSID = "@(#)1.3  3/12/99 10:47:05 src/jfs/ifs/jfs_clrbblks.c, sysjfs, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		jfs_clrbblks.c
 *
 *   COMPONENT_NAME: 	sysjfs
 *
 *   FUNCTIONS:	jfs_clrbblks
 *
*/

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include <string.h>
#include "jfs_os2.h"

#include "jfs_types.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_txnmgr.h"
#include "jfs_clrbblks.h"
#include "jfs_ea.h"
#include "jfs_proto.h"
#include "jfs_debug.h"

extern struct vfs *vfs_anchor;     /* used to find log VPB      */

/****************************************************************************
 * NAME:        jfs_clrbblks()
 *
 * FUNCTION:    relocate the specified extent for clrbblks()
 *
 *		if successful, 
 *
 *		    allocate the specified (source) block to the
 *		    JFS Bad Block Inode
 *
 */
int32 jfs_clrbblks( char	*pData,		/* ptr to plist buffer */ 
		    uint32	lenData,	/* length of buffer */
		    uint16	*pbufsize)	/* ptr to buffer length */ 
{
	int32	rc = 0;
	clrbblks_t	pList, *p = &pList;
	uint32	xtype;
	int64	offset, xoff, oxaddr, nxaddr, xaddr, EAaddr;
	int32	xlen, EAlen;
	inode_t	*ipmnt, *ipimap, *ipbmap;
	inode_t	*ip = NULL;
	xad_t	oxad;
	pxd_t	opxd;
	mode_t	imode;
	int32	tid;
	inode_t	*iplist[1];
	struct vfs *vfsp;
	union mntvfs	dummyvfs, *mntvfsp;	/* to read the bad block inode */


	if (rc = KernCopyIn(&pList, pData, lenData))  {
jFYI(0,("\njfs_clrbblks: KernCopyIn(&pList, pData, lenData) returned rc = %d\n", rc));
		return rc;
		}

	/* 
	 * get the 'mount' inode 
	 */
        for (vfsp = vfs_anchor; vfsp != NULL; vfsp = vfsp->vfs_next)  {
		if (vfsp->vfs_vpfsi->vpi_drive == pList.dev)  {
			break;
			}
		}
	if ((vfsp == NULL) || (vfsp->vfs_flag & VFS_ACCEPT)) {
		return EINVAL;
		}

	xtype = pList.flag;

	/* - - - - - - - - - - - - - - - - - - - - -
	 *
	 * synchronization request?   
	 */
	if (xtype & CLRBBLKS_SYNC) {

jFYI(0,("\njfs_clrbblks: entering for CLRBBLKS_SYNC\n"));

		if (vfsp->vfs_flag & VFS_READONLY)
			return 0;

		ipimap = (inode_t *)vfsp->vfs_data;
		ipmnt = ipimap->i_ipmnt;
		ipbmap = ipmnt->i_ipbmap;

jFYI(0,("jfs_clrbblks: ipimap:0x%08x ipmnt:0x%08x ipbmap:0x%08x\n", 
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
	 * bad block allocation request?   
	 */
	else if (xtype & CLRBBLKS_BADBLKALLOC) {

jFYI(0,("\njfs_clrbblks: entering for CLRBBLKS_BADBLKALLOC\n"));

		if (vfsp->vfs_flag & VFS_READONLY)
			return EROFS;

		/* 
		 * get the bad block address 
		 */
		xoff = 0;		/* we'll figure this out later */
		nxaddr = pList.old_xaddr;
		xlen = 1;
	
		/*
		 * open the Bad Block inode
		 *
		 */
		ipimap = (inode_t *)vfsp->vfs_data;
		ipmnt = ipimap->i_ipmnt;
		mntvfsp = (union mntvfs *)&ipmnt->i_mntvfs;
		ICACHE_LOCK();
		rc = iget((struct vfs *)mntvfsp, (ino_t)BADBLOCK_I, &ip, 0);
		ICACHE_UNLOCK();
		if(rc)  {
jFYI(0,("jfs_clrbblks: unable to open bad block inode.\n"));
			rc = EOPENFAILED;  
			goto out1;
			}
		IWRITE_LOCK(ip);
		/*
		 * i_size is the last byte 'written' to the inode.
		 * the next byte is the beginning of where to 'write'
		 * this block. 
		 * Divide by blocksize for next block to write.
		 */
		xoff = (ip->i_size + 1) / pList.agg_blksize;

jFYI(0,("jfs_clrbblks: pList.old_xaddr: 0x%16llx  xoff: 0x%08x  xlen: 0x%08x\n", 
	pList.old_xaddr, xoff, xlen));
	
		/*
		 * try to allocate the bad block
		 */
		if (rc = dbAllocExact(ip, nxaddr, xlen))  {
jFYI(0,("jfs_clrbblks: dbAllocExact(ip, nxaddr, xlen) returned rc = %d\n", rc));
jFYI(0,("jfs_clrbblks: stale bad block information?\n"));
			rc = ENOSPC;
			goto out1;
			}

		/*
		 * automatic commit before and after each relocation 
		 * may be skipped after more experience;
		 */
		/*
		 * commit any update before relocation
		 */
		if (ip->i_flag & IUPD)  {
			ip->i_flag |= IFSYNC;
			txBegin(ip->i_ipmnt, &tid, 0);
			iplist[0] = ip;
			rc = txCommit(tid, 1, &iplist[0], 0);
			if (rc)  {
jFYI(0,("\njfs_clrbblks: txCommit(tid, 1, &iplist[0], 0) returned rc = %d\n", rc));
				goto out2;
				}
			txEnd(tid);
			}

		/*
		 * allocate the block to the bad block inode 
		 */
		txBegin(ip->i_ipmnt, &tid, 0);
	
		rc = xtInsert(tid, ip, 0, xoff, xlen, &nxaddr, 0);
		if (rc)  {
jFYI(0,("jfs_clrbblks: failed to alloc to Bad Block inode.\n"));
			goto out2;
			}

		ip->i_size += pList.agg_blksize;
		ip->i_nblocks += 1;

		iplist[0] = ip;
		rc = txCommit(tid, 1, &iplist[0], 0);
		if (rc)  {
jFYI(0,("\njfs_clrbblks: txCommit(tid, 1, &iplist[0], 0) returned rc = %d\n", rc));
			goto out2;
			}
		txEnd(tid);
		goto out1;

		}  /* end bad block alloc request */


	/* - - - - - - - - - - - - - - - - - - - - -
	 *
	 * if not a relocate request, it's an error 	 
	 */
	else if ( !(xtype & CLRBBLKS_RELOCATE) ) {

jFYI(0,("\njfs_clrbblks: entered with invalid request\n\n"));

		return EINVAL;
		}

	/* - - - - - - - - - - - - - - - - - - - - -
	 *
	 * it's a relocate request 	 
	 */

jFYI(0,("\njfs_clrbblks: entering for CLRBBLKS_RELOCATE\n"));

	if (vfsp->vfs_flag & VFS_READONLY)
		return EROFS;

	/* 
	 * get the relocation parameter 
	 */
        xoff = pList.xoff;
        oxaddr = pList.old_xaddr;
        nxaddr = 0;			/* we don't know what this is yet */
        xlen = pList.xlen;

jFYI(0,("jfs_clrbblks: i:%d  xtype:0x%08x  xoff:%lld  xlen:%d  xaddr:%lld : %lld\n",
	pList.ino, xtype, xoff, xlen, oxaddr, nxaddr));

	/* get the object inode if it exist */
	ICACHE_LOCK();
	rc = iget(vfsp, pList.ino, &ip, 0);
	ICACHE_UNLOCK();
	if(rc)  {
		rc = ESTALE;  /* stale object ENOENT */
jFYI(0,("jfs_clrbblks: stale (1) target object.\n"));
		goto out1;
	        }

	IWRITE_LOCK(ip);
	
	/* 
	 * validate inode 
	 */
	if (ip->i_nlink == 0  
		|| ip->i_gen != pList.gen
		|| ip->i_fileset != pList.fileset
		|| ip->i_inostamp != pList.inostamp)  {

		rc = ESTALE;  /* stale object ENOENT */
jFYI(0,("jfs_clrbblks: stale (2) target object.\n"));
		goto out1;
		}
	
	/* 
	 * validate object type: regular file or directory 
	 */
	imode = ip->i_mode & IFMT;
	switch(imode)  {
	    case IFREG:
	    case IFDIR:
		break;
	    default:
		rc = ESTALE;	/* invalid object type ENOENT */
jFYI(0,("jfs_clrbblks: stale (3) target object.\n"));
		goto out1;
	    }

		/*
		 *	extended attributes 
		 * (either regular file or directory)
        	 */
	if( xtype & CLRBBLKS_EA ) {

jFYI(0,("jfs_clrbblks: type == relocate EA.\n"));
		/*
		 * verify that the EA field is as expected
		 */
		EAaddr =addressXAD(&(ip->i_ea));
		EAlen = lengthXAD(&(ip->i_ea));
		if( (oxaddr != EAaddr) || (xlen != EAlen) ) {
			rc = ESTALE;	/* it's stale info */
jFYI(0,("jfs_clrbblks: stale (4) target object.\n"));
			goto out2;
			}
		/*
		 * automatic commit before and after each relocation 
		 * may be skipped after more experience;
		 */
		/*
		 * commit any update before relocation
		 */
		if (ip->i_flag & IUPD)  {
			ip->i_flag |= IFSYNC;

			txBegin(ip->i_ipmnt, &tid, 0);
			iplist[0] = ip;
			rc = txCommit(tid, 1, &iplist[0], 0);
			if (rc)
				goto out2;
			txEnd(tid);
			}
		/*
		 *	relocate the EA
		 */
		txBegin(ip->i_ipmnt, &tid, 0);

		rc = eaRelocate( ip );
		if (rc)  {
jFYI(0,("\njfs_clrbblks: eaRelocate( ip ) returned rc = %d\n", rc));
			goto out2;
			}
		iplist[0] = ip;
		rc = txCommit(tid, 1, &iplist[0], 0);
		if (rc)  {
jFYI(0,("\njfs_clrbblks: txCommit(tid, 1, &iplist[0], 0) returned rc = %d\n", rc));
			goto out2;
			}
		txEnd(tid);
		/*
		 * 	return the new address
		 */
		pList.new_xaddr = addressDXD( &(ip->i_ea) );
		KernCopyOut(pData, &pList, lenData);
jFYI(0,("jfs_clrbblks: type == relocate EA: oxaddr: %lld  nxaddr: %lld\n", 
	oxaddr, pList.new_xaddr));
	
		goto out1;
		}  /* end extended attributes */
	else {	/* not extended attributes */
		/*
		 * try to allocate a destination extent, using the 
		 * old address as a hint.
		 */
	    if (rc = dbAlloc(ip, oxaddr, xlen, &nxaddr))  {  /* if rc != 0 */

jFYI(0,("jfs_clrbblks: can't allocate an extent.\n"));

		rc = ENOSPC;
		goto out1;
		}  /* end if rc != 0 */

	    iBindCache(ip);

		/*
		 *	regular file: 
	        	 */
	    if (imode == IFREG)  {
jFYI(0,("jfs_clrbblks: type == relocate for REG.\n"));
		/*
		 * automatic commit before and after each relocation 
		 * may be skipped after more experience;
		 */
		/*
		 * commit any update before relocation
		 */
		if (ip->i_flag & IUPD)  {
			ip->i_flag |= IFSYNC;

			txBegin(ip->i_ipmnt, &tid, 0);
			iplist[0] = ip;
			rc = txCommit(tid, 1, &iplist[0], 0);
			if (rc)  {
jFYI(0,("\njfs_clrbblks: txCommit(tid, 1, &iplist[0], 0) returned rc = %d\n", rc));
				goto out2;
				}
			txEnd(tid);
			}  /* end if (i_flag & IUPD) */

		/*
		 *	relocate either xtpage or data extent
		 */
		txBegin(ip->i_ipmnt, &tid, 0);

		/* 
		 * source extent xad 
		 */
		XADoffset(&oxad, xoff);
		XADaddress(&oxad, oxaddr);
		XADlength(&oxad, xlen);

		rc = xtRelocate(tid, ip, &oxad, nxaddr, xtype);
		if (rc)  {
jFYI(0,("\njfs_clrbblks: xtRelocate(tid, ip, &oxad, nxaddr, xtype) returned rc = %d\n", rc));
			goto out2;
			}
		iplist[0] = ip;
jFYI(0,("jfs_clrbblks: iplist[0] = %la\n", iplist[0] ));
		rc = txCommit(tid, 1, &iplist[0], 0);
		if (rc)  {
jFYI(0,("\njfs_clrbblks: txCommit(tid, 1, &iplist[0], 0) returned rc = %d\n", rc));
			goto out2;
			}
		txEnd(tid);
		/*
		 * return the new extent address to caller
		 */
		pList.new_xaddr = nxaddr; 
		KernCopyOut(pData, &pList, lenData);
jFYI(0,("jfs_clrbblks: REG reloc: ino: %d  oxaddr: %lld  nxaddr: %lld\n", ip->i_number,oxaddr, nxaddr));
		goto out1;
		}  /* end IFREG */
		/*
		 *	directory:
		 */
	    else  {   /* IFDIR */
jFYI(0,("jfs_clrbblks: type == relocate for DIR.\n"));
		/*
		 *	relocate dtpage
		 */
		txBegin(ip->i_ipmnt, &tid, 0);

		/* source extent pxd */
		PXDaddress(&opxd, oxaddr);
		PXDlength(&opxd, xlen);
	
		rc = dtRelocate(tid, ip, xoff, &opxd, nxaddr); 
		if (rc) {
jFYI(0,("\njfs_clrbblks: dtRelocate(tid, ip, xoff, &opxd, nxaddr) returned rc = %d\n", rc));
			goto out2;
			}
		iplist[0] = ip;
		rc = txCommit(tid, 1, &iplist[0], 0);
		if (rc) {
jFYI(0,("jfs_clrbblks: txCommit(tid, 1, &iplist[0], 0) returned rc = %d\n", rc));
			goto out2;
			}
		txEnd(tid);
		/*
		 * return the new extent address to caller
		 */
		pList.new_xaddr = nxaddr; 
		KernCopyOut(pData, &pList, lenData);
jFYI(0,("jfs_clrbblks: DIR reloc: ino: %d  oxaddr: %lld  nxaddr: %lld\n", ip->i_number,oxaddr, nxaddr));

		goto out1;
		}  /* end  else IFDIR */
	    }  /* end else not extended attributes */

out2:
	dbFree(ip, nxaddr, xlen) ;

out1:
	if (ip)  {
		IWRITE_UNLOCK(ip);
		ICACHE_LOCK();
		iput(ip, NULL);
		ICACHE_UNLOCK();
		}

jFYI(0,("jfs_clrbblks: rc=%d\n\n", rc));

	return (rc);
}  							/* end jfs_clrbblks() */
