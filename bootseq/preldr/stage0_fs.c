/*
 *
 *
 */

#include <lip.h>
#include <shared.h>

lip_t *l;
lip_t lip;

/* Far pointer as a structure */
typedef _Packed struct fp {
  unsigned short off;
  unsigned short seg;
} fp_t;

int __cdecl (*fsd_init)(lip_t *l);

extern int mem_lower;
extern int mem_upper;
extern int fsmax;

extern void __cdecl real_test(void);
extern void __cdecl call_rm(fp_t);
extern void __cdecl printmsg(char *);
extern void __cdecl printb(unsigned char);
extern void __cdecl printw(unsigned short);
extern void __cdecl printd(unsigned long);

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
  l->lip_seek  = 0; //&grub_seek;
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

  printmsg("Hello world!\r\n");
  printb(0xab);
  printmsg("\r\n");
  printw(0xbeef);
  printmsg("\r\n");
  printd(0x12345678);

  printmsg("\r\nsetting lip");
  setlip();
  printmsg("\r\nsetlip() returned\r\n");

  fsd_init = (void *)(0x39000); // uFSD base address
  fsd_init(l);
  printmsg("uFSD init returned\r\n");

  size = stage0_mount();
  printmsg("stage0_mount() returned: ");
  printd(size);

  size = grub_open("/boot/bootblock");
  printmsg("\r\ngrub_open(\"/boot/bootblock\") returned: ");
  printd(size);

  return 0;
}

#pragma aux init     "_*"
#pragma aux real_test "*"
#pragma aux call_rm   "*"
#pragma aux printmsg  "*"
#pragma aux printb    "*"
#pragma aux printw    "*"
#pragma aux printd    "*"
