/*!
 * @file rspinst_main.c
 *
 * @brief Command line front end for the OS/2 response file parser.
 *
 * @par References
 *   - IBM OS/2 Warp 4 Command Reference, "Command Names and
 *     Parameters" (a forward slash introduces a switch).
 *   - IBM Redbook SG24-2010-00, section 4.6.2, "The SEINST
 *     Command" (the /R switch takes the response file).
 *   - OpenWatcom C Library Reference, "getopt" (declared in
 *     <unistd.h>; for non-UNIX targets the alternate option
 *     character is '/').
 *
 * @par Argument syntax
 * Arguments follow the OS/2 command line convention: an option is
 * introduced by a forward slash ('/'). This is handled natively by
 * the getopt() implementation shipped with OpenWatcom for OS/2: for
 * non-UNIX targets, the library sets the alternate option character
 * to '/' rather than '-'. No transformation of argv is needed.
 *
 * @par Option summary
 *   /R <file>   Response file to parse. Required. Equivalent to
 *               the /R switch of SEINST (SG24-2010-00, 4.6.2).
 *   /D          Dump the parsed entries in nested form.
 *   /I          Dump the parsed entries in flat INI form.
 *   /V          Validate the parsed document.
 *   /Q          Quiet mode: suppress non-error diagnostics.
 *   /S          Strict mode: treat warnings as errors.
 *   /E <file>   Write diagnostics to a file.
 *   /h          Print this help and exit.
 *   /v          Print the version and exit.
 *
 * The switches /D, /I, /V, /Q, /S, /E, /h and /v are local
 * extensions of this tool; they follow the same OS/2 switch
 * convention as the /R switch of SEINST.
 *
 * All switches are case-sensitive.
 */

#include "rspinst.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ==================================================================
 * Usage and version
 * ================================================================== */

/*!
 * @brief Print the usage message to stderr.
 *
 * The option spellings shown to the user use the OS/2 '/' prefix,
 * matching the OS/2 Command Reference, "Command Names and
 * Parameters".
 */
static void rsp_main_usage(void)
{
    fprintf(stderr,
        "Usage: rspinst /R <file.rsp> [options]\n"
        "\n"
        "Options:\n"
        "  /R file     response file to parse (required)\n"
        "  /D          dump the parsed entries to stdout\n"
        "  /I          dump in flat INI form (A.B=value)\n"
        "  /V          validate\n"
        "  /Q          quiet (only errors)\n"
        "  /S          strict: a warning is treated as an error\n"
        "  /E file     write diagnostics to file instead of stderr\n"
        "  /h          show this help\n"
        "  /v          print the version and exit\n");
}

/*!
 * @brief Print the version string to stdout.
 *
 * @par Reference
 *   Version is provided by RSP_VERSION_STRING in rspinst.h.
 */
static void rsp_main_version(void)
{
    char   achBuf[128];
    ULONG  ulUsed = 0;

    if (RspQueryVersion(achBuf, sizeof(achBuf), &ulUsed) == NO_ERROR) {
        printf("%s\n", achBuf);
    }
}

/* ==================================================================
 * Diagnostic dump
 * ================================================================== */

/*!
 * @brief Print all records of a report to a stream.
 *
 * @param[in] hReport  Report handle.
 * @param[in] pStream  Destination stream. Not NULL.
 */
static void rsp_main_print_report(HRSPREPORT hReport, FILE *pStream)
{
    ULONG  ulCount = 0;
    ULONG  i;

    if (RspReportQueryCount(hReport, &ulCount) != NO_ERROR) {
        return;
    }
    for (i = 0; i < ulCount; i++) {
        RSPERR stErr;
        PCSZ   pszSeverity;

        memset(&stErr, 0, sizeof(stErr));
        if (RspReportQueryEntry(hReport, i, &stErr) != NO_ERROR) {
            continue;
        }
        switch (stErr.ulSeverity) {
        case RSP_SEV_ERROR:
            pszSeverity = "error";
            break;
        case RSP_SEV_WARNING:
            pszSeverity = "warning";
            break;
        default:
            pszSeverity = "info";
            break;
        }
        if (stErr.achFile[0] != '\0' && stErr.ulLine != 0) {
            fprintf(pStream, "%s: %s:%lu: %s\n",
                    pszSeverity, stErr.achFile, stErr.ulLine,
                    stErr.achDetail);
        } else if (stErr.achFile[0] != '\0') {
            fprintf(pStream, "%s: %s: %s\n",
                    pszSeverity, stErr.achFile, stErr.achDetail);
        } else {
            fprintf(pStream, "%s: %s\n",
                    pszSeverity, stErr.achDetail);
        }
    }
}

/* ==================================================================
 * Dump callback
 * ================================================================== */

/*!
 * @brief Callback used by RspDump to write text to a stream.
 *
 * @param[in] pszText   Text. Not NULL.
 * @param[in] ulLength  Length in bytes.
 * @param[in] pvUser    Stream pointer.
 *
 * @return APIRET.
 *
 * @retval NO_ERROR          Success.
 * @retval ERROR_WRITE_FAULT Write error.
 */
static APIRET APIENTRY rsp_main_write_cb(PCSZ pszText, ULONG ulLength,
                                         PVOID pvUser)
{
    FILE  *pStream = (FILE *)pvUser;
    size_t cbWritten;

    cbWritten = fwrite(pszText, 1, (size_t)ulLength, pStream);
    if (cbWritten != (size_t)ulLength) {
        return ERROR_WRITE_FAULT;
    }
    return NO_ERROR;
}

/* ==================================================================
 * Entry point
 * ================================================================== */

/*!
 * @brief Program entry point.
 *
 * The argument parser relies on the getopt() implementation of the
 * OpenWatcom C runtime for OS/2. For non-UNIX targets, that
 * implementation sets the alternate option character to '/', so
 * options such as /R and /V are recognised without any argv
 * transformation.
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
 * @return Process exit code.
 *
 * @retval 0  Success.
 * @retval 2  Usage error.
 * @retval 3  Parse error.
 * @retval 4  Validation error.
 */
int main(int argc, char *argv[])
{
    PCSZ     pszResponseFile = NULL;
    PCSZ     pszLogFile = NULL;
    BOOL     fDump = FALSE;
    BOOL     fDumpIni = FALSE;
    BOOL     fValidate = FALSE;
    BOOL     fQuiet = FALSE;
    BOOL     fStrict = FALSE;
    int      cOpt;
    int      iRc = 0;
    HRSPINST hDoc = NULLHANDLE;
    HRSPREPORT hReport = NULLHANDLE;
    FILE    *pLog = NULL;
    FILE    *pDiag = stderr;
    APIRET   rc;

    opterr = 0;
    while ((cOpt = getopt(argc, argv, "R:DIVQSE:hv")) != -1) {
        switch (cOpt) {
        case 'R':
            pszResponseFile = optarg;
            break;
        case 'D':
            fDump = TRUE;
            break;
        case 'I':
            fDumpIni = TRUE;
            break;
        case 'V':
            fValidate = TRUE;
            break;
        case 'Q':
            fQuiet = TRUE;
            break;
        case 'S':
            fStrict = TRUE;
            break;
        case 'E':
            pszLogFile = optarg;
            break;
        case 'h':
            rsp_main_usage();
            return 0;
        case 'v':
            rsp_main_version();
            return 0;
        case '?':
        default:
            fprintf(stderr, "invalid option: /%c\n",
                    (optopt != 0) ? optopt : '?');
            rsp_main_usage();
            return 2;
        }
    }

    if (optind < argc) {
        fprintf(stderr, "unexpected argument: %s\n", argv[optind]);
        rsp_main_usage();
        return 2;
    }

    if (pszResponseFile == NULL) {
        rsp_main_usage();
        return 2;
    }

    if (pszLogFile != NULL) {
        pLog = fopen(pszLogFile, "w");
        if (pLog == NULL) {
            fprintf(stderr, "cannot open log file: %s\n", pszLogFile);
            return 2;
        }
        pDiag = pLog;
    }

    rc = RspOpen(pszResponseFile, &hDoc);
    if (rc != NO_ERROR) {
        fprintf(pDiag, "parse failed: %s (rc=0x%04lX)\n",
                pszResponseFile, (unsigned long)rc);
        if (pLog != NULL) {
            fclose(pLog);
        }
        return 3;
    }

    if (fDump) {
        rc = RspDump(hDoc, FALSE, rsp_main_write_cb, stdout);
        if (rc != NO_ERROR) {
            fprintf(pDiag, "dump failed (rc=0x%04lX)\n",
                    (unsigned long)rc);
            iRc = 3;
        }
    }
    if (fDumpIni) {
        rc = RspDump(hDoc, TRUE, rsp_main_write_cb, stdout);
        if (rc != NO_ERROR) {
            fprintf(pDiag, "dump failed (rc=0x%04lX)\n",
                    (unsigned long)rc);
            iRc = 3;
        }
    }
    if (fValidate) {
        RSPVALIDATEOPTS stOpts;
        ULONG           ulErrors = 0;
        ULONG           ulWarnings = 0;

        (void)RspValidateDefaults(&stOpts);
        stOpts.fStrict = fStrict;

        rc = RspReportCreate(&hReport);
        if (rc != NO_ERROR) {
            fprintf(pDiag, "cannot create report (rc=0x%04lX)\n",
                    (unsigned long)rc);
            RspClose(hDoc);
            if (pLog != NULL) {
                fclose(pLog);
            }
            return 3;
        }

        rc = RspValidate(hDoc, &stOpts, hReport);
        if (!fQuiet) {
            rsp_main_print_report(hReport, pDiag);
        }
        (void)RspReportQueryErrorCount(hReport, &ulErrors);
        (void)RspReportQueryWarningCount(hReport, &ulWarnings);
        fprintf(pDiag, "validation: %lu error(s), %lu warning(s)\n",
                (unsigned long)ulErrors, (unsigned long)ulWarnings);
        if (rc == RSP_ERROR_VALIDATION_FAILED) {
            iRc = 4;
        } else if (rc != NO_ERROR) {
            fprintf(pDiag, "validation failed (rc=0x%04lX)\n",
                    (unsigned long)rc);
            iRc = 4;
        }
        RspReportFree(hReport);
    }

    RspClose(hDoc);
    if (pLog != NULL) {
        fclose(pLog);
    }
    return iRc;
}
