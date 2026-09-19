/* toml.h - TOML v1.0.0 parser (C89 + Watcom extensions) */
#ifndef TOML_H
#define TOML_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file toml.h
 * @brief Public interface of the TOML v1.0.0 parser.
 *
 * Conforms to:
 *   - TOML v1.0.0.
 *     https://toml.io/en/v1.0.0
 *   - TOML ABNF.
 *     https://github.com/toml-lang/toml/blob/1.0.0/toml.abnf
 *
 * The parser owns its internal buffers, allocates them in TomlOpen and
 * releases them in TomlClose. The consumer supplies buffers only for
 * the data being returned.
 *
 * Two access styles are provided:
 *   - Point queries: TomlQueryType / TomlQueryString / ... operate on a
 *     dotted path from the document root.
 *   - DOM traversal: TomlQueryNode / TomlNode* walk the tree node by
 *     node. Nodes are handles valid until TomlClose.
 *
 * Written for Open Watcom 1.9 in C89 style. 64-bit integers use the
 * __int64 extension. No -za99 mode is required.
 */

/* ==================================================================
 * Value types
 * ================================================================== */

/** @def TOML_TYPE_STRING @brief String value. */
#define TOML_TYPE_STRING    0
/** @def TOML_TYPE_INTEGER @brief Integer value. */
#define TOML_TYPE_INTEGER   1
/** @def TOML_TYPE_FLOAT @brief Floating-point value, incl. inf/nan. */
#define TOML_TYPE_FLOAT     2
/** @def TOML_TYPE_BOOLEAN @brief Boolean value. */
#define TOML_TYPE_BOOLEAN   3
/** @def TOML_TYPE_DATETIME @brief Date/time value (textual form). */
#define TOML_TYPE_DATETIME  4
/** @def TOML_TYPE_ARRAY @brief Array value. */
#define TOML_TYPE_ARRAY     5
/** @def TOML_TYPE_TABLE @brief Table value. */
#define TOML_TYPE_TABLE     6

/* ==================================================================
 * Return codes
 * ================================================================== */

/** @def TOML_ERROR_INVALID_SYNTAX @brief TOML syntax error.
 *  User range 0xFF01. */
#define TOML_ERROR_INVALID_SYNTAX    0xFF01
/** @def TOML_ERROR_TYPE_MISMATCH @brief Value has a different type.
 *  User range 0xFF02. */
#define TOML_ERROR_TYPE_MISMATCH     0xFF02
/** @def TOML_ERROR_DUPLICATE_KEY @brief Duplicate key.
 *  User range 0xFF03. */
#define TOML_ERROR_DUPLICATE_KEY     0xFF03
/** @def TOML_ERROR_INVALID_UTF8 @brief Invalid UTF-8 input.
 *  User range 0xFF04. */
#define TOML_ERROR_INVALID_UTF8      0xFF04

/* ==================================================================
 * Handles
 * ================================================================== */

/**
 * @typedef HTOMLDOC
 * @brief TOML document handle.
 */
typedef HANDLE HTOMLDOC;

/**
 * @typedef HTOMLFIND
 * @brief TOML enumeration cursor handle.
 */
typedef HANDLE HTOMLFIND;

/**
 * @typedef HTOMLNODE
 * @brief TOML node handle (DOM-style access).
 *
 * A node handle refers to a value inside the document tree. It is
 * valid for the lifetime of the owning document. Node handles do not
 * need to be closed explicitly; TomlClose releases all of them.
 */
typedef HANDLE HTOMLNODE;

/* ==================================================================
 * Document lifecycle
 * ================================================================== */

/**
 * @brief Open a TOML document from a file.
 *
 * Reads the file, parses TOML v1.0.0, allocates all internal buffers
 * and returns a document handle. The module owns the internal
 * structures and releases them in TomlClose.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] phToml   Handle receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      pszPath or phToml is NULL.
 * @retval ERROR_OPEN_FAILED            File cannot be opened.
 * @retval ERROR_READ_FAULT             Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Memory allocation failure.
 * @retval TOML_ERROR_INVALID_UTF8      File content is not valid
 *                                      UTF-8.
 * @retval TOML_ERROR_INVALID_SYNTAX    TOML syntax error.
 * @retval TOML_ERROR_DUPLICATE_KEY     Duplicate key.
 *
 * @note Ownership of the handle transfers to the caller. It must be
 *       released with TomlClose.
 * @see TomlClose
 */
APIRET APIENTRY TomlOpen(PCSZ pszPath, HTOMLDOC *phToml);

/**
 * @brief Close a document.
 *
 * Releases all internal buffers, including any active Find cursors and
 * node handles associated with the document. After return the handle
 * is invalid.
 *
 * @param[in] hToml  Document handle. NULLHANDLE is accepted and treated
 *                   as a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @warning Calling TomlClose twice with the same handle is undefined.
 *          The caller should set the handle to NULLHANDLE after close.
 * @see TomlOpen
 */
APIRET APIENTRY TomlClose(HTOMLDOC hToml);

/* ==================================================================
 * Point queries
 * ================================================================== */

/**
 * @brief Query the type of a value by dotted path.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path "a.b.c". Not NULL, not empty.
 * @param[out] pulType  Receiver of TOML_TYPE_*. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL or path
 *                                  empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Path not found.
 */
APIRET APIENTRY TomlQueryType(HTOMLDOC hToml, PCSZ pszPath,
                              PULONG pulType);

/**
 * @brief Query a string value by dotted path.
 *
 * Copies the string into the caller-supplied buffer and appends NUL.
 * If the buffer is too small, returns ERROR_BUFFER_OVERFLOW and
 * writes the required size (including NUL) to *pulSize.
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query only.
 *
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path. Not NULL, not empty.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval ERROR_BUFFER_OVERFLOW        Buffer too small.
 * @retval TOML_ERROR_TYPE_MISMATCH     Value is neither STRING nor
 *                                      DATETIME.
 */
APIRET APIENTRY TomlQueryString(HTOMLDOC hToml, PCSZ pszPath,
                                PSZ pszBuffer, ULONG ulBufSize,
                                PULONG pulSize);

/**
 * @brief Query an integer value by dotted path.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL, not empty.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH     Value is not INTEGER.
 */
APIRET APIENTRY TomlQueryInteger(HTOMLDOC hToml, PCSZ pszPath,
                                 PLONGLONG pllValue);

/**
 * @brief Query a floating-point value by dotted path.
 *
 * @param[in]  hToml       Handle. Not NULLHANDLE.
 * @param[in]  pszPath     Path. Not NULL, not empty.
 * @param[out] pdblValue   Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH     Value is not FLOAT.
 *
 * @note The value may be inf / -inf / nan as per TOML v1.0.0. The
 *       caller is responsible for checking finiteness.
 */
APIRET APIENTRY TomlQueryFloat(HTOMLDOC hToml, PCSZ pszPath,
                               double *pdblValue);

/**
 * @brief Query a boolean value by dotted path.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL, not empty.
 * @param[out] pfValue   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH     Value is not BOOLEAN.
 */
APIRET APIENTRY TomlQueryBoolean(HTOMLDOC hToml, PCSZ pszPath,
                                 PBOOL pfValue);

/**
 * @brief Query the number of elements in an array by dotted path.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL, not empty.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH     Value is not ARRAY.
 */
APIRET APIENTRY TomlQueryArrayCount(HTOMLDOC hToml, PCSZ pszPath,
                                    PULONG pulCount);

/**
 * @brief Query the type of one array element by index.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL, not empty.
 * @param[in]  ulIndex   Element index.
 * @param[out] pulType   Receiver of TOML_TYPE_*. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval ERROR_NO_MORE_ITEMS          Index out of range.
 * @retval TOML_ERROR_TYPE_MISMATCH     Path does not refer to an
 *                                      array.
 */
APIRET APIENTRY TomlQueryArrayType(HTOMLDOC hToml, PCSZ pszPath,
                                   ULONG ulIndex, PULONG pulType);

/**
 * @brief Query a string element of an array by index.
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query only.
 *
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path. Not NULL, not empty.
 * @param[in]  ulIndex    Element index.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval ERROR_NO_MORE_ITEMS          Index out of range.
 * @retval ERROR_BUFFER_OVERFLOW        Buffer too small.
 * @retval TOML_ERROR_TYPE_MISMATCH     Element is neither STRING nor
 *                                      DATETIME.
 */
APIRET APIENTRY TomlQueryArrayString(HTOMLDOC hToml, PCSZ pszPath,
                                     ULONG ulIndex, PSZ pszBuffer,
                                     ULONG ulBufSize, PULONG pulSize);

/**
 * @brief Query an integer element of an array by index.
 *
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path. Not NULL, not empty.
 * @param[in]  ulIndex    Element index.
 * @param[out] pllValue   Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval ERROR_NO_MORE_ITEMS          Index out of range.
 * @retval TOML_ERROR_TYPE_MISMATCH     Element is not INTEGER.
 */
APIRET APIENTRY TomlQueryArrayInteger(HTOMLDOC hToml, PCSZ pszPath,
                                      ULONG ulIndex, PLONGLONG pllValue);

/**
 * @brief Query a floating-point element of an array by index.
 *
 * @param[in]  hToml       Handle. Not NULLHANDLE.
 * @param[in]  pszPath     Path. Not NULL, not empty.
 * @param[in]  ulIndex     Element index.
 * @param[out] pdblValue   Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval ERROR_NO_MORE_ITEMS          Index out of range.
 * @retval TOML_ERROR_TYPE_MISMATCH     Element is not FLOAT.
 */
APIRET APIENTRY TomlQueryArrayFloat(HTOMLDOC hToml, PCSZ pszPath,
                                    ULONG ulIndex, double *pdblValue);

/**
 * @brief Query a boolean element of an array by index.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL, not empty.
 * @param[in]  ulIndex   Element index.
 * @param[out] pfValue   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval ERROR_NO_MORE_ITEMS          Index out of range.
 * @retval TOML_ERROR_TYPE_MISMATCH     Element is not BOOLEAN.
 */
APIRET APIENTRY TomlQueryArrayBoolean(HTOMLDOC hToml, PCSZ pszPath,
                                      ULONG ulIndex, PBOOL pfValue);

/* ==================================================================
 * DOM-style traversal
 * ================================================================== */

/**
 * @brief Obtain a node handle for a value by dotted path.
 *
 * A node handle refers to a value inside the document tree. Use "" to
 * obtain the root node. Handles remain valid until TomlClose and do
 * not need to be released individually.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path "a.b.c". Not NULL. "" for root.
 * @param[out] phNode    Node receiver. Not NULL. Set to NULLHANDLE on
 *                       error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Path not found.
 *
 * @see TomlQueryRootNode, TomlNodeGetType
 */
APIRET APIENTRY TomlQueryNode(HTOMLDOC hToml, PCSZ pszPath,
                              HTOMLNODE *phNode);

/**
 * @brief Obtain a node handle for the root table.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[out] phNode   Node receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *
 * @see TomlQueryNode
 */
APIRET APIENTRY TomlQueryRootNode(HTOMLDOC hToml, HTOMLNODE *phNode);

/**
 * @brief Query the type of a node.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pulType  Receiver of TOML_TYPE_*. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY TomlNodeGetType(HTOMLNODE hNode, PULONG pulType);

/**
 * @brief Query a string value of a node.
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query only.
 *
 * @param[in]  hNode      Node handle. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_BUFFER_OVERFLOW        Buffer too small.
 * @retval TOML_ERROR_TYPE_MISMATCH     Node is neither STRING nor
 *                                      DATETIME.
 */
APIRET APIENTRY TomlNodeGetString(HTOMLNODE hNode, PSZ pszBuffer,
                                  ULONG ulBufSize, PULONG pulSize);

/**
 * @brief Query an integer value of a node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH     Node is not INTEGER.
 */
APIRET APIENTRY TomlNodeGetInteger(HTOMLNODE hNode, PLONGLONG pllValue);

/**
 * @brief Query a floating-point value of a node.
 *
 * @param[in]  hNode       Node handle. Not NULLHANDLE.
 * @param[out] pdblValue   Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH     Node is not FLOAT.
 */
APIRET APIENTRY TomlNodeGetFloat(HTOMLNODE hNode, double *pdblValue);

/**
 * @brief Query a boolean value of a node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pfValue   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH     Node is not BOOLEAN.
 */
APIRET APIENTRY TomlNodeGetBoolean(HTOMLNODE hNode, PBOOL pfValue);

/**
 * @brief Query the number of elements in an array node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH     Node is not ARRAY.
 */
APIRET APIENTRY TomlNodeGetArrayCount(HTOMLNODE hNode, PULONG pulCount);

/**
 * @brief Query an array element by index.
 *
 * @param[in]  hNode     Node handle (array). Not NULLHANDLE.
 * @param[in]  ulIndex   Element index.
 * @param[out] phChild   Receiver of the element's node handle. Not
 *                       NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_NO_MORE_ITEMS          Index out of range.
 * @retval TOML_ERROR_TYPE_MISMATCH     Node is not ARRAY.
 */
APIRET APIENTRY TomlNodeGetArrayElement(HTOMLNODE hNode, ULONG ulIndex,
                                        HTOMLNODE *phChild);

/**
 * @brief Query the number of entries in a table node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH     Node is not TABLE.
 */
APIRET APIENTRY TomlNodeGetTableCount(HTOMLNODE hNode, PULONG pulCount);

/**
 * @brief Query a table entry by key.
 *
 * @param[in]  hNode     Node handle (table). Not NULLHANDLE.
 * @param[in]  pszKey    Key. Not NULL.
 * @param[out] phChild   Receiver of the value's node handle. Not
 *                       NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_FILE_NOT_FOUND         Key not found.
 * @retval TOML_ERROR_TYPE_MISMATCH     Node is not TABLE.
 */
APIRET APIENTRY TomlNodeGetTableEntryByKey(HTOMLNODE hNode, PCSZ pszKey,
                                           HTOMLNODE *phChild);

/**
 * @brief Query a table entry by index.
 *
 * The order of entries corresponds to the order of definition in the
 * source TOML file.
 *
 * @param[in]  hNode          Node handle (table). Not NULLHANDLE.
 * @param[in]  ulIndex        Entry index.
 * @param[out] pszKeyBuffer   Key output buffer. Not NULL.
 * @param[in]  ulKeyBufSize   Size of pszKeyBuffer in bytes.
 * @param[out] pulKeyUsed     Optional. May be NULL.
 * @param[out] phChild        Receiver of the value's node handle. Not
 *                            NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_NO_MORE_ITEMS          Index out of range.
 * @retval ERROR_BUFFER_OVERFLOW        Key buffer too small.
 * @retval TOML_ERROR_TYPE_MISMATCH     Node is not TABLE.
 */
APIRET APIENTRY TomlNodeGetTableEntryByIndex(HTOMLNODE hNode,
                                             ULONG ulIndex,
                                             PSZ pszKeyBuffer,
                                             ULONG ulKeyBufSize,
                                             PULONG pulKeyUsed,
                                             HTOMLNODE *phChild);

/* ==================================================================
 * Directory enumeration
 * ================================================================== */

/**
 * @brief Start enumerating entries in a table.
 *
 * Creates a cursor and positions it on the first entry matching the
 * pattern. The cursor is released by TomlFindClose, or by TomlClose
 * of the owning document.
 *
 * @par Pattern syntax
 * '*' matches any sequence of characters, '?' matches any single
 * character. The pattern "*" matches all entries.
 *
 * @param[in]  hToml       Handle. Not NULLHANDLE.
 * @param[in]  pszPath     Path to the table. Not NULL. Use "" for
 *                         root.
 * @param[in]  pszPattern  Pattern. Not NULL.
 * @param[out] phFind      Cursor receiver. Not NULL. Set to NULLHANDLE
 *                         on error or when there are no matching
 *                         entries.
 * @param[out] pulType     Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success. Cursor positioned on
 *                                      the first entry.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Memory allocation failure.
 * @retval ERROR_FILE_NOT_FOUND         Path not found.
 * @retval ERROR_NO_MORE_ITEMS          No entry matches the pattern.
 *                                      *phFind = NULLHANDLE.
 * @retval TOML_ERROR_TYPE_MISMATCH     Path does not refer to a
 *                                      table.
 *
 * @note The cursor owns internal buffers allocated from the document.
 *       Always release it via TomlFindClose.
 * @see TomlFindNext, TomlFindClose
 */
APIRET APIENTRY TomlFindFirst(HTOMLDOC hToml, PCSZ pszPath,
                              PCSZ pszPattern, HTOMLFIND *phFind,
                              PULONG pulType);

/**
 * @brief Advance the cursor to the next matching entry.
 *
 * @param[in]  hFind    Cursor from TomlFindFirst. Not NULLHANDLE.
 * @param[out] pulType  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No more matching entries.
 *
 * @see TomlFindFirst, TomlFindClose
 */
APIRET APIENTRY TomlFindNext(HTOMLFIND hFind, PULONG pulType);

/**
 * @brief Retrieve the key of the current entry.
 *
 * Copies the key into the caller-supplied buffer and appends NUL.
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
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY TomlFindKey(HTOMLFIND hFind,
                            PSZ pszBuffer, ULONG ulBufSize,
                            PULONG pulSize);

/**
 * @brief Retrieve the string value of the current entry.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW        Buffer too small.
 * @retval TOML_ERROR_TYPE_MISMATCH     Current entry is neither
 *                                      STRING nor DATETIME.
 */
APIRET APIENTRY TomlFindString(HTOMLFIND hFind,
                               PSZ pszBuffer, ULONG ulBufSize,
                               PULONG pulSize);

/**
 * @brief Retrieve the integer value of the current entry.
 *
 * @param[in]  hFind     Cursor. Not NULLHANDLE.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval TOML_ERROR_TYPE_MISMATCH     Current entry is not INTEGER.
 */
APIRET APIENTRY TomlFindInteger(HTOMLFIND hFind, PLONGLONG pllValue);

/**
 * @brief Retrieve the floating-point value of the current entry.
 *
 * @param[in]  hFind       Cursor. Not NULLHANDLE.
 * @param[out] pdblValue   Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval TOML_ERROR_TYPE_MISMATCH     Current entry is not FLOAT.
 */
APIRET APIENTRY TomlFindFloat(HTOMLFIND hFind, double *pdblValue);

/**
 * @brief Retrieve the boolean value of the current entry.
 *
 * @param[in]  hFind     Cursor. Not NULLHANDLE.
 * @param[out] pfValue   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval TOML_ERROR_TYPE_MISMATCH     Current entry is not BOOLEAN.
 */
APIRET APIENTRY TomlFindBoolean(HTOMLFIND hFind, PBOOL pfValue);

/**
 * @brief Close an enumeration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is accepted and treated as a
 *                   no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @note If TomlFindClose is not called, TomlClose releases all
 *       remaining cursors.
 * @see TomlFindFirst
 */
APIRET APIENTRY TomlFindClose(HTOMLFIND hFind);

#ifdef __cplusplus
}
#endif

#endif /* TOML_H */
