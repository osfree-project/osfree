/*!
   @file

   @ingroup pm

   @brief Presentation Manager GPI presentation spaces (32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

   Implements the PMGPI-side PS lifecycle (create/associate/destroy),
   the PS-scoped drawing-mode queries, the DC/PS control functions,
   and the PS save/restore stack. These functions operate on the
   PMGPI-internal PS structure and do not dispatch into the
   presentation driver (GpiErase is the exception — it forwards to
   GreErasePS). The driver only learns about drawing mode via the
   pattern of Gre* calls it receives (in DM_RETAIN PMGPI records
   elements instead of dispatching primitives).

   Error codes used by this file are all defined in <pmgpi.h> /
   <pmerr.h>:
     PMERR_INV_HPS                (0x207F)
     PMERR_INV_HDC                (0x207C)
     PMERR_INV_LENGTH_OR_COUNT    (0x2092)
     PMERR_INSUFFICIENT_MEMORY    (0x203E)
     PMERR_INV_DRAWING_MODE       (0x2065)
     PMERR_INV_RESET_OPTIONS      (0x20C2)
     PMERR_INV_DRAW_CONTROL       (0x2063)
     PMERR_INV_DRAW_VALUE         (0x2064)
     PMERR_INV_STOP_DRAW_VALUE    (0x20CF)

   Note: PMERR_INV_PSID does not exist in <pmerr.h>; for a bad save
   id GpiRestorePS uses PMERR_INV_LENGTH_OR_COUNT, which is the
   closest match among the codes defined for the GPI.
*/

#include <stdlib.h>
#include <string.h>
#include "ps.h"

/*!
   @brief Draw-control selector to bit index.

   lControl is one of DCTL_ERASE (1) .. DCTL_CORRELATE (5). The
   corresponding bit in PS.ulDrawControl is 1 << (lControl - 1).

   @param[in] c  DCTL_* selector.

   @return Bit mask.
*/
#define DCTL_BIT(c)  (1UL << ((c) - 1))

/*!
   @brief Allocate a zero-filled block using OS/2 CPI.

   Wraps DosAllocMem (PAG_COMMIT | PAG_READ | PAG_WRITE) and zeroes
   the returned memory, providing calloc-like semantics on top of the
   OS/2 memory manager. Used by PMGPI-internal structures (PS,
   segments, elements, saved-state entries) and by the 16->32 bitmap
   conversion.

   @param[in] cb  Size in bytes.

   @return Pointer to the new block, or NULL on allocation failure.
*/
static PVOID AllocMemZero(ULONG cb)
{
    PVOID p = NULL;

    if (DosAllocMem(&p, cb, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
        return NULL;
    memset(p, 0, cb);
    return p;
}

/*!
   @brief Look up a PS by its HPS (see ps.h for contract).

   @param[in] hps  Handle to look up.
   @return Pointer to PS or NULL (error info set on failure).
*/
PPS LookupPS(HPS hps)
{
    PPS pps;

    if (hps == (HPS)0) goto bad;
    pps = PS_FROM_HPS(hps);
    if (pps->ulMagic != PS_MAGIC || pps->hpsSelf != hps) goto bad;
    return pps;

bad:
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return NULL;
}

/*!
   @brief Look up a PS and require it to have an associated DC
          (see ps.h for contract).

   @param[in] hps  Handle to look up.
   @return Pointer to PS with non-zero hdc, or NULL (error info set).
*/
PPS LookupPSDC(HPS hps)
{
    PPS pps = LookupPS(hps);
    if (pps == NULL) return NULL;
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return NULL;
    }
    return pps;
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

    pps = (PPS)AllocMemZero(sizeof(PS));
    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INSUFFICIENT_MEMORY),
                        SEI_NOBEEP);
        return (HPS)0;
    }

    pps->ulMagic           = PS_MAGIC;
    pps->hpsSelf           = HPS_FROM_PS(pps);
    pps->hdc               = (HDC)0;
    pps->ulDrawingMode     = DM_DRAW;
    pps->firstSegment      = NULL;
    pps->lastSegment       = NULL;
    pps->pOpenSegment      = NULL;
    pps->ulEditMode        = SEGEM_INSERT;

    /* Initial segment attribute defaults (OS/2 PM Programming Reference
       Vol.1, "GpiSetInitialSegmentAttrs - Remarks"):
         Not detectable, Visible, Chained, Not dynamic,
         Fast chaining, Propagate detectability, Propagate visibility.
       The previous value ATTR_CHAINED | ATTR_VISIBLE was (a) missing
       Fast chaining, Propagate detectability and Propagate visibility,
       and (b) written using ATTR_* as bit masks, which overlaps
       ATTR_CHAINED == 6 with ATTR_VISIBLE == 2. Use SEGATTR_BIT()
       instead, so every attribute gets its own bit. */
    pps->ulInitialSegAttrs = SEGATTR_BIT(ATTR_VISIBLE)
                           | SEGATTR_BIT(ATTR_CHAINED)
                           | SEGATTR_BIT(ATTR_FASTCHAIN)
                           | SEGATTR_BIT(ATTR_PROP_DETECTABLE)
                           | SEGATTR_BIT(ATTR_PROP_VISIBLE);

    /* All draw controls default to DCTL_ON. */
    pps->ulDrawControl     = DCTL_BIT(DCTL_ERASE)
                           | DCTL_BIT(DCTL_DISPLAY)
                           | DCTL_BIT(DCTL_BOUNDARY)
                           | DCTL_BIT(DCTL_DYNAMIC)
                           | DCTL_BIT(DCTL_CORRELATE);
    pps->ulStopDraw        = SDW_OFF;

    pps->sizlPS.cx         = psizlSize->cx;
    pps->sizlPS.cy         = psizlSize->cy;

    pps->pSaveStack        = NULL;
    pps->cSaves            = 0;
    pps->lNextSaveId       = 1;

    if (hdc != (HDC)0) {
        if (!GpiAssociate(HPS_FROM_PS(pps), hdc)) {
            pps->ulMagic = 0;
            DosFreeMem(pps);
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
    INIT_PS(hps, FALSE);

    pps->hdc = hdc;
    return TRUE;
}

/*!
   @brief Destroy a presentation space.

   Breaks the association (DC stays open; closing it is the caller's
   responsibility via DevCloseDC), frees all segments and the save
   stack, and frees the PS structure.

   @param[in] hps   Presentation space handle.

   @return TRUE on success, FALSE if hps is invalid.

   @see GpiCreatePS
*/
BOOL APIENTRY GpiDestroyPS(HPS hps)
{
    PPSSAVE psave;

    INIT_PS(hps, FALSE);

    pps->hdc = (HDC)0;
    FreeAllSegments(pps);

    /* Free the save stack. */
    psave = pps->pSaveStack;
    while (psave != NULL) {
        PPSSAVE next = psave->next;
        DosFreeMem(psave);
        psave = next;
    }

    pps->ulMagic = 0;
    pps->hpsSelf = (HPS)0;
    DosFreeMem(pps);

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
    INIT_PSDC(hps, (HDC)0);
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
    INIT_PS(hps, FALSE);

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
    INIT_PS(hps, GPI_ERROR);
    return (LONG)pps->ulDrawingMode;
}

/*!
   @brief Compute the actual drawing mode (see ps.h for contract).

   @param[in] pps  Presentation space (may be NULL).
   @return DM_DRAW, DM_RETAIN, or DM_DRAWANDRETAIN.
*/
ULONG ActualDrawingMode(PPS pps)
{
    if (pps == NULL)                return DM_DRAW;
    if (pps->pOpenSegment == NULL)  return DM_DRAW;
    if (SegmentGetAttr(pps->pOpenSegment, ATTR_CHAINED) == ATTR_ON)
        return pps->ulDrawingMode;
    /* Unchained segment, or malformed header (GPI_ERROR == 0 == ATTR_OFF
       due to the OS/2 toolkit constant collision) — treat as unchained. */
    return DM_RETAIN;
}

/*!
   @brief Clear the output device.

   Clears the output display of the device context associated with the
   presentation space to the reset color. Intended primarily for
   displays.

   @param[in] hps  Presentation space handle.

   @return TRUE on success, FALSE on error.

   @see GreErasePS
*/
BOOL APIENTRY GpiErase(HPS hps)
{
    INIT_PSDC(hps, FALSE);
    return GreErasePS(pps->hdc);
}

/*!
   @brief Reset the presentation space.

   Resets the PS to a state as if it were newly created, without
   deleting resources unless GRES_ALL is specified.

   @param[in] hps        Presentation space handle.
   @param[in] flOptions  GRES_ATTRS, GRES_SEGMENTS, or GRES_ALL.

   @return TRUE on success, FALSE on error.

   @see GpiCreatePS
*/
BOOL APIENTRY GpiResetPS(HPS hps, ULONG flOptions)
{
    INIT_PS(hps, FALSE);

    if (flOptions != GRES_ATTRS && flOptions != GRES_SEGMENTS &&
        flOptions != GRES_ALL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_RESET_OPTIONS),
                        SEI_NOBEEP);
        return FALSE;
    }

    /* GRES_SEGMENTS and GRES_ALL discard segments. */
    if (flOptions & (GRES_SEGMENTS | GRES_ALL)) {
        FreeAllSegments(pps);
    }

    /* GRES_ATTRS and GRES_ALL reset drawing mode, draw controls,
       edit mode, etc. */
    if (flOptions & (GRES_ATTRS | GRES_ALL)) {
        pps->ulDrawingMode = DM_DRAW;
        pps->ulDrawControl = DCTL_BIT(DCTL_ERASE)
                           | DCTL_BIT(DCTL_DISPLAY)
                           | DCTL_BIT(DCTL_BOUNDARY)
                           | DCTL_BIT(DCTL_DYNAMIC)
                           | DCTL_BIT(DCTL_CORRELATE);
        pps->ulStopDraw     = SDW_OFF;
        pps->ulEditMode    = SEGEM_INSERT;
    }

    /* GRES_ALL also resets initial segment attributes to defaults. */
    if (flOptions & GRES_ALL) {
        pps->ulInitialSegAttrs = SEGATTR_BIT(ATTR_VISIBLE)
                               | SEGATTR_BIT(ATTR_CHAINED)
                               | SEGATTR_BIT(ATTR_FASTCHAIN)
                               | SEGATTR_BIT(ATTR_PROP_DETECTABLE)
                               | SEGATTR_BIT(ATTR_PROP_VISIBLE);
    }

    return TRUE;
}

/*!
   @brief Set the presentation-space size.

   The current implementation accepts only flOptions == 0 and copies
   the requested size into PS.sizlPS. Changing PS format, type, or
   units is not supported yet (see PS_FORMAT/PS_TYPE/PS_UNITS in
   <pmgpi.h> for the full set of flOptions values defined by the
   OS/2 PM API).

   @param[in] hps        Presentation space handle.
   @param[in] psizlSize  New PS size.
   @param[in] flOptions  Reserved; must be 0.

   @return TRUE on success, FALSE on error.
*/
BOOL APIENTRY GpiSetPS(HPS hps, PSIZEL psizlSize, ULONG flOptions)
{
    INIT_PS(hps, FALSE);

    if (psizlSize == NULL || psizlSize->cx <= 0 || psizlSize->cy <= 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (flOptions != 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return FALSE;
    }

    pps->sizlPS.cx = psizlSize->cx;
    pps->sizlPS.cy = psizlSize->cy;
    return TRUE;
}

/*!
   @brief Query the presentation-space size and configuration.

   Writes the current PS size into *psizlSize and returns a bit mask
   describing the PS configuration. The current implementation reports
   pels units, default format, normal type, and the association bit;
   changing format/type/units is not supported yet.

   @param[in]  hps        Presentation space handle.
   @param[out] psizlSize  Receives the PS size.

   @return Bit mask (PU_PELS | GPIF_DEFAULT | GPIT_NORMAL
           [| GPIA_ASSOC if the PS has a DC]), or 0 on error.
*/
ULONG APIENTRY GpiQueryPS(HPS hps, PSIZEL psizlSize)
{
    ULONG result;

    INIT_PS(hps, 0);

    if (psizlSize == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return 0;
    }

    psizlSize->cx = pps->sizlPS.cx;
    psizlSize->cy = pps->sizlPS.cy;

    result = PU_PELS | GPIF_DEFAULT | GPIT_NORMAL;
    if (pps->hdc != (HDC)0)
        result |= GPIA_ASSOC;
    return result;
}

/*!
   @brief Save the current presentation-space state.

   Pushes a snapshot of PS-scoped state onto the save stack and returns
   a save identifier. IDs are assigned sequentially starting from 1.

   @param[in] hps  Presentation space handle.

   @return Save identifier (> 0), or 0 on error.

   @see GpiRestorePS
*/
LONG APIENTRY GpiSavePS(HPS hps)
{
    PPSSAVE psave;

    INIT_PS(hps, 0);

    psave = (PPSSAVE)AllocMemZero(sizeof(PSSAVE));
    if (psave == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INSUFFICIENT_MEMORY),
                        SEI_NOBEEP);
        return 0;
    }

    psave->ulDrawingMode     = pps->ulDrawingMode;
    psave->ulDrawControl     = pps->ulDrawControl;
    psave->ulStopDraw         = pps->ulStopDraw;
    psave->ulEditMode        = pps->ulEditMode;
    psave->ulInitialSegAttrs = pps->ulInitialSegAttrs;
    psave->sizlPS            = pps->sizlPS;

    psave->next = pps->pSaveStack;
    pps->pSaveStack = psave;
    pps->cSaves++;

    return pps->lNextSaveId++;
}

/*!
   @brief Restore a previously saved presentation-space state.

   Per OS/2 PM Programming Reference Vol.1, "GpiRestorePS": the saved
   data for lPSid and all later saved data are removed.

   @param[in] hps    Presentation space handle.
   @param[in] lPSid  Save identifier returned by GpiSavePS.

   @return TRUE on success, FALSE on error.

   @note There is no PMERR_INV_PSID in <pmerr.h>; a bad lPSid is
         reported as PMERR_INV_LENGTH_OR_COUNT, which is the closest
         match among the GPI error codes.

   @see GpiSavePS
*/
BOOL APIENTRY GpiRestorePS(HPS hps, LONG lPSid)
{
    PPSSAVE psave;
    PPSSAVE after;
    PPSSAVE s;
    LONG    idx;
    LONG    removed;

    INIT_PS(hps, FALSE);

    if (lPSid <= 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return FALSE;
    }

    /* Locate the save entry with the matching id. Ids are assigned
       sequentially from 1, and the stack is LIFO, so the top element
       has id lNextSaveId - 1, the next has lNextSaveId - 2, etc. */
    idx   = pps->lNextSaveId - 1;
    psave = pps->pSaveStack;
    while (psave != NULL && idx != lPSid) {
        psave = psave->next;
        idx--;
    }
    if (psave == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return FALSE;
    }

    /* Apply the saved state. */
    pps->ulDrawingMode     = psave->ulDrawingMode;
    pps->ulDrawControl     = psave->ulDrawControl;
    pps->ulStopDraw         = psave->ulStopDraw;
    pps->ulEditMode        = psave->ulEditMode;
    pps->ulInitialSegAttrs = psave->ulInitialSegAttrs;
    pps->sizlPS            = psave->sizlPS;

    /* Discard psave and every element above it (LIFO: from top of
       stack down to and including psave). Per "GpiRestorePS": "the
       saved data and all the later saved data are removed". */
    after   = psave->next;
    s       = pps->pSaveStack;
    removed = 0;
    while (s != NULL) {
        PPSSAVE n    = s->next;
        BOOL    stop = (s == psave);
        DosFreeMem(s);
        removed++;
        if (stop) break;
        s = n;
    }
    pps->pSaveStack = after;
    pps->cSaves -= removed;

    return TRUE;
}

/*!
   @brief Set a draw control.

   lControl is one of DCTL_ERASE, DCTL_DISPLAY, DCTL_BOUNDARY,
   DCTL_DYNAMIC, DCTL_CORRELATE; lValue is DCTL_ON or DCTL_OFF.
   Internally stored as one bit per control in PS.ulDrawControl
   (bit N = lControl - 1).

   @param[in] hps       Presentation space handle.
   @param[in] lControl  DCTL_* selector.
   @param[in] lValue    DCTL_ON or DCTL_OFF.

   @return TRUE on success, FALSE on error.

   @see GpiQueryDrawControl
*/
BOOL APIENTRY GpiSetDrawControl(HPS hps, LONG lControl, LONG lValue)
{
    INIT_PS(hps, FALSE);

    if (lControl < DCTL_ERASE || lControl > DCTL_CORRELATE) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_DRAW_CONTROL),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (lValue == DCTL_ON) {
        pps->ulDrawControl |=  DCTL_BIT(lControl);
    } else if (lValue == DCTL_OFF) {
        pps->ulDrawControl &= ~DCTL_BIT(lControl);
    } else {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_DRAW_VALUE),
                        SEI_NOBEEP);
        return FALSE;
    }
    return TRUE;
}

/*!
   @brief Query a draw control.

   @param[in] hps       Presentation space handle.
   @param[in] lControl  DCTL_* selector.

   @return DCTL_ON, DCTL_OFF, or DCTL_ERROR.

   @see GpiSetDrawControl
*/
LONG APIENTRY GpiQueryDrawControl(HPS hps, LONG lControl)
{
    INIT_PS(hps, DCTL_ERROR);

    if (lControl < DCTL_ERASE || lControl > DCTL_CORRELATE) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_DRAW_CONTROL),
                        SEI_NOBEEP);
        return DCTL_ERROR;
    }
    return (pps->ulDrawControl & DCTL_BIT(lControl)) ? DCTL_ON : DCTL_OFF;
}

/*!
   @brief Set the stop-draw condition.

   @param[in] hps     Presentation space handle.
   @param[in] lValue  SDW_OFF or SDW_ON.

   @return TRUE on success, FALSE on error.

   @see GpiQueryStopDraw
*/
BOOL APIENTRY GpiSetStopDraw(HPS hps, LONG lValue)
{
    INIT_PS(hps, FALSE);

    if (lValue != SDW_OFF && lValue != SDW_ON) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_STOP_DRAW_VALUE),
                        SEI_NOBEEP);
        return FALSE;
    }

    pps->ulStopDraw = (ULONG)lValue;
    return TRUE;
}

/*!
   @brief Query the stop-draw condition.

   @param[in] hps  Presentation space handle.

   @return SDW_OFF, SDW_ON, or SDW_ERROR.

   @see GpiSetStopDraw
*/
LONG APIENTRY GpiQueryStopDraw(HPS hps)
{
    INIT_PS(hps, SDW_ERROR);
    return (LONG)pps->ulStopDraw;
}
