/*!
 *
 * @file ccl_base.c
 *
 * @brief Public base helpers and internal validation for the ccl
 *        library.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library.
 *
 * Provides three things:
 *
 *   - CclOptsInit: fill a CCLOPTS block with the default values
 *     (all callbacks NULL, threshold 0, flags 0).
 *   - CclValidateOpts: strict validation of a CCLOPTS block against
 *     a per-container allow/require mask pair. Rejects unknown
 *     flags, disallowed callbacks, disallowed flags, out-of-range
 *     rehash thresholds, and missing required callbacks.
 *   - ccl_validate: the shared magic-word validator used by every
 *     container's static get_ctl helper.
 */

#include <string.h>
#include "ccl_base.h"
#include "ccl_base_private.h"

/* ==================================================================
 * Public helpers
 * ================================================================== */

/*!
 * @brief Fill an option block with the default values.
 *
 * Zero-fills the block. All callback fields become NULL, the rehash
 * threshold becomes 0 (use container default), the flags become 0.
 *
 * Passing NULL is a no-op.
 *
 * @param[out] pOpts  Block to initialise. May be NULL.
 */
void APIENTRY CclOptsInit(PCCLOPTS pOpts) {
    if (!pOpts) return;
    memset(pOpts, 0, sizeof(*pOpts));
}

/*!
 * @brief Validate an option block against the caller's requirements.
 *
 * See ccl_base.h for the full validation rules. Summary of the
 * checks performed, in order:
 *
 *   1. NULL block. Equivalent to a default-initialized block; any
 *      non-zero requirement fails because every required field is
 *      NULL.
 *   2. Unknown flag bits (outside CCL_FLAG_KNOWN_MASK).
 *   3. Flag applicability: a known flag whose CCL_OPT_FLAG_* bit is
 *      not in ulAllowed is rejected.
 *   4. Callback fields present but not allowed by ulAllowed.
 *   5. Rehash threshold above CCL_REHASH_THRESHOLD_MAX.
 *   6. Rehash threshold non-zero but CCL_OPT_REHASH not in
 *      ulAllowed.
 *   7. Required callback fields missing.
 *
 * The order of checks is not significant for the result (all paths
 * return ERROR_INVALID_PARAMETER on failure); the order is presented
 * for documentation clarity.
 *
 * @param[in] pOpts      Option block, or NULL.
 * @param[in] ulAllowed  Bit mask of accepted CCL_OPT_* bits.
 * @param[in] ulRequired Bit mask of required CCL_OPT_* bits.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Validation failed.
 */
APIRET APIENTRY CclValidateOpts(PCCLOPTS pOpts, ULONG ulAllowed, ULONG ulRequired) {
    ULONG ulFlagMask;

    /* ------------------------------------------------------------------
     * 1. NULL block.
     *
     * A NULL pOpts is equivalent to a block filled by CclOptsInit.
     * All callback fields would be NULL and the threshold zero. If
     * any field is required by ulRequired, the NULL block cannot
     * satisfy it.
     * ------------------------------------------------------------------ */
    if (!pOpts) {
        return (ulRequired == 0) ? NO_ERROR : ERROR_INVALID_PARAMETER;
    }

    /* ------------------------------------------------------------------
     * 2. Unknown flag bits.
     *
     * Any bit in ulFlags outside CCL_FLAG_KNOWN_MASK is rejected.
     * This is strict: the caller must not pass flags that this
     * version of the library does not define. Forward compatibility
     * is not a goal here; CCL is an internal library, rebuilt
     * together with its consumers.
     * ------------------------------------------------------------------ */
    if (pOpts->ulFlags & ~CCL_FLAG_KNOWN_MASK) {
        return ERROR_INVALID_PARAMETER;
    }

    /* ------------------------------------------------------------------
     * 3. Flag applicability.
     *
     * Translate the flags present in ulFlags into the corresponding
     * CCL_OPT_FLAG_* bits and verify that each of them is present in
     * ulAllowed. A known flag that the container does not accept is
     * rejected, not silently ignored.
     * ------------------------------------------------------------------ */
    ulFlagMask = 0;
    if (pOpts->ulFlags & CCL_FLAG_ALLOW_DUPLICATES)
        ulFlagMask |= CCL_OPT_FLAG_DUPLICATES;
    if (pOpts->ulFlags & CCL_FLAG_DEFAULT_MANAGED)
        ulFlagMask |= CCL_OPT_FLAG_MANAGED;
    if (pOpts->ulFlags & CCL_FLAG_DEFAULT_READONLY)
        ulFlagMask |= CCL_OPT_FLAG_READONLY;
    if (ulFlagMask & ~ulAllowed) {
        return ERROR_INVALID_PARAMETER;
    }

    /* ------------------------------------------------------------------
     * 4. Callback fields present but not allowed.
     *
     * If a field is non-NULL but the corresponding CCL_OPT_* bit is
     * not set in ulAllowed, the container does not know what to do
     * with it and the caller's intent cannot be honoured. Reject it
     * rather than silently dropping the callback (which would leave
     * owned data unreleased and cause a leak far from the caller).
     * ------------------------------------------------------------------ */
    if (pOpts->pCompare != NULL && !(ulAllowed & CCL_OPT_COMPARE))
        return ERROR_INVALID_PARAMETER;
    if (pOpts->pHash != NULL && !(ulAllowed & CCL_OPT_HASH))
        return ERROR_INVALID_PARAMETER;
    if (pOpts->pKeyFn != NULL && !(ulAllowed & CCL_OPT_KEYFN))
        return ERROR_INVALID_PARAMETER;
    if (pOpts->pDestroyKey != NULL && !(ulAllowed & CCL_OPT_DESTROY_KEY))
        return ERROR_INVALID_PARAMETER;
    if (pOpts->pDestroyValue != NULL && !(ulAllowed & CCL_OPT_DESTROY_VALUE))
        return ERROR_INVALID_PARAMETER;

    /* ------------------------------------------------------------------
     * 5. Rehash threshold above maximum.
     *
     * 1000 permille equals 1.0 (rehash would never trigger). Values
     * above 1000 are meaningless. Values 1..1000 are accepted; 0
     * means "use container default".
     * ------------------------------------------------------------------ */
    if (pOpts->ulRehashThreshold > CCL_REHASH_THRESHOLD_MAX) {
        return ERROR_INVALID_PARAMETER;
    }

    /* ------------------------------------------------------------------
     * 6. Rehash threshold present but container does not rehash.
     *
     * A non-zero threshold on a container that does not accept
     * CCL_OPT_REHASH is a caller error: the value is not used and
     * the caller probably meant to configure a hash-based container.
     * ------------------------------------------------------------------ */
    if (pOpts->ulRehashThreshold != 0 && !(ulAllowed & CCL_OPT_REHASH)) {
        return ERROR_INVALID_PARAMETER;
    }

    /* ------------------------------------------------------------------
     * 7. Required callback fields missing.
     *
     * A container that cannot function without a callback (hashtable
     * without pHash, rbtree without pCompare, etc.) declares it in
     * ulRequired. A NULL here means the caller passed NULL pOpts or
     * an opts block without the required callback.
     * ------------------------------------------------------------------ */
    if ((ulRequired & CCL_OPT_COMPARE) && pOpts->pCompare == NULL)
        return ERROR_INVALID_PARAMETER;
    if ((ulRequired & CCL_OPT_HASH) && pOpts->pHash == NULL)
        return ERROR_INVALID_PARAMETER;
    if ((ulRequired & CCL_OPT_KEYFN) && pOpts->pKeyFn == NULL)
        return ERROR_INVALID_PARAMETER;
    if ((ulRequired & CCL_OPT_DESTROY_KEY) && pOpts->pDestroyKey == NULL)
        return ERROR_INVALID_PARAMETER;
    if ((ulRequired & CCL_OPT_DESTROY_VALUE) && pOpts->pDestroyValue == NULL)
        return ERROR_INVALID_PARAMETER;

    return NO_ERROR;
}

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Test whether a control block carries the expected magic.
 *
 * p is interpreted as a pointer to a ULONG at offset 0. NULL is
 * treated as invalid.
 *
 * @param[in] p        Control block, or NULL.
 * @param[in] ulMagic  Expected magic value.
 *
 * @return p if the magic matches, NULL otherwise.
 *
 * @retval NULL  p is NULL, or its magic does not match.
 */
PVOID ccl_validate(PVOID p, ULONG ulMagic) {
    if (!p) return NULL;
    if (*(const ULONG *)p != ulMagic) return NULL;
    return p;
}
