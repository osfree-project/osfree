/*
 *
 */

#include <os3/dataspace.h>
#include <os3/rm.h>
#include <os3/err.h>
#include <os3/io.h>

#if defined(L4API_l4v2)

#include <l4/sys/types.h>
#include <l4/env/env.h>
#include <l4/dm_mem/dm_mem.h>

long
l4os3_ds_allocate(l4os3_ds_t *ds, l4_addr_t offset, l4_size_t size)
{
  return l4dm_mem_open(l4env_get_default_dsm(), size, 0, 0, "", ds);
}

long
l4os3_ds_size(l4os3_ds_t ds)
{
  l4_size_t size;
  int error;

  if ((error = l4dm_mem_size(&ds, &size)))
    return error;
  else
    return size;
}

#elif defined(L4API_l4f)

// implementation

long
l4os3_ds_allocate(l4os3_ds_t ds, l4_addr_t offset, l4_size_t size)
{
  return l4re_ds_allocate(&ds, offset, size);
}

long
l4os3_ds_size(l4os3_ds_t ds)
{
  return l4re_ds_size(ds);
}

#else
#error "Not implemented!"
#endif

int
attach_ds(l4os3_ds_t *ds, l4_uint32_t flags, l4_addr_t *addr)
{
  int error;
  l4_size_t size;

  if ((size = l4os3_ds_size(*ds)) < 0)
    {
      io_log("Error %d (%s) getting size of dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  //if ((error = l4rm_attach(ds, size, 0, flags, (void **)addr)))
    if ((error = l4os3_rm_attach((void **)addr, size, flags, *ds, 0, 0)))
    {
      io_log("Error %d (%s) attaching dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  return 0;
}

/** attach dataspace to our address space. (concrete address) */
int
attach_ds_reg(l4dm_dataspace_t ds, l4_uint32_t flags, l4_addr_t addr)
{
  int error;
  l4_size_t size;
  l4_addr_t a = addr;

  /* get dataspace size */
  if ((error = l4dm_mem_size(&ds, &size)))
    {
      io_log("Error %d (%s) getting size of dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  /* attach it to a given region */  
  if ((error = l4rm_attach_to_region(&ds, (void *)a, size, 0, flags)))
    {
      io_log("Error %d (%s) attaching dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  return 0;
}

/** attach dataspace to our address space. (concrete address) */
int
attach_ds_area(l4dm_dataspace_t ds, l4_uint32_t area, l4_uint32_t flags, l4_addr_t addr)
{
  int error;
  l4_size_t size;
  l4_addr_t a = addr;

  /* get dataspace size */
  if ((error = l4dm_mem_size(&ds, &size)))
    {
      io_log("Error %d (%s) getting size of dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  /* attach it to a given region */  
  if ( (error = l4rm_area_attach_to_region(&ds, area,
                       (void *)a, size, 0, flags)) )
    {
      io_log("Error %d (%s) attaching dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  return 0;
}
