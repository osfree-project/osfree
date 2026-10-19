/*!
 * @file btr.c
 * @brief B+ tree implementation.
 */
#include "btr.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

#pragma pack(1)
/*!
 * @struct BTREEHEADER
 * @brief On-disk B-tree file header (per helpdeco).
 */
typedef struct {
    USHORT usMagic, usFlags, usPageSize;
    UCHAR  abStructure[16];
    USHORT usMustBeZero, usPageSplits, usRootPage, usMustBeNegOne,
           usTotalPages, usNLevels;
    ULONG  ulTotalBtreeEntries;
} BTREEHEADER;
/*!
 * @struct BTREENODEHEADER
 * @brief Header at the start of a B-tree leaf page.
 */
typedef struct {
    USHORT usUnknown, usNEntries, usPreviousPage, usNextPage;
} BTREENODEHEADER;
/*!
 * @struct BTREEINDEXHEADER
 * @brief Header at the start of a B-tree index page.
 */
typedef struct {
    USHORT usUnknown, usNEntries, usPreviousPage;
} BTREEINDEXHEADER;
#pragma pack()

/*!
 * @struct BtrEntryRec
 * @brief In-memory key/value entry.
 */
typedef struct { PSZ pszKey; ULONG ulValue; } BtrEntryRec;

/*!
 * @struct BtrRec
 * @brief B-tree generator state.
 */
typedef struct {
    USHORT  usPageSize;
    USHORT  usFlags;
    CHAR    szStructure[16];
    HVECTOR vEntries;
} BtrRec;

/*!
 * @struct BtrPageRec
 * @brief One leaf or index page description (in-memory).
 */
typedef struct {
    USHORT usPageNum, usIsLeaf;
    PSZ    pszFirstKey;
    ULONG  ulEntryStart, ulEntryCount;
    USHORT usPreviousPageNum;
    ULONG  ulFirstChild, ulLastChild;
} BtrPageRec;

/*!
 * @def BTR_MAX_LEVELS
 * @brief Maximum number of B-tree levels built by BtrWrite.
 */
#define BTR_MAX_LEVELS 8
/*!
 * @def BTR_FROM_HANDLE
 * @brief Convert an HBTR handle to its BtrRec pointer.
 * @param[in] h Handle.
 */
#define BTR_FROM_HANDLE(h) ((BtrRec*)(h))
/*!
 * @def BTR_HANDLE_FROM
 * @brief Convert a BtrRec pointer to an HBTR handle.
 * @param[in] d Record pointer.
 */
#define BTR_HANDLE_FROM(d) ((HANDLE)(d))

/*!
 * @brief qsort() comparator for BtrEntryRec by key.
 * @param[in] a First entry.
 * @param[in] b Second entry.
 * @return strcmp() result of the two keys.
 */
static int btr_cmp(const void* a, const void* b)
{
    return strcmp(((const BtrEntryRec*)a)->pszKey,
                  ((const BtrEntryRec*)b)->pszKey);
}
/*!
 * @brief Compute the serialized size of one leaf entry.
 * @param[in] e Entry.
 * @return Size in bytes (key + NUL + 4-byte value).
 */
static ULONG btr_leaf_size(const BtrEntryRec* e)
{
    return (ULONG)strlen(e->pszKey) + 1 + 4;
}
/*!
 * @brief Compute the serialized size of one index entry.
 * @param[in] k Key.
 * @return Size in bytes (key + NUL + 2-byte page number).
 */
static ULONG btr_idx_size(PCSZ k)
{
    return (ULONG)strlen(k) + 1 + 2;
}
/*!
 * @brief Pad a page with NUL bytes up to the configured page size.
 * @param[in] f           Output stream.
 * @param[in] lStart      Start offset of the page.
 * @param[in] usPageSize  Page size in bytes.
 */
static VOID btr_pad(FILE* f, LONG lStart, USHORT usPageSize)
{
    LONG lPos = ftell(f), lEnd = lStart + (LONG)usPageSize;
    while (lPos < lEnd) { fputc(0, f); lPos++; }
}
/*!
 * @brief Write a key plus a 32-bit value to a stream.
 * @param[in] f Stream.
 * @param[in] k NUL-terminated key.
 * @param[in] v 32-bit value.
 */
static VOID btr_kv(FILE* f, PCSZ k, ULONG v)
{
    fwrite(k, strlen(k) + 1, 1, f);
    fwrite(&v, 4, 1, f);
}
/*!
 * @brief Write a key plus a 16-bit value to a stream.
 * @param[in] f Stream.
 * @param[in] k NUL-terminated key.
 * @param[in] v 16-bit value.
 */
static VOID btr_kw(FILE* f, PCSZ k, USHORT v)
{
    fwrite(k, strlen(k) + 1, 1, f);
    fwrite(&v, 2, 1, f);
}

/*!
 * @brief Create a B-tree.
 *
 * @param[out] phBtr        Receiver. Not NULL. Set to NULLHANDLE on error.
 * @param[in]  usPageSize   Page size in bytes. Pass 0 to select 2048.
 * @param[in]  usFlags      BTR_FLAG_DIRECTORY | BTR_FLAG_ALWAYS for the
 *                          directory index.
 * @param[in]  pszStructure Structure tag up to 15 characters written to
 *                          the BTREEHEADER. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phBtr is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
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
    rc = VectorCreate(sizeof(BtrEntryRec), 0, NULL, &p->vEntries);
    if (rc != NO_ERROR) { free(p); return rc; }
    *phBtr = BTR_HANDLE_FROM(p);
    return NO_ERROR;
}

/*!
 * @brief Destroy a B-tree and release all its keys.
 *
 * @param[in] hBtr B-tree handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULLHANDLE.
 */
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

/*!
 * @brief Append a key/value pair.
 *
 * @param[in] hBtr    B-tree handle. Not NULLHANDLE.
 * @param[in] pszKey  NUL-terminated key. Not NULL.
 * @param[in] ulValue Value associated with the key.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hBtr or @a pszKey is NULL, or
 *                                  the key is already present.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
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

/*!
 * @brief Query an upper-bound size for the serialized B-tree image.
 *
 * @param[in]  hBtr    B-tree handle. Not NULLHANDLE.
 * @param[out] pulSize Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hBtr or @a pulSize is NULL.
 */
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

/*!
 * @brief Split a sorted entry array into leaf pages.
 * @param[in]  a    Sorted entries.
 * @param[in]  n    Number of entries.
 * @param[in]  ps   Page size in bytes.
 * @param[out] pp   Receiver for the page array. Not NULL.
 * @param[out] pc   Receiver for the page count. Not NULL.
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
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

/*!
 * @brief Build one index level from the level below.
 * @param[in]  a    Sorted page array.
 * @param[in]  n    Number of pages.
 * @param[in]  ps   Page size in bytes.
 * @param[out] pp   Receiver for the new page array. Not NULL.
 * @param[out] pc   Receiver for the new page count. Not NULL.
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
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

/*!
 * @brief Serialize the B-tree to a stream.
 *
 * @param[in] hBtr B-tree handle. Not NULLHANDLE.
 * @param[in] f    Output stream. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hBtr or @a f is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
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
