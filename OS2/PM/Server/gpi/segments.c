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

   The first element of every segment is an OCODE_GBSEG order whose
   payload is ORDERL_GBSEG (see pmord.h). That header carries the
   segment name and its attributes; see SegmentGetName, SegmentGetAttr,
   SegmentSetAttr. Per IBM SC31-6804, "Begin Segment (GBSEG) Order",
   pp. 102-103 (FLAG1), and AFPC-0008-03, p. 75 (FLAG2).
*/

#include <stdlib.h>
#include <string.h>

/* === GBSEG accessors =============================================== */

/*!
   @brief Return the GBSEG header of a segment, or NULL.

   @param[in] seg  Segment to query.

   @return Pointer to ORDERL_GBSEG inside the first element, or NULL
           if the segment is malformed (no header, wrong type, or
           insufficient payload).
*/
static ORDERL_GBSEG *GetGbseg(PSEGMENT seg)
{
    if (seg == NULL || seg->firstElement == NULL) return NULL;
    if (seg->firstElement->lType != OCODE_GBSEG)  return NULL;
    if (seg->firstElement->cbData < (LONG)sizeof(ORDERL_GBSEG))
        return NULL;
    return (ORDERL_GBSEG *)seg->firstElement->abData;
}

/*!
   @brief Retrieve the segment name from the GBSEG header.

   The name is stored in ORDERL_GBSEG.ulName (offset 2-5 of the
   GBSEG order). Per IBM SC31-6804, "Begin Segment Command", NAME is
   a 4-byte UBIN.

   @param[in] seg  Segment to query.

   @return 4-byte segment name, or 0 on malformed segment.
*/
LONG SegmentGetName(PSEGMENT seg)
{
    ORDERL_GBSEG *p = GetGbseg(seg);
    return p ? p->ulName : 0;
}

/*!
   @brief Map an ATTR_* to a (FLAG1-mask | FLAG2-mask, inverted) triple.

   Per IBM SC31-6804, p. 102 (FLAG1) and AFPC-0008-03, p. 75 (FLAG2).
   Only one of *pfFlag1 / *pfFlag2 is non-zero for any given ATTR_*.

   @param[in]  lAttr      ATTR_* identifier from <pmgpi.h>.
   @param[out] pfFlag1    FLAG1 mask, or 0.
   @param[out] pfFlag2    FLAG2 mask, or 0.
   @param[out] pfInverted TRUE if ATTR_ON maps to bit=0.

   @return TRUE if lAttr is a known segment attribute, FALSE otherwise.
*/
static BOOL AttrToGoca(LONG lAttr, UCHAR *pfFlag1, UCHAR *pfFlag2,
                       BOOL *pfInverted)
{
    *pfFlag1    = 0;
    *pfFlag2    = 0;
    *pfInverted = FALSE;

    switch (lAttr) {
    case ATTR_VISIBLE:
        *pfFlag1 = GBSEG_INVISIBLE; *pfInverted = TRUE;  return TRUE;
    case ATTR_PROP_VISIBLE:
        *pfFlag1 = GBSEG_PROPINV;   *pfInverted = TRUE;  return TRUE;
    case ATTR_DETECTABLE:
        *pfFlag1 = GBSEG_PICK;      *pfInverted = FALSE; return TRUE;
    case ATTR_PROP_DETECTABLE:
        *pfFlag1 = GBSEG_PROPICK;   *pfInverted = FALSE; return TRUE;
    case ATTR_DYNAMIC:
        *pfFlag1 = GBSEG_DYNAMIC;   *pfInverted = FALSE; return TRUE;
    case ATTR_FASTCHAIN:
        *pfFlag1 = GBSEG_FASTCH;    *pfInverted = FALSE; return TRUE;
    case ATTR_CHAINED:
        *pfFlag2 = GBSEG_CHAIN;     *pfInverted = TRUE;  return TRUE;
    default:
        return FALSE;
    }
}

/*!
   @brief Query a segment attribute from the GBSEG header.

   See ps.h for the ATTR_* -> FLAG1/FLAG2 mapping table.

   @param[in] seg    Segment to query.
   @param[in] lAttr  ATTR_* identifier.

   @return ATTR_ON, ATTR_OFF, or GPI_ERROR.
*/
LONG SegmentGetAttr(PSEGMENT seg, LONG lAttr)
{
    ORDERL_GBSEG *p = GetGbseg(seg);
    UCHAR  f1, f2, mask, byte;
    BOOL   inv, on;

    if (p == NULL) return GPI_ERROR;
    if (!AttrToGoca(lAttr, &f1, &f2, &inv)) return GPI_ERROR;

    mask = (f1 != 0) ? f1 : f2;
    byte = (f1 != 0) ? p->fbFlag1 : p->fbFlag2;
    on   = (byte & mask) != 0;
    if (inv) on = !on;
    return on ? ATTR_ON : ATTR_OFF;
}

/*!
   @brief Set a segment attribute in the GBSEG header.

   Same mapping as SegmentGetAttr.

   @param[in] seg     Segment to modify.
   @param[in] lAttr   ATTR_* identifier.
   @param[in] lValue  ATTR_ON or ATTR_OFF.

   @return TRUE on success, FALSE on invalid attribute.
*/
BOOL SegmentSetAttr(PSEGMENT seg, LONG lAttr, LONG lValue)
{
    ORDERL_GBSEG *p = GetGbseg(seg);
    UCHAR  f1, f2, mask;
    BOOL   inv, on;
    PUCHAR target;

    if (p == NULL) return FALSE;
    if (lValue != ATTR_ON && lValue != ATTR_OFF) return FALSE;
    if (!AttrToGoca(lAttr, &f1, &f2, &inv)) return FALSE;

    mask   = (f1 != 0) ? f1 : f2;
    target = (f1 != 0) ? &p->fbFlag1 : &p->fbFlag2;

    on = (lValue == ATTR_ON);
    if (inv) on = !on;

    if (on) *target |=  mask;
    else    *target &= ~mask;
    return TRUE;
}

/* === Segment lifecycle ============================================= */

/*!
   @brief Allocate a segment and append it to the chain (highest pri).

   A GBSEG header element is created and linked as the first element
   of the new segment. It carries:
     - ulName       = lSegId;
     - fbFlag1      = from ulAttrs (SC31-6804, p. 102);
     - fbFlag2      = GBSEG_APP_NEW, plus GBSEG_CHAIN if !ATTR_CHAINED;
     - usSegDataLen = 0 (updated only when serialising via GpiGetData);
     - ulPSName     = 0.

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
    PSEGMENT      seg;
    PELEMENT      hdr;
    ORDERL_GBSEG *p;

    if (pps == NULL) return NULL;
    seg = (PSEGMENT)AllocMemZero(sizeof(SEGMENT));
    if (seg == NULL) return NULL;

    hdr = (PELEMENT)AllocMemZero(sizeof(ELEMENT) + sizeof(ORDERL_GBSEG));
    if (hdr == NULL) {
        DosFreeMem(seg);
        return NULL;
    }

    hdr->next   = NULL;
    hdr->lType  = OCODE_GBSEG;
    hdr->lCount = 1;
    hdr->cbData = sizeof(ORDERL_GBSEG);

    p = (ORDERL_GBSEG *)hdr->abData;
    p->uchCode      = OCODE_GBSEG;
    p->uchLength    = GBSEG_LENGTH;
    p->ulName       = (ULONG)lSegId;
    p->fbFlag1      = 0;
    p->fbFlag2      = GBSEG_APP_NEW;
    p->usSegDataLen = 0;
    p->ulPSName     = 0;

    seg->firstElement   = hdr;
    seg->lastElement    = hdr;
    seg->currentElement = NULL;
    seg->lElementCount  = 0;
    seg->fInElement     = FALSE;

    /* Apply the initial attribute set to the GBSEG flags. */
    {
        static const LONG pm_attrs[] = {
            ATTR_VISIBLE, ATTR_PROP_VISIBLE,
            ATTR_DETECTABLE, ATTR_PROP_DETECTABLE,
            ATTR_DYNAMIC, ATTR_FASTCHAIN,
            ATTR_CHAINED
        };
        int i;
        for (i = 0; i < (int)(sizeof(pm_attrs)/sizeof(pm_attrs[0])); i++) {
            LONG on = (ulAttrs & SEGATTR_BIT(pm_attrs[i])) ? ATTR_ON : ATTR_OFF;
            SegmentSetAttr(seg, pm_attrs[i], on);
        }
    }

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

   Parses each segment's GBSEG header on the fly via SegmentGetName.

   @param[in] pps     Presentation space.
   @param[in] lSegId  Segment identifier.

   @return Matching segment, or NULL.
*/
PSEGMENT FindSegment(PPS pps, LONG lSegId)
{
    PSEGMENT seg;
    if (pps == NULL) return NULL;
    for (seg = pps->firstSegment; seg != NULL; seg = seg->next) {
        if (SegmentGetName(seg) == lSegId) return seg;
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

   The GBSEG header is part of the element list and is freed along
   with the body elements.

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
    seg->fInElement     = FALSE;
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

   The GBSEG header always precedes the body, so lastElement is never
   NULL for a valid open segment.

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

/*!
   @brief Dispatch one recorded element to the presentation driver.

   Non-static: GpiPutData uses it directly to replay the orders it
   deserializes. The GBSEG header is not replayed (nothing to draw);
   user elements (lType > 0xFF) are opaque and ignored.

   @param[in] pps  Presentation space.
   @param[in] el   Element to replay.

   @return TRUE on success, FALSE on error.
*/
BOOL ReplayElement(PPS pps, PELEMENT el)
{
    HDC hdc;

    if (pps == NULL || el == NULL) return FALSE;
    hdc = pps->hdc;
    if (hdc == (HDC)0) return FALSE;

    if (el->lType > 0xFF) return TRUE;  /* user elements */

    switch (el->lType) {

    /* --- GBSEG header: nothing to draw -------------------------- */
    case OCODE_GBSEG:
        return TRUE;

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
    case OCODE_GEEL:
    default:
        return TRUE;
    }
}

/*!
   @brief Replay all elements of a segment into the driver.

   The GBSEG header is skipped by the OCODE_GBSEG case in
   ReplayElement; only body elements are dispatched.

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
