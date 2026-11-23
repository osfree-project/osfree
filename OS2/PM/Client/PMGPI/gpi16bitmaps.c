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

#ifdef BIT32
#include <stdlib.h>
#include <string.h>

/*!
   @brief Allocate a zero-filled block using OS/2 CPI.

   Same semantics as AllocMemZero in ps.c but local to this translation
   unit (pmgpi16.c does not include ps.c).
*/
static PVOID BmpAllocMemZero(ULONG cb)
{
    PVOID p = NULL;

    if (DosAllocMem(&p, cb, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
        return NULL;
    memset(p, 0, cb);
    return p;
}

/* ================================================================
 * 16-bit -> 32-bit bitmap structure conversion.
 *
 * Values per IBM OS/2 Toolkit:
 *   ulCompression   = BCA_UNCOMP       (0)  - v1 has no compression
 *   usUnits         = BRU_METRIC       (0)  - unspecified, metric
 *   usRecording     = BRA_BOTTOMUP     (0)  - v1 is bottom-up by def.
 *   usRendering     = BRH_NOTHALFTONED (0)  - no halftoning info
 *   ulColorEncoding = BCE_RGB          (0)  - v1 palette is RGB
 * Color table: RGB -> RGB2, fcOptions = 0 (reserved, unused).
 * cbFix is set to sizeof(BITMAPINFOHEADER2) = 64 (fixed part of v2).
 * Color table in the source v1 structure begins at offset p16->cbFix.
 * ================================================================ */

/* Number of palette entries described by a v1/v2 header.
   Returns 0 for direct-color formats (>8bpp) or invalid counts. */
static ULONG BmpColorCount(USHORT cPlanes, USHORT cBitCount)
{
    ULONG n = (ULONG)cPlanes * cBitCount;

    if (n == 0 || n > 8)
        return 0;
    return 1UL << n;
}

static PBITMAPINFOHEADER2 BmpHdr16To32(PBITMAPINFOHEADER p16)
{
    PBITMAPINFOHEADER2 p32;

    if (p16 == NULL) return NULL;
    if (p16->cbFix < sizeof(BITMAPINFOHEADER)) return NULL;

    p32 = (PBITMAPINFOHEADER2)BmpAllocMemZero(sizeof(BITMAPINFOHEADER2));
    if (p32 == NULL) return NULL;

    p32->cbFix           = sizeof(BITMAPINFOHEADER2);   /* 64 */
    p32->cx              = (ULONG)p16->cx;
    p32->cy              = (ULONG)p16->cy;
    p32->cPlanes         = p16->cPlanes;
    p32->cBitCount       = p16->cBitCount;
    p32->ulCompression   = BCA_UNCOMP;
    p32->cbImage         = 0;
    p32->cxResolution    = 0;
    p32->cyResolution    = 0;
    p32->cclrUsed        = 0;
    p32->cclrImportant   = 0;
    p32->usUnits         = BRU_METRIC;
    p32->usReserved      = 0;
    p32->usRecording     = BRA_BOTTOMUP;
    p32->usRendering     = BRH_NOTHALFTONED;
    p32->cSize1          = 0;
    p32->cSize2          = 0;
    p32->ulColorEncoding = BCE_RGB;
    p32->ulIdentifier    = 0;
    return p32;
}

static PBITMAPINFO2 BmpInfo16To32(PBITMAPINFO p16)
{
    PBITMAPINFO2 p32;
    RGB         *p16_colors;
    ULONG        cAlloc, cCopy, i;

    if (p16 == NULL) return NULL;
    if (p16->cbFix < sizeof(BITMAPINFOHEADER)) return NULL;

    cAlloc = BmpColorCount(p16->cPlanes, p16->cBitCount);

    /* Reserve full 256-entry palette: GpiQueryBitmapBits may fill in
       cPlanes / cBitCount / palette even when caller left cBitCount = 0
       (a documented "query info" invocation). In that case the driver
       may write up to 256 entries, so allocate that much even though
       the input has no palette to copy. */
    if (p16->cBitCount == 0)
        cAlloc = 256;

    /* Number of palette entries actually present in the input palette.
       For cBitCount == 0 the input has no palette (BmpColorCount
       returns 0). */
    cCopy = BmpColorCount(p16->cPlanes, p16->cBitCount);

    p32 = (PBITMAPINFO2)BmpAllocMemZero(
              sizeof(BITMAPINFOHEADER2) + cAlloc * sizeof(RGB2));
    if (p32 == NULL) return NULL;

    p32->cbFix           = sizeof(BITMAPINFOHEADER2);   /* 64 */
    p32->cx              = (ULONG)p16->cx;
    p32->cy              = (ULONG)p16->cy;
    p32->cPlanes         = p16->cPlanes;
    p32->cBitCount       = p16->cBitCount;
    p32->ulCompression   = BCA_UNCOMP;
    p32->cbImage         = 0;
    p32->cxResolution    = 0;
    p32->cyResolution    = 0;
    p32->cclrUsed        = 0;
    p32->cclrImportant   = 0;
    p32->usUnits         = BRU_METRIC;
    p32->usReserved      = 0;
    p32->usRecording     = BRA_BOTTOMUP;
    p32->usRendering     = BRH_NOTHALFTONED;
    p32->cSize1          = 0;
    p32->cSize2          = 0;
    p32->ulColorEncoding = BCE_RGB;
    p32->ulIdentifier    = 0;

    /* Color table begins at offset cbFix from start of v1 structure. */
    p16_colors = (RGB *)((PBYTE)p16 + p16->cbFix);
    for (i = 0; i < cCopy; i++) {
        p32->argbColor[i].bBlue     = p16_colors[i].bBlue;
        p32->argbColor[i].bGreen    = p16_colors[i].bGreen;
        p32->argbColor[i].bRed      = p16_colors[i].bRed;
        p32->argbColor[i].fcOptions = 0;
    }
    return p32;
}

/* Copy back fields that a v2 call may have modified (GpiQueryBitmapBits).
   Palette is written to the v1 color table at offset p16->cbFix. */
static void BmpInfo16UpdateFrom32(PBITMAPINFO p16, PBITMAPINFO2 p32)
{
    ULONG cColors, i;
    RGB  *p16_colors;

    if (p16 == NULL || p32 == NULL) return;
    if (p16->cbFix < sizeof(BITMAPINFOHEADER)) return;

    p16->cPlanes   = p32->cPlanes;
    p16->cBitCount = p32->cBitCount;

    cColors = BmpColorCount(p32->cPlanes, p32->cBitCount);
    if (cColors == 0) return;

    p16_colors = (RGB *)((PBYTE)p16 + p16->cbFix);
    for (i = 0; i < cColors; i++) {
        p16_colors[i].bBlue  = p32->argbColor[i].bBlue;
        p16_colors[i].bGreen = p32->argbColor[i].bGreen;
        p16_colors[i].bRed   = p32->argbColor[i].bRed;
    }
}
#endif /* BIT32 */


//BOOL    WINAPI BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
EXTERN LONG APIENTRY16 GPIBITBLT(HPS a, HPS b, LONG c, PPOINTL d, LONG e, ULONG f)
{
#ifdef BIT32
	return GpiBitBlt(a, b, c, d, e, f);
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN BOOL APIENTRY16 GPIDELETEBITMAP(HBITMAP a)
{
#ifdef BIT32
	return GpiDeleteBitmap(a);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#ifndef BIT32
// For BIT32 imported from pmmerge and not found 16bit version
EXTERN HBITMAP APIENTRY16 GPILOADBITMAP(HPS a, HMODULE b, USHORT c, LONG d, LONG e)
{
  return unimplemented(__FUNCTION__);
}
#endif

EXTERN HBITMAP APIENTRY16 GPISETBITMAP(HPS a, HBITMAP b)
{
#ifdef BIT32
	return GpiSetBitmap(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN LONG APIENTRY16 GPIWCBITBLT(HPS a, HBITMAP b, LONG c, PPOINTL d, LONG e, ULONG f)
{
#ifdef BIT32
	return GpiWCBitBlt(a, b, c, d, e, f);
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN HBITMAP APIENTRY16 GPICREATEBITMAP(HPS a, PBITMAPINFOHEADER b, ULONG c, PBYTE d, PBITMAPINFO e)
{
#ifdef BIT32
	PBITMAPINFOHEADER2 b2;
	PBITMAPINFO2       e2;
	HBITMAP            hbm;

	b2 = BmpHdr16To32(b);
	if (b2 == NULL) return (HBITMAP)0;

	e2 = BmpInfo16To32(e);
	if (e != NULL && e2 == NULL) {
		DosFreeMem(b2);
		return (HBITMAP)0;
	}

	hbm = GpiCreateBitmap(a, b2, c, d, e2);
	DosFreeMem(b2);
	if (e2 != NULL) DosFreeMem(e2);
	return hbm;
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN LONG APIENTRY16 GPIQUERYBITMAPBITS(HPS a, LONG b, LONG c, PBYTE d, PBITMAPINFO e)
{
#ifdef BIT32
	PBITMAPINFO2 e2;
	LONG        rc;

	e2 = BmpInfo16To32(e);
	if (e != NULL && e2 == NULL) return GPI_ERROR;

	rc = GpiQueryBitmapBits(a, b, c, d, e2);

	if (rc < 0) {
		/* GpiQueryBitmapBits failed (GPI_ERROR): do not copy back
		   possibly stale/garbage palette data. */
		if (e2 != NULL) DosFreeMem(e2);
		return GPI_ERROR;
	}

	if (e != NULL && e2 != NULL)
		BmpInfo16UpdateFrom32(e, e2);

	if (e2 != NULL) DosFreeMem(e2);
	return rc;
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN BOOL APIENTRY16 GPIQUERYBITMAPDIMENSION(HBITMAP a, PSIZEL b)
{
#ifdef BIT32
	return GpiQueryBitmapDimension(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN HBITMAP APIENTRY16 GPIQUERYBITMAPHANDLE(HPS a, LONG b)
{
#ifdef BIT32
	return GpiQueryBitmapHandle(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}


EXTERN BOOL APIENTRY16 GPIQUERYBITMAPPARAMETERS(HBITMAP a, PBITMAPINFOHEADER b)
{
#ifdef BIT32
	return GpiQueryBitmapParameters(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN BOOL APIENTRY16 GPIQUERYDEVICEBITMAPFORMATS(HPS a, LONG b, PLONG c)
{
#ifdef BIT32
	return GpiQueryDeviceBitmapFormats(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN LONG APIENTRY16 GPIQUERYPEL(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiQueryPel(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN LONG APIENTRY16 GPISETBITMAPBITS(HPS a, LONG b, LONG c, PBYTE d, PBITMAPINFO e)
{
#ifdef BIT32
	PBITMAPINFO2 e2;
	LONG        rc;

	e2 = BmpInfo16To32(e);
	if (e != NULL && e2 == NULL) return GPI_ERROR;

	rc = GpiSetBitmapBits(a, b, c, d, e2);

	if (e2 != NULL) DosFreeMem(e2);
	return rc;
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN BOOL APIENTRY16 GPISETBITMAPDIMENSION(HBITMAP a, PSIZEL b)
{
#ifdef BIT32
	return GpiSetBitmapDimension(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN BOOL APIENTRY16 GPISETBITMAPID(HPS a, HBITMAP b, LONG c)
{
#ifdef BIT32
	return GpiSetBitmapId(a, b, c);
#else
	return unimplemented(__FUNCTION__);
#endif
}

EXTERN LONG APIENTRY16 GPISETPEL(HPS a, PPOINTL b)
{
#ifdef BIT32
	return GpiSetPel(a, b);
#else
	return unimplemented(__FUNCTION__);
#endif
}

#endif
