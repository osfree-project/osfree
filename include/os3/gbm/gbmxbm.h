/*

gbmxbm.h - X Windows bitmap support

*/

extern GBM_ERR xbm_qft(GBMFT *gbmft);
extern GBM_ERR xbm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR xbm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR xbm_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR xbm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *xbm_err(GBM_ERR rc);
