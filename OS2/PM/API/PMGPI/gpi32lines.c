/*!
   @file

   @ingroup pm

   @brief Presentation Manager GPI line/position primitives (32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

   Included by pmgpi.c. Provides 32-bit implementations of a subset of
   Gpi* line/position primitives. Each function performs PMGPI-level
   validation (PS lookup via LookupPS / LookupPSDC, DC association,
   retain-mode restrictions where applicable) before dispatching the
   operation into the presentation driver through the corresponding
   Gre* DDI macro in pmddim.h.

   Note on pInstance: the Gre* macros in pmddim.h pass 0L for the
   pInstance argument; the graphics engine substitutes the correct
   instance data associated with the hdc automatically.
*/

/*!
   @brief Draw a sequence of disjoint lines.

   Public API-level wrapper around GreDisjointLines.

   @note This function currently dispatches without PS/DC validation;
         an invalid hps will be passed into the DDI as hdc = 0.
         TODO: add the same validation as GpiPolyLine.

   @param[in] hps        Presentation space handle.
   @param[in] lCount     Number of (X, Y) coordinate pairs.
   @param[in] aptlPoints Array of lCount POINTL end points.

   @return GPI_OK on success, GPI_ERROR on error.

   @see GpiPolyLine, GreDisjointLines
   @sa  https://www.edm2.com/index.php/GreDisjointLines
*/
LONG APIENTRY GpiPolyLineDisjoint(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
  LONG rc = (LONG)GreDisjointLines(GpiQueryDevice(hps), aptlPoints, lCount);
  /* GreDisjointLines returns GPI_ERROR (-1) on failure; any other
     value (0 = OK, positive = hits/lines) is treated as success,
     which matches the documented GpiPolyLineDisjoint contract. */
  return (rc == (LONG)GPI_ERROR) ? GPI_ERROR : GPI_OK;
}

/*!
   @brief Query the current position.

   Returns the position in world coordinates if COM_TRANSFORM is set
   in the driver's lFunction, screen coordinates otherwise. Not valid
   in DM_RETAIN — no "current position" exists in that mode, because
   primitives are recorded into segments instead of being drawn.

   @param[in]  hps    Presentation space handle.
   @param[out] pptl   Receives the current position.

   @return TRUE on success, FALSE on error (WinSetErrorInfo called).

   @see GpiSetCurrentPosition, GreGetCurrentPosition
   @sa  https://www.edm2.com/index.php/GreGetCurrentPosition
*/
BOOL APIENTRY GpiQueryCurrentPosition(HPS hps, PPOINTL pptl)
{
    INIT_PSDC(hps, FALSE);

    if (pptl == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return FALSE;
    }
    if (pps->ulDrawingMode == DM_RETAIN) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_IN_RETAIN_MODE),
                        SEI_NOBEEP);
        return FALSE;
    }

    return GreGetCurrentPosition(pps->hdc, pptl);
}

/*!
   @brief Set the current position.

   Valid in all drawing modes. In DM_RETAIN the position change is
   recorded into the current segment; in the draw modes it also
   updates the immediate position used by subsequent drawing.

   @param[in] hps    Presentation space handle.
   @param[in] pptl   New current position (world coordinates if
                     COM_TRANSFORM is set, screen coordinates otherwise).

   @return TRUE on success, FALSE on error (WinSetErrorInfo called).

   @see GpiQueryCurrentPosition, GreSetCurrentPosition
   @sa  https://www.edm2.com/index.php/GreSetCurrentPosition
*/
BOOL APIENTRY GpiSetCurrentPosition(HPS hps, PPOINTL pptl)
{
    INIT_PSDC(hps, FALSE);

    if (pptl == NULL) {
        WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_COORDINATE),
                        SEI_NOBEEP);
        return FALSE;
    }

    return GreSetCurrentPosition(pps->hdc, pptl);
}

/*!
   @brief Draw a polyline.

   Draws a sequence of one or more lines starting at the current
   position. As each line is drawn, its end point becomes the start
   point for the next. Upon completion, the current position is the
   end point of the last line.

   Valid in all drawing modes (DM_DRAW, DM_RETAIN, DM_DRAWANDRETAIN);
   in the retain modes the command is recorded into the current segment.

   @param[in] hps        Presentation space handle.
   @param[in] lCount     Number of (X, Y) coordinate pairs in aptlPoints.
                         If 0, the function is a no-op.
   @param[in] aptlPoints Array of lCount POINTL end points.

   @return GPI_OK, GPI_HITS, or GPI_ERROR.

   @note  This function previously did not record the element in
          retain-mode. INIT_DRAWING + RECORD_OR_DISPATCH now records
          it, so GpiPolyLine behaves consistently with GpiLine and
          other primitives.

   @see GpiPolyLineDisjoint, GrePolyLine
   @sa  https://www.edm2.com/index.php/GrePolyLine
*/
LONG APIENTRY GpiPolyLine(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    INIT_DRAWING(hps, GPI_ERROR);

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

    RECORD_OR_DISPATCH(pps, am, OCODE_GCLINE, lCount,
                       lCount * (LONG)sizeof(POINTL), aptlPoints,
                       GPI_OK, GPI_ERROR);

    return (LONG)GrePolyLine(pps->hdc, aptlPoints, lCount);
}
