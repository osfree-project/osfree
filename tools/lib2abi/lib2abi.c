/*!
 * @file lib2abi.c
 *
 * @brief Generate a uni2h .abi file from an OMF import library.
 *
 * Command line tool. Reads an OMF .LIB file, walks every IMPDEF
 * record, and writes an .abi file containing one library block
 * (with a flat list of all imported names) followed by one module
 * block per DLL referenced from the library.
 *
 * Usage:
 * @verbatim
   lib2abi --lib=<path> --output=<path.abi>
   @endverbatim
 *
 * References:
 *   - uni2h v2.0 specification, section 3 ("The .abi file").
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os2types.h"
#include "os2err.h"
#include "lib.h"
#include "abi.h"

/*!
 * @def LIB2ABI_NAMEBUF
 * @brief Size of the local buffer used for module and entry names.
 */
#define LIB2ABI_NAMEBUF 256

/*!
 * @struct _IMP
 * @brief One imported symbol cached in memory.
 */
typedef struct _IMP {
    char   achModule[LIB2ABI_NAMEBUF];  /*!< Source DLL name. */
    char   achName[LIB2ABI_NAMEBUF];    /*!< Function name.   */
    USHORT usOrdinal;                   /*!< Ordinal.         */
} IMP;

/* ------------------------------------------------------------------ */
/* Diagnostics                                                         */
/* ------------------------------------------------------------------ */

/*!
 * @brief Print the usage message to stderr.
 *
 * @param[in] pszArgv0  Program name as passed in argv[0]. Not NULL.
 */
static void Lib2AbiUsage(PCSZ pszArgv0)
{
    fprintf(stderr,
            "Usage: %s --lib=<path> --output=<path.abi>\n"
            "\n"
            "Generate a uni2h .abi file from an OMF import library.\n"
            "\n"
            "Options:\n"
            "  --lib=<path>      Path to the .LIB file (required).\n"
            "  --output=<path>   Path to the output .abi file "
            "(required).\n"
            "  --help            Print this message and exit.\n",
            pszArgv0);
}

/*!
 * @brief Print an APIRET error to stderr.
 *
 * @param[in] pszArgv0   Program name. Not NULL.
 * @param[in] pszAction  Short description of the failed step.
 *                       Not NULL.
 * @param[in] pszPath    Path involved, or NULL.
 * @param[in] rc         Return code from the called function.
 */
static void Lib2AbiReport(PCSZ pszArgv0, PCSZ pszAction,
                          PCSZ pszPath, APIRET rc)
{
    if (pszPath != NULL)
        fprintf(stderr, "%s: %s %s (error %lu)\n",
                pszArgv0, pszAction, pszPath, (unsigned long)rc);
    else
        fprintf(stderr, "%s: %s (error %lu)\n",
                pszArgv0, pszAction, (unsigned long)rc);
}

/* ------------------------------------------------------------------ */
/* Argument parsing                                                    */
/* ------------------------------------------------------------------ */

/*!
 * @brief Extract the value of a --key=value argument.
 *
 * @param[in] pszArg  Argument to inspect. Not NULL.
 * @param[in] pszKey  Key including the leading "--". Not NULL.
 *
 * @return Pointer to the value inside @p pszArg, or NULL.
 *
 * @retval NULL  The argument does not match the key.
 */
static PCSZ Lib2AbiArgValue(PCSZ pszArg, PCSZ pszKey)
{
    size_t cbKey = strlen(pszKey);
    if (strncmp(pszArg, pszKey, cbKey) == 0 && pszArg[cbKey] == '=')
        return pszArg + cbKey + 1;
    return NULL;
}

/*!
 * @brief Derive the library name from a file path.
 *
 * Strips the directory part and a trailing ".LIB" (case
 * insensitive).
 *
 * @param[in]  pszPath  Path to the .LIB file. Not NULL.
 * @param[out] pszOut   Output buffer. Not NULL.
 * @param[in]  cbOut    Size of @p pszOut in bytes.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_BUFFER_OVERFLOW    Name does not fit.
 */
static APIRET Lib2AbiBaseName(PCSZ pszPath, PSZ pszOut, ULONG cbOut)
{
    PCSZ   pszBase = pszPath;
    PCSZ   p;
    size_t cb;

    for (p = pszPath; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\')
            pszBase = p + 1;
    }

    cb = strlen(pszBase);
    if (cb >= 4 &&
        pszBase[cb - 4] == '.' &&
        (pszBase[cb - 3] == 'l' || pszBase[cb - 3] == 'L') &&
        (pszBase[cb - 2] == 'i' || pszBase[cb - 2] == 'I') &&
        (pszBase[cb - 1] == 'b' || pszBase[cb - 1] == 'B')) {
        cb -= 4;
    }

    if (cb + 1 > cbOut)
        return ERROR_BUFFER_OVERFLOW;

    memcpy(pszOut, pszBase, cb);
    pszOut[cb] = '\0';
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

/*!
 * @brief Program entry point.
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
 * @return Process exit code.
 *
 * @retval 0  The .abi file was written, or --help was requested.
 * @retval 1  A command line or runtime error occurred.
 */
int main(int argc, char *argv[])
{
    PCSZ        pszLib  = NULL;
    PCSZ        pszOut  = NULL;
    HOMFLIB     hLib    = NULLHANDLE;
    HOMFLIBENUM hEnum   = NULLHANDLE;
    HABIDOC     hAbi    = NULLHANDLE;
    IMP        *paImp   = NULL;
    ULONG       cImp    = 0;
    ULONG       capImp  = 0;
    CHAR        achLib[LIB2ABI_NAMEBUF];
    APIRET      rc      = NO_ERROR;
    int         i;
    ULONG       j;

    for (i = 1; i < argc; i++) {
        PCSZ pszValue;

        if ((pszValue = Lib2AbiArgValue(argv[i], "--lib")) != NULL) {
            pszLib = pszValue;
        } else if ((pszValue = Lib2AbiArgValue(argv[i],
                                               "--output")) != NULL) {
            pszOut = pszValue;
        } else if (strcmp(argv[i], "--help") == 0) {
            Lib2AbiUsage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "%s: unknown argument: %s\n",
                    argv[0], argv[i]);
            Lib2AbiUsage(argv[0]);
            return 1;
        }
    }

    if (pszLib == NULL || pszOut == NULL) {
        Lib2AbiUsage(argv[0]);
        return 1;
    }

    rc = Lib2AbiBaseName(pszLib, achLib, sizeof(achLib));
    if (rc != NO_ERROR) {
        Lib2AbiReport(argv[0], "cannot derive library name",
                      pszLib, rc);
        return 1;
    }

    rc = LibOpen(pszLib, &hLib);
    if (rc != NO_ERROR) {
        Lib2AbiReport(argv[0], "cannot open", pszLib, rc);
        return 1;
    }

    rc = LibImportFindFirst(hLib, &hEnum, NULL);
    if (rc == NO_ERROR) {
        for (;;) {
            IMP stImp;

            memset(&stImp, 0, sizeof(stImp));
            LibImportGetName(hEnum, stImp.achName,
                             sizeof(stImp.achName), NULL);
            LibImportGetModule(hEnum, stImp.achModule,
                               sizeof(stImp.achModule), NULL);
            LibImportGetOrdinal(hEnum, &stImp.usOrdinal);

            if (cImp == capImp) {
                ULONG ulNew = (capImp == 0) ? 64 : capImp * 2;
                IMP *paTmp = (IMP *)realloc(paImp,
                                            ulNew * sizeof(IMP));
                if (paTmp == NULL) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                paImp = paTmp;
                capImp = ulNew;
            }
            paImp[cImp++] = stImp;

            rc = LibImportFindNext(hEnum);
            if (rc == ERROR_NO_MORE_ITEMS) {
                rc = NO_ERROR;
                break;
            }
            if (rc != NO_ERROR)
                break;
        }
        LibImportFindClose(hEnum);
    } else if (rc == ERROR_NO_MORE_ITEMS) {
        rc = NO_ERROR;
    }

    if (rc == NO_ERROR)
        rc = AbiCreateDoc(&hAbi);

    if (rc == NO_ERROR)
        rc = AbiBeginLibrary(hAbi, achLib);

    if (rc == NO_ERROR) {
        for (i = 0; (ULONG)i < cImp; i++) {
            BOOL fDup = FALSE;
            ABI_ENTRY stEntry;

            for (j = 0; j < (ULONG)i; j++) {
                if (strcmp(paImp[j].achName,
                           paImp[i].achName) == 0) {
                    fDup = TRUE;
                    break;
                }
            }
            if (fDup)
                continue;

            memset(&stEntry, 0, sizeof(stEntry));
            stEntry.pszName = paImp[i].achName;
            stEntry.pszConvention = "_System";

            rc = AbiAddEntry(hAbi, &stEntry);
            if (rc != NO_ERROR)
                break;
        }
    }

    if (rc == NO_ERROR) {
        for (i = 0; (ULONG)i < cImp; i++) {
            BOOL fDup = FALSE;

            for (j = 0; j < (ULONG)i; j++) {
                if (strcmp(paImp[j].achModule,
                           paImp[i].achModule) == 0) {
                    fDup = TRUE;
                    break;
                }
            }
            if (fDup)
                continue;

            rc = AbiBeginModule(hAbi, paImp[i].achModule);
            if (rc != NO_ERROR)
                break;

            for (j = 0; (ULONG)j < cImp; j++) {
                ABI_ENTRY stEntry;

                if (strcmp(paImp[j].achModule,
                           paImp[i].achModule) != 0)
                    continue;

                memset(&stEntry, 0, sizeof(stEntry));
                stEntry.pszName = paImp[j].achName;
                stEntry.ulOrdinal = paImp[j].usOrdinal;
                stEntry.pszConvention = "_System";

                rc = AbiAddEntry(hAbi, &stEntry);
                if (rc != NO_ERROR)
                    break;
            }
            if (rc != NO_ERROR)
                break;
        }
    }

    if (rc == NO_ERROR)
        rc = AbiWriteFile(hAbi, pszOut);

    if (rc != NO_ERROR)
        Lib2AbiReport(argv[0], "cannot generate", pszOut, rc);

    if (hAbi != NULLHANDLE)
        AbiClose(hAbi);
    if (paImp != NULL)
        free(paImp);
    LibClose(hLib);

    return (rc == NO_ERROR) ? 0 : 1;
}
