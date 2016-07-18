#include "os2app-server.h"

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
