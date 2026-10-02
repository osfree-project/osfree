/*!
 * @file dll2abi.c
 *
 * @brief Generate a uni2h .abi file from an NE or LX DLL.
 *
 * Command line tool. Reads a DLL image in NE or LX format, walks
 * the export table, and writes a module block in the .abi syntax
 * described in section 3 of the uni2h v2.0 specification.
 *
 * Usage:
 * @verbatim
   dll2abi --dll=<path> --output=<path.abi>
   @endverbatim
 *
 * The tool links against the dll and abi libraries only; it does
 * not depend on newexe or lxexe directly.
 *
 * References:
 *   - uni2h v2.0 specification, section 3 ("The .abi file").
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os2types.h"
#include "os2err.h"
#include "dll.h"
#include "abi.h"

/*!
 * @def DLL2ABI_NAMEBUF
 * @brief Size of the local buffer used for entry and module names.
 */
#define DLL2ABI_NAMEBUF   256

/*!
 * @def DLL2ABI_CONVBUF
 * @brief Size of the local buffer used for convention strings.
 */
#define DLL2ABI_CONVBUF    64

/* ------------------------------------------------------------------ */
/* Diagnostics                                                         */
/* ------------------------------------------------------------------ */

/*!
 * @brief Print the usage message to stderr.
 *
 * @param[in] pszArgv0  Program name as passed in argv[0]. Not NULL.
 */
static void Dll2AbiUsage(PCSZ pszArgv0)
{
    fprintf(stderr,
            "Usage: %s --dll=<path> --output=<path.abi>\n"
            "\n"
            "Generate a uni2h .abi file from an NE or LX DLL.\n"
            "\n"
            "Options:\n"
            "  --dll=<path>      Path to the DLL file (required).\n"
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
static void Dll2AbiReport(PCSZ pszArgv0, PCSZ pszAction,
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
static PCSZ Dll2AbiArgValue(PCSZ pszArg, PCSZ pszKey)
{
    size_t cbKey = strlen(pszKey);
    if (strncmp(pszArg, pszKey, cbKey) == 0 && pszArg[cbKey] == '=')
        return pszArg + cbKey + 1;
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Export collection                                                   */
/* ------------------------------------------------------------------ */

/*!
 * @brief Copy one export of an open DLL into an ABI_ENTRY.
 *
 * @param[in]  hEnum    Cursor positioned on an export. Not
 *                      NULLHANDLE.
 * @param[out] pEntry   Receives the entry. Not NULL.
 * @param[out] pszName  Name buffer. Not NULL.
 * @param[in]  cbName   Size of @p pszName in bytes.
 * @param[out] pszConv  Convention buffer. Not NULL.
 * @param[in]  cbConv   Size of @p pszConv in bytes.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any pointer is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Name or convention buffer too
 *                                  small.
 * @retval ERROR_READ_FAULT         Underlying reader reported an
 *                                  error.
 */
static APIRET Dll2AbiFillEntry(HDLLEXPORT hEnum, PABI_ENTRY pEntry,
                               PSZ pszName, ULONG cbName,
                               PSZ pszConv, ULONG cbConv)
{
    USHORT usOrdinal = 0;
    BOOL   fVariable = FALSE;
    APIRET rc;

    if (hEnum == NULLHANDLE || pEntry == NULL ||
        pszName == NULL || pszConv == NULL)
        return ERROR_INVALID_PARAMETER;

    memset(pEntry, 0, sizeof(*pEntry));
    pszName[0] = '\0';
    pszConv[0] = '\0';

    rc = DllExportGetOrdinal(hEnum, &usOrdinal);
    if (rc != NO_ERROR)
        return rc;
    pEntry->ulOrdinal = usOrdinal;

    rc = DllExportGetName(hEnum, pszName, cbName, NULL);
    if (rc != NO_ERROR)
        return rc;
    pEntry->pszName = pszName;

    rc = DllExportIsGlobalData(hEnum, &fVariable);
    if (rc != NO_ERROR)
        return rc;
    pEntry->fVariable = fVariable;

    if (!fVariable) {
        rc = DllExportGetConvention(hEnum, pszConv, cbConv, NULL);
        if (rc != NO_ERROR)
            return rc;
        pEntry->pszConvention = pszConv;
    }

    return NO_ERROR;
}

/*!
 * @brief Walk all exports of a DLL and add them to an ABI document.
 *
 * @param[in] pszArgv0  Program name for diagnostics. Not NULL.
 * @param[in] hDll      Open DLL. Not NULLHANDLE.
 * @param[in] hAbi      ABI document. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 All exports added.
 * @retval ERROR_INVALID_PARAMETER  Any handle is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_READ_FAULT         Underlying reader reported an
 *                                  error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET Dll2AbiCollect(PCSZ pszArgv0, HDLL hDll, HABIDOC hAbi)
{
    HDLLEXPORT hEnum = NULLHANDLE;
    APIRET     rc;

    if (hDll == NULLHANDLE || hAbi == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;

    rc = DllExportFindFirst(hDll, &hEnum, NULL);
    if (rc == ERROR_NO_MORE_ITEMS)
        return NO_ERROR;
    if (rc != NO_ERROR)
        return rc;

    for (;;) {
        CHAR      achName[DLL2ABI_NAMEBUF];
        CHAR      achConv[DLL2ABI_CONVBUF];
        ABI_ENTRY stEntry;

        rc = Dll2AbiFillEntry(hEnum, &stEntry,
                              achName, sizeof(achName),
                              achConv, sizeof(achConv));
        if (rc != NO_ERROR) {
            Dll2AbiReport(pszArgv0, "cannot read export", NULL, rc);
            break;
        }

        rc = AbiAddEntry(hAbi, &stEntry);
        if (rc != NO_ERROR) {
            Dll2AbiReport(pszArgv0, "cannot add entry",
                          achName, rc);
            break;
        }

        rc = DllExportFindNext(hEnum);
        if (rc == ERROR_NO_MORE_ITEMS) {
            rc = NO_ERROR;
            break;
        }
        if (rc != NO_ERROR) {
            Dll2AbiReport(pszArgv0, "cannot advance cursor",
                          NULL, rc);
            break;
        }
    }

    DllExportFindClose(hEnum);
    return rc;
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
    PCSZ    pszDll   = NULL;
    PCSZ    pszOut   = NULL;
    HDLL    hDll     = NULLHANDLE;
    HABIDOC hAbi     = NULLHANDLE;
    CHAR    achModule[DLL2ABI_NAMEBUF];
    APIRET  rc;
    int     i;

    for (i = 1; i < argc; i++) {
        PCSZ pszValue;

        if ((pszValue = Dll2AbiArgValue(argv[i], "--dll")) != NULL) {
            pszDll = pszValue;
        } else if ((pszValue = Dll2AbiArgValue(argv[i],
                                               "--output")) != NULL) {
            pszOut = pszValue;
        } else if (strcmp(argv[i], "--help") == 0) {
            Dll2AbiUsage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "%s: unknown argument: %s\n",
                    argv[0], argv[i]);
            Dll2AbiUsage(argv[0]);
            return 1;
        }
    }

    if (pszDll == NULL || pszOut == NULL) {
        Dll2AbiUsage(argv[0]);
        return 1;
    }

    rc = DllOpen(pszDll, &hDll);
    if (rc != NO_ERROR) {
        Dll2AbiReport(argv[0], "cannot open", pszDll, rc);
        return 1;
    }

    rc = DllQuerySelfModuleName(hDll, achModule, sizeof(achModule));
    if (rc != NO_ERROR) {
        Dll2AbiReport(argv[0], "cannot read module name",
                      pszDll, rc);
        DllClose(hDll);
        return 1;
    }

    rc = AbiCreateDoc(&hAbi);
    if (rc != NO_ERROR) {
        Dll2AbiReport(argv[0], "cannot create ABI document",
                      NULL, rc);
        DllClose(hDll);
        return 1;
    }

    rc = AbiBeginModule(hAbi, achModule);
    if (rc != NO_ERROR) {
        Dll2AbiReport(argv[0], "cannot begin module block",
                      achModule, rc);
        AbiClose(hAbi);
        DllClose(hDll);
        return 1;
    }

    rc = Dll2AbiCollect(argv[0], hDll, hAbi);

    if (rc == NO_ERROR) {
        rc = AbiWriteFile(hAbi, pszOut);
        if (rc != NO_ERROR)
            Dll2AbiReport(argv[0], "cannot write", pszOut, rc);
    }

    AbiClose(hAbi);
    DllClose(hDll);

    return (rc == NO_ERROR) ? 0 : 1;
}
