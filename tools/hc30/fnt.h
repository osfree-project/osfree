/*!
 * @file fnt.h
 * @brief |FONT internal file generator for WinHelp 3.0.
 *
 * @par Function naming
 * Function names follow OS/2 Control Program API conventions as
 * defined in OS/2 Version 2.0 - Volume 4: Writing Applications,
 * section 12.2 "Function Name Changes".
 *
 * @par References
 *  - [1] SAA Common Programming Interface, C Reference - Level 2,
 *        SC09-1308-02 (Sep 1991).
 *  - [2] OS/2 Version 2.0 - Volume 4: Writing Applications,
 *        IBM, 1993, section 12.2.
 *  - [3] Winterhoff, M., helpdeco (1997), FONTHEADER, OLDFONT.
 *  - [4] ccl container library: vector.h.
 *
 * @par Thread safety
 * The module is single threaded.
 *
 * @par Ownership
 * HFNT is caller-owned and released with FntDestroy.
 */
#ifndef FNT_H
#define FNT_H

#include "os2types.h"
#include "os2err.h"
#include <stdio.h>

/*!
 * @typedef HFNT
 * @brief Handle to a |FONT generator. Caller-owned.
 */
typedef HANDLE HFNT;

/*!
 * @typedef PHFNT
 * @brief Pointer to a |FONT generator handle.
 */
typedef HFNT  *PHFNT;

/*!
 * @def FNT_ATTR_BOLD
 * @brief Font attribute: bold.
 */
#define FNT_ATTR_BOLD 0x01

/*!
 * @def FNT_ATTR_ITAL
 * @brief Font attribute: italic.
 */
#define FNT_ATTR_ITAL 0x02

/*!
 * @def FNT_ATTR_UNDR
 * @brief Font attribute: underline.
 */
#define FNT_ATTR_UNDR 0x04

/*!
 * @def FNT_ATTR_STRK
 * @brief Font attribute: strike-through.
 */
#define FNT_ATTR_STRK 0x08

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create an empty |FONT generator.
 *
 * @param[out] phFnt Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phFnt is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY FntCreate(PHFNT phFnt);

/*!
 * @brief Destroy a |FONT generator and release all its data.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hFnt Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR              Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE  Handle is not recognized.
 */
APIRET APIENTRY FntDestroy(HFNT hFnt);

/* ==================================================================
 * Adding
 * ================================================================== */

/*!
 * @brief Add a face name and return its index.
 *
 * Face names are stored in insertion order. Duplicates are not
 * rejected; the caller may use FntGetOrCreateDescriptor to avoid
 * duplicating descriptors that share a face.
 *
 * @param[in]  hFnt     Handle. Not NULLHANDLE.
 * @param[in]  pszName  Face name. Not NULL.
 * @param[out] pulIndex Optional. Receives the index of the new face.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hFnt or @a pszName is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY FntAddFaceName(HFNT hFnt, PCSZ pszName, PULONG pulIndex);

/*!
 * @brief Query or create a font descriptor.
 *
 * If a descriptor with the given face, size, attributes, family, and
 * colors already exists, its index is returned. Otherwise a new
 * descriptor is added and its index is returned.
 *
 * @param[in]  hFnt         Handle. Not NULLHANDLE.
 * @param[in]  ulFaceIndex  Face index returned by FntAddFaceName.
 * @param[in]  ulHalfPoints Size in half-points.
 * @param[in]  ulAttributes Bit mask; see FNT_ATTR_*.
 * @param[in]  pbFGRGB      Optional. 3 bytes foreground RGB. NULL
 *                          selects black.
 * @param[in]  pbBGRGB      Optional. 3 bytes background RGB. NULL
 *                          selects white.
 * @param[in]  ulFamily     Font family code.
 * @param[out] pulIndex     Optional. Receives the descriptor index.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hFnt is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY FntGetOrCreateDescriptor(HFNT hFnt, ULONG ulFaceIndex,
                                         ULONG ulHalfPoints, ULONG ulAttributes,
                                         PBYTE pbFGRGB, PBYTE pbBGRGB,
                                         ULONG ulFamily, PULONG pulIndex);

/* ==================================================================
 * Query
 * ================================================================== */

/*!
 * @brief Query the number of face names.
 *
 * @param[in]  hFnt     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY FntQueryFaceCount(HFNT hFnt, PULONG pulCount);

/*!
 * @brief Query a face name by index.
 *
 * Uses the size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: value copied, NUL-terminated.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW, *pulUsed is required size.
 *
 * @param[in]  hFnt    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hFnt is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY FntQueryFaceName(HFNT hFnt, ULONG ulIndex,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Query the number of descriptors.
 *
 * @param[in]  hFnt     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY FntQueryDescriptorCount(HFNT hFnt, PULONG pulCount);

/* ==================================================================
 * Writing
 * ================================================================== */

/*!
 * @brief Serialize the |FONT file to a stream.
 *
 * If no face names or descriptors were added, a default set is
 * inserted: face "Helv", one descriptor, 10pt, family 2, black on
 * white.
 *
 * @param[in] hFnt Handle. Not NULLHANDLE.
 * @param[in] f    Output stream. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hFnt or @a f is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed for defaults.
 */
APIRET APIENTRY FntWrite(HFNT hFnt, FILE* f);

#endif /* FNT_H */
