/*

gbmmir.c - Produce Mirror Image of General Bitmap

*/

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "gbm.h"
#include "gbmmir.h"
#include "gbmmem.h"

gbm_boolean gbm_ref_vert(const GBM *gbm, gbm_u8 *data)
{
    const size_t stride = ( ((gbm->w * gbm->bpp + 31)/32) * 4 );
    gbm_u8 *p1 = data;
    gbm_u8 *p2 = data + stride * (gbm->h - 1);
    gbm_u8 *p3;

    if ( (p3 = gbmmem_malloc(stride)) == NULL )
    {
        return GBM_FALSE;
    }
    for ( ; p1 < p2; p1 += stride, p2 -= stride )
    {
        memcpy(p3, p1, stride);
        memcpy(p1, p2, stride);
        memcpy(p2, p3, stride);
    }
    gbmmem_free(p3);
    return GBM_TRUE;
}

static void ref_horz_64(gbm_u8 *dst, const gbm_u8 *src, int n)
{
    const gbm_u32 * src32 = (const gbm_u32 *) src;
          gbm_u32 * dst32 = (gbm_u32 *) dst;
    dst32 += n * 2;
    while ( n-- )
    {
        dst32   -= 2;
        dst32[0] = *src32++;
        dst32[1] = *src32++;
    }
}

static void ref_horz_48(gbm_u8 *dst, const gbm_u8 *src, int n)
{
    const gbm_u16 * src16 = (const gbm_u16 *) src;
          gbm_u16 * dst16 = (gbm_u16 *) dst;
    dst16 += n * 3;
    while ( n-- )
    {
        dst16 -= 3;
        *((gbm_u32 *)dst16) = *((const gbm_u32 *)src16);
        src16 += 2;
        dst16[2] = *src16++;
    }
}

static void ref_horz_32(gbm_u8 *dst, const gbm_u8 *src, int n)
{
    const gbm_u32 * src32 = (const gbm_u32 *) src;
          gbm_u32 * dst32 = (gbm_u32 *) dst;
    dst32 += n;
    while ( n-- )
    {
        *(--dst32) = *src32++;
    }
}

static void ref_horz_24(gbm_u8 *dst, const gbm_u8 *src, int n)
{
    dst += n * 3;
    while ( n-- )
    {
        dst -= 3;
        *((gbm_u16 *)dst) = *((const gbm_u16 *)src);
        src += 2;
        dst[2] = *src++;
    }
}

static void ref_horz_8(gbm_u8 *dst, const gbm_u8 *src, int n)
{
    dst += n;
    while ( n-- )
    {
        *(--dst) = *src++;
    }
}

static void ref_horz_4(gbm_u8 *dst, const gbm_u8 *src, int n)
{
    /* build lookup table */
    gbm_u8 rev4[0x100];
    unsigned int i;
    for ( i = 0; i < 0x100; i++ )
    {
        rev4[i] = (gbm_u8) ( ((i & 0x0fU) << 4) | ((i & 0xf0U) >> 4) );
    }

    /* reflect */
    if ( (n & 1) == 0 )
    {
        n /= 2;
        dst += n;
        while ( n-- )
        {
            *(--dst) = rev4[*src++];
        }
    }
    else
    {
        n /= 2;
        src += n;
        while ( n-- )
        {
            *dst    = (gbm_u8) (*(src--) & 0xf0);
            *dst++ |= (gbm_u8) (* src    & 0x0f);
        }
        *dst = (gbm_u8) (*src & 0xf0);
    }
}

static void ref_horz_1(gbm_u8 *dst, const gbm_u8 *src, int n)
{
    const int last = ( n & 7 );

    gbm_u8 rev[0x100];        /* Reverses all bits in a gbm_u8 */
    gbm_u8 rev_top[7][0x100]; /* Reverses top N bits of a gbm_u8 */
    gbm_u8 rev_bot[7][0x100]; /* Reverses bottom N bits of a gbm_u8 */

    /* build lookup table */
    unsigned int i;
    for ( i = 0; i < 0x100; i++ )
    {
        unsigned int j, p, q, b = 0;

        for ( p = 0x01U, q = 0x80U; p < 0x100U; p <<= 1, q >>= 1 )
        {
            if ( i & p ) b |= q;
        }
        rev[i] = b;

        for ( j = 1; j < 8U; j++ )
        {
            gbm_u8 l = 0, r = 0;
            gbm_u8 lm = 0x80U, lmr = (gbm_u8) (0x80U >> (j - 1));
            gbm_u8 rm = 0x01U, rmr = (gbm_u8) (0x01U << (j - 1));
            unsigned int k;

            for ( k = 0; k < j; k++ )
            {
                if ( i & (lm >> k) ) l |= (lmr << k);
                if ( i & (rm << k) ) r |= (rmr >> k);
            }
            rev_top[j-1][i] = l;
            rev_bot[j-1][i] = r;
        }
    }
    
    /* reflect */
    n >>= 3;
    if ( last == 0 )
    {
        dst += n;
        while ( n-- )
        {
            *(--dst) = rev[*src++];
        }
    }
    else
    {
        static const gbm_u8 lmask[8] = { 0, 0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe };
        const gbm_u8 *lrev = rev_top[   last -1];
        const gbm_u8 *rrev = rev_bot[(8-last)-1];
        const gbm_u8 lm = lmask[last], rm = 0xff - lm;

        src += n;
        while ( n-- )
        {
            *dst    = lrev[*(src--) & lm];
            *dst++ |= rrev[* src    & rm];
        }
        *dst = lrev[*src & lm];
    }
}

gbm_boolean gbm_ref_horz(const GBM *gbm, gbm_u8 *data)
{
    const int stride = ( ((gbm->w * gbm->bpp + 31)/32) * 4 );
    int y;
    gbm_u8 *p = data;
    gbm_u8 *tmp;

    if ( (tmp = gbmmem_malloc((size_t) stride)) == NULL )
        return GBM_FALSE;

    switch ( gbm->bpp )
    {
        case 64:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_64(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        case 48:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_48(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        case 32:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_32(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        case 24:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_24(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        case 8:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_8(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        case 4:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_4(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        case 1:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_1(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
    }

    gbmmem_free(tmp);
    return GBM_TRUE;
}

void gbm_transpose(const GBM *gbm, const gbm_u8 *data, gbm_u8 *data_t)
{
    const int stride   = ((gbm->w * gbm->bpp + 31) / 32) * 4;
    const int stride_t = ((gbm->h * gbm->bpp + 31) / 32) * 4;

    switch ( gbm->bpp )
    {
        case 64:
        {
            const int data_step = (stride - gbm->w * 8) / 2;
            const int p_step = (stride_t - 6) / 2;
            int x, y;

            const gbm_u16 * data16   = (const gbm_u16 *) data;
                  gbm_u16 * data_t16 = (gbm_u16 *) data_t;
            for ( y = 0; y < gbm->h; y++ )
            {
                gbm_u16 *p = data_t16; data_t16 += 4;
                for ( x = 0; x < gbm->w; x++ )
                {
                    const gbm_u32 *data32 = (gbm_u32 *)data16;
                          gbm_u32 *p32    = (gbm_u32 *)p;
                    *p32++ = *data32++;
                    *p32   = *data32;
                    p      += 3 + p_step;
                    data16 += 4;
                }
                data16 += data_step;
            }
            break;
        }

        case 48:
        {
            const int data_step = (stride - gbm->w * 6) / 2;
            const int p_step = (stride_t - 3) / 2;
            int x, y;

            const gbm_u16 * data16   = (const gbm_u16 *) data;
                  gbm_u16 * data_t16 = (gbm_u16 *) data_t;

            for ( y = 0; y < gbm->h; y++ )
            {
                gbm_u16 *p = data_t16; data_t16 += 3;
                for ( x = 0; x < gbm->w; x++ )
                {
                    *((gbm_u32 *)p) = *((const gbm_u32 *)data16);
                    p      += 2;
                    data16 += 2;
                    
                    *p  = *data16++;
                     p += p_step;
                }
                data16 += data_step;
            }
            break;
        }
            
        case 32:
        {
            const int p_step = stride_t / 4;
            int x, y;
            const gbm_u32 * data32   = (const gbm_u32 *) data;
                  gbm_u32 * data_t32 = (gbm_u32 *) data_t;

            for ( y = 0; y < gbm->h; y++ )
            {
                gbm_u32 *p = data_t32++;
                for ( x = 0; x < gbm->w; x++ )
                {
                    *p = *data32++;
                    p += p_step;
                }
            }
            break;
        }

        case 24:
        {
            const int data_step = stride - gbm->w * 3;
            const int p_step = stride_t - 2;
            int x, y;

            for ( y = 0; y < gbm->h; y++ )
            {
                gbm_u8 *p = data_t; data_t += 3;
                for ( x = 0; x < gbm->w; x++ )
                {
                    *((gbm_u16 *)p) = *((const gbm_u16 *)data);
                    p    += 2;
                    data += 2;
                
                    *p  = *data++;
                     p += p_step;
                }
                data += data_step;
            }
            break;
        }

        case 8:
        {
            const int data_step = stride - gbm->w;
            int x, y;

            for ( y = 0; y < gbm->h; y++ )
            {
                gbm_u8 *p = data_t++;
                for ( x = 0; x < gbm->w; x++ )
                {
                    *p = *data++;
                    p += stride_t;
                }
                data += data_step;
            }
            break;
        }

        case 4:
        {
            int x, y;
            const gbm_u8 * end   = data   + (stride   * gbm->h);
            const gbm_u8 * end_t = data_t + (stride_t * gbm->w);

            for ( y = 0; y < gbm->h; y += 2 )
            {
                for ( x = 0; x < gbm->w; x += 2 )
                /*...s2x2 transpose to 2x2:40:*/
                {
                    const gbm_u8 *src = data + y * stride + ((unsigned)x >> 1);
                    gbm_u8 ab   = src[0];
                    gbm_u8 cd   = (src + stride < end) ? src[stride] : 0;
                    gbm_u8 *dst = data_t + x * stride_t + ((unsigned)y >> 1);
                    dst[0]      = (gbm_u8) ((ab & 0xf0) | (cd >> 4));
                    if (dst + stride_t < end_t)
                    {
                      dst[stride_t] = (gbm_u8) ((ab << 4) | (cd & 0x0f));
                    }
                }
                /*...e*/
                if ( x < gbm->w )
                /*...s1x2 transpose to 2x1:40:*/
                {
                    const gbm_u8 *src = data + y * stride + ((unsigned)x >> 1);
                    gbm_u8 a0   = src[0];
                    gbm_u8 b0   = (src + stride < end) ? src[stride] : 0;
                    gbm_u8 *dst = data_t + x * stride_t + ((unsigned)y >> 1);
                    dst[0]      = (gbm_u8) ((a0 & 0xf0) | (b0 >> 4));
                }
                /*...e*/
            }
            if ( y < gbm->h )
            {
                for ( x = 0; x < gbm->w; x += 2 )
                /*...s2x1 transpose to 1x2:40:*/
                {
                    const gbm_u8 *src = data + y * stride + ((unsigned)x >> 1);
                    gbm_u8 ab   = src[0];
                    gbm_u8 *dst = data_t + x * stride_t + ((unsigned)y >> 1);
                    dst[0]      = (gbm_u8) (ab & 0xf0);
                    if (dst + stride_t < end_t)
                    {
                      dst[stride_t] = (gbm_u8) (ab << 4);
                    }
                }
                /*...e*/
                if ( x < gbm->w )
                /*...s1x1 transpose to 1x1:40:*/
                {
                    const gbm_u8 *src = data + y * stride + ((unsigned)x >> 1);
                    gbm_u8 a0   = src[0];
                    gbm_u8 *dst = data_t + x * stride_t + ((unsigned)y >> 1);
                    dst[0]      = (gbm_u8) (a0 & 0xf0);
                }
                /*...e*/
            }
            break;
        }

        case 1:
        {
            int x, y;
            gbm_u8 xbit, ybit;

            memset(data_t, 0, gbm->w * stride_t);

            ybit = 0x80;
            for ( y = 0; y < gbm->h; y++ )
            {
                xbit = 0x80;
                for ( x = 0; x < gbm->w; x++ )
                {
                    const gbm_u8 *src = data   + y * stride   + ((unsigned)x >> 3);
                          gbm_u8 *dst = data_t + x * stride_t + ((unsigned)y >> 3);

                    if ( *src & xbit )
                        *dst |= ybit;

                    if ( (xbit >>= 1) == 0 )
                        xbit = 0x80;
                }
                if ( (ybit >>= 1) == 0 )
                    ybit = 0x80;
            }
            break;
        }
    }
}

