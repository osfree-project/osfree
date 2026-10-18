/*!
 *
 * @file test_phase1_stack.c
 *
 * @brief Phase 1 tests: stack container (LIFO over vector).
 */

#include <string.h>
#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "stack.h"
#include "test_util.h"

static ULONG g_ulDestroyed = 0;
static void APIENTRY d(PVOID p) { (void)p; g_ulDestroyed++; }

static void test_stack_lifecycle(void) {
    HSTACK h;
    APIRET rc;

    rc = StackCreate(4, 0, NULL, &h);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(StackDestroy(h) == NO_ERROR);
    TEST_ASSERT(StackDestroy(NULLHANDLE) == NO_ERROR);
    TEST_ASSERT(StackCreate(0, 0, NULL, &h) == ERROR_INVALID_PARAMETER);
    TEST_ASSERT(StackCreate(4, 0, NULL, NULL) == ERROR_INVALID_PARAMETER);
}

static void test_stack_lifo(void) {
    HSTACK h;
    CCLOPTS opts;
    int a = 10, b = 20, c = 30;
    int out = 0;
    ULONG ulUsed = 0, ulCount = 0;
    PVOID p = NULL;

    CclOptsInit(&opts);
    TEST_ASSERT(StackCreate(4, 0, &opts, &h) == NO_ERROR);

    TEST_ASSERT(StackPush(h, &a) == NO_ERROR);
    TEST_ASSERT(StackPush(h, &b) == NO_ERROR);
    TEST_ASSERT(StackPush(h, &c) == NO_ERROR);

    TEST_ASSERT(StackGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 3);

    TEST_ASSERT(StackTop(h, &p) == NO_ERROR);
    TEST_ASSERT(p != NULL);
    TEST_ASSERT(*(int *)p == 30);

    TEST_ASSERT(StackPop(h, &out, sizeof(out), &ulUsed) == NO_ERROR);
    TEST_ASSERT(out == 30);
    TEST_ASSERT(ulUsed == 4);

    TEST_ASSERT(StackPop(h, &out, sizeof(out), &ulUsed) == NO_ERROR);
    TEST_ASSERT(out == 20);

    TEST_ASSERT(StackPop(h, &out, sizeof(out), &ulUsed) == NO_ERROR);
    TEST_ASSERT(out == 10);

    TEST_ASSERT(StackPop(h, &out, sizeof(out), &ulUsed)
                == ERROR_NO_MORE_ITEMS);

    TEST_ASSERT(StackDestroy(h) == NO_ERROR);
}

static void test_stack_pop_sizequery(void) {
    HSTACK h;
    int a = 7;
    ULONG ulUsed = 0, ulCount = 0;

    TEST_ASSERT(StackCreate(4, 0, NULL, &h) == NO_ERROR);
    TEST_ASSERT(StackPush(h, &a) == NO_ERROR);

    /* Size-query does not remove. */
    TEST_ASSERT(StackPop(h, NULL, 0, &ulUsed) == NO_ERROR);
    TEST_ASSERT(ulUsed == 4);
    TEST_ASSERT(StackGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 1);

    /* Buffer too small does not remove. */
    {
        char tiny[2];
        TEST_ASSERT(StackPop(h, tiny, sizeof(tiny), &ulUsed)
                    == ERROR_BUFFER_OVERFLOW);
        TEST_ASSERT(ulUsed == 4);
        TEST_ASSERT(StackGetCount(h, &ulCount) == NO_ERROR);
        TEST_ASSERT(ulCount == 1);
    }

    TEST_ASSERT(StackDestroy(h) == NO_ERROR);
}

static void test_stack_clear_and_destroy_callback(void) {
    HSTACK h;
    CCLOPTS opts;
    int a = 1, b = 2;

    CclOptsInit(&opts);
    opts.pDestroyValue = d;

    g_ulDestroyed = 0;
    TEST_ASSERT(StackCreate(4, 0, &opts, &h) == NO_ERROR);
    StackPush(h, &a);
    StackPush(h, &b);
    TEST_ASSERT(g_ulDestroyed == 0);

    TEST_ASSERT(StackClear(h) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 2);

    StackPush(h, &a);
    StackPush(h, &b);
    g_ulDestroyed = 0;
    TEST_ASSERT(StackDestroy(h) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 2);
}

int main(void) {
    TEST_RUN(test_stack_lifecycle);
    TEST_RUN(test_stack_lifo);
    TEST_RUN(test_stack_pop_sizequery);
    TEST_RUN(test_stack_clear_and_destroy_callback);
    return test_summary();
}
