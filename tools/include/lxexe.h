/*! lxexe.h - Linear eXecutable (LX) format
 *
 *  Provides the on-disk LX header structures, accessors, and the
 *  public API of the LX reader.
 */

#ifndef __LXEXE__
#define __LXEXE__

#include "os2types.h"
#include "os2err.h"
#include "mzexe.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file lxexe.h
 *  @brief Linear eXecutable (LX) format structures and access API.
 *
 *  Provides handle-based read access to LX executables. All handles
 *  (HLX, HLXENUM) are opaque; the internal representation is private
 *  to lxexe.c.
 *
 *  The MZ header that precedes the LX header is defined in mzexe.h
 *  and shared with the NE reader.
 *
 *  References:
 *    - IBM OS/2 Toolkit, "Linear Executable File Format".
 *    - OpenWatcom WLINK sources.
 */

/* ==================================================================
 * LX header constants
 * ================================================================== */

/**
 * @def LXMAGIC
 * @brief LX signature ("LX"). Value: 0x584C.
 */
#define LXMAGIC         0x584C

/* ==================================================================
 * LX module flags
 *
 * Values for the lx_mflags field. The low byte of the field
 * describes the module kind; the remaining bits carry module-level
 * attributes.
 * ================================================================== */

/**
 * @def LXF_MOD_PERPROC
 * @brief Module is per-process. Value: 0x00000001.
 */
#define LXF_MOD_PERPROC     0x00000001L

/**
 * @def LXF_MOD_EXE
 * @brief Module is an executable program. Value: 0x00000002.
 */
#define LXF_MOD_EXE         0x00000002L

/**
 * @def LXF_MOD_DLL
 * @brief Module is a dynamic link library. Value: 0x00000004.
 */
#define LXF_MOD_DLL         0x00000004L

/**
 * @def LXF_MOD_PROT
 * @brief Module runs in protected mode. Value: 0x00000008.
 */
#define LXF_MOD_PROT        0x00000008L

/**
 * @def LXF_MOD_EXT
 * @brief Module exposes the 32-bit API. Value: 0x00000010.
 */
#define LXF_MOD_EXT         0x00000010L

/* ==================================================================
 * LX entry bundle types
 *
 * The lx_enttab field points to a chain of bundles. Each bundle
 * starts with a count byte followed by a type byte; the value of
 * the type byte selects the on-disk layout of the entries in the
 * bundle.
 * ================================================================== */

/**
 * @def LXENT_EMPTY
 * @brief Empty bundle; the count ordinals are skipped.
 *        Value: 0x00.
 */
#define LXENT_EMPTY         0x00

/**
 * @def LXENT_16BIT
 * @brief 16-bit entry (6 bytes each). Value: 0x01.
 */
#define LXENT_16BIT         0x01

/**
 * @def LXENT_286CALLGATE
 * @brief 286 callgate entry (8 bytes each). Value: 0x02.
 */
#define LXENT_286CALLGATE   0x02

/**
 * @def LXENT_32BIT
 * @brief 32-bit entry (8 bytes each). Value: 0x03.
 */
#define LXENT_32BIT         0x03

/**
 * @def LXENT_FORWARDER
 * @brief Forwarder entry (8 bytes each). Value: 0x04.
 */
#define LXENT_FORWARDER     0x04

/**
 * @def LXENT_TYPEMASK
 * @brief Mask covering the bundle type bits. Value: 0x07.
 */
#define LXENT_TYPEMASK      0x07

/**
 * @def LXENT_PARAMTYPES
 * @brief Type byte flag: parameter type information is present.
 *        Value: 0x80.
 */
#define LXENT_PARAMTYPES    0x80

/* ==================================================================
 * LX entry flags
 *
 * Values for the FLAGS byte of an entry table entry.
 * ================================================================== */

/**
 * @def LXENT_EXPORTED
 * @brief Entry is exported. Value: 0x01.
 */
#define LXENT_EXPORTED      0x01

/**
 * @def LXENT_SINGLEDATA
 * @brief Entry uses single data rather than instance data.
 *        Value: 0x02.
 */
#define LXENT_SINGLEDATA    0x02

/* ==================================================================
 * LX forwarder flags
 * ================================================================== */

/**
 * @def LXFW_IMPORT_ORD
 * @brief Forwarder imports by ordinal rather than by name.
 *        Value: 0x01.
 */
#define LXFW_IMPORT_ORD     0x01

#pragma pack(push,1)

/*! @brief Linear Executable (LX) header (e32_exe).
 *
 *  Follows the MZ header at file offset E_LFANEW. The layout matches
 *  the on-disk LX header: a WORD magic followed by a BYTE border and
 *  a BYTE worder, then ULONG and USHORT fields as documented in the
 *  OS/2 Toolkit.
 */
struct lx_exe {
    WORD    lx_magic;         /*!< Signature word, LXMAGIC. */
    BYTE    lx_border;        /*!< Byte order: 0x00. */
    BYTE    lx_worder;        /*!< Word order: 0x00. */
    DWORD   lx_level;         /*!< File format level: 0. */
    WORD    lx_cpu;           /*!< CPU type: 0x02 for 386. */
    WORD    lx_os;            /*!< Target OS: 0x02 for OS/2. */
    DWORD   lx_ver;           /*!< Module version. */
    DWORD   lx_mflags;        /*!< Module flags (see LXF_*). */
    DWORD   lx_mpages;        /*!< Number of pages in the module. */
    DWORD   lx_startobj;      /*!< Object number of the entry point. */
    DWORD   lx_eip;           /*!< Entry point offset. */
    DWORD   lx_stackobj;      /*!< Object number of the stack. */
    DWORD   lx_esp;           /*!< Initial stack pointer. */
    DWORD   lx_pagesize;      /*!< Page size in bytes. */
    DWORD   lx_pageshift;     /*!< Page size as a shift count. */
    DWORD   lx_fixupsize;     /*!< Fixup section size in bytes. */
    DWORD   lx_fixupsum;      /*!< Fixup section checksum. */
    DWORD   lx_ldrsize;       /*!< Loader section size in bytes. */
    DWORD   lx_ldrsum;        /*!< Loader section checksum. */
    DWORD   lx_objtab;        /*!< Object table file offset. */
    DWORD   lx_objcnt;        /*!< Number of objects. */
    DWORD   lx_objmap;        /*!< Object page map file offset. */
    DWORD   lx_itermap;       /*!< Iterated data map file offset. */
    DWORD   lx_rsrctab;       /*!< Resource table file offset. */
    DWORD   lx_rsrccnt;       /*!< Number of resources. */
    DWORD   lx_restab;        /*!< Resident name table file offset. */
    DWORD   lx_enttab;        /*!< Entry table file offset. */
    DWORD   lx_dirtab;        /*!< Module directive table offset. */
    DWORD   lx_dircnt;        /*!< Number of module directives. */
    DWORD   lx_fpagetab;      /*!< Fixup page table file offset. */
    DWORD   lx_frectab;       /*!< Fixup record table file offset. */
    DWORD   lx_impmod;        /*!< Imported module name table
                               *   offset. */
    DWORD   lx_impmodcnt;     /*!< Number of imported modules. */
    DWORD   lx_impproc;       /*!< Imported procedure name table
                               *   offset. */
    DWORD   lx_pagesum;       /*!< Per-page checksum table offset. */
    DWORD   lx_datapage;      /*!< Data pages checksum table offset. */
    DWORD   lx_preload;       /*!< Number of preload pages. */
    DWORD   lx_nrestab;       /*!< Non-resident name table offset. */
    DWORD   lx_cbnrestab;     /*!< Non-resident name table size. */
    DWORD   lx_nressum;       /*!< Non-resident name table checksum. */
    DWORD   lx_autodata;      /*!< Auto data object number. */
    DWORD   lx_debuginfo;     /*!< Debug information offset. */
    DWORD   lx_debuglen;      /*!< Debug information length. */
    DWORD   lx_instpreload;   /*!< Number of instance pages
                               *   preloaded. */
    DWORD   lx_instdemand;    /*!< Number of instance pages loaded on
                               *   demand. */
    DWORD   lx_heapsize;      /*!< Heap size in bytes. */
    DWORD   lx_stacksize;     /*!< Stack size in bytes. */
};

/*! @brief LX object table entry (e32_object).
 *
 *  The object table describes the memory objects that make up the
 *  module. Each entry points to a virtual-address range within the
 *  linear address space and, when the object has file-backed
 *  contents, to the object page map that maps virtual pages onto
 *  file pages.
 */
struct lx_object {
    DWORD   o32_vsize;        /*!< Virtual size in bytes. */
    DWORD   o32_vbase;        /*!< Virtual base address. */
    DWORD   o32_flags;        /*!< Object flags. */
    DWORD   o32_pagemap;      /*!< Object page map index. */
    DWORD   o32_mapsize;      /*!< Number of entries in the object
                               *   page map. */
    DWORD   o32_reserved;     /*!< Reserved. */
};

/* ==================================================================
 * LX object flags
 * ================================================================== */

/**
 * @def LXOBJ_READ
 * @brief Object is readable. Value: 0x00000001.
 */
#define LXOBJ_READ          0x00000001L

/**
 * @def LXOBJ_WRITE
 * @brief Object is writable. Value: 0x00000002.
 */
#define LXOBJ_WRITE         0x00000002L

/**
 * @def LXOBJ_EXEC
 * @brief Object is executable. Value: 0x00000004.
 */
#define LXOBJ_EXEC          0x00000004L

/**
 * @def LXOBJ_DISCARDABLE
 * @brief Object is discardable. Value: 0x00000008.
 */
#define LXOBJ_DISCARDABLE   0x00000008L

/**
 * @def LXOBJ_SHARED
 * @brief Object is shared. Value: 0x00000010.
 */
#define LXOBJ_SHARED        0x00000010L

/**
 * @def LXOBJ_PRELOAD
 * @brief Object is preloaded. Value: 0x00000020.
 */
#define LXOBJ_PRELOAD       0x00000020L

/**
 * @def LXOBJ_INVALID
 * @brief Object is invalid. Value: 0x00000040.
 */
#define LXOBJ_INVALID       0x00000040L

/**
 * @def LXOBJ_ZEROINIT
 * @brief Object is zero-initialised; the page map is empty.
 *        Value: 0x00000080.
 */
#define LXOBJ_ZEROINIT      0x00000080L

/**
 * @def LXOBJ_ALIAS
 * @brief Object is an alias of another object.
 *        Value: 0x00000100.
 */
#define LXOBJ_ALIAS         0x00000100L

/**
 * @def LXOBJ_RESIDENT
 * @brief Object has resident backing.
 *        Value: 0x00000200.
 */
#define LXOBJ_RESIDENT      0x00000200L

/**
 * @def LXOBJ_BIG
 * @brief Object is a 32-bit object. Value: 0x00002000.
 */
#define LXOBJ_BIG           0x00002000L

/**
 * @def LXOBJ_CONFORM
 * @brief Object is conforming. Value: 0x00004000.
 */
#define LXOBJ_CONFORM       0x00004000L

/**
 * @def LXOBJ_IO
 * @brief Object has I/O privilege. Value: 0x00008000.
 */
#define LXOBJ_IO            0x00008000L

#pragma pack(pop)

/* ------------------------------------------------------------------ */
/* LX file access API                                                  */
/* ------------------------------------------------------------------ */

/*! @brief Handle to an open LX executable.
 *
 *  Opaque. Created by LxOpen and released by LxClose. The internal
 *  representation is private to lxexe.c.
 *
 *  @see LxOpen
 *  @see LxClose
 */
typedef HANDLE HLX;

/*! @brief Handle to an LX export enumeration cursor.
 *
 *  Opaque. Created by LxExportFindFirst, LxExportFindByName or
 *  LxExportFindByOrdinal; released by LxExportFindClose. The internal
 *  representation is private to lxexe.c.
 *
 *  @see LxExportFindFirst
 *  @see LxExportFindClose
 */
typedef HANDLE HLXENUM;

/*! @brief Open an LX executable file.
 *
 *  Opens @p pszPath, reads the MZ header and the LX header, and
 *  validates their magic numbers. The headers are cached in the
 *  handle and returned by LxQueryMZHeader and LxQueryHeader.
 *
 *  @param[in]  pszPath  Path to the executable. Not NULL.
 *  @param[out] phLx     Receives the handle. Not NULL. Set to
 *                       NULLHANDLE on failure.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phLx is NULL.
 *  @retval ERROR_OPEN_FAILED        File cannot be opened.
 *  @retval ERROR_READ_FAULT         Not a valid MZ/LX file.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see LxClose
 */
APIRET APIENTRY LxOpen(PCSZ pszPath, HLX *phLx);

/*! @brief Close an LX executable.
 *
 *  Closes the underlying file and releases the handle. This function
 *  is idempotent: passing NULLHANDLE returns NO_ERROR.
 *
 *  @param[in] hLx  Handle from LxOpen. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *  @retval NO_ERROR  Always.
 *
 *  @see LxOpen
 */
APIRET APIENTRY LxClose(HLX hLx);

/*! @brief Retrieve the cached MZ header.
 *
 *  Copies the MZ header that was read by LxOpen into the caller's
 *  buffer.
 *
 *  @param[in]  hLx  Handle. Not NULLHANDLE.
 *  @param[out] pMZ  Receives the header. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hLx or @p pMZ is invalid.
 *
 *  @see LxOpen
 */
APIRET APIENTRY LxQueryMZHeader(HLX hLx, struct exe_hdr *pMZ);

/*! @brief Retrieve the cached LX header.
 *
 *  Copies the LX header that was read by LxOpen into the caller's
 *  buffer.
 *
 *  @param[in]  hLx  Handle. Not NULLHANDLE.
 *  @param[out] pLX  Receives the header. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hLx or @p pLX is invalid.
 *
 *  @see LxOpen
 */
APIRET APIENTRY LxQueryHeader(HLX hLx, struct lx_exe *pLX);

/*! @brief Return the number of objects.
 *
 *  @param[in]  hLx       Handle. Not NULLHANDLE.
 *  @param[out] pulCount  Receives the count. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hLx or @p pulCount is invalid.
 */
APIRET APIENTRY LxQueryObjectCount(HLX hLx, PULONG pulCount);

/*! @brief Return one object table entry.
 *
 *  @param[in]  hLx      Handle. Not NULLHANDLE.
 *  @param[in]  ulIndex  Object index, 1-based.
 *  @param[out] pObj     Receives the entry. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad index or NULL pointer.
 *  @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY LxQueryObject(HLX hLx, ULONG ulIndex,
                              struct lx_object *pObj);

/*! @brief Return the module name of this LX image.
 *
 *  The module name is the first entry of the Resident Name Table,
 *  stored as a length-prefixed string without a terminating NUL.
 *  This function copies it into @p pszName as a NUL-terminated
 *  string.
 *
 *  @param[in]  hLx      Handle. Not NULLHANDLE.
 *  @param[out] pszName  Receives a NUL-terminated name. Not NULL.
 *  @param[in]  cbName   Size of @p pszName including the NUL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle or NULL pointer.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 *  @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY LxQuerySelfModuleName(HLX hLx, PSZ pszName, ULONG cbName);

/* ------------------------------------------------------------------ */
/* LX export enumeration API                                           */
/* ------------------------------------------------------------------ */

/*! @brief Open a cursor on the first export of an LX module.
 *
 *  The cursor walks the exports in increasing ordinal order. Bundles
 *  that contain no entries, and entries whose EXPORTED bit is clear,
 *  are skipped silently. Forwarder entries are included; use
 *  LxExportIsForwarder to tell them apart. Exports without a name
 *  are still reported; use LxExportIsNamed to tell them apart.
 *
 *  @param[in]  hLx       Handle. Not NULLHANDLE.
 *  @param[out] phEnum    Receives the cursor. Not NULL. Set to
 *                        NULLHANDLE on error or when the module has
 *                        no exports.
 *  @param[out] pulCount  Optional. May be NULL. On success receives
 *                        the total number of exports.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hLx or @p phEnum is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      Module has no exports.
 *                                   *phEnum = NULLHANDLE.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see LxExportFindNext
 *  @see LxExportFindClose
 */
APIRET APIENTRY LxExportFindFirst(HLX hLx, HLXENUM *phEnum,
                                  PULONG pulCount);

/*! @brief Advance an export cursor to the next export.
 *
 *  @param[in] hEnum  Cursor from LxExportFindFirst or one of the
 *                    find-by functions. Not NULLHANDLE.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      No more exports.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see LxExportFindFirst
 *  @see LxExportFindClose
 */
APIRET APIENTRY LxExportFindNext(HLXENUM hEnum);

/*! @brief Close an export cursor.
 *
 *  Passing NULLHANDLE is a no-op.
 *
 *  @param[in] hEnum  Cursor. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *  @retval NO_ERROR                Success. Also for NULLHANDLE.
 *  @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 *  @see LxExportFindFirst
 */
APIRET APIENTRY LxExportFindClose(HLXENUM hEnum);

/*! @brief Position a new cursor on an export by name.
 *
 *  Comparison is case-sensitive. If several entries share the same
 *  name (possible with alias entries), the one with the lowest
 *  ordinal is chosen.
 *
 *  @param[in]  hLx     Handle. Not NULLHANDLE.
 *  @param[in]  pszName Name to find. Not NULL.
 *  @param[out] phEnum  Receives the cursor. Not NULL. Set to
 *                      NULLHANDLE on error.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     Name not present.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see LxExportFindFirst
 *  @see LxExportFindClose
 */
APIRET APIENTRY LxExportFindByName(HLX hLx, PCSZ pszName,
                                   HLXENUM *phEnum);

/*! @brief Position a new cursor on an export by ordinal.
 *
 *  @param[in]  hLx        Handle. Not NULLHANDLE.
 *  @param[in]  usOrdinal  Ordinal to find (1-based).
 *  @param[out] phEnum     Receives the cursor. Not NULL. Set to
 *                         NULLHANDLE on error.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hLx or @p phEnum is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     Ordinal is not exported.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see LxExportFindFirst
 *  @see LxExportFindClose
 */
APIRET APIENTRY LxExportFindByOrdinal(HLX hLx, USHORT usOrdinal,
                                      HLXENUM *phEnum);

/*! @brief Retrieve the name of the current export.
 *
 *  Size-query convention:
 *    - pszBuf == NULL, ulSize == 0: only *pulUsed (size including
 *      NUL) is written, no buffer touched.
 *    - ulSize large enough: value copied and NUL-terminated; *pulUsed
 *      is the length without NUL.
 *    - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *      required size including NUL.
 *
 *  For an anonymous export (see LxExportIsNamed) a synthetic name of
 *  the form "Ordinal<N>" is returned, so the caller always receives a
 *  usable string. The real name, when present, is taken from the
 *  Resident Name Table first and from the Non-Resident Name Table
 *  otherwise.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hEnum is NULL, or @p pszBuf is
 *                                   NULL without size-query.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 *
 *  @see LxExportIsNamed
 */
APIRET APIENTRY LxExportGetName(HLXENUM hEnum,
                                PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/*! @brief Retrieve the ordinal of the current export.
 *
 *  @param[in]  hEnum       Cursor. Not NULLHANDLE.
 *  @param[out] pusOrdinal  Receives the ordinal (1-based). Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY LxExportGetOrdinal(HLXENUM hEnum, PUSHORT pusOrdinal);

/*! @brief Retrieve the raw entry flags of the current export.
 *
 *  The returned value is the FLAGS byte of the entry table record,
 *  without interpretation. Use LxExportIsGlobalData and
 *  LxExportIsForwarder for the corresponding predicates.
 *
 *  @param[in]  hEnum     Cursor. Not NULLHANDLE.
 *  @param[out] pulFlags  Receives the flags. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY LxExportGetFlags(HLXENUM hEnum, PULONG pulFlags);

/*! @brief Query whether the current export has a real name.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pfNamed  Receives TRUE if a name is present in the
 *                       Resident or Non-Resident Name Table. Not
 *                       NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *
 *  @see LxExportGetName
 */
APIRET APIENTRY LxExportIsNamed(HLXENUM hEnum, PBOOL pfNamed);

/*! @brief Query whether the current export is a variable.
 *
 *  Returns TRUE when the SINGLEDATA bit (LXENT_SINGLEDATA) is set in
 *  the entry flags.
 *
 *  @param[in]  hEnum         Cursor. Not NULLHANDLE.
 *  @param[out] pfGlobalData  Receives TRUE for a variable. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY LxExportIsGlobalData(HLXENUM hEnum, PBOOL pfGlobalData);

/*! @brief Query whether the current export is a forwarder.
 *
 *  Returns TRUE when the current entry was stored in a forwarder
 *  bundle (LXENT_FORWARDER).
 *
 *  @param[in]  hEnum        Cursor. Not NULLHANDLE.
 *  @param[out] pfForwarder  Receives TRUE for a forwarder. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY LxExportIsForwarder(HLXENUM hEnum, PBOOL pfForwarder);

/*! @brief Determine the calling convention of the current export.
 *
 *  Scans the first bytes of the function body for the first
 *  occurrence of one of the following instruction bytes:
 *
 *    - C3        ret        -> "_System"   (caller cleanup)
 *    - C2 xx xx  ret imm16  -> "_stdcall"  (callee cleanup)
 *    - CB        retf       -> "_System"   (far, caller cleanup)
 *    - CA xx xx  retf imm16 -> "_stdcall"  (far, callee cleanup)
 *
 *  Forwarder entries always yield "_System": the body of the target
 *  is not available in this module. If none of the instruction bytes
 *  is found within the object bounds, the default convention
 *  "_System" is returned. The choice is a heuristic: LX does not
 *  store the calling convention explicitly.
 *
 *  Size-query convention as for LxExportGetName.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hEnum is NULL, or @p pszBuf is
 *                                   NULL without size-query.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 *  @retval ERROR_READ_FAULT         Object body cannot be read.
 */
APIRET APIENTRY LxExportGetConvention(HLXENUM hEnum,
                                      PSZ pszBuf, ULONG ulSize,
                                      PULONG pulUsed);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __LXEXE__ */
