/*

gbmhelp.h - Internal helpers for GBM file I/O stuff

*/

#ifndef GBMHELP_H
#define	GBMHELP_H

#if defined(AIX) || defined(LINUX) || defined(SUN) || defined(MACOSX) || defined(IPHONE)
#include <unistd.h>
#else
#include <io.h>
#endif
#include <fcntl.h>
#ifdef MAC
#include <types.h>
#include <stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

/* Functions pointers for process IO rerouting */
extern int  (GBMENTRYP gbmio_file_open  )(const char *fn, int mode);
extern int  (GBMENTRYP gbmio_file_create)(const char *fn, int mode);
extern void (GBMENTRYP gbmio_file_close )(int fd);
extern long (GBMENTRYP gbmio_file_lseek )(int fd, long pos, int whence);
extern int  (GBMENTRYP gbmio_file_read  )(int fd, void *buf, int len);
extern int  (GBMENTRYP gbmio_file_write )(int fd, const void *buf, int len);
extern void gbm_restore_file_io(void);

extern int  gbm_file_open             (const char *fn, int mode);
extern int  gbm_file_open_from_codec  (const char *fn, int mode, int bitmap_fd);
extern int  gbm_file_create           (const char *fn, int mode);
extern int  gbm_file_create_from_codec(const char *fn, int mode, int bitmap_fd);
extern void gbm_file_close            (int fd);
extern long gbm_file_lseek            (int fd, long pos, int whence);
extern int  gbm_file_read             (int fd, void *buf, int len);
extern int  gbm_file_write            (int fd, const void *buf, int len);

extern gbm_boolean gbm_same(const char *s1, const char *s2, int n);
extern const char *gbm_find_word(const char *str, const char *substr);
extern const char *gbm_find_word_prefix(const char *str, const char *substr);

typedef void AHEAD;
extern AHEAD *gbm_create_ahead (int fd);
extern void   gbm_destroy_ahead(AHEAD *ahead);
extern int    gbm_look_ahead(AHEAD *ahead);
extern long   gbm_lseek_ahead  (AHEAD *ahead, long pos, int whence);
extern int    gbm_read_ahead   (AHEAD *ahead);
extern int    gbm_readbuf_ahead(AHEAD *ahead, gbm_u8 * buf, int buflen);

typedef void WCACHE;
extern WCACHE *gbm_create_wcache  (int fd);
extern int     gbm_destroy_wcache (WCACHE *wcache);
extern int     gbm_write_wcache   (WCACHE *wcache, gbm_u8 c);
extern int     gbm_writebuf_wcache(WCACHE *wcache, const gbm_u8 * buf, int buflen);

#endif

