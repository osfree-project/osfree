#ifndef __OS3_SEGMENT_H__
#define __OS3_SEGMENT_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/types.h>

void l4os3_gdt_set(void *desc, unsigned int size,
                   unsigned int entry_number_start,
                   l4_os3_cap_idx_t tid);

unsigned l4os3_gdt_get_entry_offset(void);

#if defined(__l4env__) || defined(__l4re__)

#include <l4/sys/segment.h>

#endif

#ifdef __cplusplus
  }
#endif

#endif
