/*
 *   fsys.h
 */

extern int    (*pdevread) (int sector, int byte_offset, int byte_len, char *buf);
extern int    (*prawread) (int drive, int sector, int byte_offset, int byte_len, char *buf);
extern int    (*psubstring)    (const char *s1, const char *s2);
extern int    (*pgrub_memcmp)  (const char *s1, const char *s2, int n);
extern void * (*pgrub_memmove) (void *_to, const void *_from, int _len);
extern void * (*pgrub_memset)  (void *start, char c, int len);
extern int    (*pgrub_isspace) (int c);
extern int    (*pgrub_tolower) (int c);
extern int    (*pgrub_read)    (char *buf, int len);

extern int *pmem_lower; // = 639;
extern int *pmem_upper; // = 16384;

/* filesystem common variables */
extern int *pfilepos;
extern int *pfilemax;

/* disk buffer parameters */
extern int             *pbuf_drive; // = -1;
extern int             *pbuf_track;
extern struct geometry *pbuf_geom;

extern grub_error_t    *perrnum;
//int print_possibilities;

extern unsigned long *psaved_drive;
extern unsigned long *psaved_partition;

extern unsigned long *pcurrent_drive;
extern unsigned long *pcurrent_partition;
extern int           *pcurrent_slice;
extern unsigned long *ppart_start;
extern unsigned long *ppart_length;
extern int           *pfsmax;

//extern int      debug = 0;
extern struct geometry *pbuf_geom;
