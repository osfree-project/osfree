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

20-Aug-2010: Quality fine tuning resampling scaler
             (flat colour areas are now preserved to get better quality for
              this type of images)

21-Sep-2010: Performance improvements of resampling scaler
             - Multithreading (all cores are dynamically used)
             - SSE support for Microsoft, GCC and Open Watcom Compilers

02-Nov-2010: Add resampling scaler filters:
             * blackman, catmullrom, quadratic, gaussian, kaiser
             Final fix for generated moire due to rounding issues
             in contribution calculations.
*/

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gbm.h"
#include "gbmmem.h"
#include "gbmscale.h"
#include "gbmmthrd.h"

/* ---------- */

#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))

/* ---------- */
#if defined(SSE_ENABLED)
#undef SSE_ENABLED
#endif

/* Enable SSE support for Microsoft Compiler on 32bit.
 * On 64bit SSE does not improve the performance. The compiler
 * generated code (potentially also using SSE) reaches the same
 * or even better performance. So we don't enable hand-coded SSE
 * codeon 64bit and benefit from a smaller generated binary.
 */
#if defined(_MSC_VER) && (defined(_M_IX86) /* || defined(_M_X64) */)

    #include <windows.h>
    #include <intrin.h>
    #include <xmmintrin.h>
    #define SSE_ENABLED 1

   #ifdef SSE_ENABLED
    static gbm_boolean isSupported_SSE()
    {
       /* check if the CPU has SSE support */
      #if defined(__WIN32__) || defined(WIN32)
       if (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE))
       {
           return GBM_TRUE;
       }
      #endif
       return GBM_FALSE;
    }
   #endif

/* Enable SSE support for GCC Compiler */
#elif defined(__GNUC__) && defined(__SSE__)

   /* Manual SSE optimization seems to result in worse
    * results than automatic SSE version (seen with
    * GCC 3.3.5 on OS/2 and GCC 4.4.2 on Linux).
    * Thus the hand-coded support stays disabled.
    */
   #if 0
    #include <xmmintrin.h>
    #define SSE_ENABLED 1
   #endif

   #ifdef SSE_ENABLED
    static gbm_boolean isSupported_SSE()
    {
       #define cpuid_asm(in, a, b, c, d) __asm__\
         ("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));

       unsigned long unused, edxreg = 0;
       cpuid_asm(1, unused, unused, unused, edxreg);
       if (edxreg & 0x02000000)
       {
           return GBM_TRUE;
       }
       return GBM_FALSE;
    }
   #endif

/* Enable SSE support for OpenWatcom Compiler */
#elif defined(__WATCOMC__) && defined(__386__)

   /* SSE intrinsics support on OpenWatcom does not exist
    * and also assembler coding is suboptimal because pointers
    * are used for value passing and also inter-macro optimization
    * is missing. Thus there is basically no reasonable compiler
    * support for optimized usage of SSE registers.
    *
    * At the moment the standard FPU version is much faster than the
    * SSE using reimplemented compatible SSE intrinsics version.
    * Obviously the FPU version does not reach the performance of the
    * MSC/GCC SSE versions. So for now compatible SSE intrinsics are
    * not used for OpenWatcom. To compensate for missing inter macro
    * optimization hand coded assembler macros are used in the scaler
    * algorithm that directly address XMM registers. It's ugly but
    * the speed gain is about equal to the other compilers then.
    */
   #define SSE_ENABLED 1

   #ifdef SSE_ENABLED
    /* Used SSE intrinsics not available from Compiler.
     * Thus they are manually coded here in assembler in
     * slightly modified manner to compensate for the missing
     * compiler inter macros optimization support.
     */
    #pragma aux __mm_binary0nr =                              modify exact []
    #pragma aux __mm_binary1nr = parm [eax]                   modify exact []
    #pragma aux __mm_binary4nr = parm [eax] [ebx] [ecx] [edx] modify exact []
    /* ---------------------- */
    static void _mm_set_ps_XMM1(int __D, int __C, int __B, int __A);
    #pragma aux (__mm_binary4nr) _mm_set_ps_XMM1 = \
                    ".686"                \
                    "cvtsi2ss xmm7,eax"   \
                    "cvtsi2ss xmm6,ebx"   \
                    "cvtsi2ss xmm5,ecx"   \
                    "cvtsi2ss xmm1,edx"   \
                    "unpcklps xmm1,xmm6"  \
                    "unpcklps xmm5,xmm7"  \
                    "unpcklps xmm1,xmm5"
    /* ---------------------- */
    static void _mm_load1_ps_XMM0(float const *__V);
    #pragma aux (__mm_binary1nr) _mm_load1_ps_XMM0 = \
                    ".686"                \
                    "movss  xmm0,[eax]"   \
                    "shufps xmm0,xmm0,0"
    /* ---------------------- */
    static void _mm_load1_ps_XMM2(float const *__V);
    #pragma aux (__mm_binary1nr) _mm_load1_ps_XMM2 = \
                    ".686"                \
                    "movss  xmm2,[eax]"   \
                    "shufps xmm2,xmm2,0"
    /* ---------------------- */
    static void _mm_store_ps_XMM0(float *__V);
    #pragma aux (__mm_binary1nr) _mm_store_ps_XMM0 = \
                    ".686"               \
                    "movaps [eax],xmm0"
    /* ---------------------- */
    static void _mm_mul_ps_XMM2_XMM1_XMM3(void);
    #pragma aux (__mm_binary0nr) _mm_mul_ps_XMM2_XMM1_XMM3 = \
                    ".686"             \
                    "movaps xmm3,xmm1" \
                    "mulps  xmm3,xmm2"
    /* ---------------------- */
    static void _mm_add_ps_XMM0_XMM3_XMM0(void);
    #pragma aux (__mm_binary0nr) _mm_add_ps_XMM0_XMM3_XMM0 = \
                    ".686"             \
                    "addps xmm0,xmm3"
    /* ---------------------- */

   #if 0
    /* Used SSE intrinsics not available from Compiler.
     * Thus they are manually coded here in assembler.
     * The following set resembles the intrinsics set as
     * available by other compilers. Though it is just
     * kept here for completeness. The modified set above
     * is used for OpenWatcom (see reasons there).
     */
    typedef struct
    {
        float m128_f32[4];
    } __m128;
    /* ---------------------- */
    #pragma aux __mm_binary1   = parm [eax]                   value[esi] modify exact []
    #pragma aux __mm_binary2nr = parm [eax] [edx]                        modify exact []
    #pragma aux __mm_binary2   = parm [eax] [edx]             value[esi] modify exact []
    #pragma aux __mm_binary4   = parm [eax] [ebx] [ecx] [edx] value[esi] modify exact []
    /* ---------------------- */
    static __m128 _mm_set_ps(int __D, int __C, int __B, int __A);
    #pragma aux (__mm_binary4) _mm_set_ps = \
                    ".686"                \
                    "cvtsi2ss xmm3,eax"   \
                    "cvtsi2ss xmm2,ebx"   \
                    "cvtsi2ss xmm1,ecx"   \
                    "cvtsi2ss xmm0,edx"   \
                    "unpcklps xmm0,xmm2"  \
                    "unpcklps xmm1,xmm3"  \
                    "unpcklps xmm0,xmm1"  \
                    "movups   [esi],xmm0"
    /* ---------------------- */
    static __m128 _mm_load1_ps(float const *__V);
    #pragma aux (__mm_binary1) _mm_load1_ps = \
                    ".686"                \
                    "movss  xmm0,[eax]"   \
                    "shufps xmm0,xmm0,0"  \
                    "movups [esi],xmm0"
    /* ---------------------- */
    static void _mm_store_ps(float *__V, __m128 *__m);
    #define _mm_store_ps(__V, __m)  _mm_store_ps((__V), &(__m))
    #pragma aux (__mm_binary2nr) _mm_store_ps = \
                    ".686"               \
                    "movaps xmm0,[edx]"  \
                    "movaps [eax],xmm0"
    /* ---------------------- */
    static __m128 _mm_add_ps(__m128 *__m1, __m128 *__m2);
    #define _mm_add_ps(__m1, __m2)  _mm_add_ps(&(__m1), &(__m2))
    #pragma aux (__mm_binary2) _mm_add_ps = \
                    ".686"               \
                    "movaps xmm1,[eax]"  \
                    "addps  xmm1,[edx]"  \
                    "movups [esi],xmm1"
    /* ---------------------- */
    static __m128 _mm_mul_ps(__m128 *__m1, __m128 *__m2);
    #define _mm_mul_ps(__m1, __m2)  _mm_mul_ps(&(__m1), &(__m2))
    #pragma aux (__mm_binary2) _mm_mul_ps = \
                    ".686"               \
                    "movaps xmm1,[eax]"  \
                    "mulps  xmm1,[edx]"  \
                    "movups [esi],xmm1"
    /* ---------------------- */
   #endif

    static gbm_boolean isSupported_SSE()
    {
        unsigned long _edxreg = 0;
        __asm
        {
           __asm mov eax,1
           __asm cpuid
           __asm mov _edxreg,edx
        }
        if (_edxreg & 0x02000000)
        {
           return GBM_TRUE;
        }
        return GBM_FALSE;
    }
   #endif

#endif

/* ---------- */

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
    memcpy(d, s, (size_t)dw * 3);
}

/* ---------- */

static void fast_simple_scale_32(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    xs=xs; /* Suppress warnings */
    memcpy(d, s, (size_t)dw * 4);
}

/* ---------- */

static void fast_simple_scale_48(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    xs=xs; /* Suppress warnings */
    memcpy(d, s, (size_t)dw * 6);
}

/* ---------- */

static void fast_simple_scale_64(
    const gbm_u8 *s,
    gbm_u8 *d, int dw,
    const int xs[]
    )
{
    xs=xs; /* Suppress warnings */
    memcpy(d, s, (size_t)dw * 8);
}

/* --------------------------------------- */

GBM_ERR gbm_simple_scale(
    const gbm_u8 *s, int sw, int sh,
          gbm_u8 *d, int dw, int dh,
    const int bpp
    )
{
    size_t sst = ( ((size_t)sw * bpp + 31) / 32 ) * 4;
    size_t dst = ( ((size_t)dw * bpp + 31) / 32 ) * 4;
    int *xs, *ys, i;
    void (*scaler)(const gbm_u8 *s, gbm_u8 *d, int dw, const int xs[]);

    if ((s == NULL) || (d == NULL) ||
        (sw < 1) || (sh < 1) || (dw < 1) || (dh < 1))
    {
        return GBM_ERR_BAD_ARG;
    }

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
#define MY_PI (3.14159265358979323846f) /* PI */

static double sinc(double x)
{
    x *= MY_PI;
    if ((x < 0.01f) && (x > -0.01f))
    {
        return (x*x*(-1.0f/6.0f + x*x*1.0f/120.0f) + 1.0f);
    }
    return (sin(x) / x);
}

static double bessel0(double x)
{
    const double EPSILON_RATIO = 1E-16;
    double sum, pow, ds;
    int k;

    const double xh = x * 0.5;
    sum = 1.0;
    pow = 1.0;
    k   = 0;
    ds  = 1.0;
    while (ds > sum * EPSILON_RATIO)
    {
       ++k;
       pow *= xh / k;
       ds   = pow * pow;
       sum += ds;
    }
    return sum;
}

static float clean(double t)
{
   const float EPSILON = .0000125f;
   if (fabs(t) < EPSILON)
   {
       return 0.0f;
   }
   return (float)t;
}

/* ---------- */

/* box filter */
#define NEARESTNEIGHBOR_SUPPORT (1.0f)
static float nearestneighbor_filter(float t)
{
    if ((t > -0.5f) && (t <= 0.5f)) return(1.0f);
    return(0.0f);
}

/* ---------- */

#define BILINEAR_SUPPORT (1.0f)
static float bilinear_filter(float t) /* box (*) box, bilinear/triangle */
{
    if (t < 0.0f) t = -t;
    if (t < 1.0f) return((1.0f) - t);
    return(0.0f);
}

/* ---------- */

#define BELL_SUPPORT (2.0f)
static float bell_filter(float t) /* box (*) box (*) box */
{
    if (t < 0.0f) t = -t;
    if (t < 0.5f) return(0.75f - (t * t));
    if (t < 1.5f)
    {
        t = (t - 1.5f);
        return(0.5f * (t * t));
    }
    return(0.0f);
}

/* ---------- */

#define BSPLINE_SUPPORT (2.0f)
static float bspline_filter(float t) /* box (*) box (*) box (*) box */
{
    float tt;

    if (t < 0.0f) t = -t;
    if (t < 1.0f)
    {
        tt = t * t;
        return((0.5f * tt * t) - tt + (2.0f / 3.0f));
    }
    else if (t < 2.0f) {
        t = 2.0f - t;
        return((1.0f / 6.0f) * (t * t * t));
    }
    return(0.0f);
}

/* ---------- */

/* Windowed sinc -- see "Jimm Blinn's Corner: Dirty Pixels" pg. 26. */
#define LANCZOS3_SUPPORT (3.0f)
static float lanczos3_filter(float t)
{
    if (t < 0.0f) t = -t;
    if (t < LANCZOS3_SUPPORT)
    {
        return clean(sinc(t) * sinc(t/LANCZOS3_SUPPORT));
    }
    return(0.0f);
}

/* ---------- */

/* Mitchell, D. and A. Netravali, "Reconstruction Filters in Computer Graphics."
 * Computer Graphics, Vol. 22, No. 4, pp. 221-228.
 * (B, C)
 * (1/3, 1/3) - Defaults recommended by Mitchell and Netravali
 * (1  , 0)	  - Equivalent to the Cubic B-Spline
 * (0  , 0.5) - Equivalent to the Catmull-Rom Spline
 * (0  , C)   - The family of Cardinal Cubic Splines
 * (B  , 0)   - Duff's tensioned B-Splines.
 */
static float mitchell(float t, const float B, const float C)
{
    const float tt = t * t;
    if (t < 0.0f) t = -t;
    if (t < 1.0f)
    {
        t =  ((( 12.0f -  9.0f * B - 6.0f * C) * (t * tt))
            + ((-18.0f + 12.0f * B + 6.0f * C) * tt)
            +  (  6.0f -  2.0f * B));
        return(t / 6.0f);
    }
    else if(t < 2.0f) {
        t =  ((( -1.0f * B -  6.0f * C) * (t * tt))
            + ((  6.0f * B + 30.0f * C) * tt)
            + ((-12.0f * B - 48.0f * C) * t)
            +  (  8.0f * B + 24.0f * C));
        return(t / 6.0f);
    }
    return(0.0f);
}

#define MITCHELL_SUPPORT (2.0f)
static float mitchell_filter(float t)
{
   const float p = 1.0f / 3.0f;
   return mitchell(t, p, p);
}

#define CATMULLROM_SUPPORT (2.0f)
static float catmullrom_filter(float t)
{
   return mitchell(t, 0.0f, 0.5f);
}

/* ---------- */

static double blackman_exact_window(double x)
{
    return (0.42659071f +
            0.49656062f * cos(MY_PI * x) +
            0.07684867f * cos(2.0f * MY_PI * x));
}

#define BLACKMAN_SUPPORT (3.0f)
static float blackman_filter(float t)
{
    if (t < 0.0f) t = -t;
    if (t < BLACKMAN_SUPPORT)
    {
        return clean(sinc(t) * blackman_exact_window(t / BLACKMAN_SUPPORT));
    }
    return (0.0f);
}

/* ---------- */

/* Dodgson, N., "Quadratic Interpolation for Image Resampling" */
#define QUADRATIC_SUPPORT (1.5f)
static float quadratic(float t, const float R)
{
    if (t < 0.0f) t = -t;	
    if (t < QUADRATIC_SUPPORT)
    {
        const float tt = t * t;
        if (t <= .5f)
        {
            return (-2.0f * R) * tt + .5f * (R + 1.0f);
        }
        else
        {
            return (R * tt) + (-2.0f * R - .5f) * t + (3.0f / 4.0f) * (R + 1.0f);
        }
    }
    return 0.0f;
}

/*
static float quadratic_interp_filter(float t)
{
   return quadratic(t, 1.0f);
}
static float quadratic_approx_filter(float t)
{
   return quadratic(t, .5f);
}
*/
static float quadratic_mix_filter(float t)
{
    return quadratic(t, .8f);
}

/* ---------- */

#define GAUSSIAN_SUPPORT (1.25f)
static float gaussian_filter(float t) /* with blackman window */
{
    if (t < 0) t = -t;
    if (t < GAUSSIAN_SUPPORT)
    {
        return clean(exp(-2.0f * t * t)
                     * sqrt(2.0f / MY_PI)
                     * blackman_exact_window(t / GAUSSIAN_SUPPORT));
    }
    return(0.0f);
}

/* ---------- */

static double kaiser(double alpha, double half_width, double x)
{
   const double ratio = (x / half_width);
   return bessel0(alpha * sqrt(1 - ratio * ratio)) / bessel0(alpha);
}

#define KAISER_SUPPORT (3.0f)
static float kaiser_filter(float t)
{
   if (t < 0.0f) t = -t;
   if (t < KAISER_SUPPORT)
   {
      /* db atten */
      const float att = 40.0f;
      const float alpha = (float)(exp(log((double)0.58417 * (att - 20.96)) * 0.4) + 0.07886 * (att - 20.96));
      return clean(sinc(t) * kaiser(alpha, KAISER_SUPPORT, t));
   }
   return 0.0f;
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
    gbm_s16 t = (gbm_s16) (l + 0.5f);
    t = MIN(t, 255);
    return (gbm_u8) (MAX(0, t));
}

static gbm_u16 clamp2word(const float l)
{
    gbm_s32 t = (gbm_s32) (l + 0.5f);
    t = MIN(t, 65535);
    return (gbm_u16) (MAX(0, t));
}

/* ---------- */

/* create a blank image */
static Image * new_image(int xsize, int ysize, int bpp, gbm_u8 * graylevels)
{
    const int stride = ((xsize * bpp + 31)/32) * 4;

    Image *image = (Image *)gbmmem_malloc(sizeof(Image));
    if (image == NULL)
    {
        return NULL;
    }

    image->data = (gbm_u8 *)gbmmem_malloc((size_t)stride * ysize);
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
    float center;
    int   left;
    int   right;
} CONTRIB_BOUNDS;

/* shrinked contribution to get better cache line efficiency */
#pragma pack(2)
typedef struct
{
    unsigned short pixel;
    float          weight;
} CONTRIB;
#pragma pack()

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
    const float FILTER_SCALE    = 1.0f; /* smaller for more sharpness */
    const float OO_FILTER_SCALE = 1.0f / FILTER_SCALE;
    const float NUDGE           = 0.5f;

    int   i, j, k, left, right, n, total;
    float half_width, weight, center;

    CLIST          * contrib           = NULL;
    CONTRIB_BOUNDS * contrib_bounds    = NULL;
    CONTRIB        * contrib_data      = NULL;
    CONTRIB        * contrib_data_next = NULL;

    if ((size_src < 1) || (size_dst < 1))
    {
        return NULL;
    }
    contrib = (CLIST *)gbmmem_malloc(sizeof(CLIST) * size_dst);
    if (contrib == NULL)
    {
        return NULL;
    }
    memset(contrib, 0, (size_t)size_dst * sizeof(CLIST));

    contrib_bounds = (CONTRIB_BOUNDS *)gbmmem_malloc(sizeof(CONTRIB_BOUNDS) * size_dst);
    if (contrib_bounds == NULL)
    {
        gbmmem_free(contrib);
        return NULL;
    }
    memset(contrib_bounds, 0, sizeof(CONTRIB_BOUNDS) * size_dst);

    /* protect against division by 0 */
    if (scale == 0.0f)
    {
        scale = 1.0f;
    }

    if (scale < 1.0f)
    {
        /* Handle case when there are fewer destination samples than
         * source samples (downsampling/minification).
         */

        /* stretched half width of filter */
        half_width = (fwidth / scale) * FILTER_SCALE;

         /* Find the source sample(s) that contribute to each destination sample. */
        n = 0;
        for (i = 0; i < size_dst; ++i)
        {
            /* Convert from discrete to continuous coordinates, scale,
             * then convert back to discrete.
             */
            center = ((float)i + NUDGE) / scale;
            center -= NUDGE;
            center += FILTER_SCALE;

            left   = (int) floor(center - half_width);
            right  = (int)  ceil(center + half_width);

            contrib_bounds[i].center = center;
            contrib_bounds[i].left   = left;
            contrib_bounds[i].right  = right;

            n += (right - left + 1);
        }
        total = n;
        if (0 == total)
        {
            gbmmem_free(contrib_bounds);
            gbmmem_free(contrib);
            return NULL;
        }

        /* Allocate memory for contributors. */
        contrib_data = (CONTRIB *)gbmmem_calloc(total, sizeof(CONTRIB));
        if (contrib_data == NULL)
        {
            gbmmem_free(contrib_bounds);
            gbmmem_free(contrib);
            return NULL;
        }

        /* Create the list of source samples which
         * contribute to each destination sample.
         */

        contrib_data_next = contrib_data;
        for (i = 0; i < size_dst; ++i)
        {
            int   max_k = -1;
            float max_w = -1e+20f;
            float total_weight = 0.0f;
            float norm = 0.0f;

            center = contrib_bounds[i].center;
            left   = contrib_bounds[i].left;
            right  = contrib_bounds[i].right;

            contrib[i].n = 0;
            contrib[i].p = contrib_data_next;
            contrib_data_next += (right - left + 1);
            assert((contrib_data_next - contrib_data_next) <= total);

            for (j = left; j <= right; ++j)
            {
                total_weight += (*filterf)((center - (float)j) * scale * OO_FILTER_SCALE);
            }
            if (total_weight != 0.0f)
            {
                norm = 1.0f / total_weight;
            }

            total_weight = 0.0f;

            for (j = left; j <= right; j++)
            {
                weight = (*filterf)((center - (float)j) * scale * OO_FILTER_SCALE) * norm;
                if (weight == 0.0f)
                {
                    continue;
                }

                /* Ensure that the contributing source sample is
                 * within bounds. If not clamp.
                 */
                if (j < 0)
                {
                    n = 0;
                }
                else if (j >= size_src)
                {
                    n = size_src - 1;
                }
                else
                {
                    n = j;
                }

                /* Increment the number of source samples which contribute to the
                 * current destination sample.
                 */
                k = contrib[i].n++;
                contrib[i].p[k].pixel  = (unsigned short)n; /* store src sample number */
                contrib[i].p[k].weight = weight;            /* store src sample weight */
                total_weight += weight;                     /* total weight of all contributors */

                if (weight > max_w)
                {
                    max_w = weight;
                    max_k = k;
                }
            }
            if ((max_k == -1) || (contrib[i].n == 0))
            {
                gbmmem_free(contrib_data);
                gbmmem_free(contrib_bounds);
                gbmmem_free(contrib);
                return NULL;
            }
            if (total_weight != 1.0f)
            {
                contrib[i].p[max_k].weight += 1.0f - total_weight;
            }
        }
    }
    else
    {
        /* Handle case when there are more destination samples than source
         * samples (upsampling).
         */

        half_width = fwidth * FILTER_SCALE;

        /* Find the source sample(s) that contribute to each destination sample. */
        n = 0;
        for (i = 0; i < size_dst; ++i)
        {
            /* Convert from discrete to continuous coordinates, scale,
             * then convert back to discrete.
             */
            center = ((float)i + NUDGE) / scale;
            center -= NUDGE;
            center += FILTER_SCALE;

            left   = (int) floor(center - half_width);
            right  = (int)  ceil(center + half_width);

            contrib_bounds[i].center = center;
            contrib_bounds[i].left   = left;
            contrib_bounds[i].right	 = right;

            n += (right - left + 1);
        }
        total = n;
        if (0 == total)
        {
            gbmmem_free(contrib_bounds);
            gbmmem_free(contrib);
            return NULL;
        }

        /* Allocate memory for contributors. */
        contrib_data = (CONTRIB *)gbmmem_calloc(total, sizeof(CONTRIB));
        if (contrib_data == NULL)
        {
            gbmmem_free(contrib_bounds);
            gbmmem_free(contrib);
            return NULL;
        }

        /* Create the list of source samples which
         * contribute to each destination sample.
         */

        contrib_data_next = contrib_data;
        for (i = 0; i < size_dst; ++i)
        {
            int   max_k = -1;
            float max_w = -1e+20f;
            float total_weight = 0.0f;
            float norm = 0.0f;

            center = contrib_bounds[i].center;
            left   = contrib_bounds[i].left;
            right  = contrib_bounds[i].right;

            contrib[i].n = 0;
            contrib[i].p = contrib_data_next;
            contrib_data_next += (right - left + 1);
            assert((contrib_data_next - contrib_data_next) <= total);

            for (j = left; j <= right; ++j)
            {
                total_weight += (*filterf)((center - (float)j) * OO_FILTER_SCALE);
            }
            if (total_weight != 0.0f)
            {
                norm = 1.0f / total_weight;
            }

            total_weight = 0;

            for (j = left; j <= right; j++)
            {
                weight = (*filterf)((center - (float)j) * OO_FILTER_SCALE) * norm;
                if (weight == 0.0f)
                {
                    continue;
                }

                /* Ensure that the contributing source sample is
                 * within bounds. If not clamp.
                 */
                if (j < 0)
                {
                    n = 0;
                }
                else if (j >= size_src)
                {
                    n = size_src - 1;
                }
                else
                {
                    n = j;
                }

                /* Increment the number of source samples which contribute to the
                 * current destination sample.
                 */
                k = contrib[i].n++;
                contrib[i].p[k].pixel  = (unsigned short)n; /* store src sample number */
                contrib[i].p[k].weight = weight;            /* store src sample weight */
                total_weight += weight;                     /* total weight of all contributors */

                if (weight > max_w)
                {
                    max_w = weight;
                    max_k = k;
                }
            }
            if ((max_k == -1) || (contrib[i].n == 0))
            {
                gbmmem_free(contrib_data);
                gbmmem_free(contrib_bounds);
                gbmmem_free(contrib);
                return NULL;
            }
            if (total_weight != 1.0f)
            {
                contrib[i].p[max_k].weight += 1.0f - total_weight;
            }
        }
    }

    gbmmem_free(contrib_bounds);

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
/* ---------- */

typedef GBM_ERR (*FN_APPLYFUNC)(const Image * src, const CLIST * contrib, Image * dst);

typedef struct
{
    const CLIST * contrib;
    const Image * src;
          Image * dst;
    FN_APPLYFUNC  applyFunc;
        GBM_ERR   retval;
} CONTRIB_THREAD_ARG;

/* ---------- */
/* ---------- */

/* apply filter to zoom horizontally from src to tmp */
/* --> SSE optimized version */

#ifdef SSE_ENABLED
static GBM_ERR apply_horizontal_contributions_sse(
  const Image  * src,
  const CLIST  * contrib,
        Image  * tmp)
{
    int   x, j;

    const CLIST   * contrib_x   = NULL;
    const CONTRIB * contrib_x_j = NULL;
    const gbm_u8  * pBGR8       = NULL;
    const gbm_u16 * pBGR16      = NULL;

    const gbm_u8  *raster_src   = src->data;
          gbm_u8  *raster_tmp   = tmp->data;
          gbm_u8  *raster_tmp8  = NULL;
          gbm_u16 *raster_tmp16 = NULL;
    const int      rowspan_src  = src->span;
    const int      rowspan_tmp  = tmp->span;
    const int      xsize_tmp    = tmp->xsize;
          int      ysize_tmp    = tmp->ysize;
          int      xoffset_tmp  = 0;

    gbm_u8  b8 , g8 , r8 , a8;
    gbm_u16 b16, g16, r16, a16;

    static const float FLOAT_ZERO = 0.0f;
   #pragma pack(16)
    typedef struct
    {
       #ifndef __WATCOMC__
        __m128 weightf_bgra;
        __m128 weightf_mul;
        __m128 weightf;
        __m128 data;
       #endif
        float  results[4];
    } SSE_VALUES;
   #pragma pack()

    #define ALIGN 16L
    char ssebuf[sizeof(SSE_VALUES) + ALIGN - 1] = { 0 };
    SSE_VALUES * const ssep=(SSE_VALUES*)((((ptrdiff_t)ssebuf) + ALIGN - 1) &~(ALIGN - 1));

    switch(src->bpp)
    {
        case 24: /* 24bpp */
          while (ysize_tmp-- > 0)
          {
              xoffset_tmp = 0;
              for (x = 0; x < xsize_tmp; ++x)
              {
                  contrib_x = &(contrib[x]);
                 #ifdef __WATCOMC__
                  _mm_load1_ps_XMM0(&FLOAT_ZERO);
                 #else
                  ssep->weightf_bgra = _mm_load1_ps(&FLOAT_ZERO);
                 #endif

                  for (j = 0; j < contrib_x->n; ++j)
                  {
                      contrib_x_j = &(contrib_x->p[j]);
                      pBGR8       = raster_src + (contrib_x_j->pixel * 3);
                      b8          = *pBGR8++;
                      g8          = *pBGR8++;
                      r8          = *pBGR8;
                     #ifdef __WATCOMC__
                      /*
                       * ssep->data         := XMM1
                       * ssep->weightf      := XMM2
                       * ssep->weightf_mul  := XMM3
                       * ssep->weightf_bgra := XMM0
                       */
                      _mm_set_ps_XMM1(0, r8, g8, b8);
                      _mm_load1_ps_XMM2(&(contrib_x_j->weight));
                      _mm_mul_ps_XMM2_XMM1_XMM3();
                      _mm_add_ps_XMM0_XMM3_XMM0();
                     #else
                      ssep->data         = _mm_set_ps(0, r8, g8, b8);
                      ssep->weightf      = _mm_load1_ps(&(contrib_x_j->weight));
                      ssep->weightf_mul  = _mm_mul_ps(ssep->weightf, ssep->data);
                      ssep->weightf_bgra = _mm_add_ps(ssep->weightf_bgra, ssep->weightf_mul);
                     #endif
                  }
                  /* store the pixel */
                  raster_tmp8 = raster_tmp + xoffset_tmp;
                 #ifdef __WATCOMC__
                  _mm_store_ps_XMM0(ssep->results);
                 #else
                  _mm_store_ps(ssep->results, ssep->weightf_bgra);
                 #endif
                  *raster_tmp8++ = clamp2byte(ssep->results[0]);
                  *raster_tmp8++ = clamp2byte(ssep->results[1]);
                  *raster_tmp8   = clamp2byte(ssep->results[2]);

                  xoffset_tmp += 3;
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
                 #ifdef __WATCOMC__
                  _mm_load1_ps_XMM0(&FLOAT_ZERO);
                 #else
                  ssep->weightf_bgra = _mm_load1_ps(&FLOAT_ZERO);
                 #endif

                  for (j = 0; j < contrib_x->n; ++j)
                  {
                      contrib_x_j = &(contrib_x->p[j]);
                      pBGR8       = raster_src + (contrib_x_j->pixel * 4);
                      b8          = *pBGR8++;
                      g8          = *pBGR8++;
                      r8          = *pBGR8++;
                      a8          = *pBGR8;
                     #ifdef __WATCOMC__
                      /*
                       * ssep->data         := XMM1
                       * ssep->weightf      := XMM2
                       * ssep->weightf_mul  := XMM3
                       * ssep->weightf_bgra := XMM0
                       */
                      _mm_set_ps_XMM1(a8, r8, g8, b8);
                      _mm_load1_ps_XMM2(&(contrib_x_j->weight));
                      _mm_mul_ps_XMM2_XMM1_XMM3();
                      _mm_add_ps_XMM0_XMM3_XMM0();
                     #else
                      ssep->data         = _mm_set_ps(a8, r8, g8, b8);
                      ssep->weightf      = _mm_load1_ps(&(contrib_x_j->weight));
                      ssep->weightf_mul  = _mm_mul_ps(ssep->weightf, ssep->data);
                      ssep->weightf_bgra = _mm_add_ps(ssep->weightf_bgra, ssep->weightf_mul);
                     #endif
                  }
                  /* store the pixel */
                  raster_tmp8 = raster_tmp + xoffset_tmp;
                 #ifdef __WATCOMC__
                  _mm_store_ps_XMM0(ssep->results);
                 #else
                  _mm_store_ps(ssep->results, ssep->weightf_bgra);
                 #endif
                  *raster_tmp8++ = clamp2byte(ssep->results[0]);
                  *raster_tmp8++ = clamp2byte(ssep->results[1]);
                  *raster_tmp8++ = clamp2byte(ssep->results[2]);
                  *raster_tmp8   = clamp2byte(ssep->results[3]);

                  xoffset_tmp += 4;
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
                 #ifdef __WATCOMC__
                  _mm_load1_ps_XMM0(&FLOAT_ZERO);
                 #else
                  ssep->weightf_bgra = _mm_load1_ps(&FLOAT_ZERO);
                 #endif

                  for (j = 0; j < contrib_x->n; ++j)
                  {
                      contrib_x_j = &(contrib_x->p[j]);
                      pBGR16      = (const gbm_u16 *)(raster_src + (contrib_x_j->pixel * 6));
                      b16         = *pBGR16++;
                      g16         = *pBGR16++;
                      r16         = *pBGR16;
                     #ifdef __WATCOMC__
                      /*
                       * ssep->data         := XMM1
                       * ssep->weightf      := XMM2
                       * ssep->weightf_mul  := XMM3
                       * ssep->weightf_bgra := XMM0
                       */
                      _mm_set_ps_XMM1(0, r16, g16, b16);
                      _mm_load1_ps_XMM2(&(contrib_x_j->weight));
                      _mm_mul_ps_XMM2_XMM1_XMM3();
                      _mm_add_ps_XMM0_XMM3_XMM0();
                     #else
                      ssep->data         = _mm_set_ps(0, r16, g16, b16);
                      ssep->weightf      = _mm_load1_ps(&(contrib_x_j->weight));
                      ssep->weightf_mul  = _mm_mul_ps(ssep->weightf, ssep->data);
                      ssep->weightf_bgra = _mm_add_ps(ssep->weightf_bgra, ssep->weightf_mul);
                     #endif
                  }
                  /* store the pixel */
                  raster_tmp16 = (gbm_u16 *)(raster_tmp + xoffset_tmp);
                 #ifdef __WATCOMC__
                  _mm_store_ps_XMM0(ssep->results);
                 #else
                  _mm_store_ps(ssep->results, ssep->weightf_bgra);
                 #endif
                  *raster_tmp16++ = clamp2word(ssep->results[0]);
                  *raster_tmp16++ = clamp2word(ssep->results[1]);
                  *raster_tmp16   = clamp2word(ssep->results[2]);

                  xoffset_tmp += 6;
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
                 #ifdef __WATCOMC__
                  _mm_load1_ps_XMM0(&FLOAT_ZERO);
                 #else
                  ssep->weightf_bgra = _mm_load1_ps(&FLOAT_ZERO);
                 #endif

                  for (j = 0; j < contrib_x->n; ++j)
                  {
                      contrib_x_j = &(contrib_x->p[j]);
                      pBGR16      = (const gbm_u16 *)(raster_src + (contrib_x_j->pixel * 8));
                      b16         = *pBGR16++;
                      g16         = *pBGR16++;
                      r16         = *pBGR16++;
                      a16         = *pBGR16;
                     #ifdef __WATCOMC__
                      /*
                       * ssep->data         := XMM1
                       * ssep->weightf      := XMM2
                       * ssep->weightf_mul  := XMM3
                       * ssep->weightf_bgra := XMM0
                       */
                      _mm_set_ps_XMM1(a16, r16, g16, b16);
                      _mm_load1_ps_XMM2(&(contrib_x_j->weight));
                      _mm_mul_ps_XMM2_XMM1_XMM3();
                      _mm_add_ps_XMM0_XMM3_XMM0();
                     #else
                      ssep->data          = _mm_set_ps(a16, r16, g16, b16);
                      ssep->weightf       = _mm_load1_ps(&(contrib_x_j->weight));
                      ssep->weightf_mul   = _mm_mul_ps(ssep->weightf, ssep->data);
                      ssep->weightf_bgra  = _mm_add_ps(ssep->weightf_bgra, ssep->weightf_mul);
                     #endif
                  }
                  /* store the pixel */
                  raster_tmp16 = (gbm_u16 *)(raster_tmp + xoffset_tmp);
                 #ifdef __WATCOMC__
                  _mm_store_ps_XMM0(ssep->results);
                 #else
                  _mm_store_ps(ssep->results, ssep->weightf_bgra);
                 #endif
                  *raster_tmp16++ = clamp2word(ssep->results[0]);
                  *raster_tmp16++ = clamp2word(ssep->results[1]);
                  *raster_tmp16++ = clamp2word(ssep->results[2]);
                  *raster_tmp16   = clamp2word(ssep->results[3]);

                  xoffset_tmp += 8;
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
#endif /* SSE_ENABLED */

/* ---------- */

/* apply filter to zoom horizontally from src to tmp */
/* --> Non-SSE version */

static GBM_ERR apply_horizontal_contributions(
  const Image  * src,
  const CLIST  * contrib,
        Image  * tmp)
{
    int    x, j;
    float  weightf, weightf_b, weightf_g, weightf_r, weightf_a;
    gbm_u8 graylevel;

    const CLIST   * contrib_x   = NULL;
    const CONTRIB * contrib_x_j = NULL;
    const gbm_u8  * pBGR8       = NULL;
    const gbm_u16 * pBGR16      = NULL;

    const gbm_u8  *raster_src   = src->data;
          gbm_u8  *raster_tmp   = tmp->data;
          gbm_u8  *raster_tmp8  = NULL;
          gbm_u16 *raster_tmp16 = NULL;
    const int      rowspan_src  = src->span;
    const int      rowspan_tmp  = tmp->span;
    const int      xsize_tmp    = tmp->xsize;
          int      ysize_tmp    = tmp->ysize;
          int      xoffset_tmp  = 0;

    const gbm_u8 *graylevels_src = src->graylevels;
          gbm_u8 *graylevels_tmp = tmp->graylevels;

    gbm_u8  b8 , g8 , r8 , a8;
    gbm_u16 b16, g16, r16, a16;

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
                      g8          = graylevels_src[*pBGR8];

                      weightf += contrib_x_j->weight * g8;
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
                      b8          = *pBGR8++;
                      g8          = *pBGR8++;
                      r8          = *pBGR8;

                      weightf     = contrib_x_j->weight;
                      weightf_b  += weightf * b8;
                      weightf_g  += weightf * g8;
                      weightf_r  += weightf * r8;
                  }
                  /* store the pixel */
                   raster_tmp8   = raster_tmp + xoffset_tmp;
                  *raster_tmp8++ = clamp2byte(weightf_b);
                  *raster_tmp8++ = clamp2byte(weightf_g);
                  *raster_tmp8   = clamp2byte(weightf_r);

                  xoffset_tmp += 3;
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
                      b8          = *pBGR8++;
                      g8          = *pBGR8++;
                      r8          = *pBGR8++;
                      a8          = *pBGR8;

                      weightf     = contrib_x_j->weight;
                      weightf_b  += weightf * b8;
                      weightf_g  += weightf * g8;
                      weightf_r  += weightf * r8;
                      weightf_a  += weightf * a8;
                  }
                  /* store the pixel */
                   raster_tmp8   = raster_tmp + xoffset_tmp;
                  *raster_tmp8++ = clamp2byte(weightf_b);
                  *raster_tmp8++ = clamp2byte(weightf_g);
                  *raster_tmp8++ = clamp2byte(weightf_r);
                  *raster_tmp8   = clamp2byte(weightf_a);

                  xoffset_tmp += 4;
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
                      b16         = *pBGR16++;
                      g16         = *pBGR16++;
                      r16         = *pBGR16;

                      weightf     = contrib_x_j->weight;
                      weightf_b  += weightf * b16;
                      weightf_g  += weightf * g16;
                      weightf_r  += weightf * r16;
                  }
                  /* store the pixel */
                   raster_tmp16   = (gbm_u16 *)(raster_tmp + xoffset_tmp);
                  *raster_tmp16++ = clamp2word(weightf_b);
                  *raster_tmp16++ = clamp2word(weightf_g);
                  *raster_tmp16   = clamp2word(weightf_r);

                  xoffset_tmp += 6;
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
                      b16         = *pBGR16++;
                      g16         = *pBGR16++;
                      r16         = *pBGR16++;
                      a16         = *pBGR16;

                      weightf     = contrib_x_j->weight;
                      weightf_b  += weightf * b16;
                      weightf_g  += weightf * g16;
                      weightf_r  += weightf * r16;
                      weightf_a  += weightf * a16;
                  }
                  /* store the pixel */
                   raster_tmp16   = (gbm_u16 *)(raster_tmp + xoffset_tmp);
                  *raster_tmp16++ = clamp2word(weightf_b);
                  *raster_tmp16++ = clamp2word(weightf_g);
                  *raster_tmp16++ = clamp2word(weightf_r);
                  *raster_tmp16   = clamp2word(weightf_a);

                  xoffset_tmp += 8;
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
/* ---------- */

/* apply filter to zoom vertically from tmp to dst */
/* --> SSE optimized version */

#ifdef SSE_ENABLED
static GBM_ERR apply_vertical_contributions_sse(
  const Image * tmp,
  const CLIST * contrib,
        Image * dst)
{
    int   y, j;

    const CLIST   * contrib_y   = NULL;
    const CONTRIB * contrib_y_j = NULL;
    const gbm_u8  * pBGR8       = NULL;
    const gbm_u16 * pBGR16      = NULL;

          gbm_u8  *raster_dst   = dst->data;
          gbm_u8  *raster_dst8  = NULL;
          gbm_u16 *raster_dst16 = NULL;
          int      xsize_dst    = dst->xsize;
    const int      ysize_dst    = dst->ysize;
    const int      rowspan_dst  = dst->span;
          int      xoffset_dst  = 0;

    const gbm_u8 *raster_tmp   = tmp->data;
    const int     rowspan_tmp  = tmp->span;

    gbm_u8  b8 , g8 , r8 , a8;
    gbm_u16 b16, g16, r16, a16;

    static const float FLOAT_ZERO = 0.0f;
   #pragma pack(16)
    typedef struct
    {
       #ifndef __WATCOMC__
        __m128 weightf_bgra;
        __m128 weightf_mul;
        __m128 weightf;
        __m128 data;
       #endif
        float  results[4];
    } SSE_VALUES;
   #pragma pack()

    #define ALIGN 16L
    char ssebuf[sizeof(SSE_VALUES) + ALIGN - 1] = { 0 };
    SSE_VALUES * const ssep=(SSE_VALUES*)((((ptrdiff_t)ssebuf) + ALIGN - 1) &~(ALIGN - 1));

    switch(dst->bpp)
    {
        case 24: /* 24bpp */
          while (xsize_dst-- > 0)
          {
              /* set to first row */
              raster_dst = dst->data + xoffset_dst;

              for (y = 0; y < ysize_dst; ++y)
              {
                  contrib_y = &(contrib[y]);
                 #ifdef __WATCOMC__
                  _mm_load1_ps_XMM0(&FLOAT_ZERO);
                 #else
                  ssep->weightf_bgra = _mm_load1_ps(&FLOAT_ZERO);
                 #endif

                  for (j = 0; j < contrib_y->n; ++j)
                  {
                      contrib_y_j = &(contrib_y->p[j]);
                      pBGR8       = raster_tmp + (contrib_y_j->pixel * rowspan_tmp);
                      b8          = *pBGR8++;
                      g8          = *pBGR8++;
                      r8          = *pBGR8;
                     #ifdef __WATCOMC__
                      /*
                       * ssep->data         := XMM1
                       * ssep->weightf      := XMM2
                       * ssep->weightf_mul  := XMM3
                       * ssep->weightf_bgra := XMM0
                       */
                      _mm_set_ps_XMM1(0, r8, g8, b8);
                      _mm_load1_ps_XMM2(&(contrib_y_j->weight));
                      _mm_mul_ps_XMM2_XMM1_XMM3();
                      _mm_add_ps_XMM0_XMM3_XMM0();
                     #else
                      ssep->data         = _mm_set_ps(0, r8, g8, b8);
                      ssep->weightf      = _mm_load1_ps(&(contrib_y_j->weight));
                      ssep->weightf_mul  = _mm_mul_ps(ssep->weightf, ssep->data);
                      ssep->weightf_bgra = _mm_add_ps(ssep->weightf_bgra, ssep->weightf_mul);
                     #endif
                  }
                  /* store the pixel */
                  raster_dst8 = raster_dst;
                 #ifdef __WATCOMC__
                  _mm_store_ps_XMM0(ssep->results);
                 #else
                  _mm_store_ps(ssep->results, ssep->weightf_bgra);
                 #endif
                  *raster_dst8++ = clamp2byte(ssep->results[0]);
                  *raster_dst8++ = clamp2byte(ssep->results[1]);
                  *raster_dst8   = clamp2byte(ssep->results[2]);

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
                 #ifdef __WATCOMC__
                  _mm_load1_ps_XMM0(&FLOAT_ZERO);
                 #else
                  ssep->weightf_bgra = _mm_load1_ps(&FLOAT_ZERO);
                 #endif

                  for (j = 0; j < contrib_y->n; ++j)
                  {
                      contrib_y_j = &(contrib_y->p[j]);
                      pBGR8       = raster_tmp + (contrib_y_j->pixel * rowspan_tmp);
                      b8          = *pBGR8++;
                      g8          = *pBGR8++;
                      r8          = *pBGR8++;
                      a8          = *pBGR8;
                     #ifdef __WATCOMC__
                      /*
                       * ssep->data         := XMM1
                       * ssep->weightf      := XMM2
                       * ssep->weightf_mul  := XMM3
                       * ssep->weightf_bgra := XMM0
                       */
                      _mm_set_ps_XMM1(a8, r8, g8, b8);
                      _mm_load1_ps_XMM2(&(contrib_y_j->weight));
                      _mm_mul_ps_XMM2_XMM1_XMM3();
                      _mm_add_ps_XMM0_XMM3_XMM0();
                     #else
                      ssep->data         = _mm_set_ps(a8, r8, g8, b8);
                      ssep->weightf      = _mm_load1_ps(&(contrib_y_j->weight));
                      ssep->weightf_mul  = _mm_mul_ps(ssep->weightf, ssep->data);
                      ssep->weightf_bgra = _mm_add_ps(ssep->weightf_bgra, ssep->weightf_mul);
                     #endif
                  }
                  /* store the pixel */
                  raster_dst8 = raster_dst;
                 #ifdef __WATCOMC__
                  _mm_store_ps_XMM0(ssep->results);
                 #else
                  _mm_store_ps(ssep->results, ssep->weightf_bgra);
                 #endif
                  *raster_dst8++ = clamp2byte(ssep->results[0]);
                  *raster_dst8++ = clamp2byte(ssep->results[1]);
                  *raster_dst8++ = clamp2byte(ssep->results[2]);
                  *raster_dst8   = clamp2byte(ssep->results[3]);

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
                 #ifdef __WATCOMC__
                  _mm_load1_ps_XMM0(&FLOAT_ZERO);
                 #else
                  ssep->weightf_bgra = _mm_load1_ps(&FLOAT_ZERO);
                 #endif

                  for (j = 0; j < contrib_y->n; ++j)
                  {
                      contrib_y_j = &(contrib_y->p[j]);
                      pBGR16      = (const gbm_u16 *)(raster_tmp + (contrib_y_j->pixel * rowspan_tmp));
                      b16         = *pBGR16++;
                      g16         = *pBGR16++;
                      r16         = *pBGR16;
                     #ifdef __WATCOMC__
                      /*
                       * ssep->data         := XMM1
                       * ssep->weightf      := XMM2
                       * ssep->weightf_mul  := XMM3
                       * ssep->weightf_bgra := XMM0
                       */
                      _mm_set_ps_XMM1(0, r16, g16, b16);
                      _mm_load1_ps_XMM2(&(contrib_y_j->weight));
                      _mm_mul_ps_XMM2_XMM1_XMM3();
                      _mm_add_ps_XMM0_XMM3_XMM0();
                     #else
                      ssep->data         = _mm_set_ps(0, r16, g16, b16);
                      ssep->weightf      = _mm_load1_ps(&(contrib_y_j->weight));
                      ssep->weightf_mul  = _mm_mul_ps(ssep->weightf, ssep->data);
                      ssep->weightf_bgra = _mm_add_ps(ssep->weightf_bgra, ssep->weightf_mul);
                     #endif
                  }
                  /* store the pixel */
                  raster_dst16 = (gbm_u16 *) raster_dst;
                 #ifdef __WATCOMC__
                  _mm_store_ps_XMM0(ssep->results);
                 #else
                  _mm_store_ps(ssep->results, ssep->weightf_bgra);
                 #endif
                  *raster_dst16++ = clamp2word(ssep->results[0]);
                  *raster_dst16++ = clamp2word(ssep->results[1]);
                  *raster_dst16   = clamp2word(ssep->results[2]);

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
                 #ifdef __WATCOMC__
                  _mm_load1_ps_XMM0(&FLOAT_ZERO);
                 #else
                  ssep->weightf_bgra = _mm_load1_ps(&FLOAT_ZERO);
                 #endif

                  for (j = 0; j < contrib_y->n; ++j)
                  {
                      contrib_y_j = &(contrib_y->p[j]);
                      pBGR16      = (const gbm_u16 *)(raster_tmp + (contrib_y_j->pixel * rowspan_tmp));
                      b16         = *pBGR16++;
                      g16         = *pBGR16++;
                      r16         = *pBGR16++;
                      a16         = *pBGR16;
                     #ifdef __WATCOMC__
                      /*
                       * ssep->data         := XMM1
                       * ssep->weightf      := XMM2
                       * ssep->weightf_mul  := XMM3
                       * ssep->weightf_bgra := XMM0
                       */
                      _mm_set_ps_XMM1(a16, r16, g16, b16);
                      _mm_load1_ps_XMM2(&(contrib_y_j->weight));
                      _mm_mul_ps_XMM2_XMM1_XMM3();
                      _mm_add_ps_XMM0_XMM3_XMM0();
                     #else
                      ssep->data         = _mm_set_ps(a16, r16, g16, b16);
                      ssep->weightf      = _mm_load1_ps(&(contrib_y_j->weight));
                      ssep->weightf_mul  = _mm_mul_ps(ssep->weightf, ssep->data);
                      ssep->weightf_bgra = _mm_add_ps(ssep->weightf_bgra, ssep->weightf_mul);
                     #endif
                  }
                  /* store the pixel */
                  raster_dst16 = (gbm_u16 *) raster_dst;
                 #ifdef __WATCOMC__
                  _mm_store_ps_XMM0(ssep->results);
                 #else
                  _mm_store_ps(ssep->results, ssep->weightf_bgra);
                 #endif
                  *raster_dst16++ = clamp2word(ssep->results[0]);
                  *raster_dst16++ = clamp2word(ssep->results[1]);
                  *raster_dst16++ = clamp2word(ssep->results[2]);
                  *raster_dst16   = clamp2word(ssep->results[3]);

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
#endif /* SSE_ENABLED */

/* ---------- */

/* apply filter to zoom vertically from tmp to dst */
/* --> non-SSE version */

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
          int      xsize_dst    = dst->xsize;
    const int      ysize_dst    = dst->ysize;
    const int      rowspan_dst  = dst->span;
          int      xoffset_dst  = 0;

    const gbm_u8 *raster_tmp   = tmp->data;
    const int     rowspan_tmp  = tmp->span;

    const gbm_u8 *graylevels_tmp = tmp->graylevels;
          gbm_u8 *graylevels_dst = dst->graylevels;

    gbm_u8  b8 , g8 , r8 , a8;
    gbm_u16 b16, g16, r16, a16;

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
                  weightf   = 0.0f;

                  for (j = 0; j < contrib_y->n; ++j)
                  {
                      contrib_y_j = &(contrib_y->p[j]);
                      pBGR8       = raster_tmp + (contrib_y_j->pixel * rowspan_tmp);
                      g8          = graylevels_tmp[*pBGR8];

                      weightf += contrib_y_j->weight * g8;
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
                      b8          = *pBGR8++;
                      g8          = *pBGR8++;
                      r8          = *pBGR8;

                      weightf     = contrib_y_j->weight;
                      weightf_b  += weightf * b8;
                      weightf_g  += weightf * g8;
                      weightf_r  += weightf * r8;
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
                      b8          = *pBGR8++;
                      g8          = *pBGR8++;
                      r8          = *pBGR8++;
                      a8          = *pBGR8;

                      weightf     = contrib_y_j->weight;
                      weightf_b  += weightf * b8;
                      weightf_g  += weightf * g8;
                      weightf_r  += weightf * r8;
                      weightf_a  += weightf * a8;
                  }
                  /* store the pixel */
                   raster_dst8 = raster_dst;
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
                      b16         = *pBGR16++;
                      g16         = *pBGR16++;
                      r16         = *pBGR16;

                      weightf     = contrib_y_j->weight;
                      weightf_b  += weightf * b16;
                      weightf_g  += weightf * g16;
                      weightf_r  += weightf * r16;
                  }
                  /* store the pixel */
                   raster_dst16 = (gbm_u16 *) raster_dst;
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
                      b16         = *pBGR16++;
                      g16         = *pBGR16++;
                      r16         = *pBGR16++;
                      a16         = *pBGR16;

                      weightf     = contrib_y_j->weight;
                      weightf_b  += weightf * b16;
                      weightf_g  += weightf * g16;
                      weightf_r  += weightf * r16;
                      weightf_a  += weightf * a16;
                  }
                  /* store the pixel */
                   raster_dst16 = (gbm_u16 *) raster_dst;
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

static void Thread_apply_contributions(GBM_THREAD_ARG arg)
{
    CONTRIB_THREAD_ARG * pArgs = (CONTRIB_THREAD_ARG *) arg;
    pArgs->retval = pArgs->applyFunc(pArgs->src, pArgs->contrib, pArgs->dst);
}

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
    int   numCores = 1;            /* number of system cores in SMP system */
    FN_APPLYFUNC horizontalApplyFunc = NULL;
    FN_APPLYFUNC verticalApplyFunc   = NULL;

    if ((src      == NULL) ||
        (dst      == NULL) ||
        (filterf  == NULL) ||
        (src->bpp != dst->bpp))
    {
        return GBM_ERR_BAD_ARG;
    }
    if ((src->xsize < 1) || (src->ysize < 1) ||
        (dst->xsize < 1) || (dst->ysize < 1))
    {
        return GBM_ERR_BAD_ARG;
    }
    if ((src->xsize == 0) || (src->ysize == 0) ||
        (dst->xsize == 0) || (dst->ysize == 0))
    {
        /* nothing to do */
        return GBM_ERR_OK;
    }

    xscale = (float) dst->xsize / (float) src->xsize;
    yscale = (float) dst->ysize / (float) src->ysize;

    /* create intermediate image to hold vertical zoom */
    tmp = new_image(src->xsize, dst->ysize, dst->bpp, dst->graylevels);
    if (tmp == NULL)
    {
        return GBM_ERR_MEM;
    }

    /* check if SSE optimized version can be used */
   #ifdef SSE_ENABLED
    if (isSupported_SSE() && (src->bpp >= 24))
    {
        horizontalApplyFunc = apply_horizontal_contributions_sse;
        verticalApplyFunc   = apply_vertical_contributions_sse;
    }
    else
   #endif
    {
        horizontalApplyFunc = apply_horizontal_contributions;
        verticalApplyFunc   = apply_vertical_contributions;
    }

    /* get the number of available system cores */
    numCores = (int)gbmmthrd_getNumberOfCores();

    /* apply filter to zoom vertically from src to tmp */
    /* ----------------------------------------------- */

    /* pre-calculate filter contributions for a column */
    contrib = create_filter_contributions(src->ysize,
                                          tmp->ysize,
                                          yscale,
                                          filterf,
                                          fwidth);
    if (contrib == NULL)
    {
        free_image(tmp);
        return GBM_ERR_MEM;
    }

    /* Only run multiple threads if there are enough columns to process. */
    if ((numCores > 1) && (tmp->xsize >= numCores))
    {
        /* Split image vertically into slices and spawn
         * as much threads as there are cores in the system.
         */
        int                  core       = 0;
        Image              * srcList    = NULL;
        Image              * tmpList    = NULL;
        CONTRIB_THREAD_ARG * threadArgs = NULL;
        GBM_THREAD         * threadH    = NULL;

        srcList    = calloc(numCores, sizeof(Image));
        tmpList    = calloc(numCores, sizeof(Image));
        threadArgs = calloc(numCores, sizeof(CONTRIB_THREAD_ARG));
        threadH    = calloc(numCores, sizeof(GBM_THREAD));
        if ((NULL == srcList)    || (NULL == tmpList) ||
            (NULL == threadArgs) || (NULL == threadH))
        {
            free(srcList);    free(tmpList);
            free(threadArgs); free(threadH);
            free_contributions(contrib);
            free_image(tmp);
            return GBM_ERR_MEM;
        }

        /* Schedule the Image parts in background threads */
        for (core = 0; core < numCores-1; ++core)
        {
            srcList[core] = *src;
            tmpList[core] = *tmp;

            if (core > 0)
            {
                srcList[core]       = srcList[core-1];
                srcList[core].data += srcList[core-1].xsize * srcList[core-1].bpp / 8;

                tmpList[core]       = tmpList[core-1];
                tmpList[core].data += tmpList[core-1].xsize * tmpList[core-1].bpp / 8;
            }
            else
            {
                srcList[core] = *src;
                srcList[core].xsize /= numCores;

                tmpList[core] = *tmp;
                tmpList[core].xsize /= numCores;
            }

            threadArgs[core].contrib   = contrib;
            threadArgs[core].src       = &srcList[core];
            threadArgs[core].dst       = &tmpList[core];
            threadArgs[core].retval    = GBM_ERR_OK;
            threadArgs[core].applyFunc = verticalApplyFunc;

            threadH[core] = gbmmthrd_scheduleTask(Thread_apply_contributions,
                                                  &threadArgs[core], 8*1024);
        }

        /* calculate the remaining Image part in the current thread */
        srcList[core]        = srcList[core-1];
        srcList[core].xsize += src->xsize % numCores;
        srcList[core].data  += srcList[core-1].xsize * src->bpp / 8;

        tmpList[core]        = tmpList[core-1];
        tmpList[core].xsize += tmp->xsize % numCores;
        tmpList[core].data  += tmpList[core-1].xsize * tmp->bpp / 8;

        rc = verticalApplyFunc(&srcList[core], contrib, &tmpList[core]);

        for (core = 0; core < numCores-1; ++core)
        {
            /* was the thread successfully started? */
            if (NULL == threadH[core])
            {
                /* NO -> run sequentially */
                threadArgs[core].retval = verticalApplyFunc(&srcList[core],
                                                            contrib,
                                                            &tmpList[core]);
            }
            else
            {
                gbmmthrd_finishTask(&threadH[core]);
            }

            if ((rc == GBM_ERR_OK) && (threadArgs[core].retval != GBM_ERR_OK))
            {
                rc = threadArgs[core].retval;
            }
        }
        free(srcList);    free(tmpList);
        free(threadArgs); free(threadH);
    }
    else
    {
        rc = verticalApplyFunc(src, contrib, tmp);
    }

    /* free the memory allocated for vertical filter weights */
    free_contributions(contrib);

    if (rc != GBM_ERR_OK)
    {
        free_image(tmp);
        return rc;
    }

    /* apply filter to zoom horizontally from tmp to dst */
    /* ------------------------------------------------- */

    /* pre-calculate filter contributions for a row */
    contrib = create_filter_contributions(tmp->xsize,
                                          dst->xsize,
                                          xscale,
                                          filterf,
                                          fwidth);
    if (contrib == NULL)
    {
        free_image(tmp);
        return GBM_ERR_MEM;
    }

    /* Only run multiple threads if there are enough lines to process. */
    if ((numCores > 1) && (dst->ysize >= numCores))
    {
        /* Split image vertically into slices and spawn
         * as much threads as there are cores in the system.
         */
        int                  core       = 0;
        Image              * tmpList    = NULL;
        Image              * dstList    = NULL;
        CONTRIB_THREAD_ARG * threadArgs = NULL;
        GBM_THREAD         * threadH    = NULL;

        tmpList    = calloc(numCores, sizeof(Image));
        dstList    = calloc(numCores, sizeof(Image));
        threadArgs = calloc(numCores, sizeof(CONTRIB_THREAD_ARG));
        threadH    = calloc(numCores, sizeof(GBM_THREAD));
        if ((NULL == tmpList)    || (NULL == dstList) ||
            (NULL == threadArgs) || (NULL == threadH))
        {
            free(tmpList);    free(dstList);
            free(threadArgs); free(threadH);
            free_contributions(contrib);
            free_image(tmp);
            return GBM_ERR_MEM;
        }

        /* Schedule the Image parts in background threads */
        for (core = 0; core < numCores-1; ++core)
        {
            if (core > 0)
            {
                tmpList[core]       = tmpList[core-1];
                tmpList[core].data += tmp->span * tmpList[core-1].ysize;

                dstList[core]       = dstList[core-1];
                dstList[core].data += dst->span * dstList[core-1].ysize;
            }
            else
            {
                tmpList[core] = *tmp;
                tmpList[core].ysize /= numCores;

                dstList[core] = *dst;
                dstList[core].ysize /= numCores;
            }

            threadArgs[core].contrib   = contrib;
            threadArgs[core].src       = &tmpList[core];
            threadArgs[core].dst       = &dstList[core];
            threadArgs[core].retval    = GBM_ERR_OK;
            threadArgs[core].applyFunc = horizontalApplyFunc;

            threadH[core] = gbmmthrd_scheduleTask(Thread_apply_contributions,
                                                  &threadArgs[core], 8*1024);
        }

        /* calculate the remaining Image part in the current thread */
        tmpList[core]        = tmpList[core-1];
        tmpList[core].ysize += tmp->ysize % numCores;
        tmpList[core].data  += tmp->span * tmpList[core-1].ysize;

        dstList[core]        = dstList[core-1];
        dstList[core].ysize += dst->ysize % numCores;
        dstList[core].data  += dst->span * dstList[core-1].ysize;

        rc = horizontalApplyFunc(&tmpList[core], contrib, &dstList[core]);

        for (core = 0; core < numCores-1; ++core)
        {
            /* was the thread successfully started? */
            if (NULL == threadH[core])
            {
                /* NO -> run sequentially */
                threadArgs[core].retval = horizontalApplyFunc(&tmpList[core],
                                                              contrib,
                                                              &dstList[core]);
            }
            else
            {
                gbmmthrd_finishTask(&threadH[core]);
            }

            if ((rc == GBM_ERR_OK) && (threadArgs[core].retval != GBM_ERR_OK))
            {
                rc = threadArgs[core].retval;
            }
        }
        free(tmpList);    free(dstList);
        free(threadArgs); free(threadH);
    }
    else
    {
        rc = horizontalApplyFunc(tmp, contrib, dst);
    }

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
        const size_t stride_src = ((w * sbpp + 31)/32) * 4;
        const size_t stride_dst = ((w * 8 + 31)/32) * 4;
        int   x, y, i;
        gbm_u8  c = 0;

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
        (sw < 1) || (sh < 1) || (dw < 1) || (dh < 1))
    {
        return GBM_ERR_BAD_ARG;
    }

    switch(sbpp)
    {
        case 1:
        case 4:
        {
          const size_t stride8_src = ((sw * 8 + 31)/32) * 4;
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
            filterWidth = NEARESTNEIGHBOR_SUPPORT;
            break;

        case GBM_SCALE_FILTER_BILINEAR:
            filterFunc  = bilinear_filter;
            filterWidth = BILINEAR_SUPPORT;
            break;

        case GBM_SCALE_FILTER_BELL:
            filterFunc  = bell_filter;
            filterWidth = BELL_SUPPORT;
            break;

        case GBM_SCALE_FILTER_BSPLINE:
            filterFunc  = bspline_filter;
            filterWidth = BSPLINE_SUPPORT;
            break;

        case GBM_SCALE_FILTER_MITCHELL:
            filterFunc  = mitchell_filter;
            filterWidth = MITCHELL_SUPPORT;
            break;

        case GBM_SCALE_FILTER_LANCZOS:
            filterFunc  = lanczos3_filter;
            filterWidth = LANCZOS3_SUPPORT;
            break;

        case GBM_SCALE_FILTER_BLACKMAN:
            filterFunc  = blackman_filter;
            filterWidth = BLACKMAN_SUPPORT;
            break;

        case GBM_SCALE_FILTER_CATMULLROM:
            filterFunc  = catmullrom_filter;
            filterWidth = CATMULLROM_SUPPORT;
            break;

        case GBM_SCALE_FILTER_QUADRATIC:
            filterFunc  = quadratic_mix_filter;
            filterWidth = QUADRATIC_SUPPORT;
            break;

        case GBM_SCALE_FILTER_GAUSSIAN:
            filterFunc  = gaussian_filter;
            filterWidth = GAUSSIAN_SUPPORT;
            break;

        case GBM_SCALE_FILTER_KAISER:
            filterFunc  = kaiser_filter;
            filterWidth = KAISER_SUPPORT;
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
        (sw <  1)    || (sh <  1)    || (dw < 1) || (dh < 1))
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
            filterWidth = NEARESTNEIGHBOR_SUPPORT;
            break;

        case GBM_SCALE_FILTER_BILINEAR:
            filterFunc  = bilinear_filter;
            filterWidth = BILINEAR_SUPPORT;
            break;

        case GBM_SCALE_FILTER_BELL:
            filterFunc  = bell_filter;
            filterWidth = BELL_SUPPORT;
            break;

        case GBM_SCALE_FILTER_BSPLINE:
            filterFunc  = bspline_filter;
            filterWidth = BSPLINE_SUPPORT;
            break;

        case GBM_SCALE_FILTER_MITCHELL:
            filterFunc  = mitchell_filter;
            filterWidth = MITCHELL_SUPPORT;
            break;

        case GBM_SCALE_FILTER_LANCZOS:
            filterFunc  = lanczos3_filter;
            filterWidth = LANCZOS3_SUPPORT;
            break;

        case GBM_SCALE_FILTER_BLACKMAN:
            filterFunc  = blackman_filter;
            filterWidth = BLACKMAN_SUPPORT;
            break;

        case GBM_SCALE_FILTER_CATMULLROM:
            filterFunc  = catmullrom_filter;
            filterWidth = CATMULLROM_SUPPORT;
            break;

        case GBM_SCALE_FILTER_QUADRATIC:
            filterFunc  = quadratic_mix_filter;
            filterWidth = QUADRATIC_SUPPORT;
            break;

        case GBM_SCALE_FILTER_GAUSSIAN:
            filterFunc  = gaussian_filter;
            filterWidth = GAUSSIAN_SUPPORT;
            break;

        case GBM_SCALE_FILTER_KAISER:
            filterFunc  = kaiser_filter;
            filterWidth = KAISER_SUPPORT;
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

