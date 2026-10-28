/*!
   @file
   @ingroup pm
   @brief GPI primitive drawing functions (32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   All recorded payloads use structures from <pmord.h> only.
   GpiFullArc has no struct in pmord.h — its single field (FIXED
   multiplier) is recorded as-is.
   GpiCharStringAt / GpiCharStringPosAt are decomposed into
   GpiMove + GpiCharString / GpiCharStringPos (equivalent semantics).

   Drawing control for GpiBox / GpiFullArc uses the standard OS/2
   toolkit constants DRO_FILL / DRO_OUTLINE / DRO_OUTLINEFILL
   (defined in <pmgpi.h>).

   All primitives use INIT_DRAWING (which gives `pps` and the actual
   drawing mode `am`) and, where the recording shape is a single
   RecordElement call, RECORD_OR_DISPATCH. Functions with a custom
   recorder (RecordCharStringPos, RecordPolyFilletSharp) or writing
   multiple elements keep the explicit if/else pattern on `am`.
*/

/* === GpiBox ======================================================= */

LONG APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlPoint,
                     LONG lHRound, LONG lVRound)
{
    ORDERL_GCBOX box;
    BOXPARAMS    bp;

    INIT_DRAWING(hps, GPI_ERROR);

    if (pptlPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    box.fbFlags = 0;
    if (lControl & DRO_FILL)    box.fbFlags |= GCBOX_FILL;
    if (lControl & DRO_OUTLINE) box.fbFlags |= GCBOX_BOUNDARY;
    box.uchReserved = 0;
    box.ptCorner    = *pptlPoint;
    box.hAxis       = lHRound;
    box.vAxis       = lVRound;

    RECORD_OR_DISPATCH(pps, am, OCODE_GCBOX, 1, sizeof(box), &box,
                       GPI_OK, GPI_ERROR);

    bp.ptl     = box.ptCorner;
    bp.sizl.cx = box.hAxis;
    bp.sizl.cy = box.vAxis;
    if (lControl == DRO_OUTLINEFILL)
        return (LONG)GreBoxBoth(pps->hdc, &bp);
    if (lControl & DRO_FILL)
        return (LONG)GreBoxInterior(pps->hdc, &bp);
    if (lControl & DRO_OUTLINE)
        return (LONG)GreBoxBoundary(pps->hdc, &bp);
    return GPI_OK;
}

/* === GpiFullArc =================================================== */

LONG APIENTRY GpiFullArc(HPS hps, LONG lControl, FIXED fxMultiplier)
{
    INIT_DRAWING(hps, GPI_ERROR);

    /* No struct in pmord.h for OCODE_GCFARC. The single FIXED field
       is recorded as-is. On replay, GreFullArcBoth is used; interior-
       only / boundary-only are treated identically (full arc both is
       the standard interpretation, and GreFullArcInterior/-Boundary
       would need separate OCODEs which pmord.h does not provide). */
    RECORD_OR_DISPATCH(pps, am, OCODE_GCFARC, 1, sizeof(FIXED),
                       &fxMultiplier, GPI_OK, GPI_ERROR);

    if (lControl == DRO_OUTLINEFILL)
        return (LONG)GreFullArcBoth(pps->hdc, fxMultiplier);
    if (lControl & DRO_FILL)
        return (LONG)GreFullArcInterior(pps->hdc, fxMultiplier);
    if (lControl & DRO_OUTLINE)
        return (LONG)GreFullArcBoundary(pps->hdc, fxMultiplier);
    return GPI_OK;
}

/* === GpiPartialArc ================================================ */

LONG APIENTRY GpiPartialArc(HPS hps, PPOINTL pptlCenter,
                            FIXED fxMultiplier, FIXED fxStart, FIXED fxSweep)
{
    ORDERL_GCPARC parc;

    INIT_DRAWING(hps, GPI_ERROR);

    if (pptlCenter == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    parc.ptCenter      = *pptlCenter;
    parc.ufxMultiplier = fxMultiplier;
    parc.usStartAngle  = fxStart;
    parc.usSweepAngle  = fxSweep;

    RECORD_OR_DISPATCH(pps, am, OCODE_GCPARC, 1, sizeof(parc), &parc,
                       GPI_OK, GPI_ERROR);

    return (LONG)GrePartialArc(pps->hdc, pptlCenter,
                               fxMultiplier, fxStart, fxSweep);
}

/* === GpiPointArc ================================================== */

LONG APIENTRY GpiPointArc(HPS hps, PPOINTL pptlPoint)
{
    ORDERL_GCARC arc;

    INIT_DRAWING(hps, GPI_ERROR);

    if (pptlPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    arc.ptInter = pptlPoint[0];
    arc.ptEnd   = pptlPoint[1];

    RECORD_OR_DISPATCH(pps, am, OCODE_GCARC, 1, sizeof(arc), &arc,
                       GPI_OK, GPI_ERROR);

    return (LONG)GreArc(pps->hdc, pptlPoint);
}

/* === GpiLine, GpiMove ============================================= */

LONG APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint)
{
    INIT_DRAWING(hps, GPI_ERROR);

    if (pptlEndPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    RECORD_OR_DISPATCH(pps, am, OCODE_GCLINE, 1, sizeof(POINTL),
                       pptlEndPoint, GPI_OK, GPI_ERROR);

    return (LONG)GrePolyLine(pps->hdc, pptlEndPoint, 1);
}

BOOL APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint)
{
    INIT_DRAWING(hps, FALSE);

    if (pptlPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return FALSE;
    }

    RECORD_OR_DISPATCH(pps, am, OCODE_GSCP, 1, sizeof(POINTL), pptlPoint,
                       TRUE, FALSE);

    return GreSetCurrentPosition(pps->hdc, pptlPoint);
}

/* === GpiMarker, GpiPolyMarker ===================================== */

LONG APIENTRY GpiMarker(HPS hps, PPOINTL pptlPoint)
{
    INIT_DRAWING(hps, GPI_ERROR);

    if (pptlPoint == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    RECORD_OR_DISPATCH(pps, am, OCODE_GCMRK, 1, sizeof(POINTL), pptlPoint,
                       GPI_OK, GPI_ERROR);

    return (LONG)GrePolyMarker(pps->hdc, pptlPoint, 1);
}

LONG APIENTRY GpiPolyMarker(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    INIT_DRAWING(hps, GPI_ERROR);

    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0) return GPI_OK;
    if (aptlPoints == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    RECORD_OR_DISPATCH(pps, am, OCODE_GCMRK, lCount,
                       lCount * (LONG)sizeof(POINTL), aptlPoints,
                       GPI_OK, GPI_ERROR);

    return (LONG)GrePolyMarker(pps->hdc, aptlPoints, lCount);
}

/* === GpiPolyFillet, GpiPolySpline ================================= */

LONG APIENTRY GpiPolyFillet(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    INIT_DRAWING(hps, GPI_ERROR);

    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0) return GPI_OK;
    if (aptlPoints == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    RECORD_OR_DISPATCH(pps, am, OCODE_GCFLT, lCount,
                       lCount * (LONG)sizeof(POINTL), aptlPoints,
                       GPI_OK, GPI_ERROR);

    return (LONG)GrePolyFillet(pps->hdc, aptlPoints, lCount);
}

LONG APIENTRY GpiPolySpline(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    INIT_DRAWING(hps, GPI_ERROR);

    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0) return GPI_OK;
    if (aptlPoints == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    RECORD_OR_DISPATCH(pps, am, OCODE_GCBEZ, lCount,
                       lCount * (LONG)sizeof(POINTL), aptlPoints,
                       GPI_OK, GPI_ERROR);

    return (LONG)GrePolySpline(pps->hdc, aptlPoints, lCount);
}

/* === GpiPolyFilletSharp =========================================== */

LONG APIENTRY GpiPolyFilletSharp(HPS hps, LONG lCount, PPOINTL aptlPoints,
                                 PFIXED afxSharp)
{
    INIT_DRAWING(hps, GPI_ERROR);

    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0) return GPI_OK;
    if (aptlPoints == NULL || afxSharp == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    /* Custom recorder (ORDERL_GCSFLT), not RecordElement. */
    if (PS_IS_RECORDING(am)) {
        if (!RecordPolyFilletSharp(pps, lCount, aptlPoints, afxSharp))
            return GPI_ERROR;
        if (!PS_IS_DRAWING(am))
            return GPI_OK;
    }
    return (LONG)GrePolyFilletSharp(pps->hdc, aptlPoints, lCount, afxSharp);
}

/* === Character string primitives ================================== */

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
    INIT_DRAWING(hps, GPI_ERROR);

    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0) return GPI_OK;
    if (pchString == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    RECORD_OR_DISPATCH(pps, am, OCODE_GCCHST, lCount, lCount, pchString,
                       GPI_OK, GPI_ERROR);

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
    INIT_DRAWING(hps, GPI_ERROR);

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
    if (lCount == 0) return GPI_OK;
    if (pchString == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    /* Two elements: GSCP (set position) + GCCHST (string). */
    if (PS_IS_RECORDING(am)) {
        if (!RecordElement(pps, OCODE_GSCP, 1, sizeof(POINTL), pptlPoint))
            return GPI_ERROR;
        if (!RecordElement(pps, OCODE_GCCHST, lCount, lCount, pchString))
            return GPI_ERROR;
        if (!PS_IS_DRAWING(am))
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
    INIT_DRAWING(hps, GPI_ERROR);

    if (flOptions & CHS_START_XY) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_CHAR_POS_OPTIONS),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount < 0) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }
    if (lCount == 0) return GPI_OK;
    if (pchString == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    /* Custom recorder (ORDERL_GCCHSTE). */
    if (PS_IS_RECORDING(am)) {
        if (!RecordCharStringPos(pps, flOptions, lCount, pchString,
                                 prclRect, pAdx))
            return GPI_ERROR;
        if (!PS_IS_DRAWING(am))
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
    ULONG fl;

    INIT_DRAWING(hps, GPI_ERROR);

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
    if (lCount == 0) return GPI_OK;
    if (pchString == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_LENGTH_OR_COUNT),
                        SEI_NOBEEP);
        return GPI_ERROR;
    }

    /* Two elements: GSCP + ORDERL_GCCHSTE via RecordCharStringPos. */
    if (PS_IS_RECORDING(am)) {
        fl = flOptions & ~CHS_START_XY;
        if (!RecordElement(pps, OCODE_GSCP, 1, sizeof(POINTL), pptlPoint))
            return GPI_ERROR;
        if (!RecordCharStringPos(pps, fl, lCount, pchString, prclRect, pAdx))
            return GPI_ERROR;
        if (!PS_IS_DRAWING(am))
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
    INIT_DRAWING(hps, FALSE);

    if (PS_IS_RECORDING(am)) {
        if (lLength > 0) {
            if (pbData == NULL) return FALSE;
            if (!RecordElement(pps, OCODE_GCOMT, lLength, lLength, pbData))
                return FALSE;
        }
        if (!PS_IS_DRAWING(am))
            return TRUE;
    }
    return TRUE;
}
