#ifdef ENABLE_JBIG

/*

gbmjbg.c - JBIG format (JBG)


Supported formats and options:
------------------------------
JBIG : JBIG format : .JBG .JBIG

Reads  1,8 bpp grey images.
Writes 1,8 bpp grey equivalent of passed in 1,8 or 24bpp colour data (no palette written)

Output:
-------
Can specify the colour channel the output grey values are based on
  Output option: r,g,b,k (default: k, combine color channels and write grey equivalent)


Note: For licensing please see notes part of the JBIG Kit library.
      For non-GPL compliant applications it is required to get a separate license,
      even in the case GBM.DLL is only linked dynamically to the application!


History:
--------
(Heiko Nitzsche)

12-Jun-2009: First version using JBIG-Kit (Version 2008-08-30)
             Add 1bpp read functionality.
04-Oct-2009: Read source file in chunks during decoding rather than as a whole.
             Add 1,8,24bpp write functionality.
14-Nov-2009: Add 8bpp read functionality.

TODO: 4bpp read/write deferred due to difficult decoding/encoding
      which requires an 4bpp -> 8bpp conversion during encoding
      and an 8bpp -> 4bpp stripping during decoding.
*/

#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"
#include "gbmmem.h"
#include "jbig.h"

/* ---------------------------------------- */

#define CVT(x) (((x) * 255) / ((1L << 16) - 1))

/* ---------------------------------------- */

static GBMFT jbg_gbmft =
{
   GBM_FMT_DESC_SHORT_JBG,
   GBM_FMT_DESC_LONG_JBG,
   GBM_FMT_DESC_EXT_JBG,
   GBM_FT_R1 | GBM_FT_R8 |
   GBM_FT_W1 | GBM_FT_W8 | GBM_FT_W24
};

typedef struct
{
   /* This entry will store the options provided during first header read.
    * It will keep the options for the case the header has to be reread.
    */
   char read_options[PRIV_SIZE - sizeof(int)
                               - 8 /* space for structure element padding */ ];

} JBG_PRIV_READ;

/* ---------------------------------------- */

#define  SW4(a,b,c,d)   ((a)*8+(b)*4+(c)*2+(d))

/* ---------------------------------------- */
    
static GBM_ERR bgr2grey(
  const GBM     *gbm,
  const gbm_u8  *src, const size_t stride_src,
        gbm_u8 **dst, const size_t stride_dst,
  const char    *opt,
  const gbm_boolean mirror)
{
  int i;

  const gbm_boolean kb = ( gbm_find_word(opt, "k") != NULL );
  const gbm_boolean rb = ( gbm_find_word(opt, "r") != NULL );
  const gbm_boolean gb = ( gbm_find_word(opt, "g") != NULL );
  const gbm_boolean bb = ( gbm_find_word(opt, "b") != NULL );

  if (gbm->bpp != 24)
  {
    return GBM_ERR_NOT_SUPP;
  }

  *dst = gbmmem_malloc(stride_dst * gbm->h);
  if (*dst == NULL)
  {
      return GBM_ERR_MEM;
  }
  for (i = 0; i < gbm->h; ++i)
  {
      const gbm_u8 * pSrc =  src + (stride_src * i);
            gbm_u8 * pDst = *dst + (stride_dst * (mirror ? (gbm->h - i - 1) : i));
      size_t x; 
      
      for (x = 0; x < stride_dst; ++x)
      {
        const gbm_u8 b = *pSrc++;
              gbm_u8 g = *pSrc++;
        const gbm_u8 r = *pSrc++;

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

        *pDst++ = g;
      }
    }
    return GBM_ERR_OK;
}

/* ---------------------------------------- */

static GBM_ERR palette2grey(
  const GBM    * gbm, const GBMRGB * gbmrgb, const gbm_u8 * src, const size_t stride_src,
        gbm_u8 **dst, const size_t stride_dst,
  const char   * opt,
  const gbm_boolean mirror,
  const gbm_boolean invert1bpp)
{
  const gbm_boolean  k = ( gbm_find_word(opt, "k") != NULL );
  const gbm_boolean  r = ( gbm_find_word(opt, "r") != NULL );
  const gbm_boolean  g = ( gbm_find_word(opt, "g") != NULL );
  const gbm_boolean  b = ( gbm_find_word(opt, "b") != NULL );
  gbm_u8 grey[0x100] = { 0 };
  int i;

#if 0  
  if ((gbm->bpp != 1) && (gbm->bpp != 4) && (gbm->bpp != 8))
#else  
  if ((gbm->bpp != 1) && (gbm->bpp != 8))
#endif  
  {
    return GBM_ERR_NOT_SUPP;
  }
  if (invert1bpp && (gbm->bpp != 1))
  {
    return GBM_ERR_NOT_SUPP;
  }
  if (gbm->bpp != 1)
  {
      switch ( SW4(k,r,g,b) )
      {
        case SW4(0,1,0,0):
          for ( i = 0; i < 0x100; i++ )
          {
            grey[i] = gbmrgb[i].r;
          }
          break;

        case SW4(0,0,1,0):
          for ( i = 0; i < 0x100; i++ )
          {
            grey[i] = gbmrgb[i].g;
          }
          break;

        case SW4(0,0,0,1):
          for ( i = 0; i < 0x100; i++ )
          {
            grey[i] = gbmrgb[i].b;
          }
          break;
          
          /* Default is the same as "k" */
        case SW4(1,0,0,0):
        default: /* SW4(0,0,0,0) */
          for ( i = 0; i < 0x100; i++ )
          {
            grey[i] = (gbm_u8) ( ((gbm_u16) gbmrgb[i].r *  77U +
                                  (gbm_u16) gbmrgb[i].g * 150U +
                                  (gbm_u16) gbmrgb[i].b *  29U) >> 8 );
          }
          break;
      }
  }  
  *dst = gbmmem_malloc(stride_dst * gbm->h);
  if (*dst == NULL)
  {
      return GBM_ERR_MEM;
  }
  for (i = 0; i < gbm->h; ++i)
  {
      const gbm_u8 * pSrc =  src + (stride_src * i);
            gbm_u8 * pDst = *dst + (stride_dst * (mirror ? (gbm->h - i - 1) : i));
      size_t x; 
      
      switch(gbm->bpp)
      {
          case 1:
            if (invert1bpp)
            {
              for (x = 0; x < stride_dst; ++x)
              {
                *pDst++ = ~(*pSrc);
                ++pSrc;
              }
            }
            else
            {
              memcpy(pDst, pSrc, stride_dst);
            }
            break;

#if 0            
          case 4:
            for (x = 0; x < stride_dst; ++x)
            {
              const gbm_u8 v = *pSrc++;
              const gbm_u8 inx1 = grey[(v >> 4) & 0xf];
              const gbm_u8 inx2 = grey[v & 0xf];
              *pDst++ = ((inx1 << 4) & 0xf0) | inx2;
            }
            break;
#endif
            
          case 8:
            for (x = 0; x < stride_dst; ++x)
            {
              *pDst++ = grey[*pSrc++];
            }
            break;
          
          default:  
            return GBM_ERR_NOT_SUPP;
      }
  }
  return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR jbg_qft(GBMFT *gbmft)
{
   *gbmft = jbg_gbmft;
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

typedef struct
{
   gbm_u8     *data;
   size_t      strideSrc;
   size_t      strideDst;
   size_t      fillUpLen;
   gbm_boolean fillUp;
 /*  gbm_boolean get4of8; */
} READ_DESCRIPTOR;

static void merge_callback(unsigned char *start, size_t len, void *readDescriptor)
{
  size_t i;
  READ_DESCRIPTOR * fDesc = (READ_DESCRIPTOR *)readDescriptor;
  assert(fDesc != NULL);
  
  if (fDesc->fillUp)
  {
#if 0  
    if (fDesc->get4of8)
    {
      for (i = 0; (i < len) && (fDesc->fillUpLen > 0); i += 2)
      {
        *(fDesc->data)  = ((*start) << 4) & 0xf0;
        ++start;
        --(fDesc->fillUpLen);
        --len;
        
        if ((fDesc->fillUpLen > 0) && (len > 0))
        {
          *(fDesc->data) |= (*start) & 0xf;
          ++start;
          --(fDesc->fillUpLen);
          --len;
        }
        ++(fDesc->data);
      }
    }
    else
#endif    
    {
      for (i = 0; (i < len) && (fDesc->fillUpLen > 0); ++i)
      {
        *(fDesc->data)++ = *start++;
        --(fDesc->fillUpLen);
        --len;
      }
    }
    fDesc->fillUp = GBM_FALSE;
    fDesc->data += fDesc->strideDst - fDesc->strideSrc;
  }  
   
  /* copy remaining rows */
  if (len > 0)
  {   
    const size_t rows = len / fDesc->strideSrc;
    
#if 0    
    if (fDesc->get4of8)
    {
      size_t x;
      for (i = 0; i < rows; ++i)
      {
        for (x = 0; x < fDesc->strideSrc; x += 2)
        {
          *(fDesc->data)  = ((*start) << 4) & 0xf0;
          ++start;
          *(fDesc->data) |= (*start) & 0xf;
          ++start;
          ++(fDesc->data);
        }
        len -= fDesc->strideSrc;
      }
    }
    else
#endif    
    {
      for (i = 0; i < rows; ++i)
      {
        memcpy(fDesc->data, start, fDesc->strideSrc);
        (fDesc->data) += fDesc->strideDst;
        start         += fDesc->strideSrc;
        len           -= fDesc->strideSrc;
      }
    }
    
    if (len > 0)
    {
#if 0    
      if (fDesc->get4of8)
      {
        for (i = 0; i < len; i += 2)
        {
          *(fDesc->data)  = ((*start) << 4) & 0xf0;
          ++start;
          *(fDesc->data) |= (*start) & 0xf;
          ++start;
          ++(fDesc->data);
        }
      }
      else
#endif      
      {
        memcpy(fDesc->data, start, len);
        fDesc->data += len;
      }
      fDesc->fillUp    = GBM_TRUE;
      fDesc->fillUpLen = fDesc->strideSrc - len;
    }
    else
    {
      fDesc->fillUp    = GBM_FALSE;
      fDesc->fillUpLen = 0;
    }
  }  
}

/* ---------------------------------------- */

/* Reads and decodes the JBIG file and returns its size and bpp.
 * If the passed data pointer is not NULL, also the data are
 * returned.
 */
static GBM_ERR read_jbg_header(int fd, int *w, int *h, int *bpp, gbm_u8 ** data_ext)
{
   int jbg_rc;
   struct jbg_dec_state jbg_dec_state;
   unsigned long jbg_w, jbg_h;

   if (data_ext != NULL)
   {
      *data_ext = NULL;
   }

   *w   = 0;
   *h   = 0;
   *bpp = 0;

   /* Now decode while reading the file chunkwise */
   jbg_dec_init(&jbg_dec_state);
   jbg_dec_maxsize(&jbg_dec_state, INT_MAX, INT_MAX);
   {
      const int READ_BUFSIZE = 4096;

      unsigned char * file_data = gbmmem_malloc(READ_BUFSIZE);
      if (file_data == NULL)
      {
         return GBM_ERR_MEM;
      }

      /* start at the beginning of the file */
      gbm_file_lseek(fd, 0, GBM_SEEK_SET);

      jbg_rc = JBG_EAGAIN;
      do
      {
         unsigned char *p = NULL;
         size_t declen = 0;
         int readlen = gbm_file_read(fd, file_data, READ_BUFSIZE);
         if (readlen < 0)
         {
            gbmmem_free(file_data);
            jbg_dec_free(&jbg_dec_state);
            return GBM_ERR_READ;
         }
         else if (readlen == 0)
         {
            break;
         }

         declen = 0;
         p = file_data;

         while ((readlen > 0) && ((jbg_rc == JBG_EAGAIN) || (jbg_rc == JBG_EOK)))
         {
            jbg_rc   = jbg_dec_in(&jbg_dec_state, p, readlen, &declen);
            p       += declen;
            readlen -= (int)declen;
         }
      }
      while ((jbg_rc == JBG_EAGAIN) || (jbg_rc == JBG_EOK));

      gbmmem_free(file_data);
   }

   if (jbg_rc != JBG_EOK)
   {
      jbg_dec_free(&jbg_dec_state);
      return GBM_ERR_BAD_MAGIC;
   }

   jbg_w = jbg_dec_getwidth(&jbg_dec_state);
   jbg_h = jbg_dec_getheight(&jbg_dec_state);
   if ((jbg_w > INT_MAX) || (jbg_h  > INT_MAX))
   {
      jbg_dec_free(&jbg_dec_state);
      return GBM_ERR_BAD_SIZE;
   }
   *w = (int) jbg_w;
   *h = (int) jbg_h;

   *bpp = jbg_dec_getplanes(&jbg_dec_state);
   switch(*bpp)
   {
      case 1:
     /* case 4: */
      case 8:
         break;

      default:
         jbg_dec_free(&jbg_dec_state);
         return GBM_ERR_NOT_SUPP;
   }

   /* extract the data if requested */
   if (data_ext != NULL)
   {
      const size_t stride = ((*w * *bpp + 31)/32) * 4;
      const size_t bytes  = stride * *h;
      size_t jbg_bytes = 0;

      *data_ext = gbmmem_malloc(bytes);
      if (*data_ext == NULL)
      {
         jbg_dec_free(&jbg_dec_state);
         return GBM_ERR_MEM;
      }

      switch(*bpp)
      {
         case 1:
             jbg_bytes = jbg_dec_getsize(&jbg_dec_state);
             if (jbg_bytes > bytes)
             {
                gbmmem_free(*data_ext);
                *data_ext = NULL;
                jbg_dec_free(&jbg_dec_state);
                return GBM_ERR_NOT_SUPP;
             }
             memcpy(*data_ext,
                    jbg_dec_getimage(&jbg_dec_state, 0),
                    jbg_bytes);
             break;

        /* case 4: */
         case 8:
            /* read the data and merge the planes into one plane */
            jbg_bytes = jbg_dec_getsize_merged(&jbg_dec_state);
#if 0            
            if (((*bpp == 4) && (jbg_bytes > bytes * 2)) ||
                ((*bpp == 8) && (jbg_bytes > bytes)))
#else
            if (jbg_bytes > bytes)
#endif            
            {
               gbmmem_free(*data_ext);
               *data_ext = NULL;
               jbg_dec_free(&jbg_dec_state);
               return GBM_ERR_NOT_SUPP;
            }
            else
            {
               READ_DESCRIPTOR rd;
               rd.data         = *data_ext;
               rd.strideSrc    = *w;
               rd.strideDst    = stride;
               rd.fillUpLen    = 0;
               rd.fillUp       = GBM_FALSE;
              /* rd.get4of8      = (*bpp == 4) ? GBM_TRUE : GBM_FALSE; */
               jbg_dec_merge_planes(&jbg_dec_state, 1, merge_callback, &rd);
            }
            break;

         default:
            gbmmem_free(*data_ext);
            *data_ext = NULL;
            jbg_dec_free(&jbg_dec_state);
            return GBM_ERR_NOT_SUPP;
      }
   }

   jbg_dec_free(&jbg_dec_state);

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

static GBM_ERR internal_jbg_rhdr(int fd, GBM * gbm, gbm_u8 ** data_src)
{
   /* read header info of first bitmap */
   GBM_ERR rc = read_jbg_header(fd, &gbm->w, &gbm->h, &gbm->bpp, data_src);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR jbg_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
   JBG_PRIV_READ *jbg_priv = (JBG_PRIV_READ *) gbm->priv;

   fn=fn; /* Suppress 'unref arg' compiler warnings */

   /* copy possible options */
   if (strlen(opt) >= sizeof(jbg_priv->read_options))
   {
      return GBM_ERR_BAD_OPTION;
   }
   strcpy(jbg_priv->read_options, opt);

   /* read bitmap info */
   return internal_jbg_rhdr(fd, gbm, NULL);
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR jbg_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
  fd = fd; /* suppress compiler warning */

  /* for perormance reasons we rely on a correctly passed gbm struct
   * and do not decode the image data again.
   */
  if (gbm->bpp <= 8)
  {
     /* get the palette information */
     int palette_entries = 1 << gbm->bpp;
     int increment;
     int x = 0;

     /* init palette */
     memset(gbmrgb, 0, (sizeof(GBMRGB) * palette_entries));

     switch(gbm->bpp)
     {
        case 1:
           x = 0;
           increment = 255;
           break;

#if 0           
        case 4:
           x = 255;
           increment = -17;
           break;
#endif

        case 8:
           x = 255;
           increment = -1;
           break;

        default:
           return GBM_ERR_READ;
     }

     while (palette_entries > 0)
     {
        --palette_entries;
        gbmrgb[palette_entries].r =
        gbmrgb[palette_entries].g =
        gbmrgb[palette_entries].b = x;
        x += increment;
     }
  }

  return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR jbg_rdata(int fd, GBM *gbm, gbm_u8 *data)
{
  gbm_u8 * data_src = NULL;
  size_t stride = 0;
  int y, y1;

  const GBM_ERR rc = internal_jbg_rhdr(fd, gbm, &data_src);
  if (rc != GBM_ERR_OK)
  {
     return rc;
  }

  switch(gbm->bpp)
  {
     case 1:
    /* case 4: */
     case 8:
        /* mirror the image top->down */
        stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
        y1 = gbm->h - 1;
        for (y = 0; y < gbm->h; ++y, --y1)
        {
           memcpy(data + (stride * y), data_src + (stride * y1), stride);
        }
        break;

     default:
        gbmmem_free(data_src);
        return GBM_ERR_NOT_SUPP;
  }

  gbmmem_free(data_src);
  return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

typedef struct
{
   int         fd;
   gbm_boolean hasError;
} WRITE_DESCRIPTOR;

static void write_bie_callback(unsigned char *start, size_t len, void *writeDescriptor)
{
  WRITE_DESCRIPTOR * fDesc = (WRITE_DESCRIPTOR *)writeDescriptor;
  if (! fDesc->hasError)
  {
      if (gbm_file_write(fDesc->fd, start, (int)len) != len)
      {
          fDesc->hasError = GBM_TRUE;
      }
  }
}

/* ---------------------------------------- */

GBM_ERR jbg_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
{
   GBM_ERR rc = GBM_ERR_OK;
   fn=fn;   /* Suppress 'unref arg' compiler warnings */
   opt=opt; /* Suppress 'unref arg' compiler warnings */

   switch(gbm->bpp)
   {
     case 1:
    /* case 4: */
     case 8:
     case 24:
       break;

     default:
       return GBM_ERR_NOT_SUPP;
   }

   /* Encode and write the bitmap */
   {
      struct jbg_enc_state se;
      gbm_u8 * data_dst = NULL;
      size_t stride_src, stride_dst = 0;
      int bpp_dst = 0;
      gbm_boolean invert_data = GBM_FALSE;

      WRITE_DESCRIPTOR wDescriptor;
      wDescriptor.fd       = fd;
      wDescriptor.hasError = GBM_FALSE;

      /* mirror the image top->down */
      stride_src = ((gbm->w * gbm->bpp + 31)/32) * 4;

      /* check if we need to invert the bitmap data */
      if (gbm->bpp == 1)
      {
          const gbm_u8 g0 = (gbm_u8) ( ((gbm_u16) gbmrgb[0].r *  77U +
                                        (gbm_u16) gbmrgb[0].g * 150U +
                                        (gbm_u16) gbmrgb[0].b *  29U) >> 8 );
          const gbm_u8 g1 = (gbm_u8) ( ((gbm_u16) gbmrgb[1].r *  77U +
                                        (gbm_u16) gbmrgb[1].g * 150U +
                                        (gbm_u16) gbmrgb[1].b *  29U) >> 8 );
          if (g0 < g1)
          {
             invert_data = GBM_TRUE;
          }
      }

      /* convert to greyscale data */
      if (gbm->bpp == 24)
      {
          bpp_dst = 8;
          stride_dst = (gbm->w * 8 + 7)/8;
          rc = bgr2grey(gbm, data, stride_src,
                        &data_dst, stride_dst,
                        opt, GBM_TRUE);
      }
      else
      {
          bpp_dst = gbm->bpp;
          stride_dst = (gbm->w * gbm->bpp + 7)/8;
          rc = palette2grey(gbm, gbmrgb, data, stride_src,
                            &data_dst, stride_dst, opt,
                            GBM_TRUE, invert_data);
      }
      if (rc != GBM_ERR_OK)
      {
          return rc;
      }

      switch(bpp_dst)
      {
         case 1:
           jbg_enc_init(&se, gbm->w, gbm->h, 1, &data_dst, write_bie_callback, &wDescriptor);
           jbg_enc_out(&se);
           jbg_enc_free(&se);
           gbmmem_free(data_dst);
           break;           
           
        /* case 4: */
         case 8:
           {
             const int    planes     = bpp_dst;
             const size_t plane_size = (((size_t)gbm->w + 7) / 8) * gbm->h;
             int plane;
             gbm_u8 *plane_data[8] = { NULL };

             gbm_u8 *data_planes = gbmmem_malloc(plane_size * planes);
             if (data_planes == NULL)
             {
               gbmmem_free(data_dst);
               return GBM_ERR_MEM;
             }
             for (plane = 0; plane < planes; ++plane)
             {
               plane_data[plane] = data_planes + (plane_size * plane);
             }
             jbg_split_planes(gbm->w, gbm->h, bpp_dst, planes, data_dst, &plane_data[0], 1);
             gbmmem_free(data_dst);

             jbg_enc_init(&se, gbm->w, gbm->h, planes, &plane_data[0], write_bie_callback, &wDescriptor);
             jbg_enc_out(&se);
             jbg_enc_free(&se);
             gbmmem_free(data_planes);
           }
           break;
           
         default:
           gbmmem_free(data_dst);
           return GBM_ERR_NOT_SUPP;
              
      }

      /* check for write error */
      if (wDescriptor.hasError)
      {
          return GBM_ERR_WRITE;
      }
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

const char *jbg_err(GBM_ERR rc)
{
  rc=rc; /* Suppress 'unref arg' compiler warnings */

  /* no codec specific error messages yet */
  return NULL;
}


#endif /* ENABLE_JBIG */


