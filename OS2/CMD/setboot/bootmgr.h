/* Boot manager, just enough to get by */

#ifndef __GNUC__
#define P
#else
#define P __attribute__ ((packed))
#endif

/* sector 0 */

/* The boot block is a FAT boot block.
   I think it's a standard FAT boot block for a size-0 filesystem. */

struct boot_block
{
  unsigned char jmp[3];
  unsigned char oem_id[8];		/* "APJ&WN" */
  unsigned short bytes_per_sector P;	/* 512 */
  unsigned char sectors_per_cluster;
  unsigned short n_reserved_sectors P;
  unsigned char n_fats;
  unsigned short n_rootdir_entries P;
  unsigned short n_sectors_s P;		/* 32 */
  unsigned char media_byte;
  unsigned short sectors_per_fat P;
  unsigned short sectors_per_track P;
  unsigned short heads_per_cyl P;
  unsigned long n_hidden_sectors P;	/* 32 */
  unsigned long n_sectors_l P;
  unsigned char drive_number;		/* 80h -- in this case it's true */
  unsigned char mbz;
  unsigned char sig_29h;		/* 29h */
  unsigned char vol_serno[4];
  unsigned char vol_label[11];
  unsigned char sig_fat[8];		/* "FAT     " */
  unsigned char pad[448];
  unsigned short magic;			/* aa55 */
};

/* sector 1 */

/* This looks like transient data.  SETBOOT probably writes only sector 1. */

struct boot_manager_transient
{
  unsigned long unk1, unk2 P;		/* Misc numbers, probably timeouts
					   and so on */
  unsigned short zero1 P;
  unsigned char boot_device P;		/* /IBD:X goes here */
  unsigned char zero2 P;
  unsigned long zero3[14] P;
  unsigned char boot_tag[8] P;		/* /IBA:name goes here */
  unsigned long zero4 P;
  unsigned char current_selection[8] P;	/* OS last booted.  This line is
					   highlighted in the initial menu. */
  unsigned long zero5[106] P;
};

/* sector 3 */

/* Menu info for all primary partitions on all disks.
   This is probably only written by FDISK.
   The first four entries are disk 80h, the next four are 81h, and so on. */

struct boot_manager_menu
{
  struct partition_data {
    unsigned disk_number : 7;		/* 0 = 80h, 1 = 81h, etc */
    unsigned startable : 1;		/* set on boot manager partition */
    unsigned numbers0 : 8;		/* cylinder numbers or something */
    unsigned short numbers1;		/* cylinder numbers or something */
    unsigned char bootable;		/* 0 = ignore, 1 = bootable */
    unsigned char tag[8];		/* OS name, trailing spaces */
    unsigned char zero2[3];
  } partition_data[32];
};

/* disk address format in partition tables */

struct int13
{
  unsigned char indicator;		/* indicator byte --
					   active flag, system id */
  unsigned char headn;			/* head number */
  unsigned short cyln_secn;		/* sector and cylinder numbers,
					   int 13 format */
};

/* The data for a logical partition is out in the logical partition
   itself.  The partition boot sector has an mbr-like partition table
   at the end, and the Bootable flag and OS tag are at 0x18a in the
   unused part of the block. */

struct partition_boot_sector
{
  unsigned char skip1[394];
  unsigned char bootable;		/* 01 = bootable */
  unsigned char tag[8];			/* OS name, trailing spaces */
  unsigned char skip2[43];

  struct {
    struct int13 start P;		/* boot indicator and start address */
    struct int13 end P;			/* system indicator and end address */
    unsigned long sector_number P;	/* start sector number */
    unsigned long n_sectors P;		/* size in sectors */
  } partition[4] P;

  unsigned short magic;			/* aa55 */
};

/*
   Local Variables:
   comment-column: 40
   End:
*/
