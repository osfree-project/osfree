/* OS/2 API includes */
#define INCL_DOSERRORS
#define INCL_MISC
#define INCL_ERRORS
#include <os2.h>

/* osFree includes */
#include <os3/io.h>
#include <os3/ixfmgr.h>

/* Genode includes */
#include <base/stdint.h>
#include <util/string.h>
#include <base/attached_ram_dataspace.h>
#include <dataspace/client.h>
#include <util/reconstructible.h>

/* local includes */
#include <genode_env.h>

using namespace Genode;


extern "C" int
ixfCopyModule(IXFModule *ixfDst, IXFModule *ixfSrc)
{
  Attached_ram_dataspace *_ds;
  Allocator &alloc = genode_alloc();
  Env &env = genode_env();
  size_t _size;
  IXFSYSDEP *sysdepSrc, *sysdepDst;
  l4_os3_section_t *section;
  slist_t *s, *s0, *r;
  void *_addr;
  int  i, rc = NO_ERROR;

  // 1st instance IXFModule structure
  memcpy(ixfDst, ixfSrc, sizeof(IXFModule));

  sysdepSrc = (IXFSYSDEP *)(ixfSrc->hdlSysDep);
  sysdepDst = (IXFSYSDEP *)(ixfDst->hdlSysDep);

  memcpy(sysdepDst, sysdepSrc, sizeof(IXFSYSDEP));

  for (i = 0, r = 0, s0 = sysdepSrc->seclist;
       s0; i++, r = s, s0 = s0->next)
  {
    s = new (alloc) slist_t;

    if (i == 0) // 1st loop iteration
      sysdepDst->seclist = s;
    else
    {
      r->next = s;
      s->next = 0;
    }

    section = new (alloc) l4_os3_section_t;
    s->section = section;

    memcpy(section, s0->section, sizeof(l4_os3_section_t));

    _size = Dataspace_client(*((Dataspace_capability *)s0->section->ds)).size();

    _addr = env.rm().attach(*((Dataspace_capability *)s0->section->ds));

    if (! _addr)
      return 8;

    _ds = new (alloc) Attached_ram_dataspace(env.ram(), env.rm(), _size);
    //_ds.construct(env.ram(), env.rm(), _size);

    if (! _ds)
      return ERROR_NOT_ENOUGH_MEMORY;

    memcpy(_ds->local_addr<char>(), _addr, _size);
    env.rm().detach(_addr);
    //_ds.destruct();
    //destroy(alloc, _ds);

    if (rc)
      io_log("dataspace copy rc=%d\n", rc);
  }

  return rc;
}
