#ifndef __OS3_SEGMENT_H__
#define __OS3_SEGMENT_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/types.h>

void l4os3_gdt_set(void *desc, unsigned int size,
                   unsigned int entry_number_start,
                   l4os3_cap_idx_t tid);

unsigned l4os3_gdt_get_entry_offset(void);

#if defined(L4API_l4v2)

/* l4env includes */
#include <l4/sys/segment.h>

#elif defined(L4API_l4f)

/* l4re includes */
#include <l4/sys/segment.h>

#else
#error "Not implemented!"
#endif

#ifdef __cplusplus
  }
#endif

#endif
