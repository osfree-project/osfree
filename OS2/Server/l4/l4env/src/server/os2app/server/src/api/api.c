#include <l4/generic_ts/generic_ts.h>

#include <l4/os3/gcc_os2def.h>
//#include <l4/os3/dl.h>
#include <l4/os3/io.h>
#include <l4/os3/types.h>
#include <l4/os3/dataspace.h>
#include <l4/os3/kal.h>

#include "os2app-server.h"


extern l4_uint32_t shared_memory_area;
extern char pszLoadError[260];
extern ULONG rcCode;

extern l4os3_cap_idx_t execsrv;

extern vmdata_t **pareas_list;

vmdata_t *get_area(l4_addr_t addr)
{
  vmdata_t *ptr;

  for (ptr = *pareas_list; ptr; ptr = ptr->next)
  {
    if (ptr->addr <= addr && addr <= ptr->addr + ptr->size)
      break;
  }

  return ptr;
}

vmdata_t *get_mem_by_name(char *pszName)
{
  vmdata_t *ptr;

  for (ptr = *pareas_list; ptr; ptr = ptr->next)
  {
    if (ptr->name[0] && ! strcmp(ptr->name, pszName))
      break;
  }

  return ptr;
}

void DICE_CV
os2app_app_GetLoadError_component(CORBA_Object _dice_corba_obj,
                                  char **uchLoadError /* out */,
                                  ULONG *cbLoadError  /* out */,
                                  ULONG *retCode /* out */,
                                  CORBA_Server_Environment *_dice_corba_env)
{
  memcpy(*uchLoadError, pszLoadError, sizeof(pszLoadError));
  *cbLoadError = sizeof(pszLoadError);
  *retCode = rcCode;
}

void DICE_CV
os2app_app_Terminate_component(CORBA_Object _dice_corba_obj,
                               CORBA_Server_Environment *_dice_corba_env)
{
  l4ts_exit();
}

long DICE_CV
os2app_app_AddArea_component(CORBA_Object _dice_corba_obj,
                             l4_addr_t addr /* in */,
                             l4_size_t size /* in */,
                             l4_uint32_t flags /* in */,
                             CORBA_Server_Environment *_dice_corba_env)
{
  vmdata_t *ptr;

  ptr = malloc(sizeof(vmdata_t));

  if (! ptr)
    return ERROR_INVALID_ADDRESS;

  io_log("areas_list=%x\n", *pareas_list);
  io_log("addr=%x, size=%x\n", addr, size);

  ptr->is_shared = 1;
  ptr->owner = L4_INVALID_ID;
  ptr->area = shared_memory_area;
  ptr->rights = flags;
  ptr->addr = addr;
  ptr->size = size;
  *(ptr->name) = '\0';
  if (*pareas_list) (*pareas_list)->prev = ptr;
  ptr->next = *pareas_list;
  ptr->prev = 0;
  *pareas_list = ptr;
  return 0;
}

long DICE_CV
os2app_app_AttachDataspace_component(CORBA_Object _dice_corba_obj,
                                     l4_addr_t addr /* in */,
                                     const l4dm_dataspace_t *ds /* in */,
                                     l4_uint32_t rights /* in */,
                                     CORBA_Server_Environment *_dice_corba_env)
{
  l4_uint32_t area = shared_memory_area;
  io_log("*** attach dataspace\n");
  return attach_ds_area(*ds, area, rights, addr);
}

long DICE_CV
os2app_app_ReleaseDataspace_component(CORBA_Object _dice_corba_obj,
                                      const l4dm_dataspace_t *ds /* in */,
                                      CORBA_Server_Environment *_dice_corba_env)
{
  io_log("*** release dataspace\n");
  return l4dm_close(ds);
}
