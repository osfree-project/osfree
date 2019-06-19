/*

gbmhist.h - Interface to Histogram/Frequency-of-use method of colour reduction

This code has been changed to expose the various steps in performing a
mapping to a frequency-of-use based palette. This is to allow people to write
code that computes a frequency-of-use based palette from a set of bitmaps,
and then map them all to this palette. This original gbm_hist function,
which works on one bitmap, has been rewritten in terms of the lower-level
functions for ease of compatibility.

*/

#ifndef GBMHIST_H
#define	GBMHIST_H

#ifdef __cplusplus
  extern "C"
  {
#endif

typedef void GBMHIST;

extern GBMHIST *gbm_create_hist(
	gbm_u8 rm, gbm_u8 gm, gbm_u8 bm
	);

extern void gbm_delete_hist(GBMHIST *hist);

extern gbm_boolean gbm_add_to_hist(
	GBMHIST *hist,	
	const GBM *gbm, const gbm_u8 *data24
	);

extern void gbm_pal_hist(
	GBMHIST *hist,
	GBMRGB gbmrgb[],
	int n_cols_wanted
	);

extern void gbm_map_hist(
	GBMHIST *hist,
	const GBM *gbm, const gbm_u8 *data24, gbm_u8 *data8
	);

extern gbm_boolean gbm_hist(
	const GBM *gbm, const gbm_u8 *data24,
	GBMRGB gbmrgb[],
	gbm_u8 *data8,
	int n_cols_wanted,
	gbm_u8 rm, gbm_u8 gm, gbm_u8 bm
	);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif
