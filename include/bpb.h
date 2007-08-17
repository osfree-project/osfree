/*
 *   A BPB structure
 *
 *   (c) osFree project
 *   valerius, 2007, Jan 13
 */

#include <types.h>

typedef struct _BPB {
   u8      jump[3];
   u8      sysid[8];
   // Standard BPB
   u16     secsize;
   u8      secperclu;
   u16     ressecs;
   u8      nfats;
   u16     rootdirsize;
   u16     nsec;
   u8      media;
   u16     fatsize;
   u16     secpertrack;
   u16     heads;
   u32     hiddensec;
   u32     nsec_ext;
   // Extended BPB
   u8      diskno;
   u8      logdrive;
   u8      marker;
   u32     volserno;
   u8      vollabel[11];
   u8      fsname[8];
} BPB;
