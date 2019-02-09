/* segment intrerface (working with LDT/GDT entries) */

/* osFree internal */
#include <os3/thread.h>
#include <os3/segment.h>

#if defined(L4API_l4v2)

void segment_gdt_set(void *desc, unsigned int size,
                   unsigned int entry_number_start,
                   l4_os3_thread_t tid)
{
  fiasco_gdt_set(desc, size, entry_number_start, tid.thread);
}

unsigned segment_gdt_get_entry_offset(void)
{
  return fiasco_gdt_get_entry_offset();
}

#elif defined(L4API_l4f)

#include <l4/sys/utcb.h>

void segment_gdt_set(void *desc, unsigned int size,
                   unsigned int entry_number_start,
                   l4_os3_thread_t tid)
{
  fiasco_gdt_set(tid, desc, size, entry_number_start);
}

unsigned segment_gdt_get_entry_offset(void)
{
  fiasco_gdt_get_entry_offset(tid, l4_utcb_direct());
}

#else
#error "Not implemented!"
#endif
