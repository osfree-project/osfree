/*
 *
 *
 */

#include <shared.h>
#include <lip.h>
#include "misc.h"

//#include <filesys.h>

extern int  fs_mount(void);
extern int  fs_read(char *buf, int len);
extern int  fs_dir(char *dirname);

void (*disk_read_func) (int, int, int);
void (*disk_read_hook) (int, int, int);

int    (*pdevread)      (int sector, int byte_offset, int byte_len, char *buf);
int    (*prawread)      (int drive, int sector, int byte_offset, int byte_len, char *buf);
int    (*psubstring)    (const char *s1, const char *s2);
int    (*pgrub_memcmp)  (const char *s1, const char *s2, int n);
void * (*pgrub_memmove) (void *_to, const void *_from, int _len);
void * (*pgrub_memset)  (void *start, int c, int len);
int    (*pgrub_strcmp)  (const char *, const char *);
int    (*pgrub_isspace) (int c);
int    (*pgrub_tolower) (int c);
int    (*pgrub_read)    (char *buf, int len);

int  *pmem_lower;
int  *pmem_upper;

/* filesystem common variables */
int  *pfilepos;
int  *pfilemax;

/* disk buffer parameters */
int             *pbuf_drive;
int             *pbuf_track;
struct geometry *pbuf_geom;

grub_error_t    *perrnum;

unsigned long *psaved_drive;
unsigned long *psaved_partition;

unsigned long *pcurrent_drive;
unsigned long *pcurrent_partition;
int           *pcurrent_slice;
unsigned long *ppart_start;
unsigned long *ppart_length;
int           *pfsmax;

//int      debug = 0;
struct geometry *pbuf_geom;

int __cdecl
init(lip_t *l)
{
   /* Do a linkage between FSD and other parts of loader */
   disk_read_func = disk_read_hook = 0;

   /* Set pointers to external variables */
   pdevread = l->lip_devread;
   prawread = l->lip_rawread;
   psubstring    = l->lip_substring;
   pgrub_memcmp  = l->lip_memcmp;
   pgrub_memmove = l->lip_memmove;
   pgrub_memset  = l->lip_memset;
   pgrub_strcmp  = l->lip_strcmp;
   pgrub_isspace = l->lip_isspace;
   pgrub_tolower = l->lip_tolower;
   pgrub_read    = l->lip_read;

   pmem_lower  = l->lip_mem_lower;
   pmem_upper  = l->lip_mem_upper;

   pfilepos   = l->lip_filepos;
   pfilemax   = l->lip_filemax;

   pbuf_drive = l->lip_buf_drive;
   pbuf_track = l->lip_buf_track;
   pbuf_geom  = l->lip_buf_geom;

   perrnum    = l->lip_errnum;

   psaved_drive     = l->lip_saved_drive;
   psaved_partition = l->lip_saved_partition;

   pcurrent_drive     = l->lip_current_drive;
   pcurrent_partition = l->lip_current_partition;
   pcurrent_slice     = l->lip_current_slice;

   ppart_start    = l->lip_part_start;
   ppart_length   = l->lip_part_length;
   pfsmax         = l->lip_fsmax;

   /* Set pointers to our functions */
   l->lip_fs_mount = &fs_mount;
   l->lip_fs_read  = &fs_read;
   l->lip_fs_dir   = &fs_dir;
   l->lip_fs_close = 0;
   l->lip_fs_embed = 0;

   return 0;
}
