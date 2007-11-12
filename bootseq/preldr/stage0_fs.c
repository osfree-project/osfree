/*
 *
 *
 */

#include <lip.h>
#include <types.h>
#include <loader.h>
#include <bpb.h>

#include <shared.h>

#include "fsys.h"
#include "fsd.h"

lip_t *l;
lip_t lip;

#pragma aux lip "*"

int __cdecl (*fsd_init)(lip_t *l);

extern mu_Open;
extern mu_Read;
extern mu_Close;
extern mu_Terminate;

extern unsigned long saved_drive;
extern unsigned long saved_partition;
extern unsigned long cdrom_drive;

extern FileTable ft;

extern unsigned short boot_flags;
extern unsigned long  boot_drive;
extern unsigned long  install_partition;

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

#ifndef STAGE1_5

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
#else
   return grub_open(filename);
#endif
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

  l->lip_open  = &freeldr_open;
  l->lip_read  = &freeldr_read;
  l->lip_seek  = &freeldr_seek;
  l->lip_close = &freeldr_close;
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
  int rc;
  char *buf;
  unsigned long ldrlen, mfslen;
  bios_parameters_block *bpb;
  struct geometry geom;

  /* Set boot drive and partition.  */
  saved_drive = boot_drive;
  saved_partition = install_partition;

  /* Set cdrom drive.  */

  /* Get the geometry.  */
  if (get_diskinfo (boot_drive, &geom)
      || ! (geom.flags & BIOSDISK_FLAG_CDROM))
    cdrom_drive = GRUB_INVALID_DRIVE;
  else
    cdrom_drive = boot_drive;

  /* setting LIP */
  setlip();

  /* call uFSD init (set linkage) */
  fsd_init = (void *)(UFSD_BASE); // uFSD base address
  fsd_init(l);

#ifndef STAGE1_5

  /* load os2ldr */
  rc = freeldr_open("os2ldr");
  printmsg("freeldr_open(\"/os2ldr\") returned: ");
  printd(rc);

  buf = (char *)(LDR_BASE);

  if (rc)
     ldrlen = freeldr_read(buf, -1);

  printmsg("\r\nfreeldr_read() returned size: ");
  printd(ldrlen);
  printmsg("\r\n");

  /* load minifsd */
  rc = freeldr_open("os2boot");
  printmsg("freeldr_open(\"/os2boot\") returned: ");
  printd(rc);

  buf = (char *)(MFS_BASE);

  if (rc)
     mfslen = freeldr_read(buf, -1);

  printmsg("\r\nfreeldr_read() returned size: ");
  printd(mfslen);
  printmsg("\r\n");

  /* set boot flags */
  boot_flags = BOOTFLAG_MICROFSD | BOOTFLAG_MINIFSD;
  /* set filetable */
  ft.ft_cfiles = 3;
  ft.ft_ldrseg = LDR_SEG;
  ft.ft_ldrlen = ldrlen;
  ft.ft_museg  = STAGE0_SEG;
  ft.ft_mulen  = 0xb000;     // It is empirically found maximal value
  ft.ft_mfsseg = MFS_SEG;
  ft.ft_mfslen = mfslen;
  ft.ft_ripseg = 0;          //
  ft.ft_riplen = 0;          // No RIPL data yet

  ft.ft_muOpen.seg       = STAGE0_SEG;
  ft.ft_muOpen.off       = (unsigned short)(&mu_Open);

  ft.ft_muRead.seg       = STAGE0_SEG;
  ft.ft_muRead.off       = (unsigned short)(&mu_Read);

  ft.ft_muClose.seg      = STAGE0_SEG;
  ft.ft_muClose.off      = (unsigned short)(&mu_Close);

  ft.ft_muTerminate.seg  = STAGE0_SEG;
  ft.ft_muTerminate.off  = (unsigned short)(&mu_Terminate);

  if (boot_drive == cdrom_drive) { // booting from CDROM drive
    /* set BPB */
    bpb = (bios_parameters_block *)(BOOTSEC_BASE + 0xb);
    // fill fake BPB
    grub_memset((void *)bpb, 0, sizeof(bios_parameters_block));

    bpb->sect_size  = 0x800;
    bpb->clus_size  = 0x40;
    bpb->n_sect_ext = geom.total_sectors; // 0x30d;
    bpb->media_desc = 0xf8;
    bpb->track_size = 0x3f;
    bpb->heads_cnt  = 0xff;
    bpb->disk_num   = (unsigned char)(boot_drive & 0xff);
    bpb->log_drive  = 0x82;  // fixme! (e:)
    bpb->marker     = 0x29;
  }

#else


#endif

  return 0;
}

#pragma aux init     "_*"
#pragma aux real_test "*"
#pragma aux call_rm   "*"
#pragma aux printmsg  "*"
#pragma aux printb    "*"
#pragma aux printw    "*"
#pragma aux printd    "*"
