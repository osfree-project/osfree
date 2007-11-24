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

int  num_fsys = 0;

#ifndef STAGE1_5

char linebuf[512];
char lb[80];
char driveletter = 0x80;

char *preldr_path = "/boot/freeldr/"; // freeldr path
char *fsd_dir     = "fsd/";           // uFSD's subdir
char *cfg_file    = "preldr.ini";     // .INI file

char *fsys_list[FSYS_MAX];
char fsys_stbl[FSYS_MAX*10];

#endif

#pragma aux lip "*"

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
   if (*filename != '/' && *filename != '(') {
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

void stage0_close(void)
{
  if (l->lip_fs_close)
    l->lip_fs_close();
}

int  stage0_embed(int *start_sector, int needed_sectors)
{
  if (l->lip_fs_embed)
    return l->lip_fs_embed(start_sector, needed_sectors);

  return 0;
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

#ifndef STAGE1_5

/*  Returns a next line from a file in memory
 *  and changes current position (*p)
 */
char *getline(char **p)
{
  int  i = 0;
  char *q = *p;

  if (!q)
    panic("getline(): zero pointer: ", "*p");

  while (*q != '\n' && *q != '\r' && *q != '\0' && i < 512)
    linebuf[i++] = *q++;

  if (*q == '\r') q++;
  if (*q == '\n') q++;

  linebuf[i] = '\0';
  *p = q;

  if (!*linebuf)
    return linebuf;

  /* skip comments */
  i = grub_index(';', linebuf);
  if (i) linebuf[i] = '\0';

  /* if empty line, get new line */
  if (!*linebuf)
    return getline(p);

  return linebuf;
}

/* if s is a string of type "var = val" then this
 * function returns var
 */
char *var(char *s)
{
  int i;

  i = grub_index('=', s);
  if (i) grub_strncpy(lb, s, i - 1);
  lb[i - 1] = '\0';

  return lb;
}

/* if s is a string of type "var = val" then this
 * function returns val
 */
char *val(char *s)
{
  int i, l;

  i = grub_index('=', s);
  l = grub_strlen(s) - i;
  if (i) grub_strncpy(lb, s + i, l);
  lb[l] = '\0';

  return lb;
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

/*  Parse .INI file
 *
 */
int parse_cfg(void)
{
  char buff[512];
  int  l, i, size, rc;
  char *cfg, *p, *line, *s, *r;

  l = grub_strlen(preldr_path);
  grub_memmove(buff, preldr_path, l);
  grub_memmove(buff + l, cfg_file, grub_strlen(cfg_file));

  rc = freeldr_open(buff);

  cfg = (char *)(EXT2BUF_BASE);

  if (rc) {
    printmsg("file ");
    printmsg(buff);
    printmsg(" opened, ");
    size = freeldr_read((void *)cfg, -1);
    printmsg("size: ");
    printd(size);
  } else
    return 0;

  /* parse .INI file */
  p = cfg;
  while (*p) {
    line = strip(getline(&p));
    if (!*line) continue;

    if (!grub_strcmp(line, "[global]")) {
      /* [global] section */
      while (*p) {
        line = getline(&p);
        if (!*line) break;

        if (!grub_strcmp(strip(var(line)), "driveletter")) {
          grub_strcpy(buff, strip(val(line)));
          driveletter = *buff; // first letter
          driveletter = grub_tolower(driveletter) - 'a';
          if (driveletter > 1) // not floppy
            driveletter = driveletter - 2 + 0x80;

          continue;
        }
      }
      continue;
    }
    if (!grub_strcmp(line, "[fsd]")) {
      /* [filesys] section */
      while (*p) {
        line = getline(&p);
        if (!*line) break;

        if (!grub_strcmp(var(line), "list")) {
          grub_strcpy(fsys_stbl, strip(val(line)));
          s = fsys_stbl;
          r = fsys_stbl;
          num_fsys = 0;
          while (*s) {
            while (*s && *s != ',') s++;
            *s = '\0';
            fsys_list[num_fsys] = r;
            r = s + 1;
            s = r;
            num_fsys++;
          }
          //for (i = 0; i < num_fsys; i++) {
          //  printmsg("\r\n");
          //  printmsg(fsys_list[i]);
          //}
          //while (1) {;}

          continue;
        }
      }
      continue;
    }
  }

  return 1;
}

void panic(char *msg, char *file)
{
  printmsg("Fatal error: \r\n");
  printmsg(msg);
  printmsg(file);
  while (1) {};
}

#endif

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

  /* Set cdrom drive.   */
  /* Get the geometry.  */
  if (get_diskinfo (boot_drive, &geom)
      || ! (geom.flags & BIOSDISK_FLAG_CDROM))
    cdrom_drive = GRUB_INVALID_DRIVE;
  else
    cdrom_drive = boot_drive;

  /* setting LIP */
  setlip();

  /* move boot drive uFSD to working buffer */
  grub_memmove((void *)(EXT_BUF_BASE), (void *)(UFSD_BASE), EXT_LEN);
  /* call uFSD init (set linkage) */
  fsd_init = (void *)(EXT_BUF_BASE); // uFSD base address
  fsd_init(l);

#ifndef STAGE1_5

  /* parse config file */
  if (!parse_cfg())
    panic("config file doesn't exist!", "");

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

  /* load test file */
  rc = freeldr_open("(fd0)/os2ldr.msg");
  printmsg("freeldr_open(\"(fd0)/os2ldr.msg\") returned: ");
  printd(rc);
  if (rc) {
    rc = freeldr_read((char *)(0x400000), -1);
    printmsg("\r\nfreeldr_read() returned size: ");
    printd(rc);
    printmsg("\r\n");
  }

  __asm {
    cli
    hlt
  }

  /* set boot flags */
  boot_flags = BOOTFLAG_MICROFSD | BOOTFLAG_MINIFSD | BOOTFLAG_RIPL;
  /* set filetable */
  ft.ft_cfiles = 4;
  ft.ft_ldrseg = LDR_SEG;
  ft.ft_ldrlen = ldrlen;
  ft.ft_museg  = BOOTSEC_SEG;
  ft.ft_mulen  = 0xb000;  // It is empirically found maximal value
  ft.ft_mfsseg = MFS_SEG;
  ft.ft_mfslen = mfslen;
  ft.ft_ripseg = 0x800;   //
  ft.ft_riplen = 0x3084;  // max

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
    bpb->log_drive  = driveletter; // 0x92;
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
