

BOOLEAN rgb16FromHex      (const byte *hex, int hex_len, GBMRGB_16BPP * gbmrgb16);
BOOLEAN rgb16FromColorName(const byte *color_name, int color_name_len, GBMRGB_16BPP * gbmrgb16);

BOOLEAN hexFromRgb16(const int bpp, const GBMRGB_16BPP * gbmrgb16, byte *hex, int hex_len);


