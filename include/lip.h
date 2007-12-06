/*
 *   Loader interface page
 *   The interface between loader parts
 *
 *   When loader starts a module (stage2 or uXFD or uFSD
 *   etc), the interface is similar to the interface between
 *   uFSD and OS/2 loader:
 *   1) ds:si --> BPB
 *   2) es:di --> LIP (instead of FileTable)
 */

#ifndef __LIP_H__
#define __LIP_H__

#include <uXFD/uXFD.h>
#include <shared.h>

typedef struct lip
{
   /* Filesystem access functions */
   int                   (*lip_open)  (char *);
   int                   (*lip_dir)   (char *dirname);
   int                   (*lip_read)  (char *, int);
   int                   (*lip_seek)  (int);
   void                  (*lip_close) (void);
   void                  (*lip_term)  (void);
   /* uFSD functions */
   int                   (*lip_fs_mount) (void);
   int                   (*lip_fs_read)  (char *buf, int len);
   int                   (*lip_fs_dir)   (char *dirname);
   void                  (*lip_fs_close) (void);
   int                   (*lip_fs_embed) (int *start_sector, int needed_sectors);
   /* Load function     */
   int                   (*lip_load) (char *, unsigned long, char *, struct exe_params *);
   /* Utility functions */
   int                   (*lip_memcheck) (unsigned int, int);
   void *                (*lip_memset)   (void *, int, int);
   void *                (*lip_memmove)  (void *, const void *, int);
   char *                (*lip_strcpy)   (char *, const char *);
   int                   (*lip_strcmp)   (const char *, const char *);
   int                   (*lip_memcmp)   (const char *, const char *, int);
   int                   (*lip_strlen)   (const char *);
   int                   (*lip_isspace)  (int c);
   int                   (*lip_tolower)  (int c);
   int                   (*lip_substring)(const char *s1, const char *s2);
   int                   (*lip_pos)      (const char, const char *);
   void          __cdecl (*lip_clear)    (void);

   /* Sector read functions */
   int                   (*lip_devread)  (int sector, int byte_offset, int byte_len, char *buf);
   int                   (*lip_rawread)  (int drive, int sector, int byte_offset, int byte_len, char *buf);

   /* Stage0 variables */
   int                   *lip_mem_lower;
   int                   *lip_mem_upper;
   /* filesystem common variables */
   int                   *lip_filepos;
   int                   *lip_filemax;
   /* disk buffer parameters */
   int                   *lip_buf_drive;
   int                   *lip_buf_track;
   struct geometry       *lip_buf_geom;

   grub_error_t          *lip_errnum;

   unsigned long         *lip_saved_drive;
   unsigned long         *lip_saved_partition;

   unsigned long         *lip_current_drive;
   unsigned long         *lip_current_partition;
   int                   *lip_current_slice;
   unsigned long         *lip_part_start;
   unsigned long         *lip_part_length;
   int                   *lip_fsmax;

   /* Misc functions */
   void                  (*lip_printmsg) (char *);
   void                  (*lip_printb)   (unsigned char);
   void                  (*lip_printw)   (unsigned short);
   void                  (*lip_printd)   (unsigned long);

} lip_t;


#endif
