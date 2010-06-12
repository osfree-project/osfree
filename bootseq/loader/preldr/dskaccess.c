/*
 */

#include <pc_slice.h>
#include <shared.h>
#include "fsd.h"
#include "fsys.h"

extern grub_error_t errnum;

extern unsigned long part_start;
extern unsigned long part_length;
extern unsigned long current_drive;

extern int buf_drive;
extern int buf_track;

extern void (*disk_read_func) (int, int, int);
extern void (*disk_read_hook) (int, int, int);

extern unsigned long scratchaddr;
extern unsigned long bufferaddr;

extern unsigned long sector_size;

#if 0

#ifndef u_diskctl
#define u_diskctl biosdisk
int
biosdisk (int read, int drive, struct geometry *geometry,
          int sector, int nsec, int segment);
#endif

#ifndef get_diskinfo

int
get_diskinfo (int drive, struct geometry *geometry)
{
  return u_diskctl (BIOSDISK_GEO, drive, geometry, 0, 0, 0);
}

#endif

#endif

int
rawread_loop (int num, int sector, int byte_offset, int byte_len, char *buf);

int open_device(void);
void fsys_by_num(int n, char *buf);
int set_fsys(char *fsname);

#pragma aux fsys_by_num "*"
#pragma aux set_fsys    "*"

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


int
rawread (int drive, int sector, int byte_offset, int byte_len, char *buf)
{
  int slen, sectors_per_vtrack;
  int sector_size_bits = log2 (sector_size); //buf_geom.sector_size);

  if (byte_len <= 0)
    return 1;

#ifndef STAGE1_5
  if (0xC0 <= drive && drive <= 0xC7)
    return rawread_loop (drive - 0xC0, sector, byte_offset, byte_len, buf);
#endif

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
          sector_size = buf_geom.sector_size;    //
          sector_size_bits = log2 (sector_size); //buf_geom.sector_size);
        }

      /* Make sure that SECTOR is valid.  */
      if (sector < 0 || sector >= buf_geom.total_sectors)
        {
          errnum = ERR_GEOM;
          return 0;
        }

      slen = ((byte_offset + byte_len + sector_size - 1) // buf_geom.sector_size - 1)
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
      bufaddr = ((char *) bufferaddr
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
              bufaddr = (char *) bufferaddr + byte_offset;
            }

          bios_err = biosdisk (BIOSDISK_READ, drive, &buf_geom,
                               read_start, read_len, bufferaddr >> 4);
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
                                   sector, slen, bufferaddr >> 4))
                    errnum = ERR_READ;

                  bufaddr = (char *) bufferaddr + byte_offset;
                }
            }
          else
            buf_track = track;

          if ((buf_track == 0 || sector == 0)
              && (PC_SLICE_TYPE (bufferaddr, 0) == PC_SLICE_TYPE_EZD
                  || PC_SLICE_TYPE (bufferaddr, 1) == PC_SLICE_TYPE_EZD
                  || PC_SLICE_TYPE (bufferaddr, 2) == PC_SLICE_TYPE_EZD
                  || PC_SLICE_TYPE (bufferaddr, 3) == PC_SLICE_TYPE_EZD))
            {
              /* This is a EZD disk map sector 0 to sector 1 */
              if (buf_track == 0 || slen >= 2)
                {
                  /* We already read the sector 1, copy it to sector 0 */
                  memmove ((char *) bufferaddr,
                           (char *) bufferaddr + sector_size, // buf_geom.sector_size,
                           sector_size); // buf_geom.sector_size);
                }
              else
                {
                  if (biosdisk (BIOSDISK_READ, drive, &buf_geom,
                                1, 1, bufferaddr >> 4))
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
          int length = sector_size - byte_offset; // buf_geom.sector_size - byte_offset;
          if (length > size)
            length = size;
          (*disk_read_func) (sector_num++, byte_offset, length);
          length = size - length;
          if (length > 0)
            {
              while (length > sector_size) // buf_geom.sector_size)
                {
                  (*disk_read_func) (sector_num++, 0, sector_size); // buf_geom.sector_size);
                  length -= sector_size; // buf_geom.sector_size;
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

#ifndef STAGE1_5

int
rawread_loop (int num, int sector, int byte_offset, int byte_len, char *buf)
{
  char s[0x100];
  unsigned size;
  unsigned cd = current_drive;
  unsigned cp = current_partition;
  unsigned cs = current_slice;
  int      rc;
  int      ss = buf_geom.sector_size;

  sector_size = SECTOR_SIZE; // image sector size is here
  if (u_open((char *)(LOOPARRAY + STRBUF_SIZE * num), &size))
    return 0;

  sector_size = SECTOR_SIZE; // image sector size is here

  if (u_seek((sector * sector_size) + byte_offset) == -1)
    return 0;

  sector_size = ss;

  if (u_read(buf, byte_len) != byte_len)
    return 0;

  sector_size = ss;
  current_drive = cd;
  current_partition = cp;
  current_slice = cs;

  return (!errnum);
}

int
rawwrite (int drive, int sector, char *buf)
{
  if (sector == 0)
    {
      if (biosdisk (BIOSDISK_READ, drive, &buf_geom, 0, 1, scratchaddr >> 4))
        {
          errnum = ERR_WRITE;
          return 0;
        }

      if (PC_SLICE_TYPE (scratchaddr, 0) == PC_SLICE_TYPE_EZD
          || PC_SLICE_TYPE (scratchaddr, 1) == PC_SLICE_TYPE_EZD
          || PC_SLICE_TYPE (scratchaddr, 2) == PC_SLICE_TYPE_EZD
          || PC_SLICE_TYPE (scratchaddr, 3) == PC_SLICE_TYPE_EZD)
        sector = 1;
    }

  memmove ((char *) scratchaddr, buf, SECTOR_SIZE);
  if (biosdisk (BIOSDISK_WRITE, drive, &buf_geom,
                sector, 1, scratchaddr >> 4))
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

#endif

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

