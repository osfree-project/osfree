/*!
   @file

   @ingroup pm

   @brief Presentation Manager GPI primitive drawing functions (32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   Included by pmgpi.c. Provides 32-bit implementations of primitive
   drawing functions: boxes, full/partial arcs, lines, markers,
   fillets, splines, character strings and comments. Each function
   validates its parameters and dispatches into the presentation
   driver through the corresponding Gre* DDI macro in pmddim.h.

   Each function follows the same validation sequence:

     1. LookupPS(hps)            -> PMERR_INV_HPS on failure
     2. pps->hdc != 0            -> PMERR_INV_HDC on failure
     3. parameter checks         -> PMERR_INV_* on failure
     4. DM_RETAIN check          -> no-op success (see note below)
     5. dispatch into the presentation driver via Gre* DDI

   DM_RETAIN note: in retain mode primitives must be recorded into the
   current segment and NOT dispatched to the driver. Segment support is
   not yet implemented, so retain mode is currently a no-op. This is
   forward-compatible: when segments land, block (4) will be replaced
   with segment-recording calls.

   Note on pInstance: the Gre* macros in pmddim.h pass 0L for the
   pInstance argument; the graphics engine substitutes the correct
   instance data associated with the hdc automatically.
*/

/* Compatibility definitions.

   These symbols are part of the OS/2 PM API (BOX_INTERIOR, BOX_BOUNDARY,
   CHS_START_XY, CSP_INFO, PCSP_INFO) and PM DDI (BOXPARAMS, PBOXPARAMS)

   @TODO: remove once the system headers are complete. */

#ifndef BOX_INTERIOR
#define BOX_INTERIOR  0x0001
#endif
#ifndef BOX_BOUNDARY
#define BOX_BOUNDARY  0x0002
#endif
#ifndef CHS_START_XY
#define CHS_START_XY  0x0010
#endif

typedef struct _CSP_INFO {
    ULONG cSize;
    LONG  lColor;
    LONG  lBackColor;
} CSP_INFO, *PCSP_INFO;

typedef struct _BOXPARAMS {
    POINTL ptl;
    SIZEL  sizl;
} BOXPARAMS, *PBOXPARAMS;

/*!
   @brief Draw a rectangular box.

   The current position is one corner of the box; pptlPoint specifies
   the opposite corner. The current position does not change.

   The lControl flags select what part of the box is drawn:
     - BOX_INTERIOR | BOX_BOUNDARY -> GreBoxBoth
     - BOX_INTERIOR only          -> GreBoxInterior
     - BOX_BOUNDARY only          -> GreBoxBoundary

   @param[in] hps        Presentation space handle.
   @param[in] lControl   Combination of BOX_INTERIOR and BOX_BOUNDARY.
   @param[in] pptlPoint  Opposite corner of the box.
   @param[in] lHRound    Horizontal corner rounding diameter.
   @param[in] lVRound    Vertical corner rounding diameter.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GreBoxBoth, GreBoxInterior, GreBoxBoundary
   @sa  https://www.edm2.com/index.php/GreBoxBoth
*/
LONG APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlPoint,
                     LONG lHRound, LONG lVRound)
{
    PPS       pps = LookupPS(hps);
    BOXPARAMS bp;

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pptlPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    bp.ptl     = *pptlPoint;
    bp.sizl.cx = lHRound;
    bp.sizl.cy = lVRound;

    if ((lControl & (BOX_INTERIOR | BOX_BOUNDARY)) ==
        (BOX_INTERIOR | BOX_BOUNDARY))
        return (LONG)GreBoxBoth(pps->hdc, &bp);
    if (lControl & BOX_INTERIOR)
        return (LONG)GreBoxInterior(pps->hdc, &bp);
    if (lControl & BOX_BOUNDARY)
        return (LONG)GreBoxBoundary(pps->hdc, &bp);

    return GPI_OK;
}

/*!
   @brief Draw a full arc centered on the current position.

   @param[in] hps          Presentation space handle.
   @param[in] lControl     Combination of BOX_INTERIOR and BOX_BOUNDARY.
   @param[in] fxMultiplier Size relative to the current arc parameters.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GreFullArcBoth, GreFullArcInterior, GreFullArcBoundary
*/
LONG APIENTRY GpiFullArc(HPS hps, LONG lControl, FIXED fxMultiplier)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    if ((lControl & (BOX_INTERIOR | BOX_BOUNDARY)) ==
        (BOX_INTERIOR | BOX_BOUNDARY))
        return (LONG)GreFullArcBoth(pps->hdc, fxMultiplier);
    if (lControl & BOX_INTERIOR)
        return (LONG)GreFullArcInterior(pps->hdc, fxMultiplier);
    if (lControl & BOX_BOUNDARY)
        return (LONG)GreFullArcBoundary(pps->hdc, fxMultiplier);

    return GPI_OK;
}

/*!
   @brief Draw a partial arc.

   @param[in] hps          Presentation space handle.
   @param[in] pptlCenter   Center of the arc.
   @param[in] fxMultiplier Size relative to the current arc parameters.
   @param[in] fxStart      Start angle (counterclockwise from X axis).
   @param[in] fxSweep      Sweep angle.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GrePartialArc
*/
LONG APIENTRY GpiPartialArc(HPS hps, PPOINTL pptlCenter,
                            FIXED fxMultiplier, FIXED fxStart, FIXED fxSweep)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pptlCenter == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GrePartialArc(pps->hdc, pptlCenter,
                               fxMultiplier, fxStart, fxSweep);
}

/*!
   @brief Draw an arc through three points.

   @param[in] hps       Presentation space handle.
   @param[in] pptlPoint Array of two POINTL: mid-point and end-point.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GreArc
*/
LONG APIENTRY GpiPointArc(HPS hps, PPOINTL pptlPoint)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pptlPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GreArc(pps->hdc, pptlPoint);
}

/*!
   @brief Draw a line from the current position to pptlEndPoint.

   @param[in] hps          Presentation space handle.
   @param[in] pptlEndPoint End point of the line.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GpiMove, GrePolyLine
*/
LONG APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pptlEndPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GrePolyLine(pps->hdc, pptlEndPoint, 1);
}

/*!
   @brief Set the current position.

   Valid in all drawing modes. In DM_RETAIN the position change is
   recorded into the current segment.

   @param[in] hps       Presentation space handle.
   @param[in] pptlPoint New current position.

   @return TRUE on success, FALSE on error.

   @see GpiLine, GpiQueryCurrentPosition, GreSetCurrentPosition
*/
BOOL APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return FALSE;
    }
    if (pptlPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return TRUE;
    }

    return GreSetCurrentPosition(pps->hdc, pptlPoint);
}

/*!
   @brief Draw a single marker at the specified position.

   @param[in] hps       Presentation space handle.
   @param[in] pptlPoint Marker position.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GpiPolyMarker, GrePolyMarker
*/
LONG APIENTRY GpiMarker(HPS hps, PPOINTL pptlPoint)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pptlPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GrePolyMarker(pps->hdc, pptlPoint, 1);
}

/*!
   @brief Draw a sequence of markers.

   @param[in] hps         Presentation space handle.
   @param[in] lCount      Number of markers; 0 is a no-op.
   @param[in] aptlPoints  Array of lCount POINTL centers.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GpiMarker, GrePolyMarker
*/
LONG APIENTRY GpiPolyMarker(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0)
        return GPI_OK;
    if (aptlPoints == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GrePolyMarker(pps->hdc, aptlPoints, lCount);
}

/*!
   @brief Draw a polyfillet.

   @param[in] hps         Presentation space handle.
   @param[in] lCount      Number of coordinate pairs; 0 is a no-op.
   @param[in] aptlPoints  Array of lCount end points.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GpiPolyFilletSharp, GrePolyFillet
*/
LONG APIENTRY GpiPolyFillet(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0)
        return GPI_OK;
    if (aptlPoints == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GrePolyFillet(pps->hdc, aptlPoints, lCount);
}

/*!
   @brief Draw a sequence of sharp fillets.

   @param[in] hps         Presentation space handle.
   @param[in] lCount      Number of coordinate pairs; 0 is a no-op.
   @param[in] aptlPoints  Array of lCount points.
   @param[in] afxSharp    Array of lCount/2 sharpness values.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GpiPolyFillet, GrePolyFilletSharp
*/
LONG APIENTRY GpiPolyFilletSharp(HPS hps, LONG lCount, PPOINTL aptlPoints,
                                 PFIXED afxSharp)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0)
        return GPI_OK;
    if (aptlPoints == NULL || afxSharp == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GrePolyFilletSharp(pps->hdc, aptlPoints, lCount, afxSharp);
}

/*!
   @brief Draw a sequence of Bezier splines.

   @param[in] hps         Presentation space handle.
   @param[in] lCount      Number of coordinate pairs; 0 is a no-op.
   @param[in] aptlPoints  Array of points (3 per spline).

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GrePolySpline
*/
LONG APIENTRY GpiPolySpline(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0)
        return GPI_OK;
    if (aptlPoints == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GrePolySpline(pps->hdc, aptlPoints, lCount);
}

/*!
   @brief Draw a character string at the current position.

   @param[in] hps        Presentation space handle.
   @param[in] lCount     Number of characters in the string.
   @param[in] pchString  Pointer to the character string.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GpiCharStringAt, GpiCharStringPos, GreCharString
*/
LONG APIENTRY GpiCharString(HPS hps, LONG lCount, PCH pchString)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0)
        return GPI_OK;
    if (pchString == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GreCharString(pps->hdc, lCount, pchString);
}

/*!
   @brief Draw a character string at a specified position.

   @param[in] hps        Presentation space handle.
   @param[in] pptlPoint  Start position for the string.
   @param[in] lCount     Number of characters in the string.
   @param[in] pchString  Pointer to the character string.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GpiCharString, GpiCharStringPosAt, GreCharStringPos
*/
LONG APIENTRY GpiCharStringAt(HPS hps, PPOINTL pptlPoint,
                              LONG lCount, PCH pchString)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pptlPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0)
        return GPI_OK;
    if (pchString == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GreCharStringPos(pps->hdc, pptlPoint, NULL,
                                  CHS_START_XY, lCount, pchString,
                                  NULL, NULL);
}

/*!
   @brief Draw a character string at the current position, with
          optional clipping rectangle and increment vector.

   @param[in] hps        Presentation space handle.
   @param[in] prclRect   Optional clipping/opaque rectangle (may be NULL).
   @param[in] flOptions  CHS_* option flags.
   @param[in] lCount     Number of characters in the string.
   @param[in] pchString  Pointer to the character string.
   @param[in] pAdx       Optional increment vector (may be NULL).

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GpiCharStringPosAt, GreCharStringPos
*/
LONG APIENTRY GpiCharStringPos(HPS hps, PRECTL prclRect,
                               ULONG flOptions, LONG lCount, PCH pchString,
                               PLONG pAdx)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0)
        return GPI_OK;
    if (pchString == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GreCharStringPos(pps->hdc, NULL, prclRect, flOptions,
                                  lCount, pchString, pAdx, NULL);
}

/*!
   @brief Draw a character string at a specified position, with
          optional clipping rectangle and increment vector.

   @param[in] hps        Presentation space handle.
   @param[in] pptlPoint  Start position for the string.
   @param[in] prclRect   Optional clipping/opaque rectangle (may be NULL).
   @param[in] flOptions  CHS_* option flags (CHS_START_XY is added).
   @param[in] lCount     Number of characters in the string.
   @param[in] pchString  Pointer to the character string.
   @param[in] pAdx       Optional increment vector (may be NULL).

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @see GpiCharStringPos, GreCharStringPos
*/
LONG APIENTRY GpiCharStringPosAt(HPS hps, PPOINTL pptlPoint, PRECTL prclRect,
                                 ULONG flOptions, LONG lCount, PCH pchString,
                                 PLONG pAdx)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pptlPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0)
        return GPI_OK;
    if (pchString == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        /* TODO: record into current segment when segments are implemented. */
        return GPI_OK;
    }

    return (LONG)GreCharStringPos(pps->hdc, pptlPoint, prclRect,
                                  flOptions | CHS_START_XY, lCount, pchString,
                                  pAdx, NULL);
}

/*!
   @brief Record a comment in the current segment.

   Comments have no drawing effect. They are recorded in retain mode
   and ignored in draw mode. Since segment support is not yet
   implemented, this function currently just returns success.

   @param[in] hps       Presentation space handle.
   @param[in] lLength   Length of the comment data.
   @param[in] pbData    Pointer to the comment data.

   @return TRUE on success, FALSE on invalid HPS.

   @see GpiOpenSegment, GpiCloseSegment
*/
BOOL APIENTRY GpiComment(HPS hps, LONG lLength, PBYTE pbData)
{
    PPS pps = LookupPS(hps);

    if (pps == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
        return FALSE;
    }
    if (pps->hdc == (HDC)0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
        return FALSE;
    }

    /* TODO: record in current segment when segment support lands. */
    (void)lLength;
    (void)pbData;
    return TRUE;
}
