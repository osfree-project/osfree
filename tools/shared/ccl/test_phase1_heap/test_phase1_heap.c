/*!
 *
 * @file test_phase1_heap.c
 *
 * @brief Phase 1 tests: heap container (min-heap over vector).
 */

#include <string.h>
#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "heap.h"
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

static void test_heap_lifecycle(void) {
    HHEAP h;
    CCLOPTS opts;
    APIRET rc;

    CclOptsInit(&opts);
    rc = HeapCreate(4, &opts, &h);
    TEST_ASSERT(rc == ERROR_INVALID_PARAMETER);  /* pCompare missing */

    opts.pCompare = cmp_int;
    rc = HeapCreate(4, &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(HeapDestroy(h) == NO_ERROR);
    TEST_ASSERT(HeapDestroy(NULLHANDLE) == NO_ERROR);
    TEST_ASSERT(HeapCreate(0, &opts, &h) == ERROR_INVALID_PARAMETER);
}

static void test_heap_push_pop_sorted(void) {
    HHEAP h;
    CCLOPTS opts;
    int vals[7] = {5, 2, 8, 1, 9, 3, 7};
    int sorted[7] = {1, 2, 3, 5, 7, 8, 9};
    int out = 0;
    ULONG ulUsed = 0, ulCount = 0;
    int i;

    CclOptsInit(&opts);
    opts.pCompare = cmp_int;
    TEST_ASSERT(HeapCreate(4, &opts, &h) == NO_ERROR);

    for (i = 0; i < 7; i++)
        TEST_ASSERT(HeapPush(h, &vals[i]) == NO_ERROR);

    TEST_ASSERT(HeapGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 7);

    for (i = 0; i < 7; i++) {
        TEST_ASSERT(HeapPop(h, &out, sizeof(out), &ulUsed) == NO_ERROR);
        TEST_ASSERT(out == sorted[i]);
        TEST_ASSERT(ulUsed == 4);
    }
    TEST_ASSERT(HeapPop(h, &out, sizeof(out), &ulUsed)
                == ERROR_NO_MORE_ITEMS);

    TEST_ASSERT(HeapDestroy(h) == NO_ERROR);
}

static void test_heap_top_and_pop_edge(void) {
    HHEAP h;
    CCLOPTS opts;
    int v = 42, out = 0;
    ULONG ulUsed = 0, ulCount = 0;
    PVOID p = NULL;
    char tiny[2];

    CclOptsInit(&opts);
    opts.pCompare = cmp_int;
    TEST_ASSERT(HeapCreate(4, &opts, &h) == NO_ERROR);

    TEST_ASSERT(HeapTop(h, &p) == ERROR_NO_MORE_ITEMS);

    HeapPush(h, &v);
    TEST_ASSERT(HeapTop(h, &p) == NO_ERROR);
    TEST_ASSERT(*(int *)p == 42);

    /* Size-query does not remove. */
    TEST_ASSERT(HeapPop(h, NULL, 0, &ulUsed) == NO_ERROR);
    TEST_ASSERT(ulUsed == 4);
    HeapGetCount(h, &ulCount);
    TEST_ASSERT(ulCount == 1);

    /* Buffer too small does not remove. */
    TEST_ASSERT(HeapPop(h, tiny, sizeof(tiny), &ulUsed)
                == ERROR_BUFFER_OVERFLOW);
    TEST_ASSERT(ulUsed == 4);
    HeapGetCount(h, &ulCount);
    TEST_ASSERT(ulCount == 1);

    TEST_ASSERT(HeapPop(h, &out, sizeof(out), &ulUsed) == NO_ERROR);
    TEST_ASSERT(out == 42);

    TEST_ASSERT(HeapDestroy(h) == NO_ERROR);
}

static void test_heap_destroy_callback(void) {
    HHEAP h;
    CCLOPTS opts;
    int v1 = 3, v2 = 1, v3 = 2;
    int out = 0;
    ULONG ulUsed = 0;

    CclOptsInit(&opts);
    opts.pCompare      = cmp_int;
    opts.pDestroyValue = d;

    g_ulDestroyed = 0;
    TEST_ASSERT(HeapCreate(4, &opts, &h) == NO_ERROR);

    HeapPush(h, &v1);
    HeapPush(h, &v2);
    HeapPush(h, &v3);
    TEST_ASSERT(g_ulDestroyed == 0);

    /* Pop should fire destroy on the popped root (min). */
    TEST_ASSERT(HeapPop(h, &out, sizeof(out), &ulUsed) == NO_ERROR);
    TEST_ASSERT(out == 1);
    TEST_ASSERT(g_ulDestroyed == 1);

    TEST_ASSERT(HeapClear(h) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 3);

    TEST_ASSERT(HeapDestroy(h) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 3);
}

int main(void) {
    TEST_RUN(test_heap_lifecycle);
    TEST_RUN(test_heap_push_pop_sorted);
    TEST_RUN(test_heap_top_and_pop_edge);
    TEST_RUN(test_heap_destroy_callback);
    return test_summary();
}
