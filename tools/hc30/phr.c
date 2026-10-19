/*!
 * @file phr.c
 * @brief |Phrases implementation.
 */
#include "phr.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

/*!
 * @struct PhrPhraseRec
 * @brief One stored phrase: bytes plus length.
 */
typedef struct { PBYTE pbData; ULONG ulLen; } PhrPhraseRec;
/*!
 * @struct PhrRec
 * @brief |Phrases generator state.
 */
typedef struct { HVECTOR vPhrases; } PhrRec;

/*!
 * @def PHR_FROM_HANDLE
 * @brief Convert an HPHR handle to its PhrRec pointer.
 * @param[in] h Handle.
 */
#define PHR_FROM_HANDLE(h) ((PhrRec*)(h))
/*!
 * @def PHR_HANDLE_FROM
 * @brief Convert a PhrRec pointer to an HPHR handle.
 * @param[in] d Record pointer.
 */
#define PHR_HANDLE_FROM(d) ((HANDLE)(d))

/*!
 * @brief Create an empty |Phrases generator.
 *
 * @param[out] phPhr Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phPhr is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrCreate(PHPHR phPhr)
{
    PhrRec* p; APIRET rc;
    if (!phPhr) return ERROR_INVALID_PARAMETER;
    *phPhr = NULLHANDLE;
    p = (PhrRec*)malloc(sizeof(PhrRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    rc = VectorCreate(sizeof(PhrPhraseRec), 0, NULL, &p->vPhrases);
    if (rc != NO_ERROR) { free(p); return rc; }
    *phPhr = PHR_HANDLE_FROM(p);
    return NO_ERROR;
}

/*!
 * @brief Destroy a |Phrases generator.
 *
 * @param[in] hPhr Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULLHANDLE.
 */
APIRET APIENTRY PhrDestroy(HPHR hPhr)
{
    PhrRec* p; ULONG n, i;
    if (hPhr == NULLHANDLE) return NO_ERROR;
    p = PHR_FROM_HANDLE(hPhr);
    if (p->vPhrases) {
        VectorGetCount(p->vPhrases, &n);
        for (i = 0; i < n; i++) {
            PhrPhraseRec r;
            VectorGetItem(p->vPhrases, i, &r, sizeof(r), NULL);
            if (r.pbData) free(r.pbData);
        }
        VectorDestroy(p->vPhrases);
    }
    free(p);
    return NO_ERROR;
}

/*!
 * @brief Append a phrase.
 *
 * @param[in] hPhr   Handle. Not NULLHANDLE.
 * @param[in] pvData Phrase bytes. Not NULL.
 * @param[in] ulLen  Length in bytes. Must be non-zero.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL or @a ulLen is 0.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrAddPhrase(HPHR hPhr, PCVOID pvData, ULONG ulLen)
{
    PhrRec* p; PhrPhraseRec r;
    if (hPhr == NULLHANDLE || !pvData || ulLen == 0)
        return ERROR_INVALID_PARAMETER;
    p = PHR_FROM_HANDLE(hPhr);
    r.pbData = (PBYTE)malloc(ulLen);
    if (!r.pbData) return ERROR_NOT_ENOUGH_MEMORY;
    memcpy(r.pbData, pvData, ulLen);
    r.ulLen = ulLen;
    return VectorAdd(p->vPhrases, &r);
}

/*!
 * @brief Query the number of phrases.
 *
 * @param[in]  hPhr     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY PhrQueryPhraseCount(HPHR hPhr, PULONG pulCount)
{
    if (hPhr == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(PHR_FROM_HANDLE(hPhr)->vPhrases, pulCount);
}

/*!
 * @brief Serialize the |Phrases file to a stream.
 *
 * @param[in] hPhr Handle. Not NULLHANDLE.
 * @param[in] f    Output stream. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hPhr or @a f is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrWrite(HPHR hPhr, FILE* f)
{
    PhrRec* p; ULONG n = 0, i;
    USHORT cnt, flag = 0x0100;
    if (hPhr == NULLHANDLE || !f) return ERROR_INVALID_PARAMETER;
    p = PHR_FROM_HANDLE(hPhr);
    VectorGetCount(p->vPhrases, &n);
    cnt = (USHORT)n;
    fwrite(&cnt, 2, 1, f);
    fwrite(&flag, 2, 1, f);
    if (n == 0) return NO_ERROR;
    {
        PUSHORT off = (PUSHORT)malloc((n + 1) * sizeof(USHORT));
        ULONG acc = 0;
        if (!off) return ERROR_NOT_ENOUGH_MEMORY;
        for (i = 0; i < n; i++) {
            PhrPhraseRec r;
            VectorGetItem(p->vPhrases, i, &r, sizeof(r), NULL);
            off[i] = (USHORT)acc;
            acc += r.ulLen;
        }
        off[n] = (USHORT)acc;
        fwrite(off, 2, n + 1, f);
        free(off);
    }
    for (i = 0; i < n; i++) {
        PhrPhraseRec r;
        VectorGetItem(p->vPhrases, i, &r, sizeof(r), NULL);
        fwrite(r.pbData, 1, r.ulLen, f);
    }
    return NO_ERROR;
}
