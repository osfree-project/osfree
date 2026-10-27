#ifndef PS_H_INCLUDED
#define PS_H_INCLUDED

#define PS_MAGIC            0x50535153UL  /* 'PSSQ' */

#ifndef SEGEM_ERROR
/* edit modes used by GpiSet/QueryEditMode */
#define SEGEM_ERROR                         0L
#define SEGEM_INSERT                        1L
#define SEGEM_REPLACE                       2L
#endif

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

typedef struct _SEGMENT {
    struct _SEGMENT *next;
    LONG     lSegId;
    ULONG    ulAttrs;
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
    PSEGMENT firstSegment;
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
BOOL RecordElement(PPS pps, LONG lType, LONG lCount, LONG cbData,
                   const void *pData);
BOOL RecordCharStringPos(PPS pps, ULONG flOptions, LONG cChars,
                         PCH pchString, PRECTL prclRect, PLONG pAdx);
BOOL RecordPolyFilletSharp(PPS pps, LONG lCount, PPOINTL aptlPoints,
                           PFIXED afxSharp);
BOOL     ReplaySegment(PPS pps, PSEGMENT seg);

#endif /* PS_H_INCLUDED */
