/*
 *
 *
 */

#include <shared.h>
#include <filesys.h>
#include <lip.h>
#include "fsys.h"
#include "fsd.h"

void (*disk_read_func) (int, int, int);
void (*disk_read_hook) (int, int, int);

int __cdecl (*fsd_init)(lip1_t *l1);

int mem_lower;
int mem_upper = 16384;

/* filesystem common variables */
int filepos;
int filemax;

/* disk buffer parameters */
int buf_drive = -1;
int buf_track;
struct geometry buf_geom;

extern grub_error_t errnum;
int print_possibilities;

extern unsigned long saved_drive;
extern unsigned long saved_partition;
extern unsigned long cdrom_drive;
unsigned long saved_slice;

unsigned long current_drive;
unsigned long current_partition;
int           current_slice;
unsigned long part_start;
unsigned long part_length;
int           fsmax;

int  saved_fsys_type;
int  saved_current_slice;
int  saved_fsmax;
int  saved_buf_drive;
int  saved_buf_track;
struct geometry saved_buf_geom;
unsigned long   saved_current_partition;
unsigned long   saved_current_drive;
unsigned long   saved_cdrom;
unsigned long   saved_part_start;
unsigned long   saved_part_length;
unsigned long   saved_filemax;
unsigned long   saved_filepos;

int debug = 0;
struct geometry buf_geom;

/* filesystem type */
int fsys_type = -1;
#ifndef NO_BLOCK_FILES
static int block_file = 0;
#endif /* NO_BLOCK_FILES */

static inline unsigned long
log2(unsigned long word);

static int do_completion;

int
sane_partition (void);

#pragma aux sane_partition "*"

/* FIXME: BSD evil hack */
#include "freebsd.h"
int bsd_evil_hack;

struct fsys_entry fsys_table[NUM_FSYS + 1] =
{
  /* TFTP should come first because others don't handle net device.  */
# ifdef FSYS_TFTP
  {"tftp", tftp_mount, tftp_read, tftp_dir, tftp_close, 0},
# endif
# ifdef STAGE0
  {"stage0", stage0_mount, stage0_read, stage0_dir, 0, 0},
# endif
# ifdef FSYS_FAT
  {"fat", fat_mount, fat_read, fat_dir, 0, 0},
# endif
# ifdef FSYS_EXT2FS
  {"ext2fs", ext2fs_mount, ext2fs_read, ext2fs_dir, 0, 0},
# endif
# ifdef FSYS_MINIX
  {"minix", minix_mount, minix_read, minix_dir, 0, 0},
# endif
# ifdef FSYS_REISERFS
  {"reiserfs", reiserfs_mount, reiserfs_read, reiserfs_dir, 0, reiserfs_embed},
# endif
# ifdef FSYS_VSTAFS
  {"vstafs", vstafs_mount, vstafs_read, vstafs_dir, 0, 0},
# endif
# ifdef FSYS_JFS
  {"jfs", jfs_mount, jfs_read, jfs_dir, 0, jfs_embed},
# endif
# ifdef FSYS_XFS
  {"xfs", xfs_mount, xfs_read, xfs_dir, 0, 0},
# endif
# ifdef FSYS_UFS2
  {"ufs2", ufs2_mount, ufs2_read, ufs2_dir, 0, ufs2_embed},
# endif
# ifdef FSYS_ISO9660
  {"iso9660", iso9660_mount, iso9660_read, iso9660_dir, 0, 0},
# endif
  /* XX FFS should come last as it's superblock is commonly crossing tracks
     on floppies from track 1 to 2, while others only use 1.  */
# ifdef FSYS_FFS
  {"ffs", ffs_mount, ffs_read, ffs_dir, 0, ffs_embed},
# endif
  {0, 0, 0, 0, 0, 0}
};

/* These are defined in asm.S, and never be used elsewhere, so declare the
   prototypes here.  */
//extern int biosdisk_int13_extensions (int ax, int drive, void *dap);
//extern int biosdisk_standard (int ah, int drive,
//                              int coff, int hoff, int soff,
//                              int nsec, int segment);
//extern int check_int13_extensions (int drive);
//extern int get_diskinfo_standard (int drive,
//                                  unsigned long *cylinders,
//                                  unsigned long *heads,
//                                  unsigned long *sectors);

int
rawread (int drive, int sector, int byte_offset, int byte_len, char *buf)
{
  int slen, sectors_per_vtrack;
  int sector_size_bits = log2 (buf_geom.sector_size);

  if (byte_len <= 0)
    return 1;

  while (byte_len > 0 && !errnum)
    {
      int soff, num_sect, track, size = byte_len;
      char *bufaddr;

      /*
       *  Check track buffer.  If it isn't valid or it is from the
       *  wrong disk, then reset the disk geometry.
       */
      if (buf_drive != drive)
        {
          if (get_diskinfo (drive, &buf_geom))
            {
              errnum = ERR_NO_DISK;
              return 0;
            }
          buf_drive = drive;
          buf_track = -1;
          sector_size_bits = log2 (buf_geom.sector_size);
        }

      /* Make sure that SECTOR is valid.  */
      if (sector < 0 || sector >= buf_geom.total_sectors)
        {
          errnum = ERR_GEOM;
          return 0;
        }

      slen = ((byte_offset + byte_len + buf_geom.sector_size - 1)
              >> sector_size_bits);

      /* Eliminate a buffer overflow.  */
      if ((buf_geom.sectors << sector_size_bits) > BUFFERLEN)
        sectors_per_vtrack = (BUFFERLEN >> sector_size_bits);
      else
        sectors_per_vtrack = buf_geom.sectors;

      /* Get the first sector of track.  */
      soff = sector % sectors_per_vtrack;
      track = sector - soff;
      num_sect = sectors_per_vtrack - soff;
      bufaddr = ((char *) BUFFERADDR
                 + (soff << sector_size_bits) + byte_offset);

      if (track != buf_track)
        {
          int bios_err, read_start = track, read_len = sectors_per_vtrack;

          /*
           *  If there's more than one read in this entire loop, then
           *  only make the earlier reads for the portion needed.  This
           *  saves filling the buffer with data that won't be used!
           */
          if (slen > num_sect)
            {
              read_start = sector;
              read_len = num_sect;
              bufaddr = (char *) BUFFERADDR + byte_offset;
            }

          bios_err = biosdisk (BIOSDISK_READ, drive, &buf_geom,
                               read_start, read_len, BUFFERADDR >> 4);
          if (bios_err)
            {
              buf_track = -1;

              if (bios_err == BIOSDISK_ERROR_GEOMETRY)
                errnum = ERR_GEOM;
              else
                {
                  /*
                   *  If there was an error, try to load only the
                   *  required sector(s) rather than failing completely.
                   */
                  if (slen > num_sect
                      || biosdisk (BIOSDISK_READ, drive, &buf_geom,
                                   sector, slen, BUFFERADDR >> 4))
                    errnum = ERR_READ;

                  bufaddr = (char *) BUFFERADDR + byte_offset;
                }
            }
          else
            buf_track = track;

          if ((buf_track == 0 || sector == 0)
              && (PC_SLICE_TYPE (BUFFERADDR, 0) == PC_SLICE_TYPE_EZD
                  || PC_SLICE_TYPE (BUFFERADDR, 1) == PC_SLICE_TYPE_EZD
                  || PC_SLICE_TYPE (BUFFERADDR, 2) == PC_SLICE_TYPE_EZD
                  || PC_SLICE_TYPE (BUFFERADDR, 3) == PC_SLICE_TYPE_EZD))
            {
              /* This is a EZD disk map sector 0 to sector 1 */
              if (buf_track == 0 || slen >= 2)
                {
                  /* We already read the sector 1, copy it to sector 0 */
                  memmove ((char *) BUFFERADDR,
                           (char *) BUFFERADDR + buf_geom.sector_size,
                           buf_geom.sector_size);
                }
              else
                {
                  if (biosdisk (BIOSDISK_READ, drive, &buf_geom,
                                1, 1, BUFFERADDR >> 4))
                    errnum = ERR_READ;
                }
            }
        }

      if (size > ((num_sect << sector_size_bits) - byte_offset))
        size = (num_sect << sector_size_bits) - byte_offset;
#ifndef STAGE1_5
      /*
       *  Instrumentation to tell which sectors were read and used.
       */
      if (disk_read_func)
        {
          int sector_num = sector;
          int length = buf_geom.sector_size - byte_offset;
          if (length > size)
            length = size;
          (*disk_read_func) (sector_num++, byte_offset, length);
          length = size - length;
          if (length > 0)
            {
              while (length > buf_geom.sector_size)
                {
                  (*disk_read_func) (sector_num++, 0, buf_geom.sector_size);
                  length -= buf_geom.sector_size;
                }
              (*disk_read_func) (sector_num, 0, length);
            }
        }
#endif
      grub_memmove (buf, bufaddr, size);

      buf += size;
      byte_len -= size;
      sector += num_sect;
      byte_offset = 0;
    }

  return (!errnum);
}

static inline unsigned long
log2 (unsigned long word)
{
  //asm volatile ("bsfl %1,%0"
  //              : "=r" (word)
  //              : "r" (word));
  unsigned long l;

  l = word;

  __asm {
    mov  eax, l
    bsf  eax, eax
    mov  l,   eax
  }

  return l;
}


#ifndef STAGE1_5

int
rawwrite (int drive, int sector, char *buf)
{
  if (sector == 0)
    {
      if (biosdisk (BIOSDISK_READ, drive, &buf_geom, 0, 1, SCRATCHADDR >> 4))
        {
          errnum = ERR_WRITE;
          return 0;
        }

      if (PC_SLICE_TYPE (SCRATCHADDR, 0) == PC_SLICE_TYPE_EZD
          || PC_SLICE_TYPE (SCRATCHADDR, 1) == PC_SLICE_TYPE_EZD
          || PC_SLICE_TYPE (SCRATCHADDR, 2) == PC_SLICE_TYPE_EZD
          || PC_SLICE_TYPE (SCRATCHADDR, 3) == PC_SLICE_TYPE_EZD)
        sector = 1;
    }

  memmove ((char *) SCRATCHADDR, buf, SECTOR_SIZE);
  if (biosdisk (BIOSDISK_WRITE, drive, &buf_geom,
                sector, 1, SCRATCHADDR >> 4))
    {
      errnum = ERR_WRITE;
      return 0;
    }

  if (sector - sector % buf_geom.sectors == buf_track)
    /* Clear the cache.  */
    buf_track = -1;

  return 1;
}

int
devwrite (int sector, int sector_count, char *buf)
{
#if defined(GRUB_UTIL) && defined(__linux__)
  if (current_partition != 0xFFFFFF
      && is_disk_device (device_map, current_drive))
    {
      /* If the grub shell is running under Linux and the user wants to
         embed a Stage 1.5 into a partition instead of a MBR, use system
         calls directly instead of biosdisk, because of the bug in
         Linux. *sigh*  */
      return write_to_partition (device_map, current_drive, current_partition,
                                 sector, sector_count, buf);
    }
  else
#endif /* GRUB_UTIL && __linux__ */
    {
      int i;

      for (i = 0; i < sector_count; i++)
        {
          if (! rawwrite (current_drive, part_start + sector + i,
                          buf + (i << SECTOR_BITS)))
              return 0;

        }
      return 1;
    }
}


static void
check_and_print_mount (int flags)
{
  attempt_mount ();
  if (errnum == ERR_FSYS_MOUNT)
    errnum = ERR_NONE;
  //if (!errnum)
  //  print_fsys_type ();
  //if (!flags)
  //print_error ();
}

void swap_fsys_bufs(void *buf1, void *buf2)
{
  unsigned long *b1 = (unsigned long *)buf1;
  unsigned long *b2 = (unsigned long *)buf2;
  unsigned long b;
  int n;
 
  // number of swapped dwords
  n = (EXT_LEN + 3) >> 2;

  while (n)
  {
    b   = *b1;
    *b1 = *b2;
    *b2 = b;

    b1++; b2++; n--;
  }
}

void set_boot_fsys(void)
{
  fsys_type = -1; // boot filesystem
  /* move boot drive uFSD to working buffer */
  //swap_fsys_bufs((void *)(EXT3HIBUF_BASE), (void *)(UFSD_BASE));
  grub_memmove((void *)(EXT3HIBUF_BASE), (void *)(UFSD_BASE), EXT_LEN);
  /* call uFSD init (set linkage) */
  fsd_init = (void *)(EXT3HIBUF_BASE); // uFSD base address
  fsd_init(l1);
  //printmsg("boot fs set\r\n");
}

#pragma aux set_fsys "*"

int set_fsys(char *fsname)
{
  char buf[EXT_LEN]; 
  char sbuf[0x100];
  char *s;
  int  rc;  
  int  fst;
  
  //saved_fsys_type     = fsys_type;
  //saved_current_drive = current_drive;
  //saved_current_partition = current_partition;
  //saved_cdrom = cdrom_drive;
  //saved_current_slice = current_slice;
  //saved_fsmax = fsmax;
  //saved_part_start  = part_start;
  //saved_part_length = part_length;
  //saved_filemax   = filemax;
  //saved_filepos   = filepos;
  //saved_buf_drive = buf_drive;
  //saved_buf_track = buf_track;
  //grub_memmove(&saved_buf_geom, &buf_geom, sizeof(struct geometry));

  fst = fsys_type;
  set_boot_fsys();
  rc = freeldr_open(fsname);

  if (rc)
    rc = freeldr_read(buf, -1);
  else
    panic("can't open filesystem: ", fsname);

  //printmsg("uFSD file read, size: %d\r\n", rc);
  //printd(rc);
  //printmsg("\r\n");

  grub_strcpy(sbuf, fsname);
  s = grub_strstr(sbuf, ".fsd") + 1;
  // Change ".fsd" extension to ".rel"
  grub_strcpy(s, "rel");
  //swap_fsys_bufs((void *)EXT3HIBUF_BASE, (void *)UFSD_BASE);
  
  // fixup the loaded filesystem
  reloc((char *)buf, sbuf, relshift);
  printmsg("fs relocated\r\n");
  //swap_fsys_bufs((void *)(EXT3HIBUF_BASE), (void *)UFSD_BASE);
  //swap_fsys_bufs((void *)(EXT3HIBUF_BASE), buf);
  grub_memmove((void *)(EXT3HIBUF_BASE), (void *)(buf), EXT_LEN);
  printmsg("fs moved\r\n");
  //fsys_type = saved_fsys_type;
  //current_drive = saved_current_drive;
  //current_partition = saved_current_partition;
  //cdrom_drive = saved_cdrom;
  //current_slice = saved_current_slice;
  //fsmax = saved_fsmax;
  //part_start = saved_part_start;
  //part_length = saved_part_length;
  //filemax = saved_filemax;
  //filepos = saved_filepos;
  //buf_drive = saved_buf_drive;
  //buf_track = saved_buf_track;
  //grub_memmove(&buf_geom, &saved_buf_geom, sizeof(struct geometry));
  fsys_type = fst;
  
  fsd_init = (void *)(EXT3HIBUF_BASE);
  fsd_init(l1);

  printmsg("fs initted\r\n");
  //printd(fsys_type);
  //printmsg("\r\n");

  //printmsg("trying to mount a filesystem\r\n");
  //if (open_partition() && stage0_mount())
  //{
  //  printmsg("filesystem is mounted\r\n");
  //  return 1;
  //}
  

  //printmsg("filesystem is not mounted!\r\n");
  return 0;
}

#pragma aux fsys_by_num "*"

void fsys_by_num(int n, char *buf)
{
  char *fsys_name;
  int m, t, k;
  // buf -> path to filesystem driver
  m = grub_strlen(preldr_path);
  grub_memmove((void *)buf, preldr_path, m);
  t = grub_strlen(fsd_dir);
  grub_memmove((void *)(buf + m), fsd_dir, t);
  fsys_name = fsys_list[n];
  k = grub_strlen(fsys_name);
  grub_memmove((void *)(buf + m + t), fsys_name, k);
  grub_memmove((void *)(buf + m + t + k), ".fsd\0", 5);
}

#endif /* ! STAGE1_5 */

static int
attempt_mount (void)
{

#ifndef STAGE1_5
  char buf[0x100];
  char *s;
  char *fsys_name;
  int  m, k, t, rc;
/*
  saved_fsys_type     = fsys_type;
  saved_current_drive = current_drive;
  saved_cdrom = cdrom_drive;
  saved_current_slice = current_slice;
  saved_current_partition = current_partition;
  saved_fsmax = fsmax;
  saved_part_start  = part_start;
  saved_part_length = part_length;
  saved_filemax   = filemax;
  saved_filepos   = filepos;
  saved_buf_drive = buf_drive;
  saved_buf_track = buf_track;
  grub_memmove(&saved_buf_geom, &buf_geom, sizeof(struct geometry));
 */
/*
  __asm {
    push  fsys_type
    push  current_drive
    push  cdrom_drive
    push  current_slice
    push  current_partition
    push  fsmax
    push  part_start
    push  part_length
    push  filemax
    push  filepos
    push  buf_drive
    push  buf_track
  }
 */
 
  //set_boot_fsys();
  //if (!stage0_mount())
  //{
  //  for (fsys_type = 0; fsys_type < num_fsys; fsys_type++) {
  //    fsys_by_num(fsys_type, buf);
  //
  //    if (set_fsys(buf))
  //      break;
  //  }
  //}
  
  return stage0_mount();
  
/*
  if (!stage0_mount()) 
  {
    errnum = ERR_FSYS_MOUNT;
    fsys_type = NUM_FSYS;
  }
 */
//  if (fsys_type == num_fsys && errnum == ERR_NONE)
//    errnum = ERR_FSYS_MOUNT;
  //if (fsys_type == NUM_FSYS && errnum == ERR_NONE)
  //  errnum = ERR_FSYS_MOUNT;
/*
  __asm {
    pop   buf_track
    pop   buf_drive
    pop   filepos
    pop   filemax
    pop   part_length
    pop   part_start
    pop   fsmax
    pop   current_partition
    pop   current_slice
    pop   cdrom_drive
    pop   current_drive
    pop   fsys_type
  }
 */
/*
  fsys_type = saved_fsys_type;
  current_drive = saved_current_drive;
  cdrom_drive = saved_cdrom;
  current_slice = saved_current_slice;
  current_partition = saved_current_partition;
  fsmax = saved_fsmax;
  part_start = saved_part_start;
  part_length = saved_part_length;
  filemax = saved_filemax;
  filepos = saved_filepos;
  buf_drive = saved_buf_drive;
  buf_track = saved_buf_track;
  grub_memmove(&buf_geom, &saved_buf_geom, sizeof(struct geometry));
 */
#else
  //fsys_type = 0;
  //if ((*(fsys_table[fsys_type].mount_func)) () != 1)
  //if (!stage0_mount())
  //  {
  //    fsys_type = NUM_FSYS;
  //    errnum = ERR_FSYS_MOUNT;
  //  }
  return stage0_mount();
#endif
}


#ifndef STAGE1_5

/* Forward declarations.  */
int next_bsd_partition (unsigned long drive, unsigned long dest,
                        unsigned long *partition, int *type,
                        unsigned long *start, unsigned long *len,
                        unsigned long *offset, int *entry,
                        unsigned long *ext_offset, char *buf);

int next_pc_slice (unsigned long drive, unsigned long dest,
                   unsigned long *partition, int *type,
                   unsigned long *start, unsigned long *len,
                   unsigned long *offset, int *entry,
                   unsigned long *ext_offset, char *buf);


/* Get next BSD partition in current PC slice.  */
int next_bsd_partition (unsigned long drive, unsigned long dest,
                        unsigned long *partition, int *type,
                        unsigned long *start, unsigned long *len,
                        unsigned long *offset, int *entry,
                        unsigned long *ext_offset, char *buf)
{
      int i;
      int bsd_part_no = (*partition & 0xFF00) >> 8;

      /* If this is the first time...  */
      if (bsd_part_no == 0xFF)
        {
          /* Check if the BSD label is within current PC slice.  */
          if (*len < BSD_LABEL_SECTOR + 1)
            {
              errnum = ERR_BAD_PART_TABLE;
              return 0;
            }

          /* Read the BSD label.  */
          if (! rawread (drive, *start + BSD_LABEL_SECTOR,
                         0, SECTOR_SIZE, buf))
            return 0;

          /* Check if it is valid.  */
          if (! BSD_LABEL_CHECK_MAG (buf))
            {
              errnum = ERR_BAD_PART_TABLE;
              return 0;
            }

          bsd_part_no = -1;
        }

      /* Search next valid BSD partition.  */
      for (i = bsd_part_no + 1; i < BSD_LABEL_NPARTS (buf); i++)
        {
          if (BSD_PART_TYPE (buf, i))
            {
              /* Note that *TYPE and *PARTITION were set
                 for current PC slice.  */
              *type = (BSD_PART_TYPE (buf, i) << 8) | (*type & 0xFF);
              *start = BSD_PART_START (buf, i);
              *len = BSD_PART_LENGTH (buf, i);
              *partition = (*partition & 0xFF00FF) | (i << 8);

#ifndef STAGE1_5
              /* XXX */
              if ((drive & 0x80) && BSD_LABEL_DTYPE (buf) == DTYPE_SCSI)
                bsd_evil_hack = 4;
#endif /* ! STAGE1_5 */

              return 1;
            }
        }

      errnum = ERR_NO_PART;
      return 0;
}


/* Get next PC slice. Be careful of that this function may return
   an empty PC slice (i.e. a partition whose type is zero) as well.  */
int next_pc_slice (unsigned long drive, unsigned long dest,
                     unsigned long *partition, int *type,
                     unsigned long *start, unsigned long *len,
                     unsigned long *offset, int *entry,
                     unsigned long *ext_offset, char *buf)
{
      int pc_slice_no = (*partition & 0xFF0000) >> 16;

      /* If this is the first time...  */
      if (pc_slice_no == 0xFF)
        {
          *offset = 0;
          *ext_offset = 0;
          *entry = -1;
          pc_slice_no = -1;
        }

      /* Read the MBR or the boot sector of the extended partition.  */
      if (! rawread (drive, *offset, 0, SECTOR_SIZE, buf))
        return 0;

      /* Check if it is valid.  */
      if (! PC_MBR_CHECK_SIG (buf))
        {
          errnum = ERR_BAD_PART_TABLE;
          return 0;
        }

      /* Increase the entry number.  */
      (*entry)++;

      /* If this is out of current partition table...  */
      if (*entry == PC_SLICE_MAX)
        {
          int i;

          /* Search the first extended partition in current table.  */
          for (i = 0; i < PC_SLICE_MAX; i++)
            {
              if (IS_PC_SLICE_TYPE_EXTENDED (PC_SLICE_TYPE (buf, i)))
                {
                  /* Found. Set the new offset and the entry number,
                     and restart this function.  */
                  *offset = *ext_offset + PC_SLICE_START (buf, i);
                  if (! *ext_offset)
                    *ext_offset = *offset;
                  *entry = -1;
                  return next_pc_slice (drive, dest,
                                        partition, type,
                                        start, len,
                                        offset, entry,
                                        ext_offset, buf);
                }
            }

          errnum = ERR_NO_PART;
          return 0;
        }

      *type = PC_SLICE_TYPE (buf, *entry);
      *start = *offset + PC_SLICE_START (buf, *entry);
      *len = PC_SLICE_LENGTH (buf, *entry);

      /* The calculation of a PC slice number is complicated, because of
         the rather odd definition of extended partitions. Even worse,
         there is no guarantee that this is consistent with every
         operating systems. Uggh.  */
      if (pc_slice_no < PC_SLICE_MAX
          || (! IS_PC_SLICE_TYPE_EXTENDED (*type)
              && *type != PC_SLICE_TYPE_NONE))
        pc_slice_no++;

      *partition = (pc_slice_no << 16) | 0xFFFF;
      return 1;
}


/* Get the information on next partition on the drive DRIVE.
   The caller must not modify the contents of the arguments when
   iterating this function. The partition representation in GRUB will
   be stored in *PARTITION. Likewise, the partition type in *TYPE, the
   start sector in *START, the length in *LEN, the offset of the
   partition table in *OFFSET, the entry number in the table in *ENTRY,
   the offset of the extended partition in *EXT_OFFSET.
   BUF is used to store a MBR, the boot sector of a partition, or
   a BSD label sector, and it must be at least 512 bytes length.
   When calling this function first, *PARTITION must be initialized to
   0xFFFFFF. The return value is zero if fails, otherwise non-zero.  */
int
next_partition (unsigned long drive, unsigned long dest,
                unsigned long *partition, int *type,
                unsigned long *start, unsigned long *len,
                unsigned long *offset, int *entry,
                unsigned long *ext_offset, char *buf)
{
  /* Start the body of this function.  */

#ifndef STAGE1_5
  if (current_drive == NETWORK_DRIVE)
    return 0;
#endif

  /* If previous partition is a BSD partition or a PC slice which
     contains BSD partitions...  */
  if ((*partition != 0xFFFFFF && IS_PC_SLICE_TYPE_BSD (*type & 0xff))
      || ! (drive & 0x80))
    {
      if (*type == PC_SLICE_TYPE_NONE)
        *type = PC_SLICE_TYPE_FREEBSD;

      /* Get next BSD partition, if any.  */
      if (next_bsd_partition (drive, dest,
                              partition, type,
                              start, len,
                              offset, entry,
                              ext_offset, buf))
        return 1;

      /* If the destination partition is a BSD partition and current
         BSD partition has any error, abort the operation.  */
      if ((dest & 0xFF00) != 0xFF00
          && ((dest & 0xFF0000) == 0xFF0000
              || (dest & 0xFF0000) == (*partition & 0xFF0000)))
        return 0;

      /* Ignore the error.  */
      errnum = ERR_NONE;
    }

  return next_pc_slice (drive, dest,
                        partition, type,
                        start, len,
                        offset, entry,
                        ext_offset, buf);
}


/* For simplicity.  */
//auto int next (void);
int next (int *bsd_part, int *pc_slice,
          unsigned long current_drive, unsigned long dest_partition,
          unsigned long *current_partition, int *current_slice,
          unsigned long *part_start, unsigned long *part_length,
          unsigned long *part_offset, int *entry, unsigned long *ext_offset, char *buf)
{
    int ret = next_partition (current_drive, dest_partition,
                              current_partition, current_slice,
                              part_start, part_length,
                              part_offset, entry, ext_offset, buf);
    *bsd_part = (*current_partition >> 8) & 0xFF;
    *pc_slice = *current_partition >> 16;
    return ret;
}

#ifndef STAGE1_5
static unsigned long cur_part_offset;
static unsigned long cur_part_addr;
#endif

/* Open a partition.  */
int
real_open_partition (int flags)
{
  unsigned long dest_partition = current_partition;
  unsigned long part_offset;
  unsigned long ext_offset;
  int entry;
  char buf[SECTOR_SIZE];
  int bsd_part, pc_slice;

#ifndef STAGE1_5
  /* network drive */
  if (current_drive == NETWORK_DRIVE)
    return 1;

  if (! sane_partition ())
    return 0;
#endif

  bsd_evil_hack = 0;
  current_slice = 0;
  part_start = 0;

  /* Make sure that buf_geom is valid. */
  if (buf_drive != current_drive)
    {
      if (get_diskinfo (current_drive, &buf_geom))
        {
          errnum = ERR_NO_DISK;
          return 0;
        }
      buf_drive = current_drive;
      buf_track = -1;
    }
  part_length = buf_geom.total_sectors;

  /* If this is the whole disk, return here.  */
  if (! flags && current_partition == 0xFFFFFF)
    return 1;

  if (flags)
    dest_partition = 0xFFFFFF;

  /* Initialize CURRENT_PARTITION for next_partition.  */
  current_partition = 0xFFFFFF;

  while (next (&bsd_part, &pc_slice,
               current_drive, dest_partition,
               &current_partition, &current_slice,
               &part_start, &part_length,
               &part_offset, &entry, &ext_offset, buf))
    {

#ifndef STAGE1_5
    loop_start:

      cur_part_offset = part_offset;
      cur_part_addr = BOOT_PART_TABLE + (entry << 4);
#endif /* ! STAGE1_5 */

      /* If this is a valid partition...  */
      if (current_slice)
        {
#ifndef STAGE1_5
          /* Display partition information.  */
          if (flags && ! IS_PC_SLICE_TYPE_EXTENDED (current_slice))
            {
              if (! do_completion)
                {
                  //if (current_drive & 0x80)
                  //  grub_printf ("   Partition num: %d, ",
                  //               current_partition >> 16);

                  if (! IS_PC_SLICE_TYPE_BSD (current_slice))
                    check_and_print_mount (flags);
                  else
                    {
                      int got_part = 0;
                      int saved_slice = current_slice;

                      while (next (&bsd_part, &pc_slice,
                                   current_drive, dest_partition,
                                   &current_partition, &current_slice,
                                   &part_start, &part_length,
                                   &part_offset, &entry, &ext_offset, buf))
                        {
                          if (bsd_part == 0xFF)
                            break;

                          if (! got_part)
                            {
                              //grub_printf ("[BSD sub-partitions immediately follow]\n");
                              got_part = 1;
                            }

                          //grub_printf ("     BSD Partition num: \'%c\', ",
                          //             bsd_part + 'a');
                          check_and_print_mount (flags);
                        }

                      //if (! got_part)
                      //  grub_printf (" No BSD sub-partition found, partition type 0x%x\n",
                      //               saved_slice);

                      if (errnum)
                        {
                          errnum = ERR_NONE;
                          break;
                        }

                      goto loop_start;
                    }
                }
              //else
              //  {
                  //if (bsd_part != 0xFF)
                  //  {
                      //char str[16];

                      //if (! (current_drive & 0x80)
                      //    || (dest_partition >> 16) == pc_slice)
                      //  grub_sprintf (str, "%c)", bsd_part + 'a');
                      //else
                      //  grub_sprintf (str, "%d,%c)",
                      //                pc_slice, bsd_part + 'a');
                      //print_a_completion (str);
                  //  }
                  //else if (! IS_PC_SLICE_TYPE_BSD (current_slice))
                  //  {
                  //    char str[8];

                  //    grub_sprintf (str, "%d)", pc_slice);
                  //    print_a_completion (str);
                  //  }
              //  }
            }

          errnum = ERR_NONE;
#endif /* ! STAGE1_5 */

          /* Check if this is the destination partition.  */
          if (! flags
              && (dest_partition == current_partition
                  || ((dest_partition >> 16) == 0xFF
                      && ((dest_partition >> 8) & 0xFF) == bsd_part)))
            return 1;
        }
    }

#ifndef STAGE1_5
  if (flags)
    {
      errnum = ERR_NONE;
      if (! (current_drive & 0x80))
        {
          current_partition = 0xFFFFFF;
          current_slice = 0;
          part_start = 0;
          part_length = buf_geom.total_sectors;
          check_and_print_mount (flags);
        }
      else
        {
          cur_part_addr = 0;
          cur_part_offset = 0;
        }

      errnum = ERR_NONE;
      return 1;
    }
#endif /* ! STAGE1_5 */

  return 0;
}


#endif

int
open_partition (void)
{
#ifndef STAGE1_5
  return real_open_partition (0);
#else
  return 1;
#endif
}


/*
 *  This performs a "mount" on the current device, both drive and partition
 *  number.
 */

int
open_device (void)
{
  //if (open_partition ())
  //  attempt_mount ();
  //
  //if (errnum != ERR_NONE)
  //  return 0;
  //
  //return 1;

  return open_partition() && attempt_mount();
}

/* open_device() wrapper. Attempts to set a filesystem
 * in loop, and try to mount it
 */
int
open_device2(void)
{
#ifndef STAGE1_5
  char buf[0x100];
  int fst;
  unsigned int cd = current_drive;
  unsigned int cp = current_partition;

  set_boot_fsys();
  if (open_device())
    return 1;
  
  for (fst = 0; fst < num_fsys; fst++) {
    fsys_type = fst;
    fsys_by_num(fst, buf);
    set_fsys(buf);

    current_drive = cd;
    current_partition = cp;
    
    if (open_device())
      return 1;
  }
#else
  return open_device();
#endif
  errnum = ERR_FSYS_MOUNT;
  return 0;
} 

static char *
setup_part (char *filename)
{
#ifdef STAGE1_5

  if (! (filename = set_device (filename)))
    {
      current_drive = GRUB_INVALID_DRIVE;
      return 0;
    }

# ifndef NO_BLOCK_FILES
  if (*filename != '/')
    open_partition ();
  else
# endif /* ! NO_BLOCK_FILES */
    open_device ();

#else /* ! STAGE1_5 */

  if (*filename == '(')
    {
      if ((filename = set_device (filename)) == 0)
        {
          current_drive = GRUB_INVALID_DRIVE;
          return 0;
        }
# ifndef NO_BLOCK_FILES
      if (*filename != '/')
        open_partition ();
      else
# endif /* ! NO_BLOCK_FILES */
        open_device2 ();
    }
  else if (saved_drive != current_drive
           || saved_partition != current_partition
           || (*filename == '/' && fsys_type == num_fsys)
           || buf_drive == -1)
    {
      current_drive = saved_drive;
      current_partition = saved_partition;
      /* allow for the error case of "no filesystem" after the partition
         is found.  This makes block files work fine on no filesystem */
# ifndef NO_BLOCK_FILES
      if (*filename != '/')
        open_partition ();
      else
# endif /* ! NO_BLOCK_FILES */
        open_device2 ();
    }

#endif /* ! STAGE1_5 */

  if (errnum && (*filename == '/' || errnum != ERR_FSYS_MOUNT))
    return 0;
  else
    errnum = 0;

#ifndef STAGE1_5
  if (!sane_partition ())
    return 0;
#endif

  return filename;
}


int
devread (int sector, int byte_offset, int byte_len, char *buf)
{
  /*
   *  Check partition boundaries
   */
  if (sector < 0
      || ((sector + ((byte_offset + byte_len - 1) >> SECTOR_BITS))
          >= part_length))
    {
      errnum = ERR_OUTSIDE_PART;
      return 0;
    }

  /*
   *  Get the read to the beginning of a partition.
   */
  sector += byte_offset >> SECTOR_BITS;
  byte_offset &= SECTOR_SIZE - 1;

//#if !defined(STAGE1_5)
//  if (disk_read_hook && debug)
//    printf ("<%d, %d, %d>", sector, byte_offset, byte_len);
//#endif /* !STAGE1_5 */

  /*
   *  Call RAWREAD, which is very similar, but:
   *
   *    --  It takes an extra parameter, the drive number.
   *    --  It requires that "sector" is relative to the beginning
   *            of the disk.
   *    --  It doesn't handle offsets of more than 511 bytes into the
   *            sector.
   */
  return rawread (current_drive, part_start + sector, byte_offset,
                  byte_len, buf);
}


/*
 *  This is the generic file open function.
 */

int
grub_open (char *filename)
{
#ifndef STAGE1_5
#define buf_size 1500
//const int buf_size = 1500;
  const char *try_filenames[] = { "menu.lst", "m" };
  char fn[buf_size]; /* arbitrary... */
  char *filename_orig = filename;
  int trycount = 0;

  //if (grub_strlen(filename) > buf_size)
  //  {
  //    printf("Buffer overflow: %s(%d)\n", __FILE__, __LINE__);
  //    while (1) {}
  //  }

  /* initially, we need to copy filename to fn */
  grub_strcpy(fn, filename_orig);
  fn[grub_strlen(filename_orig)] = 0;

restart:
  errnum = 0; /* hrm... */
  filename = fn;
#endif

#ifndef NO_DECOMPRESSION
  compressed_file = 0;
#endif /* NO_DECOMPRESSION */

  /* if any "dir" function uses/sets filepos, it must
     set it to zero before returning if opening a file! */
  filepos = 0;

  if (!(filename = (char *)setup_part (filename)))
    {
      if (errnum == ERR_FILE_NOT_FOUND)
        goto retry;
      else
        return 0;
    }

#ifndef NO_BLOCK_FILES
  block_file = 0;
#endif /* NO_BLOCK_FILES */

  /* This accounts for partial filesystem implementations. */
  fsmax = MAXINT;

  if (*filename != '/')
    {
#ifndef NO_BLOCK_FILES
      char *ptr = filename;
      int tmp, list_addr = BLK_BLKLIST_START;
      filemax = 0;

      while (list_addr < BLK_MAX_ADDR)
        {
          tmp = 0;
          safe_parse_maxint (&ptr, &tmp);
          errnum = 0;

          if (*ptr != '+')
            {
              if ((*ptr && *ptr != '/' && !isspace (*ptr))
                  || tmp == 0 || tmp > filemax)
                errnum = ERR_BAD_FILENAME;
              else
                filemax = tmp;

              break;
            }

          /* since we use the same filesystem buffer, mark it to
             be remounted */
          //fsys_type = NUM_FSYS;
          fsys_type = num_fsys;

          BLK_BLKSTART (list_addr) = tmp;
          ptr++;

          if (!safe_parse_maxint (&ptr, &tmp)
              || tmp == 0
              || (*ptr && *ptr != ',' && *ptr != '/' && !isspace (*ptr)))
            {
              errnum = ERR_BAD_FILENAME;
              break;
            }

          BLK_BLKLENGTH (list_addr) = tmp;

          filemax += (tmp * SECTOR_SIZE);
          list_addr += BLK_BLKLIST_INC_VAL;

          if (*ptr != ',')
            break;

          ptr++;
        }

      if (list_addr < BLK_MAX_ADDR && ptr != filename && !errnum)
        {
          block_file = 1;
          BLK_CUR_FILEPOS = 0;
          BLK_CUR_BLKLIST = BLK_BLKLIST_START;
          BLK_CUR_BLKNUM = 0;

#ifndef NO_DECOMPRESSION
          return gunzip_test_header ();
#else /* NO_DECOMPRESSION */
          return 1;
#endif /* NO_DECOMPRESSION */
        }
#else /* NO_BLOCK_FILES */
      errnum = ERR_BAD_FILENAME;
#endif /* NO_BLOCK_FILES */
    }

  if (!errnum && fsys_type == num_fsys) // fsys_type == NUM_FSYS)
    errnum = ERR_FSYS_MOUNT;

# ifndef STAGE1_5
  /* set "dir" function to open a file */
  print_possibilities = 0;
# endif
                                // NUM_FSYS + 1
  if ( !errnum && stage0_dir(filename) )
     //  (!errnum && (*(fsys_table[fsys_type].dir_func)) (filename)) )
    {
#ifndef NO_DECOMPRESSION
      return gunzip_test_header ();
#else /* NO_DECOMPRESSION */
      return 1;
#endif /* NO_DECOMPRESSION */
    }

retry:
# ifndef STAGE1_5
  if (trycount < sizeof(try_filenames) / sizeof(*try_filenames))
    {
      unsigned int l = grub_strlen(filename_orig);
      unsigned int ll = grub_strlen(try_filenames[trycount]);
      int j;

      grub_memmove(fn, filename_orig, l);

      if (filename_orig[l - 1] != '/')
        fn[l++] = '/';
      for (j = 0; j < ll; j++)
        fn[l + j] = try_filenames[trycount][j];
      fn[l + ll] = 0;

      //grub_printf("Previous try failed, trying \"%s\"\n", fn);
      trycount++;
      goto restart;
    }
# endif


  return 0;
}


int
grub_read (char *buf, int len)
{
  /* Make sure "filepos" is a sane value */
  if ((filepos < 0) || (filepos > filemax))
    filepos = filemax;

  /* Make sure "len" is a sane value */
  if ((len < 0) || (len > (filemax - filepos)))
    len = filemax - filepos;

  /* if target file position is past the end of
     the supported/configured filesize, then
     there is an error */
  if (filepos + len > fsmax)
    {
      errnum = ERR_FILELENGTH;
      return 0;
    }

#ifndef NO_DECOMPRESSION
  if (compressed_file)
    return gunzip_read (buf, len);
#endif /* NO_DECOMPRESSION */

#ifndef NO_BLOCK_FILES
  if (block_file)
    {
      int size, off, ret = 0;

      while (len && !errnum)
        {
          /* we may need to look for the right block in the list(s) */
          if (filepos < BLK_CUR_FILEPOS)
            {
              BLK_CUR_FILEPOS = 0;
              BLK_CUR_BLKLIST = BLK_BLKLIST_START;
              BLK_CUR_BLKNUM = 0;
            }

          /* run BLK_CUR_FILEPOS up to filepos */
          while (filepos > BLK_CUR_FILEPOS)
            {
              if ((filepos - (BLK_CUR_FILEPOS & ~(SECTOR_SIZE - 1)))
                  >= SECTOR_SIZE)
                {
                  BLK_CUR_FILEPOS += SECTOR_SIZE;
                  BLK_CUR_BLKNUM++;

                  if (BLK_CUR_BLKNUM >= BLK_BLKLENGTH (BLK_CUR_BLKLIST))
                    {
                      BLK_CUR_BLKLIST += BLK_BLKLIST_INC_VAL;
                      BLK_CUR_BLKNUM = 0;
                    }
                }
              else
                BLK_CUR_FILEPOS = filepos;
            }

          off = filepos & (SECTOR_SIZE - 1);
          size = ((BLK_BLKLENGTH (BLK_CUR_BLKLIST) - BLK_CUR_BLKNUM)
                  * SECTOR_SIZE) - off;
          if (size > len)
            size = len;

          disk_read_func = disk_read_hook;

          /* read current block and put it in the right place in memory */
          devread (BLK_BLKSTART (BLK_CUR_BLKLIST) + BLK_CUR_BLKNUM,
                   off, size, buf);

          disk_read_func = NULL;

          len -= size;
          filepos += size;
          ret += size;
          buf += size;
        }

      if (errnum)
        ret = 0;

      return ret;
    }
#endif /* NO_BLOCK_FILES */

                     // NUM_FSYS
  if (fsys_type == num_fsys)
    {
      errnum = ERR_FSYS_MOUNT;
      return 0;
    }

  return stage0_read(buf, len);
  //return (*(fsys_table[fsys_type].read_func)) (buf, len);
}

/* Reposition a file offset.  */
int
grub_seek (int offset)
{
  if (offset > filemax || offset < 0)
    return -1;

  filepos = offset;
  return offset;
}

#ifndef STAGE1_5
int
dir (char *dirname)
{
#ifndef NO_DECOMPRESSION
  compressed_file = 0;
#endif /* NO_DECOMPRESSION */

  if (!(dirname = setup_part (dirname)))
    return 0;

  if (*dirname != '/')
    errnum = ERR_BAD_FILENAME;
                   // NUM_FSYS
  if (fsys_type == num_fsys)
    errnum = ERR_FSYS_MOUNT;

  if (errnum)
    return 0;

  /* set "dir" function to list completions */
  print_possibilities = 1;

  //if (fsys_type == NUM_FSYS + 1)
  return stage0_dir(dirname);

  //return (*(fsys_table[fsys_type].dir_func)) (dirname);
}
#endif /* STAGE1_5 */

void
grub_close (void)
{
#ifndef NO_BLOCK_FILES
  if (block_file)
    return;
#endif /* NO_BLOCK_FILES */

  //if (fsys_type == NUM_FSYS + 1)
  stage0_close();
  //else
  //  if (fsys_table[fsys_type].close_func != 0)
  //    (*(fsys_table[fsys_type].close_func)) ();
}
