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


/* Loader Interface Page #1 (used internally in pre-loader) */
typedef struct lip1
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
   //int                   (*lip_load) (char *, unsigned long, char *, struct exe_params *);

   /* Utility functions */
   int                   (*lip_substring)(const char *s1, const char *s2);
   int                   (*lip_memcmp)   (const char *, const char *, int);
   void *                (*lip_memmove)  (void *, const void *, int);
   void *                (*lip_memset)   (void *, int, int);
   int                   (*lip_strcmp)   (const char *, const char *);
   char *                (*lip_strcpy)   (char *, const char *);
   int                   (*lip_strlen)   (const char *);
   int                   (*lip_isspace)  (int c);
   int                   (*lip_tolower)  (int c);
   //int                   (*lip_memcheck) (unsigned int, int);
   //int                   (*lip_pos)      (const char, const char *);
   //void          __cdecl (*lip_clear)    (void);

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

} lip1_t;

/* Loader Interface Page #2 (interface between loader and pre-loader) */
typedef struct lip2 lip2_t;
typedef struct lip2
{
  unsigned int           u_lip2magic;
  unsigned int __cdecl   (*u_open) (char *name, unsigned int *size);
  unsigned int __cdecl   (*u_read) (char *buf, unsigned int count);
  unsigned int __cdecl   (*u_seek) (int loffseek);
  void         __cdecl   (*u_close) (void);
  void         __cdecl   (*u_terminate) (void);
  int          __cdecl   (*u_diskctl) (int func, int drive, struct geometry *geometry, 
                                       int sector, int nsec, int addr);
  int          __cdecl   (*u_vbectl)  (int func, int mode_number, void *info);
  struct term_entry * __cdecl (*u_termctl) (int termno);
  int          __cdecl   (*u_boot) (int type);
  int          __cdecl   (*u_load) (char *image, unsigned int size, char *load_addr, struct exe_params *p);
  int          __cdecl   (*u_parm) (int parm, int action, unsigned int *val);
  void         __cdecl   (*u_msg)  (char *s);
  void         __cdecl   (*u_setlip) (lip2_t *l);
};

#define LIP2_MAGIC 0x3badb002

#endif
