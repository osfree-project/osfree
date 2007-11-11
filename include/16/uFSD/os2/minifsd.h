//
// $Header: /cur/cvsroot/boot/include/muFSD/os2/minifsd.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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

#ifndef __minifsd_h
#define __minifsd_h

#pragma pack(1)
struct minifsd_to_fsd_data {
        /*
         * magic number for sanity checks
         */
        magic_t        mfsdata_magic;           // always MINIFSD_DATA_MAGIC

        /*
         * file section
         */
#ifdef MINIFSD
        struct file  **used_hfiles;
        struct file  **free_hfiles;
        long          *nhfiles;
        long          *nfreehfiles;
        long          *nusedhfiles;
        void         (*file_remove_from_list)(struct file *, struct file **, long *);
#else
        PTR16          used_hfiles;
        PTR16          free_hfiles;
        PTR16          nhfiles;
        PTR16          nfreehfiles;
        PTR16          nusedhfiles;
        PTR16          file_remove_from_list;
#endif

        /*
         * inode section
         */
#ifdef MINIFSD
        struct inode           **first_inode;
        long                    *nr_inodes;
        long                    *nr_free_inodes;
        struct inode_hash_entry *hash_table;
        struct inode            *inode_table;
#else
        PTR16                    first_inode;
        PTR16                    nr_inodes;
        PTR16                    nr_free_inodes;
        PTR16                    hash_table;
        PTR16                    inode_table;
#endif


        /*
         * superblock section
         */
#ifdef MINIFSD
        struct super_block **used_supers;
        struct super_block **free_supers;
        long                *nsupers;
        long                *nfreesupers;
        long                *nusedsupers;
        void               (*super_remove_from_list)(struct super_block *, struct super_block **, long *);
#else
        PTR16          used_supers;
        PTR16          free_supers;
        PTR16          nsupers;
        PTR16          nfreesupers;
        PTR16          nusedsupers;
        PTR16          super_remove_from_list;
#endif

};
#pragma pack()

void buffer_stage1_to_stage2(void);
void inherit_minifsd_inodes(struct minifsd_to_fsd_data  *mfs_data);
void inherit_minifsd_files(struct minifsd_to_fsd_data  *mfs_data);
void inherit_minifsd_supers(struct minifsd_to_fsd_data  *mfs_data);

#endif /* __minifsd_h */
