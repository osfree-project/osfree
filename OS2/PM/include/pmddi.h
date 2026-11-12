#ifndef PMDDI_H
#define PMDDI_H

#define DDI_INCLUDED

#ifdef INCL_GREALL
   #define INCL_GRE_ARCS
   #define INCL_GRE_BITMAPS
   #define INCL_GRE_CLIP
   #define INCL_GRE_COLORTABLE
   #define INCL_GRE_DCS
   #define INCL_GRE_PAX
   #define INCL_GRE_DEVICE
   #define INCL_GRE_DEVMISC
   #define INCL_GRE_DEVSUPPORT
   #define INCL_GRE_FONTS
   #define INCL_GRE_INKPATH
   #define INCL_GRE_JOURNALING
   #define INCL_GRE_LCID
   #define INCL_GRE_LINES
   #define INCL_GRE_MARKERS
   #define INCL_GRE_PALETTE
   #define INCL_GRE_PATHS
   #define INCL_GRE_PICK
   #define INCL_GRE_POLYGON
   #define INCL_GRE_REGIONS
   #define INCL_GRE_SCANS
   #define INCL_GRE_SCREEN
   #define INCL_GRE_SETID
   #define INCL_GRE_STRINGS
   #define INCL_GRE_XFORMS
#endif /* INCL_GREALL */

#ifdef INCL_GRE_DEVMISC
   #define INCL_GRE_DEVMISC1
   #define INCL_GRE_DEVMISC2
   #define INCL_GRE_DEVMISC3
#endif  /* INCL_GRE_DEVMISC */

#ifdef __cplusplus
      extern "C" {
#endif

#ifdef INCL_GRE_ARCS

typedef LONG PHID;

typedef struct _BOXPARAMS
{
  POINTL ptl;
  SIZEL  sizl;
} BOXPARAMS, *PBOXPARAMS;

#endif /* INCL_GRE_ARCS */

#define CHS_START_XY                    0x00000020L
#define CHS_ATTR_INFO                   0x00000040L

typedef struct _CSP_INFO
{
  LONG  cSize;
  LONG  lColor;
  LONG  lBackColor;
} CSP_INFO, *PCSP_INFO;

/*!
   @brief Parameters for the DC Enable subfunction (EnableDeviceContext).

   Passed as @c pParam1 to @c OS2_PM_DRV_ENABLE(EN_ENABLE_CTX, ...).
   Layout matches IBM PMDDI.H, INCL_DDIMISC, "Parameters for the DC
   Enable function".
*/
typedef struct _DENPARAMS
{
    ULONG ulStateInfo;   /*!< Value returned by FillPhysicalDeviceBlock. */
    ULONG ulType;        /*!< OD_QUEUED / OD_DIRECT / OD_INFO / OD_MEMORY. */
    ULONG ulHDC;         /*!< Device context handle. */
} DENPARAMS, *PDENPARAMS;

/*!
   @brief GRE transform matrix (XFORM).

   Layout matches the OS/2 GRE XFORM structure used by
   GreGetModelXform / GreSetModelXform. 9 elements: fxM11..lM33.
*/
typedef struct _XFORM {
    FIXED fxM11;
    FIXED fxM12;
    LONG  lM13;
    FIXED fxM21;
    FIXED fxM22;
    LONG  lM23;
    LONG  lM31;
    LONG  lM32;
    LONG  lM33;
} XFORM, *PXFORM;

/* ======================================================================== */
/*  Presentation Driver Enable interface (OS2_PM_DRV_ENABLE)              */
/*                                                                        */
/*  Contract between the graphics engine and a presentation driver.        */
/*  Not part of IBM PMDDI.H — declared here so that gre.cpp (loader) and   */
/*  gre2sdl.cpp (driver) share one definition.                             */
/* ======================================================================== */

/*!
   @brief Calling convention for OS2_PM_DRV_ENABLE function pointers.

   On OS/2, @c APIENTRY expands to @c _System, which is valid in a
   function-pointer typedef.  On Win32, @c APIENTRY / @c EXPENTRY expand
   to @c __declspec(__pragma("__estdcall")), which Watcom rejects in a
   typedef; @c __stdcall is used there instead (same stack-cleanup
   semantics, standard syntax).  Same pattern as @c F_DBG_WIRE_CALLBACK
   in F_debug.h.
*/
#if defined(__OS2__) || defined(__OS2V2__)
  #define DDI_CALLCONV APIENTRY
#else
  #define DDI_CALLCONV __stdcall
#endif

/*!
   @brief Input structure for FillLogicalDeviceBlock
          (@c param1 of @c OS2_PM_DRV_ENABLE).
*/
typedef struct _FLParamsLst
{
    ULONG  Version;      /*!< GRE version in BCD. */
    USHORT TableSize;    /*!< Number of entries in the dispatch table. */
} FLParamsLst, *PFLParamsLst;

/*!
   @brief Return structure for FillLogicalDeviceBlock
          (@c param2 of @c OS2_PM_DRV_ENABLE).

   @c DispTab points to the driver's copy of the dispatch table —
   an array of 32-bit function pointers. The low-order byte of an
   @c NGre* function code identifies the offset into this array.
*/
typedef struct _FLReturnsLst
{
    PUSHORT Flags;       /*!< Pointer to a WORD of logical device flags. */
    PFN    *DispTab;     /*!< Pointer to the dispatch table (array of PFN). */
} FLReturnsLst, *PFLReturnsLst;

/*!
   @brief Type of @c OS2_PM_DRV_ENABLE (ordinal 200 of a presentation
          driver).

   Three parameters: subfunction, param1, param2.  @c pDispatchTable
   is not a fourth parameter — for @c FillLogicalDeviceBlock it is
   passed inside @c param2 (@c FLReturnsLst.DispTab).
*/
typedef ULONG (DDI_CALLCONV *PFN_OS2_PM_DRV_ENABLE)(ULONG subfunc,
                                                     PVOID param1,
                                                     PVOID param2);

/* ---- OS2_PM_DRV_ENABLE subfunction codes ------------------------------- */
#define EN_FILL_LDB         0x00000001UL  /*!< FillLogicalDeviceBlock. */
#define EN_FILL_PDB         0x00000002UL  /*!< FillPhysicalDeviceBlock. */
#define EN_DISABLE_PDB      0x00000004UL  /*!< DisablePhysicalDeviceBlock. */
#define EN_ENABLE_CTX       0x00000005UL  /*!< EnableDeviceContext. */
#define EN_DISABLE_CTX      0x00000006UL  /*!< DisableDeviceContext. */
#define EN_SAVE_CTX         0x00000007UL  /*!< SaveDCState. */
#define EN_RESTORE_CTX      0x00000008UL  /*!< RestoreDCState. */
#define EN_RESET_CTX        0x00000009UL  /*!< ResetDCState. */
#define EN_COMPLETE_OPENDC  0x0000000AUL  /*!< CompleteOpenDC. */
#define EN_BEGIN_CLOSEDC    0x0000000BUL  /*!< BeginCloseDC. */
#define EN_INSTALL_HOOKS    0x0000000CUL  /*!< InstallPMEngineDisplayHooks. */

#ifdef INCL_GRE_CLIP
   
   /* CopyClipRegion */
   
   #define COPYCRGN_ALLINTERSECT        0L
   #define COPYCRGN_VISRGN              1L
   #define COPYCRGN_CLIPRGN             2L
   
   /* SetupDC */
   
   #define SETUPDC_VISRGN               0x00000001L
   #define SETUPDC_ORIGIN               0x00000002L
   #define SETUPDC_ACCUMBOUNDSON        0x00000004L
   #define SETUPDC_ACCUMBOUNDSOFF       0x00000008L
   #define SETUPDC_RECALCCLIP           0x00000010L
   #define SETUPDC_SETOWNER             0x00000020L
   #define SETUPDC_CLEANDC              0x00000040L
   
#endif /* INCL_GRE_CLIP */


#ifdef __cplusplus
      }
#endif

#include <pmddim.h>

#endif