/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16PRIMITIVES_INCLUDED
#define GPI16PRIMITIVES_INCLUDED

BOOL APIENTRY16 GPIBEGINAREA(HPS a, ULONG b)
{
#ifdef BIT32
	return GpiBeginArea(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIBOX(HPS a, LONG b, PPOINTL c, LONG d, LONG e)
{
#ifdef BIT32
	return GpiBox(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPICHARSTRING(HPS a, LONG b, PCH c)
{
#ifdef BIT32
	return GpiCharString(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPICHARSTRINGAT(HPS a, PPOINTL b, LONG c, PCH d)
{
#ifdef BIT32
	return GpiCharStringAt(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIENDAREA(HPS hps)
{
#ifdef BIT32
	return GpiEndArea(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPILINE(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiLine(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIMOVE(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiMove(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPOLYLINE(HPS a, LONG b, PPOINTL c)
{
#ifdef BIT32
	return GpiPolyLine(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//COLORREF WINAPI GetTextColor(HDC);
LONG APIENTRY16 GPIQUERYCOLOR(HPS hps)
{
#ifdef BIT32
	return GpiQueryColor(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYPATTERN(HPS hps)
{
#ifdef BIT32
	return GpiQueryPattern(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//COLORREF WINAPI SetTextColor(HDC, COLORREF);
BOOL APIENTRY16 GPISETCOLOR(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetColor(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETPATTERN(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetPattern(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPICHARSTRINGPOS(HPS a, PRECTL b, ULONG c, LONG d, PCH e, PLONG f)
{
#ifdef BIT32
	return GpiCharStringPos(a, b, c, d, e, f);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPICHARSTRINGPOSAT(HPS a, PPOINTL b, PRECTL c, ULONG d, LONG e, PCH f, PLONG g)
{
#ifdef BIT32
	return GpiCharStringPosAt(a, b, c, d, e, f, g);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPICOMMENT(HPS a, LONG b, PBYTE c)
{
#ifdef BIT32
	return GpiComment(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIFULLARC(HPS a, LONG b, FIXED c)
{
#ifdef BIT32
	return GpiFullArc(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIIMAGE(HPS a, LONG b, PSIZEL c, LONG d, PBYTE e)
{
#ifdef BIT32
	return GpiImage(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIMARKER(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiMarker(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPARTIALARC(HPS a, PPOINTL b, FIXED c, FIXED d, FIXED e)
{
#ifdef BIT32
	return GpiPartialArc(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPOINTARC(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiPointArc(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPOLYFILLET(HPS a, LONG b, PPOINTL c)
{
#ifdef BIT32
	return GpiPolyFillet(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPOLYFILLETSHARP(HPS a, LONG b, PPOINTL c, PFIXED d)
{
#ifdef BIT32
	return GpiPolyFilletSharp(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPOLYMARKER(HPS a, LONG b, PPOINTL c)
{
#ifdef BIT32
	return GpiPolyMarker(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPOLYSPLINE(HPS a, LONG b, PPOINTL c)
{
#ifdef BIT32
	return GpiPolySpline(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIPOP(HPS a, LONG b)
{
#ifdef BIT32
	return GpiPop(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIPTVISIBLE(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiPtVisible(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYARCPARAMS(HPS a, PARCPARAMS b)
{
#ifdef BIT32
	return GpiQueryArcParams(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYATTRMODE(HPS hps)
{
#ifdef BIT32
	return GpiQueryAttrMode(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYATTRS(HPS a, LONG b, ULONG c, PBUNDLE d)
{
#ifdef BIT32
	return GpiQueryAttrs(a, b, c, d);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//COLORREF WINAPI GetBkColor(HDC);
LONG APIENTRY16 GPIQUERYBACKCOLOR(HPS hps)
{
#ifdef BIT32
	return GpiQueryBackColor(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYBACKMIX(HPS hps)
{
#ifdef BIT32
	return GpiQueryBackMix(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYCHARANGLE(HPS a, PGRADIENTL b)
{
#ifdef BIT32
	return GpiQueryCharAngle(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYCHARBOX(HPS a, PSIZEF b)
{
#ifdef BIT32
	return GpiQueryCharBox(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYCHARDIRECTION(HPS hps)
{
#ifdef BIT32
	return GpiQueryCharDirection(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYCHARMODE(HPS hps)
{
#ifdef BIT32
	return GpiQueryCharMode(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYCHARSET(HPS hps)
{
#ifdef BIT32
	return GpiQueryCharSet(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYCHARSHEAR(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiQueryCharShear(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYCHARSTRINGPOS(HPS a, ULONG b, LONG c, PCH d, PLONG e, PPOINTL f)
{
#ifdef BIT32
	return GpiQueryCharStringPos(a, b, c, d, e, f);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYCHARSTRINGPOSAT(HPS a, PPOINTL b, ULONG c, LONG d, PCH e, PLONG f, PPOINTL g)
{
#ifdef BIT32
	return GpiQueryCharStringPosAt(a, b, c, d, e, f, g);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYCURRENTPOSITION(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiQueryCurrentPosition(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYDEFCHARBOX(HPS a, PSIZEL b)
{
#ifdef BIT32
	return GpiQueryDefCharBox(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYLINEEND(HPS hps)
{
#ifdef BIT32
	return GpiQueryLineEnd(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYLINEJOIN(HPS hps)
{
#ifdef BIT32
	return GpiQueryLineJoin(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYLINETYPE(HPS hps)
{
#ifdef BIT32
	return GpiQueryLineType(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

FIXED APIENTRY16 GPIQUERYLINEWIDTH(HPS hps)
{
#ifdef BIT32
	return GpiQueryLineWidth(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYLINEWIDTHGEOM(HPS hps)
{
#ifdef BIT32
	return GpiQueryLineWidthGeom(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYMARKER(HPS hps)
{
#ifdef BIT32
	return GpiQueryMarker(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYMARKERBOX(HPS a, PSIZEF b)
{
#ifdef BIT32
	return GpiQueryMarkerBox(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYMARKERSET(HPS hps)
{
#ifdef BIT32
	return GpiQueryMarkerSet(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYMIX(HPS hps)
{
#ifdef BIT32
	return GpiQueryMix(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYPATTERNREFPOINT(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiQueryPatternRefPoint(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYPATTERNSET(HPS hps)
{
#ifdef BIT32
	return GpiQueryPatternSet(hps);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYTEXTBOX(HPS a, LONG b, PCH c, LONG d, PPOINTL e)
{
#ifdef BIT32
	return GpiQueryTextBox(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIRECTVISIBLE(HPS a, PRECTL b)
{
#ifdef BIT32
	return GpiRectVisible(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETARCPARAMS(HPS a, PARCPARAMS b)
{
#ifdef BIT32
	return GpiSetArcParams(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETATTRMODE(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetAttrMode(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETATTRS(HPS a, LONG b, ULONG c, ULONG d, PBUNDLE e)
{
#ifdef BIT32
	return GpiSetAttrs(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

//COLORREF WINAPI SetBkColor(HDC, COLORREF);
BOOL APIENTRY16 GPISETBACKCOLOR(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetBackColor(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETBACKMIX(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetBackMix(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETCHARANGLE(HPS a, PGRADIENTL b)
{
#ifdef BIT32
	return GpiSetCharAngle(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETCHARBOX(HPS a, PSIZEF b)
{
#ifdef BIT32
	return GpiSetCharBox(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETCHARDIRECTION(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetCharDirection(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETCHARMODE(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetCharMode(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETCHARSET(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetCharSet(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETCHARSHEAR(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiSetCharShear(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETCURRENTPOSITION(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiSetCurrentPosition(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}


BOOL APIENTRY16 GPISETLINEEND(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetLineEnd(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETLINEJOIN(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetLineJoin(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETLINETYPE(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetLineType(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETLINEWIDTH(HPS a, FIXED b)
{
#ifdef BIT32
	return GpiSetLineWidth(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETLINEWIDTHGEOM(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetLineWidthGeom(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETMARKER(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetMarker(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETMARKERBOX(HPS a, PSIZEF b)
{
#ifdef BIT32
	return GpiSetMarkerBox(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETMARKERSET(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetMarkerSet(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETMIX(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetMix(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETPATTERNREFPOINT(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiSetPatternRefPoint(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETPATTERNSET(HPS a, LONG b)
{
#ifdef BIT32
	return GpiSetPatternSet(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#if APILEVEL > 1
LONG APIENTRY16 GPIPOLYLINEDISJOINT(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
#ifdef BIT32
	return GpiPolyLineDisjoint(hps, lCount, aptlPoints);
#else
	return unimplemented(__FUNCTION__);
#endif
}
#endif

#endif
