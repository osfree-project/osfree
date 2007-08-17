//
// $Header: /cur/cvsroot/boot/muFSD/ext2/file.c,v 1.2 2006/11/24 11:43:13 valerius Exp $
//

// 32 bits Linux ext2 file system driver for OS/2 WARP - Allows OS/2 to
// access your Linux ext2fs partitions as normal drive letters.
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2/os2.h>

#include <string.h>

#include <os2/types.h>
#include <os2/magic.h>
#include <microfsd.h>
#include <ext2fs.h>
#include <mfs.h>
#include <linux/ext2_fs.h>
#include <linux/stat.h>

#include <add.h>
#include <i86.h>

#define NR_FILES 1

struct file flist[1] = {

{
    0, // struct inode      *f_inode;    // I-node
    0, // mode_t             f_mode;     // open mode
    0, // loff_t             f_pos;      // file pointer
    0, // unsigned short     f_count;    // use count
//    off_t              f_reada;
//    unsigned short     f_flags;
    FILE_MAGIC // unsigned short f_magic;        // magic number always equal to FILE_MAGIC
}
};


struct file *get_empty_filp(void) {
    if (flist->f_count)
        microfsd_panic("get_empty_filp : no more files");

    memset(flist, 0, sizeof(struct file));
    flist->f_count = 1;
    flist->f_magic = FILE_MAGIC;
    return flist;
}

void put_filp(struct file *filp) {
    if (flist->f_magic != FILE_MAGIC)
        microfsd_panic("put_filp : invalid magic nr");

    if (flist->f_count == 0)
        microfsd_panic("put_filp : Trying to free free filp");

    filp->f_count --;
}


struct file *open_by_inode(UINT32 ino_no)
{
    struct file  *filp;
#ifdef MCIROFSD_TRACE
    printk("open_by_inode(%lu)", ino_no);
#endif
    /*
     * Allocates a zeored file descriptor (panic if failure)
     */
    filp = get_empty_filp();

    /*
     * Gets the I-node (panic if failure)
     */
    filp->f_inode = iget(ino_no);


    return filp;
}

void vfs_close(struct file *f)
{
#ifdef MICROFSD_TRACE
    printk("vfs_close(%lu)", f->f_inode->i_ino);
#endif
    /*
     * Frees the I-node (panic if failure)
     */
    iput(f->f_inode);

    /*
     * Frees the file (panic if failure)
     */
    put_filp(f);
}


struct file *open_by_name(char *pName) {
    char         pNom[CCHMAXPATH];
    char        *pTmp;
    struct file *p_file;
    int          ok;
    int          Fin;
    int          FinPath;
    UINT32       ino_no;
    struct dirent          Dir;
    int          rc;

#ifdef MICROFSD_TRACE
    printk("open_by_name(%s)", pName);
#endif

    /*
     * Extracts the path from the name
     */
    pTmp = DecoupePath(pName, pNom);

    /*
     * If it is the root directory we've finished
     */
    if (pTmp == 0) {
        return open_by_inode(EXT2_ROOT_INO);
    } /* end if */

    /*
     * Now we loop on the path components
     */
    ino_no  = EXT2_ROOT_INO;
    FinPath = 0;
    while (FinPath == 0) {
        /*
         * Opens the "." file of the directory (failure = panic)
         */
        p_file = open_by_inode(ino_no);

            ok = 0;
            Fin = 0;
            /*** Lookup inside the "." file of the parent directory ***/
            while (Fin == 0) {
                if (VFS_readdir(p_file, &Dir) != 0) {
                    Fin = 1;
                } else {
                    /*
                     * We do a case insensitive comparaison (cast needed to get a near ptr)
                     */
                    if (stricmp(pNom, Dir.d_name) == 0) {
                        Fin = 1;
                        ok  = 1;
                    } /* end if */
                } /* end if */
            } /* end while */
            /*
             * if we didn't find the file
             */
            if (ok == 0) {
                vfs_close(p_file); /* failure in vfs_close = panic */
                return 0;
/*
                microfsd_panic("open_by_name : file not found");
*/
            }
            /*
             * If found we go down a level
             */
            ino_no = Dir.d_ino;
            vfs_close(p_file); /* failure in vfs_close = panic */

        if ((pTmp = DecoupePath(pTmp, pNom)) == 0) {
            FinPath = 1;
        }

    }

    return open_by_inode(ino_no);
}

#define NBUF 1

long ext2_file_read (struct inode *inode, struct file *filp,
                    char far *buf, long count)
{
        long read, left, chars;
        long block, blocks, offset;
        int bhrequest, uptodate;
        long clusterblocks;
        struct buffer_head **bhb, **bhe;
        struct buffer_head * bhreq[NBUF];
        struct buffer_head * buflist[NBUF];
        struct super_block *sb;
        long size;
        int err;

        /*
         * Precondition checking
         */
        if (!inode)
            microfsd_panic("ext2_file_read : inode = NULL");
        if (inode->i_magic != INODE_MAGIC)
            microfsd_panic("ext2_file_read : inode with invalid magic number");
        if (!filp)
            microfsd_panic("ext2_file_read : filp = NULL");
        if (filp->f_magic != FILE_MAGIC)
            microfsd_panic("ext2_file_read : filp with invalid magic number");



        sb = inode->i_sb;

        if (!S_ISREG(inode->i_mode)) {
                microfsd_panic("ext2_file_read : not a regular file");
        }

        offset = filp->f_pos;
        size = inode->i_size;
        if (offset > size)
                left = 0;
        else
                left = size - offset;
        if (left > count)
                left = count;
        if (left <= 0)
                return 0;
        read = 0;
        block = offset >> EXT2_BLOCK_SIZE_BITS(sb);
        offset &= (sb->s_blocksize - 1);
        size = (size + sb->s_blocksize - 1) >> EXT2_BLOCK_SIZE_BITS(sb);
        blocks = (left + offset + sb->s_blocksize - 1) >> EXT2_BLOCK_SIZE_BITS(sb);
        bhb = bhe = buflist;
#if 0
        if (filp->f_reada) {
#ifndef OS2
                if (blocks < read_ahead[MAJOR(inode->i_dev)] >> (EXT2_BLOCK_SIZE_BITS(sb) - 9))
                    blocks = read_ahead[MAJOR(inode->i_dev)] >> (EXT2_BLOCK_SIZE_BITS(sb) - 9);
#else
                if (blocks < read_ahead[0] >> (EXT2_BLOCK_SIZE_BITS(sb) - 9))
                    blocks = read_ahead[0] >> (EXT2_BLOCK_SIZE_BITS(sb) - 9);
#endif
                if (block + blocks > size)
                        blocks = size - block;
        }
#endif
        /*
         * We do this in a two stage process.  We first try and request
         * as many blocks as we can, then we wait for the first one to
         * complete, and then we try and wrap up as many as are actually
         * done.  This routine is rather generic, in that it can be used
         * in a filesystem by substituting the appropriate function in
         * for getblk
         *
         * This routine is optimized to make maximum use of the various
         * buffers and caches.
         */

        clusterblocks = 0;

        do {
                bhrequest = 0;
                uptodate = 1;
                while (blocks) {

                        --blocks;
#ifndef OS2
#if 1
                        if(!clusterblocks) clusterblocks = ext2_getcluster(inode, block);
                        if(clusterblocks) clusterblocks--;
#endif
#endif
                        *bhb = ext2_getblk (inode, block++, 0, &err);
                        if (*bhb && !(*bhb)->b_uptodate) {
                                uptodate = 0;
                                bhreq[bhrequest++] = *bhb;
                        }

                        if (++bhb == &buflist[NBUF])
                                bhb = buflist;

                        /*
                         * If the block we have on hand is uptodate, go ahead
                         * and complete processing
                         */
                        if (uptodate)
                                break;

                        if (bhb == bhe)
                                break;
                }

                /*
                 * Now request them all
                 */
                if (bhrequest)
                        ll_rw_block (READ, bhrequest, bhreq);

                do {
                        /*
                         * Finish off all I/O that has actually completed
                         */
                        if (*bhe) {
//                                wait_on_buffer (*bhe);
                                if (!(*bhe)->b_uptodate) { /* read error? */
                                        bforget(*bhe);
                                        if (++bhe == &buflist[NBUF])
                                          bhe = buflist;
                                        left = 0;
                                        break;
                                }
                        }
#ifdef OS2
                        if (left < (off_t)(sb->s_blocksize) - offset)
#else
                        if (left < sb->s_blocksize - offset)
#endif
                                chars = left;
                        else
                                chars = sb->s_blocksize - offset;
                        filp->f_pos += chars;
                        left -= chars;
                        read += chars;
                        if (*bhe) {
#ifndef OS2
                                memcpy_tofs (buf, offset + (*bhe)->b_data,
                                             chars);
#else
                                _fmemcpy(buf, offset + (*bhe)->b_data,
                                             (int)chars);
#endif
                                bforget (*bhe);
                                buf += chars;
                        } else {
#ifndef OS2
                                while (chars-- > 0)
                                        put_fs_byte (0, buf++);
#else
                                _fmemset(buf, 0, (int)chars);
                                buf += chars;
#endif
                        }
                        offset = 0;
                        if (++bhe == &buflist[NBUF])
                                bhe = buflist;

                } while (left > 0 && bhe != bhb && (!*bhe));
        } while (left > 0);

        /*
         * Release the read-ahead blocks
         */
        while (bhe != bhb) {
                bforget (*bhe);
                if (++bhe == &buflist[NBUF])
                        bhe = buflist;
        }
        if (!read)
                microfsd_panic("ext2_file_read : I/O error");
        return read;
}

int VFS_read(struct file *file, char far *buf, loff_t len, unsigned long far*pLen) {
    int  err;
    long read;

    printk("     VFS_read(buf=%04X:%04X, len=%ld)", FP_SEG(buf), FP_OFF(buf), len);
    if (!file)
        microfsd_panic("VFS_read : flist = 0");
    if (file->f_magic != FILE_MAGIC)
        microfsd_panic("VFS_read : invalid magic number");

    read = ext2_file_read(file->f_inode, file, buf, len);
    if (read >= 0) {
        *pLen = (UINT32)read;
        err   = NO_ERROR;
    } else {
        microfsd_panic("VFS_read : read error");
    }

    return 0;
}
