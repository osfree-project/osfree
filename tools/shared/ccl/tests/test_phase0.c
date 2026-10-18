/*!
 *
 * @file test_phase0.c
 *
 * @brief Phase 0 tests for the ccl library.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Covers:
 *
 *   - dlist regression after refactoring onto the shared
 *     LinkedNode primitives (lifecycle, insert/get, extract/
 *     delete/replace, transfer/append)
 *   - all_dlist regression after refactoring (stack, list)
 *   - counting allocator: dlist nodes go through (*CclMalloc)
 *     via LinkedNodeAlloc; the counter proves the hook is used
 *   - ccl_base public API: CclOptsInit defaults, CclValidateOpts
 *     strictness (NULL, disallowed fields, missing required
 *     fields, unknown/disallowed flags, permille threshold)
 *
 * The LinkedNode primitives themselves are not tested directly.
 * They are internal helpers used only by dlist.c, all_dlist.c and
 * (Phase 1) list.c; their behaviour is fully covered by the
 * container tests above.
 *
 * Build with the Watcom compiler; _System in dlist_private.h is a
 * Watcom calling-convention keyword and needs no macro definition.
 */

#include <stdlib.h>
#include <string.h>

#include "ccl.h"
#include "dlist_private.h"
#include "all_dlist_private.h"
#include "test_util.h"

/* ==================================================================
 * dlist regression
 * ================================================================== */

/*!
 * @brief CreateList / DestroyList round-trip.
 */
static void test_dlist_lifecycle(void) {
    DLIST h;
    CARDINAL32 err;

    h = CreateList();
    TEST_ASSERT(h != NULL);

    DestroyList(&h, TRUE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(h == NULL);
}

/*!
 * @brief InsertItem at end and at start, then GetItem on the
 *        current item.
 *
 * The last insertion is InsertAtStart(30) with MakeCurrent=TRUE, so
 * the current item after the three inserts is 30. GetItem returns
 * 30. GetListSize returns 3. CheckListIntegrity verifies that all
 * links are consistent.
 */
static void test_dlist_insert_and_get(void) {
    DLIST h;
    CARDINAL32 err;
    CARDINAL32 n;
    CARDINAL32 got;
    ADDRESS h1, h2, h3;

    h = CreateList();
    TEST_ASSERT(h != NULL);

    n = 10;
    h1 = InsertItem(h, sizeof(n), (ADDRESS)&n, 1,
                    NULL, AppendToList, TRUE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(h1 != NULL);

    n = 20;
    h2 = InsertItem(h, sizeof(n), (ADDRESS)&n, 1,
                    NULL, AppendToList, TRUE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);

    n = 30;
    h3 = InsertItem(h, sizeof(n), (ADDRESS)&n, 1,
                    NULL, InsertAtStart, TRUE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);

    got = 0;
    GetItem(h, sizeof(got), (ADDRESS)&got, 1, NULL, FALSE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(got == 30);

    TEST_ASSERT(GetListSize(h, &err) == 3);
    TEST_ASSERT(ListEmpty(h, &err) == FALSE);
    TEST_ASSERT(CheckListIntegrity(h) == TRUE);

    DestroyList(&h, TRUE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
}

/*!
 * @brief ExtractItem, ReplaceItem, DeleteItem, DeleteAllItems on
 *        a small list.
 *
 * The three inserts above use MakeCurrent=TRUE, so the current item
 * after them is the last one inserted. We call GoToStartOfList to
 * make the first item current before ExtractItem, otherwise the
 * extracted value would be the last one, not the first.
 */
static void test_dlist_extract_delete_replace(void) {
    DLIST h;
    CARDINAL32 err;
    CARDINAL32 n, got;

    h = CreateList();
    TEST_ASSERT(h != NULL);

    n = 1; InsertItem(h, sizeof(n), (ADDRESS)&n, 0, NULL, AppendToList, TRUE, &err);
    n = 2; InsertItem(h, sizeof(n), (ADDRESS)&n, 0, NULL, AppendToList, TRUE, &err);
    n = 3; InsertItem(h, sizeof(n), (ADDRESS)&n, 0, NULL, AppendToList, TRUE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(GetListSize(h, &err) == 3);

    /* Make the first item current, then extract it. Without this the
       current item is the last one inserted. */
    GoToStartOfList(h, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);

    /* Extract the first item into a caller buffer. */
    got = 0;
    ExtractItem(h, sizeof(got), (ADDRESS)&got, 0, NULL, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(got == 1);
    TEST_ASSERT(GetListSize(h, &err) == 2);

    /* Replace the current item's data with 99. */
    got = 99;
    ReplaceItem(h, sizeof(got), (ADDRESS)&got, 0, NULL, FALSE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);

    /* Delete the current item. */
    DeleteItem(h, TRUE, NULL, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(GetListSize(h, &err) == 1);

    DeleteAllItems(h, TRUE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(GetListSize(h, &err) == 0);

    DestroyList(&h, TRUE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
}

/*!
 * @brief TransferItem from one list to another, then AppendList.
 */
static void test_dlist_transfer_and_append(void) {
    DLIST src, tgt;
    CARDINAL32 err;
    CARDINAL32 n;
    ADDRESS h1;

    src = CreateList();
    tgt = CreateList();
    TEST_ASSERT(src && tgt);

    n = 1; h1 = InsertItem(src, sizeof(n), (ADDRESS)&n, 0, NULL,
                           AppendToList, TRUE, &err);
    TEST_ASSERT(h1 != NULL);
    n = 2; InsertItem(src, sizeof(n), (ADDRESS)&n, 0, NULL,
                      AppendToList, FALSE, &err);
    n = 3; InsertItem(src, sizeof(n), (ADDRESS)&n, 0, NULL,
                      AppendToList, FALSE, &err);
    TEST_ASSERT(GetListSize(src, &err) == 3);

    TransferItem(src, h1, tgt, NULL, AppendToList, FALSE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(GetListSize(src, &err) == 2);
    TEST_ASSERT(GetListSize(tgt, &err) == 1);

    AppendList(tgt, src, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(GetListSize(tgt, &err) == 3);
    TEST_ASSERT(GetListSize(src, &err) == 0);

    DestroyList(&src, TRUE, &err);
    DestroyList(&tgt, TRUE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
}

/* ==================================================================
 * all_dlist regression
 * ================================================================== */

/*!
 * @brief Stack push / pop / top / remove.
 *
 * After push(a), push(b), push(c), the top is c. Pop returns c,
 * top becomes b.
 *
 * stack_remove takes a node pointer, not a payload pointer. To
 * remove the node whose payload is b (which is the current top), we
 * pass *pph — the node pointer at the top of the stack. Inside
 * stack_remove, *pph is moved back to the previous node (a), then
 * the node is unlinked and freed. After that, top is a.
 *
 * Pop returns a. Pop on empty returns all_RC_DLIST_NULL.
 */
static void test_all_dlist_stack(void) {
    phStack pph;
    void *p;
    int a = 10, b = 20, c = 30;

    pph = stack_init();
    TEST_ASSERT(pph != NULL);
    TEST_ASSERT(*pph == NULL);

    TEST_ASSERT(stack_push(pph, &a) == NO_ERROR);
    TEST_ASSERT(stack_push(pph, &b) == NO_ERROR);
    TEST_ASSERT(stack_push(pph, &c) == NO_ERROR);

    TEST_ASSERT(stack_top(pph) == &c);

    TEST_ASSERT(stack_pop(pph, &p) == NO_ERROR);
    TEST_ASSERT(p == &c);
    TEST_ASSERT(stack_top(pph) == &b);

    /* Remove the top node (payload b); the stack pointer must fall
       back to a. stack_remove takes a node pointer, not a payload
       pointer; *pph is the current top node. */
    TEST_ASSERT(stack_remove(pph, (void *)*pph) == NO_ERROR);
    TEST_ASSERT(stack_top(pph) == &a);

    TEST_ASSERT(stack_pop(pph, &p) == NO_ERROR);
    TEST_ASSERT(p == &a);
    TEST_ASSERT(stack_pop(pph, &p) == all_RC_DLIST_NULL);

    free(pph);
}

/*!
 * @brief List add / insert / get / next / prev / set_start /
 *        set_end / remove / free.
 */
static void test_all_dlist_list(void) {
    phList ppl;
    int a = 1, b = 2, c = 3;

    ppl = list_init();
    TEST_ASSERT(ppl != NULL);
    TEST_ASSERT(list_isempty(ppl) == TRUE);

    TEST_ASSERT(list_add(ppl, &a) == NO_ERROR);
    TEST_ASSERT(list_add(ppl, &b) == NO_ERROR);
    TEST_ASSERT(list_add(ppl, &c) == NO_ERROR);
    TEST_ASSERT(list_isempty(ppl) == FALSE);
    TEST_ASSERT(list_get(ppl) == &c);

    TEST_ASSERT(list_set_start(ppl) == NO_ERROR);
    TEST_ASSERT(list_get(ppl) == &a);

    TEST_ASSERT(list_next(ppl) == NO_ERROR);
    TEST_ASSERT(list_get(ppl) == &b);

    TEST_ASSERT(list_prev(ppl) == NO_ERROR);
    TEST_ASSERT(list_get(ppl) == &a);

    TEST_ASSERT(list_set_end(ppl) == NO_ERROR);
    TEST_ASSERT(list_get(ppl) == &c);

    TEST_ASSERT(list_remove(ppl) == NO_ERROR);
    TEST_ASSERT(list_get(ppl) == &b);

    TEST_ASSERT(list_remove(ppl) == NO_ERROR);
    TEST_ASSERT(list_get(ppl) == &a);

    TEST_ASSERT(list_remove(ppl) == NO_ERROR);
    TEST_ASSERT(list_isempty(ppl) == TRUE);

    TEST_ASSERT(list_add(ppl, &a) == NO_ERROR);
    TEST_ASSERT(list_insert(ppl, &b) == NO_ERROR);
    TEST_ASSERT(list_free(ppl) == NO_ERROR);
    TEST_ASSERT(list_isempty(ppl) == TRUE);

    free(ppl);
}

/* ==================================================================
 * Counting allocator (SOM-style direct assignment)
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
    /* realloc keeps the block count unchanged on success or
       failure; the counter is not touched here. */
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
 * @brief dlist nodes go through (*CclMalloc) via LinkedNodeAlloc.
 *
 * Payloads of InsertItem go through malloc (legacy contract), so
 * they do not affect the counter. Only the three link nodes are
 * visible.
 */
static void test_allocator_dlist(void) {
    DLIST h;
    CARDINAL32 err;
    CARDINAL32 n;

    counting_install();

    h = CreateList();
    TEST_ASSERT(h != NULL);

    n = 1;
    InsertItem(h, sizeof(n), (ADDRESS)&n, 0, NULL, AppendToList, TRUE, &err);
    InsertItem(h, sizeof(n), (ADDRESS)&n, 0, NULL, AppendToList, FALSE, &err);
    InsertItem(h, sizeof(n), (ADDRESS)&n, 0, NULL, AppendToList, FALSE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(g_ulLive == 3);

    DestroyList(&h, TRUE, &err);
    TEST_ASSERT(err == DLIST_SUCCESS);
    TEST_ASSERT(g_ulLive == 0);

    counting_uninstall();
}

/* ==================================================================
 * ccl_base public API
 * ================================================================== */

/*!
 * @brief CclOptsInit zero-fills the block and NULL is a no-op.
 */
static void test_ccl_opts_init_defaults(void) {
    CCLOPTS opts;

    /* Fill with garbage to prove CclOptsInit zeroes. */
    memset(&opts, 0xFF, sizeof(opts));

    CclOptsInit(&opts);

    TEST_ASSERT(opts.ulRehashThreshold == 0);
    TEST_ASSERT(opts.pCompare == NULL);
    TEST_ASSERT(opts.pHash == NULL);
    TEST_ASSERT(opts.pKeyFn == NULL);
    TEST_ASSERT(opts.pDestroyKey == NULL);
    TEST_ASSERT(opts.pDestroyValue == NULL);
    TEST_ASSERT(opts.ulFlags == 0);

    /* NULL is a no-op. */
    CclOptsInit(NULL);
}

/*!
 * @brief CclValidateOpts on a NULL block.
 *
 * With no requirements, NULL is accepted (equivalent to a
 * default-initialized block). With any requirement, NULL fails.
 */
static void test_ccl_validate_opts_null(void) {
    TEST_ASSERT(CclValidateOpts(NULL, 0, 0) == NO_ERROR);

    TEST_ASSERT(CclValidateOpts(NULL, 0, CCL_OPT_COMPARE)
                == ERROR_INVALID_PARAMETER);
    TEST_ASSERT(CclValidateOpts(NULL, 0, CCL_OPT_HASH)
                == ERROR_INVALID_PARAMETER);
}

/*!
 * @brief CclValidateOpts rejects fields not in ulAllowed.
 *
 * vector: DESTROY_VALUE allowed, DESTROY_KEY not allowed.
 * set:    DESTROY_KEY not allowed even when DESTROY_VALUE is.
 */
static void test_ccl_validate_opts_field_not_allowed(void) {
    CCLOPTS opts;
    int dummy = 0;

    CclOptsInit(&opts);

    opts.pDestroyKey = (CCL_DESTROY_FN)&dummy;
    TEST_ASSERT(CclValidateOpts(&opts, CCL_OPT_DESTROY_VALUE, 0)
                == ERROR_INVALID_PARAMETER);

    opts.pDestroyKey = NULL;
    opts.pDestroyValue = (CCL_DESTROY_FN)&dummy;
    TEST_ASSERT(CclValidateOpts(&opts, CCL_OPT_DESTROY_VALUE, 0)
                == NO_ERROR);

    /* set: no DESTROY_KEY */
    opts.pDestroyValue = NULL;
    opts.pDestroyKey = (CCL_DESTROY_FN)&dummy;
    TEST_ASSERT(CclValidateOpts(&opts, CCL_OPT_DESTROY_VALUE, 0)
                == ERROR_INVALID_PARAMETER);
}

/*!
 * @brief CclValidateOpts rejects missing required fields.
 *
 * hashtable requires HASH | COMPARE. With an empty block both
 * missing; with only pHash set, pCompare still missing; with both
 * set, passes.
 */
static void test_ccl_validate_opts_required_missing(void) {
    CCLOPTS opts;
    int dummy = 0;

    CclOptsInit(&opts);

    TEST_ASSERT(CclValidateOpts(&opts,
                                CCL_OPT_HASH | CCL_OPT_COMPARE,
                                CCL_OPT_HASH | CCL_OPT_COMPARE)
                == ERROR_INVALID_PARAMETER);

    opts.pHash = (CCL_HASH_FN)&dummy;
    TEST_ASSERT(CclValidateOpts(&opts,
                                CCL_OPT_HASH | CCL_OPT_COMPARE,
                                CCL_OPT_HASH | CCL_OPT_COMPARE)
                == ERROR_INVALID_PARAMETER);

    opts.pCompare = (CCL_COMPARE_FN)&dummy;
    TEST_ASSERT(CclValidateOpts(&opts,
                                CCL_OPT_HASH | CCL_OPT_COMPARE,
                                CCL_OPT_HASH | CCL_OPT_COMPARE)
                == NO_ERROR);
}

/*!
 * @brief CclValidateOpts flag and threshold rules.
 *
 *  - unknown flag bit -> ERROR_INVALID_PARAMETER
 *  - known flag, allowed -> NO_ERROR
 *  - known flag, not allowed -> ERROR_INVALID_PARAMETER
 *  - threshold above 1000 -> ERROR_INVALID_PARAMETER
 *  - threshold non-zero, CCL_OPT_REHASH not allowed -> ERROR
 *  - threshold non-zero, CCL_OPT_REHASH allowed -> NO_ERROR
 */
static void test_ccl_validate_opts_flags(void) {
    CCLOPTS opts;

    CclOptsInit(&opts);

    /* Unknown flag bit: rejected. */
    opts.ulFlags = 0x80000000UL;
    TEST_ASSERT(CclValidateOpts(&opts, 0, 0) == ERROR_INVALID_PARAMETER);

    /* Known flag, allowed: OK. */
    opts.ulFlags = CCL_FLAG_DEFAULT_MANAGED;
    TEST_ASSERT(CclValidateOpts(&opts, CCL_OPT_FLAG_MANAGED, 0)
                == NO_ERROR);

    /* Known flag, not allowed: rejected. */
    opts.ulFlags = CCL_FLAG_ALLOW_DUPLICATES;
    TEST_ASSERT(CclValidateOpts(&opts, CCL_OPT_FLAG_MANAGED, 0)
                == ERROR_INVALID_PARAMETER);

    /* Threshold above 1000: rejected. */
    opts.ulFlags = 0;
    opts.ulRehashThreshold = 1001;
    TEST_ASSERT(CclValidateOpts(&opts, CCL_OPT_REHASH, 0)
                == ERROR_INVALID_PARAMETER);

    /* Threshold present, not allowed: rejected. */
    opts.ulRehashThreshold = 750;
    TEST_ASSERT(CclValidateOpts(&opts, 0, 0) == ERROR_INVALID_PARAMETER);

    /* Threshold present, allowed: OK. */
    TEST_ASSERT(CclValidateOpts(&opts, CCL_OPT_REHASH, 0) == NO_ERROR);
}

/* ==================================================================
 * main
 * ================================================================== */

int main(void) {
    TEST_RUN(test_dlist_lifecycle);
    TEST_RUN(test_dlist_insert_and_get);
    TEST_RUN(test_dlist_extract_delete_replace);
    TEST_RUN(test_dlist_transfer_and_append);

    TEST_RUN(test_all_dlist_stack);
    TEST_RUN(test_all_dlist_list);

    TEST_RUN(test_allocator_dlist);

    TEST_RUN(test_ccl_opts_init_defaults);
    TEST_RUN(test_ccl_validate_opts_null);
    TEST_RUN(test_ccl_validate_opts_field_not_allowed);
    TEST_RUN(test_ccl_validate_opts_required_missing);
    TEST_RUN(test_ccl_validate_opts_flags);

    return test_summary();
}
