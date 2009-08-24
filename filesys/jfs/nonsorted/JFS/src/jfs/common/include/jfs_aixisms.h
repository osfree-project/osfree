/* $Id: jfs_aixisms.h,v 1.1.1.1 2003/05/21 13:35:40 pasha Exp $ */

/* static char *SCCSID = "@(#)1.25.1.2  12/2/99 11:13:15 src/jfs/common/include/jfs_aixisms.h, sysjfs, w45.fs32, fixbld";
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
 *
 */

#ifndef _H_AIXISMS
#define _H_AIXISMS

/*
 *      jfs_aixism.h
 *
 * platform independent definitions aligned with AIX
 * from historical precedence with some POSIX flavor;
 *
 * N.B. This file should be included only by jfs_os2.h.
 */

/*
 *      DEFINITIONS
 */
typedef int32  daddr_t;
typedef uint32 dev_t;
typedef uint32 mode_t;
typedef uint32 uid_t;
typedef uint32 gid_t;
typedef uint32 ino_t;
typedef uint32 uint;
typedef int64  offset_t;
typedef int32  boolean_t;
typedef int16  nlink_t;

struct timestruc_t {
        uint32 tv_sec;
        uint32 tv_nsec;
};


/*
 *      configuration
 */
#define MAXPATHLEN      259
/* #define LINK_MAX        32767           arbitrarily, the same as AIX */
#define LINK_MAX        4294967295      /* D233784 */

#define FSYNC           IOFL_WRITETHRU  /* synchronous write flag */


/*
 *      ERRORS
 *
 * ref. c.jfs/rel/os2c/include/base/os2/bseerr.h
 */
#define ERROR_XXX       0xEE    /* appropriate error code needed */

#define EACCES          ERROR_ACCESS_DENIED     /*   4 */
#define EAGAIN          ERROR_XXX
#define EBUSY           ERROR_BUSY              /* 170 */
#define EDIRTY          627                     /* HPFS_VOLUME_DIRTY */
#define EEXIST          ERROR_FILE_EXISTS       /*  80 */
#define EFBIG           ERROR_XXX
#define EFORMAT         ERROR_XXX
#define EINTR           ERROR_INTERRUPT         /*  95 */
#define EINVAL          ERROR_INVALID_PARAMETER /*  87 */
#define EIO             ERROR_READ_FAULT        /*  30 */
#define EISDIR          ERROR_ACCESS_DENIED     /*   4 */
#define EMLINK          ERROR_BAD_COMMAND       /*  22 */
#define ENAMETOOLONG    ERROR_BUFFER_OVERFLOW   /* 111 */
#define ENFILE          ERROR_NO_MORE_FILES     /*  18 */
#define ENOENT          ERROR_FILE_NOT_FOUND    /*   2 */
#define ENOMEM          ERROR_NOT_ENOUGH_MEMORY /*   8 */
#define ENOSPC          ERROR_DISK_FULL         /* 112 */
#define ENOTDIR         ERROR_PATH_NOT_FOUND    /*   3 */
#define ENOTEMPTY       ERROR_FILE_EXISTS       /*  80 */
#define ENXIO           ERROR_XXX
#define EOPENFAILED     ERROR_OPEN_FAILED       /* 110 */
#define EPERM           ERROR_ACCESS_DENIED     /*   4 */
#define EROFS           ERROR_WRITE_PROTECT     /*  19 */
#define ESOFT           ERROR_XXX
#define ESTALE          ERROR_WRITE_FAULT       /*  29 */
#define ETXTBSY         ERROR_BUSY              /* 170 */
#define EXDEV           ERROR_BAD_COMMAND       /*  22 */


/*
 *      vfs (virtual file system)
 *
 * a virtual file system descriptor.
 * One of them is created for each mounted file-system.
 *
 * For OS/2, there is a table of vfs's that is allocated
 * at init time.
 */
struct vfs {
        struct vfs      *vfs_next;      /* vfs's are a linked list */
        struct vnode    *vfs_mntd;      /* pointer to mounted vnode, */
                                        /*      the root of this vfs */
        struct vnode    *vfs_vnodes;    /* all vnodes in this vfs */
        uint32          vfs_flag;       /* mount flags */
        uint32          vfs_bsize;      /* native block size */
        SHANDLE         vfs_hVPB;       /* handle for volume parameter block */
        SHANDLE         vfs_logVPB;     /* handle for log vpb */
        struct vpfsi    *vfs_vpfsi;     /* pointer to VPB for volume */
        struct vpfsi    *vfs_logvpfs;   /* pointer to VPB for log volume */
        void * _Seg16   vfs_strat2p;    /* pointer to DD strategy routine */
        void            (* APIENTRY vfs_strat3p)();
                                        /* pointer to flat strategy routine */
        caddr_t         vfs_data;       /* private data area pointer (imap) */
};

/* vfs flags */
#define VFS_READONLY    0x01    /* file system mounted read only */
#define VFS_ACCEPT      0x02    /* FS_MOUNT called with MOUNT_ACCEPT flag */
#define VFS_DASDLIM     0x20    /* DASD limits enabled          F226941 */
#define VFS_DASDPRIME   0x40    /* DASD limits need priming     F226941 */
#define VFS_SHUTDOWN    0x80    /* force file system unmount */


/* Structure of file locks */

struct file_lock {
  struct file_lock * next_lock;         /* Ptr to next lock in list        */
  struct file_lock * prev_lock;         /* Ptr to previous lock in list    */
  struct file_lock * vnode_next_lock;   /* Ptr to next lock in list        */
  struct file_lock * vnode_prev_lock;   /* Ptr to previous lock in list    */
  uint64 lock_start;                    /* offset in file of start of lock */
  uint64 lock_end;                      /* offset in file of end of lock   */
  void * anchor;                        /* ptr to owning anchor            */
  uint32 time_start;                    /* time started waiting            */
  uint32 time_remaining;                /* time remaining to wait          */
  uint16 share;                         /* lock is a shared lock           */
  uint16 pid;                           /* owners pid                      */
};

/* lock structure to be passed into jfs_filelock function */
struct lock_info {
  uint16  share;            /* on if lock is shared, else off */
  uint32  start;            /* start offset of lock           */
  uint32  length;           /* count of bytes to be locked    */
};

/* unlock structure to be passed into jfs_lock function */
struct unlock_info {
  uint32  start;            /* start offset of lock           */
  uint32  length;           /* count of bytes to be locked    */
};
/* longlong lock structure to be passed into jfs_filelock function */
struct lock_infol {
  uint16  share;            /* on if lock is shared, else off */
  uint16  pad;              /* pad to 4 bytes */
  int64  start;            /* start offset of lock           */
  int64  length;           /* count of bytes to be locked    */
};
/* longlong unlock structure to be passed into jfs_lock function */
struct unlock_infol {
  int64  start;            /* start offset of lock           */
  int64  length;           /* count of bytes to be locked    */
};

/*
 *      vnode (virtual inode)
 *
 * there is a single vnode for every inode,
 */
struct vnode {
        uint32 v_flag;          /* flags for this object                */
        uint32 v_count;         /* the use count of this vnode          */
        struct vfs *v_vfsp;     /* pointer to the vfs of this vnode     */
        struct gnode *v_gnode;  /* ptr to implementation gnode          */
        struct vnode *v_next;   /* ptr to next vnode for the gnode      */
                                /* this field is not used               */
        struct vnode *v_vfsnext; /* ptr to next vnode on list off of vfs */
        struct vnode *v_vfsprev; /* ptr to prev vnode on list off of vfs */
        uint32 v_reserved[5];   /* make it 48 bytes, like AIX           */
};

/* vnode flags */
#define V_ROOT  0x1             /* root node of file system */

/*
 *      vnode types
 *
 * N.B. DO NOT rearrange/redefine these first 10!
 * VNON means no type.
 */
enum vtype { VNON, VREG, VDIR, VBLK, VCHR, VLNK, VSOCK, VBAD, VFIFO, VMPC };
#define VUNDEF  VNON            /* undefined is same as nothing */

#pragma pack(4)
/*
 *      gnode (generic inode)
 *
 * embedded in inode
 */
struct gnode {
        enum vtype gn_type;             /* type of object: VDIR,VREG,... */
        uint32  gn_flags;               /* attributes of object         */
        uint32  gn_seg;                 /* segment into which file is mapped */
        uint32  gn_lgcnt;               /* total legacy opens (< 2G files) */
        uint32  gn_rdcnt;               /* total opens for read         */
        uint32  gn_wrcnt;               /* total opens for write        */
        uint32  gn_drcnt;               /* total opens with deny read   */
        uint32  gn_dwcnt;               /* total opens with deny write  */
        TSPINLOCK_T gn_byte_lock;       /* transforming spin lock for   */
                                        /* file locking */
        struct file_lock * gn_filelocks;    /* Ptr to list of file locks    */
        struct file_lock * gn_pendinglocks; /* Ptr to pending locks on file */
        uint32  gn_reserved[2];         /* make 68 bytes, like AIX      */
        struct  vnode *gn_vnode;        /* ptr to vnode for this gnode */
        dev_t   gn_rdev;        /* for devices, their "dev_t" */
        caddr_t gn_data;        /* ptr to private data (usually contiguous) */
};
#pragma pack()

/* gn_flags */
#define GNF_TCB         0x0001  /* gnode corresponds to a file in the TCB */
#define GNF_WMAP        0x0002  /* mapped for writing at some time      */
#define GNF_NSHARE      0x0004  /* opened non share                     */

#define i_lgcnt i_gnode.gn_lgcnt
#define i_drcnt i_gnode.gn_drcnt
#define i_dwcnt i_gnode.gn_dwcnt


/*
 *      vattr
 *
 * attribute descriptor of files across all the types of file systems;
 * a superset of the AIX stat structure's fields.
 */
struct vattr
{
        enum vtype      va_type;        /* vnode type                   */
        mode_t  va_mode;                /* access mode                  */
        uid_t   va_uid;                 /* owner uid                    */
        gid_t   va_gid;                 /* owner gid                    */
        dev_t   va_dev;                 /* id of device containing file */
        long    va_serialno;            /* file serial (inode) number   */
        short   va_nlink;               /* number of links              */
        short   va_flags;               /* Flags, see below for define  */
        long    va_llpad;               /* CSet++'s long long pad       */
        offset_t va_size;               /* file size in bytes           */
        long    va_blocksize;           /* preferred blocksize for io   */
        long    va_blocks;              /* kbytes of disk held by file  */
        struct  timestruc_t  va_atime;  /* time of last access */
        struct  timestruc_t  va_mtime;  /* time of last data modification */
        struct  timestruc_t  va_ctime;  /* time of last status change */
        struct  timestruc_t  va_otime;  /* time of creation */
        dev_t   va_rdev;                /* id of device                 */
};

/* macro for device assignment */
#define brdev(dev)      (dev)
#define minor(dev)      (dev)

/* vattr timestamp:
 * time in vattr in now two longs, but some measure of back compatibility
 * is maintained, the short times are "tim" vs "time"
 */
#define va_atim va_atime.tv_sec
#define va_mtim va_mtime.tv_sec
#define va_ctim va_ctime.tv_sec
#define va_otim va_otime.tv_sec

/* vn_setattr flag: Must be mutually exclusive */
#define V_MODE  0x01
#define V_OWN   0x02
#define V_UTIME 0x04
#define V_STIME 0x08


/*
 *      CREDENTIALS
 */
/* This is an abbreviated version of a credentials structure.
 * Only the user id and group id are interesting in this environment
 * -- and they are always set to 0.
 */
struct ucred {
        uid_t   cr_uid;                 /* effective user id */
        gid_t   cr_gid;                 /* effective group id */
};

extern struct ucred *pcred;

#endif /*  _H_AIXISMS */
