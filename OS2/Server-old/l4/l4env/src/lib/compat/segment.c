/*
 *
 */

#include <l4/os3/segment.h>

#if defined(L4API_l4v2)

void l4os3_gdt_set(void *desc, unsigned int size,
                   unsigned int entry_number_start,
                   l4os3_cap_idx_t tid)
{
  fiasco_gdt_set(desc, size, entry_number_start, tid);
}

unsigned l4os3_gdt_get_entry_offset(void)
{
  return fiasco_gdt_get_entry_offset();
}

#elif defined(L4API_l4f)

#include <l4/sys/utcb.h>

void l4os3_gdt_set(void *desc, unsigned int size,
                   unsigned int entry_number_start,
                   l4os3_cap_idx_t tid)
{
  fiasco_gdt_set(tid, desc, size, entry_number_start);
}

unsigned l4os3_gdt_get_entry_offset(void)
{
  fiasco_gdt_get_entry_offset(???tid ,l4_utcb_direct());
}

#else
#error "Not implemented!"
#endif
