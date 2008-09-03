/*

gbmerr.h  Interface to error diffusion module

*/

#ifndef GBMERR_H
#define	GBMERR_H

#ifdef __cplusplus
  extern "C"
  {
#endif

extern void        gbm_errdiff_line_24(const gbm_u8 *src, gbm_u8 *dest, short *errs, int cx, gbm_u8 rm, gbm_u8 gm, gbm_u8 bm);
extern gbm_boolean gbm_errdiff_24(const GBM *gbm, gbm_u8 *data24, gbm_u8 *data24a, gbm_u8 rm, gbm_u8 gm, gbm_u8 bm);

extern void        gbm_errdiff_pal_6R6G6B(GBMRGB *gbmrgb);
extern void        gbm_errdiff_line_6R6G6B(const gbm_u8 *src, gbm_u8 *dest, short *errs, int cx);
extern gbm_boolean gbm_errdiff_6R6G6B(const GBM *gbm, const gbm_u8 *data24, gbm_u8 *data8);

extern void        gbm_errdiff_pal_7R8G4B(GBMRGB *gbmrgb);
extern void        gbm_errdiff_line_7R8G4B(const gbm_u8 *src, gbm_u8 *dest, short *errs, int cx);
extern gbm_boolean gbm_errdiff_7R8G4B(const GBM *gbm, const gbm_u8 *data24, gbm_u8 *data8);

extern void        gbm_errdiff_pal_4R4G4B(GBMRGB *gbmrgb);
extern void        gbm_errdiff_line_4R4G4B(const gbm_u8 *src, gbm_u8 *dest, short *errs, int cx);
extern gbm_boolean gbm_errdiff_4R4G4B(const GBM *gbm, const gbm_u8 *data24, gbm_u8 *data8);

extern void        gbm_errdiff_pal_VGA(GBMRGB *gbmrgb);
extern void        gbm_errdiff_line_VGA(const gbm_u8 *src, gbm_u8 *dest, short *errs, int cx);
extern gbm_boolean gbm_errdiff_VGA(const GBM *gbm, const gbm_u8 *data24, gbm_u8 *data4);

extern void        gbm_errdiff_pal_8(GBMRGB *gbmrgb);
extern void        gbm_errdiff_line_8(const gbm_u8 *src, gbm_u8 *dest, short *errs, int cx);
extern gbm_boolean gbm_errdiff_8(const GBM *gbm, const gbm_u8 *data24, gbm_u8 *data4);

extern void        gbm_errdiff_pal_4G(GBMRGB *gbmrgb);
extern void        gbm_errdiff_line_4G(gbm_u8 *src, gbm_u8 *dest, short *errs, int cx);
extern gbm_boolean gbm_errdiff_4G(GBM *gbm, gbm_u8 *data24, gbm_u8 *data4);

extern void        gbm_errdiff_pal_BW(GBMRGB *gbmrgb);
extern void        gbm_errdiff_line_BW(gbm_u8 *src, gbm_u8 *dest, short *errs, int cx);
extern gbm_boolean gbm_errdiff_BW(GBM *gbm, gbm_u8 *data24, gbm_u8 *data1);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif
