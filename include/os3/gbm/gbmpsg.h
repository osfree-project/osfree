/*

gbmpsg.h - PSEG support

*/

extern GBM_ERR psg_qft(GBMFT *gbmft);
extern GBM_ERR psg_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR psg_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR psg_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR psg_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *psg_err(GBM_ERR rc);
