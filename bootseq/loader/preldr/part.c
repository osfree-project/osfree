/*
 */

#include <shared.h>
#include <pc_slice.h>
#include <freebsd.h>
#include <bpb.h>

#include "fsd.h"

extern unsigned long saved_drive;
extern unsigned long saved_partition;
extern unsigned long cdrom_drive;
unsigned long saved_slice;

extern unsigned long boot_drive;
extern unsigned long install_partition;

#pragma aux boot_drive        "*"
#pragma aux install_partition "*"

unsigned long current_drive;
unsigned long current_partition;
int           current_slice;
unsigned long part_start;
unsigned long part_length;
int           fsmax;

/* disk buffer parameters */
int buf_drive = -1;
int buf_track;
struct geometry buf_geom;

#pragma aux errnum "*"

extern grub_error_t errnum;

#ifndef STAGE1_5

int
attempt_mount (void);

static int do_completion;

extern int bsd_evil_hack;

//#ifndef STAGE1_5
int
sane_partition (void);
//#endif

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

#else // STAGE1_5

#pragma pack(1)

typedef struct _desc {
  char active;
  char start_chs[3];
  char id;
  char end_chs[3];
  unsigned long start_lba;
  unsigned long len;
} desc;

/* Partitioning sector: bootsector or EBR/MBR */
typedef struct _partsect {
  char jump[3];
  char oemid[8];
  bios_parameters_block bpb;
  char code[512 - 3 - 8 - sizeof(bios_parameters_block) - 4 * sizeof(desc) - 2];
  desc pt[4];
  char sig[2];
} partsect;

#pragma pack()

int open_partition_hiddensecs(void)
{
  partsect *p;
  int     lba;
  struct  geometry *geo;
  char    *buf = (char *)0x20000;
  desc    *q;

  p = (partsect *)(BOOTSEC_BASE);
  /* LBA of partition EBR */
  lba = p->bpb.hidden_secs - p->bpb.track_size;
  part_start = p->bpb.hidden_secs;

  rawread (boot_drive, lba, 0, 512, buf);
  p = (partsect *)buf;

  /* 1st partition descriptor */
  q = p->pt;

  current_slice = q->id;
  part_length   = q->len;

  current_drive = boot_drive;
  current_partition = install_partition;

  errnum = ERR_NONE;
  return 1;
}
#endif

int
open_partition (void)
{
#ifndef STAGE1_5
  return real_open_partition (0);
#else
  return open_partition_hiddensecs();
#endif
}
