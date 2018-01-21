//
// Memory allocation work in following logic:
//   1. Get region of memory using l4dm_mem_open
//   2. After memory opened we need to map pages
// Map can be done page by page. Whole region can't be mapped if it large of L4_PAGESIZE
//

/* OS/2 API headers */
#define  INCL_BASE
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/l4_alloc_mem.h>
#include <os3/dataspace.h>
#include <os3/cfgparser.h>
#include <os3/io.h>

/* Genode headers */
#include <dataspace/capability.h>

extern "C"
void *l4_alloc_mem(unsigned long long area, int base,
                   int size, int flags,
                   unsigned long PIC, l4_os3_dataspace_t *ds)
{
    int l4_flags = l4_translate_os2_flags(flags);
    l4_os3_dataspace_t temp_ds;
    //void *start = NULL;
    APIRET rc;

    rc = DataspaceAlloc(&temp_ds, l4_flags, 0, size);

    if (rc)
        return NULL;

    //l4rm_area_attach
    //rc = RegAreaAttachToRegion();

    return NULL;
}

extern "C"
int l4_translate_os2_flags(int flags) { /* PAG_COMMIT|PAG_EXECUTE|PAG_READ|PAG_WRITE */
                                        /* L4RM_MAP   L4DM_READ   L4DM_READ or L4DM_WRITE */
   int l4flags = 0;
   if((flags & PAG_COMMIT)==PAG_COMMIT) {
     l4flags = l4flags | L4RM_MAP;
    }
   if((flags & PAG_EXECUTE)==PAG_EXECUTE) {
     l4flags = l4flags | L4DM_READ;
    }

   if( ((flags & PAG_READ)==PAG_READ) && 
       (!((flags & PAG_WRITE)==PAG_WRITE)) ) {
     l4flags = l4flags | L4DM_WRITE;
    }

   if((flags & PAG_WRITE)==PAG_WRITE) {
     l4flags = l4flags | L4DM_WRITE;
    }


   return l4flags;
}
