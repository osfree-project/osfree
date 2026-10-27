// @todo PHID real value?
#define PHID PVOID

/*
  Documentation: https://www.edm2.com/index.php/GreBeginArea
*/
BOOL   APIENTRY GpiBeginArea(HPS hps, ULONG flOptions)
{
  return GreBeginArea(GpiQueryDevice(hps), flOptions);
}

/*
  Documentation: https://www.edm2.com/index.php/GreEndArea
*/
LONG   APIENTRY GpiEndArea(HPS hps)
{
  // @todo EA_DRAW - value?
  return GreEndArea(GpiQueryDevice(hps), 0);
}

/*
  Documentation: https://www.edm2.com/index.php/GreBeginPath
*/
BOOL   APIENTRY GpiBeginPath(HPS hps, LONG lPath)
{
  return GreBeginPath(GpiQueryDevice(hps), lPath);
}


/*
  Documentation: https://www.edm2.com/index.php/GreEndPath
*/
BOOL   APIENTRY GpiEndPath(HPS hps)
{
  //@todo EA_DRAW 0 or another value??
  return GreEndPath(GpiQueryDevice(hps), 0);
}

/*
  Documentation: https://www.edm2.com/index.php/GreCloseFigure
*/
BOOL   APIENTRY GpiCloseFigure(HPS hps)
{
  return GreCloseFigure(GpiQueryDevice(hps));
}

/*
  Documentation: https://www.edm2.com/index.php/GreFillPath
*/
LONG   APIENTRY GpiFillPath(HPS hps,LONG a,LONG b)
{
  return GreFillPath(GpiQueryDevice(hps), a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreOutlinePath
*/
LONG   APIENTRY GpiOutlinePath(HPS hps,LONG a,LONG b)
{
  return GreOutlinePath(GpiQueryDevice(hps), a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreOutlinePath
*/
BOOL   APIENTRY GpiModifyPath(HPS hps,LONG a,LONG b)
{
  return GreModifyPath(GpiQueryDevice(hps), a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreStrokePath
*/
LONG   APIENTRY GpiStrokePath(HPS hps,LONG a,ULONG b)
{
  return GreStrokePath(GpiQueryDevice(hps), a, b);
}

/*
  Documentation: https://www.edm2.com/index.php/GreSelectClipPath
*/
BOOL   APIENTRY GpiSetClipPath(HPS hps,LONG a,LONG b)
{
  return GreSelectClipPath(GpiQueryDevice(hps), a, b);
}
