/*

gbmmap.h - Map RGBA to palette or BGRx bitmap data

*/

#ifndef GBMMAP_H
#define GBMMAP_H

#pragma pack(2)
typedef struct
{
   gbm_u16 r;
   gbm_u16 g;
   gbm_u16 b;
} GBMRGB_16BPP;
#pragma pack()

gbm_boolean gbm_map_row_PAL_PAL(const gbm_u8 * data_src, const GBM * gbm_src,
                                      gbm_u8 * data_dst, const GBM * gbm_dst);

gbm_boolean gbm_map_row_PAL_BGR(const gbm_u8       * data_src, const GBM * gbm_src,
                                      gbm_u8       * data_dst, const GBM * gbm_dst,
                                const GBMRGB_16BPP * gbmrgb_src);

gbm_boolean gbm_map_row_RGBx_BGRx(const gbm_u8       * data_src, const GBM * gbm_src,
                                        gbm_u8       * data_dst, const GBM * gbm_dst,
                                  const GBMRGB_16BPP * back_rgb, const gbm_boolean unassociatedAlpha);

gbm_boolean gbm_map_row_CMYK_to_BGR(const gbm_u8 * data_src, const GBM * gbm_src,
                                          gbm_u8 * data_dst, const GBM * gbm_dst,
                                    const gbm_boolean component_inverse);

gbm_boolean gbm_map_sep_row_CMYK_to_BGR(const gbm_u8 * data_src, const GBM * gbm_src,
                                              gbm_u8 * data_dst, const GBM * gbm_dst,
                                        const int    sample  , const gbm_boolean component_inverse);

gbm_boolean gbm_map_sep_row_RGBx_BGRx(const gbm_u8       * data_src, const GBM * gbm_src,
                                            gbm_u8       * data_dst, const GBM * gbm_dst,
                                      const GBMRGB_16BPP * back_rgb, const gbm_boolean unassociatedAlpha,
                                      const int            sample);

gbm_boolean gbm_map_RGBA_BGR(const gbm_u32 * data_src,       gbm_u8       * data_dst,
                             const GBM     * gbm_dst , const GBMRGB_16BPP * back_rgb,
                             const gbm_boolean unassociatedAlpha);

gbm_boolean gbm_map_RGBA_RGB(const gbm_u32 * data_src,       gbm_u8       * data_dst,
                             const GBM     * gbm_dst , const GBMRGB_16BPP * back_rgb,
                             const gbm_boolean unassociatedAlpha);

gbm_boolean gbm_map_RGBA_PAL(const gbm_u32 * data_src,       gbm_u8 * data_dst,
                             const GBM     * gbm_dst , const GBMRGB * gbmrgb_dst);

#endif


