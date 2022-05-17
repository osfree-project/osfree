

gbm_boolean rgb16FromHex      (const gbm_u8 *hex, size_t hex_len, GBMRGB_16BPP * gbmrgb16);
gbm_boolean rgb16FromColorName(const gbm_u8 *color_name, size_t color_name_len, GBMRGB_16BPP * gbmrgb16);

gbm_boolean hexFromRgb16(const int bpp, const GBMRGB_16BPP * gbmrgb16, gbm_u8 *hex, size_t hex_len);


