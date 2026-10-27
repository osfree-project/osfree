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
 * ulAttrs word, shift 1 by the identifier value:
 *
 *     bit 1  = ATTR_DETECTABLE
 *     bit 2  = ATTR_VISIBLE
 *     bit 6  = ATTR_CHAINED
 *     bit 8  = ATTR_DYNAMIC
 *     bit 9  = ATTR_FASTCHAIN
 *     bit 10 = ATTR_PROP_DETECTABLE
 *     bit 11 = ATTR_PROP_VISIBLE
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
*/
typedef struct _SEGMENT {
    struct _SEGMENT *next;          /* toward higher priority */
    struct _SEGMENT *prev;          /* toward lower priority  */
    LONG     lSegId;
    ULONG    ulAttrs;               /* SEGATTR_BIT(ATTR_*) set */
    PELEMENT firstElement;
    PELEMENT lastElement;
    PELEMENT currentElement;
    LONG     lElementCount;
} SEGMENT, *PSEGMENT;

#define PS_IS_RECORDING(m)  ((m) == DM_RETAIN || (m) == DM_DRAWANDRETAIN)
#define PS_IS_DRAWING(m)    ((m) == DM_DRAW   || (m) == DM_DRAWANDRETAIN)

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

/* Segment internals (implemented in segments.c) */
PSEGMENT CreateSegment(PPS pps, LONG lSegId, ULONG ulAttrs);
PSEGMENT FindSegment(PPS pps, LONG lSegId);
void     FreeSegmentElements(PSEGMENT seg);
void     FreeAllSegments(PPS pps);
BOOL     RecordElement(PPS pps, LONG lType, LONG lCount, LONG cbData,
                       const void *pData);
BOOL     RecordCharStringPos(PPS pps, ULONG flOptions, LONG cChars,
                             PCH pchString, PRECTL prclRect, PLONG pAdx);
BOOL     RecordPolyFilletSharp(PPS pps, LONG lCount, PPOINTL aptlPoints,
                               PFIXED afxSharp);
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

#endif /* PS_H_INCLUDED */
