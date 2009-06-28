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

int kprintf(const char *format, ...);

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

  kprintf("**** ufs_open(\"%s\") = ", filename);

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
    {
      kprintf("0\n");
      return 0;
    }

    // filename found
    filepos  = 0;
    filemax  = mod->mod_end - mod->mod_start;
    fileaddr = mod->mod_start;

    kprintf("1\n");
    return 1;
  }

  kprintf("0\n");
  return 0;
}

int
ufs_read (char *buf, int len)
{
  kprintf("**** ufs_read(\"0x%08x, %ld\") = ", buf, len);

  if (fileaddr && buf && len)
  {
    if (len == -1) len = filemax;

    memmove(buf, (char *)fileaddr + filepos, len);

    kprintf("%lu\n", len);
    return len;
  }

  kprintf("0\n");
  return 0;
}

int
ufs_seek (int offset)
{
  kprintf("**** ufs_seek(\"%ld\")\n", offset);

  if (offset > filemax || offset < 0)
    return -1;

  filepos = offset;
  return offset;
}

void
ufs_close (void)
{
  kprintf("**** ufs_close()\n");
}

/*  Relocate mbi structure to
 *  the safe place
 */
void mbi_reloc(void)
{
  int                   i;
  unsigned long         size;
  struct mod_list       *mod;
  struct multiboot_info *mbi_new;
  struct mod_list       *mod_new;
  unsigned long         cur_addr;
  char                  *p;

  mod = (struct mod_list *)m->mods_addr;

  // find the address of modules end

  // pointer to the last module in the list
  mod += m->mods_count - 1;

  // last module end
  p = (char *)mod->mod_end;

  // skip a string after a module (cmdline, if one)
  while (*p++) ;

  cur_addr = (((unsigned long)(p + 0xfff)) & 0xfffff000);
  size = sizeof(struct multiboot_info);
  memmove((char *)cur_addr, m, size);
  m = (struct multiboot_info *)cur_addr;

  // relocate a kernel command line
  cur_addr = (((unsigned long)(cur_addr + size)) + 0xfff) & 0xfffff000;
  size = strlen((char *)m->cmdline) + 1;
  memmove((char *)cur_addr, (char *)m->cmdline, size);
  m->cmdline = cur_addr;

  // relocate mods after mbi
  if (m->flags & MB_INFO_MODS)
  {
    cur_addr = (((unsigned long)(cur_addr + size)) + 0xfff) & 0xfffff000;
    size = sizeof(struct mod_list) * m->mods_count;
    memmove((char *)cur_addr, (char *)m->mods_addr, size);
    m->mods_addr = cur_addr;

    // relocate mods command lines
    cur_addr = (((unsigned long)((char *)cur_addr + size)) + 0xfff) & 0xfffff000;
    for (i = 0, mod = (struct mod_list *)m->mods_addr; i < m->mods_count; i++, cur_addr += size, mod++)
    {
      size = strlen((char *)mod->cmdline) + 1;
      memmove((char *)cur_addr, (char *)mod->cmdline, size);
      mod->cmdline = cur_addr;
    }
  }

  // relocate memmap
  if (m->flags & MB_INFO_MEM_MAP)
  {
    cur_addr = (cur_addr + 0xfff) & 0xfffff000;
    size = m->mmap_length;
    memmove((char *)cur_addr, (char *)m->mmap_addr, size);
    m->mmap_addr = cur_addr;
  }

  // relocate drives info
  if (m->flags & MB_INFO_DRIVE_INFO)
  {
    cur_addr = (((unsigned long)((char *)cur_addr + size)) + 0xfff) & 0xfffff000;
    size = m->drives_length;
    memmove((char *)cur_addr, (char *)m->drives_addr, size);
    m->drives_addr = cur_addr;
  }

  // relocate the boot loader name
  if (m->flags & MB_INFO_BOOT_LOADER_NAME)
  {
    p = (char *)m->boot_loader_name;
    cur_addr = (((unsigned long)((char *)cur_addr + size)) + 0xfff) & 0xfffff000;
    size = strlen(p);
    strcpy((char *)cur_addr, p);
    m->boot_loader_name = cur_addr;
  }
}

void cmain (void)
{
  int  ldrbase, mfsbase = 0;
  int  ldrlen, mfslen = 0;
  char *buf;
  unsigned long *p;
  unsigned long q;
  struct geometry geom;
  bios_parameters_block *bpb;

  kprintf("**** Hello MBI microfsd!\n");

  // relocate mbi info after all modules
  kprintf("Relocating MBI info...\n");
  mbi_reloc();

  // load os2ldr
  if (ufs_open("OS2LDR"))
  {
    ldrbase = 0x10000;
    buf = (char *)ldrbase;
    ldrlen = ufs_read(buf, -1);
  }

  // load os2boot (if it exists)
  if (ufs_open("OS2BOOT"))
  {
    mfsbase = 0x7c0;
    buf = (char *)mfsbase;
    mfslen = ufs_read(buf, -1);
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
  boot_flags = BOOTFLAG_MICROFSD | BOOTFLAG_MINIFSD | BOOTFLAG_NOVOLIO | BOOTFLAG_RIPL;

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
  ft.ft_cfiles = 4;
  ft.ft_ldrseg = ldrbase >> 4;
  ft.ft_ldrlen = ldrlen;

  ft.ft_museg  = (REL1_BASE - 0x200 - 0x2000) >> 4;
  ft.ft_mulen  = (unsigned long)&stack_bottom - REL1_BASE + 0x200 + 0x2000;

  ft.ft_mfsseg = 0x7c0 >> 4;
  ft.ft_mfslen = *p;

  // if alternative os2boot is specified
  if (mfsbase && mfslen) ft.ft_mfslen = mfslen;

  // where to place RIPL data
  q = (0x7c0 + *p + 0xf) & 0xfffffff0;

  ft.ft_ripseg = q >> 4;
  ft.ft_riplen = 4;

  // pass mbi structure address to mFSD
  // as RIPL data
  *((unsigned long *)q) = (unsigned long) m;

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
