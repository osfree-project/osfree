/* LTOOLS
   Programs to read, write, delete and change Linux extended 2 filesystems under DOS

   Module group.c
   This file contains routines for handling the ext2 groups

   Copyright information and copying policy see file README.TXT

   History see file MAIN.C

 */

#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"
#include "proto.h"
#include "ldir.h"

extern int isDebug;
extern int DebugOut(int level, const char *fmt,...);

extern void *MALLOC(size_t size);
extern void FREE(void *block);


/*
 * Group Global Variables
 */
group_desc *gt;
int num_groups;
extern super_block sb;
extern unsigned short BLOCK_SIZE;
#ifdef UNIX
#define STDOUT stdout
#define STDERR stdout
#else
extern FILE *STDOUT, *STDERR;
#endif
extern super_block sb;

/* load_groups

 * Load the group descriptors into memory and set some
 * global variables.
 *
 * returns -1 for failure
 *          1 for success
 */
int load_groups(void)
{
    unsigned long lba, size;
    unsigned short offset;

#ifdef UNIX
    int i;
#endif

    DebugOut(16,"------------------Executing 'load_groups'---------------------\n");

    /* How many groups are there? */
    /* Funky math insures integer divide will round up */
    if (sb.s_blocks_per_group == 0)
	return -1;						/* error checking */

    num_groups = (sb.s_blocks_count - sb.s_first_data_block +
		  sb.s_blocks_per_group - 1) / sb.s_blocks_per_group;

    size = sizeof(group_desc) * num_groups;

    if (BLOCK_SIZE==1024)
    {	lba=(2 * BLOCK_SIZE / DISK_BLOCK_SIZE);
    	offset=0;
    } else
    {	lba=BLOCK_SIZE / DISK_BLOCK_SIZE;
    	offset=0;
    }

    if ((gt = (group_desc *) MALLOC(size)) == NULL)
    {
	fprintf(STDERR, "Memory problem in load_groups.\n");
	return (-1);						/* Memory problem, signal failure */
    }

    if (readdisk((byte *) gt, lba, offset, size) != size)
    {   fprintf(STDERR, "Disk problem in load_groups.\n");
	return (-1);					/* Disk problem, signal failure */
    }


#ifdef UNIX
    /* fix byteorder: */
    for (i = 0; i < num_groups; i++)
    {
	gt[i].bg_block_bitmap = le32_to_cpu(gt[i].bg_block_bitmap);
	gt[i].bg_inode_bitmap = le32_to_cpu(gt[i].bg_inode_bitmap);
	gt[i].bg_inode_table = le32_to_cpu(gt[i].bg_inode_table);
	gt[i].bg_free_blocks_count = le16_to_cpu(gt[i].bg_free_blocks_count);
	gt[i].bg_free_inodes_count = le16_to_cpu(gt[i].bg_free_inodes_count);
	gt[i].bg_used_dirs_count = le16_to_cpu(gt[i].bg_used_dirs_count);
    }
#endif

    return (1);							/* Otherwise everything is OK */
}


void store_groups(void)
{
    unsigned long lba, size;
    unsigned short offset;

#ifdef UNIX
    group_desc *gx;
    long i;
#endif

    DebugOut(16,"------------------Executing 'store_groups'---------------------\n");

    /* How many groups are there? */
    /* Funky math insures integer divide will round up */
    if (sb.s_blocks_per_group == 0)
	exit(-1);						/* error checking */

    num_groups = (sb.s_blocks_count - sb.s_first_data_block +
		  sb.s_blocks_per_group - 1) / sb.s_blocks_per_group;

    size = sizeof(group_desc) * num_groups;
    if (BLOCK_SIZE==1024)
    {	lba=(2 * BLOCK_SIZE / DISK_BLOCK_SIZE);
    	offset=0;
    } else
    {	lba=BLOCK_SIZE / DISK_BLOCK_SIZE;
    	offset=0;
    }
    //printf("store_groups: lba=%lu   offset=%u   size=%lu\n",lba,offset,size);

#ifdef UNIX
    /* fix byteorder: */

  if ((gx = MALLOC (size)) == NULL)
    {
      fprintf (STDERR, "Memory problem in store_groups.\n");
      exit (-1);		/* Memory problem, signal failure */
    }

    for (i = 0; i < num_groups; i++)
    {
	gx[i].bg_block_bitmap = cpu_to_le32(gt[i].bg_block_bitmap);
	gx[i].bg_inode_bitmap = cpu_to_le32(gt[i].bg_inode_bitmap);
	gx[i].bg_inode_table = cpu_to_le32(gt[i].bg_inode_table);
	gx[i].bg_free_blocks_count = cpu_to_le16(gt[i].bg_free_blocks_count);
	gx[i].bg_free_inodes_count = cpu_to_le16(gt[i].bg_free_inodes_count);
	gx[i].bg_used_dirs_count = cpu_to_le16(gt[i].bg_used_dirs_count);
    }

    if (writedisk((byte *) gx, lba, offset , size) != size)
#else

    if (writedisk((byte *) gt, lba, offset, size) != size)
#endif
    {
	fprintf(STDERR, "Disk problem in load_groups.\n");
	exit(-1);						/* Disk problem, signal failure */
    }


#ifdef UNIX
    FREE(gx);
#endif
}


/* print_groups

 * print out group information (useful for debugging)
 */
void print_groups(void)
{
    int i;
    unsigned long first_block, last_block, first_inode, last_inode;

    for (i = 0; i < num_groups; i++)
    {
	first_block = i * sb.s_blocks_per_group + sb.s_first_data_block;		
	last_block = (i + 1) * sb.s_blocks_per_group - (1 - sb.s_first_data_block);
	first_inode = i * sb.s_inodes_per_group + 1;
	last_inode = (i + 1) * sb.s_inodes_per_group;

	printf("----------------------------\n");
	printf("Group Number: %d    Blocks: %ld-%ld   Inodes:%ld-%ld\n", i,
	       first_block, last_block, first_inode, last_inode);
	printf("Blocks Bitmap Block: %lu\n", gt[i].bg_block_bitmap);
	printf("Inodes Bitmap Block: %lu\n", gt[i].bg_inode_bitmap);
	printf("Inodes Table Block: %lu\n", gt[i].bg_inode_table);
	printf("Free Blocks: %u\n", gt[i].bg_free_blocks_count);
	printf("Free Inodes: %u\n", gt[i].bg_free_inodes_count);
	printf("Used Directories: %u\n", gt[i].bg_used_dirs_count);
    }
}
