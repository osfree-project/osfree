/*
 *
 */

#include <shared.h>
#include <loader.h>
#include <bpb.h>

#include "fsd.h"

extern mu_Open;
extern mu_Read;
extern mu_Close;
extern mu_Terminate;

extern unsigned short boot_flags;
extern unsigned long  boot_drive;
extern FileTable      ft;

extern unsigned long  bss_end;
extern stack_bottom;

/* multiboot structure pointer */
extern struct multiboot_info *m;

int filemax;
int filepos;
int fileaddr;

unsigned long cdrom_drive;

#pragma aux m            "*"
#pragma aux filemax      "*"
#pragma aux filepos      "*"
#pragma aux ufs_open     "*"
#pragma aux ufs_read     "*"
#pragma aux ufs_seek     "*"
#pragma aux ufs_close    "*"
#pragma aux boot_flags   "*"
#pragma aux boot_drive   "*"
#pragma aux ft           "*"
#pragma aux mu_Open      "*"
#pragma aux mu_Read      "*"
#pragma aux mu_Close     "*"
#pragma aux mu_Terminate "*"
#pragma aux stack_bottom "*"

void init (void)
{

}

/*  Strip leading and trailing
 *  spaces
 */
char *strip(char *s)
{
  char *p = s;
  int  i;

  i = grub_strlen(p) - 1;
  while (grub_isspace(p[i])) p[i--] = '\0'; // strip trailing spaces
  while (grub_isspace(*p)) p++;             // strip leading spaces

  return p;
}

int
ufs_open (char *filename)
{
  int  mods_count;
  char *mods_addr;
  struct mod_list *mod;
  char buf1[0x100];
  char buf2[0x100];
  char *p, *q;
  int  n;

  memset(buf1, 0, sizeof(buf1));
  memset(buf2, 0, sizeof(buf2));

  if (m->flags & MB_INFO_MODS) // If there are modules
  {
    mods_count = (int)m->mods_count;
    mods_addr  = (char *)m->mods_addr;
    mod = (struct mod_list *)mods_addr;

    // search for a given filename
    for (n = 0; n < mods_count; n++)
    {
      // copy to buffers
      strcpy(buf1, (char *)mod->cmdline);
      strcpy(buf2, filename);

      // make it uppercase
      for (p = buf1; *p; p++) *p = grub_toupper(*p);
      for (p = buf2; *p; p++) *p = grub_toupper(*p);

      p = strip(buf1); q = strip(buf2);

      if (strstr(p, q))
        break;

      mod++;
    };

    // we have gone through all mods, and no given filename
    if (n == mods_count)
      return 0;

    // filename found
    filepos  = 0;
    filemax  = mod->mod_end - mod->mod_start;
    fileaddr = mod->mod_start;

    return 1;
  }

  return 0;
}

int
ufs_read (char *buf, int len)
{
  if (fileaddr && buf && len)
  {
    if (len == -1) len = filemax;

    memmove(buf, (char *)fileaddr + filepos, len);

    return len;
  }

  return 0;
}

int
ufs_seek (int offset)
{
  if (offset > filemax || offset < 0)
    return -1;

  filepos = offset;
  return offset;
}

void
ufs_close (void)
{
}

void cmain (void)
{
  int  ldrbase;
  int  ldrlen;
  char *buf;
  unsigned long *p;
  struct geometry geom;
  bios_parameters_block *bpb;

  // load os2ldr
  if (ufs_open("OS2LDR"))
  {
    ldrbase = 0x10000;
    buf = (char *)ldrbase;
    ldrlen = ufs_read(buf, -1);
  }

  // read a bootsector. there must be "*bootsec*"
  // string in a command line
  if (ufs_open("*bootsec*"))
  {
    buf = (char *)(REL1_BASE - 0x200);
    ufs_read(buf, -1);
  }

  p = (unsigned long *)(REL1_BASE + 0x20); // an address of mfs_len in the header

  /* set boot flags */
  boot_flags = BOOTFLAG_MICROFSD | BOOTFLAG_MINIFSD;

  if (m->flags & MB_INFO_BOOTDEV)
  {
    boot_drive = m->boot_device >> 24;
  }
  else
  {
    boot_drive = 0xff;
  }

  if (get_diskinfo (boot_drive, &geom)
    || ! (geom.flags & BIOSDISK_FLAG_CDROM))
    cdrom_drive = GRUB_INVALID_DRIVE;
  else
    cdrom_drive = boot_drive;

  /* set filetable */
  ft.ft_cfiles = 3;
  ft.ft_ldrseg = ldrbase >> 4;
  ft.ft_ldrlen = ldrlen;

  ft.ft_museg  = (REL1_BASE - 0x200 - 0x2000) >> 4;

  ft.ft_mulen  = (unsigned long)&stack_bottom - REL1_BASE + 0x200 + 0x2000;
  ft.ft_mfsseg = 0x7c0 >> 4;
  ft.ft_mfslen = *p;
  ft.ft_ripseg = 0;
  ft.ft_riplen = 0;

  p = (unsigned long *)(REL1_BASE + 0x10); // an address of base in the header

  ft.ft_muOpen.seg       = (*p) >> 4;
  ft.ft_muOpen.off       = (unsigned short)(&mu_Open);

  ft.ft_muRead.seg       = (*p) >> 4;
  ft.ft_muRead.off       = (unsigned short)(&mu_Read);

  ft.ft_muClose.seg      = (*p) >> 4;
  ft.ft_muClose.off      = (unsigned short)(&mu_Close);

  ft.ft_muTerminate.seg  = (*p) >> 4;
  ft.ft_muTerminate.off  = (unsigned short)(&mu_Terminate);

  /* set BPB */
  bpb = (bios_parameters_block *)(REL1_BASE - 0x200 + 0xb);

  if (boot_drive == cdrom_drive) { // booting from CDROM drive
    // fill fake BPB
    grub_memset((void *)bpb, 0, sizeof(bios_parameters_block));

    bpb->sect_size  = 0x800;
    bpb->clus_size  = 0x40;
    bpb->n_sect_ext = geom.total_sectors;
    bpb->media_desc = 0xf8;
    bpb->track_size = 0x3f;
    bpb->heads_cnt  = 0xff;
    bpb->marker     = 0x29;
  }

  bpb->disk_num    = (unsigned char)(boot_drive & 0xff);
  bpb->log_drive   = 0; //conf.driveletter; // 0x92;
  bpb->hidden_secs = 0; //part_start;

  //bpb->disk_num    = 0x3;
  //bpb->log_drive   = 0x48;
  //bpb->marker      = 0x41;
  //bpb->vol_ser_no  = 0x00000082;
}
