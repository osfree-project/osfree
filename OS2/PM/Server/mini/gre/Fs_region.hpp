#ifndef FS_REGION_HPP
#define FS_REGION_HPP

/*!
 * @class Fs_Region
 * @brief A clipping/visible region stored as an array of RECTL.
 *
 * Represents an OS/2 HRGN. Handle representation is (HRGN)(ULONG)this;
 * Fs_Region* round-trips through HRGN on 32-bit OS/2.  Storage uses
 * DosAllocMem/DosFreeMem exclusively (no C runtime heap, no STL).
 *
 * @par EDM2 Reference
 * "Region functions (INCL_GRE_REGIONS)" category — all entry points are
 * simulated by a handling routine in the graphics engine.
 */
class Fs_Region {
public:
    /*!
     * @brief Construct an empty region.
     * @return Nothing (constructor).
     * @retval (void)  No return value.
     */
    Fs_Region();

    /*!
     * @brief Construct a region containing one rectangle.
     * @param [in] r  Rectangle; coordinates stored as-given, not normalized.
     * @return Nothing (constructor).
     * @retval (void)  No return value.
     */
    explicit Fs_Region(const RECTL &r);

    /*!
     * @brief Destroy the region and release its storage.
     * @return Nothing (destructor).
     * @retval (void)  No return value.
     */
    ~Fs_Region();

    /*!
     * @brief Convert an HRGN handle back to the object pointer.
     * @param [in] hrgn  Handle previously returned by ToHandle().
     * @return Object pointer.
     * @retval NULL     hrgn was 0.
     * @retval non-NULL Valid object pointer.
     */
    static Fs_Region *FromHandle(HRGN hrgn);

    /*!
     * @brief Convert this object to an HRGN handle.
     * @return HRGN handle.
     * @retval non-NULL  Always (this pointer cast to ULONG).
     */
    HRGN ToHandle() const;

    /*!
     * @brief Allocate a new region from a rectangle array.
     * @param [in] cRect    Number of rectangles (may be 0).
     * @param [in] paRects  Rectangle array (NULL permitted when cRect == 0).
     * @return HRGN handle.
     * @retval HRGN_ERROR  Allocation failure or NULL paRects with cRect>0.
     * @retval non-NULL    Valid handle.
     *
     * @par EDM2 Reference
     * GreCreateRectRegion (https://www.edm2.com/index.php/GreCreateRectRegion):
     * "GreCreateRectRegion creates a region by taking the OR of a series
     * of rectangles. When no rectangles are specified (that is, cRect is 0),
     * an empty region is created. If COM_TRANSFORM is not set, the function
     * expects the points to be in device coordinates."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    static HRGN CreateFromRects(ULONG cRect, const RECTL *paRects);

    /*!
     * @brief Append a rectangle.
     * @param [in] r  Rectangle; stored as-given.
     * @return Nothing.
     * @retval (void)  No return value.
     * @warning Does not normalize; caller must supply a normalized rect.
     */
    void AddRect(const RECTL &r);

    /*!
     * @brief Append all rectangles of another region.
     * @param [in] other  Source region.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void AddRegion(const Fs_Region &other);

    /*!
     * @brief Replace content with a copy of another region.
     * @param [in] other  Source region.
     * @return Nothing.
     * @retval (void)  No return value.
     * @note If this == &other, the method is a no-op.
     */
    void CopyFrom(const Fs_Region &other);

    /*!
     * @brief Remove all rectangles.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void Clear();

    /*!
     * @brief Replace content with a normalized rectangle array.
     * @param [in] cRect    Number of rectangles (0 permitted).
     * @param [in] paRects  Source array (NULL permitted only when cRect==0).
     * @return Success indicator.
     * @retval TRUE   Content replaced.
     * @retval FALSE  NULL paRects with cRect>0, or allocation failure.
     *
     * @par EDM2 Reference
     * GreSetRectRegion (https://www.edm2.com/index.php/GreSetRectRegion):
     * "GreSetRectRegion replaces the contents of a region with the specified
     * array of rectangles."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL SetFromRects(ULONG cRect, const RECTL *paRects);

    /*!
     * @brief Number of rectangles in the region.
     * @return Count.
     * @retval 0    Empty region.
     * @retval >0   Number of rectangles.
     */
    ULONG Count() const { return m_ulCount; }

    /*!
     * @brief Mutable pointer to the rectangle array.
     * @return Pointer.
     * @retval NULL     Region is empty.
     * @retval non-NULL First rectangle pointer.
     */
    RECTL *Data() { return m_pRects; }

    /*!
     * @brief Const pointer to the rectangle array.
     * @return Pointer.
     * @retval NULL     Region is empty.
     * @retval non-NULL First rectangle pointer.
     */
    const RECTL *Data() const { return m_pRects; }

    /*!
     * @brief Empty-region test.
     * @return Indicator.
     * @retval TRUE   No rectangles.
     * @retval FALSE  At least one rectangle.
     */
    BOOL IsEmpty() const { return m_ulCount == 0; }

    /*!
     * @brief Query the clip-selected flag.
     * @return Flag value.
     * @retval TRUE   Selected as a DC's clip region.
     * @retval FALSE  Not selected.
     *
     * @par EDM2 Reference
     * GreDestroyRegion raises PMERR_REGION_IS_CLIP_REGION for selected regions.
     */
    BOOL IsClipSelected() const { return m_bClipSelected; }

    /*!
     * @brief Set or clear the clip-selected flag.
     * @param [in] b  New flag value.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SetClipSelected(BOOL b) { m_bClipSelected = b; }

    /*!
     * @brief Intersect this region with a rectangle (in place).
     * @param [in] r  Rectangle.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void IntersectRect(const RECTL &r);

    /*!
     * @brief Intersect this region with another region (in place).
     * @param [in] other  Source region.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void IntersectRegion(const Fs_Region &other);

    /*!
     * @brief Subtract a rectangle from this region (in place).
     * @param [in] r  Rectangle.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SubtractRect(const RECTL &r);

    /*!
     * @brief Subtract another region from this region (in place).
     * @param [in] other  Source region.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void SubtractRegion(const Fs_Region &other);

    /*!
     * @brief Offset every rectangle.
     * @param [in] dx  X delta.
     * @param [in] dy  Y delta.
     * @return Nothing.
     * @retval (void)  No return value.
     *
     * @par EDM2 Reference
     * GreOffsetRegion (https://www.edm2.com/index.php/GreOffsetRegion):
     * "GreOffsetRegion moves the given region by the specified amounts
     * (unless the region is in use as a clipping region when an error is raised)."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    void Offset(LONG dx, LONG dy);

    /*!
     * @brief Point-in-region test.
     * @param [in] pt  Point.
     * @return Indicator.
     * @retval TRUE   Inside.
     * @retval FALSE  Outside.
     *
     * @par EDM2 Reference
     * GrePtInRegion (https://www.edm2.com/index.php/GrePtInRegion):
     * "GrePtInRegion checks whether a point lies within a region.
     * GrePtInRegion raises an error when hrgn is the handle of the currently
     * selected clip region."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL Contains(const POINTL &pt) const;

    /*!
     * @brief Rectangle-overlap test.
     * @param [in] r  Rectangle.
     * @return Indicator.
     * @retval TRUE   Overlap.
     * @retval FALSE  No overlap.
     */
    BOOL Intersects(const RECTL &r) const;

    /*!
     * @brief Compute bounding rectangle and complexity.
     * @param [out] bounds  Receives bounding rectangle; set to {0,0,0,0}
     *                      when region is empty.
     * @return Complexity code.
     * @retval RGN_NULL     Region is empty.
     * @retval RGN_RECT     Exactly one rectangle.
     * @retval RGN_COMPLEX  Two or more rectangles.
     *
     * @par EDM2 Reference
     * GreGetRegionBox (https://www.edm2.com/index.php/GreGetRegionBox):
     * "GreGetRegionBox loads the buffer pointed to by prclRect with the
     * dimensions of the tightest rectangle around the region indicated by
     * hrgn."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG UpdateBounds(RECTL &bounds) const;

    /*!
     * @brief Enumerate the region's rectangles.
     * @param [in]     prclBound  Optional bounding filter (NULL = all).
     * @param [in,out] pControl   RGNRECT: ircStart, crc, crcReturned, ulDirection.
     * @param [out]    parclRect  Output buffer (NULL = count only).
     * @return Success indicator.
     * @retval TRUE   Enumeration completed.
     * @retval FALSE  pControl is NULL.
     *
     * @par EDM2 Reference
     * GreGetRegionRects (https://www.edm2.com/index.php/GreGetRegionRects):
     * "GreGetRegionRects fills the array pointed to by parclRect with a list
     * of (X,Y) coordinate pairs specifying the rectangles that together define
     * the region associated with hrgn."
     *
     * @warning ulDirection is always set to RECTDIR_LFRT_TOPBOT; other
     *          orderings (RECTDIR_RTLF_*, RECTDIR_*_BOTTOP) are not implemented.
     */
    BOOL EnumRects(const RECTL *prclBound, PRGNRECT pControl,
                   PRECTL parclRect) const;

    /*!
     * @brief Combine two regions into this region.
     * @param [in] s1    First source region.
     * @param [in] s2    Second source region.
     * @param [in] mode  CRGN_COPY / OR / AND / DIFF / XOR.
     * @return Complexity of the result.
     * @retval RGN_NULL     Result is empty.
     * @retval RGN_RECT     One rectangle.
     * @retval RGN_COMPLEX  Two or more rectangles.
     * @retval RGN_ERROR    Unknown mode.
     *
     * @par EDM2 Reference
     * GreCombineRegion (https://www.edm2.com/index.php/GreCombineRegion):
     * "GreCombineRegion combines two regions to make a third. The destination
     * region can be the same as one of the source regions. An error is raised
     * when any one of the specified regions is currently selected as the clip
     * region."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG Combine(const Fs_Region &s1, const Fs_Region &s2, ULONG mode);

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
     * GreCombineRectRegion (https://www.edm2.com/index.php/GreCombineRectRegion):
     * "GreCombineRectRegion combines a region with a rectangle to make a new
     * region. If COM_TRANSFORM is not set, the function expects the point to
     * be in device coordinates."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG CombineRect(const Fs_Region &s, const RECTL &r, ULONG mode);

    /*!
     * @brief Geometric equality test.
     * @param [in] other  Comparison region.
     * @return Equality code.
     * @retval EQRGN_EQUAL     Same geometry.
     * @retval EQRGN_NOTEQUAL  Different geometry.
     *
     * @par EDM2 Reference
     * GreEqualRegion (https://www.edm2.com/index.php/GreEqualRegion):
     * "GreEqualRegion returns an integer (lEquality) indicating whether two
     * regions are equal."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG Equal(const Fs_Region &other) const;

    /*!
     * @brief Rectangle containment test against the whole region.
     * @param [in] r  Rectangle.
     * @return Containment code.
     * @retval RRGN_INSIDE   Fully inside the region.
     * @retval RRGN_PARTIAL  Partially inside.
     * @retval RRGN_OUTSIDE  Fully outside.
     *
     * @par EDM2 Reference
     * GreRectInRegion (https://www.edm2.com/index.php/GreRectInRegion):
     * "GreRectInRegion checks whether any part of a given rectangle lies within
     * the specified region. GreRectInRegion raises an error if hrgn is the
     * handle of the currently selected clip region."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    LONG RectIn(const RECTL &r) const;

    /*!
     * @brief Deep-copy the region.
     * @return Newly-allocated region; caller owns the result.
     * @retval NULL     Allocation failure.
     * @retval non-NULL Valid copy (not clip-selected).
     */
    Fs_Region *Clone() const;

    /*!
     * @brief Half-open point-in-rectangle test.
     * @param [in] pt  Point.
     * @param [in] r   Rectangle.
     * @return Indicator.
     * @retval TRUE   Inside (left/bottom inclusive, right/top exclusive).
     * @retval FALSE  Outside.
     */
    static BOOL PointInRect(const POINTL &pt, const RECTL &r);

    /*!
     * @brief Normalize a rectangle (swap inverted coordinates).
     * @param [in,out] r  Rectangle.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    static void NormalizeRect(RECTL &r);

private:
    /*!
     * @brief Non-copyable (private copy constructor, not implemented).
     * @param [in] other  Source (ignored).
     * @return Nothing.
     * @retval (void)  No return value.
     */
    Fs_Region(const Fs_Region &);

    /*!
     * @brief Non-assignable (private assignment, not implemented).
     * @param [in] other  Source (ignored).
     * @return Reference to this (never used).
     * @retval (never)  Not implemented.
     */
    Fs_Region &operator=(const Fs_Region &);

    /*!
     * @brief Grow the rectangle buffer to hold at least cNeeded entries.
     * @param [in] cNeeded  Minimum required capacity.
     * @return Success indicator.
     * @retval TRUE   Buffer grown.
     * @retval FALSE  DosAllocMem failure.
     */
    BOOL Ensure(ULONG cNeeded);

    /*!
     * @brief Intersect two rectangles.
     * @param [in]  a    First rectangle.
     * @param [in]  b    Second rectangle.
     * @param [out] out  Intersection result.
     * @return Indicator.
     * @retval TRUE   Non-empty intersection.
     * @retval FALSE  Empty intersection.
     */
    static BOOL IntersectTwo(const RECTL &a, const RECTL &b, RECTL &out);

    RECTL *m_pRects;         /*!< Rectangle storage (DosAllocMem). */
    ULONG  m_ulCount;        /*!< Number of rectangles in use. */
    ULONG  m_ulAlloc;        /*!< Allocated capacity. */
    BOOL   m_bClipSelected;  /*!< Selected as a DC clip region. */
};

#endif /* FS_REGION_HPP */
