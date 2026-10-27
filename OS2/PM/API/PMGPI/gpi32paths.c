// @todo PHID real value?
#define PHID PVOID

/*
  Documentation: https://www.edm2.com/index.php/GreBeginArea
*/
BOOL   APIENTRY GpiBeginArea(HPS hps, ULONG flOptions)
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
  if (pps->ulDrawingMode == DM_RETAIN) {
    /* TODO: record into current segment when segments are implemented. */
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
  if (pps->ulDrawingMode == DM_RETAIN) {
    /* TODO: record into current segment when segments are implemented. */
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
  PPS pps = LookupPS(hps);

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return FALSE;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return FALSE;
  }
  if (pps->ulDrawingMode == DM_RETAIN) {
    /* TODO: record into current segment when segments are implemented. */
    return TRUE;
  }

  return GreBeginPath(pps->hdc, lPath);
}

/*
  Documentation: https://www.edm2.com/index.php/GreEndPath
*/
BOOL   APIENTRY GpiEndPath(HPS hps)
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
  if (pps->ulDrawingMode == DM_RETAIN) {
    /* TODO: record into current segment when segments are implemented. */
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
  PPS pps = LookupPS(hps);

  if (pps == NULL) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HPS), SEI_NOBEEP);
    return FALSE;
  }
  if (pps->hdc == (HDC)0) {
    WinSetErrorInfo(MAKEERRORID(SEVERITY_ERROR, PMERR_INV_HDC), SEI_NOBEEP);
    return FALSE;
  }
  if (pps->ulDrawingMode == DM_RETAIN) {
    /* TODO: record into current segment when segments are implemented. */
    return TRUE;
  }

  return GreCloseFigure(pps->hdc);
}

/*
  Documentation: https://www.edm2.com/index.php/GreFillPath
*/
LONG   APIENTRY GpiFillPath(HPS hps,LONG a,LONG b)
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

  return GreFillPath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreOutlinePath
*/
LONG   APIENTRY GpiOutlinePath(HPS hps,LONG a,LONG b)
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

  return GreOutlinePath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreOutlinePath
*/
BOOL   APIENTRY GpiModifyPath(HPS hps,LONG a,LONG b)
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
  if (pps->ulDrawingMode == DM_RETAIN) {
    /* TODO: record into current segment when segments are implemented. */
    return TRUE;
  }

  return GreModifyPath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreStrokePath
*/
LONG   APIENTRY GpiStrokePath(HPS hps,LONG a,ULONG b)
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

  return GreStrokePath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreSelectClipPath
*/
BOOL   APIENTRY GpiSetClipPath(HPS hps,LONG a,LONG b)
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
  if (pps->ulDrawingMode == DM_RETAIN) {
    /* TODO: record into current segment when segments are implemented. */
    return TRUE;
  }

  return GreSelectClipPath(pps->hdc, a, b);
}
