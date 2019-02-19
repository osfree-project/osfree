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
#include <os3/allocmem.h>
#include <os3/dataspace.h>
#include <os3/rm.h>

void *allocmem(unsigned long long area,
               int base,
               int size,
               int flags,
               unsigned long PIC,
               l4_os3_dataspace_t *ds)
{
    int rights = translate_os2_flags(flags);
    void *addr;
    APIRET rc;

    rc = DataspaceAlloc(ds, rights, DEFAULT_DSM, size);

    if (rc)
    {
      return NULL;
    }

    rc = RegAreaAttach(&addr, size, (unsigned long)area,
                       rights, *ds, 0, 0);

    if (rc)
    {
      return NULL;
    }

    return addr;

    base = base;
    PIC = PIC;
}

int translate_os2_flags(int flags)
{
    int rights = 0;

    if (flags & PAG_COMMIT)
    {
      rights |= DATASPACE_MAP;
    }

    if (flags & PAG_EXECUTE)
    {
      rights |= DATASPACE_READ;
    }

    if ( (flags & PAG_READ) && ! (flags & PAG_WRITE) )
    {
      rights |= DATASPACE_WRITE;
    }

    if (flags & PAG_WRITE)
    {
      rights |= DATASPACE_WRITE;
    }

    return rights;
}
