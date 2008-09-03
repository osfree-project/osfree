/*

gbmscale.c - Scale bitmap to new size

History
-------
(Heiko Nitzsche)

29-Sep-2005: Add support for simple scaling of 32bpp, 48bpp and 64bpp color images

 6-Sep-2007: Add resampling scaler:
             -> Filter functions:
                * nearest neighbor
                * bilinear
                * bell
                * bspline
                * mitchell
                * lanczos
             -> Supported color depths:
                *  8bpp (grayscale only)
                * 24bpp
                * 32bpp (24bpp with alpha channel)
                * 48bpp
                * 64bpp (48bpp with alpha channel)

19-Sep-2007: Fix resampling scaling rounding issues (go back to float)
08-Dec-2007: Use 1 large memory block instead of per line allocation units
             for creating filter contribution list.
21-Jan-2008: Add support for resampled scaling of 1bpp and 4bpp grayscale bitmaps.
             (automatic color depth expansion)
             -> Now supported color depths:
                *  1bpp, 4bpp, 8bpp (grayscale only)
                * 24bpp
                * 32bpp (24bpp with alpha channel)
                * 48bpp
                * 64bpp (48bpp with alpha channel)

08-Feb-2008: Allocate memory from high memory for bitmap data to
             stretch limit for out-of-memory errors
             (requires kernel with high memory support)

08-Apr-2008: Performance fine tuning resampling scaler

15-Aug-2008: Integrate new GBM types
*/

#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gbm.h"
#include "gbmmem.h"
#include "gbmscale.h"

#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))


/* gbm_simple_scale - point sampled */

static void simple_scale_1(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    int sx = 0;
    gbm_u8 bit, value;

    for ( ; dw >= 8; dw -= 8 )
    {
        for ( value = 0, bit = 0x80; bit > 0; bit >>= 1 )
        {
            if ( s[(unsigned)sx>>3]&(0x80U>>((unsigned)sx&7U)) )
                value |= bit;
            sx += *xs++;
        }
        *d++ = value;
    }

    if ( dw > 0 )
    {
        for ( value = 0, bit = 0x80; dw-- > 0; bit >>= 1 )
        {
            if ( s[(unsigned)sx>>3]&(0x80U>>((unsigned)sx&7U)) )
                value |= bit;
            sx += *xs++;
        }
        *d = value;
    }
}

/* ---------- */

static void simple_scale_4(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    int sx = 0;
    for ( ;; )
    {
        if ( dw-- == 0 ) return;
        if ( sx&1 ) *d = (s[(unsigned)sx>>1] << 4 );
        else        *d = (s[(unsigned)sx>>1]&0xf0U);
        sx += *xs++;

        if ( dw-- == 0 ) return;
        if ( sx&1 ) *d++ |= (s[(unsigned)sx>>1]&0x0fU);
        else        *d++ |= (s[(unsigned)sx>>1] >>  4);
        sx += *xs++;
    }
}

/* ---------- */

static void simple_scale_8(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    while ( dw-- > 0 )
    {
        *d++ = *s;
        s += *xs++;
    }
}

/* ---------- */

static void simple_scale_24(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    while ( dw-- > 0 )
    {
        *d++ = s[0];
        *d++ = s[1];
        *d++ = s[2];
        s += ( 3 * *xs++ );
    }
}

/* ---------- */

static void simple_scale_32(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    while ( dw-- > 0 )
    {
        *d++ = s[0];
        *d++ = s[1];
        *d++ = s[2];
        *d++ = s[3];
        s += ( 4 * *xs++ );
    }
}

/* ---------- */

static void simple_scale_48(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    const gbm_u16 * s16 = (const gbm_u16 *) s;
          gbm_u16 * d16 = (gbm_u16 *) d;

    while ( dw-- > 0 )
    {
        *d16++ = s16[0];
        *d16++ = s16[1];
        *d16++ = s16[2];
        s16 += ( 3 * *xs++ );
    }
}

/* ---------- */

static void simple_scale_64(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    const gbm_u16 * s16 = (const gbm_u16 *) s;
          gbm_u16 * d16 = (gbm_u16 *) d;

    while ( dw-- > 0 )
    {
        *d16++ = s16[0];
        *d16++ = s16[1];
        *d16++ = s16[2];
        *d16++ = s16[3];
        s16 += ( 4 * *xs++ );
    }
}

/* ---------- */

static void fast_simple_scale_1(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    xs=xs; /* Suppress warnings */
    memcpy(d, s, (unsigned)(dw+7) >> 3);
}

/* ---------- */

static void fast_simple_scale_4(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    xs=xs; /* Suppress warnings */
    memcpy(d, s, (unsigned) (dw+1)>>1);
}

/* ---------- */

static void fast_simple_scale_8(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    xs=xs; /* Suppress warnings */
    memcpy(d, s, dw);
}

/* ---------- */

static void fast_simple_scale_24(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    xs=xs; /* Suppress warnings */
    memcpy(d, s, dw*3);
}

/* ---------- */

static void fast_simple_scale_32(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    xs=xs; /* Suppress warnings */
    memcpy(d, s, dw*4);
}

/* ---------- */

static void fast_simple_scale_48(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    xs=xs; /* Suppress warnings */
    memcpy(d, s, dw*6);
}

/* ---------- */

static void fast_simple_scale_64(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    xs=xs; /* Suppress warnings */
    memcpy(d, s, dw*8);
}

/* --------------------------------------- */

GBM_ERR gbm_simple_scale(
    const gbm_u8 *s, int sw, int sh,
          gbm_u8 *d, int dw, int dh,
    const int bpp
    )
{
    int sst = ( (sw * bpp + 31) / 32 ) * 4;
    int dst = ( (dw * bpp + 31) / 32 ) * 4;
    int *xs, *ys, i;
    void (*scaler)(const gbm_u8 *s, gbm_u8 *d, int dw, const int xs[]);

    /* Allocate memory for step arrays */

    if ( (xs = gbmmem_malloc((dw+1+dh+1)*sizeof(int))) == NULL )
    {
        return GBM_ERR_MEM;
    }
    ys = xs + (dw+1);

    /* Make mapping to 0..dx from 0..sx (and same for y) */

    for ( i = 0; i <= dw; i++ )
    {
        xs[i] = (i * sw) / dw;
    }

    for ( i = 0; i <= dh; i++ )
    {
        ys[i] = (i * sh) / dh;
    }

    /* Compute step coefficients */

    for ( i = 0; i < dw; i++ )
    {
        xs[i] = xs[i+1] - xs[i];
    }

    for ( i = 0; i < dh; i++ )
    {
        ys[i] = ys[i+1] - ys[i];
    }

    /* Pick a scaling routine. Special optimisation to prevent
       excessive work scaling horizontally if widths are the same.
       Effectively reduces this code to a memcpy. */

    if ( dw == sw )
    {
        switch ( bpp )
        {
            case 1 : scaler = fast_simple_scale_1 ; break;
            case 4 : scaler = fast_simple_scale_4 ; break;
            case 8 : scaler = fast_simple_scale_8 ; break;
            case 24: scaler = fast_simple_scale_24; break;
            case 32: scaler = fast_simple_scale_32; break;
            case 48: scaler = fast_simple_scale_48; break;
            case 64: scaler = fast_simple_scale_64; break;
            default:
                gbmmem_free(xs);
                return GBM_ERR_NOT_SUPP;
        }
    }
    else
    {
        switch ( bpp )
        {
            case 1 : scaler = simple_scale_1 ; break;
            case 4 : scaler = simple_scale_4 ; break;
            case 8 : scaler = simple_scale_8 ; break;
            case 24: scaler = simple_scale_24; break;
            case 32: scaler = simple_scale_32; break;
            case 48: scaler = simple_scale_48; break;
            case 64: scaler = simple_scale_64; break;
            default:
                gbmmem_free(xs);
                return GBM_ERR_NOT_SUPP;
        }
    }

    /* Now do guts of scaling */

    while ( dh-- > 0 )
    {
        (*scaler)(s, d, dw, xs);
        d += dst;
        s += (sst * *ys++);
    }

    gbmmem_free(xs);
    return GBM_ERR_OK;
}

/* --------------------------------------- */
/* --------------------------------------- */

/* These algorithms are based on public domain code.
 * Original code by Dale Schumacher, public domain 1991.
 * See _Graphics Gems III_ "General Filtered Image Rescaling", Dale A. Schumacher
 *
 * Modified for GBM use + optimizations by Heiko Nitzsche, public domain 2007.
 */

/*
 * filter function definitions
 */

/* box filter, W = L = 1.0 */
static float nearestneighbor_filter(float t)
{
    if ((t > -0.5f) && (t <= 0.5f)) return(1.0f);
    return(0.0f);
}

/* ---------- */

/* W = L = 2.0 */
static float linear_filter(float t)
{
    if (t < 0.0f) t = -t;
    if (t < 1.0f) return(((float)1.0f) - t);
    return(0.0f);
}

/* ---------- */

/* W = L = 2.0 */
static float bell_filter(float t) /* box (*) box (*) box */
{
    if(t < 0.0f) t = -t;
    if(t < 0.5f) return((float)0.75f - (t * t));
    if(t < 1.5f) {
        t = (t - 1.5f);
        return((float)0.5f * (t * t));
    }
    return(0.0f);
}

/* ---------- */

/* W = L = 2.0 */
static float bspline_filter(float t) /* box (*) box (*) box (*) box */
{
    float tt;

    if(t < 0.0f) t = -t;
    if(t < 1.0f) {
        tt = t * t;
        return(((float)0.5f * tt * t) - tt + ((float)2.0f / 3.0f));
    } else if(t < 2.0f) {
        t = (float)2.0f - t;
        return(((float)1.0f / 6.0f) * (t * t * t));
    }
    return(0.0f);
}

/* ---------- */

static double sinc(double x)
{
    x *= 3.141592653589793f; /* PI */
    if(x != 0.0f) return(sin(x) / x);
    return(1.0f);
}
/* W = L = 3.0 */
static float lanczos_filter(float t)
{
    if(t < 0.0f) t = -t;
    if(t < 3.0f) return(sinc(t) * sinc(t/3.0f));
    return(0.0f);
}

/* ---------- */

/* W = L = 2.0 */
static float mitchell_filter(float t)
{
    #define B ((float)1.0f / 3.0f)
    #define C ((float)1.0f / 3.0f)

    float tt;

    tt = t * t;
    if(t < 0.0f) t = -t;
    if(t < 1.0f) {
        t = ((((float) 12.0f - (float) 9.0f * B - (float)6.0f * C) * (t * tt))
           + (((float)-18.0f + (float)12.0f * B + (float)6.0f * C) * tt)
           +  ((float)  6.0f - (float) 2.0f * B));
        return(t / 6.0f);
    } else if(t < 2.0f) {
        t = ((((float) -1.0f * B - (float) 6.0f * C) * (t * tt))
           + (((float)  6.0f * B + (float)30.0f * C) * tt)
           + (((float)-12.0f * B - (float)48.0f * C) * t)
           +  ((float)  8.0f * B + (float)24.0f * C));
        return(t / 6.0f);
    }
    return(0.0f);

    #undef B
    #undef C
}

/* ---------- */
/* ---------- */

typedef struct {
    int      xsize;      /* horizontal size of the image in Pixels */
    int      ysize;      /* vertical size of the image in Pixels   */
    int      span;       /* byte offset between two scanlines      */
    int      bpp;        /* bytes per pixel                        */
    gbm_u8 * data;       /* pointer to first scanline of image     */
    gbm_u8 * graylevels; /* pointer to the grayscale array with 1<<bpp entries, bpp>8 = NULL */
} Image;

/* ---------- */

static gbm_u8 clamp2byte(const float l)
{
    const long t = MIN(l, 255);
    return MAX(0, t);
}

static gbm_u16 clamp2word(const float l)
{
    const long t = MIN(l, 65535);
    return MAX(0, t);
}

/* ---------- */

/* create a blank image */
static Image * new_image(int xsize, int ysize, int bpp, gbm_u8 * graylevels)
{
    const unsigned long stride = ((xsize * bpp + 31)/32) * 4;

    Image *image = (Image *)gbmmem_malloc(sizeof(Image));
    if (image == NULL)
    {
        return NULL;
    }

    image->data = (gbm_u8 *)gbmmem_malloc(stride * ysize);
    if (image->data == NULL)
    {
        gbmmem_free(image);
        return NULL;
    }
    memset(image->data, 0, stride * ysize);

    image->xsize      = xsize;
    image->ysize      = ysize;
    image->span       = stride;
    image->bpp        = bpp;
    image->graylevels = graylevels;

    return(image);
}

static void free_image(Image *image)
{
    gbmmem_free(image->data); image->data = NULL;
    gbmmem_free(image);
}

/* ---------- */
/* ---------- */

/*
 * Image rescaling routine
 */

typedef struct
{
    int   pixel;
    float weight;
} CONTRIB;

typedef struct
{
    int      n; /* number of contributors */
    CONTRIB *p; /* pointer to list of contributions */
} CLIST;

/* ---------- */

/* pre-calculate filter contributions */
static CLIST * create_filter_contributions(const int   size_src,
                                           const int   size_dst,
                                                 float scale,
                                                 float (*filterf)(float),
                                           const float fwidth)
{
    int   i, j, k;
    int   n;
    float fscale, width, weight;
    float center, left, right;
    CLIST * contrib = NULL;
    gbm_u8  * contrib_data = NULL;
    unsigned long contrib_data_row = 0;

    if (size_dst < 1)
    {
        return NULL;
    }
    contrib = (CLIST *)gbmmem_malloc((unsigned long)size_dst * sizeof(CLIST));
    if (contrib == NULL)
    {
        return NULL;
    }
    memset(contrib, 0, (unsigned long)size_dst * sizeof(CLIST));

    /* protect against division by 0 */
    if (scale == 0.0f)
    {
        scale = 1.0f;
    }

    if (scale < 1.0f)
    {
        width  = fwidth / scale;
        fscale = ((float)1.0f) / scale;

        contrib_data_row = ((unsigned long)(width * 2 + 1)) * sizeof(CONTRIB);
        contrib_data = (gbm_u8 *)gbmmem_malloc(contrib_data_row * size_dst);
        memset(contrib_data, 0, contrib_data_row * size_dst);

        for (i = 0; i < size_dst; ++i)
        {
            contrib[i].n  = 0;
            contrib[i].p  = (CONTRIB *) contrib_data;
            contrib_data += contrib_data_row;

            center = ((float) i) / scale;
            left   =  ceil(center - width);
            right  = floor(center + width);

            for (j = left; j <= right; ++j)
            {
                weight = center - (float)j;
                weight = (*filterf)(weight / fscale) / fscale;

                if (j < 0)
                {
                    n = -j;
                }
                else if (j >= size_src)
                {
                    n = (size_src - j) + size_src - 1;
                }
                else
                {
                    n = j;
                }
                k = contrib[i].n++;
                contrib[i].p[k].pixel  = n;
                contrib[i].p[k].weight = weight;
            }
        }
    }
    else
    {
        contrib_data_row = ((unsigned long)(fwidth * 2 + 1)) * sizeof(CONTRIB);
        contrib_data = (gbm_u8 *)gbmmem_malloc(contrib_data_row * size_dst);
        memset(contrib_data, 0, contrib_data_row * size_dst);

        for (i = 0; i < size_dst; ++i)
        {
            contrib[i].n  = 0;
            contrib[i].p  = (CONTRIB *) contrib_data;
            contrib_data += contrib_data_row;

            center = ((float) i) / scale;
            left   =  ceil(center - fwidth);
            right  = floor(center + fwidth);

            for (j = left; j <= right; ++j)
            {
                weight = center - (float)j;
                weight = (*filterf)(weight);
                if (j < 0)
                {
                    n = -j;
                }
                else if (j >= size_src)
                {
                    n = (size_src - j) + size_src - 1;
                }
                else
                {
                    n = j;
                }
                k = contrib[i].n++;
                contrib[i].p[k].pixel  = n;
                contrib[i].p[k].weight = weight;
            }
        }
    }
    return contrib;
}

/* ---------- */

static void free_contributions(CLIST *contrib)
{
    /* free the memory allocated for horizontal filter weights */
    if (contrib != NULL)
    {
      gbmmem_free(contrib[0].p); /* just one pointered data block */
      gbmmem_free(contrib);
    }
}

/* ---------- */

/* apply filter to zoom horizontally from src to tmp */

static GBM_ERR apply_horizontal_contributions(
  const Image  * src,
  const CLIST  * contrib,
        Image  * tmp)
{
    int    x, j;
    float  weightf, weightf_b, weightf_g, weightf_r, weightf_a;
    gbm_u8   graylevel;

    const CLIST   * contrib_x   = NULL;
    const CONTRIB * contrib_x_j = NULL;
    const gbm_u8  * pBGR8       = NULL;
    const gbm_u16 * pBGR16      = NULL;

    const gbm_u8  *raster_src   = src->data;
          gbm_u8  *raster_tmp   = tmp->data;
          gbm_u8  *raster_tmp8  = NULL;
          gbm_u16 *raster_tmp16 = NULL;
    const int   rowspan_src  = src->span;
    const int   rowspan_tmp  = tmp->span;
    const int   xsize_tmp    = tmp->xsize;
          int   ysize_tmp    = tmp->ysize;
          int   xoffset_tmp  = 0;

    const gbm_u8 *graylevels_src = src->graylevels;
          gbm_u8 *graylevels_tmp = tmp->graylevels;

    switch(src->bpp)
    {
        case 8: /* 8bpp grayscale */
          if ((graylevels_src == NULL) || (graylevels_tmp == NULL))
          {
              return GBM_ERR_BAD_ARG;
          }
          while (ysize_tmp-- > 0)
          {
              for (x = 0; x < xsize_tmp; ++x)
              {
                  contrib_x = &(contrib[x]);
                  weightf   = 0.0f;

                  for (j = 0; j < contrib_x->n; ++j)
                  {
                      contrib_x_j = &(contrib_x->p[j]);
                      pBGR8       = raster_src + contrib_x_j->pixel;
                      weightf    += contrib_x_j->weight * graylevels_src[*pBGR8];
                  }
                  /* store the pixel and update the palette */
                  graylevel                 = clamp2byte(weightf);
                  raster_tmp[x]             = graylevel;
                  graylevels_tmp[graylevel] = graylevel;
              }
              /* address next row */
              raster_src += rowspan_src;
              raster_tmp += rowspan_tmp;
          }
          break;

        /* ---------- */

        case 24: /* 24bpp */
          while (ysize_tmp-- > 0)
          {
              xoffset_tmp = 0;
              for (x = 0; x < xsize_tmp; ++x)
              {
                  contrib_x = &(contrib[x]);
                  weightf_b = 0.0f;
                  weightf_g = 0.0f;
                  weightf_r = 0.0f;

                  for (j = 0; j < contrib_x->n; ++j)
                  {
                      contrib_x_j = &(contrib_x->p[j]);
                      pBGR8       = raster_src + (contrib_x_j->pixel * 3);
                      weightf     = contrib_x_j->weight;
                      weightf_b  += weightf * *pBGR8++;
                      weightf_g  += weightf * *pBGR8++;
                      weightf_r  += weightf * *pBGR8;
                  }
                  /* store the pixel */
                   raster_tmp8   = raster_tmp + xoffset_tmp;
                  *raster_tmp8++ = clamp2byte(weightf_b);
                  *raster_tmp8++ = clamp2byte(weightf_g);
                  *raster_tmp8   = clamp2byte(weightf_r);
                  xoffset_tmp   += 3;
              }
              /* address next row */
              raster_src += rowspan_src;
              raster_tmp += rowspan_tmp;
          }
          break;

        /* ---------- */

        case 32: /* 32bpp */
          while (ysize_tmp-- > 0)
          {
              xoffset_tmp = 0;
              for (x = 0; x < xsize_tmp; ++x)
              {
                  contrib_x = &(contrib[x]);
                  weightf_b = 0.0f;
                  weightf_g = 0.0f;
                  weightf_r = 0.0f;
                  weightf_a = 0.0f;

                  for (j = 0; j < contrib_x->n; ++j)
                  {
                      contrib_x_j = &(contrib_x->p[j]);
                      pBGR8       = raster_src + (contrib_x_j->pixel * 4);
                      weightf     = contrib_x_j->weight;
                      weightf_b  += weightf * *pBGR8++;
                      weightf_g  += weightf * *pBGR8++;
                      weightf_r  += weightf * *pBGR8++;
                      weightf_a  += weightf * *pBGR8;
                  }
                  /* store the pixel */
                   raster_tmp8   = raster_tmp + xoffset_tmp;
                  *raster_tmp8++ = clamp2byte(weightf_b);
                  *raster_tmp8++ = clamp2byte(weightf_g);
                  *raster_tmp8++ = clamp2byte(weightf_r);
                  *raster_tmp8   = clamp2byte(weightf_a);
                  xoffset_tmp   += 4;
              }
              /* address next row */
              raster_src += rowspan_src;
              raster_tmp += rowspan_tmp;
          }
          break;

        /* ---------- */

        case 48: /* 48bpp */
          while (ysize_tmp-- > 0)
          {
              xoffset_tmp = 0;
              for (x = 0; x < xsize_tmp; ++x)
              {
                  contrib_x = &(contrib[x]);
                  weightf_b = 0.0f;
                  weightf_g = 0.0f;
                  weightf_r = 0.0f;

                  for (j = 0; j < contrib_x->n; ++j)
                  {
                      contrib_x_j = &(contrib_x->p[j]);
                      pBGR16      = (const gbm_u16 *)(raster_src + (contrib_x_j->pixel * 6));
                      weightf     = contrib_x_j->weight;
                      weightf_b  += weightf * *pBGR16++;
                      weightf_g  += weightf * *pBGR16++;
                      weightf_r  += weightf * *pBGR16;
                  }
                  /* store the pixel */
                   raster_tmp16   = (gbm_u16 *)(raster_tmp + xoffset_tmp);
                  *raster_tmp16++ = clamp2word(weightf_b);
                  *raster_tmp16++ = clamp2word(weightf_g);
                  *raster_tmp16   = clamp2word(weightf_r);
                  xoffset_tmp    += 6;
              }
              /* address next row */
              raster_src += rowspan_src;
              raster_tmp += rowspan_tmp;
          }
          break;

        /* ---------- */

        case 64: /* 64bpp */
          while (ysize_tmp-- > 0)
          {
              xoffset_tmp = 0;
              for (x = 0; x < xsize_tmp; ++x)
              {
                  contrib_x = &(contrib[x]);
                  weightf_b = 0.0f;
                  weightf_g = 0.0f;
                  weightf_r = 0.0f;
                  weightf_a = 0.0f;

                  for (j = 0; j < contrib_x->n; ++j)
                  {
                      contrib_x_j = &(contrib_x->p[j]);
                      pBGR16      = (const gbm_u16 *)(raster_src + (contrib_x_j->pixel * 8));
                      weightf     = contrib_x_j->weight;
                      weightf_b  += weightf * *pBGR16++;
                      weightf_g  += weightf * *pBGR16++;
                      weightf_r  += weightf * *pBGR16++;
                      weightf_a  += weightf * *pBGR16;
                  }
                  /* store the pixel */
                   raster_tmp16   = (gbm_u16 *)(raster_tmp + xoffset_tmp);
                  *raster_tmp16++ = clamp2word(weightf_b);
                  *raster_tmp16++ = clamp2word(weightf_g);
                  *raster_tmp16++ = clamp2word(weightf_r);
                  *raster_tmp16   = clamp2word(weightf_a);
                  xoffset_tmp    += 8;
              }
              /* address next row */
              raster_src += rowspan_src;
              raster_tmp += rowspan_tmp;
          }
          break;

        /* ---------- */

        default:
          return GBM_ERR_NOT_SUPP;
    }
    return GBM_ERR_OK;
}

/* ---------- */

/* apply filter to zoom vertically from tmp to dst */

static GBM_ERR apply_vertical_contributions(
  const Image * tmp,
  const CLIST * contrib,
        Image * dst)
{
    int    y, j;
    float  weightf, weightf_b, weightf_g, weightf_r, weightf_a;
    gbm_u8 graylevel;

    const CLIST   * contrib_y   = NULL;
    const CONTRIB * contrib_y_j = NULL;
    const gbm_u8  * pBGR8       = NULL;
    const gbm_u16 * pBGR16      = NULL;

          gbm_u8  *raster_dst   = dst->data;
          gbm_u8  *raster_dst8  = NULL;
          gbm_u16 *raster_dst16 = NULL;
          int   xsize_dst    = dst->xsize;
    const int   ysize_dst    = dst->ysize;
    const int   rowspan_dst  = dst->span;
          int   xoffset_dst  = 0;

    const gbm_u8 *raster_tmp   = tmp->data;
    const int     rowspan_tmp  = tmp->span;

    const gbm_u8 *graylevels_tmp = tmp->graylevels;
          gbm_u8 *graylevels_dst = dst->graylevels;

    switch(dst->bpp)
    {
        case 8: /* 8bpp grayscale */
          if (graylevels_dst == NULL)
          {
              return GBM_ERR_BAD_ARG;
          }
          while (xsize_dst-- > 0)
          {
              /* set to first row */
              raster_dst = dst->data + xoffset_dst;

              for (y = 0; y < ysize_dst; ++y)
              {
                  contrib_y = &(contrib[y]);
                  weightf = 0.0f;

                  for (j = 0; j < contrib_y->n; ++j)
                  {
                      contrib_y_j = &(contrib_y->p[j]);
                      pBGR8       = raster_tmp + (contrib_y_j->pixel * rowspan_tmp);
                      weightf    += contrib_y_j->weight * graylevels_tmp[*pBGR8];
                  }
                  /* store the pixel and update the palette */
                  graylevel                 = clamp2byte(weightf);
                  *raster_dst               = graylevel;
                  graylevels_dst[graylevel] = graylevel;

                  /* address next row */
                  raster_dst += rowspan_dst;
              }
              xoffset_dst++;
              raster_tmp++;
          }
          break;

        /* ---------- */

        case 24: /* 24bpp */
          while (xsize_dst-- > 0)
          {
              /* set to first row */
              raster_dst = dst->data + xoffset_dst;

              for (y = 0; y < ysize_dst; ++y)
              {
                  contrib_y = &(contrib[y]);
                  weightf_b = 0.0f;
                  weightf_g = 0.0f;
                  weightf_r = 0.0f;

                  for (j = 0; j < contrib_y->n; ++j)
                  {
                      contrib_y_j = &(contrib_y->p[j]);
                      pBGR8       = raster_tmp + (contrib_y_j->pixel * rowspan_tmp);
                      weightf     = contrib_y_j->weight;
                      weightf_b  += weightf * *pBGR8++;
                      weightf_g  += weightf * *pBGR8++;
                      weightf_r  += weightf * *pBGR8;
                  }
                  /* store the pixel */
                   raster_dst8   = raster_dst;
                  *raster_dst8++ = clamp2byte(weightf_b);
                  *raster_dst8++ = clamp2byte(weightf_g);
                  *raster_dst8   = clamp2byte(weightf_r);

                  /* address next row */
                  raster_dst += rowspan_dst;
              }
              xoffset_dst += 3;
              raster_tmp  += 3;
          }
          break;

        /* ---------- */

        case 32: /* 32bpp */
          while (xsize_dst-- > 0)
          {
              /* set to first row */
              raster_dst = dst->data + xoffset_dst;

              for (y = 0; y < ysize_dst; ++y)
              {
                  contrib_y = &(contrib[y]);
                  weightf_b = 0.0f;
                  weightf_g = 0.0f;
                  weightf_r = 0.0f;
                  weightf_a = 0.0f;

                  for (j = 0; j < contrib_y->n; ++j)
                  {
                      contrib_y_j = &(contrib_y->p[j]);
                      pBGR8       = raster_tmp + (contrib_y_j->pixel * rowspan_tmp);
                      weightf     = contrib_y_j->weight;
                      weightf_b  += weightf * *pBGR8++;
                      weightf_g  += weightf * *pBGR8++;
                      weightf_r  += weightf * *pBGR8++;
                      weightf_a  += weightf * *pBGR8;
                  }
                  /* store the pixel */
                   raster_dst8   = raster_dst;
                  *raster_dst8++ = clamp2byte(weightf_b);
                  *raster_dst8++ = clamp2byte(weightf_g);
                  *raster_dst8++ = clamp2byte(weightf_r);
                  *raster_dst8   = clamp2byte(weightf_a);

                  /* address next row */
                  raster_dst += rowspan_dst;
              }
              xoffset_dst += 4;
              raster_tmp  += 4;
          }
          break;

        /* ---------- */

        case 48: /* 48bpp */
          while (xsize_dst-- > 0)
          {
              /* set to first row */
              raster_dst = dst->data + xoffset_dst;

              for (y = 0; y < ysize_dst; ++y)
              {
                  contrib_y = &(contrib[y]);
                  weightf_b = 0.0f;
                  weightf_g = 0.0f;
                  weightf_r = 0.0f;

                  for (j = 0; j < contrib_y->n; ++j)
                  {
                      contrib_y_j = &(contrib_y->p[j]);
                      pBGR16      = (const gbm_u16 *)(raster_tmp + (contrib_y_j->pixel * rowspan_tmp));
                      weightf     = contrib_y_j->weight;
                      weightf_b  += weightf * *pBGR16++;
                      weightf_g  += weightf * *pBGR16++;
                      weightf_r  += weightf * *pBGR16;
                  }
                  /* store the pixel */
                   raster_dst16   = (gbm_u16 *) raster_dst;
                  *raster_dst16++ = clamp2word(weightf_b);
                  *raster_dst16++ = clamp2word(weightf_g);
                  *raster_dst16   = clamp2word(weightf_r);

                  /* address next row */
                  raster_dst += rowspan_dst;
              }
              xoffset_dst += 6;
              raster_tmp  += 6;
          }
          break;

        /* ---------- */

        case 64: /* 64bpp */
          while (xsize_dst-- > 0)
          {
              /* set to first row */
              raster_dst = dst->data + xoffset_dst;

              for (y = 0; y < ysize_dst; ++y)
              {
                  contrib_y = &(contrib[y]);
                  weightf_b = 0.0f;
                  weightf_g = 0.0f;
                  weightf_r = 0.0f;
                  weightf_a = 0.0f;

                  for (j = 0; j < contrib_y->n; ++j)
                  {
                      contrib_y_j = &(contrib_y->p[j]);
                      pBGR16      = (const gbm_u16 *)(raster_tmp + (contrib_y_j->pixel * rowspan_tmp));
                      weightf     = contrib_y_j->weight;
                      weightf_b  += weightf * *pBGR16++;
                      weightf_g  += weightf * *pBGR16++;
                      weightf_r  += weightf * *pBGR16++;
                      weightf_a  += weightf * *pBGR16;
                  }
                  /* store the pixel */
                   raster_dst16   = (gbm_u16 *) raster_dst;
                  *raster_dst16++ = clamp2word(weightf_b);
                  *raster_dst16++ = clamp2word(weightf_g);
                  *raster_dst16++ = clamp2word(weightf_r);
                  *raster_dst16   = clamp2word(weightf_a);

                  /* address next row */
                  raster_dst += rowspan_dst;
              }
              xoffset_dst += 8;
              raster_tmp  += 8;
          }
          break;

        /* ---------- */

        default:
          return GBM_ERR_NOT_SUPP;
    }
    return GBM_ERR_OK;
}

/* ---------- */
/* ---------- */

/* resampling scaler */

static GBM_ERR resample_scale(
  const Image  *src,               /* source image structure */
        Image  *dst,               /* destination image structure */
        float  (*filterf)(float),  /* filter function */
  const float  fwidth              /* filter width (support) */)
{
    GBM_ERR rc;
    CLIST  *contrib = NULL;        /* filter contributions */
    Image  *tmp     = NULL;        /* intermediate image */
    float  xscale  = 1.0;          /* scale factor x */
    float  yscale  = 1.0;          /* scale factor y */

    if ((src      == NULL) ||
        (dst      == NULL) ||
        (filterf  == NULL) ||
        (src->bpp != dst->bpp))
    {
        return GBM_ERR_BAD_ARG;
    }

    xscale = (float) dst->xsize / (float) src->xsize;
    yscale = (float) dst->ysize / (float) src->ysize;

    /* pre-calculate filter contributions for a row */
    contrib = create_filter_contributions(src->xsize,
                                          dst->xsize,
                                          xscale,
                                          filterf,
                                          fwidth);
    if (contrib == NULL)
    {
        return GBM_ERR_MEM;
    }

    /* create intermediate image to hold horizontal zoom */
    tmp = new_image(dst->xsize, src->ysize, dst->bpp, dst->graylevels);
    if (tmp == NULL)
    {
        free_contributions(contrib);
        return GBM_ERR_MEM;
    }

    /* apply filter to zoom horizontally from src to tmp */
    rc = apply_horizontal_contributions(src, contrib, tmp);

    /* free the memory allocated for horizontal filter weights */
    free_contributions(contrib);

    if (rc != GBM_ERR_OK)
    {
        return rc;
    }

    /* pre-calculate filter contributions for a column */
    contrib = create_filter_contributions(src->ysize,
                                          dst->ysize,
                                          yscale,
                                          filterf,
                                          fwidth);
    if (contrib == NULL)
    {
        free_image(tmp);
        return GBM_ERR_MEM;
    }

    /* apply filter to zoom vertically from tmp to dst */
    rc = apply_vertical_contributions(tmp, contrib, dst);

    /* free the memory allocated for vertical filter weights */
    free_contributions(contrib);

    free_image(tmp);
    return rc;
}

/* --------------------------------------- */
/* --------------------------------------- */

static gbm_boolean isGrayscalePalette(const GBMRGB *gbmrgb, const int entries)
{
    if ((entries > 0) && (entries <= 0x100))
    {
        int i;
        for (i = 0; i < entries; i++)
        {
            if ((gbmrgb[i].r != gbmrgb[i].g) ||
                (gbmrgb[i].r != gbmrgb[i].b) ||
                (gbmrgb[i].g != gbmrgb[i].b))
            {
                return GBM_FALSE;
            }
        }
        return GBM_TRUE;
    }
    return GBM_FALSE;
}


/* Convert bitmap data with greyscale palette of 1 or 4bpp to 8bpp.
 * The destination buffer must be different to the source buffer.
 *
 * Supported are: 1 bpp -> 8 bpp
 *                4 bpp -> 8 bpp
 */
static gbm_boolean expandLowGrayTo8bpp(const gbm_u8 * data_src, const int w, const int h,
                                       const int sbpp, const GBMRGB * gbmrgb_src,
                                            gbm_u8 * data_dst, GBMRGB * gbmrgb_dst)
{
    if ((sbpp != 1) && (sbpp != 4))
    {
        return GBM_FALSE;
    }
    if (! isGrayscalePalette(gbmrgb_src, 1 << sbpp))
    {
        return GBM_FALSE;
    }

    /* convert to 8bpp gray */
    {
        const int stride_src = ((w * sbpp + 31)/32) * 4;
        const int stride_dst = ((w * 8 + 31)/32) * 4;
        int   x, y, i;
        gbm_u8  c;

        for (y = 0; y < h; y++)
        {
          const gbm_u8 *src  = data_src;
                gbm_u8 *dest = data_dst;

          switch (sbpp)
          {
            case 1:
              for (x = 0; x < w; x++)
              {
                if ((x & 7) == 0)
                {
                  c = *src++;
                }
                else
                {
                  c <<= 1;
                }

                /* As the source palette is gray, there is no need to convert it. Just use one component. */
                *dest++ = gbmrgb_src[c >> 7].r;
              }
              break;

            case 4:
              for (x = 0; x + 1 < w; x += 2)
              {
                c = *src++;

                /* As the source palette is gray, there is no need to convert it. Just use one component. */
                *dest++ = gbmrgb_src[c >> 4].r;

                /* As the source palette is gray, there is no need to convert it. Just use one component. */
                *dest++ = gbmrgb_src[c & 15].r;
              }
              if (x < w)
              {
                c = *src;
                /* As the source palette is gray, there is no need to convert it. Just use one component. */
                *dest++ = gbmrgb_src[c >> 4].r;
              }
              break;

            default:
              return GBM_FALSE;
          }
          data_src += stride_src;
          data_dst += stride_dst;
        }

        /* create full grayscale palette */
        for (i = 0; i < 0x100; i++)
        {
            gbmrgb_dst[i].r =
            gbmrgb_dst[i].g =
            gbmrgb_dst[i].b = (gbm_u8) i;
        }
    }
    return GBM_TRUE;
}

/* --------------------------------------- */

/* gbm_quality_scale_gray - resample grayscale bitmaps using different filter types
 *
 * Supported grayscale depths: 1, 4 and 8bpp (up to 256 gray levels)
 *
 * Note: Resampling will increase the number of gray levels if the source bitmap
 *       has less grayscale levels than 256. Thus the new gray levels will be
 *       stored in the provided dgbmrgb palette array which thus must always have
 *       a length of 256 entries. dgbmrgb can be the same as sgbmrgb.
 *       Also the data target buffer must be able to hold 8bpp data.
 */
GBM_ERR gbm_quality_scale_gray(
    const gbm_u8 *s , int sw, int sh, int sbpp, const GBMRGB * sgbmrgb,
          gbm_u8 *d8, int dw, int dh, GBMRGB * dgbmrgb,
    const GBM_SCALE_FILTER filter)
{
          GBM_ERR  rc          = GBM_ERR_NOT_SUPP;
          gbm_u8   * data8_src   = NULL;
    const GBMRGB * palette_src = NULL;

    /* the filter function that will be used */
    float (*filterFunc)(float) = NULL;
    float filterWidth = 0.0f;

    if ((sgbmrgb == NULL) || (dgbmrgb == NULL) ||
        (s       == NULL) || (d8      == NULL) ||
        (sw < 0) || (sh < 0) || (dw < 0) || (dh < 0))
    {
        return GBM_ERR_BAD_ARG;
    }

    switch(sbpp)
    {
        case 1:
        case 4:
        {
          const unsigned long stride8_src = ((sw * 8 + 31)/32) * 4;
          data8_src = (gbm_u8 *) gbmmem_malloc(stride8_src * sh);
          if (data8_src == NULL)
          {
            return GBM_ERR_MEM;
          }
          if (! expandLowGrayTo8bpp(s, sw, sh, sbpp, sgbmrgb, data8_src, dgbmrgb))
          {
            gbmmem_free(data8_src);
            return GBM_ERR_NOT_SUPP;
          }
          palette_src = dgbmrgb;
        }
        break;

        case 8:
          data8_src   = (gbm_u8 *) s;
          palette_src = sgbmrgb;
          break;

        default:
          return GBM_ERR_NOT_SUPP;
    }

    switch(filter)
    {
        case GBM_SCALE_FILTER_NEARESTNEIGHBOR:
            filterFunc  = nearestneighbor_filter;
            filterWidth = 1.0f;
            break;

        case GBM_SCALE_FILTER_BILINEAR:
            filterFunc  = linear_filter;
            filterWidth = 1.0f;
            break;

        case GBM_SCALE_FILTER_BELL:
            filterFunc  = bell_filter;
            filterWidth = 2.0f;
            break;

        case GBM_SCALE_FILTER_BSPLINE:
            filterFunc  = bspline_filter;
            filterWidth = 2.0f;
            break;

        case GBM_SCALE_FILTER_MITCHELL:
            filterFunc  = mitchell_filter;
            filterWidth = 2.0f;
            break;

        case GBM_SCALE_FILTER_LANCZOS:
            filterFunc  = lanczos_filter;
            filterWidth = 3.0f;
            break;

        default:
            if (data8_src != s)
            {
              gbmmem_free(data8_src);
            }
            return GBM_ERR_NOT_SUPP;
    }

    /* call the scaler function */
    {
        int i;

        gbm_u8 graylevels_src[0x100] = { 0 };
        gbm_u8 graylevels_dst[0x100] = { 0 };

        Image img_src, img_dst;

        img_src.xsize      = sw;
        img_src.ysize      = sh;
        img_src.data       = data8_src;
        img_src.span       = ((sw * 8 + 31) / 32) * 4;
        img_src.bpp        = 8;
        img_src.graylevels = graylevels_src;

        if (! isGrayscalePalette(palette_src, 0x100))
        {
            if (data8_src != s)
            {
              gbmmem_free(data8_src);
            }
            return GBM_ERR_NOT_SUPP;
        }
        for (i = 0; i < 0x100; i++)
        {
            graylevels_src[i] = palette_src[i].r;
        }

        img_dst.xsize      = dw;
        img_dst.ysize      = dh;
        img_dst.data       = d8;
        img_dst.span       = ((dw * 8 + 31) / 32) * 4;
        img_dst.bpp        = 8;
        img_dst.graylevels = graylevels_dst;

        rc = resample_scale(&img_src, &img_dst, filterFunc, filterWidth);
        if (rc != GBM_ERR_OK)
        {
            if (data8_src != s)
            {
              gbmmem_free(data8_src);
            }
            return rc;
        }

        /* update the dst palette */
        for (i = 0; i < 0x100; i++)
        {
            dgbmrgb[i].r =
            dgbmrgb[i].g =
            dgbmrgb[i].b = graylevels_dst[i];
        }

        if (data8_src != s)
        {
          gbmmem_free(data8_src);
        }

        return rc;
    }
}

/* --------------------------------------- */
/* --------------------------------------- */

/* gbm_quality_scale_bgra - resample BGR[A] bitmaps using different filter types
 *
 * Supported color depths: 24bpp
 *                         32bpp (24bpp with alpha channel)
 *                         48bpp
 *                         64bpp (48bpp with alpha channel)
 */
GBM_ERR gbm_quality_scale_bgra(
    const gbm_u8 *s, int sw, int sh,
          gbm_u8 *d, int dw, int dh,
          int  bpp,
    const GBM_SCALE_FILTER filter)
{
    /* the filter function that will be used */
    float (*filterFunc)(float) = NULL;
    float filterWidth = 0.0f;

    if ((s  == NULL) || (d  == NULL) ||
        (sw <  0)    || (sh <  0)    || (dw < 0) || (dh < 0))
    {
        return GBM_ERR_BAD_ARG;
    }

    switch(bpp)
    {
        case 24:
        case 32:
        case 48:
        case 64:
            break;

        default:
            return GBM_ERR_NOT_SUPP;
    }

    switch(filter)
    {
        case GBM_SCALE_FILTER_NEARESTNEIGHBOR:
            filterFunc  = nearestneighbor_filter;
            filterWidth = 1.0f;
            break;

        case GBM_SCALE_FILTER_BILINEAR:
            filterFunc  = linear_filter;
            filterWidth = 1.0f;
            break;

        case GBM_SCALE_FILTER_BELL:
            filterFunc  = bell_filter;
            filterWidth = 2.0f;
            break;

        case GBM_SCALE_FILTER_BSPLINE:
            filterFunc  = bspline_filter;
            filterWidth = 2.0f;
            break;

        case GBM_SCALE_FILTER_MITCHELL:
            filterFunc  = mitchell_filter;
            filterWidth = 2.0f;
            break;

        case GBM_SCALE_FILTER_LANCZOS:
            filterFunc  = lanczos_filter;
            filterWidth = 3.0f;
            break;

        default:
            return GBM_ERR_NOT_SUPP;
    }

    /* call the scaler function */
    {
        Image img_src, img_dst;

        img_src.xsize      = sw;
        img_src.ysize      = sh;
        img_src.data       = (gbm_u8 *) s;
        img_src.span       = ((sw * bpp + 31) / 32) * 4;
        img_src.bpp        = bpp;
        img_src.graylevels = NULL;

        img_dst.xsize      = dw;
        img_dst.ysize      = dh;
        img_dst.data       = d;
        img_dst.span       = ((dw * bpp + 31) / 32) * 4;
        img_dst.bpp        = bpp;
        img_dst.graylevels = NULL;

        return resample_scale(&img_src, &img_dst, filterFunc, filterWidth);
    }
}



