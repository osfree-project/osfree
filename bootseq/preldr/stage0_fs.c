/*
 *
 *
 */

#include <lip.h>
#include <shared.h>

#include "fsys.h"
#include "fsd.h"

lip_t *l;
lip_t lip;

#pragma aux lip "*"

/* Far pointer as a structure */
typedef _Packed struct fp {
  unsigned short off;
  unsigned short seg;
} fp_t;

int __cdecl (*fsd_init)(lip_t *l);

extern unsigned long saved_drive;
extern unsigned long saved_partition;
extern unsigned long cdrom_drive;


extern int mem_lower;
extern int mem_upper;
extern int fsmax;

void __cdecl real_test(void);
void __cdecl call_rm(fp_t);
void __cdecl printmsg(char *);
void __cdecl printb(unsigned char);
void __cdecl printw(unsigned short);
void __cdecl printd(unsigned long);

int
freeldr_open (char *filename)
{
   char *p;
   int  i;
   int  i0 = 0;
   char buf[128];

   printmsg(filename);
   printmsg("\r\n");

   /* prepend "/" to filename */
   if (*filename != '/') {
     buf[0] = '/';
     i0 = 1;
   }

   /* convert to lowercase */
   for (p = filename, i = 0; p[i] && i < 128; i++)
     if (p[i] > 0x40 && p[i] < 0x5b)
       buf[i + i0] = p[i] + 0x20;
     else
       buf[i + i0] = p[i];

   buf[i + i0] = '\0';

   return grub_open(buf);
}

int
freeldr_read (char *buf, int len)
{
   return grub_read(buf, len);
}

int
freeldr_seek (int offset)
{
   return grub_seek(offset);
}

void
freeldr_close (void)
{
   grub_close();
}

int  stage0_mount (void)
{
  return l->lip_fs_mount();
}

int  stage0_read (char *buf, int len)
{
  return l->lip_fs_read(buf, len);
}

int  stage0_dir (char *dirname)
{
  return l->lip_fs_dir(dirname);
}

void setlip(void)
{
  l = &lip;

  l->lip_open  = &grub_open;
  l->lip_read  = &grub_read;
  l->lip_seek  = &grub_seek;
  l->lip_close = &grub_close;
  l->lip_term  = 0;

  l->lip_memcheck = 0; //&grub_memcheck;
  l->lip_memset   = &grub_memset;
  l->lip_memmove  = &grub_memmove;
  l->lip_strcpy   = 0; //&grub_strcpy;
  l->lip_strcmp   = 0; //&grub_strcmp;
  l->lip_memcmp   = &grub_memcmp;
  l->lip_strlen   = 0; //&grub_strlen;
  l->lip_isspace  = &grub_isspace;
  l->lip_tolower  = &grub_tolower;

  l->lip_substring = &substring;
  l->lip_pos       = 0;
  l->lip_clear     = 0;

  l->lip_devread   = &devread;
  l->lip_rawread   = &rawread;

  l->lip_mem_lower = &mem_lower;
  l->lip_mem_upper = &mem_upper;

  l->lip_filepos   = &filepos;
  l->lip_filemax   = &filemax;

  l->lip_buf_drive = &buf_drive;
  l->lip_buf_track = &buf_track;
  l->lip_buf_geom  = &buf_geom;

  l->lip_errnum    = &errnum;

  l->lip_saved_drive = &saved_drive;
  l->lip_saved_partition = &saved_partition;

  l->lip_current_drive = &current_drive;
  l->lip_current_partition = &current_partition;
  l->lip_current_slice = &current_slice;
  l->lip_part_start    = &part_start;
  l->lip_part_length   = &part_length;
  l->lip_fsmax         = &fsmax;
}

int init(void)
{
  int size;
  char *buf;
  long dev = 0xe0ffffff; // 0xe0 -- bochs, qemu; 0xef -- vpc; 0x9f -- real hardware
  //char bf[256];

  /* Set boot drive and partition.  */
  saved_drive = boot_drive;
  saved_partition = install_partition;

  /* Set cdrom drive.  */
  {
    struct geometry geom;

    /* Get the geometry.  */
    if (get_diskinfo (boot_drive, &geom)
        || ! (geom.flags & BIOSDISK_FLAG_CDROM))
      cdrom_drive = GRUB_INVALID_DRIVE;
    else
      cdrom_drive = boot_drive;
  }

  setlip();
  printmsg("\r\nsetlip() returned\r\n");

  fsd_init = (void *)(UFSD_BASE); // uFSD base address
  fsd_init(l);
  printmsg("uFSD init returned\r\n");


  size = freeldr_open("os2ldr");
  printmsg("freeldr_open(\"/os2ldr\") returned: ");
  printd(size);

  buf = (char *)(0x10000);

  if (size)
     size = freeldr_read(buf, -1);

  printmsg("\r\nfreeldr_read() returned size: ");
  printd(size);
  printmsg("\r\n");


  size = freeldr_open("os2boot");
  printmsg("freeldr_open(\"/os2boot\") returned: ");
  printd(size);

  buf = (char *)(0x7c0);

  if (size)
     size = freeldr_read(buf, -1);

  printmsg("\r\nfreeldr_read() returned size: ");
  printd(size);
  printmsg("\r\n");


  return 0;
}

#pragma aux init     "_*"
#pragma aux real_test "*"
#pragma aux call_rm   "*"
#pragma aux printmsg  "*"
#pragma aux printb    "*"
#pragma aux printw    "*"
#pragma aux printd    "*"
