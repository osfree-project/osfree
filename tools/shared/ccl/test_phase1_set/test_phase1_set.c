/*!
 *
 * @file test_phase1_set.c
 *
 * @brief Phase 1 tests: set container (hash_private with value size 0).
 *
 * Covers lifecycle, add/contains/remove/clear, unique vs duplicate
 * semantics (CCL_FLAG_ALLOW_DUPLICATES), bucket count, GetIth,
 * pDestroyValue, and the counting allocator.
 */

#include <string.h>

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "set.h"
#include "test_util.h"

/* FNV-1a over the raw key bytes. */
static ULONG APIENTRY h_fnv(PCVOID pKey, PVOID pCtx) {
    const UCHAR *p = (const UCHAR *)pKey;
    ULONG h = 2166136261UL;
    int i;
    (void)pCtx;
    for (i = 0; i < 4; i++) { h ^= p[i]; h *= 16777619UL; }
    return h;
}

static int APIENTRY h_cmp(PCVOID pA, PCVOID pB, PVOID pCtx) {
    (void)pCtx;
    return memcmp(pA, pB, 4);
}

static ULONG g_ulDestroyed = 0;

static void APIENTRY h_destroy(PVOID pElem) {
    (void)pElem;
    g_ulDestroyed++;
}

static void opts_init(PCCLOPTS p, ULONG fl) {
    CclOptsInit(p);
    p->pHash    = h_fnv;
    p->pCompare = h_cmp;
    p->ulFlags  = fl;
}

static void test_set_lifecycle(void) {
    HSET h;
    APIRET rc;
    CCLOPTS opts;

    opts_init(&opts, 0);
    rc = SetCreate(4, 0, &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(SetDestroy(h) == NO_ERROR);

    TEST_ASSERT(SetDestroy(NULLHANDLE) == NO_ERROR);

    TEST_ASSERT(SetCreate(0, 0, NULL, &h) == ERROR_INVALID_PARAMETER);
    TEST_ASSERT(SetCreate(4, 0, NULL, NULL) == ERROR_INVALID_PARAMETER);

    /* NULL opts: pHash required, missing -> error. */
    TEST_ASSERT(SetCreate(4, 0, NULL, &h) == ERROR_INVALID_PARAMETER);
}

static void test_set_add_unique(void) {
    HSET h;
    APIRET rc;
    CCLOPTS opts;
    ULONG k1 = 1, k2 = 2, k3 = 3;
    BOOL fAdded;
    BOOL fPresent;
    ULONG ulCount = 0;

    opts_init(&opts, 0);
    rc = SetCreate(4, 0, &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);

    TEST_ASSERT(SetAdd(h, &k1, &fAdded) == NO_ERROR);
    TEST_ASSERT(fAdded == TRUE);

    TEST_ASSERT(SetAdd(h, &k2, &fAdded) == NO_ERROR);
    TEST_ASSERT(fAdded == TRUE);

    /* Duplicate of k1 in unique mode: no-op. */
    TEST_ASSERT(SetAdd(h, &k1, &fAdded) == NO_ERROR);
    TEST_ASSERT(fAdded == FALSE);

    TEST_ASSERT(SetGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 2);

    TEST_ASSERT(SetContains(h, &k1, &fPresent) == NO_ERROR);
    TEST_ASSERT(fPresent == TRUE);
    TEST_ASSERT(SetContains(h, &k3, &fPresent) == NO_ERROR);
    TEST_ASSERT(fPresent == FALSE);

    TEST_ASSERT(SetDestroy(h) == NO_ERROR);
}

static void test_set_duplicates(void) {
    HSET h;
    APIRET rc;
    CCLOPTS opts;
    ULONG k1 = 100;
    BOOL fAdded;
    ULONG ulCount = 0;
    BOOL fRemoved;

    opts_init(&opts, CCL_FLAG_ALLOW_DUPLICATES);
    rc = SetCreate(4, 0, &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);

    TEST_ASSERT(SetAdd(h, &k1, &fAdded) == NO_ERROR);
    TEST_ASSERT(fAdded == TRUE);
    TEST_ASSERT(SetAdd(h, &k1, &fAdded) == NO_ERROR);
    TEST_ASSERT(fAdded == TRUE);
    TEST_ASSERT(SetAdd(h, &k1, &fAdded) == NO_ERROR);
    TEST_ASSERT(fAdded == TRUE);

    TEST_ASSERT(SetGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 3);

    /* Remove removes one occurrence. */
    TEST_ASSERT(SetRemove(h, &k1, &fRemoved) == NO_ERROR);
    TEST_ASSERT(fRemoved == TRUE);
    TEST_ASSERT(SetGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 2);

    TEST_ASSERT(SetRemove(h, &k1, &fRemoved) == NO_ERROR);
    TEST_ASSERT(fRemoved == TRUE);
    TEST_ASSERT(SetGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 1);

    TEST_ASSERT(SetDestroy(h) == NO_ERROR);
}

static void test_set_remove_clear(void) {
    HSET h;
    APIRET rc;
    CCLOPTS opts;
    ULONG k1 = 1, k2 = 2;
    BOOL f;
    ULONG ulCount;

    opts_init(&opts, 0);
    rc = SetCreate(4, 0, &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);

    SetAdd(h, &k1, NULL);
    SetAdd(h, &k2, NULL);

    TEST_ASSERT(SetRemove(h, &k1, &f) == NO_ERROR);
    TEST_ASSERT(f == TRUE);
    TEST_ASSERT(SetRemove(h, &k1, &f) == NO_ERROR);
    TEST_ASSERT(f == FALSE);

    TEST_ASSERT(SetGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 1);

    TEST_ASSERT(SetClear(h) == NO_ERROR);
    TEST_ASSERT(SetGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 0);
    TEST_ASSERT(SetClear(h) == NO_ERROR);

    TEST_ASSERT(SetDestroy(h) == NO_ERROR);
}

static void test_set_bucket_count(void) {
    HSET h;
    APIRET rc;
    CCLOPTS opts;
    ULONG ulB = 0, k = 1;

    opts_init(&opts, 0);
    rc = SetCreate(4, 0, &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);

    TEST_ASSERT(SetSetBucketCount(h, 32) == NO_ERROR);
    TEST_ASSERT(SetGetBucketCount(h, &ulB) == NO_ERROR);
    TEST_ASSERT(ulB == 32);

    SetAdd(h, &k, NULL);
    TEST_ASSERT(SetSetBucketCount(h, 64) == ERROR_INVALID_PARAMETER);

    TEST_ASSERT(SetDestroy(h) == NO_ERROR);
}

static void test_set_ith(void) {
    HSET h;
    APIRET rc;
    CCLOPTS opts;
    ULONG k1 = 1, k2 = 2, k3 = 3;
    ULONG ulCount = 0, ulUsed = 0;
    ULONG out = 0;
    BOOL seen1 = FALSE, seen2 = FALSE, seen3 = FALSE;
    ULONG i;

    opts_init(&opts, 0);
    rc = SetCreate(4, 0, &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);

    SetAdd(h, &k1, NULL);
    SetAdd(h, &k2, NULL);
    SetAdd(h, &k3, NULL);

    SetGetCount(h, &ulCount);
    TEST_ASSERT(ulCount == 3);

    for (i = 0; i < 3; i++) {
        TEST_ASSERT(SetGetIth(h, i, &out, sizeof(out), &ulUsed) == NO_ERROR);
        if (out == k1) seen1 = TRUE;
        else if (out == k2) seen2 = TRUE;
        else if (out == k3) seen3 = TRUE;
    }
    TEST_ASSERT(seen1 && seen2 && seen3);

    TEST_ASSERT(SetGetIth(h, 3, &out, sizeof(out), &ulUsed)
                == ERROR_NO_MORE_ITEMS);

    TEST_ASSERT(SetDestroy(h) == NO_ERROR);
}

static void test_set_destroy_callback(void) {
    HSET h;
    APIRET rc;
    CCLOPTS opts;
    ULONG k1 = 1, k2 = 2, k3 = 3;

    CclOptsInit(&opts);
    opts.pHash        = h_fnv;
    opts.pCompare     = h_cmp;
    opts.pDestroyValue = h_destroy;

    g_ulDestroyed = 0;
    rc = SetCreate(4, 0, &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);

    SetAdd(h, &k1, NULL);
    SetAdd(h, &k2, NULL);
    SetAdd(h, &k3, NULL);
    TEST_ASSERT(g_ulDestroyed == 0);

    SetRemove(h, &k1, NULL);
    TEST_ASSERT(g_ulDestroyed == 1);

    SetClear(h);
    TEST_ASSERT(g_ulDestroyed == 3);

    /* Destroy an empty set: no additional. */
    TEST_ASSERT(SetDestroy(h) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 3);
}

static ULONG g_ulLive = 0;

static PVOID APIENTRY tm(size_t n) { PVOID p = malloc(n); if (p) g_ulLive++; return p; }
static PVOID APIENTRY tc(size_t n, size_t s) { PVOID p = calloc(n, s); if (p) g_ulLive++; return p; }
static PVOID APIENTRY tr(PVOID p, size_t n) { return realloc(p, n); }
static void APIENTRY tf(PVOID p) { if (p) g_ulLive--; free(p); }

static CCL_MALLOC_FN  gm;
static CCL_CALLOC_FN  gc;
static CCL_REALLOC_FN gr;
static CCL_FREE_FN    gf;

static void cnt_install(void) {
    gm = CclMalloc; gc = CclCalloc; gr = CclRealloc; gf = CclFree;
    g_ulLive = 0;
    CclMalloc = tm; CclCalloc = tc; CclRealloc = tr; CclFree = tf;
}
static void cnt_uninstall(void) {
    CclMalloc = gm; CclCalloc = gc; CclRealloc = gr; CclFree = gf;
}

static void test_set_allocator(void) {
    HSET h;
    CCLOPTS opts;
    ULONG k = 1;

    cnt_install();
    opts_init(&opts, 0);
    SetCreate(4, 0, &opts, &h);
    SetAdd(h, &k, NULL);
    TEST_ASSERT(g_ulLive > 0);
    SetDestroy(h);
    TEST_ASSERT(g_ulLive == 0);
    cnt_uninstall();
}

int main(void) {
    TEST_RUN(test_set_lifecycle);
    TEST_RUN(test_set_add_unique);
    TEST_RUN(test_set_duplicates);
    TEST_RUN(test_set_remove_clear);
    TEST_RUN(test_set_bucket_count);
    TEST_RUN(test_set_ith);
    TEST_RUN(test_set_destroy_callback);
    TEST_RUN(test_set_allocator);
    return test_summary();
}
