/* os2exec API, l4env-specific */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/ixfmgr.h>
#include <os3/io.h>

/* l4env includes */
#include <l4/env/env.h>
#include <l4/dm_generic/consts.h>

/* share all section dataspaces of a module with a
   given hmod to a given client */
long excShare(unsigned long hmod, void *client_id)
{
  IXFModule *ixf;
  IXFSYSDEP *sysdep;
  l4exec_section_t *section;
  l4_uint32_t rights;
  slist_t *s;
  unsigned long imp_hmod;
  int index;
  int rc;

  ixf = (IXFModule *)hmod;
  sysdep = (IXFSYSDEP *)(ixf->hdlSysDep);
  s = sysdep->seclist;

  while (s)
  {
    section = s->section;
    rights = 0;

    if (section->info.type & L4_DSTYPE_READ)
      rights |= L4DM_READ;

    if (section->info.type & L4_DSTYPE_WRITE)
      rights |= L4DM_WRITE;

    rc = l4dm_share(&section->ds, *(l4_threadid_t *)client_id, rights);

    if (rc)
    {
      io_log("error sharing dataspace %x with task %x.%x\n",
             section->ds, ((l4_threadid_t *)client_id)->id.task,
             ((l4_threadid_t *)client_id)->id.lthread);
      return rc;
    }
    io_log("dataspace %x shared\n", section->ds);
    s = s->next;
  }

  index = 0;
  while (!(rc = ExcGetImp (hmod, &index, &imp_hmod)))
  {
    if (! imp_hmod)
      continue; // KAL, no sections to share

    rc = excShare(imp_hmod, client_id);
    io_log("module %x sections shared\n", imp_hmod);

    if (rc)
      break;
  }

  if (rc == ERROR_MOD_NOT_FOUND)
    return 0;


  return rc;
}
