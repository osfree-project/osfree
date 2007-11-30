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

int i;
int  num_fsys = 0;

#ifndef STAGE1_5

char linebuf[512];
char lb[80];

char *fsys_list[FSYS_MAX];
char fsys_stbl[FSYS_MAX*10];

/* Configuration got
 from .INI file      */
struct {
  char driveletter;
  char multiboot;
  struct {
    char **fsys_list;
  } mufsd;
  struct {
    char filename[128];
    int base;
  } loader;
  struct {
    char filename[128];
    int base;
  } mini;
} conf = {0x80, 0, fsys_list, {"/os2ldr", 0x10000},
          {"/os2boot", 0x7c0}};

char *preldr_path = "/boot/freeldr/"; // freeldr path
char *fsd_dir     = "fsd/";           // uFSD's subdir
char *cfg_file    = "preldr.ini";     // .INI file

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

   printmsg("open(");
   printmsg(filename);
   printmsg(")\r\n");

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
  char buf[512];
  int  l, i, size, rc;
  char *cfg, *p, *line, *s, *r;

  l = grub_strlen(preldr_path);
  grub_memmove(buf, preldr_path, l);
  grub_memmove(buf + l, cfg_file, grub_strlen(cfg_file));
  i = grub_strlen(cfg_file);
  buf[l + i] = '\0';

  rc = freeldr_open(buf);

  cfg = (char *)(EXT2BUF_BASE);

  if (rc) {
    printmsg("file ");
    printmsg(buf);
    printmsg(" opened, ");
    size = freeldr_read((void *)cfg, -1);
    printmsg("size: ");
    printd(size);
    printmsg("\r\n");
  } else
    return 0;

  /* parse .INI file */
  p = cfg;
  while (*p) {
    line = strip(getline(&p));
    if (!*line) continue;

    if (!grub_strcmp(strip(line), "[global]")) {
      /* [global] section */
      while (*p) {
        line = getline(&p);
        if (!*line) break;

        if (!grub_strcmp(strip(var(line)), "driveletter")) {
          grub_strcpy(buf, strip(val(line)));
          conf.driveletter = *buf; // first letter
          conf.driveletter = grub_tolower(conf.driveletter) - 'a';
          if (conf.driveletter > 1) // not floppy
            conf.driveletter = conf.driveletter - 2 + 0x80;

          continue;
        }

        if (!grub_strcmp(strip(var(line)), "multiboot")) {
          if (!grub_strcmp(strip(val(line)), "yes")) {
            conf.multiboot = 1;
          } else {
            conf.multiboot = 0;
          }

          continue;
        }
      }
      continue;
    }
    if (!grub_strcmp(strip(line), "[microfsd]")) {
      /* [fsd] section */
      while (*p) {
        line = getline(&p);
        if (!*line) break;

        if (!grub_strcmp(strip(var(line)), "list")) {
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

          continue;
        }
      }
      continue;
    }
    if (!grub_strcmp(strip(line), "[loader]")) {
      /* [loader] section */
      while (*p) {
        line = getline(&p);
        if (!*line) break;

        if (!grub_strcmp(strip(var(line)), "filename")) {
          grub_strcpy(conf.loader.filename, strip(val(line)));

          continue;
        }
        if (!grub_strcmp(strip(var(line)), "base")) {
          conf.loader.base = grub_aton(strip(val(line)));

          continue;
        }
      }
      continue;
    }
    if (!grub_strcmp(strip(line), "[minifsd]")) {
      /* [loader] section */
      while (*p) {
        line = getline(&p);
        if (!*line) break;

        if (!grub_strcmp(strip(var(line)), "filename")) {
          grub_strcpy(conf.mini.filename, strip(val(line)));
          if (!grub_strcmp(conf.mini.filename, "none"))
            *(conf.mini.filename) = '\0';

          continue;
        }
        if (!grub_strcmp(strip(var(line)), "base")) {
          conf.mini.base = grub_aton(strip(val(line)));

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
  printmsg("\r\nFatal error: \r\n");
  printmsg(msg);
  printmsg(file);

  __asm {
    cli
    hlt
  }
}

#endif

int init(void)
{
  int rc, files;
  char *buf;
  char *fn;
  unsigned long ldrlen = 0, mfslen = 0;
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
    panic("config file doesn't exist!\r\n", "preldr.ini");

  /* Show config params */
  printmsg("\r\nConfig parameters:");
  printmsg("\r\ndriveletter = "); printb(conf.driveletter);
  printmsg("\r\nmultiboot = ");
  printb(conf.multiboot);
  printmsg("\r\nFilesys: ");
  for (i = 0; i < FSYS_MAX && conf.mufsd.fsys_list[i]; i++) {
    printmsg(conf.mufsd.fsys_list[i]);
    printmsg(", ");
  }
  printmsg("\r\nloader.filename = "); printmsg(conf.loader.filename);
  printmsg("\r\nloader.base = "); printd(conf.loader.base);
  printmsg("\r\nmini.filename = "); printmsg(conf.mini.filename);
  printmsg("\r\nmini.base = "); printd(conf.mini.base);
  printmsg("\r\n\r\n");

  /* load os2ldr */
  fn = conf.loader.filename;
  rc = freeldr_open(fn);
  printmsg("freeldr_open(\"");
  printmsg(fn);
  printmsg("\") returned: ");
  printd(rc);

  buf = (char *)(conf.loader.base);

  if (rc) {
    ldrlen = freeldr_read(buf, -1);

    printmsg("\r\nfreeldr_read() returned size: ");
    printd(ldrlen);
    printmsg("\r\n");
  } else {
    panic("Can't open loader file: ", conf.loader.filename);
  }

  /* load minifsd */
  fn = conf.mini.filename;
  if (*fn) { // is minifsd needed?
    rc = freeldr_open(fn);
    printmsg("freeldr_open(\"");
    printmsg(fn);
    printmsg("\") returned: ");
    printd(rc);

    buf = (char *)(conf.mini.base);

    if (rc) {
      mfslen = freeldr_read(buf, -1);

      printmsg("\r\nfreeldr_read() returned size: ");
      printd(mfslen);
      printmsg("\r\n");
    } else {
      panic("Can't open minifsd filename: ", fn);
    }
  }

  /* load test file */
  /*
  rc = freeldr_open("(fd0)/os2ldr.msg");
  printmsg("freeldr_open(\"(fd0)/os2ldr.msg\") returned: ");
  printd(rc);
  if (rc) {
    buf = (char *)(EXT2BUF_BASE);
    rc = freeldr_read(buf, -1);
    printmsg("\r\nfreeldr_read() returned size: ");
    printd(rc);
    printmsg("\r\n");
  }                      */

  //__asm {
  //  cli
  //  hlt
  //}

  /* set boot flags */
  boot_flags = BOOTFLAG_MICROFSD;
  /* set filetable */
  files = 2;
  if (*fn) { // if minifsd present
    files++;
    boot_flags |=  BOOTFLAG_MINIFSD;
  }

  ft.ft_cfiles = files;
  ft.ft_ldrseg = conf.loader.base >> 4;
  ft.ft_ldrlen = ldrlen;  // 0x3800;
  ft.ft_museg  = BOOTSEC_SEG; // 0x8600; -- OS/2 2.1 // 0x8400; -- Merlin & Warp3 // 0x8100; -- Aurora
  ft.ft_mulen  = 0xb000;      // 0x10000;            // It is empirically found maximal value
  ft.ft_mfsseg = conf.mini.base >> 4; // 0x7f;
  ft.ft_mfslen = mfslen;  // 0x95f0;
  ft.ft_ripseg = 0; // 0x800;   // end of mfs
  ft.ft_riplen = 0; // 62*1024 - mfslen; //0x3084;  // max == 62k - mfslen

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
    bpb->log_drive  = conf.driveletter; // 0x92;
    bpb->marker     = 0x29;
  }

  if (conf.multiboot) {
    /* new loader interface */
    unsigned long ldr_base = conf.loader.base;

    __asm {
      mov  dx, boot_flags
      mov  dl, byte ptr boot_drive

      // edi == pointer to filetable
      lea  edi, ft

      // esi ==  pointer to BPB
      mov  esi, BOOTSEC_BASE
      add  esi, 0xb

      // magic in eax
      mov  eax, BOOT_MAGIC
      // ebx == pointer to LIP
      lea  ebx, lip

      push ldr_base
      retn
    }
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
