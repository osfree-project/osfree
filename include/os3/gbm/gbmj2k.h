/*

gbmj2k.h - JPEG2000 Format support (JP2, J2K, JPC, JPT)

*/

#ifdef ENABLE_J2K

extern GBM_ERR j2k_jp2_qft(GBMFT *gbmft);
extern GBM_ERR j2k_j2k_qft(GBMFT *gbmft);
extern GBM_ERR j2k_jpt_qft(GBMFT *gbmft);

extern GBM_ERR j2k_jp2_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR j2k_j2k_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR j2k_jpt_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);

extern GBM_ERR j2k_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR j2k_rdata(int fd, GBM *gbm, gbm_u8 *data);

extern GBM_ERR j2k_jp2_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt);
extern GBM_ERR j2k_j2k_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt);

extern const char *j2k_err(GBM_ERR rc);

#endif

