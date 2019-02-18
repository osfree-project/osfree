/* segment intrerface (working with LDT/GDT entries) */

/* osFree internal */
#include <os3/thread.h>
#include <os3/segment.h>

extern "C" void
segment_gdt_set(void *desc, unsigned int size,
                unsigned int entry_number_start,
                l4_os3_thread_t tid)
{
}

extern "C" unsigned 
segment_gdt_get_entry_offset(void)
{
    return 0;
}
