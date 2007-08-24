/*

gbmpcx.h - ZSoft PC Paintbrush support

*/

extern GBM_ERR pcx_qft(GBMFT *gbmft);
extern GBM_ERR pcx_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR pcx_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR pcx_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR pcx_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *pcx_err(GBM_ERR rc);
