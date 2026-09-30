/*!
 * @file rtf.c
 * @brief WinHelp RTF parser implementation with build tag support.
 */
#include "rtf.h"
#include "ccl.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* ==================================================================
 * Internal record types
 * ================================================================== */

/*!
 * @brief One RTF fragment.
 */
typedef struct {
    ULONG ulKind;         /*!< RTF_FRAG_TEXT or RTF_FRAG_LINK. */
    PSZ   pszText;        /*!< Text payload or link label. */
    PSZ   pszContext;     /*!< Target context (links only). */
    BOOL  fPopup;         /*!< Popup flag (links only). */
    ULONG ulFontIndex;    /*!< Font descriptor index. */
} RtfFragRec;

/*!
 * @brief One parsed topic.
 */
typedef struct {
    char      szTitle[256];    /*!< Topic title from $ footnote. */
    char      szContext[256];  /*!< Topic context from # footnote. */
    HVECTOR   vFragments;      /*!< Vector of RtfFragRec*. */
    HVECTOR   vBuildTags;      /*!< Vector of PSZ build tags. */
} RtfTopicRec;

/*!
 * @brief One font descriptor (face + size + attributes + colors).
 */
typedef struct {
    ULONG ulFaceIndex;    /*!< Index into the face name table. */
    ULONG ulHalfPoints;   /*!< Size in half-points. */
    ULONG ulAttributes;   /*!< Attribute bits (bold, italic, ...). */
    ULONG ulFamily;       /*!< Font family code. */
    BYTE  abFGRGB[3];     /*!< Foreground RGB. */
    BYTE  abBGRGB[3];     /*!< Background RGB. */
} RtfFontDescRec;

/*!
 * @brief One entry of the color table.
 */
typedef struct {
    BYTE r;               /*!< Red component. */
    BYTE g;               /*!< Green component. */
    BYTE b;               /*!< Blue component. */
} RtfColorRec;

/*!
 * @brief Enumeration cursor state.
 */
typedef struct {
    ULONG ulIndex;        /*!< Current index in the underlying vector. */
    ULONG ulKind;         /*!< 0 = topic enumeration, 1 = fragment. */
    HRTFDOC hDoc;         /*!< Owning document (topic enumeration). */
    HRTFTOPIC hOwner;     /*!< Owning topic (fragment enumeration). */
    BOOL  fClosed;        /*!< TRUE once RtfFindClose was called. */
} RtfEnumRec;

/*!
 * @brief RTF document state.
 */
typedef struct {
    HVECTOR vTopics;      /*!< Vector of RtfTopicRec*. */
    HVECTOR vFaceNames;   /*!< Vector of PSZ face names. */
    HVECTOR vFontDesc;    /*!< Vector of RtfFontDescRec. */
    HVECTOR vColors;      /*!< Vector of RtfColorRec. */
    HVECTOR vCursors;     /*!< Vector of RtfEnumRec*. */
    HVECTOR vActiveTags;  /*!< Vector of PSZ active build tags. */
} RtfDocRec;

/*! @def DOC_FROM_HANDLE
 *  @brief Convert an HRTFDOC handle to its RtfDocRec pointer.
 *  @param[in] h HRTFDOC handle. */
#define DOC_FROM_HANDLE(h) ((RtfDocRec*)(h))

/*! @def DOC_HANDLE_FROM
 *  @brief Convert an RtfDocRec pointer to an HRTFDOC handle.
 *  @param[in] d RtfDocRec pointer. */
#define DOC_HANDLE_FROM(d) ((HANDLE)(d))

/* ==================================================================
 * Parser context
 * ================================================================== */

/*! @def RTF_MAX_FOOTNOTE
 *  @brief Footnote buffer size in bytes. */
#define RTF_MAX_FOOTNOTE 512

/*! @def RTF_MAX_LINKTEXT
 *  @brief Link-label buffer size in bytes. */
#define RTF_MAX_LINKTEXT 1024

/*! @def RTF_MAX_HIDDEN
 *  @brief Hidden-text buffer size in bytes. */
#define RTF_MAX_HIDDEN   512

/*! @def RTF_MAX_STATE_STACK
 *  @brief Maximum depth of the formatting state stack. */
#define RTF_MAX_STATE_STACK 256

/*!
 * @brief Saved formatting state for a nested RTF group.
 */
typedef struct {
    ULONG ulCurFace;
    ULONG ulCurSize;
    ULONG ulCurAttr;
    BYTE  abCurFG[3];
    BYTE  abCurBG[3];
    ULONG ulCurFamily;
} RtfStateRec;

/*!
 * @brief RTF parser state shared across one parse pass.
 */
typedef struct {
    RtfDocRec* pDoc;                   /*!< Owning document. */
    RtfTopicRec* pCurTopic;            /*!< Topic currently being built. */
    char* pszTitle;                    /*!< Pending title (from $ footnote). */
    char* pszContext;                  /*!< Pending context (from # footnote). */

    ULONG ulCurFace;                   /*!< Current font face index. */
    ULONG ulCurSize;                   /*!< Current size in half-points. */
    ULONG ulCurAttr;                   /*!< Current attribute bits. */
    BYTE  abCurFG[3];                  /*!< Current foreground RGB. */
    BYTE  abCurBG[3];                  /*!< Current background RGB. */
    ULONG ulCurFamily;                 /*!< Current font family code. */

    RtfStateRec aStateStack[RTF_MAX_STATE_STACK];
    int iStateStackTop;                /*!< Current depth of the state stack. */

    BOOL  fHaveTextFrag;               /*!< TRUE once a text fragment exists. */
    LONG  lLastTextFont;               /*!< Font index of last text fragment. */

    BOOL  fInStrike;                   /*!< Inside \\strike span. */
    BOOL  fInUlDB;                     /*!< Inside \\uldb span. */
    BOOL  fInUL;                       /*!< Inside \\ul span. */
    BOOL  fInV;                        /*!< Inside \\v hidden span. */
    char  szLinkText[RTF_MAX_LINKTEXT]; /*!< Link label buffer. */
    ULONG ulLinkTextLen;               /*!< Bytes used in szLinkText. */
    char  szHidden[RTF_MAX_HIDDEN];    /*!< Hidden-text buffer. */
    ULONG ulHiddenLen;                 /*!< Bytes used in szHidden. */

    BOOL  fInFootnote;                 /*!< Inside a {\\footnote ...} group. */
    char  chFootnote;                  /*!< Footnote marker ($, #, K, ...). */
    char  szFootnote[RTF_MAX_FOOTNOTE]; /*!< Footnote content buffer. */
    ULONG ulFootnoteLen;               /*!< Bytes used in szFootnote. */
    ULONG ulFootnoteBrace;             /*!< Brace level at footnote start. */

    ULONG ulBraceLevel;                /*!< Current brace nesting level. */

    BOOL  fSkipBuildTag;               /*!< Skipping an inactive tag region. */
    ULONG ulSkipDepth;                 /*!< Brace depth inside skipped region. */

    BOOL  fLastWasSpace;               /*!< TRUE if last emitted char was space. */
} RtfParserCtx;

/* ==================================================================
 * Helpers
 * ================================================================== */

/*!
 * @brief Parse an unsigned decimal number.
 *
 * @param[in,out] pp Cursor. Advanced past the digits.
 *
 * @return Parsed value.
 */
static ULONG rtf_number(const char** pp)
{
    ULONG n = 0;
    while (**pp && isdigit((unsigned char)**pp)) {
        n = n * 10 + (ULONG)(**pp - '0');
        (*pp)++;
    }
    return n;
}

/*!
 * @brief Skip one optional space.
 *
 * @param[in,out] pp Cursor.
 */
static void rtf_skip_space(const char** pp)
{
    if (**pp == ' ') (*pp)++;
}

/*!
 * @brief Test whether a build tag is in the active set.
 *
 * @param[in] pDoc   Document.
 * @param[in] pszTag Tag name. Not NULL.
 *
 * @return TRUE if the tag is active.
 * @retval TRUE   Tag active (or no active set configured).
 * @retval FALSE  Tag not active.
 */
static BOOL rtf_is_tag_active(RtfDocRec* pDoc, PCSZ pszTag)
{
    ULONG n = 0, i;
    if (!pDoc->vActiveTags) return TRUE;
    VectorGetCount(pDoc->vActiveTags, &n);
    if (n == 0) return TRUE;
    for (i = 0; i < n; i++) {
        PSZ s = NULL;
        VectorGetItem(pDoc->vActiveTags, i, &s, sizeof(s), NULL);
        if (s && stricmp(s, pszTag) == 0) return TRUE;
    }
    return FALSE;
}

/* ==================================================================
 * Face names and descriptors
 * ================================================================== */

/*!
 * @brief Get or create the index of a face name.
 *
 * @param[in] pDoc    Document.
 * @param[in] pszName Face name. Not NULL.
 *
 * @return Index of the face name.
 * @retval 0  Empty/NULL name or allocation failure.
 * @retval >0 Index of the newly created or existing face name.
 */
static ULONG rtf_face_get_or_create(RtfDocRec* pDoc, const char* pszName)
{
    ULONG n = 0, i;
    if (!pszName || !*pszName) return 0;
    VectorGetCount(pDoc->vFaceNames, &n);
    for (i = 0; i < n; i++) {
        PSZ s = NULL;
        VectorGetItem(pDoc->vFaceNames, i, &s, sizeof(s), NULL);
        if (s && stricmp(s, pszName) == 0) return i;
    }
    {
        PSZ pszCopy = strdup(pszName);
        if (!pszCopy) return 0;
        VectorAdd(pDoc->vFaceNames, &pszCopy);
    }
    return n;
}

/*!
 * @brief Get or create a font descriptor.
 *
 * @param[in] pDoc         Document.
 * @param[in] ulFace       Face index.
 * @param[in] ulHalfPoints Size in half-points.
 * @param[in] ulAttributes Attribute bits.
 * @param[in] pFGRGB       Foreground RGB. Not NULL.
 * @param[in] pBGRGB       Background RGB. Not NULL.
 * @param[in] ulFamily     Font family.
 *
 * @return Descriptor index.
 */
static ULONG rtf_font_get_or_create(RtfDocRec* pDoc,
                                    ULONG ulFace,
                                    ULONG ulHalfPoints,
                                    ULONG ulAttributes,
                                    const BYTE* pFGRGB,
                                    const BYTE* pBGRGB,
                                    ULONG ulFamily)
{
    ULONG n = 0, i;
    RtfFontDescRec d;
    VectorGetCount(pDoc->vFontDesc, &n);
    for (i = 0; i < n; i++) {
        VectorGetItem(pDoc->vFontDesc, i, &d, sizeof(d), NULL);
        if (d.ulFaceIndex  == ulFace &&
            d.ulHalfPoints == ulHalfPoints &&
            d.ulAttributes == ulAttributes &&
            d.ulFamily     == ulFamily &&
            memcmp(d.abFGRGB, pFGRGB, 3) == 0 &&
            memcmp(d.abBGRGB, pBGRGB, 3) == 0)
            return i;
    }
    d.ulFaceIndex  = ulFace;
    d.ulHalfPoints = ulHalfPoints;
    d.ulAttributes = ulAttributes;
    d.ulFamily     = ulFamily;
    memcpy(d.abFGRGB, pFGRGB, 3);
    memcpy(d.abBGRGB, pBGRGB, 3);
    VectorAdd(pDoc->vFontDesc, &d);
    return n;
}

/* ==================================================================
 * Topic building
 * ================================================================== */

/*!
 * @brief Free a topic and all fragments it owns.
 *
 * @param[in] pTopic Topic. May be NULL.
 */
static void rtf_topic_cleanup(RtfTopicRec* pTopic)
{
    ULONG n, i;
    if (!pTopic) return;
    if (pTopic->vFragments) {
        VectorGetCount(pTopic->vFragments, &n);
        for (i = 0; i < n; i++) {
            RtfFragRec* f = NULL;
            VectorGetItem(pTopic->vFragments, i, &f, sizeof(f), NULL);
            if (f) {
                if (f->pszText) free(f->pszText);
                if (f->pszContext) free(f->pszContext);
                free(f);
            }
        }
        VectorDestroy(pTopic->vFragments);
    }
    if (pTopic->vBuildTags) {
        VectorGetCount(pTopic->vBuildTags, &n);
        for (i = 0; i < n; i++) {
            PSZ s = NULL;
            VectorGetItem(pTopic->vBuildTags, i, &s, sizeof(s), NULL);
            if (s) free(s);
        }
        VectorDestroy(pTopic->vBuildTags);
    }
    free(pTopic);
}

/*!
 * @brief Commit the current topic to the document.
 *
 * @param[in,out] pCtx Parser context.
 */
static void rtf_flush_topic(RtfParserCtx* pCtx)
{
    ULONG n = 0;
    if (!pCtx->pCurTopic) return;
    VectorGetCount(pCtx->pCurTopic->vFragments, &n);
    if (n == 0 && !pCtx->pszTitle && !pCtx->pszContext) {
        rtf_topic_cleanup(pCtx->pCurTopic);
        pCtx->pCurTopic = NULL;
        return;
    }
    if (pCtx->pszTitle) {
        strncpy(pCtx->pCurTopic->szTitle, pCtx->pszTitle, 255);
        pCtx->pCurTopic->szTitle[255] = '\0';
    }
    if (pCtx->pszContext) {
        strncpy(pCtx->pCurTopic->szContext, pCtx->pszContext, 255);
        pCtx->pCurTopic->szContext[255] = '\0';
    }
    VectorAdd(pCtx->pDoc->vTopics, &pCtx->pCurTopic);
    pCtx->pCurTopic = NULL;
    if (pCtx->pszTitle) { free(pCtx->pszTitle); pCtx->pszTitle = NULL; }
    if (pCtx->pszContext) { free(pCtx->pszContext); pCtx->pszContext = NULL; }
}

/*!
 * @brief Begin a new topic in the parser context.
 *
 * @param[in,out] pCtx Parser context.
 */
static void rtf_start_topic(RtfParserCtx* pCtx)
{
    RtfTopicRec* pTopic;
    APIRET rc;
    rtf_flush_topic(pCtx);
    pTopic = (RtfTopicRec*)malloc(sizeof(RtfTopicRec));
    if (!pTopic) return;
    memset(pTopic, 0, sizeof(*pTopic));
    rc = VectorCreate(sizeof(RtfFragRec*), &pTopic->vFragments);
    if (rc != NO_ERROR) { free(pTopic); return; }
    rc = VectorCreate(sizeof(PSZ), &pTopic->vBuildTags);
    if (rc != NO_ERROR) {
        VectorDestroy(pTopic->vFragments);
        free(pTopic);
        return;
    }
    pCtx->pCurTopic = pTopic;

    pCtx->pszTitle = NULL;
    pCtx->pszContext = NULL;

    pCtx->ulCurFace = 0;
    pCtx->ulCurSize = 20;
    pCtx->ulCurAttr = 0;
    pCtx->abCurFG[0] = 0; pCtx->abCurFG[1] = 0; pCtx->abCurFG[2] = 0;
    pCtx->abCurBG[0] = 0xFF; pCtx->abCurBG[1] = 0xFF; pCtx->abCurBG[2] = 0xFF;
    pCtx->ulCurFamily = 2;

    pCtx->iStateStackTop = 0;

    pCtx->fHaveTextFrag = FALSE;
    pCtx->lLastTextFont = -1;

    pCtx->fInStrike = FALSE;
    pCtx->fInUlDB = FALSE;
    pCtx->fInUL = FALSE;
    pCtx->fInV = FALSE;
    pCtx->ulLinkTextLen = 0;
    pCtx->ulHiddenLen = 0;

    pCtx->fInFootnote = FALSE;
    pCtx->ulFootnoteLen = 0;
    pCtx->chFootnote = 0;

    pCtx->fSkipBuildTag = FALSE;
    pCtx->ulSkipDepth = 0;

    pCtx->fLastWasSpace = TRUE;
}

/*!
 * @brief Emit a text fragment.
 *
 * If the previous fragment has the same font descriptor, the new
 * bytes are appended in place to avoid fragment fragmentation.
 *
 * @param[in,out] pCtx  Parser context.
 * @param[in]     psz   Bytes to emit. Not NULL.
 * @param[in]     ulLen Number of bytes.
 */
static void rtf_emit_text(RtfParserCtx* pCtx, const char* psz, ULONG ulLen)
{
    RtfFragRec* pFrag;
    ULONG ulDesc;

    if (!pCtx->pCurTopic || ulLen == 0) return;
    if (pCtx->fSkipBuildTag) return;

    {
        char ch = psz[ulLen - 1];
        pCtx->fLastWasSpace = (ch == ' ' || ch == '\t' || ch == '\n' ||
                               ch == '\r');
    }

    ulDesc = rtf_font_get_or_create(pCtx->pDoc,
                                    pCtx->ulCurFace,
                                    pCtx->ulCurSize,
                                    pCtx->ulCurAttr,
                                    pCtx->abCurFG,
                                    pCtx->abCurBG,
                                    pCtx->ulCurFamily);

    if (pCtx->fHaveTextFrag && (LONG)ulDesc == pCtx->lLastTextFont) {
        ULONG n = 0;
        RtfFragRec* pLast = NULL;
        VectorGetCount(pCtx->pCurTopic->vFragments, &n);
        if (n > 0) {
            VectorGetItem(pCtx->pCurTopic->vFragments, n - 1, &pLast,
                          sizeof(pLast), NULL);
            if (pLast && pLast->ulKind == RTF_FRAG_TEXT) {
                size_t lo = pLast->pszText ? strlen(pLast->pszText) : 0;
                char* nn = (char*)realloc(pLast->pszText, lo + ulLen + 1);
                if (!nn) return;
                memcpy(nn + lo, psz, ulLen);
                nn[lo + ulLen] = '\0';
                pLast->pszText = nn;
                return;
            }
        }
    }

    pFrag = (RtfFragRec*)malloc(sizeof(RtfFragRec));
    if (!pFrag) return;
    memset(pFrag, 0, sizeof(*pFrag));
    pFrag->ulKind = RTF_FRAG_TEXT;
    pFrag->pszText = (char*)malloc(ulLen + 1);
    if (pFrag->pszText) {
        memcpy(pFrag->pszText, psz, ulLen);
        pFrag->pszText[ulLen] = '\0';
    }
    pFrag->ulFontIndex = ulDesc;
    VectorAdd(pCtx->pCurTopic->vFragments, &pFrag);
    pCtx->fHaveTextFrag = TRUE;
    pCtx->lLastTextFont = (LONG)ulDesc;
}

/*!
 * @brief Emit a link fragment.
 *
 * @param[in,out] pCtx       Parser context.
 * @param[in]     pszVisible Visible label. May be NULL.
 * @param[in]     pszHidden  Target context. May be NULL.
 * @param[in]     fPopup     Popup flag.
 */
static void rtf_emit_link(RtfParserCtx* pCtx, const char* pszVisible,
                          const char* pszHidden, BOOL fPopup)
{
    RtfFragRec* pFrag;
    ULONG ulDesc;
    if (!pCtx->pCurTopic) return;
    if (pCtx->fSkipBuildTag) return;
    pFrag = (RtfFragRec*)malloc(sizeof(RtfFragRec));
    if (!pFrag) return;
    memset(pFrag, 0, sizeof(*pFrag));
    pFrag->ulKind = RTF_FRAG_LINK;

    /* If the visible label is empty, use the hidden text (this is the
       usual case for {\v target text}). */
    if (pszVisible && *pszVisible) {
        pFrag->pszText = strdup(pszVisible);
    } else {
        pFrag->pszText = strdup(pszHidden ? pszHidden : "");
    }
    pFrag->pszContext = strdup(pszHidden ? pszHidden : "");
    pFrag->fPopup = fPopup;
    ulDesc = rtf_font_get_or_create(pCtx->pDoc,
                                    pCtx->ulCurFace,
                                    pCtx->ulCurSize,
                                    pCtx->ulCurAttr,
                                    pCtx->abCurFG,
                                    pCtx->abCurBG,
                                    pCtx->ulCurFamily);
    pFrag->ulFontIndex = ulDesc;
    VectorAdd(pCtx->pCurTopic->vFragments, &pFrag);
    pCtx->fHaveTextFrag = FALSE;
    pCtx->fLastWasSpace = FALSE;
}

/* ==================================================================
 * Font and color table parsers
 * ================================================================== */

/*!
 * @brief Parse a {\\fonttbl ...} group.
 *
 * @param[in,out] pCtx Parser context.
 * @param[in,out] pp   Cursor positioned just past "fonttbl".
 */
static void rtf_parse_fonttbl(RtfParserCtx* pCtx, const char** pp)
{
    const char* p = *pp;
    int brace = 1;
    while (*p && brace > 0) {
        if (*p == '{') {
            brace++;
            if (p[1] == '\\' && p[2] == 'f' && isdigit((unsigned char)p[3])) {
                ULONG ulNum = 0;
                char  szName[256];
                ULONG ulNameLen = 0;
                BOOL  fInCmd = FALSE;
                p++;
                while (*p && *p != '}') {
                    if (*p == '\\') {
                        p++;
                        fInCmd = TRUE;
                        if (*p == 'f' && isdigit((unsigned char)p[1])) {
                            p++;
                            ulNum = rtf_number(&p);
                            rtf_skip_space(&p);
                            continue;
                        } else {
                            while (*p && *p != ' ' && *p != '\\' &&
                                   *p != '}' && *p != '{') p++;
                            if (*p == ' ') p++;
                            fInCmd = FALSE;
                            continue;
                        }
                    } else if (*p == ';') {
                        p++;
                        break;
                    } else {
                        if (fInCmd) fInCmd = FALSE;
                        if (ulNameLen < sizeof(szName) - 1)
                            szName[ulNameLen++] = *p;
                        p++;
                    }
                }
                if (ulNameLen > 0) {
                    while (ulNameLen > 0 && szName[ulNameLen - 1] == ' ')
                        ulNameLen--;
                    szName[ulNameLen] = '\0';
                    rtf_face_get_or_create(pCtx->pDoc, szName);
                }
                (void)ulNum;
                if (*p == '}') p++;
                brace--;
                continue;
            }
            p++;
        } else if (*p == '}') {
            brace--;
            p++;
        } else {
            p++;
        }
    }
    *pp = p;
}

/*!
 * @brief Parse a {\\colortbl ...} group.
 *
 * @param[in,out] pCtx Parser context.
 * @param[in,out] pp   Cursor positioned just past "colortbl".
 */
static void rtf_parse_colortbl(RtfParserCtx* pCtx, const char** pp)
{
    const char* p = *pp;
    int brace = 1;
    int idx = 0;
    BYTE r = 0, g = 0, b = 0;
    while (*p && brace > 0) {
        if (*p == '{') { brace++; p++; continue; }
        if (*p == '}') { brace--; p++; continue; }
        if (*p == ';') {
            RtfColorRec c;
            c.r = r; c.g = g; c.b = b;
            if (idx == 0) { c.r = 0; c.g = 0; c.b = 0; }
            VectorAdd(pCtx->pDoc->vColors, &c);
            idx++;
            r = g = b = 0;
            p++;
            continue;
        }
        if (*p == '\\') {
            p++;
            if (strncmp(p, "red", 3) == 0 && !isalpha((unsigned char)p[3])) {
                p += 3; if (*p == ' ') p++;
                if (isdigit((unsigned char)*p)) r = (BYTE)rtf_number(&p);
            } else if (strncmp(p, "green", 5) == 0 && !isalpha((unsigned char)p[5])) {
                p += 5; if (*p == ' ') p++;
                if (isdigit((unsigned char)*p)) g = (BYTE)rtf_number(&p);
            } else if (strncmp(p, "blue", 4) == 0 && !isalpha((unsigned char)p[4])) {
                p += 4; if (*p == ' ') p++;
                if (isdigit((unsigned char)*p)) b = (BYTE)rtf_number(&p);
            } else {
                while (*p && *p != ' ' && *p != '\\' &&
                       *p != ';' && *p != '}' && *p != '{') p++;
            }
            continue;
        }
        p++;
    }
    *pp = p;
}

/* ==================================================================
 * Footnote handling
 * ================================================================== */

/*!
 * @brief Flush the current footnote into the topic or document.
 *
 * @param[in,out] pCtx Parser context.
 */
static void rtf_flush_footnote(RtfParserCtx* pCtx)
{
    if (!pCtx->fInFootnote) return;
    pCtx->szFootnote[pCtx->ulFootnoteLen] = '\0';

    if (pCtx->chFootnote == '$') {
        if (pCtx->pszTitle) free(pCtx->pszTitle);
        pCtx->pszTitle = strdup(pCtx->szFootnote);
    } else if (pCtx->chFootnote == '#') {
        char* p = pCtx->szFootnote;
        char* pSemi = strchr(p, ';');
        if (pSemi) {
            *pSemi = '\0';
            pSemi++;
            if (pCtx->pszContext) free(pCtx->pszContext);
            pCtx->pszContext = strdup(p);
            while (pSemi && *pSemi && pCtx->pCurTopic) {
                char* pNext = strchr(pSemi, ';');
                if (pNext) { *pNext = '\0'; pNext++; }
                if (*pSemi) {
                    PSZ pszTag = strdup(pSemi);
                    if (pszTag) VectorAdd(pCtx->pCurTopic->vBuildTags, &pszTag);
                }
                pSemi = pNext;
            }
        } else {
            if (pCtx->pszContext) free(pCtx->pszContext);
            pCtx->pszContext = strdup(p);
        }
    }
    pCtx->fInFootnote = FALSE;
    pCtx->ulFootnoteLen = 0;
    pCtx->chFootnote = 0;
}

/* ==================================================================
 * Skip helpers
 * ================================================================== */

/*!
 * @brief Skip a balanced { ... } group.
 *
 * @param[in,out] pp Cursor positioned at the opening '{'.
 */
static void rtf_skip_group(const char** pp)
{
    const char* p = *pp;
    int depth = 1;
    while (*p && depth > 0) {
        if (*p == '{') depth++;
        else if (*p == '}') depth--;
        else if (*p == '\\' && p[1] == '\'') p += 3;
        p++;
    }
    if (*p == '}') p++;
    *pp = p;
}

/*!
 * @brief Read a whitespace-delimited tag name.
 *
 * @param[in,out] pp     Cursor.
 * @param[out]    pszTag Output buffer. Not NULL.
 * @param[in]     cbTag  Size of @a pszTag in bytes.
 */
static void rtf_read_tag(const char** pp, PSZ pszTag, ULONG cbTag)
{
    const char* p = *pp;
    ULONG n = 0;
    while (*p && isspace((unsigned char)*p)) p++;
    while (*p && !isspace((unsigned char)*p) &&
           *p != '}' && *p != '\\' && n < cbTag - 1) {
        pszTag[n++] = *p++;
    }
    pszTag[n] = '\0';
    *pp = p;
}

/* ==================================================================
 * Main parse loop
 * ================================================================== */

/*!
 * @brief Parse the complete RTF buffer.
 *
 * @param[in,out] pCtx   Parser context.
 * @param[in]     pszBuf Buffer. Not NULL.
 * @param[in]     lSize  Buffer length in bytes.
 */
static void rtf_parse_buffer(RtfParserCtx* pCtx, const char* pszBuf,
                             long lSize)
{
    const char* p = pszBuf;
    const char* pEnd = pszBuf + lSize;

    rtf_start_topic(pCtx);

    while (p < pEnd && *p) {
        if (pCtx->fSkipBuildTag && *p != '\\') {
            if (*p == '{') {
                pCtx->ulBraceLevel++;
                pCtx->ulSkipDepth++;
                p++;
                continue;
            }
            if (*p == '}') {
                if (pCtx->ulBraceLevel > 0) pCtx->ulBraceLevel--;
                if (pCtx->ulSkipDepth > 0) pCtx->ulSkipDepth--;
                if (pCtx->ulSkipDepth == 0) pCtx->fSkipBuildTag = FALSE;
                p++;
                continue;
            }
            p++;
            continue;
        }

        if (*p == '\\') {
            p++;
            if (p >= pEnd) break;

            if (*p == '{' || *p == '}' || *p == '\\') {
                char c = *p;
                if (pCtx->fInFootnote) {
                    if (pCtx->ulFootnoteLen < RTF_MAX_FOOTNOTE - 1)
                        pCtx->szFootnote[pCtx->ulFootnoteLen++] = c;
                } else if (pCtx->fInV) {
                    if (pCtx->ulHiddenLen < RTF_MAX_HIDDEN - 1)
                        pCtx->szHidden[pCtx->ulHiddenLen++] = c;
                } else if (pCtx->fInStrike || pCtx->fInUlDB || pCtx->fInUL) {
                    if (pCtx->ulLinkTextLen < RTF_MAX_LINKTEXT - 1)
                        pCtx->szLinkText[pCtx->ulLinkTextLen++] = c;
                } else {
                    rtf_emit_text(pCtx, &c, 1);
                }
                p++;
                continue;
            }

            if (*p == '\'') {
                if (p[1] && p[2]) {
                    char hex[3];
                    int c;
                    char cc;
                    hex[0] = p[1]; hex[1] = p[2]; hex[2] = '\0';
                    c = (int)strtol(hex, NULL, 16);
                    cc = (char)c;
                    if (pCtx->fInFootnote) {
                        if (pCtx->ulFootnoteLen < RTF_MAX_FOOTNOTE - 1)
                            pCtx->szFootnote[pCtx->ulFootnoteLen++] = cc;
                    } else if (pCtx->fInV) {
                        if (pCtx->ulHiddenLen < RTF_MAX_HIDDEN - 1)
                            pCtx->szHidden[pCtx->ulHiddenLen++] = cc;
                    } else if (pCtx->fInStrike || pCtx->fInUlDB || pCtx->fInUL) {
                        if (pCtx->ulLinkTextLen < RTF_MAX_LINKTEXT - 1)
                            pCtx->szLinkText[pCtx->ulLinkTextLen++] = cc;
                    } else {
                        rtf_emit_text(pCtx, &cc, 1);
                    }
                    p += 3;
                } else {
                    p++;
                }
                continue;
            }

            if (*p == '*' && p[1] == '\\') {
                const char* q = p + 2;
                if (strncmp(q, "bkmkstart", 9) == 0 &&
                    !isalpha((unsigned char)q[9])) {
                    char szTag[64];
                    rtf_read_tag(&q, szTag, sizeof(szTag));
                    if (!rtf_is_tag_active(pCtx->pDoc, szTag)) {
                        pCtx->fSkipBuildTag = TRUE;
                        pCtx->ulSkipDepth = 1;
                    }
                    p = q;
                    rtf_skip_group(&p);
                    continue;
                }
                if (strncmp(q, "bkmkend", 7) == 0 &&
                    !isalpha((unsigned char)q[7])) {
                    if (pCtx->ulSkipDepth > 0) {
                        pCtx->ulSkipDepth--;
                        if (pCtx->ulSkipDepth == 0) pCtx->fSkipBuildTag = FALSE;
                    }
                    p = q;
                    rtf_skip_group(&p);
                    continue;
                }
                p = q;
                rtf_skip_group(&p);
                continue;
            }

            if (strncmp(p, "pict", 4) == 0 && !isalpha((unsigned char)p[4])) {
                rtf_skip_group(&p);
                continue;
            }

            if (strncmp(p, "chftn", 5) == 0 && !isalpha((unsigned char)p[5])) {
                p += 5;
                rtf_skip_space(&p);
                continue;
            }

            if (strncmp(p, "fonttbl", 7) == 0 && !isalpha((unsigned char)p[7])) {
                p += 7;
                rtf_skip_space(&p);
                rtf_parse_fonttbl(pCtx, &p);
                continue;
            }
            if (strncmp(p, "colortbl", 8) == 0 && !isalpha((unsigned char)p[8])) {
                p += 8;
                rtf_skip_space(&p);
                rtf_parse_colortbl(pCtx, &p);
                continue;
            }
            if (strncmp(p, "page", 4) == 0 && !isalpha((unsigned char)p[4])) {
                rtf_start_topic(pCtx);
                p += 4;
                rtf_skip_space(&p);
                continue;
            }
            /*
             * Paragraph break: emit a single CR (0x0D).  WinHelp uses
             * \r as a paragraph mark and \n as a soft line break; using
             * \r\n here would be collapsed into a single visual break
             * and blanks between \par\par paragraphs would be lost.
             */
            if (strncmp(p, "par", 3) == 0 && !isalpha((unsigned char)p[3])) {
                /* Emit a single '\r' marker.  topic.c splits the text at '\r'
                   and inserts LinkData1 opcode 0x82 (SameParagraphFormat) between
                   chunks, giving WinHelp a genuine paragraph boundary. */
                rtf_emit_text(pCtx, "\r", 1);
                p += 3; rtf_skip_space(&p); continue;
            }
            if (strncmp(p, "line", 4) == 0 && !isalpha((unsigned char)p[4])) {
                rtf_emit_text(pCtx, "\n", 1);
                p += 4; rtf_skip_space(&p); continue;
            }
            if (strncmp(p, "tab", 3) == 0 && !isalpha((unsigned char)p[3])) {
                rtf_emit_text(pCtx, "\t", 1);
                p += 3; rtf_skip_space(&p); continue;
            }
            if (strncmp(p, "b", 1) == 0 && !isalpha((unsigned char)p[1])) {
                p++;
                if (*p == '0') { pCtx->ulCurAttr &= ~0x01; p++; }
                else pCtx->ulCurAttr |= 0x01;
                rtf_skip_space(&p);
                continue;
            }
            if (strncmp(p, "i", 1) == 0 && !isalpha((unsigned char)p[1])) {
                p++;
                if (*p == '0') { pCtx->ulCurAttr &= ~0x02; p++; }
                else pCtx->ulCurAttr |= 0x02;
                rtf_skip_space(&p);
                continue;
            }
            if (strncmp(p, "strike", 6) == 0 && !isalpha((unsigned char)p[6])) {
                p += 6;
                if (*p == '0') { pCtx->fInStrike = FALSE; p++; }
                else { pCtx->fInStrike = TRUE; pCtx->ulLinkTextLen = 0; }
                rtf_skip_space(&p);
                continue;
            }
            if (strncmp(p, "ulnone", 6) == 0) {
                pCtx->fInUlDB = FALSE;
                pCtx->fInUL = FALSE;
                pCtx->ulCurAttr &= ~0x04;
                p += 6;
                rtf_skip_space(&p);
                continue;
            }
            if (strncmp(p, "uldb", 4) == 0) {
                pCtx->fInUlDB = TRUE;
                pCtx->ulLinkTextLen = 0;
                p += 4;
                rtf_skip_space(&p);
                continue;
            }
            if (strncmp(p, "ul", 2) == 0 && !isalpha((unsigned char)p[2])) {
                p += 2;
                if (*p == '0') {
                    pCtx->ulCurAttr &= ~0x04;
                    pCtx->fInUL = FALSE;
                    p++;
                } else if (!pCtx->fInV) {
                    pCtx->ulCurAttr |= 0x04;
                    pCtx->fInUL = TRUE;
                    pCtx->ulLinkTextLen = 0;
                }
                rtf_skip_space(&p);
                continue;
            }
            if (strncmp(p, "v", 1) == 0 && !isalpha((unsigned char)p[1])) {
                p += 1;
                if (*p == '0') {
                    pCtx->fInV = FALSE;
                    pCtx->szHidden[pCtx->ulHiddenLen] = '\0';
                    pCtx->szLinkText[pCtx->ulLinkTextLen] = '\0';
                    rtf_emit_link(pCtx,
                                  pCtx->szLinkText,
                                  pCtx->szHidden,
                                  pCtx->fInUL ? TRUE : FALSE);
                    pCtx->ulLinkTextLen = 0;
                    pCtx->ulHiddenLen = 0;
                    pCtx->fInStrike = FALSE;
                    pCtx->fInUlDB = FALSE;
                    pCtx->fInUL = FALSE;
                    p++;
                } else {
                    pCtx->fInV = TRUE;
                    pCtx->ulHiddenLen = 0;
                }
                rtf_skip_space(&p);
                continue;
            }
            if (strncmp(p, "plain", 5) == 0) {
                pCtx->ulCurAttr = 0;
                pCtx->ulCurSize = 20;
                pCtx->ulCurFace = 0;
                pCtx->ulCurFamily = 2;
                pCtx->abCurFG[0] = pCtx->abCurFG[1] = pCtx->abCurFG[2] = 0;
                pCtx->abCurBG[0] = pCtx->abCurBG[1] = pCtx->abCurBG[2] = 0xFF;
                p += 5;
                rtf_skip_space(&p);
                continue;
            }
            if (*p == 'f' && isdigit((unsigned char)p[1])) {
                ULONG ulNum;
                ULONG n = 0;
                p++;
                ulNum = rtf_number(&p);
                VectorGetCount(pCtx->pDoc->vFaceNames, &n);
                if (ulNum < n) pCtx->ulCurFace = ulNum;
                rtf_skip_space(&p);
                continue;
            }
            if (strncmp(p, "fs", 2) == 0 && isdigit((unsigned char)p[2])) {
                p += 2;
                pCtx->ulCurSize = rtf_number(&p);
                rtf_skip_space(&p);
                continue;
            }
            if (strncmp(p, "cf", 2) == 0 && isdigit((unsigned char)p[2])) {
                ULONG ulNum;
                ULONG n = 0;
                p += 2;
                ulNum = rtf_number(&p);
                VectorGetCount(pCtx->pDoc->vColors, &n);
                if (ulNum < n) {
                    RtfColorRec c;
                    VectorGetItem(pCtx->pDoc->vColors, ulNum, &c, sizeof(c), NULL);
                    pCtx->abCurFG[0] = c.r;
                    pCtx->abCurFG[1] = c.g;
                    pCtx->abCurFG[2] = c.b;
                }
                rtf_skip_space(&p);
                continue;
            }
            if (strncmp(p, "cb", 2) == 0 && isdigit((unsigned char)p[2])) {
                ULONG ulNum;
                ULONG n = 0;
                p += 2;
                ulNum = rtf_number(&p);
                VectorGetCount(pCtx->pDoc->vColors, &n);
                if (ulNum < n) {
                    RtfColorRec c;
                    VectorGetItem(pCtx->pDoc->vColors, ulNum, &c, sizeof(c), NULL);
                    pCtx->abCurBG[0] = c.r;
                    pCtx->abCurBG[1] = c.g;
                    pCtx->abCurBG[2] = c.b;
                }
                rtf_skip_space(&p);
                continue;
            }

            while (p < pEnd && *p && *p != ' ' &&
                   *p != '\\' && *p != '{' && *p != '}') p++;
            if (*p == ' ') p++;
            continue;
        }

        if (*p == '{') {
            pCtx->ulBraceLevel++;
            /* Save formatting state for this group. */
            if (pCtx->iStateStackTop < RTF_MAX_STATE_STACK) {
                RtfStateRec* s = &pCtx->aStateStack[pCtx->iStateStackTop++];
                s->ulCurFace = pCtx->ulCurFace;
                s->ulCurSize = pCtx->ulCurSize;
                s->ulCurAttr = pCtx->ulCurAttr;
                s->ulCurFamily = pCtx->ulCurFamily;
                memcpy(s->abCurFG, pCtx->abCurFG, 3);
                memcpy(s->abCurBG, pCtx->abCurBG, 3);
            }
            if (p[1] == '\\' && strncmp(p + 2, "footnote", 8) == 0) {
                pCtx->fInFootnote = TRUE;
                pCtx->ulFootnoteLen = 0;
                pCtx->chFootnote = 0;
                pCtx->ulFootnoteBrace = pCtx->ulBraceLevel;
                p += 9;
                continue;
            }
            p++;
            continue;
        }

        if (*p == '}') {
            /* Restore formatting state after group. */
            if (pCtx->iStateStackTop > 0) {
                RtfStateRec* s = &pCtx->aStateStack[--pCtx->iStateStackTop];
                pCtx->ulCurFace = s->ulCurFace;
                pCtx->ulCurSize = s->ulCurSize;
                pCtx->ulCurAttr = s->ulCurAttr;
                pCtx->ulCurFamily = s->ulCurFamily;
                memcpy(pCtx->abCurFG, s->abCurFG, 3);
                memcpy(pCtx->abCurBG, s->abCurBG, 3);
            }

            if (pCtx->fInFootnote &&
                pCtx->ulBraceLevel == pCtx->ulFootnoteBrace) {
                rtf_flush_footnote(pCtx);
            }

            /* Force-close a {\v ...} link at the group boundary. */
            if (pCtx->fInV) {
                pCtx->fInV = FALSE;
                pCtx->szHidden[pCtx->ulHiddenLen] = '\0';
                pCtx->szLinkText[pCtx->ulLinkTextLen] = '\0';
                rtf_emit_link(pCtx, pCtx->szLinkText, pCtx->szHidden,
                              pCtx->fInUL ? TRUE : FALSE);
                pCtx->ulLinkTextLen = 0;
                pCtx->ulHiddenLen = 0;
            }

            if (pCtx->ulBraceLevel > 0) pCtx->ulBraceLevel--;
            p++;
            continue;
        }

        /* Non-standard footnote form: <marker>{ ... \footnote ... }. */
        if (!pCtx->fInFootnote &&
            (*p == '#' || *p == '$' || *p == 'K' ||
             *p == '+' || *p == '!' || *p == '*' || *p == 'N') &&
            p[1] == '{') {
            const char* q = p + 2;
            while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n') q++;
            if (*q == '\\' && strncmp(q + 1, "footnote", 8) == 0 &&
                !isalpha((unsigned char)q[9])) {
                char marker = *p;
                pCtx->fInFootnote = TRUE;
                pCtx->ulFootnoteLen = 0;
                pCtx->chFootnote = marker;
                pCtx->ulBraceLevel++;
                pCtx->ulFootnoteBrace = pCtx->ulBraceLevel;
                p += 2;
                while (*p == '\\') {
                    p++;
                    while (*p && *p != ' ' && *p != '\\' &&
                           *p != '{' && *p != '}') p++;
                    if (*p == ' ') p++;
                }
                while (*p == ' ' || *p == '\t') p++;
                continue;
            }
        }

        /* RTF line breaks in the middle of text are word separators. */
        if ((*p == '\r' || *p == '\n') &&
            !pCtx->fInFootnote && !pCtx->fInV &&
            !pCtx->fInStrike && !pCtx->fInUlDB && !pCtx->fInUL) {
            if (!pCtx->fLastWasSpace) {
                rtf_emit_text(pCtx, " ", 1);
            }
            p++;
            continue;
        }

        if (pCtx->fInFootnote) {
            if (pCtx->chFootnote == 0 && *p != ' ' &&
                *p != '\r' && *p != '\n') {
                pCtx->chFootnote = *p;
                p++;
                while (*p == ' ') p++;
                continue;
            }
            if (pCtx->ulFootnoteLen < RTF_MAX_FOOTNOTE - 1)
                pCtx->szFootnote[pCtx->ulFootnoteLen++] = *p;
        } else if (pCtx->fInV) {
            if (pCtx->ulHiddenLen < RTF_MAX_HIDDEN - 1)
                pCtx->szHidden[pCtx->ulHiddenLen++] = *p;
        } else if (pCtx->fInStrike || pCtx->fInUlDB || pCtx->fInUL) {
            if (pCtx->ulLinkTextLen < RTF_MAX_LINKTEXT - 1)
                pCtx->szLinkText[pCtx->ulLinkTextLen++] = *p;
        } else {
            rtf_emit_text(pCtx, p, 1);
        }
        p++;
    }

    rtf_flush_topic(pCtx);
}

/* ==================================================================
 * Public API: document lifecycle
 * ================================================================== */

/*!
 * @brief Create an empty RTF document.
 *
 * @param[out] phDoc Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phDoc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY RtfCreateDoc(PHRTFDOC phDoc)
{
    RtfDocRec* pDoc;
    APIRET rc;
    if (!phDoc) return ERROR_INVALID_PARAMETER;
    *phDoc = NULLHANDLE;
    pDoc = (RtfDocRec*)malloc(sizeof(RtfDocRec));
    if (!pDoc) return ERROR_NOT_ENOUGH_MEMORY;
    memset(pDoc, 0, sizeof(*pDoc));
    if ((rc = VectorCreate(sizeof(RtfTopicRec*), &pDoc->vTopics)) != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(PSZ), &pDoc->vFaceNames)) != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(RtfFontDescRec), &pDoc->vFontDesc)) != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(RtfColorRec), &pDoc->vColors)) != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(RtfEnumRec*), &pDoc->vCursors)) != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(PSZ), &pDoc->vActiveTags)) != NO_ERROR) goto fail;
    *phDoc = DOC_HANDLE_FROM(pDoc);
    return NO_ERROR;
fail:
    if (pDoc->vTopics) VectorDestroy(pDoc->vTopics);
    if (pDoc->vFaceNames) VectorDestroy(pDoc->vFaceNames);
    if (pDoc->vFontDesc) VectorDestroy(pDoc->vFontDesc);
    if (pDoc->vColors) VectorDestroy(pDoc->vColors);
    if (pDoc->vCursors) VectorDestroy(pDoc->vCursors);
    if (pDoc->vActiveTags) VectorDestroy(pDoc->vActiveTags);
    free(pDoc);
    return rc;
}

/*!
 * @brief Destroy an RTF document.
 *
 * @param[in] hDoc Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULLHANDLE.
 */
APIRET APIENTRY RtfDestroyDoc(HRTFDOC hDoc)
{
    RtfDocRec* pDoc;
    ULONG n, i;
    if (hDoc == NULLHANDLE) return NO_ERROR;
    pDoc = DOC_FROM_HANDLE(hDoc);

    if (pDoc->vCursors) {
        VectorGetCount(pDoc->vCursors, &n);
        for (i = 0; i < n; i++) {
            RtfEnumRec* e = NULL;
            VectorGetItem(pDoc->vCursors, i, &e, sizeof(e), NULL);
            if (e) free(e);
        }
        VectorDestroy(pDoc->vCursors);
    }
    if (pDoc->vTopics) {
        VectorGetCount(pDoc->vTopics, &n);
        for (i = 0; i < n; i++) {
            RtfTopicRec* t = NULL;
            VectorGetItem(pDoc->vTopics, i, &t, sizeof(t), NULL);
            rtf_topic_cleanup(t);
        }
        VectorDestroy(pDoc->vTopics);
    }
    if (pDoc->vFaceNames) {
        VectorGetCount(pDoc->vFaceNames, &n);
        for (i = 0; i < n; i++) {
            PSZ s = NULL;
            VectorGetItem(pDoc->vFaceNames, i, &s, sizeof(s), NULL);
            if (s) free(s);
        }
        VectorDestroy(pDoc->vFaceNames);
    }
    if (pDoc->vFontDesc) VectorDestroy(pDoc->vFontDesc);
    if (pDoc->vColors) VectorDestroy(pDoc->vColors);
    if (pDoc->vActiveTags) {
        VectorGetCount(pDoc->vActiveTags, &n);
        for (i = 0; i < n; i++) {
            PSZ s = NULL;
            VectorGetItem(pDoc->vActiveTags, i, &s, sizeof(s), NULL);
            if (s) free(s);
        }
        VectorDestroy(pDoc->vActiveTags);
    }
    free(pDoc);
    return NO_ERROR;
}

/* ==================================================================
 * Public API: build tags
 * ================================================================== */

/*!
 * @brief Set the active build tag list.
 *
 * @param[in] hDoc     Document. Not NULLHANDLE.
 * @param[in] cTags    Number of tags.
 * @param[in] apszTags Tag array. May be NULL if @a cTags is 0.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 */
APIRET APIENTRY RtfSetBuildTags(HRTFDOC hDoc, ULONG cTags, PCSZ* apszTags)
{
    RtfDocRec* pDoc;
    ULONG i, n = 0;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    pDoc = DOC_FROM_HANDLE(hDoc);
    VectorGetCount(pDoc->vActiveTags, &n);
    for (i = 0; i < n; i++) {
        PSZ s = NULL;
        VectorGetItem(pDoc->vActiveTags, i, &s, sizeof(s), NULL);
        if (s) free(s);
    }
    VectorDestroy(pDoc->vActiveTags);
    VectorCreate(sizeof(PSZ), &pDoc->vActiveTags);
    for (i = 0; i < cTags; i++) {
        PSZ s = strdup(apszTags[i]);
        if (s) VectorAdd(pDoc->vActiveTags, &s);
    }
    return NO_ERROR;
}

/*!
 * @brief Query the number of build tags on a topic.
 *
 * @param[in]  hTopic   Topic. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY RtfQueryTopicBuildTagCount(HRTFTOPIC hTopic, PULONG pulCount)
{
    RtfTopicRec* t = (RtfTopicRec*)hTopic;
    if (hTopic == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(t->vBuildTags, pulCount);
}

/*!
 * @brief Query a topic build tag by index.
 *
 * @param[in]  hTopic  Topic. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTopic is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_FILE_NOT_FOUND     Stored tag is NULL.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RtfQueryTopicBuildTag(HRTFTOPIC hTopic, ULONG ulIndex,
                                      PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    RtfTopicRec* t = (RtfTopicRec*)hTopic;
    PSZ s = NULL;
    APIRET rc;
    ULONG len;
    if (hTopic == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(t->vBuildTags, ulIndex, &s, sizeof(s), NULL);
    if (rc != NO_ERROR) return rc;
    if (!s) return ERROR_FILE_NOT_FOUND;
    len = (ULONG)strlen(s);
    if (!pszBuf && ulSize == 0) { if (pulUsed) *pulUsed = len; return NO_ERROR; }
    if (ulSize < len + 1) { if (pulUsed) *pulUsed = len + 1; return ERROR_BUFFER_OVERFLOW; }
    memcpy(pszBuf, s, len + 1);
    if (pulUsed) *pulUsed = len;
    return NO_ERROR;
}

/* ==================================================================
 * Public API: parsing
 * ================================================================== */

/*!
 * @brief Read and parse one RTF file.
 *
 * @param[in] hDoc        Document. Not NULLHANDLE.
 * @param[in] pszFileName Path to the RTF file. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc or @a pszFileName is NULL.
 * @retval ERROR_FILE_NOT_FOUND     File not found.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY RtfReadFile(HRTFDOC hDoc, PCSZ pszFileName)
{
    FILE* f;
    char* pszBuf;
    long  lSize;
    RtfParserCtx ctx;
    RtfDocRec* pDoc;

    if (hDoc == NULLHANDLE || !pszFileName) return ERROR_INVALID_PARAMETER;
    pDoc = DOC_FROM_HANDLE(hDoc);

    f = fopen(pszFileName, "rb");
    if (!f) return ERROR_FILE_NOT_FOUND;
    fseek(f, 0, SEEK_END);
    lSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    pszBuf = (char*)malloc(lSize + 1);
    if (!pszBuf) { fclose(f); return ERROR_NOT_ENOUGH_MEMORY; }
    if (fread(pszBuf, 1, lSize, f) != (size_t)lSize) {
        free(pszBuf); fclose(f); return ERROR_READ_FAULT;
    }
    pszBuf[lSize] = '\0';
    fclose(f);

    memset(&ctx, 0, sizeof(ctx));
    ctx.pDoc = pDoc;

    rtf_parse_buffer(&ctx, pszBuf, lSize);

    free(pszBuf);
    return NO_ERROR;
}

/* ==================================================================
 * Query: document
 * ================================================================== */

/*!
 * @brief Query the number of topics in the document.
 *
 * @param[in]  hDoc     Document. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY RtfQueryDocTopicCount(HRTFDOC hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vTopics, pulCount);
}

/*!
 * @brief Query the number of face names.
 *
 * @param[in]  hDoc     Document. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY RtfQueryDocFontCount(HRTFDOC hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vFaceNames, pulCount);
}

/*!
 * @brief Query a face name by index.
 *
 * @param[in]  hDoc    Document. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_FILE_NOT_FOUND     No face at @a ulIndex.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RtfQueryDocFontName(HRTFDOC hDoc, ULONG ulIndex,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    PSZ s = NULL;
    APIRET rc;
    ULONG len;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vFaceNames, ulIndex,
                       &s, sizeof(s), NULL);
    if (rc != NO_ERROR) return rc;
    if (!s) return ERROR_FILE_NOT_FOUND;
    len = (ULONG)strlen(s);
    if (!pszBuf && ulSize == 0) { if (pulUsed) *pulUsed = len; return NO_ERROR; }
    if (ulSize < len + 1) { if (pulUsed) *pulUsed = len + 1; return ERROR_BUFFER_OVERFLOW; }
    memcpy(pszBuf, s, len + 1);
    if (pulUsed) *pulUsed = len;
    return NO_ERROR;
}

/*!
 * @brief Query the number of font descriptors.
 *
 * @param[in]  hDoc     Document. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY RtfQueryDocFontDescriptorCount(HRTFDOC hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vFontDesc, pulCount);
}

/*!
 * @brief Query a font descriptor by index.
 *
 * @param[in]  hDoc          Document. Not NULLHANDLE.
 * @param[in]  ulIndex       Zero-based index.
 * @param[out] pulFaceIndex  Optional. Face index.
 * @param[out] pulHalfPoints Optional. Size in half-points.
 * @param[out] pulAttributes Optional. Attribute bits.
 * @param[out] pbFGRGB       Optional. 3-byte foreground RGB.
 * @param[out] pbBGRGB       Optional. 3-byte background RGB.
 * @param[out] pulFamily     Optional. Font family code.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 */
APIRET APIENTRY RtfQueryDocFontDescriptor(HRTFDOC hDoc, ULONG ulIndex,
                                          PULONG pulFaceIndex,
                                          PULONG pulHalfPoints,
                                          PULONG pulAttributes,
                                          PBYTE  pbFGRGB,
                                          PBYTE  pbBGRGB,
                                          PULONG pulFamily)
{
    RtfFontDescRec d;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vFontDesc, ulIndex,
                       &d, sizeof(d), NULL);
    if (rc != NO_ERROR) return rc;
    if (pulFaceIndex)  *pulFaceIndex  = d.ulFaceIndex;
    if (pulHalfPoints) *pulHalfPoints = d.ulHalfPoints;
    if (pulAttributes) *pulAttributes = d.ulAttributes;
    if (pbFGRGB)       memcpy(pbFGRGB, d.abFGRGB, 3);
    if (pbBGRGB)       memcpy(pbBGRGB, d.abBGRGB, 3);
    if (pulFamily)     *pulFamily     = d.ulFamily;
    return NO_ERROR;
}

/*!
 * @brief Query the number of colors.
 *
 * @param[in]  hDoc     Document. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY RtfQueryDocColorCount(HRTFDOC hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vColors, pulCount);
}

/*!
 * @brief Query one color by index.
 *
 * @param[in]  hDoc    Document. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pbRGB   3-byte receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc or @a pbRGB is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 */
APIRET APIENTRY RtfQueryDocColor(HRTFDOC hDoc, ULONG ulIndex, PBYTE pbRGB)
{
    RtfColorRec c;
    APIRET rc;
    if (hDoc == NULLHANDLE || !pbRGB) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vColors, ulIndex,
                       &c, sizeof(c), NULL);
    if (rc != NO_ERROR) return rc;
    pbRGB[0] = c.r; pbRGB[1] = c.g; pbRGB[2] = c.b;
    return NO_ERROR;
}

/* ==================================================================
 * Enumeration
 * ================================================================== */

/*!
 * @brief Allocate and register a topic enumeration cursor.
 *
 * @param[in] pDoc Document.
 *
 * @return New cursor.
 * @retval NULL  Allocation failed.
 */
static RtfEnumRec* rtf_enum_alloc(RtfDocRec* pDoc)
{
    RtfEnumRec* e = (RtfEnumRec*)malloc(sizeof(RtfEnumRec));
    if (!e) return NULL;
    memset(e, 0, sizeof(*e));
    e->hDoc = DOC_HANDLE_FROM(pDoc);
    VectorAdd(pDoc->vCursors, &e);
    return e;
}

/*!
 * @brief Mark an enumeration cursor closed.
 *
 * @param[in] e Cursor. May be NULL.
 */
static void rtf_enum_free(RtfEnumRec* e)
{
    if (e) e->fClosed = TRUE;
}

/*!
 * @brief Open a topic enumeration.
 *
 * @param[in]  hDoc   Document. Not NULLHANDLE.
 * @param[out] phEnum Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_NO_MORE_ITEMS      Document has no topics.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY RtfFindFirstTopic(HRTFDOC hDoc, PHRTFENUM phEnum)
{
    RtfDocRec* pDoc;
    ULONG n = 0;
    RtfEnumRec* e;
    if (hDoc == NULLHANDLE || !phEnum) return ERROR_INVALID_PARAMETER;
    *phEnum = NULLHANDLE;
    pDoc = DOC_FROM_HANDLE(hDoc);
    VectorGetCount(pDoc->vTopics, &n);
    if (n == 0) return ERROR_NO_MORE_ITEMS;
    e = rtf_enum_alloc(pDoc);
    if (!e) return ERROR_NOT_ENOUGH_MEMORY;
    e->ulIndex = 0;
    e->ulKind = 0;
    *phEnum = (HRTFENUM)e;
    return NO_ERROR;
}

/*!
 * @brief Advance a topic enumeration.
 *
 * @param[in] hEnum Cursor. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR              Success.
 * @retval ERROR_INVALID_HANDLE  Cursor is NULL or closed.
 * @retval ERROR_NO_MORE_ITEMS   No more topics.
 */
APIRET APIENTRY RtfFindNextTopic(HRTFENUM hEnum)
{
    RtfEnumRec* e = (RtfEnumRec*)hEnum;
    RtfDocRec* pDoc;
    ULONG n = 0;
    if (hEnum == NULLHANDLE) return ERROR_INVALID_HANDLE;
    if (e->fClosed) return ERROR_INVALID_HANDLE;
    pDoc = DOC_FROM_HANDLE(e->hDoc);
    VectorGetCount(pDoc->vTopics, &n);
    if (e->ulIndex + 1 >= n) return ERROR_NO_MORE_ITEMS;
    e->ulIndex++;
    return NO_ERROR;
}

/*!
 * @brief Query the topic at the cursor.
 *
 * @param[in]  hEnum   Cursor. Not NULLHANDLE.
 * @param[out] phTopic Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Cursor is closed.
 * @retval ERROR_NO_MORE_ITEMS      Cursor position out of range.
 */
APIRET APIENTRY RtfQueryEnumTopic(HRTFENUM hEnum, PHRTFTOPIC phTopic)
{
    RtfEnumRec* e = (RtfEnumRec*)hEnum;
    RtfDocRec* pDoc;
    RtfTopicRec* t = NULL;
    APIRET rc;
    if (hEnum == NULLHANDLE || !phTopic) return ERROR_INVALID_PARAMETER;
    if (e->fClosed) return ERROR_INVALID_HANDLE;
    pDoc = DOC_FROM_HANDLE(e->hDoc);
    rc = VectorGetItem(pDoc->vTopics, e->ulIndex, &t, sizeof(t), NULL);
    if (rc != NO_ERROR) return rc;
    *phTopic = (HRTFTOPIC)t;
    return NO_ERROR;
}

/*!
 * @brief Close an enumeration cursor.
 *
 * @param[in] hEnum Cursor. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULLHANDLE.
 */
APIRET APIENTRY RtfFindClose(HRTFENUM hEnum)
{
    RtfEnumRec* e = (RtfEnumRec*)hEnum;
    if (hEnum == NULLHANDLE) return NO_ERROR;
    rtf_enum_free(e);
    return NO_ERROR;
}

/*!
 * @brief Query a topic title.
 *
 * @param[in]  hTopic  Topic. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTopic is NULL.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RtfQueryTopicTitle(HRTFTOPIC hTopic, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed)
{
    RtfTopicRec* t = (RtfTopicRec*)hTopic;
    ULONG len;
    if (hTopic == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    len = (ULONG)strlen(t->szTitle);
    if (!pszBuf && ulSize == 0) { if (pulUsed) *pulUsed = len; return NO_ERROR; }
    if (ulSize < len + 1) { if (pulUsed) *pulUsed = len + 1; return ERROR_BUFFER_OVERFLOW; }
    memcpy(pszBuf, t->szTitle, len + 1);
    if (pulUsed) *pulUsed = len;
    return NO_ERROR;
}

/*!
 * @brief Query a topic context string.
 *
 * @param[in]  hTopic  Topic. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTopic is NULL.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RtfQueryTopicContext(HRTFTOPIC hTopic, PSZ pszBuf,
                                     ULONG ulSize, PULONG pulUsed)
{
    RtfTopicRec* t = (RtfTopicRec*)hTopic;
    ULONG len;
    if (hTopic == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    len = (ULONG)strlen(t->szContext);
    if (!pszBuf && ulSize == 0) { if (pulUsed) *pulUsed = len; return NO_ERROR; }
    if (ulSize < len + 1) { if (pulUsed) *pulUsed = len + 1; return ERROR_BUFFER_OVERFLOW; }
    memcpy(pszBuf, t->szContext, len + 1);
    if (pulUsed) *pulUsed = len;
    return NO_ERROR;
}

/*!
 * @brief Query the number of fragments in a topic.
 *
 * @param[in]  hTopic   Topic. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY RtfQueryTopicFragmentCount(HRTFTOPIC hTopic, PULONG pulCount)
{
    RtfTopicRec* t = (RtfTopicRec*)hTopic;
    if (hTopic == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(t->vFragments, pulCount);
}

/*!
 * @brief Open a fragment enumeration.
 *
 * @param[in]  hTopic Topic. Not NULLHANDLE.
 * @param[out] phEnum Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_NO_MORE_ITEMS      Topic has no fragments.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY RtfFindFirstFragment(HRTFTOPIC hTopic, PHRTFENUM phEnum)
{
    RtfTopicRec* t = (RtfTopicRec*)hTopic;
    ULONG n = 0;
    RtfEnumRec* e;
    if (hTopic == NULLHANDLE || !phEnum) return ERROR_INVALID_PARAMETER;
    *phEnum = NULLHANDLE;
    VectorGetCount(t->vFragments, &n);
    if (n == 0) return ERROR_NO_MORE_ITEMS;
    e = (RtfEnumRec*)malloc(sizeof(RtfEnumRec));
    if (!e) return ERROR_NOT_ENOUGH_MEMORY;
    memset(e, 0, sizeof(*e));
    e->ulIndex = 0;
    e->ulKind = 1;
    e->hOwner = hTopic;
    *phEnum = (HRTFENUM)e;
    return NO_ERROR;
}

/*!
 * @brief Advance a fragment enumeration.
 *
 * @param[in] hEnum Cursor. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR              Success.
 * @retval ERROR_INVALID_HANDLE  Cursor is NULL, closed, or without owner.
 * @retval ERROR_NO_MORE_ITEMS   No more fragments.
 */
APIRET APIENTRY RtfFindNextFragment(HRTFENUM hEnum)
{
    RtfEnumRec* e = (RtfEnumRec*)hEnum;
    RtfTopicRec* t;
    ULONG n = 0;
    if (hEnum == NULLHANDLE) return ERROR_INVALID_HANDLE;
    if (e->fClosed) return ERROR_INVALID_HANDLE;
    t = (RtfTopicRec*)e->hOwner;
    if (!t) return ERROR_INVALID_HANDLE;
    VectorGetCount(t->vFragments, &n);
    if (e->ulIndex + 1 >= n) return ERROR_NO_MORE_ITEMS;
    e->ulIndex++;
    return NO_ERROR;
}

/*!
 * @brief Query the fragment at the cursor.
 *
 * @param[in]  hEnum  Cursor. Not NULLHANDLE.
 * @param[out] phFrag Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Cursor is closed or without owner.
 * @retval ERROR_NO_MORE_ITEMS      Cursor position out of range.
 */
APIRET APIENTRY RtfQueryEnumFragment(HRTFENUM hEnum, PHRTFFRAG phFrag)
{
    RtfEnumRec* e = (RtfEnumRec*)hEnum;
    RtfTopicRec* t;
    RtfFragRec* f = NULL;
    APIRET rc;
    if (hEnum == NULLHANDLE || !phFrag) return ERROR_INVALID_PARAMETER;
    if (e->fClosed) return ERROR_INVALID_HANDLE;
    t = (RtfTopicRec*)e->hOwner;
    if (!t) return ERROR_INVALID_HANDLE;
    rc = VectorGetItem(t->vFragments, e->ulIndex, &f, sizeof(f), NULL);
    if (rc != NO_ERROR) return rc;
    *phFrag = (HRTFFRAG)f;
    return NO_ERROR;
}

/*!
 * @brief Query the fragment type.
 *
 * @param[in]  hFrag   Fragment. Not NULLHANDLE.
 * @param[out] pulType Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY RtfQueryFragType(HRTFFRAG hFrag, PULONG pulType)
{
    RtfFragRec* f = (RtfFragRec*)hFrag;
    if (hFrag == NULLHANDLE || !pulType) return ERROR_INVALID_PARAMETER;
    *pulType = f->ulKind;
    return NO_ERROR;
}

/*!
 * @brief Query the fragment text.
 *
 * @param[in]  hFrag   Fragment. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hFrag is NULL.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RtfQueryFragText(HRTFFRAG hFrag, PSZ pszBuf,
                                 ULONG ulSize, PULONG pulUsed)
{
    RtfFragRec* f = (RtfFragRec*)hFrag;
    ULONG len;
    if (hFrag == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    len = (ULONG)strlen(f->pszText ? f->pszText : "");
    if (!pszBuf && ulSize == 0) { if (pulUsed) *pulUsed = len; return NO_ERROR; }
    if (ulSize < len + 1) { if (pulUsed) *pulUsed = len + 1; return ERROR_BUFFER_OVERFLOW; }
    if (f->pszText) memcpy(pszBuf, f->pszText, len + 1);
    else pszBuf[0] = '\0';
    if (pulUsed) *pulUsed = len;
    return NO_ERROR;
}

/*!
 * @brief Query the fragment link context.
 *
 * @param[in]  hFrag   Fragment. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hFrag is NULL or not a link.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RtfQueryFragContext(HRTFFRAG hFrag, PSZ pszBuf,
                                    ULONG ulSize, PULONG pulUsed)
{
    RtfFragRec* f = (RtfFragRec*)hFrag;
    ULONG len;
    if (hFrag == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    if (f->ulKind != RTF_FRAG_LINK) return ERROR_INVALID_PARAMETER;
    len = (ULONG)strlen(f->pszContext ? f->pszContext : "");
    if (!pszBuf && ulSize == 0) { if (pulUsed) *pulUsed = len; return NO_ERROR; }
    if (ulSize < len + 1) { if (pulUsed) *pulUsed = len + 1; return ERROR_BUFFER_OVERFLOW; }
    if (f->pszContext) memcpy(pszBuf, f->pszContext, len + 1);
    else pszBuf[0] = '\0';
    if (pulUsed) *pulUsed = len;
    return NO_ERROR;
}

/*!
 * @brief Query the fragment popup flag.
 *
 * @param[in]  hFrag   Fragment. Not NULLHANDLE.
 * @param[out] pfPopup Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hFrag is NULL or not a link.
 */
APIRET APIENTRY RtfQueryFragPopup(HRTFFRAG hFrag, PBOOL pfPopup)
{
    RtfFragRec* f = (RtfFragRec*)hFrag;
    if (hFrag == NULLHANDLE || !pfPopup) return ERROR_INVALID_PARAMETER;
    if (f->ulKind != RTF_FRAG_LINK) return ERROR_INVALID_PARAMETER;
    *pfPopup = f->fPopup;
    return NO_ERROR;
}

/*!
 * @brief Query the fragment font descriptor index.
 *
 * @param[in]  hFrag        Fragment. Not NULLHANDLE.
 * @param[out] pulFontIndex Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY RtfQueryFragFont(HRTFFRAG hFrag, PULONG pulFontIndex)
{
    RtfFragRec* f = (RtfFragRec*)hFrag;
    if (hFrag == NULLHANDLE || !pulFontIndex) return ERROR_INVALID_PARAMETER;
    *pulFontIndex = f->ulFontIndex;
    return NO_ERROR;
}
