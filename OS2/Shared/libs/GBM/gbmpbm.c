/*

gbmpbm.c - Poskanzers PBM format


Supported formats and options:
------------------------------
Bit-map : Portable Bit-map : .PBM

Reads  1 bpp black/white images (ASCII format P1, binary format P4).
Writes 1 bpp black/white images (ASCII format P1, binary format P4).

Input:
------

Can specify image within PBM file with multiple images (only for P4 type)
  Input option: index=# (default: 0)

Invert the data bits on input.
  Input option: invb


Output:
-------

Invert the data bits on output.
  Output option: invb

Write ASCII format P1 (default is binary P4)
  Output option: ascii

Write additonal comment
  Output option: comment=text


History:
--------
(Heiko Nitzsche)

10-Jun-2006: Initial release (binary format P4)
11-Jun-2006: Add support for reading multipage images (type P4 only)
12-Jun-2006: Add support for reading & writing ASCII format P1
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

#define  GBM_ERR_PBM_BAD_M    ((GBM_ERR) 2000)

/* ---------------------------------------- */

static GBMFT pbm_gbmft =
{
   GBM_FMT_DESC_SHORT_PBM,
   GBM_FMT_DESC_LONG_PBM,
   GBM_FMT_DESC_EXT_PBM,
   GBM_FT_R1 |
   GBM_FT_W1
};

typedef struct
{
   /* This entry will store the options provided during first header read.
    * It will keep the options for the case the header has to be reread.
    */
   char read_options[PRIV_SIZE - 8 /* space for structure element padding */ ];

} PBM_PRIV_READ;

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
    while ( (c = read_byte(fd)) != '\n' )
      ;
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

static int read_char_byte(AHEAD * ahead, int bits)
{
  int  i;
  char c;
  int  num = 0;

  if ((bits < 1) || (bits > 8))
  {
    return -1;
  }

  for (i = 0; i < bits; i++)
  {
    while ( isspace(c = read_char_ahead(ahead)) )
      ;

    switch(c)
    {
      case '0':
        break;

      case '1':
        num |= (1 << (7-i));
        break;

      case '\n':
      case '\r':
        i--;
        break;

      default:
        return -1;
    }
  }

  return num;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pbm_qft(GBMFT *gbmft)
{
   *gbmft = pbm_gbmft;
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static GBM_ERR read_pbm_header(int fd, int *h1, int *h2, int *w, int *h, int *data_bytes)
{
   *h1 = read_byte(fd);
   *h2 = read_byte(fd);
   if ( (*h1 != 'P') || ((*h2 != '1') && (*h2 != '4')) )
   {
      return GBM_ERR_BAD_MAGIC;
   }

   *w  = read_num(fd);
   *h  = read_num(fd);
   if ((*w <= 0) || (*h <= 0))
   {
      return GBM_ERR_BAD_SIZE;
   }

   *data_bytes = (((*w) + 7) / 8) * (*h);

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read number of images in the PBM file. */
GBM_ERR pbm_rimgcnt(const char *fn, int fd, int *pimgcnt)
{
   GBM_ERR rc;
   GBM     gbm;
   int     h1, h2, data_bytes;

   fn=fn; /* suppress compiler warning */

   *pimgcnt = 1;

   /* read header info of first bitmap */
   rc = read_pbm_header(fd, &h1, &h2, &gbm.w, &gbm.h, &data_bytes);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }

   /* find last available image index */

   /* we only support multipage images for binary type P4 */
   if (h2 == '4')
   {
      long image_start;

      image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;

      /* index 0 has already been read */

      /* move file pointer to beginning of the next bitmap */
      while (gbm_file_lseek(fd, image_start, GBM_SEEK_SET) >= 0)
      {
         /* read header info of next bitmap */
         rc = read_pbm_header(fd, &h1, &h2, &gbm.w, &gbm.h, &data_bytes);
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

static GBM_ERR internal_pbm_rhdr(int fd, GBM * gbm, int * type)
{
   GBM_ERR rc;
   int     h1, h2, data_bytes;
   const char *s = NULL;

   PBM_PRIV_READ *pbm_priv = (PBM_PRIV_READ *) gbm->priv;

   /* start at the beginning of the file */
   gbm_file_lseek(fd, 0, GBM_SEEK_SET);

   /* read header info of first bitmap */
   rc = read_pbm_header(fd, &h1, &h2, &gbm->w, &gbm->h, &data_bytes);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }
   *type = h2;

   /* goto requested image index */
   if ((s = gbm_find_word_prefix(pbm_priv->read_options, "index=")) != NULL)
   {
      int  image_index_curr;
      long image_start;
      int  image_index = 0;
      if (sscanf(s + 6, "%d", &image_index) != 1)
      {
         return GBM_ERR_BAD_OPTION;
      }

      /* we only support multipage images for binary type P4 */
      if ((h2 != '4') && (image_index != 0))
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
         rc = read_pbm_header(fd, &h1, &h2, &gbm->w, &gbm->h, &data_bytes);
         if (rc != GBM_ERR_OK)
         {
            return rc;
         }

         image_start = gbm_file_lseek(fd, 0, GBM_SEEK_CUR) + data_bytes;
         image_index_curr++;
      }
   }

   gbm->bpp = 1;

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pbm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
   PBM_PRIV_READ *pbm_priv = (PBM_PRIV_READ *) gbm->priv;
   int type;

   fn=fn; /* Suppress 'unref arg' compiler warnings */

   /* copy possible options */
   if (strlen(opt) >= sizeof(pbm_priv->read_options))
   {
      return GBM_ERR_BAD_OPTION;
   }
   strcpy(pbm_priv->read_options, opt);

   /* read bitmap info */
   return internal_pbm_rhdr(fd, gbm, &type);
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pbm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
  int     type;
  GBM_ERR rc = internal_pbm_rhdr(fd, gbm, &type);
  if (rc != GBM_ERR_OK)
  {
    return rc;
  }

  if (gbm->bpp == 1)
  {
    /* index 0 is always white, index 1 always black */
    gbmrgb[0].r =
    gbmrgb[0].g =
    gbmrgb[0].b = 0xff;
    gbmrgb[1].r =
    gbmrgb[1].g =
    gbmrgb[1].b = 0;
  }

  return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR pbm_rdata(int fd, GBM *gbm, byte *data)
{
  PBM_PRIV_READ *pbm_priv = (PBM_PRIV_READ *) gbm->priv;
  int     type = 0;
  BOOLEAN invert;

  const int stride = ((gbm->w * gbm->bpp + 31)/32) * 4;

  GBM_ERR rc = internal_pbm_rhdr(fd, gbm, &type);
  if (rc != GBM_ERR_OK)
  {
     return rc;
  }
  /* check for correct parameters */
  if (stride != ((gbm->w * gbm->bpp + 31)/32) * 4)
  {
     return GBM_ERR_READ;
  }

  invert = ( gbm_find_word(pbm_priv->read_options, "invb" ) != NULL );

  if (gbm->bpp != 1)
  {
     return GBM_ERR_READ;
  }

  /* binary type P4 */
  if (type == '4')
  {
    int i;
    const int line_bytes = (gbm->w + 7) / 8;

    byte * p = data + ((gbm->h - 1) * stride);

    for (i = gbm->h - 1; i >= 0; i--)
    {
       if (gbm_file_read(fd, p, line_bytes) != line_bytes)
       {
          return GBM_ERR_READ;
       }
       if (invert)
       {
          int x;
          byte * pInv = p;

          for (x = 0; x < line_bytes; x++)
          {
            *pInv = ~(*pInv);
            pInv++;
          }
       }
       p -= stride;
    }
  }
  /* ASCII type P1 */
  else if (type == '1')
  {
    int    i, x;
    int    num;
    byte * pNumFill;
    byte * p = data + ((gbm->h - 1) * stride);

    const int line_bytes     = gbm->w / 8;
    const int line_bits_left = gbm->w % 8;

    AHEAD * ahead = gbm_create_ahead(fd);
    if (ahead == NULL)
    {
        return GBM_ERR_MEM;
    }

    for (i = gbm->h - 1; i >= 0; i--)
    {
      pNumFill = p;
      if (invert)
      {
        for (x = 0; x < line_bytes; x++)
        {
          num = read_char_byte(ahead, 8);
          if ((num < 0) || (num > 0xff))
          {
            gbm_destroy_ahead(ahead);
            return GBM_ERR_READ;
          }
          *pNumFill++ = ~((byte) num);
        }
        if (line_bits_left)
        {
          num = read_char_byte(ahead, line_bits_left);
          *pNumFill++ = ~((byte) num);
        }
      }
      else
      {
        for (x = 0; x < line_bytes; x++)
        {
          num = read_char_byte(ahead, 8);
          if ((num < 0) || (num > 0xff))
          {
            gbm_destroy_ahead(ahead);
            return GBM_ERR_READ;
          }
          *pNumFill++ = (byte) num;
        }
        if (line_bits_left)
        {
          num = read_char_byte(ahead, line_bits_left);
          *pNumFill++ = (byte) num;
        }
      }

      p -= stride;
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

static BOOLEAN internal_pbm_w_ascii(WCACHE * wcache, const byte * data, int bits)
{
  char d[2];
  int  b, i, xb, c;
  const int bytes = (bits + 7) / 8;

  /* write the bytes */
  c  = 0;
  xb = 0;
  for (b = 0; b < bytes; b++, c++)
  {
    if (c == 4)
    {
      c = 0;
      if (gbm_write_wcache(wcache, '\n') != 1)
      {
        return FALSE;
      }
    }

    i = 0;
    while ((i < 8) && (xb < bits))
    {
      d[0] = (((*data) >> (7-i)) & 1) + '0';
      d[1] = ' ';
      if (gbm_writebuf_wcache(wcache, d, 2) != 2)
      {
        return FALSE;
      }
      i++;
      xb++;
    }
    data++;
  }

  if (gbm_write_wcache(wcache, '\n') != 1)
  {
    return FALSE;
  }

  return TRUE;
}

/* ---------------------------------------- */

static BOOLEAN internal_pbm_write_comment(int fd, const char *options)
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

GBM_ERR pbm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
{
  char s[100+1];
  int i, stride, line_bytes;
  const byte *p;
  byte   * linebuf = NULL;
  WCACHE * wcache  = NULL;

  const BOOLEAN invert = ( gbm_find_word(opt, "invb"  ) != NULL );
  const BOOLEAN ascii  = ( gbm_find_word(opt, "ascii" ) != NULL );

  fn=fn; opt=opt; gbmrgb=gbmrgb; /* Suppress 'unref arg' compiler warnings */

  if (gbm->bpp != 1)
  {
    return GBM_ERR_NOT_SUPP;
  }

  line_bytes = (gbm->w + 7) / 8;

  sprintf(s, "P%c\n", (ascii ? '1' : '4'));
  if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
  {
    return GBM_ERR_WRITE;
  }

  /* write optional comment */
  if (! internal_pbm_write_comment(fd, opt))
  {
    free(linebuf);
    return GBM_ERR_WRITE;
  }

  sprintf(s, "%d %d\n", gbm->w, gbm->h);
  if (gbm_file_write(fd, s, (int) strlen(s)) != (int) strlen(s))
  {
    return GBM_ERR_WRITE;
  }

  if (ascii)
  {
    wcache = gbm_create_wcache(fd);
    if (wcache == NULL)
    {
      return GBM_ERR_MEM;
    }
  }
  if (invert)
  {
    linebuf = (byte *) malloc((size_t) line_bytes);
    if (linebuf == NULL)
    {
      if (ascii)
      {
        gbm_destroy_wcache(wcache);
      }
      return GBM_ERR_MEM;
    }
  }

  stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
  p = data + ((gbm->h - 1) * stride);
  for ( i = gbm->h - 1; i >= 0; i-- )
  {
    const byte * writep = p;
    if (invert)
    {
       int x;
       const byte * pTemp = p;
       byte * pInv = linebuf;

       for (x = 0; x < line_bytes; x++)
       {
         *pInv++ = ~(*pTemp);
         pTemp++;
       }
       writep = linebuf;
    }

    /* write as ASCII pattern or as binary data */
    if (ascii)
    {
      if (! internal_pbm_w_ascii(wcache, writep, gbm->w))
      {
        if (invert)
        {
          free(linebuf);
        }
        gbm_destroy_wcache(wcache);
        return GBM_ERR_WRITE;
      }
    }
    else
    {
      if (gbm_file_write(fd, writep, line_bytes) != line_bytes)
      {
        if (ascii)
        {
          gbm_destroy_wcache(wcache);
        }
        if (invert)
        {
          free(linebuf);
        }
        return GBM_ERR_WRITE;
      }
    }
    p -= stride;
  }

  if (invert)
  {
    free(linebuf);
  }
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

const char *pbm_err(GBM_ERR rc)
{
  switch ( (int) rc )
  {
    case GBM_ERR_PBM_BAD_M:
      return "bad maximum pixel intensity";
  }
  return NULL;
}

