/*
 *  struc.h:
 *  structure definitions
 */

/*
 *  segment descriptor structure
 */
_Packed struct desc {
  unsigned short ds_limit;       // limit
  unsigned short ds_baselo;      // lower 16-bits of base
  unsigned char  ds_basehi1;     // bits 16-23 of base
  unsigned char  ds_acclo;       // attributes
  unsigned char  ds_acchi;       // attributes
  unsigned char  ds_basehi2;     // bits 24-31 of base
};

/*
 *  GDTR descriptor
 */
_Packed struct gdtr {
  unsigned short g_limit;        // limit
  unsigned long  g_base;         // base
};
