/*
 *
 *
 */

#pragma pack(1)

struct dpb {
    unsigned char        dpb_drive;      // Logical drive # assoc with DPB (A=0,B=1,...)
    unsigned char        dpb_unit;       // Driver unit number of DPB
    unsigned long        dpb_driver_addr;// Pointer to driver
    struct dpb *         dpb_next_dpb;   // Pointer to next Drive parameter block
    unsigned short       dpb_cbSector;   // sector size (for volume checking)
    unsigned short       dpb_first_FAT;  // sector of 1st FAT (for ancient dev drivers)
    unsigned long        dpb_toggle_time;// time of last drive toggle
    unsigned short       dpb_hVPB;       // handle of volume currently in drive
    unsigned char        dpb_media;      // most recent media that was in drive
    unsigned char        dpb_flags;      // synchronization flags (see below)
    unsigned short       dpb_drive_lock; // Contains pid if drive locked by process
    unsigned long        (*dpb_strategy2)(void); // strategy2 addr (or 00000000)
};

struct devcaps
{
  unsigned short dc_reserved;        // 0 reserved, set to zero
  unsigned char  dc_maj;             // 2 major version of interface supported
  unsigned char  dc_min;             // 3 minor version of interface supported
  unsigned long  dc_capabilities;    // 4 bitfield for driver capabilties
  unsigned long  dc_strategy2;       // 8 entry point for strategy-2
  unsigned long  dc_endofint;        // c entry point for DD_EndOfInt
  unsigned long  dc_chgpriority;     // 10 entry point for DD_ChgPriority
  unsigned long  dc_setrestpos;      // 14 entry point for DD_SetRestPos
  unsigned long  dc_getboundary;     // 18 entry point for DD_GetBoundary
  unsigned long  dc_strategy3;       // 1c entry point for strategy-3
};

#pragma pack()
