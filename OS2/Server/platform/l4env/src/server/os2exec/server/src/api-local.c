/* os2exec API, l4env-specific */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* libc includes */
#include <stdlib.h>

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

struct mod_list;

struct mod_list
{
  struct mod_list *next, *prev;
  unsigned long hmod;
};

typedef struct mod_list mod_list_t;

mod_list_t *module_add(mod_list_t *list, unsigned long hmod);
void module_del(mod_list_t *list, unsigned long hmod);
void module_list_free(mod_list_t *list);
BOOL module_present(mod_list_t *list, unsigned long hmod);

mod_list_t *module_add(mod_list_t *list, unsigned long hmod)
{
  mod_list_t *item = (mod_list_t *)malloc(sizeof(mod_list_t));

  if (! item)
  {
    return NULL;
  }

  item->next = list;
  item->prev = NULL;
  item->hmod = hmod;

  if (list)
  {
    list->prev = item;
  }

  return item;
}

void module_del(mod_list_t *list, unsigned long hmod)
{
  mod_list_t *item;

  for (item = list; item; item = item->next)
  {
    if (item->hmod == hmod)
    {
      break;
    }
  }

  if (item)
  {
    if (item->prev)
    {
      item->prev->next = item->next;
    }

    if (item->next)
    {
      item->next->prev = item->prev;
    }
  }
  
  free(item);
}

void module_list_free(mod_list_t *list)
{
  mod_list_t *item, *next;

  for (item = list; item; )
  {
    next = item->next;
    free(item);
    item = next;
  }
}

BOOL module_present(mod_list_t *list, unsigned long hmod)
{
  mod_list_t *item;

  for (item = list; item; item = item->next)
  {
    if (item->hmod == hmod)
    {
      return TRUE;
    }
  }

  return FALSE;
}

long excShare(mod_list_t **traversed_modules,
              unsigned long hmod,
              l4_os3_thread_t client_id);

/* share all section dataspaces of a module with a
   given hmod to a given client */
long excShare(mod_list_t **traversed_modules,
              unsigned long hmod,
              l4_os3_thread_t client_id)
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

  // check if this module already traversed, 
  // so no need to share it again, thus avoiding the dead loop
  if (! module_present(*traversed_modules, hmod))
  {
    *traversed_modules = module_add(*traversed_modules, hmod);
  }
  else
  {
    return 0;
  }

  while (s)
  {
    section = s->section;
    rights = 0;

    io_log("excShare: hmod=%lx\n", hmod);

    if (section->type & SECTYPE_READ)
    {
      io_log("read\n");
      rights |= DATASPACE_READ;
    }

    if (section->type & SECTYPE_WRITE)
    {
      io_log("write\n");
      rights |= DATASPACE_WRITE;
    }

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

    rc = excShare(traversed_modules, imp_hmod, client_id);
    io_log("module %x sections shared\n", imp_hmod);

    if (rc)
      break;
  }

  if (rc == ERROR_MOD_NOT_FOUND)
    return 0;


  return rc;
}
