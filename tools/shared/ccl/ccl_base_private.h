/*!
 *
 * @file ccl_base_private.h
 *
 * @brief Internal helpers of the ccl base module.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * Contains the shared magic-word validator that every container
 * module uses from its static get_ctl helper. The validator is
 * intentionally minimal: it checks that the control block pointer is
 * non-NULL and that the ULONG at offset 0 matches the expected
 * magic value.
 */

#ifndef CCL_BASE_PRIVATE_H
#define CCL_BASE_PRIVATE_H

#include "os2types.h"
#include "ccl_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Test whether a control block carries the expected magic.
 *
 * p is interpreted as a pointer to a ULONG at offset 0. NULL is
 * treated as invalid.
 *
 * Every container's get_ctl helper calls this once with its own
 * magic value:
 *
 *     static HVECTOR VectorValidate(HVECTOR hVector) {
 *         return (HVECTOR)ccl_validate(hVector, CCL_VECTOR_MAGIC);
 *     }
 *
 * The check rejects handles from other containers, most
 * uninitialized values, and handles released through Destroy (the
 * magic is overwritten with 0 during destroy). It does not protect
 * against a stray pointer that happens to point at a readable
 * location carrying a valid magic value; that is a fundamental
 * limitation of any magic scheme without full type tagging.
 *
 * @param[in] p        Control block, or NULL.
 * @param[in] ulMagic  Expected magic value.
 *
 * @return p if the magic matches, NULL otherwise.
 *
 * @retval NULL  p is NULL, or its magic does not match.
 */
PVOID ccl_validate(PVOID p, ULONG ulMagic);

#ifdef __cplusplus
}
#endif

#endif /* CCL_BASE_PRIVATE_H */
