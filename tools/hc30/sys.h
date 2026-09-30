/*!
 * @file sys.h
 * @brief |SYSTEM internal file generator for WinHelp 3.0 (HC30).
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
 *  - [3] Winterhoff, M., helpdeco (1997), SYSTEMHEADER.
 *  - [4] Microsoft Help Workshop, "Help Project File Sections", 1992.
 *  - [5] ccl container library: vector.h.
 *
 * @par Thread safety
 * The module is single threaded.
 *
 * @par Ownership
 * HSYS is caller-owned and released with SysDestroy. Strings passed
 * to Set methods are copied.
 */
#ifndef SYS_H
#define SYS_H

#include "os2types.h"
#include "os2err.h"
#include <stdio.h>

/*!
 * @typedef HSYS
 * @brief Handle to a |SYSTEM generator. Caller-owned.
 */
typedef HANDLE HSYS;

/*!
 * @typedef PHSYS
 * @brief Pointer to a |SYSTEM generator handle.
 */
typedef HSYS  *PHSYS;

/*!
 * @def SYS_REC_TITLE
 * @brief Record type: help file title.
 */
#define SYS_REC_TITLE     0x0001

/*!
 * @def SYS_REC_COPYRIGHT
 * @brief Record type: copyright string.
 */
#define SYS_REC_COPYRIGHT 0x0002

/*!
 * @def SYS_REC_CONTENTS
 * @brief Record type: contents topic offset.
 */
#define SYS_REC_CONTENTS  0x0003

/*!
 * @def SYS_REC_CONFIG
 * @brief Record type: [CONFIG] macro line.
 */
#define SYS_REC_CONFIG    0x0004

/*!
 * @def SYS_REC_ICON
 * @brief Record type: icon bitmap.
 */
#define SYS_REC_ICON      0x0005

/*!
 * @def SYS_REC_WINDOW
 * @brief Record type: secondary window definition.
 */
#define SYS_REC_WINDOW    0x0006

/*!
 * @def SYS_REC_CITATION
 * @brief Record type: citation string.
 */
#define SYS_REC_CITATION  0x0008

/*!
 * @def SYS_REC_LCID
 * @brief Record type: language and code page.
 */
#define SYS_REC_LCID      0x0009

/*!
 * @def SYS_REC_CNT
 * @brief Record type: contents (.cnt) file name.
 */
#define SYS_REC_CNT       0x000A

/*!
 * @def SYS_REC_DEFFONT
 * @brief Record type: default font.
 */
#define SYS_REC_DEFFONT   0x000C

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create an empty |SYSTEM generator.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: @e Create implies that a new resource
 * is created as the result of a function call.
 *
 * @param[out] phSys Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phSys is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 *
 * @sa SysDestroy
 */
APIRET APIENTRY SysCreate(PHSYS phSys);

/*!
 * @brief Destroy a |SYSTEM generator and release all its data.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hSys Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR              Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE  Handle is not recognized.
 */
APIRET APIENTRY SysDestroy(HSYS hSys);

/* ==================================================================
 * Set
 * ================================================================== */

/*!
 * @brief Set the help file title.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: @e Set implies that a system value
 * is changed.
 *
 * The title is limited to 32 characters; longer strings are
 * truncated.
 *
 * @param[in] hSys     Handle. Not NULLHANDLE.
 * @param[in] pszTitle Title string. May be NULL to clear.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SysSetTitle(HSYS hSys, PCSZ pszTitle);

/*!
 * @brief Set the copyright string.
 *
 * Passing NULL clears the string.
 *
 * @param[in] hSys         Handle. Not NULLHANDLE.
 * @param[in] pszCopyright Copyright string. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysSetCopyright(HSYS hSys, PCSZ pszCopyright);

/*!
 * @brief Set the contents topic offset.
 *
 * Records a SYS_REC_CONTENTS entry with the given topic offset,
 * resolved from the CONTEXTS option by the caller.
 *
 * @param[in] hSys           Handle. Not NULLHANDLE.
 * @param[in] ulTopicOffset  Offset of the contents topic in |TOPIC.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SysSetContents(HSYS hSys, ULONG ulTopicOffset);

/*!
 * @brief Set the citation string.
 *
 * Passing NULL clears the string.
 *
 * @param[in] hSys        Handle. Not NULLHANDLE.
 * @param[in] pszCitation Citation string. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysSetCitation(HSYS hSys, PCSZ pszCitation);

/*!
 * @brief Set the language, sub-language, and code page.
 *
 * Emits a SYS_REC_LCID record with 12 bytes: code page, language,
 * sub-language, and six zero bytes.
 *
 * @param[in] hSys       Handle. Not NULLHANDLE.
 * @param[in] usLang     Primary language identifier.
 * @param[in] usSubLang  Sub-language identifier.
 * @param[in] usCodePage Code page identifier.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SysSetLcid(HSYS hSys, USHORT usLang, USHORT usSubLang, USHORT usCodePage);

/*!
 * @brief Set the .cnt file name.
 *
 * Passing NULL clears the string.
 *
 * @param[in] hSys   Handle. Not NULLHANDLE.
 * @param[in] pszCnt Contents file name. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysSetCnt(HSYS hSys, PCSZ pszCnt);

/*!
 * @brief Append a [CONFIG] macro line.
 *
 * @param[in] hSys      Handle. Not NULLHANDLE.
 * @param[in] pszConfig Macro line. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys or @a pszConfig is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysAddConfig(HSYS hSys, PCSZ pszConfig);

/*!
 * @brief Append a secondary window definition.
 *
 * The opaque window record is copied verbatim. The size must match
 * the format expected by HC30 for SYS_REC_WINDOW.
 *
 * @param[in] hSys     Handle. Not NULLHANDLE.
 * @param[in] pvWindow Pointer to the window record. Not NULL.
 * @param[in] ulSize   Size of the window record in bytes. Non-zero.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL or @a ulSize is 0.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SysAddWindow(HSYS hSys, PCVOID pvWindow, ULONG ulSize);

/* ==================================================================
 * Writing
 * ================================================================== */

/*!
 * @brief Serialize the |SYSTEM file to a stream.
 *
 * Writes SYSTEMHEADER (Minor=15, Major=1, HC30) followed by the
 * records in canonical order: TITLE, COPYRIGHT, CONTENTS, CONFIG,
 * WINDOW, CITATION, LCID, CNT.
 *
 * @param[in] hSys Handle. Not NULLHANDLE.
 * @param[in] f    Output stream. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hSys or @a f is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SysWrite(HSYS hSys, FILE* f);

#endif /* SYS_H */
