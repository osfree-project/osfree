/*!
 *
 * @file test_phase1_hashtable.c
 *
 * @brief Phase 1 tests: hashtable container.
 *
 * Note. In Phase 1, HashtablePut always replaces an existing key
 * regardless of CCL_FLAG_ALLOW_DUPLICATES (HashtablePut calls
 * HashPut, not HashPutAppend). Duplicate-key behaviour for
 * hashtable will be finalised in Phase 5 when Stab requires it;
 * the tests below cover the replace semantics only.
 */

#include <string.h>
#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "hashtable.h"
#include "test_util.h"

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

static void opts_init(PCCLOPTS p) {
    CclOptsInit(p);
    p->pHash    = h_fnv;
    p->pCompare = h_cmp;
}

static void test_ht_lifecycle(void) {
    HHASHTABLE h;
    CCLOPTS opts;

    /* pHash/pCompare required. */
    CclOptsInit(&opts);
    TEST_ASSERT(HashtableCreate(4, 4, 0, &opts, &h)
                == ERROR_INVALID_PARAMETER);
    TEST_ASSERT(HashtableCreate(4, 4, 0, NULL, &h)
                == ERROR_INVALID_PARAMETER);

    opts_init(&opts);
    TEST_ASSERT(HashtableCreate(0, 4, 0, &opts, &h)
                == ERROR_INVALID_PARAMETER);
    TEST_ASSERT(HashtableCreate(4, 0, 0, &opts, &h)
                == ERROR_INVALID_PARAMETER);

    TEST_ASSERT(HashtableCreate(4, 4, 0, &opts, &h) == NO_ERROR);
    TEST_ASSERT(HashtableDestroy(h) == NO_ERROR);
    TEST_ASSERT(HashtableDestroy(NULLHANDLE) == NO_ERROR);
}

static void test_ht_put_get(void) {
    HHASHTABLE h;
    CCLOPTS opts;
    ULONG k = 100, v = 200, out = 0, used = 0;
    BOOL f = FALSE;

    opts_init(&opts);
    TEST_ASSERT(HashtableCreate(4, 4, 0, &opts, &h) == NO_ERROR);

    TEST_ASSERT(HashtablePut(h, &k, &v, NULL) == NO_ERROR);
    TEST_ASSERT(HashtableGet(h, &k, &out, sizeof(out), &used) == NO_ERROR);
    TEST_ASSERT(out == 200);
    TEST_ASSERT(used == 4);

    TEST_ASSERT(HashtableContains(h, &k, &f) == NO_ERROR);
    TEST_ASSERT(f == TRUE);

    k = 999;
    TEST_ASSERT(HashtableGet(h, &k, &out, sizeof(out), &used)
                == ERROR_NO_MORE_ITEMS);

    TEST_ASSERT(HashtableDestroy(h) == NO_ERROR);
}

static void test_ht_replace(void) {
    HHASHTABLE h;
    CCLOPTS opts;
    ULONG k = 1, v1 = 10, v2 = 20, out = 0;
    BOOL fReplaced = FALSE;
    ULONG ulCount = 0;

    opts_init(&opts);
    TEST_ASSERT(HashtableCreate(4, 4, 0, &opts, &h) == NO_ERROR);

    HashtablePut(h, &k, &v1, &fReplaced);
    TEST_ASSERT(fReplaced == FALSE);

    HashtablePut(h, &k, &v2, &fReplaced);
    TEST_ASSERT(fReplaced == TRUE);

    HashtableGet(h, &k, &out, sizeof(out), NULL);
    TEST_ASSERT(out == 20);
    HashtableGetCount(h, &ulCount);
    TEST_ASSERT(ulCount == 1);

    TEST_ASSERT(HashtableDestroy(h) == NO_ERROR);
}

static void test_ht_remove_clear(void) {
    HHASHTABLE h;
    CCLOPTS opts;
    ULONG k1 = 1, k2 = 2, v = 0;
    BOOL f;
    ULONG ulCount = 0;

    opts_init(&opts);
    TEST_ASSERT(HashtableCreate(4, 4, 0, &opts, &h) == NO_ERROR);

    HashtablePut(h, &k1, &v, NULL);
    HashtablePut(h, &k2, &v, NULL);

    TEST_ASSERT(HashtableRemove(h, &k1, &f) == NO_ERROR);
    TEST_ASSERT(f == TRUE);
    TEST_ASSERT(HashtableRemove(h, &k1, &f) == NO_ERROR);
    TEST_ASSERT(f == FALSE);

    HashtableGetCount(h, &ulCount);
    TEST_ASSERT(ulCount == 1);

    TEST_ASSERT(HashtableClear(h) == NO_ERROR);
    HashtableGetCount(h, &ulCount);
    TEST_ASSERT(ulCount == 0);

    TEST_ASSERT(HashtableDestroy(h) == NO_ERROR);
}

static void test_ht_bucket_count(void) {
    HHASHTABLE h;
    CCLOPTS opts;
    ULONG ulB = 0, k = 1, v = 2;

    opts_init(&opts);
    TEST_ASSERT(HashtableCreate(4, 4, 0, &opts, &h) == NO_ERROR);

    TEST_ASSERT(HashtableSetBucketCount(h, 32) == NO_ERROR);
    TEST_ASSERT(HashtableGetBucketCount(h, &ulB) == NO_ERROR);
    TEST_ASSERT(ulB == 32);

    HashtablePut(h, &k, &v, NULL);
    TEST_ASSERT(HashtableSetBucketCount(h, 64) == ERROR_INVALID_PARAMETER);

    TEST_ASSERT(HashtableDestroy(h) == NO_ERROR);
}

static void test_ht_ith(void) {
    HHASHTABLE h;
    CCLOPTS opts;
    ULONG keys[3] = {100, 200, 300};
    ULONG vals[3] = {11, 22, 33};
    ULONG ulCount = 0;
    ULONG out = 0;
    BOOL seenK100 = FALSE, seenK200 = FALSE, seenK300 = FALSE;
    ULONG i;

    opts_init(&opts);
    TEST_ASSERT(HashtableCreate(4, 4, 0, &opts, &h) == NO_ERROR);

    for (i = 0; i < 3; i++)
        HashtablePut(h, &keys[i], &vals[i], NULL);

    HashtableGetCount(h, &ulCount);
    TEST_ASSERT(ulCount == 3);

    for (i = 0; i < 3; i++) {
        TEST_ASSERT(HashtableGetIthKey(h, i, &out, sizeof(out), NULL)
                    == NO_ERROR);
        if (out == keys[0]) seenK100 = TRUE;
        else if (out == keys[1]) seenK200 = TRUE;
        else if (out == keys[2]) seenK300 = TRUE;

        TEST_ASSERT(HashtableGetIthValue(h, i, &out, sizeof(out), NULL)
                    == NO_ERROR);
        if (out != vals[0] && out != vals[1] && out != vals[2])
            TEST_ASSERT(0);
    }
    TEST_ASSERT(seenK100 && seenK200 && seenK300);

    TEST_ASSERT(HashtableDestroy(h) == NO_ERROR);
}

static ULONG g_ulDestroyed = 0;
static void APIENTRY d(PVOID p) { (void)p; g_ulDestroyed++; }

static void test_ht_destroy_callbacks(void) {
    HHASHTABLE h;
    CCLOPTS opts;
    ULONG k = 1, v = 2;
    BOOL f;

    opts_init(&opts);
    opts.pDestroyKey   = d;
    opts.pDestroyValue = d;

    g_ulDestroyed = 0;
    TEST_ASSERT(HashtableCreate(4, 4, 0, &opts, &h) == NO_ERROR);

    HashtablePut(h, &k, &v, NULL);
    TEST_ASSERT(g_ulDestroyed == 0);

    HashtableRemove(h, &k, &f);
    TEST_ASSERT(f == TRUE);
    /* One call for key, one for value. */
    TEST_ASSERT(g_ulDestroyed == 2);

    HashtablePut(h, &k, &v, NULL);
    g_ulDestroyed = 0;
    TEST_ASSERT(HashtableDestroy(h) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 2);
}

int main(void) {
    TEST_RUN(test_ht_lifecycle);
    TEST_RUN(test_ht_put_get);
    TEST_RUN(test_ht_replace);
    TEST_RUN(test_ht_remove_clear);
    TEST_RUN(test_ht_bucket_count);
    TEST_RUN(test_ht_ith);
    TEST_RUN(test_ht_destroy_callbacks);
    return test_summary();
}
