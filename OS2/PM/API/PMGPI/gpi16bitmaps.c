#ifndef GPI16BITMAPS_INCLUDED
#define GPI16BITMAPS_INCLUDED

//BOOL    WINAPI BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
// fix prototype !!!
USHORT APIENTRY16 GPIBITBLT(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIDELETEBITMAP(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#ifndef BIT32
// For BIT32 imported from pmmerge and not found 16bit version
// fix prototype !!!
USHORT APIENTRY16 GPILOADBITMAP(void)
{
  return unimplemented(__FUNCTION__);
}
#endif

// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAP(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIWCBITBLT(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPICREATEBITMAP(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPBITS(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPDIMENSION(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPHANDLE(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYBITMAPPARAMETERS(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYDEVICEBITMAPFORMATS(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPIQUERYPEL(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAPBITS(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAPDIMENSION(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPISETBITMAPID(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

// fix prototype !!!
USHORT APIENTRY16 GPISETPEL(void)
{
#ifdef BIT32
	return unimplemented(__FUNCTION__);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
