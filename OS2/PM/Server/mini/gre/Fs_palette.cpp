#include "Fs_palette.hpp"
#include <string.h>

/*!
 * @brief Construct an uncreated palette.
 * @return Nothing (constructor).
 * @retval (void)  No return value.
 */
Fs_Palette::Fs_Palette()
    : m_bCreated(FALSE), m_bRealized(FALSE), m_ulFormat(0), m_ulFlags(0),
      m_pEntries(NULL), m_ulEntryCount(0), m_ulAlloc(0)
{
}

/*!
 * @brief Destroy and release entry storage.
 * @return Nothing (destructor).
 * @retval (void)  No return value.
 */
Fs_Palette::~Fs_Palette() { Shutdown(); }

/*!
 * @brief Reset to initial state.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Palette::Init()
{
    if (m_pEntries) { DosFreeMem((PVOID)m_pEntries); m_pEntries = NULL; }
    m_bCreated    = FALSE;
    m_bRealized   = FALSE;
    m_ulFormat    = 0;
    m_ulFlags     = 0;
    m_ulEntryCount = 0;
    m_ulAlloc     = 0;
}

/*!
 * @brief Alias for Init.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Palette::Shutdown() { Init(); }

/*!
 * @brief Create a palette object for the DC.
 * @param [in] flOptions  PC_* flags.
 * @param [in] ulFormat   Entry format.
 * @param [in] cEntries   Entry count (0 defaults to 256).
 * @param [in] pEntries   Optional initial entries.
 * @return Success indicator.
 * @retval TRUE   Created.
 * @retval FALSE  Allocation failure.
 *
 * @par EDM2 Reference
 * GreDeviceCreatePalette — see header.
 */
BOOL Fs_Palette::Create(ULONG flOptions, ULONG ulFormat,
                        ULONG cEntries, const ULONG *pEntries)
{
    Init();

    if (cEntries == 0) cEntries = 256;

    PVOID pv = NULL;
    if (DosAllocMem(&pv, cEntries * sizeof(RGB2),
                    PAG_READ | PAG_WRITE | PAG_COMMIT) != 0) return FALSE;

    m_pEntries     = (RGB2 *)pv;
    m_ulAlloc      = cEntries;
    m_ulEntryCount = cEntries;
    m_ulFormat     = ulFormat;
    m_ulFlags      = flOptions;
    m_bCreated     = TRUE;

    if (pEntries) {
        for (ULONG i = 0; i < cEntries; i++) {
            m_pEntries[i].bBlue     = (BYTE)(pEntries[i] & 0xFF);
            m_pEntries[i].bGreen    = (BYTE)((pEntries[i] >> 8) & 0xFF);
            m_pEntries[i].bRed      = (BYTE)((pEntries[i] >> 16) & 0xFF);
            m_pEntries[i].fcOptions = 0;
        }
    } else {
        memset(m_pEntries, 0, cEntries * sizeof(RGB2));
    }
    return TRUE;
}

/*!
 * @brief Delete the palette object.
 * @return Success indicator.
 * @retval TRUE   Deleted.
 * @retval FALSE  No palette.
 *
 * @par EDM2 Reference
 * GreDeviceDeletePalette — see header.
 */
BOOL Fs_Palette::Delete()
{
    if (!m_bCreated) return FALSE;
    Init();
    return TRUE;
}

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
 * GreDeviceSetPaletteEntries — see header.
 */
BOOL Fs_Palette::SetEntries(ULONG ulFormat, ULONG ulStart,
                            ULONG cCount, const ULONG *pEntries)
{
    (void)ulFormat;
    if (!m_bCreated || !pEntries) return FALSE;
    if (ulStart >= m_ulEntryCount) return FALSE;
    if (ulStart + cCount > m_ulEntryCount)
        cCount = m_ulEntryCount - ulStart;

    for (ULONG i = 0; i < cCount; i++) {
        m_pEntries[ulStart + i].bBlue  = (BYTE)(pEntries[i] & 0xFF);
        m_pEntries[ulStart + i].bGreen = (BYTE)((pEntries[i] >> 8) & 0xFF);
        m_pEntries[ulStart + i].bRed   = (BYTE)((pEntries[i] >> 16) & 0xFF);
    }
    return TRUE;
}

/*!
 * @brief Animate the palette.
 * @param [in] ulFormat  Entry format.
 * @param [in] ulStart   Start index.
 * @param [in] cCount    Count.
 * @param [in] pEntries  New RGB entries.
 * @return Success indicator.
 * @retval TRUE   Animated.
 * @retval FALSE  Invalid arguments.
 *
 * @par EDM2 Reference
 * GreDeviceAnimatePalette — see header.
 *
 * @warning Immediate reflection on hardware slots is not implemented.
 */
BOOL Fs_Palette::Animate(ULONG ulFormat, ULONG ulStart,
                         ULONG cCount, const ULONG *pEntries)
{
    /* Per EDM2 the effect is immediate for entries with hardware slots.
     * We simply update stored entries; hardware reflection is a gap. */
    return SetEntries(ulFormat, ulStart, cCount, pEntries);
}

/*!
 * @brief Resize the palette.
 * @param [in] cNewEntries  New entry count.
 * @return Success indicator.
 * @retval TRUE   Resized.
 * @retval FALSE  No palette or zero count.
 *
 * @par EDM2 Reference
 * GreDeviceResizePalette — see header.
 */
BOOL Fs_Palette::Resize(ULONG cNewEntries)
{
    if (!m_bCreated || cNewEntries == 0) return FALSE;

    if (cNewEntries == m_ulAlloc) {
        m_ulEntryCount = cNewEntries;
        return TRUE;
    }

    PVOID pv = NULL;
    if (DosAllocMem(&pv, cNewEntries * sizeof(RGB2),
                    PAG_READ | PAG_WRITE | PAG_COMMIT) != 0) return FALSE;

    ULONG cCopy = (cNewEntries < m_ulEntryCount) ? cNewEntries : m_ulEntryCount;
    memcpy(pv, m_pEntries, cCopy * sizeof(RGB2));
    if (cNewEntries > m_ulEntryCount) {
        memset((BYTE *)pv + cCopy * sizeof(RGB2), 0,
               (cNewEntries - cCopy) * sizeof(RGB2));
    }

    DosFreeMem((PVOID)m_pEntries);
    m_pEntries     = (RGB2 *)pv;
    m_ulAlloc      = cNewEntries;
    m_ulEntryCount = cNewEntries;
    return TRUE;
}

/*!
 * @brief Realize the palette.
 * @param [in]  flType    RP_* type.
 * @param [out] pulCount  Optional output count.
 * @return Success indicator.
 * @retval TRUE   Realized.
 * @retval FALSE  No palette.
 *
 * @par EDM2 Reference
 * GreRealizePalette — see header.
 */
BOOL Fs_Palette::Realize(ULONG flType, PULONG pulCount)
{
    (void)flType;
    if (!m_bCreated) return FALSE;
    m_bRealized = TRUE;
    if (pulCount) *pulCount = m_ulEntryCount;
    return TRUE;
}

/*!
 * @brief Query hardware palette info.
 * @param [in]  ulStart    Start index.
 * @param [in]  cCount     Count (0 returns required size only).
 * @param [out] pulArray   Output RGB array.
 * @return TRUE on success, FALSE on error.
 * @retval TRUE   Filled (or size returned when cCount==0).
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreQueryHWPaletteInfo — see header.
 */
BOOL Fs_Palette::QueryHWInfo(ULONG ulStart, ULONG cCount, PULONG pulArray) const
{
    if (!m_bCreated) return FALSE;

    /* Per EDM2: if cCount == 0, only the required buffer size is returned. */
    if (cCount == 0) {
        if (!pulArray) return FALSE;
        pulArray[0] = m_ulEntryCount;
        return TRUE;
    }

    if (!pulArray || ulStart >= m_ulEntryCount) return FALSE;
    if (ulStart + cCount > m_ulEntryCount)
        cCount = m_ulEntryCount - ulStart;

    for (ULONG i = 0; i < cCount; i++) {
        const RGB2 &e = m_pEntries[ulStart + i];
        pulArray[i] = ((ULONG)e.bRed << 16) | ((ULONG)e.bGreen << 8) | e.bBlue;
    }
    return TRUE;
}

/*!
 * @brief Update colors (remap visible area).
 * @param [in] flOptions  Options.
 * @return Count of entries remapped.
 * @retval 0    No remap done.
 * @retval >0   Entries remapped.
 *
 * @par EDM2 Reference
 * GreUpdateColors — see header.
 *
 * @warning Pixel-level remapping not implemented.
 */
ULONG Fs_Palette::UpdateColors(ULONG flOptions)
{
    (void)flOptions;
    if (!m_bCreated || !m_bRealized) return 0;
    /* Per EDM2 the engine performs a pel-by-pel remap of the visible area.
     * This is a gap; return entry count as an indication of work done. */
    return m_ulEntryCount;
}

/*!
 * @brief Query palette realization mapping.
 * @param [in]  ulStart   Start index.
 * @param [in]  cCount    Count.
 * @param [out] pulArray  Output realization flags.
 * @return TRUE on success, FALSE on error.
 * @retval TRUE   Mapping returned.
 * @retval FALSE  Error.
 *
 * @par EDM2 Reference
 * GreQueryPaletteRealization — see header.
 */
BOOL Fs_Palette::QueryRealization(ULONG ulStart, ULONG cCount,
                                   PULONG pulArray) const
{
    if (!m_bCreated || !pulArray || ulStart >= m_ulEntryCount) return FALSE;
    if (ulStart + cCount > m_ulEntryCount)
        cCount = m_ulEntryCount - ulStart;

    for (ULONG i = 0; i < cCount; i++)
        pulArray[i] = m_bRealized ? 1 : 0;
    return TRUE;
}
