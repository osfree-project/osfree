/* spdx_sbom_opts.c - SBOM command line parsing (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_sbom_opts.h"

/**
 * @file spdx_sbom_opts.c
 * @brief Implementation of the SBOM command line parser.
 */

/* ------------------------------------------------------------------ */
/* List helpers                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief Append a copy of a string range to a string list.
 *
 * @param[in,out] ppapszList  Pointer to the list pointer. Not NULL.
 * @param[in,out] pulCount    Pointer to the element count. Not NULL.
 * @param[in]     pszStr      Pointer to the first character.
 * @param[in]     cbLen       Number of characters to copy.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET add_list_item(PSZ **ppapszList, PULONG pulCount,
                            PCSZ pszStr, size_t cbLen) {
    PSZ pszCopy;
    PSZ *papszNew;

    if (cbLen == 0) return NO_ERROR;

    pszCopy = (PSZ)malloc(cbLen + 1);
    if (!pszCopy) return ERROR_NOT_ENOUGH_MEMORY;
    memcpy(pszCopy, pszStr, cbLen);
    pszCopy[cbLen] = '\0';

    papszNew = (PSZ*)realloc(*ppapszList,
        (size_t)(*pulCount + 1) * sizeof(PSZ));
    if (!papszNew) {
        free(pszCopy);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    *ppapszList = papszNew;
    (*ppapszList)[*pulCount] = pszCopy;
    (*pulCount)++;
    return NO_ERROR;
}

/**
 * @brief Split a whitespace-separated string into a list of tokens.
 *
 * Tokens are separated by spaces or tabs. Quoting is not supported.
 *
 * @param[in]     pszArg       Source string. Not NULL.
 * @param[in,out] ppapszList   Receiver list. Not NULL.
 * @param[in,out] pulCount     Receiver count. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET parse_list(PCSZ pszArg, PSZ **ppapszList, PULONG pulCount) {
    PCSZ pszPos = pszArg;

    while (*pszPos) {
        PCSZ pszStart;
        APIRET rc;

        while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
        if (!*pszPos) break;

        pszStart = pszPos;
        while (*pszPos && *pszPos != ' ' && *pszPos != '\t') pszPos++;

        rc = add_list_item(ppapszList, pulCount, pszStart,
                           (size_t)(pszPos - pszStart));
        if (rc != NO_ERROR) return rc;
    }
    return NO_ERROR;
}

/**
 * @brief Parse the --objects=<list> option.
 *
 * @param[in]     pszArg  Option value. Not NULL.
 * @param[in,out] pOpts   Options structure. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET parse_object_list(PCSZ pszArg, PSBOMOPTIONS pOpts) {
    return parse_list(pszArg, &pOpts->papszObjectFiles,
                      &pOpts->ulObjectCount);
}

/**
 * @brief Parse the --res=<list> option.
 *
 * @param[in]     pszArg  Option value. Not NULL.
 * @param[in,out] pOpts   Options structure. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET parse_res_list(PCSZ pszArg, PSBOMOPTIONS pOpts) {
    return parse_list(pszArg, &pOpts->papszResFiles,
                      &pOpts->ulResCount);
}

/**
 * @brief Parse the --extracted-license=<id>:<path> option.
 *
 * @param[in,out] pOpts   Options structure. Not NULL.
 * @param[in]     pszArg  Option value in the form "<id>:<path>".
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Missing colon or empty id.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET add_extracted(PSBOMOPTIONS pOpts, PCSZ pszArg) {
    PCSZ pszColon = strchr(pszArg, ':');
    size_t cbIdLen;
    PSPDXEXTRACTEDLICENSESOURCE pNew;
    PSPDXEXTRACTEDLICENSESOURCE pSrc;

    if (!pszColon) return ERROR_INVALID_PARAMETER;
    cbIdLen = (size_t)(pszColon - pszArg);
    if (cbIdLen == 0 || cbIdLen >= 256) return ERROR_INVALID_PARAMETER;

    pNew = (PSPDXEXTRACTEDLICENSESOURCE)realloc(pOpts->paExtractedSources,
        (size_t)(pOpts->ulExtractedCount + 1) *
        sizeof(SPDXEXTRACTEDLICENSESOURCE));
    if (!pNew) return ERROR_NOT_ENOUGH_MEMORY;
    pOpts->paExtractedSources = pNew;
    pSrc = &pOpts->paExtractedSources[pOpts->ulExtractedCount];

    memcpy(pSrc->achId, pszArg, cbIdLen);
    pSrc->achId[cbIdLen] = '\0';

    strncpy(pSrc->achPath, pszColon + 1, sizeof(pSrc->achPath) - 1);
    pSrc->achPath[sizeof(pSrc->achPath) - 1] = '\0';

    pOpts->ulExtractedCount++;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Parse the command line.
 *
 * @param[in]  argc   Argument count.
 * @param[in]  argv   Argument vector. Not NULL.
 * @param[out] pOpts  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success, or help was requested.
 * @retval ERROR_INVALID_PARAMETER  pOpts is NULL, an option is
 *                                  rejected, or a required option
 *                                  is missing.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomParseCommandLine(int argc, char *argv[],
                                     PSBOMOPTIONS pOpts) {
    int i;

    if (!pOpts) return ERROR_INVALID_PARAMETER;

    memset(pOpts, 0, sizeof(*pOpts));
    pOpts->pszDir = ".";
    pOpts->pszFormat = "spdx-json";

    for (i = 1; i < argc; i++) {
        PCSZ pszArg = argv[i];

        if (strcmp(pszArg, "--help") == 0 || strcmp(pszArg, "-h") == 0) {
            pOpts->fHelpRequested = TRUE;
            return NO_ERROR;
        }
        else if (strncmp(pszArg, "--output=", 9) == 0)
            pOpts->pszOutput = pszArg + 9;
        else if (strncmp(pszArg, "--format=", 9) == 0)
            pOpts->pszFormat = pszArg + 9;
        else if (strncmp(pszArg, "--default-license=", 18) == 0)
            pOpts->pszDefaultLicense = pszArg + 18;
        else if (strncmp(pszArg, "--default-copyright=", 20) == 0)
            pOpts->pszDefaultCopyright = pszArg + 20;
        else if (strncmp(pszArg, "--name=", 7) == 0)
            pOpts->pszDocName = pszArg + 7;
        else if (strncmp(pszArg, "--version=", 10) == 0)
            pOpts->pszPackageVersion = pszArg + 10;
        else if (strncmp(pszArg, "--supplier=", 11) == 0)
            pOpts->pszPackageSupplier = pszArg + 11;
        else if (strncmp(pszArg, "--creator=", 10) == 0)
            pOpts->pszCreator = pszArg + 10;
        else if (strncmp(pszArg, "--purpose=", 10) == 0)
            pOpts->pszPackagePurpose = pszArg + 10;
        else if (strncmp(pszArg, "--file=", 7) == 0)
            pOpts->pszBinaryFile = pszArg + 7;
        else if (strncmp(pszArg, "--objects=", 10) == 0) {
            APIRET rc = parse_object_list(pszArg + 10, pOpts);
            if (rc != NO_ERROR) {
                pOpts->pszBadOption = pszArg;
                return rc;
            }
        }
        else if (strncmp(pszArg, "--res=", 6) == 0) {
            APIRET rc = parse_res_list(pszArg + 6, pOpts);
            if (rc != NO_ERROR) {
                pOpts->pszBadOption = pszArg;
                return rc;
            }
        }
        else if (strncmp(pszArg, "--source-sbom=", 14) == 0)
            pOpts->pszSourceSbomPath = pszArg + 14;
        else if (strncmp(pszArg, "--spdx-db=", 10) == 0)
            pOpts->pszSpdxDbRoot = pszArg + 10;
        else if (strncmp(pszArg, "--cache=", 8) == 0)
            pOpts->pszCacheFile = pszArg + 8;
        else if (strncmp(pszArg, "--extracted-license=", 20) == 0) {
            APIRET rc = add_extracted(pOpts, pszArg + 20);
            if (rc != NO_ERROR) {
                pOpts->pszBadOption = pszArg;
                return rc;
            }
        }
        else if (strcmp(pszArg, "--no-gitignore") == 0)
            pOpts->fNoGitignore = TRUE;
        else if (pszArg[0] != '-')
            pOpts->pszDir = pszArg;
        else {
            pOpts->pszBadOption = pszArg;
            return ERROR_INVALID_PARAMETER;
        }
    }

    if (!pOpts->pszBinaryFile) {
        pOpts->pszBadOption = "--file";
        return ERROR_INVALID_PARAMETER;
    }
    if (!pOpts->pszDocName) {
        pOpts->pszBadOption = "--name";
        return ERROR_INVALID_PARAMETER;
    }
    if (!pOpts->pszSpdxDbRoot) {
        pOpts->pszBadOption = "--spdx-db";
        return ERROR_INVALID_PARAMETER;
    }
    if (strcmp(pOpts->pszFormat, "spdx-json") != 0 &&
        strcmp(pOpts->pszFormat, "spdx-tag") != 0) {
        pOpts->pszBadOption = "--format";
        return ERROR_INVALID_PARAMETER;
    }
    return NO_ERROR;
}

/**
 * @brief Release memory owned by an options structure.
 *
 * @param[in,out] pOpts  Options. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pOpts is NULL.
 */
APIRET APIENTRY SbomFreeOptions(PSBOMOPTIONS pOpts) {
    ULONG ulIdx;

    if (!pOpts) return ERROR_INVALID_PARAMETER;

    if (pOpts->papszObjectFiles) {
        for (ulIdx = 0; ulIdx < pOpts->ulObjectCount; ulIdx++)
            free(pOpts->papszObjectFiles[ulIdx]);
        free(pOpts->papszObjectFiles);
    }
    if (pOpts->papszResFiles) {
        for (ulIdx = 0; ulIdx < pOpts->ulResCount; ulIdx++)
            free(pOpts->papszResFiles[ulIdx]);
        free(pOpts->papszResFiles);
    }
    free(pOpts->paExtractedSources);

    pOpts->papszObjectFiles = NULL;
    pOpts->ulObjectCount = 0;
    pOpts->papszResFiles = NULL;
    pOpts->ulResCount = 0;
    pOpts->paExtractedSources = NULL;
    pOpts->ulExtractedCount = 0;
    return NO_ERROR;
}
