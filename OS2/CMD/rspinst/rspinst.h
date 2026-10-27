/*!
 * @file rspinst.h
 *
 * @brief Public interface of the OS/2 response file parser.
 *
 * Parses response files used by the OS/2 Warp 4 CID installation
 * process and exposes their contents through a handle-based API.
 * The module performs no installation.
 *
 * @par References
 *   - IBM Redbook SG24-2010-00, section 7.1.2,
 *     "Detailed Keyword Description".
 *   - IBM Redbook SG24-2010-00, section 7.1.3,
 *     "New Keywords in OS/2 Warp 4".
 *   - IBM Redbook SG24-2010-00, section 7.1.5,
 *     "Include Keywords Processing Sequence".
 *   - IBM Redbook SG24-2010-00, section 7.2,
 *     "Feature Installer Response File".
 *   - IBM OS/2 Warp 4 Toolkit, header file OS2DEF.H, which defines
 *     LHANDLE, APIRET, ULONG, PCSZ, BOOL, APIENTRY, NO_ERROR,
 *     ERROR_* and NULLHANDLE.
 *
 * @par Syntax summary
 *   - '#' , ';' and '*' introduce a comment line
 *   - KEY=value establishes a scalar entry
 *   - KEY=( ... ) establishes a nested block
 *   - Include=<file>        deferred include (FIFO, breadth-first);
 *                           see SG24-2010-00 §7.1.5.3
 *   - IncludeInLine=<file>  immediate include (depth-first);
 *                           see SG24-2010-00 §7.1.5.2
 *   - keyword lookup is case-insensitive; the original spelling is
 *     preserved for round-tripping
 *   - a duplicate keyword keeps every occurrence in document order;
 *     RspFindLast and RspFindFirst select one of them
 *
 * @par Buffer size query
 * All accessors that return a string follow the standard size
 * query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written
 *   - ulSize large enough: text copied and NUL-terminated, *pulUsed
 *     is the length without the NUL
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW, *pulUsed is the
 *     required size including the NUL
 */

#ifndef RSPINST_H
#define RSPINST_H

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Constants
 * ================================================================== */

/*!
 * @def RSP_LINE_MAX
 * @brief Maximum length of one physical line, in bytes.
 *
 * Lines longer than this are truncated and reported as
 * RSP_ERROR_LINE_TOO_LONG. Value: 8192.
 *
 * @par Reference
 *   Length chosen to cover the largest response file example in
 *   IBM Redbook SG24-2010-00, Figure 57, with headroom.
 */
#define RSP_LINE_MAX            8192

/*!
 * @def RSP_MAX_INCLUDE_DEPTH
 * @brief Maximum nesting depth for Include / block recursion.
 *
 * Value: 64.
 *
 * @par Reference
 *   Bounds the recursive processing described in IBM Redbook
 *   SG24-2010-00, section 7.1.5.3.
 */
#define RSP_MAX_INCLUDE_DEPTH   64

/*!
 * @def RSP_MAX_FILES
 * @brief Maximum number of distinct files visited per document.
 *
 * Value: 512.
 *
 * @par Reference
 *   Guards against unbounded Include graphs as described in IBM
 *   Redbook SG24-2010-00, section 7.1.5.
 */
#define RSP_MAX_FILES           512

/*!
 * @def RSP_VERSION_STRING
 * @brief Version string reported by RspQueryVersion.
 */
#define RSP_VERSION_STRING      "rspinst 1.0.0"

/* ==================================================================
 * Handles
 * ================================================================== */

/*!
 * @typedef HRSPINST
 * @brief Handle to a parsed response file.
 *
 * Created by RspOpen and released by RspClose. NULLHANDLE means
 * "no document". A non-null handle is an opaque token; passing a
 * stale or foreign handle to any Rsp* function is detected and
 * reported as ERROR_INVALID_HANDLE.
 *
 * @par Reference
 *   Type LHANDLE is defined in IBM OS/2 Toolkit OS2DEF.H, and is
 *   the standard 32-bit handle type used by OS/2 APIs.
 */
typedef LHANDLE HRSPINST;

/*!
 * @typedef HRSPANN
 * @brief Borrowed handle to one entry inside a document.
 *
 * An entry is either a scalar (name plus value) or a block (name
 * plus a sequence of child entries). Entry handles are valid for
 * the lifetime of the owning document; do not release them
 * separately.
 */
typedef LHANDLE HRSPANN;

/*!
 * @typedef HRSPREPORT
 * @brief Handle to an accumulated validation report.
 *
 * Created by RspReportCreate, released by RspReportFree. Use
 * RspReportQueryCount and RspReportQueryEntry to walk the records.
 */
typedef LHANDLE HRSPREPORT;

/* ==================================================================
 * Entry type
 * ================================================================== */

/*!
 * @def RSP_ENTRYTYPE_SCALAR
 * @brief Entry is a scalar (name plus value). Value: 0.
 *
 * @par Reference
 *   Form "KEY=value", IBM Redbook SG24-2010-00, section 7.1.2.
 */
#define RSP_ENTRYTYPE_SCALAR    0

/*!
 * @def RSP_ENTRYTYPE_BLOCK
 * @brief Entry is a block (name plus children). Value: 1.
 *
 * @par Reference
 *   Form "KEY=( ... )", IBM Redbook SG24-2010-00, section 7.2.
 */
#define RSP_ENTRYTYPE_BLOCK     1

/* ==================================================================
 * Diagnostic severity
 * ================================================================== */

/*!
 * @def RSP_SEV_ERROR
 * @brief Violation of a MUST requirement. Value: 1.
 */
#define RSP_SEV_ERROR           1

/*!
 * @def RSP_SEV_WARNING
 * @brief Violation of a SHOULD requirement. Value: 2.
 */
#define RSP_SEV_WARNING         2

/*!
 * @def RSP_SEV_INFO
 * @brief Purely informational. Value: 3.
 */
#define RSP_SEV_INFO            3

/* ==================================================================
 * Return codes
 * ================================================================== */

/*!
 * @def RSP_ERROR_SYNTAX
 * @brief Response file is syntactically malformed. Value: 0xFF01.
 *
 * @par Reference
 *   Syntax rules from IBM Redbook SG24-2010-00, sections 7.1.2
 *   and 7.2.
 */
#define RSP_ERROR_SYNTAX                0xFF01

/*!
 * @def RSP_ERROR_INCLUDE_DEPTH
 * @brief Include or block nesting exceeds RSP_MAX_INCLUDE_DEPTH.
 * Value: 0xFF02.
 *
 * @par Reference
 *   Include processing per IBM Redbook SG24-2010-00, section
 *   7.1.5.
 */
#define RSP_ERROR_INCLUDE_DEPTH         0xFF02

/*!
 * @def RSP_ERROR_CYCLIC_INCLUDE
 * @brief An include cycle was detected. Value: 0xFF03.
 *
 * @par Reference
 *   Include cycle detection per IBM Redbook SG24-2010-00, section
 *   7.1.5.1.
 */
#define RSP_ERROR_CYCLIC_INCLUDE        0xFF03

/*!
 * @def RSP_ERROR_LINE_TOO_LONG
 * @brief A physical line exceeds RSP_LINE_MAX. Value: 0xFF04.
 */
#define RSP_ERROR_LINE_TOO_LONG         0xFF04

/*!
 * @def RSP_ERROR_UNEXPECTED_EOF
 * @brief Unexpected end of file inside an open block. Value: 0xFF05.
 *
 * @par Reference
 *   Block syntax per IBM Redbook SG24-2010-00, section 7.2.
 */
#define RSP_ERROR_UNEXPECTED_EOF        0xFF05

/*!
 * @def RSP_ERROR_TOO_MANY_FILES
 * @brief Number of distinct files exceeds RSP_MAX_FILES.
 * Value: 0xFF06.
 */
#define RSP_ERROR_TOO_MANY_FILES        0xFF06

/*!
 * @def RSP_ERROR_VALIDATION_FAILED
 * @brief Validation found at least one error-severity record.
 * Value: 0xFF07.
 */
#define RSP_ERROR_VALIDATION_FAILED     0xFF07

/*!
 * @def RSP_WARN_UNKNOWN_KEY
 * @brief Keyword is not present in the RSPINST keyword table.
 * Value: 0xFF10.
 */
#define RSP_WARN_UNKNOWN_KEY            0xFF10

/*!
 * @def RSP_WARN_DEPRECATED_KEY
 * @brief Keyword is documented as deprecated. Value: 0xFF11.
 *
 * @par Reference
 *   BaseFileSystem and FormatPartition are superseded by FormatFAT
 *   and FormatHPFS per IBM Redbook SG24-2010-00, section 7.1.2.
 *   WARMPLUG.Selection is not supported per the same section.
 */
#define RSP_WARN_DEPRECATED_KEY         0xFF11

/*!
 * @def RSP_WARN_VALUE_NOT_INTEGER
 * @brief A numeric field could not be parsed as an integer.
 * Value: 0xFF12.
 */
#define RSP_WARN_VALUE_NOT_INTEGER      0xFF12

/*!
 * @def RSP_WARN_VALUE_OUT_OF_RANGE
 * @brief A numeric field is outside its documented range.
 * Value: 0xFF13.
 */
#define RSP_WARN_VALUE_OUT_OF_RANGE     0xFF13

/*!
 * @def RSP_WARN_VALUE_NOT_DRIVE
 * @brief A field that expects a drive letter was malformed.
 * Value: 0xFF14.
 */
#define RSP_WARN_VALUE_NOT_DRIVE        0xFF14

/*!
 * @def RSP_WARN_MUTEX_FORMAT_KEYWORDS
 * @brief FormatFAT/FormatHPFS mixed with the older BaseFileSystem
 * or FormatPartition keywords. Value: 0xFF15.
 *
 * @par Reference
 *   Old and new formatting keywords are mutually exclusive per IBM
 *   Redbook SG24-2010-00, section 7.1.2, note on "Support for Old
 *   Keywords".
 */
#define RSP_WARN_MUTEX_FORMAT_KEYWORDS  0xFF15

/*!
 * @def RSP_WARN_CID_EXIT_ON_ERROR
 * @brief ExitOnError is not set to 1, which CID requires.
 * Value: 0xFF16.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, note on ExitOnError.
 */
#define RSP_WARN_CID_EXIT_ON_ERROR      0xFF16

/*!
 * @def RSP_WARN_CID_REBOOT_REQUIRED
 * @brief RebootRequired is not set to 0, which CID requires.
 * Value: 0xFF17.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, note on
 *   RebootRequired.
 */
#define RSP_WARN_CID_REBOOT_REQUIRED    0xFF17

/*!
 * @def RSP_WARN_WINOS2_CONFLICT
 * @brief WIN-OS/2Support uses 0 together with other values.
 * Value: 0xFF18.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, keyword
 *   WIN-OS/2Support.
 */
#define RSP_WARN_WINOS2_CONFLICT        0xFF18

/*!
 * @def RSP_WARN_WINOS2_REDUNDANT
 * @brief WIN-OS/2Support uses 1 with additional values.
 * Value: 0xFF19.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, keyword
 *   WIN-OS/2Support.
 */
#define RSP_WARN_WINOS2_REDUNDANT       0xFF19

/*!
 * @def RSP_WARN_WINOS2_CONTRADICT
 * @brief WIN-OS/2Support uses 6 with additional values.
 * Value: 0xFF1A.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, keyword
 *   WIN-OS/2Support.
 */
#define RSP_WARN_WINOS2_CONTRADICT      0xFF1A

/*!
 * @def RSP_WARN_EMPTY_PATH
 * @brief A path keyword is empty. Value: 0xFF1B.
 */
#define RSP_WARN_EMPTY_PATH             0xFF1B

/*!
 * @def RSP_WARN_BAD_PRINTER_ENTRY
 * @brief AdditionalPrinters entry is not "port:index".
 * Value: 0xFF1C.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, keyword
 *   AdditionalPrinters.
 */
#define RSP_WARN_BAD_PRINTER_ENTRY      0xFF1C

/*!
 * @def RSP_WARN_BAD_OS2INIDATA
 * @brief OS2IniData entry is not "/App/Key/Value/".
 * Value: 0xFF1D.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, keyword OS2IniData.
 */
#define RSP_WARN_BAD_OS2INIDATA         0xFF1D

/*!
 * @def RSP_WARN_BAD_COPY_ENTRY
 * @brief Copy entry is not "source destination". Value: 0xFF1E.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, keyword Copy.
 */
#define RSP_WARN_BAD_COPY_ENTRY         0xFF1E

/* ==================================================================
 * Keyword table types
 * ================================================================== */

/*!
 * @enum _RSPKEYTYPE
 * @brief Category of a documented keyword.
 */
typedef enum _RSPKEYTYPE {
    RSP_KEYTYPE_STRING     = 0,  /*!< Free-form string.                  */
    RSP_KEYTYPE_INTEGER    = 1,  /*!< Integer inside [lMin, lMax].       */
    RSP_KEYTYPE_BOOLEAN    = 2,  /*!< 0 or 1.                            */
    RSP_KEYTYPE_LIST       = 3,  /*!< Comma-separated list.              */
    RSP_KEYTYPE_BLOCK      = 4,  /*!< Nested block.                      */
    RSP_KEYTYPE_DRIVE      = 5,  /*!< Drive letter, optionally with ':'. */
    RSP_KEYTYPE_PATH       = 6,  /*!< File system path.                  */
    RSP_KEYTYPE_INCLUDE    = 7,  /*!< Include or IncludeInLine.          */
    RSP_KEYTYPE_DEPRECATED = 8   /*!< Deprecated; see pszReplacement.    */
} RSPKEYTYPE;

/*!
 * @struct _RSPKEYINFO
 * @brief Static metadata for one documented keyword.
 *
 * The pointer returned by RspLookupKey points into a static table
 * with the lifetime of the process. Fields must not be freed.
 *
 * Every entry carries pszSource, which names the normative source
 * from which the keyword, its range and its meaning are taken.
 */
typedef struct _RSPKEYINFO {
    PCSZ       pszName;         /*!< Keyword name, case-preserved.      */
    RSPKEYTYPE type;            /*!< Category.                          */
    LONG       lMin;            /*!< Minimum value for INTEGER/BOOLEAN. */
    LONG       lMax;            /*!< Maximum value for INTEGER/BOOLEAN. */
    PCSZ       pszDescription;  /*!< One-line description.              */
    PCSZ       pszReplacement;  /*!< Replacement for deprecated keys;
                                 *   NULL otherwise.                     */
    PCSZ       pszSource;       /*!< Normative source reference
                                 *   (SG24-2010-00 section, figure or
                                 *   table).                             */
} RSPKEYINFO, *PRSPKEYINFO;

/* ==================================================================
 * Diagnostic record
 * ================================================================== */

/*!
 * @struct _RSPERR
 * @brief One diagnostic emitted by RspValidate.
 *
 * All string fields are NUL-terminated. The structure is owned by
 * the report handle and remains valid until the report is freed.
 */
typedef struct _RSPERR {
    ULONG ulCode;           /*!< One of RSP_ERROR_* or RSP_WARN_*.     */
    ULONG ulSeverity;       /*!< One of RSP_SEV_*.                     */
    ULONG ulLine;           /*!< Source line, 0 if not applicable.     */
    CHAR  achFile[512];     /*!< Source file path, "" if not set.      */
    CHAR  achDetail[256];   /*!< Human-readable detail.                */
} RSPERR, *PRSPERR;

/*!
 * @struct _RSPVALIDATEOPTS
 * @brief Policy switches for RspValidate.
 *
 * A zero-initialised block applies no CID-specific rules and emits
 * warnings only. Use RspValidateDefaults to obtain the
 * CID-compatible defaults.
 *
 * @par Reference
 *   CID requirements for ExitOnError and RebootRequired are stated
 *   in IBM Redbook SG24-2010-00, section 7.1.2.
 */
typedef struct _RSPVALIDATEOPTS {
    BOOL fRequireCidExitOnError;    /*!< Require ExitOnError=1.         */
    BOOL fRequireCidRebootRequired; /*!< Require RebootRequired=0.      */
    BOOL fWarnUnknown;              /*!< Warn on unknown keywords.      */
    BOOL fWarnDeprecated;           /*!< Warn on deprecated keywords.   */
    BOOL fStrict;                   /*!< Treat warnings as errors.      */
} RSPVALIDATEOPTS, *PRSPVALIDATEOPTS;

/* ==================================================================
 * Document lifecycle
 * ================================================================== */

/*!
 * @brief Open and parse a response file.
 *
 * Reads @p pszPath, resolves Include and IncludeInLine references
 * recursively, and stores every entry in document order. All
 * buffers are owned by the resulting handle.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.5.
 *
 * @param[in]  pszPath  Path to the response file. Not NULL.
 * @param[out] phDoc    Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phDoc is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval RSP_ERROR_SYNTAX         Syntax error.
 * @retval RSP_ERROR_LINE_TOO_LONG  Line exceeds RSP_LINE_MAX.
 * @retval RSP_ERROR_UNEXPECTED_EOF End of file inside an open block.
 * @retval RSP_ERROR_INCLUDE_DEPTH  Include nesting too deep.
 * @retval RSP_ERROR_CYCLIC_INCLUDE Include cycle detected.
 * @retval RSP_ERROR_TOO_MANY_FILES Too many distinct files.
 *
 * @see RspClose
 */
APIRET APIENTRY RspOpen(PCSZ pszPath, HRSPINST *phDoc);

/*!
 * @brief Release a parsed response file.
 *
 * Every entry, every include list and every buffer owned by the
 * handle is released. Passing NULLHANDLE is a no-op.
 *
 * @param[in] hDoc  Handle. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 *
 * @see RspOpen
 */
APIRET APIENTRY RspClose(HRSPINST hDoc);

/* ==================================================================
 * Document metadata
 * ================================================================== */

/*!
 * @brief Query the original path passed to RspOpen.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_INVALID_DATA       Stored source path is missing.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RspQuerySourcePath(HRSPINST hDoc, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Query the number of entries at the top level.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspQueryEntryCount(HRSPINST hDoc, PULONG pulCount);

/*!
 * @brief Query the number of parse errors recorded while opening.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspQueryErrorCount(HRSPINST hDoc, PULONG pulCount);

/*!
 * @brief Query the number of parse warnings recorded while opening.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspQueryWarningCount(HRSPINST hDoc, PULONG pulCount);

/*!
 * @brief Query the version string.
 *
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pszBuf is NULL without size
 *                                  query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RspQueryVersion(PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Entry access
 * ================================================================== */

/*!
 * @brief Obtain a borrowed handle to the entry at the top level.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] phAnn    Receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY RspQueryEntry(HRSPINST hDoc, ULONG ulIndex,
                              HRSPANN *phAnn);

/*!
 * @brief Query the type of an entry.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pulType  Receiver of RSP_ENTRYTYPE_*. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 */
APIRET APIENTRY RspEntryQueryType(HRSPANN hAnn, PULONG pulType);

/*!
 * @brief Query the keyword name of an entry.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RspEntryQueryName(HRSPANN hAnn, PSZ pszBuf,
                                  ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Query the value of a scalar entry.
 *
 * Blocks have no scalar value; the function returns
 * ERROR_INVALID_DATA for a block handle.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_INVALID_DATA       Entry is a block.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RspEntryQueryValue(HRSPANN hAnn, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Query the source file path of an entry.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RspEntryQuerySourceFile(HRSPANN hAnn, PSZ pszBuf,
                                        ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Query the 1-based source line of an entry.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pulLine  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 */
APIRET APIENTRY RspEntryQueryLineNumber(HRSPANN hAnn, PULONG pulLine);

/*!
 * @brief Query the number of children of a block entry.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_INVALID_DATA       Entry is a scalar.
 */
APIRET APIENTRY RspEntryQueryChildCount(HRSPANN hAnn, PULONG pulCount);

/*!
 * @brief Obtain a borrowed handle to one child of a block.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based child index.
 * @param[out] phChild  Receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_INVALID_DATA       Entry is a scalar.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY RspEntryQueryChild(HRSPANN hAnn, ULONG ulIndex,
                                   HRSPANN *phChild);

/*!
 * @brief Find a child of a block by name.
 *
 * The comparison is case-insensitive. If several children share
 * the name, the last one wins.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[in]  pszName  Child name. Not NULL.
 * @param[out] phChild  Receiver. Not NULL. Set to NULLHANDLE when
 *                      the child is not present.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_INVALID_DATA       Entry is a scalar.
 * @retval ERROR_FILE_NOT_FOUND     No child with that name.
 */
APIRET APIENTRY RspEntryFindChild(HRSPANN hAnn, PCSZ pszName,
                                  HRSPANN *phChild);

/* ==================================================================
 * Top-level lookup
 * ================================================================== */

/*!
 * @brief Find the last top-level entry with the given name.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[in]  pszName  Keyword name. Not NULL.
 * @param[out] phAnn    Receiver. Not NULL. Set to NULLHANDLE when
 *                      the keyword is not present.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No matching entry.
 */
APIRET APIENTRY RspFindLast(HRSPINST hDoc, PCSZ pszName, HRSPANN *phAnn);

/*!
 * @brief Find the first top-level entry with the given name.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[in]  pszName  Keyword name. Not NULL.
 * @param[out] phAnn    Receiver. Not NULL. Set to NULLHANDLE when
 *                      the keyword is not present.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No matching entry.
 */
APIRET APIENTRY RspFindFirst(HRSPINST hDoc, PCSZ pszName, HRSPANN *phAnn);

/*!
 * @brief Count top-level entries with the given name.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[in]  pszName  Keyword name. Not NULL.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspCount(HRSPINST hDoc, PCSZ pszName, PULONG pulCount);

/*!
 * @brief Find the last top-level entry using a dotted path.
 *
 * The path "A.B.C" first tries the flat keyword "A.B.C" (for
 * keywords such as "ART.Selection"), then walks the nested blocks
 * A -> B -> C.
 *
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[in]  pszDotted Dotted keyword path. Not NULL.
 * @param[out] phAnn     Receiver. Not NULL. Set to NULLHANDLE when
 *                       the keyword is not present.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No matching entry.
 */
APIRET APIENTRY RspFindLastDotted(HRSPINST hDoc, PCSZ pszDotted,
                                  HRSPANN *phAnn);

/*!
 * @brief Find the first top-level entry using a dotted path.
 *
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[in]  pszDotted Dotted keyword path. Not NULL.
 * @param[out] phAnn     Receiver. Not NULL. Set to NULLHANDLE when
 *                       the keyword is not present.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No matching entry.
 */
APIRET APIENTRY RspFindFirstDotted(HRSPINST hDoc, PCSZ pszDotted,
                                   HRSPANN *phAnn);

/* ==================================================================
 * Keyword metadata
 * ================================================================== */

/*!
 * @brief Look up the static metadata of a documented keyword.
 *
 * The comparison is case-insensitive. The returned pointer refers
 * to a static table and must not be freed. The pszSource field of
 * the returned structure names the normative source from which the
 * keyword is taken.
 *
 * @param[in] pszName  Keyword name. Not NULL.
 *
 * @return Pointer to the metadata, or NULL when the keyword is not
 *         documented.
 *
 * @retval NULL     @p pszName is NULL or the keyword is unknown.
 * @retval other    A pointer into the static keyword table.
 */
const RSPKEYINFO * APIENTRY RspLookupKey(PCSZ pszName);

/* ==================================================================
 * Dump helpers
 * ================================================================== */

/*!
 * @typedef RSPWRITEFN
 * @brief Callback used by RspDump to emit text.
 *
 * @param[in] pszText   NUL-terminated text. Not NULL.
 * @param[in] ulLength  Length in bytes, without the NUL.
 * @param[in] pvUser    Caller context from the RspDump call.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Write accepted.
 */
typedef APIRET (APIENTRY *RSPWRITEFN)(PCSZ pszText, ULONG ulLength,
                                      PVOID pvUser);

/*!
 * @brief Serialise the whole document through a callback.
 *
 * When @p fIniStyle is FALSE, the output is the original nested
 * form: each entry as "KEY=value", each block as "KEY=(" followed
 * by indented children and ")". When @p fIniStyle is TRUE, every
 * scalar is flattened to "A.B.C=value" and blocks are not echoed
 * separately.
 *
 * @param[in] hDoc       Handle. Not NULLHANDLE.
 * @param[in] fIniStyle  TRUE for flat INI form, FALSE for nested.
 * @param[in] pfnWrite   Callback. Not NULL.
 * @param[in] pvUser     Context passed to @p pfnWrite.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hDoc or @p pfnWrite is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_WRITE_FAULT        @p pfnWrite rejected a chunk.
 */
APIRET APIENTRY RspDump(HRSPINST hDoc, BOOL fIniStyle,
                        RSPWRITEFN pfnWrite, PVOID pvUser);

/* ==================================================================
 * Validation report
 * ================================================================== */

/*!
 * @brief Create an empty validation report.
 *
 * @param[out] phReport  Receiver. Not NULL. Set to NULLHANDLE on
 *                       error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p phReport is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @see RspReportFree
 */
APIRET APIENTRY RspReportCreate(HRSPREPORT *phReport);

/*!
 * @brief Release a validation report.
 *
 * @param[in] hReport  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 */
APIRET APIENTRY RspReportFree(HRSPREPORT hReport);

/*!
 * @brief Query the number of records in a report.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspReportQueryCount(HRSPREPORT hReport, PULONG pulCount);

/*!
 * @brief Query the number of records with severity RSP_SEV_ERROR.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspReportQueryErrorCount(HRSPREPORT hReport,
                                         PULONG pulCount);

/*!
 * @brief Query the number of records with severity RSP_SEV_WARNING.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspReportQueryWarningCount(HRSPREPORT hReport,
                                           PULONG pulCount);

/*!
 * @brief Retrieve one diagnostic record by index.
 *
 * The returned structure is a snapshot; string fields point to
 * memory owned by the report and remain valid until RspReportFree.
 *
 * @param[in]  hReport  Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pErr     Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY RspReportQueryEntry(HRSPREPORT hReport, ULONG ulIndex,
                                    PRSPERR pErr);

/*!
 * @brief Fill a validate-options block with CID defaults.
 *
 * The resulting block sets fRequireCidExitOnError and
 * fRequireCidRebootRequired to TRUE, fWarnUnknown and
 * fWarnDeprecated to TRUE, and fStrict to FALSE.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, notes on ExitOnError
 *   and RebootRequired.
 *
 * @param[out] pOpts  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pOpts is NULL.
 */
APIRET APIENTRY RspValidateDefaults(PRSPVALIDATEOPTS pOpts);

/*!
 * @brief Validate a parsed document.
 *
 * Every record produced by validation is appended to @p hReport.
 * The function itself returns NO_ERROR when no error-severity
 * record was produced; RSP_ERROR_VALIDATION_FAILED means at least
 * one error record is now in the report.
 *
 * @param[in] hDoc    Handle. Not NULLHANDLE.
 * @param[in] pOpts   Policy block. Not NULL.
 * @param[in] hReport Report handle. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                       Validation completed with
 *                                       no error-severity records.
 * @retval RSP_ERROR_VALIDATION_FAILED    At least one error record
 *                                       was produced.
 * @retval ERROR_INVALID_PARAMETER        Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE           A handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY        Allocation failure.
 */
APIRET APIENTRY RspValidate(HRSPINST hDoc,
                            const RSPVALIDATEOPTS *pOpts,
                            HRSPREPORT hReport);

#ifdef __cplusplus
}
#endif

#endif /* RSPINST_H */
