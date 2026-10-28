#ifndef PS_H_INCLUDED
#define PS_H_INCLUDED

#define PS_MAGIC            0x50535153UL  /* 'PSSQ' */

#ifndef SEGEM_ERROR
/* Edit modes used by GpiSet/QueryEditMode. Not present in pmgpi.h,
   so kept here as private constants. */
#define SEGEM_ERROR                         0L
#define SEGEM_INSERT                        1L
#define SEGEM_REPLACE                       2L
#endif

/* -----------------------------------------------------------------
 * Attribute storage.
 *
 * ATTR_* identifiers and ATTR_ON/ATTR_OFF values come from
 * <pmgpi.h>. They are NOT bit masks: for example ATTR_CHAINED == 6
 * (which would overlap ATTR_VISIBLE == 2 if used directly as a bit
 * mask). To keep one independent bit per attribute in the internal
 * ulInitialSegAttrs word, shift 1 by the identifier value:
 *
 *     bit 1  = ATTR_DETECTABLE
 *     bit 2  = ATTR_VISIBLE
 *     bit 6  = ATTR_CHAINED
 *     bit 8  = ATTR_DYNAMIC
 *     bit 9  = ATTR_FASTCHAIN
 *     bit 10 = ATTR_PROP_DETECTABLE
 *     bit 11 = ATTR_PROP_VISIBLE
 *
 * This bit-set form is used only for PS.ulInitialSegAttrs (defaults).
 * Per-segment attributes live in the GBSEG header; see SegmentGetAttr
 * and SegmentSetAttr below.
 *
 * See OS/2 Presentation Manager Programming Reference Vol.1,
 * "GpiQuerySegmentAttrs - Parameters" and
 * "GpiSetSegmentAttrs - Parameters".
 *
 * DO NOT redefine ATTR_* here.
 * ----------------------------------------------------------------- */
#define SEGATTR_BIT(a)  (1UL << (ULONG)(a))

/*!
   @brief One recorded segment element.

   lType is a GOCA order code (OCODE_*) from <pmord.h>, or a user-
   defined element type > 0xFF passed to GpiElement. abData holds
   [LONG lCount][item0]...[itemN-1] where each item is cbItemSize bytes.
   This is NOT the on-disk GOCA format — it is a compact internal
   representation sufficient for replay via Gre* APIs.

   The first ELEMENT of every segment is always OCODE_GBSEG with a
   payload of ORDERL_GBSEG (see pmord.h). It carries the segment name
   (ulName), the SC31-6804 FLAG1 attributes, and the AFPC-0008-03
   FLAG2 CHAIN bit. That header is NOT counted in lElementCount.
*/
typedef struct _ELEMENT {
    struct _ELEMENT *next;
    LONG   lType;      /* OCODE_* from <pmord.h>; > 0xFF for user elements */
    LONG   lCount;
    LONG   cbData;
    BYTE   abData[1];
} ELEMENT, *PELEMENT;

/*!
   @brief One retained segment.

   The segment chain is kept in priority order: firstSegment points to
   the lowest-priority segment and next links toward higher priority.
   This is the order required by GpiDrawChain (OS/2 PM Programming
   Reference Vol.1, "GpiDrawChain - Remarks": "the lowest priority
   chained graphic segment starts to draw ... until the highest
   priority graphic segment is drawn"). prev links toward lower
   priority and is used by GpiSetSegmentPriority.

   No lSegId or ulAttrs fields are kept here. The segment name and
   attributes live in the OCODE_GBSEG header of firstElement; use
   SegmentGetName, SegmentGetAttr, and SegmentSetAttr to access them.

   fInElement tracks the GpiBeginElement / GpiEndElement bracket
   state (see gpi32segments.c). It is cleared by CreateSegment,
   FreeSegmentElements, and GpiCloseSegment.
*/
typedef struct _SEGMENT {
    struct _SEGMENT *next;          /* toward higher priority */
    struct _SEGMENT *prev;          /* toward lower priority  */
    PELEMENT firstElement;          /* always OCODE_GBSEG header */
    PELEMENT lastElement;
    PELEMENT currentElement;
    LONG     lElementCount;         /* does not count GBSEG header */
    BOOL     fInElement;            /* GpiBeginElement/EndElement bracket */
} SEGMENT, *PSEGMENT;

#define PS_IS_RECORDING(m)  ((m) == DM_RETAIN || (m) == DM_DRAWANDRETAIN)
#define PS_IS_DRAWING(m)    ((m) == DM_DRAW   || (m) == DM_DRAWANDRETAIN)

/*!
   @brief One saved presentation-space state (GpiSavePS/GpiRestorePS).

   A coarse snapshot of PS-level state. DC-level state (attributes,
   transforms) is handled by the driver and is not saved here; the
   OS/2 PM Reference says GpiSavePS saves only the state that belongs
   to the PS itself.

   Entries form a LIFO stack. IDs are assigned sequentially starting
   from 1 by GpiSavePS. GpiRestorePS(lPSid) discards the entry with
   that ID and all later entries (per "GpiRestorePS": "the saved data
   and all the later saved data are removed").

   @see GpiSavePS, GpiRestorePS
*/
typedef struct _PSSAVE {
    struct _PSSAVE *next;           /* toward older save */
    ULONG    ulDrawingMode;
    ULONG    ulDrawControl;
    ULONG    ulStopDraw;
    ULONG    ulEditMode;
    ULONG    ulInitialSegAttrs;
    SIZEL    sizlPS;
} PSSAVE, *PPSSAVE;

/*!
   @brief Presentation space (PS) instance data.

   PMGPI-level state associated with an HPS. Only PS-scoped state lives
   here — DC-scoped state (current position, attributes, clip, bounds,
   correlation) is held by the presentation driver in its DC instance
   data. The Gre* DDI macros in pmddim.h pass 0L for pInstance; the
   graphics engine substitutes the correct instance data by hdc.
*/
typedef struct _PS {
    ULONG    ulMagic;          /*!< PS_MAGIC — validation */
    HPS      hpsSelf;          /*!< self-reference */
    HDC      hdc;              /*!< associated DC; 0 = no association */
    ULONG    ulDrawingMode;    /*!< DM_DRAW / DM_RETAIN / DM_DRAWANDRETAIN */

    /* Segment support */
    PSEGMENT firstSegment;     /*!< lowest-priority segment in chain */
    PSEGMENT lastSegment;      /*!< highest-priority segment in chain */
    PSEGMENT pOpenSegment;
    ULONG    ulEditMode;
    ULONG    ulInitialSegAttrs;

    /* Draw control (GpiSetDrawControl / GpiQueryDrawControl).
       Bit N (N = lControl - 1) corresponds to DCTL_ERASE + N. */
    ULONG    ulDrawControl;

    /* Stop draw (GpiSetStopDraw / GpiQueryStopDraw). SDW_OFF or SDW_ON. */
    ULONG    ulStopDraw;

    /* PS size (GpiSetPS / GpiQueryPS). */
    SIZEL    sizlPS;

    /* Saved-state stack (GpiSavePS / GpiRestorePS). */
    PPSSAVE  pSaveStack;       /*!< most recent save, LIFO */
    LONG     cSaves;           /*!< number of entries in pSaveStack */
    LONG     lNextSaveId;      /*!< next save id to assign (starts at 1) */
} PS, *PPS;

#define PS_FROM_HPS(h)  ((PPS)(h))
#define HPS_FROM_PS(p)  ((HPS)(p))

/*!
   @brief Look up a PS by its HPS.

   Validates the magic and self-reference. On failure calls
   WinSetErrorInfo with PMERR_INV_HPS and returns NULL — callers
   only need to check for NULL and return their error value.

   Does NOT require the PS to have an associated DC. Editing
   functions (GpiSetEditMode, GpiSetElementPointer, GpiLabel,
   GpiSetSegmentPriority, GpiAssociate(hps, 0), ...) must be usable
   on a PS without a DC, so they use this function, not LookupPSDC.

   @param[in] hps  Handle to look up.

   @return Pointer to PS, or NULL (error info set).
*/
PPS LookupPS(HPS hps);

/*!
   @brief Look up a PS and require it to have an associated DC.

   Calls LookupPS first. If the PS has no DC, calls WinSetErrorInfo
   with PMERR_INV_HDC and returns NULL. Drawing primitives and any
   function that dispatches into Gre* must use this, not LookupPS.

   @param[in] hps  Handle to look up.

   @return Pointer to PS with a non-zero hdc, or NULL (error info set).
*/
PPS LookupPSDC(HPS hps);

/*!
   @brief Compute the actual drawing mode.

   The value set by GpiSetDrawingMode is only a desired mode; the
   actual behaviour depends on the current context, per OS/2 PM
   Programming Reference Vol.1, "Actual Drawing Mode":

     - Outside a segment          : always DM_DRAW
     - Inside an unchained segment: always DM_RETAIN
     - Inside a chained segment   : whatever GpiSetDrawingMode set

   A segment is chained when its ATTR_CHAINED attribute is ON. See
   "GpiSetSegmentAttrs - Parameters" for the attribute.

   If the segment header cannot be parsed (SegmentGetAttr returns
   GPI_ERROR, which coincides with ATTR_OFF == 0), the segment is
   treated as unchained, i.e. the function returns DM_RETAIN. This
   is the safest interpretation for a corrupted header.

   @param[in] pps  Presentation space (may be NULL).

   @return One of DM_DRAW, DM_RETAIN, DM_DRAWANDRETAIN.
*/
ULONG ActualDrawingMode(PPS pps);

/* Segment internals (implemented in segments.c) */
PSEGMENT CreateSegment(PPS pps, LONG lSegId, ULONG ulAttrs);
PSEGMENT FindSegment(PPS pps, LONG lSegId);
void     FreeSegmentElements(PSEGMENT seg);
void     FreeAllSegments(PPS pps);

/*!
   @brief Retrieve the segment name from the GBSEG header.

   The name is stored in ORDERL_GBSEG.ulName (offset 2–5 of the GBSEG
   order). Per IBM SC31-6804, "Begin Segment Command", NAME is a
   4-byte UBIN.

   @param[in] seg  Segment to query.

   @return 4-byte segment name, or 0 on malformed segment.
*/
LONG SegmentGetName(PSEGMENT seg);

/*!
   @brief Query a segment attribute from the GBSEG header.

   Mapping (IBM SC31-6804 FLAG1 + AFPC-0008-03 FLAG2):

     ATTR_VISIBLE          <-> !FLAG1 bit 0 (INVISIBLE)
     ATTR_PROP_VISIBLE     <-> !FLAG1 bit 1 (PROPINV)
     ATTR_DETECTABLE       <->  FLAG1 bit 2 (PICK)
     ATTR_PROP_DETECTABLE  <->  FLAG1 bit 3 (PROPICK)
     ATTR_DYNAMIC          <->  FLAG1 bit 6 (DYNAMIC)
     ATTR_FASTCHAIN        <->  FLAG1 bit 7 (FASTCH)
     ATTR_CHAINED          <-> !FLAG2 bit 0 (CHAIN)

   HILITE/PROPHI (FLAG1 bits 4–5) have no ATTR_* equivalent in
   <pmgpi.h> and are not exposed here.

   See IBM SC31-6804, "Begin Segment (GBSEG) Order", pp. 102–103,
   and AFPC-0008-03, "Begin Segment Command", p. 75.

   @param[in] seg    Segment to query.
   @param[in] lAttr  ATTR_* identifier.

   @return ATTR_ON, ATTR_OFF, or GPI_ERROR.
*/
LONG SegmentGetAttr(PSEGMENT seg, LONG lAttr);

/*!
   @brief Set a segment attribute in the GBSEG header.

   Same mapping as SegmentGetAttr.

   @param[in] seg     Segment to modify.
   @param[in] lAttr   ATTR_* identifier.
   @param[in] lValue  ATTR_ON or ATTR_OFF.

   @return TRUE on success, FALSE on invalid attribute.
*/
BOOL SegmentSetAttr(PSEGMENT seg, LONG lAttr, LONG lValue);

BOOL     RecordElement(PPS pps, LONG lType, LONG lCount, LONG cbData,
                       const void *pData);
BOOL     RecordCharStringPos(PPS pps, ULONG flOptions, LONG cChars,
                             PCH pchString, PRECTL prclRect, PLONG pAdx);
BOOL     RecordPolyFilletSharp(PPS pps, LONG lCount, PPOINTL aptlPoints,
                               PFIXED afxSharp);
BOOL     ReplayElement(PPS pps, PELEMENT el);
BOOL     ReplaySegment(PPS pps, PSEGMENT seg);

/*!
   @brief Insert a segment into the chain in priority order.

   If lRefSegId is 0, position depends on lOrder:
     LOWER_PRI  -> end of chain (highest priority)
     HIGHER_PRI -> beginning of chain (lowest priority)

   If lRefSegId is non-zero, the segment is placed immediately before
   (LOWER_PRI) or after (HIGHER_PRI) the reference segment.

   @param[in] pps       Presentation space.
   @param[in] seg       Segment to insert (must already be unlinked).
   @param[in] lRefSegId Reference segment identifier (0 = chain end).
   @param[in] lOrder    LOWER_PRI or HIGHER_PRI.

   @return TRUE on success, FALSE if lRefSegId is not found.

   @see GpiSetSegmentPriority
*/
BOOL InsertSegmentByPriority(PPS pps, PSEGMENT seg,
                             LONG lRefSegId, LONG lOrder);

/* ==================================================================
 * Boilerplate macros
 *
 * The three INIT_* macros declare the `pps` (and, for INIT_DRAWING,
 * also the `am`) locals and validate the HPS/DC in one line. They
 * MUST be the last thing before the first statement of the function
 * (C89: declarations-before-statements). Callers add the trailing
 * semicolon.
 *
 * After INIT_DRAWING, `pps` and `am` are both visible; `am` is the
 * actual drawing mode computed by ActualDrawingMode, so all further
 * PS_IS_RECORDING / PS_IS_DRAWING checks MUST use `am`, not
 * `pps->ulDrawingMode`.
 * ================================================================== */

/*!
   @brief Declare and initialize a PS local (no DC required).

   Expands to:
     PPS pps = LookupPS(hps);
     if (pps == NULL) return (errval);

   Use in editing functions that work on a PS without an associated
   DC (GpiSetEditMode, GpiSetElementPointer, GpiLabel, GpiElement,
   GpiSetInitialSegmentAttrs, GpiSetSegmentPriority, GpiAssociate,
   GpiSetDrawingMode, GpiQueryDrawingMode, ...).

   @param[in] hps     Presentation space handle (function argument).
   @param[in] errval  Value to return on bad hps (FALSE, GPI_ERROR,
                      0, (HBITMAP)0, ...).

   @see LookupPS
*/
#define INIT_PS(hps, errval) \
    PPS pps = LookupPS(hps); \
    do { \
        if (pps == NULL) return (errval); \
    } while (0)

/*!
   @brief Declare and initialize a PS local with a DC (no mode).

   Expands to:
     PPS pps = LookupPSDC(hps);
     if (pps == NULL) return (errval);

   Use in functions that dispatch into Gre* or otherwise need a DC
   but do not compute an actual drawing mode (GpiQueryCurrentPosition,
   GpiSetCurrentPosition, GpiDrawChain, GpiDrawFrom, GpiDrawSegment,
   GpiDrawDynamics, GpiRemoveDynamics, GpiErase, ...).

   @param[in] hps     Presentation space handle (function argument).
   @param[in] errval  Value to return on bad hps / missing DC.

   @see LookupPSDC
*/
#define INIT_PSDC(hps, errval) \
    PPS pps = LookupPSDC(hps); \
    do { \
        if (pps == NULL) return (errval); \
    } while (0)

/*!
   @brief Declare and initialize PS/mode locals for a drawing primitive.

   Expands to:
     PPS   pps = LookupPSDC(hps);
     ULONG am;
     if (pps == NULL) return (errval);
     am = ActualDrawingMode(pps);

   Use in every drawing primitive that records and/or dispatches into
   Gre* — GpiLine, GpiBox, GpiFullArc, GpiPartialArc, GpiPointArc,
   GpiMove, GpiMarker, GpiPolyMarker, GpiPolyFillet, GpiPolySpline,
   GpiCharString, GpiPolyLine, all GpiBegin/End/…Path functions, etc.

   `am` is already the actual drawing mode, so all subsequent
   PS_IS_RECORDING / PS_IS_DRAWING checks must use `am`, not
   `pps->ulDrawingMode`.

   @param[in] hps     Presentation space handle (function argument).
   @param[in] errval  Value to return on bad hps / missing DC.

   @see LookupPSDC, ActualDrawingMode, RECORD_OR_DISPATCH
*/
#define INIT_DRAWING(hps, errval) \
    PPS   pps = LookupPSDC(hps); \
    ULONG am; \
    do { \
        if (pps == NULL) return (errval); \
        am = ActualDrawingMode(pps); \
    } while (0)

/*!
   @brief Record an element if recording, then decide whether to dispatch.

   Encapsulates the "record primitive" half of every drawing primitive:

     1. If ActualDrawingMode is a recording mode, call RecordElement.
        On failure, return errval.
     2. If the mode is recording-only (not drawing), return okval
        immediately (no dispatch).
     3. Otherwise fall through — the caller proceeds to call Gre*.

   Callers must have already obtained `pps` (via LookupPSDC) and `am`
   (via ActualDrawingMode). This macro expands a `return`, so it must
   be used inside a function whose return type matches okval/errval.
   Do not use it in a function that needs cleanup before returning.

   @param[in] pps    Presentation space.
   @param[in] am     Actual drawing mode (from ActualDrawingMode).
   @param[in] lType  GOCA order code (OCODE_*).
   @param[in] lCount Number of items in payload.
   @param[in] cbData Payload size in bytes.
   @param[in] pData  Payload pointer (may be NULL when cbData == 0).
   @param[in] okval  Return value when dispatch is not needed
                     (GPI_OK or TRUE).
   @param[in] errval Return value on RecordElement failure
                     (GPI_ERROR or FALSE).

   @note Not usable in GpiCharStringAt (writes GSCP + GCCHST),
         GpiCharStringPos / GpiCharStringPosAt (custom recorder),
         GpiPolyFilletSharp (custom recorder), or GpiComment
         (conditional record). Those functions keep the explicit
         if/else pattern.

   @see RecordElement, ActualDrawingMode
*/
#define RECORD_OR_DISPATCH(pps, am, lType, lCount, cbData, pData, \
                           okval, errval) \
    do { \
        if (PS_IS_RECORDING(am)) { \
            if (!RecordElement((pps), (lType), (lCount), (cbData), (pData))) \
                return (errval); \
            if (!PS_IS_DRAWING(am)) \
                return (okval); \
        } \
    } while (0)

#endif /* PS_H_INCLUDED */
