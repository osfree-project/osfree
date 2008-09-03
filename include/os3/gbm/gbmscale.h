/*

gbmscale.h - Interface to scaling code

*/

#ifndef GBMSCALE_H
#define GBMSCALE_H

#ifdef __cplusplus
  extern "C"
  {
#endif

/* ------------------------ */

extern GBM_ERR gbm_simple_scale(
    const gbm_u8 *s, int sw, int sh,
          gbm_u8 *d, int dw, int dh,
    const int bpp);

/* ------------------------ */

typedef enum { GBM_SCALE_FILTER_NEARESTNEIGHBOR = 1,
               GBM_SCALE_FILTER_BILINEAR        = 2,
               GBM_SCALE_FILTER_BELL            = 3,
               GBM_SCALE_FILTER_BSPLINE         = 4,
               GBM_SCALE_FILTER_MITCHELL        = 5,
               GBM_SCALE_FILTER_LANCZOS         = 6
 } GBM_SCALE_FILTER;

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
extern GBM_ERR gbm_quality_scale_gray(
    const gbm_u8 *s , int sw, int sh, int sbpp, const GBMRGB * sgbmrgb,
          gbm_u8 *d8, int dw, int dh, GBMRGB * dgbmrgb,
    const GBM_SCALE_FILTER filter);

/* gbm_quality_scale_bgra - resample BGR[A] bitmaps using different filter types
 *
 * Supported color depths: 24bpp
 *                         32bpp (24bpp with alpha channel)
 *                         48bpp
 *                         64bpp (48bpp with alpha channel)
 */
extern GBM_ERR gbm_quality_scale_bgra(
    const gbm_u8 *s, int sw, int sh,
          gbm_u8 *d, int dw, int dh,
          int  bpp,
    const GBM_SCALE_FILTER filter);

/* ------------------------ */

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif
