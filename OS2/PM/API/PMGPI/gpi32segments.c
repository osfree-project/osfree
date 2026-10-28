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

   Segment name and attributes live in the GBSEG header (OCODE_GBSEG)
   at the head of each segment's element list. Access via
   SegmentGetName / SegmentGetAttr / SegmentSetAttr (see segments.c).

   Functions are grouped by DC requirement:
     - editing functions (INIT_PS)      : GpiCloseSegment, GpiDeleteSegment,
       GpiDeleteSegments, GpiSetSegmentAttrs, GpiQuerySegmentAttrs,
       GpiSetInitialSegmentAttrs, GpiQueryInitialSegmentAttrs,
       GpiQuerySegmentNames, GpiSetEditMode, GpiQueryEditMode,
       GpiSetElementPointer, GpiQueryElementPointer,
       GpiOffsetElementPointer, GpiLabel, GpiElement,
       GpiSetSegmentPriority, GpiQuerySegmentPriority;
     - drawing functions (INIT_PSDC)    : GpiOpenSegment, GpiDrawSegment,
       GpiDrawChain, GpiDrawFrom, GpiDrawDynamics, GpiRemoveDynamics,
       GpiCallSegmentMatrix.
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
    POINTL ptl;

    INIT_PSDC(hps, FALSE);

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
    INIT_PS(hps, FALSE);

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
    PSEGMENT seg;

    INIT_PS(hps, FALSE);

    if (pps->pOpenSegment != NULL &&
        SegmentGetName(pps->pOpenSegment) == lSegId) {
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
    PSEGMENT seg, next;

    INIT_PS(hps, FALSE);

    seg = pps->firstSegment;
    while (seg != NULL) {
        LONG lSegId = SegmentGetName(seg);
        next = seg->next;
        if (lSegId >= lFirst && lSegId <= lLast) {
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
    PSEGMENT seg;

    INIT_PSDC(hps, FALSE);

    if (pps->pOpenSegment != NULL &&
        SegmentGetName(pps->pOpenSegment) == lSegId) {
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
    PSEGMENT seg;

    INIT_PSDC(hps, FALSE);

    for (seg = pps->firstSegment; seg != NULL; seg = seg->next) {
        if (SegmentGetAttr(seg, ATTR_CHAINED) != ATTR_ON) continue;
        if (SegmentGetAttr(seg, ATTR_DYNAMIC) == ATTR_ON) continue;
        if (SegmentGetAttr(seg, ATTR_VISIBLE) != ATTR_ON) continue;
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
    PSEGMENT seg;

    INIT_PSDC(hps, FALSE);

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
        LONG lSegId = SegmentGetName(seg);
        if (lSegId < lFirstSegId) continue;
        if (lSegId > lLastSegId)   break;
        if (SegmentGetAttr(seg, ATTR_CHAINED) != ATTR_ON) continue;
        if (SegmentGetAttr(seg, ATTR_DYNAMIC) == ATTR_ON) continue;
        if (SegmentGetAttr(seg, ATTR_VISIBLE) != ATTR_ON) continue;
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

   @note  FM_XOR / BM_LEAVEALONE are set via GreSetAttributes with
          LBB/CBB/MBB/ABB/IBB bundle structures from <pmgpi.h>. This
          is the DDI way to set mix attributes; the current driver
          may or may not honour them. If it ignores them, replay
          still happens but XOR erase will not work. Known
          limitation.
*/
BOOL APIENTRY GpiDrawDynamics(HPS hps)
{
    PSEGMENT seg;
    HDC      hdc;
    LINEBUNDLE   lb;
    CHARBUNDLE   cb;
    MARKERBUNDLE mb;
    AREABUNDLE   ab;
    IMAGEBUNDLE  ib;

    INIT_PSDC(hps, FALSE);
    hdc = pps->hdc;

    /* Set FM_XOR / BM_LEAVEALONE through GreSetAttributes. GreSetMix
       and GreSetBackMix are not part of the DDI in pmddim.h. */
    memset(&lb, 0, sizeof(lb));
    memset(&cb, 0, sizeof(cb));
    memset(&mb, 0, sizeof(mb));
    memset(&ab, 0, sizeof(ab));
    memset(&ib, 0, sizeof(ib));

    lb.usMixMode = FM_XOR; lb.usBackMixMode = BM_LEAVEALONE;
    cb.usMixMode = FM_XOR; cb.usBackMixMode = BM_LEAVEALONE;
    mb.usMixMode = FM_XOR; mb.usBackMixMode = BM_LEAVEALONE;
    ab.usMixMode = FM_XOR; ab.usBackMixMode = BM_LEAVEALONE;
    ib.usMixMode = FM_XOR; ib.usBackMixMode = BM_LEAVEALONE;

    GreSetAttributes(hdc, LBB_MIX_MODE | LBB_BACK_MIX_MODE, PRIM_LINE,   0, &lb);
    GreSetAttributes(hdc, CBB_MIX_MODE | CBB_BACK_MIX_MODE, PRIM_CHAR,   0, &cb);
    GreSetAttributes(hdc, MBB_MIX_MODE | MBB_BACK_MIX_MODE, PRIM_MARKER, 0, &mb);
    GreSetAttributes(hdc, ABB_MIX_MODE | ABB_BACK_MIX_MODE, PRIM_AREA,   0, &ab);
    GreSetAttributes(hdc, IBB_MIX_MODE | IBB_BACK_MIX_MODE, PRIM_IMAGE,  0, &ib);

    for (seg = pps->firstSegment; seg != NULL; seg = seg->next) {
        if (SegmentGetAttr(seg, ATTR_DYNAMIC) != ATTR_ON) continue;
        if (SegmentGetAttr(seg, ATTR_CHAINED) != ATTR_ON) continue;
        if (SegmentGetAttr(seg, ATTR_VISIBLE) != ATTR_ON) continue;
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
    PSEGMENT seg;
    HDC      hdc;
    LINEBUNDLE   lb;
    CHARBUNDLE   cb;
    MARKERBUNDLE mb;
    AREABUNDLE   ab;
    IMAGEBUNDLE  ib;

    INIT_PSDC(hps, FALSE);
    hdc = pps->hdc;

    /* Same mix handling as GpiDrawDynamics: set FM_XOR / BM_LEAVEALONE
       through GreSetAttributes (GreSetMix/GreSetBackMix are not part
       of the DDI in pmddim.h). */
    memset(&lb, 0, sizeof(lb));
    memset(&cb, 0, sizeof(cb));
    memset(&mb, 0, sizeof(mb));
    memset(&ab, 0, sizeof(ab));
    memset(&ib, 0, sizeof(ib));

    lb.usMixMode = FM_XOR; lb.usBackMixMode = BM_LEAVEALONE;
    cb.usMixMode = FM_XOR; cb.usBackMixMode = BM_LEAVEALONE;
    mb.usMixMode = FM_XOR; mb.usBackMixMode = BM_LEAVEALONE;
    ab.usMixMode = FM_XOR; ab.usBackMixMode = BM_LEAVEALONE;
    ib.usMixMode = FM_XOR; ib.usBackMixMode = BM_LEAVEALONE;

    GreSetAttributes(hdc, LBB_MIX_MODE | LBB_BACK_MIX_MODE, PRIM_LINE,   0, &lb);
    GreSetAttributes(hdc, CBB_MIX_MODE | CBB_BACK_MIX_MODE, PRIM_CHAR,   0, &cb);
    GreSetAttributes(hdc, MBB_MIX_MODE | MBB_BACK_MIX_MODE, PRIM_MARKER, 0, &mb);
    GreSetAttributes(hdc, ABB_MIX_MODE | ABB_BACK_MIX_MODE, PRIM_AREA,   0, &ab);
    GreSetAttributes(hdc, IBB_MIX_MODE | IBB_BACK_MIX_MODE, PRIM_IMAGE,  0, &ib);

    for (seg = pps->firstSegment; seg != NULL; seg = seg->next) {
        LONG lSegId = SegmentGetName(seg);
        if (lSegId < lFirstSegId) continue;
        if (lSegId > lLastSegId)   break;
        if (SegmentGetAttr(seg, ATTR_DYNAMIC) != ATTR_ON) continue;
        if (SegmentGetAttr(seg, ATTR_CHAINED) != ATTR_ON) continue;
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
    PSEGMENT seg;
    XFORM    oldXform;
    XFORM    newXform;
    LONG     rc;

    INIT_PSDC(hps, GPI_ERROR);

    (void)cElements;
    (void)lType;

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
    if (pps->pOpenSegment != NULL &&
        SegmentGetName(pps->pOpenSegment) == lSegId) {
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
    PSEGMENT seg;

    INIT_PS(hps, FALSE);

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
    PSEGMENT seg;

    INIT_PS(hps, 0);

    if (lRefSegId == 0) {
        if (lOrder == HIGHER_PRI)
            return (pps->firstSegment != NULL)
                   ? SegmentGetName(pps->firstSegment) : 0;
        else
            return (pps->lastSegment != NULL)
                   ? SegmentGetName(pps->lastSegment) : 0;
    }

    seg = FindSegment(pps, lRefSegId);
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                        SEI_NOBEEP);
        return 0;
    }

    if (lOrder == HIGHER_PRI)
        return (seg->next != NULL) ? SegmentGetName(seg->next) : 0;
    else
        return (seg->prev != NULL) ? SegmentGetName(seg->prev) : 0;
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
          "GpiSetSegmentAttrs - Parameters". The attribute is written
          into the GBSEG header (see SegmentSetAttr; IBM SC31-6804,
          pp. 102-103 for FLAG1, AFPC-0008-03, p. 75 for FLAG2).
*/
BOOL APIENTRY GpiSetSegmentAttrs(HPS hps, LONG lSegId,
                                 LONG lAttr, LONG lAttrValue)
{
    PSEGMENT seg;

    INIT_PS(hps, FALSE);

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
    if (!SegmentSetAttr(seg, lAttr, lAttrValue)) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_ATTR),
                        SEI_NOBEEP);
        return FALSE;
    }
    return TRUE;
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
          "GpiQuerySegmentAttrs - Parameters". Values are read from
          the GBSEG header (see SegmentGetAttr).
*/
LONG APIENTRY GpiQuerySegmentAttrs(HPS hps, LONG lSegId, LONG lAttr)
{
    PSEGMENT seg;

    INIT_PS(hps, GPI_ERROR);

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
    return SegmentGetAttr(seg, lAttr);
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
    INIT_PS(hps, FALSE);

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
    INIT_PS(hps, GPI_ERROR);

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
    PSEGMENT seg;
    LONG     n = 0;

    INIT_PS(hps, GPI_ERROR);

    if (lMax <= 0 || alSegIds == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    for (seg = pps->firstSegment; seg != NULL && n < lMax; seg = seg->next) {
        LONG lSegId = SegmentGetName(seg);
        if (lSegId >= lFirst && lSegId <= lLast)
            alSegIds[n++] = lSegId;
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
    INIT_PS(hps, FALSE);

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
    INIT_PS(hps, GPI_ERROR);
    return (LONG)pps->ulEditMode;
}

/*!
   @brief Set the element pointer of the currently open segment.

   The GBSEG header is skipped: element 0 is the first user-visible
   element of the segment body.

   @param[in] hps       Presentation space handle.
   @param[in] lElement  Zero-based element index (0 .. lElementCount).

   @return TRUE on success, FALSE on error.
*/
BOOL APIENTRY GpiSetElementPointer(HPS hps, LONG lElement)
{
    PSEGMENT seg;
    PELEMENT el;
    LONG     i;

    INIT_PS(hps, FALSE);

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
    el = seg->firstElement->next;   /* skip GBSEG header */
    for (i = 0; i < lElement && el != NULL; i++) el = el->next;
    seg->currentElement = el;
    return TRUE;
}

/*!
   @brief Query the element pointer.

   The GBSEG header is skipped.

   @param[in] hps  Presentation space handle.

   @return Zero-based element index, or GPI_ERROR on invalid hps.
*/
LONG APIENTRY GpiQueryElementPointer(HPS hps)
{
    PSEGMENT seg;
    PELEMENT el;
    LONG     i = 0;

    INIT_PS(hps, GPI_ERROR);

    seg = pps->pOpenSegment;
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    for (el = seg->firstElement->next;   /* skip GBSEG header */
         el != NULL && el != seg->currentElement;
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
    INIT_PS(hps, FALSE);

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
    INIT_PS(hps, FALSE);

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
    INIT_PS(hps, FALSE);

    (void)pszDesc;
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
