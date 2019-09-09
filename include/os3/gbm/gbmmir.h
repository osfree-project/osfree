/*

gbmmir.h - Interface to Mirror Image of General Bitmap

*/

#ifndef GBMMIR_H
#define	GBMMIR_H

#ifdef __cplusplus
  extern "C"
  {
#endif

extern gbm_boolean gbm_ref_vert(const GBM *gbm, gbm_u8 *data);
extern gbm_boolean gbm_ref_horz(const GBM *gbm, gbm_u8 *data);
extern void gbm_transpose(const GBM *gbm, const gbm_u8 *data, gbm_u8 *data_t);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif
