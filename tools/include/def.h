/*! def.h - module-definition (.DEF) file parser
 *
 *  Provides a handle-based API for reading a .DEF file as
 *  understood by IBM LINK386. All statements documented in the
 *  LINK386 Reference are recognized and parsed.
 */

#ifndef __DEF__
#define __DEF__

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file def.h
 *  @brief Module-definition (.DEF) file parser.
 *
 *  Provides a handle-based API for reading a .DEF file as
 *  understood by IBM LINK386. All statements documented in the
 *  LINK386 Reference are recognized and parsed:
 *
 *    BASE             base address
 *    CODE             default code segment attributes
 *    DATA             default data segment attributes
 *    DESCRIPTION      module description text
 *    EXETYPE          target operating system
 *    EXPORTS          exported functions
 *    HEAPSIZE         local heap size
 *    IMPORTS          imported functions
 *    LIBRARY          library name, initialization, termination
 *    NAME             application name and type
 *    OLD              old module for ordinal preservation
 *    PHYSICAL DEVICE  physical device driver name
 *    PROTMODE         protected-mode-only flag
 *    SEGMENTS         per-segment attributes
 *    STACKSIZE        local stack size
 *    STUB             DOS stub file name
 *    VIRTUAL DEVICE   virtual device driver name
 *
 *  All handles (HDEFFILE, HDEFIMPORT, HDEFEXPORT, HDEFSEGMENT) are
 *  opaque; the internal representation is private to def.c.
 *
 *  References:
 *    - IBM OS/2 Warp 4.5 Toolkit, Tools Reference, LINK386:
 *        "Module Statement Rules"
 *        "Module Statements"
 *        "BASE Statement"
 *        "CODE Statement" and its attribute pages
 *        "DATA Statement" and its attribute pages
 *        "DESCRIPTION Statement"
 *        "EXETYPE Statement"
 *        "EXPORTS Statement"
 *        "HEAPSIZE Statement"
 *        "IMPORTS Statement"
 *        "LIBRARY Statement"
 *        "NAME Statement"
 *        "OLD Statement"
 *        "PHYSICAL DEVICE Statement"
 *        "PROTMODE Statement"
 *        "SEGMENTS Statement" and its attribute pages
 *        "STACKSIZE Statement"
 *        "STUB Statement"
 *        "VIRTUAL DEVICE Statement"
 *        "Entering Numeric Arguments"
 *      (os2tk45/toolsref, komh.github.io/os2books)
 *    - SAA CPI C Reference, Level 2 (SC09-1308-02, Sep 1991).
 */

/* ==================================================================
 * Handles
 * ================================================================== */

/*! @brief Handle to a parsed .DEF file.
 *
 *  Opaque. Created by DefOpen and released by DefClose. The internal
 *  representation is private to def.c.
 *
 *  @see DefOpen
 *  @see DefClose
 */
typedef HANDLE HDEFFILE;

/*! @brief Pointer to a .DEF file handle. */
typedef HDEFFILE *PHDEFFILE;

/*! @brief Handle to an IMPORTS enumeration cursor.
 *
 *  Opaque. Created by DefFindFirstImport and released by
 *  DefFindCloseImport. The internal representation is private to
 *  def.c.
 *
 *  @see DefFindFirstImport
 *  @see DefFindCloseImport
 */
typedef HANDLE HDEFIMPORT;

/*! @brief Handle to an EXPORTS enumeration cursor.
 *
 *  Opaque. Created by DefFindFirstExport and released by
 *  DefFindCloseExport. The internal representation is private to
 *  def.c.
 *
 *  @see DefFindFirstExport
 *  @see DefFindCloseExport
 */
typedef HANDLE HDEFEXPORT;

/*! @brief Handle to a SEGMENTS enumeration cursor.
 *
 *  Opaque. Created by DefFindFirstSegment and released by
 *  DefFindCloseSegment. The internal representation is private to
 *  def.c.
 *
 *  @see DefFindFirstSegment
 *  @see DefFindCloseSegment
 */
typedef HANDLE HDEFSEGMENT;

/* ==================================================================
 * Module types
 * ================================================================== */

/**
 * @def DEF_MODTYPE_UNSPECIFIED
 * @brief No identifying statement was present. Value: 0.
 */
#define DEF_MODTYPE_UNSPECIFIED      0

/**
 * @def DEF_MODTYPE_APPLICATION
 * @brief NAME statement was present. Value: 1.
 */
#define DEF_MODTYPE_APPLICATION      1

/**
 * @def DEF_MODTYPE_LIBRARY
 * @brief LIBRARY statement was present. Value: 2.
 */
#define DEF_MODTYPE_LIBRARY          2

/**
 * @def DEF_MODTYPE_PHYSICAL_DEVICE
 * @brief PHYSICAL DEVICE statement was present. Value: 3.
 */
#define DEF_MODTYPE_PHYSICAL_DEVICE  3

/**
 * @def DEF_MODTYPE_VIRTUAL_DEVICE
 * @brief VIRTUAL DEVICE statement was present. Value: 4.
 */
#define DEF_MODTYPE_VIRTUAL_DEVICE   4

/* ==================================================================
 * EXETYPE values
 * ================================================================== */

/**
 * @def DEF_EXETYPE_UNSPECIFIED
 * @brief No EXETYPE statement was present. Value: 0.
 */
#define DEF_EXETYPE_UNSPECIFIED  0

/**
 * @def DEF_EXETYPE_OS2
 * @brief EXETYPE OS2. Value: 1.
 */
#define DEF_EXETYPE_OS2          1

/**
 * @def DEF_EXETYPE_WINDOWS
 * @brief EXETYPE WINDOWS. Value: 2.
 */
#define DEF_EXETYPE_WINDOWS      2

/**
 * @def DEF_EXETYPE_UNKNOWN
 * @brief EXETYPE UNKNOWN. Value: 3.
 */
#define DEF_EXETYPE_UNKNOWN      3

/* ==================================================================
 * LIBRARY initialization and termination
 * ================================================================== */

/**
 * @def DEF_INIT_UNSPECIFIED
 * @brief No initialization keyword. Value: 0.
 */
#define DEF_INIT_UNSPECIFIED     0

/**
 * @def DEF_INIT_GLOBAL
 * @brief INITGLOBAL. Value: 1.
 */
#define DEF_INIT_GLOBAL          1

/**
 * @def DEF_INIT_INSTANCE
 * @brief INITINSTANCE. Value: 2.
 */
#define DEF_INIT_INSTANCE        2

/**
 * @def DEF_TERM_UNSPECIFIED
 * @brief No termination keyword. Value: 0.
 */
#define DEF_TERM_UNSPECIFIED     0

/**
 * @def DEF_TERM_GLOBAL
 * @brief TERMGLOBAL. Value: 1.
 */
#define DEF_TERM_GLOBAL          1

/**
 * @def DEF_TERM_INSTANCE
 * @brief TERMINSTANCE. Value: 2.
 */
#define DEF_TERM_INSTANCE        2

/* ==================================================================
 * NAME application types
 * ================================================================== */

/**
 * @def DEF_APPTYPE_UNSPECIFIED
 * @brief No application type keyword. Value: 0.
 */
#define DEF_APPTYPE_UNSPECIFIED      0

/**
 * @def DEF_APPTYPE_WINDOWAPI
 * @brief WINDOWAPI. Value: 1.
 */
#define DEF_APPTYPE_WINDOWAPI        1

/**
 * @def DEF_APPTYPE_WINDOWCOMPAT
 * @brief WINDOWCOMPAT. Value: 2.
 */
#define DEF_APPTYPE_WINDOWCOMPAT     2

/**
 * @def DEF_APPTYPE_NOTWINDOWCOMPAT
 * @brief NOTWINDOWCOMPAT. Value: 3.
 */
#define DEF_APPTYPE_NOTWINDOWCOMPAT  3

/* ==================================================================
 * Attribute values shared by CODE, DATA and SEGMENTS
 * ================================================================== */

/**
 * @def DEF_LOAD_UNSPECIFIED
 * @brief No load attribute. Value: 0.
 */
#define DEF_LOAD_UNSPECIFIED  0

/**
 * @def DEF_LOAD_PRELOAD
 * @brief PRELOAD. Value: 1.
 */
#define DEF_LOAD_PRELOAD      1

/**
 * @def DEF_LOAD_LOADONCALL
 * @brief LOADONCALL. Value: 2.
 */
#define DEF_LOAD_LOADONCALL   2

/**
 * @def DEF_RW_UNSPECIFIED
 * @brief No read/write attribute. Value: 0.
 */
#define DEF_RW_UNSPECIFIED  0

/**
 * @def DEF_RW_READONLY
 * @brief READONLY. Value: 1.
 */
#define DEF_RW_READONLY     1

/**
 * @def DEF_RW_READWRITE
 * @brief READWRITE. Value: 2.
 */
#define DEF_RW_READWRITE    2

/**
 * @def DEF_RX_UNSPECIFIED
 * @brief No read/execute attribute. Value: 0.
 */
#define DEF_RX_UNSPECIFIED  0

/**
 * @def DEF_RX_EXECUTEONLY
 * @brief EXECUTEONLY. Value: 1.
 */
#define DEF_RX_EXECUTEONLY  1

/**
 * @def DEF_RX_EXECUTEREAD
 * @brief EXECUTEREAD. Value: 2.
 */
#define DEF_RX_EXECUTEREAD  2

/**
 * @def DEF_IOPL_UNSPECIFIED
 * @brief No I/O privilege attribute. Value: 0.
 */
#define DEF_IOPL_UNSPECIFIED  0

/**
 * @def DEF_IOPL_YES
 * @brief IOPL. Value: 1.
 */
#define DEF_IOPL_YES          1

/**
 * @def DEF_IOPL_NO
 * @brief NOIOPL. Value: 2.
 */
#define DEF_IOPL_NO           2

/**
 * @def DEF_CONF_UNSPECIFIED
 * @brief No conforming attribute. Value: 0.
 */
#define DEF_CONF_UNSPECIFIED   0

/**
 * @def DEF_CONF_CONFORMING
 * @brief CONFORMING. Value: 1.
 */
#define DEF_CONF_CONFORMING    1

/**
 * @def DEF_CONF_NONCONFORMING
 * @brief NONCONFORMING. Value: 2.
 */
#define DEF_CONF_NONCONFORMING 2

/**
 * @def DEF_SHARING_UNSPECIFIED
 * @brief No sharing attribute. Value: 0.
 */
#define DEF_SHARING_UNSPECIFIED  0

/**
 * @def DEF_SHARING_NONE
 * @brief NONE. Value: 1.
 */
#define DEF_SHARING_NONE         1

/**
 * @def DEF_SHARING_SINGLE
 * @brief SINGLE. Value: 2.
 */
#define DEF_SHARING_SINGLE       2

/**
 * @def DEF_SHARING_MULTIPLE
 * @brief MULTIPLE. Value: 3.
 */
#define DEF_SHARING_MULTIPLE     3

/**
 * @def DEF_SHARE_UNSPECIFIED
 * @brief No shareable attribute. Value: 0.
 */
#define DEF_SHARE_UNSPECIFIED  0

/**
 * @def DEF_SHARE_SHARED
 * @brief SHARED. Value: 1.
 */
#define DEF_SHARE_SHARED       1

/**
 * @def DEF_SHARE_NONSHARED
 * @brief NONSHARED. Value: 2.
 */
#define DEF_SHARE_NONSHARED    2

/* ==================================================================
 * Attribute selectors
 * ================================================================== */

/**
 * @def DEF_ATTR_LOAD_CODE
 * @brief Selector for DefQueryCodeAttr. Value: 1.
 */
#define DEF_ATTR_LOAD_CODE  1

/**
 * @def DEF_ATTR_RX
 * @brief Selector for DefQueryCodeAttr. Value: 2.
 */
#define DEF_ATTR_RX         2

/**
 * @def DEF_ATTR_IOPL_CODE
 * @brief Selector for DefQueryCodeAttr. Value: 3.
 */
#define DEF_ATTR_IOPL_CODE  3

/**
 * @def DEF_ATTR_CONF
 * @brief Selector for DefQueryCodeAttr. Value: 4.
 */
#define DEF_ATTR_CONF       4

/**
 * @def DEF_ATTR_LOAD_DATA
 * @brief Selector for DefQueryDataAttr. Value: 1.
 */
#define DEF_ATTR_LOAD_DATA  1

/**
 * @def DEF_ATTR_RW
 * @brief Selector for DefQueryDataAttr. Value: 2.
 */
#define DEF_ATTR_RW         2

/**
 * @def DEF_ATTR_SHARING
 * @brief Selector for DefQueryDataAttr. Value: 3.
 */
#define DEF_ATTR_SHARING    3

/**
 * @def DEF_ATTR_SHARE
 * @brief Selector for DefQueryDataAttr. Value: 4.
 */
#define DEF_ATTR_SHARE      4

/**
 * @def DEF_ATTR_IOPL_DATA
 * @brief Selector for DefQueryDataAttr. Value: 5.
 */
#define DEF_ATTR_IOPL_DATA  5

/**
 * @def DEF_SEGATTR_LOAD
 * @brief Selector for DefQuerySegmentAttr. Value: 1.
 */
#define DEF_SEGATTR_LOAD    1

/**
 * @def DEF_SEGATTR_RW
 * @brief Selector for DefQuerySegmentAttr. Value: 2.
 */
#define DEF_SEGATTR_RW      2

/**
 * @def DEF_SEGATTR_RX
 * @brief Selector for DefQuerySegmentAttr. Value: 3.
 */
#define DEF_SEGATTR_RX      3

/**
 * @def DEF_SEGATTR_IOPL
 * @brief Selector for DefQuerySegmentAttr. Value: 4.
 */
#define DEF_SEGATTR_IOPL    4

/**
 * @def DEF_SEGATTR_CONF
 * @brief Selector for DefQuerySegmentAttr. Value: 5.
 */
#define DEF_SEGATTR_CONF    5

/**
 * @def DEF_SEGATTR_MIXED
 * @brief Selector for DefQuerySegmentAttr. Value: 6.
 */
#define DEF_SEGATTR_MIXED   6

/**
 * @def DEF_SEGATTR_ALIAS
 * @brief Selector for DefQuerySegmentAttr. Value: 7.
 */
#define DEF_SEGATTR_ALIAS   7

/**
 * @def DEF_SEGATTR_SHARE
 * @brief Selector for DefQuerySegmentAttr. Value: 8.
 */
#define DEF_SEGATTR_SHARE   8

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*! @brief Open and parse a .DEF file.
 *
 *  Reads the file, strips comments, dispatches on the first keyword
 *  of each line and collects the IMPORTS, EXPORTS and SEGMENTS
 *  entries into vectors inside the handle.
 *
 *  @par Reference
 *  LINK386 Reference, "Module Statement Rules".
 *
 *  @param[in]  pszPath  Path to the .DEF file. Not NULL.
 *  @param[out] phDef    Receives the handle. Not NULL. Set to
 *                       NULLHANDLE on failure.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phDef is NULL.
 *  @retval ERROR_OPEN_FAILED        File cannot be opened.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see DefClose
 */
APIRET APIENTRY DefOpen(PCSZ pszPath, PHDEFFILE phDef);

/*! @brief Close a parsed .DEF file and release all records.
 *
 *  Releases the import, export and segment vectors together with
 *  every string owned by the handle. Passing NULLHANDLE is a no-op.
 *
 *  @param[in] hDef  Handle. May be NULLHANDLE.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR               Success. Also for NULLHANDLE.
 *  @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 *
 *  @see DefOpen
 */
APIRET APIENTRY DefClose(HDEFFILE hDef);

/* ==================================================================
 * Module identification
 * ================================================================== */

/*! @brief Return the module type.
 *
 *  @par Reference
 *  LINK386 Reference, "LIBRARY Statement", "NAME Statement",
 *  "PHYSICAL DEVICE Statement", "VIRTUAL DEVICE Statement".
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[out] pulType  Receives one of DEF_MODTYPE_*. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryModuleType(HDEFFILE hDef, PULONG pulType);

/*! @brief Return the module name.
 *
 *  The name comes from LIBRARY, NAME, PHYSICAL DEVICE or VIRTUAL
 *  DEVICE, whichever was present. If the statement carried no name,
 *  ERROR_FILE_NOT_FOUND is returned; the linker's fallback to the
 *  executable file name is not performed by this parser.
 *
 *  @par Reference
 *  LINK386 Reference, "LIBRARY Statement", "NAME Statement",
 *  "PHYSICAL DEVICE Statement", "VIRTUAL DEVICE Statement".
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer, or NULL for a size query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     No identifying statement carried
 *                                   a name.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryModuleName(HDEFFILE hDef,
                                   PSZ pszBuf, ULONG ulSize,
                                   PULONG pulUsed);

/*! @brief Return the LIBRARY initialization mode.
 *
 *  @par Reference
 *  LINK386 Reference, "LIBRARY Statement": INITGLOBAL (default) or
 *  INITINSTANCE.
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[out] pulInit  Receives one of DEF_INIT_*. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     Module is not a LIBRARY.
 */
APIRET APIENTRY DefQueryLibraryInit(HDEFFILE hDef, PULONG pulInit);

/*! @brief Return the LIBRARY termination mode.
 *
 *  @par Reference
 *  LINK386 Reference, "LIBRARY Statement": TERMGLOBAL (default) or
 *  TERMINSTANCE.
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[out] pulTerm  Receives one of DEF_TERM_*. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     Module is not a LIBRARY.
 */
APIRET APIENTRY DefQueryLibraryTerm(HDEFFILE hDef, PULONG pulTerm);

/*! @brief Return the NAME application type.
 *
 *  @par Reference
 *  LINK386 Reference, "NAME Statement": WINDOWAPI, WINDOWCOMPAT or
 *  NOTWINDOWCOMPAT.
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[out] pulType  Receives one of DEF_APPTYPE_*. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     Module is not an application.
 */
APIRET APIENTRY DefQueryApplicationType(HDEFFILE hDef, PULONG pulType);

/*! @brief Return the DESCRIPTION text.
 *
 *  @par Reference
 *  LINK386 Reference, "DESCRIPTION Statement":
 *    DESCRIPTION 'text'
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer, or NULL for a size query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     No DESCRIPTION statement present.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryDescription(HDEFFILE hDef,
                                    PSZ pszBuf, ULONG ulSize,
                                    PULONG pulUsed);

/*! @brief Return the EXETYPE value.
 *
 *  @par Reference
 *  LINK386 Reference, "EXETYPE Statement": OS2 (default), WINDOWS or
 *  UNKNOWN.
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[out] pulType  Receives one of DEF_EXETYPE_*. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryExeType(HDEFFILE hDef, PULONG pulType);

/*! @brief Return the BASE address.
 *
 *  @par Reference
 *  LINK386 Reference, "BASE Statement": syntax BASE=n.
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[out] pulBase  Receives the base address. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     No BASE statement present.
 */
APIRET APIENTRY DefQueryBase(HDEFFILE hDef, PULONG pulBase);

/*! @brief Return the HEAPSIZE value.
 *
 *  @par Reference
 *  LINK386 Reference, "HEAPSIZE Statement":
 *    HEAPSIZE bytes | MAXVAL
 *
 *  @param[in]  hDef      Handle. Not NULLHANDLE.
 *  @param[out] pulBytes  Receives the heap size. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     No HEAPSIZE statement present.
 */
APIRET APIENTRY DefQueryHeapSize(HDEFFILE hDef, PULONG pulBytes);

/*! @brief Test whether HEAPSIZE was given as MAXVAL.
 *
 *  @par Reference
 *  LINK386 Reference, "HEAPSIZE Statement".
 *
 *  @param[in]  hDef      Handle. Not NULLHANDLE.
 *  @param[out] pfMaxVal  Receives TRUE if MAXVAL was given. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryHeapSizeMaxVal(HDEFFILE hDef, PBOOL pfMaxVal);

/*! @brief Return the STACKSIZE value.
 *
 *  @par Reference
 *  LINK386 Reference, "STACKSIZE Statement":
 *    STACKSIZE number
 *
 *  @param[in]  hDef      Handle. Not NULLHANDLE.
 *  @param[out] pulBytes  Receives the stack size. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     No STACKSIZE statement present.
 */
APIRET APIENTRY DefQueryStackSize(HDEFFILE hDef, PULONG pulBytes);

/*! @brief Return the OLD module file name.
 *
 *  @par Reference
 *  LINK386 Reference, "OLD Statement":
 *    OLD 'filename'
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer, or NULL for a size query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     No OLD statement present.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryOldName(HDEFFILE hDef,
                                PSZ pszBuf, ULONG ulSize,
                                PULONG pulUsed);

/*! @brief Return the STUB file name.
 *
 *  @par Reference
 *  LINK386 Reference, "STUB Statement":
 *    STUB 'filename'
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer, or NULL for a size query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     No STUB statement present.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryStubName(HDEFFILE hDef,
                                 PSZ pszBuf, ULONG ulSize,
                                 PULONG pulUsed);

/*! @brief Test whether PROTMODE was present.
 *
 *  @par Reference
 *  LINK386 Reference, "PROTMODE Statement": syntax PROTMODE.
 *
 *  @param[in]  hDef        Handle. Not NULLHANDLE.
 *  @param[out] pfProtMode  Receives TRUE if PROTMODE was present.
 *                          Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryProtMode(HDEFFILE hDef, PBOOL pfProtMode);

/*! @brief Return the number of parse errors encountered.
 *
 *  Counts lines that failed to parse: malformed entries, oversized
 *  lines, statements out of order, duplicate identifying statements.
 *  This counter is an extension; it is not part of the LINK386
 *  Reference.
 *
 *  @param[in]  hDef      Handle. Not NULLHANDLE.
 *  @param[out] pulCount  Receives the error count. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryParseErrorCount(HDEFFILE hDef, PULONG pulCount);

/* ==================================================================
 * CODE and DATA default attributes
 * ================================================================== */

/*! @brief Return a default CODE attribute.
 *
 *  @par Reference
 *  LINK386 Reference, "CODE Statement" and its attribute pages:
 *  "Load Code Attributes", "Read/Execute Code Attributes",
 *  "I/O Privilege Code Attributes", "Conforming Code Attributes".
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[in]  ulAttr   One of DEF_ATTR_LOAD_CODE, DEF_ATTR_RX,
 *                       DEF_ATTR_IOPL_CODE, DEF_ATTR_CONF.
 *  @param[out] pulValue Receives the attribute value. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad attribute selector or NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryCodeAttr(HDEFFILE hDef, ULONG ulAttr,
                                 PULONG pulValue);

/*! @brief Return a default DATA attribute.
 *
 *  @par Reference
 *  LINK386 Reference, "DATA Statement" and its attribute pages:
 *  "Load Data Attributes", "Read/Write Data Attributes",
 *  "Sharing Data Attributes", "Shareable Data Attributes",
 *  "LINK386 -I/O Privilege Data Attributes".
 *
 *  @param[in]  hDef     Handle. Not NULLHANDLE.
 *  @param[in]  ulAttr   One of DEF_ATTR_LOAD_DATA, DEF_ATTR_RW,
 *                       DEF_ATTR_SHARING, DEF_ATTR_SHARE,
 *                       DEF_ATTR_IOPL_DATA.
 *  @param[out] pulValue Receives the attribute value. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad attribute selector or NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryDataAttr(HDEFFILE hDef, ULONG ulAttr,
                                 PULONG pulValue);

/* ==================================================================
 * SEGMENTS enumeration
 * ================================================================== */

/*! @brief Open a cursor on the first SEGMENTS entry.
 *
 *  @par Reference
 *  LINK386 Reference, "SEGMENTS Statement".
 *
 *  @param[in]  hDef       Handle. Not NULLHANDLE.
 *  @param[out] phSegment  Receives the cursor. Not NULL. Set to
 *                         NULLHANDLE on error or when there are no
 *                         entries.
 *  @param[out] pulCount   Optional. May be NULL. On success receives
 *                         the total number of entries.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hDef or @p phSegment is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      No entries. *phSegment is
 *                                   NULLHANDLE.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see DefFindNextSegment
 *  @see DefFindCloseSegment
 */
APIRET APIENTRY DefFindFirstSegment(HDEFFILE hDef, HDEFSEGMENT *phSegment,
                                    PULONG pulCount);

/*! @brief Advance a SEGMENTS cursor to the next entry.
 *
 *  @param[in] hSegment  Cursor. Not NULLHANDLE.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      No more entries.
 *
 *  @see DefFindFirstSegment
 *  @see DefFindCloseSegment
 */
APIRET APIENTRY DefFindNextSegment(HDEFSEGMENT hSegment);

/*! @brief Close a SEGMENTS cursor.
 *
 *  Passing NULLHANDLE is a no-op.
 *
 *  @param[in] hSegment  Cursor. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                Success. Also for NULLHANDLE.
 *  @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 *  @see DefFindFirstSegment
 */
APIRET APIENTRY DefFindCloseSegment(HDEFSEGMENT hSegment);

/*! @brief Return the name of the current segment.
 *
 *  @par Reference
 *  LINK386 Reference, "SEGMENTS Statement".
 *
 *  @param[in]  hSegment  Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf    Output buffer, or NULL for a size query.
 *  @param[in]  ulSize    Size of @p pszBuf in bytes.
 *  @param[out] pulUsed   Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQuerySegmentName(HDEFSEGMENT hSegment,
                                    PSZ pszBuf, ULONG ulSize,
                                    PULONG pulUsed);

/*! @brief Return the class name of the current segment.
 *
 *  When the source line had no CLASS argument, the accessor returns
 *  the string "CODE", which is the documented default.
 *
 *  @par Reference
 *  LINK386 Reference, "SEGMENTS Statement": "If you do not use the
 *  CLASS argument, LINK386 assumes that the class is CODE."
 *
 *  @param[in]  hSegment  Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf    Output buffer, or NULL for a size query.
 *  @param[in]  ulSize    Size of @p pszBuf in bytes.
 *  @param[out] pulUsed   Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQuerySegmentClass(HDEFSEGMENT hSegment,
                                     PSZ pszBuf, ULONG ulSize,
                                     PULONG pulUsed);

/*! @brief Return an attribute of the current segment.
 *
 *  @par Reference
 *  LINK386 Reference, "SEGMENTS Statement" and its attribute pages:
 *  "Load Segments Attributes", "Read/Write Segments Attributes",
 *  "Read/Execute Segments Attributes",
 *  "I/O Privilege Segments Attributes",
 *  "Conforming Segments Attributes",
 *  "Specify Mixed 16 and 32-Bit Segments",
 *  "Specify that Segment is Aliased",
 *  "Specify that Segment is Shared".
 *
 *  @param[in]  hSegment  Cursor. Not NULLHANDLE.
 *  @param[in]  ulAttr    One of DEF_SEGATTR_*.
 *  @param[out] pulValue  Receives the attribute value. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad attribute selector or NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQuerySegmentAttr(HDEFSEGMENT hSegment, ULONG ulAttr,
                                    PULONG pulValue);

/* ==================================================================
 * IMPORTS enumeration
 * ================================================================== */

/*! @brief Open a cursor on the first IMPORTS entry.
 *
 *  @param[in]  hDef      Handle. Not NULLHANDLE.
 *  @param[out] phImport  Receives the cursor. Not NULL. Set to
 *                        NULLHANDLE on error or when there are no
 *                        entries.
 *  @param[out] pulCount  Optional. May be NULL. On success receives
 *                        the total number of entries.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hDef or @p phImport is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      No entries. *phImport is
 *                                   NULLHANDLE.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see DefFindNextImport
 *  @see DefFindCloseImport
 */
APIRET APIENTRY DefFindFirstImport(HDEFFILE hDef, HDEFIMPORT *phImport,
                                   PULONG pulCount);

/*! @brief Advance an IMPORTS cursor to the next entry.
 *
 *  @param[in] hImport  Cursor. Not NULLHANDLE.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      No more entries.
 *
 *  @see DefFindFirstImport
 *  @see DefFindCloseImport
 */
APIRET APIENTRY DefFindNextImport(HDEFIMPORT hImport);

/*! @brief Close an IMPORTS cursor.
 *
 *  Passing NULLHANDLE is a no-op.
 *
 *  @param[in] hImport  Cursor. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                Success. Also for NULLHANDLE.
 *  @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 *  @see DefFindFirstImport
 */
APIRET APIENTRY DefFindCloseImport(HDEFIMPORT hImport);

/*! @brief Return the internal name of the current IMPORTS entry.
 *
 *  The internal name is the left-hand side of "NAME = MOD.EXT". If
 *  the source line omitted the "internalname=" prefix, the accessor
 *  returns the external name (the documented default).
 *
 *  @par Reference
 *  LINK386 Reference, "IMPORTS Statement":
 *    [internalname=]modulename.entry
 *
 *  @param[in]  hImport  Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer, or NULL for a size query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryImportInternal(HDEFIMPORT hImport,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed);

/*! @brief Return the module name of the current IMPORTS entry.
 *
 *  @par Reference
 *  LINK386 Reference, "IMPORTS Statement": modulename.
 *
 *  @param[in]  hImport  Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer, or NULL for a size query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryImportModule(HDEFIMPORT hImport,
                                     PSZ pszBuf, ULONG ulSize,
                                     PULONG pulUsed);

/*! @brief Return the entry of the current IMPORTS entry.
 *
 *  The entry is the right-hand side of "NAME = MOD.EXT". It may be
 *  a name or an ordinal number.
 *
 *  @par Reference
 *  LINK386 Reference, "IMPORTS Statement": entry.
 *
 *  @param[in]  hImport  Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer, or NULL for a size query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryImportExternal(HDEFIMPORT hImport,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed);

/*! @brief Test whether the current IMPORTS entry used an explicit
 *         internalname.
 *
 *  @par Reference
 *  LINK386 Reference, "IMPORTS Statement": "If an ordinal value is
 *  given, then <internalname> is required."
 *
 *  @param[in]  hImport      Cursor. Not NULLHANDLE.
 *  @param[out] pfExplicit   Receives TRUE if the source line carried
 *                           an explicit assignment. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryImportInternalExplicit(HDEFIMPORT hImport,
                                               PBOOL pfExplicit);

/*! @brief Test whether the current IMPORTS entry's external reference
 *         is an ordinal value.
 *
 *  @par Reference
 *  LINK386 Reference, "IMPORTS Statement": "If an ordinal value is
 *  given, then <internalname> is required."
 *
 *  @param[in]  hImport     Cursor. Not NULLHANDLE.
 *  @param[out] pfOrdinal   Receives TRUE if entry is a numeric
 *                          ordinal. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryImportOrdinal(HDEFIMPORT hImport,
                                      PBOOL pfOrdinal);

/* ==================================================================
 * EXPORTS enumeration
 * ================================================================== */

/*! @brief Open a cursor on the first EXPORTS entry.
 *
 *  @param[in]  hDef      Handle. Not NULLHANDLE.
 *  @param[out] phExport  Receives the cursor. Not NULL. Set to
 *                        NULLHANDLE on error or when there are no
 *                        entries.
 *  @param[out] pulCount  Optional. May be NULL. On success receives
 *                        the total number of entries.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hDef or @p phExport is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      No entries. *phExport is
 *                                   NULLHANDLE.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see DefFindNextExport
 *  @see DefFindCloseExport
 */
APIRET APIENTRY DefFindFirstExport(HDEFFILE hDef, HDEFEXPORT *phExport,
                                   PULONG pulCount);

/*! @brief Advance an EXPORTS cursor to the next entry.
 *
 *  @param[in] hExport  Cursor. Not NULLHANDLE.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      No more entries.
 *
 *  @see DefFindFirstExport
 *  @see DefFindCloseExport
 */
APIRET APIENTRY DefFindNextExport(HDEFEXPORT hExport);

/*! @brief Close an EXPORTS cursor.
 *
 *  Passing NULLHANDLE is a no-op.
 *
 *  @param[in] hExport  Cursor. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                Success. Also for NULLHANDLE.
 *  @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 *  @see DefFindFirstExport
 */
APIRET APIENTRY DefFindCloseExport(HDEFEXPORT hExport);

/*! @brief Return the entryname of the current EXPORTS entry.
 *
 *  @par Reference
 *  LINK386 Reference, "EXPORTS Statement":
 *    entryname [=internalname] [@ord[RESIDENTNAME]] [pwords]
 *
 *  @param[in]  hExport  Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer, or NULL for a size query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryExportName(HDEFEXPORT hExport,
                                   PSZ pszBuf, ULONG ulSize,
                                   PULONG pulUsed);

/*! @brief Return the internalname of the current EXPORTS entry.
 *
 *  When the source line carried no "=internalname", the accessor
 *  returns ERROR_FILE_NOT_FOUND. The caller may then fall back to
 *  the entryname, which is the documented default.
 *
 *  @par Reference
 *  LINK386 Reference, "EXPORTS Statement": "By default, this name
 *  is the same as <entryname>."
 *
 *  @param[in]  hExport  Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer, or NULL for a size query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     No internalname was specified.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryExportInternal(HDEFEXPORT hExport,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed);

/*! @brief Return the ordinal of the current EXPORTS entry.
 *
 *  @par Reference
 *  LINK386 Reference, "EXPORTS Statement": the optional @ord gives
 *  the ordinal position of the function within the module definition
 *  table.
 *
 *  @param[in]  hExport     Cursor. Not NULLHANDLE.
 *  @param[out] pusOrdinal  Receives the ordinal. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     No @ord was specified.
 */
APIRET APIENTRY DefQueryExportOrdinal(HDEFEXPORT hExport,
                                      PUSHORT pusOrdinal);

/*! @brief Return the parameter word count of the current EXPORTS
 *         entry.
 *
 *  @par Reference
 *  LINK386 Reference, "EXPORTS Statement": pwords is the total size
 *  of the function's parameters in words. Required only for I/O
 *  privileged functions.
 *
 *  @param[in]  hExport    Cursor. Not NULLHANDLE.
 *  @param[out] pusPWords  Receives the word count. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     No pwords was specified.
 */
APIRET APIENTRY DefQueryExportPWords(HDEFEXPORT hExport,
                                     PUSHORT pusPWords);

/*! @brief Test whether the current EXPORTS entry had the
 *         RESIDENTNAME keyword.
 *
 *  @par Reference
 *  LINK386 Reference, "EXPORTS Statement": RESIDENTNAME is
 *  applicable only if @ord is used.
 *
 *  @param[in]  hExport     Cursor. Not NULLHANDLE.
 *  @param[out] pfResident  Receives TRUE if RESIDENTNAME was
 *                          present. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryExportResident(HDEFEXPORT hExport,
                                       PBOOL pfResident);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __DEF__ */
