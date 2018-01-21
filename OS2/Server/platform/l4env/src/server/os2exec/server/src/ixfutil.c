/* OS/2 API includes */
#define INCL_DOSERRORS
#define INCL_MISC
#define INCL_ERRORS
#include <os2.h>

/* osFree includes */
#include <os3/io.h>
#include <os3/ixfmgr.h>

/* l4env includes */
#include <l4/env/env.h>
#include <l4/dm_generic/consts.h>

int ixfCopyModule(IXFModule *ixfDst, IXFModule *ixfSrc)
{
  l4dm_dataspace_t *_ds;
  l4_size_t _size;
  IXFSYSDEP *sysdepSrc, *sysdepDst;
  l4_os3_section_t *section;
  slist_t *s, *s0, *r;
  void *_addr;
  int  rc, i;

  // 1st instance IXFModule structure
  memmove(ixfDst, ixfSrc, sizeof(IXFModule));

  sysdepSrc = (IXFSYSDEP *)(ixfSrc->hdlSysDep);
  sysdepDst = (IXFSYSDEP *)(ixfDst->hdlSysDep);

  memcpy(sysdepDst, sysdepSrc, sizeof(IXFSYSDEP));

  for (i = 0, r = 0, s0 = sysdepSrc->seclist;
       s0; i++, r = s, s0 = s0->next)
  {
    s = (slist_t *)malloc(sizeof(slist_t));

    if (i == 0) // 1st loop iteration
      sysdepDst->seclist = s;
    else
    {
      r->next = s;
      s->next = 0;
    }

    section = (l4_os3_section_t *)malloc(sizeof(l4_os3_section_t));
    s->section = section;

    memmove(section, s0->section, sizeof(l4_os3_section_t));

    // create Copy-On-Write copy of original dataspace
    rc = l4dm_copy(&s0->section->ds.ds, L4DM_COW, 
                   "os2exec section", &section->ds);

    if (rc)
      io_log("dataspace copy rc=%d\n", rc);
  }

  return 0;
}
