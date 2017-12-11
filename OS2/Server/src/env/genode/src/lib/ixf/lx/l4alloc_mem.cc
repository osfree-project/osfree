//
// Memmory allocation work in following logic:
//   1. Get region of memory using l4dm_mem_open
//   2. After memory opened we need to map pages
// Map can be done page by page. Whole region can't be mapped if it large of L4_PAGESIZE
//

/* OS/2 API headers */
#define  INCL_BASE
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/l4_alloc_mem.h>
#include <os3/cfgparser.h>
#include <os3/io.h>

/* Genode headers */
#include <dataspace/capability.h>

void *l4_alloc_mem(unsigned long area, int base, int size, int flags, unsigned long PIC, void *ds)
{
    return NULL;
}

#if 0

int l4_translate_os2_flags(int flags)
{
    int l4flags=0;

    if ((flags & PAG_COMMIT) == PAG_COMMIT)
        l4flags |= ;
}

#endif
