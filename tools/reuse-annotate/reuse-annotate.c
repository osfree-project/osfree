/* reuse-annotate.c - SPDX tag annotation tool (C89) */

/*!
 * @file reuse-annotate.c
 * @brief SPDX tag annotation tool.
 *
 * Walks the project tree, resolves license and copyright for each
 * file (REUSE.toml, sidecar, in-file tags), and writes the missing
 * SPDX headers. Sidecar files are created for binary files. The
 * LICENSES/ directory is populated with the license texts declared
 * by the project.
 *
 * Comment styles and comment-marker construction come from the
 * attached comments.toml through the reuse_comments API. The
 * --comment-style CLI option is parsed but not yet applied (TODO);
 * a warning is printed when it is present.
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 *   - SPDX 2.3, Annex D (license expression grammar).
 *     https://spdx.github.io/spdx-spec/v2.3/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#ifdef __LINUX__
#include <unistd.h>
#else
#include <direct.h>
#include <io.h>
#endif
#include <reuse.h>
#include "ccl.h"
#include "spdx.h"
#include "spdx_db.h"
#include "reuse_discover.h"
#include "reuse_licenses.h"
#include "git.h"
#include "path.h"
#include "reuse_lic.h"

/*!
 * @def MAX_LINE
 * @brief Maximum length in bytes of a single input line.
 */
#define MAX_LINE     4096

/*!
 * @def BINARY_PROBE
 * @brief Number of leading bytes inspected to decide whether a file is binary.
 */
#define BINARY_PROBE 8192

/*!
 * @def PATH_BUF
 * @brief Maximum length in bytes of a path buffer used by the tool.
 */
#define PATH_BUF     1024

/* ------------------------------------------------------------------ */
/* Legacy --comment-style parsing (kept for compatibility, see TODO)   */
/* ------------------------------------------------------------------ */

/*!
 * @enum _COMMENTSTYLE
 * @brief Comment style names accepted by --comment-style.
 *
 * The enum itself is kept only for parsing --comment-style so that
 * the option still accepts and validates the same names as before.
 * It is not used when writing: comment markers come from
 * comments.toml via ReuseQueryCommentBlock.
 */
typedef enum _COMMENTSTYLE {
    STYLE_C,          /**< C-style comment.                  */
    STYLE_HASH,       /**< Hash comment.                     */
    STYLE_REM,        /**< REM or @REM comment.              */
    STYLE_SIDECAR,    /**< Sidecar <file>.license.           */
    STYLE_UNKNOWN     /**< Unrecognized file type.           */
} COMMENTSTYLE;

/*!
 * @struct _STYLEOVERRIDE
 * @brief One --comment-style override entry.
 *
 * Parsed and stored, but not yet applied.
 */
typedef struct _STYLEOVERRIDE {
    PSZ           pszExt;      /**< Extension or base name.       */
    COMMENTSTYLE  style;       /**< Style to apply.               */
} STYLEOVERRIDE, *PSTYLEOVERRIDE;

/* ------------------------------------------------------------------ */
/* Global state                                                        */
/* ------------------------------------------------------------------ */

/*!
 * @brief Array of user-supplied comment style overrides.
 *
 * Populated from --comment-style, not yet used (TODO).
 */
static PSTYLEOVERRIDE g_paStyleOverrides = NULL;

/*!
 * @brief Number of entries in @c g_paStyleOverrides.
 */
static ULONG          g_ulStyleOverrideCount = 0;

/* ------------------------------------------------------------------ */
/* Heap helpers                                                        */
/* ------------------------------------------------------------------ */

/*!
 * @brief Read a whole file into a heap string.
 *
 * Uses the size-query convention of SpdxReadFileAll.
 *
 * @param[in] pszPath  Path. Not NULL.
 *
 * @retval NULL Read error, empty file, or allocation failure.
 * @return malloc'd NUL-terminated content.
 */
static PSZ read_file_to_heap(PCSZ pszPath) {
    ULONG ulSize = 0;
    PSZ pszOut;
    if (SpdxReadFileAll(pszPath, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszOut = (PSZ)malloc(ulSize);
    if (!pszOut) return NULL;
    if (SpdxReadFileAll(pszPath, pszOut, ulSize, NULL) != NO_ERROR) {
        free(pszOut);
        return NULL;
    }
    return pszOut;
}

/*!
 * @brief Normalize text into a heap string.
 *
 * Uses the size-query convention of SpdxNormalizeText.
 *
 * @param[in] pszSrc  Source text. Not NULL.
 *
 * @retval NULL Read error, empty result, or allocation failure.
 * @return malloc'd normalized text.
 */
static PSZ normalize_to_heap(PCSZ pszSrc) {
    ULONG ulSize = 0;
    PSZ pszOut;
    if (SpdxNormalizeText(pszSrc, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszOut = (PSZ)malloc(ulSize);
    if (!pszOut) return NULL;
    if (SpdxNormalizeText(pszSrc, pszOut, ulSize, NULL) != NO_ERROR) {
        free(pszOut);
        return NULL;
    }
    return pszOut;
}

/* ------------------------------------------------------------------ */
/* Filesystem helpers                                                  */
/* ------------------------------------------------------------------ */

/*!
 * @brief Query whether a path exists.
 *
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return TRUE if the path exists, FALSE otherwise.
 */
static BOOL FileExists(PCSZ pszPath) {
#ifdef __LINUX__
    return (access(pszPath, F_OK) == 0) ? TRUE : FALSE;
#else
    return (_access(pszPath, 0) == 0) ? TRUE : FALSE;
#endif
}

/*!
 * @brief Query whether a file contains a NUL byte in its first
 *        BINARY_PROBE bytes.
 *
 * @param[in] pszPath  Path to the file. Not NULL.
 *
 * @retval TRUE  File is binary (contains a NUL byte in the probe).
 * @retval FALSE File is text, empty, or cannot be read.
 * @return TRUE if binary, FALSE if text or unreadable.
 */
static BOOL IsBinaryFile(PCSZ pszPath) {
    FILE *fp = fopen(pszPath, "rb");
    UCHAR auchBuf[BINARY_PROBE];
    size_t cbRead, i;

    if (!fp) return FALSE;
    cbRead = fread(auchBuf, 1, sizeof(auchBuf), fp);
    fclose(fp);

    for (i = 0; i < cbRead; i++)
        if (auchBuf[i] == 0) return TRUE;
    return FALSE;
}

/* ------------------------------------------------------------------ */
/* Legacy --comment-style parsing                                      */
/* ------------------------------------------------------------------ */

/*!
 * @brief Translate a --comment-style value into a COMMENTSTYLE.
 *
 * @param[in] pszName  Style name. Not NULL.
 *
 * @retval STYLE_C       "c" or "slash" was passed.
 * @retval STYLE_HASH    "hash" was passed.
 * @retval STYLE_REM     "rem" or "cmd" was passed.
 * @retval STYLE_SIDECAR "binary" or "sidecar" was passed.
 * @retval STYLE_UNKNOWN Unrecognized style name.
 * @return The matching COMMENTSTYLE.
 */
static COMMENTSTYLE ParseStyleName(PCSZ pszName) {
    if (strcmp(pszName, "c") == 0 || strcmp(pszName, "slash") == 0)
        return STYLE_C;
    if (strcmp(pszName, "hash") == 0)
        return STYLE_HASH;
    if (strcmp(pszName, "rem") == 0 || strcmp(pszName, "cmd") == 0)
        return STYLE_REM;
    if (strcmp(pszName, "binary") == 0 ||
        strcmp(pszName, "sidecar") == 0)
        return STYLE_SIDECAR;
    return STYLE_UNKNOWN;
}

/*!
 * @brief Append one --comment-style override.
 *
 * On error prints a diagnostic and exits with EXIT_FAILURE.
 * The override is stored but not applied (see TODO).
 *
 * @param[in] pszArg  Value in the form "<ext>=<style>". Not NULL.
 */
static void AddStyleOverride(PCSZ pszArg) {
    PCSZ pszEq = strchr(pszArg, '=');
    PSZ pszExt;
    PCSZ pszName;
    COMMENTSTYLE style;
    size_t cbLen;

    if (!pszEq) {
        printf("ERROR: invalid --comment-style: %s\n"
               "       Expected format: "
               "--comment-style=<ext-or-name>=<style>\n"
               "       Example: --comment-style=.rb=hash\n",
               pszArg);
        exit(EXIT_FAILURE);
    }
    cbLen = (size_t)(pszEq - pszArg);
    pszExt = (PSZ)malloc(cbLen + 1);
    if (!pszExt) { printf("ERROR: out of memory\n"); exit(EXIT_FAILURE); }
    memcpy(pszExt, pszArg, cbLen);
    pszExt[cbLen] = '\0';
    pszName = pszEq + 1;
    style = ParseStyleName(pszName);
    if (style == STYLE_UNKNOWN) {
        printf("ERROR: unknown comment style: %s\n"
               "       Supported: slash (or c), hash, rem (or cmd), "
               "binary (or sidecar)\n", pszName);
        free(pszExt);
        exit(EXIT_FAILURE);
    }
    g_paStyleOverrides = (PSTYLEOVERRIDE)realloc(g_paStyleOverrides,
        (size_t)(g_ulStyleOverrideCount + 1) * sizeof(STYLEOVERRIDE));
    if (!g_paStyleOverrides) {
        printf("ERROR: out of memory\n"); exit(EXIT_FAILURE);
    }
    g_paStyleOverrides[g_ulStyleOverrideCount].pszExt = pszExt;
    g_paStyleOverrides[g_ulStyleOverrideCount].style = style;
    g_ulStyleOverrideCount++;
}

/* ------------------------------------------------------------------ */
/* SPDX header body construction                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Build the SPDX tag-value body for a file header.
 *
 * Produces one "SPDX-FileCopyrightText: <line>" per copyright line
 * (joined with '\n'), followed by one "SPDX-License-Identifier:
 * <license>" line. Each line ends with '\n'. Comment markers are not
 * added here; that is the caller's job (ReuseQueryCommentBlock for
 * in-file headers, raw text for sidecars).
 *
 * @param[in] pszLicense    License expression, or NULL.
 * @param[in] pszCopyright  Copyright text, or NULL.
 *
 * @retval NULL  Allocation failure.
 * @return       malloc'd body, owned by the caller.
 */
static PSZ build_spdx_body(PCSZ pszLicense, PCSZ pszCopyright) {
    static const char pszCopTag[] = "SPDX-FileCopyrightText: ";
    static const char pszLicTag[] = "SPDX-License-Identifier: ";
    size_t cbCap;
    PSZ pszBuf;
    size_t cbLen = 0;
    PCSZ pszPos;

    cbCap = 64;
    if (pszCopyright) {
        size_t cbCopLen = strlen(pszCopyright);
        size_t cbLineCount = 1;
        size_t i;
        for (i = 0; i < cbCopLen; i++)
            if (pszCopyright[i] == '\n') cbLineCount++;
        cbCap += cbCopLen + cbLineCount * (sizeof(pszCopTag) + 4);
    }
    if (pszLicense)
        cbCap += strlen(pszLicense) + sizeof(pszLicTag) + 4;

    pszBuf = (PSZ)malloc(cbCap);
    if (!pszBuf) return NULL;
    pszBuf[0] = '\0';

    if (pszCopyright) {
        pszPos = pszCopyright;
        while (*pszPos) {
            PCSZ pszEol = strchr(pszPos, '\n');
            size_t cbLine = pszEol ? (size_t)(pszEol - pszPos)
                                   : strlen(pszPos);
            memcpy(pszBuf + cbLen, pszCopTag, sizeof(pszCopTag) - 1);
            cbLen += sizeof(pszCopTag) - 1;
            memcpy(pszBuf + cbLen, pszPos, cbLine);
            cbLen += cbLine;
            pszBuf[cbLen++] = '\n';
            if (!pszEol) break;
            pszPos = pszEol + 1;
        }
    }
    if (pszLicense) {
        memcpy(pszBuf + cbLen, pszLicTag, sizeof(pszLicTag) - 1);
        cbLen += sizeof(pszLicTag) - 1;
        memcpy(pszBuf + cbLen, pszLicense, strlen(pszLicense));
        cbLen += strlen(pszLicense);
        pszBuf[cbLen++] = '\n';
    }
    pszBuf[cbLen] = '\0';
    return pszBuf;
}

/* ------------------------------------------------------------------ */
/* Printing helpers                                                    */
/* ------------------------------------------------------------------ */

/*!
 * @brief Print a multi-line block with an indent.
 *
 * @param[in] pszText    Text to print. Not NULL.
 * @param[in] pszIndent  Indent prefix. Not NULL.
 */
static void PrintBlock(PCSZ pszText, PCSZ pszIndent) {
    PCSZ pszPos = pszText;
    PCSZ pszLineStart;

    while (*pszPos) {
        pszLineStart = pszPos;
        while (*pszPos && *pszPos != '\n') pszPos++;
        printf("%s", pszIndent);
        fwrite(pszLineStart, 1, (size_t)(pszPos - pszLineStart), stdout);
        printf("\n");
        if (*pszPos == '\n') pszPos++;
    }
}

/* ------------------------------------------------------------------ */
/* Per-file annotation                                                 */
/* ------------------------------------------------------------------ */

/*!
 * @brief Annotate one file with SPDX tags.
 *
 * Comment style, "is this a sidecar?", and the comment block itself
 * all come from comments.toml via the reuse_comments API. The
 * in-file or sidecar mode is selected by pInfo->fSidecar.
 *
 * In-file: writes the SPDX tag-value body at the top of the file
 * (after line 1 if ReuseQueryShebang says the first line must be
 * kept). Sidecar: writes the raw tag-value body to <file>.license.
 *
 * @param[in] pszFilename   File path. Not NULL.
 * @param[in] pszLicense    License expression. Not NULL.
 * @param[in] pszCopyright  Copyright text. Not NULL.
 * @param[in] hStyle        Comment style. Not NULLHANDLE.
 * @param[in] pInfo         Style properties. Not NULL.
 * @param[in] fForce        TRUE to overwrite existing tags.
 * @param[in] fDryRun       TRUE to skip writes.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_DATA       Binary/text mismatch.
 * @retval ERROR_OPEN_FAILED        Cannot write.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET AnnotateOne(PCSZ pszFilename,
                          PCSZ pszLicense,
                          PCSZ pszCopyright,
                          HREUSECOMMENTSTYLE hStyle,
                          const REUSECOMMENTSTYLEINFO *pInfo,
                          BOOL fForce,
                          BOOL fDryRun) {
    BOOL fIsBin;
    PSZ pszBody = NULL;
    PSZ pszBlock = NULL;
    ULONG ulBlockLen = 0;
    APIRET rc;

    fIsBin = IsBinaryFile(pszFilename);

    if (pInfo->fSidecar && !fIsBin) {
        printf("ERROR: %s: declared as binary (sidecar style) but "
               "content is text.\n"
               "       Fix one of:\n"
               "         - add a [[map]] entry for this file with a "
               "text style;\n"
               "         - or fix the file if its content is wrong.\n",
               pszFilename);
        return ERROR_INVALID_DATA;
    }

    if (!pInfo->fSidecar && fIsBin) {
        printf("ERROR: %s: declared as text but content is binary.\n"
               "       Fix one of:\n"
               "         - add a [[map]] entry for this file with "
               "style = \"uncommentable\";\n"
               "         - or fix the file if its content is wrong.\n",
               pszFilename);
        return ERROR_INVALID_DATA;
    }

    pszBody = build_spdx_body(pszLicense, pszCopyright);
    if (!pszBody) return ERROR_NOT_ENOUGH_MEMORY;

    if (pInfo->fSidecar) {
        pszBlock = pszBody;
        pszBody = NULL;
    } else {
        rc = ReuseQueryCommentBlock(hStyle, pszBody, FALSE,
                                    NULL, 0, &ulBlockLen);
        if (rc != NO_ERROR) {
            free(pszBody);
            return rc;
        }
        pszBlock = (PSZ)malloc(ulBlockLen);
        if (!pszBlock) {
            free(pszBody);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        rc = ReuseQueryCommentBlock(hStyle, pszBody, FALSE,
                                    pszBlock, ulBlockLen, NULL);
        free(pszBody);
        if (rc != NO_ERROR) {
            free(pszBlock);
            return rc;
        }
    }

    if (pInfo->fSidecar) {
        CHAR achSidecar[1200];
        BOOL fExists;
        BOOL fEqual = FALSE;

        snprintf(achSidecar, sizeof(achSidecar), "%s.license", pszFilename);
        fExists = FileExists(achSidecar);

        if (fExists) {
            PSZ pszExisting;
            PSZ pszN1 = NULL, pszN2 = NULL;
            pszExisting = read_file_to_heap(achSidecar);
            if (pszExisting) {
                pszN1 = normalize_to_heap(pszExisting);
                pszN2 = normalize_to_heap(pszBlock);
                if (pszN1 && pszN2 && strcmp(pszN1, pszN2) == 0)
                    fEqual = TRUE;
                free(pszN1); free(pszN2);
                free(pszExisting);
            }
        }

        if (fEqual) {
            printf("Up to date (sidecar): %s\n", achSidecar);
            free(pszBlock);
            return NO_ERROR;
        }

        if (fExists && !fForce) {
            printf("Outdated (sidecar):   %s\n", achSidecar);
            printf("    reason:           sidecar content differs\n");
            printf("    action:           use --force to overwrite\n");
            free(pszBlock);
            return NO_ERROR;
        }

        if (fDryRun) {
            printf("%s %s\n",
                   fExists ? "Would update (sidecar):"
                           : "Would create (sidecar):",
                   achSidecar);
            printf("    reason:           binary file\n");
            printf("    tags:\n");
            PrintBlock(pszBlock, "        ");
            free(pszBlock);
            return NO_ERROR;
        }

        {
            FILE *fpW = fopen(achSidecar, "wb");
            if (!fpW || (pszBlock && fputs(pszBlock, fpW) == EOF)) {
                if (fpW) fclose(fpW);
                printf("ERROR: cannot write file: %s\n"
                       "       Check directory permissions.\n", achSidecar);
                free(pszBlock);
                return ERROR_OPEN_FAILED;
            }
            fclose(fpW);
        }
        printf("%s %s\n",
               fExists ? "Updated (sidecar):   " : "Created (sidecar):   ",
               achSidecar);
        free(pszBlock);
        return NO_ERROR;
    } else {
        BOOL fHasTag = FALSE;
        BOOL fKeepFirstLine = FALSE;
        CHAR achFirstLine[MAX_LINE];
        CHAR achTempName[1024];
        FILE *fp, *fpOut;
        CHAR achLine[MAX_LINE];

        rc = ReuseQueryFileHasTag(pszFilename, hStyle, &fHasTag);
        if (rc != NO_ERROR) {
            free(pszBlock);
            return rc;
        }

        if (fHasTag && !fForce) {
            printf("Skipped (has tags):   %s\n", pszFilename);
            free(pszBlock);
            return NO_ERROR;
        }

        achFirstLine[0] = '\0';
        fp = fopen(pszFilename, "r");
        if (fp) {
            if (fgets(achFirstLine, sizeof(achFirstLine), fp)) {
            }
            fclose(fp);
        }

        rc = ReuseQueryShebang(hStyle, achFirstLine, &fKeepFirstLine);
        if (rc != NO_ERROR) {
            free(pszBlock);
            return rc;
        }

        if (fDryRun) {
            printf("%s %s\n",
                   fHasTag ? "Would update tags:   "
                           : "Would add tags:      ",
                   pszFilename);
            printf("    comment style:    %s\n",
                   pInfo->pszName ? pInfo->pszName : "(unknown)");
            if (fKeepFirstLine)
                printf("    insertion:        after first line "
                       "(kept as-is)\n");
            else
                printf("    insertion:        at top of file\n");
            PrintBlock(pszBlock, "        ");
            free(pszBlock);
            return NO_ERROR;
        }

        snprintf(achTempName, sizeof(achTempName), "%s.tmp", pszFilename);
        fpOut = fopen(achTempName, "w");
        if (!fpOut) {
            printf("ERROR: cannot open file for writing: %s\n"
                   "       Check directory permissions.\n", achTempName);
            free(pszBlock);
            return ERROR_OPEN_FAILED;
        }

        if (fKeepFirstLine) {
            fputs(achFirstLine, fpOut);
            if (achFirstLine[0] != '\0' &&
                achFirstLine[strlen(achFirstLine) - 1] != '\n') {
                fputc('\n', fpOut);
            }
            fputs(pszBlock, fpOut);

            fp = fopen(pszFilename, "r");
            if (fp) {
                CHAR achSkip[MAX_LINE];
                if (fgets(achSkip, sizeof(achSkip), fp)) {
                    while (fgets(achLine, sizeof(achLine), fp))
                        fputs(achLine, fpOut);
                }
                fclose(fp);
            }
        } else {
            fputs(pszBlock, fpOut);
            fp = fopen(pszFilename, "r");
            if (fp) {
                while (fgets(achLine, sizeof(achLine), fp))
                    fputs(achLine, fpOut);
                fclose(fp);
            }
        }
        fclose(fpOut);

        remove(pszFilename);
        if (rename(achTempName, pszFilename) != 0) {
            printf("ERROR: cannot rename %s to %s\n"
                   "       Check file permissions.\n",
                   achTempName, pszFilename);
            free(pszBlock);
            return ERROR_OPEN_FAILED;
        }
        printf("%s %s\n",
               fHasTag ? "Updated tags:        " : "Added tags:          ",
               pszFilename);
        free(pszBlock);
        return NO_ERROR;
    }
}

/* ------------------------------------------------------------------ */
/* LICENSES/ report printing                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Human-readable text for a LICENSES/ reason code.
 *
 * @param[in] ulCode  One of REUSE_LICENSES_*.
 *
 * @return Static description, or "" for unknown codes.
 */
static PCSZ LicensesReasonText(ULONG ulCode) {
    switch (ulCode) {
    case REUSE_LICENSES_DIR_MISSING:
        return "LICENSES/ directory is missing";
    case REUSE_LICENSES_DIR_CREATED:
        return "LICENSES/ directory was created";
    case REUSE_LICENSES_BAD_NAME:
        return "file name is not a valid SPDX identifier";
    case REUSE_LICENSES_UNUSED_FILE:
        return "file is not used by any license";
    case REUSE_LICENSES_MISSING_FILE:
        return "used license has no file in LICENSES/";
    case REUSE_LICENSES_DEPRECATED_ID:
        return "identifier is deprecated by SPDX";
    case REUSE_LICENSES_NO_EXTENSION:
        return "license file has no extension";
    case REUSE_LICENSES_TEXT_MISMATCH:
        return "license text does not match the SPDX database";
    case REUSE_LICENSES_FILE_CREATED:
        return "license file was created";
    case REUSE_LICENSES_FILE_UPDATED:
        return "license file was updated";
    case REUSE_LICENSES_FILE_UP_TO_DATE:
        return "license file is up to date";
    case REUSE_LICENSES_FILE_OUTDATED:
        return "license file is outdated; use --force";
    case REUSE_LICENSES_MANUAL_REQUIRED:
        return "LicenseRef-* requires manual text";
    case REUSE_LICENSES_NO_DB_TEXT:
        return "no text in the SPDX database";
    default:
        return "";
    }
}

/*!
 * @brief Print every record of a LICENSES/ report to stdout.
 *
 * @param[in]  hReport      Report handle, or NULLHANDLE.
 * @param[out] pulErrors    Receiver for the error count. Not NULL.
 * @param[out] pulWarnings  Receiver for the warning count. Not NULL.
 */
static void PrintLicensesReport(HREUSELICENSEREPORT hReport,
                                PULONG pulErrors, PULONG pulWarnings) {
    ULONG ulCount = 0, ulIdx;

    *pulErrors = 0;
    *pulWarnings = 0;

    if (hReport == NULLHANDLE) return;
    if (ReuseLicensesReportGetCount(hReport, &ulCount) != NO_ERROR)
        return;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        REUSEERR err;
        PCSZ pszSev;
        PCSZ pszReason;

        if (ReuseLicensesReportGet(hReport, ulIdx, &err) != NO_ERROR)
            continue;

        switch (err.ulSeverity) {
        case REUSE_SEV_ERROR:
            pszSev = "ERROR"; (*pulErrors)++; break;
        case REUSE_SEV_WARNING:
            pszSev = "WARNING"; (*pulWarnings)++; break;
        default:
            pszSev = ""; break;
        }

        pszReason = LicensesReasonText(err.ulCode);
        if (!pszReason[0]) pszReason = err.achDetail;

        if (err.achFile[0] && err.achDetail[0] &&
            strcmp(err.achDetail, pszReason) != 0)
            printf("%s%s%s: %s (%s)\n",
                   pszSev[0] ? pszSev : "",
                   pszSev[0] ? ": " : "",
                   err.achFile, pszReason, err.achDetail);
        else if (err.achFile[0])
            printf("%s%s%s: %s\n",
                   pszSev[0] ? pszSev : "",
                   pszSev[0] ? ": " : "",
                   err.achFile, pszReason);
        else if (err.achDetail[0])
            printf("%s%s%s\n",
                   pszSev[0] ? pszSev : "",
                   pszSev[0] ? ": " : "",
                   pszReason);
    }
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

/*!
 * @brief Entry point of the annotate tool.
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
 * @retval 0 Success.
 * @retval 1 Error.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char *argv[]) {
    PCSZ pszDir = ".";
    BOOL fDryRun = TRUE;
    BOOL fForce = FALSE;
    BOOL fNoGitignore = FALSE;
    int i;
    PCSZ pszLicenseOverride = NULL;
    PCSZ pszCopyrightOverride = NULL;
    PCSZ pszSpdxDbRoot = NULL;
    PCSZ pszCacheFile = NULL;
    PCSZ pszCommentsConfig = NULL;
    HREUSETREE hTree = NULLHANDLE;
    APIRET rcDb;
    HSTRSET hUsedLicenses = NULLHANDLE;
    HSTRSET hPaths = NULLHANDLE;
    HREUSELICENSEREPORT hLicReport = NULLHANDLE;
    REUSEDISCOVEROPTIONS walk_opts;
    PSZ pszRepoRoot = NULL;
    GITIGNORELIST gitignore_rules;
    BOOL fHasGitignore = FALSE;
    ULONG ulTotalErrors = 0;
    PCSZ pszWcc;
    int nExit;
#ifdef __LINUX__
    pszWcc = "_wcc.sh";
#else
    pszWcc = "_wcc.cmd";
#endif

    GitIgnoreListInit(&gitignore_rules);

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: reuse-annotate [options] [<directory>]\n"
                   "  --write                    Apply changes "
                   "(default is dry-run)\n"
                   "  --dry-run                  Show what would be done, "
                   "do not write (default)\n"
                   "  --force                    Overwrite existing tags / "
                   "outdated sidecar / outdated license texts\n"
                   "  --license=<id>             Override license for all "
                   "files\n"
                   "  --copyright=<text>         Override copyright for all "
                   "files\n"
                   "  --spdx-db=<path>           SPDX database root "
                   "(required)\n"
                   "  --comments-config=<path>   comments.toml describing "
                   "comment styles\n"
                   "                             for in-file SPDX tags "
                   "(required)\n"
                   "  --cache=<path>             SPDX database cache "
                   "file\n"
                   "  --comment-style=<ext>=<style>\n"
                   "                             Legacy option; parsed "
                   "and validated, not yet applied\n"
                   "  --no-gitignore             Do not apply .gitignore "
                   "rules\n"
                   "  --help, -h                 Show this help\n");
            GitIgnoreListFree(&gitignore_rules);
            return 0;
        }
        if (strcmp(argv[i], "--write") == 0) {
            fDryRun = FALSE;
        } else if (strcmp(argv[i], "--dry-run") == 0) {
            fDryRun = TRUE;
        } else if (strcmp(argv[i], "--force") == 0) {
            fForce = TRUE;
        } else if (strcmp(argv[i], "--no-gitignore") == 0) {
            fNoGitignore = TRUE;
        } else if (strncmp(argv[i], "--license=", 10) == 0) {
            pszLicenseOverride = argv[i] + 10;
        } else if (strncmp(argv[i], "--copyright=", 12) == 0) {
            pszCopyrightOverride = argv[i] + 12;
        } else if (strncmp(argv[i], "--spdx-db=", 10) == 0) {
            pszSpdxDbRoot = argv[i] + 10;
        } else if (strncmp(argv[i], "--cache=", 8) == 0) {
            pszCacheFile = argv[i] + 8;
        } else if (strncmp(argv[i], "--comments-config=", 18) == 0) {
            pszCommentsConfig = argv[i] + 18;
        } else if (strncmp(argv[i], "--comment-style=", 16) == 0) {
            AddStyleOverride(argv[i] + 16);
        } else if (argv[i][0] != '-') {
            pszDir = argv[i];
        } else {
            printf("ERROR: unknown option: %s\n"
                   "       Run 'reuse-annotate --help' for usage.\n",
                   argv[i]);
            GitIgnoreListFree(&gitignore_rules);
            return 1;
        }
    }

    if (g_ulStyleOverrideCount > 0) {
        printf("WARNING: --comment-style is not yet supported; "
               "use [[map]] in comments.toml instead.\n");
    }

    if (!pszSpdxDbRoot) {
        printf("ERROR: SPDX database is not configured.\n"
               "       --spdx-db=<path> is required.\n"
               "       Run 'reuse-annotate --help' for usage.\n");
        GitIgnoreListFree(&gitignore_rules);
        return 1;
    }

    rcDb = SpdxOpenDatabase(pszSpdxDbRoot, pszCacheFile);
    if (rcDb & SPDXDB_ERROR_LICENSES) {
        printf("ERROR: SPDX license database is unavailable "
               "(licenses.json not loaded).\n"
               "       Expected at <spdx-db>/licenses.json.\n"
               "       Cannot validate SPDX identifiers. Aborting.\n");
        SpdxCloseDatabase();
        GitIgnoreListFree(&gitignore_rules);
        return 1;
    }
    if (rcDb & SPDXDB_ERROR_EXCEPTIONS) {
        printf("ERROR: SPDX exceptions database is unavailable "
               "(exceptions.json not loaded).\n"
               "       Expected at <spdx-db>/exceptions.json.\n"
               "       Cannot validate SPDX identifiers. Aborting.\n");
        SpdxCloseDatabase();
        GitIgnoreListFree(&gitignore_rules);
        return 1;
    }
    if (rcDb & SPDXDB_ERROR_CACHE)
        printf("WARNING: cache could not be written.\n"
               "         Next run will re-parse JSON indexes.\n");

    if (StrSetCreate(&hUsedLicenses) != NO_ERROR) {
        printf("ERROR: out of memory\n");
        SpdxCloseDatabase();
        GitIgnoreListFree(&gitignore_rules);
        return 1;
    }

    {
        ULONG ulSize = 0;
        if (GitFindRepoRoot(pszDir, NULL, 0, &ulSize) == NO_ERROR &&
            ulSize > 0) {
            pszRepoRoot = (PSZ)malloc(ulSize);
            if (pszRepoRoot) {
                if (GitFindRepoRoot(pszDir, pszRepoRoot, ulSize, NULL)
                        != NO_ERROR) {
                    free(pszRepoRoot);
                    pszRepoRoot = NULL;
                }
            }
        }
    }

    {
        APIRET rc = ReuseOpenTree(pszDir, pszCommentsConfig, &hTree);
        if (rc == REUSE_ERROR_NO_COMMENT_STYLE) {
            printf("ERROR: --comments-config=<path> is required.\n"
                   "       It defines the comment styles used to read "
                   "and write in-file SPDX tags.\n"
                   "       Run 'reuse-annotate --help' for usage.\n");
            StrSetDestroy(hUsedLicenses);
            GitIgnoreListFree(&gitignore_rules);
            free(pszRepoRoot);
            SpdxCloseDatabase();
            return 1;
        }
        if (rc != NO_ERROR) {
            printf("ERROR: cannot open REUSE project at %s\n"
                   "       The directory is missing or unreadable.\n",
                   pszDir);
            StrSetDestroy(hUsedLicenses);
            GitIgnoreListFree(&gitignore_rules);
            free(pszRepoRoot);
            SpdxCloseDatabase();
            return 1;
        }
        {
            ULONG ulErrs = 0;
            if (ReuseTreeGetErrorCount(hTree, &ulErrs) == NO_ERROR)
                ulTotalErrors += ulErrs;
        }
    }

    if (!fNoGitignore) {
        if (GitCollectGitignores(pszRepoRoot, pszDir, &gitignore_rules)
                == NO_ERROR && gitignore_rules.ulCount > 0) {
            fHasGitignore = TRUE;
        }
    }

    ReuseSetDiscoverOptionsDefault(&walk_opts);
    walk_opts.fRecursive             = FALSE;
    walk_opts.fSkipHidden            = TRUE;
    walk_opts.fSkipVcsDirs           = TRUE;
    walk_opts.fSkipLicensesDir       = TRUE;
    walk_opts.fSkipReuseDir          = TRUE;
    walk_opts.fSkipLicenseSidecars   = TRUE;
    walk_opts.fSkipReuseToml         = TRUE;
    walk_opts.fSkipLicenseFiles      = TRUE;
    if (fHasGitignore) {
        walk_opts.fUseGitignore   = TRUE;
        walk_opts.pszRepoRoot     = pszRepoRoot ? pszRepoRoot : pszDir;
        walk_opts.pGitignoreRules = &gitignore_rules;
    }

    if (StrSetCreate(&hPaths) != NO_ERROR) {
        printf("ERROR: out of memory\n");
        StrSetDestroy(hUsedLicenses);
        ReuseCloseTree(hTree);
        GitIgnoreListFree(&gitignore_rules);
        free(pszRepoRoot);
        SpdxCloseDatabase();
        return 1;
    }

    if (ReuseDiscoverWalkTree(pszDir, &walk_opts, hPaths) != NO_ERROR) {
        printf("ERROR: cannot walk tree: %s\n"
               "       Check that the directory exists and is "
               "readable.\n",
               pszDir);
        StrSetDestroy(hPaths);
        StrSetDestroy(hUsedLicenses);
        ReuseCloseTree(hTree);
        GitIgnoreListFree(&gitignore_rules);
        free(pszRepoRoot);
        SpdxCloseDatabase();
        return 1;
    }

    if (fDryRun) {
        printf("Mode: dry-run (use %s annotate-write to apply changes)\n",
               pszWcc);
    } else {
        printf("Mode: write\n");
    }

    printf("\n=== Tags ===\n");

    {
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetEnumFirst(hPaths, &hEnum) == NO_ERROR) {
            do {
                CHAR achFullPath[PATH_BUF];
                PCSZ pszLicense;
                PCSZ pszCopyright;
                PSZ pszReuseLicense = NULL;
                PSZ pszReuseCopyright = NULL;
                PSZ pszNormalized = NULL;
                HREUSECOMMENTSTYLE hStyle = NULLHANDLE;
                REUSECOMMENTSTYLEINFO info;
                BOOL fStyleFound = FALSE;
                APIRET rc;

                if (StrSetEnumGet(hEnum, achFullPath, sizeof(achFullPath),
                                  NULL) != NO_ERROR)
                    continue;

                {
                    REUSELICENSEINFO resolved;
                    APIRET rcResolve;
                    memset(&resolved, 0, sizeof(resolved));
                    rcResolve = ReuseResolveLicense(hTree, achFullPath,
                                                    NULL, NULL, &resolved);
                    if (rcResolve == REUSE_ERROR_NO_COMMENT_STYLE) {
                        printf("ERROR: %s: no [[map]] entry in "
                               "comments.toml matches this file.\n"
                               "       Add a [[map]] entry "
                               "(e.g. glob = \"*.<ext>\", style = ...).\n",
                               achFullPath);
                        ulTotalErrors++;
                        continue;
                    }
                    if (rcResolve == NO_ERROR) {
                        if (resolved.achLicense[0]) {
                            size_t cbN = strlen(resolved.achLicense);
                            pszReuseLicense = (PSZ)malloc(cbN + 1);
                            if (pszReuseLicense)
                                memcpy(pszReuseLicense,
                                       resolved.achLicense, cbN + 1);
                        }
                        if (resolved.achCopyright[0]) {
                            size_t cbN = strlen(resolved.achCopyright);
                            pszReuseCopyright = (PSZ)malloc(cbN + 1);
                            if (pszReuseCopyright)
                                memcpy(pszReuseCopyright,
                                       resolved.achCopyright, cbN + 1);
                        }
                    }
                }

                rc = ReuseQueryTreeCommentStyle(hTree, achFullPath,
                                                &fStyleFound, &hStyle);
                if (rc != NO_ERROR) {
                    printf("ERROR: %s: cannot resolve comment style "
                           "(code %lu).\n",
                           achFullPath, (unsigned long)rc);
                    ulTotalErrors++;
                    free(pszReuseLicense);
                    free(pszReuseCopyright);
                    continue;
                }
                if (!fStyleFound || hStyle == NULLHANDLE) {
                    printf("ERROR: %s: no [[map]] entry in "
                           "comments.toml matches this file.\n",
                           achFullPath);
                    ulTotalErrors++;
                    free(pszReuseLicense);
                    free(pszReuseCopyright);
                    continue;
                }

                memset(&info, 0, sizeof(info));
                rc = ReuseQueryCommentStyleInfo(hStyle, &info);
                if (rc != NO_ERROR) {
                    printf("ERROR: %s: cannot query comment style info "
                           "(code %lu).\n",
                           achFullPath, (unsigned long)rc);
                    ulTotalErrors++;
                    free(pszReuseLicense);
                    free(pszReuseCopyright);
                    continue;
                }

                pszLicense = pszReuseLicense ? pszReuseLicense
                                             : pszLicenseOverride;
                pszCopyright = pszReuseCopyright ? pszReuseCopyright
                                                 : pszCopyrightOverride;

                if (pszLicense) {
                    ULONG ulSize = 0;
                    if (SpdxQueryExpressionCanonical(pszLicense,
                                                     NULL, 0,
                                                     &ulSize) == NO_ERROR &&
                        ulSize > 0) {
                        pszNormalized = (PSZ)malloc(ulSize);
                        if (pszNormalized) {
                            if (SpdxQueryExpressionCanonical(pszLicense,
                                                             pszNormalized,
                                                             ulSize,
                                                             NULL)
                                    == NO_ERROR)
                                pszLicense = pszNormalized;
                        }
                    }
                }

                if (!pszLicense || !pszCopyright) {
                    CHAR achMfPath[1100];

                    PathMakeJoin(pszDir, "makefile", achMfPath,
                                 sizeof(achMfPath), NULL);

                    if (!pszLicense) {
                        printf("ERROR: %s: no license information "
                               "available.\n"
                               "       Annotate needs to know which "
                               "license to write.\n"
                               "       Fix one of:\n"
                               "         - add a [[annotations]] entry in "
                               "REUSE.toml;\n"
                               "         - or pass --license=<id> on the "
                               "command line;\n"
                               "         - or set LICENSE in %s (e.g. "
                               "LICENSE = MIT).\n",
                               achFullPath, achMfPath);
                        ulTotalErrors++;
                    }

                    if (!pszCopyright) {
                        printf("ERROR: %s: no copyright information "
                               "available.\n"
                               "       Annotate needs to know which "
                               "copyright to write.\n"
                               "       Fix one of:\n"
                               "         - add a [[annotations]] entry in "
                               "REUSE.toml;\n"
                               "         - or pass --copyright=<text> on "
                               "the command line;\n"
                               "         - or set COPYRIGHT in %s (e.g. "
                               "COPYRIGHT = Copyright (C) 2025 "
                               "<holder>).\n",
                               achFullPath, achMfPath);
                        ulTotalErrors++;
                    }

                    free(pszNormalized);
                    free(pszReuseLicense);
                    free(pszReuseCopyright);
                    continue;
                }

                {
                    HSTRSET hIds = NULLHANDLE;
                    HSTRSETENUM hIdEnum = NULLHANDLE;
                    if (StrSetCreate(&hIds) == NO_ERROR) {
                        SpdxExpressionCollectIds(pszLicense, hIds, NULL);
                        if (StrSetEnumFirst(hIds, &hIdEnum) == NO_ERROR) {
                            do {
                                CHAR achId[256];
                                CHAR achCanon[256];
                                ULONG ulCanonSize = sizeof(achCanon);

                                if (StrSetEnumGet(hIdEnum, achId,
                                                  sizeof(achId),
                                                  NULL) != NO_ERROR)
                                    continue;
                                if (SpdxQueryCanonicalId(achId, achCanon,
                                                         ulCanonSize,
                                                         NULL)
                                        == NO_ERROR) {
                                    StrSetAdd(hUsedLicenses, achCanon);
                                } else {
                                    StrSetAdd(hUsedLicenses, achId);
                                }
                            } while (StrSetEnumNext(hIdEnum) == NO_ERROR);
                            StrSetEnumClose(hIdEnum);
                        }
                        StrSetDestroy(hIds);
                    }
                }

                rc = AnnotateOne(achFullPath, pszLicense, pszCopyright,
                                 hStyle, &info, fForce, fDryRun);
                if (rc != NO_ERROR) ulTotalErrors++;

                free(pszNormalized);
                free(pszReuseLicense);
                free(pszReuseCopyright);
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }

    StrSetDestroy(hPaths);

    printf("\n=== LICENSES/ ===\n");
    if (ReuseLicensesReportCreate(&hLicReport) == NO_ERROR) {
        ULONG ulLicErrors = 0;
        ULONG ulLicWarnings = 0;
        ReuseLicensesEnsure(hTree, hUsedLicenses, fForce, fDryRun,
                            hLicReport);
        PrintLicensesReport(hLicReport, &ulLicErrors, &ulLicWarnings);
        ulTotalErrors += ulLicErrors;
        ReuseLicensesReportFree(hLicReport);
    } else {
        printf("ERROR: out of memory\n");
        ulTotalErrors++;
    }

    StrSetDestroy(hUsedLicenses);
    ReuseCloseTree(hTree);
    GitIgnoreListFree(&gitignore_rules);
    free(pszRepoRoot);
    SpdxCloseDatabase();
    {
        ULONG k;
        for (k = 0; k < g_ulStyleOverrideCount; k++)
            free(g_paStyleOverrides[k].pszExt);
        free(g_paStyleOverrides);
    }

    nExit = (ulTotalErrors > 0) ? 1 : 0;
    return nExit;
}
