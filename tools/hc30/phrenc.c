/*!
 * @file phrenc.c
 * @brief Phrase-table encoder implementation with automatic
 *        size-benefit selection.
 *
 * The encoder scans the accumulated corpus, keeps the most frequent
 * phrases, and then measures whether the resulting |Phrases table
 * pays for itself.  If the total encoded |TOPIC plus the |Phrases
 * table is not smaller than the raw |TOPIC alone, the table is
 * discarded and the caller falls back to plain text.
 */
#include "phrenc.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*!
 * @def PHRENC_MAX_PHRASES
 * @brief Hard ceiling on phrase-table entries (format limit is 1920).
 */
#define PHRENC_MAX_PHRASES 1920

/*!
 * @def PHRENC_MIN_LEN
 * @brief Minimum phrase length considered by the scanner.
 */
#define PHRENC_MIN_LEN     4

/*!
 * @def PHRENC_MAX_LEN
 * @brief Maximum phrase length considered by the scanner.
 */
#define PHRENC_MAX_LEN     40

/*!
 * @def PHRENC_MIN_COUNT
 * @brief Minimum occurrence count required for a phrase to be kept.
 */
#define PHRENC_MIN_COUNT   3

/*!
 * @brief One phrase candidate.
 */
typedef struct {
    PSZ   pszText;   /*!< NUL-terminated phrase. Heap allocated. */
    ULONG ulLen;     /*!< Cached strlen(pszText). */
    ULONG ulCount;   /*!< Occurrence count. */
    ULONG ulNumber;  /*!< Final phrase index, assigned in build. */
} PhrCandidateRec;

/*!
 * @brief Encoder state.
 */
typedef struct {
    HVECTOR vCandidates;    /*!< PhrCandidateRec by value. */
    HVECTOR vCorpus;        /*!< PSZ copies of all added texts. */
    BOOL    fTableBuilt;    /*!< TRUE once phrenc_build has run. */
    BOOL    fTableKept;     /*!< TRUE if the table survived the size check. */
} PhrEncRec;

/*! @def ENC_FROM_HANDLE
 *  @brief Convert an encoder handle to its record pointer.
 *  @param[in] h Encoder handle. */
#define ENC_FROM_HANDLE(h) ((PhrEncRec*)(h))

/*! @def ENC_HANDLE_FROM
 *  @brief Convert an encoder record pointer to a handle.
 *  @param[in] d Encoder record pointer. */
#define ENC_HANDLE_FROM(d) ((HANDLE)(d))

/* ------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------ */

/*!
 * @brief Encode a string using the phrase table (internal helper).
 *
 * @param[in]  pEnc     Encoder state. Not NULL.
 * @param[in]  pszIn    Input string. Not NULL.
 * @param[out] pbOut    Output buffer. Not NULL.
 * @param[in]  cbOut    Size of the output buffer.
 * @param[out] pcbUsed  Receives the number of bytes written.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_BUFFER_OVERFLOW    Output buffer is too small.
 */
static APIRET phrenc_encode_into(PhrEncRec* pEnc, const char* pszIn,
                                 PBYTE pbOut, ULONG cbOut, PULONG pcbUsed);

/* ------------------------------------------------------------------
 * Candidate list
 * ------------------------------------------------------------------ */

/*!
 * @brief Compare two candidates by text and length.
 *
 * @param[in] a First candidate.
 * @param[in] b Second candidate.
 *
 * @return Comparison result per qsort convention.
 * @retval -1  @a a sorts before @a b.
 * @retval 0   @a a and @a b are equal.
 * @retval 1   @a a sorts after @a b.
 */
static int phrenc_cmp_by_text(const void* a, const void* b)
{
    const PhrCandidateRec* ra = (const PhrCandidateRec*)a;
    const PhrCandidateRec* rb = (const PhrCandidateRec*)b;
    ULONG nMin = ra->ulLen < rb->ulLen ? ra->ulLen : rb->ulLen;
    int cmp = memcmp(ra->pszText, rb->pszText, nMin);
    if (cmp != 0) return cmp;
    if (ra->ulLen < rb->ulLen) return -1;
    if (ra->ulLen > rb->ulLen) return 1;
    return 0;
}

/*!
 * @brief Compare two candidates by count descending, then by length.
 *
 * @param[in] a First candidate.
 * @param[in] b Second candidate.
 *
 * @return Comparison result per qsort convention.
 * @retval -1  @a a sorts before @a b.
 * @retval 0   @a a and @a b are equal.
 * @retval 1   @a a sorts after @a b.
 */
static int phrenc_cmp_by_count(const void* a, const void* b)
{
    const PhrCandidateRec* ra = (const PhrCandidateRec*)a;
    const PhrCandidateRec* rb = (const PhrCandidateRec*)b;
    if (ra->ulCount > rb->ulCount) return -1;
    if (ra->ulCount < rb->ulCount) return 1;
    if (ra->ulLen  > rb->ulLen)  return -1;
    if (ra->ulLen  < rb->ulLen)  return 1;
    return 0;
}

/* ------------------------------------------------------------------
 * Corpus scan
 * ------------------------------------------------------------------ */

/*!
 * @brief Append all candidate substrings of one text to the vector.
 *
 * @param[in] pEnc    Encoder state. Not NULL.
 * @param[in] pszText Text to scan. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET phrenc_scan(PhrEncRec* pEnc, const char* pszText)
{
    ULONG ulLen = (ULONG)strlen(pszText);
    ULONG pos;

    for (pos = 0; pos < ulLen; pos++) {
        ULONG maxL;
        ULONG l;
        if (!isalnum((unsigned char)pszText[pos]) && pszText[pos] != '_')
            continue;
        maxL = ulLen - pos;
        if (maxL > PHRENC_MAX_LEN) maxL = PHRENC_MAX_LEN;
        for (l = PHRENC_MIN_LEN; l <= maxL; l++) {
            PhrCandidateRec r;
            r.pszText = (PSZ)malloc(l + 1);
            if (!r.pszText) return ERROR_NOT_ENOUGH_MEMORY;
            memcpy(r.pszText, pszText + pos, l);
            r.pszText[l] = '\0';
            r.ulLen = l;
            r.ulCount = 1;
            r.ulNumber = 0;
            if (VectorAdd(pEnc->vCandidates, &r) != NO_ERROR) {
                free(r.pszText);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------
 * Benefit analysis
 * ------------------------------------------------------------------ */

/*!
 * @brief Decide whether the current phrase table saves bytes.
 *
 * Computes:
 *   raw_total   = sum over corpus of (strlen+1)
 *   enc_total   = sum over corpus of encode(text).len
 *   table_total = 4 + 2*(n+1) + sum of phrase bodies
 *
 * The table is kept when table_total + enc_total < raw_total + 4,
 * where the "4" is the empty |Phrases header written anyway.
 *
 * @param[in] pEnc Encoder state with a built table.
 *
 * @return TRUE if the table saves bytes, FALSE otherwise.
 * @retval TRUE   Keep the table.
 * @retval FALSE  Discard the table and fall back to plain text.
 */
static BOOL phrenc_is_beneficial(PhrEncRec* pEnc)
{
    ULONG nCorpus = 0, i;
    ULONG rawTotal = 0, encTotal = 0, tableTotal;
    BOOL  fAnyText = FALSE;

    VectorGetCount(pEnc->vCorpus, &nCorpus);
    for (i = 0; i < nCorpus; i++) {
        PSZ text = NULL;
        PBYTE tmp;
        ULONG tmpCap, used = 0;
        VectorGetItem(pEnc->vCorpus, i, &text, sizeof(text), NULL);
        if (!text) continue;
        fAnyText = TRUE;
        rawTotal += (ULONG)strlen(text) + 1;
        tmpCap = (ULONG)(2 * strlen(text)) + 2;
        tmp = (PBYTE)malloc(tmpCap);
        if (!tmp) continue;
        if (phrenc_encode_into(pEnc, text, tmp, tmpCap, &used) == NO_ERROR)
            encTotal += used;
        free(tmp);
    }

    if (!fAnyText) {
        /* Nothing to measure; keep an empty table. */
        return FALSE;
    }

    /* |Phrases size with the current table. */
    tableTotal = 4;                               /* count + flag */
    {
        ULONG nKept = 0;
        VectorGetCount(pEnc->vCandidates, &nKept);
        tableTotal += 2 * (nKept + 1);            /* offset array */
        for (i = 0; i < nKept; i++) {
            PhrCandidateRec r;
            VectorGetItem(pEnc->vCandidates, i, &r, sizeof(r), NULL);
            tableTotal += r.ulLen;
        }
    }

    /* Total file impact:
     *   with phrases:    tableTotal + encTotal
     *   without phrases: 4          + rawTotal
     * Keep the table only if strictly smaller. */
    return (tableTotal + encTotal) < (rawTotal + 4);
}

/* ------------------------------------------------------------------
 * Table building
 * ------------------------------------------------------------------ */

/*!
 * @brief Build the phrase table from accumulated candidates.
 *
 * After the initial candidate merge, the encoder measures whether the
 * resulting table actually reduces total file size.  If not, the
 * table is discarded and `PhrEncWrite` writes an empty |Phrases file.
 *
 * @param[in] pEnc Encoder state. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET phrenc_build(PhrEncRec* pEnc)
{
    HVECTOR v = pEnc->vCandidates;
    ULONG n = 0, i;
    PhrCandidateRec* aSorted;
    HVECTOR vKept;
    APIRET rc;

    if (pEnc->fTableBuilt) return NO_ERROR;

    VectorGetCount(v, &n);
    if (n == 0) {
        rc = VectorCreate(sizeof(PhrCandidateRec), &vKept);
        if (rc != NO_ERROR) return rc;
        VectorDestroy(v);
        pEnc->vCandidates = vKept;
        pEnc->fTableBuilt = TRUE;
        pEnc->fTableKept = FALSE;
        return NO_ERROR;
    }

    aSorted = (PhrCandidateRec*)malloc(n * sizeof(PhrCandidateRec));
    if (!aSorted) return ERROR_NOT_ENOUGH_MEMORY;
    for (i = 0; i < n; i++)
        VectorGetItem(v, i, &aSorted[i], sizeof(PhrCandidateRec), NULL);

    /* Sort by text and merge adjacent duplicates. */
    qsort(aSorted, n, sizeof(PhrCandidateRec), phrenc_cmp_by_text);
    {
        ULONG out = 0;
        for (i = 0; i < n; i++) {
            if (out > 0 &&
                aSorted[out - 1].ulLen == aSorted[i].ulLen &&
                memcmp(aSorted[out - 1].pszText, aSorted[i].pszText,
                       aSorted[i].ulLen) == 0) {
                aSorted[out - 1].ulCount += aSorted[i].ulCount;
                free(aSorted[i].pszText);
            } else {
                if (out != i) aSorted[out] = aSorted[i];
                out++;
            }
        }
        n = out;
    }

    /* Sort by count descending and keep the top entries. */
    qsort(aSorted, n, sizeof(PhrCandidateRec), phrenc_cmp_by_count);

    rc = VectorCreate(sizeof(PhrCandidateRec), &vKept);
    if (rc != NO_ERROR) {
        for (i = 0; i < n; i++) if (aSorted[i].pszText) free(aSorted[i].pszText);
        free(aSorted);
        return rc;
    }

    {
        ULONG ulKept = 0;
        for (i = 0; i < n && ulKept < PHRENC_MAX_PHRASES; i++) {
            if (aSorted[i].ulCount < PHRENC_MIN_COUNT) break;
            aSorted[i].ulNumber = ulKept;
            VectorAdd(vKept, &aSorted[i]);
            ulKept++;
        }
        for (; i < n; i++) {
            if (aSorted[i].pszText) free(aSorted[i].pszText);
        }
    }
    free(aSorted);
    VectorDestroy(v);
    pEnc->vCandidates = vKept;

    /* Size-benefit check: does the table actually help? */
    if (!phrenc_is_beneficial(pEnc)) {
        ULONG nKept = 0;
        VectorGetCount(vKept, &nKept);
        for (i = 0; i < nKept; i++) {
            PhrCandidateRec r;
            VectorGetItem(vKept, i, &r, sizeof(r), NULL);
            if (r.pszText) free(r.pszText);
        }
        VectorDestroy(vKept);
        rc = VectorCreate(sizeof(PhrCandidateRec), &pEnc->vCandidates);
        if (rc != NO_ERROR) return rc;
        pEnc->fTableKept = FALSE;
    } else {
        pEnc->fTableKept = TRUE;
    }

    pEnc->fTableBuilt = TRUE;
    return NO_ERROR;
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */

/*!
 * @brief Create an empty phrase encoder.
 *
 * @param[out] phEnc Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phEnc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrEncCreate(PHPHRE phEnc)
{
    PhrEncRec* p;
    APIRET rc;
    if (!phEnc) return ERROR_INVALID_PARAMETER;
    *phEnc = NULLHANDLE;
    p = (PhrEncRec*)malloc(sizeof(PhrEncRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    rc = VectorCreate(sizeof(PhrCandidateRec), &p->vCandidates);
    if (rc != NO_ERROR) { free(p); return rc; }
    rc = VectorCreate(sizeof(PSZ), &p->vCorpus);
    if (rc != NO_ERROR) { VectorDestroy(p->vCandidates); free(p); return rc; }
    *phEnc = ENC_HANDLE_FROM(p);
    return NO_ERROR;
}

/*!
 * @brief Destroy a phrase encoder and release all its data.
 *
 * @param[in] hEnc Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULLHANDLE.
 */
APIRET APIENTRY PhrEncDestroy(HPHRE hEnc)
{
    PhrEncRec* p;
    ULONG n, i;
    if (hEnc == NULLHANDLE) return NO_ERROR;
    p = ENC_FROM_HANDLE(hEnc);
    if (p->vCandidates) {
        VectorGetCount(p->vCandidates, &n);
        for (i = 0; i < n; i++) {
            PhrCandidateRec r;
            VectorGetItem(p->vCandidates, i, &r, sizeof(r), NULL);
            if (r.pszText) free(r.pszText);
        }
        VectorDestroy(p->vCandidates);
    }
    if (p->vCorpus) {
        VectorGetCount(p->vCorpus, &n);
        for (i = 0; i < n; i++) {
            PSZ s = NULL;
            VectorGetItem(p->vCorpus, i, &s, sizeof(s), NULL);
            if (s) free(s);
        }
        VectorDestroy(p->vCorpus);
    }
    free(p);
    return NO_ERROR;
}

/*!
 * @brief Add a text to the corpus used for phrase table building.
 *
 * @param[in] hEnc    Handle. Not NULLHANDLE.
 * @param[in] pszText Text to scan. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrEncAddText(HPHRE hEnc, PCSZ pszText)
{
    PhrEncRec* p;
    PSZ copy;
    if (hEnc == NULLHANDLE || !pszText) return ERROR_INVALID_PARAMETER;
    p = ENC_FROM_HANDLE(hEnc);
    copy = strdup(pszText);
    if (copy) {
        if (VectorAdd(p->vCorpus, &copy) != NO_ERROR) {
            free(copy);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    return phrenc_scan(p, pszText);
}

/*!
 * @brief Build the phrase table from the accumulated corpus.
 *
 * @param[in] hEnc Handle. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hEnc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrEncBuildTable(HPHRE hEnc)
{
    PhrEncRec* p;
    if (hEnc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = ENC_FROM_HANDLE(hEnc);
    return phrenc_build(p);
}

/*!
 * @brief Encode a string using the phrase table (internal helper).
 *
 * @param[in]  pEnc     Encoder state. Not NULL.
 * @param[in]  pszIn    Input string. Not NULL.
 * @param[out] pbOut    Output buffer. Not NULL.
 * @param[in]  cbOut    Size of the output buffer.
 * @param[out] pcbUsed  Receives the number of bytes written.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_BUFFER_OVERFLOW    Output buffer is too small.
 */
static APIRET phrenc_encode_into(PhrEncRec* pEnc, const char* pszIn,
                                 PBYTE pbOut, ULONG cbOut, PULONG pcbUsed)
{
    ULONG n = 0, i;
    ULONG pos = 0;
    ULONG ulLen;
    ULONG ulOut = 0;

    ulLen = (ULONG)strlen(pszIn);
    VectorGetCount(pEnc->vCandidates, &n);

    while (pos < ulLen) {
        LONG bestIdx = -1;
        ULONG bestLen = 0;
        for (i = 0; i < n; i++) {
            PhrCandidateRec r;
            VectorGetItem(pEnc->vCandidates, i, &r, sizeof(r), NULL);
            if (r.ulLen <= bestLen) continue;
            if (pos + r.ulLen > ulLen) continue;
            if (memcmp(pszIn + pos, r.pszText, r.ulLen) == 0) {
                bestIdx = (LONG)i;
                bestLen = r.ulLen;
            }
        }

        if (bestIdx >= 0) {
            ULONG ulPhrase = (ULONG)bestIdx;
            ULONG ulCode;
            BOOL  fSpace = FALSE;

            if (pos + bestLen < ulLen && pszIn[pos + bestLen] == ' ')
                fSpace = TRUE;

            ulCode = ulPhrase * 2 + (fSpace ? 1 : 0);
            if (ulCode > 3839) {
                bestIdx = -1;
            } else {
                UCHAR b1 = (UCHAR)(ulCode / 256) + 1;
                UCHAR b2 = (UCHAR)(ulCode % 256);
                if (ulOut + 2 > cbOut) return ERROR_BUFFER_OVERFLOW;
                pbOut[ulOut++] = b1;
                pbOut[ulOut++] = b2;
                pos += bestLen + (fSpace ? 1 : 0);
                continue;
            }
        }

        if (ulOut + 1 > cbOut) return ERROR_BUFFER_OVERFLOW;
        pbOut[ulOut++] = (UCHAR)pszIn[pos];
        pos++;
    }

    if (ulOut + 1 > cbOut) return ERROR_BUFFER_OVERFLOW;
    pbOut[ulOut++] = 0;
    *pcbUsed = ulOut;
    return NO_ERROR;
}

/*!
 * @brief Encode a string using the phrase table.
 *
 * @param[in]  hEnc     Handle. Not NULLHANDLE.
 * @param[in]  pszIn    Input string. Not NULL.
 * @param[out] pbOut    Output buffer. Not NULL.
 * @param[in]  cbOut    Size of the output buffer.
 * @param[out] pcbUsed  Receives the number of bytes written.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_BUFFER_OVERFLOW    Output buffer is too small.
 */
APIRET APIENTRY PhrEncEncode(HPHRE hEnc, PCSZ pszIn, PBYTE pbOut,
                             ULONG cbOut, PULONG pcbUsed)
{
    PhrEncRec* p;
    if (hEnc == NULLHANDLE || !pszIn || !pbOut || !pcbUsed)
        return ERROR_INVALID_PARAMETER;
    p = ENC_FROM_HANDLE(hEnc);
    return phrenc_encode_into(p, pszIn, pbOut, cbOut, pcbUsed);
}

/*!
 * @brief Query the number of phrases in the table.
 *
 * @param[in]  hEnc     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY PhrEncQueryCount(HPHRE hEnc, PULONG pulCount)
{
    if (hEnc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(ENC_FROM_HANDLE(hEnc)->vCandidates, pulCount);
}

/*!
 * @brief Write the |Phrases file to a stream.
 *
 * The offset array is always written, even when there are no phrases,
 * because the format requires exactly count+1 offset entries.  An
 * empty table therefore occupies 6 bytes: count (u16), flag (u16)
 * and a single offset word (u16).
 *
 * @param[in] hEnc Handle. Not NULLHANDLE.
 * @param[in] f    Output stream. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hEnc or @a f is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrEncWrite(HPHRE hEnc, FILE* f)
{
    PhrEncRec* p;
    ULONG n = 0, i;
    USHORT count;
    USHORT flag = 0x0100;
    PUSHORT offsets;

    if (hEnc == NULLHANDLE || !f) return ERROR_INVALID_PARAMETER;
    p = ENC_FROM_HANDLE(hEnc);
    VectorGetCount(p->vCandidates, &n);
    if (n > PHRENC_MAX_PHRASES) n = PHRENC_MAX_PHRASES;
    count = (USHORT)n;

    fwrite(&count, sizeof(count), 1, f);
    fwrite(&flag, sizeof(flag), 1, f);

    /*
     * Offset array.  Even an empty table writes one word (the base
     * offset), so the total empty |Phrases is exactly 6 bytes.
     */
    offsets = (PUSHORT)malloc((n + 1) * sizeof(USHORT));
    if (!offsets) return ERROR_NOT_ENOUGH_MEMORY;

    {
        ULONG ulBase = (ULONG)(n + 1) * sizeof(USHORT);
        ULONG acc = ulBase;
        for (i = 0; i < n; i++) {
            PhrCandidateRec r;
            offsets[i] = (USHORT)acc;
            VectorGetItem(p->vCandidates, i, &r, sizeof(r), NULL);
            acc += r.ulLen;
        }
        offsets[n] = (USHORT)acc;
    }
    fwrite(offsets, sizeof(USHORT), n + 1, f);
    free(offsets);

    /* Phrase bodies. Empty when count == 0. */
    for (i = 0; i < n; i++) {
        PhrCandidateRec r;
        VectorGetItem(p->vCandidates, i, &r, sizeof(r), NULL);
        fwrite(r.pszText, 1, r.ulLen, f);
    }
    return NO_ERROR;
}
