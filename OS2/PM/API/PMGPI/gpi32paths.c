/*!
   @file
   @ingroup pm
   @brief Presentation Manager GPI area/path primitives (32-bit)

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   All functions use INIT_DRAWING + RECORD_OR_DISPATCH. The recorded
   payload is always a single ORDER / ORDER_G* structure from
   <pmord.h>.
*/

// @todo PHID real value?
#define PHID PVOID


/*
  Documentation: https://www.edm2.com/index.php/GreBeginArea
*/
BOOL   APIENTRY GpiBeginArea(HPS hps, ULONG flOptions)
{
  ORDER  ord;
  UCHAR  bFlags = 0x80;   /* GBAR_RESERVED, GBAR_NOBOUNDARY, GBAR_ALTERNATE */

  INIT_DRAWING(hps, FALSE);

  if (flOptions & BA_BOUNDARY) bFlags |= 0x40;   /* GBAR_BOUNDARY   */
  if (flOptions & BA_WINDING)  bFlags |= 0x20;   /* GBAR_WINDING    */

  ord.idCode  = OCODE_GBAR;
  ord.uchData = bFlags;

  RECORD_OR_DISPATCH(pps, am, OCODE_GBAR, 1, sizeof(ord), &ord,
                     TRUE, FALSE);

  return GreBeginArea(pps->hdc, flOptions);
}

/*
  Documentation: https://www.edm2.com/index.php/GreEndArea
*/
LONG   APIENTRY GpiEndArea(HPS hps)
{
  INIT_DRAWING(hps, GPI_ERROR);

  RECORD_OR_DISPATCH(pps, am, OCODE_GEAR, 0, 0, NULL, GPI_OK, GPI_ERROR);

  // @todo EA_DRAW - value?
  return GreEndArea(pps->hdc, 0);
}

/*
  Documentation: https://www.edm2.com/index.php/GreBeginPath
*/
BOOL   APIENTRY GpiBeginPath(HPS hps, LONG lPath)
{
  ORDER_GBPTH p;

  INIT_DRAWING(hps, FALSE);

  p.usReserved = 0;
  p.idPath     = lPath;

  RECORD_OR_DISPATCH(pps, am, OCODE_GBPTH, 1, sizeof(p), &p,
                     TRUE, FALSE);

  return GreBeginPath(pps->hdc, lPath);
}

/*
  Documentation: https://www.edm2.com/index.php/GreEndPath
*/
BOOL   APIENTRY GpiEndPath(HPS hps)
{
  ORDER  ord;

  INIT_DRAWING(hps, FALSE);

  ord.idCode  = OCODE_GEPTH;
  ord.uchData = 0;

  RECORD_OR_DISPATCH(pps, am, OCODE_GEPTH, 1, sizeof(ord), &ord,
                     TRUE, FALSE);

  //@todo EA_DRAW 0 or another value??
  return GreEndPath(pps->hdc, 0);
}

/*
  Documentation: https://www.edm2.com/index.php/GreCloseFigure
*/
BOOL   APIENTRY GpiCloseFigure(HPS hps)
{
  ORDER  ord;

  INIT_DRAWING(hps, FALSE);

  ord.idCode  = OCODE_GCFIG;
  ord.uchData = 0;

  RECORD_OR_DISPATCH(pps, am, OCODE_GCFIG, 1, sizeof(ord), &ord,
                     TRUE, FALSE);

  return GreCloseFigure(pps->hdc);
}

/*
  Documentation: https://www.edm2.com/index.php/GreFillPath
*/
LONG   APIENTRY GpiFillPath(HPS hps,LONG a,LONG b)
{
  ORDER_GFPTH fp;

  INIT_DRAWING(hps, GPI_ERROR);

  fp.fbFlags     = (b == FPATH_WINDING) ? GFPTH_WINDING : GFPTH_ALTERNATE;
  fp.uchReserved = 0;
  fp.idPath      = a;

  RECORD_OR_DISPATCH(pps, am, OCODE_GFPTH, 1, sizeof(fp), &fp,
                     GPI_OK, GPI_ERROR);

  return GreFillPath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreOutlinePath
*/
LONG   APIENTRY GpiOutlinePath(HPS hps,LONG a,LONG b)
{
  ORDER_GOPTH op;

  INIT_DRAWING(hps, GPI_ERROR);

  op.fbFlags     = 0;
  op.uchReserved = 0;
  op.idPath      = a;

  RECORD_OR_DISPATCH(pps, am, OCODE_GOPTH, 1, sizeof(op), &op,
                     GPI_OK, GPI_ERROR);

  return GreOutlinePath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreModifyPath
*/
BOOL   APIENTRY GpiModifyPath(HPS hps,LONG a,LONG b)
{
  ORDER_GMPTH mp;

  INIT_DRAWING(hps, FALSE);

  mp.uchMode     = (UCHAR)b;
  mp.uchReserved = 0;
  mp.idPath      = a;

  RECORD_OR_DISPATCH(pps, am, OCODE_GMPTH, 1, sizeof(mp), &mp,
                     TRUE, FALSE);

  return GreModifyPath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreStrokePath
*/
LONG   APIENTRY GpiStrokePath(HPS hps,LONG a,ULONG b)
{
  ORDER_GMPTH mp;

  INIT_DRAWING(hps, GPI_ERROR);

  mp.uchMode     = GMPTH_STROKE;
  mp.uchReserved = 0;
  mp.idPath      = a;

  RECORD_OR_DISPATCH(pps, am, OCODE_GMPTH, 1, sizeof(mp), &mp,
                     GPI_OK, GPI_ERROR);

  return GreStrokePath(pps->hdc, a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreSelectClipPath
*/
BOOL   APIENTRY GpiSetClipPath(HPS hps,LONG a,LONG b)
{
  ORDER_GSCPTH cp;

  INIT_DRAWING(hps, FALSE);

  cp.fbFlags = 0;
  if (b & SCP_WINDING)  cp.fbFlags |= GSCPTH_WINDING;
  if (b & SCP_AND)      cp.fbFlags |= GSCPTH_INTERSECT;
  cp.uchReserved = 0;
  cp.idPath      = a;

  RECORD_OR_DISPATCH(pps, am, OCODE_GSCPTH, 1, sizeof(cp), &cp,
                     TRUE, FALSE);

  return GreSelectClipPath(pps->hdc, a, b);
}
