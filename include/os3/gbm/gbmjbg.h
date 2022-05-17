/*

gbmjbg.h - JBIG format (JBG)

*/

#ifdef ENABLE_JBIG

extern GBM_ERR jbg_qft(GBMFT *gbmft);
extern GBM_ERR jbg_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR jbg_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR jbg_rdata(int fd, GBM *gbm, gbm_u8 *data);
extern GBM_ERR jbg_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt);
extern const char *jbg_err(GBM_ERR rc);

#endif /* ENABLE_JBIG */


