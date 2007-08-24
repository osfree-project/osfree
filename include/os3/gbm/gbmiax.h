/*

gbmiax.h - IBM Image Access eXecutive support

*/

extern GBM_ERR iax_qft(GBMFT *gbmft);
extern GBM_ERR iax_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR iax_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR iax_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR iax_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *iax_err(GBM_ERR rc);
