/* LTOOLS
   Programs to read, write, delete and change Linux extended 2 filesystems under DOS

   Module inode.c
   This file contains routines for handling inodes

   Copyright information and copying policy see file README.TXT

   History see file MAIN.C
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ext2.h"
#include "proto.h"
#include "ldir.h"

extern int isDebug;
extern int DebugOut(int level, const char *fmt,...);

extern unsigned short NBLOCK;
extern super_block sb;
extern group_desc *gt;
extern unsigned short BLOCK_SIZE;
#ifdef UNIX
#define STDOUT stdout
#define STDERR stdout
#else
extern FILE *STDOUT, *STDERR;
#endif

#ifndef max
#define max(a,b) (a>b ? a : b)
#endif
#ifndef min
#define min(a,b) (a<b ? a : b)
#endif

extern enum
{
    LDIR, LREAD, LWRITE, LTEST
}
modus;
extern enum
{
    CHANGE, DEL, COPY, REN
}
u_modus;
/*CHANGE    =change file access rights, GID or UID
   DEL      =delete file
   WRITE            =write file
   COPY     =copy file from DOS to Linux
 */
extern enum
{
    SDIR, SFILE
}
source_type;
extern unsigned int disk_no;

extern void *MALLOC(size_t size);
extern void FREE(void *block);


#ifdef UNIX
/* convert disk to CPU byteorder */
void inode_fix_byteorder(inode * i)
{
    unsigned long block;

    i->i_mode = le16_to_cpu(i->i_mode);
    i->i_uid = le16_to_cpu(i->i_uid);
    i->i_uid_high = le16_to_cpu(i->i_uid_high);
    i->i_gid = le16_to_cpu(i->i_gid);
    i->i_gid_high = le16_to_cpu(i->i_gid_high);
    i->i_links_count = le16_to_cpu(i->i_links_count);
    i->i_size = le32_to_cpu(i->i_size);
    i->i_atime = le32_to_cpu(i->i_atime);
    i->i_ctime = le32_to_cpu(i->i_ctime);
    i->i_mtime = le32_to_cpu(i->i_mtime);
    i->i_dtime = le32_to_cpu(i->i_dtime);
    i->i_blocks = le32_to_cpu(i->i_blocks);
    i->i_flags = le32_to_cpu(i->i_flags);
    i->i_faddr = le32_to_cpu(i->i_faddr);
    i->i_file_acl = le32_to_cpu(i->i_file_acl);
    i->i_version = le32_to_cpu(i->i_version);
    if (!S_ISLNK(i->i_mode))
	for (block = 0; block < EXT2_N_BLOCKS; block++)
	    i->i_block[block] = le32_to_cpu(i->i_block[block]);
    if ( (!S_ISDIR(i->i_mode)) && i->i_dir_acl)
      {
	fprintf(STDERR,"WARNING: file seems longer than 4GB, may be truncated\n");
	if (modus == LWRITE)
	  exit(1);
	i->i_size=(_u32)-1;
      }
}

#endif


/* load_inode

 * Read the inode with the given inode number in the inode table
 *
 * Return NULL for failure
 *        address for success
 */
inode *load_inode(unsigned long inode_no)
{
    static inode res;
    unsigned long lba;
    unsigned short offset;

#ifdef UNIX
    unsigned long block;
#endif

    DebugOut(32,"----------Executing load_inode---# %li---------------------\n", inode_no);

    if (inode_no < 1 || inode_no > sb.s_inodes_count)
    {
	fprintf(STDERR, "Inode value %lu was out of range in load_inode.\n Limit is %ld\n",
		inode_no, sb.s_inodes_count);
	return (NULL);						/* Inode out of range */
    }
/*
    loc = gt[(inode_no - 1) / sb.s_inodes_per_group].bg_inode_table * BLOCK_SIZE +
	((inode_no - 1) % sb.s_inodes_per_group) * sizeof(inode);
    lba = loc / DISK_BLOCK_SIZE;
    offset = loc % DISK_BLOCK_SIZE;
*/
    toLba(gt[(inode_no - 1) / sb.s_inodes_per_group].bg_inode_table, (inode_no - 1) % sb.s_inodes_per_group, &lba, &offset);

    if (readdisk((byte *) & res, lba, offset, sizeof(inode)) != sizeof(inode))
    {
	fprintf(STDERR, "Disk problem in load_inode.\n");
	return (NULL);
    } else
    {
	if (isDebug & 32) print_inode(&res, inode_no);

#ifdef UNIX
	inode_fix_byteorder(&res);
#endif
	return (&res);
    }
}

/* Modify the block bitmap
   mode=0 : clear bit in bitmap (bitmap is loaded if necessary) -> Success  : return 0 Fail: exit -1
   mode=1 : set   bit in bitmap (bitmap is loaded if necessary) -> Success  : return 0 Fail: exit -1
   mode=2 : test  bit in bitmap (bitmap is loaded if necessary) -> Bit clear: return 0 set:  return 1
   mode=3 : discard bitmap without writing it to disk              ->                    return 0
   mode=4 : write bitmap to disk and discard it                 -> Success  : return 0 Fail: exit -1
 */
int modifyBlockBitmap(unsigned long block_num, int mode)
{
    static byte *bitmap = NULL, modified = 0;
    byte Bit;
    unsigned long Byte, group_block_num, group_num;
    static unsigned long active_group = 0;
    unsigned long lba;
    unsigned short offset;

    DebugOut(32,"----------Executing modifyBlockBitmap---Block:%ld--mode:%d------\n", block_num, mode);

    switch (mode)
    {
	case (0):
	case (1):
	case (2):
	    {	group_num = (block_num - sb.s_first_data_block) / sb.s_blocks_per_group;
/*               printf("Block_num=%ld  group_num=%ld active_group=%ld\n",block_num,group_num,active_group); */

		if (bitmap == NULL)
		{
		    if ((bitmap = (byte *) MALLOC(BLOCK_SIZE)) == NULL)		/* Allocate memory for inode bitmap */
		    {
			fprintf(STDERR, "Memory problem in modifyBlockBitmap.\n");
			exit(-1);
		    }
		    lba=gt[group_num].bg_block_bitmap * (BLOCK_SIZE / DISK_BLOCK_SIZE);
		    offset=(gt[group_num].bg_block_bitmap * BLOCK_SIZE) % DISK_BLOCK_SIZE;
		    if (readdisk(bitmap, lba, offset, BLOCK_SIZE) != BLOCK_SIZE)
		    {
			fprintf(STDERR, "Disk problem in modifyBlockBitmap\n");
			exit(-1);
		    }
		    active_group = group_num;
		    modified = 0;
/*                       printf("Reading in new bitmap for active_group=%ld from block=%ld\n",active_group,gt[group_num].bg_block_bitmap); */
		}
		if (group_num != active_group)
		{
		    if (modified != 0)
		    {
/*                      printf("Writing bitmap for active group=%ld to block=%ld\n",active_group,gt[active_group].bg_block_bitmap); */
			lba=gt[active_group].bg_block_bitmap * (BLOCK_SIZE / DISK_BLOCK_SIZE);
			offset=(gt[active_group].bg_block_bitmap * BLOCK_SIZE) % DISK_BLOCK_SIZE;
			writedisk(bitmap, lba, offset, BLOCK_SIZE);
		    };
    		    lba=gt[group_num].bg_block_bitmap * (BLOCK_SIZE / DISK_BLOCK_SIZE);
		    offset=(gt[group_num].bg_block_bitmap * BLOCK_SIZE) % DISK_BLOCK_SIZE;
		    if (readdisk(bitmap, lba, offset, BLOCK_SIZE) != BLOCK_SIZE)
		    {
			fprintf(STDERR, "Disk problem in modifyBlockBitmap\n");
			exit(-1);
		    }
/*                   printf("Reading in new bitmap for active_group=%ld from block=%ld\n",active_group,gt[group_num].bg_block_bitmap); */
		    active_group = group_num;
		    modified = 0;
		}
		if (block_num < sb.s_first_data_block)
		{
		    fprintf(STDERR, "wrong block number in modifyBlockBitmap: %ld\n", block_num);
		    exit(-1);
		}
		if ((group_block_num = (block_num - sb.s_first_data_block) % sb.s_blocks_per_group) >= (8*BLOCK_SIZE))
		{
		    fprintf(STDERR, "wrong block number in modifyBlockBitmap: %ld\n", block_num);
		    exit(-1);
		}

		Bit  = group_block_num % 8;
		Byte = group_block_num / 8;

		if (mode == 0)					/*clear bit */
		{
		  //printf("Clearing block_num=%lu   group_block_num=%lu   Bit=%u   Byte=%u  \n",block_num, group_block_num, Bit, Byte);

		    if (Bit == 0)
			bitmap[Byte] = bitmap[Byte] & 0xFE;	/*1111 1110B */
		    else if (Bit == 1)
			bitmap[Byte] = bitmap[Byte] & 0xFD;	/*1111 1101B */
		    else if (Bit == 2)
			bitmap[Byte] = bitmap[Byte] & 0xFB;	/*1111 1011B */
		    else if (Bit == 3)
			bitmap[Byte] = bitmap[Byte] & 0xF7;	/*1111 0111B */
		    else if (Bit == 4)
			bitmap[Byte] = bitmap[Byte] & 0xEF;	/*1110 1111B */
		    else if (Bit == 5)
			bitmap[Byte] = bitmap[Byte] & 0xDF;	/*1101 1111B */
		    else if (Bit == 6)
			bitmap[Byte] = bitmap[Byte] & 0xBF;	/*1011 1111B */
		    else if (Bit == 7)
			bitmap[Byte] = bitmap[Byte] & 0x7F;	/*0111 1111B */
		    modified = 1;
		} else if (mode == 1)				/*set bit */
		{
		    if (Bit == 0)
			bitmap[Byte] = bitmap[Byte] | 0x01;
		    else if (Bit == 1)
			bitmap[Byte] = bitmap[Byte] | 0x02;
		    else if (Bit == 2)
			bitmap[Byte] = bitmap[Byte] | 0x04;
		    else if (Bit == 3)
			bitmap[Byte] = bitmap[Byte] | 0x08;
		    else if (Bit == 4)
			bitmap[Byte] = bitmap[Byte] | 0x10;
		    else if (Bit == 5)
			bitmap[Byte] = bitmap[Byte] | 0x20;
		    else if (Bit == 6)
			bitmap[Byte] = bitmap[Byte] | 0x40;
		    else if (Bit == 7)
			bitmap[Byte] = bitmap[Byte] | 0x80;
		    modified = 1;
		} else
		    /*  mode==2   test bit */
		{
		    if (Bit == 0)
			if (bitmap[Byte] & 0x01)
			    return (1);
			else
			    return (0);
		    if (Bit == 1)
			if (bitmap[Byte] & 0x02)
			    return (1);
			else
			    return (0);
		    if (Bit == 2)
			if (bitmap[Byte] & 0x04)
			    return (1);
			else
			    return (0);
		    if (Bit == 3)
			if (bitmap[Byte] & 0x08)
			    return (1);
			else
			    return (0);
		    if (Bit == 4)
			if (bitmap[Byte] & 0x10)
			    return (1);
			else
			    return (0);
		    if (Bit == 5)
			if (bitmap[Byte] & 0x20)
			    return (1);
			else
			    return (0);
		    if (Bit == 6)
			if (bitmap[Byte] & 0x40)
			    return (1);
			else
			    return (0);
		    if (Bit == 7)
			if (bitmap[Byte] & 0x80)
			    return (1);
			else
			    return (0);
		}
		return 0;
	    }
	case (3):
	case (4):
	    {
		if ((mode == 4) && (bitmap != NULL))
		{   lba=gt[active_group].bg_block_bitmap * (BLOCK_SIZE / DISK_BLOCK_SIZE);
		    offset=(gt[active_group].bg_block_bitmap * BLOCK_SIZE) % DISK_BLOCK_SIZE;

		    if (writedisk(bitmap, lba, offset, BLOCK_SIZE) != BLOCK_SIZE)
		    {
			fprintf(STDERR, "Disk problem in modifyBlockBitmap\n");
			exit(-1);
		    }
/*                   printf("Writing bitmap for active group=%ld to block=%ld\n",active_group,gt[active_group].bg_block_bitmap); */
		    FREE(bitmap);				/*free memory for bitmap */
		    bitmap = NULL;
		    modified = 0;
		}
	    }
    }
    return 0;
}


/* Modifiy the inode bitmap
   mode=0 : clear bit in bitmap (bitmap is loaded if necessary) -> Success  : return 0 Fail: exit -1
   mode=1 : set   bit in bitmap (bitmap is loaded if necessary) -> Success  : return 0 Fail: exit -1
   mode=2 : test  bit in bitmap (bitmap is loaded if necessary) -> Bit clear: return 0 set:  return 1
   mode=3 : discard bitmap without writing it to disk              ->                    return 0
   mode=4 : write bitmap to disk and discard it                 -> Success  : return 0 Fail: exit -1
 */
int modifyInodeBitmap(unsigned long inode_num, int mode)
{
    static byte *bitmap = NULL, modified = 0;
    byte Bit;
    unsigned long Byte, group_inode_num, group_num;
    static unsigned long active_group = 0;
    unsigned long lba;
    unsigned short offset;

    DebugOut(32,"----------Executing modifyInodeBitmap---Inode:%ld--mode:%d------\n", inode_num, mode);

    switch (mode)
    {
	case (0):
	case (1):
	case (2):
	    {
		group_num = (inode_num - 1) / sb.s_inodes_per_group;

/*               printf("Inode_num=%ld  group_num=%ld active_group=%ld\n",inode_num,group_num,active_group); */

		if (bitmap == NULL)
		{
		    if ((bitmap = (byte *) MALLOC(BLOCK_SIZE)) == NULL)		/* Allocate memory for bitmap */
		    {
			fprintf(STDERR, "Memory problem in modifyInodeBitmap.\n");
			exit(-1);
		    }
    		    lba=gt[group_num].bg_inode_bitmap * (BLOCK_SIZE / DISK_BLOCK_SIZE);
		    offset=(gt[group_num].bg_inode_bitmap * BLOCK_SIZE) % DISK_BLOCK_SIZE;
		    if (readdisk(bitmap, lba, offset, BLOCK_SIZE) != BLOCK_SIZE)
		    {
			fprintf(STDERR, "Disk problem in modifyInodeBitmap\n");
			exit(-1);
		    }
/*                   printf("Reading in new bitmap for active_group=%ld from block=%ld\n",active_group,gt[group_num].bg_inode_bitmap); */
		    active_group = group_num;
		    modified = 0;
		}
		if (group_num != active_group)
		{
		    if (modified != 0)
		    {
    		    	lba=gt[active_group].bg_inode_bitmap * (BLOCK_SIZE / DISK_BLOCK_SIZE);
		    	offset=(gt[active_group].bg_inode_bitmap * BLOCK_SIZE) % DISK_BLOCK_SIZE;
			writedisk(bitmap, lba, offset, BLOCK_SIZE);
/*                   printf("Writing bitmap for active group=%ld to block=%ld\n",active_group,gt[active_group].bg_inode_bitmap); */
		    };
    		    lba=gt[group_num].bg_inode_bitmap * (BLOCK_SIZE / DISK_BLOCK_SIZE);
		    offset=(gt[group_num].bg_inode_bitmap * BLOCK_SIZE) % DISK_BLOCK_SIZE;
		    if (readdisk(bitmap, lba, offset, BLOCK_SIZE) != BLOCK_SIZE)
		    {
			fprintf(STDERR, "Disk problem in modifyInodeBitmap\n");
			exit(-1);
		    }
/*                   printf("Reading in new bitmap for active_group=%ld from block=%ld\n",active_group,gt[group_num].bg_inode_bitmap); */
		    active_group = group_num;
		    modified = 0;
		}
		if (inode_num == 0)
		{
		    fprintf(STDERR, "wrong inode number in modifyInodeBitmap: %ld\n", inode_num);
		    exit(-1);
		}
		if ((group_inode_num = (inode_num - 1) % sb.s_inodes_per_group) >= (8*BLOCK_SIZE))
		{
		    fprintf(STDERR, "wrong inode number in modifyInodeBitmap: %ld\n", inode_num);
		    exit(-1);
		}
		Bit = group_inode_num % 8;
		Byte = group_inode_num / 8;

		if (mode == 0)					/*clear bit */
		{
		    if (Bit == 0)
			bitmap[Byte] = bitmap[Byte] & 0xFE;	/*1111 1110B */
		    else if (Bit == 1)
			bitmap[Byte] = bitmap[Byte] & 0xFD;	/*1111 1101B */
		    else if (Bit == 2)
			bitmap[Byte] = bitmap[Byte] & 0xFB;	/*1111 1011B */
		    else if (Bit == 3)
			bitmap[Byte] = bitmap[Byte] & 0xF7;	/*1111 0111B */
		    else if (Bit == 4)
			bitmap[Byte] = bitmap[Byte] & 0xEF;	/*1110 1111B */
		    else if (Bit == 5)
			bitmap[Byte] = bitmap[Byte] & 0xDF;	/*1101 1111B */
		    else if (Bit == 6)
			bitmap[Byte] = bitmap[Byte] & 0xBF;	/*1011 1111B */
		    else if (Bit == 7)
			bitmap[Byte] = bitmap[Byte] & 0x7F;	/*0111 1111B */
		    modified = 1;
		} else if (mode == 1)				/*set bit */
		{
		    if (Bit == 0)
			bitmap[Byte] = bitmap[Byte] | 0x01;
		    else if (Bit == 1)
			bitmap[Byte] = bitmap[Byte] | 0x02;
		    else if (Bit == 2)
			bitmap[Byte] = bitmap[Byte] | 0x04;
		    else if (Bit == 3)
			bitmap[Byte] = bitmap[Byte] | 0x08;
		    else if (Bit == 4)
			bitmap[Byte] = bitmap[Byte] | 0x10;
		    else if (Bit == 5)
			bitmap[Byte] = bitmap[Byte] | 0x20;
		    else if (Bit == 6)
			bitmap[Byte] = bitmap[Byte] | 0x40;
		    else if (Bit == 7)
			bitmap[Byte] = bitmap[Byte] | 0x80;
		    modified = 1;
		} else
		    /*  mode==2   test bit */
		{
		    if (Bit == 0)
			if (bitmap[Byte] & 0x01)
			    return (1);
			else
			    return (0);
		    if (Bit == 1)
			if (bitmap[Byte] & 0x02)
			    return (1);
			else
			    return (0);
		    if (Bit == 2)
			if (bitmap[Byte] & 0x04)
			    return (1);
			else
			    return (0);
		    if (Bit == 3)
			if (bitmap[Byte] & 0x08)
			    return (1);
			else
			    return (0);
		    if (Bit == 4)
			if (bitmap[Byte] & 0x10)
			    return (1);
			else
			    return (0);
		    if (Bit == 5)
			if (bitmap[Byte] & 0x20)
			    return (1);
			else
			    return (0);
		    if (Bit == 6)
			if (bitmap[Byte] & 0x40)
			    return (1);
			else
			    return (0);
		    if (Bit == 7)
			if (bitmap[Byte] & 0x80)
			    return (1);
			else
			    return (0);
		}
		return 0;
	    }
	case (3):
	case (4):
	    {
		if ((mode == 4) && (bitmap != NULL))
		{
   		    lba=gt[active_group].bg_inode_bitmap * (BLOCK_SIZE / DISK_BLOCK_SIZE);
		    offset=(gt[active_group].bg_inode_bitmap * BLOCK_SIZE) % DISK_BLOCK_SIZE;
		    if (writedisk(bitmap, lba, offset, BLOCK_SIZE) != BLOCK_SIZE)
		    {
			fprintf(STDERR, "Disk problem in modifyInodeBitmap\n");
			exit(-1);
		    }
/*                   printf("Writing bitmap for active group=%ld to block=%ld\n",active_group,gt[active_group].bg_inode_bitmap); */
		    FREE(bitmap);				/*free memory for bitmap */
		    bitmap = NULL;
		    modified = 0;
		}
	    }
    }
    return 0;
}






/* store_inode

 * Store the inode with the given inode number in the inode table
 * and write the modified inode table to the disk
 *
 *  Return  0 for success
 *         -1 for failure
 */
int store_inode(unsigned long inode_no, inode * i)
{
    unsigned long lba;
    unsigned short offset;
#ifdef UNIX
    unsigned long block;
    inode tempi;
#endif

    DebugOut(32,"----------Executing store_inode---# %li---------------------\n", inode_no);

    if (inode_no < 1 || inode_no > sb.s_inodes_count)
    {
	fprintf(STDERR, "Inode value %lu was out of range in store_inode.\n Limit is %ld\n",
		inode_no, sb.s_inodes_count);

	DebugOut(32,"----------Leaving store_inode------------------------\n");

	return (-1);						/* Inode out of range */
    }
/*  loc = gt[(inode_no - 1) / sb.s_inodes_per_group].bg_inode_table * BLOCK_SIZE +
	((inode_no - 1) % sb.s_inodes_per_group) * sizeof(inode);
    lba = loc / DISK_BLOCK_SIZE;
    offset = loc % DISK_BLOCK_SIZE;
*/
    toLba(gt[(inode_no - 1) / sb.s_inodes_per_group].bg_inode_table, (inode_no - 1) % sb.s_inodes_per_group, &lba, &offset);

#ifdef UNIX
    /* make sure to copy everything : */
    tempi = *i;

    tempi.i_mode = cpu_to_le16(i->i_mode);
    tempi.i_uid = cpu_to_le16(i->i_uid);
    tempi.i_gid = cpu_to_le16(i->i_gid);
    tempi.i_links_count = cpu_to_le16(i->i_links_count);
    tempi.i_size = cpu_to_le32(i->i_size);
    tempi.i_atime = cpu_to_le32(i->i_atime);
    tempi.i_ctime = cpu_to_le32(i->i_ctime);
    tempi.i_mtime = cpu_to_le32(i->i_mtime);
    tempi.i_dtime = cpu_to_le32(i->i_dtime);
    tempi.i_blocks = cpu_to_le32(i->i_blocks);
    tempi.i_flags = cpu_to_le32(i->i_flags);
    tempi.i_faddr = cpu_to_le32(i->i_faddr);
    tempi.i_file_acl = cpu_to_le32(i->i_file_acl);
    tempi.i_version = cpu_to_le32(i->i_version);
    if (!S_ISLNK(i->i_mode))
	for (block = 0; block < EXT2_N_BLOCKS; block++)
	    tempi.i_block[block] = cpu_to_le32(i->i_block[block]);
    else							/* don't fix string */
	for (block = 0; block < EXT2_N_BLOCKS; block++)
	    tempi.i_block[block] = i->i_block[block];

    if (writedisk((byte *) & tempi, lba, offset, sizeof(inode)) != sizeof(inode))
#else
    if (writedisk((byte *) i, lba, offset, sizeof(inode)) != sizeof(inode))
#endif
    {
	fprintf(STDERR, "Disk problem in store_inode.\n");
	return (-1);
    } else
	return (0);
}


/* print_inode

 * print out an inode (good for debugging)
 */
void print_inode(inode * i, unsigned long int inode_num)
{
    unsigned long a_time, c_time, m_time, d_time;

    if (i == NULL)
	return;							/* Early Bailout */
    a_time = i->i_atime + TIMEZONE;
    c_time = i->i_ctime + TIMEZONE;
    m_time = i->i_mtime + TIMEZONE;
    d_time = i->i_dtime;
    printf("Print Inode ---------------");
    if (inode_num)
	printf("#: %li\n", inode_num);
    else
	printf("\n");
    printf("File Mode: %o\n", i->i_mode);
    printf("Owner UID: %u\n", i->i_uid);
    printf("Size (bytes): %lu\n", i->i_size);
    printf("Access Time: %s", ctime((time_t *) & a_time));
    printf("Creation Time: %s", ctime((time_t *) & c_time));
    printf("Modification Time: %s", ctime((time_t *) & m_time));
    printf("Deletion Time: %s", ctime((time_t *) & d_time));
    printf("Owner GID: %u\n", i->i_gid);
    printf("Links Count: %u\n", i->i_links_count);
    printf("(512 byte)Blocks Count: %lu\n", i->i_blocks);
    printf("File Flags: 0x%lX\n", i->i_flags);
    printf("File Version: %lu\n", i->i_version);
    printf("File ACL: %lu\n", i->i_file_acl);
    printf("Directory ACL: %lu\n", i->i_dir_acl);
    printf("Fragment Address: %lu\n", i->i_faddr);
    printf("Fragment Number: %u\n", i->i_frag);
    printf("Fragment Size: %u\n", i->i_fsize);
}

/* block_list

 * This function returns the sequence of blocks used by the inode.
 * The first time it should be called with i equal the inode, all
 * subsequent time with i equal NULL.
 * On error returns 0.
 */
unsigned long block_list(inode * i)
{
    int j;
    static unsigned long block_num;
    static int blocks_per_block;
    static unsigned long blocks_used;
    static unsigned long *sind = NULL;
    static unsigned long *dind = NULL;
    static unsigned long *tind = NULL;
    static unsigned long inode_blocks[15];
    static unsigned long blocks_so_far, s_idx, d_idx, t_idx;

    DebugOut(32,"----------Executing block_list------------------------\n");

    if (i != NULL)						/* New inode, do initialization */
    {
	blocks_per_block = BLOCK_SIZE / 4;
	blocks_used = DISK_BLOCK_SIZE * i->i_blocks / BLOCK_SIZE;

	FREE(sind);
	sind = NULL;
	FREE(dind);
	dind = NULL;
	FREE(tind);
	tind = NULL;

	/* Save vital inode info */
	for (j = 0; j < 15; j++)
	    inode_blocks[j] = i->i_block[j];

	blocks_so_far = 0;					/* Reset block counter */
	s_idx = d_idx = t_idx = 0;				/* Reset index counters */
    }
    if (blocks_so_far == blocks_used)				/* No more blocks to return */
    {
	FREE(tind);
	tind = NULL;
	FREE(dind);
	dind = NULL;
	FREE(sind);
	sind = NULL;
	return (0);						/* Give 'em a zero */
    }
    if (blocks_so_far < 12)					/* Block numbers in inode */
    {
	DebugOut(32,"block_list:%u %ld\n", blocks_so_far, inode_blocks[blocks_so_far]);

	if (inode_blocks[blocks_so_far] == 0)
	    fprintf(STDERR, "Problem 1 in block_list  %lu  %lu\n",blocks_so_far,blocks_used);
	FREE(tind);
	tind = NULL;
	FREE(dind);
	dind = NULL;
	FREE(sind);
	sind = NULL;
	return (inode_blocks[blocks_so_far++]);
    }
/*
   if( must load single block )
   1 get from inode or
   2 get from double indirect
   if( must load double block )
   1 get from inode or
   2 get from triple indirect
   if( must load triple indirect )
   1 get from inode
 */

    /* Load Single Indirect Block? */
    if (s_idx % blocks_per_block == 0)
    {								/* Get block number from inode */
	if (s_idx < blocks_per_block)
	    block_num = inode_blocks[12];

	/* Or get block number from Double Indirect Block */
	else
	{							/* Load Double Indirect Block? */
	    if (d_idx % blocks_per_block == 0)
	    {							/* Get block number from inode */
		if (d_idx < blocks_per_block)
		    block_num = inode_blocks[13];

		/* Or get block number from Triple Indirect Block */
		else
		{						/* Load Triple Indirect Block? */
		    if (t_idx % blocks_per_block == 0)
		    {
			if (t_idx == blocks_per_block)
			{
			    fprintf(STDERR, "You want too many blocks in block_list.\n");
			    exit(-1);				/* You want too many blocks */
			}
			blocks_so_far++;			/* inc for tind block */

			/* Allocate tind block */
			if ((tind = (unsigned long *) MALLOC(BLOCK_SIZE)) == NULL)
			{
			    fprintf(STDERR, "Memory problem in block_list.\n");
			    exit(-1);				/* Memory problem */
			}
			/* Get block from disk */
			if (readdisk((byte *) tind, inode_blocks[14] * (BLOCK_SIZE / DISK_BLOCK_SIZE),
						   (unsigned short) ((inode_blocks[14] * BLOCK_SIZE) % DISK_BLOCK_SIZE), BLOCK_SIZE) != BLOCK_SIZE)
			{
			    fprintf(STDERR, "Disk problem in block_list.\n");
			    exit(-1);				/* Disk problem */
			}
#ifdef UNIX
			{					/* fix byteorder, reverse done in main.c */
			    int i;
			    _u32 *p = tind;
			    for (i = 0; i < blocks_per_block; i++, p++)
				*p = le32_to_cpu(*p);
			}
#endif
			if ((modus == LWRITE) && (u_modus == DEL))	/*necessary to delete indirection blocks */
			{
			    sb.s_free_blocks_count++;		/*adapt free block count */
			    gt[(inode_blocks[14] - 1) / sb.s_blocks_per_group].bg_free_blocks_count++;
			    modifyBlockBitmap(inode_blocks[14], 0);	/*clear bit in block bitmap */
			}
			t_idx = 0;				/* Reset triple block index */
		    }
		    block_num = tind[t_idx++];			/* exit if w/block_num */
		}

		blocks_so_far++;				/* inc for dind block */

		/* See if dind block allocated yet */
		if (dind == NULL)
		{
		    if ((dind = (unsigned long *) MALLOC(BLOCK_SIZE)) == NULL)
		    {
			fprintf(STDERR, "Memory problem in block_list.\n");
			exit(-1);				/* Memory problem */
		    }
		}
		/* Get block from disk */
		if (readdisk((byte *) dind, block_num * (BLOCK_SIZE / DISK_BLOCK_SIZE),
					   (unsigned short) ((block_num * BLOCK_SIZE) % DISK_BLOCK_SIZE), BLOCK_SIZE) != BLOCK_SIZE)
		{
		    fprintf(STDERR, "Disk problem in block_list.\n");
		    exit(-1);					/* Disk problem */
		}
#ifdef UNIX
		{						/* fix byteorder */
		    int i;
		    _u32 *p = dind;
		    for (i = 0; i < blocks_per_block; i++, p++)
			*p = le32_to_cpu(*p);
		}
#endif
		if ((modus == LWRITE) && (u_modus == DEL))
		{
		    sb.s_free_blocks_count++;			/*adapt free block count */
		    gt[(block_num - sb.s_first_data_block) / sb.s_blocks_per_group].bg_free_blocks_count++;	
		    modifyBlockBitmap(block_num, 0);		/*necessary to delete indirection blocks */
		}
		d_idx = 0;					/* Reset single block index */
	    }
	    block_num = dind[d_idx++];				/* exit if with block_num */
	}

	blocks_so_far++;					/* inc for sind block */

	/* See if sind block allocated yet */
	if (sind == NULL)
	{
	    if ((sind = (unsigned long *) MALLOC(BLOCK_SIZE)) == NULL)
	    {
		fprintf(STDERR, "Memory problem in block_list.\n");
		exit(-1);					/* Memory problem */
	    }
	}
	/* Get block from disk */
	if (readdisk((byte *) sind, block_num * (BLOCK_SIZE / DISK_BLOCK_SIZE),
			 	   (unsigned short) ((block_num * BLOCK_SIZE) % DISK_BLOCK_SIZE), BLOCK_SIZE) != BLOCK_SIZE)
	{
	    fprintf(STDERR, "Disk problem in block_list.\n");
	    exit(-1);						/* Disk problem */
	}
#ifdef UNIX
	{						/* fix byteorder */
	  int i;
	  _u32 *p = sind;
	  for (i = 0; i < blocks_per_block; i++, p++)
	    *p = le32_to_cpu(*p);
	}
#endif
	if ((modus == LWRITE) && (u_modus == DEL))
	{
	    sb.s_free_blocks_count++;				/*adapt free block count */
	    gt[(block_num - sb.s_first_data_block) / sb.s_blocks_per_group].bg_free_blocks_count++;
	    modifyBlockBitmap(block_num, 0);			/*necessary to delete indirection blocks */
	}
	s_idx = 0;						/* Reset single block index */
    }
    blocks_so_far++;

    DebugOut(32,"block_list:%u %lu\n", blocks_so_far, sind[s_idx]);

    return (sind[s_idx++]);					/* return the block number */
}


/* read_inode

 * Read size bytes of the data blocks to which the inode i is pointing to.
 * If i equal NULL continue reading from last inode.
 * Returns the number of bytes read, -1 on error.
 */

int read_inode(inode * i, char * buffer, unsigned long size, char ctrl)
{
    static byte *buf = NULL;
    static unsigned long total_bytes, bytes_so_far, index, block_count, j, start_block_num, next_block_num;

    DebugOut(32,"----------Executing read_inode--------------size=%lu---ctrl=%x\n", size, ctrl);

    if (ctrl == KILL_BUF)
    {
	FREE(buf);
	buf = NULL;
	return (0);
    }
    if (i != NULL)						/* Initialize stuff */
    {								/* Allocate the buffer */
	if (buf == NULL)
	{
	    if ((buf = (byte *) MALLOC(NBLOCK * BLOCK_SIZE)) == NULL)
	    {
		fprintf(STDERR, "Memory problem in read_inode. %u -- %u\n",NBLOCK,BLOCK_SIZE);
		return (-1);					/* Memory Problems */
	    }
	}
	total_bytes = i->i_size;
	bytes_so_far = 0;
	index = 0;
	block_count = 1;
	start_block_num = block_list(i);
	next_block_num = block_list(NULL);
    }
    for (j = 0; (j < size) && (bytes_so_far < total_bytes); j++)
    {								/* Load new block? We do some caching here for contiguous blocks */
	if ((index == 0) || (index >= block_count * BLOCK_SIZE))
	{
	    for (block_count = 1; block_count < NBLOCK; block_count++)
	    {
		if (disk_no < 128)
		    break;					/*no caching for floppy disks */
		if (next_block_num != start_block_num + block_count)
		{
		    break;
		}
		next_block_num = block_list(NULL);
	    }

	    DebugOut(2, "--->starting at %ld   blocks in sequence=%ld\n", start_block_num, block_count);

	    if (readdisk(buf, start_block_num * (BLOCK_SIZE / DISK_BLOCK_SIZE),
	    		     (unsigned short) ((start_block_num * BLOCK_SIZE) % DISK_BLOCK_SIZE), block_count * BLOCK_SIZE) != block_count * BLOCK_SIZE)
	    {
		fprintf(STDERR, "Disk problem in read_inode.\n");
		return (-1);					/* Disk Problem */
	    }
	    index = 0;
	    start_block_num = next_block_num;
	    next_block_num = block_list(NULL);
	}
	if (index >= NBLOCK * BLOCK_SIZE)
	{
	    fprintf(STDERR, "Buffer overrun in read_inode\n");
	    return (-1);
	}
	/* Copy bytes */
	buffer[j] = buf[index++];
	bytes_so_far++;
    }

    if (ctrl == RELEASE_BUF)
    {
	FREE(buf);
	buf = NULL;
    }
    return (j);
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Miller 7.12.98 !!!!!!!!!!!!!!!!!!! */
/* write_inode

 * Write size bytes of the data blocks to which the inode i is pointing to
 * to disk.
 * Returns 0 on success, -1 on error.
 */
int write_inode(inode * i, char * buffer, unsigned long size)
{
    byte *buf;
    unsigned long total_bytes, bytes_so_far, index;
    unsigned long j, block_num;


    DebugOut(32,"----------Executing write_inode--------------size=%lu\n", size);


    if (i == NULL)
    {
	fprintf(STDERR, "Can not call write_inode with i==NULL\n");
	return (-1);
    }
    if ((buf = (byte *) MALLOC(BLOCK_SIZE)) == NULL)
    {
	fprintf(STDERR, "Memory problem in write_inode.\n");
	return (-1);						/* Memory Problems */
    }
    total_bytes = i->i_size;
    bytes_so_far = 0;
    index = 0;
    block_num = block_list(i);

/*----------------------Delete file entry in directory inode */
    for (j = 0; (j < size) && (bytes_so_far < total_bytes); j++)
    {
	buf[index++] = buffer[j];
	/* Write block ? */
	if (index == BLOCK_SIZE)
	{
	    if (writedisk(buf, block_num * (BLOCK_SIZE / DISK_BLOCK_SIZE), (unsigned short) ((block_num * BLOCK_SIZE) % DISK_BLOCK_SIZE), BLOCK_SIZE) != BLOCK_SIZE)
	    {
		fprintf(STDERR, "Disk problem in read_inode.\n");
		return (-1);					/* Disk Problem */
	    }
	    block_num = block_list(NULL);
	    index = 0;
	    bytes_so_far++;
	}
    }
    FREE(buf);
    return (j);
}
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Ende Miller 7.12.98 !!!!!!!!!!!!!!! */

//This function computes the location on the disk as 64bit value and then transforms it to a 32bit lba and a 16bit offset
void toLba(unsigned long g, unsigned long h, unsigned long *lba, unsigned short *offset)
{   unsigned long tempgH, tempgL, temphH, temphL, locH, locL;
    unsigned long k,n,m,i;

    tempgH = g;			//gt[(inode_no - 1) / sb.s_inodes_per_group].bg_inode_table * BLOCK_SIZE
    tempgL = g;
    temphH = h;			//((inode_no - 1) % sb.s_inodes_per_group) * 128
    temphL = h;

    for (i=1, n=0; i < BLOCK_SIZE; 	i=i*2, n++);
    for (i=1, m=0; i < sizeof(inode); 	i=i*2, m++);
    for (i=1, k=0; i < DISK_BLOCK_SIZE; i=i*2, k++);

    tempgL=tempgL << n;
    tempgH=tempgH >> (32-n);

    temphL=temphL <<  m;
    temphH=temphH >> (32-m);

    locL=tempgL + temphL;
    locH=tempgH + temphH;
    if (locL < min(tempgL,temphL))	//Overflow
    	locH++;

    *lba = (locL >> k) + (locH << (32-k));
    *offset = locL % DISK_BLOCK_SIZE;
}

