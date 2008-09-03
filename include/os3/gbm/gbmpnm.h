/*

gbmpnm.h - Poskancers PNM format

*/

extern GBM_ERR pnm_qft(GBMFT *gbmft);
extern GBM_ERR pnm_rimgcnt(const char *fn, int fd, int *pimgcnt);
extern GBM_ERR pnm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR pnm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR pnm_rdata(int fd, GBM *gbm, gbm_u8 *data);
extern GBM_ERR pnm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt);
extern const char *pnm_err(GBM_ERR rc);

