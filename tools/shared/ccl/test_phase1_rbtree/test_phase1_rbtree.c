/*!
 *
 * @file test_phase1_rbtree.c
 *
 * @brief Phase 1 tests: rbtree container (red-black tree).
 */

#include <string.h>
#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "rbtree.h"
#include "test_util.h"

static int APIENTRY cmp_int(PCVOID pA, PCVOID pB, PVOID pCtx) {
    int a = *(const int *)pA;
    int b = *(const int *)pB;
    (void)pCtx;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static ULONG g_ulDestroyed = 0;
static void APIENTRY d(PVOID p) { (void)p; g_ulDestroyed++; }

static void test_rb_lifecycle(void) {
    HRBTREE h;
    CCLOPTS opts;
    APIRET rc;

    CclOptsInit(&opts);
    rc = RbtreeCreate(4, &opts, &h);
    TEST_ASSERT(rc == ERROR_INVALID_PARAMETER);  /* pCompare missing */

    opts.pCompare = cmp_int;
    rc = RbtreeCreate(4, &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(RbtreeDestroy(h) == NO_ERROR);
    TEST_ASSERT(RbtreeDestroy(NULLHANDLE) == NO_ERROR);

    /* pDestroyKey rejected by CclValidateOpts. */
    CclOptsInit(&opts);
    opts.pCompare     = cmp_int;
    opts.pDestroyKey  = d;
    TEST_ASSERT(RbtreeCreate(4, &opts, &h) == ERROR_INVALID_PARAMETER);
}

static void test_rb_insert_find(void) {
    HRBTREE h;
    CCLOPTS opts;
    int v[5] = {50, 30, 70, 20, 40};
    int k = 30, nk = 99, out = 0;
    BOOL f = FALSE;
    ULONG ulUsed = 0, ulCount = 0;
    int i;

    CclOptsInit(&opts);
    opts.pCompare = cmp_int;
    TEST_ASSERT(RbtreeCreate(4, &opts, &h) == NO_ERROR);

    for (i = 0; i < 5; i++)
        TEST_ASSERT(RbtreeInsert(h, &v[i], NULL) == NO_ERROR);

    TEST_ASSERT(RbtreeGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 5);

    TEST_ASSERT(RbtreeFind(h, &k, &out, sizeof(out), &ulUsed) == NO_ERROR);
    TEST_ASSERT(out == 30);
    TEST_ASSERT(RbtreeContains(h, &k, &f) == NO_ERROR);
    TEST_ASSERT(f == TRUE);
    TEST_ASSERT(RbtreeContains(h, &nk, &f) == NO_ERROR);
    TEST_ASSERT(f == FALSE);

    TEST_ASSERT(RbtreeDestroy(h) == NO_ERROR);
}

static void test_rb_unique_no_op(void) {
    HRBTREE h;
    CCLOPTS opts;
    int v1 = 10, v2 = 10, out = 0;
    BOOL fInserted = TRUE;
    ULONG ulCount = 0, ulUsed = 0;

    CclOptsInit(&opts);
    opts.pCompare = cmp_int;
    TEST_ASSERT(RbtreeCreate(4, &opts, &h) == NO_ERROR);

    TEST_ASSERT(RbtreeInsert(h, &v1, &fInserted) == NO_ERROR);
    TEST_ASSERT(fInserted == TRUE);

    TEST_ASSERT(RbtreeInsert(h, &v2, &fInserted) == NO_ERROR);
    TEST_ASSERT(fInserted == FALSE);

    RbtreeGetCount(h, &ulCount);
    TEST_ASSERT(ulCount == 1);
    RbtreeFind(h, &v1, &out, sizeof(out), &ulUsed);
    TEST_ASSERT(out == 10);

    TEST_ASSERT(RbtreeDestroy(h) == NO_ERROR);
}

static void test_rb_duplicates(void) {
    HRBTREE h;
    CCLOPTS opts;
    int v = 5;
    BOOL fInserted;
    ULONG ulCount = 0;
    BOOL fRemoved;

    CclOptsInit(&opts);
    opts.pCompare = cmp_int;
    opts.ulFlags  = CCL_FLAG_ALLOW_DUPLICATES;
    TEST_ASSERT(RbtreeCreate(4, &opts, &h) == NO_ERROR);

    TEST_ASSERT(RbtreeInsert(h, &v, &fInserted) == NO_ERROR);
    TEST_ASSERT(fInserted == TRUE);
    TEST_ASSERT(RbtreeInsert(h, &v, &fInserted) == NO_ERROR);
    TEST_ASSERT(fInserted == TRUE);
    TEST_ASSERT(RbtreeInsert(h, &v, &fInserted) == NO_ERROR);
    TEST_ASSERT(fInserted == TRUE);

    RbtreeGetCount(h, &ulCount);
    TEST_ASSERT(ulCount == 3);

    RbtreeRemove(h, &v, &fRemoved);
    TEST_ASSERT(fRemoved == TRUE);
    RbtreeGetCount(h, &ulCount);
    TEST_ASSERT(ulCount == 2);

    TEST_ASSERT(RbtreeDestroy(h) == NO_ERROR);
}

static void test_rb_min_max_ith(void) {
    HRBTREE h;
    CCLOPTS opts;
    int v[6] = {40, 10, 60, 20, 50, 30};
    int expected[6] = {10, 20, 30, 40, 50, 60};
    int out = 0;
    ULONG ulUsed = 0;
    int i;

    CclOptsInit(&opts);
    opts.pCompare = cmp_int;
    TEST_ASSERT(RbtreeCreate(4, &opts, &h) == NO_ERROR);

    for (i = 0; i < 6; i++) RbtreeInsert(h, &v[i], NULL);

    TEST_ASSERT(RbtreeGetMin(h, &out, sizeof(out), &ulUsed) == NO_ERROR);
    TEST_ASSERT(out == 10);

    TEST_ASSERT(RbtreeGetMax(h, &out, sizeof(out), &ulUsed) == NO_ERROR);
    TEST_ASSERT(out == 60);

    for (i = 0; i < 6; i++) {
        TEST_ASSERT(RbtreeGetIth(h, i, &out, sizeof(out), &ulUsed)
                    == NO_ERROR);
        TEST_ASSERT(out == expected[i]);
    }
    TEST_ASSERT(RbtreeGetIth(h, 6, &out, sizeof(out), &ulUsed)
                == ERROR_NO_MORE_ITEMS);

    TEST_ASSERT(RbtreeDestroy(h) == NO_ERROR);
}

static void test_rb_destroy_callback(void) {
    HRBTREE h;
    CCLOPTS opts;
    int v[4] = {4, 2, 6, 1};
    BOOL fRemoved;
    int r = 2;

    CclOptsInit(&opts);
    opts.pCompare      = cmp_int;
    opts.pDestroyValue = d;

    g_ulDestroyed = 0;
    TEST_ASSERT(RbtreeCreate(4, &opts, &h) == NO_ERROR);
    {
        int i;
        for (i = 0; i < 4; i++) RbtreeInsert(h, &v[i], NULL);
    }
    TEST_ASSERT(g_ulDestroyed == 0);

    RbtreeRemove(h, &r, &fRemoved);
    TEST_ASSERT(fRemoved == TRUE);
    TEST_ASSERT(g_ulDestroyed == 1);

    RbtreeClear(h);
    TEST_ASSERT(g_ulDestroyed == 4);

    RbtreeInsert(h, &v[0], NULL);
    g_ulDestroyed = 0;
    TEST_ASSERT(RbtreeDestroy(h) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 1);
}

int main(void) {
    TEST_RUN(test_rb_lifecycle);
    TEST_RUN(test_rb_insert_find);
    TEST_RUN(test_rb_unique_no_op);
    TEST_RUN(test_rb_duplicates);
    TEST_RUN(test_rb_min_max_ith);
    TEST_RUN(test_rb_destroy_callback);
    return test_summary();
}
