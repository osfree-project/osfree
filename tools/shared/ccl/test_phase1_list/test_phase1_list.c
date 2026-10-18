/*!
 *
 * @file test_phase1_list.c
 *
 * @brief Phase 1 tests: list container.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Covers:
 *
 *   - ListCreate / ListDestroy, including NULL handle
 *   - ListAdd + ListGetCount + ListGetAt
 *   - ListInsertAt at 0, middle and end
 *   - ListRemoveAt at 0, middle and last
 *   - ListClear
 *   - ListContains with and without pCompare
 *   - pDestroyValue callback invoked on remove, clear and destroy
 *   - node allocation goes through (*CclMalloc) (counting allocator)
 *
 * The list is a pointer container (elemt-model). Elements are
 * opaque PVOID values; the test uses addresses of local variables
 * and a few cast integer constants as element values. No element
 * is dereferenced.
 */

#include <string.h>

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "list.h"
#include "test_util.h"

/* ==================================================================
 * Test callbacks
 * ================================================================== */

/*!
 * @brief Destruction callback: count how many elements were
 *        released.
 *
 * The callback receives a pointer to the stored bytes of one
 * element (the address of the elemt field), as per the ccl
 * convention. It does not free anything (the test elements are
 * stack addresses or integer constants); it only increments a
 * counter so the test can verify that the callback was called the
 * expected number of times.
 *
 * @param[in] pElem  Pointer to the stored bytes. Not NULL.
 */
static ULONG g_ulDestroyed = 0;

static void APIENTRY test_destroy(PVOID pElem) {
    (void)pElem;
    g_ulDestroyed++;
}

/*!
 * @brief Comparison callback: pointer ordering.
 *
 * Both arguments point to the stored bytes of a PVOID (the elemt
 * field). The callback reads the stored pointer out of those
 * bytes and compares the two pointers by identity.
 *
 * @param[in] pA    Pointer to the stored bytes of the first PVOID.
 * @param[in] pB    Pointer to the stored bytes of the second PVOID.
 * @param[in] pCtx  Unused.
 *
 * @return Negative, zero or positive.
 */
static int APIENTRY test_compare(PCVOID pA, PCVOID pB, PVOID pCtx) {
    PVOID a;
    PVOID b;
    (void)pCtx;
    a = *(PVOID *)pA;
    b = *(PVOID *)pB;
    if (a == b) return 0;
    return (a < b) ? -1 : 1;
}

/* ==================================================================
 * Helpers
 * ================================================================== */

/*!
 * @brief Build a CCLOPTS block with the given callbacks.
 *
 * @param[out] pOpts      Receiver. Not NULL.
 * @param[in]  pDestroy   Optional pDestroyValue.
 * @param[in]  pCompare   Optional pCompare.
 */
static void opts_init(PCCLOPTS pOpts, CCL_DESTROY_FN pDestroy,
                      CCL_COMPARE_FN pCompare) {
    CclOptsInit(pOpts);
    pOpts->pDestroyValue = pDestroy;
    pOpts->pCompare      = pCompare;
}

/* ==================================================================
 * Basic lifecycle
 * ================================================================== */

/*!
 * @brief ListCreate / ListDestroy round-trip and error paths.
 */
static void test_list_lifecycle(void) {
    HLIST h;
    APIRET rc;

    rc = ListCreate((ULONG)sizeof(PVOID), NULL, &h);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(h != NULLHANDLE);
    TEST_ASSERT(ListDestroy(h) == NO_ERROR);

    /* NULL handle is a no-op. */
    TEST_ASSERT(ListDestroy(NULLHANDLE) == NO_ERROR);

    /* Bad arguments. */
    TEST_ASSERT(ListCreate(0, NULL, &h) == ERROR_INVALID_PARAMETER);
    TEST_ASSERT(ListCreate((ULONG)sizeof(PVOID), NULL, NULL)
                == ERROR_INVALID_PARAMETER);
}

/* ==================================================================
 * Adding and reading
 * ================================================================== */

/*!
 * @brief ListAdd + ListGetCount + ListGetAt.
 */
static void test_list_add_get(void) {
    HLIST h;
    APIRET rc;
    ULONG ulCount = 0;
    PVOID pOut = NULL;
    int a = 10, b = 20, c = 30;

    rc = ListCreate((ULONG)sizeof(PVOID), NULL, &h);
    TEST_ASSERT(rc == NO_ERROR);

    TEST_ASSERT(ListGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 0);

    TEST_ASSERT(ListAdd(h, &a) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &b) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &c) == NO_ERROR);

    TEST_ASSERT(ListGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 3);

    TEST_ASSERT(ListGetAt(h, 0, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &a);
    TEST_ASSERT(ListGetAt(h, 1, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &b);
    TEST_ASSERT(ListGetAt(h, 2, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &c);

    TEST_ASSERT(ListGetAt(h, 3, &pOut) == ERROR_NO_MORE_ITEMS);

    TEST_ASSERT(ListDestroy(h) == NO_ERROR);
}

/*!
 * @brief ListInsertAt at 0, middle, end.
 */
static void test_list_insert_at(void) {
    HLIST h;
    APIRET rc;
    ULONG ulCount = 0;
    PVOID pOut = NULL;
    int a = 1, b = 2, c = 3, d = 4, e = 5;

    rc = ListCreate((ULONG)sizeof(PVOID), NULL, &h);
    TEST_ASSERT(rc == NO_ERROR);

    /* InsertAt on an empty list. */
    TEST_ASSERT(ListInsertAt(h, 0, &a) == NO_ERROR);
    TEST_ASSERT(ListGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 1);
    TEST_ASSERT(ListGetAt(h, 0, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &a);

    /* InsertAt 0 on a non-empty list. */
    TEST_ASSERT(ListInsertAt(h, 0, &b) == NO_ERROR);
    TEST_ASSERT(ListGetAt(h, 0, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &b);
    TEST_ASSERT(ListGetAt(h, 1, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &a);

    /* InsertAt middle (index 1). */
    TEST_ASSERT(ListInsertAt(h, 1, &c) == NO_ERROR);
    TEST_ASSERT(ListGetAt(h, 0, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &b);
    TEST_ASSERT(ListGetAt(h, 1, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &c);
    TEST_ASSERT(ListGetAt(h, 2, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &a);

    /* InsertAt end. */
    TEST_ASSERT(ListInsertAt(h, 3, &d) == NO_ERROR);
    TEST_ASSERT(ListGetAt(h, 3, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &d);

    /* Index beyond end. */
    TEST_ASSERT(ListInsertAt(h, 5, &e) == ERROR_NO_MORE_ITEMS);

    TEST_ASSERT(ListGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 4);

    TEST_ASSERT(ListDestroy(h) == NO_ERROR);
}

/* ==================================================================
 * Removing
 * ================================================================== */

/*!
 * @brief ListRemoveAt at 0, middle and last.
 */
static void test_list_remove_at(void) {
    HLIST h;
    APIRET rc;
    ULONG ulCount = 0;
    PVOID pOut = NULL;
    int a = 1, b = 2, c = 3;

    rc = ListCreate((ULONG)sizeof(PVOID), NULL, &h);
    TEST_ASSERT(rc == NO_ERROR);

    TEST_ASSERT(ListAdd(h, &a) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &b) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &c) == NO_ERROR);

    /* Remove middle (b). */
    TEST_ASSERT(ListRemoveAt(h, 1) == NO_ERROR);
    TEST_ASSERT(ListGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 2);
    TEST_ASSERT(ListGetAt(h, 0, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &a);
    TEST_ASSERT(ListGetAt(h, 1, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &c);

    /* Remove head (a). */
    TEST_ASSERT(ListRemoveAt(h, 0) == NO_ERROR);
    TEST_ASSERT(ListGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 1);
    TEST_ASSERT(ListGetAt(h, 0, &pOut) == NO_ERROR);
    TEST_ASSERT(pOut == &c);

    /* Remove last (c). */
    TEST_ASSERT(ListRemoveAt(h, 0) == NO_ERROR);
    TEST_ASSERT(ListGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 0);

    /* Out of range. */
    TEST_ASSERT(ListRemoveAt(h, 0) == ERROR_NO_MORE_ITEMS);

    TEST_ASSERT(ListDestroy(h) == NO_ERROR);
}

/*!
 * @brief ListClear on a non-empty list.
 */
static void test_list_clear(void) {
    HLIST h;
    APIRET rc;
    ULONG ulCount = 0;
    int a = 1, b = 2;

    rc = ListCreate((ULONG)sizeof(PVOID), NULL, &h);
    TEST_ASSERT(rc == NO_ERROR);

    TEST_ASSERT(ListAdd(h, &a) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &b) == NO_ERROR);
    TEST_ASSERT(ListGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 2);

    TEST_ASSERT(ListClear(h) == NO_ERROR);
    TEST_ASSERT(ListGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 0);

    /* Clear on empty is a no-op. */
    TEST_ASSERT(ListClear(h) == NO_ERROR);

    /* Can still add after clear. */
    TEST_ASSERT(ListAdd(h, &a) == NO_ERROR);
    TEST_ASSERT(ListGetCount(h, &ulCount) == NO_ERROR);
    TEST_ASSERT(ulCount == 1);

    TEST_ASSERT(ListDestroy(h) == NO_ERROR);
}

/* ==================================================================
 * Contains
 * ================================================================== */

/*!
 * @brief ListContains with pointer identity (no pCompare).
 */
static void test_list_contains_identity(void) {
    HLIST h;
    APIRET rc;
    BOOL fFound = FALSE;
    int a = 1, b = 2, c = 3;

    rc = ListCreate((ULONG)sizeof(PVOID), NULL, &h);
    TEST_ASSERT(rc == NO_ERROR);

    TEST_ASSERT(ListAdd(h, &a) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &b) == NO_ERROR);

    TEST_ASSERT(ListContains(h, &a, &fFound) == NO_ERROR);
    TEST_ASSERT(fFound == TRUE);
    TEST_ASSERT(ListContains(h, &b, &fFound) == NO_ERROR);
    TEST_ASSERT(fFound == TRUE);
    TEST_ASSERT(ListContains(h, &c, &fFound) == NO_ERROR);
    TEST_ASSERT(fFound == FALSE);

    TEST_ASSERT(ListDestroy(h) == NO_ERROR);
}

/*!
 * @brief ListContains with pCompare (equality on stored bytes).
 */
static void test_list_contains_compare(void) {
    HLIST h;
    APIRET rc;
    BOOL fFound = FALSE;
    CCLOPTS opts;
    int a = 1, b = 2, c = 3;

    opts_init(&opts, NULL, test_compare);

    rc = ListCreate((ULONG)sizeof(PVOID), &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);

    TEST_ASSERT(ListAdd(h, &a) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &b) == NO_ERROR);

    TEST_ASSERT(ListContains(h, &a, &fFound) == NO_ERROR);
    TEST_ASSERT(fFound == TRUE);
    TEST_ASSERT(ListContains(h, &c, &fFound) == NO_ERROR);
    TEST_ASSERT(fFound == FALSE);

    TEST_ASSERT(ListDestroy(h) == NO_ERROR);
}

/* ==================================================================
 * pDestroyValue callback
 * ================================================================== */

/*!
 * @brief pDestroyValue is called on remove, clear and destroy.
 */
static void test_list_destroy_callback(void) {
    HLIST h;
    APIRET rc;
    CCLOPTS opts;
    int a = 1, b = 2, c = 3;

    opts_init(&opts, test_destroy, NULL);
    g_ulDestroyed = 0;

    rc = ListCreate((ULONG)sizeof(PVOID), &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);

    TEST_ASSERT(ListAdd(h, &a) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &b) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &c) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 0);

    /* Remove one element: one destroy. */
    TEST_ASSERT(ListRemoveAt(h, 1) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 1);

    /* Clear: two remaining elements -> two more destroys. */
    TEST_ASSERT(ListClear(h) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 3);

    /* Destroy an empty list: no additional destroys. */
    TEST_ASSERT(ListDestroy(h) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 3);

    /* Now add two, and destroy without clear: two destroys. */
    g_ulDestroyed = 0;
    rc = ListCreate((ULONG)sizeof(PVOID), &opts, &h);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &a) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &b) == NO_ERROR);
    TEST_ASSERT(ListDestroy(h) == NO_ERROR);
    TEST_ASSERT(g_ulDestroyed == 2);
}

/* ==================================================================
 * Counting allocator
 * ================================================================== */

static ULONG g_ulLive = 0;

static PVOID APIENTRY test_malloc(size_t cbSize) {
    PVOID p = malloc(cbSize);
    if (p) g_ulLive++;
    return p;
}

static PVOID APIENTRY test_calloc(size_t cbNum, size_t cbSize) {
    PVOID p = calloc(cbNum, cbSize);
    if (p) g_ulLive++;
    return p;
}

static PVOID APIENTRY test_realloc(PVOID pPtr, size_t cbSize) {
    return realloc(pPtr, cbSize);
}

static void APIENTRY test_free(PVOID pPtr) {
    if (pPtr) g_ulLive--;
    free(pPtr);
}

static CCL_MALLOC_FN  g_saved_malloc  = NULL;
static CCL_CALLOC_FN  g_saved_calloc  = NULL;
static CCL_REALLOC_FN g_saved_realloc = NULL;
static CCL_FREE_FN    g_saved_free    = NULL;

static void counting_install(void) {
    g_saved_malloc  = CclMalloc;
    g_saved_calloc  = CclCalloc;
    g_saved_realloc = CclRealloc;
    g_saved_free    = CclFree;

    g_ulLive = 0;
    CclMalloc  = test_malloc;
    CclCalloc  = test_calloc;
    CclRealloc = test_realloc;
    CclFree    = test_free;
}

static void counting_uninstall(void) {
    CclMalloc  = g_saved_malloc;
    CclCalloc  = g_saved_calloc;
    CclRealloc = g_saved_realloc;
    CclFree    = g_saved_free;
}

/*!
 * @brief Nodes go through (*CclMalloc); the counter returns to 0
 *        after the list is destroyed.
 */
static void test_list_allocator(void) {
    HLIST h;
    APIRET rc;
    int a = 1, b = 2, c = 3;

    counting_install();

    rc = ListCreate((ULONG)sizeof(PVOID), NULL, &h);
    TEST_ASSERT(rc == NO_ERROR);

    TEST_ASSERT(ListAdd(h, &a) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &b) == NO_ERROR);
    TEST_ASSERT(ListAdd(h, &c) == NO_ERROR);

    /* Three nodes (control block is allocated before counting_install
       is called? No: control block is created *inside* the counting
       window, so it also counts). */
    TEST_ASSERT(g_ulLive >= 3);

    TEST_ASSERT(ListDestroy(h) == NO_ERROR);
    TEST_ASSERT(g_ulLive == 0);

    counting_uninstall();
}

/* ==================================================================
 * main
 * ================================================================== */

int main(void) {
    TEST_RUN(test_list_lifecycle);
    TEST_RUN(test_list_add_get);
    TEST_RUN(test_list_insert_at);
    TEST_RUN(test_list_remove_at);
    TEST_RUN(test_list_clear);
    TEST_RUN(test_list_contains_identity);
    TEST_RUN(test_list_contains_compare);
    TEST_RUN(test_list_destroy_callback);
    TEST_RUN(test_list_allocator);

    return test_summary();
}
