/* $Id: jfs_mount.c,v 1.1.1.1 2003/05/21 13:38:00 pasha Exp $ */

static char *SCCSID = "@(#)1.30  11/1/99 13:57:32 src/jfs/ifs/jfs_mount.c, sysjfs, w45.fs32, fixbld";
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
 *   MODULE_NAME:		jfs_mount.c
 *
 *   COMPONENT_NAME:		sysjfs
 *
 *
*/

/*
 * Change History :
 *
 */

/*
 * Module: jfs_mount.c
 *
 * note: file system in transition to aggregate/fileset:
 *
 * file system mount is interpreted as the mount of aggregate, 
 * if not already mounted, and mount of the single/only fileset in 
 * the aggregate;
 *
 * a file system/aggregate is represented by an internal inode
 * (aka mount inode) initialized with aggregate superblock;
 * each vfs represents a fileset, and points to its "fileset inode 
 * allocation map inode" (aka fileset inode):
 * (an aggregate itself is structured recursively as a filset: 
 * an internal vfs is constructed and points to its "fileset inode 
 * allocation map inode" (aka aggregate inode) where each inode 
 * represents a fileset inode) so that inode number is mapped to 
 * on-disk inode in uniform way at both aggregate and fileset level;
 *
 * each vnode/inode of a fileset is linked to its vfs (to facilitate
 * per fileset inode operations, e.g., unmount of a fileset, etc.);
 * each inode points to the mount inode (to facilitate access to
 * per aggregate information, e.g., block size, etc.) as well as
 * its file set inode.
 *
 *   aggregate 
 *   ipmnt
 *   mntvfs -> fileset ipimap+ -> aggregate ipbmap -> aggregate ipaimap;
 *             fileset vfs     -> vp(1) <-> ... <-> vp(n) <->vproot;
 */
#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_bufmgr.h"
#include "jfs_logmgr.h"
#include "jfs_txnmgr.h"
#include "jfs_superblock.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_proto.h"
#include "jfs_dasdlim.h"					// D233382
#include "jfs_debug.h"

/*
 * external references
 */
void	vPut(struct inode *ip);
int32	iget(struct vfs	*vfsp, ino_t ino, inode_t **ipp, uint32	flag);

/*
 * forward references
 */
int32 readSuper(inode_t	*ipmnt, cbuf_t	**bpp);
static int32 chkSuper(struct inode *ipmnt);
static int32 updateSuper(inode_t *ipmnt);
static int32 logMOUNT(inode_t	*ipmnt);

/*
 * NAME:	jfs_mount(vfsp, crp)
 *
 * FUNCTION:	vfs_mount()
 *
 * PARAMETER:	vfsp	- virtual file system pointer
 *		crp	- credential
 *
 * RETURN:	EBUSY	- device already mounted or open for write
 *		EBUSY	- cvrdvp already mounted;
 *		EBUSY	- mount table full
 *		ENOTDIR	- cvrdvp not directory on a device mount
 *		ENXIO	- device open failure
 */
jfs_mount(
	struct vfs	*vfsp,		/* vfs to be mounted */
	struct ucred	*crp)		/* credential */
{
	int32	rc = 0;			/* Return code		*/
	union	mntvfs	dummyvfs, *mntvfsp;
	struct vnode	*cvrdvp;	/* covered vnode */
	struct vnode	*mntdvp = NULL;	/* mounted device object vnode */
	dev_t	fsdev = 0;		/* mounted fs device number */
	struct file *fp;		/* mounted fs device file ptr */
	dev_t	logdev = 0;		/* log device number	*/
	struct inode	*ipmnt = NULL;	/* mount inode */
	struct vnode	*vpmnt;		/* mount vnode */
	struct inode	*iplog = NULL;
	struct inode	*ipaimap = NULL;
	struct inode	*ipbmap = NULL;
	struct inode	*ipimap = NULL;
	struct inode	*iplist[8] = {0,0,0,0,0,0,0,0};
	struct inode	*ipaimap2 = NULL;
	struct inode	**ipp = &iplist[0];
	struct inode *iproot;		/* root inode */
	struct vnode *vproot;		/* root vnode */
	int32	flag;
	int32	pbsize;

jFYI(1,("\n	Mount JFS: vfs:0x%08x\n", vfsp));

	/*
	 * serialize mount/unmount with big bouncer lock
	 * (until Logical File System provides serialization
	 * between mount, unmount, name/file handle translation)
	 */
	JFS_LOCK();

	/*
	 * get the file system device being mounted
	 */
#ifdef _JFS_OS2
	/* get the device id for the file system being mounted */
	fsdev = vfsp->vfs_vpfsi->vpi_unit;
#endif /* _JFS_OS2 */

	/*
	 *	mount device:
	 */
mntDevice:
	/*
	 * get the file system inode (aka mount inode)
	 * (dev_t = file system device, fileset number = 0, i_number = 0)
	 *
	 * hand craft dummy/transient vfs to force iget()/iread() to 
	 * the special case of an in-memory inode allocation without 
	 * on-disk inode
	 */
	bzero(&dummyvfs, sizeof(struct vfs));
	/* dummyvfs.filesetvfs.vfs_data = NULL; */
	dummyvfs.dummyvfs.dev = fsdev;
	/* dummyvfs.dummyvfs.ipmnt = NULL; */
	ICACHE_LOCK();
	rc = iget((struct vfs *)&dummyvfs, 0, &ipmnt, 0);
	ICACHE_UNLOCK();
	if (rc)
		goto errout10;
jEVENT(0,("jfs_mount: ipmnt:0x%08x\n", ipmnt));

	/* aggregate mounted ? */
	if (ipmnt->i_devfp)
		goto mntLog;

	/* decouple mount inode from dummy vfs */
	vPut(ipmnt);

	/* ipmnt point itself */
	ipmnt->i_ipmnt = ipmnt;

	/*
	 * open the mounted fs device
	 */
#ifdef _JFS_OS2
	ipmnt->i_devfp = (struct file *)vfsp->vfs_vpfsi->vpi_hDEV;
	/* extended device driver strategy entry point */
	ipmnt->i_Strategy2 = vfsp->vfs_strat2p;
	ipmnt->i_Strategy3 = vfsp->vfs_strat3p;
	/* max S/G list entries per one i/o command */
	ipmnt->i_maxSGList = ((VolChars *)vfsp->vfs_vpfsi->vpi_pVCS)->MaxSGList;
#endif /* _JFS_OS2 */

	/*
	 * retrieve physical block (sector) size
	 */
#ifdef _JFS_OS2
	ipmnt->i_pbsize = vfsp->vfs_bsize;
	ipmnt->i_l2pbsize = log2shift(vfsp->vfs_bsize);
#endif /* _JFS_OS2 */

	/*
	 * read/validate superblock 
	 * (initialize mount inode from the superblock)
	 */
	if (rc = chkSuper(ipmnt))
	{
		if (rc == EROFS)
			vfsp->vfs_flag |= VFS_READONLY;
		else
			goto errout12;
	}
// BEGIN F226941
	else
	{
		if (ipmnt->i_mntflag & JFS_DASD_ENABLED)
			vfsp->vfs_flag |= VFS_DASDLIM;

		if (ipmnt->i_mntflag & JFS_DASD_PRIME)
			vfsp->vfs_flag |= VFS_DASDPRIME;
	}
// END F226941

	/*
	 * allocate/initialize cache device for aggregate i/o
	 */
        if (rc = cmMount(ipmnt, 64))
		goto errout12;

	/*
	 *	mount aggregate:
	 */
mntAggregate:
	/*
	 * open the aggregate inode allocation map (aka aggregate inode)
	 *
	 * aggregate itself is represented as a fileset (i.e., consists of 
	 * inode allocation map for allocating fileset inodes, and fileset
	 * inodes allocated in the aggregate).
	 *
	 * hand craft a vfs for the aggregate in the mount inode to
	 * represent the aggregate as a fileset; initialize the fileset
	 * vfs to force iget()/iread() to the special case of access of 
	 * inode of a fileset without the fileset inode allocation map
	 * of the fileset.
	 */
	mntvfsp = (union mntvfs *)&ipmnt->i_mntvfs;
	mntvfsp->filesetvfs.vfs_data = NULL;
	mntvfsp->dummyvfs.dev = fsdev;
	mntvfsp->dummyvfs.ipmnt = ipmnt;
#ifdef _JFS_OS2
	mntvfsp->filesetvfs.vfs_strat2p = vfsp->vfs_strat2p;
	mntvfsp->filesetvfs.vfs_strat3p = vfsp->vfs_strat3p;
#endif /* _JFS_OS2 */

	ICACHE_LOCK();
	rc = iget((struct vfs *)mntvfsp, AGGREGATE_I, ipp, 0);
	ICACHE_UNLOCK();
	if (rc)
	{
		/* open aggregate secondary inode allocation map */
		goto errout20;
	}
	ipaimap = *ipp++;
jEVENT(0,("jfs_mount: ipaimap:0x%08x\n", ipaimap));

	ipaimap->i_ipmnt = ipmnt;
	ipaimap->i_ipimap = ipaimap;

	ipmnt->i_ipaimap = ipaimap;

	/* map further access of per aggerate inodes
	 * via aggregate inode allocation map;
	 */
	mntvfsp->filesetvfs.vfs_data = (caddr_t)ipaimap;

	/*
	 * initialize aggregate inode allocation map
	 */
	if (rc = diMount(ipaimap))
		goto errout21;

	/*
	 * open aggregate block allocation map
	 */
	ICACHE_LOCK();
	rc = iget((struct vfs *)mntvfsp, BMAP_I, ipp, 0);
	ICACHE_UNLOCK();
	if (rc)
	{
		/* open aggregate secondary block allocation map */
		goto errout22;
	}

	ipbmap = *ipp++;
jEVENT(0,("jfs_mount: ipbmap:0x%08x\n", ipbmap));

	ipmnt->i_ipbmap = ipbmap;

	/*
	 * initialize aggregate block allocation map
	 */
	if (rc = dbMount(ipbmap))
		goto errout22;

	*ipp = NULL; /* mark end of inode open list */

#ifdef	_JFS_COMPRESSION
	/*
	 * initialize for compressed file system
	 *
	 * initialize decompression storage. 
	 * if not read-only create compression daemon.
	 */
#endif	/* _JFS_COMPRESSION */

	/*
	 *	mount log associated with the aggregate
	 */
mntLog:
	/* open log ? */
	if (ipmnt->i_iplog ||			/* log opened ? */
	    ipmnt->i_mntflag & JFS_TMPFS ||	/* tmp fs ? */
	    vfsp->vfs_flag & VFS_READONLY)	/* rofs ? */
		goto mntFileset;

	/*
	 * get log device associated with the fs being mounted;
	 */
#ifdef _JFS_OS2
	if (ipmnt->i_mntflag & JFS_INLINELOG)
	{
		vfsp->vfs_logVPB = vfsp->vfs_hVPB;
		vfsp->vfs_logvpfs = vfsp->vfs_vpfsi;
	}
	else if (vfsp->vfs_logvpfs == NULL)
	{
		/*
		 * XXX: there's only one external log per system;
		 */
jERROR(1,("jfs_mount: Mount Failure! No Log Device.\n"));
		goto errout30;
	}

	logdev = vfsp->vfs_logvpfs->vpi_unit;
	ipmnt->i_logdev = logdev;
#endif /* _JFS_OS2 */

	/*
	 * open/initialize log
	 */
	if (rc = lmLogOpen(ipmnt, &iplog))
		goto errout30;

	ipmnt->i_iplog = iplog;

	/* validate file system/commit option */
	if ((ipmnt->i_mntflag & JFS_COMMIT) != 
	    (((log_t *)iplog)->flag & JFS_COMMIT))
		goto errout31;

	/*
	 * update file system superblock;
	 */
	if (rc = updateSuper(ipmnt))
		goto errout31;

	/*
	 * write MOUNT log record of the file system
	 */
	logMOUNT(ipmnt);

	/* initialize aggregate wide rename lock */
	MUTEXLOCK_ALLOC(&ipmnt->i_renamelock,LOCK_ALLOC_PAGED,
			JFS_RENAME_LOCK_CLASS, -1);
	MUTEXLOCK_INIT(&ipmnt->i_renamelock);

mntAggregate2:
	/*
	 * open the secondary aggregate inode allocation map
	 *
	 * This is a duplicate of the aggregate inode allocation map.
	 *
	 * hand craft a vfs in the same fashion as we did to read ipaimap.
	 * By adding INOSPEREXT (32) to the inode number, we are telling
	 * diReadSpecial that we are reading from the secondary aggregate
	 * inode table.  This also creates a unique entry in the inode hash
	 * table.
	 */
	if ((ipmnt->i_mntflag & JFS_BAD_SAIT) == 0)
	{
		dummyvfs.filesetvfs.vfs_data = NULL;
		dummyvfs.dummyvfs.dev = fsdev;
		dummyvfs.dummyvfs.ipmnt = ipmnt;
#ifdef _JFS_OS2
		dummyvfs.filesetvfs.vfs_strat2p = vfsp->vfs_strat2p;
		dummyvfs.filesetvfs.vfs_strat3p = vfsp->vfs_strat3p;
#endif /* _JFS_OS2 */

		ICACHE_LOCK();
		rc = iget((struct vfs *)&dummyvfs, AGGREGATE_I + INOSPEREXT,
			  ipp, 0);
		ICACHE_UNLOCK();
		if (rc)
		{
			/* open aggregate secondary inode allocation map */
			goto errout35;
		}
		ipaimap2 = *ipp++;
jEVENT(0,("jfs_mount: ipaimap2:0x%08x\n", ipaimap2));

		/* decouple mount inode from dummy vfs */
		vPut(ipaimap2);

		/* We're going to lie about this inode's i_number so that the
		 * primary and secondary aggregate inodes will be uniquely
		 * identified in the inode hash table.  (Actually, the
		 * secondary aggregate inodes will not be found in the hash
		 * table, but they are not held in the cache anyway.)
		 */
		ipaimap2->i_number = 5;	/* Aggregate inode 5 is not used. */
		ipaimap2->i_ipmnt = ipmnt;
		ipaimap2->i_ipimap = ipaimap2;

		ipmnt->i_ipaimap2 = ipaimap2;

		/*
		 * initialize secondary aggregate inode allocation map
		 */
		if (rc = diMount(ipaimap2))
			goto errout35;
	}
	else
		/* Secondary aggregate inode table is not valid */
		ipmnt->i_ipaimap2 = 0;

	/*
	 *	mount (the only/single) fileset
	 */
mntFileset:
	/*
	 * open fileset inode allocation map (aka fileset inode)
	 */
	ICACHE_LOCK();
	rc = iget((struct vfs *)mntvfsp, FILESYSTEM_I, &ipimap, 0);
	ICACHE_UNLOCK();
	if (rc)
	{
		/* open fileset secondary inode allocation map */
		goto errout40;
	}
jEVENT(0,("jfs_mount: ipimap:0x%08x\n", ipimap));

	/* map further access of per fileset inodes by the fileset inode */
	vfsp->vfs_data = (caddr_t)ipimap;
	ipmnt->i_ipimap = ipimap;				// D230860

	/* initialize fileset inode allocation map */
	if (rc = diMount(ipimap))
		goto errout41;

	/*
	 * get root vnode/inode of mounted fileset
	 */
	ICACHE_LOCK();
	rc = iget(vfsp, ROOT_I, &iproot, 0);
	ICACHE_UNLOCK();
	if (rc)
		goto errout42;
        vproot = IP2VP(iproot);
	vproot->v_flag |= V_ROOT;
jEVENT(0,("jfs_mount: iproot:0x%08x\n", iproot));

	/* establish mounted/root vnode of mounted fileset */
	vfsp->vfs_mntd = vproot;

jFYI(0,("	Mount JFS Complete.\n", rc));
	goto out;

	/*
	 *	unwind on error
	 */
errout42: /* close fileset inode allocation map */
	diUnmount(ipimap, 1);

errout41: /* close fileset inode allocation map inode */
	bmInodeInvalidate(ipimap);
	iUnbindCache(ipimap);

	ICACHE_LOCK();
	iunhash(ipimap);
	ICACHE_UNLOCK();

errout40: /* fileset closed */

	/* close secondary aggregate inode allocation map */
	if (ipaimap2)
		diUnmount(ipaimap2, 1);

errout35:

	/* release aggregate rename lock */
	MUTEXLOCK_FREE(&ipmnt->i_renamelock);

errout31: /* close log */
	if (iplog)
		lmLogClose(ipmnt, iplog);

errout30: /* log closed */

	/* close aggregate block allocation map */
	dbUnmount(ipbmap, 1);

errout22: /* close aggregate inode allocation map */
	diUnmount(ipaimap, 1);

errout21: /* close aggregate inodes */
	if (ipmnt->i_count == 1)
	{
		for (ipp = iplist; *ipp; ipp++)
		{
			bmInodeInvalidate(*ipp);
			iUnbindCache(*ipp);

			ICACHE_LOCK();
			iunhash(*ipp);
			ICACHE_UNLOCK();
		}
	}

errout20: /* aggregate closed */

	/* free cache device of the file system */
	if (ipmnt->i_count == 1)
		cmUnmount(ipmnt->i_cachedev);

errout12: /* close file system device */

errout11: /* close mount inode */
	if (ipmnt->i_count == 1)
	{
		ICACHE_LOCK();
		iunhash(ipmnt);
		ICACHE_UNLOCK();
	}
	else
		ipmnt->i_count--;

errout10: /* device unmounted */

out:
	JFS_UNLOCK();

	if (rc)
	{
jFYI(1,("	Mount JFS Failure: %d\n", rc));
	}
	return rc;
}


/*
 *	chkSuper()
 *
 * validate the superblock of the file system to be mounted and 
 * get the file system parameters.
 *
 * returns
 *	0 with fragsize set if check successful
 *	error code if not successful
 */
static int32
chkSuper(
	struct inode	*ipmnt)
{
	int32	rc = 0;
	cbuf_t	*bp;
	struct superblock *sb;
	int32	niperblk;
	int32 AIM_bytesize, AIT_bytesize;
	int32 expected_AIM_bytesize, expected_AIT_bytesize;
	int64 AIM_byte_addr, AIT_byte_addr, fsckwsp_addr;
	int64 byte_addr_diff0, byte_addr_diff1;

	if (rc = readSuper(ipmnt, &bp))
		return rc;
	sb = (struct superblock *)(bp->cm_cdata);

	/*
	 * validate superblock
	 */
	/* validate fs signature */
	if (strncmp(sb->s_magic, JFS_MAGIC, 4) ||
	    sb->s_version != JFS_VERSION)
	{
		rc = EFORMAT;
		goto out;
	}

jFYI(0,("superblock: flag:0x%08x state:0x%08x size:0x%x%08x\n", 
	sb->s_flag, sb->s_state, sb->s_size));

	/* validate the descriptors for Secondary AIM and AIT */
	if( (sb->s_flag & JFS_BAD_SAIT) != JFS_BAD_SAIT ) 
	{
		expected_AIM_bytesize = 2 * PSIZE;
		AIM_bytesize = lengthPXD(&(sb->s_aim2)) * sb->s_bsize;
		expected_AIT_bytesize = 4 * PSIZE;
		AIT_bytesize = lengthPXD(&(sb->s_ait2)) * sb->s_bsize;
		AIM_byte_addr = addressPXD(&(sb->s_aim2)) * sb->s_bsize;
		AIT_byte_addr = addressPXD(&(sb->s_ait2)) * sb->s_bsize;
		byte_addr_diff0 = AIT_byte_addr - AIM_byte_addr;
		fsckwsp_addr = addressPXD(&(sb->s_fsckpxd)) * sb->s_bsize;
		byte_addr_diff1 = fsckwsp_addr - AIT_byte_addr;
		if(	(AIM_bytesize != expected_AIM_bytesize) ||
			(AIT_bytesize != expected_AIT_bytesize) ||
			(byte_addr_diff0 != AIM_bytesize) ||
			(byte_addr_diff1 <= AIT_bytesize) ) 
			sb->s_flag |= JFS_BAD_SAIT;
	}

	/* in release 1, the flag MUST reflect OS2, inline log, and group commit */
	if( (sb->s_flag & JFS_INLINELOG) != JFS_INLINELOG )
		sb->s_flag |= JFS_INLINELOG;
	if( (sb->s_flag & JFS_OS2) != JFS_OS2 )
		sb->s_flag |= JFS_OS2;
	if( (sb->s_flag & JFS_GROUPCOMMIT) != JFS_GROUPCOMMIT )
		sb->s_flag |= JFS_GROUPCOMMIT;
jFYI(0,("superblock: flag:0x%08x state:0x%08x size:0x%x%08x\n", 
	sb->s_flag, sb->s_state, sb->s_size));

	/* validate fs state */
	if (sb->s_state != FM_CLEAN)
	{
jERROR(1,("jfs_mount: Mount Failure: File System Dirty.\n"));
		rc = EFORMAT;
		goto out;
	}

	/*
	 * initialize mount inode from superblock
	 */
	ipmnt->i_mntflag = sb->s_flag;
	ipmnt->i_mntflag |= JFS_GROUPCOMMIT;
/*
	ipmnt->i_mntflag |= JFS_LAZYCOMMIT;
*/
	if (ipmnt->i_pbsize != sb->s_pbsize ||
	    ipmnt->i_l2pbsize != sb->s_l2pbsize)
	{
		rc = EFORMAT;
		goto out;
	}

	ipmnt->i_blocks = sb->s_size;
	ipmnt->i_bsize = sb->s_bsize;
	ipmnt->i_l2bsize = sb->s_l2bsize;
	ipmnt->i_l2bfactor = sb->s_l2bfactor;
	ipmnt->i_compress = sb->s_compress;
	ipmnt->i_ait2 = sb->s_ait2;

	ipmnt->i_nbperpage = PSIZE >> ipmnt->i_l2bsize;
	ipmnt->i_l2nbperpage = log2shift(ipmnt->i_nbperpage);
	niperblk = ipmnt->i_bsize / DISIZE;
	ipmnt->i_l2niperblk = log2shift(niperblk);
jFYI(0,("superblock: bsize:%d(%d) pbsize:%d(%d) l2factor:%d\n",
	ipmnt->i_bsize, ipmnt->i_l2bsize, ipmnt->i_pbsize, ipmnt->i_l2pbsize,
	ipmnt->i_l2bfactor)); 

	if (sb->s_flag & JFS_INLINELOG)
		ipmnt->i_logpxd = sb->s_logpxd;

out:
	/* if we've overridden any fields we need to write it to the vol */
	if(rc == 0)
	{
		if (rawWrite(ipmnt, bp, 1))
			rc = EROFS;
	}	
	else
		rawRelease(bp);

	return rc;
}


/*
 *	updateSuper()
 *
 * update synchronously superblock if it is mounted read-write.
 */
static int32
updateSuper(
	inode_t	*ipmnt)
{
	int32	rc;
	cbuf_t	*bp;
	struct superblock *sb;

	if (rc = readSuper(ipmnt, &bp))
		return rc;
	sb = (struct superblock *)(bp->cm_cdata);

	/*
	 * file system state transition: mounted-clean. 
	 */
	sb->s_state = FM_MOUNT;

#ifdef _JFS_FASTDASD
	/*
	 * When mounted and DASD limits enabled, we set the DASD_PRIME flag
	 * in the superblock.  If we don't unmount properly, DASD usage will
	 * be recaculated at the next reboot.
	 */
	if (sb->s_flag & JFS_DASD_ENABLED)			// D233382
		sb->s_flag |= JFS_DASD_PRIME;			// D233382
#endif /* _JFS_FASTDASD */

	/* record log's dev_t and mount serial number */
	sb->s_logdev = ipmnt->i_logdev;
	sb->s_logserial = ((log_t *)ipmnt->i_iplog)->serial;

	rawWrite(ipmnt, bp, 1);

	return 0;
}


/*
 *	readSuper()
 *
 * read superblock by raw sector address
 */
int32
readSuper(
	inode_t	*ipmnt,
	cbuf_t	**bpp)
{
	int32	rc = 0;

	/* read in primary superblock */ 
	if (rc = rawRead(ipmnt, SUPER1_OFF, bpp))
	{
		/* read in secondary/replicated superblock */ 
		rc = rawRead(ipmnt, SUPER2_OFF, bpp);
	}

	return rc;
}


/*
 *	logMOUNT()
 *
 * function: write a MOUNT log record for file system.
 *
 * MOUNT record keeps logredo() from processing log records
 * for this file system past this point in log.
 * it is harmless if mount fails.
 *
 * note: MOUNT record is at aggregate level, not at fileset level, 
 * since log records of previous mounts of a fileset
 * (e.g., AFTER record of extent allocation) have to be processed 
 * to update block allocation map at aggregate level.
 */
static int32
logMOUNT(
	inode_t	*ipmnt)
{
	inode_t	*iplog = ipmnt->i_iplog;
	lrd_t	lrd;

	lrd.logtid = 0;
	lrd.backchain = 0;
	lrd.type = LOG_MOUNT;
	lrd.length = 0;
	lrd.aggregate = ipmnt->i_dev;
	lmLog((log_t *)iplog, NULL, &lrd, NULL);

	return 0;
}
