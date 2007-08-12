/* LTOOLS
   Programs to read, write, delete and change Linux extended 2 filesystems under DOS

   Module super.c
   Routines for reading and manipulating the ext2 superblock

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

/*
 * Global variables calculated from the super block info
 */
super_block sb;
unsigned short NBLOCK=32;
unsigned short BLOCK_SIZE;
unsigned short FRAG_SIZE;
#ifdef UNIX
#define STDOUT stdout
#define STDERR stderr
#else
extern FILE *STDOUT, *STDERR;
#endif

/* load_super

 * returns -4 wrong inode size
 *	   -3 wrong block size
 *	   -2 for drive unreadable
 *         -1 for bad magic
 *          0 for OK
 */
int load_super(void)
{
    DebugOut(4,"-------------Executing 'load_super'--------------\n");

    if (readdisk((byte *) &sb, 1024 / DISK_BLOCK_SIZE, 1024 % DISK_BLOCK_SIZE, sizeof(sb)) != sizeof(sb))
    {
	//fprintf(STDERR, "ERROR: Drive unreadable.\n");
	return (-2);						/* read failed */
    }
#ifdef UNIX
    sb.s_inodes_count = le32_to_cpu(sb.s_inodes_count);
    sb.s_blocks_count = le32_to_cpu(sb.s_blocks_count);
    sb.s_r_blocks_count = le32_to_cpu(sb.s_r_blocks_count);
    sb.s_free_blocks_count = le32_to_cpu(sb.s_free_blocks_count);
    sb.s_free_inodes_count = le32_to_cpu(sb.s_free_inodes_count);
    sb.s_first_data_block = le32_to_cpu(sb.s_first_data_block);
    sb.s_log_block_size = le32_to_cpu(sb.s_log_block_size);
    sb.s_log_frag_size = le32_to_cpu(sb.s_log_frag_size);
    sb.s_blocks_per_group = le32_to_cpu(sb.s_blocks_per_group);
    sb.s_frags_per_group = le32_to_cpu(sb.s_frags_per_group);
    sb.s_inodes_per_group = le32_to_cpu(sb.s_inodes_per_group);
    sb.s_mtime = le32_to_cpu(sb.s_mtime);
    sb.s_wtime = le32_to_cpu(sb.s_wtime);

    sb.s_mnt_count = le16_to_cpu(sb.s_mnt_count);
    sb.s_max_mnt_count = le16_to_cpu(sb.s_max_mnt_count);
    sb.s_magic = le16_to_cpu(sb.s_magic);
    sb.s_state = le16_to_cpu(sb.s_state);
    sb.s_errors = le16_to_cpu(sb.s_errors);
    sb.s_minor_rev_level = le16_to_cpu(sb.s_minor_rev_level);

    sb.s_lastcheck = le32_to_cpu(sb.s_lastcheck);
    sb.s_checkinterval = le32_to_cpu(sb.s_checkinterval);
    sb.s_creator_os = le32_to_cpu(sb.s_creator_os);
    sb.s_rev_level = le32_to_cpu(sb.s_rev_level);
    sb.s_def_resuid = le32_to_cpu(sb.s_def_resuid);
    sb.s_def_resgid = le32_to_cpu(sb.s_def_resgid);

    sb.s_first_ino=le32_to_cpu(sb.s_first_ino);
    sb.s_inode_size=le16_to_cpu(sb.s_inode_size);
    sb.s_block_group_nr=le16_to_cpu(sb.s_block_group_nr);
    sb.s_feature_compat=le32_to_cpu(sb.s_feature_compat);
    sb.s_feature_incompat=le32_to_cpu(sb.s_feature_incompat);
    sb.s_feature_ro_compat=le32_to_cpu(sb.s_feature_ro_compat);
#endif

    /* Now set a few of our (shhhh!) Global Variables */
    BLOCK_SIZE = EXT2_MIN_BLOCK << sb.s_log_block_size;
    FRAG_SIZE = EXT2_MIN_FRAG << sb.s_log_frag_size;

    NBLOCK = BUFSIZE / BLOCK_SIZE;
    if ((NBLOCK <=0) || !((BLOCK_SIZE==1024) || (BLOCK_SIZE==2048) || (BLOCK_SIZE==4096)))
    {	//fprintf(STDERR, "Bad block size	%u - BUF_SIZE=%u  - NBLOCK=%u\n",BLOCK_SIZE,NBLOCK,BUFSIZE);
        return (-3);
    }

    if (isDebug & 4) print_super();


    if (sb.s_magic != EXT2_SUPER_MAGIC)
    {
	//fprintf(STDERR, "Bad magic number in superblock. Not Linux.\n");
	return (-1);
    }

    if (sb.s_rev_level!=EXT2_GOOD_OLD_REV)
    {	if (sb.s_inode_size!=EXT2_GOOD_OLD_INODE_SIZE)
    	{   fprintf(STDERR, "ERROR: Sorry, your disk's inode size is %d - LTOOLS can only handle inode size %d\n",
    						sb.s_inode_size,EXT2_GOOD_OLD_INODE_SIZE);
    	    return (-4);
    	}
    }

    return (0);							/* everything OK */
}


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Miller 20.12.98 !!!!!!!!!!!!!!!!!!!!!!!!! */
int write_super(void)
{
#ifdef UNIX
    static super_block tempsb;
#endif

    DebugOut(4,"-------------Executing 'write_super'--------------\n");

#ifdef UNIX
    tempsb = sb;

    tempsb.s_inodes_count = cpu_to_le32(sb.s_inodes_count);
    tempsb.s_blocks_count = cpu_to_le32(sb.s_blocks_count);
    tempsb.s_r_blocks_count = cpu_to_le32(sb.s_r_blocks_count);
    tempsb.s_free_blocks_count = cpu_to_le32(sb.s_free_blocks_count);
    tempsb.s_free_inodes_count = cpu_to_le32(sb.s_free_inodes_count);
    tempsb.s_first_data_block = cpu_to_le32(sb.s_first_data_block);
    tempsb.s_log_block_size = cpu_to_le32(sb.s_log_block_size);
    tempsb.s_log_frag_size = cpu_to_le32(sb.s_log_frag_size);
    tempsb.s_blocks_per_group = cpu_to_le32(sb.s_blocks_per_group);
    tempsb.s_frags_per_group = cpu_to_le32(sb.s_frags_per_group);
    tempsb.s_inodes_per_group = cpu_to_le32(sb.s_inodes_per_group);
    tempsb.s_mtime = cpu_to_le32(sb.s_mtime);
    tempsb.s_wtime = cpu_to_le32(sb.s_wtime);

    tempsb.s_mnt_count = cpu_to_le16(sb.s_mnt_count);
    tempsb.s_max_mnt_count = cpu_to_le16(sb.s_max_mnt_count);
    tempsb.s_magic = cpu_to_le16(sb.s_magic);
    tempsb.s_state = cpu_to_le16(sb.s_state);
    tempsb.s_errors = cpu_to_le16(sb.s_errors);
    tempsb.s_minor_rev_level = cpu_to_le16(sb.s_minor_rev_level);

    tempsb.s_lastcheck = cpu_to_le32(sb.s_lastcheck);
    tempsb.s_checkinterval = cpu_to_le32(sb.s_checkinterval);
    tempsb.s_creator_os = cpu_to_le32(sb.s_creator_os);
    tempsb.s_rev_level = cpu_to_le32(sb.s_rev_level);
    tempsb.s_def_resuid = cpu_to_le32(sb.s_def_resuid);
    tempsb.s_def_resgid = cpu_to_le32(sb.s_def_resgid);

    if (writedisk((byte *) & tempsb, 1024 / DISK_BLOCK_SIZE, 1024 % DISK_BLOCK_SIZE, sizeof(sb)) != sizeof(sb))
#else
    if (writedisk((byte *) & sb, 1024 / DISK_BLOCK_SIZE, 1024 % DISK_BLOCK_SIZE, sizeof(sb)) != sizeof(sb))
#endif
    {
	fprintf(STDERR, "Drive unreadable in write_super.\n");
	return (-1);						/* read failed */
    }
    return (1);							/* everything OK */
}
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Ende Miller 20.12.98 !!!!!!!!!!!!!!!!!!!! */


/* print_super

 * print out a super block (good for debugging)
 */
void print_super(void)
{
    printf("Inode Count: %lu\n", sb.s_inodes_count);
    printf("Block Count: %lu\n", sb.s_blocks_count);
    printf("Reserved Block Count: %lu\n", sb.s_r_blocks_count);
    printf("Free Blocks: %lu\n", sb.s_free_blocks_count);
    printf("Free Inodes: %lu\n", sb.s_free_inodes_count);
    printf("First Data Block: %lu\n", sb.s_first_data_block);
    printf("Log Block Size: %lu\n", sb.s_log_block_size);
    printf("Abs.Block Size: %u\n", BLOCK_SIZE);
    printf("Log Frag Size: %ld\n", sb.s_log_frag_size);
    printf("Blocks per Group: %lu\n", sb.s_blocks_per_group);
    printf("Fragments per Group: %lu\n", sb.s_frags_per_group);
    printf("Inodes per Group: %lu\n", sb.s_inodes_per_group);
    printf("Mount Time: %s", ctime((time_t *) & (sb.s_mtime)));
    printf("Write Time: %s", ctime((time_t *) & (sb.s_wtime)));
    printf("Mount Count: %u\n", sb.s_mnt_count);
    printf("Max Mount Count: %d\n", sb.s_max_mnt_count);
    printf("Magic Number: %X  (%s)\n", sb.s_magic,
	   sb.s_magic == EXT2_SUPER_MAGIC ? "OK" : "BAD");
    printf("File System State: %X\n", sb.s_state);
    printf("Error Behaviour: %X\n", sb.s_errors);
    printf("Minor Revision Level: %u\n", sb.s_minor_rev_level);
    printf("Last Check: %s", ctime((time_t *) & (sb.s_lastcheck)));
    printf("Check Interval: %lu\n", sb.s_checkinterval);
    printf("Creator OS: %lu\n", sb.s_creator_os);
    printf("Revision Level: %lu\n", sb.s_rev_level);
    printf("Reserved Block Default UID: %u\n", sb.s_def_resuid);
    printf("Reserved Block Default GID: %u\n", sb.s_def_resgid);
    printf("First Inode:              %lu\n",sb.s_first_ino);
    printf("Inode Size:               %u\n",sb.s_inode_size);
    printf("Block Group No:	      %u\n",sb.s_block_group_nr);
    printf("Compatible Feature Set:   %lX\n",sb.s_feature_compat);
    printf("Incompatible Feature Set: %lX\n",sb.s_feature_incompat);
    printf("Read Only Feature Set:    %lX\n",sb.s_feature_ro_compat);
    printf("An' a bunch of padding we ain't gonna show ya!\n");
}

