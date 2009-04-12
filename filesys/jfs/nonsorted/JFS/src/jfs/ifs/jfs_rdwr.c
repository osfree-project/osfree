/* $Id: jfs_rdwr.c,v 1.2 2003/09/21 08:42:58 pasha Exp $ */

static char *SCCSID = "@(#)1.27.1.2  12/2/99 11:13:34 src/jfs/ifs/jfs_rdwr.c, sysjfs, w45.fs32, fixbld";
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
 */

/*
 * Change History :
 * PS 2001-2003 apply IBM fixes
 *
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#include "jfs_util.h"
#endif  /* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_cachemgr.h"
#include "jfs_xtree.h"
#include "jfs_txnmgr.h"
#include "jfs_proto.h"
#include "jfs_dio.h"
#include "jfs_dasdlim.h"                                        // F226941
#include "jfs_debug.h"

/*
 * external references
 */
int32   extAlloc(inode_t *ip, int64 xlen, int64 pno, xad_t *xp, boolean_t abnr);
int32   extFill(inode_t *ip, cbuf_t *cp);
int32   extHint(inode_t *ip, int64 offset, xad_t *xp);
int32   extRealloc(inode_t *ip, cbuf_t *cp, int64 nxlen, xad_t *xp,
                   boolean_t abnr);
int32   extRecord(inode_t *ip, cbuf_t *cp);

extern int32    LazyOff;        /* TRUE if lazy write disabled */
extern int32    TlocksLow;      /* TRUE if running out of transaction locks */
extern int32    SyncRunning;    /* PS21092003 */

/*
 * forward references
 */
static int32    PromoteI(inode_t *ip, int64 offset);
static void     WRClean(inode_t *ip, int64 oldsize, boolean_t bmoved,
                        boolean_t overalloc);
static int32    SyncDataI(inode_t *ip, int64 offset, int64 nbytes);
static int32    jfs_read_cached(inode_t *ip, int64 offset, caddr_t pdata,
                                int64 *plen);


/*
 * NAME:        jfs_read_direct()
 *
 * FUNCTION: Do direct I/O on the specified file
 *
 * PRECONDITIONS: Caller already adjusted the number of bytes to make sure we
 *      aren't trying to read past the end of the file.  Caller already took
 *      read lock on inode.
 *
 * PARAMETERS:
 *      ip      - Inode of file to be read
 *      offset  - Starting byte offset in file of read request
 *      pdata   - Starting address of the destination of the read
 *      plen    - Number of bytes to be read; On return, filled in with number
 *                of bytes actually read
 *
 * NOTES: Will do direct I/O on any pieces of the file we can.  Others will go
 *      through jfs_read_cached().
 *
 * RETURNS:
 *      0 : success
 *      other : errors from subroutines
 */
static int32
jfs_read_direct(inode_t *ip,
                int64   offset,
                caddr_t pdata,
                int64   *plen)
{
        int64   nbytes, fbytes, mbytes, lbytes;
        int32   rc, rc2;
        dio_t   *dp;
        int64   off, m, bcount, blkno, nblks, xoff;
        lxdlist_t       lxdl;
        lxd_t           lxd;
        xadlist_t       xadl;
        xad_t           xad;
        boolean_t       direct = TRUE;
        int64           direct_len, cur_len, coff;
        int64           baddr, eoff;
        int32           pbsize, l2bsize, l2bfactor, l2pbsize;

jEVENT(0,("Read Direct: offset: %08x %08x\tpdata: %08x\tlength: %08x %08x\n",
        offset, pdata, *plen));

        pbsize = ip->i_ipmnt->i_pbsize;
        l2bsize = ip->i_ipmnt->i_l2bsize;
        l2pbsize = ip->i_ipmnt->i_l2pbsize;
        l2bfactor = ip->i_ipmnt->i_l2bfactor;
        INODE_LOCK(ip);
        if (ip->i_ra_lock == 0)
                ip->i_rapage = -1;
        INODE_UNLOCK(ip);

        /* At end of processing nbytes should be equal to number of bytes not
         * read
         */
        nbytes = *plen;

        /* First determine if there is a piece of the read which does not
         * completely cover a physical block.  If so, we will use the normal
         * cached read to read the logical page of the file which contains this
         * block.  (Since the cached read will bring the entire logical page
         * into the cache.)  fbytes is the number of bytes in the front of the
         * read which need to be read in with a cached read.  lbytes is the
         * number of bytes at the end of the read which need to be read in with
         * a cached read.  mbytes is the number of bytes in the middle of the
         * read to try reading with direct I/O.
         */
        fbytes = (offset & (pbsize - 1)) ?
                        MIN(nbytes,CM_BSIZE - (offset & CM_OFFSET)) : 0;

        coff = offset + nbytes;
        lbytes = ((coff & (pbsize - 1)) && fbytes != nbytes) ?
                        MIN(coff & CM_OFFSET, nbytes-fbytes) :  0;

        mbytes = nbytes - (fbytes + lbytes);

        if (mbytes == 0 || nbytes < pbsize)
        {
                /* Everything to be read should be a cached read.
                 */
                return(jfs_read_cached(ip,offset,pdata,plen));
        }

        if (fbytes)
        {
                /* Do a cached read of the logical page of the file containing
                 * the initial incomplete physical block to be read.
                 */
                rc = jfs_read_cached(ip,offset,pdata,&fbytes);

                nbytes -= fbytes;

                if (rc)
                        goto out;

                offset += fbytes;
                pdata += fbytes;
        }

        /* Setup direct I/O data structure
         */
        if (rc = dioStart(ip, &dp))
        {
                goto out;
        }

        /* Read the middle part of the file, doing direct I/O whenever possible
         */
        lxdl.lxd = &lxd;
        lxdl.maxnlxd = 1;
        lxdl.nlxd = 1;
        xadl.maxnxad = 1;
        xadl.xad = &xad;
        for (off = offset, m = mbytes; m > 0; off += bcount, pdata += bcount,
                                                                 m -= bcount)
        {
                blkno = off >> l2bsize;
                nblks = (m + ip->i_ipmnt->i_bsize - 1) >> l2bsize;

                LXDoffset(&lxd, blkno);
                LXDlength(&lxd, nblks);

                xadl.nxad = 0;
                if (rc = xtLookupList(ip, &lxdl, &xadl, 0))
                {
                        /* We had some I/O error with the lookup, will have to
                         * give up
                         */
                        bcount = 0;
                        break;
                }

                if (xadl.nxad == 0)
                {
                        /* There wasn't a disk address for this block, that
                         * means we have a file hole.  We will have to go the
                         * normal route and do a cached read of this.
                         */
                        assert(ISSPARSE(ip));
                        direct = FALSE;
                        bcount = m;
                }
                else
                {
                        xoff = offsetXAD(&xad);

                        if (xoff > blkno)
                        {
                                /* There was a hole in the file, we need to
                                 * handle the hole using a cached read.  But we
                                 * can read the rest of the request with a
                                 * direct read.
                                 */
                                assert(ISSPARSE(ip));
                                direct = FALSE;
                                cur_len = (xoff << l2bsize) - off;
                                bcount = MIN(cur_len,m);
                        }
                        else
                        {
                                /* We found a disk address for this part of the
                                 * file; we need to make sure this part of the
                                 * file has actually been written to disk and
                                 * that it is not already in our cache before
                                 * doing the direct read.
                                 *
                                 * First, we need to handle that the extent
                                 * might describe blocks of the file earlier
                                 * than our first offset we are trying to read
                                 * now.
                                 */
                                direct_len = off - (xoff << l2bsize);
                                cur_len = (lengthXAD(&xad) << l2bsize)
                                                - direct_len;
                                cur_len = MIN(cur_len,m);

                                if (xad.flag & XAD_NOTRECORDED)
                                {
                                        direct = FALSE;
                                        bcount = cur_len;
                                }
                                else
                                {
                                        /* Check if the buffer is in the cache;
                                         * if so we have to do a cached read
                                         * cmQueryCache() returns the number of
                                         * bytes not cached starting at the
                                         * specified offset, up to the length
                                         * specified.
                                         */
                                        if (bcount = cmQueryCache(ip, off,
                                                                  cur_len))
                                        {
                                                direct = TRUE;
                                        }
                                        else
                                        {
                                                /* Since our request may not be
                                                 * on a page boundary, we need
                                                 * to adjust our byte count to
                                                 * the logical page size in the
                                                 * cache.
                                                 */
                                                direct = FALSE;
                                                bcount = MIN(cur_len,
                                        CM_BSIZE - (CM_OFFSET & (uint32)off));
                                        }
                                }
                        }
                }

                if (direct == TRUE)
                {
                        /* We found a segment we can read by direct I/O
                         */
                        baddr = (addressXAD(&xad) << l2bfactor)
                                                 + (direct_len >> l2pbsize);

                        if (rc = dioRead(dp,baddr,bcount,off,pdata))
                        {
                                bcount = 0;
                                break;
                        }
                }
                else
                {
                        if (rc = jfs_read_cached(ip,off,pdata,&bcount))
                                break;
                }
        }

        /* On failure eoff is filled in with byte offset of first failing block
         */
        rc2 = dioEnd(dp,&eoff);

        if (rc2)
        {
                nbytes -= (eoff - offset);
                rc = rc2;
        }
        else if (rc)
        {
                nbytes -= ((off - offset) + bcount);
        }
        else
        {
                nbytes -= mbytes;

                if (lbytes)
                {
                        /* Do a cached read of the logical page of the file
                         * containing the last incomplete physical block to be
                         * read.
                         */
                        rc = jfs_read_cached(ip,off,pdata,&lbytes);
                        nbytes -= lbytes;
                }
        }

out:
        /* At this point nbytes should be set to the number of bytes not read
         * from the original request.
         */
        *plen -= nbytes;

        return(rc);
}


/*
 * NAME:        jfs_read_cached()
 *
 * FUNCTION: Perform read using cache
 *
 * PARAMETERS:
 *      ip      - Inode of file to be read
 *      offset  - Starting byte offset in file of read request
 *      pdata   - Starting address of the destination of the read
 *      plen    - Number of bytes to be read; On return, filled in with number
 *                of bytes actually read
 *
 * RETURNS:
 *      0       - success
 *      other   - failures from subroutines
 *
 * serialization: IREAD_LOCK held on entry/exit;
 */
static int32
jfs_read_cached(inode_t *ip,
                int64   offset,
                caddr_t pdata,
                int64   *plen)
{
        int32   rc = 0;
        int64   nbytes, nbrd;
        int32   nb, npages;
        uint32  coff;
        cbuf_t  *cp;

        nbytes = *plen;
        npages = ((offset + nbytes - 1) >> CM_L2BSIZE) -
                        (offset >> CM_L2BSIZE) + 1;

        /* process the request one cache buffer at a time */
        for (nbrd = 0; nbrd < nbytes; nbrd += nb,
                offset += nb, pdata += nb, npages--)
        {
                /* get the cache buffer offset and the number of bytes
                 * to be copied from the buffer.
                 */
                coff = (uint)offset & CM_OFFSET;
                nb = MIN(nbytes - nbrd, CM_BSIZE - coff);

                /* get the cache buffer */
                if (rc = cmRead(ip, offset, npages, &cp))
                        break;

                /* copy out the data */
                rc = copyout(cp->cm_cdata+coff,pdata,nb);

                cmPut(cp,FALSE);

                /* stop if we got an error */
                if (rc)
                        break;
        }

        *plen = nbrd;

        return(rc);
}


/*
 * NAME:        jfs_read()
 *
 * FUNCTION:    entry point for read operations.
 *
 * PARAMETERS:
 *      vp      - the vnode of the file.
 *      offset  - starting byte offset within the inode of the read
 *      pdata   - starting address of the destination of the read
 *      plen    - on input, pointer to the number of bytes to read.
 *                on exit, set with the number of bytes actually read.
 *      flags   - open flags.
 *
 * RETURN VALUES:
 *      0       - success
 *      errors from subroutines.
 *
 *      IREAD_LOCK(IP) must be held on entry.
 */
int32
jfs_read(struct vnode   *vp,
         int64          offset,
         caddr_t        pdata,
         int64          *plen,
         int32          flags)
{
        int32   rc = 0;
        inode_t *ip = VP2IP(vp);
        int64 nbytes, mbytes, chunksize, remaining = 0;                        //230531
        int32 type, l2bsize;                                                   //230531

        /* validate the offset */
        if (offset < 0)
                return(EINVAL);

        /* better be a regular file, directory or symlink */
        type = ip->i_mode & IFMT;
/* ??? directory ? */
        assert(type == IFREG || type == IFDIR || type == IFLNK);

        nbytes = *plen;
        *plen = 0;

        /* adjust the number of bytes to read if the request wants to
         * read beyond end of file.
         */
        nbytes = MIN(nbytes, ip->i_size - offset);

        if(flags & IOFL_NOCACHE)
        {
                /* 230531
                 *
                 * Break up large direct IO requests into smaller chunks. Make sure
                 * that we don't need more than 16 locks simultaneously (between
                 * dioStart and dioEnd in jfs_read_direct)
                 */
                l2bsize = ip->i_ipmnt->i_l2bsize;                              //230531
                chunksize = ISSPARSE(ip) ? ((0x01L<<l2bsize)*16) : 0x100000;   //230531

                for (remaining = nbytes; remaining > 0; remaining -= mbytes, offset += mbytes, pdata += mbytes)    //230531
                {                                                              //230531
                        mbytes = MIN(chunksize, remaining);                    //230531
                        rc = jfs_read_direct(ip, offset, pdata, &mbytes);
                        if (rc)                                                //230531
                        {                                                      //230531
                                remaining -= mbytes;                           //230531
                                break;                                         //230531
                        }                                                      //230531
                }                                                              //230531
        }
        else
        {
                rc = jfs_read_cached(ip, offset, pdata, &nbytes);
        }

        /* mark the inode as accessed */
        INODE_LOCK(ip);
        imark(ip, IACC);
        INODE_UNLOCK(ip);

        /* return the number of bytes read */
        *plen = nbytes - remaining;                                            //230531

        return(rc);
}


/*
 * NAME:        jfs_write_cached()
 *
 * FUNCTION:    write thru cache;
 *
 * PARAMETERS:
 *      vp      - the vnode of the file.
 *      offset  - starting byte offset within the inode of the read
 *      pdata   - starting address of the destination of the read
 *      plen    - on input, pointer to the number of bytes to read.
 *                on exit, set with the number of bytes actually read.
 *      flags   - open flags.
 *
 * RETURN VALUES:
 *      0       - success
 *      errors from subroutines.
 *
 * serialization: IWRITE_LOCK held on entry/exit;
 */
static int32
jfs_write_cached(inode_t        *ip,
                 int64          offset,
                 caddr_t        pdata,
                 int64          *plen,
                 int32          flags)
{
        int32   rc = 0;
        int64 nbytes, newsize, nbc, reqblks, xaddr, asize, oldsize;
        int64 ablks, xlen, off, nbwr;
        int32 endblks, partial, coff, nbperpage, l2bsize;
        int32 nblks, nb, rem, endbytes;
        xad_t xad, *xp = &xad;
        cbuf_t *cp;
        int64   first, last, cllast, wbclust;

        nbwr = nbytes = *plen;
        *plen = 0;

        /* remember the current file size for backout */
        oldsize = ip->i_size;

        /* if the write starts beyond the last byte
         * of the file + 1 (i.e. lseek beyond end of file
         * then write), extend the file size to the starting
         * offset of the write.
         */
        if (offset > ip->i_size)
        {
                if (rc = PromoteI(ip,offset))
                {
                        return(rc);
                }
        }

        /* get the log2 block size and number of blocks per page */
        l2bsize = ip->i_ipmnt->i_l2bsize;
        nbperpage = ip->i_ipmnt->i_nbperpage;

        /* initialize the allocation hint/return value.  the xad
         * pointed to by xp will hold the input allocation hint
         * provided to the extent allocator as well as the results
         * (set by the allocator) of the allocation.
         */
        XADaddress(xp,0);

        /* get the number of blocks backing the last page of the
         * file, the number of bytes backed by these blocks, and
         * indicator of whether the last page is fully backed.
         */
        endblks = CM_BTOBLKS(ip->i_size,ip->i_ipmnt->i_bsize,l2bsize);
        endbytes = endblks << l2bsize;
        partial = (endblks != nbperpage);

        /* compute the ending size of the write */
        newsize = offset + nbytes;
        off = offset;

        /* get the file's current allocation size.
         * this size is defined as part (pages) of the file
         * which can handle the current write request without
         * any new disk allocation required (excluding disk
         * allocation to fill file holes for sparse files).
         */
        asize = (ip->i_size) ? ((ip->i_size-1) & ~CM_OFFSET) + endbytes : 0;

        /* force allocated size to page boundary to defer
         * extending last partial page in step II.
         */
        if (partial && asize < newsize)
                asize &= ~CM_OFFSET;

        /*
         * step I: if any of the current write request is within
         * the file's current allocation size, handle this part
         * of the write request.
         */
        if ((nbc = asize - off) > 0)
        {
                /* loop over the pages (cbufs) for this portion of
                 * the write, allocating disk space (file holes within
                 * sparse file) and copying data as neccessary.
                 */
                for (nbc = MIN(nbc,nbytes); nbc > 0; nbc -= nb,
                        nbytes -= nb, off += nb, pdata += nb)
                {
                        /* get the starting offset within the cbuf at
                         * which data will be copied and the number of
                         * bytes to copy.
                         */
                        coff = (uint)off & CM_OFFSET;
                        nb = MIN(nbc, CM_BSIZE - coff);

                        /* get the cbuf for the current page */
                        if (rc = cmReadWR(ip,off,nb,&cp))
                                goto out;

                        /* does current page represent a file hole ? */
                        if (cp->cm_hole)
                        {
                                /* must be a sparse file to have a hole.
                                 */
                                assert(ISSPARSE(ip));

                                /* allocate disk space for the hole */
/* ??? block allocation per page at a time ? */
                                if (rc = extFill(ip,cp))
                                {
                                        cmPut(cp,FALSE);
                                        goto out;
                                }
                        }

                        /* is current page allocated but not
                         * recorded ?
                         */
                        if (cp->cm_abnr)
                        {
                                /* change the page to allocated and
                                 * recorded.
                                 */
                                if (rc = extRecord(ip,cp))
                                {
                                        cmPut(cp,FALSE);
                                        goto out;
                                }
                        }

                        /* copy in the data to the cbuf */
                        if (rc = copyin(pdata,cp->cm_cdata+coff,nb))
                        {
                                cmPut(cp,FALSE);
                                goto out;
                        }

                        /* remember the disk address of this page in
                         * case we need it for an allocation hint.
                         */
                        xaddr = addressPXD(&cp->cm_pxd);

                        cmPut(cp,TRUE);
                }

                /* adjust the file size if this portion of the
                 * write has extended it.
                 */
                ip->i_size = MAX(ip->i_size,off);

                /* are we done copying data for this write ? */
                if (nbytes == 0)
                        goto out;

                /* more data to write. if the last page of the
                 * file is NOT partial (i.e. new extent allocation
                 * is required), construct the allocation hint
                 * that will be used for the extent allocation.
                 */
                if (!partial)
                {
                        XADaddress(xp,xaddr);
                        XADoffset(xp,((off-1) & ~CM_OFFSET) >> l2bsize);
                        XADlength(xp,nbperpage);
                        xp->flag = 0;
                }
        }

        /* compute number of blocks required to hold the remaining
         * portion of the write.
         */
        reqblks = CM_BTOFBLKS(newsize - (off & ~CM_OFFSET),l2bsize);

        /*
         * step II: if the last page of the file is partially backed,
         * the allocation for this partial page must be extended
         * as part of the write and we'll handle that here
         * along with the data movement for the page.
         */
        if (partial)
        {
                /* get the starting offset within the cbuf at
                 * which data will be copied and the number of
                 * bytes to copy.
                 */
                coff = (uint)off & CM_OFFSET;
                nb = MIN(nbytes, CM_BSIZE - coff);

                /* get the cbuf for the current page */
                if (rc = cmReadWR(ip,off,nb,&cp))
                        goto out;

                /* copy the data to the cbuf */
                if (rc = copyin(pdata,cp->cm_cdata+coff,nb))
                {
                        cmPut(cp,FALSE);
                        goto out;
                }

                /* zero any bytes between the end of the file
                 * and the start of the write.
                 */
                if ((rem = coff - (endbytes)) > 0)
                        bzero(cp->cm_cdata+endbytes,rem);

                /* extend the allocation.
                 * in extending the allocation for the current
                 * (partial) page, we attempt to extend the allocation
                 * such that it is sufficient to handle the remaining
                 * allocation requirements of the current write
                 * request (i.e. we ask extRealloc() to allocate
                 * an extent of reqblks blocks).
                 * extRealloc() may provide us with an extent with
                 * less blocks than we asked for but the number
                 * of block provided should always be sufficient to
                 * cover new requirements of the partial page.
                 */
                if (rc = extRealloc(ip,cp,reqblks,xp,FALSE))
                {
                        coff = (oldsize-1) & CM_OFFSET;
                        bzero(cp->cm_cdata+coff,CM_BSIZE - coff);
                        cmPut(cp,FALSE);
                        goto out;
                }

                /* get the size of the extent allocated and
                 * its starting disk block address.
                 */
                xlen = lengthXAD(xp);
                xaddr = addressXAD(xp);

                /* get the number of blocks required for the current
                 * page.
                 */
                nblks = MIN(nbperpage,xlen);

                /* zero any bytes beyond the end of the write and
                 * up to the end of the backed portion of the cbuf.
                 */
                if (rem = (nblks << l2bsize) - (coff+nb))
                        bzero(cp->cm_cdata+coff+nb,rem);

                /* update the cbuf with the starting address and
                 * number of blocks for the page.
                 */
                PXDaddress(&cp->cm_pxd,xaddr);
                PXDlength(&cp->cm_pxd,nblks);

                cmPut(cp,TRUE);

                /* adjust the file offset, user write buffer address,
                 * and number of bytes written.
                 */
                off += nb;
                pdata += nb;
                nbytes -= nb;

                /* adjust the file size to reflect the bytes written */
                ip->i_size = off;

                /* adjust the number of disk blocks required with
                 * the number of block contained within the newly
                 * allocated extent.
                 */
                reqblks -= xlen;

                /* adjust the newly allocated extents starting address
                 * and number of block remaining to reflect the blocks
                 * used by the current page.
                 */
                xlen -= nblks;
                xaddr += nblks;
        }
        else
                xlen = 0;

        /* get allocation hint if we currently do not have one */
        if (addressXAD(xp) == 0)
        {
                /* get a hint */
                if (rc = extHint(ip,off,xp))
                {
                        WRClean(ip,oldsize,nbwr != nbytes,FALSE);
                        goto out;
                }
        }

        /*
         * step III: handle the portion of the write that is beyond
         * the last page of the file, a page (cbuf) at a time.
         */
        for (ablks = reqblks; nbytes > 0; nbytes -= nb,
                off += nb, pdata += nb, xlen -= nblks, xaddr += nblks)
        {
                /* get the starting offset within the cbuf at
                 * which data will be copied and the number of
                 * bytes to copy.
                 */
                coff = (uint)off & CM_OFFSET;
                nb = MIN(nbytes, CM_BSIZE - coff);

                /* check if we have an extent in hand with free space in
                 * it. if not, allocate a new extent. extAlloc() is used
                 * to allocate the extent and we ask it allocate ablks
                 * blocks.  however, it may allocate an extent with less
                 * than ablks (this many contigious free blocks are not
                 * avaliable) but the extent will always be large enough
                 * to cover the current page.  initially, we ask this
                 * routine to allocate reqblks blocks (i.e. ablks ==
                 * reqblks).  however, if less than reqblks are provided
                 * next allocation request will be for the minimum
                 * of the number of blocks still required and the number
                 * of block (last) provided.
                 */
                if (xlen == 0)
                {
                        /* try to allocate an extent containing ablks blocks.
                         */
                        if (rc = extAlloc(ip,ablks,CM_OFFTOCBLK(off),xp,FALSE))
                        {
                                WRClean(ip,oldsize,nbwr != nbytes,FALSE);
                                break;
                        }

                        /* get the size of the extent allocated and its
                         * starting address.
                         */
                        xlen = lengthXAD(xp);
                        xaddr = addressXAD(xp);

                        /* update the number of required blocks to reflect
                         * the number of blocks just allocated.
                         */
                        reqblks -= xlen;

                        /* update ablks for the next go round */
                        ablks = MIN(reqblks,xlen);
                }

                /* get the cbuf for the new page */
                cp = cmGetE(ip->i_cacheid,CM_OFFTOCBLK(off), CM_WRITE);
                cp->j_ip = ip;

                /* copy in the data */
                if (rc = copyin(pdata,cp->cm_cdata+coff,nb))
                {
                        cmPut(cp,FALSE);
                        WRClean(ip,oldsize,nbwr != nbytes,TRUE);
                        break;
                }

                /* determine how many blocks are required for this page.
                 */
                nblks = MIN(nbperpage,xlen);

                /* zero any bytes between the end of the file
                 * and the start of the write.
                 */
                if (coff)
                        bzero(cp->cm_cdata,coff);

                /* zero any bytes beyond the end of the write and
                 * up to the end of the backed portion of the cbuf.
                 */
                if (rem = (nblks << l2bsize) - (coff+nb))
                        bzero(cp->cm_cdata+coff+nb,rem);

                /* update the cbuf with the starting address and
                 * number of blocks for the page.
                 */
                PXDaddress(&cp->cm_pxd,xaddr);
                PXDlength(&cp->cm_pxd,nblks);

                cmPut(cp,TRUE);

                /* adjust the file size to reflect the data written */
                ip->i_size = off + nb;
        }

out:
        /* determine how many bytes were actually written */
        nbytes = nbwr - nbytes;

        *plen = nbytes;

        /*
         * if file was open O_SYNC and if we actually modified the file,
         * sync the modifications.
         */
        if (flags & FSYNC)
        {
                if (nbytes || ip->i_size != oldsize)
                        rc = SyncDataI(ip,offset,nbwr);
                return rc;
        }

        if (nbytes == 0)
                return rc;

        /*
         *      write-behind:
         *
         * initiate async write of all pages outside of last cluster
         * of current write;
         */
        first   = offset >> CM_L2BSIZE; /* first page of current write */
        last = (offset + nbytes - 1) >> L2PSIZE; /* last page of current write */
        cllast = last & ~(CM_WRCLNBLKS - 1);

        /* If Lazywrite is turned off, asynchronously write the data
         */
        if (LazyOff)
        {
                cmAsyncWrite(ip->i_cacheid, first, last-first+1);
                ip->i_wrbehind = FALSE;
                ip->i_wbpage = last;
                return rc;
        }

        /*
         * sequential write:
         *
         * either the current write starts in the same block
         * as the previous write and continues into another block,
         * or the write starts in the contiguous block to
         * the previous write;
         */
        if ((first == ip->i_wbpage && last != ip->i_wbpage) ||
            (first == ip->i_wbpage + 1))
        {
                wbclust = ip->i_wbpage & ~(CM_WRCLNBLKS - 1);

                if ((ip->i_wrbehind) &&    /* prior behavior was sequential */
                    (cllast > wbclust))    /* write extends past last cluster*/
                {
jEVENT(0,("wrbehind: s start:%d%d n:%d%d\n", ip->i_wbpage, cllast - wbclust));
                        /* init async write of preceding cluster(s) */
                        cmAsyncWrite(ip->i_cacheid, wbclust,
                                     (int64)(cllast - wbclust));
                }
                ip->i_wrbehind = TRUE;
        }
        else if (first == ip->i_wbpage)         /* write within same page */
                ip->i_wrbehind = TRUE;
        else
                ip->i_wrbehind = FALSE;

        ip->i_wbpage = last;

        return(rc);
}


/*
 * NAME:        jfs_write_direct()
 *
 * FUNCTION:    write direct
 *
 * PARAMETERS:
 *      vp      - the vnode of the file.
 *      offset  - starting byte offset within the inode of the read
 *      pdata   - starting address of the destination of the read
 *      plen    - on input, pointer to the number of bytes to read.
 *                on exit, set with the number of bytes actually read.
 *      flags   - open flags.
 *
 * RETURN VALUES:
 *      0       - success
 *      errors from subroutines.
 */
static int32
jfs_write_direct(inode_t        *ip,
                 int64          offset,
                 caddr_t        pdata,
                 int64          *plen,
                 int32          flags)
{
        int64   nbytes, fbytes, mbytes, lbytes;
        int32   rc, rc2;
        dio_t   *dp;
        int64   off, m, bcount, blkno, nblks, xoff;
        lxdlist_t       lxdl;
        lxd_t           lxd;
        xadlist_t       xadl;
        xad_t           xad, hint_xad, *xp = &hint_xad;
        boolean_t       direct = TRUE;
        int64           direct_len, cur_len, coff;
        int64           baddr, eoff;
        int32           partial;
        int64           endsize, bcount1;                                     //PS21092003
        int32           l2pbsize, l2bsize, pbsize, l2bfactor;
        boolean_t       abnr;

jEVENT(0,("Write Direct: offset: %08x %08x\tpdata: %08x\tlength: %08x %08x\n",
        offset, pdata, *plen));
        l2pbsize = ip->i_ipmnt->i_l2pbsize;
        l2bsize = ip->i_ipmnt->i_l2bsize;
        pbsize = ip->i_ipmnt->i_pbsize;
        l2bfactor = ip->i_ipmnt->i_l2bfactor;
        nbytes = *plen;
        /* Extend the file to the new size.
         */
        if ( (offset + *plen > ip->i_size) && (!ISSPARSE(ip)) )        //230531
        {
                if (rc = ExtendI(ip, offset+*plen, TRUE))
                {
                        *plen -= nbytes;                               //PS21092003
                        return(rc);
                }
        }

        /* At end of processing, nbytes should be equal to number of bytes not
         * written
         */
//PS21092003        nbytes = *plen;

        /* First determine if there is a piece of the write which does not
         * completely cover a physical block.  If so, we will use the normal
         * cached write to write the logical page of the file which contains
         * this physical block.  (Since the cached write will bring the entire
         * logical page into the cache.)  fbytes is the number of bytes in the
         * front of the write which need to be written with a cached write.
         * lbytes is the number of bytes at the end of the write which need to
         * be written with a cached write.  mbytes is the number of bytes in the
         * middle of the write to try writing with direct I/O.
         */
        fbytes = (offset & (pbsize - 1)) ?
                        MIN(nbytes,CM_BSIZE - (offset & CM_OFFSET)) : 0;

        coff = offset + nbytes;
        lbytes = ((coff & (pbsize - 1)) && fbytes != nbytes) ?
                        MIN(nbytes-fbytes,coff & CM_OFFSET) :  0;

        mbytes = nbytes - (fbytes + lbytes);

        if (mbytes == 0 || nbytes < pbsize)
        {
                /* Everything to be written should be a cached write.
                 */
                return(jfs_write_cached(ip,offset,pdata,plen,flags));
        }

        if (fbytes)
        {
                /* Do a cached write of the first logical page of the request
                 * containing an initial incomplete physical block of the write.
                 */
                rc = jfs_write_cached(ip,offset,pdata,&fbytes,flags);

                nbytes -= fbytes;

                if (rc)
                        goto out;

                offset += fbytes;
                pdata += fbytes;
        }

        /* Setup direct I/O data structure
         */
        if (rc = dioStart(ip, &dp))
        {
                goto out;
        }

        /* Write middle part of the request; doing direct I/O whenever possible
         */
        lxdl.lxd = &lxd;
        lxdl.maxnlxd = 1;
        lxdl.nlxd = 1;
        xadl.maxnxad = 1;
        xadl.xad = &xad;

        for (off = offset, m = mbytes; m > 0; off += bcount, pdata += bcount,
                                                                 m -= bcount)
        {
                blkno = off >> l2bsize;
                nblks = (m + ip->i_ipmnt->i_bsize - 1) >> l2bsize;

                LXDoffset(&lxd, blkno);
                LXDlength(&lxd, nblks);

                xadl.nxad = 0;
                if (rc = xtLookupList(ip, &lxdl, &xadl, 0))
                {
                        /* We had some I/O error with the lookup, will have to
                         * give up
                         */
                        bcount = 0;
                        break;
                }

                if (xadl.nxad == 0)
                {
                        /* There wasn't a disk address for this block, that
                         * means we have a file hole.  We will have to go the
                         * normal route and do a cached write of this.
                         */
                        assert(ISSPARSE(ip));
                        direct = FALSE;
                        bcount = m;
                }
                else
                {
                        xoff = offsetXAD(&xad);

                        if (xoff > blkno)
                        {
                                /* There was a hole in the file, we need to
                                 * handle the hole using a cached write.  But we
                                 * can write the rest of the request with a
                                 * direct write.
                                 */
                                assert(ISSPARSE(ip));
                                direct = FALSE;
                                cur_len = (xoff << l2bsize) - off;
                                bcount = MIN(cur_len,m);
                        }
                        else
                        {
                                /* We found a disk address for this part of the
                                 * file; we need to make sure this part of the
                                 * file has actually been written to disk and
                                 * that it is not already in our cache before
                                 * doing the direct write.
                                 *
                                 * First, we need to handle that the extent
                                 * might describe blocks of the file earlier
                                 * than our first offset we are trying to write
                                 * now.
                                 */
                                direct_len = off - (xoff << l2bsize);
                                cur_len = (lengthXAD(&xad) << l2bsize)
                                                - direct_len;
                                cur_len = MIN(cur_len,m);
//PS21092003 Begin
                                if (bcount = cmQueryCache(ip, off,                          
                                                          cur_len))
                                {
                                    //bcount has the number of bytes starting from off
                                    //that is not in cache

                                    //if abnr, we write direct only integral number of pages
                                    //if less than a logical page, write cached

                                    if (xad.flag & XAD_NOTRECORDED)    
                                      {                               // extent is not recorded
                                      bcount1 = bcount;             // hold bcount in case it's < 1 page
                                      bcount = ((bcount1 >> CM_L2BSIZE) << CM_L2BSIZE);
                                      coff = off & CM_OFFSET;
                                       if (bcount && (coff == 0))
                                        {
                                                direct = TRUE;
                                                abnr   = TRUE;
                                        }
                                        else
                                        {
                                                direct = FALSE;
                                                bcount = MIN(bcount1,       //less than one logical page
                                                  CM_BSIZE - (off & CM_OFFSET));
                                        }
                                    }

                                    else
                                    {                                          //extent is recorded
                                      direct = TRUE;
                                      abnr   = FALSE;
                                    }                                       

                                }
                                else
                                {                                           //cache hit
                                    direct = FALSE;
                                    bcount = MIN(cur_len,
                                    CM_BSIZE - (CM_OFFSET & (uint32)off));
                                }
/*PS21092003                                if (xad.flag & XAD_NOTRECORDED)
                                {
                                        /* If xoff lies on a logical page
                                         * boundary, and cur_len is at least a
                                         * logical page long, then use direct
                                         * write for that piece.  Otherwise use
                                         * a cached write.
                                         *
                                        bcount = (cur_len >> CM_L2BSIZE)
                                                        << CM_L2BSIZE;
                                        coff = off & CM_OFFSET;
                                        if (bcount && (coff == 0))
                                        {
                                                direct = TRUE;
                                                abnr = TRUE;
                                        }
                                        else
                                        {
                                                direct = FALSE;
                                                bcount = MIN(cur_len,
                                                  CM_BSIZE - (off & CM_OFFSET));
                                        }
                                }
                                else
                                {
                                        /* Check if the buffer is in the cache;
                                         * if so we have to do a cached write
                                         * cmQueryCache() returns the number of
                                         * bytes not cached starting at the
                                         * specified offset, up to the length
                                         * specified.
                                         * 
                                        if (bcount = cmQueryCache(ip, off,
                                                                  cur_len))
                                        {
                                                direct = TRUE;
                                                abnr = FALSE;
                                        }
                                        else
                                        {
                                                direct = FALSE;
                                                bcount = MIN(cur_len,
                                        CM_BSIZE - (CM_OFFSET & (uint32)off));
                                        }
                                } PS21092003 */
                        }
                }

                if (direct == TRUE)
                {
                        /* We found a segment we can write by direct I/O
                         */
                        baddr = (addressXAD(&xad) << l2bfactor)
                                                 + (direct_len >> l2pbsize);

                        /* dioWrite() will handle updating the extent
                         * information for the ABNR pages.
                         */
                        if (rc = dioWrite(dp,baddr,bcount,off,pdata,abnr))
                        {
                                bcount = 0;
                                break;
                        }
                }
                else
                {
                        if (rc = jfs_write_cached(ip,off,pdata,&bcount,flags))
                                break;
                }
        }

        /* On failure eoff is filled in with byte offset of first failing block
         */
        rc2 = dioEnd(dp,&eoff);

        if (rc2)
        {
                nbytes -= (eoff - offset);
                rc = rc2;
        }
        else if (rc)
        {
                nbytes -= ((off - offset) + bcount);
        }
        else
        {
                nbytes -= mbytes;

                if (lbytes)
                {
                        rc = jfs_write_cached(ip,off,pdata,&lbytes,flags);
                        nbytes -= lbytes;
                }
        }

out:
        /* At this point nbytes should be set to the number of bytes not written
         * from the original request.
         */
        *plen -= nbytes;

        return(rc);
}


/*
 * NAME:        jfs_write()
 *
 * FUNCTION:    entry point for write operations.
 *
 * PARAMETERS:
 *      vp      - the vnode of the file.
 *      offset  - starting byte offset within the inode of the write
 *      pdata   - starting address of the source of the write
 *      plen    - on input, pointer to the number of bytes to write.
 *                on exit, set with the number of bytes actually write.
 *      flags   - open flags.
 *
 * RETURN VALUES:
 *      0       - success
 *      errors from subroutines.
 *
 *   IWRITE_LOCK(IP) must be held on entry
 */
int32
jfs_write(struct vnode  *vp,
          int64         offset,
          caddr_t       pdata,
          int64         *plen,
          int32         flags)
{
        int32   rc = 0;
        inode_t *ip = VP2IP(vp);
        int64 orig_nblocks;                                     // F226941
        int64 nbytes, mbytes, chunksize, remaining = 0;                        //230531
        int32 type, l2bsize;                                                   //230531
        int32   rc2 = 0;                                                       //PS21092003

        /* validate the offset */
        if (offset < 0)
                return(EINVAL);


        /* must be a regular file or a symbolic link */
        type = ip->i_mode & IFMT;
        assert(type == IFREG || type == IFLNK);

        nbytes = *plen;
// PS21092003 Begin
// in legacy open mode, make sure we don't write > 2gb-1 bytes
        if ((ip->i_lgcnt > 0) && ((offset+nbytes) > 0x7fffffff)) 
           {   
           nbytes = 0x7fffffff - offset;          
           *plen = nbytes;                 
          }                       
//PS End

        /* any work to do ? */
        if (nbytes <= 0)
        {
                return(0);
        }

        if (isReadOnly(ip))
                return EROFS;

// BEGIN D233382
#ifdef _JFS_LAZYCOMMIT
//PS21092003        if (offset+nbytes > ip->i_size)
                /*
                 * Block here if we are constrained, before we allocate tlocks
                 */
                txBeginAnon(ip->i_ipmnt);
#endif /* _JFS_LAZYCOMMIT */
// END D233382

        orig_nblocks = ip->i_nblocks;                           // F226941

        if(flags & IOFL_NOCACHE)
        {
                /* 230531
                 *
                 * Break up large direct IO requests into smaller chunks. Make sure
                 * that we don't need more than 16 locks simultaneously (between
                 * dioStart and dioEnd in jfs_write_direct)
                 */
                l2bsize = ip->i_ipmnt->i_l2bsize;                              //230531
                chunksize = ISSPARSE(ip) ? ((0x01L<<l2bsize)*16) : 0x100000;   //230531

                for (remaining = nbytes; remaining > 0; remaining -= mbytes, offset += mbytes, pdata += mbytes)    //230531
                {                                                              //230531
                        mbytes = MIN(chunksize, remaining);                    //230531
                        rc = jfs_write_direct(ip, offset, pdata, &mbytes, flags);
                        if (rc)                                                //230531
                        {                                                      //230531
                                remaining -= mbytes;                           //230531
                                break;                                         //230531
                        }                                                      //230531
                }                                                              //230531
        }
        else
        {
                rc = jfs_write_cached(ip, offset, pdata, &nbytes, flags);
        }

        /* mark the inode as changed and updated */
        imark(ip, IUPD|ICHG|IFSYNC);                            // D233382

// BEGIN F226941
#ifdef _JFS_FASTDASD                                            // D233382
        DLIM_UPDATE(0, ip, ip->i_nblocks - orig_nblocks);       // D233382
#else /* ! _JFS_FASTDASD */                                     // D233382
        if (ip->i_dasdlim && (ip->i_nblocks != orig_nblocks))
        {
                int32   tid;

                txBegin(ip->i_ipmnt, &tid, COMMIT_FORCE);       // D233382
                dasd_update(tid, ip, ip->i_nblocks - orig_nblocks);
                rc2 = DLIM_TXCOMMIT(tid, ip, 0);
                txEnd(tid);
        }
        /* if available tlocks are scarce and inode has anonymous tlocks,
         * OR if we modified a metadata page that is on the synclist, OR
         * if writing through to disk and metadata was modified,
         * commit the inode.
         */
        else
#endif /* _JFS_FASTDASD */                                      // D233382
        if (
            (TlocksLow && ip->i_atlhead) || (ip->i_flag & ISYNCLIST) ||
            ( ((flags & FSYNC) && (ip->i_flag & IUPDNEW))  ||
            (SyncRunning == 0) )    //PS21092003
           ) 
        {
                int32   tid;

#ifdef _JFS_LAZYCOMMIT                                          // D233382
                txBegin(ip->i_ipmnt, &tid, COMMIT_FORCE);       // D233382
#else                                                           // D233382
                txBegin(ip->i_ipmnt, &tid, (ip->i_flag & ISYNCLIST)?1:0);
#endif                                                          // D233382
                rc2 = txCommit(tid, 1, &ip, 0);                 // PS21092003
                txEnd(tid);
        }
// END F226941

        /* return the number of bytes written */
        *plen = nbytes - remaining;                                            //230531

        return(rc);
}


/*
 * NAME:         PromoteI()
 *
 * FUNCTION:     extend the size of a file in preparation for
 *               a write operation that starts beyond the last fully
 *               allocated page of a file.
 *
 * PARAMETERS:
 *      ip      - the inode of the file.
 *      offset  - starting offset of the write.
 *
 * RETURN VALUES:
 *      0       - success.
 *      EIO     - i/o error.
 *      ENOSPC  - insufficient disk resources.
 */
static int32
PromoteI(inode_t *ip,
        int64 offset)
{
        int32 rc;
        inode_t *ipmnt;
        int64 pnosz, pnooff;

        /* offset must beyond end of file */
        assert(offset > ip->i_size);

        /* compute the page numbers of the last page of
         * the file and the page containing the starting
         * offset of the write.
         */
        pnosz = CM_BTOCBLK(ip->i_size);
        pnooff = offset >> CM_L2BSIZE;

        /* check if the pages are the same.
         */
        if (pnooff != pnosz)
        {
                /* for a dense file: if there are pages between
                 * the last page of the file and the page containing
                 * the offset, extend the file to the end of the
                 * page prior to the starting offset.
                 */
                if (ISSPARSE(ip) == FALSE &&
                    pnooff-1 > pnosz)
                        return(ExtendI(ip,CM_CBLKTOB(pnooff-1),TRUE));

                /* if the last page of the file is a partially
                 * backed page, extend the file to the end
                 * of the last page.
                 */
                ipmnt = ip->i_ipmnt;
                if (CM_BTOBLKS(ip->i_size, ipmnt->i_bsize, ipmnt->i_l2bsize) != ipmnt->i_nbperpage)
                        return(ExtendI(ip,CM_CBLKTOB(pnosz),TRUE));
        }

        return(0);
}


/*
 * NAME:        ExtendI()
 *
 * FUNCTION:    extend the size of a file.
 *
 * PARAMETERS:
 *      ip      - the inode of the file.
 *      newsize - new file size.
 *      abnr    - boolean_t indicating whether the extents allocated
 *                as part of the extension should be allocated but
 *                not recorded.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO     - i/o error.
 *      ENOSPC  - insufficient disk resources.
 */
ExtendI(inode_t *ip,
        int64 newsize,
        boolean_t abnr)
{
        int32 nbperpage, l2bsize, endblks, rc, sparse, endbytes, rc2;
        int64 asize, reqblks, xlen, pnoold, pnonew, oldsize;
        xad_t xad, *xp = &xad;
        cbuf_t *cp;
        int32 tid;                                              //PS21092003
        inode_t   *iplist[1];                                   //PS21092003

        /* new size should be greater than current size */
        assert(newsize > ip->i_size);

        /* Cannot grow a file larger than 2GB if there are any legacy opens
         * on this file.
         */
        if ((newsize > 0x7fffffff) && (ip->i_lgcnt > 0))
                return ERROR_SHARING_VIOLATION;

        /* get the log2 block size and number of blocks per page */
        l2bsize = ip->i_ipmnt->i_l2bsize;
        nbperpage = ip->i_ipmnt->i_nbperpage;

        if (newsize > ((int64)1 << (40+l2bsize)))
                return ENOSPC;

        /* get the number of blocks backing the last page of the
         * file and the number of bytes backed by these blocks.
         */
        endblks = CM_BTOBLKS(ip->i_size, ip->i_ipmnt->i_bsize, l2bsize);
        endbytes = endblks << l2bsize;

        /* get the file's current allocation size */
        asize = (ip->i_size) ? ((ip->i_size-1) & ~CM_OFFSET) + endbytes
                             : 0;

        /* if the new size is within the allocated size,
         * set the new size and return.
         */
        if (newsize <= asize)
        {
                ip->i_size = newsize;
                return(0);
        }

        /* remember the current file size for backout */
        oldsize = ip->i_size;

        /* initialize the allocation hint/return value.
         * the xad pointed to by xp will hold the input allocation
         * hint provided to the extent allocator as well as
         * the results (set by the allocator) of the allocation.
         */
        XADaddress(xp,0);

        /* get an indicator of whether the file is sparse */
        sparse = ISSPARSE(ip);

        /* determine how many additional blocks must be added to
         * the file, excluding partial page requirements, if any.
         */
        reqblks = (sparse) ? CM_BTOBLKS(newsize,ip->i_ipmnt->i_bsize,l2bsize) :
                             CM_BTOFBLKS(newsize - asize,l2bsize);

        /*
         * if the last page of the file partially backed,
         * handle the extension of this page.
         */
        if (endblks != nbperpage)
        {
                /* get the cbuf for the last (partial) page of the file.
                 */
                if (rc = cmRead(ip, ip->i_size-1, -1, &cp))
                        return(rc);

                /* determine how many additional blocks are required
                 * for the partial page and how many blocks will be
                 * allocated.
                 */
                if (sparse)
                {
                        /* get the page numbers for the pages containing
                         * the current size and newsize.
                         */
                        pnoold = CM_BTOCBLK(ip->i_size);
                        pnonew = CM_BTOCBLK(newsize);

                        /* if they are the same than no additional blocks
                         * are required and will try to allocate a single
                         * extent for the last page's new requirements.
                         */
                        if (pnoold == pnonew)
                        {
                                xlen = reqblks;
                        }
                        else
                        {
                                /* pages are not the same, so we have to
                                 * account for all of the last partial
                                 * page. if the page containing newsize
                                 * is the page after the last page we
                                 * will try to allocate a single extent
                                 * for both.  otherwise, we'll only allocate
                                 * an extent for the partial page's new
                                 * requirements and let the code below
                                 * handle the new size page.
                                 */
                                reqblks += nbperpage;
                                xlen = (pnoold+1 == pnonew) ? reqblks :
                                                         nbperpage;
                        }
                }
                else
                {
                        /* for dense files, we have to take into
                         * account the existing blocks of the last
                         * page. and we'll try to allocate a single
                         * extent for all of the new blocks required
                         * by the file extension.
                         */
                        reqblks += endblks;
                        xlen = reqblks;
                }

                /* extend the allocation. in extending the allocation for
                 * the current (partial) page, we attempt to extend the
                 * allocation such that it maybe sufficient to handle the
                 * remaining allocation requirements of the current extend
                 * request. extRealloc() may provide us with an extent with
                 * less blocks than we asked for but the number of block
                 * provided should always be sufficient to cover new
                 * requirements of the partial page.
                 */
                if (rc = extRealloc(ip,cp,xlen,xp,abnr))
                {
                        cmPut(cp,FALSE);
                        return(rc);
                }

                /* get the number of blocks in the new allocated
                 * extent.
                 */
                xlen = lengthXAD(xp);

                /* update the cbuf with the starting address and
                 * number of blocks for the page.
                 */
                PXDlength(&cp->cm_pxd,MIN(nbperpage,xlen));
                PXDaddress(&cp->cm_pxd,addressXAD(xp));

                /* zero the portion of the cbuf that reflects the
                 * additional allocation.
                 */
                bzero(cp->cm_cdata+endbytes,CM_BSIZE-endbytes);

                /* put the cbuf.
                 */
                cmPut(cp,TRUE);

                /* update the file's current allocated size to reflect
                 * the new extent.
                 */
                asize += ((xlen - endblks) << l2bsize);

                /* adjust the file size to reflect this size.
                 */
                ip->i_size = MIN(newsize,asize);

                /* adjust the number of blocks required to reflect
                 * the newly allocated extent.
                 */
                reqblks -= xlen;
        }

        /* for sparse file: recompute the file's current allocation
         * size as the start of the page containing the new size.
         */
        if (sparse)
                asize = (newsize-1) & ~CM_OFFSET;

        /* if no allocation hint, call extHint() to get one */
        if (addressXAD(xp) == 0)
        {
                if (rc = extHint(ip,asize,xp))
                        return(rc);
        }

        /*
         * add extents to the end of the file until we have extended
         * the file such that it reflects the new size.
         */
        for (xlen = reqblks; reqblks > 0; reqblks -= xlen)
        {
                /* extAlloc() is used to allocate the extents and we ask
                 * it allocate xlen blocks.  however, it may allocate an
                 * extent with less than xlen (this many contigious free
                 * blocks are not avaliable) but the extent will always
                 * be large enough to cover a page.  initially, we ask
                 * ask this routine to allocate reqblks blocks (i.e. xlen ==
                 * reqblks).  however, if less than reqblks are provided,
                 * the next allocation request will be for the minimum
                 * of the number of blocks still required and the number
                 * of block (last) provided.
                 */
                xlen = MIN(reqblks,xlen);
                if (rc = extAlloc(ip,xlen,CM_OFFTOCBLK(asize),xp,abnr))
//PS21092003 Begin
//                        goto error;
                   {                                                        
                       /* start transaction */                                     
                   txBegin(ip->i_ipmnt, &tid, COMMIT_FORCE);                    
                      /* update the inode map addressing structure to point to it */ 
                    iplist[0] = ip;                                             
                    if (ip->i_size != oldsize)                             
                       {                                                   
                       rc2 = xtTruncate(tid,ip,oldsize,COMMIT_PWMAP);      
                       assert(rc2 == 0);                                     
                       }                                                   
                    rc2 = txCommit(tid, 1, &iplist[0], 0);                 
                    assert(rc2 == 0);                                    
                    txEnd(tid);                                         
                    return(rc);                                         
                   }                                                         

                /* get the number of blocks in the new allocated
                 * extent.
                 */
                xlen = lengthXAD(xp);

                /* update the file's current allocated size to reflect
                 * the new extent.
                 */
                asize += (xlen << l2bsize);

                /* adjust the file size to reflect this size.
                 */
                ip->i_size = MIN(newsize,asize);
        }

        return(rc);                                      //PS21092003

/* error:
        * if we have done some extension of the file size, move
         * it back to the old size.
         *
        if (ip->i_size != oldsize)
        {
                rc2 = xtTruncate(0,ip,oldsize,COMMIT_WMAP);
                assert(rc2 == 0);
        }

        return(rc); */
}


/*
 * NAME:        WRClean()
 *
 * FUNCTION:    clean up after a write request exception.
 *
 * PARAMETERS:
 *      ip        - the inode of the file.
 *      oldsize   - starting file size.
 *      bmoved    - TRUE is some of the write has completed; otherwise,
 *                  FALSE.
 *      overalloc - TRUE if file is overallocated; otherwise, FALSE.
 *
 * RETURN VALUES: none.
 */
static void
WRClean(inode_t *ip,
        int64 oldsize,
        boolean_t bmoved,
        boolean_t overalloc)
{
        int32 rc;

        /* truncate the file back to its old size if no
         * data was moved but the file was extended (i.e.
         * PromoteI).
         */
        if (bmoved == FALSE && ip->i_size != oldsize)
        {
                rc = xtTruncate(0,ip,oldsize,COMMIT_WMAP);
                assert(rc == 0);
                return;
        }

        /* truncate the file's allocation to i_size if
         * it is over allocated.
         */
        if (overalloc)
        {
                rc = xtTruncate(0,ip,ip->i_size,COMMIT_WMAP);
                assert(rc == 0);
        }

        return;
}


/*
 * NAME:        SyncDataI()
 *
 * FUNCTION:    synchrnously write a range within a file to disk.
 *
 * PARAMETERS:
 *      ip      - the inode of the file.
 *      offset  - starting byte offset of the range.
 *      nbytes  - number of bytes within the range.
 *
 * RETURN VALUES:
 *      0       - success.
 *      EIO     - i/o error.
 */
static int32
SyncDataI(inode_t *ip,
          int64 offset,
          int64 nbytes)
{
        int64   firstb, lastb;

        /* if file meta data has been modified (e.g., data extent
         * (re)allocation, data extent state change, etc), we will
         * commit the transaction at the end of jfs_write().
         * Otherwise (i.e., overwrites only), just write the range
         * synchronously.
         */
        if (ip->i_flag & IUPDNEW)                         // PS21092003
           return 0;
//PS21092003 Begin
    /* get the logical block numbers of the first and last
     * cache buffers of the range.
     */
    firstb = offset >> CM_L2BSIZE;
    lastb = (offset + nbytes) >> CM_L2BSIZE;
    return cmWrite(ip->i_cacheid, firstb, lastb - firstb + 1); 
//PS end
}

/*
 * NAME:        jfs_readFAC()
 *
 * FUNCTION:    entry point for read, returning cachelist
 *
 * PARAMETERS:
 *      ip      - the inode of the file.
 *      offset  - starting byte offset within the inode of the read
 *      length  - the number of bytes to read.
 *      cacheList - pointer to cachelist
 *
 * RETURN VALUES:
 *      0       - success
 *      errors from subroutines.
 *
 *      IREAD_LOCK(IP) must be held on entry.
 */
int32
jfs_readFAC(inode_t     *ip,
         int64          offset,
         int64          *pLen,
         cache_list_t   *cacheList)
{
        int64   nbytes;
        int32   rc;
        int32   type;

        nbytes = *pLen;
        *pLen = 0;

        /* validate the offset */
        if (offset < 0 || nbytes <= 0)
                return EINVAL;

        /* better be a regular file, directory or symlink */
        type = ip->i_mode & IFMT;
/* ??? directory ? */
        assert(type == IFREG || type == IFDIR || type == IFLNK);

        /* Don't allow read past end of file */
        nbytes = MIN(nbytes, ip->i_size - offset);
        if (nbytes <= 0)
                return EINVAL;

        rc = cmSendFile(ip, offset, &nbytes, cacheList);

        *pLen = nbytes;

        /* mark the inode as accessed */
        INODE_LOCK(ip);
        imark(ip, IACC);
        INODE_UNLOCK(ip);

        return rc;
}
