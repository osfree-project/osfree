/*

gbmmap.h - Map RGBA to palette or BGRx bitmap data

*/

#ifndef GBMMAP_H
#define GBMMAP_H

#pragma pack(2)
typedef struct
{
   word r;
   word g;
   word b;
} GBMRGB_16BPP;
#pragma pack()

BOOLEAN gbm_map_row_PAL_PAL(const byte * data_src, const GBM * gbm_src,
                                  byte * data_dst, const GBM * gbm_dst);

BOOLEAN gbm_map_row_PAL_BGR(const byte         * data_src, const GBM * gbm_src,
                                  byte         * data_dst, const GBM * gbm_dst,
                            const GBMRGB_16BPP * gbmrgb_src);

BOOLEAN gbm_map_row_RGBx_BGRx(const byte         * data_src, const GBM * gbm_src,
                                    byte         * data_dst, const GBM * gbm_dst,
                              const GBMRGB_16BPP * back_rgb, const BOOLEAN unassociatedAlpha);

BOOLEAN gbm_map_row_CMYK_to_BGR(const byte * data_src, const GBM * gbm_src,
                                      byte * data_dst, const GBM * gbm_dst,
                                const BOOLEAN component_inverse);

BOOLEAN gbm_map_sep_row_CMYK_to_BGR(const byte * data_src, const GBM * gbm_src,
                                          byte * data_dst, const GBM * gbm_dst,
                                    const int    sample  , const BOOLEAN component_inverse);

BOOLEAN gbm_map_sep_row_RGBx_BGRx(const byte         * data_src, const GBM * gbm_src,
                                        byte         * data_dst, const GBM * gbm_dst,
                                  const GBMRGB_16BPP * back_rgb, const BOOLEAN unassociatedAlpha,
                                  const int            sample);

BOOLEAN gbm_map_RGBA_BGR(const dword   * data_src,       byte         * data_dst,
                         const GBM     * gbm_dst , const GBMRGB_16BPP * back_rgb,
                         const BOOLEAN   unassociatedAlpha);

BOOLEAN gbm_map_RGBA_RGB(const dword * data_src,       byte         * data_dst,
                         const GBM   * gbm_dst , const GBMRGB_16BPP * back_rgb,
                         const BOOLEAN unassociatedAlpha);

BOOLEAN gbm_map_RGBA_PAL(const dword * data_src,       byte   * data_dst,
                         const GBM   * gbm_dst , const GBMRGB * gbmrgb_dst);

#endif


