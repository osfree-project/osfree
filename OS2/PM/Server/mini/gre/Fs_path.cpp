#include "Fs_path.hpp"
#include "Fs_region.hpp"
#include <string.h>

/*!
 * @brief Construct an empty path.
 * @return Nothing (constructor).
 * @retval (void)  No return value.
 */
Fs_Path::Fs_Path()
    : m_hdc(0), m_bOpen(FALSE), m_pLines(NULL), m_ulLineCount(0), m_ulAlloc(0),
      m_pSaveStack(NULL), m_ulSaveCount(0), m_ulSaveAlloc(0)
{
    m_ptlFigureStart.x = m_ptlFigureStart.y = 0;
    m_ptlCurrent.x     = m_ptlCurrent.y     = 0;
}

/*!
 * @brief Destroy the path and release storage.
 * @return Nothing (destructor).
 * @retval (void)  No return value.
 */
Fs_Path::~Fs_Path() { Shutdown(); }

/*!
 * @brief Reset to initial state.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Path::Init()
{
    FreeLines();
    for (ULONG i = 0; i < m_ulSaveCount; i++)
        if (m_pSaveStack[i].pLines) DosFreeMem((PVOID)m_pSaveStack[i].pLines);
    if (m_pSaveStack) { DosFreeMem((PVOID)m_pSaveStack); m_pSaveStack = NULL; }
    m_ulSaveCount = 0;
    m_ulSaveAlloc = 0;
    m_bOpen = FALSE;
    m_ptlFigureStart.x = m_ptlFigureStart.y = 0;
    m_ptlCurrent.x     = m_ptlCurrent.y     = 0;
}

/*!
 * @brief Alias for Init.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Path::Shutdown() { Init(); }

/*!
 * @brief Release the LINE array and reset counters.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Path::FreeLines()
{
    if (m_pLines) { DosFreeMem((PVOID)m_pLines); m_pLines = NULL; }
    m_ulLineCount = 0;
    m_ulAlloc = 0;
}

/*!
 * @brief Ensure capacity for cNeeded LINE entries.
 * @param [in] cNeeded  Minimum required capacity.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Path::EnsureCapacity(ULONG cNeeded)
{
    if (cNeeded <= m_ulAlloc) return;
    ULONG cNew = m_ulAlloc ? m_ulAlloc : 16;
    while (cNew < cNeeded) cNew *= 2;
    PVOID pv = NULL;
    if (DosAllocMem(&pv, cNew * sizeof(LINE),
                    PAG_READ | PAG_WRITE | PAG_COMMIT) != 0) return;
    if (m_pLines && m_ulLineCount)
        memcpy(pv, m_pLines, m_ulLineCount * sizeof(LINE));
    if (m_pLines) DosFreeMem((PVOID)m_pLines);
    m_pLines = (LINE *)pv;
    m_ulAlloc = cNew;
}

/*!
 * @brief Begin a path definition.
 * @param [in] idPath  Path identifier; must be 1.
 * @return Success indicator.
 * @retval TRUE   Path started.
 * @retval FALSE  idPath != 1, or already open.
 *
 * @par EDM2 Reference
 * GreBeginPath — see header.
 */
BOOL Fs_Path::BeginPath(LONG idPath)
{
    if (idPath != 1) return FALSE;
    if (m_bOpen) return FALSE;
    FreeLines();
    m_bOpen = TRUE;
    m_ptlFigureStart = m_ptlCurrent;
    return TRUE;
}

/*!
 * @brief End a path definition.
 * @param [in] flOptions  Reserved.
 * @return TRUE.
 * @retval TRUE  Always.
 *
 * @par EDM2 Reference
 * GreEndPath — see header.
 */
BOOL Fs_Path::EndPath(ULONG flOptions)
{
    (void)flOptions;
    m_bOpen = FALSE;
    return TRUE;
}

/*!
 * @brief Close the current figure.
 * @return Success indicator.
 * @retval TRUE   Figure closed.
 * @retval FALSE  No path open.
 *
 * @par EDM2 Reference
 * GreCloseFigure — see header.
 */
BOOL Fs_Path::CloseFigure()
{
    if (!m_bOpen) return TRUE;
    if (m_ptlCurrent.x != m_ptlFigureStart.x ||
        m_ptlCurrent.y != m_ptlFigureStart.y)
    {
        AddLine(m_ptlCurrent, m_ptlFigureStart);
    }
    m_ptlCurrent = m_ptlFigureStart;
    return TRUE;
}

/*!
 * @brief Add a line segment to the path.
 * @param [in] ptlStart  Start point.
 * @param [in] ptlEnd    End point.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Path::AddLine(const POINTL &ptlStart, const POINTL &ptlEnd)
{
    if (!m_bOpen) return;
    if (m_ulLineCount == 0) m_ptlFigureStart = ptlStart;
    EnsureCapacity(m_ulLineCount + 1);
    if (m_ulLineCount >= m_ulAlloc) return;
    LINE *pLine = &m_pLines[m_ulLineCount];
    memset(pLine, 0, sizeof(LINE));
    pLine->bIdent = LINE_IDENTIFIER;
    pLine->bType  = LINE_IDENTIFIER;
    pLine->ptlA   = ptlStart;
    pLine->ptlC   = ptlEnd;
    pLine->ptfxA.x = MAKEFIXED(ptlStart.x, 0);
    pLine->ptfxA.y = MAKEFIXED(ptlStart.y, 0);
    pLine->ptfxC.x = MAKEFIXED(ptlEnd.x, 0);
    pLine->ptfxC.y = MAKEFIXED(ptlEnd.y, 0);
    pLine->pcvNext = NULL;
    pLine->pcvPrev = (m_ulLineCount > 0)
                        ? (PCURVE)&m_pLines[m_ulLineCount - 1]
                        : NULL;
    if (m_ulLineCount > 0)
        m_pLines[m_ulLineCount - 1].pcvNext = (PCURVE)pLine;
    m_ulLineCount++;
    m_ptlCurrent = ptlEnd;
}

/*!
 * @brief Fill the path interior.
 * @param [in] flOptions  FPATH_* flags.
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 *
 * @par EDM2 Reference
 * GreFillPath — see header.
 */
LONG Fs_Path::FillPath(ULONG flOptions)
{
    (void)flOptions;
    return Flush(0, TRUE);
}

/*!
 * @brief Outline the path.
 * @param [in] flOptions  OPTH_* flags.
 * @return cHits code or point count.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 * @retval >=0  Point count (OPTH_QRY_PATH_POINTS).
 *
 * @par EDM2 Reference
 * GreOutlinePath — see header.
 */
LONG Fs_Path::OutlinePath(ULONG flOptions)
{
    if (flOptions & OPTH_QRY_PATH_POINTS) return (LONG)m_ulLineCount;
    return Flush(flOptions, FALSE);
}

/*!
 * @brief Modify a path.
 * @param [in] cmdMode  MPATH_STROKE.
 * @return Success indicator.
 * @retval TRUE   Accepted.
 * @retval FALSE  Invalid cmdMode or no path.
 *
 * @par EDM2 Reference
 * GreModifyPath — see header.
 * @warning Envelope construction not implemented.
 */
BOOL Fs_Path::ModifyPath(LONG cmdMode)
{
    if (cmdMode != MPATH_STROKE) return FALSE;
    if (!m_bOpen) return FALSE;
    return TRUE;
}

/*!
 * @brief Stroke a path.
 * @param [in] flOptions  Reserved.
 * @return cHits code.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 *
 * @par EDM2 Reference
 * GreStrokePath — see header.
 */
LONG Fs_Path::StrokePath(ULONG flOptions)
{
    (void)flOptions;
    if (!ModifyPath(MPATH_STROKE)) return GPI_ERROR;
    return Flush(0, TRUE);
}

/*!
 * @brief Save path state.
 * @param [in] cSave  Save level (1 = init, N>=2 = push).
 * @return Success indicator.
 * @retval TRUE   Saved.
 * @retval FALSE  Negative level or allocation failure.
 *
 * @par EDM2 Reference
 * GreSavePath — see header.
 */
BOOL Fs_Path::SavePath(LONG cSave)
{
    if (cSave < 0) return FALSE;
    if (cSave == 1) { Init(); return TRUE; }

    if (m_ulSaveCount >= m_ulSaveAlloc) {
        ULONG cNew = m_ulSaveAlloc ? m_ulSaveAlloc * 2 : 4;
        PVOID pv = NULL;
        if (DosAllocMem(&pv, cNew * sizeof(SaveState),
                        PAG_READ | PAG_WRITE | PAG_COMMIT) != 0) return FALSE;
        if (m_pSaveStack)
            memcpy(pv, m_pSaveStack, m_ulSaveCount * sizeof(SaveState));
        if (m_pSaveStack) DosFreeMem((PVOID)m_pSaveStack);
        m_pSaveStack  = (SaveState *)pv;
        m_ulSaveAlloc = cNew;
    }

    SaveState &s = m_pSaveStack[m_ulSaveCount];
    s.bOpen          = m_bOpen;
    s.ulLineCount    = m_ulLineCount;
    s.ptlFigureStart = m_ptlFigureStart;
    s.ptlCurrent     = m_ptlCurrent;
    s.pLines = NULL;
    if (m_ulLineCount > 0) {
        PVOID pv = NULL;
        if (DosAllocMem(&pv, m_ulLineCount * sizeof(LINE),
                        PAG_READ | PAG_WRITE | PAG_COMMIT) != 0) return FALSE;
        memcpy(pv, m_pLines, m_ulLineCount * sizeof(LINE));
        s.pLines  = (PLINE)pv;
        s.ulAlloc = m_ulLineCount;
    } else {
        s.ulAlloc = 0;
    }
    m_ulSaveCount++;
    return TRUE;
}

/*!
 * @brief Restore path state.
 * @param [in] cSave  -1/0/1 = reset; N>=2 = restore level N.
 * @return Success indicator.
 * @retval TRUE   Restored.
 * @retval FALSE  Level out of range.
 *
 * @par EDM2 Reference
 * GreRestorePath — see header.
 */
BOOL Fs_Path::RestorePath(LONG cSave)
{
    if (cSave <= 0 || cSave == 1) { Init(); return TRUE; }
    ULONG targetIdx = (ULONG)cSave - 2;
    if (targetIdx >= m_ulSaveCount) return FALSE;

    SaveState &s = m_pSaveStack[targetIdx];
    FreeLines();
    m_bOpen          = s.bOpen;
    m_ulLineCount    = s.ulLineCount;
    m_ptlFigureStart = s.ptlFigureStart;
    m_ptlCurrent     = s.ptlCurrent;
    m_pLines         = s.pLines;
    m_ulAlloc        = s.ulAlloc;
    s.pLines = NULL;

    for (ULONG i = targetIdx + 1; i < m_ulSaveCount; i++)
        if (m_pSaveStack[i].pLines) DosFreeMem((PVOID)m_pSaveStack[i].pLines);
    m_ulSaveCount = targetIdx + 1;
    return TRUE;
}

/*!
 * @brief Emit lines via GreDrawLinesInPath with real HDC, no error masking.
 * @param [in] flOptions  Flags passed to GreDrawLinesInPath.
 * @param [in] bFill      TRUE for fill, FALSE for outline.
 * @return Result of GreDrawLinesInPath.
 * @retval GPI_OK / GPI_HITS / GPI_ERROR.
 *
 * @par EDM2 Reference
 * GreOutlinePath — see header.
 *
 * @warning Returns whatever the mandatory GreDrawLinesInPath returns.
 *          A HDC of 0 would crash; therefore Flush requires SetHdc() to
 *          have been called.  Fs_Dc::Init() calls SetHdc(iHdc).
 */
LONG Fs_Path::Flush(ULONG flOptions, BOOL bFill)
{
    (void)bFill;
    if (m_ulLineCount == 0) { FreeLines(); m_bOpen = FALSE; return GPI_OK; }

    RECTL rclBound;
    rclBound.xLeft   =  0x7FFFFFFF;
    rclBound.yBottom =  0x7FFFFFFF;
    rclBound.xRight  = -0x7FFFFFFF;
    rclBound.yTop    = -0x7FFFFFFF;
    for (ULONG i = 0; i < m_ulLineCount; i++) {
        POINTL a = m_pLines[i].ptlA, b = m_pLines[i].ptlC;
        if (a.x < rclBound.xLeft)   rclBound.xLeft   = a.x;
        if (a.y < rclBound.yBottom) rclBound.yBottom = a.y;
        if (a.x > rclBound.xRight)  rclBound.xRight  = a.x;
        if (a.y > rclBound.yTop)    rclBound.yTop    = a.y;
        if (b.x < rclBound.xLeft)   rclBound.xLeft   = b.x;
        if (b.y < rclBound.yBottom) rclBound.yBottom = b.y;
        if (b.x > rclBound.xRight)  rclBound.xRight  = b.x;
        if (b.y > rclBound.yTop)    rclBound.yTop    = b.y;
    }

    LONG rc = (LONG)GreDrawLinesInPath((HDC)m_hdc, &rclBound,
                                       (PCURVE)m_pLines, m_ulLineCount,
                                       flOptions, NGreDrawLinesInPath);
    FreeLines();
    m_bOpen = FALSE;
    return rc;
}

/*!
 * @brief Convert current path to a region (rectangular approximation).
 * @param [out] out  Receives the resulting region.
 * @return Success indicator.
 * @retval TRUE   Converted.
 * @retval FALSE  Allocation failure (via AddRect).
 */
BOOL Fs_Path::ToRegion(Fs_Region &out) const
{
    out.Clear();
    for (ULONG i = 0; i < m_ulLineCount; i++) {
        POINTL a = m_pLines[i].ptlA, b = m_pLines[i].ptlC;
        RECTL r;
        if (a.y == b.y) {
            r.xLeft = (a.x < b.x) ? a.x : b.x;
            r.xRight = (a.x > b.x) ? a.x : b.x;
            r.yBottom = a.y; r.yTop = a.y + 1;
        } else if (a.x == b.x) {
            r.xLeft = a.x; r.xRight = a.x + 1;
            r.yBottom = (a.y < b.y) ? a.y : b.y;
            r.yTop = (a.y > b.y) ? a.y : b.y;
        } else {
            r.xLeft = (a.x < b.x) ? a.x : b.x;
            r.xRight = (a.x > b.x) ? a.x : b.x;
            r.yBottom = (a.y < b.y) ? a.y : b.y;
            r.yTop = (a.y > b.y) ? a.y : b.y;
        }
        if (r.xLeft < r.xRight && r.yBottom < r.yTop) out.AddRect(r);
    }
    return TRUE;
}
