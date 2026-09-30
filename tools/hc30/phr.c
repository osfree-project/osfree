/*!
 * @file phr.c
 * @brief |Phrases implementation.
 */
#include "phr.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

typedef struct { PBYTE pbData; ULONG ulLen; } PhrPhraseRec;
typedef struct { HVECTOR vPhrases; } PhrRec;

#define PHR_FROM_HANDLE(h) ((PhrRec*)(h))
#define PHR_HANDLE_FROM(d) ((HANDLE)(d))

APIRET APIENTRY PhrCreate(PHPHR phPhr)
{
    PhrRec* p; APIRET rc;
    if (!phPhr) return ERROR_INVALID_PARAMETER;
    *phPhr = NULLHANDLE;
    p = (PhrRec*)malloc(sizeof(PhrRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    rc = VectorCreate(sizeof(PhrPhraseRec), &p->vPhrases);
    if (rc != NO_ERROR) { free(p); return rc; }
    *phPhr = PHR_HANDLE_FROM(p);
    return NO_ERROR;
}

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

APIRET APIENTRY PhrQueryPhraseCount(HPHR hPhr, PULONG pulCount)
{
    if (hPhr == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(PHR_FROM_HANDLE(hPhr)->vPhrases, pulCount);
}

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
