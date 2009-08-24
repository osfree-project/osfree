/* $Id: jfs_extendfs.c,v 1.1.1.1 2003/05/21 13:37:04 pasha Exp $ */

static char *SCCSID = "@(#)1.13  8/17/99 11:05:51 src/jfs/ifs/jfs_extendfs.c, sysjfs, w45.fs32, currbld";
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
 *   MODULE_NAME:               jfs_extendfs.c
 *
 *   COMPONENT_NAME:    jfssys
 *
 *   FUNCTIONS:         jfs_extendfs
 *                      quiesceFS
 *                      resumeFS
 *
*/
 /*
 * Change history:
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
#include "jfs_filsys.h"
#include "jfs_superblock.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_bufmgr.h"
#include "jfs_logmgr.h"
#include "jfs_txnmgr.h"
#include "jfs_extendfs.h"
#include "jfs_proto.h"
#include "jfs_debug.h"

#ifdef  _JFS_OS2
extern struct vfs *vfs_anchor;     /* used to find log VPB      */
#endif  /* _JFS_OS2 */

#define BITSPERPAGE     (PSIZE << 3)
#define L2MEGABYTE      20
#define MEGABYTE        (1 << L2MEGABYTE)
#define MEGABYTE32     (MEGABYTE << 5)

/* convert block number to bmap file page number */
#define BLKTODMAPN(b)\
        (((b) >> 13) + ((b) >> 23) + ((b) >> 33) + 3 + 1)

/*
 * external references
 */
extern int32 readSuper(inode_t *ipmnt, cbuf_t **bpp);
extern int32 iQuiesce(struct vfs *vfsp);
extern int32 iResume(struct vfs *vfsp);
extern int32 bmLogSync(log_t *log);

/*
 * forward references
 */
static void quiesceFS(struct vfs *vfsp, inode_t *ipmnt);
static void resumeFS(struct vfs *vfsp, inode_t  *ipmnt);


/*
 *      jfs_extendfs()
 *
 * function: extend file system;
 *
 *   |-------------------------------|----------|----------|
 *   file system space               fsck       inline log
 *                                   workspace  space
 *
 * input:
 *      new LVSize: in LV blocks (required)
 *      new LogSize: in LV blocks (optional)
 *      new FSSize: in LV blocks (optional)
 *
 * new configuration:
 * 1. set new LogSize as specified or default from new LVSize;
 * 2. compute new FSCKSize from new LVSize;
 * 3. set new FSSize as MIN(FSSize, LVSize-(LogSize+FSCKSize)) where
 *    assert(new FSSize >= old FSSize),
 *    i.e., file system must not be shrinked;
 */
int32 jfs_extendfs(
        char    *pData,         /* pointer to buffer containing plist */
        uint32  lenData,        /* length of buffer */
        uint16  *pbufsize)      /* pointer of buffer length */
{
        int32   rc = 0;
        inode_t *ipmnt, *ipimap, *ipbmap, *iplog;
        log_t   *log;
        bmap_t  *mp;
        extendfs_t      pList, *p = &pList;
        int64   newLVSize, LVSize, newFSSize;
        int64   newLogAddress, newFSCKAddress;
        int32   newLogSize, newFSCKSize;
        int64   newMapSize, mapSize;
        int64   XAddress, XSize, nblocks, xoff, xaddr, t64;
        int32   newNpages, nPages, newPage, xlen, t32;
        int32   tid;
        inode_t *iplist[1];
        struct superblock *sb;
        jbuf_t  *sbp, *bp;
        dinode_t        *dip;
#ifdef  _JFS_OS2
        struct vfs *vfsp;
#endif  /* _JFS_OS2 */

#ifdef  _JFS_OS2
        /* copyin parameter list */
        if (rc = KernCopyIn(&pList, pData, lenData))
                return rc;

        /* get the mounted VFS */
        for (vfsp = vfs_anchor; vfsp != NULL; vfsp = vfsp->vfs_next)
                if (vfsp->vfs_vpfsi->vpi_drive == pList.dev)
                        break;
        if ((vfsp == NULL) || (vfsp->vfs_flag & VFS_ACCEPT))
                return EINVAL;
#endif  /* _JFS_OS2 */

        /* get the parameters */
        newLVSize = pList.LVSize;
        newLogSize = pList.LogSize;
        newFSSize = pList.FSSize;

        ipimap = (inode_t *)vfsp->vfs_data;
        ipmnt = ipimap->i_ipmnt;
        ipbmap = ipmnt->i_ipbmap;
        mp = ipbmap->i_bmap;
        log = (log_t *)(ipmnt->i_iplog);

        /* If the volume hasn't grown, get out now */
        if (rc = readSuper(ipmnt, &sbp))
                return rc;
        sb = (struct superblock *)(sbp->b_bdata);

        if (ipmnt->i_mntflag & JFS_INLINELOG)
        {
                if ((addressPXD(&sb->s_logpxd) + lengthPXD(&sb->s_logpxd)) >=
                    (newLVSize >> ipmnt->i_l2bfactor))
                {
                        rawRelease(sbp);
                        return 0;
                }
        }
        else
        {
                if ((addressPXD(&sb->s_fsckpxd) + lengthPXD(&sb->s_fsckpxd)) >=
                    (newLVSize >> ipmnt->i_l2bfactor))
                {
                        rawRelease(sbp);
                        return 0;
                }
        }
        rawRelease(sbp);

        /* Can't extend write-protected drive */
        if (vfsp->vfs_flag & VFS_READONLY)
                return EROFS;

jEVENT(0,("jfs_extendfs: ipmnt:0x%x LVSize:%lld LogSize:%d FSSize:%lld\n",
        ipmnt, newLVSize, newLogSize, newFSSize));

        /*
         * prevent concurrent file system reconfiguration
         */
        IWRITE_LOCK(ipmnt);

        /*
         *      reconfigure LV spaces
         *      ---------------------
         *
         * validate new size, or, if not specified, determine new size
         */

        /* convert LV size to aggregate blocks */
        newLVSize = newLVSize >> ipmnt->i_l2bfactor;
        LVSize = newLVSize;

        /*
         * reconfigure inline log space:
         */
        if (!(ipmnt->i_mntflag & JFS_INLINELOG))
                goto moveFSCKWorkSpace;

        if (newLogSize == 0)
        {
                /*
                 * no size specified: default to 1/256 of aggregate size;
                 * rounded up to a megabyte boundary;
                 */
                newLogSize = LVSize >> 8;
                t32 = (1 << (20 - ipmnt->i_l2bsize)) - 1;
                newLogSize = (newLogSize + t32) & ~t32;
                newLogSize = MIN(newLogSize, MEGABYTE32 >> ipmnt->i_l2bsize);
        }
        else
        {
                /*
                 * convert the newLogSize to fs blocks.
                 *
                 * Since this is given in megabytes, it will always be an
                 * even number of pages.
                 */
                newLogSize = (newLogSize * MEGABYTE) >> ipmnt->i_l2bsize;
        }

        newLogAddress = LVSize - newLogSize;
        LVSize -= newLogSize;

        /*
         * reconfigure fsck work space:
         *
         * configure it to the end of the logical volume regardless of
         * whether file system extends to the end of the aggregate;
         * Need enough 4k pages to cover:
         *  - 1 bit per block in aggregate rounded up to BPERDMAP boundary
         *  - 1 extra page to handle control page and intermediate level pages
         *  - 50 extra pages for the chkdsk service log
         */
moveFSCKWorkSpace:
        t64 = ((LVSize + BPERDMAP - 1) >> L2BPERDMAP) << L2BPERDMAP;
        t32 = ((t64 + (BITSPERPAGE - 1)) / BITSPERPAGE) + 1 + 50;
        newFSCKSize = t32 << ipmnt->i_l2nbperpage;
        newFSCKAddress = LVSize - newFSCKSize;
        LVSize -= newFSCKSize;

        /*
         * compute new file system space;
         */
        if (newFSSize == 0)
                /* no size specified: extend to full available space */
                newFSSize = LVSize;
        else
        {
                newFSSize >>= ipmnt->i_l2bfactor;
                newFSSize = MIN(newFSSize, LVSize);
        }

        /* file system cannot be shrinked */
        if (newFSSize < mp->db_mapsize)
        {
                rc = EINVAL;
                goto out;
        }

jFYI(1,("jfs_extendfs: ipmnt:0x%x FSSpace:(%lld) fsckSpace:%lld(%d) LogSpace:%lld(%d)\n",
        ipmnt, newFSSize, newFSCKAddress, newFSCKSize, newLogAddress, newLogSize));

        /*
         *      quiesce file system
         *
         * (prepare to move the inline log and to prevent map update)
         *
         * block any new transactions and wait for completion of
         * all wip transactions and flush modified pages s.t.
         * on-disk file system is in consistent state and
         * log is not required for recovery.
         */
        quiesceFS(vfsp, ipmnt);

        if (!(ipmnt->i_mntflag & JFS_INLINELOG))
                goto extendBmap;

        /*
         * deactivate old inline log
         */
        lmLogQuiesce(log);

        /*
         * mark on-disk super block for fs in transition;
         *
         * update on-disk superblock for the new space configuration
         * of inline log space and fsck work space descriptors:
         * N.B. FS descriptor is NOT updated;
         *
         * crash recovery:
         * logredo(): if EXTENDFS_WIP, return to fsck() for cleanup;
         * fsck(): if EXTENDFS_WIP, reformat inline log and fsck workspace
         * from superblock inline log descriptor and fsck workspace
         * descriptor;
         */
        /* read in superblock */
        if (rc = readSuper(ipmnt, &sbp))
                goto out;
        sb = (struct superblock *)(sbp->b_bdata);

        /* mark extendfs() in progress */
        sb->s_state |= FM_EXTENDFS;
        sb->s_xsize = newFSSize << ipmnt->i_l2bfactor;
        PXDaddress(&sb->s_xfsckpxd, newFSCKAddress);
        PXDlength(&sb->s_xfsckpxd, newFSCKSize);
        PXDaddress(&sb->s_xlogpxd, newLogAddress);
        PXDlength(&sb->s_xlogpxd, newLogSize);

        /* synchronously update superblock */
        if (rc = rawWrite(ipmnt, sbp, 0))
                goto out;

        /*
         * format new inline log synchronously;
         *
         * crash recovery: if log move in progress,
         * reformat log and exit success;
         */
        if (rc = lmLogFormat(ipmnt, newLogAddress, newLogSize))
                goto error_out;

        /*
         * activate new log
         */
        PXDaddress(&ipmnt->i_logpxd, newLogAddress);
        PXDlength(&ipmnt->i_logpxd, newLogSize);
        if (rc = lmLogResume(log, ipmnt))
                goto error_out;

        /*
         *      extend block allocation map
         *      ---------------------------
         *
         * extendfs() for new extension, retry after crash recovery;
         *
         * note: both logredo() and fsck() rebuild map from
         * the bitmap and configuration parameter from superblock
         * (disregarding all other control information in the map);
         *
         * superblock:
         *  s_size: aggregate size in LVM blocks;
         */
        /*
         *      compute the new block allocation map configuration
         *
         * map dinode:
         *  di_size: map file size in byte;
         *  di_nblocks: number of blocks allocated for map file;
         *  di_mapsize: number of blocks in aggregate (covered by map);
         * map control page:
         *  db_mapsize: number of blocks in aggregate (covered by map);
         */
        newMapSize = newFSSize;
        /* number of data pages of new bmap file:
         * roundup new size to full dmap page boundary and
         * add 1 extra dmap page for next extendfs()
         */
        t64 = (newMapSize - 1) + BPERDMAP;
        newNpages = BLKTODMAPN(t64) + 1;

        /*
         *      extend map from current map (WITHOUT growing mapfile)
         *
         * map new extension with unmapped part of the last partial
         * dmap page, if applicable, and extra page(s) allocated
         * at end of bmap by mkfs() or previous extendfs();
         */
extendBmap:
        /* compute number of blocks requested to extend */
        mapSize = mp->db_mapsize;
        XAddress = mapSize; /* eXtension Address */
        XSize = newMapSize - mapSize; /* eXtension Size */

        /* compute number of blocks that can be extended by current mapfile */
        t64 = dbMapFileSizeToMapSize(ipbmap);
        if (mapSize > t64)
        {
                jEVENT(1,("jfs_extendfs: mapSize (0x%llx) > t64 (0x%llx)\n",
                        mapSize, t64));
                rc = EIO;
                goto error_out;
        }
        nblocks = MIN(t64 - mapSize, XSize);

        /*
         * update map pages for new extension:
         *
         * update/init dmap and bubble up the control hierarchy
         * incrementally fold up dmaps into upper levels;
         * update bmap control page;
         */
        if (rc = dbExtendFS(ipbmap, XAddress, nblocks))
                goto error_out;
        /*
         * the map now has extended to cover additional nblocks:
         * dn_mapsize = oldMapsize + nblocks;
         */
        /* ipbmap->i_mapsize += nblocks; */
        XSize -= nblocks;

        /*
         *      grow map file to cover remaining extension
         *      and/or one extra dmap page for next extendfs();
         *
         * allocate new map pages and its backing blocks, and
         * update map file xtree
         */
        /* compute number of data pages of current bmap file */
        nPages = ipbmap->i_size >> L2PSIZE;

        /* need to grow map file ? */
        if (nPages == newNpages)
                goto updateImap;

        /*
         * grow bmap file for the new map pages required:
         *
         * allocate growth at the start of newly extended region;
         * bmap file only grows sequentially, i.e., both data pages
         * and possibly xtree index pages may grow in append mode,
         * s.t. logredo() can reconstruct pre-extension state
         * by washing away bmap file of pages outside s_size boundary;
         */
        /*
         * journal map file growth as if a regular file growth:
         * (note: bmap is created with di_mode = IFJOURNAL|IFREG);
         *
         * journaling of bmap file growth is not required since
         * logredo() do/can not use log records of bmap file growth
         * but it provides careful write semantics, pmap update, etc.;
         */
        /* synchronous write of data pages: bmap data pages are
         * cached in meta-data cache, and not written out
         * by txCommit();
         */
        bmInodeWrite(ipbmap);

        txBegin(ipmnt, &tid, 0);

        newPage = nPages; /* first new page number */
        xoff = newPage << ipmnt->i_l2nbperpage;
        xlen = (newNpages - nPages) << ipmnt->i_l2nbperpage;
        xlen = MIN(xlen, nblocks) & ~(ipmnt->i_nbperpage - 1);
        xaddr = XAddress;
        if (rc = xtAppend(tid, ipbmap, 0, xoff, nblocks, &xlen, &xaddr, 0))     /* @GD1 */
                goto error_out;
        /* update bmap file size */
        ipbmap->i_size += xlen << ipmnt->i_l2bsize;
        ipbmap->i_nblocks += xlen;

        imark(ipbmap, ICHG|IUPD);

        iplist[0] = ipbmap;
        rc = txCommit(tid, 1, &iplist[0], COMMIT_FORCE);

        txEnd(tid);

        if (rc)
                goto error_out;

        /*
         * map file has been grown now to cover extension to further out;
         * di_size = new map file size;
         *
         * if huge extension, the previous extension based on previous
         * map file size may not have been sufficient to cover whole extension
         * (it could have been used up for new map pages),
         * but the newly grown map file now covers lot bigger new free space
         * available for further extension of map;
         */
        /* any more blocks to extend ? */
        if (XSize)
                goto extendBmap;

        /* finalize bmap */
        dbFinalizeBmap(ipbmap);

        /*
         *      update inode allocation map
         *      ---------------------------
         *
         * move iag lists from old to new iag;
         * agstart field is not updated for logredo() to reconstruct
         * iag lists if system crash occurs.
         * (computation of ag number from agstart based on agsize
         * will correctly identify the new ag);
         */
updateImap:
        /* if new AG size the same as old AG size, done! */
        if (mp->db_agsize == sb->s_agsize)
                goto finalize;

        if (rc = diExtendFS(ipimap, ipbmap))
                goto error_out;

        /* finalize imap */
        if (rc = diSync(ipimap))
                goto error_out;

        /*
         *      finalize
         *      --------
         *
         * extension is committed when on-disk super block is
         * updated with new descriptors: logredo will recover
         * crash before it to pre-extension state;
         */
finalize:
        /* sync log to skip log replay of bmap file growth transaction; */
        /* lmLogSync(log, 1); */

        /*
         * synchronous write bmap global control page;
         * for crash before completion of write
         * logredo() will recover to pre-extendfs state;
         * for crash after completion of write,
         * logredo() will recover post-extendfs state;
         */
        if (rc = dbSync(ipbmap))
                goto error_out;

        /*
         * copy primary bmap inode to secondary bmap inode
         * Not forgetting to reset di_ixpxd since they are in different
         * inode extents.
         */
        t64 = addressPXD(&sb->s_ait2) << ipmnt->i_l2bsize;
        if (rc = rawRead(ipmnt, t64, &bp))
                goto error_out;
        dip = (dinode_t *)bp->b_bdata;
        dip += BMAP_I;
        memcpy(dip, &ipbmap->i_dinode, DISIZE);
        memcpy((void *)&(dip->di_ixpxd), (void *)&(sb->s_ait2), sizeof(pxd_t) );
        if (rc = rawWrite(ipmnt, bp, 1))
                goto error_out;
        /*
         *      update superblock
         */
#ifdef  _JFS_FYI
        uint32  s_flag;         /* 4: aggregate attributes */
        uint32  s_state;        /* 4: mount/unmount/recovery state: */
        int64   s_size;         /* 8: aggregate size in LV blocks */
        uint32  s_agsize;       /* 4: allocation group size in aggr. blocks */
        pxd_t   s_logpxd;       /* 8: inline log extent */
        pxd_t   s_fsckpxd;      /* 8: inline fsck work space extent */
#endif  /* _JFS_FYI */

        /* mark extendfs() completion */
        sb->s_state &= ~FM_EXTENDFS;
        sb->s_size = mp->db_mapsize << ipmnt->i_l2bfactor;
        sb->s_agsize =  mp->db_agsize;

        /* update inline log space descriptor */
        if (sb->s_flag & JFS_INLINELOG)
        {
                PXDaddress(&(sb->s_logpxd), newLogAddress);
                PXDlength(&(sb->s_logpxd), newLogSize);
        }

        /* record log's mount serial number */
        sb->s_logserial = ((log_t *)ipmnt->i_iplog)->serial;

        /* update fsck work space descriptor */
        PXDaddress(&(sb->s_fsckpxd), newFSCKAddress);
        PXDlength(&(sb->s_fsckpxd), newFSCKSize);
        sb->s_fscklog = 1;
        /* sb->s_fsckloglen remains the same */

        /* write primary superblock */
        rawWrite(ipmnt, sbp, 0);

        /* write 2ndary superblock */
        sbp->cm_blkno = SUPER2_OFF >> ipmnt->i_l2pbsize;
        rawWrite(ipmnt, sbp, 1);

        goto out;

error_out:
        sb->s_state = FM_DIRTY;
        (void) rawWrite(ipmnt, sbp, 1);

out:
        /*
         *      resume file system transactions
         */
        resumeFS(vfsp, ipmnt);

        IWRITE_UNLOCK(ipmnt);


#ifdef  _JFS_OS2
        /* copyout parameter list */
        pList.LVSize = newLVSize << ipmnt->i_l2bfactor;
        pList.LogSize = newLogSize << ipmnt->i_l2bfactor;
        pList.FSSize = newFSSize << ipmnt->i_l2bfactor;
        KernCopyOut(pData, &pList, lenData);
#endif  /* _JFS_OS2 */

jEVENT(0,("jfs_extendfs: rc=%d\n", rc));
        return rc;
}


/*
 *      quiesceFS()
 *
 * function: disable transactions for the specified file system;
 *
 * serialization: ipmnt->i_rdwrlock held on entry/exit;
 */
static void quiesceFS(
        struct vfs      *vfsp,
        inode_t *ipmnt)
{
        /* block sync daemon for the file system */
        /* IWRITE_LOCK_TRY(ipmnt); */

        /*
         * all user transactions have been completed and
         * any new user transaction will be blocked;
         * user may still invoke vnode ops and inodes may be locked
         * but no update operation is/will be in progress;
         * note: write() will do txBegin(TX_ANONYMOUS)/txEnd(TX_ANONYMOUS)
         * to serialize with txQuiesce() without allocation/free of
         * txblock in case write() will allocate new file system blocks;
         */

        /*
         * commit modified objects of the file system:
         * regular files which has been modified but have not
         * been committed
         * (all dirty data pages will be synchronously flushed and
         * initiate page out for meta-data pages when committed);
         */
        iQuiesce(vfsp);

        /* write out any remaining homeok meta-data pages
         * (committed by transactions before iQuiesce()), and
         * map files (map pages are always homeok);
         */
        bmLogSync((log_t *)ipmnt->i_iplog);

        /* wait for i/o completion of all flushed pages; */
        bmSync(ipmnt);

        /*
         * all pages of the file system are clean;
         */
}


/*
 *      resumeFS()
 *
 * function: resume transactions for the specified file system;
 */
static void resumeFS(
        struct vfs      *vfsp,
        inode_t *ipmnt)
{
        iResume(vfsp);
}

/*
 *      hard_quiesce()
 *
 * function: disable all I/O for the specified file system;
 */
int32   hard_quiesce(
struct vfs      *vfsp)
{
        inode_t *ipmnt;
        cmdev_t *cdp;
        int32   ipri;

        ipmnt = ((inode_t *)vfsp->vfs_data)->i_ipmnt;

        IWRITE_LOCK(ipmnt);
        cdp = ipmnt->i_cachedev;

        if (cdp->cd_flag & CD_QUIESCE)
        {
                /* We can't nest these things */
                IWRITE_UNLOCK(ipmnt);
                return ERROR_NOT_READY;
        }
        quiesceFS(vfsp, ipmnt);

        ipri = IOCACHE_LOCK();
        cdp->cd_flag |= CD_QUIESCE;
        while (cdp->cd_pending_requests)
                IOCACHE_SLEEP(&cdp->cd_iowait, 0);
        IOCACHE_UNLOCK(ipri);

        IWRITE_UNLOCK(ipmnt);

        return NO_ERROR;
}

/*
 *      hard_resume()
 *
 * function: re-enable all I/O for the specified file system;
 */
int32   hard_resume(
struct vfs      *vfsp)
{
        inode_t *ipmnt;
        cmdev_t *cdp;
        int32   ipri;

        ipmnt = ((inode_t *)vfsp->vfs_data)->i_ipmnt;

//      IWRITE_LOCK(ipmnt);
        WRITE_LOCK(&ipmnt->i_rdwrlock);

        cdp = ipmnt->i_cachedev;

        if ((cdp->cd_flag & CD_QUIESCE) == 0)
        {
                /* Drive has not been quiesced! */
                IWRITE_UNLOCK(ipmnt);
                return ERROR_INVALID_FUNCTION;
        }

        ipri = IOCACHE_LOCK();
        cdp->cd_flag &= ~CD_QUIESCE;
        IOCACHE_WAKEUP(&cdp->cd_quiesce_event);
        IOCACHE_UNLOCK(ipri);

        resumeFS(vfsp, ipmnt);

        IWRITE_UNLOCK(ipmnt);

        return NO_ERROR;
}
