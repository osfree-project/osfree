/*!
   @file
   @ingroup pm
   @brief GPI segment API (32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   Fact-check sources (OS/2 Presentation Manager Programming Reference,
   Volume 1) used below:
     - GpiOpenSegment         : lSegId > 0, no already-open segment.
     - GpiGetData             : DFORM_NOCONV only in this implementation;
                                not valid for the currently open segment.
     - GpiPutData             : invalid in SEGEM_REPLACE mode;
                                invalid in DM_RETAIN without an open
                                segment; incomplete trailing data is
                                left unprocessed.
     - GpiBeginElement        : lType > 0; PMERR_ALREADY_IN_ELEMENT on
                                a nested begin.
     - GpiEndElement          : PMERR_NOT_IN_ELEMENT without a begin.
     - GpiDeleteElement       : element pointer; PMERR_NO_CURRENT_ELEMENT.
     - GpiDeleteElementRange  : 0-based, inclusive.
     - GpiDeleteElementsBetweenLabels : labels themselves kept;
                                PMERR_LABEL_NOT_FOUND if either missing.
     - GpiSetElementPointerAtLabel : pointer to element after label.
     - GpiQueryElement        : copies min(lLength, cbData).
     - GpiQueryElementType    : ORDER_GBEL type/description.
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

   GpiGetData / GpiPutData use the canonical GOCA order stream
   (AFP GOCA Reference, "Order Formats":
     fixed 1-byte: code
     fixed 2-byte: code + operand
     long        : code + length + data
     extended    : X'FE' + qualifier + 2-byte BE length + data)
   Coordinates are 16-bit SBIN big-endian; multi-byte fields
   (FIXED / LONG) are big-endian.

   Elements are converted between the internal 32-bit ELEMENT
   representation and the canonical GOCA byte stream as follows:

     - OCODE_GBSEG (segment header) is NOT serialized: it is
       segment-control metadata, not a drawing order in the body.
       The canonical X'70' Begin Segment order carries SEGL (the
       length of the following SEGDATA), which cannot be written
       correctly during a single-pass per-element serialization
       because the total body length is not known until every
       element has been visited. OS/2 PM Reference, "GpiGetData",
       and EDM2, "GpiGetData", describe the result as "a list of
       drawing orders" — the segment name and attributes are
       restored via GpiOpenSegment / GpiSetSegmentAttrs, not via
       GpiGetData. A byte-for-byte round-trip that includes the
       X'70' order would require two-pass serialization; this is
       a separate task.

     - OCODE_GLABL (PM GPI label) is serialized as X'D3' + length(4)
       + 4-byte BE label value, per the decision recorded in the
       comment on OCODE_GLABL in pmord.h. GOCA has no Label order
       (see also the note on GLBL in IBM SC31-6804, "REQUIRES
       VERIFICATION"), so X'D3' in the byte stream is an osFree
       PM GPI extension. Readers that do not know it should treat
       it as a No-Op (same policy as X'D3' in pmord.h).

     - OCODE_GBEL / OCODE_GEEL (GpiBeginElement / GpiEndElement
       bracket markers, PM GPI only) are serialized in extended
       format with qualifier OSFREE_Q_USERELEM (0xF5), same as
       user elements, so that GpiGetData -> GpiPutData preserves
       the bracket. There is no canonical GOCA code for them.

     - User elements (ELEMENT.lType > 0xFF, stored by GpiElement)
       are serialized in extended format with qualifier
       OSFREE_Q_USERELEM (0xF5) and payload:
         LONG lType  (BE)
         LONG lCount (BE)
         LONG cbData (BE)
         BYTE payload[cbData]
       This is a serialization escape, not a drawn order; it exists
       precisely to make GpiGetData -> GpiPutData a full round-trip
       for user elements. ReplayElement ignores it (user elements
       are opaque to replay).

     - All other supported elements are converted 1:1 to / from
       their canonical GOCA long or extended format. GCFARC is
       converted per "Full Arc (GFARC, GCFARC) Orders" (p. 107):
       FIXED 16.16 -> MH/MFR (8.8). Where the internal ORDERL_*
       in pmord.h still differs structurally from the canonical
       ORDERS_* form (GCPARC: FIXED 16.16 vs MH/MFR for multiplier
       and LONG vs FIXED114 for angles; GBPTH: extra reserved
       USHORT), this implementation follows pmord.h. Aligning
       pmord.h itself with the AFP GOCA Reference is a separate
       task; each such deviation is noted in the corresponding
       ElementToGoca case.

   Functions are grouped by DC requirement:
     - editing functions (INIT_PS)    : GpiCloseSegment, GpiDeleteSegment,
       GpiDeleteSegments, GpiGetData, GpiSetSegmentAttrs,
       GpiQuerySegmentAttrs, GpiSetInitialSegmentAttrs,
       GpiQueryInitialSegmentAttrs, GpiQuerySegmentNames,
       GpiSetEditMode, GpiQueryEditMode, GpiSetElementPointer,
       GpiQueryElementPointer, GpiOffsetElementPointer, GpiLabel,
       GpiElement, GpiBeginElement, GpiEndElement, GpiDeleteElement,
       GpiDeleteElementRange, GpiDeleteElementsBetweenLabels,
       GpiSetElementPointerAtLabel, GpiQueryElement,
       GpiQueryElementType, GpiSetSegmentPriority,
       GpiQuerySegmentPriority;
     - drawing functions (INIT_PSDC)  : GpiOpenSegment, GpiDrawSegment,
       GpiDrawChain, GpiDrawFrom, GpiDrawDynamics, GpiRemoveDynamics,
       GpiCallSegmentMatrix;
     - mode-dependent (INIT_PS + am)  : GpiPutData.
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

/* === Canonical GOCA serialization for GpiGetData / GpiPutData === */

/*!
   @def OSFREE_Q_USERELEM
   @brief Extended-format qualifier for osFree user elements.

   Not part of AFP GOCA or IBM SC31-6804. X'F5' is free in
   <pmord.h> (X'B0' = GCCHSTE, X'F0' = GCHSTE, X'F4' = GTCHSPA,
   X'D5' = GEESCP are taken). The payload layout is described in
   the file header comment.
*/
#define OSFREE_Q_USERELEM  0xF5

/*!
   @brief Total length of a canonical GOCA order, or 0 if truncated.

   @param[in] pb       Pointer to the order.
   @param[in] cbAvail  Bytes available.

   @return Total order length (code + length + data), or 0 if
           truncated or the code is unknown.
*/
static ULONG GocaOrderLength(const BYTE *pb, ULONG cbAvail)
{
    UCHAR code;
    ULONG cb;

    if (cbAvail < 1) return 0;
    code = pb[0];

    /* Fixed 1-byte: only GNOP1 (X'00'). */
    if (code == 0x00) return 1;

    /* Extended: X'FE' + qualifier + 2-byte BE length + data. */
    if (code == 0xFE) {
        if (cbAvail < 4) return 0;
        cb = 4 + ((ULONG)pb[2] << 8) + pb[3];
        return (cb <= cbAvail) ? cb : 0;
    }

    /* Fixed 2-byte: bit 0 = 0 and bit 4 = 1. */
    if ((code & 0x01) == 0 && (code & 0x10) != 0) {
        return (cbAvail >= 2) ? 2 : 0;
    }

    /* Long: code + length + data. */
    if (cbAvail < 2) return 0;
    cb = 2 + (ULONG)pb[1];
    return (cb <= cbAvail) ? cb : 0;
}

/*!
   @brief Write a 16-bit SBIN in big-endian order.

   @param[out] pb    Destination (2 bytes).
   @param[in]  value Host-order value; truncated to 16 bits.
*/
static void PutSbin(BYTE *pb, LONG value)
{
    pb[0] = (BYTE)((value >> 8) & 0xFF);
    pb[1] = (BYTE)( value       & 0xFF);
}

/*!
   @brief Write a 32-bit value in big-endian order.

   @param[out] pb    Destination (4 bytes).
   @param[in]  value Host-order value.
*/
static void PutLongBE(BYTE *pb, ULONG value)
{
    pb[0] = (BYTE)((value >> 24) & 0xFF);
    pb[1] = (BYTE)((value >> 16) & 0xFF);
    pb[2] = (BYTE)((value >>  8) & 0xFF);
    pb[3] = (BYTE)( value        & 0xFF);
}

/*!
   @brief Read a 16-bit SBIN from big-endian bytes.

   @param[in] pb  Source (2 bytes).

   @return Sign-extended 16-bit value in host order.
*/
static LONG GetSbin(const BYTE *pb)
{
    return (LONG)(SHORT)(((USHORT)pb[0] << 8) | pb[1]);
}

/*!
   @brief Read a 32-bit value from big-endian bytes.

   @param[in] pb  Source (4 bytes).

   @return Host-order value.
*/
static ULONG GetLongBE(const BYTE *pb)
{
    return ((ULONG)pb[0] << 24) | ((ULONG)pb[1] << 16)
         | ((ULONG)pb[2] <<  8) |  (ULONG)pb[3];
}

/*!
   @brief Convert one internal ELEMENT to a canonical GOCA order.

   - User elements (lType > 0xFF) and the PM-GPI-only OCODE_GBEL /
     OCODE_GEEL bracket markers are serialized in extended format
     with qualifier OSFREE_Q_USERELEM (0xF5). See the file header.
   - OCODE_GBSEG (segment header) is NOT serialized (see file
     header for why).
   - OCODE_GLABL is serialized as X'D3' + length(4) + 4-byte BE
     label value (see pmord.h comment on OCODE_GLABL).
   - All other supported elements are converted to long or extended
     GOCA format as indicated in each case.

   @param[in]  el     Element to convert.
   @param[out] pb     Destination buffer.
   @param[in]  cbMax  Size of pb.

   @return Total order length written, or 0 if the element cannot be
           represented in canonical GOCA.
*/
static ULONG ElementToGoca(PELEMENT el, PBYTE pb, ULONG cbMax)
{
    /* User elements (lType > 0xFF) AND the PM-GPI-only OCODE_GBEL /
       OCODE_GEEL bracket markers are serialized in extended format
       with qualifier OSFREE_Q_USERELEM (0xF5). The bracket markers
       have no GOCA representation (OCODE_GBEL = 0xD2 would be read
       as a fixed 2-byte order, OCODE_GEEL = 0x49 is PM-only), so
       they take the same escape route as user elements to survive
       GpiGetData -> GpiPutData. */
    if (el != NULL && (el->lType > 0xFF
                       || el->lType == OCODE_GBEL
                       || el->lType == OCODE_GEEL)) {
        ULONG cbUser  = (ULONG)el->cbData;
        ULONG cbTotal = 12 + cbUser;   /* 3 BE LONGs + payload */
        if (cbTotal > 0xFFFF || cbMax < 4 + cbTotal) return 0;
        pb[0] = 0xFE;
        pb[1] = OSFREE_Q_USERELEM;
        pb[2] = (BYTE)((cbTotal >> 8) & 0xFF);
        pb[3] = (BYTE)( cbTotal       & 0xFF);
        PutLongBE(pb + 4,  (ULONG)el->lType);
        PutLongBE(pb + 8,  (ULONG)el->lCount);
        PutLongBE(pb + 12, cbUser);
        if (cbUser > 0)
            memcpy(pb + 16, el->abData, cbUser);
        return 4 + cbTotal;
    }

    if (el == NULL || el->lType == OCODE_GBSEG)
        return 0;

    switch ((UCHAR)el->lType) {

    /* Fixed 2-byte orders. */
    case OCODE_GBAR: { /* X'68', one byte of flags.
                          Internal ORDER is {idCode, uchData}; the
                          flags live in abData[1]. */
        if (el->cbData < 2 || cbMax < 2) return 0;
        pb[0] = OCODE_GBAR;
        pb[1] = el->abData[1];
        return 2;
    }

    /* Long orders: code + length + data. */
    case OCODE_GCLINE:
    case OCODE_GCMRK:
    case OCODE_GCFLT:
    case OCODE_GCBEZ: {
        LONG   n;
        LONG   i;
        ULONG  cbData;
        PBYTE  p;

        if (el->lCount <= 0
            || el->cbData < el->lCount * (LONG)sizeof(POINTL))
            return 0;
        cbData = (ULONG)el->lCount * 4UL;
        if (cbData > 0xFF || cbMax < 2 + cbData) return 0;
        pb[0] = (BYTE)el->lType;
        pb[1] = (BYTE)cbData;
        p = pb + 2;
        n = el->lCount;
        for (i = 0; i < n; i++) {
            POINTL *ptl = (POINTL *)el->abData + i;
            PutSbin(p,     ptl->x);
            PutSbin(p + 2, ptl->y);
            p += 4;
        }
        return 2 + cbData;
    }

    case OCODE_GCCHST: { /* X'83', length = char count */
        ULONG n;
        if (el->cbData < 1 || el->lCount <= 0 || el->lCount > 255)
            return 0;
        n = (ULONG)el->lCount;
        if (cbMax < 2 + n) return 0;
        pb[0] = OCODE_GCCHST;
        pb[1] = (BYTE)n;
        memcpy(pb + 2, el->abData, n);
        return 2 + n;
    }

    case OCODE_GSCP: { /* X'21', POINTS */
        if (el->cbData < (LONG)sizeof(POINTL) || cbMax < 6) return 0;
        pb[0] = OCODE_GSCP;
        pb[1] = 4;
        PutSbin(pb + 2, ((POINTL *)el->abData)->x);
        PutSbin(pb + 4, ((POINTL *)el->abData)->y);
        return 6;
    }

    case OCODE_GCOMT: { /* X'01', length + raw data */
        ULONG n = (ULONG)el->cbData;
        if (n > 255 || cbMax < 2 + n) return 0;
        pb[0] = OCODE_GCOMT;
        pb[1] = (BYTE)n;
        if (n > 0) memcpy(pb + 2, el->abData, n);
        return 2 + n;
    }

    /* PM GPI Label, per pmord.h. Not in AFP GOCA. */
    case OCODE_GLABL: { /* X'D3', long format: 4-byte BE label value */
        LONG labelValue;
        if (el->cbData < (LONG)sizeof(LONG) || cbMax < 6) return 0;
        memcpy(&labelValue, el->abData, sizeof(LONG));
        pb[0] = OCODE_GLABL;
        pb[1] = 4;
        PutLongBE(pb + 2, (ULONG)labelValue);
        return 6;
    }

    case OCODE_GCBOX: { /* X'80', 10-byte payload */
        ORDERL_GCBOX *b = (ORDERL_GCBOX *)el->abData;
        if (el->cbData < (LONG)sizeof(ORDERL_GCBOX) || cbMax < 2 + 10)
            return 0;
        pb[0]  = OCODE_GCBOX;
        pb[1]  = 10;
        pb[2]  = b->fbFlags;
        pb[3]  = b->uchReserved;
        PutSbin(pb + 4,  b->ptCorner.x);
        PutSbin(pb + 6,  b->ptCorner.y);
        PutSbin(pb + 8,  b->hAxis);
        PutSbin(pb + 10, b->vAxis);
        return 2 + 10;
    }

    /* @note Non-canonical serialization. pmord.h's ORDERL_GCPARC
             stores the multiplier as FIXED (16.16, 4 bytes) and the
             angles as LONG (4 bytes each); canonical GCPARC uses
             MH/MFR (8.8, 2 bytes) for the multiplier and FIXED114
             (2.14, 2 bytes each) for the angles. Serialized here
             as-is (16-byte payload, matching pmord.h) so that
             round-trip within osFree PMGPI is exact. Converting to
             canonical GCPARC is deferred until pmord.h itself is
             aligned with the AFP GOCA Reference — otherwise the
             ORDERL_GCPARC and ORDERS_GCPARC structs within the same
             header would disagree. */
    case OCODE_GCPARC: { /* X'A3', 16-byte payload (pmord.h convention) */
        ORDERL_GCPARC *a = (ORDERL_GCPARC *)el->abData;
        if (el->cbData < (LONG)sizeof(ORDERL_GCPARC) || cbMax < 2 + 16)
            return 0;
        pb[0]  = OCODE_GCPARC;
        pb[1]  = 16;
        PutSbin(pb + 2,  a->ptCenter.x);
        PutSbin(pb + 4,  a->ptCenter.y);
        PutLongBE(pb + 6,  (ULONG)a->ufxMultiplier);
        PutLongBE(pb + 10, (ULONG)a->usStartAngle);
        PutLongBE(pb + 14, (ULONG)a->usSweepAngle);
        return 2 + 16;
    }

    case OCODE_GCARC: { /* X'86', 8-byte payload */
        ORDERL_GCARC *a = (ORDERL_GCARC *)el->abData;
        if (el->cbData < (LONG)sizeof(ORDERL_GCARC) || cbMax < 2 + 8)
            return 0;
        pb[0] = OCODE_GCARC;
        pb[1] = 8;
        PutSbin(pb + 2, a->ptInter.x);
        PutSbin(pb + 4, a->ptInter.y);
        PutSbin(pb + 6, a->ptEnd.x);
        PutSbin(pb + 8, a->ptEnd.y);
        return 2 + 8;
    }

    case OCODE_GBPTH: { /* X'D0', 4-byte BE idPath */
        ORDER_GBPTH *p = (ORDER_GBPTH *)el->abData;
        if (el->cbData < (LONG)sizeof(ORDER_GBPTH) || cbMax < 2 + 4)
            return 0;
        pb[0] = OCODE_GBPTH;
        pb[1] = 4;
        PutLongBE(pb + 2, (ULONG)p->idPath);
        return 2 + 4;
    }

    case OCODE_GFPTH: { /* X'D7', 6-byte payload */
        ORDER_GFPTH *p = (ORDER_GFPTH *)el->abData;
        if (el->cbData < (LONG)sizeof(ORDER_GFPTH) || cbMax < 2 + 6)
            return 0;
        pb[0] = OCODE_GFPTH;
        pb[1] = 6;
        pb[2] = p->fbFlags;
        pb[3] = p->uchReserved;
        PutLongBE(pb + 4, (ULONG)p->idPath);
        return 2 + 6;
    }

    case OCODE_GOPTH: { /* X'D4', 6-byte payload */
        ORDER_GOPTH *p = (ORDER_GOPTH *)el->abData;
        if (el->cbData < (LONG)sizeof(ORDER_GOPTH) || cbMax < 2 + 6)
            return 0;
        pb[0] = OCODE_GOPTH;
        pb[1] = 6;
        pb[2] = p->fbFlags;
        pb[3] = p->uchReserved;
        PutLongBE(pb + 4, (ULONG)p->idPath);
        return 2 + 6;
    }

    case OCODE_GMPTH: { /* X'D8', 6-byte payload */
        ORDER_GMPTH *p = (ORDER_GMPTH *)el->abData;
        if (el->cbData < (LONG)sizeof(ORDER_GMPTH) || cbMax < 2 + 6)
            return 0;
        pb[0] = OCODE_GMPTH;
        pb[1] = 6;
        pb[2] = p->uchMode;
        pb[3] = p->uchReserved;
        PutLongBE(pb + 4, (ULONG)p->idPath);
        return 2 + 6;
    }

    case OCODE_GSCPTH: { /* X'B4', 6-byte payload */
        ORDER_GSCPTH *p = (ORDER_GSCPTH *)el->abData;
        if (el->cbData < (LONG)sizeof(ORDER_GSCPTH) || cbMax < 2 + 6)
            return 0;
        pb[0] = OCODE_GSCPTH;
        pb[1] = 6;
        pb[2] = p->fbFlags;
        pb[3] = p->uchReserved;
        PutLongBE(pb + 4, (ULONG)p->idPath);
        return 2 + 6;
    }

    /* GCFARC per "Full Arc (GFARC, GCFARC) Orders", p. 107:
         offset 2 : MH   integer portion of scale factor (UBIN)
         offset 3 : MFR  fractional portion, MFR/256 (UBIN)
       Length field = 2. Total order = 4 bytes.
       Internal pmord.h stores a 16.16 FIXED. Convert:
         MH  = (FIXED >> 16) & 0xFF   (top byte of 16-bit integer part)
         MFR = (FIXED >>  8) & 0xFF   (top byte of 16-bit fractional part)
       Fractional precision is truncated from 16 to 8 bits, as
       required by the GOCA encoding. */
    case OCODE_GCFARC: {
        ULONG fx;
        if (el->cbData < (LONG)sizeof(FIXED) || cbMax < 4) return 0;
        fx    = (ULONG)*(FIXED *)el->abData;
        pb[0] = OCODE_GCFARC;
        pb[1] = 2;
        pb[2] = (BYTE)((fx >> 16) & 0xFF);
        pb[3] = (BYTE)((fx >>  8) & 0xFF);
        return 4;
    }

    case OCODE_GEPTH: { /* X'7F', length 0 */
        if (cbMax < 2) return 0;
        pb[0] = OCODE_GEPTH;
        pb[1] = 0;
        return 2;
    }

    case OCODE_GCFIG: { /* X'7D', length 0 */
        if (cbMax < 2) return 0;
        pb[0] = OCODE_GCFIG;
        pb[1] = 0;
        return 2;
    }

    case OCODE_GEAR: { /* X'60', length 0 */
        if (cbMax < 2) return 0;
        pb[0] = OCODE_GEAR;
        pb[1] = 0;
        return 2;
    }

    /* Extended format: X'FE' + qualifier + 2-byte BE length
       + ORDERL_GCCHSTE body (fixed part + string + optional adx). */
    case OCODEQ_GCCHSTE: {
        ORDERL_GCCHSTE *p = (ORDERL_GCCHSTE *)el->abData;
        LONG   cbFixed;
        LONG   cch;
        ULONG  cbData;
        LONG   hasVector;

        if (el->cbData < (LONG)sizeof(ORDERL_GCCHSTE)) return 0;
        cbFixed   = sizeof(ORDERL_GCCHSTE) - sizeof(CHAR) - sizeof(LONG);
        cch       = (p->cchString.HiByte << 8) | p->cchString.LoByte;
        hasVector = (el->cbData > cbFixed + cch);
        cbData    = (ULONG)(cbFixed + cch
                            + (hasVector ? cch * (LONG)sizeof(LONG) : 0));
        if (cbData > 0xFFFF || cbMax < 4 + cbData) return 0;

        pb[0] = 0xFE;
        pb[1] = OCODEQ_GCCHSTE;
        pb[2] = (BYTE)((cbData >> 8) & 0xFF);
        pb[3] = (BYTE)( cbData       & 0xFF);

        /* Fixed part (including cchString SWPUSHORT, already BE). */
        memcpy(pb + 4, p, cbFixed);

        /* String bytes. */
        if (cch > 0)
            memcpy(pb + 4 + cbFixed, (PBYTE)p + cbFixed, (ULONG)cch);

        /* Optional increment vector, each LONG big-endian. */
        if (hasVector) {
            PLONG pAdx = (PLONG)((PBYTE)p + cbFixed + cch);
            PBYTE pOut = pb + 4 + cbFixed + cch;
            LONG  i;
            for (i = 0; i < cch; i++) {
                PutLongBE(pOut, (ULONG)pAdx[i]);
                pOut += 4;
            }
        }
        return 4 + cbData;
    }

    default:
        return 0;   /* unsupported by GpiGetData */
    }
}

/*!
   @brief Convert one canonical GOCA order into an internal ELEMENT.

   Handles OSFREE_Q_USERELEM escapes, GLABL, and the same set of
   canonical orders as ElementToGoca.

   @param[in]  pb       Order buffer.
   @param[in]  cbOrder  Total order length (from GocaOrderLength).
   @param[out] ppEl     Receives a newly allocated ELEMENT, or NULL.

   @return TRUE on success, FALSE if the order is unsupported or
           malformed. Caller owns *ppEl and must DosFreeMem it.
*/
static BOOL GocaToElement(const BYTE *pb, ULONG cbOrder, PELEMENT *ppEl)
{
    UCHAR   code;
    ULONG   cbData;
    ULONG   allocSize;
    PELEMENT el;
    PBYTE   p;

    *ppEl = NULL;
    if (cbOrder < 1) return FALSE;
    code = pb[0];

    /* osFree user element escape. Covers user elements (lType > 0xFF)
       and the PM-GPI-only OCODE_GBEL / OCODE_GEEL bracket markers. */
    if (code == 0xFE && cbOrder >= 4 && pb[1] == OSFREE_Q_USERELEM) {
        LONG     lType, lCount, cbUser;
        PELEMENT uel;

        if (cbOrder < 16) return FALSE;
        lType  = (LONG)GetLongBE(pb + 4);
        lCount = (LONG)GetLongBE(pb + 8);
        cbUser = (LONG)GetLongBE(pb + 12);
        if (lType <= 0x00 || lCount < 0 || cbUser < 0)  return FALSE;
        if (cbOrder < 16 + (ULONG)cbUser)                return FALSE;
        uel = (PELEMENT)AllocMemZero(sizeof(ELEMENT) + (ULONG)cbUser);
        if (uel == NULL) return FALSE;
        uel->next   = NULL;
        uel->lType  = lType;
        uel->lCount = lCount;
        uel->cbData = cbUser;
        if (cbUser > 0)
            memcpy(uel->abData, pb + 16, (ULONG)cbUser);
        *ppEl = uel;
        return TRUE;
    }

    /* Determine cbData from the GOCA order. */
    cbData = 0;
    switch (code) {
    case 0x00:  /* GNOP1 */
        return TRUE;
    case 0x68:  /* GBAR, fixed 2-byte */
    case 0x81: case 0x82: case 0x85: case 0xA5:   /* GCLINE/GCMRK/GCFLT/GCBEZ */
    case 0x83:                                     /* GCCHST */
    case 0x21:                                     /* GSCP */
    case 0x01:                                     /* GCOMT */
    case 0xD3:                                     /* GLABL */
    case 0x80:                                     /* GCBOX */
    case 0xA3:                                     /* GCPARC */
    case 0x86:                                     /* GCARC */
    case 0xD0:                                     /* GBPTH */
    case 0xD7:                                     /* GFPTH */
    case 0xD4:                                     /* GOPTH */
    case 0xD8:                                     /* GMPTH */
    case 0xB4:                                     /* GSCPTH */
    case 0x87:                                     /* GCFARC */
    case 0x7F: case 0x7D: case 0x60:               /* GEPTH/GCFIG/GEAR */
        if (cbOrder < 2) return FALSE;
        cbData = pb[1];
        break;
    case 0xFE:  /* extended format (non-osFree qualifier) */
        if (cbOrder < 4) return FALSE;
        cbData = ((ULONG)pb[2] << 8) + pb[3];
        break;
    default:
        return FALSE;
    }

    if (code != 0xFE && cbOrder < 2 + cbData) return FALSE;
    if (code == 0xFE && cbOrder < 4 + cbData) return FALSE;

    /* Allocate enough for the internal ORDERL_* that will be written
       into abData. Some internal ORDERL_* are larger than the GOCA
       payload (e.g. ORDERL_GCBOX is 18 bytes, GOCA payload is 10;
       GCFARC payload is 2, internal FIXED is 4). Using a floor of
       32 bytes keeps every cast write in bounds. */
    allocSize = cbData > 32 ? cbData : 32;
    el = (PELEMENT)AllocMemZero(sizeof(ELEMENT) + allocSize);
    if (el == NULL) return FALSE;

    el->next   = NULL;
    el->lType  = (LONG)code;
    el->lCount = 1;
    el->cbData = (LONG)cbData;

    p = (code == 0xFE) ? (PBYTE)pb + 4 : (PBYTE)pb + 2;

    switch (code) {

    case 0x68:  /* GBAR: one byte of flags.
                   Restore {idCode, uchData} so ReplayElement's
                   ((ORDER *)abData)->uchData sees the flags. */
        if (cbData < 1) { DosFreeMem(el); return FALSE; }
        el->abData[0] = OCODE_GBAR;
        el->abData[1] = p[0];
        el->cbData    = 2;
        break;

    case 0x81: case 0x82: case 0x85: case 0xA5: {
        LONG  n = (LONG)cbData / 4;
        LONG  i;
        POINTL *ptl = (POINTL *)el->abData;
        if (cbData % 4 != 0) { DosFreeMem(el); return FALSE; }
        for (i = 0; i < n; i++) {
            ptl[i].x = GetSbin(p + i * 4);
            ptl[i].y = GetSbin(p + i * 4 + 2);
        }
        el->lCount = n;
        break;
    }

    case 0x83:  /* GCCHST */
        if (cbData > 0) memcpy(el->abData, p, cbData);
        el->lCount = (LONG)cbData;
        break;

    case 0x21: { /* GSCP */
        POINTL *ptl = (POINTL *)el->abData;
        if (cbData < 4) { DosFreeMem(el); return FALSE; }
        ptl->x = GetSbin(p);
        ptl->y = GetSbin(p + 2);
        break;
    }

    case 0x01:  /* GCOMT */
        if (cbData > 0) memcpy(el->abData, p, cbData);
        el->lCount = (LONG)cbData;
        break;

    case 0xD3: { /* GLABL */
        LONG labelValue;
        if (cbData < (ULONG)sizeof(LONG)) { DosFreeMem(el); return FALSE; }
        labelValue = (LONG)GetLongBE(p);
        memcpy(el->abData, &labelValue, sizeof(LONG));
        el->lCount = 1;
        break;
    }

    case 0x80: { /* GCBOX */
        ORDERL_GCBOX *b = (ORDERL_GCBOX *)el->abData;
        if (cbData < 10) { DosFreeMem(el); return FALSE; }
        b->fbFlags     = p[0];
        b->uchReserved = p[1];
        b->ptCorner.x  = GetSbin(p + 2);
        b->ptCorner.y  = GetSbin(p + 4);
        b->hAxis       = GetSbin(p + 6);
        b->vAxis       = GetSbin(p + 8);
        break;
    }

    case 0xA3: { /* GCPARC */
        ORDERL_GCPARC *a = (ORDERL_GCPARC *)el->abData;
        if (cbData < 16) { DosFreeMem(el); return FALSE; }
        a->ptCenter.x    = GetSbin(p);
        a->ptCenter.y    = GetSbin(p + 2);
        a->ufxMultiplier = (FIXED)GetLongBE(p + 4);
        a->usStartAngle  = (LONG)GetLongBE(p + 8);
        a->usSweepAngle  = (LONG)GetLongBE(p + 12);
        break;
    }

    case 0x86: { /* GCARC */
        ORDERL_GCARC *a = (ORDERL_GCARC *)el->abData;
        if (cbData < 8) { DosFreeMem(el); return FALSE; }
        a->ptInter.x = GetSbin(p);
        a->ptInter.y = GetSbin(p + 2);
        a->ptEnd.x   = GetSbin(p + 4);
        a->ptEnd.y   = GetSbin(p + 6);
        break;
    }

    case 0xD0: { /* GBPTH */
        ORDER_GBPTH *o = (ORDER_GBPTH *)el->abData;
        if (cbData < 4) { DosFreeMem(el); return FALSE; }
        o->idPath = (LONG)GetLongBE(p);
        break;
    }

    case 0xD7: { /* GFPTH */
        ORDER_GFPTH *o = (ORDER_GFPTH *)el->abData;
        if (cbData < 6) { DosFreeMem(el); return FALSE; }
        o->fbFlags     = p[0];
        o->uchReserved = p[1];
        o->idPath      = (LONG)GetLongBE(p + 2);
        break;
    }

    case 0xD4: { /* GOPTH */
        ORDER_GOPTH *o = (ORDER_GOPTH *)el->abData;
        if (cbData < 6) { DosFreeMem(el); return FALSE; }
        o->fbFlags     = p[0];
        o->uchReserved = p[1];
        o->idPath      = (LONG)GetLongBE(p + 2);
        break;
    }

    case 0xD8: { /* GMPTH */
        ORDER_GMPTH *o = (ORDER_GMPTH *)el->abData;
        if (cbData < 6) { DosFreeMem(el); return FALSE; }
        o->uchMode     = p[0];
        o->uchReserved = p[1];
        o->idPath      = (LONG)GetLongBE(p + 2);
        break;
    }

    case 0xB4: { /* GSCPTH */
        ORDER_GSCPTH *o = (ORDER_GSCPTH *)el->abData;
        if (cbData < 6) { DosFreeMem(el); return FALSE; }
        o->fbFlags     = p[0];
        o->uchReserved = p[1];
        o->idPath      = (LONG)GetLongBE(p + 2);
        break;
    }

    case 0x87: { /* GCFARC: length 2 (MH + MFR) on the wire;
                    4-byte FIXED 16.16 internally. cbData is
                    overridden to match the internal size. */
        FIXED fx;
        if (cbData < 2) { DosFreeMem(el); return FALSE; }
        fx = (FIXED)(((ULONG)p[0] << 24) | ((ULONG)p[1] << 16));
        *(FIXED *)el->abData = fx;
        el->cbData = sizeof(FIXED);
        break;
    }

    case 0x7F:  /* GEPTH */
    case 0x7D:  /* GCFIG */
    case 0x60:  /* GEAR */
        break;

    case 0xFE: { /* extended GCCHSTE */
        UCHAR qual = pb[1];
        ORDERL_GCCHSTE *p32;
        LONG cbFixed;
        LONG cch;
        PBYTE src;
        LONG hasVector;

        if (qual != OCODEQ_GCCHSTE) { DosFreeMem(el); return FALSE; }

        p32     = (ORDERL_GCCHSTE *)el->abData;
        cbFixed = sizeof(ORDERL_GCCHSTE) - sizeof(CHAR) - sizeof(LONG);
        if (cbData < (ULONG)cbFixed) { DosFreeMem(el); return FALSE; }

        memcpy(p32, p, cbFixed);
        cch = (p32->cchString.HiByte << 8) | p32->cchString.LoByte;
        src = p + cbFixed;

        if (cbData < (ULONG)(cbFixed + cch)) {
            DosFreeMem(el);
            return FALSE;
        }
        if (cch > 0)
            memcpy((PBYTE)p32 + cbFixed, src, (ULONG)cch);
        src += cch;

        hasVector = (cbData > (ULONG)(cbFixed + cch));
        if (hasVector) {
            PLONG pAdx = (PLONG)((PBYTE)p32 + cbFixed + cch);
            LONG  i;
            if (cbData < (ULONG)(cbFixed + cch + cch * sizeof(LONG))) {
                DosFreeMem(el);
                return FALSE;
            }
            for (i = 0; i < cch; i++) {
                pAdx[i] = (LONG)GetLongBE(src);
                src += 4;
            }
        }
        el->lType  = OCODEQ_GCCHSTE;
        el->lCount = 1;
        el->cbData = (LONG)cbData;
        break;
    }

    default:
        DosFreeMem(el);
        return FALSE;
    }

    *ppEl = el;
    return TRUE;
}

/*!
   @brief Serialize a segment body into a canonical GOCA stream.

   Iterates over the element list, converts each supported element
   via ElementToGoca, and writes into pbData starting at byte offset
   startOff. Unsupported elements (GBSEG header, unknown element
   types) are skipped.

   @param[in]  pps       Presentation space.
   @param[in]  seg       Segment to serialize.
   @param[in]  startOff  Byte offset within the stream.
   @param[in]  cbTarget  Size of pbData.
   @param[out] pbData    Destination buffer.

   @return Bytes written.
*/
static ULONG SerializeSegmentGoca(PPS pps, PSEGMENT seg, ULONG startOff,
                                  ULONG cbTarget, PBYTE pbData)
{
    PELEMENT el;
    ULONG    written = 0;
    ULONG    skip    = startOff;
    BYTE     ord[512];

    if (pps == NULL || seg == NULL) return 0;

    for (el = seg->firstElement; el != NULL; el = el->next) {
        ULONG cb;

        if (el->lType == OCODE_GBSEG) continue;

        cb = ElementToGoca(el, ord, sizeof(ord));
        if (cb == 0) continue;

        if (skip >= cb) {
            skip -= cb;
            continue;
        }

        {
            ULONG off;
            for (off = skip; off < cb && written < cbTarget; off++)
                pbData[written++] = ord[off];
        }
        skip = 0;
        if (written >= cbTarget) break;
    }
    return written;
}

/*!
   @brief Retrieve a segment body as a canonical GOCA order stream.

   @param[in]     hps      Presentation space handle.
   @param[in]     lSegid   Segment identifier (must be > 0).
   @param[in,out] plOffset Byte offset within the segment; advanced by
                           the number of bytes written.
   @param[in]     lFormat  Must be DFORM_NOCONV.
   @param[in]     lLength  Size of pbData.
   @param[out]    pbData   Destination buffer.

   @return Bytes returned, or GPI_ERROR on error.

   @note Only DFORM_NOCONV is supported: SBIN coordinate conversion
         for DFORM_S370SHORT / DFORM_PCSHORT / DFORM_PCLONG is not
         implemented. See OS/2 PM Reference, "GpiGetData", and
         AFP GOCA Reference, "Order Formats".
   @note The segment header (X'70' Begin Segment) is NOT part of the
         returned byte stream. See the file header for why.
*/
LONG APIENTRY GpiGetData(HPS hps, LONG lSegid, PLONG plOffset,
                         LONG lFormat, LONG lLength, PBYTE pbData)
{
    PSEGMENT seg;
    ULONG    written;

    INIT_PS(hps, GPI_ERROR);

    if (lFormat != DFORM_NOCONV) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR,
                                    PMERR_INV_GETDATA_CONTROL),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (plOffset == NULL || *plOffset < 0
        || pbData == NULL || lLength < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR,
                                    PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lLength > 64512) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_DATA_TOO_LONG),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    seg = FindSegment(pps, lSegid);
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_NOT_FOUND),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->pOpenSegment == seg) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_SEG_IS_CURRENT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    written = SerializeSegmentGoca(pps, seg, (ULONG)*plOffset,
                                   (ULONG)lLength, pbData);
    *plOffset += (LONG)written;
    return (LONG)written;
}

/*!
   @brief Append a canonical GOCA order stream to the current segment,
          or draw it, according to the actual drawing mode.

   Parses the buffer order by order. For each successfully parsed
   order calls GocaToElement; the resulting ELEMENT is replayed (if
   drawing) and/or recorded (if retaining). A trailing partial order
   is left unprocessed and *plCount is set to its first byte offset.

   @param[in]     hps      Presentation space handle.
   @param[in]     lFormat  DFORM_NOCONV / DFORM_S370SHORT /
                           DFORM_PCSHORT / DFORM_PCLONG. Non-NOCONV
                           values are accepted syntactically but no
                           coordinate conversion is performed.
   @param[in,out] plCount  On input: buffer length. On output: offset
                           of the first unprocessed byte.
   @param[in]     pbData   Canonical GOCA order stream.

   @return GPI_OK / GPI_HITS on success, GPI_ERROR on error.

   @note Invalid in SEGEM_REPLACE mode, and in DM_RETAIN without an
         open segment (see "GpiPutData - Errors").
*/
LONG APIENTRY GpiPutData(HPS hps, LONG lFormat, PLONG plCount, PBYTE pbData)
{
    ULONG off, total;
    ULONG am;

    INIT_PS(hps, GPI_ERROR);

    if (lFormat != DFORM_NOCONV && lFormat != DFORM_S370SHORT
        && lFormat != DFORM_PCSHORT && lFormat != DFORM_PCLONG) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR,
                                    PMERR_INV_PUTDATA_FORMAT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (plCount == NULL || pbData == NULL || *plCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR,
                                    PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (*plCount > 64512) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_DATA_TOO_LONG),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->pOpenSegment != NULL && pps->ulEditMode == SEGEM_REPLACE) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR,
                                    PMERR_INV_REPLACE_MODE_FUNC),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN && pps->pOpenSegment == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    am    = ActualDrawingMode(pps);
    total = (ULONG)*plCount;
    off   = 0;

    while (off < total) {
        ULONG    cb;
        PELEMENT el = NULL;

        cb = GocaOrderLength(pbData + off, total - off);
        if (cb == 0) break;   /* incomplete trailing order */

        if (!GocaToElement(pbData + off, cb, &el)) {
            /* Unsupported or malformed order: stop, do not consume it. */
            break;
        }

        if (el != NULL) {
            if (PS_IS_DRAWING(am) && pps->hdc != (HDC)0)
                ReplayElement(pps, el);
            if (PS_IS_RECORDING(am) && pps->pOpenSegment != NULL) {
                if (!RecordElement(pps, el->lType, el->lCount,
                                   el->cbData, el->abData)) {
                    DosFreeMem(el);
                    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR,
                                                PMERR_INSUFFICIENT_MEMORY),
                                    SEI_NOBEEP);
                    return GPI_ERROR;
                }
            }
            DosFreeMem(el);
        }

        off += cb;
    }

    *plCount = (LONG)off;
    return GPI_OK;
}

/* === Drawing ====================================================== */

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
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_SEG_NAME),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (lLastSegId < lFirstSegId) {
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

/* === Element editing ============================================== */

/*!
   @brief Begin a user-defined element in the current segment.

   Creates an OCODE_GBEL element carrying lType and pszDesc. The
   bracket is closed by GpiEndElement. The bracket state is tracked
   in SEGMENT.fInElement.

   @param[in] hps      Presentation space handle.
   @param[in] lType    User element type; any value > 0 (per
                       "GpiBeginElement - Parameters": 0 is
                       reserved for system use).
   @param[in] pszDesc  NUL-terminated description; truncated to fit
                       ORDER_GBEL.achDesc (251 bytes including NUL).

   @return TRUE on success, FALSE on error.

   @note At most one GpiBeginElement bracket may be open at a time;
         a nested GpiBeginElement reports PMERR_ALREADY_IN_ELEMENT.

   @see GpiEndElement, GpiElement
*/
BOOL APIENTRY GpiBeginElement(HPS hps, LONG lType, PSZ pszDesc)
{
    ORDER_GBEL ord;

    INIT_PS(hps, FALSE);

    if (pps->pOpenSegment == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (pps->pOpenSegment->fInElement) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_ALREADY_IN_ELEMENT),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (lType <= 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_ID),
                        SEI_NOBEEP);
        return FALSE;
    }

    memset(&ord, 0, sizeof(ord));
    ord.lElementType = lType;
    if (pszDesc != NULL) {
        LONG len = (LONG)strlen(pszDesc);
        if (len > (LONG)sizeof(ord.achDesc) - 1)
            len = (LONG)sizeof(ord.achDesc) - 1;
        memcpy(ord.achDesc, pszDesc, (ULONG)len);
        ord.achDesc[len] = '\0';
    }

    if (!RecordElement(pps, OCODE_GBEL, 1, sizeof(ord), &ord))
        return FALSE;

    pps->pOpenSegment->fInElement = TRUE;
    return TRUE;
}

/*!
   @brief End the currently open user-defined element.

   Records an OCODE_GEEL marker and clears SEGMENT.fInElement.

   @param[in] hps  Presentation space handle.

   @return TRUE on success, FALSE on error.

   @note Reports PMERR_NOT_IN_ELEMENT if no GpiBeginElement bracket
         is open.

   @see GpiBeginElement
*/
BOOL APIENTRY GpiEndElement(HPS hps)
{
    INIT_PS(hps, FALSE);

    if (pps->pOpenSegment == NULL ||
        !pps->pOpenSegment->fInElement) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_ELEMENT),
                        SEI_NOBEEP);
        return FALSE;
    }

    if (!RecordElement(pps, OCODE_GEEL, 0, 0, NULL))
        return FALSE;

    pps->pOpenSegment->fInElement = FALSE;
    return TRUE;
}

/*!
   @brief Unlink and free one element, updating the segment's
          first/last/count bookkeeping.

   @param[in] pps      Presentation space.
   @param[in] seg      Segment containing el.
   @param[in] el       Element to remove.
   @param[in] prev     Element immediately preceding el (NULL if el
                       is firstElement).
*/
static void RemoveElement(PPS pps, PSEGMENT seg, PELEMENT el,
                          PELEMENT prev)
{
    if (prev == NULL) seg->firstElement = el->next;
    else              prev->next        = el->next;

    if (seg->lastElement == el) seg->lastElement = prev;
    if (seg->currentElement == el) seg->currentElement = el->next;

    seg->lElementCount--;
    (void)pps;
    DosFreeMem(el);
}

/*!
   @brief Find the nth element of the segment body (0-based,
          skipping the GBSEG header).

   @param[in]  seg    Segment.
   @param[in]  idx    Zero-based element index.
   @param[out] pprev  Receives the preceding element, or NULL.

   @return Element, or NULL if idx is out of range.
*/
static PELEMENT ElementAt(PSEGMENT seg, LONG idx, PELEMENT *pprev)
{
    PELEMENT el, prev = NULL;

    if (seg == NULL || idx < 0) return NULL;
    el = seg->firstElement->next;   /* skip GBSEG header */
    while (el != NULL && idx > 0) {
        prev = el;
        el   = el->next;
        idx--;
    }
    if (pprev != NULL) *pprev = prev;
    return el;
}

/*!
   @brief Delete the element addressed by the element pointer.

   @param[in] hps  Presentation space handle.

   @return TRUE on success, FALSE on error.

   @note After deletion, the element pointer is left on the element
         that followed the deleted one (or NULL if it was the last).

   @see GpiDeleteElementRange, GpiDeleteElementsBetweenLabels
*/
BOOL APIENTRY GpiDeleteElement(HPS hps)
{
    PSEGMENT seg;
    PELEMENT el, prev = NULL;

    INIT_PS(hps, FALSE);

    seg = pps->pOpenSegment;
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }
    el = seg->currentElement;
    if (el == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NO_CURRENT_ELEMENT),
                        SEI_NOBEEP);
        return FALSE;
    }

    if (seg->firstElement->next != el) {
        prev = seg->firstElement;
        while (prev->next != NULL && prev->next != el)
            prev = prev->next;
    }

    RemoveElement(pps, seg, el, prev);
    return TRUE;
}

/*!
   @brief Delete a range of elements by index.

   @param[in] hps     Presentation space handle.
   @param[in] lFirst  First element index (0-based, >= 0).
   @param[in] lLast   Last element index (inclusive, <= lElementCount-1).

   @return TRUE on success, FALSE on error.

   @see GpiDeleteElement, GpiDeleteElementsBetweenLabels
*/
BOOL APIENTRY GpiDeleteElementRange(HPS hps, LONG lFirst, LONG lLast)
{
    PSEGMENT seg;
    PELEMENT el, prev = NULL;
    LONG     i;

    INIT_PS(hps, FALSE);

    seg = pps->pOpenSegment;
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (lFirst < 0 || lLast < lFirst || lLast >= seg->lElementCount) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_ELEMENT_POINTER),
                        SEI_NOBEEP);
        return FALSE;
    }

    /* Locate prev of lFirst once, then delete (lLast - lFirst + 1)
       elements in a row from that position. */
    el = ElementAt(seg, lFirst, &prev);
    if (el == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_ELEMENT_POINTER),
                        SEI_NOBEEP);
        return FALSE;
    }

    for (i = lFirst; i <= lLast; i++) {
        PELEMENT next = el->next;
        RemoveElement(pps, seg, el, prev);
        el = next;
    }
    return TRUE;
}

/*!
   @brief Delete elements between two labels.

   The labels themselves are not removed. The first occurrence of
   lFirstLabel and the first subsequent occurrence of lLastLabel are
   used. All elements strictly between them are deleted.

   @param[in] hps          Presentation space handle.
   @param[in] lFirstLabel  Label preceding the region to delete.
   @param[in] lLastLabel   Label following the region to delete.

   @return TRUE on success, FALSE on error.

   @note If either label is not found, PMERR_LABEL_NOT_FOUND. If
         lFirstLabel == lLastLabel, elements between two consecutive
         occurrences of that label are deleted.

   @see GpiDeleteElement, GpiDeleteElementRange, GpiLabel
*/
BOOL APIENTRY GpiDeleteElementsBetweenLabels(HPS hps,
                                             LONG lFirstLabel,
                                             LONG lLastLabel)
{
    PSEGMENT seg;
    PELEMENT el;
    PELEMENT firstLab = NULL, lastLab = NULL;
    PELEMENT prev;

    INIT_PS(hps, FALSE);

    seg = pps->pOpenSegment;
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }

    prev = NULL;
    for (el = seg->firstElement->next; el != NULL; el = el->next) {
        if (el->lType == OCODE_GLABL && el->cbData >= (LONG)sizeof(LONG)) {
            LONG lbl;
            memcpy(&lbl, el->abData, sizeof(LONG));
            if (firstLab == NULL) {
                if (lbl == lFirstLabel) {
                    firstLab = el;
                }
            } else if (lastLab == NULL && lbl == lLastLabel) {
                lastLab = el;
            }
            if (firstLab != NULL && lastLab != NULL) break;
        }
        prev = el;
    }

    if (firstLab == NULL || lastLab == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_LABEL_NOT_FOUND),
                        SEI_NOBEEP);
        return FALSE;
    }

    /* Walk the list from firstLab->next until lastLab, deleting
       each element in between. */
    prev = firstLab;
    el   = firstLab->next;
    while (el != NULL && el != lastLab) {
        PELEMENT next = el->next;
        RemoveElement(pps, seg, el, prev);
        el = next;
    }
    return TRUE;
}

/*!
   @brief Move the element pointer to the element following a label.

   @param[in] hps     Presentation space handle.
   @param[in] lLabel  Label value to search for.

   @return TRUE on success, FALSE on error.

   @note Reports PMERR_LABEL_NOT_FOUND if no OCODE_GLABL with the
         given value exists in the open segment.

   @see GpiLabel, GpiSetElementPointer
*/
BOOL APIENTRY GpiSetElementPointerAtLabel(HPS hps, LONG lLabel)
{
    PSEGMENT seg;
    PELEMENT el;

    INIT_PS(hps, FALSE);

    seg = pps->pOpenSegment;
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return FALSE;
    }

    for (el = seg->firstElement->next; el != NULL; el = el->next) {
        if (el->lType == OCODE_GLABL && el->cbData >= (LONG)sizeof(LONG)) {
            LONG lbl;
            memcpy(&lbl, el->abData, sizeof(LONG));
            if (lbl == lLabel) {
                seg->currentElement = el->next;
                return TRUE;
            }
        }
    }

    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_LABEL_NOT_FOUND),
                    SEI_NOBEEP);
    return FALSE;
}

/*!
   @brief Query an element's data by index.

   @param[in]  hps       Presentation space handle.
   @param[in]  lElement  Zero-based element index (0 .. lElementCount-1).
   @param[in]  lLength   Size of pbData.
   @param[out] pbData    Receives the element payload. Only the first
                         min(lLength, cbData) bytes are copied.

   @return Element data length in bytes (>= 0), or GPI_ERROR on error.

   @note For an OCODE_GBEL / OCODE_GEEL marker the payload returned
         is the raw ORDER_GBEL / empty block; use GpiQueryElementType
         to obtain the user element type and description.
*/
LONG APIENTRY GpiQueryElement(HPS hps, LONG lElement,
                              LONG lLength, PBYTE pbData)
{
    PSEGMENT seg;
    PELEMENT el;

    INIT_PS(hps, GPI_ERROR);

    if (lLength < 0 || (lLength > 0 && pbData == NULL)) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    seg = pps->pOpenSegment;
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    el = ElementAt(seg, lElement, NULL);
    if (el == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_ELEMENT_POINTER),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    if (lLength > 0 && el->cbData > 0) {
        LONG n = (el->cbData < lLength) ? el->cbData : lLength;
        memcpy(pbData, el->abData, (ULONG)n);
    }
    return el->cbData;
}

/*!
   @brief Query the type and description of the current element.

   If the element pointer addresses an OCODE_GBEL marker, the
   element's type and description are taken from ORDER_GBEL. Any
   other element reports type 0 and an empty description.

   @param[in]  hps       Presentation space handle.
   @param[out] plType    Receives the element type (0 for non-user
                         elements).
   @param[in]  lLength   Size of pszDesc.
   @param[out] pszDesc   Receives the NUL-terminated description.

   @return Element data length in bytes, or GPI_ERROR on error.

   @note Reports PMERR_NOT_IN_SEG when no segment is open; the
         current element may be NULL, in which case type 0 and an
         empty description are returned with length 0.
*/
LONG APIENTRY GpiQueryElementType(HPS hps, PLONG plType,
                                  LONG lLength, PSZ pszDesc)
{
    PSEGMENT seg;
    PELEMENT el;

    INIT_PS(hps, GPI_ERROR);

    if (plType == NULL || lLength < 0
        || (lLength > 0 && pszDesc == NULL)) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    seg = pps->pOpenSegment;
    if (seg == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_NOT_IN_SEG),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    el = seg->currentElement;

    *plType = 0;
    if (lLength > 0) pszDesc[0] = '\0';

    if (el == NULL) return 0;

    if (el->lType == OCODE_GBEL && el->cbData >= (LONG)sizeof(ORDER_GBEL)) {
        ORDER_GBEL *ord = (ORDER_GBEL *)el->abData;
        LONG        len;

        *plType = ord->lElementType;
        if (lLength > 0) {
            len = (LONG)strlen(ord->achDesc);
            if (len > lLength - 1) len = lLength - 1;
            memcpy(pszDesc, ord->achDesc, (ULONG)len);
            pszDesc[len] = '\0';
        }
    }
    return el->cbData;
}
