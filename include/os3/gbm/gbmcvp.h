/*

gbmcvp.h - IBM Image Access eXecutive support

*/

extern GBM_ERR cvp_qft(GBMFT *gbmft);
extern GBM_ERR cvp_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR cvp_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR cvp_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR cvp_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *cvp_err(GBM_ERR rc);
