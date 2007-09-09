/*
 *   Loader interface page
 *   The interface between loader parts
 *
 *   When loader starts a module (stage1 or uXFD or uFSD
 *   etc), the interface is similar to the interface between
 *   uFSD and OS/2 loader:
 *   1) ds:si --> BPB
 *   2) es:di --> LIP (instead of FileTable)
 */

#ifndef __LIP_H__
#define __LIP_H__

#include <uXFD/uXFD.h>

typedef struct
{
   /* Filesystem access functions */
   unsigned long __cdecl (far *lip_open)  (char far *);
   unsigned long __cdecl (far *lip_read)  (unsigned long, unsigned long);
   unsigned long __cdecl (far *lip_seek)  (unsigned long);
   void          __cdecl (far *lip_close) (void);
   void          __cdecl (far *lip_term)  (void);
   /* Load function     */
   int                   (far *lip_load) (unsigned long, unsigned long, unsigned long, struct exe_params far *);
   /* Utility functions */
   long                  (far *lip_memcheck) (unsigned long, long);
   void far *            (far *lip_memset)   (void far *, char, long);
   void far *            (far *lip_memmove)  (void far *, const void far *, long);
   unsigned long         (far *lip_memmove_phys) (unsigned long, unsigned long, long);
   char far *            (far *lip_strcpy)   (char far *, const char far *);
   long                  (far *lip_strcmp)   (const char far *, const char far *);
   long                  (far *lip_memcmp)   (const char far *, const char far *, long);
   int                   (far *lip_strlen)   (const char far *);
   int                   (far *lip_pos)      (const char, const char far *);
   void          __cdecl (far *lip_clear)    (void);
   /* Misc functions */
   int           __cdecl (far *lip_printk)   (const char far *, ...);
} lip_t;


#endif
