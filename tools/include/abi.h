/*!
 * @file abi.h
 *
 * @brief Writer for uni2h .abi files.
 *
 * Generates a sequence of blocks (module or library) in the .abi
 * syntax described in section 3 of the uni2h v2.0 specification.
 * Only writing is implemented; reading .abi files is a separate
 * task.
 *
 * References:
 *   - uni2h v2.0 specification, section 3 ("The .abi file").
 */

#ifndef ABI_H
#define ABI_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file abi.h
 *  @brief Writer for uni2h .abi files.
 *
 *  A document is an ordered list of blocks. Each block is either a
 *  module block or a library block and contains its own entry list.
 *  The caller opens a block with AbiBeginModule or AbiBeginLibrary,
 *  appends entries with AbiAddEntry, and closes the document with
 *  AbiClose. Opening a new block implicitly closes the previous
 *  one.
 */

/*! @brief Handle to an .abi document under construction. */
typedef HANDLE HABIDOC;

/*! @brief One entry to add to the current block. */
typedef struct _ABI_ENTRY {
    PCSZ  pszName;       /*!< Entry name. Not NULL, not empty. */
    PCSZ  pszInternal;   /*!< Internal name for an alias entry;
                          *   NULL for a plain entry. */
    ULONG ulOrdinal;     /*!< Ordinal. 0 means "not specified". */
    PCSZ  pszConvention; /*!< Convention string, or NULL for the
                          *   default "_System". Ignored in library
                          *   blocks. */
    BOOL  fVariable;     /*!< TRUE: emit as a variable. Ignored in
                          *   library blocks. */
} ABI_ENTRY, *PABI_ENTRY;

/*! @brief Create an empty document.
 *
 *  The document contains no blocks. Use AbiBeginLibrary or
 *  AbiBeginModule to add blocks.
 *
 *  @param[out] phDoc  Receives the handle. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p phDoc is NULL.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see AbiClose
 */
APIRET APIENTRY AbiCreateDoc(HABIDOC *phDoc);

/*! @brief Begin a new module block.
 *
 *  Closes the currently open block, if any, and opens a new module
 *  block with the given name. Subsequent AbiAddEntry calls append
 *  to this block. Module blocks emit the number and convention
 *  clauses of each entry.
 *
 *  @param[in] hDoc          Handle from AbiCreateDoc. Not
 *                           NULLHANDLE.
 *  @param[in] pszModuleName Module name. Not NULL, not empty.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad arguments.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see AbiBeginLibrary
 *  @see AbiAddEntry
 */
APIRET APIENTRY AbiBeginModule(HABIDOC hDoc, PCSZ pszModuleName);

/*! @brief Begin a new library block.
 *
 *  Same as AbiBeginModule, but emits a library block instead. A
 *  library block carries a flat list of entries and does not emit
 *  per-entry ordinals or conventions.
 *
 *  @param[in] hDoc           Handle from AbiCreateDoc. Not
 *                            NULLHANDLE.
 *  @param[in] pszLibraryName Library name. Not NULL, not empty.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad arguments.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see AbiBeginModule
 *  @see AbiAddEntry
 */
APIRET APIENTRY AbiBeginLibrary(HABIDOC hDoc, PCSZ pszLibraryName);

/*! @brief Append one entry to the currently open block.
 *
 *  The entry is appended in call order; the writer does not sort by
 *  ordinal. The caller is responsible for supplying entries in the
 *  order in which they should appear in the output.
 *
 *  @param[in] hDoc    Handle from AbiCreateDoc. Not NULLHANDLE.
 *  @param[in] pEntry  Entry to append. Not NULL; pEntry->pszName
 *                     must not be NULL or empty.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hDoc or @p pEntry is invalid,
 *                                   no block is open, or
 *                                   pEntry->pszName is NULL or
 *                                   empty.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY AbiAddEntry(HABIDOC hDoc, const ABI_ENTRY *pEntry);

/*! @brief Release a document.
 *
 *  Frees every block and every entry, and invalidates the handle.
 *  This function is idempotent: passing NULLHANDLE returns
 *  NO_ERROR.
 *
 *  @param[in] hDoc  Handle from AbiCreateDoc. NULLHANDLE is
 *                   accepted.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR               Success. Also for NULLHANDLE.
 *  @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 *
 *  @see AbiCreateDoc
 */
APIRET APIENTRY AbiClose(HABIDOC hDoc);

/*! @brief Serialize the document into a caller-supplied buffer.
 *
 *  Size-query convention:
 *    - pszBuf == NULL, ulSize == 0: only *pulUsed (size including
 *      NUL) is written, no buffer touched.
 *    - ulSize large enough: text copied and NUL-terminated; *pulUsed
 *      is the length without NUL.
 *    - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *      required size including NUL.
 *
 *  @param[in]  hDoc     Handle. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hDoc is NULLHANDLE, or
 *                                   @p pszBuf is NULL without
 *                                   size-query.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY AbiQueryText(HABIDOC hDoc, PSZ pszBuf, ULONG ulSize,
                             PULONG pulUsed);

/*! @brief Write the document to a file.
 *
 *  Serializes the document and writes it to @p pszPath. An existing
 *  file is overwritten. The output is NUL-terminated and uses the
 *  conventional line endings of the host platform.
 *
 *  @param[in] hDoc     Handle. Not NULLHANDLE.
 *  @param[in] pszPath  Output file path. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hDoc or @p pszPath is invalid.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_OPEN_FAILED        Cannot open output file.
 *  @retval ERROR_WRITE_FAULT        Write error.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY AbiWriteFile(HABIDOC hDoc, PCSZ pszPath);

#ifdef __cplusplus
}
#endif

#endif /* ABI_H */
