/*

gbmtif.h - Microsoft/Aldus Tagged Image File Format support

*/

#ifdef ENABLE_TIF

extern GBM_ERR tif_qft(GBMFT *gbmft);
extern GBM_ERR tif_rimgcnt(const char *fn, int fd, int *pimgcnt);
extern GBM_ERR tif_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR tif_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR tif_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR tif_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *tif_err(GBM_ERR rc);

#endif

