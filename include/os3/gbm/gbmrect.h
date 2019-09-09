/*

gbmrect.c - Subrectangle Transfer

*/

#ifndef GBMRECT_H
#define	GBMRECT_H

#ifdef __cplusplus
  extern "C"
  {
#endif

extern void gbm_subrectangle(
	const GBM *gbm,
	int x, int y, int w, int h,
	const gbm_u8 *data_src, gbm_u8 *data_dst
	);

extern void gbm_blit(
	const gbm_u8 *s, int sw, int sx, int sy,
	      gbm_u8 *d, int dw, int dx, int dy,
	int w, int h,
	int bpp
	);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif
