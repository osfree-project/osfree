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

int mem_lower;
int mem_upper = 16384;

/* filesystem common variables */
int filepos;
int filemax;

void (*disk_read_func) (int, int, int) = 0;
void (*disk_read_hook) (int, int, int) = 0;

extern unsigned long current_drive;
extern int           fsmax;

#ifndef STAGE1_5

extern char *strpos;
extern char at_drive[16];

#endif

extern grub_error_t errnum;
int print_possibilities;

int  saved_fsys_type;
int  saved_current_slice;
int  saved_fsmax;
int  saved_buf_drive;
int  saved_buf_track;
struct geometry saved_buf_geom;
unsigned long   saved_current_partition;
unsigned long   saved_current_drive;
unsigned long   saved_cdrom;
unsigned long   saved_part_start;
unsigned long   saved_part_length;
unsigned long   saved_filemax;
unsigned long   saved_filepos;

extern unsigned long saved_drive;
extern unsigned long saved_partition;
extern unsigned long cdrom_drive;

int debug = 0;
struct geometry buf_geom;

/* filesystem type */
int fsys_type = -1;
#ifndef NO_BLOCK_FILES
static int block_file = 0;
#endif /* NO_BLOCK_FILES */

int
sane_partition (void);

//#pragma aux sane_partition "*"

/* FIXME: BSD evil hack */
#include "freebsd.h"
int bsd_evil_hack;

#if 0

struct fsys_entry fsys_table[NUM_FSYS + 1] =
{
  /* TFTP should come first because others don't handle net device.  */
# ifdef FSYS_TFTP
  {"tftp", tftp_mount, tftp_read, tftp_dir, tftp_close, 0},
# endif
# ifdef STAGE0
  {"stage0", stage0_mount, stage0_read, stage0_dir, 0, 0},
# endif
# ifdef FSYS_FAT
  {"fat", fat_mount, fat_read, fat_dir, 0, 0},
# endif
# ifdef FSYS_EXT2FS
  {"ext2fs", ext2fs_mount, ext2fs_read, ext2fs_dir, 0, 0},
# endif
# ifdef FSYS_MINIX
  {"minix", minix_mount, minix_read, minix_dir, 0, 0},
# endif
# ifdef FSYS_REISERFS
  {"reiserfs", reiserfs_mount, reiserfs_read, reiserfs_dir, 0, reiserfs_embed},
# endif
# ifdef FSYS_VSTAFS
  {"vstafs", vstafs_mount, vstafs_read, vstafs_dir, 0, 0},
# endif
# ifdef FSYS_JFS
  {"jfs", jfs_mount, jfs_read, jfs_dir, 0, jfs_embed},
# endif
# ifdef FSYS_XFS
  {"xfs", xfs_mount, xfs_read, xfs_dir, 0, 0},
# endif
# ifdef FSYS_UFS2
  {"ufs2", ufs2_mount, ufs2_read, ufs2_dir, 0, ufs2_embed},
# endif
# ifdef FSYS_ISO9660
  {"iso9660", iso9660_mount, iso9660_read, iso9660_dir, 0, 0},
# endif
  /* XX FFS should come last as it's superblock is commonly crossing tracks
     on floppies from track 1 to 2, while others only use 1.  */
# ifdef FSYS_FFS
  {"ffs", ffs_mount, ffs_read, ffs_dir, 0, ffs_embed},
# endif
  {0, 0, 0, 0, 0, 0}
};

#endif

/* These are defined in asm.S, and never be used elsewhere, so declare the
   prototypes here.  */
//extern int biosdisk_int13_extensions (int ax, int drive, void *dap);
//extern int biosdisk_standard (int ah, int drive,
//                              int coff, int hoff, int soff,
//                              int nsec, int segment);
//extern int check_int13_extensions (int drive);
//extern int get_diskinfo_standard (int drive,
//                                  unsigned long *cylinders,
//                                  unsigned long *heads,
//                                  unsigned long *sectors);


#ifndef STAGE1_5

void swap_fsys_bufs(void *buf1, void *buf2)
{
  unsigned long *b1 = (unsigned long *)buf1;
  unsigned long *b2 = (unsigned long *)buf2;
  unsigned long b;
  int n;

  // number of swapped dwords
  n = (EXT_LEN + 3) >> 2;

  while (n)
  {
    b   = *b1;
    *b1 = *b2;
    *b2 = b;

    b1++; b2++; n--;
  }
}

void set_boot_fsys(void)
{
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

  //saved_fsys_type     = fsys_type;
  //saved_current_drive = current_drive;
  //saved_current_partition = current_partition;
  //saved_cdrom = cdrom_drive;
  //saved_current_slice = current_slice;
  //saved_fsmax = fsmax;
  //saved_part_start  = part_start;
  //saved_part_length = part_length;
  //saved_filemax   = filemax;
  //saved_filepos   = filepos;
  //saved_buf_drive = buf_drive;
  //saved_buf_track = buf_track;
  //grub_memmove(&saved_buf_geom, &buf_geom, sizeof(struct geometry));

  fst = fsys_type;
  set_boot_fsys();
  rc = freeldr_open(fsname);

  buf = (char *)EXT4HIBUF_BASE;

  if (rc)
    rc = freeldr_read(buf, -1);
  else
    panic("can't open filesystem: ", fsname);

  /* clear the BSS of the uFSD */
  //start = *((unsigned long *)(buf + 2));
  //len   = *((unsigned long *)(buf + 6)) - start;
  //memset((void *)start, 0, len);

  //printmsg("uFSD file read, size: %d\r\n", rc);
  //printd(rc);
  //printmsg("\r\n");

  grub_strcpy(sbuf, fsname);
  s = grub_strstr(sbuf, ".mdl") + 1;
  // Change ".fsd" extension to ".rel"
  grub_strcpy(s, "rel");
  //swap_fsys_bufs((void *)EXT3HIBUF_BASE, (void *)UFSD_BASE);

  // fixup the loaded filesystem
  reloc(buf, sbuf, EXT3HIBUF_BASE - EXT_BUF_BASE + SHIFT);
  //printmsg("fs relocated\r\n");
  //swap_fsys_bufs((void *)(EXT3HIBUF_BASE), (void *)UFSD_BASE);
  //swap_fsys_bufs((void *)(EXT3HIBUF_BASE), buf);
  grub_memmove((void *)(EXT3HIBUF_BASE), (void *)(buf), EXT_LEN);
  //printmsg("fs moved\r\n");
  //fsys_type = saved_fsys_type;
  //current_drive = saved_current_drive;
  //current_partition = saved_current_partition;
  //cdrom_drive = saved_cdrom;
  //current_slice = saved_current_slice;
  //fsmax = saved_fsmax;
  //part_start = saved_part_start;
  //part_length = saved_part_length;
  //filemax = saved_filemax;
  //filepos = saved_filepos;
  //buf_drive = saved_buf_drive;
  //buf_track = saved_buf_track;
  //grub_memmove(&buf_geom, &saved_buf_geom, sizeof(struct geometry));
  fsys_type = fst;

  fsd_init = (void *)(EXT3HIBUF_BASE);
  fsd_init(l1);

  printmsg(".\r\n");
  //printd(fsys_type);
  //printmsg("\r\n");

  //printmsg("trying to mount a filesystem\r\n");
  //if (open_partition() && stage0_mount())
  //{
  //  printmsg("filesystem is mounted\r\n");
  //  return 1;
  //}


  //printmsg("filesystem is not mounted!\r\n");
  return 0;
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

#endif /* ! STAGE1_5 */

int
attempt_mount (void)
{

#ifndef STAGE1_5
  char buf[0x100];
  char *s;
  char *fsys_name;
  int  m, k, t, rc;
/*
  saved_fsys_type     = fsys_type;
  saved_current_drive = current_drive;
  saved_cdrom = cdrom_drive;
  saved_current_slice = current_slice;
  saved_current_partition = current_partition;
  saved_fsmax = fsmax;
  saved_part_start  = part_start;
  saved_part_length = part_length;
  saved_filemax   = filemax;
  saved_filepos   = filepos;
  saved_buf_drive = buf_drive;
  saved_buf_track = buf_track;
  grub_memmove(&saved_buf_geom, &buf_geom, sizeof(struct geometry));
 */
/*
  __asm {
    push  fsys_type
    push  current_drive
    push  cdrom_drive
    push  current_slice
    push  current_partition
    push  fsmax
    push  part_start
    push  part_length
    push  filemax
    push  filepos
    push  buf_drive
    push  buf_track
  }
 */

  //set_boot_fsys();
  //if (!stage0_mount())
  //{
  //  for (fsys_type = 0; fsys_type < num_fsys; fsys_type++) {
  //    fsys_by_num(fsys_type, buf);
  //
  //    if (set_fsys(buf))
  //      break;
  //  }
  //}

  return stage0_mount();

/*
  if (!stage0_mount())
  {
    errnum = ERR_FSYS_MOUNT;
    fsys_type = NUM_FSYS;
  }
 */
//  if (fsys_type == num_fsys && errnum == ERR_NONE)
//    errnum = ERR_FSYS_MOUNT;
  //if (fsys_type == NUM_FSYS && errnum == ERR_NONE)
  //  errnum = ERR_FSYS_MOUNT;
/*
  __asm {
    pop   buf_track
    pop   buf_drive
    pop   filepos
    pop   filemax
    pop   part_length
    pop   part_start
    pop   fsmax
    pop   current_partition
    pop   current_slice
    pop   cdrom_drive
    pop   current_drive
    pop   fsys_type
  }
 */
/*
  fsys_type = saved_fsys_type;
  current_drive = saved_current_drive;
  cdrom_drive = saved_cdrom;
  current_slice = saved_current_slice;
  current_partition = saved_current_partition;
  fsmax = saved_fsmax;
  part_start = saved_part_start;
  part_length = saved_part_length;
  filemax = saved_filemax;
  filepos = saved_filepos;
  buf_drive = saved_buf_drive;
  buf_track = saved_buf_track;
  grub_memmove(&buf_geom, &saved_buf_geom, sizeof(struct geometry));
 */
#else
  //fsys_type = 0;
  //if ((*(fsys_table[fsys_type].mount_func)) () != 1)
  //if (!stage0_mount())
  //  {
  //    fsys_type = NUM_FSYS;
  //    errnum = ERR_FSYS_MOUNT;
  //  }
  return stage0_mount();
#endif
}


/*
 *  This performs a "mount" on the current device, both drive and partition
 *  number.
 */
int
open_device (void)
{
  //if (open_partition ())
  //  attempt_mount ();
  if (!open_partition () || !attempt_mount ())
    return 0;

  if (errnum != ERR_NONE)
    return 0;

  return 1;

  //return open_partition() && attempt_mount();
}

/* open_device() wrapper. Attempts to set a filesystem
 * in loop, and try to mount it
 */
int
open_device2(void)
{
#ifndef STAGE1_5
  char buf[0x100];
  int fst;
  unsigned int cd = current_drive;
  unsigned int cp = current_partition;
  unsigned int fp = filepos;
  unsigned int fm = filemax;

  set_boot_fsys();
  if (open_device())
    return 1;

  for (fst = 0; fst < num_fsys; fst++) {
    fsys_type = fst;
    fsys_by_num(fst, buf);
    set_fsys(buf);

    current_drive = cd;
    current_partition = cp;
    filepos = fp;
    filemax = fm;

    if (open_device())
      return 1;
  }
#else
  return open_device();
#endif
  errnum = ERR_FSYS_MOUNT;
  return 0;
}


static char *
setup_part (char *filename)
{
#ifdef STAGE1_5

  if (! (filename = set_device (filename)))
    {
      current_drive = GRUB_INVALID_DRIVE;
      return 0;
    }

# ifndef NO_BLOCK_FILES
  if (*filename != '/')
    open_partition ();
  else
# endif /* ! NO_BLOCK_FILES */
    open_device ();

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
#define buf_size 1500
//const int buf_size = 1500;
  const char *try_filenames[] = { "menu.lst", "m" };

  char fn[buf_size]; /* arbitrary... */
  char *filename_orig = filename;
  int trycount = 0;

  //if (grub_strlen(filename) > buf_size)
  //  {
  //    printf("Buffer overflow: %s(%d)\n", __FILE__, __LINE__);
  //    while (1) {}
  //  }

  /* initially, we need to copy filename to fn */
  grub_strcpy(fn, filename_orig);
  fn[grub_strlen(filename_orig)] = 0;

restart:
  errnum = 0; /* hrm... */
  filename = fn;
#endif

#ifndef NO_DECOMPRESSION
  compressed_file = 0;
#endif /* NO_DECOMPRESSION */

  /* if any "dir" function uses/sets filepos, it must
     set it to zero before returning if opening a file! */
  filepos = 0;

  if (!(filename = (char *)setup_part (filename)))
    {
      if (errnum == ERR_FILE_NOT_FOUND)
        goto retry;
      else
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
          safe_parse_maxint (&ptr, &tmp);
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

          if (!safe_parse_maxint (&ptr, &tmp)
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

retry:
# ifndef STAGE1_5
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

/* Reposition a file offset.  */
int
grub_seek (int offset)
{
  if (offset > filemax || offset < 0)
    return -1;

  filepos = offset;
  return offset;
}

#ifndef STAGE1_5
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
#endif /* STAGE1_5 */

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
