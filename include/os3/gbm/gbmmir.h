/*

gbmmir.h - Interface to Mirror Image of General Bitmap

*/

#ifndef GBMMIR_H
#define	GBMMIR_H

#ifdef __cplusplus
  extern "C"
  {
#endif

extern BOOLEAN gbm_ref_vert(const GBM *gbm, byte *data);
extern BOOLEAN gbm_ref_horz(const GBM *gbm, byte *data);
extern void gbm_transpose(const GBM *gbm, const byte *data, byte *data_t);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif
