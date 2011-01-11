/*
 *
 *
 */

#include <shared.h>
#include <filesys.h>
#include <lip.h>
#include "fsys.h"
#include "fsd.h"

int __cdecl (*fsd_init)(lip1_t *l1);

/* filesystem common variables */
int filepos;
int filemax;

void (*disk_read_func) (int, int, int) = 0;
void (*disk_read_hook) (int, int, int) = 0;

int open_partition_hiddensecs(void);

unsigned long sector_size; // = SECTOR_SIZE;

#ifndef STAGE1_5

extern int buf_drive;
extern char *preldr_path;

#pragma aux filetab_ptr "*"
extern unsigned long filetab_ptr;

// max count of file aliases
#define MAX_ALIAS 0x10

/* Configuration got
   from .INI file      */
extern _Packed struct {
  unsigned char driveletter;
  unsigned char multiboot;
  unsigned char root;
  struct {
    char microfsd;
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
} conf;

extern unsigned long current_drive;
extern unsigned long current_partition;
extern int current_slice;
extern int num_fsys;
extern int fsys_type;
extern int           fsmax;

extern char install_filesys[0x10];
#pragma aux install_filesys "*"
//#pragma aux mem_lower "*"

//int mem_lower;
//int mem_upper = 16384;

extern char *strpos;
extern char at_drive[16];
extern int do_completion;

#endif

extern grub_error_t errnum;
extern int print_possibilities;

extern unsigned long saved_drive;
extern unsigned long saved_partition;
extern int saved_slice;
extern int saved_fsys_type;
extern unsigned long cdrom_drive;

/* filesystem type */
int fsys_type = -1;
int saved_fsys_type = -1;
//unsigned long old_slice = -1;
//unsigned long old_drive = -1;
//unsigned long old_partition = -1;
#ifndef NO_BLOCK_FILES
static int block_file = 0;
#endif /* NO_BLOCK_FILES */

int
sane_partition (void);

//#pragma aux sane_partition "*"

/* FIXME: BSD evil hack */
#include "freebsd.h"
//int bsd_evil_hack;

#ifndef STAGE1_5

/* Print the root device information.  */
void
print_root_device (void)
{
  if (saved_drive == NETWORK_DRIVE)
    {
      /* Network drive.  */
      grub_printf (" (nd):");
    }
  else if (saved_drive == cdrom_drive)
         grub_printf (" (cd):");
  else if (saved_drive & 0x80)
    {
      /* Hard disk drive.  */
      grub_printf (" (hd%d", saved_drive - 0x80);

      if ((saved_partition & 0xFF0000) != 0xFF0000)
        grub_printf (",%d", saved_partition >> 16);

      if ((saved_partition & 0x00FF00) != 0x00FF00)
        grub_printf (",%c", ((saved_partition >> 8) & 0xFF) + 'a');

      grub_printf ("):");
    }
  else
    {
      /* Floppy disk drive.  */
      grub_printf (" (fd%d):", saved_drive);
    }

  /* Print the filesystem information.  */
  current_partition = saved_partition;
  current_drive = saved_drive;
  current_slice = saved_slice;
  fsys_type = saved_fsys_type;
  print_fsys_type ();
}

/*
 *  This prints the filesystem type or gives relevant information.
 */

void
print_fsys_type (void)
{
  if (! do_completion)
    {
      printf (" Filesystem type ");

      if (fsys_type == -1) // the boot one
        printf ("is %s, ", install_filesys);
      else if (fsys_type != num_fsys)
        printf ("is %s, ", fsys_list[fsys_type]);
      else
        printf ("unknown, ");

      if (current_partition == 0xFFFFFF)
        printf ("using whole disk\n");
      else
        printf ("partition type 0x%x\n", current_slice & 0xFF);
    }
}

void set_boot_fsys(void)
{
  saved_fsys_type = fsys_type;
  fsys_type = -1; // boot filesystem
  /* move boot drive uFSD to working buffer */
  //swap_fsys_bufs((void *)(EXT3HIBUF_BASE), (void *)(UFSD_BASE));
  grub_memmove((void *)(EXT3HIBUF_BASE), (void *)(UFSD_BASE), EXT_LEN);
  /* call uFSD init (set linkage) */
  fsd_init = (void *)(EXT3HIBUF_BASE); // uFSD base address
  fsd_init(l1);
  //printmsg("boot fs set\r\n");
}

#pragma aux set_fsys "*"

int set_fsys(char *fsname)
{
  char *buf; //[EXT_LEN];
  char sbuf[0x100];
  char *s;
  int  rc;
  int  fst;
  int  start, len;

  fst = fsys_type;
  if (!filetab_ptr) set_boot_fsys();
  rc = freeldr_open(fsname);

  buf = (char *)EXT4HIBUF_BASE;

  if (rc)
  {
    rc = freeldr_read(buf, -1);
    freeldr_close();
  }
  else
    return 0;

  /* clear the BSS of the uFSD */
  grub_strcpy(sbuf, fsname);
  s = grub_strstr(sbuf, ".mdl") + 1;
  // Change ".fsd" extension to ".rel"
  grub_strcpy(s, "rel");
  reloc(buf, sbuf, EXT3HIBUF_BASE - EXT_BUF_BASE + SHIFT);
  grub_memmove((void *)(EXT3HIBUF_BASE), (void *)(buf), EXT_LEN);
  fsys_type = fst;

  fsd_init = (void *)(EXT3HIBUF_BASE);
  fsd_init(l1);

  printmsg(".\r\n");

  return 1;
}

#pragma aux fsys_by_num "*"

void fsys_by_num(int n, char *buf)
{
  char *fsys_name;
  int m, t, k;
  // buf -> path to filesystem driver
  m = grub_strlen(preldr_path);
  grub_memmove((void *)buf, preldr_path, m);
  t = grub_strlen(fsd_dir);
  grub_memmove((void *)(buf + m), fsd_dir, t);
  fsys_name = fsys_list[n];
  k = grub_strlen(fsys_name);
  grub_memmove((void *)(buf + m + t), fsys_name, k);
  grub_memmove((void *)(buf + m + t + k), ".mdl\0", 5);
}


/*
 *  This performs a "mount" on the current device, both drive and partition
 *  number.
 */
int
open_device (void)
{
  if (!(
        ((current_drive == boot_drive) &&
         (current_partition == 0xffffff) &&
         (boot_drive != cdrom_drive) &&
         (boot_drive >= 0x80) &&
         open_partition_hiddensecs () &&
         stage0_mount ()) ||
        (open_partition () &&
         stage0_mount ())
        ))
    return 0;

  if (errnum != ERR_NONE)
    return 0;

  return 1;
}

/* open_device() wrapper. Attempts to set a filesystem
 * in loop, and try to mount it
 */
int
open_device2(void)
{
  char buf[0x100];
  int fst;
  unsigned int cd = current_drive;
  unsigned int cp = current_partition;

  /* First try to open device by current FS */

  if (!filetab_ptr)
  {
    if (open_device())
      return 1;

    /* Then try the boot one */
    set_boot_fsys();

    if (open_device())
      return 1;
  }

  /* If that won't help, try each FS from list */

  for (fst = 0; fst < num_fsys; fst++)
  {
    fsys_type = fst;
    fsys_by_num(fst, buf);

    if (!set_fsys(buf))
      continue;

    current_drive = cd;
    current_partition = cp;

    if (open_device())
      return 1;
  }

  saved_fsys_type = fsys_type = -1;
  saved_drive = boot_drive;
  saved_partition = install_partition;

  errnum = ERR_FSYS_MOUNT;
  return 0;
}

#endif /* ! STAGE1_5 */

static char *
setup_part (char *filename)
{
#ifdef STAGE1_5

  //if (! (filename = set_device (filename)))
  //  {
  //    current_drive = GRUB_INVALID_DRIVE;
  //    return 0;
  //  }

# ifndef NO_BLOCK_FILES
  if (*filename != '/')
    open_partition ();
  else
# endif /* ! NO_BLOCK_FILES */
  {
    open_partition_hiddensecs ();
    stage0_mount ();
  }
#else /* ! STAGE1_5 */

  if (*filename == '(')
    {
      if ((filename = set_device (filename)) == 0)
        {
          current_drive = GRUB_INVALID_DRIVE;
          return 0;
        }
# ifndef NO_BLOCK_FILES
      if (*filename != '/')
        open_partition ();
      else
# endif /* ! NO_BLOCK_FILES */
        open_device2 ();
    }
  else if (saved_drive != current_drive
           || saved_partition != current_partition
           || (*filename == '/' && fsys_type == num_fsys)
           || buf_drive == -1)
    {
      current_drive = saved_drive;
      current_partition = saved_partition;
      current_slice = saved_slice;
      fsys_type = saved_fsys_type;
      /* allow for the error case of "no filesystem" after the partition
         is found.  This makes block files work fine on no filesystem */
# ifndef NO_BLOCK_FILES
      if (*filename != '/')
        open_partition ();
      else
# endif /* ! NO_BLOCK_FILES */
        open_device2 (); // 2
    }

#endif /* ! STAGE1_5 */

  if (errnum && (*filename == '/' || errnum != ERR_FSYS_MOUNT))
    return 0;
  else
    errnum = 0;

#ifndef STAGE1_5
  if (!sane_partition ())
    return 0;
#endif

  return filename;
}


/*
 *  This is the generic file open function.
 */
int
grub_open (char *filename)
{
#ifndef STAGE1_5
//#define buf_size 1500
//const int buf_size = 1500;
//  const char *try_filenames[] = { "menu.lst", "m" };

  //char fn[buf_size]; /* arbitrary... */
  //char *filename_orig = filename;
  //int trycount = 0;

  //if (grub_strlen(filename) > buf_size)
  //  {
  //    printf("Buffer overflow: %s(%d)\n", __FILE__, __LINE__);
  //    while (1) {}
  //  }

  /* initially, we need to copy filename to fn */
  //grub_strcpy(fn, filename_orig);
  //fn[grub_strlen(filename_orig)] = 0;

//restart:
  errnum = 0; /* hrm... */
  //filename = fn;
#endif

#ifndef NO_DECOMPRESSION
  compressed_file = 0;
#endif /* NO_DECOMPRESSION */

  /* if any "dir" function uses/sets filepos, it must
     set it to zero before returning if opening a file! */
  filepos = 0;

  if (!(filename = (char *)setup_part (filename)))
    {
      //if (errnum == ERR_FILE_NOT_FOUND)
      //  goto retry;
      //else
        return 0;
    }

#ifndef NO_BLOCK_FILES
  block_file = 0;
#endif /* NO_BLOCK_FILES */

  /* This accounts for partial filesystem implementations. */
  fsmax = MAXINT;

  if (*filename != '/')
    {
#ifndef NO_BLOCK_FILES
      char *ptr = filename;
      int tmp, list_addr = BLK_BLKLIST_START;
      filemax = 0;

      while (list_addr < BLK_MAX_ADDR)
        {
          tmp = 0;
          safe_parse_maxint (&ptr, (long *)&tmp);
          errnum = 0;

          if (*ptr != '+')
            {
              if ((*ptr && *ptr != '/' && !isspace (*ptr))
                  || tmp == 0 || tmp > filemax)
                errnum = ERR_BAD_FILENAME;
              else
                filemax = tmp;

              break;
            }

          /* since we use the same filesystem buffer, mark it to
             be remounted */
          //fsys_type = NUM_FSYS;
          fsys_type = num_fsys;

          BLK_BLKSTART (list_addr) = tmp;
          ptr++;

          if (!safe_parse_maxint (&ptr, (long *)&tmp)
              || tmp == 0
              || (*ptr && *ptr != ',' && *ptr != '/' && !isspace (*ptr)))
            {
              errnum = ERR_BAD_FILENAME;
              break;
            }

          BLK_BLKLENGTH (list_addr) = tmp;

          filemax += (tmp * SECTOR_SIZE);
          list_addr += BLK_BLKLIST_INC_VAL;

          if (*ptr != ',')
            break;

          ptr++;
        }

      if (list_addr < BLK_MAX_ADDR && ptr != filename && !errnum)
        {
          block_file = 1;
          BLK_CUR_FILEPOS = 0;
          BLK_CUR_BLKLIST = BLK_BLKLIST_START;
          BLK_CUR_BLKNUM = 0;

#ifndef NO_DECOMPRESSION
          return gunzip_test_header ();
#else /* NO_DECOMPRESSION */
          return 1;
#endif /* NO_DECOMPRESSION */
        }
#else /* NO_BLOCK_FILES */
      errnum = ERR_BAD_FILENAME;
#endif /* NO_BLOCK_FILES */
    }

  if (!errnum && fsys_type == num_fsys) // fsys_type == NUM_FSYS)
    errnum = ERR_FSYS_MOUNT;

# ifndef STAGE1_5
  /* set "dir" function to open a file */
  print_possibilities = 0;
# endif
                                // NUM_FSYS + 1
  if ( !errnum && stage0_dir(filename) )
     //  (!errnum && (*(fsys_table[fsys_type].dir_func)) (filename)) )
    {
#ifndef NO_DECOMPRESSION
      return gunzip_test_header ();
#else /* NO_DECOMPRESSION */
      return 1;
#endif /* NO_DECOMPRESSION */
    }

//retry:
# if 0 // ndef STAGE1_5
  if (trycount < sizeof(try_filenames) / sizeof(*try_filenames))
    {
      unsigned int l = grub_strlen(filename_orig);
      unsigned int ll = grub_strlen(try_filenames[trycount]);
      int j;

      grub_memmove(fn, filename_orig, l);

      if (filename_orig[l - 1] != '/')
        fn[l++] = '/';
      for (j = 0; j < ll; j++)
        fn[l + j] = try_filenames[trycount][j];
      fn[l + ll] = 0;

      //grub_printf("Previous try failed, trying \"%s\"\n", fn);
      trycount++;
      goto restart;
    }
# endif


  return 0;
}


int
grub_read (char *buf, int len)
{
  /* Make sure "filepos" is a sane value */
  if ((filepos < 0) || (filepos > filemax))
    filepos = filemax;

  /* Make sure "len" is a sane value */
  if ((len < 0) || (len > (filemax - filepos)))
    len = filemax - filepos;

  /* if target file position is past the end of
     the supported/configured filesize, then
     there is an error */
  if (filepos + len > fsmax)
    {
      errnum = ERR_FILELENGTH;
      return 0;
    }

#ifndef NO_DECOMPRESSION
  if (compressed_file)
    return gunzip_read (buf, len);
#endif /* NO_DECOMPRESSION */

#ifndef NO_BLOCK_FILES
  if (block_file)
    {
      int size, off, ret = 0;

      while (len && !errnum)
        {
          /* we may need to look for the right block in the list(s) */
          if (filepos < BLK_CUR_FILEPOS)
            {
              BLK_CUR_FILEPOS = 0;
              BLK_CUR_BLKLIST = BLK_BLKLIST_START;
              BLK_CUR_BLKNUM = 0;
            }

          /* run BLK_CUR_FILEPOS up to filepos */
          while (filepos > BLK_CUR_FILEPOS)
            {
              if ((filepos - (BLK_CUR_FILEPOS & ~(SECTOR_SIZE - 1)))
                  >= SECTOR_SIZE)
                {
                  BLK_CUR_FILEPOS += SECTOR_SIZE;
                  BLK_CUR_BLKNUM++;

                  if (BLK_CUR_BLKNUM >= BLK_BLKLENGTH (BLK_CUR_BLKLIST))
                    {
                      BLK_CUR_BLKLIST += BLK_BLKLIST_INC_VAL;
                      BLK_CUR_BLKNUM = 0;
                    }
                }
              else
                BLK_CUR_FILEPOS = filepos;
            }

          off = filepos & (SECTOR_SIZE - 1);
          size = ((BLK_BLKLENGTH (BLK_CUR_BLKLIST) - BLK_CUR_BLKNUM)
                  * SECTOR_SIZE) - off;
          if (size > len)
            size = len;

          disk_read_func = disk_read_hook;

          /* read current block and put it in the right place in memory */
          devread (BLK_BLKSTART (BLK_CUR_BLKLIST) + BLK_CUR_BLKNUM,
                   off, size, buf);

          disk_read_func = NULL;

          len -= size;
          filepos += size;
          ret += size;
          buf += size;
        }

      if (errnum)
        ret = 0;

      return ret;
    }
#endif /* NO_BLOCK_FILES */

                     // NUM_FSYS
  if (fsys_type == num_fsys)
    {
      errnum = ERR_FSYS_MOUNT;
      return 0;
    }

  return stage0_read(buf, len);
  //return (*(fsys_table[fsys_type].read_func)) (buf, len);
}

#ifndef STAGE1_5

/* Reposition a file offset.  */
int
grub_seek (int offset)
{
  if (offset > filemax || offset < 0)
    return -1;

  filepos = offset;
  return offset;
}

int
dir (char *dirname)
{
#ifndef NO_DECOMPRESSION
  compressed_file = 0;
#endif /* NO_DECOMPRESSION */

  if (!(dirname = setup_part (dirname)))
    return 0;

  if (*dirname != '/')
    errnum = ERR_BAD_FILENAME;
                   // NUM_FSYS
  if (fsys_type == num_fsys)
    errnum = ERR_FSYS_MOUNT;

  if (errnum)
    return 0;

  /* set "dir" function to list completions */
  print_possibilities = 1;

  //if (fsys_type == NUM_FSYS + 1)
  return stage0_dir(dirname);

  //return (*(fsys_table[fsys_type].dir_func)) (dirname);
}

void
grub_close (void)
{
#ifndef NO_BLOCK_FILES
  if (block_file)
    return;
#endif /* NO_BLOCK_FILES */

  //if (fsys_type == NUM_FSYS + 1)
  stage0_close();
  //else
  //  if (fsys_table[fsys_type].close_func != 0)
  //    (*(fsys_table[fsys_type].close_func)) ();
}

#endif /* STAGE1_5 */
