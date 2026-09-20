/* reuse-annotate.c - SPDX tag annotation tool (C89) */

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
#include "spdx_tag.h"
#include "reuse_lic.h"

/**
 * @file reuse-annotate.c
 * @brief SPDX tag annotation tool.
 *
 * Walks the project tree, resolves license and copyright for each
 * file (REUSE.toml, sidecar, in-file tags), and writes the missing
 * SPDX headers. Sidecar files are created for binary files. The
 * LICENSES/ directory is populated with the license texts declared
 * by the project.
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 *   - SPDX 2.3, Annex D (license expression grammar).
 *     https://spdx.github.io/spdx-spec/v2.3/
 */

#define MAX_LINE     4096
#define BINARY_PROBE 8192
#define PATH_BUF     1024

/* ------------------------------------------------------------------ */
/* Types                                                               */
/* ------------------------------------------------------------------ */

/**
 * @enum _COMMENTSTYLE
 * @brief Comment style used when writing SPDX tags.
 */
typedef enum _COMMENTSTYLE {
    STYLE_C,          /**< C-style comment.                  */
    STYLE_HASH,       /**< Hash comment.                     */
    STYLE_REM,        /**< REM or @REM comment.              */
    STYLE_SIDECAR,    /**< Sidecar <file>.license.           */
    STYLE_UNKNOWN     /**< Unrecognized file type.           */
} COMMENTSTYLE;

/**
 * @struct _STYLEOVERRIDE
 * @brief One --comment-style override entry.
 */
typedef struct _STYLEOVERRIDE {
    PSZ           pszExt;      /**< Extension or base name.       */
    COMMENTSTYLE  style;       /**< Style to apply.               */
} STYLEOVERRIDE, *PSTYLEOVERRIDE;

/* ------------------------------------------------------------------ */
/* Global state                                                        */
/* ------------------------------------------------------------------ */

static PSTYLEOVERRIDE g_paStyleOverrides = NULL;
static ULONG          g_ulStyleOverrideCount = 0;

/* ------------------------------------------------------------------ */
/* Heap helpers                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief Read a whole file into a heap string.
 *
 * Uses the size-query convention of SpdxReadFileAll.
 *
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return malloc'd NUL-terminated content, or NULL on error.
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

/**
 * @brief Normalize text into a heap string.
 *
 * Uses the size-query convention of SpdxNormalizeText.
 *
 * @param[in] pszSrc  Source text. Not NULL.
 *
 * @return malloc'd normalized text, or NULL on error.
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

/**
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

/**
 * @brief Query whether a file contains a NUL byte in its first
 *        BINARY_PROBE bytes.
 *
 * @param[in] pszPath  Path to the file. Not NULL.
 *
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
/* Comment style handling                                              */
/* ------------------------------------------------------------------ */

/**
 * @brief Translate a --comment-style value into a COMMENTSTYLE.
 *
 * @param[in] pszName  Style name. Not NULL.
 *
 * @return The matching COMMENTSTYLE, or STYLE_UNKNOWN.
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

/**
 * @brief Append one --comment-style override.
 *
 * On error prints a diagnostic and exits with EXIT_FAILURE.
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

/**
 * @brief Query the comment style for a file name.
 *
 * @param[in] pszFilename  File name. Not NULL.
 *
 * @return The matching COMMENTSTYLE, or STYLE_UNKNOWN.
 */
static COMMENTSTYLE DetectStyle(PCSZ pszFilename) {
    PCSZ pszBase = SpdxGetFileName(pszFilename);
    PCSZ pszExt = strrchr(pszBase, '.');
    static PCSZ apszCExt[] = {
        ".c", ".cpp", ".h", ".hpp", ".cc", ".cxx",
        ".asm", ".rc", ".inc", NULL
    };
    static PCSZ apszHashExt[] = {
        ".sh", ".py", ".pl", ".toml", ".yml", ".yaml",
        ".md", ".txt", ".ini", ".cfg", ".conf", ".mk", NULL
    };
    static PCSZ apszRemExt[] = { ".cmd", ".bat", NULL };
    static PCSZ apszBinaryExt[] = {
        ".exe", ".obj", ".lib", ".dll", ".res", ".ico",
        ".bmp", ".png", ".jpg", ".jpeg", ".gif", ".sys",
        ".com", ".o", ".a", ".so", ".zip", ".gz", ".tar",
        ".pdf", ".dat", ".bin", ".cur", ".fon", ".ttf",
        ".hlp", ".wnf", ".inf", ".chm", ".mo", ".qm", NULL
    };
    ULONG ulIdx;

    for (ulIdx = 0; ulIdx < g_ulStyleOverrideCount; ulIdx++) {
        PCSZ pszPat = g_paStyleOverrides[ulIdx].pszExt;
        if (pszPat[0] == '.') {
            if (pszExt && strcmp(pszPat, pszExt) == 0)
                return g_paStyleOverrides[ulIdx].style;
        } else {
            if (strcmp(pszPat, pszBase) == 0)
                return g_paStyleOverrides[ulIdx].style;
        }
    }

    if (strcmp(pszBase, "makefile") == 0 ||
        strcmp(pszBase, "Makefile") == 0 ||
        strcmp(pszBase, "GNUmakefile") == 0 ||
        strncmp(pszBase, "README", 6) == 0 ||
        strncmp(pszBase, "readme", 6) == 0 ||
        strcmp(pszBase, "AUTHORS") == 0 ||
        strcmp(pszBase, "NEWS") == 0 ||
        strcmp(pszBase, "ChangeLog") == 0 ||
        strcmp(pszBase, "CHANGELOG") == 0 ||
        strcmp(pszBase, "TODO") == 0 ||
        strcmp(pszBase, "INSTALL") == 0)
        return STYLE_HASH;

    if (pszExt) {
        for (ulIdx = 0; apszCExt[ulIdx]; ulIdx++)
            if (strcmp(pszExt, apszCExt[ulIdx]) == 0) return STYLE_C;
        for (ulIdx = 0; apszHashExt[ulIdx]; ulIdx++)
            if (strcmp(pszExt, apszHashExt[ulIdx]) == 0)
                return STYLE_HASH;
        for (ulIdx = 0; apszRemExt[ulIdx]; ulIdx++)
            if (strcmp(pszExt, apszRemExt[ulIdx]) == 0) return STYLE_REM;
        for (ulIdx = 0; apszBinaryExt[ulIdx]; ulIdx++)
            if (strcmp(pszExt, apszBinaryExt[ulIdx]) == 0)
                return STYLE_SIDECAR;
    }
    return STYLE_UNKNOWN;
}

/**
 * @brief Human-readable name of a comment style.
 *
 * @param[in] style  Comment style.
 *
 * @return Static NUL-terminated description.
 */
static PCSZ StyleName(COMMENTSTYLE style) {
    switch (style) {
    case STYLE_C:       return "C comment";
    case STYLE_HASH:    return "hash comment";
    case STYLE_REM:     return "REM comment";
    case STYLE_SIDECAR: return "sidecar";
    default:            return "unknown";
    }
}

/* ------------------------------------------------------------------ */
/* Insertion block                                                     */
/* ------------------------------------------------------------------ */

/**
 * @brief Build the SPDX header block for a file.
 *
 * @param[in] style            Comment style. Not STYLE_UNKNOWN.
 * @param[in] pszLicense       License expression, or NULL.
 * @param[in] pszCopyright     Copyright text, or NULL.
 * @param[in] fEchoOffPresent  TRUE if a leading @echo off was found.
 *
 * @return malloc'd block, or NULL on OOM.
 */
static PSZ BuildInsertion(COMMENTSTYLE style,
                          PCSZ pszLicense,
                          PCSZ pszCopyright,
                          BOOL fEchoOffPresent) {
    size_t cbCap = 256;
    size_t cbLen = 0;
    PSZ pszBuf;
    PCSZ pszPos;

    if (pszLicense)   cbCap += strlen(pszLicense) * 4;
    if (pszCopyright) cbCap += strlen(pszCopyright) * 4;

    pszBuf = (PSZ)malloc(cbCap);
    if (!pszBuf) return NULL;
    pszBuf[0] = '\0';

    switch (style) {
    case STYLE_C:
        cbLen += (size_t)sprintf(pszBuf + cbLen, "/*\n");
        if (pszCopyright) {
            pszPos = pszCopyright;
            while (*pszPos) {
                PCSZ pszEol = strchr(pszPos, '\n');
                size_t cbLine = pszEol ? (size_t)(pszEol - pszPos)
                                       : strlen(pszPos);
                cbLen += (size_t)sprintf(pszBuf + cbLen,
                                         " * SPDX-FileCopyrightText: ");
                memcpy(pszBuf + cbLen, pszPos, cbLine);
                cbLen += cbLine;
                pszBuf[cbLen++] = '\n';
                if (!pszEol) break;
                pszPos = pszEol + 1;
            }
        }
        if (pszLicense) {
            pszPos = pszLicense;
            while (*pszPos) {
                PCSZ pszEol = strchr(pszPos, '\n');
                size_t cbLine = pszEol ? (size_t)(pszEol - pszPos)
                                       : strlen(pszPos);
                cbLen += (size_t)sprintf(pszBuf + cbLen,
                                         " * SPDX-License-Identifier: ");
                memcpy(pszBuf + cbLen, pszPos, cbLine);
                cbLen += cbLine;
                pszBuf[cbLen++] = '\n';
                if (!pszEol) break;
                pszPos = pszEol + 1;
            }
        }
        cbLen += (size_t)sprintf(pszBuf + cbLen, " */\n\n");
        break;
    case STYLE_REM: {
        PCSZ pszPfx = fEchoOffPresent ? "rem " : "@rem ";
        if (pszCopyright) {
            pszPos = pszCopyright;
            while (*pszPos) {
                PCSZ pszEol = strchr(pszPos, '\n');
                size_t cbLine = pszEol ? (size_t)(pszEol - pszPos)
                                       : strlen(pszPos);
                cbLen += (size_t)sprintf(pszBuf + cbLen,
                                         "%sSPDX-FileCopyrightText: ",
                                         pszPfx);
                memcpy(pszBuf + cbLen, pszPos, cbLine);
                cbLen += cbLine;
                pszBuf[cbLen++] = '\n';
                if (!pszEol) break;
                pszPos = pszEol + 1;
            }
        }
        if (pszLicense) {
            pszPos = pszLicense;
            while (*pszPos) {
                PCSZ pszEol = strchr(pszPos, '\n');
                size_t cbLine = pszEol ? (size_t)(pszEol - pszPos)
                                       : strlen(pszPos);
                cbLen += (size_t)sprintf(pszBuf + cbLen,
                                         "%sSPDX-License-Identifier: ",
                                         pszPfx);
                memcpy(pszBuf + cbLen, pszPos, cbLine);
                cbLen += cbLine;
                pszBuf[cbLen++] = '\n';
                if (!pszEol) break;
                pszPos = pszEol + 1;
            }
        }
        cbLen += (size_t)sprintf(pszBuf + cbLen, "\n");
        break;
    }
    case STYLE_SIDECAR:
        if (pszCopyright) {
            pszPos = pszCopyright;
            while (*pszPos) {
                PCSZ pszEol = strchr(pszPos, '\n');
                size_t cbLine = pszEol ? (size_t)(pszEol - pszPos)
                                       : strlen(pszPos);
                cbLen += (size_t)sprintf(pszBuf + cbLen,
                                         "SPDX-FileCopyrightText: ");
                memcpy(pszBuf + cbLen, pszPos, cbLine);
                cbLen += cbLine;
                pszBuf[cbLen++] = '\n';
                if (!pszEol) break;
                pszPos = pszEol + 1;
            }
        }
        if (pszLicense) {
            pszPos = pszLicense;
            while (*pszPos) {
                PCSZ pszEol = strchr(pszPos, '\n');
                size_t cbLine = pszEol ? (size_t)(pszEol - pszPos)
                                       : strlen(pszPos);
                cbLen += (size_t)sprintf(pszBuf + cbLen,
                                         "SPDX-License-Identifier: ");
                memcpy(pszBuf + cbLen, pszPos, cbLine);
                cbLen += cbLine;
                pszBuf[cbLen++] = '\n';
                if (!pszEol) break;
                pszPos = pszEol + 1;
            }
        }
        break;
    case STYLE_HASH:
    default:
        if (pszCopyright) {
            pszPos = pszCopyright;
            while (*pszPos) {
                PCSZ pszEol = strchr(pszPos, '\n');
                size_t cbLine = pszEol ? (size_t)(pszEol - pszPos)
                                       : strlen(pszPos);
                cbLen += (size_t)sprintf(pszBuf + cbLen,
                                         "# SPDX-FileCopyrightText: ");
                memcpy(pszBuf + cbLen, pszPos, cbLine);
                cbLen += cbLine;
                pszBuf[cbLen++] = '\n';
                if (!pszEol) break;
                pszPos = pszEol + 1;
            }
        }
        if (pszLicense) {
            pszPos = pszLicense;
            while (*pszPos) {
                PCSZ pszEol = strchr(pszPos, '\n');
                size_t cbLine = pszEol ? (size_t)(pszEol - pszPos)
                                       : strlen(pszPos);
                cbLen += (size_t)sprintf(pszBuf + cbLen,
                                         "# SPDX-License-Identifier: ");
                memcpy(pszBuf + cbLen, pszPos, cbLine);
                cbLen += cbLine;
                pszBuf[cbLen++] = '\n';
                if (!pszEol) break;
                pszPos = pszEol + 1;
            }
        }
        cbLen += (size_t)sprintf(pszBuf + cbLen, "\n");
        break;
    }
    return pszBuf;
}

/**
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

/**
 * @brief Query whether the first line is a shebang.
 *
 * @param[in] pszLine  First line. Not NULL.
 *
 * @return TRUE if the line starts with "#!".
 */
static BOOL FirstLineIsShebang(PCSZ pszLine) {
    return (pszLine[0] == '#' && pszLine[1] == '!') ? TRUE : FALSE;
}

/**
 * @brief Query whether the first line is an @echo off statement.
 *
 * @param[in] pszLine  First line. Not NULL.
 *
 * @return TRUE if the line is "@echo off" or "echo off".
 */
static BOOL FirstLineIsEchoOff(PCSZ pszLine) {
    PCSZ pszPos = pszLine;

    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
    if (*pszPos == '@') pszPos++;
    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;

    if (tolower((unsigned char)pszPos[0]) != 'e' ||
        tolower((unsigned char)pszPos[1]) != 'c' ||
        tolower((unsigned char)pszPos[2]) != 'h' ||
        tolower((unsigned char)pszPos[3]) != 'o' ||
        pszPos[4] != ' ' ||
        tolower((unsigned char)pszPos[5]) != 'o' ||
        tolower((unsigned char)pszPos[6]) != 'f' ||
        tolower((unsigned char)pszPos[7]) != 'f')
        return FALSE;

    {
        CHAR ch = pszPos[8];
        if (ch == '\0' || ch == ' ' || ch == '\t' ||
            ch == '\r' || ch == '\n')
            return TRUE;
    }
    return FALSE;
}

/* ------------------------------------------------------------------ */
/* Per-file annotation                                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Annotate one file with SPDX tags.
 *
 * @param[in] pszFilename   File path. Not NULL.
 * @param[in] pszLicense    License expression. Not NULL.
 * @param[in] pszCopyright  Copyright text. Not NULL.
 * @param[in] fForce        TRUE to overwrite existing tags.
 * @param[in] fDryRun       TRUE to skip writes.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     Unknown file type.
 * @retval ERROR_INVALID_DATA       Binary/text mismatch.
 * @retval ERROR_OPEN_FAILED        Cannot write.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET AnnotateOne(PCSZ pszFilename,
                          PCSZ pszLicense,
                          PCSZ pszCopyright,
                          BOOL fForce,
                          BOOL fDryRun) {
    COMMENTSTYLE style;
    COMMENTSTYLE declared;
    BOOL fIsBin;
    PSZ pszBlock;
    CHAR achSidecar[1200];
    FILE *fp, *fpOut;
    CHAR achLine[MAX_LINE];
    CHAR achFirstLine[MAX_LINE];
    BOOL fHas;
    BOOL fHasShebang = FALSE;
    BOOL fHasEchoOff = FALSE;
    CHAR achTempName[1024];

    declared = DetectStyle(pszFilename);

    if (declared == STYLE_UNKNOWN) {
        printf("ERROR: %s: unknown file type.\n"
               "       Fix one of:\n"
               "         - specify the comment style explicitly:\n"
               "           --comment-style=<ext-or-name>="
               "<slash|hash|rem|binary>\n"
               "         - or remove the file from the project.\n",
               pszFilename);
        return ERROR_FILE_NOT_FOUND;
    }

    fIsBin = IsBinaryFile(pszFilename);

    if (declared == STYLE_SIDECAR && !fIsBin) {
        printf("ERROR: %s: declared as binary but content is text.\n"
               "       Fix one of:\n"
               "         - specify the comment style explicitly:\n"
               "           --comment-style=<ext-or-name>=<slash|hash|rem>\n"
               "         - or fix the file if its content is wrong.\n",
               pszFilename);
        return ERROR_INVALID_DATA;
    }

    if (declared != STYLE_SIDECAR && fIsBin) {
        printf("ERROR: %s: declared as text but content is binary.\n"
               "       Fix one of:\n"
               "         - specify the comment style explicitly:\n"
               "           --comment-style=<ext-or-name>=binary\n"
               "         - or fix the file if its content is wrong.\n",
               pszFilename);
        return ERROR_INVALID_DATA;
    }

    style = declared;

    if (style == STYLE_SIDECAR) {
        BOOL fExists;
        BOOL fEqual = FALSE;

        pszBlock = BuildInsertion(style, pszLicense, pszCopyright, FALSE);
        if (!pszBlock) return ERROR_NOT_ENOUGH_MEMORY;

        snprintf(achSidecar, sizeof(achSidecar), "%s.license", pszFilename);
        fExists = FileExists(achSidecar);

        if (fExists) {
            PSZ pszExisting = NULL;
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
    }

    {
        BOOL fHasTag = FALSE;
        SpdxQueryFileHasTag(pszFilename, &fHasTag);
        fHas = fHasTag;
    }

    if (fHas && !fForce) {
        printf("Skipped (has tags):   %s\n", pszFilename);
        return NO_ERROR;
    }

    achFirstLine[0] = '\0';
    fp = fopen(pszFilename, "r");
    if (fp) {
        if (fgets(achFirstLine, sizeof(achFirstLine), fp)) {
        }
        fclose(fp);
    }

    if (style == STYLE_HASH) {
        if (FirstLineIsShebang(achFirstLine)) fHasShebang = TRUE;
    } else if (style == STYLE_REM) {
        if (FirstLineIsEchoOff(achFirstLine)) fHasEchoOff = TRUE;
    }

    pszBlock = BuildInsertion(style, pszLicense, pszCopyright, fHasEchoOff);
    if (!pszBlock) return ERROR_NOT_ENOUGH_MEMORY;

    if (fDryRun) {
        printf("%s %s\n",
               fHas ? "Would update tags:   " : "Would add tags:      ",
               pszFilename);
        printf("    comment style:    %s\n", StyleName(style));
        if (fHasShebang)
            printf("    insertion:        after shebang (line 1)\n");
        else if (fHasEchoOff)
            printf("    insertion:        after @echo off (line 1)\n");
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

    if (fHasShebang || fHasEchoOff) {
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
           fHas ? "Updated tags:        " : "Added tags:          ",
           pszFilename);
    free(pszBlock);
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* LICENSES/ report printing                                           */
/* ------------------------------------------------------------------ */

/**
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

/**
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

/**
 * @brief Entry point of the annotate tool.
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
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
                   "  --cache=<path>             SPDX database cache "
                   "file\n"
                   "  --comment-style=<ext>=<style>\n"
                   "                             Set comment style for a "
                   "given extension\n"
                   "                             or base name. Styles: "
                   "slash, hash, rem, binary\n"
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
        APIRET rc = ReuseTreeOpen(pszDir, &hTree);
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
        ReuseTreeClose(hTree);
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
        ReuseTreeClose(hTree);
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
                APIRET rc;

                if (StrSetEnumGet(hEnum, achFullPath, sizeof(achFullPath),
                                  NULL) != NO_ERROR)
                    continue;

                {
                    REUSELICENSEINFO resolved;
                    memset(&resolved, 0, sizeof(resolved));
                    if (ReuseResolveLicense(hTree, achFullPath,
                                            NULL, NULL, &resolved)
                            == NO_ERROR) {
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
                    size_t cbDirLen = strlen(pszDir);

#ifdef __LINUX__
                    snprintf(achMfPath, sizeof(achMfPath),
                             "%s%smakefile", pszDir,
                             (cbDirLen > 0 &&
                              pszDir[cbDirLen-1] == '/') ? "" : "/");
#else
                    snprintf(achMfPath, sizeof(achMfPath),
                             "%s%smakefile", pszDir,
                             (cbDirLen > 0 &&
                              (pszDir[cbDirLen-1] == '\\' ||
                               pszDir[cbDirLen-1] == '/')) ? "" : "\\");
#endif

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
                                 fForce, fDryRun);
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
    ReuseTreeClose(hTree);
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
