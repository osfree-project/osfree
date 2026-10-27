#ifndef PS_H_INCLUDED
#define PS_H_INCLUDED

/*
 * <os2.h> уже подт€гивает:
 *   os2def.h   Ч ERRORID, MAKEERRORID, SEVERITY_*, HPS, HDC, BOOL, ULONG, SIZEL
 *   bse.h      Ч base system
 *   pm.h -> pmwin.h, pmgpi.h, pmdev.h, pmwp.h, pmstddlg.h
 *              Ч PSIZEL, DM_DRAW / DM_RETAIN / DM_DRAWANDRETAIN,
 *                GPI_OK / GPI_HITS / GPI_ERROR
 *   pmerr.h    Ч PMERR_* (при INCL_WINERRORS / INCL_PM)
 */
#define INCL_PM
#define INCL_GPIERRORS
#include <os2.h>

#define PS_MAGIC            0x50535153UL  /* 'PSSQ' */

/*!
   @brief Presentation space (PS) instance data.

   PMGPI-level state associated with an HPS. Only PS-scoped state lives
   here Ч DC-scoped state (current position, attributes, clip, bounds,
   correlation) is held by the presentation driver in its DC instance
   data. The Gre* DDI macros in pmddim.h pass 0L for pInstance; the
   graphics engine substitutes the correct instance data by hdc.

   @todo: see struct  F_PS in FreePM.hpp. We must make in sync.
*/
typedef struct _PS {
    ULONG   ulMagic;          /*!< PS_MAGIC Ч validation */
    HPS     hpsSelf;          /*!< self-reference */
    HDC     hdc;              /*!< associated DC; 0 = no association */
    ULONG   ulDrawingMode;    /*!< DM_DRAW / DM_RETAIN / DM_DRAWANDRETAIN */

    /* TODO по мере реализации:
     *   SIZEL          sizlSize;
     *   ULONG          ulPageUnits;      (PU_*)
     *   SegmentList    segList;          (GpiOpenSegment/CloseSegment)
     *   SaveStack      saveStack;        (GpiSavePS/RestorePS)
     *   ULONG          ulFlagsArea;      (in-area state)
     *   ULONG          ulFlagsPath;      (in-path state)
     */
} PS, *PPS;

#define PS_FROM_HPS(h)  ((PPS)(h))
#define HPS_FROM_PS(p)  ((HPS)(p))

/*!
   @brief Look up a PS by its HPS.

   Validates the magic and self-reference, and returns the PS pointer.
   Intended for use inside PMGPI translation units (e.g. gpi32lines.c);
   not part of the public API.

   @param[in] hps  Handle to look up.

   @return Pointer to PS, or NULL if hps is not a valid PS handle.
*/
PPS LookupPS(HPS hps);

#endif /* PS_H_INCLUDED */
