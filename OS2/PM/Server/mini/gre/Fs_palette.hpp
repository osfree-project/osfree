#ifndef FS_PALETTE_HPP
#define FS_PALETTE_HPP

/*!
 * @class Fs_Palette
 * @brief Palette object owned by one DC.
 *
 * Stores a PALETTEINFO-style array of RGB2 entries.  Per EDM2 Revision 2.0+
 * all palette-manager entry points are simulated by the graphics engine.
 *
 * @par EDM2 Reference
 * "Palette Manager functions": GreDeviceCreatePalette,
 * GreDeviceDeletePalette, GreDeviceSetPaletteEntries,
 * GreDeviceAnimatePalette, GreDeviceResizePalette, GreRealizePalette,
 * GreQueryHWPaletteInfo, GreUpdateColors, GreQueryPaletteRealization.
 * All are simulated by a handling routine in Revision 2.0 (or later) of
 * the graphics engine.
 */
class Fs_Palette {
public:
    /*!
     * @brief Construct an uncreated palette.
     * @return Nothing (constructor).
     * @retval (void)  No return value.
     */
    Fs_Palette();

    /*!
     * @brief Destroy and release entry storage.
     * @return Nothing (destructor).
     * @retval (void)  No return value.
     */
    ~Fs_Palette();

    /*!
     * @brief Reset to initial state (no palette, no entries).
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void Init();

    /*!
     * @brief Alias for Init.
     * @return Nothing.
     * @retval (void)  No return value.
     */
    void Shutdown();

    /*!
     * @brief Create a palette object for the DC.
     * @param [in] flOptions  PC_RESERVED / PC_EXPLICIT / PC_NOCOLLAPSE.
     * @param [in] ulFormat   Entry format.
     * @param [in] cEntries   Entry count (0 defaults to 256).
     * @param [in] pEntries   Optional initial entries (ULONG RGB).
     * @return Success indicator.
     * @retval TRUE   Created.
     * @retval FALSE  Allocation failure.
     *
     * @par EDM2 Reference
     * GreDeviceCreatePalette
     * (https://www.edm2.com/index.php/GreDeviceCreatePalette):
     * "GreDeviceCreatePalette is called by GpiSelectPalette, to process an
     * application's GpiSelectPalette request."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in Revision 2.0
     * (or later) of the graphics engine."
     */
    BOOL Create(ULONG flOptions, ULONG ulFormat,
                ULONG cEntries, const ULONG *pEntries);

    /*!
     * @brief Delete the palette object.
     * @return Success indicator.
     * @retval TRUE   Deleted.
     * @retval FALSE  No palette was created.
     *
     * @par EDM2 Reference
     * GreDeviceDeletePalette
     * (https://www.edm2.com/index.php/GreDeviceDeletePalette):
     * "GreDeviceDeletePalette is called by the graphics engine when a palette
     * is selected out of a device context."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in Revision 2.0
     * (or later) of the graphics engine."
     */
    BOOL Delete();

    /*!
     * @brief Change entries in the palette.
     * @param [in] ulFormat  Entry format.
     * @param [in] ulStart   Start index.
     * @param [in] cCount    Count.
     * @param [in] pEntries  New RGB entries.
     * @return Success indicator.
     * @retval TRUE   Entries changed.
     * @retval FALSE  No palette, NULL entries, or invalid start.
     *
     * @par EDM2 Reference
     * GreDeviceSetPaletteEntries
     * (https://www.edm2.com/index.php/GreDeviceSetPaletteEntries):
     * "GreDeviceSetPaletteEntries changes the entries in a palette as a
     * result of an application's GpiSetPaletteEntries request."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in Revision 2.0
     * (or later) of the graphics engine."
     */
    BOOL SetEntries(ULONG ulFormat, ULONG ulStart,
                    ULONG cCount, const ULONG *pEntries);

    /*!
     * @brief Animate the palette.
     * @param [in] ulFormat  Entry format.
     * @param [in] ulStart   Start index.
     * @param [in] cCount    Count.
     * @param [in] pEntries  New RGB entries.
     * @return Success indicator.
     * @retval TRUE   Animated.
     * @retval FALSE  No palette or invalid arguments.
     *
     * @par EDM2 Reference
     * GreDeviceAnimatePalette
     * (https://www.edm2.com/index.php/GreDeviceAnimatePalette):
     * "GreDeviceAnimatePalette is the presentation driver version of
     * GpiAnimatePalette."
     * "This function modifies the colors in a palette. Changes are reflected
     * immediately for each animated index that has a hardware slot."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in Revision 2.0
     * (or higher) of the graphics engine."
     *
     * @warning Immediate reflection on hardware slots is not implemented;
     *          the call simply updates stored entries.
     */
    BOOL Animate(ULONG ulFormat, ULONG ulStart,
                 ULONG cCount, const ULONG *pEntries);

    /*!
     * @brief Resize the palette.
     * @param [in] cNewEntries  New entry count.
     * @return Success indicator.
     * @retval TRUE   Resized.
     * @retval FALSE  No palette or zero count.
     *
     * @par EDM2 Reference
     * GreDeviceResizePalette (simulated, Revision 2.0+ per PDRREF).
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in Revision 2.0
     * (or higher) of the graphics engine."
     */
    BOOL Resize(ULONG cNewEntries);

    /*!
     * @brief Realize the palette.
     * @param [in]  flType    RP_BACKGROUND / RP_FOREGROUND /
     *                        RP_DEFAULTSCHANGED.
     * @param [out] pulCount  Optional output count.
     * @return Success indicator.
     * @retval TRUE   Realized.
     * @retval FALSE  No palette.
     *
     * @par EDM2 Reference
     * GreRealizePalette
     * (https://www.edm2.com/index.php/GreRealizePalette):
     * "GreRealizePalette is called by the Window Manager during processing of
     * a call to WinRealizePalette. GreRealizePalette takes a logical palette
     * and assigns it slots in the hardware palette."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in Revision 2.0
     * (or higher) of the graphics engine."
     */
    BOOL Realize(ULONG flType, PULONG pulCount);

    /*!
     * @brief Query hardware palette info.
     * @param [in]  ulStart    Start index.
     * @param [in]  cCount     Count (0 returns required size only).
     * @param [out] pulArray   Output RGB array (ignored when cCount == 0).
     * @return TRUE on success, FALSE on error.
     * @retval TRUE   Filled (or size returned via pulArray[0] when cCount==0).
     * @retval FALSE  No palette, NULL output with cCount>0, or invalid start.
     *
     * @par EDM2 Reference
     * GreQueryHWPaletteInfo
     * (https://www.edm2.com/index.php/GreQueryHWPaletteInfo):
     * "GreQueryHWPaletteInfo fills a buffer with the RGB value information from
     * the hardware palette."
     * "If cclr=0, only the size required for the buffer referenced by pclr is
     * returned."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in the graphics engine."
     */
    BOOL QueryHWInfo(ULONG ulStart, ULONG cCount, PULONG pulArray) const;

    /*!
     * @brief Update colors (remap visible area).
     * @param [in] flOptions  Options.
     * @return Count of entries remapped (0 if not realized).
     * @retval 0    No remap done.
     * @retval >0   Entries remapped.
     *
     * @par EDM2 Reference
     * GreUpdateColors
     * (https://www.edm2.com/index.php/GreUpdateColors):
     * "GreUpdateColors causes a pel-by-pel remapping of the Device Context's
     * visible area on the screen."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in Revision 2.0
     * (or later) of the graphics engine."
     *
     * @warning Pixel-level remapping of the visible area is not implemented;
     *          returns the entry count as an indication of work done.
     */
    ULONG UpdateColors(ULONG flOptions);

    /*!
     * @brief Query palette realization mapping.
     * @param [in]  ulStart   Start index.
     * @param [in]  cCount    Count.
     * @param [out] pulArray  Output realization flags.
     * @return TRUE on success, FALSE on error.
     * @retval TRUE   Mapping returned.
     * @retval FALSE  No palette, NULL output, or invalid start.
     *
     * @par EDM2 Reference
     * GreQueryPaletteRealization
     * (https://www.edm2.com/index.php/GreQueryPaletteRealization):
     * "GreQueryPaletteRealization returns the mapping from the logical palette
     * in the device context to the HW palette as an array of ULONGs."
     *
     * @par Simulation support
     * "This function is simulated by a handling routine in Revision 2.0
     * (or later) of the graphics engine."
     */
    BOOL QueryRealization(ULONG ulStart, ULONG cCount, PULONG pulArray) const;

    /*!
     * @brief Whether a palette has been created.
     * @return Flag.
     * @retval TRUE   Palette exists.
     * @retval FALSE  No palette.
     */
    BOOL IsCreated() const { return m_bCreated; }

    /*!
     * @brief Number of palette entries.
     * @return Count.
     * @retval >=0  Entry count.
     */
    ULONG EntryCount() const { return m_ulEntryCount; }

private:
    BOOL    m_bCreated;         /*!< Palette exists. */
    BOOL    m_bRealized;        /*!< Palette has been realized. */
    ULONG   m_ulFormat;         /*!< Entry format. */
    ULONG   m_ulFlags;          /*!< PC_* flags from Create. */
    RGB2   *m_pEntries;         /*!< Entry array (DosAllocMem). */
    ULONG   m_ulEntryCount;     /*!< Number of entries in use. */
    ULONG   m_ulAlloc;          /*!< Allocated capacity. */
};

#endif /* FS_PALETTE_HPP */
