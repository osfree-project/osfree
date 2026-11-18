/*!
 * @brief Dispatch for GreGetRegionBox — get the bounding box of a region.
 *
 * @param [in]  hdc        Device context handle.
 * @param [in]  hrgn       Region handle.
 * @param [out] prclRect   Pointer to a RECTL structure that receives the
 *                         dimensions of the tightest rectangle around the
 *                         region indicated by hrgn. The rectangle is always
 *                         returned in device coordinates.
 * @param [in]  pInstance  Pointer to instance data.
 * @param [in]  lFunction  High-order WORD=flags; low-order
 *                         WORD=NGreGetRegionBox.
 *
 * @return Integer (lComplexity) indicating the complexity of the region.
 * @retval RGN_ERROR   Error.
 * @retval RGN_NULL    Null region.
 * @retval RGN_RECT    Rectangular region.
 * @retval RGN_COMPLEX Complex region (more than 1 rectangle).
 *
 * @par EDM2 Reference
 * GreGetRegionBox (https://www.edm2.com/index.php/GreGetRegionBox):
 * "GreGetRegionBox loads the buffer pointed to by prclRect with the
 * dimensions of the tightest rectangle around the region indicated by hrgn.
 * Note that the rectangle is always returned in device coordinates."
 * "GreGetRegionBox raises an error when hrgn is the handle of the
 * currently selected clip region."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |  Can   |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HRGN_BUSY
 * - PMERR_INV_HRGN
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "When the region is empty, the rectangle is returned with the right
 * boundary equal to the left, and the top boundary equal to the bottom."
 * "GreGetRegionBox must return an error when the region indicated by hrgn
 * is selected into the DC as a clip region."
 *
 */
static LONG __cdecl GreGetRegionBox(HDC hdc, HRGN hrgn,
                                         PRECTL prclRect, PVOID pInstance,
                                         ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn || !prclRect) return RGN_ERROR;
    if (pRgn->IsClipSelected()) return RGN_ERROR;
    return (ULONG)pRgn->UpdateBounds(*prclRect);
}

/*!
 * @brief Dispatch for GreGetRegionRects — enumerate rectangles in a region.
 *
 * @param [in]  hdc            Device context handle.
 * @param [in]  hrgn           Region handle.
 * @param [in]  prclBoundRect  Pointer to a RECTL structure specifying a
 *                             bounding rectangle. Only rectangles
 *                             intersecting this bounding rectangle are
 *                             returned. If this pointer is NULL, all
 *                             rectangles in the region are enumerated.
 *                             If this pointer is not NULL, only rectangles
 *                             that intersect the bounding rectangle are
 *                             returned. Each of these rectangles is the
 *                             intersection of the bounding rectangle with
 *                             a rectangle in the region.
 * @param [in,out] pControl    Pointer to a RGNRECT structure controlling
 *                             the enumeration.
 * @param [out] parclRect      Pointer to an array of RECTL structures
 *                             that receive the rectangles defining the
 *                             region. All rectangle coordinates are
 *                             inclusive at the bottom-left corner, and
 *                             exclusive at the top-right corner. The
 *                             coordinates of the bounding box and the
 *                             rectangles returned will be in device
 *                             coordinates.
 * @param [in]  pInstance      Pointer to instance data.
 * @param [in]  lFunction      High-order WORD=flags; low-order
 *                             WORD=NGreGetRegionRects.
 *
 * @return BOOLEAN success indicator.
 * @retval TRUE   Successful.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreGetRegionRects (https://www.edm2.com/index.php/GreGetRegionRects):
 * "GreGetRegionRects fills the array pointed to by parclRect with a list
 * of (X,Y) coordinate pairs specifying the rectangles that together define
 * the region associated with hrgn."
 * "GreGetRegionRects raises an error when hrgn is the handle of a
 * currently selected clip region."
 * "This function can be used to determine if the banding rectangle is
 * inside, outside, or crossing the device's clip rectangle."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |  Can   |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HRGN_BUSY
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HRGN
 * - PMERR_INV_RECT
 * - PMERR_INV_REGION_CONTROL
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static BOOL __cdecl GreDrvGetRegionRects(HDC hdc, HRGN hrgn,
                                          PRECTL prclBoundRect, PRGNRECT pControl,
                                          PRECTL parclRect, PVOID pInstance,
                                          ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle(hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn) return FALSE;
    if (pRgn->IsClipSelected()) return FALSE;
    return pRgn->EnumRects((const RECTL *)prclBound,
                                   pControl, parclRect);
}

/*!
 * @brief Dispatch for GreOffsetRegion — move a region by specified offsets.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] hrgn       Region handle.
 * @param [in] pdpt       Pointer to a POINTL structure specifying the offset
 *                        by which the region is to be moved, in device
 *                        coordinates:
 *                        - dx: X offset
 *                        - dy: Y offset
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreOffsetRegion.
 *
 * @return BOOLEAN success indicator.
 * @retval TRUE   Successful.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreOffsetRegion (https://www.edm2.com/index.php/GreOffsetRegion):
 * "GreOffsetRegion moves the given region by the specified amounts (unless
 * the region is in use as a clipping region when an error is raised)."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_COORDINATE_OVERFLOW
 * - PMERR_HRGN_BUSY
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HRGN
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static BOOL __cdecl GreOffsetRegion(HDC hdc, HRGN hrgn, PPOINTL pdpt,
                                         PVOID pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle(hrgn);
    PPOINTL pptl = pdpt;
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn || !pptl) return FALSE;
    if (pRgn->IsClipSelected()) return FALSE;
    pRgn->Offset(pptl->x, pptl->y);
    return TRUE;
}


/*!
 * @brief Dispatch for GrePtInRegion — test if a point lies within a region.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] hrgn       Region handle.
 * @param [in] pptlPoint  Pointer to a POINTL structure giving the (x, y)
 *                        point to test, specified in device coordinates.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGrePtInRegion.
 *
 * @return Integer (lInside) indicating whether the point is inside the region.
 * @retval RGN_ERROR    Error.
 * @retval PRGN_INSIDE  Point is inside the region.
 * @retval PRGN_OUTSIDE Point is outside the region.
 *
 * @par EDM2 Reference
 * GrePtInRegion (https://www.edm2.com/index.php/GrePtInRegion):
 * "GrePtInRegion checks whether a point lies within a region."
 * "GrePtInRegion raises an error when hrgn is the handle of the currently
 * selected clip region. The point is always expected in device
 * coordinates."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |  Can   |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HRGN_BUSY
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HRGN
 * - PMERR_INV_RECT
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static LONG __cdecl GrePtInRegion(HDC hdc, HRGN hrgn, PPOINTL pptl,
                                       PVOID pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle(hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn || !pptl) return PRGN_ERROR;
    if (pRgn->IsClipSelected()) return PRGN_ERROR;
    return (pRgn->Contains(*pptl) ? PRGN_INSIDE : PRGN_OUTSIDE);
}

/*!
 * @brief Dispatch for GreRectInRegion — test if a rectangle lies within a region.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] hrgn       Region handle.
 * @param [in] prclRect   Pointer to a RECTL structure giving the rectangle
 *                        to test, in device coordinates:
 *                        - xLeft:   minimum x-coordinate
 *                        - yBottom: minimum y-coordinate
 *                        - xRight:  maximum x-coordinate (exclusive)
 *                        - yTop:    maximum y-coordinate (exclusive)
 *                        The rectangle passed is considered "exclusive" on
 *                        the right and top sides. If the rectangle represents
 *                        the bounding box of a graphic object to be drawn,
 *                        the drawing might include the right and top sides.
 *                        In this case, the caller should "inflate" the top
 *                        and right sides by one unit in device-coordinate
 *                        space.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreRectInRegion.
 *
 * @return Integer (lInside) indicating whether the rectangle is inside
 *         the region.
 * @retval RRGN_ERROR   Error.
 * @retval RRGN_INSIDE  All of the rectangle is inside the region.
 * @retval RRGN_PARTIAL Part of the rectangle is inside the region.
 * @retval RRGN_OUTSIDE None of the rectangle is inside the region.
 *
 * @par EDM2 Reference
 * GreRectInRegion (https://www.edm2.com/index.php/GreRectInRegion):
 * "GreRectInRegion checks whether any part of a given rectangle lies
 * within the specified region. GreRectInRegion raises an error if hrgn
 * is the handle of the currently selected clip region."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HRGN_BUSY
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HRGN
 * - PMERR_INV_RECT
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static LONG __cdecl GreRectInRegion(HDC hdc, HRGN hrgn,
                                         PRECTL prclRect, PVOID pInstance,
                                         ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle(hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn || !prclRect) return RRGN_ERROR;
    if (pRgn->IsClipSelected()) return RRGN_ERROR;
    return pRgn->RectIn(*prclRect);
}

/*!
 * @brief Dispatch for GreCreateRectRegion — create a region from rectangles.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] paRegion   Pointer to an array of RECTL structures defining
 *                        the region. Each rectangle is described by a
 *                        RECTL structure. If COM_TRANSFORM is not set,
 *                        the function expects the points to be in device
 *                        coordinates.
 * @param [in] cRect      Number of rectangles in the region definition.
 *                        If this is 0, an empty region is created.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreCreateRectRegion.
 *
 * @return Region handle (HRGN) on success, or RGN_ERROR on failure.
 * @retval HRGN       Handle to the newly created region.
 * @retval RGN_ERROR  Error occurred during region creation.
 *
 * @par EDM2 Reference
 * GreCreateRectRegion (https://www.edm2.com/index.php/GreCreateRectRegion):
 * "GreCreateRectRegion creates a region by taking the OR of a series of
 * rectangles. When no rectangles are specified (that is, cRect is 0), an
 * empty region is created."
 * "If COM_TRANSFORM is not set, the function expects the points to be in
 * device coordinates."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HRGN
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_RECT
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static HRGN __cdecl GreCreateRectRegion(HDC hdc, PRECT paRegion,
                                              LONG cRect, PVOID pInstance,
                                              ULONG lFunction)
{
    (void)hdc; (void)pInstance; (void)lFunction;
    return Fs_Region::CreateFromRects(cRect, (const RECTL *)paRegion);
}

/*!
 * @brief Dispatch for GreDestroyRegion — delete a region.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] hrgn       Region handle to destroy. When a null region is
 *                        specified, the function does not delete any region
 *                        and returns without logging an error.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreDestroyRegion.
 *
 * @return BOOLEAN success indicator.
 * @retval TRUE   Successful.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreDestroyRegion (https://www.edm2.com/index.php/GreDestroyRegion):
 * "GreDestroyRegion deletes the specified region unless it has been
 * selected as a clipping region. In this case, an error is raised."
 * "When a null region is specified, GreDestroyRegion does not delete any
 * region and returns without logging an error."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HRGN_BUSY
 * - PMERR_INV_HRGN
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static BOOL __cdecl GreDestroyRegion(HDC hdc, HRGN hrgn,
                                          PVOID pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle(hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn) return FALSE;
    if (pRgn->IsClipSelected()) return FALSE;
    delete pRgn;
    return TRUE;
}

   /**
    * @brief Set a rectangular region.
    *
    * @param[in] hdc   Device context handle.
    * @param[in] hrgn  Region handle.
    * @param[in] prcl  Pointer to a RECTL structure.
    * @param[in] lMode Mode.
    *
    * @return BOOLEAN success indicator.
    *
    * @retval TRUE  Successful.
    * @retval FALSE Error.
    *
    * @note If the function returns FALSE, the handling routine must call
    *       WinSetErrorInfo. WinGetLastError may return one of the following
    *       errors:
    *       - PMERR_HRGN_BUSY
    *       - PMERR_INV_COORDINATE
    *       - PMERR_INV_RECT
    */
static BOOL __cdecl GreDrvSetRectRegion(HDC hdc, HRGN hrgn,
                                          PRECTL paRegion, LONG cRect,
                                          PVOID pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle(hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn) return FALSE;
    if (pRgn->IsClipSelected()) return FALSE;
    return pRgn->SetFromRects(cRect, (const RECTL *)paRegion);
}

/*!
 * @brief Dispatch for GreCombineRegion — combine two regions into a third.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] hrgnDst    Destination region handle. May be the same as one
 *                        of the source regions.
 * @param [in] hrgnSrc1   First source region handle.
 * @param [in] hrgnSrc2   Second source region handle.
 * @param [in] cmdMode    Method of combination:
 *                        - CRGN_OR:   union of hrgnSrc1 and hrgnSrc2
 *                        - CRGN_COPY: uses hrgnSrc1 only; hrgnSrc2 is ignored
 *                        - CRGN_XOR:  symmetric difference of hrgnSrc1
 *                                     and hrgnSrc2
 *                        - CRGN_AND:  intersection of hrgnSrc1 and hrgnSrc2
 *                        - CRGN_DIFF: hrgnSrc1 and not hrgnSrc2
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreCombineRegion.
 *
 * @return Integer (lComplexity) indicating the complexity of the resulting
 *         region.
 * @retval RGN_ERROR   Error.
 * @retval RGN_NULL    Null region.
 * @retval RGN_RECT    Rectangular region.
 * @retval RGN_COMPLEX Complex region (more than 1 rectangle).
 *
 * @par EDM2 Reference
 * GreCombineRegion (https://www.edm2.com/index.php/GreCombineRegion):
 * "GreCombineRegion combines two regions to make a third. The destination
 * region can be the same as one of the source regions."
 * "An error is raised when any one of the specified regions is currently
 * selected as the clip region. All source and target regions must be of
 * the same device class."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HRGN_BUSY
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HRGN
 * - PMERR_INV_LENGTH_OR_COUNT
 * - PMERR_INV_RECT
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static LONG __cdecl GreCombineRegion(HDC hdc, HRGN hrgnDst,
                                          HRGN hrgnSrc1, HRGN hrgnSrc2,
                                          LONG cmdMode, PVOID pInstance,
                                          ULONG lFunction)
{
    Fs_Region *pDst = Fs_Region::FromHandle(hrgnDst);
    Fs_Region *pS1  = Fs_Region::FromHandle(hrgnSrc1);
    Fs_Region *pS2  = Fs_Region::FromHandle(hrgnSrc2);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pDst || !pS1 || !pS2) return RGN_ERROR;
    if (pDst->IsClipSelected() || pS1->IsClipSelected() || pS2->IsClipSelected())
        return RGN_ERROR;
    return pDst->Combine(*pS1, *pS2, cmdMode);
}

/*!
 * @brief Dispatch for GreCombineRectRegion — combine a region with a rectangle.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] hrgnDst    Destination region handle. May be the same as the
 *                        source region.
 * @param [in] prclRect   Pointer to a RECTL structure defining the
 *                        rectangle. If COM_TRANSFORM is not set, the
 *                        function expects the rectangle to be in device
 *                        coordinates.
 * @param [in] hrgnSrc    Source region handle.
 * @param [in] cmdMode    Method of combination:
 *                        - CRGN_OR:   union of hrgnSrc and prclRect
 *                        - CRGN_COPY: uses prclRect only; the hrgnSrc
 *                                     parameter is ignored
 *                        - CRGN_XOR:  symmetric difference of hrgnSrc
 *                                     and prclRect
 *                        - CRGN_AND:  intersection of hrgnSrc and prclRect
 *                        - CRGN_DIFF: hrgnSrc and not prclRect
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreCombineRectRegion.
 *
 * @return Integer (lComplexity) indicating the complexity of the resulting
 *         region.
 * @retval RGN_ERROR   Error.
 * @retval RGN_NULL    Null region.
 * @retval RGN_RECT    Rectangular region.
 * @retval RGN_COMPLEX Complex region (more than one rectangle).
 *
 * @par EDM2 Reference
 * GreCombineRectRegion (https://www.edm2.com/index.php/GreCombineRectRegion):
 * "GreCombineRectRegion combines a region with a rectangle to make a new
 * region."
 * "If COM_TRANSFORM is not set, the function expects the point to be in
 * device coordinates."
 * "The destination region can be the same as the source region."
 * "An error is raised if either of the regions specified is currently
 * selected as the clip region. The source and destination regions must be
 * of the same device class."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HRGN_BUSY
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_COORDINATE
 * - PMERR_INV_HRGN
 * - PMERR_INV_RECT
 * - PMERR_INV_REGION_MIX_MODE
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static LONG __cdecl GreCombineRectRegion(HDC hdc, HRGN hrgnDst,
                                              PRECTL prclRect, HRGN hrgnSrc,
                                              LONG cmdMode, PVOID pInstance,
                                              ULONG lFunction)
{
    Fs_Region *pDst = Fs_Region::FromHandle(hrgnDst);
    Fs_Region *pSrc = Fs_Region::FromHandle(hrgnSrc);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pDst || !pSrc || !prclRect) return RGN_ERROR;
    if (pDst->IsClipSelected() || pSrc->IsClipSelected())
        return RGN_ERROR;
    return pDst->CombineRect(*pSrc, *prclRect, cmdMode);
}

/*!
 * @brief Dispatch for GreCombineShortLineRegion — combine a SCANDATA area with a region.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] hrgn       Region handle. An error is raised when this region
 *                        is currently selected as the clip region.
 * @param [in] pScanData  Pointer to a SCANDATA structure representing the
 *                        area between polyshortline pairs to be ORed into
 *                        the region:
 *                        - pslFirstLeft:  pointer to left end of first polyshortline
 *                        - pslLastLeft:   pointer to left end of last polyshortline
 *                        - pslFirstRight: pointer to right edge of first polyshortline
 *                        - pslLastRight:  pointer to right edge of last polyshortline
 *                        - c:             number of scan lines
 *                        - rclBound:      RECTL structure defining the bounding rectangle
 *                        - slh:           SHORTLINEHEADER structure (line style, format,
 *                                         start/stop positions, left/right edges,
 *                                         pointers to next/prev shortline)
 *                        - ax:            array of X values as device coordinates
 *                        This structure is a discrete representation of a curve
 *                        that starts at point (x0, y0) and ends at point (x1, y1).
 *                        For each of the (y1-y0+1) rows, there is exactly one X
 *                        value contained in the ax array. The final point in the
 *                        series is not drawn.
 *                        The function always expects the points in the shortlines
 *                        to be in device coordinates.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreCombineShortLineRegion.
 *
 * @return BOOLEAN success indicator.
 * @retval TRUE   Successful.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreCombineShortLineRegion (https://www.edm2.com/index.php/GreCombineShortLineRegion):
 * "GreCombineShortLineRegion combines an area lying between polyshortline
 * pairs, which is represented by a SCANDATA structure, with a region."
 * "The pScanData parameter is ORed into the region."
 * "This function is used to build regions for path simulation."
 * "The function always expects the points in the shortlines to be in
 * device coordinates."
 * "An error is raised when the region specified is currently selected as
 * the clip region."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HRGN_BUSY
 * - PMERR_INSUFFICIENT_MEMORY
 * - PMERR_INV_HRGN
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static BOOL __cdecl GreCombineShortLineRegion(HDC hdc, HRGN hrgn,
                                                   PSCANDATA psl,
                                                   PVOID pInstance, ULONG lFunction)
{
    (void)hdc; (void)hrgn; (void)psl; (void)pInstance; (void)lFunction;
    return RGN_ERROR;
}

/*!
 * @brief Dispatch for GreEqualRegion — test whether two regions are identical.
 *
 * @param [in] hdc        Device context handle identifying the device that
 *                        owns both regions.
 * @param [in] hrgnSrc1   First region handle.
 * @param [in] hrgnSrc2   Second region handle.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGreEqualRegion.
 *
 * @return Integer (lEquality) indicating whether the regions are equal.
 * @retval EQRGN_EQUAL    Regions are equal.
 * @retval EQRGN_ERROR    Error.
 * @retval EQRGN_NOTEQUAL Regions are not equal.
 *
 * @par EDM2 Reference
 * GreEqualRegion (https://www.edm2.com/index.php/GreEqualRegion):
 * "GreEqualRegion checks whether two regions, owned by the device that is
 * identified by hdc, are identical."
 * "An error is raised when either region is currently selected as the
 * clip region."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HRGN_BUSY
 * - PMERR_INV_HRGN
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 */
static LONG __cdecl GreEqualRegion(HDC hdc, HRGN hrgn1, HRGN hrgn2,
                                        PVOID pInstance, ULONG lFunction)
{
    Fs_Region *p1 = Fs_Region::FromHandle(hrgn1);
    Fs_Region *p2 = Fs_Region::FromHandle(hrgn2);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!p1 || !p2) return EQRGN_ERROR;
    if (p1->IsClipSelected() || p2->IsClipSelected())
        return EQRGN_ERROR;
    return p1->Equal(*p2);
}

/*!
 * @brief Dispatch for GrePaintRegion — paint a region using current area colors.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] hrgn       Region handle to paint.
 * @param [in] pInstance  Pointer to instance data.
 * @param [in] lFunction  High-order WORD=flags; low-order
 *                        WORD=NGrePaintRegion.
 *
 * @return Integer (cHits) indicating correlation/error status.
 * @retval GPI_OK    Successful.
 * @retval GPI_HITS  Successful with correlation hit (returned by display
 *                   drivers when the correlation flag is ON, and a hit
 *                   is detected).
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GrePaintRegion (https://www.edm2.com/index.php/GrePaintRegion):
 * "GrePaintRegion paints the specified region by using the current area
 * foreground and background colors."
 *
 * @par Support
 * | Simulated | Hooked | Chained |
 * |-----------|--------|---------|
 * |    Yes    |   Can  |   Can   |
 *
 * @par Possible Errors
 * When an error is detected, the handling routine must call WinSetErrorInfo
 * to post the condition. Error codes for conditions that the handling
 * routine is expected to check include:
 * - PMERR_HDC_BUSY
 * - PMERR_HRGN_BUSY
 * - PMERR_INV_HDC
 * - PMERR_INV_HRGN
 * - PMERR_INV_IN_AREA
 * - PMERR_INV_IN_PATH
 * - PMERR_REGION_IS_CLIP_REGION
 *
 * Refer to the "Error Explanations" section in the Presentation Manager
 * Programming Reference for further explanation.
 *
 * @par Remarks
 * "Mixing is controlled only by the area foreground mix."
 * "GrePaintRegion returns an error when the region is currently selected
 * as a clip region."
 * "If the preprocessor in the graphics engine receives a GrePaintRegion
 * call for a null region, the preprocessor returns GPI_OK and does not
 * forward the call through the dispatch table to the handling routine."
 * "Presentation drivers for devices that do not support BitBlt operations
 * (for example, vector devices such as plotters) do not hook this
 * function. Such drivers hook only the function to return an error code."
 *
 */
static LONG __cdecl GrePaintRegion(HDC hdc, HRGN hrgn,
                                        PVOID pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    Fs_Region *pRgn = Fs_Region::FromHandle(hrgn);
    (void)pInstance; (void)lFunction;
    if (!pDc || !pRgn) return GPI_ERROR;
    if (pRgn->IsClipSelected()) return GPI_ERROR;
    return GPI_OK;
}

/**
 * @brief
 *
 * @param[in] a
 * @param[in] b
 * @param[in] c
 *
 * @return 
 *
 * @retval 
 *
 * @note Not found any information yet and not found any driver used it?
 */
static ULONG __cdecl GreSetRegionOwner(ULONG hdc, ULONG hrgn, ULONG lOwner,
                                           ULONG pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)hdc; (void)lOwner; (void)pInstance; (void)lFunction;
    if (!pRgn) return (ULONG)FALSE;
    return (ULONG)TRUE;
}

/**
 * @brief
 *
 * @param[in] a
 * @param[in] b
 * @param[in] c
 *
 * @return 
 *
 * @retval 
 *
 * @note Not found any information yet and not found any driver used it?
 */
static ULONG __cdecl GreFrameRegion(ULONG hdc, ULONG hrgn,
                                        ULONG pszl, ULONG pInstance,
                                        ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)pszl; (void)pInstance; (void)lFunction;
    if (!pDc || !pRgn) return (ULONG)GPI_ERROR;
    if (pRgn->IsClipSelected()) return (ULONG)GPI_ERROR;
    return (ULONG)GPI_OK;
}
