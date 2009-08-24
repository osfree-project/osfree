/*

gbmspr.h - Archimedes Sprite from RiscOS Format support

*/

extern GBM_ERR spr_qft(GBMFT *gbmft);
extern GBM_ERR spr_rimgcnt(const char *fn, int fd, int *pimgcnt);
extern GBM_ERR spr_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR spr_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR spr_rdata(int fd, GBM *gbm, gbm_u8 *data);
extern GBM_ERR spr_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt);
extern const char *spr_err(GBM_ERR rc);
