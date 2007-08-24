/*

gbmmcut.h - Interface to Median Cut colour reduction

This code is arranged to allow generation of a palette from a set of input
images. Quantisation is fixed at 5 bits each for red, green and blue. This
is finer than most implementations, but costly on memory.

*/

#ifndef GBMMCUT_H
#define GBMMCUT_H

#ifdef __cplusplus
  extern "C"
  {
#endif

typedef void GBMMCUT;

extern GBMMCUT *gbm_create_mcut(void);

extern void gbm_delete_mcut(GBMMCUT *mcut);

extern void gbm_add_to_mcut(
	GBMMCUT *mcut,	
	const GBM *gbm, const byte *data24
	);

extern void gbm_pal_mcut(
	GBMMCUT *mcut,
	GBMRGB gbmrgb[],
	int n_cols_wanted
	);

extern void gbm_map_mcut(
	GBMMCUT *mcut,
	const GBM *gbm, const byte *data24, byte *data8
	);

extern BOOLEAN gbm_mcut(
	const GBM *gbm, const byte *data24,
	GBMRGB gbmrgb[],
	byte *data8,
	int n_cols_wanted
	);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif
