/* dd3Di.h */
/*****************************************************************************
 * SOURCE FILE NAME = dd3Di.h
 * DESCRIPTIVE NAME = Structures and defines for the DD3DI
 *
 * Copyright : Evgen & LightElf
 *
 * ==========================================================================
 *
 * VERSION = 0.01
 *
 * DESCRIPTION
 *      defines and data structures that are shared between the
 *      low level video 3D device driver and Mesa3d for OS/2
 *
 * NOTES
 *
 ****************************************************************************/

#ifndef  DD3DI_INCLUDED
  #define DD3DI_INCLUDED

#include "F_pgl.hpp"

#if defined(__IBMC__) || defined(__IBMCPP__)
  #if defined(__DLL__)
      #define SYSTEMCALL _System _Export
  #else
      #define SYSTEMCALL _System
  #endif
#else
  #define SYSTEMCALL _System _export
#endif


  /*
  **  Prototype declarations for low level driver routines
  */

  /*
  **  Graphics Hardware Interface (DD3DI) commands
  */

  #define DD3_CMD_INIT           0
  #define DD3_CMD_INITPROC       1
  #define DD3_CMD_TERM           2
  #define DD3_CMD_TERMPROC       3
  #define DD3_CMD_QUERYCAPS      4
  #define DD3_CMD_QUERYMODES     5
  #define DD3_CMD_SETMODE        6
  #define DD3_CMD_QUERYCURRENTMODE 7
  #define DD3_CMD_MAX            8

  /*
  **  Defines and structures for the DD3_CMD_QUERYCURRENTMODE
  */

  typedef struct _DD3_GDDMODEINFO {     /* gddmodeinfo */
     ULONG  ulLength;
     ULONG  ulModeId;               /* used to make SETMODE request     */
     ULONG  ulBpp;                  /* no of colors (bpp)               */
     ULONG  ulHorizResolution;      /* horizontal pels                  */
     ULONG  ulVertResolution;       /* vertical scan lines              */
     ULONG  ulRefreshRate;          /* in Hz (0 if not available)       */
     PBYTE  pbVRAMPhys;             /* physical address of VRAM         */
     ULONG  ulApertureSize;         /* Current bank size                */
     ULONG  ulScanLineSize;         /* size (in bytes) of one scan line */
     ULONG  fccColorEncoding;       /* Pel format (defines above        */
     ULONG  ulTotalVRAMSize;        /* Total size of VRAM in bytes      */
     ULONG  cColors;                /* Total number of colors           */
/* [... add something ...] */
  } DD3_GDDMODEINFO;
  typedef DD3_GDDMODEINFO *PDD3_GDDMODEINFO;
#define PDD3_GDDMODEINFO_DEFINED 1

  /*
  **  Defines and structures for the DD3_CMD_QUERYCAPS DDI
  */
  typedef struct _CAPSINFO3D {          /* capsinfo */
     ULONG        ulLength;               /* sizeof CAPSINFO3D structure    */
     char         szFunctionClassID[80];  /* Name describing function set   */
     ULONG        ulFCFlags;              /* Function class 2D specific flags  */
     ULONG        ulFCFlags3D;            /* Function class 3D specific flags  */
  } CAPSINFO3D;
  typedef CAPSINFO3D  *PCAPSINFO3D;

  /*
  ** Defines for ulFCFlags field of the CAPSINFO data structure
  ** NOTE: The flags below are GRADD Caps for the base function class.
  */
  #define GC_SEND_MEM_TO_MEM         0x00000001     // GRADD wants to see M2M blits
  #define GC_SIM_SRC_PAT_ROPS        0x00000002     // GRADD wants to simulate 3way rops as sequence of 2way rops.
  //RESERVED - NOT USED NOW
  #define GC_ALPHA_SUPPORT           0x00000004     // GRADD supports alpha blending rules and
                                                    // ALPHABITBLTINFO.
                                                    // If this flag is not returned,
                                                    // VMAN will never call the Gradd for
                                                    // Alpha Blending.
  // SHOULD match DS2_* in pmddi.h
  #define GC_SRC_STRETCH             0x00000008     // GRADD handles stretchblts
  #define GC_POLYGON_SIZE_ANY        0x00000010     // GRADD can handle polygon(concave or convex)
                                                    // of any size.
                                                    // GC_POLYGON_FILL must be set
  #define GC_CLIP                    0x00000020     // GRADD can handle single clip rect of
                                                    // polygons for pfnPolygon and
                                                    // source bitmap for pfnBitblt with
                                                    // GC_SRC_STRETCH set.
                                                    // see GC_CLP_COMPLEX
  #define GC_CLIP_COMPLEX            0x00000040     // GRADD can handle clipping
                                                    // with more than one clip rect
                                                    // GC_CLIP must be set.



  /* following defines GC_* SHOULD match DS_* in pmddi.h */
  #define GC_TEXTBLT_DOWNLOADABLE    0x00002000     // Downloadable Fonts
  #define GC_TEXTBLT_CLIPABLE        0x00004000     // CLIPPABLE    Fonts
  #define GC_TEXTBLT_DS_DEVICE_FONTS 0x00008000     // Device has Hardware Fonts
  #define GC_SIMPLE_LINES            0x00800000     // Device handles LINEINFO2.
  // For compatibility with old source code retain old definitions
  #define TEXTBLT_DOWNLOADABLE       GC_TEXTBLT_DOWNLOADABLE
  #define TEXTBLT_CLIPABLE           GC_TEXTBLT_CLIPABLE
  #define TEXTBLT_DS_DEVICE_FONTS    GC_TEXTBLT_DS_DEVICE_FONTS
#ifndef DS_SIMPLE_LINES
  #define DS_SIMPLE_LINES            GC_SIMPLE_LINES
#endif
  //RESERVED - NOT USED NOW
  #define GC_SRC_CONVERT             0x02000000     // send Device independent bitmaps
                                                    // without converting to device internal format.
                                                    // Gradd will convert during blts or
                                                    // call softdraw to simulate.
                                                    // If not set, GRE will convert DIB in a temporary buffer.
                                                    // Gradd should set this based on performance.
  #define GC_POLYGON_FILL            0x08000000     // GRADD can handle polygon(triangle and quad)
                                                    // fills and POLYGONINFO. see GC_POLYGON_SIZE_ANY
                                                    // if set, Graphics Engine call pfnPolygon
                                                    // for fills instead of pfnBitBlt.



#endif
       /* DD3DI_INCLUDED */
