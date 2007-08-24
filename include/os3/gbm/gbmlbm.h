/*

gbmlbm.c - Amiga IFF / ILBM file format

*/

extern GBM_ERR lbm_qft(GBMFT *gbmft);
extern GBM_ERR lbm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR lbm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR lbm_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR lbm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *lbm_err(GBM_ERR rc);
