/*!
   @file
   @ingroup pm
   @brief Presentation Manager GPI area/path primitives (32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website
*/

// @todo PHID real value?
#define PHID PVOID


/*
  Documentation: https://www.edm2.com/index.php/GreBeginArea
*/
BOOL   APIENTRY GpiBeginArea(HPS hps, ULONG flOptions)
{
  PPS    pps = LookupPS(hps);
  ORDER  ord;
  UCHAR  bFlags = 0x80;   /* GBAR_RESERVED, GBAR_NOBOUNDARY, GBAR_ALTERNATE */

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return FALSE;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return FALSE;
  }

  if (flOptions & BA_BOUNDARY) bFlags |= 0x40;   /* GBAR_BOUNDARY   */
  if (flOptions & BA_WINDING)  bFlags |= 0x20;   /* GBAR_WINDING    */

  ord.idCode  = OCODE_GBAR;
  ord.uchData = bFlags;

  if (PS_IS_RECORDING(pps->ulDrawingMode)) {
    if (!RecordElement(pps, OCODE_GBAR, 1, sizeof(ord), &ord))
      return FALSE;
    if (!PS_IS_DRAWING(pps->ulDrawingMode))
      return TRUE;
  }
  return GreBeginArea(pps->hdc, flOptions);
}

/*
  Documentation: https://www.edm2.com/index.php/GreEndArea
*/
LONG   APIENTRY GpiEndArea(HPS hps)
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
  if (PS_IS_RECORDING(pps->ulDrawingMode)) {
    if (!RecordElement(pps, OCODE_GEAR, 0, 0, NULL))
      return GPI_ERROR;
    if (!PS_IS_DRAWING(pps->ulDrawingMode))
      return GPI_OK;
  }
  // @todo EA_DRAW - value?
  return GreEndArea(pps->hdc, 0);
}

/*
  Documentation: https://www.edm2.com/index.php/GreBeginPath
*/
BOOL   APIENTRY GpiBeginPath(HPS hps, LONG lPath)
{
  PPS         pps = LookupPS(hps);
  ORDER_GBPTH p;

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return FALSE;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return FALSE;
  }

  p.usReserved = 0;
  p.idPath     = lPath;

  if (PS_IS_RECORDING(pps->ulDrawingMode)) {
    if (!RecordElement(pps, OCODE_GBPTH, 1, sizeof(p), &p))
      return FALSE;
    if (!PS_IS_DRAWING(pps->ulDrawingMode))
      return TRUE;
  }
  return GreBeginPath(pps->hdc, lPath);
}

/*
  Documentation: https://www.edm2.com/index.php/GreEndPath
*/
BOOL   APIENTRY GpiEndPath(HPS hps)
{
  PPS    pps = LookupPS(hps);
  ORDER  ord;

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return FALSE;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return FALSE;
  }

  ord.idCode  = OCODE_GEPTH;
  ord.uchData = 0;

  if (PS_IS_RECORDING(pps->ulDrawingMode)) {
    if (!RecordElement(pps, OCODE_GEPTH, 1, sizeof(ord), &ord))
      return FALSE;
    if (!PS_IS_DRAWING(pps->ulDrawingMode))
      return TRUE;
  }
  //@todo EA_DRAW 0 or another value??
  return GreEndPath(pps->hdc, 0);
}

/*
  Documentation: https://www.edm2.com/index.php/GreCloseFigure
*/
BOOL   APIENTRY GpiCloseFigure(HPS hps)
{
  PPS    pps = LookupPS(hps);
  ORDER  ord;

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return FALSE;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return FALSE;
  }

  ord.idCode  = OCODE_GCFIG;
  ord.uchData = 0;

  if (PS_IS_RECORDING(pps->ulDrawingMode)) {
    if (!RecordElement(pps, OCODE_GCFIG, 1, sizeof(ord), &ord))
      return FALSE;
    if (!PS_IS_DRAWING(pps->ulDrawingMode))
      return TRUE;
  }
  return GreCloseFigure(pps->hdc);
}

/*
  Documentation: https://www.edm2.com/index.php/GreFillPath
*/
LONG   APIENTRY GpiFillPath(HPS hps,LONG a,LONG b)
{
  PPS         pps = LookupPS(hps);
  ORDER_GFPTH fp;

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return GPI_ERROR;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return GPI_ERROR;
  }

  fp.fbFlags     = (b == FPATH_WINDING) ? GFPTH_WINDING : GFPTH_ALTERNATE;
  fp.uchReserved = 0;
  fp.idPath      = a;

  if (PS_IS_RECORDING(pps->ulDrawingMode)) {
    if (!RecordElement(pps, OCODE_GFPTH, 1, sizeof(fp), &fp))
      return GPI_ERROR;
    if (!PS_IS_DRAWING(pps->ulDrawingMode))
      return GPI_OK;
  }
  return GreFillPath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreOutlinePath
*/
LONG   APIENTRY GpiOutlinePath(HPS hps,LONG a,LONG b)
{
  PPS         pps = LookupPS(hps);
  ORDER_GOPTH op;

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return GPI_ERROR;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return GPI_ERROR;
  }

  op.fbFlags     = 0;
  op.uchReserved = 0;
  op.idPath      = a;

  if (PS_IS_RECORDING(pps->ulDrawingMode)) {
    if (!RecordElement(pps, OCODE_GOPTH, 1, sizeof(op), &op))
      return GPI_ERROR;
    if (!PS_IS_DRAWING(pps->ulDrawingMode))
      return GPI_OK;
  }
  return GreOutlinePath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreModifyPath
*/
BOOL   APIENTRY GpiModifyPath(HPS hps,LONG a,LONG b)
{
  PPS         pps = LookupPS(hps);
  ORDER_GMPTH mp;

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return FALSE;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return FALSE;
  }

  mp.uchMode     = (UCHAR)b;
  mp.uchReserved = 0;
  mp.idPath      = a;

  if (PS_IS_RECORDING(pps->ulDrawingMode)) {
    if (!RecordElement(pps, OCODE_GMPTH, 1, sizeof(mp), &mp))
      return FALSE;
    if (!PS_IS_DRAWING(pps->ulDrawingMode))
      return TRUE;
  }
  return GreModifyPath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreStrokePath
*/
LONG   APIENTRY GpiStrokePath(HPS hps,LONG a,ULONG b)
{
  PPS         pps = LookupPS(hps);
  ORDER_GMPTH mp;

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return GPI_ERROR;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return GPI_ERROR;
  }

  mp.uchMode     = GMPTH_STROKE;
  mp.uchReserved = 0;
  mp.idPath      = a;

  if (PS_IS_RECORDING(pps->ulDrawingMode)) {
    if (!RecordElement(pps, OCODE_GMPTH, 1, sizeof(mp), &mp))
      return GPI_ERROR;
    if (!PS_IS_DRAWING(pps->ulDrawingMode))
      return GPI_OK;
  }
  return GreStrokePath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreSelectClipPath
*/
BOOL   APIENTRY GpiSetClipPath(HPS hps,LONG a,LONG b)
{
  PPS          pps = LookupPS(hps);
  ORDER_GSCPTH cp;

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return FALSE;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return FALSE;
  }

  cp.fbFlags = 0;
  if (b & SCP_WINDING)  cp.fbFlags |= GSCPTH_WINDING;
  if (b & SCP_AND)      cp.fbFlags |= GSCPTH_INTERSECT;
  cp.uchReserved = 0;
  cp.idPath      = a;

  if (PS_IS_RECORDING(pps->ulDrawingMode)) {
    if (!RecordElement(pps, OCODE_GSCPTH, 1, sizeof(cp), &cp))
      return FALSE;
    if (!PS_IS_DRAWING(pps->ulDrawingMode))
      return TRUE;
  }
  return GreSelectClipPath(pps->hdc, a, b);
}
