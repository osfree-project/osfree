/*

gbmpng.h - Portable Network Graphics Format support

*/

#ifdef ENABLE_PNG

extern GBM_ERR png_qft(GBMFT *gbmft);
extern GBM_ERR png_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR png_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR png_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR png_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *png_err(GBM_ERR rc);

#endif

