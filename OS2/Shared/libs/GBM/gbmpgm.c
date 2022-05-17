/*

gbmpgm.c - Poskanzers PGM format


Supported formats and options:
------------------------------
Greymap : Portable Greyscale-map : .PGM

Reads   8 bpp grey images (ASCII format P2, binary format P5).
Reads  16 bpp grey images (ASCII format P2, binary format P5) and presents them as 8 bpp.

Extended formats (not backward compatible, import option ext_bpp required):
  Reads 16 bpp grey images (ASCII format P2, binary format P5) and presents them as 48 bpp.

Writes  8 bpp grey equivalent of passed in 8 bit colour data (no palette written)
       (ASCII format P2, binary format P5)
Writes  8 bpp grey equivalent of passed in 24 bit colour data (no palette written)
       (ASCII format P2, binary format P5)
Writes 16 bpp grey equivalent of passed in 48 bit colour data (no palette written)
       (ASCII format P2, binary format P5)

Input:
------

Can specify image within PGM file with multiple images (only for P5 type)
  Input option: index=# (default: 0)

Output:
-------

Can specify the colour channel the output grey values are based on
  Output option: r,g,b,k (default: k, combine color channels and write grey equivalent)

Write ASCII format P2 (default is binary P5)
  Output option: ascii

Write additonal comment
  Output option: comment=text


History:
--------
(Heiko Nitzsche)

22-Feb-2006: Move format description strings to gbmdesc.h
11-Jun-2006: Add function to query number of images (type P5 only)
             Add read support for multipage images  (type P5 only)
16-Jun-2006: Add support for reading & writing ASCII format P2
23-Sep-2006: Use read ahead and write cache for ASCII format to improve speed
15-Aug-2008: Integrate new GBM types
23-Feb-2009: Add support for reading/writing 16bpp greylevel images (P2 and P5)
*/

#define _XOPEN_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"
#include "gbmmem.h"

/* ---------------------------------------- */

#define  GBM_ERR_PGM_BAD_M    ((GBM_ERR) 2100)

#define CVT(x) (((x) * 255) / ((1L << 16) - 1))

#if defined(WIN32) && defined(_MSC_VER)
  /* Use ISO variants */
  #define gbm_swab _swab
#else
  /* Use POSIX variants */
  #if !defined(__IBMC__)
    #include <unistd.h>
  #endif
  #define gbm_swab swab
#endif

/* ---------------------------------------- */

static GBMFT pgm_gbmft =
{
   GBM_FMT_DESC_SHORT_PGM,
   GBM_FMT_DESC_LONG_PGM,
   GBM_FMT_DESC_EXT_PGM,
   GBM_FT_R8 | GBM_FT_R48 |
   GBM_FT_W8 | GBM_FT_W24 | GBM_FT_W48
};

typedef struct
{
   unsigned int max_intensity;

   /* This entry will store the options provided during first header read.
    * It will keep the options for the case the header has to be reread.
    */
   char read_options[PRIV_SIZE - sizeof(int)
                               - 8 /* space for structure element padding */ ];

} PGM_PRIV_READ;

/* ---------------------------------------- */

#define  low_byte(w)    ((gbm_u8)  ((w)&0x00ff)    )
#define  high_byte(w)   ((gbm_u8) (((w)&0xff00)>>8))
#define  make_word(a,b) (((gbm_u16)a) + (((gbm_u16)b) << 8))
#define  SW4(a,b,c,d)   ((a)*8+(b)*4+(c)*2+(d))

/* ---------------------------------------- */

static void g16msb_g8(const gbm_u16 * p16, gbm_u8 * q, int n)
{
   gbm_u16 g;
   while ( n-- )
   {
      g = *p16++;
      gbm_swab((char *) &g, (char *) &g, 2);
      *q++ = (gbm_u8) CVT(g);
   }
}

static void g16msb_bgr16lsb(const gbm_u16 *p16, gbm_u16 *q16, int n, const unsigned int max_intensity)
{
   gbm_u16 g;
   if (max_intensity < 65535)
   {
      while ( n-- )
      {
         g = *p16++;

         gbm_swab((char *) &g, (char *) &g, 2);

         g = (g * 65535U) / max_intensity;

         *q16++ = g;
         *q16++ = g;
         *q16++ = g;
      }
   }
   else
   {
      while ( n-- )
      {
         g = *p16++;

         gbm_swab((char *) &g, (char *) &g, 2);

         *q16++ = g;
         *q16++ = g;
         *q16++ = g;
      }
   }
}

static void bgr_g(const gbm_u8 *p, gbm_u8 *q, int n, const char *opt)
{
  gbm_u8 r,g,b;

  const gbm_boolean kb = ( gbm_find_word(opt, "k") != NULL );
  const gbm_boolean rb = ( gbm_find_word(opt, "r") != NULL );
  const gbm_boolean gb = ( gbm_find_word(opt, "g") != NULL );
  const gbm_boolean bb = ( gbm_find_word(opt, "b") != NULL );

  while ( n-- )
  {
     b = *p++;
     g = *p++;
     r = *p++;

     switch ( SW4(kb,rb,gb,bb) )
     {
       case SW4(0,1,0,0):
         g = r;
         break;

       case SW4(0,0,1,0):
         break;

       case SW4(0,0,0,1):
         g = b;
         break;

       case SW4(0,0,0,0):
         /* Default is the same as "k" */
       case SW4(1,0,0,0):
       default:
         g = (gbm_u8) ( ((gbm_u16) r *  77U +
                         (gbm_u16) g * 150U +
                         (gbm_u16) b *  29U) >> 8 );
         break;
     }

     *q++ = g;
  }
}


static void bgr16lsb_g16msb(const gbm_u16 *p16, gbm_u16 *q16, int n, const char *opt)
{
  gbm_u16 r,g,b;

  const gbm_boolean kb = ( gbm_find_word(opt, "k") != NULL );
  const gbm_boolean rb = ( gbm_find_word(opt, "r") != NULL );
  const gbm_boolean gb = ( gbm_find_word(opt, "g") != NULL );
  const gbm_boolean bb = ( gbm_find_word(opt, "b") != NULL );

  while ( n-- )
  {
     b = *p16++;
     g = *p16++;
     r = *p16++;

     switch ( SW4(kb,rb,gb,bb) )
     {
       case SW4(0,1,0,0):
         g = r;
         break;

       case SW4(0,0,1,0):
         break;

       case SW4(0,0,0,1):
         g = b;
         break;

       case SW4(0,0,0,0):
         /* Default is the same as "k" */
       case SW4(1,0,0,0):
       default:
         g = (gbm_u16) ( ((gbm_u32) r *  (77U * 255U) +
                          (gbm_u32) g * (150U * 255U) +
                          (gbm_u32) b *  (29U * 255U)) >> 16 );
         break;
     }

     gbm_swab((char *) &g, (char *) &g, 2);

     *q16++ = g;
  }
}

/* ---------------------------------------- */

static gbm_boolean make_output_palette(const GBMRGB gbmrgb[], gbm_u8 grey[], const char *opt)
{
  gbm_boolean  k = ( gbm_find_word(opt, "k") != NULL );
  gbm_boolean  r = ( gbm_find_word(opt, "r") != NULL );
  gbm_boolean  g = ( gbm_find_word(opt, "g") != NULL );
  gbm_boolean  b = ( gbm_find_word(opt, "b") != NULL );
  int i;

  switch ( SW4(k,r,g,b) )
  {
    case SW4(0,0,0,0):
      /* Default is the same as "k" */
    case SW4(1,0,0,0):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = (gbm_u8) ( ((gbm_u16) gbmrgb[i].r *  77U +
                              (gbm_u16) gbmrgb[i].g * 150U +
                              (gbm_u16) gbmrgb[i].b *  29U) >> 8 );
      }
      return GBM_TRUE;

    case SW4(0,1,0,0):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = gbmrgb[i].r;
      }
      return GBM_TRUE;

    case SW4(0,0,1,0):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = gbmrgb[i].g;
      }
      return GBM_TRUE;

    case SW4(0,0,0,1):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = gbmrgb[i].b;
      }
      return GBM_TRUE;
  }
  return GBM_FALSE;
}

/* ---------------------------------------- */

static gbm_u8 read_byte(int fd)
{
  gbm_u8 b = 0;
  gbm_file_read(fd, (char *) &b, 1);
  return b;
}

/* ---------------------------------------- */

static char read_char(int fd)
{
  char c;
  while ( (c = read_byte(fd)) == '#' )
  {
    /* Discard to end of line */
    while ( (c = read_byte(fd)) != '\n' );
  }
  return c;
}

static int read_num(int fd)
{
  char c;
  int num;

  while ( isspace(c = read_char(fd)) )
    ;
  num = c - '0';
  while ( isdigit(c = read_char(fd)) )
  {
    num = num * 10 + (c - '0');
  }
  return num;
}

/* ---------------------------------------- */

static char read_char_ahead(AHEAD * ahead)
{
  char c;
  while ( (c = gbm_read_ahead(ahead)) == '#' )
  {
    /* Discard to end of line */
    while ( (c = gbm_read_ahead(ahead)) != '\n' );
  }
  return c;
}

static int read_num_data(AHEAD * ahead)
{
  char c;
  int num;

  do
  {
    c = read_char_ahead(ahead);
  }
  while (isspace(c) || (c == '\n') || (c == '\r'));

  num = c - '0';
  while ( isdigit(c = read_char_ahead(ahead)) )
  {
    num = num * 10 + (c - '0');
  }
  return num;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pgm_qft(GBMFT *gbmft)
{
   *gbmft = pgm_gbmft;
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static GBM_ERR read_pgm_header(int fd, int *h1, int *h2, int *w, int *h, int *m, int *data_bytes)
{
   *h1 = read_byte(fd);
   *h2 = read_byte(fd);
   if ( (*h1 != 'P') || ((*h2 != '2') && (*h2 != '5')) )
   {
      return GBM_ERR_BAD_MAGIC;
   }

   *w  = read_num(fd);
   *h  = read_num(fd);
   if ((*w <= 0) || (*h <= 0))
   {
      return GBM_ERR_BAD_SIZE;
   }

   *m  = read_num(fd);
   if (*m <= 1)
   {
      return GBM_ERR_PGM_BAD_M;
   }

   /* check whether 1 byte or 2 byte format */
   if (*m < 0x100)
   {
      *data_bytes = (*w) * (*h);
   }
   else if (*m < 0x10000)
   {
      *data_bytes = 2 * (*w) * (*h);
   }
   else
   {
      return GBM_ERR_PGM_BAD_M;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read number of images in the PGM file. */
GBM_ERR pgm_rimgcnt(const char *fn, int fd, int *pimgcnt)
{
   GBM_ERR rc;
   GBM     gbm;
   int     h1, h2, m, data_bytes;

   fn=fn; /* suppress compiler warning */

   *pimgcnt = 1;

   /* read header info of first bitmap */
   rc = read_pgm_header(fd, &h1, &h2, &gbm.w, &gbm.h, &m, &data_bytes);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }

   /* find last available image index */

   /* we only support multipage images for binary type P5 */
   if (h2 == '5')
   {
      long image_start;

      image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;

      /* index 0 has already been read */

      /* move file pointer to beginning of the next bitmap */
      while (gbm_file_lseek(fd, image_start, GBM_SEEK_SET) >= 0)
      {
         /* read header info of next bitmap */
         rc = read_pgm_header(fd, &h1, &h2, &gbm.w, &gbm.h, &m, &data_bytes);
         if (rc != GBM_ERR_OK)
         {
            break;
         }
         (*pimgcnt)++;

         image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;
      }
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

static GBM_ERR internal_pgm_rhdr(int fd, GBM * gbm, GBM * gbm_src, int * type)
{
   GBM_ERR rc;
   int     h1, h2, m, data_bytes;
   gbm_boolean use_native_bpp;
   const char *s = NULL;

   PGM_PRIV_READ *pgm_priv = (PGM_PRIV_READ *) gbm->priv;

   /* check if extended color depths are requested */
   use_native_bpp = (gbm_find_word(pgm_priv->read_options, "ext_bpp") != NULL)
                    ? GBM_TRUE : GBM_FALSE;

   /* start at the beginning of the file */
   gbm_file_lseek(fd, 0, GBM_SEEK_SET);

   /* read header info of first bitmap */
   rc = read_pgm_header(fd, &h1, &h2, &gbm->w, &gbm->h, &m, &data_bytes);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }
   *type = h2;

   /* goto requested image index */
   if ((s = gbm_find_word_prefix(pgm_priv->read_options, "index=")) != NULL)
   {
      int  image_index_curr;
      long image_start;
      int  image_index = 0;
      if (sscanf(s + 6, "%d", &image_index) != 1)
      {
         return GBM_ERR_BAD_OPTION;
      }

      /* we only support multipage images for binary type P5 */
      if ((h2 != '5') && (image_index != 0))
      {
         return GBM_ERR_BAD_OPTION;
      }

      image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;

      /* index 0 has already been read */
      image_index_curr = 0;
      while (image_index_curr < image_index)
      {
         /* move file pointer to beginning of the next bitmap */
         if (gbm_file_lseek(fd, image_start, GBM_SEEK_SET) < 0)
         {
            return GBM_ERR_READ;
         }

         /* read header info of next bitmap */
         rc = read_pgm_header(fd, &h1, &h2, &gbm->w, &gbm->h, &m, &data_bytes);
         if (rc != GBM_ERR_OK)
         {
            return rc;
         }

         image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;
         image_index_curr++;
      }
   }

   pgm_priv->max_intensity = (unsigned int) m;

   /* check whether 1 byte or 2 byte format */
   if (pgm_priv->max_intensity < 0x100)
   {
      gbm    ->bpp = 8;
      gbm_src->bpp = 8;
   }
   else if (pgm_priv->max_intensity < 0x10000)
   {
      gbm    ->bpp = use_native_bpp ? 48 : 8;
      gbm_src->bpp = 16;
   }
   else
   {
      return GBM_ERR_PGM_BAD_M;
   }

   gbm_src->w = gbm->w;
   gbm_src->h = gbm->h;

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pgm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
   PGM_PRIV_READ *pgm_priv = (PGM_PRIV_READ *) gbm->priv;
   GBM gbm_src;
   int type;

   fn=fn; /* Suppress 'unref arg' compiler warnings */

   /* copy possible options */
   if (strlen(opt) >= sizeof(pgm_priv->read_options))
   {
      return GBM_ERR_BAD_OPTION;
   }
   strcpy(pgm_priv->read_options, opt);

   /* read bitmap info */
   return internal_pgm_rhdr(fd, gbm, &gbm_src, &type);
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pgm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
  PGM_PRIV_READ *pgm_priv = (PGM_PRIV_READ *) gbm->priv;

  int type;
  GBM gbm_src;

  GBM_ERR rc = internal_pgm_rhdr(fd, gbm, &gbm_src, &type);
  if (rc != GBM_ERR_OK)
  {
    return rc;
  }

  if (gbm->bpp == 8)
  {
    /* check whether 1 byte or 2 byte format */
    if (pgm_priv->max_intensity < 0x100)
    {
      unsigned int i;
      for ( i = 0; i <= pgm_priv->max_intensity; i++ )
      {
        gbmrgb[i].r =
        gbmrgb[i].g =
        gbmrgb[i].b = (gbm_u8) (i * 255U / pgm_priv->max_intensity);
      }
      for ( i = pgm_priv->max_intensity + 1; i <= 255; i++ )
      {
        gbmrgb[i].r =
        gbmrgb[i].g =
        gbmrgb[i].b = 0;
      }
    }
    else if (pgm_priv->max_intensity < 0x10000)
    {
      unsigned int i;
      for ( i = 0; i <= 255; i++ )
      {
        gbmrgb[i].r =
        gbmrgb[i].g =
        gbmrgb[i].b = (gbm_u8) (i * 65535U / pgm_priv->max_intensity);
      }
    }
    else
    {
      return GBM_ERR_PGM_BAD_M;
    }
  }

  return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pgm_rdata(int fd, GBM *gbm, gbm_u8 *data)
{
  PGM_PRIV_READ *pgm_priv = (PGM_PRIV_READ *) gbm->priv;

  int type = 0;
  GBM gbm_src;

  const size_t stride = ((gbm->w * gbm->bpp + 31)/32) * 4;

  GBM_ERR rc = internal_pgm_rhdr(fd, gbm, &gbm_src, &type);
  if (rc != GBM_ERR_OK)
  {
     return rc;
  }
  /* check for correct parameters */
  if (stride != (size_t)((gbm->w * gbm->bpp + 31)/32) * 4)
  {
     return GBM_ERR_READ;
  }

  /* binary type P5 */
  if (type == '5')
  {
     int i;
     gbm_u8 * p = data + (stride * (gbm->h - 1));

     const int line_bytes = gbm_src.w * (gbm_src.bpp / 8);

     switch(gbm->bpp)
     {
        case 8:
           switch(gbm_src.bpp)
           {
              case 8:
                 for (i = gbm->h - 1; i >= 0; i--)
                 {
                    if (gbm_file_read(fd, p, line_bytes) != line_bytes)
                    {
                      return GBM_ERR_READ;
                    }
                    p -= stride;
                 }
                 break;

             case 16:
                 {
                    gbm_u8 * src_data = (gbm_u8 *) gbmmem_malloc(line_bytes);
                    if (src_data == NULL)
                    {
                       return GBM_ERR_MEM;
                    }
                    for (i = gbm->h - 1; i >= 0; i--)
                    {
                       if (gbm_file_read(fd, src_data, line_bytes) != line_bytes)
                       {
                         gbmmem_free(src_data);
                         return GBM_ERR_READ;
                       }
                       g16msb_g8((const gbm_u16 *) src_data, p, gbm->w);
                       p -= stride;
                    }
                    gbmmem_free(src_data);
                 }
                 break;

             default:
                 return GBM_ERR_READ;
           }
           break;

        /* ---------------- */

        case 48:
           if (gbm_src.bpp == 16)
           {
              gbm_u8 * src_data = (gbm_u8 *) gbmmem_malloc(line_bytes);
              if (src_data == NULL)
              {
                 return GBM_ERR_MEM;
              }
              for (i = gbm->h - 1; i >= 0; i--)
              {
                 if (gbm_file_read(fd, src_data, line_bytes) != line_bytes)
                 {
                   gbmmem_free(src_data);
                   return GBM_ERR_READ;
                 }
                 g16msb_bgr16lsb((const gbm_u16 *) src_data, (gbm_u16 *) p, gbm->w, pgm_priv->max_intensity);
                 p -= stride;
              }
              gbmmem_free(src_data);
           }
           else
           {
              return GBM_ERR_READ;
           }
           break;

        default:
           return GBM_ERR_READ;
     }
  }
  /* ASCII type P2 */
  else if (type == '2')
  {
    int    i, x;
    int    num;
    gbm_u8  * pNumFill8  = NULL;
    gbm_u16 * pNumFill16 = NULL;
    gbm_u8  * p = data + (stride * (gbm->h - 1));

    AHEAD * ahead = gbm_create_ahead(fd);
    if (ahead == NULL)
    {
        return GBM_ERR_MEM;
    }

     switch(gbm->bpp)
     {
        case 8:
           switch(gbm_src.bpp)
           {
              case 8:
                 for (i = gbm->h - 1; i >= 0; i--)
                 {
                    pNumFill8 = p;
                    for (x = 0; x < gbm_src.w; x++)
                    {
                      num = read_num_data(ahead);
                      if ((num < 0) || (num > 0xff))
                      {
                        gbm_destroy_ahead(ahead);
                        return GBM_ERR_READ;
                      }
                      *pNumFill8++ = (gbm_u8) num;
                    }
                    p -= stride;
                 }
                 break;

             case 16:
                 for (i = gbm->h - 1; i >= 0; i--)
                 {
                    pNumFill8 = p;
                    for (x = 0; x < gbm_src.w; x++)
                    {
                      num = read_num_data(ahead);
                      if ((num < 0) || (num > 0xffff))
                      {
                        gbm_destroy_ahead(ahead);
                        return GBM_ERR_READ;
                      }
                      *pNumFill8++ = (gbm_u8) CVT(num);
                    }
                    p -= stride;
                 }
                 break;

             default:
                 return GBM_ERR_READ;
           }
           break;

        /* ---------------- */

        case 48:
           for (i = gbm->h - 1; i >= 0; i--)
           {
              pNumFill16 = (gbm_u16 *) p;
              for (x = 0; x < gbm_src.w; x++)
              {
                num = read_num_data(ahead);
                if ((num < 0) || (num > 0xffff))
                {
                  gbm_destroy_ahead(ahead);
                  return GBM_ERR_READ;
                }
                *pNumFill16++ = (gbm_u16) num;
                *pNumFill16++ = (gbm_u16) num;
                *pNumFill16++ = (gbm_u16) num;
              }
              p -= stride;
           }
           break;

        default:
           return GBM_ERR_READ;
     }

     gbm_destroy_ahead(ahead);
  }
  else
  {
    return GBM_ERR_NOT_SUPP;
  }

  return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static gbm_boolean internal_pgm_w_ascii(WCACHE * wcache, const gbm_u8 * data, int bytes)
{
  gbm_u8 d[4] = { ' ' };
  int  b, c;

  d[3] = ' ';

  /* write the bytes */
  c  = 0;
  for (b = 0; b < bytes; b++)
  {
    if (c > 66)
    {
      c = 0;
      if (gbm_write_wcache(wcache, '\n') != 1)
      {
        return GBM_FALSE;
      }
    }

    /* write always 3 digits, also if they are 0 */
    d[0] = '0' +   (*data / 100);
    d[1] = '0' + (((*data % 100) - (*data % 10)) / 10);
    d[2] = '0' +   (*data % 10);
    /* d[3] is already filled in before the loop */

    if (gbm_writebuf_wcache(wcache, d, 4) != 4)
    {
      return GBM_FALSE;
    }
    c += 4;
    data++;
  }

  if (gbm_write_wcache(wcache, '\n') != 1)
  {
    return GBM_FALSE;
  }

  return GBM_TRUE;
}

static gbm_boolean internal_pgm_w24_ascii(WCACHE * wcache, const gbm_u8 * data, int tripels, const char *opt)
{
  gbm_u8 d[4] = { ' ' };
  gbm_u8 r,g,b;
  int c, t;

  const gbm_boolean kb = ( gbm_find_word(opt, "k") != NULL );
  const gbm_boolean rb = ( gbm_find_word(opt, "r") != NULL );
  const gbm_boolean gb = ( gbm_find_word(opt, "g") != NULL );
  const gbm_boolean bb = ( gbm_find_word(opt, "b") != NULL );

  d[3] = ' ';

  /* write the bytes */
  c  = 0;
  for (t = 0; t < tripels; t++)
  {
    if (c > 66)
    {
      c = 0;
      if (gbm_write_wcache(wcache, '\n') != 1)
      {
        return GBM_FALSE;
      }
    }

    b = *data++;
    g = *data++;
    r = *data++;

    switch ( SW4(kb,rb,gb,bb) )
    {
      case SW4(0,0,0,0):
        /* Default is the same as "k" */
      case SW4(1,0,0,0):
        g = (gbm_u8) ( ((gbm_u16) r *  77U +
                        (gbm_u16) g * 150U +
                        (gbm_u16) b *  29U) >> 8 );
        break;

      case SW4(0,1,0,0):
        g = r;
        break;

      case SW4(0,0,1,0):
        break;

      case SW4(0,0,0,1):
        g = b;
        break;

      default:
        return GBM_FALSE;
    }

    /* write always 3 digits, also if they are 0 */
    d[0] = '0' +   (g / 100);
    d[1] = '0' + (((g % 100) - (g % 10)) / 10);
    d[2] = '0' +   (g % 10);
    /* d[3] is already filled in before the loop */

    if (gbm_writebuf_wcache(wcache, d, 4) != 4)
    {
      return GBM_FALSE;
    }
    c += 4;
  }

  if (gbm_write_wcache(wcache, '\n') != 1)
  {
    return GBM_FALSE;
  }

  return GBM_TRUE;
}

static gbm_boolean internal_pgm_w_ascii_word(WCACHE * wcache, const gbm_u16 * data16, int words, const char *opt)
{
  int  w, c;
  gbm_u8  d[6] = { ' ' };
  gbm_u16 r,g,b;

  const gbm_boolean kb = ( gbm_find_word(opt, "k") != NULL );
  const gbm_boolean rb = ( gbm_find_word(opt, "r") != NULL );
  const gbm_boolean gb = ( gbm_find_word(opt, "g") != NULL );
  const gbm_boolean bb = ( gbm_find_word(opt, "b") != NULL );

  d[5]  = ' ';

  /* write the bytes */
  c  = 0;
  for (w = 0; w < words; w++)
  {
    if (c > 64)
    {
      c = 0;
      if (gbm_write_wcache(wcache, '\n') != 1)
      {
        return GBM_FALSE;
      }
    }

    b = *data16++;
    g = *data16++;
    r = *data16++;

    switch ( SW4(kb,rb,gb,bb) )
    {
      case SW4(0,0,0,0):
        /* Default is the same as "k" */
      case SW4(1,0,0,0):
        g = (gbm_u16) ( ((gbm_u32) r *  (77U * 255U) +
                         (gbm_u32) g * (150U * 255U) +
                         (gbm_u32) b *  (29U * 255U)) >> 16 );
        break;

      case SW4(0,1,0,0):
        g = r;
        break;

      case SW4(0,0,1,0):
        break;

      case SW4(0,0,0,1):
        g = b;
        break;

      default:
        return GBM_FALSE;
    }

    /* write always 5 digits, also if they are 0 */

    /* grey */
    d[0] = '0' +   (g / 10000);
    d[1] = '0' + (((g % 10000) - (g % 1000)) / 1000);
    d[2] = '0' + (((g % 1000)  - (g % 100))  / 100);
    d[3] = '0' + (((g % 100)   - (g % 10))   / 10);
    d[4] = '0' +   (g % 10);
    /* d[5] is already filled in before the loop */
    if (gbm_writebuf_wcache(wcache, d, 6) != 6)
    {
      return GBM_FALSE;
    }

    c += 6;
  }

  if (gbm_write_wcache(wcache, '\n') != 1)
  {
    return GBM_FALSE;
  }
  return GBM_TRUE;
}

/* ---------------------------------------- */

static gbm_boolean internal_pgm_write_comment(int fd, const char *options)
{
   const char *s;

   if ((s = gbm_find_word_prefix(options, "comment=")) != NULL)
   {
     int   len = 0;
     char  buf[200+1] = { 0 };

     if (sscanf(s + 8, "%200[^\"]", buf) != 1)
     {
        if (sscanf(s + 8, "%200[^ ]", buf) != 1)
        {
           return GBM_FALSE;
        }
     }

     if (gbm_file_write(fd, "# ", 2) != 2)
     {
       return GBM_FALSE;
     }

     len = (int)strlen(buf);
     if (gbm_file_write(fd, (gbm_u8 *)buf, len) != len)
     {
       return GBM_FALSE;
     }

     if (gbm_file_write(fd, "\n", 1) != 1)
     {
       return GBM_FALSE;
     }
   }

   return GBM_TRUE;
}

/* ---------------------------------------- */

GBM_ERR pgm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
{
   char s[100+1] = { 0 };
   int i, j;
   const gbm_u8  *p       = NULL;
         gbm_u8  *linebuf = NULL;
         gbm_u8  * grey8  = NULL;
         WCACHE *wcache   = NULL;

   size_t stride     = 0;
   int    line_bytes = 0;

   const gbm_boolean ascii = ( gbm_find_word(opt, "ascii" ) != NULL );

   fn=fn; /* Suppress 'unref arg' compiler warnings */

   stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
   if ((gbm->bpp == 8) || (gbm->bpp == 24))
   {
     /* output as 8bpp */
     line_bytes = gbm->w;
   }
   else if (gbm->bpp == 48)
   {
     /* output as 16bpp */
     line_bytes = gbm->w * 2;
   }
   else
   {
     return GBM_ERR_NOT_SUPP;
   }
   if ((linebuf = (gbm_u8 *) gbmmem_malloc(line_bytes)) == NULL)
   {
      return GBM_ERR_MEM;
   }

   if (ascii)
   {
     wcache = gbm_create_wcache(fd);
     if (wcache == NULL)
     {
       return GBM_ERR_MEM;
     }
   }

   sprintf(s, "P%c\n", (ascii ? '2' : '5'));
   if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
   {
     gbmmem_free(linebuf);
     if (ascii)
     {
       gbm_destroy_wcache(wcache);
     }
     return GBM_ERR_WRITE;
   }

   /* write optional comment */
   if (! internal_pgm_write_comment(fd, opt))
   {
     gbmmem_free(linebuf);
     if (ascii)
     {
       gbm_destroy_wcache(wcache);
     }
     return GBM_ERR_WRITE;
   }

   switch(gbm->bpp)
   {
      case 8:
         sprintf(s, "%d %d\n255\n", gbm->w, gbm->h);
         if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
         {
           gbmmem_free(linebuf);
           if (ascii)
           {
             gbm_destroy_wcache(wcache);
           }
           return GBM_ERR_WRITE;
         }

         grey8 = (gbm_u8 *) gbmmem_malloc(0x100);
         if (grey8 == NULL)
         {
           gbmmem_free(linebuf);
           if (ascii)
           {
             gbm_destroy_wcache(wcache);
           }
           return GBM_ERR_MEM;
         }
         if (! make_output_palette(gbmrgb, grey8, opt))
         {
           gbmmem_free(grey8);
           gbmmem_free(linebuf);
           if (ascii)
           {
             gbm_destroy_wcache(wcache);
           }
           return GBM_ERR_BAD_OPTION;
         }

         p = data + (stride * (gbm->h - 1));
         for ( i = gbm->h - 1; i >= 0; i-- )
         {
           for ( j = 0; j < gbm->w; j++ )
           {
             linebuf[j] = grey8[p[j]];
           }

           /* write as ASCII pattern or as binary data */
           if (ascii)
           {
             if (! internal_pgm_w_ascii(wcache, linebuf, gbm->w))
             {
               gbmmem_free(grey8);
               gbmmem_free(linebuf);
               gbm_destroy_wcache(wcache);
               return GBM_ERR_WRITE;
             }
           }
           else
           {
             if (gbm_file_write(fd, linebuf, gbm->w) != gbm->w)
             {
               gbmmem_free(grey8);
               gbmmem_free(linebuf);
               if (ascii)
               {
                 gbm_destroy_wcache(wcache);
               }
               return GBM_ERR_WRITE;
             }
           }
           p -= stride;
         }
         gbmmem_free(grey8); grey8 = NULL;
         break;

      /* ------------------------------ */

      case 24:
         sprintf(s, "%d %d\n255\n", gbm->w, gbm->h);
         if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
         {
           gbmmem_free(linebuf);
           if (ascii)
           {
             gbm_destroy_wcache(wcache);
           }
           return GBM_ERR_WRITE;
         }

         p = data + (stride * (gbm->h - 1));
         for (i = gbm->h - 1; i >= 0; i--)
         {
            /* write as ASCII pattern or as binary data */
            if (ascii)
            {
              if (! internal_pgm_w24_ascii(wcache, p, gbm->w, opt))
              {
                gbm_destroy_wcache(wcache);
                return GBM_ERR_WRITE;
              }
            }
            else
            {
              bgr_g(p, linebuf, gbm->w, opt);
              if (gbm_file_write(fd, linebuf, line_bytes) != line_bytes)
              {
                gbmmem_free(linebuf);
                return GBM_ERR_WRITE;
              }
            }
            p -= stride;
         }
         break;

      /* ------------------------------ */

      case 48:
         sprintf(s, "%d %d\n65535\n", gbm->w, gbm->h);
         if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
         {
           gbmmem_free(linebuf);
           if (ascii)
           {
             gbm_destroy_wcache(wcache);
           }
           return GBM_ERR_WRITE;
         }

         p = data + (stride * (gbm->h - 1));
         for (i = gbm->h - 1; i >= 0; i--)
         {
            /* write as ASCII pattern or as binary data */
            if (ascii)
            {
              if (! internal_pgm_w_ascii_word(wcache, (const gbm_u16 *) p, gbm->w, opt))
              {
                gbm_destroy_wcache(wcache);
                return GBM_ERR_WRITE;
              }
            }
            else
            {
              bgr16lsb_g16msb((const gbm_u16 *) p, (gbm_u16 *) linebuf, gbm->w, opt);
              if (gbm_file_write(fd, linebuf, line_bytes) != line_bytes)
              {
                gbmmem_free(linebuf);
                return GBM_ERR_WRITE;
              }
            }
            p -= stride;
         }
         break;

      default:
         gbmmem_free(linebuf);
         if (ascii)
         {
           gbm_destroy_wcache(wcache);
         }
         return GBM_ERR_NOT_SUPP;
   }

   gbmmem_free(linebuf);
   if (ascii)
   {
     if (gbm_destroy_wcache(wcache) == -1)
     {
       return GBM_ERR_WRITE;
     }
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

const char *pgm_err(GBM_ERR rc)
{
  switch ( (int) rc )
  {
    case GBM_ERR_PGM_BAD_M:
      return "bad maximum pixel intensity";
  }
  return NULL;
}

