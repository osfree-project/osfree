/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/io.h>

/* Genode includes */
#include <base/allocator.h>
#include <dataspace/client.h>
#include <dataspace/capability.h>
#include <rom_session/connection.h>

/* local includes */
#include <genode_env.h>

using namespace Genode;

static struct vmdata *root = NULL;

struct vmdata
{
  void *addr;
  Rom_connection *rom;
  struct vmdata *next, *prev;
};

extern "C"
int io_load_file(const char *filename, void **addr, unsigned long *size)
{
  struct vmdata *node, *next, *last = NULL;

  if (!filename || !*filename)
    return ERROR_INVALID_PARAMETER;

  if (! addr || ! size)
    return ERROR_INVALID_PARAMETER;

  Allocator &alloc = genode_alloc();
  Env &env = genode_env();
  Rom_connection *rom;
  Dataspace_capability ds;

  try
  {
    rom = new (alloc) Rom_connection(env, filename);
  }
  catch (Rom_connection::Rom_connection_failed)
  {
    return ERROR_FILE_NOT_FOUND;
  }

  try
  {
    ds = rom->dataspace();
    *addr = (void *)(env.rm().attach(ds));
  }
  catch (Region_map::Invalid_dataspace)
  {
    destroy(alloc, rom);
    return ERROR_INVALID_PARAMETER;
  }

  *size = Dataspace_client(ds).size();

  try
  {
    node = new (alloc) vmdata;
  }
  catch (Out_of_ram)
  {
    env.rm().detach(*addr);
    destroy(alloc, rom);
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  /* fill in the new node */
  node->addr = *addr;
  node->rom = rom;
  node->next = NULL;

  /* get the last list node */
  for (next = root; next; next = next->next)
      last = next;

  /* add the node to the list tail */
  if (last)
  {
      last->next = node;
      node->prev = last;
  }
  else
  {
      root = node;
      node->prev = NULL;
  }

  //io_log("size=%lu\n", *size);
  //io_log("addr=%p\n",  *addr);

  return 0;
}

extern "C"
int io_close_file(void *addr)
{
  if (! addr)
    return ERROR_INVALID_PARAMETER;

  Allocator &alloc = genode_alloc();
  Env &env = genode_env();

  struct vmdata *next = NULL;

  /* find the node with the same address */
  for (next = root; next; next = next->next)
  {
      if (next->addr == addr)
          break;
  }

  /* the node is found */
  if (next)
  {
      /* delete it from the list */
      if (next->prev)
          next->prev->next = next->next;
      else
          root = next->next;

      if (next->next)
          next->next->prev = next->prev;

      /* detach dataspace at addr */
      env.rm().detach(addr);

      /* destroy objects */
      destroy(alloc, next->rom);
      destroy(alloc, next);
  }

  return 0;
}
