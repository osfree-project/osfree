/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/types.h>
#include <os3/dataspace.h>
#include <os3/kal.h>

/* local includes */
#include "api.h"


extern l4_uint32_t shared_memory_area;
extern char pszLoadError[260];
extern ULONG rcCode;

extern l4_os3_cap_idx_t execsrv;

extern vmdata_t *areas_list;

void DICE_CV
os2app_app_GetLoadError_component(CORBA_Object _dice_corba_obj,
                                  char **uchLoadError /* out */,
                                  ULONG *cbLoadError  /* out */,
                                  ULONG *retCode /* out */,
                                  CORBA_Server_Environment *_dice_corba_env)
{
  AppGetLoadError(*uchLoadError, cbLoadError, retCode);
}

void DICE_CV
os2app_app_Terminate_component(CORBA_Object _dice_corba_obj,
                               CORBA_Server_Environment *_dice_corba_env)
{
  AppTerminate();
}

long DICE_CV
os2app_app_AddArea_component(CORBA_Object _dice_corba_obj,
                             l4_addr_t addr /* in */,
                             l4_size_t size /* in */,
                             l4_uint32_t flags /* in */,
                             CORBA_Server_Environment *_dice_corba_env)
{
  return AppAddArea(addr, size, flags);
}

long DICE_CV
os2app_app_AttachDataspace_component(CORBA_Object _dice_corba_obj,
                                     l4_addr_t addr /* in */,
                                     const l4dm_dataspace_t *ds /* in */,
                                     l4_uint32_t rights /* in */,
                                     CORBA_Server_Environment *_dice_corba_env)
{
  return AppAttachDataspace(addr, (l4_os3_dataspace_t)ds, rights);
}

long DICE_CV
os2app_app_ReleaseDataspace_component(CORBA_Object _dice_corba_obj,
                                      const l4dm_dataspace_t *ds /* in */,
                                      CORBA_Server_Environment *_dice_corba_env)
{
  return AppReleaseDataspace((l4_os3_dataspace_t)ds);
}
