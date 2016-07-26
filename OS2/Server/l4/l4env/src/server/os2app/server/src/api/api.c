#include <l4/generic_ts/generic_ts.h>

#include <l4/os3/dl.h>
#include <l4/os3/dataspace.h>

#include "os2app-server.h"


extern l4_uint32_t shared_memory_area;
extern char pszLoadError[260];
extern ULONG rcCode;

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
os2app_app_AttachDataspace_component(CORBA_Object _dice_corba_obj,
                                     l4_addr_t addr /* in */,
                                     const l4dm_dataspace_t *ds /* in */,
                                     l4_uint32_t rights /* in */,
                                     CORBA_Server_Environment *_dice_corba_env)
{
  l4_uint32_t area = shared_memory_area;
  return attach_ds_area(*ds, area, rights, addr);
}
