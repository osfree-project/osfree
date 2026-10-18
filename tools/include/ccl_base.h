/*!
 *
 * @file ccl_base.h
 *
 * @brief Public base types shared by all ccl containers.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. This is a public header.
 *
 * Contains the callback typedefs, the option block (CCLOPTS) shared
 * by every container constructor, and the constants used by
 * CclValidateOpts and by the iterator subsystem. Size parameters
 * (element size, key size, initial capacity) are part of each
 * container's own Create signature and are not stored in CCLOPTS.
 *
 * Callback calling convention. The container callbacks
 * (CCL_COMPARE_FN, CCL_HASH_FN, CCL_DESTROY_FN, CCL_KEY_FN) are
 * declared without APIENTRY. This differs from the allocator hooks
 * in ccl_alloc.h, which do use APIENTRY. The reason is ownership of
 * the calling-convention adaptation: allocator hooks are installed
 * once at startup by the host (SOM-init) and must exactly match the
 * hook type, so APIENTRY is baked into the typedef. Container
 * callbacks are supplied by the caller per CCLOPTS and are typically
 * ordinary C functions or SOM method adapters; the SOM adapter is
 * responsible for any calling-convention adaptation on its side.
 * CCL never calls these callbacks through a function pointer that
 * requires a specific calling convention.
 */

#ifndef CCL_BASE_H
#define CCL_BASE_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Callbacks
 * ================================================================== */

/*!
 * @typedef CCL_COMPARE_FN
 * @brief Ordering callback.
 *
 * Returns negative, zero or positive according to the usual ordering
 * contract. Receives pointers to the stored bytes of the two elements
 * (or to the key bytes, for containers with a separate key) and the
 * caller-supplied context.
 *
 * @param[in] pA    Pointer to the first element's bytes.
 * @param[in] pB    Pointer to the second element's bytes.
 * @param[in] pCtx  Caller context (SOM Environment *, or NULL).
 *
 * @return Negative, zero or positive.
 */
typedef int   (APIENTRY *CCL_COMPARE_FN)(PCVOID pA, PCVOID pB, PVOID pCtx);

/*!
 * @typedef CCL_HASH_FN
 * @brief Hash callback.
 *
 * Receives a pointer to the stored bytes of the key and the caller
 * context.
 *
 * @param[in] pKey  Pointer to the key bytes.
 * @param[in] pCtx  Caller context.
 *
 * @return A 32-bit hash value.
 */
typedef ULONG (APIENTRY *CCL_HASH_FN)(PCVOID pKey, PVOID pCtx);

/*!
 * @typedef CCL_DESTROY_FN
 * @brief Destruction callback.
 *
 * Receives a pointer to the stored bytes of one element (or key, or
 * value). The callback reads what it needs from those bytes (for
 * example, a string pointer) and frees the pointed-to data. The
 * container itself does not interpret the bytes.
 *
 * Not reentrant: must not modify the container, must not retain the
 * pointer it receives, and must not assume the container is still
 * alive (the control block may already be gone during Destroy).
 *
 * @param[in] pElem  Pointer to the stored bytes.
 */
typedef void  (APIENTRY *CCL_DESTROY_FN)(PVOID pElem);

/*!
 * @typedef CCL_KEY_FN
 * @brief Key extraction callback.
 *
 * Returns a borrowed pointer to the key bytes inside the element.
 * The pointer is valid while the element stays in the container.
 *
 * @param[in] pElem  Pointer to the element bytes.
 * @param[in] pCtx   Caller context.
 *
 * @return A borrowed pointer to the key bytes.
 */
typedef PVOID (APIENTRY *CCL_KEY_FN)(PCVOID pElem, PVOID pCtx);

/* ==================================================================
 * Option block
 * ================================================================== */

/*!
 * @struct _CCLOPTS
 * @brief Optional parameters for container constructors.
 *
 * Every container constructor accepts a PCCLOPTS. Passing NULL is
 * equivalent to passing a block filled by CclOptsInit: all callbacks
 * NULL, rehash threshold 0 (use the container's default), no flags.
 *
 * The block is read once at creation; the container copies the
 * fields it needs. The caller may reuse or release the block after
 * the constructor returns.
 *
 * Validation. The container constructor validates the block through
 * CclValidateOpts with a container-specific ulAllowed / ulRequired
 * mask pair. A field that is present but not allowed, or a required
 * field that is missing, causes ERROR_INVALID_PARAMETER. See
 * CclValidateOpts below for the full validation rules, and the
 * architecture document (§5.1) for the per-container mask table.
 */
typedef struct _CCLOPTS {
    /*!
     * Rehash threshold in permille: 0 = use container default (750),
     * 1..1000 = explicit value (750 = 0.75). Values above 1000 are
     * rejected by CclValidateOpts. Ignored by containers that do
     * not rehash.
     */
    ULONG          ulRehashThreshold;

    /*!
     * Ordering callback. Required by sorted containers and by any
     * operation that needs to compare keys by value. NULL otherwise.
     */
    CCL_COMPARE_FN pCompare;

    /*!
     * Hash callback. Required by the hash-based containers. NULL
     * otherwise.
     */
    CCL_HASH_FN    pHash;

    /*!
     * Key extraction callback. NULL means the element itself is the
     * key. Non-NULL is used by Key* variants and by the containers
     * that store a separate key plus a value in one element.
     */
    CCL_KEY_FN     pKeyFn;

    /*!
     * Key destruction callback. Called when a key is removed or the
     * container is destroyed. NULL means the key is not owned.
     */
    CCL_DESTROY_FN pDestroyKey;

    /*!
     * Value destruction callback. Called when an element's value is
     * removed or the container is destroyed. NULL means the value is
     * not owned.
     */
    CCL_DESTROY_FN pDestroyValue;

    /*!
     * Behaviour flags (CCL_FLAG_*). See below.
     */
    ULONG          ulFlags;
} CCLOPTS;

/*!
 * @typedef PCCLOPTS
 * @brief Pointer to an option block.
 */
typedef CCLOPTS *PCCLOPTS;

/* ==================================================================
 * Thresholds (permille)
 * ================================================================== */

/*!
 * @def CCL_REHASH_THRESHOLD_DEFAULT
 * @brief Container default rehash threshold (0.75). Value: 750.
 */
#define CCL_REHASH_THRESHOLD_DEFAULT   750UL

/*!
 * @def CCL_REHASH_THRESHOLD_MIN
 * @brief Minimum accepted explicit threshold. Value: 1.
 */
#define CCL_REHASH_THRESHOLD_MIN         1UL

/*!
 * @def CCL_REHASH_THRESHOLD_MAX
 * @brief Maximum accepted threshold (1.0). Value: 1000.
 */
#define CCL_REHASH_THRESHOLD_MAX      1000UL

/* ==================================================================
 * Flags (CCLOPTS.ulFlags)
 * ================================================================== */

/*!
 * @def CCL_FLAG_ALLOW_DUPLICATES
 * @brief Bag / relation semantics: duplicate keys allowed.
 *
 * A lookup returns the first inserted occurrence; Remove removes
 * one occurrence. Ignored by containers that require unique keys.
 */
#define CCL_FLAG_ALLOW_DUPLICATES      0x00000001UL

/*!
 * @def CCL_FLAG_DEFAULT_MANAGED
 * @brief Iterators created without IteratorSetMode default to
 *        managed mode.
 */
#define CCL_FLAG_DEFAULT_MANAGED       0x00000002UL

/*!
 * @def CCL_FLAG_DEFAULT_READONLY
 * @brief Iterators created without IteratorSetMode default to
 *        read-only; IteratorRemove returns ERROR_NOT_SUPPORTED.
 */
#define CCL_FLAG_DEFAULT_READONLY      0x00000004UL

/*!
 * @def CCL_FLAG_KNOWN_MASK
 * @brief Bit mask of all currently defined flags.
 *
 * Unknown bits in CCLOPTS.ulFlags are rejected by CclValidateOpts.
 */
#define CCL_FLAG_KNOWN_MASK            0x00000007UL

/* ==================================================================
 * Options masks (ulAllowed / ulRequired of CclValidateOpts)
 *
 * Bits 0..7 describe callback fields of CCLOPTS. Bits 8..15
 * describe flag applicability: a flag is "allowed" if its
 * CCL_OPT_FLAG_* bit is set in ulAllowed. This separation lets a
 * container declare "callbacks I accept" and "flags I accept"
 * independently.
 * ================================================================== */

/*!
 * @def CCL_OPT_COMPARE
 * @brief pCompare is allowed (or required).
 */
#define CCL_OPT_COMPARE                0x0001UL

/*!
 * @def CCL_OPT_HASH
 * @brief pHash is allowed (or required).
 */
#define CCL_OPT_HASH                   0x0002UL

/*!
 * @def CCL_OPT_KEYFN
 * @brief pKeyFn is allowed (or required).
 */
#define CCL_OPT_KEYFN                  0x0004UL

/*!
 * @def CCL_OPT_DESTROY_KEY
 * @brief pDestroyKey is allowed (or required).
 */
#define CCL_OPT_DESTROY_KEY            0x0008UL

/*!
 * @def CCL_OPT_DESTROY_VALUE
 * @brief pDestroyValue is allowed (or required).
 */
#define CCL_OPT_DESTROY_VALUE          0x0010UL

/*!
 * @def CCL_OPT_REHASH
 * @brief ulRehashThreshold may be specified explicitly (non-zero).
 */
#define CCL_OPT_REHASH                 0x0020UL

/*!
 * @def CCL_OPT_FLAG_DUPLICATES
 * @brief CCL_FLAG_ALLOW_DUPLICATES is allowed in ulFlags.
 */
#define CCL_OPT_FLAG_DUPLICATES        0x0100UL

/*!
 * @def CCL_OPT_FLAG_MANAGED
 * @brief CCL_FLAG_DEFAULT_MANAGED is allowed in ulFlags.
 */
#define CCL_OPT_FLAG_MANAGED           0x0200UL

/*!
 * @def CCL_OPT_FLAG_READONLY
 * @brief CCL_FLAG_DEFAULT_READONLY is allowed in ulFlags.
 */
#define CCL_OPT_FLAG_READONLY          0x0400UL

/* ==================================================================
 * Helpers
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
void APIENTRY CclOptsInit(PCCLOPTS pOpts);

/*!
 * @brief Validate an option block against the caller's requirements.
 *
 * Validation rules:
 *
 *   - pOpts == NULL is equivalent to a block filled by CclOptsInit.
 *     If ulRequired is non-zero, a NULL block fails (all required
 *     fields are missing).
 *   - Unknown bits in ulFlags (outside CCL_FLAG_KNOWN_MASK) cause
 *     ERROR_INVALID_PARAMETER.
 *   - A flag whose CCL_OPT_FLAG_* bit is not in ulAllowed causes
 *     ERROR_INVALID_PARAMETER.
 *   - A callback field whose CCL_OPT_* bit is not in ulAllowed
 *     causes ERROR_INVALID_PARAMETER.
 *   - A non-zero ulRehashThreshold requires CCL_OPT_REHASH in
 *     ulAllowed; a value above CCL_REHASH_THRESHOLD_MAX is
 *     rejected.
 *   - A callback field whose CCL_OPT_* bit is in ulRequired but
 *     whose value is NULL causes ERROR_INVALID_PARAMETER.
 *
 * @param[in] pOpts      Option block, or NULL.
 * @param[in] ulAllowed  Bit mask of accepted CCL_OPT_* bits.
 * @param[in] ulRequired Bit mask of required CCL_OPT_* bits.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Validation failed.
 */
APIRET APIENTRY CclValidateOpts(PCCLOPTS pOpts, ULONG ulAllowed, ULONG ulRequired);

#ifdef __cplusplus
}
#endif

#endif /* CCL_BASE_H */
