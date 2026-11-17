#include "Fs_area.hpp"
#include "Fs_path.hpp"
#include <string.h>

/*!
 * @brief Construct an empty area state.
 * @return Nothing (constructor).
 * @retval (void)  No return value.
 */
Fs_Area::Fs_Area()
    : m_pPath(NULL), m_bOpen(FALSE), m_ulBeginFlags(0), m_lPrimType(0),
      m_ulDefsMask(0), m_ulAttrsMask(0), m_pAttrs(NULL), m_ulAttrsSize(0)
{
}

/*!
 * @brief Destroy and release attribute copy.
 * @return Nothing (destructor).
 * @retval (void)  No return value.
 */
Fs_Area::~Fs_Area() { Shutdown(); }

/*!
 * @brief Reset to initial state.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Area::Init()
{
    FreeAttrs();
    m_bOpen        = FALSE;
    m_ulBeginFlags = 0;
    m_lPrimType    = 0;
    m_ulDefsMask   = 0;
    m_ulAttrsMask  = 0;
}

/*!
 * @brief Alias for Init.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Area::Shutdown() { Init(); }

/*!
 * @brief Release the stored attribute copy.
 * @return Nothing.
 * @retval (void)  No return value.
 */
void Fs_Area::FreeAttrs()
{
    if (m_pAttrs) { DosFreeMem((PVOID)m_pAttrs); m_pAttrs = NULL; }
    m_ulAttrsSize = 0;
}

/*!
 * @brief Begin an area bracket.
 * @param [in] flOptions  BA_* flags.
 * @return Success indicator.
 * @retval TRUE   Area bracket opened.
 * @retval FALSE  Area bracket already open.
 *
 * @par EDM2 Reference
 * GreBeginArea — see header.
 */
BOOL Fs_Area::Begin(ULONG flOptions)
{
    if (m_bOpen) return FALSE;
    m_bOpen = TRUE;
    m_ulBeginFlags = flOptions;
    return TRUE;
}

/*!
 * @brief End an area bracket and fill it.
 * @param [in] flCancel  EA_DRAW or EA_CANCEL.
 * @return cHits code.
 * @retval GPI_OK    Filled, or cancelled/no-op.
 * @retval GPI_HITS  Filled with correlation hit.
 * @retval GPI_ERROR Error.
 *
 * @par EDM2 Reference
 * GreEndArea — see header.
 */
LONG Fs_Area::End(ULONG flCancel)
{
    if (!m_bOpen) {
        /* Per EDM2: when GreBeginArea was not called and EA_CANCEL is set,
         * GreEndArea is valid but has no effect. */
        return (flCancel & EA_CANCEL) ? GPI_OK : GPI_ERROR;
    }

    m_bOpen = FALSE;

    if (flCancel & EA_CANCEL) {
        FreeAttrs();
        return GPI_OK;
    }

    /* Translate BA_* flags to FPATH_* flags for the fill. */
    ULONG flPath = 0;
    if (m_ulBeginFlags & BA_WINDING) flPath |= FPATH_WINDING;
    if (m_ulBeginFlags & BA_EXCL)    flPath |= FPATH_EXCL;
    /* BA_ALTERNATE is the default (0). */
    /* BA_BOUNDARY / BA_NOBOUNDARY affect line drawing of the boundary,
     * which is handled by the driver's primitive calls inside the bracket.
     * They are not passed to FillPath. */

    /* Fill through the bound path.  If no path is bound, we cannot honour
     * the contract — return GPI_ERROR (not silent GPI_OK). */
    LONG rc = m_pPath ? m_pPath->FillPath(flPath) : GPI_ERROR;

    FreeAttrs();
    return rc;
}

/*!
 * @brief Save area attributes for later fill.
 * @param [in] lPrimType   PRIM_* bundle type.
 * @param [in] flDefsMask  Attributes to reset.
 * @param [in] flAttrsMask Attributes to modify.
 * @param [in] pAttrs      Pointer to bundle.
 * @return Success indicator.
 * @retval TRUE   Stored (or no-op when mask == 0).
 * @retval FALSE  Allocation failure or unknown primitive type.
 *
 * @par EDM2 Reference
 * GreAreaSetAttributes — see header.
 *
 * @warning The engine copies sizeof(bundle) bytes.  Per EDM2, the caller
 *          must provide a buffer large enough for the highest offset
 *          attribute referenced; for partial bundles the caller should
 *          pad the buffer to the full bundle size.
 */
BOOL Fs_Area::SetAttributes(LONG lPrimType, ULONG flDefsMask,
                            ULONG flAttrsMask, PVOID pAttrs)
{
    FreeAttrs();
    m_lPrimType   = lPrimType;
    m_ulDefsMask  = flDefsMask;
    m_ulAttrsMask = flAttrsMask;

    if (!pAttrs || flAttrsMask == 0) return TRUE;

    ULONG ulSize = 0;
    switch (lPrimType) {
    case PRIM_LINE:   ulSize = sizeof(LINEBUNDLE);   break;
    case PRIM_CHAR:   ulSize = sizeof(CHARBUNDLE);   break;
    case PRIM_MARKER: ulSize = sizeof(MARKERBUNDLE); break;
    case PRIM_AREA:   ulSize = sizeof(AREABUNDLE);   break;
    case PRIM_IMAGE:  ulSize = sizeof(IMAGEBUNDLE);  break;
    default:          return FALSE;
    }

    PVOID pv = NULL;
    if (DosAllocMem(&pv, ulSize, PAG_READ | PAG_WRITE | PAG_COMMIT) != 0)
        return FALSE;
    memcpy(pv, pAttrs, ulSize);
    m_pAttrs      = (BYTE *)pv;
    m_ulAttrsSize = ulSize;
    return TRUE;
}
