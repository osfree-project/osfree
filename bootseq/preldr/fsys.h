/*
 *   fsys.h
 */

#include <lip.h>

void printmsg(char *);
void printb(unsigned char);
void printw(unsigned short);
void printd(unsigned long);

extern char *preldr_path; // preldr files path
extern char *fsd_dir;     // uFSD's dir
extern char *cfg_file;    // preldr config file name
extern char *fsys_list[]; // file systems list
extern int  num_fsys;

extern lip_t *l;

int __cdecl (*fsd_init)(lip_t *l);

void panic(char *msg, char *file);

#pragma aux mu_Open           "*"
#pragma aux mu_Read           "*"
#pragma aux mu_Close          "*"
#pragma aux mu_Terminate      "*"

#pragma aux ft                "*"
#pragma aux boot_drive        "*"
#pragma aux boot_flags        "*"
#pragma aux install_partition "*"

#pragma aux l                 "*"

#pragma aux printmsg "*"
#pragma aux printb   "*"
#pragma aux printw   "*"
#pragma aux printd   "*"

#pragma aux filemax    "*"
#pragma aux grub_open  "*"
#pragma aux grub_read  "*"
#pragma aux grub_seek  "*"
#pragma aux grub_close "*"
#pragma aux freeldr_open  "*"
#pragma aux freeldr_read  "*"
#pragma aux freeldr_seek  "*"
#pragma aux freeldr_close "*"

int  stage0_mount (void);
int  stage0_read (char *buf, int len);
int  stage0_dir (char *dirname);
void stage0_close(void);
int  stage0_embed(int *start_sector, int needed_sectors);

int  freeldr_open (char *filename);
int  freeldr_read (char *buf, int len);
int  freeldr_seek (int offset);
void freeldr_close (void);

int  grub_open (char *filename);
int  grub_read (char *buf, int len);
int  grub_seek (int offset);
void grub_close (void);

extern int    (*pdevread) (int sector, int byte_offset, int byte_len, char *buf);
extern int    (*prawread) (int drive, int sector, int byte_offset, int byte_len, char *buf);
extern int    (*psubstring)    (const char *s1, const char *s2);
extern int    (*pgrub_memcmp)  (const char *s1, const char *s2, int n);
extern void * (*pgrub_memmove) (void *_to, const void *_from, int _len);
extern void * (*pgrub_memset)  (void *start, char c, int len);
extern int    (*pgrub_strcmp)  (const char *, const char *);
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
