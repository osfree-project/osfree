/* dep5.h - Debian Copyright Format 1.0 (DEP5) parser
 * (C89 + Watcom extensions) */
#ifndef DEP5_H
#define DEP5_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file dep5.h
 * @brief Public interface of the DEP5 parser.
 *
 * Conforms to the machine-readable debian/copyright format,
 * version 1.0:
 *   - https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
 *
 * Three stanza kinds are recognized:
 *   - header stanza (exactly one, first in file);
 *   - Files stanzas (one or more);
 *   - stand-alone License stanzas (zero or more).
 *
 * The parser owns its internal buffers, allocates them in Dep5Open and
 * releases them in Dep5Close. The consumer supplies buffers only for
 * the data being returned.
 *
 * Written for Open Watcom 1.9 in C89 style. No -za99 mode is required.
 */

/* ==================================================================
 * Return codes
 * ================================================================== */

/** @def DEP5_ERROR_INVALID_SYNTAX @brief DEP5 syntax error.
 *  User range 0xFF01. */
#define DEP5_ERROR_INVALID_SYNTAX    0xFF01

/* ==================================================================
 * Handles
 * ================================================================== */

/**
 * @typedef HDEP5DOC
 * @brief DEP5 document handle.
 */
typedef HANDLE HDEP5DOC;

/**
 * @typedef HDEP5FIND
 * @brief DEP5 enumeration cursor handle.
 *
 * Used both for Files stanzas (Dep5FilesFindFirst) and stand-alone
 * License stanzas (Dep5LicenseFindFirst). A cursor obtained from one
 * family of functions must be released by the corresponding FindClose
 * and must not be passed to functions of the other family.
 */
typedef HANDLE HDEP5FIND;

/* ==================================================================
 * Document lifecycle
 * ================================================================== */

/**
 * @brief Open and parse a debian/copyright file.
 *
 * Reads the file, splits it into stanzas, classifies each stanza, and
 * returns a document handle. All internal buffers are owned by the
 * module and released by Dep5Close.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] phDoc    Handle receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      pszPath or phDoc is NULL.
 * @retval ERROR_OPEN_FAILED            File cannot be opened.
 * @retval ERROR_READ_FAULT             Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Memory allocation failure.
 * @retval DEP5_ERROR_INVALID_SYNTAX    Stanza structure is invalid
 *                                      (missing header, missing
 *                                      required fields, unknown
 *                                      stanza kind).
 *
 * @note Ownership of the handle transfers to the caller. It must be
 *       released with Dep5Close.
 * @see Dep5Close
 */
APIRET APIENTRY Dep5Open(PCSZ pszPath, HDEP5DOC *phDoc);

/**
 * @brief Close a document.
 *
 * Releases all internal buffers, including any active Files and
 * License cursors created from this document. After return the handle
 * is invalid.
 *
 * @param[in] hDoc  Document handle. NULLHANDLE is accepted and treated
 *                  as a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @warning Calling Dep5Close twice with the same handle is undefined.
 *          The caller should set the handle to NULLHANDLE after close.
 * @see Dep5Open
 */
APIRET APIENTRY Dep5Close(HDEP5DOC hDoc);

/* ==================================================================
 * Header stanza
 * ================================================================== */

/**
 * @brief Retrieve a field value from the header stanza.
 *
 * Supported fields (case-insensitive):
 *   "Format", "Upstream-Name", "Upstream-Contact", "Source",
 *   "Disclaimer", "Comment", "License", "Copyright".
 * Extra fields present in the source are also accessible by their
 * exact name.
 *
 * If the field is multi-line, its lines are joined with '\n'.
 * Whitespace-separated-list and line-based-list fields are returned
 * verbatim (list splitting is the caller's responsibility).
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query only
 * and returns the required size (including NUL) in *pulSize.
 *
 * @param[in]  hDoc       Handle. Not NULLHANDLE.
 * @param[in]  pszField   Field name. Not NULL, not empty.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL. On success — string
 *                        length without NUL. On BUFFER_OVERFLOW —
 *                        required size including NUL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL or field
 *                                  name empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Field not present in the header.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY Dep5HeaderGetField(HDEP5DOC hDoc, PCSZ pszField,
                                   PSZ pszBuffer, ULONG ulBufSize,
                                   PULONG pulSize);

/* ==================================================================
 * Files stanzas
 * ================================================================== */

/**
 * @brief Start enumerating Files stanzas.
 *
 * Creates a cursor and positions it on the first Files stanza. The
 * total number of Files stanzas is returned in *pulCount, allowing
 * the caller to pre-allocate memory.
 *
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[out] phFind    Cursor receiver. Not NULL. Set to NULLHANDLE
 *                       on error or if the document has no Files
 *                       stanzas.
 * @param[out] pulCount  Optional. May be NULL. On success receives
 *                       the total number of Files stanzas.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc or phFind is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 * @retval ERROR_NO_MORE_ITEMS      Document has no Files stanzas.
 *                                  *phFind = NULLHANDLE.
 *
 * @note Release the cursor with Dep5FilesFindClose (or Dep5Close if
 *       the caller forgot).
 * @see Dep5FilesFindNext, Dep5FilesFindClose
 */
APIRET APIENTRY Dep5FilesFindFirst(HDEP5DOC hDoc, HDEP5FIND *phFind,
                                   PULONG pulCount);

/**
 * @brief Advance the cursor to the next Files stanza.
 *
 * @param[in] hFind  Cursor from Dep5FilesFindFirst. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No more Files stanzas.
 *
 * @see Dep5FilesFindFirst, Dep5FilesFindClose
 */
APIRET APIENTRY Dep5FilesFindNext(HDEP5FIND hFind);

/**
 * @brief Close a Files enumeration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is accepted and treated as a
 *                   no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @note If Dep5FilesFindClose is not called, Dep5Close releases all
 *       remaining cursors.
 * @see Dep5FilesFindFirst
 */
APIRET APIENTRY Dep5FilesFindClose(HDEP5FIND hFind);

/**
 * @brief Retrieve a field value from the current Files stanza.
 *
 * Supported fields (case-insensitive):
 *   "Files", "Copyright", "License", "Comment".
 * Extra fields present in the source are also accessible by their
 * exact name.
 *
 * The returned value is the field's full text, with lines joined by
 * '\n'. Whitespace-separated lists (Files) and line-based lists
 * (Copyright) are returned verbatim; use Dep5FilesGetPattern and
 * Dep5FilesGetPatternCount for parsed Files patterns.
 *
 * The License field's indented body (after the first line) is not
 * included. Only the synopsis (short name or names, first line) is
 * returned. To fetch the license text, look it up through the
 * stand-alone License stanzas (Dep5LicenseFindFirst /
 * Dep5LicenseGetText).
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query only.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[in]  pszField   Field name. Not NULL, not empty.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Field not present in this stanza.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY Dep5FilesGetField(HDEP5FIND hFind, PCSZ pszField,
                                  PSZ pszBuffer, ULONG ulBufSize,
                                  PULONG pulSize);

/**
 * @brief Number of patterns in the Files field of the current stanza.
 *
 * Patterns are the tokens of the whitespace-separated Files list. Each
 * token has been unescaped: @c \* is @c * , @c \? is @c ? , and
 * @c \\ is @c \ . The space character cannot appear inside a pattern.
 *
 * @param[in]  hFind     Cursor. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hFind or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY Dep5FilesGetPatternCount(HDEP5FIND hFind,
                                         PULONG pulCount);

/**
 * @brief Retrieve one pattern from the Files field by index.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[in]  ulIndex    Pattern index. Range [0, count).
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Index out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY Dep5FilesGetPattern(HDEP5FIND hFind, ULONG ulIndex,
                                    PSZ pszBuffer, ULONG ulBufSize,
                                    PULONG pulSize);

/* ==================================================================
 * Stand-alone License stanzas
 * ================================================================== */

/**
 * @brief Start enumerating stand-alone License stanzas.
 *
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[out] phFind    Cursor receiver. Not NULL. Set to NULLHANDLE
 *                       on error or if the document has no License
 *                       stanzas.
 * @param[out] pulCount  Optional. May be NULL. On success receives
 *                       the total number of License stanzas.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc or phFind is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 * @retval ERROR_NO_MORE_ITEMS      Document has no License stanzas.
 *                                  *phFind = NULLHANDLE.
 *
 * @note Release the cursor with Dep5LicenseFindClose (or Dep5Close if
 *       the caller forgot).
 * @see Dep5LicenseFindNext, Dep5LicenseFindClose
 */
APIRET APIENTRY Dep5LicenseFindFirst(HDEP5DOC hDoc, HDEP5FIND *phFind,
                                     PULONG pulCount);

/**
 * @brief Advance the cursor to the next License stanza.
 *
 * @param[in] hFind  Cursor from Dep5LicenseFindFirst. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No more License stanzas.
 *
 * @see Dep5LicenseFindFirst, Dep5LicenseFindClose
 */
APIRET APIENTRY Dep5LicenseFindNext(HDEP5FIND hFind);

/**
 * @brief Close a License enumeration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is accepted and treated as a
 *                   no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @see Dep5LicenseFindFirst
 */
APIRET APIENTRY Dep5LicenseFindClose(HDEP5FIND hFind);

/**
 * @brief Retrieve the short name from the current License stanza.
 *
 * The short name is the first line of the License field, up to the
 * first whitespace. For example, for a stanza beginning with
 * "License: GPL-2+ with OpenSSL exception", the short name is
 * "GPL-2+ with OpenSSL exception".
 *
 * If the stanza is missing the License field (which the parser does
 * not allow), ERROR_FILE_NOT_FOUND is returned.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Stanza has no License field.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY Dep5LicenseGetShortName(HDEP5FIND hFind,
                                        PSZ pszBuffer, ULONG ulBufSize,
                                        PULONG pulSize);

/**
 * @brief Retrieve the license text from the current License stanza.
 *
 * The license text is the body of the License field (all lines after
 * the first), with leading whitespace of one level removed, lines
 * joined by '\n'. A single dot on a line (".") denotes a blank line
 * per DEP5 §4.4 and Debian Policy §5.6.13.
 *
 * If the stanza has no body (only the synopsis), ERROR_FILE_NOT_FOUND
 * is returned.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Stanza has no license text.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY Dep5LicenseGetText(HDEP5FIND hFind,
                                   PSZ pszBuffer, ULONG ulBufSize,
                                   PULONG pulSize);

#ifdef __cplusplus
}
#endif

#endif /* DEP5_H */
