/*

gbmgif.h - Graphics Interchange Format support

*/

extern GBM_ERR gif_qft(GBMFT *gbmft);
extern GBM_ERR gif_rimgcnt(const char *fn, int fd, int *pimgcnt);
extern GBM_ERR gif_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR gif_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR gif_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR gif_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *gif_err(GBM_ERR rc);
