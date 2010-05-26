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

lip1_t *l1, lip1;
int __cdecl (*fsd_init)(lip1_t *l);

#pragma aux gateA20 "*"
void gateA20(int);

int  num_fsys = 0;

unsigned long stage0base;
unsigned long scratchaddr;    // 512-byte scratch area
unsigned long bufferaddr;

#pragma aux stage0base      "*"

int print_possibilities = 0;

#ifndef STAGE1_5

extern int do_completion;

static int unique;
static char *unique_string;

extern void *filetab_ptr = 0;

extern int mem_lower;
extern int mem_upper;

extern unsigned char use_term;

extern unsigned long extended_memory;
#pragma aux extended_memory "*"
#pragma aux mu_Open_wr      "*"
#pragma aux mu_Read_wr      "*"
#pragma aux mu_Close_wr     "*"
#pragma aux mu_Terminate_wr "*"

int lastpos(char c, char *s);
void setlip2(lip2_t *l2);
int blackbox_load(char *path, int bufno, void *p);

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

#pragma aux install_filesys "*"

#pragma aux get_vbe_controller_info "*"
#pragma aux get_vbe_mode_info       "*"
#pragma aux set_vbe_mode            "*"
#pragma aux reset_vbe_mode          "*"
#pragma aux get_vbe_pmif            "*"

lip2_t *l2, lip2;

int (*process_cfg_line)(char *line);

int i;
int  num_term = 0;

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
#pragma aux gdtsrc  "*"
#pragma aux gdtdesc "*"
extern struct desc gdtsrc[10];
extern struct gdtr gdtdesc;

#pragma aux set_fsys    "*"
#pragma aux fsys_by_num "*"
int  set_fsys(char *fsname);
void fsys_by_num(int n, char *buf);

char lb[80];

struct geometry geom;

char at_drive[16];

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
  unsigned char driveletter;
  unsigned char multiboot;
  struct {
    char ignorecase;
    char **fsys_list;
  } mufsd;
  struct {
    char name[0x20];
    int  base;
    int  len;
  } loader;
  struct {
    char name[0x20];
    int  base;
    int  len;
  } extloader;
  struct {
    char name[0x20];
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
} conf = {0x80, 2, {0, fsys_list}, {"/boot/loader/freeldr.mdl", 0x20000, 0xf}, {"/os2ldr", 0x10000, 0xf},
          {"/os2boot", 0x7c0}, {0, term_list},};

char *redir_list[] = {"OS2LDR", "OS2LDR.MSG", "OS2KRNL", "OS2LDR.INI", "OS2DUMP", "OS2LDR.FNT", "OS2DBCS.FNT", 0};

char freeldr_path[0x20];
char *preldr_path = "/boot/loader/"; // freeldr path
char *fsd_dir     = "fsd/";           // uFSD's subdir
char *term_dir    = "term/";          // term   subdir
char *cfg_file    = "preldr.ini";     // .INI file

#pragma aux lip2 "*"

extern mu_Open;
extern mu_Read;
extern mu_Close;
extern mu_Terminate;

extern FileTable ft;

#endif

#pragma aux lip1 "*"

extern unsigned long saved_drive;
extern unsigned long saved_partition;
extern int saved_slice;
extern int saved_fsys_type;
extern unsigned long cdrom_drive;

extern unsigned long current_drive;
extern unsigned long current_partition;
extern int           current_slice;
extern unsigned long part_start;
extern unsigned long part_length;
extern int           fsmax;

extern int buf_drive;
extern int buf_track;

extern unsigned short boot_flags;
extern unsigned long  boot_drive;
extern unsigned long  install_partition;
extern char install_filesys[0x10];

extern int fsmax;

void __cdecl real_test(void);
void __cdecl call_rm(fp_t);

void idt_init(void);
#pragma aux idt_init "*"

#ifndef STAGE1_5

char prefix[0x200];

unsigned long ldrlen = 0;

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

int __cdecl
u_dir (char *dirname)
{
  return freeldr_dir(dirname);
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
    case PARM_BUF_DRIVE:
      {
        if (action == ACT_GET)
          *val = buf_drive;
        else
          buf_drive = *val;

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
    case PARM_PART_START:
      {
        if (action == ACT_GET)
          *val = part_start;
        else
          part_start = *val;

        return 0;
      };
    case PARM_PART_LENGTH:
      {
        if (action == ACT_GET)
          *val = part_length;
        else
          part_length = *val;

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
    case PARM_PREFIX:
      {
        if (action == ACT_GET)
          strcpy((char *)val, prefix);
        else
          strcpy(prefix, (char *)val);

        return 0;
      }
    case PARM_LDRLEN:
      {
        if (action == ACT_GET)
          *val = ldrlen;
        else
          ldrlen = *val;

        return 0;
      };
    case PARM_AT_DRIVE:
      {
        if (action == ACT_GET)
          strcpy((char *)val, at_drive);
        else
          strcpy(at_drive, (char *)val);

        return 0;
      }
    case PARM_DISK_READ_HOOK:
      {
        if (action == ACT_GET)
          *val = (unsigned int)disk_read_hook;
        else
          disk_read_hook = (disk_read_hook_t)*val;

        return 0;
      }
    case PARM_DISK_READ_FUNC:
      {
        if (action == ACT_GET)
          *val = (unsigned int)disk_read_func;
        else
          disk_read_func = (disk_read_hook_t)*val;

        return 0;
      }
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
  i = grub_strlen(freeldr_path);
  grub_strcpy(term, freeldr_path);
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
    printf("\r\nterminal loaded\r\n");
    if (trm.flags == TERM_NEED_INIT)
      trm.startup(0);
  }
  else
    return 0;

  return &trm;
}

int redir_file(char *file)
{
  char **p = redir_list;
  char *q;
  char buf[0x200];

  grub_strncpy(buf, file, strlen(file));
  for (q = buf; *q && q - buf < 0x200; q++)
    *q = grub_toupper(*q);

  while (*p)
  {
    if (!strcmp(buf, *p))
      return 1;
    else
      p++;
  }

  return 0;
}

#endif

int
freeldr_open (char *filename)
{

#ifndef STAGE1_5

   char *p;
   int  i, l, k;
   int  i0 = 0;
   int  rc;
   unsigned short ret;
   char buf[0x200];

   if ((*filename == '(' && (p = strstr(filename + 1, ")")) &&
      (p - filename < 9) && strlen(filename) == p - filename + 1) ||
      !*filename)
   {
     //printf("rt %s\r\n", filename);
     rc = mkroot(filename);
     //if (rc) printf(" failed!\r\n");
     return !rc;
   }

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

     // if redirection prefix is set and
     // file is from redir list
     if (*prefix && redir_file(filename))
     {
       l = strlen(prefix);
       grub_strncpy(buf, prefix, l);
       buf[l] = '/';
       k = strlen(filename);
       grub_strncpy(buf + l + 1, filename, k);
       buf[l + k + 1] = '\0';
     }
     else
     {
       /* prepend "/" to filename */
       if (*filename != '/' && *filename != '(') {
         buf[0] = '/';
         i0 = 1;
       }
       grub_strcpy(buf + i0, filename);
     }

     //printf("o %s", buf);
     //u_msg("o ");
     //u_msg(filename);
     //u_msg("\r\n");

     for (i = 0; buf[i] && i < 0x200; i++) {
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

         for (; buf[i] && i < 0x200; i++) {
           buf[i] = grub_toupper(buf[i]);
         }

         rc = grub_open(buf);
       }

       if (!rc) {
         /* try to open lowercase filename */
         for (i = 0; buf[i] && i < 0x200; i++)
           buf[i] = grub_tolower(buf[i]);

         rc = grub_open(buf);
       }
     }

     //if (!rc)
     //  printf(" fail!");

     //printf("\r\n");

     return rc;
   }
#else
   return grub_open(filename);
#endif
}

#ifndef STAGE1_5
int
freeldr_dir (char *name)
{
  return dir (name);
}
#endif

int
freeldr_read (char *buf, int len)
{
   int rc;
#ifndef STAGE1_5
   //printf("r 0x%x %d", buf, len);
   if (filetab_ptr)
   {
     // use 16-bit uFSD
     rc = mu_Read_wr(filepos, buf, len);
   }
   else
#endif
   {
     // use 32-bit uFSD
     rc = grub_read(buf, len);
   }
#ifndef STAGE1_5
   //printf(" sz %d\r\n", rc);
#endif
   return rc;
}

#ifndef STAGE1_5
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

#endif

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

#ifndef STAGE1_5

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


void setlip2(lip2_t *l2)
{
  l2->u_lip2magic       = LIP2_MAGIC;
  l2->u_open            = &u_open;
  l2->u_dir             = &u_dir;
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

/* If DO_COMPLETION is true, just print NAME. Otherwise save the unique
   part into UNIQUE_STRING.  */
void
print_a_completion (char *name)
{
  /* If NAME is "." or "..", do not count it.  */
  if (grub_strcmp (name, ".") == 0 || grub_strcmp (name, "..") == 0)
    return;

  if (do_completion)
    {
      char *buf = unique_string;

      if (! unique)
        while ((*buf++ = *name++))
          ;
      else
        {
          while (*buf && (*buf == *name))
            {
              buf++;
              name++;
            }
          /* mismatch, strip it.  */
          *buf = '\0';
        }
    }
  else
    grub_printf (" %s", name);

  unique++;
}

#endif

void setlip1(lip1_t *l1)
{
  l1->lip_open  = &freeldr_open;
  l1->lip_read  = &freeldr_read;
#ifndef STAGE1_5
  l1->lip_seek  = &freeldr_seek;
  l1->lip_term  = 0;
  l1->lip_close = &freeldr_close;
#endif

  //l->lip_memcheck = 0; //&grub_memcheck;
  l1->lip_memset   = &grub_memset;
  l1->lip_memmove  = &grub_memmove;
  l1->lip_strcpy   = &grub_strcpy;
  l1->lip_strcmp   = &grub_strcmp;
  l1->lip_memcmp   = &grub_memcmp;
  l1->lip_strlen   = &grub_strlen;
  l1->lip_isspace  = &grub_isspace;
  l1->lip_tolower  = &grub_tolower;

  l1->lip_substring = &substring;
  //l1->lip_pos       = 0;
  //l1->lip_clear     = 0;
  l1->lip_devread   = &devread;
  l1->lip_rawread   = &rawread;

#ifndef STAGE1_5
  l1->lip_mem_lower = &mem_lower;
  l1->lip_mem_upper = &mem_upper;
#endif
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
  l1->lip_print_possibilities = &print_possibilities;
  l1->lip_disk_read_hook      = (disk_read_hook_t)&disk_read_hook;
  l1->lip_disk_read_func      = (disk_read_hook_t)&disk_read_func;
#ifndef STAGE1_5
  l1->lip_print_a_completion  = &print_a_completion;
  l1->lip_printf        = &printf;

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
       if (safe_parse_maxint(&line, (long *)&n))
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
       if (safe_parse_maxint(&line, (long *)&n))
         conf.loader.base = n;
       else
         panic("process_cfg_line: incorrect loader load base value!", "");
     }
     else if (abbrev(line, "len", 3))
     {
       line = strip(skip_to(1, line));
       if (safe_parse_maxint(&line, (long *)&n))
         conf.loader.len = n;
       else
         panic("process_cfg_line: incorrect loader length value!", "");
     }
     else
     {
     }

     return 1;
   }
   else if (!grub_strcmp(section, "extloader"))
   {
     /* [extloader] section */
     if (abbrev(line, "name", 4)) {
       line = strip(skip_to(1, line));
       grub_strncpy(conf.extloader.name, line, sizeof(conf.extloader.name));
     }
     else if (abbrev(line, "base", 4))
     {
       line = strip(skip_to(1, line));
       if (safe_parse_maxint(&line, (long *)&n))
         conf.extloader.base = n;
       else
         panic("process_cfg_line: incorrect extloader load base value!", "");
     }
     else if (abbrev(line, "len", 3))
     {
       line = strip(skip_to(1, line));
       if (safe_parse_maxint(&line, (long *)&n))
         conf.extloader.len = n;
       else
         panic("process_cfg_line: incorrect extloader length value!", "");
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
       if (safe_parse_maxint(&line, (long *)&n))
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
  void (*blackbox_init)(lip1_t *l, void *p, unsigned int shift, char *gdt);
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
  blackbox_init(l1, p, relshift, (char *)&gdtsrc);

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
    panic("Can't open .rel file: ", rel_file);
  }

  /* number of reloc items */
  n = *((unsigned short *)(buf)) / 3;
  p = (rel_item *)(buf + 2);

  for (i = 0; i < n; i++) {
    addr  = (unsigned long *)(base + p[i].addr);
    *addr += shift >> p[i].shift;
  }
}

void __cdecl set_addr (void)
{
  struct desc *z;
  unsigned long ldrbase, preldr_size;
  unsigned long base;
  unsigned long *basep;
  int i, k;

  if (!grub_strcmp(conf.loader.name, "default"))
  {
    if (conf.multiboot)
      grub_strcpy(conf.loader.name, "/boot/loader/freeldr.mdl\0");
    else
      grub_strcpy(conf.loader.name, "/os2ldr\0");
  }

  if (*conf.extloader.name)
    freeldr_open(conf.extloader.name);
  else
    freeldr_open(conf.loader.name);

  if (!ldrlen)
  {
    /* size of the loader */
    ldrlen  = filemax;

    printf("mem_lower=0x%x\r\n", mem_lower);

    ldrlen = (ldrlen + 0x1280 + 0x1000 + 0xfff) >> PAGESHIFT;
  } /* os2ldr stack size--^       ^----arena info size  */

  /* calculate highest available address
     -- os2ldr base or top of low memory  */
  if (!conf.multiboot || *conf.extloader.name) // os2ldr
    ldrbase = ((mem_lower >> (PAGESHIFT - KSHIFT)) - ldrlen) << PAGESHIFT;
  else
    ldrbase = mem_lower << KSHIFT;

  printf("ldr base 0x%x\r\n", ldrbase);

  /* the correction shift added while relocating */
  //relshift = ldrbase - (PREFERRED_BASE + 0x10000);
  preldr_size = 0x1000 + 0x4e00 + 0x2000 + 0x2000;
  /* int 13h buf--^  stack--^  preldr--^ (low) */
  relshift = ldrbase - (PREFERRED_BASE + preldr_size);

  scratchaddr += relshift;
  bufferaddr  += relshift;

  /* move preldr and its data */
  //grub_memmove((char *)(ldrbase - 0x10000),
  //             (char *)(EXT_BUF_BASE + EXT_LEN - 0x10000),
  //             0x10000);
  /* move lower part to the end of conventional memory */
  grub_memmove((char *)(ldrbase - preldr_size),
               (char *)(PREFERRED_BASE),
               preldr_size);
  /* move upper part to high memory */
  grub_memmove((char *)(PRELDR_BASE),
               (char *)(stage0base),
               0x10000);

  __asm {
    /* move 16-bit stack to the place of former uFSD buffer */
    //std  // copy in backward direction
    //mov  esi, EXT_BUF_BASE - 4
    //mov  edi, EXT_BUF_BASE + EXT_LEN - 4
    //add  edi, relshift
    //mov  ecx, EXT_BUF_BASE
    //xor  eax, eax
    //mov  ax,  ds:[RMSTACK]      // 16-bit stack SP
    //add  eax, stage0base       // base + SP == stack top phys address
    // now we got stack top phys address in EAX
    //sub  ecx, eax               // stack length in bytes
    //add  ecx, 3                 // round up to a multiple of 4
    //shr  ecx, 2                 // and / 4 to get size in 32-bit words
    //rep  movsd                  // move stack up
    // adjust SP to EXT_LEN bytes
    //xor  eax, eax
    //mov  ax, ds:[RMSTACK]       // SP former value
    //add  ax, EXT_LEN
    //mov  ds:[RMSTACK], ax       // adjusted SP value
    // relocation shift
    mov  eax, relshift
    /* switch 32-bit stack to the place of relocation */
    add  esp, eax
    //add  ebp,   eax
    /* fixup words in 32-bit stack */
    add  [esp], eax          // ebp on return from init()
    //add  [esp + 8],    eax
    //add  [esp + 0xc],  eax

    /*  */
    mov  eax, PRELDR_BASE
    lea  edx, stage0base
    sub  eax, [edx]
    add  [esp + 0x10], eax       // return addr of set_addr()

    mov  eax, relshift
    add  [esp + 0xb8], eax
    add  [esp + 0xd0], eax
    add  [esp + 0xd8], eax
    add  [esp + 0xdc], eax
    add  [esp + 0xec], eax
    /* fix up stack base */
    shr  eax, 4
    add  word ptr ds:[RMSTACK + 2], ax
  }

  printf("rel shift 0x%x\r\n", relshift);
  //printmsg("\r\nrelshift=");
  //printd(relshift);
  //printmsg("\r\n");

  /* disable use of terminal blackbox
     before it gets re-initted (old preldr copy) */
  use_term = 0;

  /* fixup preldr */
  //reloc((char *)(STAGE0_BASE  + relshift), "/boot/loader/preldr0.rel", relshift);
  reloc((char *)(PRELDR_BASE), "/boot/loader/preldr0.rel", PRELDR_BASE - stage0base);

  /* jump to relocated pre-loader */
  //jmp_reloc(relshift);
  jmp_reloc(PRELDR_BASE - stage0base);
  /* now we are at the place of relocation */

  /* disable use of terminal blackbox
     before it gets re-initted (new preldr copy) */
  use_term = 0;

  stage0base  += relshift;
  /* fixup base in preldr header */
  basep  = (unsigned long *)((char *)stage0base + 0x8);
  *basep += relshift;
  /* copy relshift to preldr header */
  basep  = (unsigned long *)((char *)stage0base + 0x38);
  *basep =  PRELDR_BASE - stage0base;
  basep  = (unsigned long *)((char *)stage0base + 0x3c);
  *basep =  relshift;

  idt_init();

  //l1 += relshift;
  //l2 += relshift;

  /* setting LIP */
  setlip();

  /* save boot drive uFSD */
  //grub_memmove((void *)(UFSD_BASE), (void *)(EXT3HIBUF_BASE), EXT_LEN);
  /* call uFSD init (set linkage) */
  fsd_init = (void *)(EXT3HIBUF_BASE); // uFSD base address
  fsd_init(l1);

  //z = &gdt;
  //z = (struct desc *)GDT_ADDR;
  z = &gdtsrc;

  /* fix bases of gdt descriptors */
  //base = STAGE0_BASE;
  base = (ldrbase - 0x2000);
  /* fix 16-bit DS and CS descriptors */
  for (i = 3; i < 5; i++) {
    (z + i)->ds_baselo  = base & 0xffff;
    (z + i)->ds_basehi1 = (base & 0xff0000) >> 16;
    (z + i)->ds_basehi2 = (base & 0xff000000) >> 24;
  }
  /* fix 16-bit SS descriptor */
  //i++;
  base -= 0x1060 << 4;
  (z + i)->ds_baselo  = base & 0xffff;
  (z + i)->ds_basehi1 = (base & 0xff0000) >> 16;
  (z + i)->ds_basehi2 = (base & 0xff000000) >> 24;

  // GDT base
  //gdtdesc.g_base = GDT_ADDR;
  gdtdesc.g_base = (unsigned long)z;


  /* set new gdt */
  __asm {
    lea  eax, gdtdesc
    lgdt fword ptr [eax]
  }
}

void set_ftable (int which_ldr)
{
  int   ldrlen;
  int   mfslen;
  int   ldrbase;
  int   mfsbase;
  char  *fn;
  int   rc;
  char  *buf;
  int   files;
  char *old_ft_addr;
  bios_parameters_block *bpb;

  if (which_ldr)
  {
    fn = conf.extloader.name;
    ldrbase = conf.extloader.base;
  }
  else
  {
    fn = conf.loader.name;
    ldrbase = conf.loader.base;
  }
  mfsbase = conf.mini.base;

  /* load os2ldr */
  rc = freeldr_open(fn);

  buf = (char *)(ldrbase);

  if (rc) {
    ldrlen = freeldr_read(buf, -1);
  } else {
    panic("Can't open loader file: ", fn);
  }

  /* load minifsd */
  fn = conf.mini.name;
  if (*fn) { // is minifsd needed?
    rc = freeldr_open(fn);

    buf = (char *)(mfsbase);

    if (rc) {
      mfslen = freeldr_read(buf, -1);
    } else {
      panic("Can't open minifsd filename: ", fn);
    }
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
  ft.ft_ldrseg = ldrbase >> 4;
  ft.ft_ldrlen = ldrlen; // 0x3800;
  ft.ft_museg  = (scratchaddr) >> 4; // 0x8500; -- OS/2 2.0       // 0x8600; -- OS/2 2.1
                                         // 0x8400; -- Merlin & Warp3 // 0x8100; -- Aurora
  ft.ft_mulen  = 0x1000 + 0x4e00 + 0x2000 + 0x2000; // 0x10000;     // It is empirically found maximal value
  ft.ft_mfsseg = mfsbase >> 4;    // 0x7c;
  ft.ft_mfslen = mfslen;  // 0x95f0;
  ft.ft_ripseg = 0; // 0x800;   // end of mfs
  ft.ft_riplen = 0; // 62*1024 - mfslen; //0x3084;  // max == 62k - mfslen

  ft.ft_muOpen.seg       = (stage0base) >> 4;
  ft.ft_muOpen.off       = (unsigned short)(&mu_Open);

  ft.ft_muRead.seg       = (stage0base) >> 4;
  ft.ft_muRead.off       = (unsigned short)(&mu_Read);

  ft.ft_muClose.seg      = (stage0base) >> 4;
  ft.ft_muClose.off      = (unsigned short)(&mu_Close);

  ft.ft_muTerminate.seg  = (stage0base) >> 4;
  ft.ft_muTerminate.off  = (unsigned short)(&mu_Terminate);

  /* set BPB */
  bpb = (bios_parameters_block *)(stage0base - 0x200 + 0xb);

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

  /* copy filetable down to 1st megabyte */
  old_ft_addr = (char *)(&ft) - (PRELDR_BASE - stage0base);
  memmove(old_ft_addr, &ft, sizeof(FileTable));

  //bpb->disk_num    = 0x3;
  //bpb->log_drive   = 0x48;
  //bpb->marker      = 0x41;
  //bpb->vol_ser_no  = 0x00000082;
}

#if 1

void determine_boot_drive(void)
{
  unsigned long drv;
  char buf[4];
  char c = 0;

  memset(at_drive, 0, sizeof(at_drive));

  if (boot_drive == cdrom_drive && cdrom_drive != 0xff)
  {
    /* cd-rom */
    grub_strcpy(at_drive, "(cd");
    c = 0;
  }
  else if (boot_drive & 0x80)
  {
    /* hard disk */
    grub_strcpy(at_drive, "(hd");
    c = 1;
  }
  else
  {
    /* floppy disk */
    grub_strcpy(at_drive, "(fd");
    c = 2;
  }

  /* clear high bits */
  drv = boot_drive & 0xff;
  /* clear hard disk bit */
  drv = drv & ~0x80;

  if (c)
  {
    grub_strcat(at_drive, at_drive, ltoa(drv, buf, 10));
    /* shift and clear high bits */
    drv = (install_partition >> 16) & 0xff;
    if (drv != 0xff)
    {
      grub_strcat(at_drive, at_drive, ",");
      grub_strcat(at_drive, at_drive, ltoa(drv, buf, 10));
      drv = (install_partition >> 8) & 0xff;
      if (drv != 0xff)
      {
        /* a sub-partition */
        buf[0] = 'a' + drv;
        buf[1] = '\0';
        grub_strcat(at_drive, at_drive, buf);
      }
    }
  }
  grub_strcat(at_drive, at_drive, ")");
}

#endif
#endif

void init(void)
{
#ifndef STAGE1_5
  char cfg[0x20];
  char str[0x80];
  char *s;
  unsigned short *p;
  unsigned long  *q;
  struct desc *z;
  unsigned long ldrlen = 0, mfslen = 0;
  unsigned long ldrbase;
  unsigned long base;
  int i, k, l;
  int key;
#endif
  extern char preldr[];
  int    rc;
  char   *buf;

  stage0base  = STAGE0_BASE;
  scratchaddr = SCRATCHADDR;
  bufferaddr  = BUFFERADDR;

  gateA20(1);

#ifndef STAGE1_5
  memset(prefix, 0, sizeof(prefix));
  relshift = 0;
  /* use putchar() implementation through printmsg() */
  use_term = 0;
#endif
  //idt_init();

  /* Set boot drive and partition.  */
  saved_drive = boot_drive;
  saved_partition = install_partition;

#ifndef STAGE1_5
  /* Set cdrom drive.   */
  /* Get the geometry.  */
  if (get_diskinfo (boot_drive, &geom)
      || ! (geom.flags & BIOSDISK_FLAG_CDROM))
    cdrom_drive = GRUB_INVALID_DRIVE;
  else
  {
    cdrom_drive = boot_drive;
    //part_start = 0;
    //part_length = geom.total_sectors;
    //current_slice = 0;
  }
#else
  cdrom_drive = GRUB_INVALID_DRIVE;
#endif
  current_drive = GRUB_INVALID_DRIVE;
  current_partition = 0xffffff;

  /* setting LIP */
  setlip();

#ifndef STAGE1_5
  // backup uFSD
  grub_memmove((void *)(UFSD_BASE), (void *)(EXT_BUF_BASE), EXT_LEN);

  // zero-out FS buffer
  memset((char *)FSYS_BUF, 0, 0x8000);

  /* call uFSD init (set linkage) */
  if (!filetab_ptr)
#endif
  {
    fsd_init = (void *)(EXT_BUF_BASE); // uFSD base address
    fsd_init(l1);
  }
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

  /* After process_cfg() current_partition is set to
     the same partition, on which config file is installed,
     i.e., install_partition. We set the latter to the first. */
  install_partition = current_partition;
  saved_partition = current_partition;
  saved_drive = current_drive;
  saved_slice = current_slice;
  /* determine a drive string for '@' config file macro */
  determine_boot_drive();
  grub_strcat(freeldr_path, at_drive, preldr_path);

  relshift = 0;
  //init_term();
  /* use putchar() implementation through term blackbox */
  //use_term = 1;

  /* move uFSD */
  grub_memmove((char *)(EXT3HIBUF_BASE),
               (char *)(EXT_BUF_BASE),
               EXT_LEN);

  /* build filesystem driver .rel file path */
  grub_strcpy(str, freeldr_path);
  i = grub_strlen(freeldr_path);
  grub_strcpy(str + i, fsd_dir);
  k = grub_strlen(fsd_dir);
  grub_strcpy(str + i + k, install_filesys);
  l = grub_strlen(install_filesys);
  grub_strcpy(str + i + k + l, ".rel\0");

  /* fixup uFSD */
  reloc((char *)(EXT3HIBUF_BASE), str, EXT3HIBUF_BASE - EXT_BUF_BASE + SHIFT);

  /* call uFSD init (set linkage) */
  //if (!filetab_ptr)
  //{
    fsd_init = (void *)(EXT3HIBUF_BASE); // uFSD base address
    fsd_init(l1);
  //}

  // backup uFSD
  grub_memmove((void *)(UFSD_BASE), (void *)(EXT3HIBUF_BASE), EXT_LEN);

  /* Init terminal */
  init_term();
  use_term = 1;
  /* Init info in mbi structure */
  init_bios_info();

  /* Set filetable values (freeldr.mdl) */
  set_ftable(0);

  if (conf.multiboot == 1) {
    /* return to loader from protected mode */
    unsigned long ldr_base = conf.loader.base;
    printf("]");

    __asm {
      mov  dx, boot_flags
      mov  dl, byte ptr boot_drive

      // edi == pointer to filetable
      lea  edi, ft

      // esi ==  pointer to BPB
      mov  esi, BOOTSEC_BASE + 0xb

      //push ldr_base

      //retn

      // magic in eax
      mov  eax, BOOT_MAGIC

      // ebx == pointer to LIP
      mov  ebx, l2

      push ldr_base

      call [esp]
      add  esp, 4
    }
    /* Relocate itself to an address before os2ldr */
    set_addr();
    /* Init terminal */
    init_term();
    use_term = 1;
    /* Set filetable values for os2ldr */
    set_ftable(1);
  }
#else
  /* Load a pre-loader full version */
  rc = freeldr_open(preldr);

  buf = (char *)(0x10000);

  if (rc) {
    rc = freeldr_read(buf, -1);
    if (!rc) {
      __asm {
        cli
        hlt
      }
    }
  } else {
    __asm {
      cli
      hlt
    }
  }
#endif
}

#pragma aux init     "_*"
#pragma aux real_test "*"
#pragma aux call_rm   "*"
#pragma aux printmsg  "*"
#pragma aux printb    "*"
#pragma aux printw    "*"
#pragma aux printd    "*"
