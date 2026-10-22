/* spdx-sbom.c - SBOM generator (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <reuse.h>
#include "sha1.h"
#include "ccl.h"
#include "spdx.h"
#include "spdx_db.h"
#include "osadl.h"

#include "spdx_sbom_types.h"
#include "spdx_sbom_utils.h"
#include "spdx_sbom_opts.h"
#include "spdx_sbom_scan.h"
#include "reuse_lic.h"
#include "spdx_sbom_doc.h"
#include "spdx_sbom_extracted.h"
#include "spdx_sbom_out.h"

#include "reuse_discover.h"

/*!
 * @file spdx-sbom.c
 * @brief Command line entry point of the SBOM generator.
 *
 * Collects licensing information from the project tree (or from a
 * single binary artifact), builds an in-memory SBOM document, and
 * emits it in the requested format.
 *
 * In source mode, the CLI resolves licensing information through
 * the REUSE resolver and hands the result to the SPDX SBOM layer
 * as a list of SPDXFILEINPUT entries. The SBOM layer does not call
 * the REUSE resolver itself.
 *
 * When --compat-matrix is given, every resolved file is also
 * checked against the project license (--default-license) via the
 * OSADL matrix. The compatibility verdict is diagnostic output: it
 * is reported to stderr and counted, but it is not written into
 * the SBOM. SPDX 2.3 has no field for a tool verdict, and adding
 * one to comment / licenseComments would misrepresent SPDX's data
 * model.
 *
 * Conforms to:
 *   - SPDX 2.3.
 *     https://spdx.github.io/spdx-spec/v2.3/
 *   - ISO/IEC 5230:2020 §3.3.2 (when --compat-matrix is used).
 */

/* ==================================================================
 * Compatibility counters
 * ================================================================== */

/*!
 * @brief Number of files whose license was compatible with the
 *        project license (Same or Yes).
 */
static ULONG g_ulCompatCompatible = 0;
/*!
 * @brief Number of files whose license requires a dependency check.
 */
static ULONG g_ulCompatCheckDep   = 0;
/*!
 * @brief Number of files whose license is not compatible.
 */
static ULONG g_ulCompatNo         = 0;
/*!
 * @brief Number of files whose compatibility is unknown.
 */
static ULONG g_ulCompatUnknown    = 0;
/*!
 * @brief Number of files whose license is absent from the matrix.
 */
static ULONG g_ulCompatUndef      = 0;

/* ==================================================================
 * Help text
 * ================================================================== */

/*!
 * @brief Print command line usage to stdout.
 */
static void print_help(void) {
    printf("Usage: spdx-sbom [options] [<directory>]\n"
           "\n"
           "Required:\n"
           "  --name=<name>              Package name\n"
           "  --file=<binary file>       Binary artifact to describe\n"
           "  --spdx-db=<path>           SPDX database root "
           "(licenses.json,\n"
           "                             exceptions.json, details/, "
           "exceptions/)\n"
           "  --comments-config=<path>   comments.toml describing comment "
           "styles\n"
           "                             for in-file SPDX tags\n"
           "\n"
           "Optional:\n"
           "  --version=<ver>            Package version\n"
           "  --supplier=<name>          Package supplier\n"
           "  --creator=<name>           SBOM creator\n"
           "  --purpose=<purpose>        Package purpose "
           "(SOURCE, BINARY, LIBRARY, ...)\n"
           "  --output=<file>            Write SBOM to file "
           "(default: stdout)\n"
           "  --format=<fmt>             Output format:\n"
           "                               spdx-json  - SPDX 2.3 JSON "
           "(default)\n"
           "                               spdx-tag   - SPDX 2.3 tag-value\n"
           "  --default-license=<id>     Project license identifier; "
           "recommended\n"
           "                             default for files without own "
           "license\n"
           "  --default-copyright=<text> Project copyright text; "
           "recommended\n"
           "                             default for files without own "
           "copyright\n"
           "  --compat-matrix=<path>     OSADL matrix.json; enable license\n"
           "                             compatibility check against\n"
           "                             --default-license. The verdict\n"
           "                             is diagnostic output only and\n"
           "                             is not written into the SBOM\n"
           "  --compat-strict            Treat incompatible license as "
           "ERROR\n"
           "                             (default: WARNING)\n"
           "  --source-sbom=<file>       Source SBOM for binary mode "
           "(with --purpose != SOURCE)\n"
           "  --objects=<list>           Object files (space-separated)\n"
           "  --res=<list>               Resource files (space-separated)\n"
           "  --extracted-license=<id>:<path>\n"
           "                             Provide text for a "
           "LicenseRef-* license\n"
           "  --cache=<path>             SPDX database cache file\n"
           "  --help, -h                 Show this help\n");
}

/* ==================================================================
 * Helpers
 * ================================================================== */

/*!
 * @brief Check whether the SBOM describes a binary artifact.
 *
 * @param[in] pOpts  Parsed options. Not NULL.
 *
 * @return TRUE for binary mode, FALSE otherwise.
 */
static BOOL is_binary_mode(const SBOMOPTIONS *pOpts) {
    return (pOpts->pszPackagePurpose &&
            strcmp(pOpts->pszPackagePurpose, "SOURCE") != 0)
               ? TRUE : FALSE;
}

/*!
 * @brief Resolve the license for the package as a whole.
 *
 * @param[in]  pOpts       Parsed options. Not NULL.
 * @param[in]  hProject    Project handle. May be NULLHANDLE.
 * @param[out] ppszLicense Receiver. Not NULL.
 * @param[out] pfIsHeap    Receiver: TRUE if the pointer is malloc'd.
 *                         Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  A parameter is NULL.
 * @retval ERROR_FILE_NOT_FOUND     No license available.
 * @retval ERROR_INVALID_DATA       The license expression is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET resolve_package_license(const SBOMOPTIONS *pOpts,
                                      HREUSEPROJECT hProject,
                                      PSZ *ppszLicense,
                                      PBOOL pfIsHeap) {
    PCSZ pszLic = pOpts->pszDefaultLicense;
    PSZ pszHeap = NULL;
    APIRET rc;
    PCSZ pszBad = NULL;

    *ppszLicense = NULL;
    *pfIsHeap = FALSE;

    if (!pszLic && hProject != NULLHANDLE) {
        HREUSEPROJECTFILE hFile = NULLHANDLE;
        rc = ReuseOpenFile(hProject, "**", &hFile, NULL);
        if (rc == NO_ERROR && hFile != NULLHANDLE) {
            ULONG ulSize = 0;
            rc = ReuseQueryFileLicense(hFile, NULL, 0, &ulSize);
            if (rc == NO_ERROR && ulSize > 0) {
                PSZ pszBuf = (PSZ)malloc(ulSize);
                if (pszBuf) {
                    rc = ReuseQueryFileLicense(hFile, pszBuf, ulSize,
                                               NULL);
                    if (rc == NO_ERROR && pszBuf[0] != '\0') {
                        pszHeap = pszBuf;
                        pszLic = pszHeap;
                    } else {
                        free(pszBuf);
                    }
                }
            }
            ReuseCloseFile(hFile);
        }
    }
    if (!pszLic || !pszLic[0]) {
        free(pszHeap);
        return ERROR_FILE_NOT_FOUND;
    }

    rc = SpdxQueryExpression(pszLic, &pszBad);
    if (rc != NO_ERROR) {
        free(pszHeap);
        return ERROR_INVALID_DATA;
    }

    *ppszLicense = (PSZ)pszLic;
    *pfIsHeap = (pszLic == pszHeap) ? TRUE : FALSE;
    return NO_ERROR;
}

/*!
 * @brief Query the OSADL verdict for one file, count it, report
 *        to stderr.
 *
 * The verdict is diagnostic output of the tool. It is not stored
 * in any SPDX structure and it does not change the emitted SBOM.
 * On --compat-strict, an OSADL_COMPAT_NO verdict is reported as
 * ERROR; otherwise as WARNING.
 *
 * @param[in] hCompat            Matrix handle, or NULLHANDLE.
 * @param[in] fCompatStrict      TRUE if --compat-strict.
 * @param[in] pszFullPath        File path for diagnostics. Not NULL.
 * @param[in] pszFileLicense     File license expression. Not NULL.
 * @param[in] pszProjectLicense  Project license. Not NULL.
 */
static void check_compat(HOSADLMATRIX hCompat, BOOL fCompatStrict,
                         PCSZ pszFullPath, PCSZ pszFileLicense,
                         PCSZ pszProjectLicense) {
    OSADLCOMPAT compat;
    APIRET rc;

    if (hCompat == NULLHANDLE) return;
    if (!pszProjectLicense || !pszFileLicense || !pszFileLicense[0])
        return;

    rc = OsadlQueryPair(hCompat, pszProjectLicense, pszFileLicense,
                        &compat);
    if (rc != NO_ERROR) {
        fprintf(stderr,
                "WARNING: %s: OSADL compatibility check failed "
                "(code %lu). [not in spec]\n",
                pszFullPath, (unsigned long)rc);
        return;
    }

    switch (compat) {
    case OSADL_COMPAT_SAME:
    case OSADL_COMPAT_YES:
        g_ulCompatCompatible++;
        break;
    case OSADL_COMPAT_NO:
        g_ulCompatNo++;
        if (fCompatStrict) {
            fprintf(stderr,
                    "ERROR: %s: license '%s' is not compatible with "
                    "project license '%s'.\n"
                    "       [OSADL matrix; ISO/IEC 5230:2020 §3.3.2]\n",
                    pszFullPath, pszFileLicense, pszProjectLicense);
        } else {
            fprintf(stderr,
                    "WARNING: %s: license '%s' is not compatible with "
                    "project license '%s'.\n"
                    "         [OSADL matrix; ISO/IEC 5230:2020 §3.3.2]\n"
                    "         Pass --compat-strict to treat as ERROR.\n",
                    pszFullPath, pszFileLicense, pszProjectLicense);
        }
        break;
    case OSADL_COMPAT_CHECK_DEP:
        g_ulCompatCheckDep++;
        fprintf(stderr,
                "WARNING: %s: license '%s' requires a dependency check "
                "against project license '%s'.\n"
                "         [OSADL matrix; ISO/IEC 5230:2020 §3.3.2]\n",
                pszFullPath, pszFileLicense, pszProjectLicense);
        break;
    case OSADL_COMPAT_UNKNOWN:
        g_ulCompatUnknown++;
        fprintf(stderr,
                "INFO: %s: OSADL compatibility of '%s' with '%s' "
                "is unknown.\n"
                "      [OSADL matrix]\n",
                pszFullPath, pszFileLicense, pszProjectLicense);
        break;
    case OSADL_COMPAT_UNDEF:
        g_ulCompatUndef++;
        fprintf(stderr,
                "INFO: %s: license '%s' is not in the OSADL matrix.\n"
                "      [OSADL matrix]\n",
                pszFullPath, pszFileLicense);
        break;
    }
}

/*!
 * @brief Resolve licensing information for every discovered file.
 *
 * For each path in @p hPaths, ReuseResolveLicense is called and the
 * result is appended to @p hResolved as an SPDXFILEINPUT entry. A
 * file with no licensing information at all is reported to stderr
 * and skipped.
 *
 * When @p hCompat is not NULLHANDLE and the file's license did not
 * come from the project fallback, check_compat is called. Its
 * verdict is reported to stderr and counted; it is not stored
 * anywhere and does not affect the SBOM.
 *
 * @param[in]  hPaths           Set of file paths. Not NULLHANDLE.
 * @param[in]  hProject         Project handle. May be NULLHANDLE.
 * @param[in]  pszDefLic        Project license, or NULL.
 * @param[in]  pszDefCop        Project copyright, or NULL.
 * @param[in]  hCompat          OSADL matrix, or NULLHANDLE.
 * @param[in]  fCompatStrict    TRUE if --compat-strict.
 * @param[in]  hResolved        Destination list.
 * @param[out] pulSkipped       Receiver for skipped count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      A required parameter is NULL.
 * @retval ERROR_INVALID_HANDLE         A container is not recognized.
 * @retval REUSE_ERROR_NO_COMMENT_STYLE No [[map]] for a file.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Allocation failure.
 */
static APIRET resolve_all_files(HSTRSET hPaths,
                                HREUSEPROJECT hProject,
                                PCSZ pszDefLic,
                                PCSZ pszDefCop,
                                HOSADLMATRIX hCompat,
                                BOOL fCompatStrict,
                                HVECTOR hResolved,
                                PULONG pulSkipped) {
    HSTRSETENUM hEnum = NULLHANDLE;
    APIRET rc;

    *pulSkipped = 0;

    if (hPaths == NULLHANDLE || hResolved == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;

    if (StrSetEnumFirst(hPaths, &hEnum) != NO_ERROR)
        return NO_ERROR;

    do {
        CHAR achFull[1024];
        REUSELICENSEINFO lic;
        SPDXFILEINPUT input;

        if (StrSetEnumGet(hEnum, achFull, sizeof(achFull), NULL)
                != NO_ERROR)
            continue;

        rc = ReuseResolveLicense(hProject, achFull, pszDefLic, pszDefCop,
                                 &lic);
        if (rc == REUSE_ERROR_NO_COMMENT_STYLE) {
            fprintf(stderr,
                    "ERROR: %s: no [[map]] entry in comments.toml "
                    "matches this file.\n"
                    "       Add a [[map]] entry "
                    "(e.g. glob = \"*.<ext>\", style = ...).\n",
                    achFull);
            StrSetEnumClose(hEnum);
            return REUSE_ERROR_NO_COMMENT_STYLE;
        }
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "WARNING: %s: no licensing information, skipped.\n"
                    "         Fix one of:\n"
                    "           - add SPDX tags in the file header;\n"
                    "           - or create a sidecar <file>.license;\n"
                    "           - or add a [[annotations]] entry "
                    "in REUSE.toml;\n"
                    "           - or pass --default-license and "
                    "--default-copyright.\n",
                    achFull);
            (*pulSkipped)++;
            continue;
        }

        memset(&input, 0, sizeof(input));
        strncpy(input.achFullPath, achFull,
                sizeof(input.achFullPath) - 1);
        strncpy(input.achLicense, lic.achLicense,
                sizeof(input.achLicense) - 1);
        strncpy(input.achCopyright, lic.achCopyright,
                sizeof(input.achCopyright) - 1);

        if (hCompat != NULLHANDLE && !lic.fLicenseFromDefault) {
            check_compat(hCompat, fCompatStrict, achFull,
                         lic.achLicense, pszDefLic);
        }

        rc = SbomAddFileInput(hResolved, &input);
        if (rc != NO_ERROR) {
            StrSetEnumClose(hEnum);
            return rc;
        }
    } while (StrSetEnumNext(hEnum) == NO_ERROR);
    StrSetEnumClose(hEnum);

    return NO_ERROR;
}

/* ==================================================================
 * Entry point
 * ================================================================== */

/*!
 * @brief Entry point of the SBOM generator.
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
 * @retval 0 SBOM generated successfully.
 * @retval 1 Generation failed.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char *argv[]) {
    SBOMOPTIONS opts;
    SPDXDOCUMENT doc;
    HREUSEPROJECT hProject = NULLHANDLE;
    HREUSETAGSREPORT hTagsReport = NULLHANDLE;
    HSTRSET hPaths = NULLHANDLE;
    HVECTOR hResolved = NULLHANDLE;
    HOSADLMATRIX hCompat = NULLHANDLE;
    PSZ pszPkgLicense = NULL;
    BOOL fPkgLicenseIsHeap = FALSE;
    BOOL fBinaryMode;
    ULONG ulSkipped = 0;
    CHAR achBaseNoExt[256];
    APIRET rc;
    APIRET rcDb;
    int nExit = 1;

    rc = SbomParseCommandLine(argc, argv, &opts);
    if (rc != NO_ERROR) {
        if (opts.fHelpRequested) {
            print_help();
            SbomFreeOptions(&opts);
            return 0;
        }
        if (opts.pszBadOption)
            fprintf(stderr,
                    "ERROR: bad option: %s\n"
                    "       Run 'spdx-sbom --help' for usage.\n",
                    opts.pszBadOption);
        else
            fprintf(stderr,
                    "ERROR: invalid command line.\n"
                    "       Run 'spdx-sbom --help' for usage.\n");
        SbomFreeOptions(&opts);
        return 1;
    }

    rcDb = SpdxOpenDatabase(opts.pszSpdxDbRoot, opts.pszCacheFile);
    if (rcDb & SPDXDB_ERROR_LICENSES) {
        fprintf(stderr,
                "ERROR: SPDX license database is unavailable "
                "(licenses.json not loaded).\n"
                "       Expected at <spdx-db>/licenses.json.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        SbomFreeOptions(&opts);
        SpdxCloseDatabase();
        return 1;
    }
    if (rcDb & SPDXDB_ERROR_EXCEPTIONS) {
        fprintf(stderr,
                "ERROR: SPDX exceptions database is unavailable "
                "(exceptions.json not loaded).\n"
                "       Expected at <spdx-db>/exceptions.json.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        SbomFreeOptions(&opts);
        SpdxCloseDatabase();
        return 1;
    }
    if (rcDb & SPDXDB_ERROR_CACHE)
        fprintf(stderr,
                "WARNING: cache could not be written.\n"
                "         Next run will re-parse JSON indexes.\n");

    rc = ReuseOpenProject(opts.pszDir, opts.pszCommentsConfig, &hProject);
    if (rc == REUSE_ERROR_NO_COMMENT_STYLE) {
        fprintf(stderr,
                "ERROR: --comments-config=<path> is required.\n"
                "       It defines the comment styles used to read "
                "in-file SPDX tags.\n"
                "       Run 'spdx-sbom --help' for usage.\n");
        goto cleanup;
    }
    if (rc != NO_ERROR) {
        fprintf(stderr,
                "ERROR: cannot open REUSE project at %s\n"
                "       The directory is missing or unreadable.\n",
                opts.pszDir);
        goto cleanup;
    }
    {
        ULONG ulTotal = 0;
        ULONG ulErrs = 0;
        ULONG ulIdx;
        if (ReuseProjectGetRecordCount(hProject, &ulTotal) == NO_ERROR) {
            for (ulIdx = 0; ulIdx < ulTotal; ulIdx++) {
                REUSEERR err;
                if (ReuseProjectGetError(hProject, ulIdx, &err)
                        != NO_ERROR)
                    continue;
                if (err.ulSeverity == REUSE_SEV_ERROR)
                    ulErrs++;
            }
        }
        if (ulErrs > 0) {
            fprintf(stderr,
                    "ERROR: %u REUSE.toml file(s) could not be parsed.\n"
                    "       SBOM cannot be generated reliably.\n",
                    (unsigned)ulErrs);
            goto cleanup;
        }
    }

    if (ReuseQueryProjectTagsReport(hProject, &hTagsReport) != NO_ERROR)
        hTagsReport = NULLHANDLE;

    if (opts.pszCompatMatrix) {
        if (!opts.pszDefaultLicense) {
            fprintf(stderr,
                    "WARNING: --compat-matrix is set but "
                    "--default-license is not.\n"
                    "         Compatibility check will be skipped; "
                    "there is no\n"
                    "         outbound (project) license to compare "
                    "against.\n"
                    "         [ISO/IEC 5230:2020 §3.3.2]\n");
        } else {
            APIRET rcCompat = OsadlOpenMatrix(opts.pszCompatMatrix,
                                              &hCompat);
            if (rcCompat != NO_ERROR) {
                fprintf(stderr,
                        "ERROR: cannot open OSADL matrix at %s "
                        "(code %lu).\n"
                        "       Download matrix.json from\n"
                        "       https://www.osadl.org/fileadmin/"
                        "checklists/matrix.json\n",
                        opts.pszCompatMatrix,
                        (unsigned long)rcCompat);
                goto cleanup;
            }
            {
                BOOL fProjPresent = FALSE;
                if (OsadlQueryHasLicense(hCompat,
                                         opts.pszDefaultLicense,
                                         &fProjPresent) == NO_ERROR
                    && !fProjPresent) {
                    fprintf(stderr,
                            "WARNING: project license '%s' is not in "
                            "the OSADL matrix.\n"
                            "         Compatibility check will be "
                            "skipped.\n"
                            "         [OSADL matrix]\n",
                            opts.pszDefaultLicense);
                    OsadlCloseMatrix(hCompat);
                    hCompat = NULLHANDLE;
                }
            }
        }
    }

    fBinaryMode = is_binary_mode(&opts);

    rc = resolve_package_license(&opts, hProject, &pszPkgLicense,
                                 &fPkgLicenseIsHeap);
    if (rc != NO_ERROR) {
        fprintf(stderr,
                "ERROR: no license for package.\n"
                "       Fix one of:\n"
                "         - pass --default-license=<id>;\n"
                "         - or add a [[annotations]] entry with "
                "path = \"**\" to REUSE.toml.\n");
        goto cleanup;
    }

    rc = SbomCreateDocument(&doc,
                            opts.pszDocName,
                            opts.pszPackageVersion,
                            opts.pszPackageSupplier,
                            opts.pszCreator,
                            pszPkgLicense,
                            opts.pszDefaultCopyright,
                            opts.pszPackagePurpose,
                            opts.pszBinaryFile,
                            fBinaryMode);
    if (rc != NO_ERROR) {
        fprintf(stderr,
                "ERROR: cannot create SBOM document (%lu).\n",
                (unsigned long)rc);
        goto cleanup;
    }

    if (fBinaryMode) {
        SPDXFILEINFO info;
        memset(&info, 0, sizeof(info));
        rc = SbomFillFileBasic(opts.pszBinaryFile,
                               SpdxGetFileName(opts.pszBinaryFile),
                               &info);
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: cannot read binary artifact: %s\n",
                    opts.pszBinaryFile);
            goto cleanup;
        }
        strncpy(info.achFileType, "BINARY",
                sizeof(info.achFileType) - 1);
        strncpy(info.achLicense, pszPkgLicense,
                sizeof(info.achLicense) - 1);
        if (opts.pszDefaultCopyright) {
            strncpy(info.achCopyright, opts.pszDefaultCopyright,
                    sizeof(info.achCopyright) - 1);
        }
        rc = SbomAddFile(doc.hFiles, &info);
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: cannot append binary file entry.\n");
            goto cleanup;
        }
    } else {
        rc = StrSetCreate(&hPaths);
        if (rc != NO_ERROR) {
            fprintf(stderr, "ERROR: out of memory\n");
            goto cleanup;
        }
        {
            PCSZ *papszArtifacts = NULL;
            ULONG ulArtifactCount = 0;
            ULONG ulTotal = opts.ulObjectCount + opts.ulResCount;
            ULONG ulIdx;
            APIRET rcDiscover;

            if (ulTotal > 0) {
                papszArtifacts = (PCSZ*)malloc(
                    (size_t)ulTotal * sizeof(PCSZ));
                if (!papszArtifacts) {
                    fprintf(stderr, "ERROR: out of memory\n");
                    goto cleanup;
                }
                for (ulIdx = 0; ulIdx < opts.ulObjectCount; ulIdx++)
                    papszArtifacts[ulArtifactCount++] =
                        opts.papszObjectFiles[ulIdx];
                for (ulIdx = 0; ulIdx < opts.ulResCount; ulIdx++)
                    papszArtifacts[ulArtifactCount++] =
                        opts.papszResFiles[ulIdx];
            }

            rcDiscover = ReuseDiscoverFiles(opts.pszDir,
                                            papszArtifacts,
                                            ulArtifactCount,
                                            TRUE, hPaths);
            free(papszArtifacts);

            if (rcDiscover == ERROR_FILE_NOT_FOUND) {
                fprintf(stderr,
                        "ERROR: none of the artifacts yielded "
                        "source names.\n"
                        "       The artifacts may be corrupted, in an "
                        "unsupported format,\n"
                        "       or contain no source information.\n");
                goto cleanup;
            }
            if (rcDiscover != NO_ERROR) {
                fprintf(stderr,
                        "ERROR: cannot walk tree: %s\n"
                        "       Check that the directory exists and is "
                        "readable.\n",
                        opts.pszDir);
                goto cleanup;
            }
        }

        rc = SbomCreateFileInputList(&hResolved);
        if (rc != NO_ERROR) {
            fprintf(stderr, "ERROR: out of memory\n");
            goto cleanup;
        }
        rc = resolve_all_files(hPaths, hProject,
                               opts.pszDefaultLicense,
                               opts.pszDefaultCopyright,
                               hCompat, opts.fCompatStrict,
                               hResolved, &ulSkipped);
        if (rc == REUSE_ERROR_NO_COMMENT_STYLE) {
            goto cleanup;
        }
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: cannot resolve licensing information "
                    "(%lu).\n", (unsigned long)rc);
            goto cleanup;
        }
        if (ulSkipped > 0) {
            fprintf(stderr,
                    "WARNING: %lu file(s) skipped (no licensing "
                    "information).\n", (unsigned long)ulSkipped);
        }

        rc = SbomCollectFiles(hResolved, doc.hFiles, doc.hSnippets,
                              hProject, hTagsReport);
        if (rc == REUSE_ERROR_NO_COMMENT_STYLE) {
            goto cleanup;
        }
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: failed to collect files for the SBOM "
                    "(%lu).\n",
                    (unsigned long)rc);
            goto cleanup;
        }
        StrSetDestroy(hPaths);
        hPaths = NULLHANDLE;
        SbomFreeFileInputList(hResolved);
        hResolved = NULLHANDLE;
    }

    rc = SbomCollectExtractedLicenses(doc.hExtractedLicenses,
                                      doc.hFiles,
                                      opts.pszDir,
                                      opts.paExtractedSources,
                                      opts.ulExtractedCount);
    if (rc != NO_ERROR) {
        fprintf(stderr,
                "ERROR: missing text for a LicenseRef-* license.\n"
                "       Fix one of:\n"
                "         - create <project-root>/LICENSES/<id>.txt "
                "with the license text;\n"
                "         - or pass --extracted-license=<id>:<path>.\n");
        goto cleanup;
    }

    rc = SbomComputeVerification(&doc);
    if (rc != NO_ERROR) {
        fprintf(stderr,
                "ERROR: cannot compute PackageVerificationCode "
                "(%lu).\n",
                (unsigned long)rc);
        goto cleanup;
    }

    strncpy(achBaseNoExt, SpdxGetFileName(opts.pszBinaryFile),
            sizeof(achBaseNoExt) - 1);
    achBaseNoExt[sizeof(achBaseNoExt) - 1] = '\0';
    SbomRemoveExtension(achBaseNoExt);

    if (fBinaryMode && opts.pszSourceSbomPath) {
        CHAR achSha1Hex[41];
        rc = Sha1File(opts.pszSourceSbomPath, achSha1Hex,
                      sizeof(achSha1Hex), NULL);
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: cannot compute SHA1 for source SBOM: %s\n"
                    "       Check that the file exists and is "
                    "readable.\n",
                    opts.pszSourceSbomPath);
            goto cleanup;
        }
        rc = SbomSetDocumentExternalReference(&doc,
                                              opts.pszSourceSbomPath,
                                              achSha1Hex);
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: cannot set external document reference.\n");
            goto cleanup;
        }
    }

    rc = SbomBuildRelationships(&doc, achBaseNoExt, fBinaryMode);
    if (rc != NO_ERROR) {
        fprintf(stderr,
                "ERROR: cannot build SPDX relationships (%lu).\n",
                (unsigned long)rc);
        goto cleanup;
    }

    if (opts.pszOutput) {
        if (!freopen(opts.pszOutput, "w", stdout)) {
            fprintf(stderr,
                    "ERROR: cannot open output file: %s\n"
                    "       Check directory permissions.\n",
                    opts.pszOutput);
            goto cleanup;
        }
    }

    rc = SbomOutput(&doc, opts.pszFormat);
    if (rc != NO_ERROR) {
        fprintf(stderr,
                "ERROR: cannot emit SBOM (%lu).\n",
                (unsigned long)rc);
        goto cleanup;
    }

    nExit = 0;
    if (hCompat != NULLHANDLE && opts.fCompatStrict && g_ulCompatNo > 0)
        nExit = 1;

cleanup:
    SbomFreeDocument(&doc);
    if (hPaths != NULLHANDLE) StrSetDestroy(hPaths);
    if (hResolved != NULLHANDLE) SbomFreeFileInputList(hResolved);
    if (hCompat != NULLHANDLE) OsadlCloseMatrix(hCompat);
    if (fPkgLicenseIsHeap) free(pszPkgLicense);
    if (hProject != NULLHANDLE) ReuseCloseProject(hProject);
    SpdxCloseDatabase();
    SbomFreeOptions(&opts);
    return nExit;
}
