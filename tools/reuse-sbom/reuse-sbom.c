/* reuse-sbom.c - SBOM generator (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <reuse.h>
#include "sha1.h"
#include "ccl.h"
#include "spdx.h"
#include "spdx_db.h"
#include "git.h"

#include "spdx_sbom_types.h"
#include "spdx_sbom_utils.h"
#include "spdx_sbom_opts.h"
#include "spdx_sbom_scan.h"
#include "reuse_lic.h"
#include "spdx_sbom_doc.h"
#include "spdx_sbom_extracted.h"
#include "spdx_sbom_out.h"

#include "reuse_discover.h"

/**
 * @file reuse-sbom.c
 * @brief Command line entry point of the SBOM generator.
 *
 * Collects licensing information from the project tree (or from a
 * single binary artifact), builds an in-memory SBOM document, and
 * emits it in the requested format.
 *
 * Conforms to:
 *   - SPDX 2.3.
 *     https://spdx.github.io/spdx-spec/v2.3/
 *   - OS/2 Control Program Interface (naming, types, conventions).
 */

/* ==================================================================
 * Help text
 * ================================================================== */

/**
 * @brief Print command line usage to stdout.
 */
static void print_help(void) {
    printf("Usage: reuse-sbom [options] [<directory>]\n"
           "\n"
           "Required:\n"
           "  --name=<name>              Package name\n"
           "  --file=<binary file>       Binary artifact to describe\n"
           "  --spdx-db=<path>           SPDX database root "
           "(licenses.json,\n"
           "                             exceptions.json, details/, "
           "exceptions/)\n"
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
           "  --default-license=<id>     Fallback license identifier\n"
           "  --default-copyright=<text> Fallback copyright text\n"
           "  --source-sbom=<file>       Source SBOM for binary mode "
           "(with --purpose != SOURCE)\n"
           "  --objects=<list>           Object files (space-separated)\n"
           "  --res=<list>               Resource files (space-separated)\n"
           "  --extracted-license=<id>:<path>\n"
           "                             Provide text for a "
           "LicenseRef-* license\n"
           "  --cache=<path>             SPDX database cache file\n"
           "  --no-gitignore             Do not apply .gitignore rules\n"
           "  --help, -h                 Show this help\n");
}

/* ==================================================================
 * Helpers
 * ================================================================== */

/**
 * @brief Check whether the SBOM describes a binary artifact.
 *
 * Binary mode is enabled when a package purpose other than SOURCE
 * is requested. SOURCE means the SBOM is generated for the source
 * tree; any other value selects binary mode, where the SBOM is
 * built from a single binary file.
 *
 * @param[in] pOpts  Parsed options. Not NULL.
 *
 * @return TRUE_ for binary mode, FALSE_ otherwise.
 */
static BOOL is_binary_mode(const SBOMOPTIONS *pOpts) {
    return (pOpts->pszPackagePurpose &&
            strcmp(pOpts->pszPackagePurpose, "SOURCE") != 0)
               ? TRUE_ : FALSE_;
}

/**
 * @brief Resolve the license for the package as a whole.
 *
 * Order of resolution:
 *   1. --default-license on the command line.
 *   2. The first [[annotations]] entry with path "**" found in any
 *      REUSE.toml, scanned from the deepest (closest to the target
 *      directory) to the root.
 *
 * The resolved expression is validated against the SPDX grammar.
 *
 * @param[in]  pOpts       Parsed options. Not NULL.
 * @param[in]  hTree       Project handle. May be NULLHANDLE.
 * @param[out] ppszLicense Receiver. Not NULL. On success receives a
 *                         malloc'd string owned by the caller, or
 *                         @c pOpts->pszDefaultLicense itself (not to
 *                         be freed). On failure set to NULL.
 * @param[out] pfIsHeap    Receiver: TRUE_ if the returned pointer was
 *                         allocated and must be freed. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  A parameter is NULL.
 * @retval ERROR_FILE_NOT_FOUND     No license available.
 * @retval ERROR_INVALID_DATA       The license expression is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET resolve_package_license(const SBOMOPTIONS *pOpts,
                                      HREUSETREE hTree,
                                      PSZ *ppszLicense,
                                      PBOOL pfIsHeap) {
    PCSZ pszLic = pOpts->pszDefaultLicense;
    PSZ pszHeap = NULL;
    APIRET rc;
    PCSZ pszBad = NULL;

    *ppszLicense = NULL;
    *pfIsHeap = FALSE_;

    if (!pszLic && hTree != NULLHANDLE) {
        HREUSETREEFILE hFile = NULLHANDLE;
        rc = ReuseTreeResolveFile(hTree, "**", &hFile, NULL);
        if (rc == NO_ERROR && hFile != NULLHANDLE) {
            ULONG ulSize = 0;
            rc = ReuseTreeFileGetLicense(hFile, NULL, 0, &ulSize);
            if (rc == NO_ERROR && ulSize > 0) {
                PSZ pszBuf = (PSZ)malloc(ulSize);
                if (pszBuf) {
                    rc = ReuseTreeFileGetLicense(hFile, pszBuf, ulSize,
                                                 NULL);
                    if (rc == NO_ERROR && pszBuf[0] != '\0') {
                        pszHeap = pszBuf;
                        pszLic = pszHeap;
                    } else {
                        free(pszBuf);
                    }
                }
            }
            ReuseTreeFileClose(hFile);
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
    *pfIsHeap = (pszLic == pszHeap) ? TRUE_ : FALSE_;
    return NO_ERROR;
}

/* ==================================================================
 * Entry point
 * ================================================================== */

/**
 * @brief Entry point of the SBOM generator.
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
 * @return 0 on success, 1 on error.
 */
int main(int argc, char *argv[]) {
    SBOMOPTIONS opts;
    SPDXDOCUMENT doc;
    HREUSETREE hTree = NULLHANDLE;
    HSTRSET hPaths = NULLHANDLE;
    REUSEDISCOVEROPTIONS walk_opts;
    GITIGNORELIST gitignore_rules;
    BOOL fHasGitignore = FALSE_;
    PSZ pszRepoRoot = NULL;
    PSZ pszPkgLicense = NULL;
    BOOL fPkgLicenseIsHeap = FALSE_;
    BOOL fBinaryMode;
    CHAR achBaseNoExt[256];
    APIRET rc;
    APIRET rcDb;
    int nExit = 1;

    GitIgnoreListInit(&gitignore_rules);

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
                    "       Run 'reuse-sbom --help' for usage.\n",
                    opts.pszBadOption);
        else
            fprintf(stderr,
                    "ERROR: invalid command line.\n"
                    "       Run 'reuse-sbom --help' for usage.\n");
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

    /* Two-phase GitFindRepoRoot: first query the size, then value.
     * If there is no repository, the target directory is used as the
     * base for .gitignore lookups. */
    {
        ULONG ulSize = 0;
        if (GitFindRepoRoot(opts.pszDir, NULL, 0, &ulSize) == NO_ERROR &&
            ulSize > 0) {
            pszRepoRoot = (PSZ)malloc(ulSize);
            if (pszRepoRoot) {
                if (GitFindRepoRoot(opts.pszDir, pszRepoRoot, ulSize,
                                    NULL) != NO_ERROR) {
                    free(pszRepoRoot);
                    pszRepoRoot = NULL;
                }
            }
        }
    }

    rc = ReuseTreeOpen(opts.pszDir, &hTree);
    if (rc != NO_ERROR) {
        fprintf(stderr,
                "ERROR: cannot open REUSE project at %s\n"
                "       The directory is missing or unreadable.\n",
                opts.pszDir);
        goto cleanup;
    }
    {
        ULONG ulErrs = 0;
        if (ReuseTreeGetErrorCount(hTree, &ulErrs) == NO_ERROR &&
            ulErrs > 0) {
            fprintf(stderr,
                    "ERROR: %u REUSE.toml file(s) could not be parsed.\n"
                    "       SBOM cannot be generated reliably.\n",
                    (unsigned)ulErrs);
            goto cleanup;
        }
    }

    if (!opts.fNoGitignore) {
        if (GitCollectGitignores(pszRepoRoot, opts.pszDir,
                                 &gitignore_rules) == NO_ERROR &&
            gitignore_rules.ulCount > 0) {
            fHasGitignore = TRUE_;
        }
    }

    fBinaryMode = is_binary_mode(&opts);

    /* Resolve the package license BEFORE SbomCreateDocument so the
     * value is fixed inside the package structure. In binary mode
     * the same resolution path is used: REUSE.toml (path="**") >
     * --default-license. */
    rc = resolve_package_license(&opts, hTree, &pszPkgLicense,
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
        ReuseSetDiscoverOptionsDefault(&walk_opts);
        if (fHasGitignore) {
            walk_opts.fUseGitignore   = TRUE_;
            walk_opts.pszRepoRoot     = pszRepoRoot ? pszRepoRoot
                                                    : opts.pszDir;
            walk_opts.pGitignoreRules = &gitignore_rules;
        }
        rc = StrSetCreate(&hPaths);
        if (rc != NO_ERROR) {
            fprintf(stderr, "ERROR: out of memory\n");
            goto cleanup;
        }
        rc = ReuseDiscover(opts.pszDir,
                           opts.papszObjectFiles, opts.ulObjectCount,
                           opts.papszResFiles, opts.ulResCount,
                           &walk_opts, hPaths);
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: cannot walk tree: %s\n"
                    "       Check that the directory exists and is "
                    "readable.\n",
                    opts.pszDir);
            goto cleanup;
        }
        rc = SbomCollectFiles(hPaths, hTree,
                              opts.pszDefaultLicense,
                              opts.pszDefaultCopyright,
                              doc.hFiles,
                              doc.hSnippets);
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: failed to collect files for the SBOM "
                    "(%lu).\n",
                    (unsigned long)rc);
            goto cleanup;
        }
        StrSetDestroy(hPaths);
        hPaths = NULLHANDLE;
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

cleanup:
    SbomFreeDocument(&doc);
    if (hPaths != NULLHANDLE) StrSetDestroy(hPaths);
    if (fPkgLicenseIsHeap) free(pszPkgLicense);
    if (hTree != NULLHANDLE) ReuseTreeClose(hTree);
    GitIgnoreListFree(&gitignore_rules);
    free(pszRepoRoot);
    SpdxCloseDatabase();
    SbomFreeOptions(&opts);
    return nExit;
}
