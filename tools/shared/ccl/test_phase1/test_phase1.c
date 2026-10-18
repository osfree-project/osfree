/*!
 *
 * @file test_phase1.c
 *
 * @brief Phase 1 tests: hash_private, alignment of HASHNODE,
 *        ccl_node_alloc.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Covers:
 *
 *   - CCL_ALIGN_UP macro
 *   - offsetof(HASHNODE, u) == CCL_MAX_ALIGN
 *   - ccl_node_alloc returns an 8-aligned pointer, and
 *     (pNode + ulFixedPart) is 8-aligned
 *   - hash table basic operations (create, put, get, remove,
 *     destroy) with different key sizes
 *   - hash table replace semantics (pfReplaced, new value visible)
 *   - hash table grow (load factor exceeded, count preserved)
 *   - HASHNODE key and value alignment for ulKeySize = 1, 4, 5,
 *     7, 8, 12, 16
 *   - HashSetBucketCount permitted only when count == 0
 *
 * The test uses the private headers on purpose: the alignment
 * invariants are the whole point of hash_private and cannot be
 * observed through the public container API.
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "os2types.h"
#include "os2err.h"
#include "ccl_base.h"
#include "ccl_alloc.h"
#include "ccl_base_private.h"
#include "node_private.h"
#include "hash_private.h"
#include "test_util.h"

/* ==================================================================
 * Test callbacks
 *
 * hash_private requires pHash and pCompare but does not pass a
 * context to them, so the callbacks must know the key size from
 * somewhere. For tests we use a file-scope variable and set it
 * before each HashCreate.
 * ================================================================== */

static ULONG g_ulTestKeySize = 4;

static ULONG APIENTRY test_hash(PCVOID pKey, PVOID pCtx) {
    const UCHAR *p = (const UCHAR *)pKey;
    ULONG ulHash = 2166136261UL;
    ULONG i;

    (void)pCtx;
    for (i = 0; i < g_ulTestKeySize; i++) {
        ulHash ^= (ULONG)p[i];
        ulHash *= 16777619UL;
    }
    return ulHash;
}

static int APIENTRY test_compare(PCVOID pA, PCVOID pB, PVOID pCtx) {
    (void)pCtx;
    return memcmp(pA, pB, g_ulTestKeySize);
}

/* ==================================================================
 * Alignment checks
 * ================================================================== */

/*!
 * @brief CCL_ALIGN_UP rounds up to the next multiple of 8.
 */
static void test_align_up(void) {
    TEST_ASSERT(CCL_ALIGN_UP(0)  == 0);
    TEST_ASSERT(CCL_ALIGN_UP(1)  == 8);
    TEST_ASSERT(CCL_ALIGN_UP(7)  == 8);
    TEST_ASSERT(CCL_ALIGN_UP(8)  == 8);
    TEST_ASSERT(CCL_ALIGN_UP(9)  == 16);
    TEST_ASSERT(CCL_ALIGN_UP(15) == 16);
    TEST_ASSERT(CCL_ALIGN_UP(16) == 16);
    TEST_ASSERT(CCL_ALIGN_UP(17) == 24);
}

/*!
 * @brief offsetof(HASHNODE, u) is 8 because the union contains a
 *        double.
 */
static void test_hashnode_layout(void) {
    TEST_ASSERT(offsetof(HASHNODE, u) == 8);
    TEST_ASSERT(offsetof(HASHNODE, u) % CCL_MAX_ALIGN == 0);
}

/*!
 * @brief ccl_node_alloc returns an 8-aligned pointer, and
 *        (pNode + ulFixedPart) is also 8-aligned.
 */
static void test_node_alloc_alignment(void) {
    void *p;
    void *pElem;

    p = ccl_node_alloc(8, 8, 8);
    TEST_ASSERT(p != NULL);
    TEST_ASSERT(((ULONG)p) % 8 == 0);
    pElem = (void *)((char *)p + 8);
    TEST_ASSERT(((ULONG)pElem) % 8 == 0);
    ccl_node_free(p);

    p = ccl_node_alloc(16, 16, 16);
    TEST_ASSERT(p != NULL);
    TEST_ASSERT(((ULONG)p) % 16 == 0);
    pElem = (void *)((char *)p + 16);
    TEST_ASSERT(((ULONG)pElem) % 16 == 0);
    ccl_node_free(p);

    /* NULL-safe free. */
    ccl_node_free(NULL);
}

/*!
 * @brief For various key sizes, the value slot inside the HASHNODE
 *        is 8-aligned. Both pNode->u.c (key start) and
 *        pNode->u.c + ulValueOffset (value start) must be aligned
 *        to CCL_MAX_ALIGN.
 *
 * This is the test the spec was missing: without the alignment
 * invariant, a double value inside the hash table would be stored
 * at a non-aligned address and any read as double would be
 * undefined behaviour.
 */
static void test_hashnode_value_alignment(void) {
    HASHCTL hCtl;
    CCLOPTS opts;
    APIRET rc;
    ULONG keySizes[] = { 1, 4, 5, 7, 8, 12, 16 };
    ULONG nSizes = sizeof(keySizes) / sizeof(keySizes[0]);
    ULONG i;
    ULONG j;
    UCHAR keyBuf[32];
    double dValue = 3.14159;
    PHASHNODE pNode;
    UCHAR *pKeyBytes;
    UCHAR *pValueBytes;

    CclOptsInit(&opts);
    opts.pHash = test_hash;
    opts.pCompare = test_compare;

    for (i = 0; i < nSizes; i++) {
        g_ulTestKeySize = keySizes[i];
        memset(keyBuf, 0xAA, sizeof(keyBuf));

        rc = HashCreate(8, keySizes[i], sizeof(double), &opts, &hCtl);
        TEST_ASSERT(rc == NO_ERROR);

        /* The HASHCTL has the expected value offset. */
        TEST_ASSERT(hCtl.ulValueOffset == CCL_ALIGN_UP(keySizes[i]));
        TEST_ASSERT(hCtl.ulValueOffset % CCL_MAX_ALIGN == 0);

        rc = HashPut(&hCtl, keyBuf, &dValue, NULL);
        TEST_ASSERT(rc == NO_ERROR);

        /* Find the single node in the bucket array. */
        pNode = NULL;
        for (j = 0; j < hCtl.ulInternalCapacity; j++) {
            if (hCtl.pInternalBuckets[j]) {
                pNode = (PHASHNODE)hCtl.pInternalBuckets[j];
                break;
            }
        }
        TEST_ASSERT(pNode != NULL);

        /* pNode is 8-aligned. */
        TEST_ASSERT(((ULONG)pNode) % CCL_MAX_ALIGN == 0);

        /* pNode->u is 8-aligned, so the key start is too. */
        pKeyBytes = (UCHAR *)pNode->u.c;
        TEST_ASSERT(((ULONG)pKeyBytes) % CCL_MAX_ALIGN == 0);

        /* The value start is 8-aligned regardless of key size. */
        pValueBytes = pKeyBytes + hCtl.ulValueOffset;
        TEST_ASSERT(((ULONG)pValueBytes) % CCL_MAX_ALIGN == 0);

        /* Reading the value as a double must work. */
        TEST_ASSERT(*(double *)pValueBytes == dValue);

        HashDestroy(&hCtl);
    }
}

/* ==================================================================
 * Basic hash table operations
 * ================================================================== */

/*!
 * @brief Create, put, get, remove, destroy with key size 4.
 */
static void test_hash_basic(void) {
    HASHCTL hCtl;
    CCLOPTS opts;
    APIRET rc;
    ULONG ulKey = 0x12345678;
    ULONG ulValue = 42;
    ULONG ulGot = 0;
    BOOL fFound = FALSE;

    g_ulTestKeySize = 4;
    CclOptsInit(&opts);
    opts.pHash = test_hash;
    opts.pCompare = test_compare;

    rc = HashCreate(8, sizeof(ULONG), sizeof(ULONG), &opts, &hCtl);
    TEST_ASSERT(rc == NO_ERROR);

    rc = HashPut(&hCtl, &ulKey, &ulValue, NULL);
    TEST_ASSERT(rc == NO_ERROR);

    rc = HashGet(&hCtl, &ulKey, &ulGot, sizeof(ulGot), NULL);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(ulGot == 42);

    /* Missing key. */
    ulKey = 0xDEADBEEF;
    rc = HashGet(&hCtl, &ulKey, &ulGot, sizeof(ulGot), NULL);
    TEST_ASSERT(rc == ERROR_NO_MORE_ITEMS);

    /* Remove the existing entry. */
    ulKey = 0x12345678;
    rc = HashRemove(&hCtl, &ulKey, &fFound);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(fFound == TRUE);

    /* Removing it again: not found. */
    fFound = TRUE;
    rc = HashRemove(&hCtl, &ulKey, &fFound);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(fFound == FALSE);

    HashDestroy(&hCtl);
}

/*!
 * @brief Replace semantics: HashPut on an existing key returns
 *        pfReplaced = TRUE and the new value is visible.
 */
static void test_hash_replace(void) {
    HASHCTL hCtl;
    CCLOPTS opts;
    APIRET rc;
    ULONG ulKey = 0xCAFEBABE;
    ULONG ulVal1 = 100;
    ULONG ulVal2 = 200;
    ULONG ulGot = 0;
    BOOL fReplaced = FALSE;

    g_ulTestKeySize = 4;
    CclOptsInit(&opts);
    opts.pHash = test_hash;
    opts.pCompare = test_compare;

    rc = HashCreate(8, sizeof(ULONG), sizeof(ULONG), &opts, &hCtl);
    TEST_ASSERT(rc == NO_ERROR);

    rc = HashPut(&hCtl, &ulKey, &ulVal1, &fReplaced);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(fReplaced == FALSE);

    rc = HashPut(&hCtl, &ulKey, &ulVal2, &fReplaced);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(fReplaced == TRUE);

    rc = HashGet(&hCtl, &ulKey, &ulGot, sizeof(ulGot), NULL);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(ulGot == 200);

    HashDestroy(&hCtl);
}

/*!
 * @brief Insert 100 entries; the table must grow and count must
 *        reach 100.
 */
static void test_hash_grow(void) {
    HASHCTL hCtl;
    CCLOPTS opts;
    APIRET rc;
    ULONG i;
    ULONG ulKey;
    ULONG ulVal;
    ULONG ulCount = 0;
    ULONG ulBuckets = 0;

    g_ulTestKeySize = 4;
    CclOptsInit(&opts);
    opts.pHash = test_hash;
    opts.pCompare = test_compare;

    rc = HashCreate(8, sizeof(ULONG), sizeof(ULONG), &opts, &hCtl);
    TEST_ASSERT(rc == NO_ERROR);

    for (i = 0; i < 100; i++) {
        ulKey = i;
        ulVal = i * 3;
        rc = HashPut(&hCtl, &ulKey, &ulVal, NULL);
        TEST_ASSERT(rc == NO_ERROR);
    }

    rc = HashGetCount(&hCtl, &ulCount);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(ulCount == 100);

    rc = HashGetBucketCount(&hCtl, &ulBuckets);
    TEST_ASSERT(rc == NO_ERROR);
    /* 100 entries at 0.75 load need at least 134 buckets, so at
       least 256. */
    TEST_ASSERT(ulBuckets >= 128);

    HashDestroy(&hCtl);
}

/*!
 * @brief HashSetBucketCount works only when the table is empty.
 */
static void test_hash_set_bucket_count(void) {
    HASHCTL hCtl;
    CCLOPTS opts;
    APIRET rc;
    ULONG ulKey = 1;
    ULONG ulVal = 2;
    ULONG ulBuckets = 0;

    g_ulTestKeySize = 4;
    CclOptsInit(&opts);
    opts.pHash = test_hash;
    opts.pCompare = test_compare;

    rc = HashCreate(8, sizeof(ULONG), sizeof(ULONG), &opts, &hCtl);
    TEST_ASSERT(rc == NO_ERROR);

    /* Empty table: allowed. */
    rc = HashSetBucketCount(&hCtl, 64);
    TEST_ASSERT(rc == NO_ERROR);
    rc = HashGetBucketCount(&hCtl, &ulBuckets);
    TEST_ASSERT(rc == NO_ERROR);
    TEST_ASSERT(ulBuckets == 64);

    /* Insert one entry, then try again: rejected. */
    rc = HashPut(&hCtl, &ulKey, &ulVal, NULL);
    TEST_ASSERT(rc == NO_ERROR);
    rc = HashSetBucketCount(&hCtl, 128);
    TEST_ASSERT(rc == ERROR_INVALID_PARAMETER);

    HashDestroy(&hCtl);
}

/* ==================================================================
 * main
 * ================================================================== */

int main(void) {
    TEST_RUN(test_align_up);
    TEST_RUN(test_hashnode_layout);
    TEST_RUN(test_node_alloc_alignment);
    TEST_RUN(test_hashnode_value_alignment);
    TEST_RUN(test_hash_basic);
    TEST_RUN(test_hash_replace);
    TEST_RUN(test_hash_grow);
    TEST_RUN(test_hash_set_bucket_count);

    return test_summary();
}
