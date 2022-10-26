/*!
   @file

   @ingroup pm

   @brief Presentation Manager 16-bit Graphics Programming Interface

   (c) osFree Project 2002-2008, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev (yuri.prokushev@gmail.com)

*/

#ifndef GPI16BITMAPS_INCLUDED
#define GPI16BITMAPS_INCLUDED

//BOOL    WINAPI BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
LONG APIENTRY16 GPIBITBLT(HPS a, HPS b, LONG c, PPOINTL d, LONG e, ULONG f)
{
#ifdef BIT32
	return GpiBitBlt(a, b, c, d, e, f);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIDELETEBITMAP(HBITMAP a)
{
#ifdef BIT32
	return GpiDeleteBitmap(a);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#ifndef BIT32
// For BIT32 imported from pmmerge and not found 16bit version
HBITMAP APIENTRY16 GPILOADBITMAP(HPS a, HMODULE b, USHORT c, LONG d, LONG e)
{
  return unimplemented(__FUNCTION__);
}
#endif

HBITMAP APIENTRY16 GPISETBITMAP(HPS a, HBITMAP b)
{
#ifdef BIT32
	return GpiSetBitmap(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIWCBITBLT(HPS a, HBITMAP b, LONG c, PPOINTL d, LONG e, ULONG f)
{
#ifdef BIT32
	return GpiWCBitBlt(a, b, c, d, e, f);
#else
	return unimplemented(__FUNCTION__);
#endif
}

HBITMAP APIENTRY16 GPICREATEBITMAP(HPS a, PBITMAPINFOHEADER b, ULONG c, PBYTE d, PBITMAPINFO e)
{
#ifdef BIT32
// @todo convert BITMAPINFOHEADER/BITMAPINFOHEADER2 and BITMAPINFO/BITMAPINFO2
//HBITMAP APIENTRY GpiCreateBitmap(HPS,PBITMAPINFOHEADER2,ULONG,PBYTE,PBITMAPINFO2);
	return GpiCreateBitmap(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYBITMAPBITS(HPS a, LONG b, LONG c, PBYTE d, PBITMAPINFO e)
{
#ifdef BIT32
// @todo convert BITMAPINFO/BITMAPINFO2
	return GpiQueryBitmapBits(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYBITMAPDIMENSION(HBITMAP a, PSIZEL b)
{
#ifdef BIT32
	return GpiQueryBitmapDimension(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

HBITMAP APIENTRY16 GPIQUERYBITMAPHANDLE(HPS a, LONG b)
{
#ifdef BIT32
	return GpiQueryBitmapHandle(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}


BOOL APIENTRY16 GPIQUERYBITMAPPARAMETERS(HBITMAP a, PBITMAPINFOHEADER b)
{
#ifdef BIT32
	return GpiQueryBitmapParameters(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPIQUERYDEVICEBITMAPFORMATS(HPS a, LONG b, PLONG c)
{
#ifdef BIT32
	return GpiQueryDeviceBitmapFormats(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPIQUERYPEL(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiQueryPel(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPISETBITMAPBITS(HPS a, LONG b, LONG c, PBYTE d, PBITMAPINFO e)
{
#ifdef BIT32
// @todo convert BITMAPINFO/BITMAPINFO2
	return GpiSetBitmapBits(a, b, c, d, e);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETBITMAPDIMENSION(HBITMAP a, PSIZEL b)
{
#ifdef BIT32
	return GpiSetBitmapDimension(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

BOOL APIENTRY16 GPISETBITMAPID(HPS a, HBITMAP b, LONG c)
{
#ifdef BIT32
	return GpiSetBitmapId(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

LONG APIENTRY16 GPISETPEL(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiSetPel(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
