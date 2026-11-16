#include "Fs_region.hpp"
#include <string.h>

/*!
 * @brief Construct an empty region.
 * @return Nothing (constructor).
 * @retval (void)  No return value.
 */
Fs_Region::Fs_Region()
    : m_pRects(NULL), m_ulCount(0), m_ulAlloc(0), m_bClipSelected(FALSE) {}

/*!
 * @brief Construct a region containing one rectangle.
 * @param [in] r  Rectangle (stored as-given).
 * @return Nothing (constructor).
 * @retval (void)  No return value.
 */
Fs_Region::Fs_Region(const RECTL &r)
    : m_pRects(NULL), m_ulCount(0), m_ulAlloc(0), m_bClipSelected(FALSE)
{ AddRect(r); }

/*!
 * @brief Destroy the region and release its storage.
 * @return Nothing (destructor).
 * @retval (void)  No return value.
 */
Fs_Region::~Fs_Region()
{
    if (m_pRects) { DosFreeMem((PVOID)m_pRects); m_pRects = NULL; }
    m_ulCount = m_ulAlloc = 0;
}

/*!
 * @brief Grow the rectangle buffer.
 * @param [in] cNeeded  Minimum required capacity.
 * @return Success indicator.
 * @retval TRUE   Buffer grown.
 * @retval FALSE  DosAllocMem failure.
 * @note Uses DosAllocMem + memcpy + DosFreeMem to emulate realloc,
 *       because DosAllocMem blocks have no OS/2-side realloc.
 */
BOOL Fs_Region::Ensure(ULONG cNeeded)
{
    if (cNeeded <= m_ulAlloc) return TRUE;
    ULONG cNew = m_ulAlloc ? m_ulAlloc : 4;
    while (cNew < cNeeded) cNew *= 2;
    PVOID pv = NULL;
    if (DosAllocMem(&pv, cNew * sizeof(RECTL),
                    PAG_READ | PAG_WRITE | PAG_COMMIT) != 0) return FALSE;
    if (m_pRects && m_ulCount)
        memcpy(pv, m_pRects, m_ulCount * sizeof(RECTL));
    if (m_pRects) DosFreeMem((PVOID)m_pRects);
    m_pRects = (RECTL *)pv;
    m_ulAlloc = cNew;
    return TRUE;
}

/*!
 * @brief Convert HRGN to Fs_Region pointer.
 * @param [in] hrgn  Handle.
 * @return Object pointer.
 * @retval NULL     hrgn was 0.
 * @retval non-NULL Valid object pointer.
 */
Fs_Region *Fs_Region::FromHandle(HRGN hrgn)
{ return (Fs_Region *)(ULONG)hrgn; }

/*!
 * @brief Convert this object to HRGN.
 * @return HRGN handle.
 * @retval non-NULL  Always.
 */
HRGN Fs_Region::ToHandle() const
{ return (HRGN)(ULONG)this; }

/*!
 * @brief Allocate a new region from a rectangle array.
 * @param [in] cRect    Number of rectangles.
 * @param [in] paRects  Rectangle array.
 * @return HRGN handle.
 * @retval HRGN_ERROR  Allocation failure or NULL paRects with cRect>0.
 * @retval non-NULL    Valid handle.
 *
 * @par EDM2 Reference
 * GreCreateRectRegion — see header.
 */
HRGN Fs_Region::CreateFromRects(ULONG cRect, const RECTL *paRects)
{
    Fs_Region *p = new Fs_Region();
    if (!p) return HRGN_ERROR;
    if (!p->SetFromRects(cRect, paRects)) { delete p; return HRGN_ERROR; }
    return p->ToHandle();
}

/*!
 * @brief Append one rectangle.
 * @param [in] r  Rectangle.
 * @return Nothing.
 * @retval (void)  No return value.
 * @note Silently ignored on allocation failure.
 */
void Fs_Region::AddRect(const RECTL &r)
{ if (!Ensure(m_ulCount + 1)) return; m_pRects[m_ulCount++] = r; }

/*!
 * @brief Append all rectangles of another region.
 * @param [in] other  Source region.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Region::AddRegion(const Fs_Region &other)
{ for (ULONG i = 0; i < other.m_ulCount; i++) AddRect(other.m_pRects[i]); }

/*!
 * @brief Replace content with a copy of another region.
 * @param [in] other  Source region.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Region::CopyFrom(const Fs_Region &other)
{ if (this == &other) return; Clear(); AddRegion(other); }

/*!
 * @brief Remove all rectangles.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Region::Clear() { m_ulCount = 0; }

/*!
 * @brief Replace content with a normalized rectangle array.
 * @param [in] cRect    Number of rectangles.
 * @param [in] paRects  Source array.
 * @return Success indicator.
 * @retval TRUE   Content replaced.
 * @retval FALSE  NULL paRects with cRect>0, or allocation failure.
 *
 * @par EDM2 Reference
 * GreSetRectRegion — see header.
 */
BOOL Fs_Region::SetFromRects(ULONG cRect, const RECTL *paRects)
{
    Clear();
    if (cRect == 0) return TRUE;
    if (!paRects || !Ensure(cRect)) return FALSE;
    for (ULONG i = 0; i < cRect; i++) {
        RECTL r = paRects[i];
        NormalizeRect(r);
        m_pRects[m_ulCount++] = r;
    }
    return TRUE;
}

/*!
 * @brief Half-open point-in-rectangle test.
 * @param [in] pt  Point.
 * @param [in] r   Rectangle.
 * @return Indicator.
 * @retval TRUE   Inside (left/bottom inclusive, right/top exclusive).
 * @retval FALSE  Outside.
 */
BOOL Fs_Region::PointInRect(const POINTL &pt, const RECTL &r)
{ return (pt.x >= r.xLeft && pt.x < r.xRight &&
          pt.y >= r.yBottom && pt.y < r.yTop) ? TRUE : FALSE; }

/*!
 * @brief Normalize a rectangle.
 * @param [in,out] r  Rectangle.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Region::NormalizeRect(RECTL &r)
{
    LONG t;
    if (r.xLeft > r.xRight) { t = r.xLeft; r.xLeft = r.xRight; r.xRight = t; }
    if (r.yBottom > r.yTop) { t = r.yBottom; r.yBottom = r.yTop; r.yTop = t; }
}

/*!
 * @brief Intersect two rectangles.
 * @param [in]  a    First rectangle.
 * @param [in]  b    Second rectangle.
 * @param [out] out  Intersection.
 * @return Indicator.
 * @retval TRUE   Non-empty intersection.
 * @retval FALSE  Empty intersection.
 */
BOOL Fs_Region::IntersectTwo(const RECTL &a, const RECTL &b, RECTL &out)
{
    out.xLeft   = (a.xLeft   > b.xLeft)   ? a.xLeft   : b.xLeft;
    out.yBottom = (a.yBottom > b.yBottom) ? a.yBottom : b.yBottom;
    out.xRight  = (a.xRight  < b.xRight)  ? a.xRight  : b.xRight;
    out.yTop    = (a.yTop    < b.yTop)    ? a.yTop    : b.yTop;
    return (out.xLeft < out.xRight && out.yBottom < out.yTop) ? TRUE : FALSE;
}

/*!
 * @brief Intersect this region with a rectangle.
 * @param [in] r  Rectangle.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Region::IntersectRect(const RECTL &r)
{
    ULONG out = 0;
    for (ULONG i = 0; i < m_ulCount; i++) {
        RECTL rcl;
        if (IntersectTwo(m_pRects[i], r, rcl)) m_pRects[out++] = rcl;
    }
    m_ulCount = out;
}

/*!
 * @brief Intersect this region with another region.
 * @param [in] other  Source region.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Region::IntersectRegion(const Fs_Region &other)
{
    Fs_Region result;
    for (ULONG i = 0; i < m_ulCount; i++)
        for (ULONG j = 0; j < other.m_ulCount; j++) {
            RECTL r;
            if (IntersectTwo(m_pRects[i], other.m_pRects[j], r)) result.AddRect(r);
        }
    CopyFrom(result);
}

/*!
 * @brief Subtract a rectangle from this region.
 * @param [in] r  Rectangle.
 * @return Nothing.
 * @retval (void)  No return value.
 * @note Partial overlaps are split into up to four disjoint rectangles.
 */
void Fs_Region::SubtractRect(const RECTL &r)
{
    if (m_ulCount == 0) return;
    ULONG cMax = m_ulCount * 4 + 1;
    PVOID pv = NULL;
    if (DosAllocMem(&pv, cMax * sizeof(RECTL),
                    PAG_READ | PAG_WRITE | PAG_COMMIT) != 0) return;
    RECTL *pNew = (RECTL *)pv;
    ULONG out = 0;
    for (ULONG i = 0; i < m_ulCount; i++) {
        const RECTL &orig = m_pRects[i];
        RECTL inter;
        if (!IntersectTwo(orig, r, inter)) { pNew[out++] = orig; continue; }
        if (orig.yTop > r.yTop) {
            pNew[out].xLeft=orig.xLeft; pNew[out].yBottom=r.yTop;
            pNew[out].xRight=orig.xRight; pNew[out].yTop=orig.yTop; out++;
        }
        if (orig.yBottom < r.yBottom) {
            pNew[out].xLeft=orig.xLeft; pNew[out].yBottom=orig.yBottom;
            pNew[out].xRight=orig.xRight; pNew[out].yTop=r.yBottom; out++;
        }
        LONG yb = (orig.yBottom > r.yBottom) ? orig.yBottom : r.yBottom;
        LONG yt = (orig.yTop    < r.yTop)    ? orig.yTop    : r.yTop;
        if (yb < yt) {
            if (orig.xLeft < r.xLeft) {
                pNew[out].xLeft=orig.xLeft; pNew[out].yBottom=yb;
                pNew[out].xRight=r.xLeft; pNew[out].yTop=yt; out++;
            }
            if (orig.xRight > r.xRight) {
                pNew[out].xLeft=r.xRight; pNew[out].yBottom=yb;
                pNew[out].xRight=orig.xRight; pNew[out].yTop=yt; out++;
            }
        }
    }
    DosFreeMem((PVOID)m_pRects);
    m_pRects = pNew; m_ulCount = out; m_ulAlloc = cMax;
}

/*!
 * @brief Subtract another region from this region.
 * @param [in] other  Source region.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Region::SubtractRegion(const Fs_Region &other)
{ for (ULONG i = 0; i < other.Count(); i++) SubtractRect(other.Data()[i]); }

/*!
 * @brief Offset every rectangle.
 * @param [in] dx  X delta.
 * @param [in] dy  Y delta.
 * @return Nothing.
 * @retval (void)  No return value.
 *
 * @par EDM2 Reference
 * GreOffsetRegion — see header.
 */
void Fs_Region::Offset(LONG dx, LONG dy)
{
    for (ULONG i = 0; i < m_ulCount; i++) {
        m_pRects[i].xLeft   += dx; m_pRects[i].xRight  += dx;
        m_pRects[i].yBottom += dy; m_pRects[i].yTop    += dy;
    }
}

/*!
 * @brief Point-in-region test.
 * @param [in] pt  Point.
 * @return Indicator.
 * @retval TRUE   Inside.
 * @retval FALSE  Outside.
 *
 * @par EDM2 Reference
 * GrePtInRegion — see header.
 */
BOOL Fs_Region::Contains(const POINTL &pt) const
{
    for (ULONG i = 0; i < m_ulCount; i++)
        if (PointInRect(pt, m_pRects[i])) return TRUE;
    return FALSE;
}

/*!
 * @brief Rectangle-overlap test.
 * @param [in] r  Rectangle.
 * @return Indicator.
 * @retval TRUE   Overlap.
 * @retval FALSE  No overlap.
 */
BOOL Fs_Region::Intersects(const RECTL &r) const
{
    for (ULONG i = 0; i < m_ulCount; i++) {
        RECTL t;
        if (IntersectTwo(m_pRects[i], r, t)) return TRUE;
    }
    return FALSE;
}

/*!
 * @brief Compute bounding rectangle and complexity.
 * @param [out] bounds  Bounding rectangle.
 * @return Complexity code.
 * @retval RGN_NULL     Region is empty.
 * @retval RGN_RECT     Exactly one rectangle.
 * @retval RGN_COMPLEX  Two or more rectangles.
 *
 * @par EDM2 Reference
 * GreGetRegionBox — see header.
 */
LONG Fs_Region::UpdateBounds(RECTL &bounds) const
{
    if (m_ulCount == 0) {
        bounds.xLeft = bounds.xRight = 0;
        bounds.yBottom = bounds.yTop = 0;
        return RGN_NULL;
    }
    bounds = m_pRects[0];
    for (ULONG i = 1; i < m_ulCount; i++) {
        if (m_pRects[i].xLeft   < bounds.xLeft)   bounds.xLeft   = m_pRects[i].xLeft;
        if (m_pRects[i].yBottom < bounds.yBottom) bounds.yBottom = m_pRects[i].yBottom;
        if (m_pRects[i].xRight  > bounds.xRight)  bounds.xRight  = m_pRects[i].xRight;
        if (m_pRects[i].yTop    > bounds.yTop)    bounds.yTop    = m_pRects[i].yTop;
    }
    return (m_ulCount == 1) ? RGN_RECT : RGN_COMPLEX;
}

/*!
 * @brief Enumerate the region's rectangles.
 * @param [in]     prclBound  Optional bounding filter (NULL = all).
 * @param [in,out] pControl   RGNRECT.
 * @param [out]    parclRect  Output buffer (NULL = count only).
 * @return Success indicator.
 * @retval TRUE   Enumeration completed.
 * @retval FALSE  pControl is NULL.
 *
 * @par EDM2 Reference
 * GreGetRegionRects — see header.
 *
 * @warning ulDirection always RECTDIR_LFRT_TOPBOT.
 */
BOOL Fs_Region::EnumRects(const RECTL *prclBound, PRGNRECT pControl,
                          PRECTL parclRect) const
{
    if (!pControl) return FALSE;
    ULONG ircStart = pControl->ircStart ? pControl->ircStart : 1;
    ULONG cWritten = 0;
    for (ULONG i = ircStart - 1; i < m_ulCount; i++) {
        RECTL rcl = m_pRects[i];
        if (prclBound) {
            RECTL t;
            if (!IntersectTwo(rcl, *prclBound, t)) continue;
            rcl = t;
        }
        if (!parclRect) cWritten++;
        else if (cWritten < pControl->crc) parclRect[cWritten++] = rcl;
        else break;
    }
    pControl->crcReturned = cWritten;
    pControl->ircStart   += cWritten;
    pControl->ulDirection = RECTDIR_LFRT_TOPBOT;
    return TRUE;
}

/*!
 * @brief Combine two regions into this region.
 * @param [in] s1    First source.
 * @param [in] s2    Second source.
 * @param [in] mode  CRGN_COPY / OR / AND / DIFF / XOR.
 * @return Complexity of the result.
 * @retval RGN_NULL     Result is empty.
 * @retval RGN_RECT     One rectangle.
 * @retval RGN_COMPLEX  Two or more rectangles.
 * @retval RGN_ERROR    Unknown mode.
 *
 * @par EDM2 Reference
 * GreCombineRegion — see header.
 */
LONG Fs_Region::Combine(const Fs_Region &s1, const Fs_Region &s2, ULONG mode)
{
    Fs_Region tmp;
    switch (mode) {
    case CRGN_COPY: tmp.CopyFrom(s1); break;
    case CRGN_OR:
        tmp.CopyFrom(s1);
        for (ULONG j = 0; j < s2.Count(); j++) {
            Fs_Region piece;
            piece.AddRect(s2.Data()[j]);
            piece.SubtractRegion(tmp);
            tmp.AddRegion(piece);
        }
        break;
    case CRGN_AND:
        for (ULONG i = 0; i < s1.Count(); i++)
            for (ULONG j = 0; j < s2.Count(); j++) {
                RECTL r;
                if (IntersectTwo(s1.Data()[i], s2.Data()[j], r)) tmp.AddRect(r);
            }
        break;
    case CRGN_DIFF:
        tmp.CopyFrom(s1); tmp.SubtractRegion(s2); break;
    case CRGN_XOR: {
        Fs_Region a, b;
        a.CopyFrom(s1); a.SubtractRegion(s2);
        b.CopyFrom(s2); b.SubtractRegion(s1);
        tmp.CopyFrom(a); tmp.AddRegion(b);
        break;
    }
    default: return RGN_ERROR;
    }
    CopyFrom(tmp);
    RECTL b;
    return UpdateBounds(b);
}

/*!
 * @brief Combine a region with a rectangle.
 * @param [in] s     Source region.
 * @param [in] r     Rectangle.
 * @param [in] mode  CRGN_COPY / OR / AND / DIFF / XOR.
 * @return Complexity of the result.
 * @retval RGN_NULL     Result is empty.
 * @retval RGN_RECT     One rectangle.
 * @retval RGN_COMPLEX  Two or more rectangles.
 * @retval RGN_ERROR    Unknown mode.
 *
 * @par EDM2 Reference
 * GreCombineRectRegion — see header.
 */
LONG Fs_Region::CombineRect(const Fs_Region &s, const RECTL &r, ULONG mode)
{
    Fs_Region tmp;
    switch (mode) {
    case CRGN_COPY: tmp.AddRect(r); break;
    case CRGN_OR:
        tmp.CopyFrom(s);
        { Fs_Region p; p.AddRect(r); p.SubtractRegion(tmp); tmp.AddRegion(p); }
        break;
    case CRGN_AND:
        for (ULONG i = 0; i < s.Count(); i++) {
            RECTL o;
            if (IntersectTwo(s.Data()[i], r, o)) tmp.AddRect(o);
        }
        break;
    case CRGN_DIFF:
        tmp.CopyFrom(s); tmp.SubtractRect(r); break;
    case CRGN_XOR: {
        Fs_Region a, b;
        a.CopyFrom(s); a.SubtractRect(r);
        b.AddRect(r);  b.SubtractRegion(s);
        tmp.CopyFrom(a); tmp.AddRegion(b);
        break;
    }
    default: return RGN_ERROR;
    }
    CopyFrom(tmp);
    RECTL b;
    return UpdateBounds(b);
}

/*!
 * @brief Geometric equality test.
 * @param [in] other  Comparison region.
 * @return Equality code.
 * @retval EQRGN_EQUAL     Same geometry.
 * @retval EQRGN_NOTEQUAL  Different geometry.
 *
 * @par EDM2 Reference
 * GreEqualRegion — see header.
 */
LONG Fs_Region::Equal(const Fs_Region &other) const
{
    Fs_Region d1 = *this;
    d1.SubtractRegion(other);
    if (!d1.IsEmpty()) return EQRGN_NOTEQUAL;
    Fs_Region d2 = other;
    d2.SubtractRegion(*this);
    return d2.IsEmpty() ? EQRGN_EQUAL : EQRGN_NOTEQUAL;
}

/*!
 * @brief Rectangle containment test.
 * @param [in] r  Rectangle.
 * @return Containment code.
 * @retval RRGN_INSIDE   Fully inside the region.
 * @retval RRGN_PARTIAL  Partially inside.
 * @retval RRGN_OUTSIDE  Fully outside.
 *
 * @par EDM2 Reference
 * GreRectInRegion — see header.
 */
LONG Fs_Region::RectIn(const RECTL &r) const
{
    RECTL nr = r;
    NormalizeRect(nr);
    if (!Intersects(nr)) return RRGN_OUTSIDE;
    Fs_Region p;
    p.AddRect(nr);
    p.SubtractRegion(*this);
    return p.IsEmpty() ? RRGN_INSIDE : RRGN_PARTIAL;
}

/*!
 * @brief Deep-copy the region.
 * @return Newly-allocated region.
 * @retval NULL     Allocation failure.
 * @retval non-NULL Valid copy (not clip-selected).
 */
Fs_Region *Fs_Region::Clone() const
{
    Fs_Region *p = new Fs_Region();
    if (!p) return NULL;
    p->CopyFrom(*this);
    p->m_bClipSelected = FALSE;
    return p;
}
