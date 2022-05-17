/*

gbmpnm.c - PNM format

Credit for writing this module must go to Heiko Nitzsche.
This file is just as public domain as the rest of GBM.

Supported formats and options:
------------------------------
Anymap : Portable Anymap : .PNM

Standard formats (backward compatible):
  Reads  1 bpp black/white images     (ASCII format P1, binary format P4).
  Reads  8 bpp grey images                 (ASCII format P2, binary format P5).
  Reads 16 bpp grey images                (ASCII format P2, binary format P5) and presents them as 8 bpp.
  Reads 24 bpp unpalettised RGB files (ASCII format P3, binary format P6).
  Reads 48 bpp unpalettised RGB files and presents them as 24 bpp (ASCII format P3, binary format P6).

Extended formats (not backward compatible, import option ext_bpp required):
  Reads 16 bpp grey images (ASCII format P2, binary format P5) and presents them as 48 bpp.
  Reads 48 bpp unpalettised RGB files and presents them as 48 bpp (ASCII format P3, binary format P6).

Writes 1 bpp black/white images (ASCII format P1, binary format P4).
Writes grey equivalent of passed in 8 bit colour data (no palette written) (ASCII format P2, binary format P5).
Writes 24 and 48 bpp unpalettised RGB files (ASCII format P3, binary format P6).


Input:
------

Can specify image within PNM file with multiple images
  Input option: index=# (default: 0)

Can specify that non-standard GBM color depth is exported (48 bpp)
  Input option: ext_bpp (default: bpp is downsampled to 24 bpp)

Invert the data bits on input (1 bpp only)
  Input options: invb


Output:
-------

Invert the data bits on output (1 bpp only)
  Output options: invb

Can specify the colour channel the output grey values are based on
  Output options: r,g,b,k (default: k, combine color channels and write grey equivalent)

Write ASCII format (default is binary)
  Output option: ascii

Write additonal comment
  Output option: comment=text


History:
--------
(Heiko Nitzsche)

11-Jun-2006: Initial release
16-Jun-2006: Add support for ASCII subformats
15-Aug-2008: Integrate new GBM types
*/

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"

#include "gbmpbm.h"
#include "gbmpgm.h"
#include "gbmppm.h"

/* ---------------------------------------- */

#define  GBM_ERR_PNM_BAD_M    ((GBM_ERR) 2200)

/* ---------------------------------------- */

static GBMFT pnm_gbmft =
{
   GBM_FMT_DESC_SHORT_PNM,
   GBM_FMT_DESC_LONG_PNM,
   GBM_FMT_DESC_EXT_PNM,
   GBM_FT_R1 | GBM_FT_R8 | GBM_FT_R24 | GBM_FT_R48 |
   GBM_FT_W1 | GBM_FT_W8 | GBM_FT_W24 | GBM_FT_W48
};

/* ---------------------------------------- */

static gbm_u8 read_byte(int fd)
{
   gbm_u8 b = 0;
   gbm_file_read(fd, (char *) &b, 1);
   return b;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pnm_qft(GBMFT *gbmft)
{
   *gbmft = pnm_gbmft;
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static GBM_ERR read_pnm_header(int fd, int *type)
{
   int h1;

   /* start at the beginning of the file */
   gbm_file_lseek(fd, 0, GBM_SEEK_SET);

   h1    = read_byte(fd);
   *type = read_byte(fd);
   if ((h1 != 'P') || ((*type != '1') && (*type != '2') && (*type != '3') &&
                       (*type != '4') && (*type != '5') && (*type != '6')))
   {
      return GBM_ERR_BAD_MAGIC;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read number of images in the PNM file. */
GBM_ERR pnm_rimgcnt(const char *fn, int fd, int *pimgcnt)
{
   GBM_ERR rc;
   int     type;

   fn=fn; /* suppress compiler warning */

   *pimgcnt = 1;

   /* read header info of first bitmap */
   rc = read_pnm_header(fd, &type);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }

   /* start again at the beginning of the file */
   gbm_file_lseek(fd, 0, GBM_SEEK_SET);

   /* dispatch to the specific codec (PBM, PGM or PPM) */
   switch(type)
   {
     case '1': /* PBM */
     case '4': /* PBM */
       if (pbm_rimgcnt(fn, fd, pimgcnt) != GBM_ERR_OK)
       {
         return GBM_ERR_PNM_BAD_M;
       }
       break;

     case '2': /* PGM */
     case '5': /* PGM */
       if (pgm_rimgcnt(fn, fd, pimgcnt) != GBM_ERR_OK)
       {
         return GBM_ERR_PNM_BAD_M;
       }
       break;

     case '3': /* PPM */
     case '6': /* PPM */
       if (ppm_rimgcnt(fn, fd, pimgcnt) != GBM_ERR_OK)
       {
         return GBM_ERR_PNM_BAD_M;
       }
       break;

     default:
       return GBM_ERR_BAD_MAGIC;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pnm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
   GBM_ERR rc;
   int     type;

   /* read header info of first bitmap */
   rc = read_pnm_header(fd, &type);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }

   /* dispatch to the specific codec (PBM, PGM or PPM) */
   switch(type)
   {
     case '1': /* PBM */
     case '4': /* PBM */
       if (pbm_rhdr(fn, fd, gbm, opt) != GBM_ERR_OK)
       {
         return GBM_ERR_BAD_SIZE;
       }
       break;

     case '2': /* PGM */
     case '5': /* PGM */
       if (pgm_rhdr(fn, fd, gbm, opt) != GBM_ERR_OK)
       {
         return GBM_ERR_BAD_SIZE;
       }
       break;

     case '3': /* PPM */
     case '6': /* PPM */
       if (ppm_rhdr(fn, fd, gbm, opt) != GBM_ERR_OK)
       {
         return GBM_ERR_BAD_SIZE;
       }
       break;

     default:
       return GBM_ERR_BAD_MAGIC;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pnm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
   GBM_ERR rc;
   int     type;

   /* read header info of first bitmap */
   rc = read_pnm_header(fd, &type);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }

   /* dispatch to the specific codec (PBM, PGM or PPM) */
   switch(type)
   {
     case '1': /* PBM */
     case '4': /* PBM */
       if (pbm_rpal(fd, gbm, gbmrgb) != GBM_ERR_OK)
       {
         return GBM_ERR_BAD_SIZE;
       }
       break;

     case '2': /* PGM */
     case '5': /* PGM */
       if (pgm_rpal(fd, gbm, gbmrgb) != GBM_ERR_OK)
       {
         return GBM_ERR_BAD_SIZE;
       }
       break;

     case '3': /* PPM */
     case '6': /* PPM */
       if (ppm_rpal(fd, gbm, gbmrgb) != GBM_ERR_OK)
       {
         return GBM_ERR_BAD_SIZE;
       }
       break;

     default:
       return GBM_ERR_BAD_MAGIC;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pnm_rdata(int fd, GBM *gbm, gbm_u8 *data)
{
   GBM_ERR rc;
   int     type;

   /* read header info of first bitmap */
   rc = read_pnm_header(fd, &type);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }

   /* dispatch to the specific codec (PBM, PGM or PPM) */
   switch(type)
   {
     case '1': /* PBM */
     case '4': /* PBM */
       if (pbm_rdata(fd, gbm, data) != GBM_ERR_OK)
       {
         return GBM_ERR_BAD_SIZE;
       }
       break;

     case '2': /* PGM */
     case '5': /* PGM */
       if (pgm_rdata(fd, gbm, data) != GBM_ERR_OK)
       {
         return GBM_ERR_BAD_SIZE;
       }
       break;

     case '3': /* PPM */
     case '6': /* PPM */
       if (ppm_rdata(fd, gbm, data) != GBM_ERR_OK)
       {
         return GBM_ERR_BAD_SIZE;
       }
       break;

     default:
       return GBM_ERR_BAD_MAGIC;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pnm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
{
   #define  SW4(a,b,c,d)   ((a)*8+(b)*4+(c)*2+(d))

  const gbm_boolean kb = ( gbm_find_word(opt, "k") != NULL );
  const gbm_boolean rb = ( gbm_find_word(opt, "r") != NULL );
  const gbm_boolean gb = ( gbm_find_word(opt, "g") != NULL );
  const gbm_boolean bb = ( gbm_find_word(opt, "b") != NULL );
    
   /* dispatch to the specific codec (PBM, PGM or PPM) */
   switch(gbm->bpp)
   {
     case 1: /* PBM */
       return pbm_w(fn, fd, gbm, gbmrgb, data, opt);

     case 8: /* PGM */
       return pgm_w(fn, fd, gbm, gbmrgb, data, opt);

     case 24: /* PGM/PPM */
     case 48: /* PGM/PPM */
       /* if gray output is requested, export a PGM file. The encoder supports conversion for 24->8bpp and 48->16bpp gray */
       if (kb || rb || gb || bb)
       {
         return pgm_w(fn, fd, gbm, gbmrgb, data, opt);
       }
       return ppm_w(fn, fd, gbm, gbmrgb, data, opt);

     default: /* all others are not supported */
       break;
   }

   return GBM_ERR_NOT_SUPP;
}

/* ---------------------------------------- */

const char *pnm_err(GBM_ERR rc)
{
   switch ( (int) rc )
   {
      case GBM_ERR_PNM_BAD_M:
         return "bad maximum pixel intensity";
   }
   return NULL;
}

