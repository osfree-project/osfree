/*

gbmscale.h - Interface to scaling code

*/

#ifndef GBMSCALE_H
#define	GBMSCALE_H

#ifdef __cplusplus
  extern "C"
  {
#endif

extern GBM_ERR gbm_simple_scale(
	const byte *s, int sw, int sh,
	      byte *d, int dw, int dh,
	int bpp
	);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif
