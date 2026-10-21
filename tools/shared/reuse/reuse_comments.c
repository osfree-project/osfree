/*!
 * @file reuse_comments.c
 *
 * @brief Implementation of the REUSE 3.3 comment style configuration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "reuse.h"
#include "reuse_glob.h"
#include "path.h"
#include "toml.h"

/*!
 * @def REUSECOMMENTSCFG_MAGIC
 * @brief Magic value identifying a valid config handle.
 */
#define REUSECOMMENTSCFG_MAGIC  0x52434D54UL  /* "RCMT" */

/*!
 * @def REUSECOMMENTSTYLE_MAGIC
 * @brief Magic value identifying a valid style handle.
 */
#define REUSECOMMENTSTYLE_MAGIC 0x52535459UL  /* "RSTY" */

/* ==================================================================
 * Internal types
 * ================================================================== */

/*!
 * @struct _REUSECOMMENTSTYLE
 * @brief One parsed comment style.
 */
typedef struct _REUSECOMMENTSTYLE {
    ULONG ulMagic;
    PSZ   pszName;
    PSZ   pszSingleLine;
    PSZ   pszSingleLineRegexp;     /*!< Alternative to pszSingleLine.  */
    PSZ   pszIndentAfterSingle;
    PSZ   pszMultiStart;
    PSZ   pszMultiMiddle;
    PSZ   pszMultiEnd;
    PSZ   pszIndentBeforeMiddle;
    PSZ   pszIndentAfterMiddle;
    PSZ   pszIndentBeforeEnd;
    PSZ  *papszShebangs;
    ULONG ulShebangCount;
    BOOL  fCaseInsensitive;        /*!< Applies to single / regexp.    */
} REUSECOMMENTSTYLE, *PREUSECOMMENTSTYLE;

/*!
 * @struct _REUSECOMMENTSMAP
 * @brief One glob-to-style mapping entry.
 */
typedef struct _REUSECOMMENTSMAP {
    PSZ   pszGlob;
    ULONG ulStyleIndex;
} REUSECOMMENTSMAP, *PREUSECOMMENTSMAP;

/*!
 * @struct _REUSECOMMENTSCFG
 * @brief Parsed comments.toml configuration.
 */
typedef struct _REUSECOMMENTSCFG {
    ULONG              ulMagic;
    REUSECOMMENTSTYLE *paStyles;
    ULONG              ulStyleCount;
    ULONG              ulStyleCapacity;
    REUSECOMMENTSMAP  *paMaps;
    ULONG              ulMapCount;
    ULONG              ulMapCapacity;
} REUSECOMMENTSCFG, *PREUSECOMMENTSCFG;

/* ==================================================================
 * Minimal regular-expression matcher (prefix-anchored)
 *
 * Supported: '^' (no-op anchor), '.', '[...]' (with '^'/'!' negate
 * and a-z ranges), '*', '+', '?', '\\b' (word boundary at the end
 * of a word), '\\X' (literal X), any other character is literal.
 *
 * Not supported: '$', '{n,m}', '|', '()', backreferences, lookaround.
 * Anchored at the start of the text (equivalent to leading '^').
 *
 * Atoms under a quantifier must match exactly one text character;
 * this is true for all marker patterns used in practice.
 * ================================================================== */

/*!
 * @brief Case-aware character compare.
 *
 * @param[in] chA  First character.
 * @param[in] chB  Second character.
 * @param[in] fCI  TRUE for case-insensitive comparison.
 *
 * @return BOOL
 *
 * @retval TRUE   chA and chB are equal under the active rule.
 * @retval FALSE  Otherwise.
 */
static BOOL re_char_eq(char chA, char chB, BOOL fCI) {
    if (fCI)
        return tolower((unsigned char)chA) == tolower((unsigned char)chB)
                   ? TRUE : FALSE;
    return (chA == chB) ? TRUE : FALSE;
}

/*!
 * @brief Word character test (used by \b).
 *
 * @param[in] ch  Character.
 *
 * @return BOOL
 *
 * @retval TRUE   ch is alphanumeric or '_'.
 * @retval FALSE  Otherwise.
 */
static BOOL re_is_word(char ch) {
    return (isalnum((unsigned char)ch) || ch == '_') ? TRUE : FALSE;
}

/*!
 * @brief Return the byte length in the pattern of the atom at pszPat.
 *
 * @param[in] pszPat  Start of an atom. Not NULL.
 *
 * @return ULONG
 *
 * @return Length in bytes (>= 1).
 */
static ULONG re_atom_len(PCSZ pszPat) {
    ULONG ulLen;

    if (*pszPat == '\\') {
        if (pszPat[1] == '\0') ulLen = 1;
        else ulLen = 2;
    } else if (*pszPat == '[') {
        PCSZ p = pszPat + 1;
        if (*p == '^' || *p == '!') p++;
        while (*p && *p != ']') {
            if (p[1] == '-' && p[2] && p[2] != ']') p += 3;
            else p++;
        }
        if (*p == ']') ulLen = (ULONG)(p - pszPat) + 1;
        else ulLen = 1;
    } else {
        ulLen = 1;
    }
    return ulLen;
}

/*!
 * @brief Match one atom at the start of pszText.
 *
 * @param[in]  pszPat   Start of atom. Not NULL.
 * @param[in]  pszText  Input text. Not NULL.
 * @param[in]  fCI      Case-insensitive flag.
 * @param[out] pcbPat   Bytes consumed from pattern. Not NULL.
 * @param[out] pcbText  Bytes consumed from text. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   The atom matches.
 * @retval FALSE  The atom does not match.
 */
static BOOL re_match_atom(PCSZ pszPat, PCSZ pszText, BOOL fCI,
                          PULONG pcbPat, PULONG pcbText) {
    *pcbPat = 0;
    *pcbText = 0;

    if (*pszPat == '\\') {
        char n = pszPat[1];
        if (n == '\0') return FALSE;
        if (n == 'b') {
            if (*pszText && re_is_word(*pszText)) return FALSE;
            *pcbPat = 2;
            *pcbText = 0;
            return TRUE;
        }
        if (*pszText == '\0') return FALSE;
        if (!re_char_eq(*pszText, n, fCI)) return FALSE;
        *pcbPat = 2;
        *pcbText = 1;
        return TRUE;
    }

    if (*pszPat == '.') {
        if (*pszText == '\0') return FALSE;
        *pcbPat = 1;
        *pcbText = 1;
        return TRUE;
    }

    if (*pszPat == '[') {
        PCSZ p = pszPat + 1;
        BOOL fNeg = FALSE;
        BOOL fFound = FALSE;
        char cur = *pszText;
        char curCmp;

        if (*p == '^' || *p == '!') { fNeg = TRUE; p++; }

        if (cur == '\0') {
            while (*p && *p != ']') {
                if (p[1] == '-' && p[2] && p[2] != ']') p += 3;
                else p++;
            }
            return FALSE;
        }

        curCmp = fCI ? (char)tolower((unsigned char)cur) : cur;
        while (*p && *p != ']') {
            if (p[1] == '-' && p[2] && p[2] != ']') {
                char loC = fCI ? (char)tolower((unsigned char)p[0]) : p[0];
                char hiC = fCI ? (char)tolower((unsigned char)p[2]) : p[2];
                if (curCmp >= loC && curCmp <= hiC) fFound = TRUE;
                p += 3;
            } else {
                if (re_char_eq(cur, *p, fCI)) fFound = TRUE;
                p++;
            }
        }
        if (*p != ']') return FALSE;
        if (fNeg) fFound = fFound ? FALSE : TRUE;
        if (!fFound) return FALSE;
        *pcbPat = (ULONG)(p - pszPat) + 1;
        *pcbText = 1;
        return TRUE;
    }

    if (*pszText == '\0') return FALSE;
    if (!re_char_eq(*pszText, *pszPat, fCI)) return FALSE;
    *pcbPat = 1;
    *pcbText = 1;
    return TRUE;
}

/*!
 * @brief Match the remaining pattern at the start of pszText.
 *
 * @param[in]  pszPat      Pattern. Not NULL.
 * @param[in]  pszText     Text. Not NULL.
 * @param[in]  fCI         Case-insensitive flag.
 * @param[out] pulConsumed Text bytes consumed on success. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   The remaining pattern matches from pszText.
 * @retval FALSE  It does not.
 */
static BOOL re_match_here(PCSZ pszPat, PCSZ pszText, BOOL fCI,
                          PULONG pulConsumed) {
    PCSZ pszTextStart = pszText;

    while (*pszPat != '\0') {
        ULONG cbAtomLen;
        char  quant;

        if (*pszPat == '^') { pszPat++; continue; }

        cbAtomLen = re_atom_len(pszPat);
        quant = pszPat[cbAtomLen];

        if (quant == '*' || quant == '+' || quant == '?') {
            ULONG ulMin = (quant == '+') ? 1 : 0;
            ULONG ulMax = (quant == '?') ? 1 : 0xFFFFFFFFUL;
            PCSZ  pszAtom = pszPat;
            PCSZ  pszRest = pszPat + cbAtomLen + 1;
            PCSZ  pszRun = pszText;
            ULONG ulMatched = 0;
            ULONG cbP, cbT;
            ULONG k;

            while (ulMatched < ulMax) {
                if (!re_match_atom(pszAtom, pszRun, fCI, &cbP, &cbT))
                    break;
                if (cbT == 0) break;
                pszRun += cbT;
                ulMatched++;
            }
            if (ulMatched < ulMin) return FALSE;

            for (k = ulMatched + 1; k-- > ulMin; ) {
                ULONG ulRest = 0;
                if (re_match_here(pszRest, pszRun, fCI, &ulRest)) {
                    *pulConsumed = (ULONG)(pszRun - pszTextStart) + ulRest;
                    return TRUE;
                }
                if (k == ulMin) break;
                pszRun--;
            }
            return FALSE;
        }

        {
            ULONG cbPat = 0, cbText = 0;
            if (!re_match_atom(pszPat, pszText, fCI, &cbPat, &cbText))
                return FALSE;
            pszPat += cbPat;
            pszText += cbText;
        }
    }

    *pulConsumed = (ULONG)(pszText - pszTextStart);
    return TRUE;
}

/*!
 * @brief Match a regexp against the prefix of a text.
 *
 * @param[in]  pszPattern  Pattern. Not NULL.
 * @param[in]  pszText     Text. Not NULL.
 * @param[in]  fCI         Case-insensitive flag.
 * @param[out] pulMatched  Receiver for the matched text length.
 *                         Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   The pattern matches at the start of the text.
 * @retval FALSE  It does not.
 */
static BOOL re_match_prefix(PCSZ pszPattern, PCSZ pszText,
                            BOOL fCI, PULONG pulMatched) {
    ULONG ulConsumed = 0;
    if (!pszPattern || !pszText || !pulMatched) return FALSE;
    if (!re_match_here(pszPattern, pszText, fCI, &ulConsumed))
        return FALSE;
    *pulMatched = ulConsumed;
    return TRUE;
}

/* ==================================================================
 * Handle accessors
 * ================================================================== */

/*!
 * @brief Return the internal config pointer, or NULL when the magic
 *        does not match.
 *
 * @param[in] hCfg  Handle. May be NULLHANDLE.
 *
 * @return PREUSECOMMENTSCFG
 *
 * @retval NULL  Handle is NULLHANDLE or not a config handle.
 * @return       Internal pointer.
 */
static PREUSECOMMENTSCFG as_cfg(HREUSECOMMENTSCFG hCfg) {
    PREUSECOMMENTSCFG pCfg = (PREUSECOMMENTSCFG)hCfg;
    if (pCfg == NULL) return NULL;
    if (pCfg->ulMagic != REUSECOMMENTSCFG_MAGIC) return NULL;
    return pCfg;
}

/*!
 * @brief Return the internal style pointer, or NULL when the magic
 *        does not match.
 *
 * @param[in] hStyle  Handle. May be NULLHANDLE.
 *
 * @return PREUSECOMMENTSTYLE
 *
 * @retval NULL  Handle is NULLHANDLE or not a style handle.
 * @return       Internal pointer.
 */
static PREUSECOMMENTSTYLE as_style(HREUSECOMMENTSTYLE hStyle) {
    PREUSECOMMENTSTYLE pStyle = (PREUSECOMMENTSTYLE)hStyle;
    if (pStyle == NULL) return NULL;
    if (pStyle->ulMagic != REUSECOMMENTSTYLE_MAGIC) return NULL;
    return pStyle;
}

/* ==================================================================
 * Small helpers
 * ================================================================== */

/*!
 * @brief Case-insensitive prefix match.
 *
 * @param[in] pszStr     Haystack. Not NULL.
 * @param[in] pszPrefix  Prefix. Not NULL.
 * @param[in] cbLen      Prefix length.
 *
 * @return BOOL
 *
 * @retval TRUE   pszStr starts with pszPrefix.
 * @retval FALSE  Otherwise, or pszStr is shorter than pszPrefix.
 */
static BOOL case_insensitive_prefix(PCSZ pszStr, PCSZ pszPrefix,
                                    size_t cbLen) {
    size_t i;
    for (i = 0; i < cbLen; i++) {
        if (pszStr[i] == '\0') return FALSE;
        if (tolower((unsigned char)pszStr[i]) !=
            tolower((unsigned char)pszPrefix[i]))
            return FALSE;
    }
    return TRUE;
}

/*!
 * @brief Free a NULL-terminated string array and the array itself.
 *
 * @param[in] papszList  Array or NULL.
 */
static void free_string_array(PSZ *papszList) {
    ULONG i;
    if (papszList == NULL) return;
    for (i = 0; papszList[i] != NULL; i++)
        free(papszList[i]);
    free(papszList);
}

/*!
 * @brief Free one parsed style.
 *
 * @param[in] pStyle  Style or NULL.
 */
static void free_style(PREUSECOMMENTSTYLE pStyle) {
    if (pStyle == NULL) return;
    free(pStyle->pszName);
    free(pStyle->pszSingleLine);
    free(pStyle->pszSingleLineRegexp);
    free(pStyle->pszIndentAfterSingle);
    free(pStyle->pszMultiStart);
    free(pStyle->pszMultiMiddle);
    free(pStyle->pszMultiEnd);
    free(pStyle->pszIndentBeforeMiddle);
    free(pStyle->pszIndentAfterMiddle);
    free(pStyle->pszIndentBeforeEnd);
    free_string_array(pStyle->papszShebangs);
}

/*!
 * @brief Free the whole config.
 *
 * @param[in] pCfg  Config or NULL.
 */
static void free_cfg(PREUSECOMMENTSCFG pCfg) {
    ULONG i;
    if (pCfg == NULL) return;
    for (i = 0; i < pCfg->ulStyleCount; i++)
        free_style(&pCfg->paStyles[i]);
    free(pCfg->paStyles);
    for (i = 0; i < pCfg->ulMapCount; i++)
        free(pCfg->paMaps[i].pszGlob);
    free(pCfg->paMaps);
    free(pCfg);
}

/* ==================================================================
 * TOML field readers
 * ================================================================== */

/*!
 * @brief Read a string field from a TOML table.
 *
 * @param[in] hTable  TOML table. Not NULLHANDLE.
 * @param[in] pszKey  Field name. Not NULL.
 *
 * @return PSZ
 *
 * @retval NULL  Field absent, wrong type, or allocation failure.
 * @return       malloc'd value, caller frees with free().
 */
static PSZ rc_get_string(HTOMLNODE hTable, PCSZ pszKey) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulSize = 0;
    PSZ pszBuf;

    if (TomlNodeGetTableEntryByKey(hTable, pszKey, &hChild) != NO_ERROR)
        return NULL;
    if (TomlNodeGetString(hChild, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszBuf = (PSZ)malloc(ulSize);
    if (pszBuf == NULL) return NULL;
    if (TomlNodeGetString(hChild, pszBuf, ulSize, NULL) != NO_ERROR) {
        free(pszBuf);
        return NULL;
    }
    return pszBuf;
}

/*!
 * @brief Read a boolean field from a TOML table.
 *
 * Absent field, wrong type, or read error yields FALSE.
 *
 * @param[in] hTable  TOML table. Not NULLHANDLE.
 * @param[in] pszKey  Field name. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Field present, of type BOOLEAN, and TRUE.
 * @retval FALSE  Field absent, wrong type, FALSE, or read error.
 */
static BOOL rc_get_bool(HTOMLNODE hTable, PCSZ pszKey) {
    HTOMLNODE hChild = NULLHANDLE;
    BOOL fValue = FALSE;

    if (TomlNodeGetTableEntryByKey(hTable, pszKey, &hChild) != NO_ERROR)
        return FALSE;
    if (TomlNodeGetBoolean(hChild, &fValue) != NO_ERROR)
        return FALSE;
    return fValue;
}

/*!
 * @brief Read a string array field from a TOML table.
 *
 * @param[in]  hTable    TOML table. Not NULLHANDLE.
 * @param[in]  pszKey    Field name. Not NULL.
 * @param[out] pppList   Receiver for the NULL-terminated array. Not
 *                       NULL.
 * @param[out] pulCount  Receiver for the number of entries. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (possibly empty).
 * @retval ERROR_INVALID_DATA       Field is not an array of strings.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET rc_get_string_array(HTOMLNODE hTable, PCSZ pszKey,
                                  PSZ **pppList, PULONG pulCount) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0;
    ULONG ulCount = 0;
    ULONG ulIdx;
    PSZ *ppList;

    *pppList = NULL;
    *pulCount = 0;

    if (TomlNodeGetTableEntryByKey(hTable, pszKey, &hChild) != NO_ERROR)
        return NO_ERROR;
    if (TomlNodeGetType(hChild, &ulType) != NO_ERROR)
        return ERROR_INVALID_DATA;
    if (ulType != TOML_TYPE_ARRAY)
        return ERROR_INVALID_DATA;
    if (TomlNodeGetArrayCount(hChild, &ulCount) != NO_ERROR)
        return ERROR_INVALID_DATA;
    if (ulCount == 0) return NO_ERROR;

    ppList = (PSZ *)calloc(ulCount + 1, sizeof(PSZ));
    if (ppList == NULL) return ERROR_NOT_ENOUGH_MEMORY;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HTOMLNODE hElem = NULLHANDLE;
        ULONG ulElemType = 0;
        ULONG ulElemSize = 0;

        if (TomlNodeGetArrayElement(hChild, ulIdx, &hElem) != NO_ERROR) {
            free_string_array(ppList);
            return ERROR_INVALID_DATA;
        }
        if (TomlNodeGetType(hElem, &ulElemType) != NO_ERROR ||
            ulElemType != TOML_TYPE_STRING) {
            free_string_array(ppList);
            return ERROR_INVALID_DATA;
        }
        if (TomlNodeGetString(hElem, NULL, 0, &ulElemSize) != NO_ERROR) {
            free_string_array(ppList);
            return ERROR_INVALID_DATA;
        }
        if (ulElemSize == 0) continue;
        ppList[ulIdx] = (PSZ)malloc(ulElemSize);
        if (ppList[ulIdx] == NULL) {
            free_string_array(ppList);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        if (TomlNodeGetString(hElem, ppList[ulIdx], ulElemSize, NULL)
                != NO_ERROR) {
            free_string_array(ppList);
            return ERROR_INVALID_DATA;
        }
    }

    *pppList = ppList;
    *pulCount = ulCount;
    return NO_ERROR;
}

/* ==================================================================
 * Config parsing
 * ================================================================== */

/*!
 * @brief Populate one style record from its [[style]] table.
 *
 * @param[in]  hItem   TOML table. Not NULLHANDLE.
 * @param[out] pStyle  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_DATA       Missing required "name", or both
 *                                  "single" and "single_line_regexp"
 *                                  are present.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET parse_one_style(HTOMLNODE hItem, PREUSECOMMENTSTYLE pStyle) {
    APIRET rc;

    pStyle->ulMagic = REUSECOMMENTSTYLE_MAGIC;
    pStyle->pszName = rc_get_string(hItem, "name");
    if (pStyle->pszName == NULL || pStyle->pszName[0] == '\0')
        return ERROR_INVALID_DATA;

    pStyle->pszSingleLine         = rc_get_string(hItem, "single");
    pStyle->pszSingleLineRegexp   = rc_get_string(hItem, "single_line_regexp");
    pStyle->pszIndentAfterSingle  = rc_get_string(hItem, "indent_after_single");
    pStyle->pszMultiStart         = rc_get_string(hItem, "multi_start");
    pStyle->pszMultiMiddle        = rc_get_string(hItem, "multi_middle");
    pStyle->pszMultiEnd           = rc_get_string(hItem, "multi_end");
    pStyle->pszIndentBeforeMiddle = rc_get_string(hItem, "indent_before_middle");
    pStyle->pszIndentAfterMiddle  = rc_get_string(hItem, "indent_after_middle");
    pStyle->pszIndentBeforeEnd    = rc_get_string(hItem, "indent_before_end");

    pStyle->fCaseInsensitive = rc_get_bool(hItem, "case_insensitive");

    if (pStyle->pszSingleLine != NULL &&
        pStyle->pszSingleLineRegexp != NULL)
        return ERROR_INVALID_DATA;

    rc = rc_get_string_array(hItem, "shebangs",
                             &pStyle->papszShebangs,
                             &pStyle->ulShebangCount);
    return rc;
}

/*!
 * @brief Parse all [[style]] entries, rejecting duplicate names.
 *
 * @param[in]  hDoc  TOML document. Not NULLHANDLE.
 * @param[out] pCfg  Config. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_DATA       Parse error or duplicate name.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET parse_styles(HTOMLDOC hDoc, PREUSECOMMENTSCFG pCfg) {
    HTOMLNODE hArr = NULLHANDLE;
    ULONG ulCount = 0;
    ULONG ulIdx;
    ULONG ulJ;
    APIRET rc;

    rc = TomlQueryNode(hDoc, "style", &hArr);
    if (rc != NO_ERROR) return NO_ERROR;
    if (TomlNodeGetArrayCount(hArr, &ulCount) != NO_ERROR)
        return ERROR_INVALID_DATA;
    if (ulCount == 0) return NO_ERROR;

    pCfg->paStyles = (REUSECOMMENTSTYLE *)calloc(ulCount,
                                                 sizeof(REUSECOMMENTSTYLE));
    if (pCfg->paStyles == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pCfg->ulStyleCapacity = ulCount;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HTOMLNODE hItem = NULLHANDLE;
        PREUSECOMMENTSTYLE pStyle = &pCfg->paStyles[ulIdx];

        if (TomlNodeGetArrayElement(hArr, ulIdx, &hItem) != NO_ERROR)
            return ERROR_INVALID_DATA;

        rc = parse_one_style(hItem, pStyle);
        if (rc != NO_ERROR) return rc;

        pCfg->ulStyleCount++;

        for (ulJ = 0; ulJ + 1 < pCfg->ulStyleCount; ulJ++) {
            if (strcmp(pCfg->paStyles[ulJ].pszName, pStyle->pszName) == 0)
                return ERROR_INVALID_DATA;
        }
    }
    return NO_ERROR;
}

/*!
 * @brief Locate a style index by name.
 *
 * @param[in]  pCfg      Config. Not NULL.
 * @param[in]  pszName   Style name. Not NULL.
 * @param[out] pulIndex  Receiver for the index. Not NULL.
 * @param[out] pfFound   Receiver for the found flag. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Always (check *pfFound).
 */
static APIRET find_style_index(PREUSECOMMENTSCFG pCfg, PCSZ pszName,
                               PULONG pulIndex, PBOOL pfFound) {
    ULONG i;
    *pfFound = FALSE;
    for (i = 0; i < pCfg->ulStyleCount; i++) {
        if (strcmp(pCfg->paStyles[i].pszName, pszName) == 0) {
            *pulIndex = i;
            *pfFound = TRUE;
            return NO_ERROR;
        }
    }
    return NO_ERROR;
}

/*!
 * @brief Parse all [[map]] entries, resolving each style name to a
 *        style index.
 *
 * @param[in]  hDoc  TOML document. Not NULLHANDLE.
 * @param[out] pCfg  Config. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_DATA       Parse error or unknown style name.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET parse_maps(HTOMLDOC hDoc, PREUSECOMMENTSCFG pCfg) {
    HTOMLNODE hArr = NULLHANDLE;
    ULONG ulCount = 0;
    ULONG ulIdx;
    APIRET rc;

    rc = TomlQueryNode(hDoc, "map", &hArr);
    if (rc != NO_ERROR) return NO_ERROR;
    if (TomlNodeGetArrayCount(hArr, &ulCount) != NO_ERROR)
        return ERROR_INVALID_DATA;
    if (ulCount == 0) return NO_ERROR;

    pCfg->paMaps = (REUSECOMMENTSMAP *)calloc(ulCount,
                                              sizeof(REUSECOMMENTSMAP));
    if (pCfg->paMaps == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pCfg->ulMapCapacity = ulCount;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HTOMLNODE hItem = NULLHANDLE;
        PREUSECOMMENTSMAP pMap = &pCfg->paMaps[ulIdx];
        PSZ pszStyle;
        BOOL fFound = FALSE;

        if (TomlNodeGetArrayElement(hArr, ulIdx, &hItem) != NO_ERROR)
            return ERROR_INVALID_DATA;

        pMap->pszGlob = rc_get_string(hItem, "glob");
        if (pMap->pszGlob == NULL || pMap->pszGlob[0] == '\0')
            return ERROR_INVALID_DATA;

        pszStyle = rc_get_string(hItem, "style");
        if (pszStyle == NULL || pszStyle[0] == '\0') {
            free(pszStyle);
            return ERROR_INVALID_DATA;
        }

        rc = find_style_index(pCfg, pszStyle, &pMap->ulStyleIndex, &fFound);
        free(pszStyle);
        if (rc != NO_ERROR) return rc;
        if (!fFound) return ERROR_INVALID_DATA;

        pCfg->ulMapCount++;
    }
    return NO_ERROR;
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Open and parse a comments.toml file.
 *
 * @param[in]  pszConfigPath  Path to comments.toml. Not NULL.
 * @param[out] phCfg          Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_INVALID_DATA       Parse error or semantic error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseOpenComments(PCSZ pszConfigPath,
                                  HREUSECOMMENTSCFG *phCfg) {
    HTOMLDOC hDoc = NULLHANDLE;
    PREUSECOMMENTSCFG pCfg;
    APIRET rc;

    if (pszConfigPath == NULL || phCfg == NULL)
        return ERROR_INVALID_PARAMETER;
    *phCfg = NULLHANDLE;

    pCfg = (PREUSECOMMENTSCFG)calloc(1, sizeof(REUSECOMMENTSCFG));
    if (pCfg == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pCfg->ulMagic = REUSECOMMENTSCFG_MAGIC;

    rc = TomlOpen(pszConfigPath, &hDoc);
    if (rc == ERROR_OPEN_FAILED) {
        free_cfg(pCfg);
        return ERROR_OPEN_FAILED;
    }
    if (rc == ERROR_READ_FAULT) {
        free_cfg(pCfg);
        return ERROR_OPEN_FAILED;
    }
    if (rc == ERROR_NOT_ENOUGH_MEMORY) {
        free_cfg(pCfg);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    if (rc != NO_ERROR) {
        free_cfg(pCfg);
        return ERROR_INVALID_DATA;
    }

    rc = parse_styles(hDoc, pCfg);
    if (rc == NO_ERROR)
        rc = parse_maps(hDoc, pCfg);

    TomlClose(hDoc);

    if (rc != NO_ERROR) {
        free_cfg(pCfg);
        return rc;
    }

    *phCfg = (HREUSECOMMENTSCFG)pCfg;
    return NO_ERROR;
}

/*!
 * @brief Close a config and release all styles.
 *
 * @param[in] hCfg  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY ReuseCloseComments(HREUSECOMMENTSCFG hCfg) {
    PREUSECOMMENTSCFG pCfg;
    if (hCfg == NULLHANDLE) return NO_ERROR;
    pCfg = as_cfg(hCfg);
    if (pCfg == NULL) return ERROR_INVALID_HANDLE;
    pCfg->ulMagic = 0;
    free_cfg(pCfg);
    return NO_ERROR;
}

/* ==================================================================
 * Style resolution
 * ================================================================== */

/*!
 * @brief Resolve the comment style for a file by [[map]] glob.
 *
 * @param[in]  hCfg         Config. Not NULLHANDLE.
 * @param[in]  pszFilePath  Path to the file. Not NULL.
 * @param[out] pfFound      TRUE if a [[map]] matched. Not NULL.
 * @param[out] phStyle      Borrowed style handle on match. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hCfg is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure while
 *                                  normalizing the path.
 */
APIRET APIENTRY ReuseQueryCommentStyle(HREUSECOMMENTSCFG hCfg,
                                       PCSZ pszFilePath,
                                       PBOOL pfFound,
                                       HREUSECOMMENTSTYLE *phStyle) {
    PREUSECOMMENTSCFG pCfg;
    PCSZ pszBase = NULL;
    PSZ pszNorm = NULL;
    ULONG ulNormSize = 0;
    ULONG i;
    APIRET rc;

    if (!pfFound || !phStyle || pszFilePath == NULL)
        return ERROR_INVALID_PARAMETER;
    *pfFound = FALSE;
    *phStyle = NULLHANDLE;

    pCfg = as_cfg(hCfg);
    if (pCfg == NULL) return ERROR_INVALID_HANDLE;

    rc = PathGetBaseName(pszFilePath, &pszBase);
    if (rc != NO_ERROR) return rc;

    rc = PathNormalize(pszFilePath, NULL, 0, &ulNormSize);
    if (rc != NO_ERROR) return rc;
    if (ulNormSize > 0) {
        pszNorm = (PSZ)malloc(ulNormSize);
        if (pszNorm == NULL) return ERROR_NOT_ENOUGH_MEMORY;
        rc = PathNormalize(pszFilePath, pszNorm, ulNormSize, NULL);
        if (rc != NO_ERROR) {
            free(pszNorm);
            return rc;
        }
    }

    for (i = 0; i < pCfg->ulMapCount; i++) {
        BOOL fMatch = FALSE;
        BOOL fFullMatch = FALSE;

        rc = ReuseQueryGlobMatch(pCfg->paMaps[i].pszGlob, pszBase, &fMatch);
        if (rc != NO_ERROR) {
            free(pszNorm);
            return rc;
        }
        if (!fMatch && pszNorm != NULL) {
            rc = ReuseQueryGlobMatch(pCfg->paMaps[i].pszGlob, pszNorm,
                                     &fFullMatch);
            if (rc != NO_ERROR) {
                free(pszNorm);
                return rc;
            }
        }

        if (fMatch || fFullMatch) {
            *pfFound = TRUE;
            *phStyle = (HREUSECOMMENTSTYLE)
                       &pCfg->paStyles[pCfg->paMaps[i].ulStyleIndex];
            free(pszNorm);
            return NO_ERROR;
        }
    }

    free(pszNorm);
    return NO_ERROR;
}

/*!
 * @brief Resolve a style by its configured name.
 *
 * @param[in]  hCfg     Config. Not NULLHANDLE.
 * @param[in]  pszName  Style name. Not NULL.
 * @param[out] pfFound  TRUE if the name is defined. Not NULL.
 * @param[out] phStyle  Borrowed style handle on match. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hCfg is not recognized.
 */
APIRET APIENTRY ReuseQueryCommentStyleByName(HREUSECOMMENTSCFG hCfg,
                                             PCSZ pszName,
                                             PBOOL pfFound,
                                             HREUSECOMMENTSTYLE *phStyle) {
    PREUSECOMMENTSCFG pCfg;
    ULONG ulIndex = 0;
    BOOL fFound = FALSE;
    APIRET rc;

    if (!pszName || !pfFound || !phStyle)
        return ERROR_INVALID_PARAMETER;
    *pfFound = FALSE;
    *phStyle = NULLHANDLE;

    pCfg = as_cfg(hCfg);
    if (pCfg == NULL) return ERROR_INVALID_HANDLE;

    rc = find_style_index(pCfg, pszName, &ulIndex, &fFound);
    if (rc != NO_ERROR) return rc;
    if (!fFound) return NO_ERROR;

    *pfFound = TRUE;
    *phStyle = (HREUSECOMMENTSTYLE)&pCfg->paStyles[ulIndex];
    return NO_ERROR;
}

/* ==================================================================
 * Style queries
 * ================================================================== */

/*!
 * @brief Query static properties of a style.
 *
 * @param[in]  hStyle  Borrowed style. Not NULLHANDLE.
 * @param[out] pInfo   Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pInfo is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 */
APIRET APIENTRY ReuseQueryCommentStyleInfo(HREUSECOMMENTSTYLE hStyle,
                                           PREUSECOMMENTSTYLEINFO pInfo) {
    PREUSECOMMENTSTYLE pStyle;

    if (pInfo == NULL) return ERROR_INVALID_PARAMETER;
    pStyle = as_style(hStyle);
    if (pStyle == NULL) return ERROR_INVALID_HANDLE;

    pInfo->pszName = pStyle->pszName;
    pInfo->fSingleLine = (pStyle->pszSingleLine != NULL ||
                          pStyle->pszSingleLineRegexp != NULL) ? TRUE : FALSE;
    pInfo->fMultiLine = (pStyle->pszMultiStart != NULL &&
                         pStyle->pszMultiEnd != NULL) ? TRUE : FALSE;
    pInfo->fSidecar = (!pInfo->fSingleLine && !pInfo->fMultiLine)
                          ? TRUE : FALSE;
    return NO_ERROR;
}

/* ==================================================================
 * Style operations
 * ================================================================== */

/*!
 * @brief Locate the body of a single line after stripping the leading
 *        comment marker.
 *
 * Single-line styles are tried in this order:
 *   1. If pszSingleLineRegexp is set, match it via re_match_prefix.
 *   2. Otherwise, if pszSingleLine is set, match it literally
 *      (case-insensitively if fCaseInsensitive).
 *
 * When a single-line marker matches, indent_after_single is skipped
 * (if present) and the offset of the body is returned.
 *
 * Multi-line markers (multi_start, multi_middle) are then tried as
 * before.
 *
 * @param[in]  hStyle     Borrowed style. Not NULLHANDLE.
 * @param[in]  pszLine    Input line. Not NULL.
 * @param[out] pulOffset  Byte offset in pszLine. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Marker found; *pulOffset is set.
 * @retval ERROR_FILE_NOT_FOUND     Line is not a comment.
 * @retval ERROR_INVALID_PARAMETER  Argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 */
APIRET APIENTRY ReuseQueryCommentBody(HREUSECOMMENTSTYLE hStyle,
                                      PCSZ pszLine,
                                      PULONG pulOffset) {
    PREUSECOMMENTSTYLE pStyle;
    PCSZ pszPos;
    size_t cbMarker;

    if (!pszLine || !pulOffset) return ERROR_INVALID_PARAMETER;
    pStyle = as_style(hStyle);
    if (pStyle == NULL) return ERROR_INVALID_HANDLE;

    pszPos = pszLine;
    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;

    /* Single-line, regexp form. */
    if (pStyle->pszSingleLineRegexp != NULL) {
        ULONG ulMatched = 0;
        if (re_match_prefix(pStyle->pszSingleLineRegexp, pszPos,
                            pStyle->fCaseInsensitive, &ulMatched) &&
            ulMatched > 0) {
            pszPos += ulMatched;
            if (pStyle->pszIndentAfterSingle != NULL) {
                PCSZ pszIndent = pStyle->pszIndentAfterSingle;
                size_t cbIndent = strlen(pszIndent);
                if (strncmp(pszPos, pszIndent, cbIndent) == 0)
                    pszPos += cbIndent;
            }
            *pulOffset = (ULONG)(pszPos - pszLine);
            return NO_ERROR;
        }
    }

    /* Single-line, literal form. */
    if (pStyle->pszSingleLine != NULL) {
        cbMarker = strlen(pStyle->pszSingleLine);
        {
            BOOL fMatch;
            if (pStyle->fCaseInsensitive) {
                size_t i;
                fMatch = TRUE;
                for (i = 0; i < cbMarker; i++) {
                    if (tolower((unsigned char)pszPos[i]) !=
                        tolower((unsigned char)pStyle->pszSingleLine[i])) {
                        fMatch = FALSE;
                        break;
                    }
                }
            } else {
                fMatch = (strncmp(pszPos, pStyle->pszSingleLine,
                                  cbMarker) == 0) ? TRUE : FALSE;
            }
            if (fMatch) {
                pszPos += cbMarker;
                if (pStyle->pszIndentAfterSingle != NULL) {
                    PCSZ pszIndent = pStyle->pszIndentAfterSingle;
                    size_t cbIndent = strlen(pszIndent);
                    if (strncmp(pszPos, pszIndent, cbIndent) == 0)
                        pszPos += cbIndent;
                }
                *pulOffset = (ULONG)(pszPos - pszLine);
                return NO_ERROR;
            }
        }
    }

    if (pStyle->pszMultiStart != NULL) {
        cbMarker = strlen(pStyle->pszMultiStart);
        if (strncmp(pszPos, pStyle->pszMultiStart, cbMarker) == 0) {
            pszPos += cbMarker;
            *pulOffset = (ULONG)(pszPos - pszLine);
            return NO_ERROR;
        }
    }

    if (pStyle->pszMultiMiddle != NULL) {
        cbMarker = strlen(pStyle->pszMultiMiddle);
        if (strncmp(pszPos, pStyle->pszMultiMiddle, cbMarker) == 0) {
            pszPos += cbMarker;
            if (pStyle->pszIndentAfterMiddle != NULL) {
                PCSZ pszIndent = pStyle->pszIndentAfterMiddle;
                size_t cbIndent = strlen(pszIndent);
                if (strncmp(pszPos, pszIndent, cbIndent) == 0)
                    pszPos += cbIndent;
            }
            *pulOffset = (ULONG)(pszPos - pszLine);
            return NO_ERROR;
        }
    }

    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Compute the size (without NUL) of the block that
 *        ReuseQueryCommentBlock would produce.
 *
 * @param[in] pStyle     Style. Not NULL.
 * @param[in] pszText    Text. Not NULL.
 * @param[in] fUseMulti  TRUE for multi-line layout.
 *
 * @return ULONG
 *
 * @return Required size in bytes, excluding the trailing NUL.
 */
static ULONG comment_block_size(PREUSECOMMENTSTYLE pStyle,
                                PCSZ pszText, BOOL fUseMulti) {
    ULONG ulSize = 0;
    PCSZ pszLine = pszText;

    if (fUseMulti) {
        if (pStyle->pszMultiStart != NULL)
            ulSize += (ULONG)strlen(pStyle->pszMultiStart) + 1;
        while (*pszLine != '\0') {
            PCSZ pszEol = strchr(pszLine, '\n');
            size_t cbLine = pszEol ? (size_t)(pszEol - pszLine)
                                   : strlen(pszLine);
            if (pStyle->pszIndentBeforeMiddle != NULL)
                ulSize += (ULONG)strlen(pStyle->pszIndentBeforeMiddle);
            if (pStyle->pszMultiMiddle != NULL)
                ulSize += (ULONG)strlen(pStyle->pszMultiMiddle);
            if (pStyle->pszIndentAfterMiddle != NULL)
                ulSize += (ULONG)strlen(pStyle->pszIndentAfterMiddle);
            ulSize += (ULONG)cbLine + 1;
            if (!pszEol) break;
            pszLine = pszEol + 1;
        }
        if (pStyle->pszIndentBeforeEnd != NULL)
            ulSize += (ULONG)strlen(pStyle->pszIndentBeforeEnd);
        if (pStyle->pszMultiEnd != NULL)
            ulSize += (ULONG)strlen(pStyle->pszMultiEnd) + 1;
    } else {
        while (*pszLine != '\0') {
            PCSZ pszEol = strchr(pszLine, '\n');
            size_t cbLine = pszEol ? (size_t)(pszEol - pszLine)
                                   : strlen(pszLine);
            if (pStyle->pszSingleLine != NULL)
                ulSize += (ULONG)strlen(pStyle->pszSingleLine);
            if (cbLine > 0 && pStyle->pszIndentAfterSingle != NULL)
                ulSize += (ULONG)strlen(pStyle->pszIndentAfterSingle);
            ulSize += (ULONG)cbLine + 1;
            if (!pszEol) break;
            pszLine = pszEol + 1;
        }
    }
    return ulSize;
}

/*!
 * @brief Emit the block into a caller-supplied buffer.
 *
 * @param[in] pStyle     Style. Not NULL.
 * @param[in] pszText    Text. Not NULL.
 * @param[in] fUseMulti  TRUE for multi-line layout.
 * @param[out] pszBuf    Destination buffer. Not NULL; must have room
 *                       for comment_block_size() + 1 bytes.
 */
static void comment_block_emit(PREUSECOMMENTSTYLE pStyle, PCSZ pszText,
                               BOOL fUseMulti, PSZ pszBuf) {
    PSZ pszOut = pszBuf;
    PCSZ pszLine = pszText;

    if (fUseMulti) {
        if (pStyle->pszMultiStart != NULL) {
            size_t cb = strlen(pStyle->pszMultiStart);
            memcpy(pszOut, pStyle->pszMultiStart, cb);
            pszOut += cb;
            *pszOut++ = '\n';
        }
        while (*pszLine != '\0') {
            PCSZ pszEol = strchr(pszLine, '\n');
            size_t cbLine = pszEol ? (size_t)(pszEol - pszLine)
                                   : strlen(pszLine);
            if (pStyle->pszIndentBeforeMiddle != NULL) {
                size_t cb = strlen(pStyle->pszIndentBeforeMiddle);
                memcpy(pszOut, pStyle->pszIndentBeforeMiddle, cb);
                pszOut += cb;
            }
            if (pStyle->pszMultiMiddle != NULL) {
                size_t cb = strlen(pStyle->pszMultiMiddle);
                memcpy(pszOut, pStyle->pszMultiMiddle, cb);
                pszOut += cb;
            }
            if (pStyle->pszIndentAfterMiddle != NULL) {
                size_t cb = strlen(pStyle->pszIndentAfterMiddle);
                memcpy(pszOut, pStyle->pszIndentAfterMiddle, cb);
                pszOut += cb;
            }
            if (cbLine > 0) {
                memcpy(pszOut, pszLine, cbLine);
                pszOut += cbLine;
            }
            *pszOut++ = '\n';
            if (!pszEol) break;
            pszLine = pszEol + 1;
        }
        if (pStyle->pszIndentBeforeEnd != NULL) {
            size_t cb = strlen(pStyle->pszIndentBeforeEnd);
            memcpy(pszOut, pStyle->pszIndentBeforeEnd, cb);
            pszOut += cb;
        }
        if (pStyle->pszMultiEnd != NULL) {
            size_t cb = strlen(pStyle->pszMultiEnd);
            memcpy(pszOut, pStyle->pszMultiEnd, cb);
            pszOut += cb;
            *pszOut++ = '\n';
        }
    } else {
        while (*pszLine != '\0') {
            PCSZ pszEol = strchr(pszLine, '\n');
            size_t cbLine = pszEol ? (size_t)(pszEol - pszLine)
                                   : strlen(pszLine);
            if (pStyle->pszSingleLine != NULL) {
                size_t cb = strlen(pStyle->pszSingleLine);
                memcpy(pszOut, pStyle->pszSingleLine, cb);
                pszOut += cb;
            }
            if (cbLine > 0 && pStyle->pszIndentAfterSingle != NULL) {
                size_t cb = strlen(pStyle->pszIndentAfterSingle);
                memcpy(pszOut, pStyle->pszIndentAfterSingle, cb);
                pszOut += cb;
            }
            if (cbLine > 0) {
                memcpy(pszOut, pszLine, cbLine);
                pszOut += cbLine;
            }
            *pszOut++ = '\n';
            if (!pszEol) break;
            pszLine = pszEol + 1;
        }
    }
    *pszOut = '\0';
}

/*!
 * @brief Build the SPDX comment block for the given text.
 *
 * @param[in]  hStyle      Borrowed style. Not NULLHANDLE.
 * @param[in]  pszText     Text to comment. Not NULL.
 * @param[in]  fForceMulti TRUE to force multi-line output.
 * @param[out] pszBuf      Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize      Size of pszBuf in bytes.
 * @param[out] pulUsed     Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 * @retval ERROR_INVALID_DATA       Style cannot produce the block.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseQueryCommentBlock(HREUSECOMMENTSTYLE hStyle,
                                       PCSZ pszText, BOOL fForceMulti,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed) {
    PREUSECOMMENTSTYLE pStyle;
    BOOL fUseMulti;
    ULONG ulRequired;

    if (!pszText) return ERROR_INVALID_PARAMETER;
    if (pszBuf != NULL && ulSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    pStyle = as_style(hStyle);
    if (pStyle == NULL) return ERROR_INVALID_HANDLE;

    fUseMulti = (fForceMulti ||
                 (pStyle->pszSingleLine == NULL &&
                  pStyle->pszSingleLineRegexp == NULL)) ? TRUE : FALSE;
    if (fUseMulti) {
        if (pStyle->pszMultiStart == NULL || pStyle->pszMultiEnd == NULL)
            return ERROR_INVALID_DATA;
    } else {
        if (pStyle->pszSingleLine == NULL)
            return ERROR_INVALID_DATA;
    }

    ulRequired = comment_block_size(pStyle, pszText, fUseMulti);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = ulRequired + 1;
        return NO_ERROR;
    }
    if (ulSize < ulRequired + 1) {
        if (pulUsed) *pulUsed = ulRequired + 1;
        return ERROR_BUFFER_OVERFLOW;
    }

    comment_block_emit(pStyle, pszText, fUseMulti, pszBuf);
    if (pulUsed) *pulUsed = ulRequired;
    return NO_ERROR;
}

/*!
 * @brief Query whether the first line of a file is a shebang that
 *        this style recognizes.
 *
 * @param[in]  hStyle          Borrowed style. Not NULLHANDLE.
 * @param[in]  pszFirstLine    First line of the file. Not NULL.
 * @param[out] pfKeepFirstLine TRUE to keep line 1. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  Argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 */
APIRET APIENTRY ReuseQueryShebang(HREUSECOMMENTSTYLE hStyle,
                                  PCSZ pszFirstLine,
                                  PBOOL pfKeepFirstLine) {
    PREUSECOMMENTSTYLE pStyle;
    PCSZ pszPos;
    ULONG i;

    if (!pszFirstLine || !pfKeepFirstLine) return ERROR_INVALID_PARAMETER;
    pStyle = as_style(hStyle);
    if (pStyle == NULL) return ERROR_INVALID_HANDLE;

    *pfKeepFirstLine = FALSE;
    if (pStyle->papszShebangs == NULL || pStyle->ulShebangCount == 0)
        return NO_ERROR;

    pszPos = pszFirstLine;
    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;

    for (i = 0; i < pStyle->ulShebangCount; i++) {
        PCSZ pszShebang = pStyle->papszShebangs[i];
        size_t cbLen;
        if (pszShebang == NULL) continue;
        cbLen = strlen(pszShebang);
        if (cbLen == 0) continue;
        if (case_insensitive_prefix(pszPos, pszShebang, cbLen)) {
            *pfKeepFirstLine = TRUE;
            return NO_ERROR;
        }
    }
    return NO_ERROR;
}
