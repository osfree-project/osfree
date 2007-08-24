/*

gbm.c - Generalised Bitmap Module

History:
--------
(Heiko Nitzsche)

19-Feb-2006: Add function to query number of images
             Add function to reset user defined I/O functions to internal ones
27-May-2006: Protect all readers and writer against illegal colour depth requests
             (gbm_read_palette, gbm_read_data, gbm_write)
07-Jun-2006: Add PBM, PNM formats and multipage handler for PGM
26-Aug-2006: Add XPM format
19-Jan-2007: Add missing braces around format decoder/encoder registry
*/

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#if defined(AIX) || defined(LINUX) || defined(SUN) || defined(MAC)
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
#include "gbmhelp.h"

/*...sentrypoints:0:*/
#include "gbmxpm.h"
#include "gbmpbm.h"
#include "gbmpgm.h"
#include "gbmpnm.h"
#include "gbmppm.h"
#include "gbmpng.h"
#include "gbmbmp.h"
#include "gbmtga.h"
#include "gbmkps.h"
#include "gbmiax.h"
#include "gbmpcx.h"
#include "gbmtif.h"
#include "gbmlbm.h"
#include "gbmvid.h"
#include "gbmgif.h"
#include "gbmxbm.h"
#include "gbmspr.h"
#include "gbmpsg.h"
#include "gbmgem.h"
#include "gbmcvp.h"
#include "gbmjpg.h"


#define GBM_VERSION   151  /* 1.51 */

/* --------------------------- */

typedef struct
{
    GBM_ERR      (*query_filetype   )(GBMFT *gbmft);
    /* set to NULL if max. image count is 1 */
    GBM_ERR      (*read_image_count )(const char *fn, int fd, int *pimgcnt);
    GBM_ERR      (*read_header      )(const char *fn, int fd, GBM *gbm, const char *opt);
    GBM_ERR      (*read_palette     )(int fd, GBM *gbm, GBMRGB *gbmrgb);
    GBM_ERR      (*read_data        )(int fd, GBM *gbm, byte *data);
    GBM_ERR      (*write            )(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
    const char * (*err              )(GBM_ERR rc);
} FT;

static FT fts[] =
{
   { bmp_qft, bmp_rimgcnt, bmp_rhdr, bmp_rpal, bmp_rdata, bmp_w, bmp_err },
   { gif_qft, gif_rimgcnt, gif_rhdr, gif_rpal, gif_rdata, gif_w, gif_err },
   { pcx_qft, NULL       , pcx_rhdr, pcx_rpal, pcx_rdata, pcx_w, pcx_err },
#ifdef ENABLE_TIF
   { tif_qft, tif_rimgcnt, tif_rhdr, tif_rpal, tif_rdata, tif_w, tif_err },
#endif
   { tga_qft, NULL       , tga_rhdr, tga_rpal, tga_rdata, tga_w, tga_err },
   { lbm_qft, NULL       , lbm_rhdr, lbm_rpal, lbm_rdata, lbm_w, lbm_err },
   { vid_qft, NULL       , vid_rhdr, vid_rpal, vid_rdata, vid_w, vid_err },
   { pbm_qft, pbm_rimgcnt, pbm_rhdr, pbm_rpal, pbm_rdata, pbm_w, pbm_err },
   { pgm_qft, pgm_rimgcnt, pgm_rhdr, pgm_rpal, pgm_rdata, pgm_w, pgm_err },
   { ppm_qft, ppm_rimgcnt, ppm_rhdr, ppm_rpal, ppm_rdata, ppm_w, ppm_err },
   { pnm_qft, pnm_rimgcnt, pnm_rhdr, pnm_rpal, pnm_rdata, pnm_w, pnm_err },
   { kps_qft, NULL       , kps_rhdr, kps_rpal, kps_rdata, kps_w, kps_err },
   { iax_qft, NULL       , iax_rhdr, iax_rpal, iax_rdata, iax_w, iax_err },
   { xbm_qft, NULL       , xbm_rhdr, xbm_rpal, xbm_rdata, xbm_w, xbm_err },
   { xpm_qft, NULL       , xpm_rhdr, xpm_rpal, xpm_rdata, xpm_w, xpm_err },
   { spr_qft, spr_rimgcnt, spr_rhdr, spr_rpal, spr_rdata, spr_w, spr_err },
   { psg_qft, NULL       , psg_rhdr, psg_rpal, psg_rdata, psg_w, psg_err },
   { gem_qft, NULL       , gem_rhdr, gem_rpal, gem_rdata, gem_w, gem_err },
   { cvp_qft, NULL       , cvp_rhdr, cvp_rpal, cvp_rdata, cvp_w, cvp_err },
#ifdef ENABLE_PNG
   { png_qft, NULL       , png_rhdr, png_rpal, png_rdata, png_w, png_err },
#endif
#ifdef ENABLE_IJG
   { jpg_qft, NULL       , jpg_rhdr, jpg_rpal, jpg_rdata, jpg_w, jpg_err },
#endif
};

#define N_FT (sizeof(fts) / sizeof(fts[0]))

/* --------------------------- */

static const char *extension(const char *fn)
{
    const char *dot, *slash;

    if ( (dot = strrchr(fn, '.')) == NULL )
        return NULL;

    if ( (slash = strpbrk(fn, "/\\")) == NULL )
        return dot + 1;

    return ( slash < dot ) ? dot + 1 : NULL;
}

/* --------------------------- */

GBMEXPORT GBM_ERR GBMENTRY gbm_init(void)
{
    return GBM_ERR_OK;
}

/* --------------------------- */

GBMEXPORT GBM_ERR GBMENTRY gbm_deinit(void)
{
    return GBM_ERR_OK;
}

/* --------------------------- */

/* Register/unregister process specific IO callbacks */

GBMEXPORT GBM_ERR GBMENTRY gbm_io_setup(
    int  (GBMENTRYP open  )(const char *fn, int mode),
    int  (GBMENTRYP create)(const char *fn, int mode),
    void (GBMENTRYP close )(int fd),
    long (GBMENTRYP lseek )(int fd, long pos, int whence),
    int  (GBMENTRYP read  )(int fd, void *buf, int len),
    int  (GBMENTRYP write )(int fd, const void *buf, int len))
{
    if ((open  == NULL) || (create == NULL) || (close == NULL) ||
        (lseek == NULL) || (read   == NULL) || (write == NULL))
    {
       return GBM_ERR_BAD_ARG;
    }
    gbmio_file_open   = open  ;
    gbmio_file_create = create;
    gbmio_file_close  = close ;
    gbmio_file_lseek  = lseek ;
    gbmio_file_read   = read  ;
    gbmio_file_write  = write ;
    return GBM_ERR_OK;
}

GBMEXPORT GBM_ERR GBMENTRY_SYS gbm_restore_io_setup(void)
{
   gbm_restore_file_io();
   return GBM_ERR_OK;
}

/* --------------------------- */

GBMEXPORT int  GBMENTRY gbm_io_open(const char *fn, int mode)
{ return gbm_file_open(fn, mode); }

GBMEXPORT int  GBMENTRY gbm_io_create(const char *fn, int mode)
{ return gbm_file_create(fn, mode); }

GBMEXPORT void GBMENTRY gbm_io_close(int fd)
{ gbm_file_close(fd); }

/* --------------------------- */

GBMEXPORT long GBMENTRY gbm_io_lseek(int fd, long pos, int whence)
{ return gbm_file_lseek(fd, pos, whence); }

GBMEXPORT int  GBMENTRY gbm_io_read (int fd, void *buf, int len)
{ return gbm_file_read(fd, buf, len); }

GBMEXPORT int  GBMENTRY gbm_io_write(int fd, const void *buf, int len)
{ return gbm_file_write(fd, buf, len); }

/* --------------------------- */

GBMEXPORT GBM_ERR GBMENTRY gbm_query_n_filetypes(int *n_ft)
{
    if ( n_ft == NULL )
        return GBM_ERR_BAD_ARG;
    *n_ft = N_FT;
    return GBM_ERR_OK;
}

/* --------------------------- */

GBMEXPORT GBM_ERR GBMENTRY gbm_guess_filetype(const char *fn, int *ft)
{
    int i;
    const char *ext;

    if ( fn == NULL || ft == NULL )
        return GBM_ERR_BAD_ARG;

    if ( (ext = extension(fn)) == NULL )
        ext = "";

    for ( i = 0; i < N_FT; i++ )
    {
        GBMFT gbmft;
        char  buf[100+1], *s;

        fts[i].query_filetype(&gbmft);
        for ( s  = strtok(strcpy(buf, gbmft.extensions), " \t,");
              s != NULL;
              s  = strtok(NULL, " \t,") )
            if ( gbm_same(s, ext, (int) strlen(ext) + 1) )
            {
                *ft = i;
                return GBM_ERR_OK;
            }
    }
    return GBM_ERR_NOT_FOUND;
}

/* --------------------------- */

GBMEXPORT GBM_ERR GBMENTRY gbm_query_filetype(int ft, GBMFT *gbmft)
{
    if ( gbmft == NULL )
        return GBM_ERR_BAD_ARG;
    return (*fts[ft].query_filetype)(gbmft);
}

/* --------------------------- */

GBMEXPORT GBM_ERR GBMENTRY_SYS gbm_read_imgcount(const char *fn, int fd, int ft, int *pimgcnt)
{
   if ( fn == NULL || pimgcnt == NULL)
   {
      return GBM_ERR_BAD_ARG;
   }
   gbm_file_lseek(fd, 0L, GBM_SEEK_SET);

   if (fts[ft].read_image_count)
   {
      return (*fts[ft].read_image_count)(fn, fd, pimgcnt);
   }
   else
   {
      /* try to read header to at least detect if there is an image at all */
      GBM gbm;
      GBM_ERR rc = (*fts[ft].read_header)(fn, fd, &gbm, "");
      if (rc != GBM_ERR_OK)
      {
         return rc;
      }
      *pimgcnt = 1;
   }

   return GBM_ERR_OK;
}

/* --------------------------- */

GBMEXPORT GBM_ERR GBMENTRY gbm_read_header(const char *fn, int fd, int ft, GBM *gbm, const char *opt)
{
    if ( fn == NULL || opt == NULL || gbm == NULL )
        return GBM_ERR_BAD_ARG;
    gbm_file_lseek(fd, 0L, GBM_SEEK_SET);
    return (*fts[ft].read_header)(fn, fd, gbm, opt);
}

/* --------------------------- */

GBMEXPORT GBM_ERR GBMENTRY gbm_read_palette(int fd, int ft, GBM *gbm, GBMRGB *gbmrgb)
{
  int   flag = 0;
  GBMFT gbmft = { 0 };

  if ( gbm == NULL || gbmrgb == NULL )
    return GBM_ERR_BAD_ARG;

  /* check if the input format supports the requested color depth */
  gbm_query_filetype(ft, &gbmft);
  switch ( gbm->bpp )
  {
    case 64: flag = GBM_FT_R64; break;
    case 48: flag = GBM_FT_R48; break;
    case 32: flag = GBM_FT_R32; break;
    case 24: flag = GBM_FT_R24; break;
    case  8: flag = GBM_FT_R8;  break;
    case  4: flag = GBM_FT_R4;  break;
    case  1: flag = GBM_FT_R1;  break;
    default: flag = 0;          break;
  }
  if ( (gbmft.flags & flag) == 0 )
  {
    return GBM_ERR_NOT_SUPP;
  }

  return (*fts[ft].read_palette)(fd, gbm, gbmrgb);
}

/* --------------------------- */

GBMEXPORT GBM_ERR GBMENTRY gbm_read_data(int fd, int ft, GBM *gbm, byte *data)
{
  int   flag = 0;
  GBMFT gbmft = { 0 };

  if ( gbm == NULL || data == NULL )
    return GBM_ERR_BAD_ARG;

  /* check if the input format supports the requested color depth */
  gbm_query_filetype(ft, &gbmft);
  switch ( gbm->bpp )
  {
    case 64: flag = GBM_FT_R64; break;
    case 48: flag = GBM_FT_R48; break;
    case 32: flag = GBM_FT_R32; break;
    case 24: flag = GBM_FT_R24; break;
    case  8: flag = GBM_FT_R8;  break;
    case  4: flag = GBM_FT_R4;  break;
    case  1: flag = GBM_FT_R1;  break;
    default: flag = 0;          break;
  }
  if ( (gbmft.flags & flag) == 0 )
  {
    return GBM_ERR_NOT_SUPP;
  }

  return (*fts[ft].read_data)(fd, gbm, data);
}

/* --------------------------- */

GBMEXPORT GBM_ERR GBMENTRY gbm_write(const char *fn, int fd, int ft, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
{
  int   flag = 0;
  GBMFT gbmft = { 0 };

  if ( fn == NULL || opt == NULL )
    return GBM_ERR_BAD_ARG;

  /* check if the output format supports the requested color depth */
  gbm_query_filetype(ft, &gbmft);
  switch ( gbm->bpp )
  {
    case 64: flag = GBM_FT_W64; break;
    case 48: flag = GBM_FT_W48; break;
    case 32: flag = GBM_FT_W32; break;
    case 24: flag = GBM_FT_W24; break;
    case  8: flag = GBM_FT_W8;  break;
    case  4: flag = GBM_FT_W4;  break;
    case  1: flag = GBM_FT_W1;  break;
    default: flag = 0;          break;
  }
  if ( (gbmft.flags & flag) == 0 )
  {
    return GBM_ERR_NOT_SUPP;
  }

  return (*fts[ft].write)(fn, fd, gbm, gbmrgb, data, opt);
}

/* --------------------------- */

GBMEXPORT const char * GBMENTRY gbm_err(GBM_ERR rc)
{
    int ft;

    switch ( (int) rc )
    {
        case GBM_ERR_OK:
            return "ok";
        case GBM_ERR_MEM:
            return "out of memory";
        case GBM_ERR_NOT_SUPP:
            return "not supported";
        case GBM_ERR_BAD_OPTION:
            return "bad option(s)";
        case GBM_ERR_NOT_FOUND:
            return "not found";
        case GBM_ERR_BAD_MAGIC:
            return "bad magic number / signiture block";
        case GBM_ERR_BAD_SIZE:
            return "bad bitmap size";
        case GBM_ERR_READ:
            return "can't read file";
        case GBM_ERR_WRITE:
            return "can't write file";
        case GBM_ERR_BAD_ARG:
            return "bad argument to gbm function";
    }

    for ( ft = 0; ft < N_FT; ft++ )
    {
        const char *s;
        if ( (s = (*fts[ft].err)(rc)) != NULL )
            return s;
    }

    return "general error";
}

/* --------------------------- */

GBMEXPORT int GBMENTRY gbm_version(void)
{
    return GBM_VERSION;
}

/* --------------------------- */

#if defined(__OS2__) || defined(OS2)
/*...s_System entrypoints:0:*/
/* For GBM.DLL to be callable from IBM Smalltalk under OS/2, the entrypoints
   must be of _System calling convention. These veneers help out here.
   I can't just change the usual entrypoints because people depend on them. */

GBM_ERR _System Gbm_init(void)
{ return gbm_init(); }

GBM_ERR _System Gbm_deinit(void)
{ return gbm_deinit(); }

/* --------------------------- */

/* Gbm_io_setup omitted for now...
   Implies GBM.DLL must call out to non-_Optlink callback routines. */

/* --------------------------- */

int _System Gbm_io_open(const char *fn, int mode)
{ return gbm_io_open(fn, mode); }

int _System Gbm_io_create(const char *fn, int mode)
{ return gbm_io_create(fn, mode); }

void _System Gbm_io_close(int fd)
{ gbm_io_close(fd); }

long _System Gbm_io_lseek(int fd, long pos, int whence)
{ return gbm_file_lseek(fd, pos, whence); }

int _System Gbm_io_read (int fd, void *buf, int len)
{ return gbm_file_read(fd, buf, len); }

int _System Gbm_io_write(int fd, const void *buf, int len)
{ return gbm_file_write(fd, buf, len); }

/* --------------------------- */

GBM_ERR _System Gbm_query_n_filetypes(int *n_ft)
{ return gbm_query_n_filetypes(n_ft); }

GBM_ERR _System Gbm_guess_filetype(const char *fn, int *ft)
{ return gbm_guess_filetype(fn, ft); }

GBM_ERR    _System Gbm_query_filetype(int ft, GBMFT *gbmft)
{ return gbm_query_filetype(ft, gbmft); }

/* --------------------------- */

GBM_ERR _System Gbm_read_header(const char *fn, int fd, int ft, GBM *gbm, const char *opt)
{ return gbm_read_header(fn, fd, ft, gbm, opt); }

GBM_ERR _System Gbm_read_palette(int fd, int ft, GBM *gbm, GBMRGB *gbmrgb)
{ return gbm_read_palette(fd, ft, gbm, gbmrgb); }

GBM_ERR _System Gbm_read_data(int fd, int ft, GBM *gbm, byte *data)
{ return gbm_read_data(fd, ft, gbm, data); }

GBM_ERR _System Gbm_write(const char *fn, int fd, int ft, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
{ return gbm_write(fn, fd, ft, gbm, gbmrgb, data, opt); }

/* --------------------------- */

const char * _System Gbm_err(GBM_ERR rc)
{ return gbm_err(rc); }

int _System Gbm_version(void)
{ return gbm_version(); }

#endif

