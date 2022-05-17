/*

gbmraw.h - RAW formats

Reads and writes 24 and 48 bit RGB.

*/

#ifdef ENABLE_RAW

extern GBM_ERR raw_qft(GBMFT *gbmft);
extern GBM_ERR raw_rimgcnt(const char *fn, int fd, int *pimgcnt);
extern GBM_ERR raw_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR raw_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR raw_rdata(int fd, GBM *gbm, gbm_u8 *data);
extern const char *raw_err(GBM_ERR rc);

#endif
