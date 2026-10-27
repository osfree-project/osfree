/*!
   @file
   @ingroup pm
   @brief GPI segment API (32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   Fact-check sources (OS/2 Presentation Manager Programming Reference,
   Volume 1) used below:
     - GpiOpenSegment         : lSegId > 0, no already-open segment.
     - GpiDrawChain           : lowest -> highest priority order.
     - GpiDrawFrom            : idFirstSegment > 0, idLastSegment >= idFirstSegment.
     - GpiDrawDynamics        : dynamic segments drawn with XOR mix.
     - GpiRemoveDynamics      : XOR mix, optionally restricted by range.
     - GpiCallSegmentMatrix   : does not modify the current model transform.
     - GpiSetSegmentPriority  : LOWER_PRI / HIGHER_PRI, 0 = chain end.
     - GpiQuerySegmentPriority: same LOWER_PRI / HIGHER_PRI semantics.
     - GpiQuerySegmentNames   : lMax is count of identifiers to return.

   Attribute storage: ulAttrs is a bit set. Each ATTR_* identifier from
   <pmgpi.h> selects one bit via SEGATTR_BIT(a). ATTR_* are identifiers
   (1..11), NOT bit masks — see ps.h for details.
*/

/*!
   @brief Open a segment for definition or editing.

   @param[in] hps     Presentation space handle.
   @param[in] lSegId  Segment identifier; must be > 0.

   @return TRUE on success, FALSE on error.

   @see GpiCloseSegment
*/
BOOL APIENTRY GpiOpenSegment(HPS hps, LONG lSegId)
{
    PPS pps = LookupPS(hps);
    POINTL ptl;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->ulDrawingMode == DM_DRAW) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_IN_DRAW_MODE),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (lSegId <= 0) {
        /* "GpiOpenSegment - Parameters": lSegId must be greater than 0. */
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_NAME),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (pps->pOpenSegment != NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_ALREADY_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (FindSegment(pps, lSegId) != NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_DUP_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }

    pps->pOpenSegment = CreateSegment(pps, lSegId, pps->ulInitialSegAttrs);
    if (pps->pOpenSegment == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INSUFFICIENT_MEMORY),
                        SEI_NOBEEP);
        return FALSE;
    }

    if (GreGetCurrentPosition(pps->hdc, &ptl))
        RecordElement(pps, OCODE_GSCP, 1, sizeof(POINTL), &ptl);
    return TRUE;
}

/*!
   @brief Close the currently open segment.

   @param[in] hps  Presentation space handle.

   @return TRUE on success, FALSE on error.
*/
BOOL APIENTRY GpiCloseSegment(HPS hps)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->pOpenSegment == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }
    pps->pOpenSegment = NULL;
    return TRUE;
}

/*!
   @brief Delete a single segment.

   @param[in] hps     Presentation space handle.
   @param[in] lSegId  Segment identifier.

   @return TRUE on success, FALSE on error.
*/
BOOL APIENTRY GpiDeleteSegment(HPS hps, LONG lSegId)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->pOpenSegment != NULL && pps->pOpenSegment->lSegId == lSegId) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_IS_CURRENT),
                        SEI_NOBEEP);
        return FALSE;
    }

    seg = FindSegment(pps, lSegId);
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                        SEI_NOBEEP);
        return FALSE;
    }

    UnlinkSegment(pps, seg);
    FreeSegmentElements(seg);
    DosFreeMem(seg);
    return TRUE;
}

/*!
   @brief Delete a range of segments.

   @param[in] hps     Presentation space handle.
   @param[in] lFirst  First segment id in range.
   @param[in] lLast   Last segment id in range.

   @return TRUE on success, FALSE on error.
*/
BOOL APIENTRY GpiDeleteSegments(HPS hps, LONG lFirst, LONG lLast)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg, next;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    seg = pps->firstSegment;
    while (seg != NULL) {
        next = seg->next;
        if (seg->lSegId >= lFirst && seg->lSegId <= lLast) {
            if (pps->pOpenSegment == seg) {
                WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_IS_CURRENT),
                                SEI_NOBEEP);
                return FALSE;
            }
            UnlinkSegment(pps, seg);
            FreeSegmentElements(seg);
            DosFreeMem(seg);
        }
        seg = next;
    }
    return TRUE;
}

/*!
   @brief Draw a single segment.

   @param[in] hps     Presentation space handle.
   @param[in] lSegId  Segment identifier.

   @return TRUE on success, FALSE on error.

   @see GpiDrawChain, GpiDrawFrom
*/
BOOL APIENTRY GpiDrawSegment(HPS hps, LONG lSegId)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->pOpenSegment != NULL && pps->pOpenSegment->lSegId == lSegId) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_IS_CURRENT),
                        SEI_NOBEEP);
        return FALSE;
    }
    seg = FindSegment(pps, lSegId);
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                        SEI_NOBEEP);
        return FALSE;
    }
    return ReplaySegment(pps, seg);
}

/*!
   @brief Draw the entire segment chain in priority order.

   The chain is stored lowest-priority first, so forward iteration
   draws from lowest to highest priority as required by
   OS/2 PM Programming Reference Vol.1, "GpiDrawChain - Remarks".

   @param[in] hps  Presentation space handle.

   @return TRUE on success, FALSE on error.

   @note  Fast-chaining ("flush attribute state between segments when
          ATTR_FASTCHAIN is off") is not yet implemented; the current
          implementation replays each segment with whatever attribute
          state the driver currently holds. See
          "GpiDrawChain - Remarks" for the full semantics.
*/
BOOL APIENTRY GpiDrawChain(HPS hps)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return FALSE;
    }

    for (seg = pps->firstSegment; seg != NULL; seg = seg->next) {
        if ((seg->ulAttrs & SEGATTR_BIT(ATTR_CHAINED)) == 0) continue;
        if (seg->ulAttrs & SEGATTR_BIT(ATTR_DYNAMIC))        continue;
        if ((seg->ulAttrs & SEGATTR_BIT(ATTR_VISIBLE)) == 0) continue;
        if (!ReplaySegment(pps, seg)) return FALSE;
    }
    return TRUE;
}

/*!
   @brief Draw a contiguous range of the segment chain.

   @param[in] hps          Presentation space handle.
   @param[in] lFirstSegId  First segment id (must be > 0).
   @param[in] lLastSegId   Last segment id (must be >= lFirstSegId).

   @return TRUE on success, FALSE on error.

   @see GpiDrawChain, GpiDrawSegment
*/
BOOL APIENTRY GpiDrawFrom(HPS hps, LONG lFirstSegId, LONG lLastSegId)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return FALSE;
    }
    if (lFirstSegId <= 0) {
        /* "GpiDrawFrom - Parameters": idFirstSegment must be > 0. */
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_NAME),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (lLastSegId < lFirstSegId) {
        /* "GpiDrawFrom - Parameters": idLastSegment must be greater
           than or equal to idFirstSegment. */
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_NAME),
                        SEI_NOBEEP);
        return FALSE;
    }

    for (seg = pps->firstSegment; seg != NULL; seg = seg->next) {
        if (seg->lSegId < lFirstSegId) continue;
        if (seg->lSegId > lLastSegId)   break;
        if ((seg->ulAttrs & SEGATTR_BIT(ATTR_CHAINED)) == 0) continue;
        if (seg->ulAttrs & SEGATTR_BIT(ATTR_DYNAMIC))        continue;
        if ((seg->ulAttrs & SEGATTR_BIT(ATTR_VISIBLE)) == 0) continue;
        if (!ReplaySegment(pps, seg)) return FALSE;
    }
    return TRUE;
}

/*!
   @brief Draw dynamic segments using XOR mix.

   Per OS/2 PM Programming Reference Vol.1, "GpiDrawDynamics -
   Remarks", dynamic segments are drawn with mix mode FM_XOR and
   background mix BM_LEAVEALONE, so that a subsequent GpiDrawDynamics
   or GpiRemoveDynamics call erases them.

   @param[in] hps  Presentation space handle.

   @return TRUE on success, FALSE on error.

   @note  FM_XOR / BM_LEAVEALONE are set via GreSetMix / GreSetBackMix.
          These are not mandatory DDI entry points; if the driver
          ignores them, replay still happens but XOR erase will not
          work. This is a known limitation.
*/
BOOL APIENTRY GpiDrawDynamics(HPS hps)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;
    HDC      hdc;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    hdc = pps->hdc;
    if (hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return FALSE;
    }

    GreSetMix(hdc, FM_XOR);
    GreSetBackMix(hdc, BM_LEAVEALONE);

    for (seg = pps->firstSegment; seg != NULL; seg = seg->next) {
        if ((seg->ulAttrs & SEGATTR_BIT(ATTR_DYNAMIC)) == 0) continue;
        if ((seg->ulAttrs & SEGATTR_BIT(ATTR_CHAINED)) == 0) continue;
        if ((seg->ulAttrs & SEGATTR_BIT(ATTR_VISIBLE)) == 0) continue;
        if (!ReplaySegment(pps, seg)) return FALSE;
    }
    return TRUE;
}

/*!
   @brief Remove dynamic segments from the screen using XOR mix.

   Per OS/2 PM Programming Reference Vol.1, "GpiRemoveDynamics -
   Remarks", the call may set a range for a subsequent
   GpiDrawDynamics call. This implementation draws the segments in
   the specified range with XOR; the effect on the visible image is
   to erase them.

   @param[in] hps          Presentation space handle.
   @param[in] lFirstSegId  First dynamic segment to remove.
   @param[in] lLastSegId   Last dynamic segment to remove.

   @return TRUE on success, FALSE on error.

   @see GpiDrawDynamics
*/
BOOL APIENTRY GpiRemoveDynamics(HPS hps, LONG lFirstSegId, LONG lLastSegId)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;
    HDC      hdc;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    hdc = pps->hdc;
    if (hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return FALSE;
    }

    GreSetMix(hdc, FM_XOR);
    GreSetBackMix(hdc, BM_LEAVEALONE);

    for (seg = pps->firstSegment; seg != NULL; seg = seg->next) {
        if (seg->lSegId < lFirstSegId) continue;
        if (seg->lSegId > lLastSegId)   break;
        if ((seg->ulAttrs & SEGATTR_BIT(ATTR_DYNAMIC)) == 0) continue;
        if ((seg->ulAttrs & SEGATTR_BIT(ATTR_CHAINED)) == 0) continue;
        if (!ReplaySegment(pps, seg)) return FALSE;
    }
    return TRUE;
}

/*!
   @brief Call a segment with an instance transformation.

   Combines the instance transform with the current model transform,
   draws the segment, then restores the model transform. The current
   model transform is NOT modified permanently (OS/2 PM Programming
   Reference Vol.1, "GpiCallSegmentMatrix - Remarks").

   @param[in] hps        Presentation space handle.
   @param[in] lSegId     Segment identifier (must be > 0).
   @param[in] cElements  Number of matrix elements used (0..9).
   @param[in] pmatlf     Instance transformation matrix.
   @param[in] lType      TRANSFORM_ADD, TRANSFORM_PREEMPT, or
                         TRANSFORM_REPLACE.

   @return GPI_OK / GPI_HITS on success, GPI_ERROR on failure.

   @note  Because segment-level transforms
          (GpiSetSegmentTransformMatrix) are not implemented, the
          distinction between TRANSFORM_ADD / TRANSFORM_PREEMPT /
          TRANSFORM_REPLACE collapses to REPLACE: the instance matrix
          becomes the effective model transform for the duration of
          the call. This is a documented limitation, not a silent
          bug: "GpiCallSegmentMatrix - Remarks" describes combining
          the instance transform with the SEGMENT transform, which
          does not exist here.
*/
LONG APIENTRY GpiCallSegmentMatrix(HPS hps, LONG lSegId, LONG cElements,
                                   PMATRIXLF pmatlf, LONG lType)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;
    XFORM    oldXform;
    XFORM    newXform;
    LONG     rc;

    (void)cElements;
    (void)lType;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lSegId <= 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_NAME),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pmatlf == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_MATRIX_ELEMENT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    seg = FindSegment(pps, lSegId);
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->pOpenSegment != NULL && pps->pOpenSegment->lSegId == lSegId) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_IS_CURRENT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    if (!GreGetModelXform(pps->hdc, &oldXform)) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_MATRIX_ELEMENT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    memset(&newXform, 0, sizeof(newXform));
    newXform.fxM11 = pmatlf->fxM11;
    newXform.fxM12 = pmatlf->fxM12;
    newXform.lM13  = pmatlf->lM13;
    newXform.fxM21 = pmatlf->fxM21;
    newXform.fxM22 = pmatlf->fxM22;
    newXform.lM23  = pmatlf->lM23;
    newXform.lM31  = pmatlf->lM31;
    newXform.lM32  = pmatlf->lM32;
    newXform.lM33  = pmatlf->lM33;

    if (!GreSetModelXform(pps->hdc, &newXform, TRANSFORM_REPLACE)) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_MATRIX_ELEMENT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    rc = ReplaySegment(pps, seg) ? GPI_OK : GPI_ERROR;

    GreSetModelXform(pps->hdc, &oldXform, TRANSFORM_REPLACE);

    return rc;
}

/*!
   @brief Set or query the priority of a segment.

   @param[in] hps        Presentation space handle.
   @param[in] lSegId     Segment whose priority changes (must be > 0).
   @param[in] lRefSegId  Reference segment (0 = chain end).
   @param[in] lOrder     LOWER_PRI or HIGHER_PRI.

   @return TRUE on success, FALSE on error.

   @note  The specified segment may already be chained or unchained.
          An unchained segment is added to the chain at the specified
          position. See OS/2 PM Programming Reference Vol.1,
          "GpiSetSegmentPriority - Remarks".
*/
BOOL APIENTRY GpiSetSegmentPriority(HPS hps, LONG lSegId,
                                    LONG lRefSegId, LONG lOrder)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (lSegId <= 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_NAME),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (lSegId == lRefSegId) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_NAME),
                        SEI_NOBEEP);
        return FALSE;
    }
    seg = FindSegment(pps, lSegId);
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                        SEI_NOBEEP);
        return FALSE;
    }

    UnlinkSegment(pps, seg);
    return InsertSegmentByPriority(pps, seg, lRefSegId, lOrder);
}

/*!
   @brief Query the next segment at a given priority relative to a
          reference segment.

   @param[in] hps        Presentation space handle.
   @param[in] lRefSegId  Reference segment (0 = query chain end).
   @param[in] lOrder     LOWER_PRI or HIGHER_PRI.

   @return Segment identifier, or 0 if none.

   @note  If lRefSegId is 0 and lOrder is HIGHER_PRI, returns the
          lowest-priority segment. If lRefSegId is 0 and lOrder is
          LOWER_PRI, returns the highest-priority segment. See
          OS/2 PM Programming Reference Vol.1,
          "GpiQuerySegmentPriority - Returns".
*/
LONG APIENTRY GpiQuerySegmentPriority(HPS hps, LONG lRefSegId, LONG lOrder)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return 0;
    }

    if (lRefSegId == 0) {
        if (lOrder == HIGHER_PRI)
            return (pps->firstSegment != NULL) ? pps->firstSegment->lSegId : 0;
        else
            return (pps->lastSegment != NULL) ? pps->lastSegment->lSegId : 0;
    }

    seg = FindSegment(pps, lRefSegId);
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                        SEI_NOBEEP);
        return 0;
    }

    if (lOrder == HIGHER_PRI)
        return (seg->next != NULL) ? seg->next->lSegId : 0;
    else
        return (seg->prev != NULL) ? seg->prev->lSegId : 0;
}

/*!
   @brief Set one attribute for a segment.

   @param[in] hps        Presentation space handle.
   @param[in] lSegId     Segment identifier (must be > 0).
   @param[in] lAttr      ATTR_* identifier.
   @param[in] lAttrValue ATTR_ON or ATTR_OFF.

   @return TRUE on success, FALSE on error.

   @note  Segment id 0 is reserved; its attributes cannot be changed.
          See OS/2 PM Programming Reference Vol.1,
          "GpiSetSegmentAttrs - Parameters".
*/
BOOL APIENTRY GpiSetSegmentAttrs(HPS hps, LONG lSegId,
                                 LONG lAttr, LONG lAttrValue)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (lSegId <= 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_NAME),
                        SEI_NOBEEP);
        return FALSE;
    }
    seg = FindSegment(pps, lSegId);
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                        SEI_NOBEEP);
        return FALSE;
    }
    switch (lAttr) {
    case ATTR_DETECTABLE:
    case ATTR_VISIBLE:
    case ATTR_CHAINED:
    case ATTR_DYNAMIC:
    case ATTR_FASTCHAIN:
    case ATTR_PROP_DETECTABLE:
    case ATTR_PROP_VISIBLE:
        if (lAttrValue == ATTR_ON)
            seg->ulAttrs |=  SEGATTR_BIT(lAttr);
        else if (lAttrValue == ATTR_OFF)
            seg->ulAttrs &= ~SEGATTR_BIT(lAttr);
        else {
            WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_ATTR_VALUE),
                            SEI_NOBEEP);
            return FALSE;
        }
        return TRUE;
    default:
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_ATTR),
                        SEI_NOBEEP);
        return FALSE;
    }
}

/*!
   @brief Query one attribute of a segment.

   @param[in] hps     Presentation space handle.
   @param[in] lSegId  Segment identifier (must be > 0).
   @param[in] lAttr   ATTR_* identifier.

   @return ATTR_ON, ATTR_OFF, or GPI_ERROR.

   @note  All seven attributes are supported:
          ATTR_DETECTABLE, ATTR_VISIBLE, ATTR_CHAINED, ATTR_DYNAMIC,
          ATTR_FASTCHAIN, ATTR_PROP_DETECTABLE, ATTR_PROP_VISIBLE.
          See OS/2 PM Programming Reference Vol.1,
          "GpiQuerySegmentAttrs - Parameters".
*/
LONG APIENTRY GpiQuerySegmentAttrs(HPS hps, LONG lSegId, LONG lAttr)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lSegId <= 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_NAME),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    seg = FindSegment(pps, lSegId);
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    switch (lAttr) {
    case ATTR_DETECTABLE:
    case ATTR_VISIBLE:
    case ATTR_CHAINED:
    case ATTR_DYNAMIC:
    case ATTR_FASTCHAIN:
    case ATTR_PROP_DETECTABLE:
    case ATTR_PROP_VISIBLE:
        return (seg->ulAttrs & SEGATTR_BIT(lAttr)) ? ATTR_ON : ATTR_OFF;
    default:
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_ATTR),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
}

/*!
   @brief Set a default (initial) segment attribute.

   @param[in] hps        Presentation space handle.
   @param[in] lAttr      ATTR_* identifier.
   @param[in] lAttrValue ATTR_ON or ATTR_OFF.

   @return TRUE on success, FALSE on error.

   @note  Default initial values are: Not detectable, Visible,
          Chained, Not dynamic, Fast chaining, Propagate
          detectability, Propagate visibility. See
          OS/2 PM Programming Reference Vol.1,
          "GpiSetInitialSegmentAttrs - Remarks".
*/
BOOL APIENTRY GpiSetInitialSegmentAttrs(HPS hps, LONG lAttr, LONG lAttrValue)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    switch (lAttr) {
    case ATTR_DETECTABLE:
    case ATTR_VISIBLE:
    case ATTR_CHAINED:
    case ATTR_DYNAMIC:
    case ATTR_FASTCHAIN:
    case ATTR_PROP_DETECTABLE:
    case ATTR_PROP_VISIBLE:
        if (lAttrValue == ATTR_ON)
            pps->ulInitialSegAttrs |=  SEGATTR_BIT(lAttr);
        else if (lAttrValue == ATTR_OFF)
            pps->ulInitialSegAttrs &= ~SEGATTR_BIT(lAttr);
        else {
            WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_ATTR_VALUE),
                            SEI_NOBEEP);
            return FALSE;
        }
        return TRUE;
    default:
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_ATTR),
                        SEI_NOBEEP);
        return FALSE;
    }
}

/*!
   @brief Query a default (initial) segment attribute.

   @param[in] hps   Presentation space handle.
   @param[in] lAttr ATTR_* identifier.

   @return ATTR_ON, ATTR_OFF, or GPI_ERROR.
*/
LONG APIENTRY GpiQueryInitialSegmentAttrs(HPS hps, LONG lAttr)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    switch (lAttr) {
    case ATTR_DETECTABLE:
    case ATTR_VISIBLE:
    case ATTR_CHAINED:
    case ATTR_DYNAMIC:
    case ATTR_FASTCHAIN:
    case ATTR_PROP_DETECTABLE:
    case ATTR_PROP_VISIBLE:
        return (pps->ulInitialSegAttrs & SEGATTR_BIT(lAttr)) ? ATTR_ON : ATTR_OFF;
    default:
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_ATTR),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
}

/*!
   @brief Query segment names in a range.

   @param[in]  hps       Presentation space handle.
   @param[in]  lFirst    First segment id in range.
   @param[in]  lLast     Last segment id in range.
   @param[in]  lMax      Maximum number of ids to return (must be > 0).
   @param[out] alSegIds  Receives segment ids (must be non-NULL).

   @return Number of ids written, or GPI_ERROR on error.

   @note  Per OS/2 PM Programming Reference Vol.1,
          "GpiQuerySegmentNames - Parameters", lMax is the maximum
          number of identifiers to be returned; it must be greater
          than zero.
*/
LONG APIENTRY GpiQuerySegmentNames(HPS hps, LONG lFirst, LONG lLast,
                                   LONG lMax, PLONG alSegIds)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;
    LONG     n = 0;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lMax <= 0 || alSegIds == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    for (seg = pps->firstSegment; seg != NULL && n < lMax; seg = seg->next) {
        if (seg->lSegId >= lFirst && seg->lSegId <= lLast)
            alSegIds[n++] = seg->lSegId;
    }
    return n;
}

/*!
   @brief Set the edit mode.

   @param[in] hps    Presentation space handle.
   @param[in] lMode  SEGEM_INSERT or SEGEM_REPLACE.

   @return TRUE on success, FALSE on error.
*/
BOOL APIENTRY GpiSetEditMode(HPS hps, LONG lMode)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (lMode != SEGEM_INSERT && lMode != SEGEM_REPLACE) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_EDIT_MODE),
                        SEI_NOBEEP);
        return FALSE;
    }
    pps->ulEditMode = (ULONG)lMode;
    return TRUE;
}

/*!
   @brief Query the edit mode.

   @param[in] hps  Presentation space handle.

   @return Current edit mode, or GPI_ERROR on invalid hps.
*/
LONG APIENTRY GpiQueryEditMode(HPS hps)
{
    PPS pps = LookupPS(hps);
    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    return (LONG)pps->ulEditMode;
}

/*!
   @brief Set the element pointer of the currently open segment.

   @param[in] hps       Presentation space handle.
   @param[in] lElement  Zero-based element index (0 .. lElementCount).

   @return TRUE on success, FALSE on error.
*/
BOOL APIENTRY GpiSetElementPointer(HPS hps, LONG lElement)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;
    PELEMENT el;
    LONG     i;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    seg = pps->pOpenSegment;
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (lElement < 0 || lElement > seg->lElementCount) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_ELEMENT_POINTER),
                        SEI_NOBEEP);
        return FALSE;
    }
    el = seg->firstElement;
    for (i = 0; i < lElement && el != NULL; i++) el = el->next;
    seg->currentElement = el;
    return TRUE;
}

/*!
   @brief Query the element pointer.

   @param[in] hps  Presentation space handle.

   @return Zero-based element index, or GPI_ERROR on invalid hps.
*/
LONG APIENTRY GpiQueryElementPointer(HPS hps)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;
    PELEMENT el;
    LONG     i = 0;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    seg = pps->pOpenSegment;
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    for (el = seg->firstElement; el != NULL && el != seg->currentElement;
         el = el->next)
        i++;
    return i;
}

/*!
   @brief Offset the element pointer.

   @param[in] hps      Presentation space handle.
   @param[in] lOffset  Signed offset.

   @return TRUE on success, FALSE on error.
*/
BOOL APIENTRY GpiOffsetElementPointer(HPS hps, LONG lOffset)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->pOpenSegment == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }
    return GpiSetElementPointer(hps, GpiQueryElementPointer(hps) + lOffset);
}

/*!
   @brief Place a label element in the currently open segment.

   @param[in] hps     Presentation space handle.
   @param[in] lLabel  Label value.

   @return TRUE on success, FALSE on error.
*/
BOOL APIENTRY GpiLabel(HPS hps, LONG lLabel)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->pOpenSegment == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }
    return RecordElement(pps, OCODE_GLABL, 1, sizeof(LONG), &lLabel);
}

/*!
   @brief Place a user-defined element in the currently open segment.

   @param[in] hps      Presentation space handle.
   @param[in] lType    User element type (> 0xFF).
   @param[in] pszDesc  Element descriptor (currently ignored).
   @param[in] lLength  Data length in bytes.
   @param[in] pbData   Data payload.

   @return TRUE on success, FALSE on error.
*/
BOOL APIENTRY GpiElement(HPS hps, LONG lType, PSZ pszDesc,
                         LONG lLength, PBYTE pbData)
{
    PPS pps = LookupPS(hps);

    (void)pszDesc;
    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->pOpenSegment == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (lLength < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return FALSE;
    }
    return RecordElement(pps, lType, lLength, lLength, pbData);
}
