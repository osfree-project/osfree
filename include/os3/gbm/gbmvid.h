/*

gbmvid.h - YUV12C M-Motion Video Frame Buffer format

*/

extern GBM_ERR vid_qft(GBMFT *gbmft);
extern GBM_ERR vid_rhdr(const char *fn, int fd, GBM *gbm, const char *opt);
extern GBM_ERR vid_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb);
extern GBM_ERR vid_rdata(int fd, GBM *gbm, byte *data);
extern GBM_ERR vid_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt);
extern const char *vid_err(GBM_ERR rc);
