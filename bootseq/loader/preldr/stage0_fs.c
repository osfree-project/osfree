/*
 *
 *
 */

#include <lip.h>
#include <types.h>
#include <loader.h>
#include <bpb.h>

#include <shared.h>

#include "term.h"
#include "fsys.h"
#include "fsd.h"
#include "struc.h"

#ifndef STAGE1_5
extern unsigned char use_term;
#endif

extern unsigned long extended_memory;
#pragma aux extended_memory "*"

void *filetab_ptr = 0;

#pragma aux mu_Open_wr      "*"
#pragma aux mu_Read_wr      "*"
#pragma aux mu_Close_wr     "*"
#pragma aux mu_Terminate_wr "*"

unsigned short __cdecl
  mu_Open_wr(char *pName,
             unsigned long *pulFileSize);

unsigned long  __cdecl
  mu_Read_wr(long loffseek,
             char *pBuf,
             unsigned long cbBuf);

void __cdecl
  mu_Close_wr(void);

void __cdecl
  mu_Terminate_wr(void);

char test = 0;

struct multiboot_info mbi;
unsigned long linux_text_len;
char *linux_data_real_addr;
char *linux_data_tmp_addr;

unsigned int relshift;

#pragma aux mbi "*"
#pragma aux extended_memory "*"
#pragma aux linux_data_real_addr "*"
#pragma aux linux_data_tmp_addr "*"
#pragma aux linux_text_len "*"

#pragma aux install_filesys "*"

#pragma aux get_vbe_controller_info "*"
#pragma aux get_vbe_mode_info       "*"
#pragma aux set_vbe_mode            "*"
#pragma aux reset_vbe_mode          "*"
#pragma aux get_vbe_pmif            "*"

#pragma aux gateA20 "*"

void gateA20(int);

lip1_t *l1, lip1;
lip2_t *l2, lip2;

extern int __cdecl (*fsd_init)(lip1_t *l);
int (*process_cfg_line)(char *line);

int i;
int  num_fsys = 0;
int  num_term = 0;

#ifndef STAGE1_5

#pragma aux  jmp_reloc    "*"
#pragma aux  high_stack   "*"
#pragma aux  switch_stack_flag  "*"

void jmp_reloc(unsigned long addr);
void high_stack(void);
void low_stack(void);
extern unsigned char switch_stack_flag;

#pragma aux  preldr_ds "*"
#pragma aux  preldr_es "*"
#pragma aux  preldr_ss_sp "*"

extern unsigned short preldr_ds;
extern unsigned short preldr_es;
extern unsigned long  preldr_ss_sp;

/* GDT */
#pragma aux gdt     "*"
#pragma aux gdtdesc "*"
extern struct desc gdt[5];
extern struct gdtr gdtdesc;

#pragma aux set_fsys    "*"
#pragma aux fsys_by_num "*"
int  set_fsys(char *fsname);
void fsys_by_num(int n, char *buf);

char lb[80];

// string table size;
#define STRTBL_LEN 0x800
/* String table */
char strtbl[STRTBL_LEN];
/* End position in this table */
char *strpos = strtbl;

char *fsys_list[FSYS_MAX];
char *term_list[FSYS_MAX];

/* a structure with term blackbox entry points */
struct term_entry trm, *t = 0;

// max count of file aliases
#define MAX_ALIAS 0x10

/* Configuration got
   from .INI file      */
_Packed struct {
  char driveletter;
  char multiboot;
  struct {
    char ignorecase;
    char **fsys_list;
  } mufsd;
  struct {
    char name[0x18];
    int  base;
    // char multiboot;
  } loader;
  struct {
    char name[0x18];
    int  base;
  } mini;
  struct {
    int  _default;
    char **term_list;
  } term;
  struct {
    char *name;
    char *alias;
  } alias[MAX_ALIAS];
} conf = {0x80, 2, {0, fsys_list}, {"/os2ldr", 0x10000},
          {"/os2boot", 0x7c0}, {0, term_list},};

char *preldr_path = "/boot/loader/"; // freeldr path
char *fsd_dir     = "fsd/";           // uFSD's subdir
char *term_dir    = "term/";          // term   subdir
char *cfg_file    = "preldr.ini";     // .INI file

#endif

#pragma aux lip1 "*"
#pragma aux lip2 "*"

extern mu_Open;
extern mu_Read;
extern mu_Close;
extern mu_Terminate;

extern unsigned long saved_drive;
extern unsigned long saved_partition;
extern unsigned long saved_slice;
extern unsigned long cdrom_drive;

extern unsigned long current_drive;
extern unsigned long current_partition;
extern int           current_slice;
extern unsigned long part_start;
extern unsigned long part_length;
extern int           fsmax;

extern int buf_drive;
extern int buf_track;

extern FileTable ft;

extern unsigned short boot_flags;
extern unsigned long  boot_drive;
extern unsigned long  install_partition;
extern char install_filesys[0x10];

extern int mem_lower;
extern int mem_upper;
extern int fsmax;

void __cdecl real_test(void);
void __cdecl call_rm(fp_t);

//void trap_3(void);
//#pragma aux trap_3 "*"

void idt_init(void);
#pragma aux idt_init "*"

#ifndef STAGE1_5

#pragma aux stop_floppy "*"
void stop_floppy(void);

/*   u_* functions are designed to be more like
 *   original IBM's microfsd functions, than GRUB
 *   functions and do not expose filepos
 *   and filemax variables to pre-loader
 *   clients, their values can be determined
 *   from these functions' return values
 */

/*   change/query current drive. all drives are
 *   specified in GRUB's notation, like (hdA,B)
 *   if successful, it returns 0, otherwise
 *   returns errnum.
 */

/*  open a file and return its size, return
 *  zero if no error, 1 otherwize
 */
unsigned int __cdecl
u_open (char *name, unsigned int *size)
{
  int rc;

  rc = freeldr_open(name);
  *size = 0;

  if (rc) {
    *size = filemax;
    return 0;
  }

  return 1;
}

/*  read count bytes to buf buffer and return
 *  the number of bytes actually read
 */
unsigned int __cdecl
u_read (char *buf, unsigned int count)
{
  return freeldr_read(buf, count);
}

/*  set or query current file position. If loffseek
 *  is equal to -1, the current offset is returned,
 *  if it's not -1, the specified offset is set
 */
unsigned int __cdecl
u_seek (int loffseek)
{
  if (loffseek == -1)
    return filepos;

  return freeldr_seek(loffseek);
}

/*  close current file
 */
void __cdecl
u_close (void)
{
  freeldr_close();
}

void __cdecl
u_terminate (void)
{
}

/*  32-bit disk low-level function.
 *  Read/write NSEC sectors starting from SECTOR in DRIVE disk with GEOMETRY
 *  from/into SEGMENT segment. If READ is BIOSDISK_READ, then read it,
 *  else if READ is BIOSDISK_WRITE, then write it. If an geometry error
 *  occurs, return BIOSDISK_ERROR_GEOMETRY, and if other error occurs, then
 *  return the error number. Otherwise, return 0.
 */
int __cdecl
u_diskctl (int func, int drive, struct geometry *geometry,
          int sector, int nsec, int addr)
{
  if (addr & 0xf != 0)      // check if addr is paragraph-aligned
    return ERR_UNALIGNED;

  if (func == BIOSDISK_READ || func == BIOSDISK_WRITE)
    return biosdisk(func, drive, geometry,
                    sector, nsec, addr >> 4);

  if (func == BIOSDISK_GEO)
    return get_diskinfo(drive, geometry);

  if (func == BIOSDISK_STOP_FLOPPY)
    stop_floppy();

  return 0;
}

/*
 *  Boot different types of
 *  operating systems.
 */

int __cdecl
u_boot (int type)
{
  if (type == KERNEL_TYPE_MULTIBOOT)
     return 0;

  if (type == KERNEL_TYPE_FREEBSD)
     return 0;

  if (type == KERNEL_TYPE_NETBSD)
     return 0;

  if (type == KERNEL_TYPE_LINUX)
     return 0;

  if (type == KERNEL_TYPE_BIG_LINUX)
     return 0;

  return 0;
}

/*   Load and relocate executable file
 */
int __cdecl
u_load (char *image, unsigned int size,
         char *load_addr, struct exe_params *p)
{
  return errnum;
}

/*   Changes/queries a parameter value.
 *   if action == ACT_GET, it returns value in 'val' variable;
 *   if action == ACT_SET, it changes its value to the value of 'val'.
 */

int __cdecl
u_parm (int parm, int action, unsigned int *val)
{
  switch (parm)
  {
    case PARM_BOOT_DRIVE:
      {
        if (action == ACT_GET)
          *val = boot_drive;
        else
          boot_drive = *val;

        return 0;
      };
    case PARM_CDROM_DRIVE:
      {
        if (action == ACT_GET)
          *val = cdrom_drive;
        else
          cdrom_drive = *val;

        return 0;
      };
    case PARM_CURRENT_DRIVE:
      {
        if (action == ACT_GET)
          *val = current_drive;
        else
          current_drive = *val;

        return 0;
      };
    case PARM_CURRENT_PARTITION:
      {
        if (action == ACT_GET)
          *val = current_partition;
        else
          current_partition = *val;

        return 0;
      };
    case PARM_CURRENT_SLICE:
      {
        if (action == ACT_GET)
          *val = current_slice;
        else
          current_slice = *val;

        return 0;
      };
    case PARM_SAVED_DRIVE:
      {
        if (action == ACT_GET)
          *val = saved_drive;
        else
          saved_drive = *val;

        return 0;
      };
    case PARM_SAVED_PARTITION:
      {
        if (action == ACT_GET)
          *val = saved_partition;
        else
          saved_partition = *val;

        return 0;
      };
    case PARM_SAVED_SLICE:
      {
        if (action == ACT_GET)
          *val = saved_slice;
        else
          saved_slice = *val;

        return 0;
      };
    case PARM_MBI:
      {
        if (action == ACT_GET)
        {
          *val = (unsigned int)(&mbi);
          return 0;
        }
        else
          return -1;
      };
    case PARM_ERRNUM:
      {
        if (action == ACT_GET)
          *val = errnum;
        else
          errnum = *val;

        return 0;
      };
    case PARM_FILEPOS:
      {
        if (action == ACT_GET)
          *val = filepos;
        else
          filepos = *val;

        return 0;
      };
    case PARM_FILEMAX:
      {
        if (action == ACT_GET)
          *val = filemax;
        else
          filemax = *val;

        return 0;
      };
    case PARM_EXTENDED_MEMORY:
      {
        if (action == ACT_GET)
          *val = extended_memory;
        else
          extended_memory = *val;

        return 0;
      };
/*    case PARM_LINUX_TEXT_LEN:
      {
        if (action == ACT_GET)
          *val = linux_text_len;
        else
          linux_text_len = *val;

        return 0;
      };
    case PARM_LINUX_DATA_REAL_ADDR:
      {
        if (action == ACT_GET)
          *val = (unsigned int)linux_data_real_addr;
        else
          linux_data_real_addr = (char *)*val;

        return 0;
      };
    case PARM_LINUX_DATA_TMP_ADDR:
      {
        if (action == ACT_GET)
          *val = (unsigned int)linux_data_tmp_addr;
        else
          linux_data_tmp_addr = (char *)*val;

        return 0;
      }; */
    default:
      ;
  }

  return -1;
}

void __cdecl
u_msg (char *s)
{
  char buf[0x100];

  grub_strncpy(buf, s, sizeof(buf));

  printmsg(buf);
}

void __cdecl
u_setlip (lip2_t *l)
{
  setlip2(l);
}

int __cdecl
u_vbectl(int func, int mode_number, void *info)
{
  struct pmif *pmif = (struct pmif *)info;

  switch (func)
  {
    case VBE_FUNC_RESET:
    {
      reset_vbe_mode ();
      return 0;
    }
    case VBE_FUNC_GET_CTRLR_INFO:
    {
      return get_vbe_controller_info ((struct vbe_controller *)info);
    }
    case VBE_FUNC_GET_MODE_INFO:
    {
      return get_vbe_mode_info (mode_number, (struct vbe_mode *)info);
    }
    case VBE_FUNC_SET_MODE:
    {
      return set_vbe_mode (mode_number);
    }
    case VBE_FUNC_GET_PMIF:
    {
      get_vbe_pmif(pmif->pmif_segoff, pmif->pmif_len);
      return 0;
    }
    default:
    {
    }
  }
  return 1;
}

/*  Get (if termno == -1) or set (otherwise)
 *  a pointer to term_entry structure, describing
 *  the loaded terminal
 */
struct term_entry * __cdecl
u_termctl(int termno)
{
  int i, n;
  char term[0x100];

  if (termno == -1)
    return &trm;
  else
    n = termno;

  /* build a path to term blackbox */
  i = grub_strlen(preldr_path);
  grub_strcpy(term, preldr_path);
  grub_strcpy(term + i, term_dir);
  i = grub_strlen(term);
  grub_strcpy(term + i, conf.term.term_list[n]);
  i = grub_strlen(term);
  grub_strcpy(term + i, ".mdl\0");

  /* terminal name */
  trm.name = conf.term.term_list[n];

  test++;

  //printf("terminal: %s\r\n", term);

  if (blackbox_load(term, 2, &trm))
  {
    printf("terminal loaded\r\n");
  }
  else
    return 0;

  return &trm;
}

/*
int open2 (char *filename)
{
  char buf[0x100];
  int  rc;

  for (fsys_type = 0; fsys_type < num_fsys; fsys_type++)
  {
    fsys_by_num(fsys_type, buf);
    set_fsys(buf);
    rc = grub_open(filename);

    if (errnum == ERR_NONE)
      return rc;
  }

  return 1;
}
*/

#endif

int
freeldr_open (char *filename)
{

#ifndef STAGE1_5

   char *p;
   int  i;
   int  i0 = 0;
   int  rc;
   unsigned short ret;
   char buf[0x100];

   printf("o %s", filename);
   //u_msg("o ");
   //u_msg(filename);
   //u_msg("\r\n");

   if (filetab_ptr)
   {
     // we're using 16-bit uFSD
     ret = mu_Open_wr(filename,
                      (unsigned long *)&rc);

     if (!ret)
     {
       // success
       return rc;
     }
     else
     {
       // failure
       return 0;
     }
   }
   else
   {
     // we're using 32-bit uFSD

     /* prepend "/" to filename */
     if (*filename != '/' && *filename != '(') {
       buf[0] = '/';
       i0 = 1;
     }

     grub_strcpy(buf + i0, filename);

     for (i = 0; i < 128 && buf[i]; i++) {
       /* change "\" to "/" */
       if (buf[i] == '\\')
         buf[i] = '/';
     }

     /* try open filename as is */
     rc = grub_open(buf);

     if (conf.mufsd.ignorecase) {
       /* if ignore case */
       if (!rc) {
         /* try to open uppercase filename */
         /* skip device name like "(cd)" */
         i = 0;
         if (*buf == '(')
           while (buf[i] && buf[i] != ')') i++;

         for (; i < 128 && buf[i]; i++) {
           buf[i] = grub_toupper(buf[i]);
         }

         rc = grub_open(buf);
       }

       if (!rc) {
         /* try to open lowercase filename */
         for (i = 0; i < 128 && buf[i]; i++)
           buf[i] = grub_tolower(buf[i]);

         rc = grub_open(buf);
       }
     }

     if (!rc)
       printf(" fail!");

     printf("\r\n");

     return rc;
   }
#else
   return grub_open(filename);
#endif
}

int
freeldr_read (char *buf, int len)
{
   int rc;
#ifndef STAGE1_5
   printf("r 0x%x %d", buf, len);
#endif
   if (filetab_ptr)
   {
     // use 16-bit uFSD
     rc = mu_Read_wr(filepos, buf, len);
   }
   else
   {
     // use 32-bit uFSD
     rc = grub_read(buf, len);
   }
#ifndef STAGE1_5
   printf(" sz %d\r\n", rc);
#endif
   return rc;
}

int
freeldr_seek (int offset)
{
   return grub_seek(offset);
}

void
freeldr_close (void)
{
   if (filetab_ptr)
   {
     // use 16-bit uFSD
     mu_Close_wr();
   }
   else
   {
     // use 32-bit uFSD
     grub_close();
   }
}

int  stage0_mount (void)
{
  return l1->lip_fs_mount();
}

int  stage0_read (char *buf, int len)
{
  return l1->lip_fs_read(buf, len);
}

int  stage0_dir (char *dirname)
{
  return l1->lip_fs_dir(dirname);
}

void stage0_close(void)
{
  if (l1->lip_fs_close)
    l1->lip_fs_close();
}

int  stage0_embed(int *start_sector, int needed_sectors)
{
  if (l1->lip_fs_embed)
    return l1->lip_fs_embed(start_sector, needed_sectors);

  return 0;
}

#ifndef STAGE1_5

void setlip2(lip2_t *l2)
{
  l2->u_lip2magic       = LIP2_MAGIC;
  l2->u_open            = &u_open;
  l2->u_read            = &u_read;
  l2->u_seek            = &u_seek;
  l2->u_close           = &u_close;
  l2->u_terminate       = &u_terminate;
  l2->u_load            = &u_load;
  l2->u_boot            = &u_boot;
  l2->u_parm            = &u_parm;
  l2->u_diskctl         = &u_diskctl;
  l2->u_vbectl          = &u_vbectl;
  l2->u_termctl         = &u_termctl;
  l2->u_msg             = &u_msg;
  l2->u_setlip          = &u_setlip;
}

#endif

void setlip1(lip1_t *l1)
{
  l1->lip_open  = &freeldr_open;
  l1->lip_read  = &freeldr_read;
  l1->lip_seek  = &freeldr_seek;
  l1->lip_close = &freeldr_close;
  l1->lip_term  = 0;

  //l->lip_memcheck = 0; //&grub_memcheck;
  l1->lip_memset   = &grub_memset;
  l1->lip_memmove  = &grub_memmove;
  l1->lip_strcpy   = 0; //&grub_strcpy;
  l1->lip_strcmp   = 0; //&grub_strcmp;
  l1->lip_memcmp   = &grub_memcmp;
  l1->lip_strlen   = 0; //&grub_strlen;
  l1->lip_isspace  = &grub_isspace;
  l1->lip_tolower  = &grub_tolower;

  l1->lip_substring = &substring;
  //l1->lip_pos       = 0;
  //l1->lip_clear     = 0;

  l1->lip_devread   = &devread;
  l1->lip_rawread   = &rawread;

  l1->lip_mem_lower = &mem_lower;
  l1->lip_mem_upper = &mem_upper;

  l1->lip_filepos   = &filepos;
  l1->lip_filemax   = &filemax;

  l1->lip_buf_drive = &buf_drive;
  l1->lip_buf_track = &buf_track;
  l1->lip_buf_geom  = &buf_geom;

  l1->lip_errnum    = &errnum;

  l1->lip_saved_drive = &saved_drive;
  l1->lip_saved_partition = &saved_partition;

  l1->lip_current_drive = &current_drive;
  l1->lip_current_partition = &current_partition;
  l1->lip_current_slice = &current_slice;
  l1->lip_part_start    = &part_start;
  l1->lip_part_length   = &part_length;
  l1->lip_fsmax         = &fsmax;
#ifndef STAGE1_5
  l1->lip_printmsg      = &printmsg;
  l1->lip_printb        = &printb;
  l1->lip_printw        = &printw;
  l1->lip_printd        = &printd;
#endif
}

void setlip(void)
{
  l1 = &lip1;
  setlip1(l1);
#ifndef STAGE1_5
  l2 = &lip2;
  setlip2(l2);
#endif
}

#ifndef STAGE1_5

void panic(char *msg, char *file);
int  abbrev(char *s1, char *s2, int n);
char *strip(char *s);
char *trim(char *s);

int process_cfg(char *cfg);
char *skip_to (int after_equal, char *cmdline);

int process_cfg_line1(char *line)
{
   int i, n;
   char *r, *s;
   static char section[0x20];
   static int sec_to_load;

   if (!*line) return 1;

   i = grub_strlen(line) - 1;
   /* section header (a word in brackets) */
   if (line[0] == '[' && line[i] == ']')
   {
     line++;
     line[i - 1] = '\0';
     grub_strncpy(section, line, sizeof(section));
     //printmsg(section);
     return 1;
   }

   //return 1;

   if (!grub_strcmp(section, "global"))
   {
     if (abbrev(line, "driveletter", 11))
     {
       line = skip_to(1, line);
       // first letter
       conf.driveletter = grub_tolower(*line) - 'a';
       if (conf.driveletter > 8) // not floppy
         conf.driveletter = conf.driveletter - 2 + 0x80;
     }
     else if (abbrev(line, "multiboot", 9))
     {
       line = strip(skip_to(1, line));
       if (!grub_strcmp(line, "yes") || !grub_strcmp(line, "on"))
         n = 1;
       else if (!grub_strcmp(line, "ask"))
         n = 2;
       else
         n = 0;
       conf.multiboot = n;
     }
     else
     {
     }

     return 1;
   }
   else if (!grub_strcmp(section, "microfsd"))
   {
     if (abbrev(line, "list", 4))
     {
       line = skip_to(1, line);
       grub_strcpy(strpos, line);
       s = strpos;
       r = strpos;
       num_fsys = 0;
       while (*s) {
         while (*s && *s != ',') s++;
         *s = '\0';
         fsys_list[num_fsys] = r;
         r = s + 1;
         s = r;
         strpos = r;
         num_fsys++;
       }
     }
     else if (abbrev(line, "ignorecase", 10))
     {
        line = skip_to(1, line);
        if (!grub_strcmp(line, "yes") || !grub_strcmp(line, "on"))
          n = 1;
        else
          n = 0;
        conf.mufsd.ignorecase = n;
     }
     else
     {
     }

     return 1;
   }
   else if (!grub_strcmp(section, "term"))
   {
     if (abbrev(line, "list", 4))
     {
       line = skip_to(1, line);
       grub_strcpy(strpos, line);
       s = strpos;
       r = strpos;
       num_term = 0;
       while (*s) {
         while (*s && *s != ',') s++;
         *s = '\0';
         term_list[num_term] = r;
         r = s + 1;
         s = r;
         strpos = r;
         num_term++;
       }
     }
     else if (abbrev(line, "default", 7))
     {
       line = skip_to(1, line);
       if (safe_parse_maxint(&line, &n))
         conf.term._default = n;
       else
         panic("process_cfg_line: incorrect default terminal!", "");
     }
     else
     {
     }

     return 1;
   }
   else if (!grub_strcmp(section, "loader"))
   {
     /* [loader] section */
     if (abbrev(line, "name", 4)) {
       line = strip(skip_to(1, line));
       grub_strncpy(conf.loader.name, line, sizeof(conf.loader.name));
     }
     else if (abbrev(line, "base", 4))
     {
       line = strip(skip_to(1, line));
       if (safe_parse_maxint(&line, &n))
         conf.loader.base = n;
       else
         panic("process_cfg_line: incorrect loader load base value!", "");
     }
     else
     {
     }

     return 1;
   }
   else if (!grub_strcmp(section, "minifsd"))
   {
     /* [minifsd] section */
     if (abbrev(line, "name", 4)) {
       line = strip(skip_to(1, line));
       if (!grub_strcmp(line, "none")) *line = '\0';
       grub_strncpy(conf.mini.name, line, sizeof(conf.mini.name));
     }
     else if (abbrev(line, "base", 4))
     {
       line = strip(skip_to(1, line));
       if (safe_parse_maxint(&line, &n))
         conf.mini.base = n;
       else
         panic("process_cfg_line: incorrect minifsd load base value!", "");
     }
     else
     {
     }

     return 1;
   }
   else if (!grub_strcmp(section, "aliases"))
   {
     /* file aliases section */
     if (abbrev(line, "enable", 6))
     {
     }

     return 1;
   }
   else
   {
     return 1;
   }

   return 0;
}

/*  init default terminal blackbox
 *
 */
void init_term(void)
{
  int n;

  /* default terminal to init */
  n = conf.term._default;
  t = u_termctl(n);
}

/*  Load a pre-loader blackbox from file
 *  with specified path to a pair of buffers
 *  (in low memory and in high one) with
 *  specified number and init the blackbox.
 *  A buffer is chosen from 1,2,3rd
 *  blackbox buffers.
 *  p is a pointer to structure filled
 *  by blackbox init function with
 *  pointers to blackbox entry points.
 */
int blackbox_load(char *path, int bufno, void *p)
{
  void (*blackbox_init)(void *p, unsigned int shift);
  //char buf[EXT_LEN];
  char rel_file[0x100];
  char *lodest, *hidest;
  int  i, rc;

  switch (bufno)
  {
    case 1:
    {
      hidest = (char *)EXT1HIBUF_BASE;
      lodest = (char *)EXT1LOBUF_BASE + relshift;
      break;
    }
    case 2:
    {
      hidest = (char *)EXT2HIBUF_BASE;
      lodest = (char *)EXT2LOBUF_BASE + relshift;
      break;
    }
    case 3:
    {
      hidest = (char *)EXT3HIBUF_BASE;
      lodest = (char *)EXT3LOBUF_BASE + relshift;
      break;
    }
    default:
    {
      return 0;
    }
  }

  //u_msg("!!");

  rc = freeldr_open(path);

  if (rc)
  {
    rc = freeldr_read(hidest, -1);
  }
  else
  {
    panic("Can't open blackbox file:", path);
  }

  /* copy realmode part to a low memory buffer */
  //grub_memmove(hidest, buf, EXT_LEN);
  grub_memmove(lodest, hidest, EXTLO_LEN);
  grub_memmove(rel_file, path, 0x100);

  /* change file extension to .rel */
  i = lastpos('.', rel_file) - 1;
  grub_strcpy(rel_file + i, ".rel\0");

  reloc(hidest, rel_file, (unsigned long)(hidest - EXT_BUF_BASE + SHIFT));

  /* init blackbox */
  blackbox_init = (void *)hidest;
  blackbox_init(p, relshift);

  return 1;
}

/*  returns the last position of symbol
 *  in string.
 */
int lastpos(char c, char *s)
{
  int i = grub_strlen(s) - 1;
  while (i >= 0 && s[i] != c) i--;

  return i + 1;
}

/*  Relocate a file in memory using its
 *  .rel file.
 *  base is a file base, rel_file is .rel
 *  file name and shift is the relocation
 *  shift
 */
void reloc(char *base, char *rel_file, unsigned long shift)
{
  int  i, n, rc;
  char buf[0x1000];

  typedef _Packed struct {
    unsigned short addr;
    unsigned char  shift;
  } rel_item;

  rel_item *p;
  unsigned long *addr;

  /* Load .rel file */
  rc = freeldr_open(rel_file);

  if (rc) {
    // buf = (char *)(EXT2BUF_BASE);
    rc  = freeldr_read(buf, -1);
  } else {
    panic("Can't open .rel file:", rel_file);
  }

  /* number of reloc items */
  n = *((unsigned short *)(buf)) / 3;
  p = (rel_item *)(buf + 2);

  for (i = 0; i < n; i++) {
    addr  = (unsigned long *)(base + p[i].addr);
    *addr += shift >> p[i].shift;
  }
}

#endif

int init(void)
{
  char cfg[0x20];
  char str[0x80];
  int rc, files;
  char *buf;
  char *fn;
  char *s;
  unsigned long ldrlen = 0, mfslen = 0;
  unsigned long ldrbase;
  bios_parameters_block *bpb;
  struct geometry geom;
  unsigned short *p;
  unsigned long  *q;
  struct desc *z;
  unsigned long base;
  int i, k, l;
  int key;

#ifndef STAGE1_5

  gateA20(1);
  /* use putchar() implementation through printmsg() */
  use_term = 0;
#endif
  //idt_init();

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

  // backup uFSD
  grub_memmove((void *)(UFSD_BASE), (void *)(EXT_BUF_BASE), EXT_LEN);

  /* call uFSD init (set linkage) */
  fsd_init = (void *)(EXT_BUF_BASE); // uFSD base address
  fsd_init(l1);

#ifndef STAGE1_5

  /* build config filename */
  rc = grub_strlen(preldr_path);
  grub_strcpy(cfg, preldr_path);
  grub_strcpy(cfg + rc, cfg_file);

  /* parse config file */
  process_cfg_line = process_cfg_line1;
  if (!(rc = process_cfg(cfg)))
  {
    panic("Error parsing loader config file!", cfg);
  }
  else if (rc == -1)
  {
    panic("Load error!", "");
  }

  relshift = 0;
  init_term();
  /* use putchar() implementation through term blackbox */
  use_term = 1;

  // empty keyboard buffer
  //while (t->checkkey() != -1) ;

  if (conf.multiboot == 2)
  {
    printf("multiboot = (y)es/(n)o?: ");

    do {
      key = t->getkey() & 0xff;
    } while (key != 'y' && key != 'Y' && key != 'n' && key != 'N');

    if (key == 'N' || key == 'n')
      conf.multiboot = 0;
    else
      conf.multiboot = 1;

    printf("\r\n");
  }

  if (!grub_strcmp(conf.loader.name, "default"))
  {
    if (conf.multiboot)
      grub_strcpy(conf.loader.name, "/boot/loader/freeldr.mdl\0");
    else
      grub_strcpy(conf.loader.name, "/os2ldr\0");
  }

  /* Show config params */ /*
  printmsg("\r\nConfig parameters:");
  printmsg("\r\ndriveletter = "); printb(conf.driveletter);
  printmsg("\r\nmultiboot = ");   printb(conf.multiboot);
  printmsg("\r\nmufsd.ignorecase = ");   printb(conf.mufsd.ignorecase);
  printmsg("\r\nFilesys: ");
  for (i = 0; i < FSYS_MAX && conf.mufsd.fsys_list[i]; i++) {
    printmsg(conf.mufsd.fsys_list[i]);
    printmsg(" ");
  }
  printmsg("\r\nloader.name = "); printmsg(conf.loader.name);
  printmsg("\r\nloader.base = "); printd(conf.loader.base);
  printmsg("\r\nmini.name = "); printmsg(conf.mini.name);
  printmsg("\r\nmini.base = "); printd(conf.mini.base);
  printmsg("\r\n");
  for (i = 0; i < MAX_ALIAS; i++) {
    s = conf.alias[i].name;
    if (!s) break;
    printmsg(s);
    printmsg("=");
    s = conf.alias[i].alias;
    printmsg(s);
    printmsg("\r\n");
  }
  for (i = 0; i < num_term; i++)
  {
    printmsg(conf.term.term_list[i]);
    printmsg("\r\n");
  }
  printd(conf.term._default);
  __asm {
    cli
    hlt
  }       */

  /* load os2ldr */
  fn = conf.loader.name;
  rc = freeldr_open(fn);
  //printf("o ret %d\r\n", rc);
  //printmsg("freeldr_open(\"");
  //printmsg(fn);
  //printmsg("\") returned: ");
  //printd(rc);

  buf = (char *)(conf.loader.base);

  if (rc) {
    ldrlen = freeldr_read(buf, -1);
    //printf("r 0x%x %d cnt %d\r\n", buf, -1, ldrlen);
    //printmsg("\r\nfreeldr_read() returned size: ");
    //printd(ldrlen);
    //printmsg("\r\n");
  } else {
    panic("Can't open loader file: ", fn);
  }

  /* load minifsd */
  fn = conf.mini.name;
  if (*fn) { // is minifsd needed?
    rc = freeldr_open(fn);
    //printf("o ret %d\r\n", rc);
    //printmsg("freeldr_open(\"");
    //printmsg(fn);
    //printmsg("\") returned: ");
    //printd(rc);

    buf = (char *)(conf.mini.base);

    if (rc) {
      mfslen = freeldr_read(buf, -1);
      //printf("r 0x%x %d cnt %d\r\n", buf, -1, mfslen);
      //printmsg("\r\nfreeldr_read() returned size: ");
      //printd(mfslen);
      //printmsg("\r\n");
    } else {
      panic("Can't open minifsd filename: ", fn);
    }
  }

  printf("mem_lower=0x%x\r\n", mem_lower);
  //printmsg("mem_lower=");
  //printd(mem_lower);

  /* calculate highest available address
     -- os2ldr base or top of low memory  */

  k = ldrlen >> (PAGESHIFT - 3);
  i = k >> 3;

  /* special case: os2ldr of aurora sized
     44544 bytes, 44544 >> 12 == 0xa      */
  if (k == 0x57) i++; // one page more

  if (!conf.multiboot) // os2ldr
    ldrbase = ((mem_lower >> (PAGESHIFT - KSHIFT)) - (i + 3)) << PAGESHIFT;
  else                 // multiboot loader
    ldrbase =  mem_lower << KSHIFT;

  printf("ldr base 0x%x\r\n", ldrbase);
  //printmsg("\r\n");
  //printd(ldrbase);

  /* the correction shift added while relocating */
  relshift = ldrbase - (PREFERRED_BASE + 0x10000);

  /* move preldr and its data */
  grub_memmove((char *)(ldrbase - 0x10000),
               (char *)(EXT_BUF_BASE + EXT_LEN - 0x10000),
               0x10000);

  /* move uFSD */
  grub_memmove((char *)(EXT3HIBUF_BASE),
               (char *)(EXT_BUF_BASE),
               EXT_LEN);

  __asm {
    /* move 16-bit stack to the place of former uFSD buffer */
    std  // copy in backward direction
    mov  esi, EXT_BUF_BASE - 4
    mov  edi, EXT_BUF_BASE + EXT_LEN - 4
    add  edi, relshift
    mov  ecx, EXT_BUF_BASE
    xor  eax, eax
    mov  ax,  ds:[RMSTACK]      // 16-bit stack SP
    add  eax, STAGE0_BASE       // base + SP == stack top phys address
    // now we got stack top phys address in EAX
    sub  ecx, eax               // stack length in bytes
    add  ecx, 3                 // round up to a multiple of 4
    shr  ecx, 2                 // and / 4 to get size in 32-bit words
    rep  movsd                  // move stack up
    // adjust SP to EXT_LEN bytes
    xor  eax, eax
    mov  ax, ds:[RMSTACK]       // SP former value
    add  ax, EXT_LEN
    mov  ds:[RMSTACK], ax       // adjusted SP value
    // relocation shift
    mov  eax, relshift
    /* switch 32-bit stack to the place of relocation */
    add  esp,   eax
    add  ebp,   eax
    /* fixup words in 32-bit stack */
    add  [ebp], eax
    add  [ebp - 0xc],  eax
    add  [ebp + 0x18], eax
    add  [ebp + 0x20], eax
  }

  printf("rel shift 0x%x\r\n", relshift);
  //printmsg("\r\nrelshift=");
  //printd(relshift);
  //printmsg("\r\n");

  /* disable use of terminal blackbox
     before it gets re-initted (old preldr copy) */
  use_term = 0;

  /* fixup preldr */
  reloc((char *)(STAGE0_BASE  + relshift), "/boot/loader/preldr0.rel", relshift);

  /* build filesystem driver .rel file path */
  grub_strcpy(str, preldr_path);
  i = grub_strlen(preldr_path);
  grub_strcpy(str + i, fsd_dir);
  k = grub_strlen(fsd_dir);
  grub_strcpy(str + i + k, install_filesys);
  l = grub_strlen(install_filesys);
  grub_strcpy(str + i + k + l, ".rel\0");

  /* fixup uFSD */
  reloc((char *)(EXT3HIBUF_BASE), str, EXT3HIBUF_BASE - EXT_BUF_BASE + SHIFT);

  /* jump to relocated pre-loader */
  jmp_reloc(relshift);
  /* now we are at the place of relocation */

  /* disable use of terminal blackbox
     before it gets re-initted (new preldr copy) */
  use_term = 0;

  idt_init();

  //l1 += relshift;
  //l2 += relshift;

  /* setting LIP */
  setlip();

  /* save boot drive uFSD */
  grub_memmove((void *)(UFSD_BASE), (void *)(EXT3HIBUF_BASE), EXT_LEN);
  /* call uFSD init (set linkage) */
  fsd_init = (void *)(EXT3HIBUF_BASE); // uFSD base address
  fsd_init(l1);

  //z = &gdt;
  z = (struct desc *)GDT_ADDR;

  /* fix bases of gdt descriptors */
  base = STAGE0_BASE;
  for (i =  3; i < 6; i++) {
    (z + i)->ds_baselo  = base & 0xffff;
    (z + i)->ds_basehi1 = (base & 0xff0000) >> 16;
    (z + i)->ds_basehi2 = (base & 0xff000000) >> 24;
  }

  // GDT base
  gdtdesc.g_base = GDT_ADDR;

  /* set new gdt */
  __asm {
    lea  eax, gdtdesc
    lgdt fword ptr [eax]
  }

  /* set boot flags */
  boot_flags = BOOTFLAG_MICROFSD;

  files = 2;
  if (*fn) { // if minifsd present
    files++;
    boot_flags |=  BOOTFLAG_MINIFSD;
  }

  /* set filetable */
  ft.ft_cfiles = files;
  ft.ft_ldrseg = conf.loader.base >> 4;
  ft.ft_ldrlen = ldrlen; // 0x3800;
  ft.ft_museg  = (EXT_BUF_BASE + EXT_LEN - 0x10000) >> 4; // 0x8500; -- OS/2 2.0       // 0x8600; -- OS/2 2.1
                                         // 0x8400; -- Merlin & Warp3 // 0x8100; -- Aurora
  ft.ft_mulen  = 0x10000;     // It is empirically found maximal value
  ft.ft_mfsseg = conf.mini.base >> 4;    // 0x7c;
  ft.ft_mfslen = mfslen;  // 0x95f0;
  ft.ft_ripseg = 0; // 0x800;   // end of mfs
  ft.ft_riplen = 0; // 62*1024 - mfslen; //0x3084;  // max == 62k - mfslen

  ft.ft_muOpen.seg       = STAGE0_BASE >> 4;
  ft.ft_muOpen.off       = (unsigned short)(&mu_Open);

  ft.ft_muRead.seg       = STAGE0_BASE >> 4;
  ft.ft_muRead.off       = (unsigned short)(&mu_Read);

  ft.ft_muClose.seg      = STAGE0_BASE >> 4;
  ft.ft_muClose.off      = (unsigned short)(&mu_Close);

  ft.ft_muTerminate.seg  = STAGE0_BASE >> 4;
  ft.ft_muTerminate.off  = (unsigned short)(&mu_Terminate);

  /* set BPB */
  bpb = (bios_parameters_block *)(BOOTSEC_BASE + 0xb);

  if (boot_drive == cdrom_drive) { // booting from CDROM drive
    // fill fake BPB
    grub_memset((void *)bpb, 0, sizeof(bios_parameters_block));

    bpb->sect_size  = 0x800;
    bpb->clus_size  = 0x40;
    bpb->n_sect_ext = geom.total_sectors; // 0x30d;
    bpb->media_desc = 0xf8;
    bpb->track_size = 0x3f;
    bpb->heads_cnt  = 0xff;
    bpb->marker     = 0x29;
  }

  bpb->disk_num    = (unsigned char)(boot_drive & 0xff);
  bpb->log_drive   = conf.driveletter; // 0x92;
  bpb->hidden_secs = part_start;

  //bpb->disk_num    = 0x3;
  //bpb->log_drive   = 0x48;
  //bpb->marker      = 0x41;
  //bpb->vol_ser_no  = 0x00000082;

  /* Init terminal */
  init_term();
  use_term = 1;

  /* Init info in mbi structure */
  init_bios_info();

  if (conf.multiboot == 1) {
    /* return to loader from protected mode */
    unsigned long ldr_base = conf.loader.base;

    __asm {
      mov  dx, boot_flags
      mov  dl, byte ptr boot_drive

      // edi == pointer to filetable
      lea  edi, ft

      // esi ==  pointer to BPB
      mov  esi, BOOTSEC_BASE + 0xb

      // magic in eax
      mov  eax, BOOT_MAGIC

      // ebx == pointer to LIP
      mov  ebx, l2

      push ldr_base
      //int 3h
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
