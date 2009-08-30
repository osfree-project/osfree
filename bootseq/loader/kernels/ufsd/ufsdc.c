/*
 *
 */

#include <shared.h>
#include <loader.h>
#include <bpb.h>

#include "fsd.h"
#include "serial.h"

int serial_init (long port, long speed,
                int word_len, int parity, int stop_bit_len);

int assign_drvletter (char *mode);

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

extern char default_part_types[];
extern char part_types[];

int filemax;
int filepos;
int fileaddr;

unsigned long cdrom_drive;
char drvletter;
char mode[12];

extern char debug;

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

int toupper (int c);

int kprintf(const char *format, ...);
void comout(unsigned short port, unsigned char c);

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
  int  i;

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
      //for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x1781 + i)));
      //kprintf("\n");
      return 0;
    }

    // filename found
    filepos  = 0;
    filemax  = mod->mod_end - mod->mod_start;
    fileaddr = mod->mod_start;

    kprintf("1\n");
    //for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x1781 + i)));
    //kprintf("\n");
    return 1;
  }

  kprintf("0\n");
  //for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x1781 + i)));
  //kprintf("\n");
  return 0;
}

int
ufs_read (char *buf, int len)
{
  int i;
  kprintf("**** ufs_read(0x%08x, %ld) = ", buf, len);

  if (fileaddr && buf && len)
  {
    if (len == -1) len = filemax;

    memmove(buf, (char *)fileaddr + filepos, len);

    kprintf("%lu\n", len);
    //for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x1781 + i)));
    //kprintf("\n");
    return len;
  }

  kprintf("0\n");
  //for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x1781 + i)));
  //kprintf("\n");

  return 0;
}

int
ufs_seek (int offset)
{
  int i;
  kprintf("**** ufs_seek(\"%ld\")\n", offset);

  if (offset > filemax || offset < 0)
    return -1;

  filepos = offset;
  //for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x1781 + i)));
  //kprintf("\n");

  return offset;
}

void
ufs_close (void)
{
  int i;
  kprintf("**** ufs_close()\n");

  //for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x1781 + i)));
  //kprintf("\n");
}


void cmain (void)
{
  int  ldrbase, mfsbase = 0;
  int  ldrlen, mfslen = 0;
  char *buf;
  char type[8];
  unsigned long *p;
  unsigned long q;
  struct geometry geom;
  bios_parameters_block *bpb;
  long port = 0x3f8, t;
  long speed = 9600;
  char *pp, *r;
  int  i;

  if (m->flags & MB_INFO_CMDLINE)
  {
    if (pp = strstr((char *)m->cmdline, "--debug"))
    {
      debug = 1;
    }

    // if "--serial=..." specified on the command line
    if (pp = strstr((char *)m->cmdline, "--port"))
    {
      pp = skip_to(1, pp);
      safe_parse_maxint(&pp, &port);
    }

    if (pp = strstr((char *)m->cmdline, "--speed"))
    {
      pp = skip_to(1, pp);
      safe_parse_maxint(&pp, &speed);
    }

    if (pp = strstr((char *)m->cmdline, "--pt"))
    {
      pp = skip_to(1, pp);
      // copy default partition types
      for (r = default_part_types, i = 0; *r; r++, i++) part_types[i] = *r;
      // copy partition types specified on the command line
      while (i <= 0x20)
      {
        // find part type end
        for (r = pp; *r && *r != ' ' && *r != ','; r++) ;
        //memmove(type, pp, r - pp);
        //type[r - pp] = '\0';
        safe_parse_maxint(&pp, &t);
        part_types[i++] = t;
        if (!*r || *r == ' ') break;
        pp = r + 1;
      }
      part_types[i] = '\0';
    }

    memset(mode, 0, sizeof(mode));
    if (pp = strstr((char *)m->cmdline, "--drv"))
    {
      pp = skip_to(1, pp);
      // find name end
      for (r = pp; *r && *r != ' '; r++) ;
      memmove(mode, pp, r - pp);
      mode[r - pp] = '\0';
      // make name uppercase
      r = mode;
      while (*r) *r++ = toupper(*r);
    }
  }

  // init serial port
  serial_init(port, speed, UART_8BITS_WORD, UART_NO_PARITY, UART_1_STOP_BIT);

  kprintf("**** Hello MBI microfsd!\n");
  kprintf("comport = 0x%x\n", port);

  kprintf("part_types=");
  for (r = part_types; *r; r++) kprintf("0x%02x,", *r);
  kprintf("\n");

  // set a drive letter according the DLAT info or AUTO algorithm
  drvletter = assign_drvletter(mode);

  // correct the command line according the drive letter got
  pp[0] = (char)drvletter;
  for (i = 1; i < grub_strlen(mode); i++) pp[i] = ' '; // pad with spaces

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
    ufs_read(buf, 512);
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

  //boot_drive = 0x80;

  /* set filetable */
  ft.ft_cfiles = 4;
  ft.ft_ldrseg = ldrbase >> 4;
  ft.ft_ldrlen = ldrlen;

  ft.ft_museg  = (REL1_BASE - 0x200 - 0x2000) >> 4;
  ft.ft_mulen  = (unsigned long)&stack_bottom - REL1_BASE + 0x200 + 0x2000;

  ft.ft_mfsseg = 0x7c0 >> 4;
  ft.ft_mfslen = *p;

  // where to place mbi pointer
  q = (0x7c0 + *p + 0xf) & 0xfffffff0;

  ft.ft_ripseg = q >> 4;
  ft.ft_riplen = 4;

  // if alternative os2boot is specified
  if (mfsbase && mfslen) ft.ft_mfslen = mfslen;

  // where to place mbi pointer
  //q = 0x7c0 + *p - 4;

  // pass mbi structure address to mFSD
  // as a variable at its end
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
  bpb->log_drive   = 0x80 + (drvletter - 'C'); // c:;
  //bpb->hidden_secs = 0; //part_start;

  //bpb->disk_num    = 0x3;
  //bpb->log_drive   = 0x48;
  //bpb->marker      = 0x41;
  //bpb->vol_ser_no  = 0x00000082;
  //for (i = 0; i < 0x40; i++) kprintf("0x%02x,", *((char *)(0x7c0 + 0x1781 + i)));
  //for (i = 0; i < 0x2b36; i++) kprintf("0x%02x,", *((char *)(0x7c0 + i)));
  //kprintf("\n");
}
