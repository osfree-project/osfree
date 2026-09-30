/*!
 * @file hpj.h
 * @brief WinHelp Help Project (HPJ) parser library.
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
 *  - [3] Borland Languages Help Compiler User's Guide, 1991,
 *        "The Help Project File".
 *  - [4] Microsoft Help Workshop, "Help Project File Sections",
 *        MSDN Library, 1992.
 *  - [5] Microsoft Help Workshop, "Building a Help File with Build Tags".
 *  - [6] ccl container library: vector.h.
 *
 * @par Supported sections
 *  - [OPTIONS]    build options
 *  - [FILES]      topic files (RTF)
 *  - [ALIAS]      context string aliases
 *  - [MAP]        context number mapping; supports #include and
 *                 #define directives
 *  - [WINDOWS]    window definitions
 *  - [CONFIG]     macros and DLL registrations
 *  - [BITMAPS]    bitmap files
 *  - [BAGGAGE]    embedded files
 *  - [BUILDTAGS]  build tags
 *
 * @par Thread safety
 * The library is single threaded. Callers must provide locking if an
 * HHPJ is shared between threads.
 *
 * @par Ownership
 * HHPJ is caller-owned and released with HpjDestroyDoc. Strings
 * returned through Query APIs are copied into caller buffers. All
 * internal data is copied into the document on parse.
 */
#ifndef HPJ_H
#define HPJ_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @typedef HHPJ
 * @brief Handle to an HPJ document. Caller-owned.
 */
typedef HANDLE HHPJ;

/*!
 * @typedef HHPJENUM
 * @brief Handle to an enumeration cursor. Caller-owned.
 *
 * Reserved for future use; retained for API stability.
 */
typedef HANDLE HHPJENUM;

typedef HHPJ     *PHHPJ;
typedef HHPJENUM *PHHPJENUM;

/* ==================================================================
 * Section names
 * ================================================================== */

/*!
 * @def HPJ_SECTION_OPTIONS
 * @brief Name of the [OPTIONS] section.
 */
#define HPJ_SECTION_OPTIONS   "OPTIONS"

/*!
 * @def HPJ_SECTION_FILES
 * @brief Name of the [FILES] section.
 */
#define HPJ_SECTION_FILES     "FILES"

/*!
 * @def HPJ_SECTION_ALIAS
 * @brief Name of the [ALIAS] section.
 */
#define HPJ_SECTION_ALIAS     "ALIAS"

/*!
 * @def HPJ_SECTION_MAP
 * @brief Name of the [MAP] section.
 */
#define HPJ_SECTION_MAP       "MAP"

/*!
 * @def HPJ_SECTION_WINDOWS
 * @brief Name of the [WINDOWS] section.
 */
#define HPJ_SECTION_WINDOWS   "WINDOWS"

/*!
 * @def HPJ_SECTION_CONFIG
 * @brief Name of the [CONFIG] section.
 */
#define HPJ_SECTION_CONFIG    "CONFIG"

/*!
 * @def HPJ_SECTION_BITMAPS
 * @brief Name of the [BITMAPS] section.
 */
#define HPJ_SECTION_BITMAPS   "BITMAPS"

/*!
 * @def HPJ_SECTION_BAGGAGE
 * @brief Name of the [BAGGAGE] section.
 */
#define HPJ_SECTION_BAGGAGE   "BAGGAGE"

/*!
 * @def HPJ_SECTION_BUILDTAGS
 * @brief Name of the [BUILDTAGS] section.
 */
#define HPJ_SECTION_BUILDTAGS "BUILDTAGS"

/* ==================================================================
 * Window definition
 * ================================================================== */

/*!
 * @struct HPJWINDOW
 * @brief Parsed window definition from the [WINDOWS] section.
 *
 * The structure is a flat view of one line of the [WINDOWS] section.
 * Unused fields are left at zero.
 */
typedef struct {
    CHAR  szName[64];       /*!< Window name (left of '='). */
    CHAR  szCaption[256];   /*!< Window caption in quotes. */
    int   x;                /*!< X position, 0..1000. */
    int   y;                /*!< Y position, 0..1000. */
    int   w;                /*!< Width, 0..1000. */
    int   h;                /*!< Height, 0..1000. */
    int   state;            /*!< Maximize / autosize state. */
    BYTE  rgb[3];           /*!< Scrollable-region RGB. */
    BYTE  rgbNsr[3];        /*!< Non-scrollable-region RGB. */
    int   top;              /*!< Window-on-top flag. */
    int   scrollbars;       /*!< Scrollbar style. */
} HPJWINDOW;

/* ==================================================================
 * Document lifecycle
 * ================================================================== */

/*!
 * @brief Create an empty HPJ document.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: @e Create implies that a new
 * resource is created as the result of a function call.
 *
 * @param[out] phDoc Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phDoc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 *
 * @sa HpjDestroyDoc
 */
APIRET APIENTRY HpjCreateDoc(PHHPJ phDoc);

/*!
 * @brief Destroy an HPJ document and release all its data.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hDoc Document handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR              Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE  Handle is not recognized.
 *
 * @sa HpjCreateDoc
 */
APIRET APIENTRY HpjDestroyDoc(HHPJ hDoc);

/* ==================================================================
 * Parsing
 * ================================================================== */

/*!
 * @brief Read one HPJ file and populate the document.
 *
 * Sections are parsed semantically. Line comments introduced by a
 * semicolon outside quotes are removed. Sections may appear in any
 * order. The [MAP] section supports #include and #define directives;
 * #include files are searched relative to the directory of the HPJ
 * file that contains the directive.
 *
 * @param[in] hDoc        Document handle. Not NULLHANDLE.
 * @param[in] pszFileName NUL-terminated file name. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc or @a pszFileName is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     File could not be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HpjReadFile(HHPJ hDoc, PCSZ pszFileName);

/*!
 * @brief Read a list of HPJ files and populate the document.
 *
 * Sections from later files are appended to sections from earlier
 * files.
 *
 * @param[in] hDoc     Document handle. Not NULLHANDLE.
 * @param[in] cFiles   Number of files. Must be > 0.
 * @param[in] apszFiles Array of NUL-terminated file names. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL or @a cFiles <= 0.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     A file could not be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HpjReadFiles(HHPJ hDoc, int cFiles, PCSZ* apszFiles);

/* ==================================================================
 * Query: options ([OPTIONS])
 * ================================================================== */

/*!
 * @brief Query the number of [OPTIONS] entries.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: @e Query implies that a system value
 * is returned.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryOptionCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query the option name at the given index.
 *
 * Uses the size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: value copied, NUL-terminated.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW, *pulUsed is required size.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryOptionName(HHPJ hDoc, ULONG ulIndex,
                                   PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Query an option value by name.
 *
 * The name is compared case-insensitively. Returns
 * ERROR_FILE_NOT_FOUND if the option is not present.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  pszName Option name. Not NULL.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc or @a pszName is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Option not present.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryOptionValue(HHPJ hDoc, PCSZ pszName,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Query: files ([FILES])
 * ================================================================== */

/*!
 * @brief Query the number of files listed in [FILES].
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryFileCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query a file name from [FILES] by index.
 *
 * Uses the size-query convention.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryFileName(HHPJ hDoc, ULONG ulIndex,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Query: aliases ([ALIAS])
 * ================================================================== */

/*!
 * @brief Query the number of alias entries.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryAliasCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query an alias name by index.
 *
 * Uses the size-query convention.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryAliasName(HHPJ hDoc, ULONG ulIndex,
                                  PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Query the context string that an alias points to.
 *
 * Uses the size-query convention.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryAliasContext(HHPJ hDoc, ULONG ulIndex,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Resolve an alias name to its context string.
 *
 * The name is compared case-insensitively. Returns
 * ERROR_FILE_NOT_FOUND if the alias is not present.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  pszName Alias name. Not NULL.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc or @a pszName is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Alias not present.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryAliasResolve(HHPJ hDoc, PCSZ pszName,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Query: maps ([MAP])
 * ================================================================== */

/*!
 * @brief Query the number of numeric context mappings.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryMapCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query a context string from [MAP] by index.
 *
 * Uses the size-query convention.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryMapContext(HHPJ hDoc, ULONG ulIndex,
                                   PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Query the numeric identifier from [MAP] by index.
 *
 * @param[in]  hDoc      Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex   Zero-based index.
 * @param[out] pulNumber Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 */
APIRET APIENTRY HpjQueryMapNumber(HHPJ hDoc, ULONG ulIndex, PULONG pulNumber);

/*!
 * @brief Resolve a context string to its numeric identifier.
 *
 * The context is compared case-insensitively. Returns
 * ERROR_FILE_NOT_FOUND if the context is not present.
 *
 * @param[in]  hDoc      Document handle. Not NULLHANDLE.
 * @param[in]  pszContext Context string. Not NULL.
 * @param[out] pulNumber Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Context not present.
 */
APIRET APIENTRY HpjQueryMapResolve(HHPJ hDoc, PCSZ pszContext, PULONG pulNumber);

/* ==================================================================
 * Query: #define symbols from [MAP]
 * ================================================================== */

/*!
 * @brief Query the number of #define symbols found in [MAP].
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryDefineCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query a #define symbol name by index.
 *
 * Uses the size-query convention.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryDefineName(HHPJ hDoc, ULONG ulIndex,
                                   PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Query a #define symbol value by index.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pulValue Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 */
APIRET APIENTRY HpjQueryDefineValue(HHPJ hDoc, ULONG ulIndex, PULONG pulValue);

/* ==================================================================
 * Query: #include files from [MAP]
 * ================================================================== */

/*!
 * @brief Query the number of #include files found in [MAP].
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryIncludeCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query an #include file name by index.
 *
 * Uses the size-query convention.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryIncludeName(HHPJ hDoc, ULONG ulIndex,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Query: windows ([WINDOWS])
 * ================================================================== */

/*!
 * @brief Query the number of window definitions.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryWindowCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query a window definition by index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pWin    Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 */
APIRET APIENTRY HpjQueryWindow(HHPJ hDoc, ULONG ulIndex, HPJWINDOW* pWin);

/*!
 * @brief Query a window definition by name.
 *
 * The name is compared case-insensitively.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  pszName Window name. Not NULL.
 * @param[out] pWin    Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Window not present.
 */
APIRET APIENTRY HpjQueryWindowByName(HHPJ hDoc, PCSZ pszName, HPJWINDOW* pWin);

/* ==================================================================
 * Query: config macros ([CONFIG])
 * ================================================================== */

/*!
 * @brief Query the number of [CONFIG] macro lines.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryConfigCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query a [CONFIG] macro line by index.
 *
 * Uses the size-query convention.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryConfig(HHPJ hDoc, ULONG ulIndex,
                               PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Query: bitmaps ([BITMAPS])
 * ================================================================== */

/*!
 * @brief Query the number of bitmap file names.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryBitmapCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query a bitmap file name by index.
 *
 * Uses the size-query convention.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryBitmapName(HHPJ hDoc, ULONG ulIndex,
                                   PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Query: baggage ([BAGGAGE])
 * ================================================================== */

/*!
 * @brief Query the number of baggage file names.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryBaggageCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query a baggage file name by index.
 *
 * Uses the size-query convention.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryBaggageName(HHPJ hDoc, ULONG ulIndex,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Query: build tags ([BUILDTAGS])
 * ================================================================== */

/*!
 * @brief Query the number of build tags.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HpjQueryBuildTagCount(HHPJ hDoc, PULONG pulCount);

/*!
 * @brief Query a build tag by index.
 *
 * The returned string is the tag name with any leading '+' or '-'
 * prefix preserved.
 *
 * Uses the size-query convention.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryBuildTagName(HHPJ hDoc, ULONG ulIndex,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Testing
 * ================================================================== */

/*!
 * @brief Compare two HPJ documents for equality.
 *
 * Intended for unit tests. Returns TRUE if both documents have the
 * same sections and entries in the same order.
 *
 * @param[in] hDocA First document. Not NULLHANDLE.
 * @param[in] hDocB Second document. Not NULLHANDLE.
 *
 * @return TRUE if equal, FALSE otherwise.
 */
BOOL APIENTRY HpjEquals(HHPJ hDocA, HHPJ hDocB);

#ifdef __cplusplus
}
#endif

#endif /* HPJ_H */
