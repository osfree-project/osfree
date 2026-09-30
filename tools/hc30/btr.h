/*!
 * @file btr.h
 * @brief B+ tree for HC30 directory and index files.
 *
 * @par Function naming
 * Function names follow OS/2 Control Program API conventions as
 * defined in OS/2 Version 2.0 - Volume 4: Writing Applications,
 * section 12.2 "Function Name Changes":
 *   - Function class prefix, then verb, then noun.
 *   - Initial characters of class, verb, noun are capitalized.
 *   - Verbs are placed before nouns.
 *   - Create / Get / Set / Query semantics per SAA.
 *
 * @par References
 *  - [1] SAA Common Programming Interface, C Reference - Level 2,
 *        SC09-1308-02 (Sep 1991).
 *  - [2] OS/2 Version 2.0 - Volume 4: Writing Applications,
 *        IBM, 1993, section 12.2.
 *  - [3] Winterhoff, M., helpdeco (1997), BTREEHEADER,
 *        BTREENODEHEADER, BTREEINDEXHEADER.
 *  - [4] ccl container library: vector.h.
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * B-tree handle is shared between threads.
 *
 * @par Ownership
 * HBTR is caller-owned and released with BtrDestroy. Keys are copied
 * into the B-tree; the caller's strings are not referenced after
 * BtrAddEntry returns.
 */
#ifndef BTR_H
#define BTR_H

#include "os2types.h"
#include "os2err.h"
#include <stdio.h>

/*!
 * @typedef HBTR
 * @brief Handle to a B-tree. Caller-owned.
 */
typedef HANDLE HBTR;

/*!
 * @typedef PHBTR
 * @brief Pointer to a B-tree handle.
 */
typedef HBTR  *PHBTR;

/*!
 * @def BTR_FLAG_DIRECTORY
 * @brief Marks the tree as the HLP directory index.
 *
 * Used in the BTREEHEADER flags field together with BTR_FLAG_ALWAYS.
 * See helpdeco: directory B-trees set 0x0400 in addition to 0x0002.
 */
#define BTR_FLAG_DIRECTORY 0x0400

/*!
 * @def BTR_FLAG_ALWAYS
 * @brief Always-set flag per HC30 documentation.
 *
 * Every HC30 B-tree has this bit set in its BTREEHEADER.
 */
#define BTR_FLAG_ALWAYS    0x0002

/*!
 * @brief Create a B-tree.
 *
 * @param[out] phBtr        Receiver. Not NULL. Set to NULLHANDLE on error.
 * @param[in]  usPageSize   Page size in bytes. Pass 0 to select 2048.
 * @param[in]  usFlags      BTR_FLAG_DIRECTORY | BTR_FLAG_ALWAYS for the
 *                          directory index.
 * @param[in]  pszStructure Structure tag up to 15 characters written to
 *                          the BTREEHEADER. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phBtr is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 *
 * @sa BtrDestroy
 */
APIRET APIENTRY BtrCreate(PHBTR phBtr, USHORT usPageSize, USHORT usFlags,
                          PCSZ pszStructure);

/*!
 * @brief Destroy a B-tree and release all its keys.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hBtr B-tree handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR              Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE  Handle is not recognized.
 */
APIRET APIENTRY BtrDestroy(HBTR hBtr);

/*!
 * @brief Append a key/value pair.
 *
 * Keys are stored in insertion order. Duplicate keys are rejected
 * with ERROR_INVALID_PARAMETER. The key string is copied into the
 * B-tree; the caller's buffer is not referenced after return.
 *
 * @param[in] hBtr    B-tree handle. Not NULLHANDLE.
 * @param[in] pszKey  NUL-terminated key. Not NULL.
 * @param[in] ulValue Value associated with the key.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hBtr or @a pszKey is NULL, or
 *                                  the key is already present.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY BtrAddEntry(HBTR hBtr, PCSZ pszKey, ULONG ulValue);

/*!
 * @brief Query the serialized size of the B-tree image.
 *
 * Returns an upper bound on the serialized image: the estimated page
 * count multiplied by the page size. The exact size is available
 * after BtrWrite.
 *
 * @param[in]  hBtr    B-tree handle. Not NULLHANDLE.
 * @param[out] pulSize Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hBtr or @a pulSize is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY BtrQuerySize(HBTR hBtr, PULONG pulSize);

/*!
 * @brief Serialize the B-tree to a stream.
 *
 * Writes BTREEHEADER, then leaf pages, then index pages up to the root
 * page. Pages are padded to the configured page size. The stream
 * position is left just after the last page.
 *
 * @param[in] hBtr B-tree handle. Not NULLHANDLE.
 * @param[in] f    Output stream. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hBtr or @a f is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY BtrWrite(HBTR hBtr, FILE* f);

#endif /* BTR_H */
