/*!
 * @file lib.c
 *
 * @brief Implementation of the OMF library reader.
 *
 * OMF library (.LIB) reader (C89). See lib.h for the public API.
 *
 * All public functions declared in lib.h are implemented here. The
 * internal representation of HOMFLIB is defined in this translation
 * unit only; callers see it as an opaque HANDLE.
 *
 * References:
 *   - TIS Portable Formats Specification, Version 1.1 (Relocatable
 *     Object Module Format), Linux Foundation.
 *     https://refspecs.linuxfoundation.org/elf/elfspec.pdf
 *   - OpenWatcom WLIB librarian sources.
 *   - Microsoft OMF specification, "Relocatable Object Module
 *     Format", version 1.1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"
#include "omf.h"

/*!
 * @brief Maximum size of an OMF record processed by the reader.
 */
#define LIB_RECORD_MAX 512

/*!
 * @struct OMFLIB
 * @brief Internal representation behind HOMFLIB.
 *
 * Not exposed to callers. lib.h declares the handle as HANDLE, so
 * the layout of this structure may change freely.
 */
struct OMFLIB {
    FILE *fp;   /*!< Underlying file stream. */
};

/*!
 * @brief Inspect one COMENT payload as a possible IMPDEF record.
 *
 * An IMPDEF record maps a (module, ordinal) pair to a function
 * name. The payload layout recognized by this function is:
 * @verbatim
   [type:1][class:1][subtype:1][ord_flag:1][name_len:1]
   [name:name_len][mod_len:1][module:mod_len][ordinal:2]
   @endverbatim
 * The @c type byte must be OMF_COMENT_TYPE_NOECHO or
 * OMF_COMENT_TYPE_NORMAL, @c class must be
 * LIB_COMENT_CLASS_IMPDEF, @c subtype must be
 * LIB_IMPDEF_SUBTYPE_IMPORT, and @c ord_flag must be non-zero
 * (ordinal form).
 *
 * On a successful match the function name is copied into
 * @p pszName and @p pfMatched is set to 1. On a non-match
 * @p pfMatched is set to 0 and the function returns NO_ERROR.
 *
 * @param[in]  puchBuf    Record payload (after the OMF header).
 *                        Not NULL.
 * @param[in]  cbLen      Payload length in bytes.
 * @param[in]  pszModule  Module name to match. Not NULL.
 * @param[in]  usOrdinal  Ordinal to match.
 * @param[out] pszName    Output buffer for the function name.
 *                        Not NULL.
 * @param[in]  cbName     Size of @p pszName in bytes, including
 *                        space for the NUL terminator.
 * @param[out] pfMatched  Set to 1 on match, 0 otherwise. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Record processed (matched or
 *                                  not).
 * @retval ERROR_INVALID_PARAMETER  Match found but the name does
 *                                  not fit in @p pszName.
 */
static APIRET lib_check_impdef(const UCHAR *puchBuf, ULONG cbLen,
                               PCSZ pszModule, USHORT usOrdinal,
                               PSZ pszName, ULONG cbName,
                               int *pfMatched)
{
    ULONG  cbNameLen;
    ULONG  cbModOffset;
    ULONG  cbModLen;
    ULONG  cbOrdOffset;
    char   achMod[256];
    USHORT usOrd;

    *pfMatched = 0;

    /* IMPDEF layout:
     *   [type:1][class:1][subtype:1][ord_flag:1][name_len:1]
     *   [name:name_len][mod_len:1][module:mod_len][ordinal:2]
     */
    if (cbLen < 5) return NO_ERROR;
    if (puchBuf[0] != OMF_COMENT_TYPE_NOECHO &&
        puchBuf[0] != OMF_COMENT_TYPE_NORMAL) return NO_ERROR;
    if (puchBuf[1] != LIB_COMENT_CLASS_IMPDEF) return NO_ERROR;
    if (puchBuf[2] != LIB_IMPDEF_SUBTYPE_IMPORT) return NO_ERROR;
    if (puchBuf[3] == 0) return NO_ERROR;

    cbNameLen   = puchBuf[4];
    cbModOffset = 5 + cbNameLen;
    if (cbModOffset >= cbLen) return NO_ERROR;

    cbModLen    = puchBuf[cbModOffset];
    cbOrdOffset = cbModOffset + 1 + cbModLen;
    if (cbOrdOffset + 1 >= cbLen) return NO_ERROR;
    if (cbModLen >= sizeof(achMod)) return NO_ERROR;
    if (cbNameLen >= cbName) return ERROR_INVALID_PARAMETER;

    memcpy(achMod, &puchBuf[cbModOffset + 1], cbModLen);
    achMod[cbModLen] = '\0';

    usOrd = (USHORT)(puchBuf[cbOrdOffset] |
                     ((USHORT)puchBuf[cbOrdOffset + 1] << 8));

    if (usOrd != usOrdinal) return NO_ERROR;
    if (strcmp(achMod, pszModule) != 0) return NO_ERROR;

    /* Match. */
    memcpy(pszName, &puchBuf[5], cbNameLen);
    pszName[cbNameLen] = '\0';
    *pfMatched = 1;
    return NO_ERROR;
}

/*!
 * @brief Open an OMF library file.
 *
 * Opens @p pszPath for reading and returns a handle to be used
 * with the other LibQuery* functions. The handle owns the
 * underlying FILE stream and must be released with LibClose.
 *
 * @param[in]  pszPath  Path to the .LIB file. Not NULL.
 * @param[out] phLib    Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on failure.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phLib is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @see LibClose
 */
APIRET APIENTRY LibOpen(PCSZ pszPath, HOMFLIB *phLib)
{
    FILE *fp;
    struct OMFLIB *pLib;

    if (!pszPath || !phLib) return ERROR_INVALID_PARAMETER;
    *phLib = NULLHANDLE;

    fp = fopen(pszPath, "rb");
    if (!fp) return ERROR_OPEN_FAILED;

    pLib = (struct OMFLIB *)malloc(sizeof(*pLib));
    if (!pLib) {
        fclose(fp);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pLib->fp = fp;
    *phLib = (HOMFLIB)pLib;
    return NO_ERROR;
}

/*!
 * @brief Close an OMF library.
 *
 * Flushes and closes the underlying stream and releases the handle.
 * Idempotent: passing NULLHANDLE returns NO_ERROR.
 *
 * @param[in] hLib  Handle from LibOpen. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Always.
 *
 * @see LibOpen
 */
APIRET APIENTRY LibClose(HOMFLIB hLib)
{
    struct OMFLIB *pLib;

    if (hLib == NULLHANDLE) return NO_ERROR;
    pLib = (struct OMFLIB *)hLib;
    if (pLib->fp) fclose(pLib->fp);
    free(pLib);
    return NO_ERROR;
}

/*!
 * @brief Look up a function name by module and ordinal.
 *
 * Scans the library from the beginning and inspects every COMENT
 * record whose class identifies it as an IMPDEF record. The first
 * record whose module name and ordinal match @p pszModule and
 * @p usOrdinal provides the answer; the function name is copied
 * into @p pszName.
 *
 * @param[in]  hLib       Handle from LibOpen. Not NULLHANDLE.
 * @param[in]  pszModule  Module name to match. Not NULL.
 * @param[in]  usOrdinal  Ordinal to match.
 * @param[out] pszName    Output buffer. Not NULL.
 * @param[in]  cbName     Size of @p pszName in bytes, including
 *                        space for the NUL terminator.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 A matching record was found and
 *                                  the name was copied.
 * @retval ERROR_INVALID_PARAMETER  Bad handle, NULL argument, zero
 *                                  buffer size, or the name does
 *                                  not fit in @p pszName.
 * @retval ERROR_READ_FAULT         Read error or seek error.
 * @retval ERROR_FILE_NOT_FOUND     No matching record was found.
 *
 * @see LibOpen
 */
APIRET APIENTRY LibQueryFunction(HOMFLIB hLib, PCSZ pszModule,
                                 USHORT usOrdinal,
                                 PSZ pszName, ULONG cbName)
{
    struct OMFLIB *pLib;
    OMF_RECORD_HEADER head;
    UCHAR auchBuf[LIB_RECORD_MAX];
    long lPos;
    int fMatched;
    APIRET rc;

    if (hLib == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    if (!pszModule || !pszName || cbName == 0)
        return ERROR_INVALID_PARAMETER;

    pLib = (struct OMFLIB *)hLib;
    if (!pLib->fp) return ERROR_INVALID_PARAMETER;

    if (fseek(pLib->fp, 0, SEEK_SET) != 0)
        return ERROR_READ_FAULT;

    for (;;) {
        size_t cbRead;

        cbRead = fread(&head, 1, sizeof(head), pLib->fp);
        if (cbRead != sizeof(head)) break;

        if (head.usLength > sizeof(auchBuf)) {
            /* Record too long to process; skip past it. */
            if (fseek(pLib->fp, (long)head.usLength, SEEK_CUR) != 0)
                return ERROR_READ_FAULT;
            continue;
        }

        if (head.usLength > 0) {
            if (fread(auchBuf, 1, head.usLength, pLib->fp)
                != head.usLength)
                return ERROR_READ_FAULT;

            if (head.uchType == OMF_TYPE_COMENT) {
                rc = lib_check_impdef(auchBuf, head.usLength,
                                      pszModule, usOrdinal,
                                      pszName, cbName, &fMatched);
                if (rc != NO_ERROR) return rc;
                if (fMatched) return NO_ERROR;
            }
        }

        if (head.uchType == LIB_TYPE_TERMINATOR) break;

        if (head.uchType == OMF_TYPE_MODEND) {
            /* Each object module in a library is padded to the next
             * 16-byte boundary after MODEND. */
            lPos = ftell(pLib->fp);
            if (lPos < 0) return ERROR_READ_FAULT;
            if ((16 - lPos % 16) != 16) {
                if (fseek(pLib->fp, (long)(16 - lPos % 16),
                          SEEK_CUR) != 0)
                    return ERROR_READ_FAULT;
            }
        }
    }

    return ERROR_FILE_NOT_FOUND;
}
