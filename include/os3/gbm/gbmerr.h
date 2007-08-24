/*

gbmerr.h  Interface to error diffusion module

*/

#ifndef GBMERR_H
#define	GBMERR_H

#ifdef __cplusplus
  extern "C"
  {
#endif

extern void    gbm_errdiff_line_24(const byte *src, byte *dest, short *errs, int cx, byte rm, byte gm, byte bm);
extern BOOLEAN gbm_errdiff_24(const GBM *gbm, byte *data24, byte *data24a, byte rm, byte gm, byte bm);

extern void    gbm_errdiff_pal_6R6G6B(GBMRGB *gbmrgb);
extern void    gbm_errdiff_line_6R6G6B(const byte *src, byte *dest, short *errs, int cx);
extern BOOLEAN gbm_errdiff_6R6G6B(const GBM *gbm, const byte *data24, byte *data8);

extern void    gbm_errdiff_pal_7R8G4B(GBMRGB *gbmrgb);
extern void    gbm_errdiff_line_7R8G4B(const byte *src, byte *dest, short *errs, int cx);
extern BOOLEAN gbm_errdiff_7R8G4B(const GBM *gbm, const byte *data24, byte *data8);

extern void    gbm_errdiff_pal_4R4G4B(GBMRGB *gbmrgb);
extern void    gbm_errdiff_line_4R4G4B(const byte *src, byte *dest, short *errs, int cx);
extern BOOLEAN gbm_errdiff_4R4G4B(const GBM *gbm, const byte *data24, byte *data8);

extern void    gbm_errdiff_pal_VGA(GBMRGB *gbmrgb);
extern void    gbm_errdiff_line_VGA(const byte *src, byte *dest, short *errs, int cx);
extern BOOLEAN gbm_errdiff_VGA(const GBM *gbm, const byte *data24, byte *data4);

extern void    gbm_errdiff_pal_8(GBMRGB *gbmrgb);
extern void    gbm_errdiff_line_8(const byte *src, byte *dest, short *errs, int cx);
extern BOOLEAN gbm_errdiff_8(const GBM *gbm, const byte *data24, byte *data4);

extern void    gbm_errdiff_pal_4G(GBMRGB *gbmrgb);
extern void    gbm_errdiff_line_4G(byte *src, byte *dest, short *errs, int cx);
extern BOOLEAN gbm_errdiff_4G(GBM *gbm, byte *data24, byte *data4);

extern void    gbm_errdiff_pal_BW(GBMRGB *gbmrgb);
extern void    gbm_errdiff_line_BW(byte *src, byte *dest, short *errs, int cx);
extern BOOLEAN gbm_errdiff_BW(GBM *gbm, byte *data24, byte *data1);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif
