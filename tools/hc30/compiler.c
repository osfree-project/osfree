/*!
 * @file compiler.c
 * @brief Project compilation implementation.
 *
 * @par References
 *  [1] Winterhoff, M., helpdeco (1997), ContextId, AddTopic, Guess,
 *      FirstPass, TopicDump.
 *  [2] SAA CPI C Reference - Level 2, SC09-1308-02 (Sep 1991).
 *  [3] OS/2 V2.0 Vol.4, IBM, 1993, section 12.2.
 */
#include "compiler.h"
#include "topic.h"
#include "fnt.h"
#include "phrenc.h"
#include "tom.h"
#include "sys.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ==================================================================
 * Context map
 * ================================================================== */

/*!
 * @brief One context-string-to-topic-number entry.
 */
typedef struct {
    PSZ   pszContext;      /*!< Context string, heap-allocated. */
    ULONG ulTopicNumber;   /*!< Assigned topic number (1-based). */
} CmpContextRec;

/*!
 * @brief Create the context map.
 *
 * @param[out] phvMap Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET cmp_map_create(HVECTOR* phvMap)
{
    return VectorCreate(sizeof(CmpContextRec), phvMap);
}

/*!
 * @brief Destroy the context map and release all context strings.
 *
 * @param[in] hvMap Context map. May be NULLHANDLE.
 */
static VOID cmp_map_destroy(HVECTOR hvMap)
{
    ULONG n = 0, i;
    if (hvMap == NULLHANDLE) return;
    VectorGetCount(hvMap, &n);
    for (i = 0; i < n; i++) {
        CmpContextRec r;
        VectorGetItem(hvMap, i, &r, sizeof(r), NULL);
        if (r.pszContext) free(r.pszContext);
    }
    VectorDestroy(hvMap);
}

/*!
 * @brief Register a context string, returning its topic number.
 *
 * @param[in]  hvMap          Context map. Not NULLHANDLE.
 * @param[in]  pszContext     Context string. Not NULL.
 * @param[out] pulTopicNumber Optional. Receives the topic number.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hvMap or @a pszContext is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET cmp_map_register(HVECTOR hvMap, PCSZ pszContext,
                               PULONG pulTopicNumber)
{
    ULONG n = 0, i;
    CmpContextRec r;
    if (hvMap == NULLHANDLE || !pszContext) return ERROR_INVALID_PARAMETER;
    VectorGetCount(hvMap, &n);
    for (i = 0; i < n; i++) {
        VectorGetItem(hvMap, i, &r, sizeof(r), NULL);
        if (r.pszContext && stricmp(r.pszContext, pszContext) == 0) {
            if (pulTopicNumber) *pulTopicNumber = r.ulTopicNumber;
            return NO_ERROR;
        }
    }
    r.pszContext = strdup(pszContext);
    if (!r.pszContext) return ERROR_NOT_ENOUGH_MEMORY;
    r.ulTopicNumber = n + 1;
    if (pulTopicNumber) *pulTopicNumber = r.ulTopicNumber;
    return VectorAdd(hvMap, &r);
}

/*!
 * @brief Look up a context string in the map.
 *
 * @param[in]  hvMap          Context map. Not NULLHANDLE.
 * @param[in]  pszContext     Context string. Not NULL.
 * @param[out] pulTopicNumber Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Context not present.
 */
static APIRET cmp_map_lookup(HVECTOR hvMap, PCSZ pszContext,
                             PULONG pulTopicNumber)
{
    ULONG n = 0, i;
    if (hvMap == NULLHANDLE || !pszContext || !pulTopicNumber)
        return ERROR_INVALID_PARAMETER;
    VectorGetCount(hvMap, &n);
    for (i = 0; i < n; i++) {
        CmpContextRec r;
        VectorGetItem(hvMap, i, &r, sizeof(r), NULL);
        if (r.pszContext && stricmp(r.pszContext, pszContext) == 0) {
            *pulTopicNumber = r.ulTopicNumber;
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/* ==================================================================
 * Topic collection
 * ================================================================== */

/*!
 * @brief One RTF topic paired with its assigned number.
 */
typedef struct {
    HRTFTOPIC hTopic;        /*!< Topic handle. */
    ULONG     ulTopicNumber; /*!< Assigned topic number. */
} CmpTopicRec;

/*!
 * @brief Collect all RTF topics and register their contexts.
 *
 * @param[in]  hDoc      RTF document. Not NULLHANDLE.
 * @param[in]  hvMap     Context map. Not NULLHANDLE.
 * @param[out] phvTopics Receiver for the topic list. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET cmp_collect_topics(HRTFDOC hDoc, HVECTOR hvMap,
                                 HVECTOR* phvTopics)
{
    HRTFENUM hEnum;
    APIRET   rc;
    ULONG    ulNext = 1;

    rc = VectorCreate(sizeof(CmpTopicRec), phvTopics);
    if (rc != NO_ERROR) return rc;

    rc = RtfFindFirstTopic(hDoc, &hEnum);
    if (rc == ERROR_NO_MORE_ITEMS) return NO_ERROR;
    if (rc != NO_ERROR) return rc;

    do {
        HRTFTOPIC hTopic;
        char      szContext[256];
        CmpTopicRec rec;
        ULONG     ulRegistered = 0;

        rc = RtfQueryEnumTopic(hEnum, &hTopic);
        if (rc != NO_ERROR) continue;

        szContext[0] = '\0';
        RtfQueryTopicContext(hTopic, szContext, sizeof(szContext), NULL);
        if (szContext[0]) {
            rc = cmp_map_register(hvMap, szContext, &ulRegistered);
            if (rc != NO_ERROR) { RtfFindClose(hEnum); return rc; }
        }

        rec.hTopic = hTopic;
        rec.ulTopicNumber = szContext[0] ? ulRegistered : ulNext++;
        rc = VectorAdd(*phvTopics, &rec);
        if (rc != NO_ERROR) { RtfFindClose(hEnum); return rc; }
    } while (RtfFindNextTopic(hEnum) == NO_ERROR);

    RtfFindClose(hEnum);
    return NO_ERROR;
}

/* ==================================================================
 * Fonts
 * ================================================================== */

/*!
 * @brief Copy font faces and descriptors from the RTF document to |FONT.
 *
 * @param[in]  hDoc         RTF document. Not NULLHANDLE.
 * @param[in]  hFnt         |FONT generator. Not NULLHANDLE.
 * @param[out] paulMap      Receiver for the descriptor mapping array.
 *                          Not NULL.
 * @param[out] pulCount     Receiver for the array length. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET cmp_copy_fonts(HRTFDOC hDoc, HFNT hFnt,
                             PULONG* paulMap, PULONG pulCount)
{
    ULONG  ulFaceCount = 0, ulDescCount = 0, ulIdx;
    PULONG aulMap;

    RtfQueryDocFontCount(hDoc, &ulFaceCount);
    for (ulIdx = 0; ulIdx < ulFaceCount; ulIdx++) {
        char szName[256];
        szName[0] = '\0';
        if (RtfQueryDocFontName(hDoc, ulIdx, szName,
                                sizeof(szName), NULL) == NO_ERROR)
            FntAddFaceName(hFnt, szName, NULL);
    }

    RtfQueryDocFontDescriptorCount(hDoc, &ulDescCount);
    aulMap = (PULONG)malloc((ulDescCount ? ulDescCount : 1) * sizeof(ULONG));
    if (!aulMap) return ERROR_NOT_ENOUGH_MEMORY;

    for (ulIdx = 0; ulIdx < ulDescCount; ulIdx++) {
        ULONG ulFace = 0, ulHalf = 20, ulAttr = 0, ulFamily = 2;
        BYTE  abFG[3];
        BYTE  abBG[3];
        ULONG ulIndex = 0;
        abFG[0] = 0; abFG[1] = 0; abFG[2] = 0;
        abBG[0] = 0xFF; abBG[1] = 0xFF; abBG[2] = 0xFF;
        if (RtfQueryDocFontDescriptor(hDoc, ulIdx, &ulFace, &ulHalf,
                                      &ulAttr, abFG, abBG,
                                      &ulFamily) != NO_ERROR) {
            aulMap[ulIdx] = 0;
            continue;
        }
        if (FntGetOrCreateDescriptor(hFnt, ulFace, ulHalf, ulAttr,
                                     abFG, abBG, ulFamily,
                                     &ulIndex) == NO_ERROR) {
            aulMap[ulIdx] = ulIndex;
        } else {
            aulMap[ulIdx] = 0;
        }
    }
    *paulMap = aulMap;
    *pulCount = ulDescCount;
    return NO_ERROR;
}

/* ==================================================================
 * Build tag filtering
 * ================================================================== */

/*!
 * @brief Test whether a topic passes the active build tag filter.
 *
 * @param[in] hTopic  Topic handle. Not NULLHANDLE.
 * @param[in] vActive Vector of active tag names (PSZ).
 *
 * @return TRUE if the topic has no tags or any of its tags is active.
 * @retval TRUE   Topic passes the filter.
 * @retval FALSE  Topic is filtered out.
 */
static BOOL cmp_topic_matches_active(HRTFTOPIC hTopic, HVECTOR vActive)
{
    ULONG n = 0, na = 0, i, j;
    RtfQueryTopicBuildTagCount(hTopic, &n);
    if (n == 0) return TRUE;
    VectorGetCount(vActive, &na);
    if (na == 0) return TRUE;
    for (i = 0; i < n; i++) {
        char szTag[128];
        szTag[0] = '\0';
        if (RtfQueryTopicBuildTag(hTopic, i, szTag, sizeof(szTag), NULL)
            != NO_ERROR)
            continue;
        for (j = 0; j < na; j++) {
            PSZ s = NULL;
            VectorGetItem(vActive, j, &s, sizeof(s), NULL);
            if (s && stricmp(s, szTag) == 0) return TRUE;
        }
    }
    return FALSE;
}

/* ==================================================================
 * Phrase table
 * ================================================================== */

/*!
 * @brief Feed all active topic texts into the phrase encoder.
 *
 * @param[in] hDoc    RTF document. Not NULLHANDLE.
 * @param[in] hEnc    Phrase encoder. Not NULLHANDLE.
 * @param[in] vActive Vector of active tag names.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET cmp_build_phrases(HRTFDOC hDoc, HPHRE hEnc, HVECTOR vActive)
{
    HRTFENUM hEnum;
    APIRET rc;

    rc = RtfFindFirstTopic(hDoc, &hEnum);
    if (rc == ERROR_NO_MORE_ITEMS) return NO_ERROR;
    if (rc != NO_ERROR) return rc;

    do {
        HRTFTOPIC hTopic;
        HRTFENUM hFragEnum;
        if (RtfQueryEnumTopic(hEnum, &hTopic) != NO_ERROR) continue;
        if (!cmp_topic_matches_active(hTopic, vActive)) continue;
        if (RtfFindFirstFragment(hTopic, &hFragEnum) != NO_ERROR) continue;
        do {
            HRTFFRAG hFrag;
            ULONG ulType = 0;
            if (RtfQueryEnumFragment(hFragEnum, &hFrag) != NO_ERROR) continue;
            RtfQueryFragType(hFrag, &ulType);
            if (ulType == RTF_FRAG_TEXT) {
                char szText[4096];
                szText[0] = '\0';
                RtfQueryFragText(hFrag, szText, sizeof(szText), NULL);
                PhrEncAddText(hEnc, szText);
            }
        } while (RtfFindNextFragment(hFragEnum) == NO_ERROR);
        RtfFindClose(hFragEnum);
    } while (RtfFindNextTopic(hEnum) == NO_ERROR);
    RtfFindClose(hEnum);

    return PhrEncBuildTable(hEnc);
}

/* ==================================================================
 * Topic emission
 *
 * NOTE: WinHelp 3.0 uses 0-based font indices in the |TOPIC stream.
 * Index 0 references the first descriptor in |FONT, index 1 the
 * second, and so on.
 * ================================================================== */

/*!
 * @brief Emit one topic (text and link fragments) into |TOPIC.
 *
 * @param[in] hTop           |TOPIC generator. Not NULLHANDLE.
 * @param[in] hTopic         Source topic handle.
 * @param[in] hvMap          Context map for link resolution.
 * @param[in] aulFontMap     Descriptor index remap table.
 * @param[in] ulFontMapCount Length of @a aulFontMap.
 * @param[in] pszTitle       Topic title.
 * @param[in] pszContext     Topic context string.
 * @param[in] hvTopics       Vector of CmpTopicRec for title lookup.
 * @param[in] ulTopicCount   Number of topics in @a hvTopics.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
/* ==================================================================
 * Topic emission
 *
 * NOTE: WinHelp 3.0 uses 1-based font indices in the |TOPIC stream.
 * Index 0 designates the built-in "system" font; descriptors stored
 * in |FONT are referenced by index 1, 2, 3, ...  We add +1 to the
 * zero-based descriptor index returned by FntGetOrCreateDescriptor
 * before emitting it into the |TOPIC stream.
 * ================================================================== */

static APIRET cmp_emit_topic(HTOP hTop, HRTFTOPIC hTopic, HVECTOR hvMap,
                             const ULONG* aulFontMap, ULONG ulFontMapCount,
                             PCSZ pszTitle, PCSZ pszContext,
                             HVECTOR hvTopics, ULONG ulTopicCount)
{
    HRTFENUM hEnum;
    APIRET   rc;

    rc = TopAddTopic(hTop, pszTitle, pszContext);
    if (rc != NO_ERROR) return rc;

    rc = RtfFindFirstFragment(hTopic, &hEnum);
    if (rc == ERROR_NO_MORE_ITEMS) return NO_ERROR;
    if (rc != NO_ERROR) return rc;

    do {
        HRTFFRAG hFrag;
        ULONG    ulType = 0;
        rc = RtfQueryEnumFragment(hEnum, &hFrag);
        if (rc != NO_ERROR) continue;
        RtfQueryFragType(hFrag, &ulType);

        if (ulType == RTF_FRAG_TEXT) {
            char  szText[4096];
            ULONG ulFontIdx = 0, ulMapped = 0;
            szText[0] = '\0';
            RtfQueryFragText(hFrag, szText, sizeof(szText), NULL);
            RtfQueryFragFont(hFrag, &ulFontIdx);
            if (aulFontMap && ulFontIdx < ulFontMapCount)
                ulMapped = aulFontMap[ulFontIdx] + 1;   /* 1-based */
            rc = TopAddText(hTop, szText, ulMapped);
            if (rc != NO_ERROR) { RtfFindClose(hEnum); return rc; }
        } else if (ulType == RTF_FRAG_LINK) {
            char  szText[1024];
            char  szCtx[256];
            BOOL  fPopup = FALSE;
            ULONG ulTarget = TOP_UNRESOLVED_LINK;
            ULONG ulFontIdx = 0, ulMapped = 0;
            szText[0] = '\0';
            szCtx[0]  = '\0';
            RtfQueryFragText(hFrag, szText, sizeof(szText), NULL);
            RtfQueryFragContext(hFrag, szCtx, sizeof(szCtx), NULL);
            RtfQueryFragPopup(hFrag, &fPopup);

            if (szCtx[0]) {
                ULONG ulNum = 0;
                if (cmp_map_lookup(hvMap, szCtx, &ulNum) == NO_ERROR) {
                    ulTarget = ulNum;
                } else {
                    /* Fallback: resolve the target by topic title. */
                    ULONG k;
                    for (k = 0; k < ulTopicCount; k++) {
                        CmpTopicRec rec2;
                        char szTitle2[256];
                        VectorGetItem(hvTopics, k, &rec2, sizeof(rec2), NULL);
                        RtfQueryTopicTitle(rec2.hTopic, szTitle2,
                                           sizeof(szTitle2), NULL);
                        if (stricmp(szTitle2, szCtx) == 0) {
                            ulTarget = rec2.ulTopicNumber;
                            break;
                        }
                    }
                }
            }

            if (szText[0]) {
                RtfQueryFragFont(hFrag, &ulFontIdx);
                if (aulFontMap && ulFontIdx < ulFontMapCount)
                    ulMapped = aulFontMap[ulFontIdx] + 1;   /* 1-based */
                rc = TopAddText(hTop, szText, ulMapped);
                if (rc != NO_ERROR) { RtfFindClose(hEnum); return rc; }
            }
            rc = TopAddLink(hTop, ulTarget, fPopup);
            if (rc != NO_ERROR) { RtfFindClose(hEnum); return rc; }
        }
    } while (RtfFindNextFragment(hEnum) == NO_ERROR);

    RtfFindClose(hEnum);
    return NO_ERROR;
}

/* ==================================================================
 * HPJ aliases, windows, config
 * ================================================================== */

/*!
 * @brief Register HPJ aliases into the context map.
 *
 * @param[in] hHpj  HPJ document. Not NULLHANDLE.
 * @param[in] hvMap Context map. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET cmp_register_aliases(HHPJ hHpj, HVECTOR hvMap)
{
    ULONG n = 0, i;
    APIRET rc;
    rc = HpjQueryAliasCount(hHpj, &n);
    if (rc != NO_ERROR) return NO_ERROR;
    for (i = 0; i < n; i++) {
        char szName[256];
        char szCtx[256];
        ULONG ulNum = 0;
        szName[0] = '\0';
        szCtx[0]  = '\0';
        if (HpjQueryAliasName(hHpj, i, szName, sizeof(szName), NULL) != NO_ERROR)
            continue;
        if (HpjQueryAliasContext(hHpj, i, szCtx, sizeof(szCtx), NULL) != NO_ERROR)
            continue;
        rc = cmp_map_register(hvMap, szCtx, &ulNum);
        if (rc != NO_ERROR) return rc;
        {
            ULONG ulExisting = 0;
            if (cmp_map_lookup(hvMap, szName, &ulExisting) != NO_ERROR) {
                CmpContextRec rec;
                rec.pszContext = strdup(szName);
                rec.ulTopicNumber = ulNum;
                if (rec.pszContext) VectorAdd(hvMap, &rec);
            }
        }
    }
    return NO_ERROR;
}

/*!
 * @brief Emit [WINDOWS] definitions into |SYSTEM.
 *
 * @param[in] hHpj HPJ document. Not NULLHANDLE.
 * @param[in] hSys |SYSTEM generator. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Always (errors are treated as absent sections).
 */
static APIRET cmp_add_windows(HHPJ hHpj, HSYS hSys)
{
    ULONG n = 0, i;
    APIRET rc;
    rc = HpjQueryWindowCount(hHpj, &n);
    if (rc != NO_ERROR) return NO_ERROR;
    for (i = 0; i < n; i++) {
        HPJWINDOW win;
        UCHAR abRec[128];
        ULONG ulLen = 0;
        memset(&win, 0, sizeof(win));
        if (HpjQueryWindow(hHpj, i, &win) != NO_ERROR) continue;
        {
            USHORT usFlags = 0;
            size_t nName = strlen(win.szName);
            size_t nCap  = strlen(win.szCaption);
            if (nName) usFlags |= 0x0002;
            if (nCap)  usFlags |= 0x0004;
            if (win.x || win.y || win.w || win.h) usFlags |= 0x0078;
            usFlags |= 0x0180;
            memcpy(abRec + ulLen, &usFlags, 2); ulLen += 2;
            if (nName) { memcpy(abRec + ulLen, win.szName, nName + 1); ulLen += (ULONG)nName + 1; }
            else abRec[ulLen++] = 0;
            if (nCap)  { memcpy(abRec + ulLen, win.szCaption, nCap + 1); ulLen += (ULONG)nCap + 1; }
            else abRec[ulLen++] = 0;
            memcpy(abRec + ulLen, &win.x, 2); ulLen += 2;
            memcpy(abRec + ulLen, &win.y, 2); ulLen += 2;
            memcpy(abRec + ulLen, &win.w, 2); ulLen += 2;
            memcpy(abRec + ulLen, &win.h, 2); ulLen += 2;
            memcpy(abRec + ulLen, &win.state, 2); ulLen += 2;
            memcpy(abRec + ulLen, win.rgb, 3); ulLen += 3;
            abRec[ulLen++] = 0;
            memcpy(abRec + ulLen, win.rgbNsr, 3); ulLen += 3;
            abRec[ulLen++] = 0;
        }
        SysAddWindow(hSys, abRec, ulLen);
    }
    return NO_ERROR;
}

/*!
 * @brief Emit [CONFIG] macro lines into |SYSTEM.
 *
 * @param[in] hHpj HPJ document. Not NULLHANDLE.
 * @param[in] hSys |SYSTEM generator. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Always.
 */
static APIRET cmp_add_configs(HHPJ hHpj, HSYS hSys)
{
    ULONG n = 0, i;
    APIRET rc;
    rc = HpjQueryConfigCount(hHpj, &n);
    if (rc != NO_ERROR) return NO_ERROR;
    for (i = 0; i < n; i++) {
        char szLine[1024];
        szLine[0] = '\0';
        if (HpjQueryConfig(hHpj, i, szLine, sizeof(szLine), NULL) == NO_ERROR)
            SysAddConfig(hSys, szLine);
    }
    return NO_ERROR;
}

/*!
 * @brief Populate the |SYSTEM generator from HPJ options and sections.
 *
 * @param[in] hHpj  HPJ document. Not NULLHANDLE.
 * @param[in] hSys  |SYSTEM generator. Not NULLHANDLE.
 * @param[in] hvMap Context map, used to resolve CONTENTS. May be
 *                  NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET cmp_fill_system(HHPJ hHpj, HSYS hSys, HVECTOR hvMap)
{
    char szTmp[256];
    szTmp[0] = '\0';
    if (HpjQueryOptionValue(hHpj, "TITLE", szTmp, sizeof(szTmp), NULL) == NO_ERROR)
        SysSetTitle(hSys, szTmp);

    szTmp[0] = '\0';
    if (HpjQueryOptionValue(hHpj, "COPYRIGHT", szTmp, sizeof(szTmp), NULL) == NO_ERROR)
        SysSetCopyright(hSys, szTmp);

    szTmp[0] = '\0';
    if (HpjQueryOptionValue(hHpj, "CONTENTS", szTmp, sizeof(szTmp), NULL) == NO_ERROR) {
        ULONG ulNum = 0;
        if (hvMap != NULLHANDLE &&
            cmp_map_lookup(hvMap, szTmp, &ulNum) == NO_ERROR)
            SysSetContents(hSys, ulNum);
        else
            SysSetContents(hSys, 0);
    }

    szTmp[0] = '\0';
    if (HpjQueryOptionValue(hHpj, "CITATION", szTmp, sizeof(szTmp), NULL) == NO_ERROR)
        SysSetCitation(hSys, szTmp);

    szTmp[0] = '\0';
    if (HpjQueryOptionValue(hHpj, "CNT", szTmp, sizeof(szTmp), NULL) == NO_ERROR)
        SysSetCnt(hSys, szTmp);

    szTmp[0] = '\0';
    if (HpjQueryOptionValue(hHpj, "LCID", szTmp, sizeof(szTmp), NULL) == NO_ERROR) {
        unsigned int a = 0, b = 0, c = 0;
        if (sscanf(szTmp, "0x%x 0x%x 0x%x", &a, &b, &c) == 3)
            SysSetLcid(hSys, (USHORT)a, (USHORT)b, (USHORT)c);
        else if (sscanf(szTmp, "%u %u %u", &a, &b, &c) == 3)
            SysSetLcid(hSys, (USHORT)a, (USHORT)b, (USHORT)c);
    }
    cmp_add_windows(hHpj, hSys);
    cmp_add_configs(hHpj, hSys);
    return NO_ERROR;
}

/* ==================================================================
 * Writers for HfsAddFileFromWriter
 * ================================================================== */

/*! @brief Writer argument for |SYSTEM. */
typedef struct { HSYS  hSys; } CmpWriterSys;
/*! @brief Writer argument for |FONT. */
typedef struct { HFNT  hFnt; } CmpWriterFnt;
/*! @brief Writer argument for |Phrases. */
typedef struct { HPHRE hEnc; } CmpWriterPhr;
/*! @brief Writer argument for |TOMAP. */
typedef struct { HTOM  hTom; } CmpWriterTom;
/*! @brief Writer argument for |TOPIC. */
typedef struct { HTOP  hTop; HTOM hTom; HPHRE hEnc; } CmpWriterTop;

/*!
 * @brief Serialize |SYSTEM through the HfsAddFileFromWriter callback.
 *
 * @param[in] pArg Pointer to CmpWriterSys.
 * @param[in] f    Output stream.
 */
static VOID cmp_writer_sys(PVOID pArg, FILE* f)
{
    CmpWriterSys* w = (CmpWriterSys*)pArg;
    SysWrite(w->hSys, f);
}

/*!
 * @brief Serialize |FONT through the HfsAddFileFromWriter callback.
 *
 * @param[in] pArg Pointer to CmpWriterFnt.
 * @param[in] f    Output stream.
 */
static VOID cmp_writer_fnt(PVOID pArg, FILE* f)
{
    CmpWriterFnt* w = (CmpWriterFnt*)pArg;
    FntWrite(w->hFnt, f);
}

/*!
 * @brief Serialize |Phrases through the HfsAddFileFromWriter callback.
 *
 * @param[in] pArg Pointer to CmpWriterPhr.
 * @param[in] f    Output stream.
 */
static VOID cmp_writer_phr(PVOID pArg, FILE* f)
{
    CmpWriterPhr* w = (CmpWriterPhr*)pArg;
    PhrEncWrite(w->hEnc, f);
}

/*!
 * @brief Serialize |TOMAP through the HfsAddFileFromWriter callback.
 *
 * @param[in] pArg Pointer to CmpWriterTom.
 * @param[in] f    Output stream.
 */
static VOID cmp_writer_tom(PVOID pArg, FILE* f)
{
    CmpWriterTom* w = (CmpWriterTom*)pArg;
    TomWrite(w->hTom, f);
}

/*!
 * @brief Serialize |TOPIC and populate |TOMAP from its offsets.
 *
 * @param[in] pArg Pointer to CmpWriterTop.
 * @param[in] f    Output stream for |TOPIC.
 */
static VOID cmp_writer_top(PVOID pArg, FILE* f)
{
    CmpWriterTop* w = (CmpWriterTop*)pArg;
    HVECTOR vF = NULLHANDLE;

    TopWrite(w->hTop, f, &vF, w->hEnc);

    if (vF) {
        ULONG n = 0, j;
        ULONG payload = (ULONG)TOP_BLOCK_SIZE - 12UL;
        VectorGetCount(vF, &n);
        for (j = 0; j < n; j++) {
            ULONG fo = 0, block, off, phys;
            VectorGetItem(vF, j, &fo, sizeof(fo), NULL);
            block = fo / payload;
            off = fo % payload;
            phys = block * (ULONG)TOP_BLOCK_SIZE + 12UL + off;
            TomAddOffset(w->hTom, phys);
        }
        VectorDestroy(vF);
    }
}

/* ==================================================================
 * Main entry point
 * ================================================================== */

/*!
 * @brief Compile an HPJ document plus its RTF into an HLP container.
 *
 * @param[in]  hHpj      HPJ document. Not NULLHANDLE.
 * @param[in]  hDoc      RTF document. Not NULLHANDLE.
 * @param[in]  pszHpjPath Reserved for future use. May be NULL.
 * @param[out] phHfs     Receiver for the HLP container. Not NULL.
 *                       Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any required handle is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY CmpCompile(HHPJ hHpj, HRTFDOC hDoc, PCSZ pszHpjPath,
                           PHHFS phHfs)
{
    HHFS     hHfs = NULLHANDLE;
    HTOP     hTop = NULLHANDLE;
    HFNT     hFnt = NULLHANDLE;
    HPHRE    hEnc = NULLHANDLE;
    HTOM     hTom = NULLHANDLE;
    HSYS     hSys = NULLHANDLE;
    HVECTOR  hvMap = NULLHANDLE;
    HVECTOR  hvTopics = NULLHANDLE;
    HVECTOR  vActive = NULLHANDLE;
    PULONG   aulFontMap = NULL;
    ULONG    ulFontMapCount = 0;
    APIRET   rc;
    ULONG    ulCount = 0, i;

    if (!phHfs || hHpj == NULLHANDLE || hDoc == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;
    *phHfs = NULLHANDLE;
    (void)pszHpjPath;

    if ((rc = HfsCreate(&hHfs)) != NO_ERROR) goto fail;
    if ((rc = TopCreate(&hTop)) != NO_ERROR) goto fail;
    if ((rc = FntCreate(&hFnt)) != NO_ERROR) goto fail;
    if ((rc = PhrEncCreate(&hEnc)) != NO_ERROR) goto fail;
    if ((rc = TomCreate(&hTom)) != NO_ERROR) goto fail;
    if ((rc = SysCreate(&hSys)) != NO_ERROR) goto fail;
    if ((rc = cmp_map_create(&hvMap)) != NO_ERROR) goto fail;

    /* Active build tags from HPJ. */
    {
        ULONG n = 0, j;
        rc = VectorCreate(sizeof(PSZ), &vActive);
        if (rc != NO_ERROR) goto fail;
        HpjQueryBuildTagCount(hHpj, &n);
        for (j = 0; j < n; j++) {
            char szTag[128];
            szTag[0] = '\0';
            if (HpjQueryBuildTagName(hHpj, j, szTag, sizeof(szTag), NULL)
                != NO_ERROR)
                continue;
            if (szTag[0] == '-') continue;
            {
                const char* p = szTag;
                if (*p == '+') p++;
                if (*p) {
                    PSZ s = strdup(p);
                    if (s) VectorAdd(vActive, &s);
                }
            }
        }
    }

    if ((rc = cmp_copy_fonts(hDoc, hFnt, &aulFontMap, &ulFontMapCount)) != NO_ERROR)
        goto fail;

    if ((rc = cmp_build_phrases(hDoc, hEnc, vActive)) != NO_ERROR)
        goto fail;

    if ((rc = cmp_collect_topics(hDoc, hvMap, &hvTopics)) != NO_ERROR)
        goto fail;

    if ((rc = cmp_register_aliases(hHpj, hvMap)) != NO_ERROR)
        goto fail;

    VectorGetCount(hvTopics, &ulCount);
    for (i = 0; i < ulCount; i++) {
        CmpTopicRec rec;
        char        szTitle[256];
        char        szContext[256];
        VectorGetItem(hvTopics, i, &rec, sizeof(rec), NULL);
        if (!cmp_topic_matches_active(rec.hTopic, vActive)) continue;
        szTitle[0] = '\0';
        szContext[0] = '\0';
        RtfQueryTopicTitle(rec.hTopic, szTitle, sizeof(szTitle), NULL);
        RtfQueryTopicContext(rec.hTopic, szContext, sizeof(szContext), NULL);
        rc = cmp_emit_topic(hTop, rec.hTopic, hvMap, aulFontMap,
                            ulFontMapCount, szTitle, szContext,
                            hvTopics, ulCount);
        if (rc != NO_ERROR) goto fail;
    }

    if ((rc = cmp_fill_system(hHpj, hSys, hvMap)) != NO_ERROR)
        goto fail;

    {
        CmpWriterTop wTop;
        CmpWriterTom wTom;
        CmpWriterSys wSys;
        CmpWriterFnt wFnt;
        CmpWriterPhr wPhr;

        wTop.hTop = hTop;
        wTop.hTom = hTom;
        wTop.hEnc = hEnc;
        wTom.hTom = hTom;
        wSys.hSys = hSys;
        wFnt.hFnt = hFnt;
        wPhr.hEnc = hEnc;

        rc = HfsAddFileFromWriter(hHfs, "|TOPIC",   cmp_writer_top, &wTop);
        if (rc != NO_ERROR) goto fail;
        rc = HfsAddFileFromWriter(hHfs, "|TOMAP",   cmp_writer_tom, &wTom);
        if (rc != NO_ERROR) goto fail;
        rc = HfsAddFileFromWriter(hHfs, "|SYSTEM",  cmp_writer_sys, &wSys);
        if (rc != NO_ERROR) goto fail;
        rc = HfsAddFileFromWriter(hHfs, "|FONT",    cmp_writer_fnt, &wFnt);
        if (rc != NO_ERROR) goto fail;
        rc = HfsAddFileFromWriter(hHfs, "|Phrases", cmp_writer_phr, &wPhr);
        if (rc != NO_ERROR) goto fail;
    }

    *phHfs = hHfs;
    hHfs = NULLHANDLE;
    rc = NO_ERROR;

fail:
    if (hHfs) HfsDestroy(hHfs);
    if (hTop) TopDestroy(hTop);
    if (hFnt) FntDestroy(hFnt);
    if (hEnc) PhrEncDestroy(hEnc);
    if (hTom) TomDestroy(hTom);
    if (hSys) SysDestroy(hSys);
    if (hvMap) cmp_map_destroy(hvMap);
    if (hvTopics) VectorDestroy(hvTopics);
    if (vActive) {
        ULONG n = 0, j;
        VectorGetCount(vActive, &n);
        for (j = 0; j < n; j++) {
            PSZ s = NULL;
            VectorGetItem(vActive, j, &s, sizeof(s), NULL);
            if (s) free(s);
        }
        VectorDestroy(vActive);
    }
    if (aulFontMap) free(aulFontMap);
    return rc;
}
