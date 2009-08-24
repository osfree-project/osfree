/* $Id: jfs_dinode.h,v 1.1 2000/04/21 10:57:56 ktk Exp $ */

/* static char *SCCSID = "@(#)1.11.1.2  12/2/99 11:13:17 src/jfs/common/include/jfs_dinode.h, sysjfs, w45.fs32, fixbld";
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

#ifndef _H_JFS_DINODE
#define _H_JFS_DINODE

/*
 *      jfs_dinode.h: on-disk inode manager
 *
 */

#ifndef _JFS_OS2
#include <sys/types.h>
#include <sys/mode.h>
#include <sys/time.h>
#include <sys/lock_def.h>
#endif /* _JFS_OS2 */

#include "jfs_types.h"

#define INODESLOTSIZE           128
#define L2INODESLOTSIZE         7
#define log2INODESIZE           9      /* log2(bytes per dinode) */

/* extended mode bits (on-disk inode i_mode) */
#define IFJOURNAL       0x00010000      /* journalled file */
#define ISPARSE         0x00020000      /* sparse file enabled */
#define INLINEEA        0x00040000      /* inline EA area free */


/*
 *      on-disk inode (dinode_t): 512 bytes
 *
 * note: align 64-bit fields on 8-byte boundary.
 */
struct dinode
{
        /*
         *      I. base area (128 bytes)
         *      ------------------------
         *
         * define generic/POSIX attributes
         */
        uint32  di_inostamp;    /* 4: stamp to show inode belongs to fileset */
        int32   di_fileset;     /* 4: fileset number */
        ino_t   di_number;      /* 4: inode number, aka file serial number */
        uint32  di_gen;         /* 4: inode generation number */

        pxd_t   di_ixpxd;       /* 8: inode extent descriptor */

        int64   di_size;        /* 8: size */
        int64   di_nblocks;     /* 8: number of blocks allocated */

        uint32   di_nlink;      /* 4: number of links to the object */    /* D233784 */

        uid_t   di_uid;         /* 4: user id of owner */
        gid_t   di_gid;         /* 4: group id of owner */

        mode_t  di_mode;        /* 4: attribute, format and permission */

        struct timestruc_t  di_atime;   /* 8: time last data accessed */
        struct timestruc_t  di_ctime;   /* 8: time last status changed */
        struct timestruc_t  di_mtime;   /* 8: time last data modified */
        struct timestruc_t  di_otime;   /* 8: time created */

        dxd_t   di_acl;         /* 16: acl descriptor */

        dxd_t   di_ea;          /* 16: ea descriptor */

        int32   di_compress;    /* 4: compression */

        int32   di_acltype;     /* 4: Type of ACL */

        /*
         *      II. extension area (128 bytes)
         *      ------------------------------
         */
        /*
         *      extended attributes for file system (96);
         */
        union {
                uint8   _data[96];

                /*
                 *      DFS VFS+ support (preliminary place holder)
                 */
                struct {
                        uint8   _data[96];
                } _dfs;

                /*
                 *      block allocation map
                 */
                struct {
                        struct bmap *__bmap;    /* 4: incore bmap descriptor */
                } _bmap;
#define di_bmap         _data2._bmap.__bmap

                /*
                 *      inode allocation map (fileset inode 1st half)
                 */
                struct {
                        struct inomap *__imap;  /* 4: incore imap control */
                        uint32  _gengen;        /* 4: di_gen generator */
                } _imap;
        } _data2;
#define di_imap         _data2._imap.__imap
#define di_gengen       _data2._imap._gengen

        /*
         *      B+-tree root header (32)
         *
         * B+-tree root node header, or
         * data extent descriptor for inline data;
         * N.B. must be on 8-byte boundary.
         */
        dasd_t  di_DASD;        // 16: DASD limit info for directories  F226941
        dxd_t   di_dxd;         /* 16: data extent descriptor */
#define di_btroot       di_DASD                                 // F226941
#define di_parent       di_dxd.size

        /*
         *      III. type-dependent area (128 bytes)
         *      ------------------------------------
         *
         * B+-tree root node xad array or inline data
         */
        union {
                uint8   _data[128];
#define di_inlinedata   _data3._data

                /*
                 *      regular file or directory
                 *
                 * B+-tree root node/inline data area
                 */
                struct {
                        uint8   _xad[128];
                } _file;

                /*
                 *      device special file
                 */
                struct {
                        dev_t   _rdev;       /* device major and minor */
                } _specfile;
#define di_rdev         _data3._specfile._rdev

                /*
                 *      symbolic link.
                 *
                 * link is stored in inode if its length is less than
                 * D_PRIVATE. Otherwise stored like a regular file.
                 */
                struct {
                        uint8   _fastsymlink[128];
                } _symlink;
#define di_fastsymlink  _data3._symlink._fastsymlink
        } _data3;

        /*
         *      IV. type-dependent extension area (128 bytes)
         *      -----------------------------------------
         *
         *      user-defined attribute, or
         *      inline data continuation, or
         *      B+-tree root node continuation
         *
         */
        union {
                uint8   _data[128];
#define di_inlineea     _data4._data
        } _data4;
};

typedef struct dinode   dinode_t;


/*
 *      dfs: fileset inode 2nd half (preliminary place holder)
 */
struct filset_dinode2 {
        uint8   _data[512];
};

#endif /*_H_JFS_DINODE */
