/*
 *
 *
 */

#include <pc_slice.h>
#include <freebsd.h>
#include <shared.h>
#include <fsys.h>
#include <fsd.h>

#ifndef STAGE1_5

extern unsigned long scratchaddr;

/* The boot device.  */
static int bootdev;
int bsd_evil_hack;

extern unsigned long cur_part_offset;
extern unsigned long cur_part_addr;

unsigned long boot_part_addr;

int
real_mkroot (char *arg, int attempt_mount)
{
  int hdbias = 0;
  char *biasptr;
  char *next;

  errnum = 0;

  /* If ARG is empty, just print the current root device.  */
  //if (! *arg)
  //  {
  //    print_root_device ();
  //    return 0;
  //  }

  /* Call set_device to get the drive and the partition in ARG.  */
  next = set_device (arg);
  if (! next)
    return 1;

  /* Ignore ERR_FSYS_MOUNT.  */
  if (attempt_mount)
    {
      if (! open_device () && errnum != ERR_FSYS_MOUNT)
        return 1;
    }
  else
    {
      /* This is necessary, because the location of a partition table
         must be set appropriately.  */
      if (open_partition ())
        {
          set_bootdev (0);
          if (errnum)
            return 1;
        }
    }

  /* Clear ERRNUM.  */
  errnum = 0;
  saved_partition = current_partition;
  saved_drive = current_drive;

  if (attempt_mount)
    {
      /* BSD and chainloading evil hacks !!  */
      biasptr = skip_to (0, next);
      safe_parse_maxint (&biasptr, &hdbias);
      errnum = 0;
      bootdev = set_bootdev (hdbias);
      if (errnum)
        return 1;

      /* Print the type of the filesystem.  */
      //print_fsys_type ();
    }

  return 0;
}

int
mkroot (char *arg)
{
  return real_mkroot (arg, 1);
}

int
set_bootdev (int hdbias)
{
  int i, j;

  /* Copy the boot partition information to 0x7be-0x7fd for chain-loading.  */
  if ((saved_drive & 0x80) && cur_part_addr)
    {
      if (rawread (saved_drive, cur_part_offset,
                   0, SECTOR_SIZE, (char *) scratchaddr))
        {
          char *dst, *src;

          /* Need only the partition table.
             XXX: We cannot use grub_memmove because BOOT_PART_TABLE
             (0x07be) is less than 0x1000.  */
          dst = (char *) BOOT_PART_TABLE;
          src = (char *) scratchaddr + BOOTSEC_PART_OFFSET;
          while (dst < (char *) BOOT_PART_TABLE + BOOTSEC_PART_LENGTH)
            *dst++ = *src++;

          /* Set the active flag of the booted partition.  */
          for (i = 0; i < 4; i++)
            PC_SLICE_FLAG (BOOT_PART_TABLE, i) = 0;

          *((unsigned char *) cur_part_addr) = PC_SLICE_FLAG_BOOTABLE;
          boot_part_addr = cur_part_addr;
        }
      else
        return 0;
    }

  /*
   *  Set BSD boot device.
   */
  i = (saved_partition >> 16) + 2;
  if (saved_partition == 0xFFFFFF)
    i = 1;
  else if ((saved_partition >> 16) == 0xFF)
    i = 0;

  /* FIXME: extremely evil hack!!! */
  j = 2;
  if (saved_drive & 0x80)
    j = bsd_evil_hack;

  return MAKEBOOTDEV (j, (i >> 4), (i & 0xF),
                      ((saved_drive - hdbias) & 0x7F),
                      ((saved_partition >> 8) & 0xFF));
}
#endif /* STAGE1_5 */
