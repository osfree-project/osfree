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
        if (prclBounds) {
            memcpy(&rcl, (PVOID)prclBounds, sizeof(RECTL));
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
