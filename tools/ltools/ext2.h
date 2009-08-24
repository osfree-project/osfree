/* ext2.h
 *
 * This file contains important file system information like
 * the superblock, inode and group structures and a few global
 * variables.

   Copyright information and copying policy see file README.TXT

   History see file MAIN.C
 */

#ifndef _EXT2_H_
#define _EXT2_H_

/* This is screwy because under Linux (at least on Jason't system)
 * sys/stat.h calls sys/types.h which calls Linux/types.h which is
 * asm/types.h which already has __xx.  Get that?  So I made it _xx here
 * to avoid conflicts.
 * --Jason
 */

typedef signed char _s8;
typedef unsigned char _u8;
typedef unsigned char byte;

typedef signed short _s16;
typedef unsigned short _u16;

typedef signed long _s32;
typedef unsigned long _u32;

#define EXT2_SUPER_MAGIC 0xEF53
#define EXT2_MIN_BLOCK  1024
#define EXT2_MIN_FRAG   1024

#define	EXT2_NDIR_BLOCKS 12
#define	EXT2_IND_BLOCK  EXT2_NDIR_BLOCKS
#define	EXT2_DIND_BLOCK	(EXT2_IND_BLOCK + 1)
#define	EXT2_TIND_BLOCK	(EXT2_DIND_BLOCK + 1)
#define	EXT2_N_BLOCKS	(EXT2_TIND_BLOCK + 1)


#undef S_IFMT
#undef S_IFSOCK
#undef S_IFLNK
#undef S_IFFIL
#undef S_IFBLK
#undef S_IFDIR
#undef S_IFCHR
#undef S_IFIFO

#undef S_ISSOCK
#undef S_ISLNK
#undef S_ISFIL
#undef S_ISBLK
#undef S_ISDIR
#undef S_ISCHR
#undef S_ISFIFO

#define S_IFMT   0x0F000					/*017 0000 */

#define S_IFSOCK 0x0C000					/*014 0000 */
#define S_IFLNK  0x0A000					/*012 0000 */
#define S_IFFIL  0x08000					/*010 0000 */
#define S_IFBLK  0x06000					/*006 0000 */
#define S_IFDIR  0x04000					/*004 0000 */
#define S_IFCHR  0x02000					/*002 0000 */
#define S_IFIFO  0x01000					/*001 0000 */

#define S_ISSOCK(m)     (((m) & S_IFMT) == S_IFSOCK)
#define S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)
#define S_ISFIL(m)      (((m) & S_IFMT) == S_IFFIL)
#define S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)
#define S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)

/*not yet supported: */
#define S_IFUID  0x00800					/*000 4000 */
#define S_IFGID  0x00400					/*000 2000 */
#define S_IFVTX  0x00200					/*000 1000 */


/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
#define EXT2_FT_UNKNOWN		0
#define EXT2_FT_REG_FILE	1
#define EXT2_FT_DIR		2
#define EXT2_FT_CHRDEV		3
#define EXT2_FT_BLKDEV 		4
#define EXT2_FT_FIFO		5
#define EXT2_FT_SOCK		6
#define EXT2_FT_SYMLINK		7
#define EXT2_FT_MAX		8

#define EXT2_GOOD_OLD_REV	 0
#define EXT2_DYNAMIC_REV	 1
#define EXT2_GOOD_OLD_INODE_SIZE 128

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC	0x0001

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE	0x0002
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR	0x0004

#define EXT2_FEATURE_INCOMPAT_COMPRESSION	0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE		0x0002

/*
 * Super Block Structure
 */
typedef struct
{
    _u32 s_inodes_count;					/* Inodes count */
    _u32 s_blocks_count;					/* Blocks count */
    _u32 s_r_blocks_count;					/* Reserved blocks count */
    _u32 s_free_blocks_count;					/* Free blocks count */
    _u32 s_free_inodes_count;					/* Free inodes count */
    _u32 s_first_data_block;					/* First Data Block */
    _u32 s_log_block_size;					/* Block size */
    _s32 s_log_frag_size;					/* Fragment size */
    _u32 s_blocks_per_group;					/* # Blocks per group */
    _u32 s_frags_per_group;					/* # Fragments per group */
    _u32 s_inodes_per_group;					/* # Inodes per group */
    _u32 s_mtime;						/* Mount time */
    _u32 s_wtime;						/* Write time */
    _u16 s_mnt_count;						/* Mount count */
    _s16 s_max_mnt_count;					/* Maximal mount count */
    _u16 s_magic;						/* Magic signature */
    _u16 s_state;						/* File system state */
    _u16 s_errors;						/* Behaviour when detecting errors */
    _u16 s_minor_rev_level;					/* Minor revision level */
    _u32 s_lastcheck;						/* time of last check */
    _u32 s_checkinterval;					/* max. time between checks */
    _u32 s_creator_os;						/* OS */
    _u32 s_rev_level;						/* Revision level */
    _u16 s_def_resuid;						/* Default uid for reserved blocks */
    _u16 s_def_resgid;						/* Default gid for reserved blocks */

    /* These fields are for EXT2_DYNAMIC_REV superblocks only. */
    _u32 s_first_ino; 		/* First non-reserved inode */
    _u16 s_inode_size; 		/* size of inode structure */
    _u16 s_block_group_nr; 	/* block group # of this superblock */
    _u32 s_feature_compat; 	/* compatible feature set */
    _u32 s_feature_incompat; 	/* incompatible feature set */
    _u32 s_feature_ro_compat; 	/* readonly-compatible feature set */
    _u32 s_pad[230];	/* Padding to the end of the block */
}
super_block;

/*
 * Feature set definitions
 */
#define EXT2_FEATURE_COMPAT_DIR_PREALLOC	0x0001

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE	0x0002
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR	0x0004

#define EXT2_FEATURE_INCOMPAT_COMPRESSION	0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE		0x0002

#define EXT2_FEATURE_COMPAT_SUPP	0
#define EXT2_FEATURE_INCOMPAT_SUPP	EXT2_FEATURE_INCOMPAT_FILETYPE
#define EXT2_FEATURE_RO_COMPAT_SUPP	(EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER| \
					 EXT2_FEATURE_RO_COMPAT_LARGE_FILE| \
					 EXT2_FEATURE_RO_COMPAT_BTREE_DIR)

/*
 * Group Descriptor Structure
 */
typedef struct
{
    _u32 bg_block_bitmap;					/* Blocks bitmap block */
    _u32 bg_inode_bitmap;					/* Inodes bitmap block */
    _u32 bg_inode_table;					/* Inodes table block */
    _u16 bg_free_blocks_count;					/* Free blocks count */
    _u16 bg_free_inodes_count;					/* Free inodes count */
    _u16 bg_used_dirs_count;					/* Directories count */
    _u16 bg_pad;
    _u32 bg_reserved[3];
}
group_desc;

/*
 * Inode Structure
 */
typedef struct
{
    _u16 i_mode;						/* File mode */
    _u16 i_uid;							/* Owner Uid */
    _u32 i_size;						/* Size in bytes */
    _u32 i_atime;						/* Access time */
    _u32 i_ctime;						/* Creation time */
    _u32 i_mtime;						/* Modification time */
    _u32 i_dtime;						/* Deletion Time */
    _u16 i_gid;							/* Group Id */
    _u16 i_links_count;						/* Links count */
    _u32 i_blocks;						/* Blocks count */
    _u32 i_flags;						/* File flags */
    _u32 i_reserved1;						/* Reserved 1 */
    _u32 i_block[15];						/* Pointers to blocks */
    _u32 i_version;						/* File version (for NFS) */
    _u32 i_file_acl;						/* File ACL */
    _u32 i_dir_acl;						/* Directory ACL */
    _u32 i_faddr;						/* Fragment address */
    _u8 i_frag;							/* Fragment number */
    _u8 i_fsize;						/* Fragment size */
    _u16 i_pad1;
    _u16 i_uid_high;						/* high bits of uid */
    _u16 i_gid_high;						/* high bits of gid */
    _u32 i_reserved2;
// _u32 i_reserved2[2];						/* Reserved 2 */
}
inode;

/*
 * Directory Structure
 */
struct dir
{
    _u32 inode_num;
    _u16 rec_len;
    _u8 name_len;
    _u8 file_type;
    char name[256];						/* between 0 and 256 chars */
};

/*
 * Partition Table Entry
 */
typedef struct
{
    _u8 bootIndicator;
    _u8 startHead;
    _u8 startSector;
    _u8 startCylinder;
    _u8 system;
    _u8 endHead;
    _u8 endSector;
    _u8 endCylinder;
    _u32 leadSectors;
    _u32 numSectors;
}
partitionTableEntry;

#endif								/* _EXT2_H_ */
