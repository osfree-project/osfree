/*!
 * @file btr.c
 * @brief B+ tree implementation.
 */
#include "btr.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

/* Local fallback: os2types.h does not provide VOID on host builds. */
#ifndef VOID
#define VOID void
#endif

#pragma pack(1)
typedef struct {
    USHORT usMagic, usFlags, usPageSize;
    UCHAR  abStructure[16];
    USHORT usMustBeZero, usPageSplits, usRootPage, usMustBeNegOne,
           usTotalPages, usNLevels;
    ULONG  ulTotalBtreeEntries;
} BTREEHEADER;
typedef struct {
    USHORT usUnknown, usNEntries, usPreviousPage, usNextPage;
} BTREENODEHEADER;
typedef struct {
    USHORT usUnknown, usNEntries, usPreviousPage;
} BTREEINDEXHEADER;
#pragma pack()

typedef struct { PSZ pszKey; ULONG ulValue; } BtrEntryRec;

typedef struct {
    USHORT  usPageSize;
    USHORT  usFlags;
    CHAR    szStructure[16];
    HVECTOR vEntries;
} BtrRec;

typedef struct {
    USHORT usPageNum, usIsLeaf;
    PSZ    pszFirstKey;
    ULONG  ulEntryStart, ulEntryCount;
    USHORT usPreviousPageNum;
    ULONG  ulFirstChild, ulLastChild;
} BtrPageRec;

#define BTR_MAX_LEVELS 8
#define BTR_FROM_HANDLE(h) ((BtrRec*)(h))
#define BTR_HANDLE_FROM(d) ((HANDLE)(d))

static int btr_cmp(const void* a, const void* b)
{
    return strcmp(((const BtrEntryRec*)a)->pszKey,
                  ((const BtrEntryRec*)b)->pszKey);
}
static ULONG btr_leaf_size(const BtrEntryRec* e)
{
    return (ULONG)strlen(e->pszKey) + 1 + 4;
}
static ULONG btr_idx_size(PCSZ k)
{
    return (ULONG)strlen(k) + 1 + 2;
}
static VOID btr_pad(FILE* f, LONG lStart, USHORT usPageSize)
{
    LONG lPos = ftell(f), lEnd = lStart + (LONG)usPageSize;
    while (lPos < lEnd) { fputc(0, f); lPos++; }
}
static VOID btr_kv(FILE* f, PCSZ k, ULONG v)
{
    fwrite(k, strlen(k) + 1, 1, f);
    fwrite(&v, 4, 1, f);
}
static VOID btr_kw(FILE* f, PCSZ k, USHORT v)
{
    fwrite(k, strlen(k) + 1, 1, f);
    fwrite(&v, 2, 1, f);
}

APIRET APIENTRY BtrCreate(PHBTR phBtr, USHORT usPageSize, USHORT usFlags,
                          PCSZ pszStructure)
{
    BtrRec* p; APIRET rc;
    if (!phBtr) return ERROR_INVALID_PARAMETER;
    *phBtr = NULLHANDLE;
    p = (BtrRec*)malloc(sizeof(BtrRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    p->usPageSize = usPageSize ? usPageSize : 0x0800;
    p->usFlags = usFlags ? usFlags : (BTR_FLAG_DIRECTORY | BTR_FLAG_ALWAYS);
    if (pszStructure) {
        strncpy(p->szStructure, pszStructure, 15);
        p->szStructure[15] = '\0';
    }
    rc = VectorCreate(sizeof(BtrEntryRec), &p->vEntries);
    if (rc != NO_ERROR) { free(p); return rc; }
    *phBtr = BTR_HANDLE_FROM(p);
    return NO_ERROR;
}

APIRET APIENTRY BtrDestroy(HBTR hBtr)
{
    BtrRec* p; ULONG n, i;
    if (hBtr == NULLHANDLE) return NO_ERROR;
    p = BTR_FROM_HANDLE(hBtr);
    if (p->vEntries) {
        VectorGetCount(p->vEntries, &n);
        for (i = 0; i < n; i++) {
            BtrEntryRec e;
            VectorGetItem(p->vEntries, i, &e, sizeof(e), NULL);
            if (e.pszKey) free(e.pszKey);
        }
        VectorDestroy(p->vEntries);
    }
    free(p);
    return NO_ERROR;
}

APIRET APIENTRY BtrAddEntry(HBTR hBtr, PCSZ pszKey, ULONG ulValue)
{
    BtrRec* p; BtrEntryRec e; ULONG n, i;
    if (hBtr == NULLHANDLE || !pszKey) return ERROR_INVALID_PARAMETER;
    p = BTR_FROM_HANDLE(hBtr);
    VectorGetCount(p->vEntries, &n);
    for (i = 0; i < n; i++) {
        BtrEntryRec x;
        VectorGetItem(p->vEntries, i, &x, sizeof(x), NULL);
        if (x.pszKey && strcmp(x.pszKey, pszKey) == 0)
            return ERROR_INVALID_PARAMETER;
    }
    e.pszKey = strdup(pszKey);
    if (!e.pszKey) return ERROR_NOT_ENOUGH_MEMORY;
    e.ulValue = ulValue;
    return VectorAdd(p->vEntries, &e);
}

APIRET APIENTRY BtrQuerySize(HBTR hBtr, PULONG pulSize)
{
    BtrRec* p; ULONG n = 0;
    if (hBtr == NULLHANDLE || !pulSize) return ERROR_INVALID_PARAMETER;
    p = BTR_FROM_HANDLE(hBtr);
    VectorGetCount(p->vEntries, &n);
    {
        ULONG usr = (ULONG)p->usPageSize - (ULONG)sizeof(BTREENODEHEADER);
        ULONG per = (usr && usr > 32) ? (usr / 32) : 1;
        ULONG pages = (n + per - 1) / per;
        if (pages == 0) pages = 1;
        *pulSize = pages * (ULONG)p->usPageSize;
    }
    return NO_ERROR;
}

static APIRET btr_leaves(BtrEntryRec* a, ULONG n, USHORT ps,
                         BtrPageRec** pp, ULONG* pc)
{
    ULONG cap = n ? n + 1 : 1, pcnt = 0, i = 0;
    BtrPageRec* ap = (BtrPageRec*)malloc(cap * sizeof(BtrPageRec));
    USHORT payload = (USHORT)(ps - sizeof(BTREENODEHEADER));
    if (!ap) return ERROR_NOT_ENOUGH_MEMORY;
    if (n == 0) {
        memset(&ap[0], 0, sizeof(BtrPageRec));
        ap[0].usIsLeaf = 1;
        ap[0].pszFirstKey = "";
        *pp = ap; *pc = 1;
        return NO_ERROR;
    }
    while (i < n) {
        ULONG start = i; USHORT used = 0;
        ap[pcnt].usPageNum = (USHORT)pcnt;
        ap[pcnt].usIsLeaf = 1;
        ap[pcnt].pszFirstKey = a[i].pszKey;
        ap[pcnt].ulEntryStart = start;
        ap[pcnt].usPreviousPageNum = 0;
        ap[pcnt].ulFirstChild = ap[pcnt].ulLastChild = 0;
        while (i < n) {
            ULONG sz = btr_leaf_size(&a[i]);
            if ((ULONG)used + sz > (ULONG)payload) break;
            used += (USHORT)sz;
            i++;
        }
        ap[pcnt].ulEntryCount = i - start;
        pcnt++;
    }
    *pp = ap; *pc = pcnt;
    return NO_ERROR;
}

static APIRET btr_index(BtrPageRec* a, ULONG n, USHORT ps,
                        BtrPageRec** pp, ULONG* pc)
{
    ULONG cap = n ? n + 1 : 1, pcnt = 0, i = 0;
    BtrPageRec* ap = (BtrPageRec*)malloc(cap * sizeof(BtrPageRec));
    USHORT payload = (USHORT)(ps - sizeof(BTREEINDEXHEADER));
    if (!ap) return ERROR_NOT_ENOUGH_MEMORY;
    while (i < n) {
        USHORT used = 0; ULONG j = i + 1;
        ap[pcnt].usPageNum = 0;
        ap[pcnt].usIsLeaf = 0;
        ap[pcnt].pszFirstKey = a[i].pszFirstKey;
        ap[pcnt].usPreviousPageNum = a[i].usPageNum;
        ap[pcnt].ulFirstChild = i;
        ap[pcnt].ulLastChild = i + 1;
        ap[pcnt].ulEntryStart = ap[pcnt].ulEntryCount = 0;
        while (j < n) {
            ULONG sz = btr_idx_size(a[j].pszFirstKey);
            if ((ULONG)used + sz > (ULONG)payload) break;
            used += (USHORT)sz;
            j++;
        }
        ap[pcnt].ulLastChild = j;
        pcnt++;
        i = j;
    }
    *pp = ap; *pc = pcnt;
    return NO_ERROR;
}

APIRET APIENTRY BtrWrite(HBTR hBtr, FILE* f)
{
    BtrRec* p; ULONG n, i;
    BtrEntryRec* a;
    BtrPageRec* lv[BTR_MAX_LEVELS];
    ULONG lc[BTR_MAX_LEVELS];
    USHORT nlev = 0, root = 0;
    ULONG total = 0;
    LONG fstart;
    APIRET rc;
    USHORT k;

    if (hBtr == NULLHANDLE || !f) return ERROR_INVALID_PARAMETER;
    p = BTR_FROM_HANDLE(hBtr);
    VectorGetCount(p->vEntries, &n);
    fstart = ftell(f);
    memset(lv, 0, sizeof(lv));
    memset(lc, 0, sizeof(lc));
    fseek(f, sizeof(BTREEHEADER), SEEK_CUR);
    a = (BtrEntryRec*)malloc((n ? n : 1) * sizeof(BtrEntryRec));
    if (!a) return ERROR_NOT_ENOUGH_MEMORY;
    for (i = 0; i < n; i++)
        VectorGetItem(p->vEntries, i, &a[i], sizeof(BtrEntryRec), NULL);
    qsort(a, n, sizeof(BtrEntryRec), btr_cmp);

    rc = btr_leaves(a, n, p->usPageSize, &lv[0], &lc[0]);
    if (rc != NO_ERROR) { free(a); return rc; }
    nlev = 1;
    while (lc[nlev - 1] > 1 && nlev < BTR_MAX_LEVELS) {
        rc = btr_index(lv[nlev - 1], lc[nlev - 1], p->usPageSize,
                       &lv[nlev], &lc[nlev]);
        if (rc != NO_ERROR) {
            for (k = 0; k < nlev; k++) free(lv[k]);
            free(a);
            return rc;
        }
        nlev++;
    }
    {
        ULONG next = 0, m; USHORT l;
        for (l = 0; l < nlev; l++)
            for (m = 0; m < lc[l]; m++)
                lv[l][m].usPageNum = (USHORT)next++;
        total = next;
    }
    root = lv[nlev - 1][0].usPageNum;

    {
        ULONG m;
        for (m = 0; m < lc[0]; m++) {
            BtrPageRec* pl = &lv[0][m];
            BTREENODEHEADER nd;
            ULONG q; LONG ps = ftell(f);
            memset(&nd, 0, sizeof(nd));
            nd.usNEntries = (USHORT)pl->ulEntryCount;
            nd.usPreviousPage = (m == 0) ? 0 : lv[0][m - 1].usPageNum;
            nd.usNextPage = (m + 1 < lc[0]) ? lv[0][m + 1].usPageNum : 0xFFFF;
            fwrite(&nd, sizeof(nd), 1, f);
            for (q = 0; q < pl->ulEntryCount; q++) {
                BtrEntryRec* e = &a[pl->ulEntryStart + q];
                btr_kv(f, e->pszKey, e->ulValue);
            }
            btr_pad(f, ps, p->usPageSize);
        }
    }
    {
        USHORT l;
        for (l = 1; l < nlev; l++) {
            ULONG m;
            for (m = 0; m < lc[l]; m++) {
                BtrPageRec* pi = &lv[l][m];
                BTREEINDEXHEADER ih;
                ULONG q; LONG ps = ftell(f);
                memset(&ih, 0, sizeof(ih));
                ih.usNEntries = (USHORT)(pi->ulLastChild - pi->ulFirstChild - 1);
                ih.usPreviousPage = lv[l - 1][pi->ulFirstChild].usPageNum;
                fwrite(&ih, sizeof(ih), 1, f);
                for (q = pi->ulFirstChild + 1; q < pi->ulLastChild; q++) {
                    BtrPageRec* ch = &lv[l - 1][q];
                    btr_kw(f, ch->pszFirstKey, ch->usPageNum);
                }
                btr_pad(f, ps, p->usPageSize);
            }
        }
    }
    {
        BTREEHEADER h;
        LONG save = ftell(f);
        fseek(f, fstart, SEEK_SET);
        memset(&h, 0, sizeof(h));
        h.usMagic = 0x293B;
        h.usFlags = p->usFlags;
        h.usPageSize = p->usPageSize;
        memcpy(h.abStructure, p->szStructure, 16);
        h.usRootPage = root;
        h.usMustBeNegOne = 0xFFFF;
        h.usTotalPages = (USHORT)total;
        h.usNLevels = nlev;
        h.ulTotalBtreeEntries = n;
        fwrite(&h, sizeof(h), 1, f);
        fseek(f, save, SEEK_SET);
    }
    for (k = 0; k < nlev; k++) free(lv[k]);
    free(a);
    return NO_ERROR;
}
