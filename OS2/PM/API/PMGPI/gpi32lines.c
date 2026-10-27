/*
  Documentation: https://www.edm2.com/index.php/GreDisjointLines
*/
LONG   APIENTRY GpiPolyLineDisjoint(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
  return GreDisjointLines(GpiQueryDevice(hps), aptlPoints, lCount);
}

/*
  Documentation: https://www.edm2.com/index.php/GreGetCurrentPosition
*/
BOOL   APIENTRY GpiQueryCurrentPosition(HPS hps,PPOINTL a)
{
  return GreGetCurrentPosition(GpiQueryDevice(hps), a);
}

/*
  Documentation: https://www.edm2.com/index.php/GreSetCurrentPosition
*/
BOOL   APIENTRY GpiSetCurrentPosition(HPS hps,PPOINTL a)
{
  return GreSetCurrentPosition(GpiQueryDevice(hps), a);
}

/*
  Documentation: https://www.edm2.com/index.php/GrePolyLine
*/
LONG   APIENTRY GpiPolyLine(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
  return GrePolyLine(GpiQueryDevice(hps), aptlPoints, lCount);
}
