/*  terminal blackbox init
 *  (setting term_entry)
 */

#include "term.h"
#include "fsd.h"
#include "lip.h"

#undef putchar

#pragma aux relshift    "*"
#pragma aux base        "*"
#pragma aux base32      "*"
#pragma aux set_gdt     "*"

#pragma aux u_msg "*"

void set_gdt(char *);

extern base;
struct term_entry *t;
extern unsigned int relshift;
unsigned int base32;
unsigned int s;

void (*disk_read_func) (int, int, int);
void (*disk_read_hook) (int, int, int);

#ifdef TERM_GRAPHICS
int __cdecl console_checkkey (void);
int __cdecl console_getkey (void);
#pragma aux console_checkkey "*"
#pragma aux console_getkey   "*"
#endif

int    (*pdevread)      (int sector, int byte_offset, int byte_len, char *buf);
int    (*prawread)      (int drive, int sector, int byte_offset, int byte_len, char *buf);
int    (*psubstring)    (const char *s1, const char *s2);
int    (*pgrub_memcmp)  (const char *s1, const char *s2, int n);
void * (*pgrub_memmove) (void *_to, const void *_from, int _len);
void * (*pgrub_memcpy) (void *_to, const void *_from, int _len);
void * (*pgrub_memset)  (void *start, int c, int len);
int    (*pgrub_strcmp)  (const char *, const char *);
int    (*pgrub_isspace) (int c);
int    (*pgrub_tolower) (int c);
int    (*pgrub_open)    (char *filename);
int    (*pgrub_read)    (char *buf, int len);
int    (*pgrub_seek)    (int offset);
int    (*pgrub_dir)     (char *dir);
void   (*pgrub_close)   (void);
void   (*pgrub_printf)  (const char *format,...);

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

void init(lip1_t *l, struct term_entry *te, unsigned int shift, char *gdt)
{
   unsigned int *p;
   t = te;

   /* Do a linkage between FSD and other parts of loader */
   disk_read_func = disk_read_hook = 0;

   /* Set pointers to external variables */
   pdevread      = l->lip_devread;
   prawread      = l->lip_rawread;
   psubstring    = l->lip_substring;
   pgrub_memcmp  = l->lip_memcmp;
   pgrub_memmove = l->lip_memmove;
   pgrub_memset  = l->lip_memset;
   pgrub_strcmp  = l->lip_strcmp;
   pgrub_isspace = l->lip_isspace;
   pgrub_tolower = l->lip_tolower;
   pgrub_open    = l->lip_open;
   pgrub_read    = l->lip_read;
   pgrub_seek    = l->lip_seek;
   pgrub_dir     = l->lip_dir;
   pgrub_close   = l->lip_close;

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

#ifdef TERM_CONSOLE
  te->flags         = 0;
  te->putchar       = &console_putchar;
  te->checkkey      = &console_checkkey;
  te->getkey        = &console_getkey;
  te->getxy         = &console_getxy;
  te->gotoxy        = &console_gotoxy;
  te->cls           = &console_cls;
  te->setcolorstate = &console_setcolorstate;
  te->setcolor      = &console_setcolor;
  te->setcursor     = &console_setcursor;
#else
#ifdef TERM_HERCULES
  te->flags         = 0;
  te->putchar       = &hercules_putchar;
  te->checkkey      = &console_checkkey;
  te->getkey        = &console_getkey;
  te->getxy         = &hercules_getxy;
  te->gotoxy        = &hercules_gotoxy;
  te->cls           = &hercules_cls;
  te->setcolorstate = &hercules_setcolorstate;
  te->setcolor      = &hercules_setcolor;
  te->setcursor     = &hercules_setcursor;
#else
#ifdef TERM_SERIAL
  te->flags         = TERM_NEED_INIT;
  te->putchar       = &serial_putchar;
  te->checkkey      = &serial_checkkey;
  te->getkey        = &serial_getkey;
  te->getxy         = &serial_getxy;
  te->gotoxy        = &serial_gotoxy;
  te->cls           = &serial_cls;
  te->setcolorstate = &serial_setcolorstate;
  te->setcolor      = 0;
  te->setcursor     = 0;
  te->startup       = &serial_init;
  te->shutdown      = 0;
#else
#ifdef TERM_GRAPHICS
  te->flags         = TERM_NEED_INIT;
  te->putchar       = &graphics_putchar;
  te->checkkey      = &console_checkkey;
  te->getkey        = &console_getkey;
  te->getxy         = &graphics_getxy;
  te->gotoxy        = &graphics_gotoxy;
  te->cls           = &graphics_cls;
  te->setcolorstate = &graphics_setcolorstate;
  te->setcolor      = &graphics_setcolor;
  te->setcursor     = &graphics_setcursor;
  te->startup       = &graphics_init;
  te->shutdown      = &graphics_end;
#endif
#endif
#endif
#endif
  // remember a pre-loader relocation shift
  relshift = shift;
  // base is located in 16-bit realmode segment.
  // it is a base of segment before relocation.
  // adding shift to it gives us its value after relocation.
  s = 0x10; // base offset
  p = (unsigned int *)((char *)s + (TERMLO_BASE + shift));
  base32 = *p;
  *p += shift;
  base32 += shift;
  // set bases of GDT descriptors
  set_gdt(gdt);
}

void cmain(void)
{

}
