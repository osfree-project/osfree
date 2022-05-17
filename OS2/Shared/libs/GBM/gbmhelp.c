/*

gbmhelp.c - Helpers for GBM file I/O stuff

*/

/*...sincludes:0:*/
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
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
#include "gbm.h"
#include "gbmmem.h"

/* ---------------------------- */
/* ---------------------------- */

gbm_boolean gbm_same(const char *s1, const char *s2, size_t n)
{
    const size_t s1len = strlen(s1);
    const size_t s2len = strlen(s2);
    size_t i;

    for (i = 0; (i < n) && (i < s1len) && (i < s2len); i++, s1++, s2++ )
    {
        if ( tolower(*s1) != tolower(*s2) )
        {
            return GBM_FALSE;
        }
    }
    if (i < n)
    {
        return GBM_FALSE;
    }
    return GBM_TRUE;
}

/* ---------------------------- */

const char *gbm_find_word(const char *str, const char *substr)
{
         char        *buf, *s;
         char        strbuf[51] = { 0 };
         size_t      len            = strlen(substr);
   const size_t      strlength      = strlen(str);
   const gbm_boolean needsDynBuffer = (strlength > (sizeof(strbuf)-1));

   /* only use dynamic buffer if needed to speedup a bit */
   if (needsDynBuffer)
   {
     buf = (char *) gbmmem_calloc(strlength + 1, sizeof(char));
     if (buf == NULL)
     {
       return NULL;
     }
   }
   else
   {
     buf = strbuf;
   }
   for ( s  = strtok(strcpy(buf, str), " \t,");
         s != NULL;
         s  = strtok(NULL, " \t,") )
   {
     if ( gbm_same(s, substr, len) && s[len] == '\0' )
     {
       int inx = (int)(s - buf);
       if (needsDynBuffer)
       {
         gbmmem_free(buf);
       }
       return str + inx;
     }
   }
   if (needsDynBuffer)
   {
     gbmmem_free(buf);
   }
   return NULL;
}

/* ---------------------------- */

const char *gbm_find_word_prefix(const char *str, const char *substr)
{
         char        *buf, *s;
         char        strbuf[51] = { 0 };
         size_t      len            = strlen(substr);
   const size_t      strlength      = strlen(str);
   const gbm_boolean needsDynBuffer = (strlength > (sizeof(strbuf)-1));

   /* only use dynamic buffer if needed to speedup a bit */
   if (needsDynBuffer)
   {
     buf = (char *) gbmmem_calloc(strlength + 1, sizeof(char));
     if (buf == NULL)
     {
       return NULL;
     }
   }
   else
   {
     buf = strbuf;
   }
   for ( s  = strtok(strcpy(buf, str), " \t,");
         s != NULL;
         s  = strtok(NULL, " \t,") )
   {
     if ( gbm_same(s, substr, len) )
     {
       int inx = (int)(s - buf);
       if (needsDynBuffer)
       {
         gbmmem_free(buf);
       }
       return str + inx;
     }
   }
   if (needsDynBuffer)
   {
     gbmmem_free(buf);
   }
   return NULL;
}

/* ---------------------------- */

/* map supported public defines to compiler specific ones */
static int get_checked_internal_open_mode(int mode)
{
   const static int supported_open_modes = GBM_O_RDONLY |
                                           GBM_O_WRONLY |
                                           GBM_O_RDWR   |
                                           GBM_O_EXCL   |
                                           GBM_O_NOINHERIT;

   /* internal, compiler specific open mode */
   int open_mode = 0;

   /* map supported public defines to compiler specific ones */

   /* check if only supported modes are provided */
   if (!(mode & supported_open_modes))
   {
      return 0xffffffff;
   }

   /* mask external binary mode bit */
   mode &= ~GBM_O_BINARY;

   if (mode & GBM_O_RDONLY)
   {
      open_mode |= O_RDONLY;
   }
   else if (mode & GBM_O_WRONLY)
   {
      open_mode |= O_WRONLY;
   }
   else if (mode & GBM_O_RDWR)
   {
      open_mode |= O_RDWR;
   }
   else
   {
      return 0xffffffff;
   }

   if (mode & GBM_O_EXCL)
   {
      open_mode |= O_EXCL;
   }
   #ifdef O_NOINHERIT
     if (mode & GBM_O_NOINHERIT)
     {
        open_mode |= O_NOINHERIT;
     }
   #endif

   /* force binary mode if necessary */
   #ifdef O_BINARY
     open_mode |= O_BINARY;
   #endif

   return open_mode;
}

/* ---------------------------- */

#if defined(WIN32) && defined(_MSC_VER)
  /* Use ISO variants */
  #define	fd_creat _creat
  #define	fd_open  _open
  #define	fd_close _close
  #define	fd_read  _read
  #define	fd_write _write
  #define	fd_lseek _lseek
#else
  /* Use POSIX variants */
  #define	fd_creat creat
  #define	fd_open  open
  #define	fd_close close
  #define	fd_read  read
  #define	fd_write write
  #define	fd_lseek lseek
#endif

/* Looking at this, you might think that the gbm_file_* function pointers
   could be made to point straight at the regular read,write etc..
   If we do this then we get into problems with different calling conventions
   (for example read is _Optlink under C-Set++ on OS/2), and also where
   function arguments differ (the length field to read is unsigned on OS/2).
   This simplest thing to do is simply to use the following veneers. */

static int GBMENTRY def_open(const char *fn, int mode)
{
   const int internal_mode = get_checked_internal_open_mode(mode);

   /* In case of a mapping error we get 0xffffffff which is an illegal mode.
    * So let the OS take care for correct error reporting.
    */
   return fd_open(fn, internal_mode);
}

/* ---------------------------- */

static int GBMENTRY def_create(const char *fn, int mode)
{
   const int internal_mode = get_checked_internal_open_mode(mode);

   /* In case of a mapping error we get 0xffffffff which is an illegal mode.
    * So let the OS take care for correct error reporting.
    */

#ifdef MAC
   return open(fn, O_CREAT | O_TRUNC | internal_mode);
        /* S_IREAD and S_IWRITE won't exist on the Mac until MacOS/X */
#else
 #if defined(S_IWOTH) /* Unix like system */
   return fd_open(fn, O_CREAT | O_TRUNC | internal_mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
 #else
   return fd_open(fn, O_CREAT | O_TRUNC | internal_mode, S_IREAD | S_IWRITE);
 #endif
#endif
}

/* ---------------------------- */

static void GBMENTRY def_close(int fd)
{ fd_close(fd); }

/* ---------------------------- */

static long GBMENTRY def_lseek(int fd, long pos, int whence)
{
   int internal_whence = -1;

   switch(whence)
   {
      case GBM_SEEK_SET:
         internal_whence = SEEK_SET;
         break;

      case GBM_SEEK_CUR:
         internal_whence = SEEK_CUR;
         break;

      case GBM_SEEK_END:
         internal_whence = SEEK_END;
         break;

      default:
         /* as we provide an illegal whence value, the OS takes care
          * for correct error reporting
          */
         break;
   }

   return fd_lseek(fd, pos, internal_whence);
}

/* ---------------------------- */

static int GBMENTRY def_read(int fd, void *buf, int len)
    { return fd_read(fd, buf, len); }

/* ---------------------------- */

static int GBMENTRY def_write(int fd, const void *buf, int len)
#ifdef MAC
    /* Prototype for write is missing a 'const' */
    { return write(fd, (void *) buf, len); }
#else
    { return fd_write(fd, buf, len); }
#endif

/* ---------------------------- */
/* ---------------------------- */
/* ---------------------------- */

/* Functions pointers for global IO rerouting */
int  (GBMENTRYP gbmio_file_open  )(const char *fn, int mode)         = def_open  ;
int  (GBMENTRYP gbmio_file_create)(const char *fn, int mode)         = def_create;
void (GBMENTRYP gbmio_file_close )(int fd)                           = def_close ;
long (GBMENTRYP gbmio_file_lseek )(int fd, long pos, int whence)     = def_lseek ;
int  (GBMENTRYP gbmio_file_read  )(int fd, void *buf, int len)       = def_read  ;
int  (GBMENTRYP gbmio_file_write )(int fd, const void *buf, int len) = def_write ;

/* Reset the global remappable I/O functions to the initial ones. */
void gbm_restore_file_io(void)
{
  gbmio_file_open   = def_open  ;
  gbmio_file_create = def_create;
  gbmio_file_close  = def_close ;
  gbmio_file_lseek  = def_lseek ;
  gbmio_file_read   = def_read  ;
  gbmio_file_write  = def_write ;
}

/* The following GBM internally used IO functions will route
 * requests to either:
 * - the default IO functions (file IO)
 * - registered user functions for the whole process
 *
 * Note: The gbm_file_* functions use default calling convention but
 *       map the parameters to functions with other calling conventions!!!
 */

int gbm_file_open(const char *fn, int mode)
{
   /* call globally registered IO function for the process */
   return gbmio_file_open(fn, mode);
}

int gbm_file_open_from_codec(const char *fn, int mode, int bitmap_fd)
{
   bitmap_fd = bitmap_fd; /* suppress compiler warning */

   /* call globally registered IO function for the process */
   return gbmio_file_open(fn, mode);
}

/* ---------------------------- */

int gbm_file_create(const char *fn, int mode)
{
   /* call globally registered IO function for the process */
   return gbmio_file_create(fn, mode);
}

int gbm_file_create_from_codec(const char *fn, int mode, int bitmap_fd)
{
   bitmap_fd = bitmap_fd; /* suppress compiler warning */

   /* call globally registered IO function for the process */
   return gbmio_file_create(fn, mode);
}

/* ---------------------------- */

void gbm_file_close(int fd)
{
   gbmio_file_close(fd);
}

/* ---------------------------- */

long gbm_file_lseek(int fd, long pos, int whence)
{
   /* call globally registered IO function for the process */
   return gbmio_file_lseek(fd, pos, whence);
}

/* ---------------------------- */

int gbm_file_read(int fd, void *buf, int len)
{
   /* call globally registered IO function for the process */
   return gbmio_file_read(fd, buf, len);
}

/* ---------------------------- */

int gbm_file_write(int fd, const void *buf, int len)
{
   /* call globally registered IO function for the process */
   return gbmio_file_write(fd, buf, len);
}

/* ---------------------------- */
/* ---------------------------- */

/* Read ahead support to improve performance on reading many single bytes */

#define AHEAD_BUF 0x4000

typedef struct
{
   gbm_u8 buf[AHEAD_BUF];
   int    inx, cnt;
   long   pos; /* file pointer of the last read byte */
   int    fd;
} AHEAD;

AHEAD *gbm_create_ahead(int fd)
{
   AHEAD *ahead = gbmmem_malloc(sizeof(AHEAD));
   if (ahead == NULL)
   {
      return NULL;
   }
   ahead->inx = 0;
   ahead->cnt = 0;
   ahead->pos = gbm_file_lseek(fd, 0, GBM_SEEK_CUR);
   ahead->fd  = fd;
   return ahead;
}

void gbm_destroy_ahead(AHEAD *ahead)
{
   if (ahead != NULL)
   {
     /* Move file pointer to the last real file position */
     /* so that normal file access is still possible.    */
     gbm_file_lseek(ahead->fd, ahead->pos, GBM_SEEK_SET);
     ahead->fd = -1; /* safety: prevent further use */
     gbmmem_free(ahead);
   }
}

int gbm_look_ahead(AHEAD *ahead)
{
   if (ahead != NULL)
   {
     if ( ahead->inx >= ahead->cnt )
     {
        int b = 0;

        /* don't drop the current buffer but read a single byte locally */
        if (gbm_file_read(ahead->fd, (gbm_u8 *) &b, 1) != 1)
        {
            return -1;
        }
        /* set the file pointer back to the position before */
        if (gbm_file_lseek(ahead->fd, ahead->pos, GBM_SEEK_SET) == -1)
        {
            return -1;
        }
        return b;
     }
     return (int) ahead->buf[ahead->inx];
   }
   return -1;
}

long gbm_lseek_ahead(AHEAD *ahead, long pos, int whence)
{
   if (ahead != NULL)
   {
      /* check if the client only tries to lookup current file pos */
      if (((whence == GBM_SEEK_CUR) && (pos == 0)) ||
          ((whence == GBM_SEEK_SET) && (pos == ahead->pos)))
      {
          return ahead->pos;
      }

      /* set the file pointer back to the real read position */
      if (gbm_file_lseek(ahead->fd, ahead->pos, GBM_SEEK_SET) == -1)
      {
          return -1;
      }

      /* drop the buffer content and start from scratch */
      ahead->inx = 0;
      ahead->cnt = 0;
      ahead->pos = gbm_file_lseek(ahead->fd, pos, whence);
      return ahead->pos;
   }
   return -1;
}

int gbm_read_ahead(AHEAD *ahead)
{
   if (ahead != NULL)
   {
     if ( ahead->inx >= ahead->cnt )
     {
        ahead->cnt = gbm_file_read(ahead->fd, (gbm_u8 *) ahead->buf, AHEAD_BUF);
        if ( ahead->cnt <= 0 )
        {
           return -1;
        }
        ahead->inx = 0;
     }
     (ahead->pos)++;
     return (int) ahead->buf[ahead->inx++];
   }
   return -1;
}

int gbm_readbuf_ahead(AHEAD *ahead, gbm_u8 * buf, int buflen)
{
   if (ahead != NULL)
   {
     int bytesToRead = buflen;

     int rest = ahead->cnt - ahead->inx;
     if (bytesToRead <= rest)
     {
        /* copy the already read buffer */
        memcpy(buf, &(ahead->buf[ahead->inx]), bytesToRead);
        buf         += bytesToRead;
        ahead->inx  += bytesToRead;
        bytesToRead  = 0;
     }
     else if (rest > 0)
     {
        /* copy as much as the already read buffer contains that fits into client buffer */
        memcpy(buf, &(ahead->buf[ahead->inx]), rest);
        bytesToRead -= rest;
        buf         += rest;
        ahead->cnt   = 0;
        ahead->inx   = 0;
     }

     if (bytesToRead > AHEAD_BUF)
     {
        /* read the buffer directly */
        if (gbm_file_read(ahead->fd, buf, bytesToRead) <= 0)
        {
           return -1;
        }
        bytesToRead = 0;
        ahead->cnt  = 0;
        ahead->inx  = 0;
     }
     else
     {
        /* Read ahead buffer empty ? */
        rest = ahead->cnt - ahead->inx;
        if (rest < 1)
        {
           /* read into ahead buffer */
           ahead->cnt = gbm_file_read(ahead->fd, (gbm_u8 *) ahead->buf, AHEAD_BUF);
           if ( ahead->cnt < 0 )
           {
              return -1;
           }
        }
        if (bytesToRead > 0)
        {
           memcpy(buf, ahead->buf, bytesToRead);
           ahead->inx = bytesToRead;
        }
     }

     ahead->pos += buflen;
     return buflen;
   }
   return -1;
}

/* ---------------------------- */
/* ---------------------------- */

/* Write cache support to improve performance on writing many single bytes */

#define WCACHE_BUF 0x4000

typedef struct
{
   gbm_u8 buf[WCACHE_BUF];
   int    outx;
   int    fd;
} WCACHE;

WCACHE *gbm_create_wcache(int fd)
{
   WCACHE *wcache = gbmmem_malloc(sizeof(WCACHE));
   if (wcache == NULL)
   {
      return NULL;
   }
   wcache->outx = 0;
   wcache->fd   = fd;
   return wcache;
}

int gbm_destroy_wcache(WCACHE *wcache)
{
   if (wcache != NULL)
   {
     if (wcache->outx > 0)
     {
        if (gbm_file_write(wcache->fd, (gbm_u8 *) wcache->buf, wcache->outx) < wcache->outx)
        {
          wcache->fd = -1; /* safety: prevent further use */
          gbmmem_free(wcache);
          return -1;
        }
     }
     wcache->fd = -1; /* safety: prevent further use */
     gbmmem_free(wcache);
   }
   return 0;
}

int gbm_write_wcache(WCACHE *wcache, gbm_u8 c)
{
   if (wcache != NULL)
   {
     wcache->buf[wcache->outx] = c;
     wcache->outx++;

     if ( wcache->outx >= WCACHE_BUF )
     {
        if (gbm_file_write(wcache->fd, (gbm_u8 *) wcache->buf, WCACHE_BUF) < WCACHE_BUF)
        {
           return -1;
        }
        wcache->outx = 0;
     }
     return 1;
   }
   return -1;
}

int gbm_writebuf_wcache(WCACHE *wcache, const gbm_u8 * buf, int buflen)
{
   if (wcache != NULL)
   {
     if (wcache->outx + buflen < WCACHE_BUF)
     {
       /* data fits fully into the cache, so just copy */
       memcpy(&wcache->buf[wcache->outx], buf, buflen);
       wcache->outx += buflen;
     }
     else
     {
       /* split into multiple chunks and flush the cache inbetween if necessary */
       const gbm_u8 * splitBuf = NULL;
       int splitLen  = buflen;
       int numChunks = 0;
       int i;

       /* first fill the remaining buffer and flush */
       int remaining = WCACHE_BUF - wcache->outx;

       memcpy(&wcache->buf[wcache->outx], buf, remaining);
       wcache->outx += remaining;
       splitBuf      = buf + remaining;
       splitLen     -= remaining;

       if (gbm_file_write(wcache->fd, (gbm_u8 *) wcache->buf, WCACHE_BUF) < WCACHE_BUF)
       {
          return -1;
       }
       wcache->outx = 0;

       /* now calc the number of additional chunks and flush them */
       numChunks = splitLen / WCACHE_BUF;

       for (i = 0; i < numChunks; i++)
       {
          if (gbm_file_write(wcache->fd, splitBuf, WCACHE_BUF) < WCACHE_BUF)
          {
             return -1;
          }
          splitBuf += WCACHE_BUF;
          splitLen -= WCACHE_BUF;
       }

       /* and finally copy the remaining bytes into the new buffer */
       memcpy(&wcache->buf[wcache->outx], splitBuf, splitLen);
       wcache->outx += splitLen;
     }
     return buflen;
   }
   return -1;
}

