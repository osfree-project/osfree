/*!
 * @file topic.c
 * @brief |TOPIC implementation with HC30 block organisation,
 *        physical PrevBlock/NextBlock chaining, and phrase encoding.
 */
#include "topic.h"
#include <stdlib.h>
#include <string.h>

#pragma pack(1)

/*! @brief Per-block header at the start of each 0x800-byte TOPIC block. */
typedef struct {
    ULONG ulLastTopicLink;   /*!< Physical offset of last TOPICLINK in block. */
    ULONG ulFirstTopicLink;  /*!< Physical offset of first TOPICLINK in block. */
    ULONG ulLastTopicHeader; /*!< Physical offset of last TOPICHEADER30 in block. */
} TOPICBLOCKHEADER;

/*!
 * @brief TOPICLINK header (21 bytes on disk).
 *
 * Field order per helpdeco.h:
 *   offset  0: BlockSize  (u32)
 *   offset  4: DataLen2   (u32)
 *   offset  8: PrevBlock  (u32)
 *   offset 12: NextBlock  (u32)
 *   offset 16: DataLen1   (u32)
 *   offset 20: RecordType (u8)
 *
 * DataLen1 includes this 21-byte header; BlockSize >= DataLen1.
 */
typedef struct {
    ULONG ulBlockSize;   /*!< Total size INCLUDING this 21-byte header. */
    ULONG ulDataLen2;    /*!< Length of LinkData2. */
    ULONG ulPrevBlock;   /*!< Distance to previous TOPICLINK. */
    ULONG ulNextBlock;   /*!< Distance to next TOPICLINK. */
    ULONG ulDataLen1;    /*!< End offset of LinkData1 (incl. header). */
    UCHAR bRecordType;   /*!< 0x01 content, 0x02 header, ... */
} TOPICLINK;

/*! @brief Per-topic header (HC30 form). */
typedef struct {
    LONG  lBlockSize;     /*!< Total size of following data. */
    short sPrevTopicNum;  /*!< Previous topic number, or -1. */
    short sUnused1;       /*!< Padding. */
    short sNextTopicNum;  /*!< Next topic number, or -1. */
    short sUnused2;       /*!< Padding. */
} TOPICHEADER30;

#pragma pack()

/*! @def TOP_FRAG_TEXT
 *  @brief Fragment kind: text. */
#define TOP_FRAG_TEXT 0

/*! @def TOP_FRAG_LINK
 *  @brief Fragment kind: link. */
#define TOP_FRAG_LINK 1

/*! @def TOP_LINK_PREV_OFFSET
 *  @brief Offset of PrevBlock field within TOPICLINK. */
#define TOP_LINK_PREV_OFFSET 8

/*! @def TOP_LINK_NEXT_OFFSET
 *  @brief Offset of NextBlock field within TOPICLINK. */
#define TOP_LINK_NEXT_OFFSET 12

/*! @def TOP_BLOCKHDR_SIZE
 *  @brief Size of TOPICBLOCKHEADER. */
#define TOP_BLOCKHDR_SIZE    12

/*! @brief One fragment inside a topic. */
typedef struct {
    ULONG ulKind;         /*!< TOP_FRAG_*. */
    PSZ   pszText;        /*!< Text or link label. */
    ULONG ulFontIndex;    /*!< Font descriptor index. */
    ULONG ulTargetTopic;  /*!< Target topic number (links). */
    BOOL  fPopup;         /*!< Popup flag (links). */
} TopFragRec;

/*! @brief One topic with its fragments. */
typedef struct {
    PSZ     pszTitle;     /*!< Topic title. */
    PSZ     pszContext;   /*!< Context string. */
    HVECTOR vFragments;   /*!< Vector of TopFragRec*. */
} TopTopicRec;

/*! @brief TOPIC generator state. */
typedef struct {
    HVECTOR vTopics;      /*!< Vector of TopTopicRec*. */
} TopRec;

/*! @def TOP_FROM_HANDLE
 *  @brief Convert HTOP handle to record pointer.
 *  @param[in] h HTOP handle. */
#define TOP_FROM_HANDLE(h) ((TopRec*)(h))

/*! @def TOP_HANDLE_FROM
 *  @brief Convert TopRec pointer to HTOP handle.
 *  @param[in] d TopRec pointer. */
#define TOP_HANDLE_FROM(d) ((HANDLE)(d))

/* ------------------------------------------------------------------
 * Growing buffer
 * ------------------------------------------------------------------ */

/*! @brief Growing byte buffer. */
typedef struct {
    PBYTE pbData;  /*!< Buffer bytes. */
    ULONG ulLen;   /*!< Current length. */
    ULONG ulCap;   /*!< Capacity. */
} TopBufRec;

/*!
 * @brief Ensure the buffer has room for @a ulExtra more bytes.
 *
 * @param[in,out] pBuf    Buffer.
 * @param[in]     ulExtra Bytes needed.
 *
 * @return APIRET
 * @retval NO_ERROR                 Space is available.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failed.
 */
static APIRET top_buf_reserve(TopBufRec* pBuf, ULONG ulExtra)
{
    if (pBuf->ulLen + ulExtra <= pBuf->ulCap) return NO_ERROR;
    {
        ULONG ulNewCap = pBuf->ulCap ? pBuf->ulCap : 256;
        while (ulNewCap < pBuf->ulLen + ulExtra) ulNewCap *= 2;
        pBuf->pbData = (PBYTE)realloc(pBuf->pbData, ulNewCap);
        if (!pBuf->pbData) return ERROR_NOT_ENOUGH_MEMORY;
        pBuf->ulCap = ulNewCap;
    }
    return NO_ERROR;
}

/*!
 * @brief Append raw bytes to the buffer.
 *
 * @param[in,out] pBuf   Buffer.
 * @param[in]     pvData Bytes to append. Not NULL.
 * @param[in]     ulLen  Number of bytes.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failed.
 */
static APIRET top_buf_append(TopBufRec* pBuf, PCVOID pvData, ULONG ulLen)
{
    APIRET rc = top_buf_reserve(pBuf, ulLen);
    if (rc != NO_ERROR) return rc;
    memcpy(pBuf->pbData + pBuf->ulLen, pvData, ulLen);
    pBuf->ulLen += ulLen;
    return NO_ERROR;
}

/*!
 * @brief Append one byte.
 *
 * @param[in,out] pBuf Buffer.
 * @param[in]     b    Byte value.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failed.
 */
static APIRET top_buf_byte(TopBufRec* pBuf, UCHAR b)
{
    return top_buf_append(pBuf, &b, 1);
}

/*!
 * @brief Append a 16-bit value.
 *
 * @param[in,out] pBuf Buffer.
 * @param[in]     us   Value.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failed.
 */
static APIRET top_buf_u16(TopBufRec* pBuf, USHORT us)
{
    return top_buf_append(pBuf, &us, sizeof(us));
}

/*!
 * @brief Append a 32-bit value.
 *
 * @param[in,out] pBuf Buffer.
 * @param[in]     ul   Value.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failed.
 */
static APIRET top_buf_u32(TopBufRec* pBuf, ULONG ul)
{
    return top_buf_append(pBuf, &ul, sizeof(ul));
}

/*!
 * @brief Append a NUL-terminated string with its terminator.
 *
 * @param[in,out] pBuf Buffer.
 * @param[in]     psz  String. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failed.
 */
static APIRET top_buf_str(TopBufRec* pBuf, PCSZ psz)
{
    return top_buf_append(pBuf, psz, (ULONG)strlen(psz) + 1);
}

/*!
 * @brief Pad the flat buffer so the next TOPICLINK fits in one block.
 *
 * If the remaining space in the current physical 2048-byte block is
 * smaller than the 21-byte TOPICLINK header, append zero bytes up to
 * the block boundary.  The next TOPICLINK then starts at the first
 * data byte of the next block, matching the format's guarantee that
 * a header never straddles a block boundary.
 *
 * @param[in,out] pBuf Flat buffer.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failed.
 */
static APIRET top_align_before_link(TopBufRec* pBuf)
{
    ULONG payload = (ULONG)TOP_BLOCK_SIZE - (ULONG)TOP_BLOCKHDR_SIZE;
    ULONG off = pBuf->ulLen % payload;
    ULONG remaining = payload - off;
    if (remaining < (ULONG)sizeof(TOPICLINK)) {
        UCHAR zeros[64];
        memset(zeros, 0, sizeof(zeros));
        return top_buf_append(pBuf, zeros, remaining);
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------
 * Lifecycle
 * ------------------------------------------------------------------ */

/*!
 * @brief Create an empty |TOPIC generator.
 *
 * @param[out] phTop Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phTop is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY TopCreate(PHTOP phTop)
{
    TopRec* p;
    APIRET  rc;
    if (!phTop) return ERROR_INVALID_PARAMETER;
    *phTop = NULLHANDLE;
    p = (TopRec*)malloc(sizeof(TopRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    rc = VectorCreate(sizeof(TopTopicRec*), &p->vTopics);
    if (rc != NO_ERROR) { free(p); return rc; }
    *phTop = TOP_HANDLE_FROM(p);
    return NO_ERROR;
}

/*!
 * @brief Release one topic and its fragments.
 *
 * @param[in] pTopic Topic. May be NULL.
 */
static void top_free_topic(TopTopicRec* pTopic)
{
    ULONG n, i;
    if (!pTopic) return;
    if (pTopic->pszTitle) free(pTopic->pszTitle);
    if (pTopic->pszContext) free(pTopic->pszContext);
    if (pTopic->vFragments) {
        VectorGetCount(pTopic->vFragments, &n);
        for (i = 0; i < n; i++) {
            TopFragRec* f = NULL;
            VectorGetItem(pTopic->vFragments, i, &f, sizeof(f), NULL);
            if (f) {
                if (f->pszText) free(f->pszText);
                free(f);
            }
        }
        VectorDestroy(pTopic->vFragments);
    }
    free(pTopic);
}

/*!
 * @brief Destroy a |TOPIC generator.
 *
 * @param[in] hTop Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULLHANDLE.
 */
APIRET APIENTRY TopDestroy(HTOP hTop)
{
    TopRec* p;
    ULONG n, i;
    if (hTop == NULLHANDLE) return NO_ERROR;
    p = TOP_FROM_HANDLE(hTop);
    if (p->vTopics) {
        VectorGetCount(p->vTopics, &n);
        for (i = 0; i < n; i++) {
            TopTopicRec* t = NULL;
            VectorGetItem(p->vTopics, i, &t, sizeof(t), NULL);
            top_free_topic(t);
        }
        VectorDestroy(p->vTopics);
    }
    free(p);
    return NO_ERROR;
}

/*!
 * @brief Append a new topic.
 *
 * @param[in] hTop       Handle. Not NULLHANDLE.
 * @param[in] pszTitle   Title. May be NULL.
 * @param[in] pszContext Context string. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTop is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY TopAddTopic(HTOP hTop, PCSZ pszTitle, PCSZ pszContext)
{
    TopRec* p;
    TopTopicRec* t;
    APIRET rc;
    if (hTop == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = TOP_FROM_HANDLE(hTop);
    t = (TopTopicRec*)malloc(sizeof(TopTopicRec));
    if (!t) return ERROR_NOT_ENOUGH_MEMORY;
    memset(t, 0, sizeof(*t));
    if (pszTitle) t->pszTitle = strdup(pszTitle);
    if (pszContext) t->pszContext = strdup(pszContext);
    rc = VectorCreate(sizeof(TopFragRec*), &t->vFragments);
    if (rc != NO_ERROR) { top_free_topic(t); return rc; }
    rc = VectorAdd(p->vTopics, &t);
    if (rc != NO_ERROR) { top_free_topic(t); return rc; }
    return NO_ERROR;
}

/*!
 * @brief Return the last topic, or NULL when the generator is empty.
 *
 * @param[in] pTop State. Not NULL.
 *
 * @return Last topic.
 * @retval NULL  No topics.
 */
static TopTopicRec* top_last_topic(TopRec* pTop)
{
    ULONG n = 0;
    TopTopicRec* t = NULL;
    VectorGetCount(pTop->vTopics, &n);
    if (n == 0) return NULL;
    VectorGetItem(pTop->vTopics, n - 1, &t, sizeof(t), NULL);
    return t;
}

/*!
 * @brief Append text to the last topic.
 *
 * @param[in] hTop    Handle. Not NULLHANDLE.
 * @param[in] pszText Text. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTop or @a pszText is NULL, or
 *                                  there is no current topic.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY TopAddText(HTOP hTop, PCSZ pszText)
{
    TopRec* p;
    TopTopicRec* t;
    TopFragRec* f;
    ULONG n;
    if (hTop == NULLHANDLE || !pszText) return ERROR_INVALID_PARAMETER;
    p = TOP_FROM_HANDLE(hTop);
    t = top_last_topic(p);
    if (!t) return ERROR_INVALID_PARAMETER;

    VectorGetCount(t->vFragments, &n);
    if (n > 0) {
        TopFragRec* last = NULL;
        VectorGetItem(t->vFragments, n - 1, &last, sizeof(last), NULL);
        if (last && last->ulKind == TOP_FRAG_TEXT) {
            size_t lo = strlen(last->pszText);
            size_t la = strlen(pszText);
            char* nn = (char*)realloc(last->pszText, lo + la + 1);
            if (!nn) return ERROR_NOT_ENOUGH_MEMORY;
            memcpy(nn + lo, pszText, la + 1);
            last->pszText = nn;
            return NO_ERROR;
        }
    }

    f = (TopFragRec*)malloc(sizeof(TopFragRec));
    if (!f) return ERROR_NOT_ENOUGH_MEMORY;
    memset(f, 0, sizeof(*f));
    f->ulKind = TOP_FRAG_TEXT;
    f->pszText = strdup(pszText);
    if (!f->pszText) { free(f); return ERROR_NOT_ENOUGH_MEMORY; }
    return VectorAdd(t->vFragments, &f);
}

/*!
 * @brief Append a link to the last topic.
 *
 * @param[in] hTop         Handle. Not NULLHANDLE.
 * @param[in] ulTargetTopic Target topic number.
 * @param[in] fPopup       Popup flag.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTop is NULL, or there is no
 *                                  current topic.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY TopAddLink(HTOP hTop, ULONG ulTargetTopic, BOOL fPopup)
{
    TopRec* p;
    TopTopicRec* t;
    TopFragRec* f;
    if (hTop == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = TOP_FROM_HANDLE(hTop);
    t = top_last_topic(p);
    if (!t) return ERROR_INVALID_PARAMETER;
    f = (TopFragRec*)malloc(sizeof(TopFragRec));
    if (!f) return ERROR_NOT_ENOUGH_MEMORY;
    memset(f, 0, sizeof(*f));
    f->ulKind = TOP_FRAG_LINK;
    f->pszText = strdup("");
    f->ulTargetTopic = ulTargetTopic;
    f->fPopup = fPopup;
    if (!f->pszText) { free(f); return ERROR_NOT_ENOUGH_MEMORY; }
    return VectorAdd(t->vFragments, &f);
}

/*!
 * @brief Set the font of the last text fragment.
 *
 * @param[in] hTop        Handle. Not NULLHANDLE.
 * @param[in] ulFontIndex Font descriptor index.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTop is NULL, or there is no
 *                                  current text fragment.
 */
APIRET APIENTRY TopSetLastFont(HTOP hTop, ULONG ulFontIndex)
{
    TopRec* p;
    TopTopicRec* t;
    ULONG n;
    TopFragRec* f = NULL;
    if (hTop == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = TOP_FROM_HANDLE(hTop);
    t = top_last_topic(p);
    if (!t) return ERROR_INVALID_PARAMETER;
    VectorGetCount(t->vFragments, &n);
    if (n == 0) return ERROR_INVALID_PARAMETER;
    VectorGetItem(t->vFragments, n - 1, &f, sizeof(f), NULL);
    if (f && f->ulKind == TOP_FRAG_TEXT) f->ulFontIndex = ulFontIndex;
    return NO_ERROR;
}

/*!
 * @brief Query the number of topics.
 *
 * @param[in]  hTop     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY TopQueryTopicCount(HTOP hTop, PULONG pulCount)
{
    if (hTop == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(TOP_FROM_HANDLE(hTop)->vTopics, pulCount);
}

/* ------------------------------------------------------------------
 * Serialization
 * ------------------------------------------------------------------ */

/*! @brief Flat offset of one TOPICLINK inside the unblockified stream. */
typedef struct { ULONG ulFlatOffset; } TopLinkOffsetRec;

/*!
 * @brief Emit one topic (header link + content link) into the flat buffer.
 *
 * Layout produced (all sizes LE):
 *
 *   [TOPICLINK 21] BlockSize=33+n1, DataLen2=n1, Prev, Next, DataLen1=33, Rec=0x02
 *   [TOPICHEADER30 12]
 *   [LinkData2 = title + NUL, n1 bytes]
 *
 *   [TOPICLINK 21] BlockSize=21+d1+d2, DataLen2=d2, Prev, Next, DataLen1=21+d1, Rec=0x01
 *   [LinkData1 = d1 bytes]
 *   [LinkData2 = d2 bytes]
 *
 * Before each TOPICLINK the buffer is padded so the 21-byte header does
 * not straddle a physical block boundary.
 *
 * @param[in,out] flat       Flat buffer.
 * @param[in]     t          Topic. Not NULL.
 * @param[in]     num        Topic number (1-based, unused).
 * @param[in,out] vLinks     Vector receiving TopLinkOffsetRec entries.
 * @param[in]     hEnc       Phrase encoder, or NULLHANDLE for plain text.
 * @param[out]    pOutOffset Optional. Receives the topic's flat offset.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET top_emit_topic(TopBufRec* flat, TopTopicRec* t, ULONG num,
                             HVECTOR vLinks, HPHRE hEnc, ULONG* pOutOffset)
{
    TopBufRec d1, d2;
    APIRET rc;
    ULONG nf = 0, i;
    LONG prevFont = -1;
    TopLinkOffsetRec offRec;
    ULONG nTitle;

    memset(&d1, 0, sizeof(d1));
    memset(&d2, 0, sizeof(d2));

    if (pOutOffset) *pOutOffset = flat->ulLen;

    VectorGetCount(t->vFragments, &nf);

    {
        UCHAR abZero[8];
        memset(abZero, 0, sizeof(abZero));
        rc = top_buf_append(&d1, abZero, 8);
        if (rc != NO_ERROR) goto fail;
    }

    for (i = 0; i < nf; i++) {
        TopFragRec* f = NULL;
        VectorGetItem(t->vFragments, i, &f, sizeof(f), NULL);
        if (!f) continue;
        if (f->ulKind == TOP_FRAG_TEXT) {
            if ((LONG)f->ulFontIndex != prevFont) {
                rc = top_buf_byte(&d1, 0x80);
                if (rc != NO_ERROR) goto fail;
                rc = top_buf_u16(&d1, (USHORT)f->ulFontIndex);
                if (rc != NO_ERROR) goto fail;
                prevFont = (LONG)f->ulFontIndex;
            }
            if (hEnc != NULLHANDLE) {
                const char* pszSrc = f->pszText ? f->pszText : "";
                ULONG cbMax = (ULONG)(2 * strlen(pszSrc) + 2);
                PBYTE pbTmp = (PBYTE)malloc(cbMax);
                ULONG cbUsed = 0;
                if (!pbTmp) { rc = ERROR_NOT_ENOUGH_MEMORY; goto fail; }
                rc = PhrEncEncode(hEnc, pszSrc, pbTmp, cbMax, &cbUsed);
                if (rc == NO_ERROR) {
                    rc = top_buf_append(&d2, pbTmp, cbUsed);
                }
                free(pbTmp);
                if (rc != NO_ERROR) goto fail;
            } else {
                rc = top_buf_str(&d2, f->pszText ? f->pszText : "");
                if (rc != NO_ERROR) goto fail;
            }
        } else if (f->ulKind == TOP_FRAG_LINK) {
            rc = top_buf_byte(&d1, f->fPopup ? 0xE0 : 0xE1);
            if (rc != NO_ERROR) goto fail;
            rc = top_buf_u32(&d1, f->ulTargetTopic);
            if (rc != NO_ERROR) goto fail;
            rc = top_buf_str(&d2, f->pszText ? f->pszText : "");
            if (rc != NO_ERROR) goto fail;
            rc = top_buf_byte(&d1, 0x89);
            if (rc != NO_ERROR) goto fail;
            rc = top_buf_str(&d2, "");
            if (rc != NO_ERROR) goto fail;
        }
    }

    rc = top_buf_byte(&d1, 0xFF);
    if (rc != NO_ERROR) goto fail;
    rc = top_buf_str(&d2, "");
    if (rc != NO_ERROR) goto fail;

    nTitle = t->pszTitle ? (ULONG)strlen(t->pszTitle) + 1 : 1;

    /* --- Header link (RecordType = 0x02) --- */
    {
        TOPICLINK lk;
        TOPICHEADER30 hd;

        rc = top_align_before_link(flat);
        if (rc != NO_ERROR) goto fail;

        offRec.ulFlatOffset = flat->ulLen;
        VectorAdd(vLinks, &offRec);

        memset(&lk, 0, sizeof(lk));
        lk.ulBlockSize = 21UL + (ULONG)sizeof(TOPICHEADER30) + nTitle;
        lk.ulDataLen2  = nTitle;
        lk.ulPrevBlock = 0;
        lk.ulNextBlock = 0;
        lk.ulDataLen1  = 21UL + (ULONG)sizeof(TOPICHEADER30);
        lk.bRecordType = 0x02;
        rc = top_buf_append(flat, &lk, sizeof(lk));
        if (rc != NO_ERROR) goto fail;

        memset(&hd, 0, sizeof(hd));
        hd.lBlockSize = (LONG)sizeof(TOPICHEADER30);
        hd.sPrevTopicNum = -1;
        hd.sNextTopicNum = -1;
        (void)num;
        rc = top_buf_append(flat, &hd, sizeof(hd));
        if (rc != NO_ERROR) goto fail;

        if (t->pszTitle) {
            rc = top_buf_str(flat, t->pszTitle);
        } else {
            rc = top_buf_byte(flat, 0);
        }
        if (rc != NO_ERROR) goto fail;
    }

    /* --- Content link (RecordType = 0x01) --- */
    {
        TOPICLINK lk;

        rc = top_align_before_link(flat);
        if (rc != NO_ERROR) goto fail;

        offRec.ulFlatOffset = flat->ulLen;
        VectorAdd(vLinks, &offRec);

        memset(&lk, 0, sizeof(lk));
        lk.ulBlockSize = 21UL + d1.ulLen + d2.ulLen;
        lk.ulDataLen2  = d2.ulLen;
        lk.ulPrevBlock = 0;
        lk.ulNextBlock = 0;
        lk.ulDataLen1  = 21UL + d1.ulLen;
        lk.bRecordType = 0x01;
        rc = top_buf_append(flat, &lk, sizeof(lk));
        if (rc != NO_ERROR) goto fail;
        rc = top_buf_append(flat, d1.pbData, d1.ulLen);
        if (rc != NO_ERROR) goto fail;
        rc = top_buf_append(flat, d2.pbData, d2.ulLen);
        if (rc != NO_ERROR) goto fail;
    }

    free(d1.pbData);
    free(d2.pbData);
    return NO_ERROR;

fail:
    free(d1.pbData);
    free(d2.pbData);
    return rc;
}

/*!
 * @brief Convert a flat offset to its physical offset in the blocked stream.
 *
 * @param[in] ulFlat Flat offset.
 *
 * @return Physical offset.
 */
static ULONG top_flat_to_physical(ULONG ulFlat)
{
    ULONG payload = (ULONG)TOP_BLOCK_SIZE - (ULONG)TOP_BLOCKHDR_SIZE;
    ULONG block = ulFlat / payload;
    ULONG off   = ulFlat % payload;
    return block * (ULONG)TOP_BLOCK_SIZE + (ULONG)TOP_BLOCKHDR_SIZE + off;
}

/*!
 * @brief Split the flat buffer into 0x800-byte blocks and fill block headers.
 *
 * @param[in,out] flat   Flat buffer.
 * @param[in]     vLinks Vector of TopLinkOffsetRec from top_emit_topic.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET top_blockify(TopBufRec* flat, HVECTOR vLinks)
{
    ULONG payload = (ULONG)TOP_BLOCK_SIZE - (ULONG)TOP_BLOCKHDR_SIZE;
    ULONG blocks = (flat->ulLen + payload - 1) / payload;
    ULONG outLen, bi, i, nLinks;
    PBYTE pbOut;
    ULONG src = 0;

    if (blocks == 0) blocks = 1;
    outLen = blocks * (ULONG)TOP_BLOCK_SIZE;
    pbOut = (PBYTE)malloc(outLen);
    if (!pbOut) return ERROR_NOT_ENOUGH_MEMORY;
    memset(pbOut, 0, outLen);

    for (bi = 0; bi < blocks; bi++) {
        ULONG bs = bi * (ULONG)TOP_BLOCK_SIZE;
        ULONG dl = flat->ulLen - src;
        if (dl > payload) dl = payload;
        if (dl) memcpy(pbOut + bs + TOP_BLOCKHDR_SIZE, flat->pbData + src, dl);
        src += dl;
    }

    VectorGetCount(vLinks, &nLinks);
    for (bi = 0; bi < blocks; bi++) {
        ULONG bs = bi * (ULONG)TOP_BLOCK_SIZE;
        ULONG firstLink = 0, lastLink = 0, lastHeader = 0;

        for (i = 0; i < nLinks; i++) {
            TopLinkOffsetRec rec;
            ULONG flatOff, blockIdx, phys;

            VectorGetItem(vLinks, i, &rec, sizeof(rec), NULL);
            flatOff  = rec.ulFlatOffset;
            blockIdx = flatOff / payload;
            if (blockIdx != bi) continue;

            phys = bs + TOP_BLOCKHDR_SIZE + (flatOff % payload);
            if (firstLink == 0 || phys < firstLink) firstLink = phys;
            if (phys > lastLink) lastLink = phys;

            if ((i % 2) == 0) {
                ULONG hdrFlat = flatOff + (ULONG)sizeof(TOPICLINK);
                ULONG hdrBlk  = hdrFlat / payload;
                if (hdrBlk == bi) {
                    ULONG hdrPhys = bs + TOP_BLOCKHDR_SIZE + (hdrFlat % payload);
                    if (hdrPhys > lastHeader) lastHeader = hdrPhys;
                }
            }
        }

        memcpy(pbOut + bs + 0, &lastLink,   sizeof(lastLink));
        memcpy(pbOut + bs + 4, &firstLink,  sizeof(firstLink));
        memcpy(pbOut + bs + 8, &lastHeader, sizeof(lastHeader));
    }

    free(flat->pbData);
    flat->pbData = pbOut;
    flat->ulLen = outLen;
    flat->ulCap = outLen;
    return NO_ERROR;
}

/*!
 * @brief Patch PrevBlock/NextBlock distances in every TOPICLINK.
 *
 * @param[in,out] flat   Blocked buffer.
 * @param[in]     vLinks Vector of TopLinkOffsetRec.
 */
static void top_patch_links(TopBufRec* flat, HVECTOR vLinks)
{
    ULONG n = 0, i;
    VectorGetCount(vLinks, &n);
    if (n == 0) return;

    for (i = 0; i < n; i++) {
        TopLinkOffsetRec rec;
        ULONG ulPhys, ulPrevDist, ulNextDist;

        VectorGetItem(vLinks, i, &rec, sizeof(rec), NULL);
        ulPhys = top_flat_to_physical(rec.ulFlatOffset);

        if (i == 0) {
            ulPrevDist = 0;
        } else {
            TopLinkOffsetRec rp;
            ULONG ulPrevPhys;
            VectorGetItem(vLinks, i - 1, &rp, sizeof(rp), NULL);
            ulPrevPhys = top_flat_to_physical(rp.ulFlatOffset);
            ulPrevDist = ulPhys - ulPrevPhys;
        }
        if (i == n - 1) {
            ulNextDist = 0;
        } else {
            TopLinkOffsetRec rn;
            ULONG ulNextPhys;
            VectorGetItem(vLinks, i + 1, &rn, sizeof(rn), NULL);
            ulNextPhys = top_flat_to_physical(rn.ulFlatOffset);
            ulNextDist = ulNextPhys - ulPhys;
        }

        memcpy(flat->pbData + ulPhys + TOP_LINK_PREV_OFFSET,
               &ulPrevDist, sizeof(ulPrevDist));
        memcpy(flat->pbData + ulPhys + TOP_LINK_NEXT_OFFSET,
               &ulNextDist, sizeof(ulNextDist));
    }
}

/*!
 * @brief Serialize the |TOPIC stream.
 *
 * @param[in]  hTop       Handle. Not NULLHANDLE.
 * @param[in]  f          Output stream. Not NULL.
 * @param[out] phvOffsets Optional. Receives a vector of ULONG offsets.
 * @param[in]  hEnc       Phrase encoder, or NULLHANDLE for plain text.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTop or @a f is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY TopWrite(HTOP hTop, FILE* f, PHVECTOR phvOffsets, HPHRE hEnc)
{
    TopRec* p;
    TopBufRec flat;
    HVECTOR vLinks = NULLHANDLE;
    HVECTOR vF = NULLHANDLE;
    ULONG nt, i;
    APIRET rc;

    if (hTop == NULLHANDLE || !f) return ERROR_INVALID_PARAMETER;
    p = TOP_FROM_HANDLE(hTop);
    VectorGetCount(p->vTopics, &nt);

    memset(&flat, 0, sizeof(flat));

    rc = VectorCreate(sizeof(TopLinkOffsetRec), &vLinks);
    if (rc != NO_ERROR) return rc;
    if (phvOffsets) {
        rc = VectorCreate(sizeof(ULONG), &vF);
        if (rc != NO_ERROR) { VectorDestroy(vLinks); return rc; }
    }

    for (i = 0; i < nt; i++) {
        TopTopicRec* t = NULL;
        ULONG off;
        VectorGetItem(p->vTopics, i, &t, sizeof(t), NULL);
        if (!t) continue;
        rc = top_emit_topic(&flat, t, i + 1, vLinks, hEnc, &off);
        if (rc != NO_ERROR) {
            free(flat.pbData);
            VectorDestroy(vLinks);
            if (vF) VectorDestroy(vF);
            return rc;
        }
        if (vF) VectorAdd(vF, &off);
    }

    rc = top_blockify(&flat, vLinks);
    if (rc != NO_ERROR) {
        free(flat.pbData);
        VectorDestroy(vLinks);
        if (vF) VectorDestroy(vF);
        return rc;
    }

    top_patch_links(&flat, vLinks);
    VectorDestroy(vLinks);

    if (flat.ulLen) fwrite(flat.pbData, 1, flat.ulLen, f);
    if (phvOffsets) *phvOffsets = vF;

    free(flat.pbData);
    return NO_ERROR;
}
