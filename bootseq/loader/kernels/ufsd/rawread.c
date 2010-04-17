#include <pc_slice.h>
#include <shared.h>
#include "fsd.h"

#define BUF_LEN  0x1000
#define BUF_ADDR (REL1_BASE - 0x200 - 0x2000 - 0x1000)

int  buf_drive = -1;
int  buf_track = -1;
struct geometry buf_geom;
grub_error_t errnum = 0;
void (*disk_read_func) (int, int, int) = 0;

int kprintf(const char *format, ...);

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
          kprintf("enter\n");
          if (get_diskinfo (drive, &buf_geom))
            {
              errnum = ERR_NO_DISK;
              return 0;
            }
          kprintf("exit\n");
          kprintf("errnum=0x%x\n", errnum);
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
      if ((buf_geom.sectors << sector_size_bits) > BUF_LEN)
        sectors_per_vtrack = (BUF_LEN >> sector_size_bits);
      else
        sectors_per_vtrack = buf_geom.sectors;

      /* Get the first sector of track.  */
      soff = sector % sectors_per_vtrack;
      track = sector - soff;
      num_sect = sectors_per_vtrack - soff;
      bufaddr = ((char *) BUF_ADDR
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
              bufaddr = (char *) BUF_ADDR + byte_offset;
            }

          kprintf("biosdisk1 enter\n");
          bios_err = biosdisk (BIOSDISK_READ, drive, &buf_geom,
                               read_start, read_len, BUF_ADDR >> 4);
          kprintf("biosdisk1 exit\n");
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
                  kprintf("biosdisk2 enter\n");
                  if (slen > num_sect
                      || biosdisk (BIOSDISK_READ, drive, &buf_geom,
                                   sector, slen, BUF_ADDR >> 4))
                    errnum = ERR_READ;

                  kprintf("biosdisk2 exit\n");

                  bufaddr = (char *) BUF_ADDR + byte_offset;
                }
            }
          else
            buf_track = track;

          if ((buf_track == 0 || sector == 0)
              && (PC_SLICE_TYPE (BUF_ADDR, 0) == PC_SLICE_TYPE_EZD
                  || PC_SLICE_TYPE (BUF_ADDR, 1) == PC_SLICE_TYPE_EZD
                  || PC_SLICE_TYPE (BUF_ADDR, 2) == PC_SLICE_TYPE_EZD
                  || PC_SLICE_TYPE (BUF_ADDR, 3) == PC_SLICE_TYPE_EZD))
            {
              /* This is a EZD disk map sector 0 to sector 1 */
              if (buf_track == 0 || slen >= 2)
                {
                  /* We already read the sector 1, copy it to sector 0 */
                  memmove ((char *) BUF_ADDR,
                           (char *) BUF_ADDR + buf_geom.sector_size,
                           buf_geom.sector_size);
                }
              else
                {
                  kprintf("biosdisk3 enter\n");
                  if (biosdisk (BIOSDISK_READ, drive, &buf_geom,
                                1, 1, BUF_ADDR >> 4))
                    errnum = ERR_READ;
                  kprintf("biosdisk3 exit\n");
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
      memmove (buf, bufaddr, size);

      buf += size;
      byte_len -= size;
      sector += num_sect;
      byte_offset = 0;
    }

  return (!errnum);
}
