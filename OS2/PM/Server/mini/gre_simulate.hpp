/*!
 * @brief Default GreSetupDC implementation in GRE.
 *
 * If the presentation driver does not override slot 0x81 in
 * FillLogicalDeviceBlock, GRE simulates the operation through the
 * mandatory driver hooks GreDeviceSetDCOrigin (0x40BB) and
 * GreNotifyClipChange (0x40A4). If the driver does override the slot,
 * this function is never invoked.
 *
 * @param [in] hdc         Device context handle.
 * @param [in] hrgnVis     Visible region handle (not used; region
 *                         support is a separate step).
 * @param [in] xOrg        DC origin X.
 * @param [in] yOrg        DC origin Y.
 * @param [in] prclBounds  Visible bounds rectangle (may be NULL).
 * @param [in] flFlags     SETUPDC_* flags.
 * @param [in] pInstance   Driver instance (passed by Gre32Entry8).
 * @param [in] fnCode      NGreSetupDC (0x4081).
 * @return GPI_OK on success, GPI_ERROR otherwise.
 * @retval GPI_OK    All requested suboperations succeeded.
 * @retval GPI_ERROR At least one mandatory Gre hook returned failure.
 */
static ULONG __cdecl GreDrvSetupDC(ULONG hdc, ULONG hrgnVis,
                                    ULONG xOrg, ULONG yOrg,
                                    ULONG prclBounds, ULONG flFlags,
                                    ULONG pInstance, ULONG fnCode)
{
    int  ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                              "[GRE] GreSetupDC(hdc=%lu, fl=0x%lx, fn=0x%lx)",
                              (unsigned long)hdc, (unsigned long)flFlags,
                              (unsigned long)fnCode);
    BOOL ok  = TRUE;

    (void)hrgnVis;
    (void)pInstance;

    if (flFlags & SETUPDC_ORIGIN) {
        POINTL ptl;
        ptl.x = (LONG)xOrg;
        ptl.y = (LONG)yOrg;
        if (!GreDeviceSetDCOrigin((HDC)hdc, &ptl))
            ok = FALSE;
    }

    if (flFlags & SETUPDC_VISRGN) {
        RECTL rcl;
        /* GreSetupDC receives prclBounds in device coordinates;
         * GreNotifyClipChange expects screen coordinates.
         * Convert device -> screen by adding the DC origin. */
        if (prclBounds) {
            PRECTL p = (PRECTL)prclBounds;
            rcl.xLeft   = p->xLeft   + (LONG)xOrg;
            rcl.yBottom = p->yBottom + (LONG)yOrg;
            rcl.xRight  = p->xRight  + (LONG)xOrg;
            rcl.yTop    = p->yTop    + (LONG)yOrg;
        } else {
            rcl.xLeft   = 0;
            rcl.yBottom = 0;
            rcl.xRight  = 0;
            rcl.yTop    = 0;
        }
        if (!GreNotifyClipChange((HDC)hdc, &rcl, 1, 0))
            ok = FALSE;
    }

    F_DbgCtxExitf(ctx, "%d", ok ? 1 : 0);
    return ok ? (ULONG)GPI_OK : (ULONG)GPI_ERROR;
}


/*!
 * @brief GRE-side default implementation of GreNotifyClipChange.
 *
 * This is the "default dispatch table" entry that a presentation
 * driver is expected to save (in FillLogicalDeviceBlock) and
 * re-dispatch to after its own processing, per OS/2 PM Ref Vol. 2
 * ("On completion, the handling routine must re-dispatch this call
 * to the graphics engine using the pointer supplied in the default
 * dispatch table").
 *
 * The minimal GRE keeps no DC-side clip state: the driver is
 * responsible for caching the rectangles.  The default entry
 * therefore only accepts the notification and reports success, which
 * matches the documented contract for drivers that do not cache
 * clip rectangles ("should return TRUE, if there are no errors").
 *
 * @param [in] hdc        Device context handle.
 * @param [in] prclBound  Bounding rectangle of the new clip region
 *                        in screen coordinates.
 * @param [in] cRect      Number of rectangles in the new clip region.
 * @param [in] idClipPath Current ClipPath ID.
 * @param [in] pInstance  Per-DC instance data (driver-owned).
 * @param [in] fnCode     NGreNotifyClipChange.
 * @return GPI_OK unconditionally.
 * @retval GPI_OK Notification accepted.
 */
static ULONG __cdecl GreDrvNotifyClipChange(ULONG hdc, ULONG prclBound,
                                             ULONG cRect, ULONG idClipPath,
                                             ULONG pInstance, ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE] GreNotifyClipChange(hdc=%lu, "
                             "cRect=%lu, idClipPath=%lu, fn=0x%lx)",
                             (unsigned long)hdc, (unsigned long)cRect,
                             (unsigned long)idClipPath,
                             (unsigned long)fnCode);
    (void)prclBound; (void)pInstance;
    F_DbgCtxExitf(ctx, "GPI_OK");
    return (ULONG)GPI_OK;
}
