/*!
 * @file tom.c
 * @brief |TOMAP implementation.
 */
#include "tom.h"
#include "vector.h"
#include <stdlib.h>

typedef struct { HVECTOR vOffsets; } TomRec;

#define TOM_FROM_HANDLE(h) ((TomRec*)(h))
#define TOM_HANDLE_FROM(d) ((HANDLE)(d))

APIRET APIENTRY TomCreate(PHTOM phTom)
{
    TomRec* p; APIRET rc;
    if (!phTom) return ERROR_INVALID_PARAMETER;
    *phTom = NULLHANDLE;
    p = (TomRec*)malloc(sizeof(TomRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    rc = VectorCreate(sizeof(ULONG), &p->vOffsets);
    if (rc != NO_ERROR) { free(p); return rc; }
    *phTom = TOM_HANDLE_FROM(p);
    return NO_ERROR;
}

APIRET APIENTRY TomDestroy(HTOM hTom)
{
    TomRec* p;
    if (hTom == NULLHANDLE) return NO_ERROR;
    p = TOM_FROM_HANDLE(hTom);
    if (p->vOffsets) VectorDestroy(p->vOffsets);
    free(p);
    return NO_ERROR;
}

APIRET APIENTRY TomAddOffset(HTOM hTom, ULONG ulOffset)
{
    TomRec* p;
    if (hTom == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = TOM_FROM_HANDLE(hTom);
    return VectorAdd(p->vOffsets, &ulOffset);
}

APIRET APIENTRY TomQueryOffsetCount(HTOM hTom, PULONG pulCount)
{
    if (hTom == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(TOM_FROM_HANDLE(hTom)->vOffsets, pulCount);
}

APIRET APIENTRY TomWrite(HTOM hTom, FILE* f)
{
    TomRec* p; ULONG n, i;
    if (hTom == NULLHANDLE || !f) return ERROR_INVALID_PARAMETER;
    p = TOM_FROM_HANDLE(hTom);
    VectorGetCount(p->vOffsets, &n);
    for (i = 0; i < n; i++) {
        ULONG o;
        VectorGetItem(p->vOffsets, i, &o, sizeof(o), NULL);
        fwrite(&o, 4, 1, f);
    }
    return NO_ERROR;
}
