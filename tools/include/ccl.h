/* ccl.h - public umbrella of the ccl container library.
 * Part of the ccl container library. */
/**
 * @file ccl.h
 * @brief Public umbrella header of the ccl container library.
 *
 * Consumers include this header to obtain the public ccl containers
 * and the public allocator installation API. Only our own containers
 * are exported here; legacy IBM Dlist and all_dlist headers are not
 * part of this umbrella.
 *
 * SOM and other host environments that want all ccl allocations to
 * go through their own allocator include this header and call
 * CclSetAllocator once at startup.
 */
#ifndef CCL_H
#define CCL_H

#include "ccl_alloc.h"
#include "vector.h"
#include "strset.h"

#endif /* CCL_H */
