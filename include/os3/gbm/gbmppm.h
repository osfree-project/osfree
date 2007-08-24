/*

gbmppm.h - PPM format

Reads and writes 24 and 48 bit RGB.

*/

extern GBM_ERR ppm_qft(GBMFT *gbmft);
extern GBM_ERR ppm_rimgcnt(const char *fn, int fd, int *pimgcnt);
extern GBM_ERR ppm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR ppm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR ppm_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR ppm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *ppm_err(GBM_ERR rc);

