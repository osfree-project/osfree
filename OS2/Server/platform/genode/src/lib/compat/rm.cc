/* Region mapper interface */

#include <stdlib.h>

/* osFree Internal */
#include <os3/io.h>
#include <os3/rm.h>

/* Genode includes */
//#include <dataspace/client.h>
#include <dataspace/capability.h>
#include <rom_session/connection.h>

/* local includes */
#include "genode_env.h"

#define REGION_FREE      0
#define REGION_DATASPACE 1
#define REGION_RESERVED  2

using namespace Genode;

struct vmdata *root = NULL;

struct vmdata
{
  void *addr;
  unsigned long size;
  unsigned long offset;
  l4_os3_dataspace_t ds;
  //Dataspace_capability ds;
  //Rom_connection *rom;
  struct vmdata *next, *prev;
};

int add_node(void *addr,
             unsigned long size,
             unsigned long offset,
             l4_os3_dataspace_t ds)
{
    struct vmdata *ptr;

    ptr = (struct vmdata *)malloc(sizeof(struct vmdata));

    if (! ptr)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ptr->addr   = addr;
    ptr->size   = size;
    ptr->offset = offset;
    ptr->ds     = ds;

    if (! root)
    {
        root = ptr;
        ptr->prev = NULL;
        ptr->next = NULL;
    }
    else
    {
        ptr->next = root;
        ptr->prev = NULL;
        root = ptr;
        ptr->next->prev = ptr;
    }

    return NO_ERROR;
}

struct vmdata *lookup_node(void *addr)
{
    struct vmdata *ptr;

    for (ptr = root; ptr; ptr = ptr->next)
    {
        if ((char *)ptr->addr <= (char *)addr && (char *)addr <= (char *)ptr->addr + ptr->size)
        {
            break;
        }
    }

    if (ptr)
    {
        // node found
        // remove it from the list
        if (! ptr->prev)
        {
            if (ptr->next)
            {
                ptr->next->prev = NULL;
                root = ptr->next;
            }
            else
            {
                root = NULL;
            }
        }
        else
        {
            ptr->prev->next = ptr->next;

            if (ptr->next)
            {
                ptr->next->prev = ptr->prev;
            }
            else
            {
                ptr->prev->next = NULL;
            }
        }
    }
    else
    {
        return NULL;
    }

    return ptr;
}

int del_node(void *addr)
{
    struct vmdata *ptr;

    ptr = lookup_node(addr);

    if (ptr)
    {
        free(ptr);
    }
    else
    {
        return ERROR_FILE_NOT_FOUND;
    }

    return NO_ERROR;
}

extern "C"
long RegDetach(void *addr)
{
    return 0;
}

extern "C"
long RegAttach(void               **start,
               unsigned long      size,
               unsigned long      flags,
               l4_os3_dataspace_t ds,
               unsigned long      offset,
               unsigned char      align)
{
    *start = NULL;

    return RegAttachToRegion(start, size, flags,
                             ds, offset, align);
}

extern "C"
long RegAttachToRegion(void               **start,
                       unsigned long      size,
                       unsigned long      flags,
                       l4_os3_dataspace_t ds,
                       unsigned long      offset,
                       unsigned char      align)
{
    Genode::Env &env = genode_env();
    Genode::Dataspace_capability _ds;
    bool executable = false;
    bool writable = false;
    void *addr = *start;
    bool is_local_addr = false;

    if (addr)
    {
        is_local_addr = true;
    }

    if (flags & (DATASPACE_READ | DATASPACE_WRITE))
    {
        writable = true;
    }
    else if (flags & DATASPACE_READ)
    {
        writable = false;
    }

    if (flags & DATASPACE_EXECUTE)
    {
        executable = true;
    }

    _ds = *(Genode::Dataspace_capability *)ds;

    if (! _ds.valid())
    {
        return ERROR_INVALID_DATASPACE;
    }

    try
    {
        *start = (void *)env.rm().attach(_ds, size, offset,
                     is_local_addr, addr, executable, writable);
    }
    catch (Region_map::Invalid_dataspace)
    {
        return ERROR_INVALID_DATASPACE;
    }
    catch (Region_map::Region_conflict)
    {
        return ERROR_ALREADY_USED;
    }

    if (add_node(addr, size, offset, ds))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return NO_ERROR;
}

extern "C"
long RegAreaRelease(unsigned long long area)
{
    return 0;
}

extern "C"
long RegAreaReleaseAddr(void *addr)
{
    return 0;
}

extern "C"
long RegAreaReserveInArea(unsigned long size,
                          unsigned long flags,
                          void          **addr,
                          unsigned long long *area)
{
    return 0;
}

extern "C"
long RegAreaReserveRegionInArea(unsigned long size,
                                unsigned long flags,
                                void          *addr,
                                unsigned long long *area)
{
    return 0;
}

extern "C"
long RegAreaReserve(unsigned long size,
                          unsigned long flags,
                          void          **addr,
                          unsigned long long *area)
{
    unsigned long long a = 0;
    *area = a;
    return RegAreaReserveInArea(size, flags, addr, area);
}

extern "C"
long RegAreaAttachToRegion(void               *start,
                           unsigned long      size,
                           unsigned long long area,
                           unsigned long      flags,
                           l4_os3_dataspace_t ds,
                           unsigned long      offset,
                           unsigned char      align)
{
    return 0;
}

extern "C"
long RegAttachDataspaceToArea(l4_os3_dataspace_t ds,
                              unsigned long long area,
                              unsigned long      flags,
                              void               *addr)
{
    //ULONG rights = 0;

    //if (flags & DATASPACE_READ)
    //    rights |= L4DM_READ;
    //if (flags & DATASPACE_WRITE)
    //    rights |= L4DM_WRITE;

    return attach_ds_area(ds, area, flags, addr);
}

extern "C"
long RegLookupRegion(void               *addr,
                     void               **addr_new,
                     unsigned long      *size,
                     unsigned long      *offset,
                     l4_os3_dataspace_t *ds)
{
    return 0;
}


extern "C"
long attach_ds(l4_os3_dataspace_t ds, unsigned long flags, void **addr)
//long attach_ds(l4os3_ds_t *ds, unsigned long flags, void **addr)
{
  int error;
  ULONG size;
  //ULONG rights = 0;
  APIRET rc;

  //if (flags & DATASPACE_READ)
    //rights |= L4DM_READ;
  //if (flags & DATASPACE_WRITE)
    //rights |= L4DM_WRITE;

  rc = DataspaceGetSize(ds, (unsigned long *)&size);

  //if ((size = l4os3_ds_size(ds)) < 0)
  if (rc)
    {
      //io_log("Error %d (%s) getting size of dataspace\n",
          //error, l4os3_errtostr(error));
      return rc;
    }

    //if ((error = l4rm_attach(ds, size, 0, flags, (void **)addr)))
    //if ((error = l4os3_rm_attach((void **)addr, size, flags, *ds, 0, 0)))
    if ((error = RegAttach((void **)addr, size, flags, ds, 0, 0)))
    {
      io_log("Error %d attaching dataspace\n",
          error);
      return error;
    }

  return 0;
}

/** attach dataspace to our address space. (concrete address) */
extern "C"
long attach_ds_reg(l4_os3_dataspace_t ds, unsigned long flags, void *addr)
{
  //int error;
  ULONG size;
  //ULONG rights = 0;
  APIRET rc;

  //if (flags & DATASPACE_READ)
    //rights |= L4DM_READ;
  //if (flags & DATASPACE_WRITE)
    //rights |= L4DM_WRITE;

  rc = DataspaceGetSize(ds, (unsigned long *)&size);

  /* get dataspace size */
  //if ((error = l4dm_mem_size(&ds, &size)))
  if (rc)
    {
      //io_log("Error %d (%s) getting size of dataspace\n",
	//  error, l4os3_errtostr(error));
      io_log("Error %u getting size of dataspace\n", rc);
      return rc;
    }

  /* attach it to a given region */
  //if ((error = l4rm_attach_to_region(&ds, (void *)a, size, 0, flags)))
  if ( (rc = RegAttachToRegion(&addr, size, flags, ds, 0, 0)) )
    {
      io_log("Error %d (%s) attaching dataspace\n", rc);
      return rc;
    }

  return 0;
}

/** attach dataspace to our address space. (concrete address) */
extern "C"
long attach_ds_area(l4_os3_dataspace_t ds, unsigned long long area, unsigned long flags, void *addr)
{
  //int error;
  ULONG size;
  //ULONG rights = 0;
  APIRET rc;

  //if (flags & DATASPACE_READ)
    //rights |= L4DM_READ;
  //if (flags & DATASPACE_WRITE)
    //rights |= L4DM_WRITE;

  rc = DataspaceGetSize(ds, (unsigned long *)&size);

  /* get dataspace size */
  //if ((error = l4dm_mem_size(&ds, &size)))
  if (rc)
    {
      //io_log("Error %d (%s) getting size of dataspace\n",
      //  error, l4os3_errtostr(error));
      io_log("Error %u getting size of dataspace\n", rc);
      return rc;
    }

  /* attach it to a given region */
  //if ( (error = l4rm_area_attach_to_region(&ds, area,
    //                   (void *)a, size, 0, rights)) )
  if ( (rc = RegAreaAttachToRegion(addr, size, area,
                                   flags, ds, 0, 0)) )
    {
      io_log("Error %d (%s) attaching dataspace\n", rc);
      return rc;
    }

  return 0;
}
