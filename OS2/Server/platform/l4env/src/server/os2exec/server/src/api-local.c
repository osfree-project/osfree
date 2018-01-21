/* os2exec API, l4env-specific */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/ixfmgr.h>
#include <os3/dataspace.h>
#include <os3/thread.h>
#include <os3/io.h>

/* l4env includes */
//#include <l4/env/env.h>
//#include <l4/dm_generic/consts.h>

/* local includes */
#include "api.h"

long excShare(unsigned long hmod, l4_os3_thread_t client_id);

/* share all section dataspaces of a module with a
   given hmod to a given client */
long excShare(unsigned long hmod, l4_os3_thread_t client_id)
{
  IXFModule *ixf;
  IXFSYSDEP *sysdep;
  l4_os3_section_t *section;
  l4_os3_dataspace_t tmp_ds;
  ULONG rights;
  slist_t *s;
  unsigned long imp_hmod;
  unsigned long index;
  int rc;

  ixf = (IXFModule *)hmod;
  sysdep = (IXFSYSDEP *)(ULONG)(ixf->hdlSysDep);
  s = sysdep->seclist;

  while (s)
  {
    section = s->section;
    rights = 0;

    if (section->type & SECTYPE_READ)
      rights |= L4DM_READ;

    if (section->type & SECTYPE_WRITE)
      rights |= L4DM_WRITE;

    tmp_ds = section->ds;

    //rc = l4dm_share(&section->ds, *(l4_threadid_t *)client_id, rights);
    rc = DataspaceShare(tmp_ds, client_id, rights);

    if (rc)
    {
      //io_log("error sharing dataspace %x with task %x.%x\n",
        //     section->ds, client_id.thread.id.task,
        //     client_id.thread.id.lthread);
      return rc;
    }

    io_log("dataspace %x shared\n", section->ds);
    s = s->next;
  }

  index = 0;
  while (! (rc = ExcGetImp(hmod, &index, &imp_hmod)) )
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
