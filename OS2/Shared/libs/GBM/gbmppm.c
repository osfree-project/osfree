/*

gbmppm.c - Poskanzers PPM format

Credit for writing this module must go to Heiko Nitzsche.
This file is just as public domain as the rest of GBM.

Supported formats and options:
------------------------------
Pixmap : Portable Pixel-map : .PPM

Standard formats (backward compatible):
  Reads 24 bpp unpalettised RGB files (ASCII format P3, binary format P6).
  Reads 48 bpp unpalettised RGB files and presents them as 24 bpp (ASCII format P3, binary format P6).

Extended formats (not backward compatible, import option ext_bpp required):
  Reads 48 bpp unpalettised RGB files and presents them as 48 bpp (ASCII format P3, binary format P6).

Writes 24 and 48 bpp unpalettised RGB files (ASCII format P3, binary format P6).


Input:
------

Can specify image within PPM file with multiple images (only for P6 type)
  Input option: index=# (default: 0)

Can specify that non-standard GBM color depth is exported (48 bpp)
  Input option: ext_bpp (default: bpp is downsampled to 24 bpp)


Output:
-------

Write ASCII format P3 (default is binary P6)
  Output option: ascii

Write additonal comment
  Output option: comment=text


History:
--------
(Heiko Nitzsche)

19-Feb-2006: Add function to query number of images
22-Feb-2006: Move format description strings to gbmdesc.h
10-Jun-2006: Add some additional error checking
16-Jun-2006: Add support for ASCII read/write
23-Sep-2006: Use read ahead and write cache for ASCII format to improve speed
*/

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"

/* ---------------------------------------- */

#define  GBM_ERR_PPM_BAD_M    ((GBM_ERR) 2300)

/* ---------------------------------------- */

static GBMFT ppm_gbmft =
{
   GBM_FMT_DESC_SHORT_PPM,
   GBM_FMT_DESC_LONG_PPM,
   GBM_FMT_DESC_EXT_PPM,
   GBM_FT_R24 | GBM_FT_R48 |
   GBM_FT_W24 | GBM_FT_W48
};


typedef struct
{
   unsigned int max_intensity;

   /* This entry will store the options provided during first header read.
    * It will keep the options for the case the header has to be reread.
    */
   char read_options[PRIV_SIZE - sizeof(int)
                               - 8 /* space for structure element padding */ ];

} PPM_PRIV_READ;


/* ---------------------------------------- */

static void rgb_bgr(const byte *p, byte *q, int n, const unsigned int max_intensity)
{
   byte r, g, b;

   if (max_intensity < 255)
   {
      while ( n-- )
      {
         r = *p++;
         g = *p++;
         b = *p++;

         *q++ = (byte) ((b * 255U) / max_intensity);
         *q++ = (byte) ((g * 255U) / max_intensity);
         *q++ = (byte) ((r * 255U) / max_intensity);
      }
   }
   else
   {
      while ( n-- )
      {
         r = *p++;
         g = *p++;
         b = *p++;

         *q++ = b;
         *q++ = g;
         *q++ = r;
      }
   }
}

static void rgb16msb_bgr(const byte * p, byte * q, int n, const unsigned int max_intensity)
{
   word r, g, b;

   word * p16 = (word *) p;

   if (max_intensity < 65535)
   {
      while ( n-- )
      {
         r = *p16++;
         g = *p16++;
         b = *p16++;

         swab((char *) &r, (char *) &r, 2);
         swab((char *) &g, (char *) &g, 2);
         swab((char *) &b, (char *) &b, 2);

         r = (r * 65535U) / max_intensity;
         g = (g * 65535U) / max_intensity;
         b = (b * 65535U) / max_intensity;

         #define CVT(x) (((x) * 255) / ((1L << 16) - 1))

         *q++ = CVT(b);
         *q++ = CVT(g);
         *q++ = CVT(r);
      }
   }
   else
   {
      while ( n-- )
      {
         r = *p16++;
         g = *p16++;
         b = *p16++;

         swab((char *) &r, (char *) &r, 2);
         swab((char *) &g, (char *) &g, 2);
         swab((char *) &b, (char *) &b, 2);

         #define CVT(x) (((x) * 255) / ((1L << 16) - 1))

         *q++ = CVT(b);
         *q++ = CVT(g);
         *q++ = CVT(r);
      }
   }
}

static void rgb16msb_bgr16lsb(const byte *p, byte *q, int n, const unsigned int max_intensity)
{
   word r, g, b;

   word * p16 = (word *) p;
   word * q16 = (word *) q;

   if (max_intensity < 65535)
   {
      while ( n-- )
      {
         r = *p16++;
         g = *p16++;
         b = *p16++;

         swab((char *) &r, (char *) &r, 2);
         swab((char *) &g, (char *) &g, 2);
         swab((char *) &b, (char *) &b, 2);

         *q16++ = (b * 65535U) / max_intensity;
         *q16++ = (g * 65535U) / max_intensity;
         *q16++ = (r * 65535U) / max_intensity;
      }
   }
   else
   {
      while ( n-- )
      {
         r = *p16++;
         g = *p16++;
         b = *p16++;

         swab((char *) &r, (char *) &r, 2);
         swab((char *) &g, (char *) &g, 2);
         swab((char *) &b, (char *) &b, 2);

         *q16++ = b;
         *q16++ = g;
         *q16++ = r;
      }
   }
}

/* ---------------------------------------- */

static byte read_byte(int fd)
{
   byte b = 0;
   gbm_file_read(fd, (char *) &b, 1);
   return b;
}

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

   while ( isspace(c = read_char(fd)) );

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

GBM_ERR ppm_qft(GBMFT *gbmft)
{
   *gbmft = ppm_gbmft;
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static GBM_ERR read_ppm_header(int fd, int *h1, int *h2, int *w, int *h, int *m, int *data_bytes)
{
   *h1 = read_byte(fd);
   *h2 = read_byte(fd);
   if ( (*h1 != 'P') || ((*h2 != '3') && (*h2 != '6')) )
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
      return GBM_ERR_PPM_BAD_M;
   }

   /* check whether 1 byte or 2 byte format */
   if (*m < 0x100)
   {
      *data_bytes = 3 * (*w) * (*h);
   }
   else if (*m < 0x10000)
   {
      *data_bytes = 6 * (*w) * (*h);
   }
   else
   {
      return GBM_ERR_PPM_BAD_M;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read number of images in the PPM file. */
GBM_ERR ppm_rimgcnt(const char *fn, int fd, int *pimgcnt)
{
   GBM_ERR rc;
   GBM     gbm;
   int     h1, h2, m, data_bytes;

   fn=fn; /* suppress compiler warning */

   *pimgcnt = 1;

   /* read header info of first bitmap */
   rc = read_ppm_header(fd, &h1, &h2, &gbm.w, &gbm.h, &m, &data_bytes);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }

   /* find last available image index */

   /* we only support multipage images for binary type P6 */
   if (h2 == '6')
   {
      long image_start;

      image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;

      /* index 0 has already been read */

      /* move file pointer to beginning of the next bitmap */
      while (gbm_file_lseek(fd, image_start, GBM_SEEK_SET) >= 0)
      {
         /* read header info of next bitmap */
         rc = read_ppm_header(fd, &h1, &h2, &gbm.w, &gbm.h, &m, &data_bytes);
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

static GBM_ERR internal_ppm_rhdr(int fd, GBM * gbm, GBM * gbm_src, int * type)
{
   GBM_ERR rc;
   int     h1, h2, m, data_bytes;
   BOOLEAN use_native_bpp;
   const char *s = NULL;

   PPM_PRIV_READ *ppm_priv = (PPM_PRIV_READ *) gbm->priv;

   /* check if extended color depths are requested */
   use_native_bpp = (gbm_find_word(ppm_priv->read_options, "ext_bpp") != NULL)
                    ? TRUE : FALSE;

   /* start at the beginning of the file */
   gbm_file_lseek(fd, 0, GBM_SEEK_SET);

   /* read header info of first bitmap */
   rc = read_ppm_header(fd, &h1, &h2, &gbm->w, &gbm->h, &m, &data_bytes);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }
   *type = h2;

   /* goto requested image index */
   if ((s = gbm_find_word_prefix(ppm_priv->read_options, "index=")) != NULL)
   {
      int  image_index_curr;
      long image_start;
      int  image_index = 0;
      if (sscanf(s + 6, "%d", &image_index) != 1)
      {
         return GBM_ERR_BAD_OPTION;
      }

      /* we only support multipage images for binary type P6 */
      if ((h2 != '6') && (image_index != 0))
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
         rc = read_ppm_header(fd, &h1, &h2, &gbm->w, &gbm->h, &m, &data_bytes);
         if (rc != GBM_ERR_OK)
         {
            return rc;
         }

         image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;
         image_index_curr++;
      }
   }
   ppm_priv->max_intensity = (unsigned int) m;

   /* check whether 1 byte or 2 byte format */
   if (ppm_priv->max_intensity < 0x100)
   {
      gbm    ->bpp = 24;
      gbm_src->bpp = 24;
   }
   else if (ppm_priv->max_intensity < 0x10000)
   {
      gbm    ->bpp = use_native_bpp ? 48 : 24;
      gbm_src->bpp = 48;
   }
   else
   {
      return GBM_ERR_PPM_BAD_M;
   }

   gbm_src->w = gbm->w;
   gbm_src->h = gbm->h;

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

GBM_ERR ppm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
   PPM_PRIV_READ *ppm_priv = (PPM_PRIV_READ *) gbm->priv;
   GBM gbm_src;
   int type;

   fn=fn; /* Suppress 'unref arg' compiler warnings */

   /* copy possible options */
   if (strlen(opt) >= sizeof(ppm_priv->read_options))
   {
      return GBM_ERR_BAD_OPTION;
   }
   strcpy(ppm_priv->read_options, opt);

   /* read bitmap info */
   return internal_ppm_rhdr(fd, gbm, &gbm_src, &type);
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR ppm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
   fd=fd; gbm=gbm; gbmrgb=gbmrgb; /* Suppress 'unref arg' compiler warnings */

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR ppm_rdata(int fd, GBM *gbm, byte *data)
{
   PPM_PRIV_READ *ppm_priv = (PPM_PRIV_READ *) gbm->priv;

   int type = 0;
   GBM gbm_src;

   const int stride = ((gbm->w * gbm->bpp + 31)/32) * 4;

   GBM_ERR rc = internal_ppm_rhdr(fd, gbm, &gbm_src, &type);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }
   /* check for correct parameters */
   if (stride != ((gbm->w * gbm->bpp + 31)/32) * 4)
   {
      return GBM_ERR_READ;
   }

   /* binary type P6 */
   if (type == '6')
   {
     int i;
     const int line_bytes = gbm_src.w * (gbm_src.bpp / 8);

     byte * p = data + ((gbm->h - 1) * stride);

     switch(gbm->bpp)
     {
        case 24:
           /* check whether 1 byte or 2 byte format */
           if (ppm_priv->max_intensity < 0x100)
           {
              for (i = gbm->h - 1; i >= 0; i--)
              {
                 if (gbm_file_read(fd, p, line_bytes) != line_bytes)
                 {
                   return GBM_ERR_READ;
                 }
                 rgb_bgr(p, p, gbm->w, ppm_priv->max_intensity);
                 p -= stride;
              }
           }
           else
           {
              byte * src_data = (byte *) malloc(line_bytes);
              if (src_data == NULL)
              {
                 return GBM_ERR_MEM;
              }
              for (i = gbm->h - 1; i >= 0; i--)
              {
                 if (gbm_file_read(fd, src_data, line_bytes) != line_bytes)
                 {
                   free(src_data);
                   return GBM_ERR_READ;
                 }
                 rgb16msb_bgr(src_data, p, gbm->w, ppm_priv->max_intensity);
                 p -= stride;
              }
              free(src_data);
           }
           break;

        case 48:
           for (i = gbm->h - 1; i >= 0; i--)
           {
              if (gbm_file_read(fd, p, line_bytes) != line_bytes)
              {
                return GBM_ERR_READ;
              }
              rgb16msb_bgr16lsb(p, p, gbm->w, ppm_priv->max_intensity);
              p -= stride;
           }
           break;

        default:
           return GBM_ERR_READ;
     }
   }
   /* ASCII type P3 */
   else if (type == '3')
   {
     int    i, x;
     int    num_r, num_g, num_b;
     byte * pNumFill8;
     word * pNumFill16;
     byte * p = data + ((gbm->h - 1) * stride);

     AHEAD * ahead = gbm_create_ahead(fd);
     if (ahead == NULL)
     {
         return GBM_ERR_MEM;
     }

     switch(gbm->bpp)
     {
        case 24:
           /* check whether 1 byte or 2 byte format */
           if (ppm_priv->max_intensity < 0x100)
           {
              for (i = gbm->h - 1; i >= 0; i--)
              {
                 pNumFill8 = p;
                 for (x = 0; x < gbm_src.w; x++)
                 {
                   /* RGB order in ASCII but BGR in GBM */
                   num_r = read_num_data(ahead);
                   num_g = read_num_data(ahead);
                   num_b = read_num_data(ahead);

                   if ((num_r < 0) || (num_r > 0xff) ||
                       (num_g < 0) || (num_g > 0xff) ||
                       (num_b < 0) || (num_b > 0xff))
                   {
                     gbm_destroy_ahead(ahead);
                     return GBM_ERR_READ;
                   }
                   *pNumFill8++ = (byte) num_b;
                   *pNumFill8++ = (byte) num_g;
                   *pNumFill8++ = (byte) num_r;
                 }
                 p -= stride;
              }
           }
           else
           {
              for (i = gbm->h - 1; i >= 0; i--)
              {
                 pNumFill8 = p;
                 for (x = 0; x < gbm_src.w; x++)
                 {
                   /* RGB order in ASCII but BGR in GBM */
                   num_r = read_num_data(ahead);
                   num_g = read_num_data(ahead);
                   num_b = read_num_data(ahead);

                   if ((num_r < 0) || (num_r > 0xffff) ||
                       (num_g < 0) || (num_g > 0xffff) ||
                       (num_b < 0) || (num_b > 0xffff))
                   {
                     gbm_destroy_ahead(ahead);
                     return GBM_ERR_READ;
                   }

                   #define CVT(x) (((x) * 255) / ((1L << 16) - 1))
                   *pNumFill8++ = (byte) CVT(num_b);
                   *pNumFill8++ = (byte) CVT(num_g);
                   *pNumFill8++ = (byte) CVT(num_r);
                   #undef CVT
                 }
                 p -= stride;
              }
           }
           break;

        case 48:
           for (i = gbm->h - 1; i >= 0; i--)
           {
              pNumFill16 = (word *) p;
              for (x = 0; x < gbm_src.w; x++)
              {
                /* RGB order in ASCII but BGR in GBM */
                num_r = read_num_data(ahead);
                num_g = read_num_data(ahead);
                num_b = read_num_data(ahead);

                if ((num_r < 0) || (num_r > 0xffff) ||
                    (num_g < 0) || (num_g > 0xffff) ||
                    (num_b < 0) || (num_b > 0xffff))
                {
                  gbm_destroy_ahead(ahead);
                  return GBM_ERR_READ;
                }
                *pNumFill16++ = (word) num_b;
                *pNumFill16++ = (word) num_g;
                *pNumFill16++ = (word) num_r;
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

static BOOLEAN internal_ppm_w_ascii_byte_bgr2rgb(WCACHE * wcache, const byte * data, int words)
{
  int  w, c;
  char d[12] = { 0 };
  char r,g,b;

  d[3]  = ' ';
  d[7]  = ' ';
  d[11] = ' ';

  /* write the bytes */
  c  = 0;
  for (w = 0; w < words; w++)
  {
    if (c > 58)
    {
      c = 0;
      if (gbm_write_wcache(wcache, '\n') != 1)
      {
        return FALSE;
      }
    }

    b = *data++;
    g = *data++;
    r = *data++;

    /* write always 3 digits, also if they are 0 */

    /* red */
    d[0] = '0' +   (r / 100);
    d[1] = '0' + (((r % 100) - (r % 10)) / 10);
    d[2] = '0' +   (r % 10);
    /* d[3] is already filled in before the loop */

    /* green */
    d[4] = '0' +   (g / 100);
    d[5] = '0' + (((g % 100) - (g % 10)) / 10);
    d[6] = '0' +   (g % 10);
    /* d[7] is already filled in before the loop */

    /* blue */
    d[8]  = '0' +   (b / 100);
    d[9]  = '0' + (((b % 100) - (b % 10)) / 10);
    d[10] = '0' +   (b % 10);
    /* d[11] is already filled in before the loop */

    if (gbm_writebuf_wcache(wcache, d, 12) != 12)
    {
      return FALSE;
    }

    c += 12;
  }

  if (gbm_write_wcache(wcache, '\n') != 1)
  {
    return FALSE;
  }
  return TRUE;
}

static BOOLEAN internal_ppm_w_ascii_word_bgr2rgb(WCACHE * wcache, const word * data16, int words)
{
  int  w, c;
  char d[18] = { 0 };
  word r,g,b;

  d[5]  = ' ';
  d[11] = ' ';
  d[17] = ' ';

  /* write the bytes */
  c  = 0;
  for (w = 0; w < words; w++)
  {
    if (c > 52)
    {
      c = 0;
      if (gbm_write_wcache(wcache, '\n') != 1)
      {
        return FALSE;
      }
    }

    b = *data16++;
    g = *data16++;
    r = *data16++;

    /* write always 5 digits, also if they are 0 */

    /* red */
    d[0] = '0' +   (r / 10000);
    d[1] = '0' + (((r % 10000) - (r % 1000)) / 1000);
    d[2] = '0' + (((r % 1000)  - (r % 100))  / 100);
    d[3] = '0' + (((r % 100)   - (r % 10))   / 10);
    d[4] = '0' +   (r % 10);
    /* d[5] is already filled in before the loop */

    /* green */
    d[6]  = '0' +   (g / 10000);
    d[7]  = '0' + (((g % 10000) - (g % 1000)) / 1000);
    d[8]  = '0' + (((g % 1000)  - (g % 100))  / 100);
    d[9]  = '0' + (((g % 100)   - (g % 10))   / 10);
    d[10] = '0' +   (g % 10);
    /* d[11] is already filled in before the loop */

    /* blue */
    d[12] = '0' +   (b / 10000);
    d[13] = '0' + (((b % 10000) - (b % 1000)) / 1000);
    d[14] = '0' + (((b % 1000)  - (b % 100))  / 100);
    d[15] = '0' + (((b % 100)   - (b % 10))   / 10);
    d[16] = '0' +   (b % 10);
    /* d[17] is already filled in before the loop */
    if (gbm_writebuf_wcache(wcache, d, 18) != 18)
    {
      return FALSE;
    }

    c += 18;
  }

  if (gbm_write_wcache(wcache, '\n') != 1)
  {
    return FALSE;
  }
  return TRUE;
}

/* ---------------------------------------- */

static BOOLEAN internal_ppm_write_comment(int fd, const char *options)
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
           return FALSE;
        }
     }

     if (gbm_file_write(fd, "# ", 2) != 2)
     {
       return FALSE;
     }

     len = strlen(buf);
     if (gbm_file_write(fd, buf, len) != len)
     {
       return FALSE;
     }

     if (gbm_file_write(fd, "\n", 1) != 1)
     {
       return FALSE;
     }
   }

   return TRUE;
}

/* ---------------------------------------- */

GBM_ERR ppm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
{
   char  s[100+1];
   int   i;
   const byte *p;
   byte   *linebuf = NULL;
   WCACHE *wcache  = NULL;

   const int stride     = ((gbm->w * gbm->bpp + 31)/32) * 4;
   const int line_bytes = gbm->w * (gbm->bpp / 8);

   const BOOLEAN ascii = ( gbm_find_word(opt, "ascii" ) != NULL );

   fn=fn; gbmrgb=gbmrgb; opt=opt; /* Suppress 'unref arg' compiler warnings */

   sprintf(s, "P%c\n", (ascii ? '3' : '6'));
   if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
   {
     free(linebuf);
     return GBM_ERR_WRITE;
   }

   /* we only need a conversion buffer for binary output */
   if (! ascii)
   {
     if ((linebuf = (byte *) malloc(stride)) == NULL)
     {
        return GBM_ERR_MEM;
     }
   }
   else
   {
     wcache = gbm_create_wcache(fd);
     if (wcache == NULL)
     {
       return GBM_ERR_MEM;
     }
   }

   switch(gbm->bpp)
   {
      case 24:
         /* write optional comment */
         if (! internal_ppm_write_comment(fd, opt))
         {
           free(linebuf);
           if (ascii)
           {
             gbm_destroy_wcache(wcache);
           }
           return GBM_ERR_WRITE;
         }

         sprintf(s, "%d %d\n255\n", gbm->w, gbm->h);
         if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
         {
           free(linebuf);
           if (ascii)
           {
             gbm_destroy_wcache(wcache);
           }
           return GBM_ERR_WRITE;
         }

         p = data + ((gbm->h - 1) * stride);
         for (i = gbm->h - 1; i >= 0; i--)
         {
            /* write as ASCII pattern or as binary data */
            if (ascii)
            {
              if (! internal_ppm_w_ascii_byte_bgr2rgb(wcache, p, gbm->w))
              {
                gbm_destroy_wcache(wcache);
                return GBM_ERR_WRITE;
              }
            }
            else
            {
              rgb_bgr(p, linebuf, gbm->w, 0x100);
              if (gbm_file_write(fd, linebuf, line_bytes) != line_bytes)
              {
                free(linebuf);
                return GBM_ERR_WRITE;
              }
            }
            p -= stride;
         }
         break;

      case 48:
         /* write optional comment */
         if (! internal_ppm_write_comment(fd, opt))
         {
           free(linebuf);
           if (ascii)
           {
             gbm_destroy_wcache(wcache);
           }
           return GBM_ERR_WRITE;
         }

         sprintf(s, "%d %d\n65535\n", gbm->w, gbm->h);
         if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
         {
           free(linebuf);
           if (ascii)
           {
             gbm_destroy_wcache(wcache);
           }
           return GBM_ERR_WRITE;
         }

         p = data + ((gbm->h - 1) * stride);
         for (i = gbm->h - 1; i >= 0; i--)
         {
            /* write as ASCII pattern or as binary data */
            if (ascii)
            {
              if (! internal_ppm_w_ascii_word_bgr2rgb(wcache, (const word *) p, gbm->w))
              {
                gbm_destroy_wcache(wcache);
                return GBM_ERR_WRITE;
              }
            }
            else
            {
              rgb16msb_bgr16lsb(p, linebuf, gbm->w, 0x10000);
              if (gbm_file_write(fd, linebuf, line_bytes) != line_bytes)
              {
                free(linebuf);
                return GBM_ERR_WRITE;
              }
            }
            p -= stride;
         }
         break;

      default:
         free(linebuf);
         if (ascii)
         {
           gbm_destroy_wcache(wcache);
         }
         return GBM_ERR_NOT_SUPP;
   }

   free(linebuf);

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

const char *ppm_err(GBM_ERR rc)
{
   switch ( (int) rc )
   {
      case GBM_ERR_PPM_BAD_M:
         return "bad maximum pixel intensity";
   }
   return NULL;
}

