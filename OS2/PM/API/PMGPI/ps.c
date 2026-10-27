/*!
   @file

   @ingroup pm

   @brief Presentation Manager GPI presentation spaces (32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

   Implements the PMGPI-side PS lifecycle (create/associate/destroy)
   and the PS-scoped drawing-mode queries. These functions operate on
   the PMGPI-internal PS structure and do not dispatch into the
   presentation driver; the driver only learns about drawing mode via
   the pattern of Gre* calls it receives (in DM_RETAIN PMGPI records
   elements instead of dispatching primitives).
*/

#include <stdlib.h>
#include "ps.h"

/*!
   @brief Look up a PS by its HPS (see ps.h for contract).
*/
PPS LookupPS(HPS hps)
{
    PPS pps;
    if (hps == (HPS)0) return NULL;
    pps = PS_FROM_HPS(hps);
    if (pps->ulMagic != PS_MAGIC || pps->hpsSelf != hps) return NULL;
    return pps;
}

/* Тихо: без WinSetErrorInfo. Возвращает FALSE и при невалидном HPS,
   и при отсутствии ассоциации — вызывающий сам решает, какой
   PMERR_* ставить. */
static BOOL HpsToDc(HPS hps, HDC *phdc)
{
    PPS pps = LookupPS(hps);
    if (pps == NULL || pps->hdc == (HDC)0) return FALSE;
    *phdc = pps->hdc;
    return TRUE;
}

/*!
   @brief Create a presentation space.

   Allocates a PMGPI PS structure, initializes it with default drawing
   state (DM_DRAW, no associated DC), and associates it with the given
   DC if hdc != 0.

   @param[in] hab          Anchor-block handle (currently unused).
   @param[in] hdc          DC handle to associate, or 0.
   @param[in] psizlSize    Required PS size (must not be NULL).
   @param[in] flOptions    Flags (currently unused; GPIA_ASSOC == 0
                           and is the default).

   @return Handle of the new PS, or 0 on error.

   @see GpiAssociate, GpiDestroyPS
*/
HPS APIENTRY GpiCreatePS(HAB hab, HDC hdc, PSIZEL psizlSize, ULONG flOptions)
{
    PPS pps;

    (void)hab;
    (void)flOptions;

    if (psizlSize == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return (HPS)0;
    }

    pps = (PPS)calloc(1, sizeof(PS));
    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INSUFFICIENT_MEMORY),
                        SEI_NOBEEP);
        return (HPS)0;
    }

    pps->ulMagic       = PS_MAGIC;
    pps->hpsSelf       = HPS_FROM_PS(pps);
    pps->hdc           = (HDC)0;
    pps->ulDrawingMode = DM_DRAW;

    /* TODO: сохранить psizlSize и page units (flOptions & PU_*).
       TODO: инициализировать пустой список сегментов, save stack. */

    if (hdc != (HDC)0) {
        if (!GpiAssociate(HPS_FROM_PS(pps), hdc)) {
            pps->ulMagic = 0;
            free(pps);
            return (HPS)0;
        }
    }

    return HPS_FROM_PS(pps);
}

/*!
   @brief Associate a presentation space with a device context.

   Sets (or replaces) the DC associated with hps. Passing hdc = 0 breaks
   the association without destroying the PS or the DC.

   @param[in] hps   Presentation space handle.
   @param[in] hdc   DC to associate, or 0 to detach.

   @return TRUE on success, FALSE if hps is invalid.

   @see GpiQueryDevice
*/
BOOL APIENTRY GpiAssociate(HPS hps, HDC hdc)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }

    pps->hdc = hdc;
    return TRUE;
}

/*!
   @brief Destroy a presentation space.

   Breaks the association (DC stays open; closing it is the caller's
   responsibility via DevCloseDC) and frees the PS structure.

   @param[in] hps   Presentation space handle.

   @return TRUE on success, FALSE if hps is invalid.

   @see GpiCreatePS
*/
BOOL APIENTRY GpiDestroyPS(HPS hps)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }

    pps->hdc = (HDC)0;

    /* TODO: освободить сегменты, save stack, прочие ресурсы. */

    pps->ulMagic = 0;
    pps->hpsSelf = (HPS)0;
    free(pps);

    return TRUE;
}

/*!
   @brief Return the DC associated with a presentation space.

   @param[in] hps   Presentation space handle.

   @return Associated DC handle, or 0 if hps is invalid or the PS has
           no associated DC.

   @see GpiAssociate
*/
HDC APIENTRY GpiQueryDevice(HPS hps)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return (HDC)0;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return (HDC)0;
    }
    return pps->hdc;
}

/*!
   @brief Set the drawing mode.

   DM_DRAW and DM_DRAWANDRETAIN dispatch primitives into the driver;
   DM_RETAIN records them into the current segment without invoking
   the driver.

   @param[in] hps    Presentation space handle.
   @param[in] lMode  DM_DRAW, DM_RETAIN, or DM_DRAWANDRETAIN.

   @return TRUE on success, FALSE on invalid hps or mode.

   @see GpiQueryDrawingMode
*/
BOOL APIENTRY GpiSetDrawingMode(HPS hps, LONG lMode)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (lMode != DM_DRAW && lMode != DM_RETAIN && lMode != DM_DRAWANDRETAIN) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_DRAWING_MODE),
                        SEI_NOBEEP);
        return FALSE;
    }

    pps->ulDrawingMode = (ULONG)lMode;
    return TRUE;
}

/*!
   @brief Query the drawing mode.

   @param[in] hps   Presentation space handle.

   @return Current drawing mode, or GPI_ERROR on invalid hps.

   @see GpiSetDrawingMode
*/
LONG APIENTRY GpiQueryDrawingMode(HPS hps)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    return (LONG)pps->ulDrawingMode;
}
