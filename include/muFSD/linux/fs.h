//
// $Header: /cur/cvsroot/boot/include/muFSD/linux/fs.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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



//
// This file is a OS/2 "port" of /usr/src/linux-1.2.1/include/linux/fs.h
// It contains the main Linux VFS definitions - I have yet to make it look
// like /usr/src/linux-1.2.1/include/linux/fs.h !
//

#ifndef __FS_H
#define __FS_H

#include <os2/magic.h>
//#include <linux/sched.h>
#include <linux/wait.h>


#define NR_INODE 2048        /* this should be bigger than NR_FILE */
#define NR_IHASH 131

/*
 * These are the fs-independent mount-flags: up to 16 flags are supported
 */
#define MS_RDONLY         1 /* mount read-only */
#define MS_NOSUID         2 /* ignore suid and sgid bits */
#define MS_NODEV         4 /* disallow access to device special files */
#define MS_NOEXEC         8 /* disallow program execution */
#define MS_SYNCHRONOUS        16 /* writes are synced at once */
#define MS_REMOUNT        32 /* alter flags of a mounted FS */

#define S_APPEND    256 /* append-only file */
#define S_IMMUTABLE 512 /* immutable file */

/*
 * Flags that can be altered by MS_REMOUNT
 */
#define MS_RMT_MASK (MS_RDONLY)

/*
 * Magic mount flag number. Has to be or-ed to the flag values.
 */
#define MS_MGC_VAL 0xC0ED0000 /* magic flag number to indicate "new" flags */
#define MS_MGC_MSK 0xffff0000 /* magic flag number mask */

/*
 * Note that read-only etc flags are inode-specific: setting some file-system
 * flags just means all the inodes inherit those flags by default. It might be
 * possible to override it selectively if you really wanted to with some
 * ioctl() that is not currently implemented.
 *
 * Exception: MS_RDONLY is always applied to the entire file system.
 */
#define IS_RDONLY(inode) (((inode)->i_sb) && ((inode)->i_sb->s_flags & MS_RDONLY))
#define IS_NOSUID(inode) ((inode)->i_flags & MS_NOSUID)
#define IS_NODEV(inode) ((inode)->i_flags & MS_NODEV)
#define IS_NOEXEC(inode) ((inode)->i_flags & MS_NOEXEC)
#define IS_SYNC(inode) ((inode)->i_flags & MS_SYNCHRONOUS)

#define IS_APPEND(inode) ((inode)->i_flags & S_APPEND)
#define IS_IMMUTABLE(inode) ((inode)->i_flags & S_IMMUTABLE)

#define BLOCK_SIZE          1024

/*************************************************************************/
/*** Mapping of Linux's VFS buffer_head structure                      ***/
/*** (/usr/src/linux-1.2.1/include/linux/fs.h)                         ***/
/*** This is to implement the following VFS routines :                 ***/
/***     - bread()   in /usr/src/linux-1.2.1/fs/buffer.c               ***/
/***     - brelse()  in /usr/src/linux-1.2.1/fs/buffer.c               ***/
/*************************************************************************/
#define STATUS_FILEBUF_FREE 1
#define STATUS_FILEBUF_USED 2

struct buffer_head {
    char               *b_data;                  // pointer to data block
    unsigned long       b_size;                  // block size
    blk_t               b_blocknr;               // block number
    unsigned short      b_count;                 // users using this block
    unsigned char       b_uptodate;              // contents of buffer is valid (1) or not (0)
    unsigned long       b_flushtime;             // time when buffer should be written
    unsigned long       b_lru_time;              // Time when this buffer was last used.
    unsigned char       b_dirt;                  // 0-clean, 1-dirty
    unsigned char       b_lock;                  // 0 - ok, 1 -locked
    dev_t               b_dev;                   // hVPB
    struct buffer_head *b_next;                  // doubly linked list of hash-queue
    struct buffer_head *b_prev;
    struct buffer_head *b_next_free;             // doubly linked list of buffers
    struct buffer_head *b_prev_free;
    struct buffer_head *b_this_page;             // circular list of buffers in one SEGMENT

    unsigned char       b_list;                  // List that this buffer appears
    unsigned char       b_req;                   // 0 if the buffer has been invalidated ????
#ifndef OS2
    struct wait_queue  *b_wait;
#endif
//
// OS/2 specific fieds
//
#ifdef OS2
    unsigned long       b_physaddr;     // physical address of b_data
    magic_t             b_magic;        //  Magic signature always equal to  BUFFER_HEAD_MAGIC
#endif
};

typedef struct buffer_head *pbuffer_head;  // Far pointer to buffer_head *** NOW OBSOLETE
typedef pbuffer_head *ppbuffer_head;       // Far pointer to far pointer to buffer_head *** NOW OBSOLETE
/*************************************************************************/



/*********************************************************/
/*** Memory I-node                                     ***/
/*********************************************************/

#ifndef MINIFSD
#include <linux/ext2_fs_i.h>       // ext2 definitions
#else
#include <linux/e2_fs_i.h>         // ext2 definitions
#endif

//
// From /usr/src/linux-1.2.1/include/linux/fs.h
//
#pragma pack(4)
struct inode {
        dev_t           i_dev;
        unsigned long   i_ino;
        umode_t         i_mode;
        nlink_t         i_nlink;
        uid_t           i_uid;
        gid_t           i_gid;
        dev_t           i_rdev;
        off_t           i_size;
        time_t          i_atime;
        time_t          i_mtime;
        time_t          i_ctime;
        unsigned long   i_blksize;
        unsigned long   i_blocks;
        unsigned long    i_version;
        struct semaphore i_sem;
        unsigned char    i_lock;

        unsigned long i_wait;
        struct inode * i_next, * i_prev;
        struct inode * i_hash_next, * i_hash_prev;

        unsigned short i_count;
        unsigned char  i_dirt;
        struct super_block      * i_sb;     // superblock
        struct inode_operations * i_op;
        unsigned short i_flags;

    union {
        struct ext2_inode_info ext2_i;
    } u;

};
#pragma pack()

/*********************************************************/


/******************************************************/
/*** File descriptor structure (open file instance) ***/
/******************************************************/

#define OPENMODE_READONLY  1
#define OPENMODE_WRITEONLY 2
#define OPENMODE_READWRITE 4
#define OPENMODE_EXECUTE   8
#define OPENMODE_DOSBOX    16
#define OPENMODE_DASD      32

#define STATUS_HFILE_FREE 1
#define STATUS_HFILE_USED 2

#pragma pack(4)
struct file {
    mode_t                  f_mode;     // open mode
    loff_t                  f_pos;      // file pointer
    unsigned short          f_count;    // use count
    off_t                   f_reada;
    struct file *f_next,   *f_prev;
    struct inode           *f_inode;    // memory I-node
    unsigned long           f_version;
    unsigned short          f_flags;
    struct file_operations *f_op;

//
// OS/2 Specific fields
//
    unsigned short f_magic;        // magic number always equal to FILE_MAGIC
    struct file  **f_list;         // pointer to head of list holding this file
};
#pragma pack()
/******************************************************/


/************************************************/
/*** Structure de volume                      ***/
/************************************************/
#ifndef MINIFSD
#include <linux/ext2_fs_sb.h>
#else
#include <linux/e2_fs_sb.h>
#endif

#pragma pack(1)
struct super_block {

        dev_t s_dev;
        unsigned long s_blocksize;
        unsigned char s_blocksize_bits;
        unsigned char s_lock;
        unsigned char s_rd_only;
        unsigned char s_dirt;
//      struct file_system_type *s_type;
        struct super_operations *s_op;
        unsigned long s_flags;
        unsigned long s_magic;
        unsigned long s_time;
//      struct inode * s_covered;
      struct inode * s_mounted;
#ifndef OS2
        struct wait_queue * s_wait;
#else
        unsigned long       s_wait;
#endif

        union {
//              struct minix_sb_info minix_sb;
//              struct ext_sb_info ext_sb;
                struct ext2_sb_info ext2_sb;
//              struct hpfs_sb_info hpfs_sb;
//              struct msdos_sb_info msdos_sb;
//              struct isofs_sb_info isofs_sb;
//              struct nfs_sb_info nfs_sb;
//              struct xiafs_sb_info xiafs_sb;
//              struct sysv_sb_info sysv_sb;
//              void *generic_sbp;
        } u;



//  OS/2 specific fields

    //
    // Strat 2 fields
    //
    unsigned char  s_unit;              // Unit code (pvpfsi->vpi_unit)
    unsigned char  s_drive;             // Drive     (pvpfsi->vpi_drive)
#ifndef MINIFSD
    PTR16          s_strat2;            // Extended strategy entry point ()
#else
    void          (*s_strat2)();
#endif
    UINT32         sector_size;       /* media sector size in bytes          */
    UINT32         sectors_per_block; /* number of media sector per FS block */

    UINT32         nb_sectors;        /* number of media sectors in media    */

    unsigned char   s_blocks_per_page;
    unsigned char   s_is_swapper_device;  // 1 means SWAPPER.DAT resides on this device

    struct super_block  *s_next;
    struct super_block  *s_prev;
    struct super_block **s_list;
    magic_t              s_magic_internal;
    unsigned char        s_status;
/*******************************************************************/

};
#pragma pack()

/************************************************/
#define VOL_STATUS_FREE                 0
#define VOL_STATUS_MOUNTED              2
#define VOL_STATUS_REMOVED              3
#define VOL_STATUS_MOUNT_IN_PROGRESS    4


//
// From /usr/src/linux-1.2.1/include/linux/dirent.h
//
#ifdef OS2
#define NAME_MAX 255
#endif

struct dirent {
        long                d_ino;
        off_t               d_off;
        unsigned short      d_reclen;
        char                d_name[NAME_MAX + 1];
};

#ifdef OS2
/*********************************************************/
/*** dummy definitions for OS/2                        ***/
/*********************************************************/
struct vm_area_struct;
struct statfs;
struct iattr;
/*********************************************************/
#endif

#ifndef OS2
struct file_operations {
        int (*lseek) (struct inode *, struct file *, off_t, int);
        int (*read) (struct inode *, struct file *, char *, int);
        int (*write) (struct inode *, struct file *, char *, int);
        int (*readdir) (struct inode *, struct file *, struct dirent *, int);
        int (*select) (struct inode *, struct file *, int, select_table *);
        int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
        int (*mmap) (struct inode *, struct file *, struct vm_area_struct *);
        int (*open) (struct inode *, struct file *);
        void (*release) (struct inode *, struct file *);
        int (*fsync) (struct inode *, struct file *);
        int (*fasync) (struct inode *, struct file *, int);
        int (*check_media_change) (dev_t dev);
        int (*revalidate) (dev_t dev);
};
#else
struct file_operations {
        int (*lseek) (struct inode *, struct file *, off_t, int);
#ifndef MINIFSD
        int (*read) (struct inode *, struct file *, char *, int);
#else
        long (*read) (struct inode *, struct file *, char *, long);
#endif
        int (*write) (struct inode *, struct file *, char *, int);
        int (*readdir) (struct inode *, struct file *, struct dirent *, int);
        int (*select) (void);
        int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
        int (*mmap) (struct inode *, struct file *, struct vm_area_struct *);
        int (*open) (struct inode *, struct file *);
        void (*release) (struct inode *, struct file *);
        int (*fsync) (struct inode *, struct file *);
        int (*fasync) (struct inode *, struct file *, int);
        int (*check_media_change) (dev_t dev);
        int (*revalidate) (dev_t dev);
};
#endif
struct inode_operations {
        struct file_operations * default_file_ops;
        int (*create) (struct inode *,const char *,int,int,struct inode **);
        int (*lookup) (struct inode *,const char *,int,struct inode **);
        int (*link) (struct inode *,struct inode *,const char *,int);
        int (*unlink) (struct inode *,const char *,int);
        int (*symlink) (struct inode *,const char *,int,const char *);
        int (*mkdir) (struct inode *,const char *,int,int);
        int (*rmdir) (struct inode *,const char *,int);
        int (*mknod) (struct inode *,const char *,int,int,int);
        int (*rename) (struct inode *,const char *,int,struct inode *,const char *,int);
        int (*readlink) (struct inode *,char *,int);
        int (*follow_link) (struct inode *,struct inode *,int,int,struct inode **);
        int (*bmap) (struct inode *,int);
        void (*truncate) (struct inode *);
        int (*permission) (struct inode *, int);
        int (*smap) (struct inode *,int);
};

struct super_operations {
        void (*read_inode) (struct inode *);
        int (*notify_change) (struct inode *, struct iattr *);
        void (*write_inode) (struct inode *);
        void (*put_inode) (struct inode *);
        void (*put_super) (struct super_block *);
        void (*write_super) (struct super_block *);
        void (*statfs) (struct super_block *, struct statfs *);
        int (*remount_fs) (struct super_block *, int *, char *);
};


//
//  Linux block device read_ahead capabilities
//
extern long read_ahead[];


#if !defined(MICROFSD) && !defined(MINIFSD)
#define BUF_CLEAN 0
#define BUF_UNSHARED 1 /* Buffers that were shared but are not any more */
#define BUF_LOCKED 2   /* Buffers scheduled for write */
#define BUF_LOCKED1 3  /* Supers, inodes */
#define BUF_DIRTY 4    /* Dirty buffers, not yet scheduled for write */
#define BUF_SHARED 5   /* Buffers shared */
#define NR_LIST 6

extern void refile_buffer(struct buffer_head * buf);
void set_writetime(struct buffer_head * buf, int flag);
INLINE void mark_buffer_clean(struct buffer_head * bh)
{
  if(bh->b_dirt) {
    bh->b_dirt = 0;
    if(bh->b_list == BUF_DIRTY) refile_buffer(bh);
  }
}

INLINE void mark_buffer_dirty(struct buffer_head * bh, int flag)
{
  if(!bh->b_dirt) {
    bh->b_dirt = 1;
    set_writetime(bh, flag);
    if(bh->b_list != BUF_DIRTY) refile_buffer(bh);
  }
}
#endif



#define READ 0
#define WRITE 1
#define READA 2         /* read-ahead    - don't pause                */
#define WRITEA 3        /* "write-ahead" - silly, but somewhat useful */


struct file_system_type {
        struct super_block *(*read_super) (struct super_block *, void *, int);
        const char *name;
        int requires_dev;
        struct file_system_type * next;
};

#endif /* __FS_H */

