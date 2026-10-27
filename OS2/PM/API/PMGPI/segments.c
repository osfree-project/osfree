/*!
   @file
   @ingroup pm
   @brief GPI segment storage and replay (internal, 32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   Element payloads are typed using structures from <pmord.h>.
   The segment chain is a doubly-linked list kept in priority order
   (lowest priority first) to support GpiDrawChain / GpiDrawFrom /
   GpiSetSegmentPriority semantics.
*/

#include <stdlib.h>
#include <string.h>

/* === Segment lifecycle ============================================= */

/*!
   @brief Allocate a segment and append it to the chain (highest pri).

   @param[in] pps      Presentation space.
   @param[in] lSegId   Segment identifier (caller validates lSegId > 0).
   @param[in] ulAttrs  Initial attribute bits (SEGATTR_BIT values).

   @return New segment, or NULL on allocation failure.

   @note  A new segment becomes the highest-priority segment in the
          chain because it is appended after all existing ones
          (OS/2 PM Programming Reference Vol.1, "GpiOpenSegment -
          Remarks": new segments are chained to the previous segment).
*/
PSEGMENT CreateSegment(PPS pps, LONG lSegId, ULONG ulAttrs)
{
    PSEGMENT seg;

    if (pps == NULL) return NULL;
    seg = (PSEGMENT)AllocMemZero(sizeof(SEGMENT));
    if (seg == NULL) return NULL;

    seg->lSegId  = lSegId;
    seg->ulAttrs = ulAttrs;

    seg->prev = pps->lastSegment;
    if (pps->lastSegment != NULL)
        pps->lastSegment->next = seg;
    else
        pps->firstSegment = seg;
    pps->lastSegment = seg;

    return seg;
}

/*!
   @brief Find a segment by identifier.

   @param[in] pps     Presentation space.
   @param[in] lSegId  Segment identifier.

   @return Matching segment, or NULL.
*/
PSEGMENT FindSegment(PPS pps, LONG lSegId)
{
    PSEGMENT seg;
    if (pps == NULL) return NULL;
    for (seg = pps->firstSegment; seg != NULL; seg = seg->next) {
        if (seg->lSegId == lSegId) return seg;
    }
    return NULL;
}

/*!
   @brief Unlink a segment from the chain without freeing it.

   @param[in] pps  Presentation space.
   @param[in] seg  Segment to unlink.
*/
static void UnlinkSegment(PPS pps, PSEGMENT seg)
{
    if (seg->prev != NULL)
        seg->prev->next = seg->next;
    else
        pps->firstSegment = seg->next;

    if (seg->next != NULL)
        seg->next->prev = seg->prev;
    else
        pps->lastSegment = seg->prev;

    seg->next = NULL;
    seg->prev = NULL;
}

/*!
   @brief Insert a segment into the chain in priority order.

   @param[in] pps       Presentation space.
   @param[in] seg       Segment to insert (must already be unlinked).
   @param[in] lRefSegId Reference segment id (0 = chain end).
   @param[in] lOrder    LOWER_PRI or HIGHER_PRI.

   @return TRUE on success, FALSE if lRefSegId is not found.

   @see GpiSetSegmentPriority
*/
BOOL InsertSegmentByPriority(PPS pps, PSEGMENT seg,
                             LONG lRefSegId, LONG lOrder)
{
    PSEGMENT ref;

    if (pps == NULL || seg == NULL) return FALSE;

    if (lRefSegId == 0) {
        if (lOrder == LOWER_PRI) {
            seg->prev = pps->lastSegment;
            if (pps->lastSegment != NULL)
                pps->lastSegment->next = seg;
            else
                pps->firstSegment = seg;
            pps->lastSegment = seg;
        } else {
            seg->next = pps->firstSegment;
            if (pps->firstSegment != NULL)
                pps->firstSegment->prev = seg;
            else
                pps->lastSegment = seg;
            pps->firstSegment = seg;
        }
        return TRUE;
    }

    ref = FindSegment(pps, lRefSegId);
    if (ref == NULL) return FALSE;

    if (lOrder == LOWER_PRI) {
        seg->prev = ref->prev;
        seg->next = ref;
        if (ref->prev != NULL)
            ref->prev->next = seg;
        else
            pps->firstSegment = seg;
        ref->prev = seg;
    } else {
        seg->next = ref->next;
        seg->prev = ref;
        if (ref->next != NULL)
            ref->next->prev = seg;
        else
            pps->lastSegment = seg;
        ref->next = seg;
    }
    return TRUE;
}

/*!
   @brief Free all elements of a segment (segment itself untouched).

   @param[in] seg  Segment whose element list is freed.
*/
void FreeSegmentElements(PSEGMENT seg)
{
    PELEMENT el;
    if (seg == NULL) return;
    el = seg->firstElement;
    while (el != NULL) {
        PELEMENT next = el->next;
        DosFreeMem(el);
        el = next;
    }
    seg->firstElement   = NULL;
    seg->lastElement    = NULL;
    seg->currentElement = NULL;
    seg->lElementCount  = 0;
}

/*!
   @brief Free all segments of a presentation space.

   @param[in] pps  Presentation space.
*/
void FreeAllSegments(PPS pps)
{
    PSEGMENT seg;
    if (pps == NULL) return;
    seg = pps->firstSegment;
    while (seg != NULL) {
        PSEGMENT next = seg->next;
        FreeSegmentElements(seg);
        DosFreeMem(seg);
        seg = next;
    }
    pps->firstSegment = NULL;
    pps->lastSegment  = NULL;
    pps->pOpenSegment = NULL;
}

/* === Recording ===================================================== */

/*!
   @brief Append an element to the currently open segment.

   @param[in] pps     Presentation space.
   @param[in] lType   Order code or user element type.
   @param[in] lCount  Number of items.
   @param[in] cbData  Payload size in bytes.
   @param[in] pData   Payload (may be NULL when cbData == 0).

   @return TRUE on success, FALSE on error.
*/
BOOL RecordElement(PPS pps, LONG lType, LONG lCount, LONG cbData,
                   const void *pData)
{
    PELEMENT el;

    if (pps == NULL || pps->pOpenSegment == NULL) return FALSE;
    if (lCount < 0 || cbData < 0)                 return FALSE;

    el = (PELEMENT)AllocMemZero(sizeof(ELEMENT) + cbData);
    if (el == NULL) return FALSE;

    el->next   = NULL;
    el->lType  = lType;
    el->lCount = lCount;
    el->cbData = cbData;

    if (cbData > 0 && pData != NULL)
        memcpy(el->abData, pData, cbData);

    if (pps->pOpenSegment->lastElement == NULL) {
        pps->pOpenSegment->firstElement = el;
    } else {
        pps->pOpenSegment->lastElement->next = el;
    }
    pps->pOpenSegment->lastElement = el;
    pps->pOpenSegment->lElementCount++;
    return TRUE;
}

/*!
   @brief Record a GpiCharStringPos element (ORDERL_GCCHSTE layout).

   Payload is ORDERL_GCCHSTE followed by `cChars` bytes of string, then
   (if CHS_VECTOR) `cChars` LONGs of adx. cchString is stored as
   SWPUSHORT (big-endian) per <pmord.h>.
*/
BOOL RecordCharStringPos(PPS pps, ULONG flOptions, LONG cChars,
                         PCH pchString, PRECTL prclRect, PLONG pAdx)
{
    ORDERL_GCCHSTE *pBuf;
    LONG            cbFixed;
    LONG            cbTotal;
    UCHAR           fbFlags = 0;
    PBYTE           pTrail;
    BOOL            result;

    /* Map GPI CHS_* flags to GOCA GCCHSTE_* flags. CHS_START_XY is
       not representable in ORDERL_GCCHSTE and is rejected earlier. */
    if (flOptions & CHS_OPAQUE)    fbFlags |= GCCHSTE_DRAWRECT;
    if (flOptions & CHS_CLIP)      fbFlags |= GCCHSTE_CLIP;
    if (flOptions & CHS_LEAVEPOS)  fbFlags |= GCCHSTE_LEAVEPOS;
    if (flOptions & CHS_UNDERSCORE) fbFlags |= GCCHSTE_UNDERSCORE;
    if (flOptions & CHS_STRIKEOUT) fbFlags |= GCCHSTE_STRIKEOUT;

    cbFixed = sizeof(ORDERL_GCCHSTE) - sizeof(CHAR) - sizeof(LONG);
    cbTotal = cbFixed + cChars * sizeof(CHAR)
              + ((flOptions & CHS_VECTOR) ? cChars * sizeof(LONG) : 0);

    pBuf = (ORDERL_GCCHSTE *)AllocMemZero(cbTotal);
    if (pBuf == NULL) return FALSE;

    pBuf->fbFlags     = fbFlags;
    pBuf->uchReserved = 0;
    if (prclRect) {
        pBuf->ptRect[0].x = prclRect->xLeft;
        pBuf->ptRect[0].y = prclRect->yBottom;
        pBuf->ptRect[1].x = prclRect->xRight;
        pBuf->ptRect[1].y = prclRect->yTop;
    }
    pBuf->cchString.HiByte = (UCHAR)((cChars >> 8) & 0xFF);
    pBuf->cchString.LoByte = (UCHAR)( cChars       & 0xFF);

    pTrail = (PBYTE)pBuf + cbFixed;
    memcpy(pTrail, pchString, cChars);
    if (flOptions & CHS_VECTOR)
        memcpy(pTrail + cChars, pAdx, cChars * sizeof(LONG));

    result = RecordElement(pps, OCODEQ_GCCHSTE, 1, cbTotal, pBuf);
    DosFreeMem(pBuf);
    return result;
}

/*!
   @brief Record a GpiPolyFilletSharp element (ORDERL_GCSFLT layout).

   <pmord.h> defines ORDERL_GCSFLT with fixed-size arrays
   apt[2*GCSFLT_SMF] and afxSharpness[GCSFLT_SMF]. lCount must be
   <= GCSFLT_SMF (21). If lCount > GCSFLT_SMF, caller must split
   into multiple elements.
*/
BOOL RecordPolyFilletSharp(PPS pps, LONG lCount, PPOINTL aptlPoints,
                           PFIXED afxSharp)
{
    ORDERL_GCSFLT flt;
    LONG          i;

    if (lCount <= 0 || lCount > GCSFLT_SMF) return FALSE;

    memset(&flt, 0, sizeof(flt));
    for (i = 0; i < lCount; i++) {
        flt.apt[i * 2]     = aptlPoints[i];
        flt.apt[i * 2 + 1] = aptlPoints[i];
    }
    for (i = 0; i < lCount / 2; i++)
        flt.afxSharpness[i] = afxSharp[i];

    return RecordElement(pps, OCODE_GCSFLT, 1, sizeof(flt), &flt);
}

/* === Replay ======================================================== */

static BOOL ReplayElement(PPS pps, PELEMENT el)
{
    HDC hdc;

    if (pps == NULL || el == NULL) return FALSE;
    hdc = pps->hdc;
    if (hdc == (HDC)0) return FALSE;

    if (el->lType > 0xFF) return TRUE;  /* user elements */

    switch (el->lType) {

    /* --- Simple arrays ------------------------------------------ */
    case OCODE_GCLINE:
        return (LONG)GrePolyLine(hdc, (PPOINTL)el->abData, el->lCount)
               != (LONG)GPI_ERROR;
    case OCODE_GCMRK:
        return (LONG)GrePolyMarker(hdc, (PPOINTL)el->abData, el->lCount)
               != (LONG)GPI_ERROR;
    case OCODE_GCFLT:
        return (LONG)GrePolyFillet(hdc, (PPOINTL)el->abData, el->lCount)
               != (LONG)GPI_ERROR;
    case OCODE_GCBEZ:
        return (LONG)GrePolySpline(hdc, (PPOINTL)el->abData, el->lCount)
               != (LONG)GPI_ERROR;
    case OCODE_GCCHST:
        return (LONG)GreCharString(hdc, el->lCount, (PCH)el->abData)
               != (LONG)GPI_ERROR;
    case OCODE_GSCP:
        return GreSetCurrentPosition(hdc, (PPOINTL)el->abData);
    case OCODE_GCOMT:
        return TRUE;

    /* --- pmord.h structures ------------------------------------- */
    case OCODE_GCBOX: {
        ORDERL_GCBOX *p = (ORDERL_GCBOX *)el->abData;
        BOXPARAMS     bp;
        bp.ptl     = p->ptCorner;
        bp.sizl.cx = p->hAxis;
        bp.sizl.cy = p->vAxis;
        if ((p->fbFlags & (GCBOX_FILL | GCBOX_BOUNDARY)) ==
            (GCBOX_FILL | GCBOX_BOUNDARY))
            return (LONG)GreBoxBoth(hdc, &bp)     != (LONG)GPI_ERROR;
        if (p->fbFlags & GCBOX_FILL)
            return (LONG)GreBoxInterior(hdc, &bp) != (LONG)GPI_ERROR;
        if (p->fbFlags & GCBOX_BOUNDARY)
            return (LONG)GreBoxBoundary(hdc, &bp) != (LONG)GPI_ERROR;
        return TRUE;
    }
    case OCODE_GCPARC: {
        ORDERL_GCPARC *p = (ORDERL_GCPARC *)el->abData;
        return (LONG)GrePartialArc(hdc, &p->ptCenter, p->ufxMultiplier,
                                   p->usStartAngle, p->usSweepAngle)
               != (LONG)GPI_ERROR;
    }
    case OCODE_GCARC: {
        ORDERL_GCARC *p = (ORDERL_GCARC *)el->abData;
        POINTL        apt[2];
        apt[0] = p->ptInter;
        apt[1] = p->ptEnd;
        return (LONG)GreArc(hdc, apt) != (LONG)GPI_ERROR;
    }
    case OCODE_GBPTH: {
        ORDER_GBPTH *p = (ORDER_GBPTH *)el->abData;
        return GreBeginPath(hdc, p->idPath) != GPI_ERROR;
    }
    case OCODE_GFPTH: {
        ORDER_GFPTH *p = (ORDER_GFPTH *)el->abData;
        LONG lMode = (p->fbFlags & GFPTH_WINDING) ? FPATH_WINDING
                                                   : FPATH_ALTERNATE;
        return (LONG)GreFillPath(hdc, p->idPath, lMode) != (LONG)GPI_ERROR;
    }
    case OCODE_GOPTH: {
        ORDER_GOPTH *p = (ORDER_GOPTH *)el->abData;
        return (LONG)GreOutlinePath(hdc, p->idPath, 0) != (LONG)GPI_ERROR;
    }
    case OCODE_GMPTH: {
        ORDER_GMPTH *p = (ORDER_GMPTH *)el->abData;
        if (p->uchMode == GMPTH_STROKE)
            return (LONG)GreStrokePath(hdc, p->idPath, 0) != (LONG)GPI_ERROR;
        return GreModifyPath(hdc, p->idPath, p->uchMode) != GPI_ERROR;
    }
    case OCODE_GSCPTH: {
        ORDER_GSCPTH *p = (ORDER_GSCPTH *)el->abData;
        LONG lMode = (p->fbFlags & GSCPTH_WINDING) ? SCP_WINDING
                                                    : SCP_ALTERNATE;
        if (p->fbFlags & GSCPTH_INTERSECT) lMode |= SCP_AND;
        else                                lMode |= SCP_RESET;
        return GreSelectClipPath(hdc, p->idPath, lMode) != GPI_ERROR;
    }
    case OCODEQ_GCCHSTE: {
        ORDERL_GCCHSTE *p    = (ORDERL_GCCHSTE *)el->abData;
        LONG            cch  = (p->cchString.HiByte << 8)
                               | p->cchString.LoByte;
        LONG            cbFixed = sizeof(ORDERL_GCCHSTE)
                                  - sizeof(CHAR) - sizeof(LONG);
        PCH             pch  = (PCH)((PBYTE)p + cbFixed);
        PLONG           pAdx = (PLONG)(pch + cch);
        ULONG           fl   = 0;
        RECTL           rcl;
        PRECTL          prcl = NULL;
        PLONG           pAdxP = NULL;
        LONG            cbExpected;

        if (p->fbFlags & GCCHSTE_DRAWRECT) fl |= CHS_OPAQUE;
        if (p->fbFlags & GCCHSTE_CLIP)     fl |= CHS_CLIP;
        if (p->fbFlags & GCCHSTE_LEAVEPOS) fl |= CHS_LEAVEPOS;
        if (p->fbFlags & GCCHSTE_UNDERSCORE) fl |= CHS_UNDERSCORE;
        if (p->fbFlags & GCCHSTE_STRIKEOUT) fl |= CHS_STRIKEOUT;

        if (p->fbFlags & (GCCHSTE_DRAWRECT | GCCHSTE_CLIP)) {
            rcl.xLeft   = p->ptRect[0].x;
            rcl.yBottom = p->ptRect[0].y;
            rcl.xRight  = p->ptRect[1].x;
            rcl.yTop    = p->ptRect[1].y;
            prcl = &rcl;
        }

        /* Detect CHS_VECTOR by total size. */
        cbExpected = cbFixed + cch * sizeof(CHAR);
        if ((LONG)el->cbData > cbExpected) {
            fl |= CHS_VECTOR;
            pAdxP = pAdx;
        }

        return (LONG)GreCharStringPos(hdc, NULL, prcl, fl,
                                      cch, pch, pAdxP, NULL)
               != (LONG)GPI_ERROR;
    }
    case OCODE_GCSFLT: {
        ORDERL_GCSFLT *p = (ORDERL_GCSFLT *)el->abData;
        /* apt[2*i] and apt[2*i+1] hold start/end of fillet i; only
           the end point matters to GrePolyFilletSharp, so extract
           every second point. */
        POINTL apt[GCSFLT_SMF];
        LONG   i, n = el->lCount;
        for (i = 0; i < n; i++)
            apt[i] = p->apt[i * 2 + 1];
        return (LONG)GrePolyFilletSharp(hdc, apt, n, p->afxSharpness)
               != (LONG)GPI_ERROR;
    }

    /* --- 2-byte orders ------------------------------------------ */
    case OCODE_GBAR: {
        ORDER *p = (ORDER *)el->abData;
        ULONG  flOptions = BA_ALTERNATE;
        if (p->uchData & 0x40) flOptions |= BA_BOUNDARY;
        if (p->uchData & 0x20) flOptions |= BA_WINDING;
        return GreBeginArea(hdc, flOptions) != GPI_ERROR;
    }
    case OCODE_GEPTH:
        return GreEndPath(hdc, 0) != GPI_ERROR;
    case OCODE_GCFIG:
        return GreCloseFigure(hdc) != GPI_ERROR;
    case OCODE_GEAR:
        return (LONG)GreEndArea(hdc, 0) != (LONG)GPI_ERROR;

    /* --- GpiFullArc: only FIXED field, no struct in pmord.h ----- */
    case OCODE_GCFARC:
        return (LONG)GreFullArcBoth(hdc, *(PFIXED)el->abData)
               != (LONG)GPI_ERROR;

    case OCODE_GLABL:
    case OCODE_GBEL:
    default:
        return TRUE;
    }
}

/*!
   @brief Replay all elements of a segment into the driver.

   @param[in] pps  Presentation space.
   @param[in] seg  Segment to replay.

   @return TRUE on success, FALSE on error.
*/
BOOL ReplaySegment(PPS pps, PSEGMENT seg)
{
    PELEMENT el;
    if (pps == NULL || seg == NULL) return FALSE;
    for (el = seg->firstElement; el != NULL; el = el->next) {
        if (!ReplayElement(pps, el)) return FALSE;
    }
    return TRUE;
}
