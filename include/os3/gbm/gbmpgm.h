/*

gbmpgm.h - Poskancers PGM format

*/

extern GBM_ERR pgm_qft(GBMFT *gbmft);
extern GBM_ERR pgm_rimgcnt(const char *fn, int fd, int *pimgcnt);
extern GBM_ERR pgm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR pgm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR pgm_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR pgm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *pgm_err(GBM_ERR rc);

