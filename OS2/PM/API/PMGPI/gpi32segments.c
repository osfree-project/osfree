/*!
   @file
   @ingroup pm
   @brief GPI segment API (32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website
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

BOOL APIENTRY GpiDeleteSegment(HPS hps, LONG lSegId)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT prev, seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->pOpenSegment != NULL && pps->pOpenSegment->lSegId == lSegId) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_IS_CURRENT),
                        SEI_NOBEEP);
        return FALSE;
    }

    prev = NULL;
    for (seg = pps->firstSegment; seg != NULL; seg = seg->next) {
        if (seg->lSegId == lSegId) {
            if (prev == NULL) pps->firstSegment = seg->next;
            else              prev->next = seg->next;
            FreeSegmentElements(seg);
            free(seg);
            return TRUE;
        }
        prev = seg;
    }
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                    SEI_NOBEEP);
    return FALSE;
}

BOOL APIENTRY GpiDeleteSegments(HPS hps, LONG lFirst, LONG lLast)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT prev, seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    prev = NULL;
    seg  = pps->firstSegment;
    while (seg != NULL) {
        PSEGMENT next = seg->next;
        if (seg->lSegId >= lFirst && seg->lSegId <= lLast) {
            if (pps->pOpenSegment == seg) {
                WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_IS_CURRENT),
                                SEI_NOBEEP);
                return FALSE;
            }
            if (prev == NULL) pps->firstSegment = next;
            else              prev->next = next;
            FreeSegmentElements(seg);
            free(seg);
        } else {
            prev = seg;
        }
        seg = next;
    }
    return TRUE;
}

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
        if ((seg->ulAttrs & (ATTR_CHAINED | ATTR_FASTCHAIN)) == 0) continue;
        if (seg->ulAttrs & ATTR_DYNAMIC)      continue;
        if ((seg->ulAttrs & ATTR_VISIBLE) == 0) continue;
        if (!ReplaySegment(pps, seg)) return FALSE;
    }
    return TRUE;
}

BOOL APIENTRY GpiSetSegmentAttrs(HPS hps, LONG lSegId,
                                 LONG lAttr, LONG lAttrValue)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    seg = FindSegment(pps, lSegId);
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                        SEI_NOBEEP);
        return FALSE;
    }
    switch (lAttr) {
    case ATTR_CHAINED: case ATTR_DYNAMIC: case ATTR_FASTCHAIN:
    case ATTR_VISIBLE: case ATTR_DETECTABLE:
        if (lAttrValue == ATTR_ON)       seg->ulAttrs |=  (ULONG)lAttr;
        else if (lAttrValue == ATTR_OFF) seg->ulAttrs &= ~(ULONG)lAttr;
        else                             return FALSE;
        return TRUE;
    default: return FALSE;
    }
}

LONG APIENTRY GpiQuerySegmentAttrs(HPS hps, LONG lSegId, LONG lAttr)
{
    PPS      pps = LookupPS(hps);
    PSEGMENT seg;

    if (pps == NULL) return GPI_ERROR;
    seg = FindSegment(pps, lSegId);
    if (seg == NULL) return GPI_ERROR;
    switch (lAttr) {
    case ATTR_CHAINED: case ATTR_DYNAMIC: case ATTR_FASTCHAIN:
    case ATTR_VISIBLE: case ATTR_DETECTABLE:
        return (seg->ulAttrs & (ULONG)lAttr) ? ATTR_ON : ATTR_OFF;
    default: return GPI_ERROR;
    }
}

BOOL APIENTRY GpiSetInitialSegmentAttrs(HPS hps, LONG lAttr, LONG lAttrValue)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    switch (lAttr) {
    case ATTR_CHAINED: case ATTR_DYNAMIC: case ATTR_FASTCHAIN:
    case ATTR_VISIBLE: case ATTR_DETECTABLE:
        if (lAttrValue == ATTR_ON)       pps->ulInitialSegAttrs |=  (ULONG)lAttr;
        else if (lAttrValue == ATTR_OFF) pps->ulInitialSegAttrs &= ~(ULONG)lAttr;
        else                             return FALSE;
        return TRUE;
    default: return FALSE;
    }
}

LONG APIENTRY GpiQueryInitialSegmentAttrs(HPS hps, LONG lAttr)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) return GPI_ERROR;
    switch (lAttr) {
    case ATTR_CHAINED: case ATTR_DYNAMIC: case ATTR_FASTCHAIN:
    case ATTR_VISIBLE: case ATTR_DETECTABLE:
        return (pps->ulInitialSegAttrs & (ULONG)lAttr) ? ATTR_ON : ATTR_OFF;
    default: return GPI_ERROR;
    }
}

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
    if (lMax < 0 || (lMax > 0 && alSegIds == NULL)) {
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

LONG APIENTRY GpiQueryEditMode(HPS hps)
{
    PPS pps = LookupPS(hps);
    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    return (LONG)pps->ulEditMode;
}

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
