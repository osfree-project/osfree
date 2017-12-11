#ifndef __OS3_IPC_H__
#define __OS3_IPC_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* dice */
#include <dice/dice.h>

#define CORBA_env       CORBA_Environment
#define CORBA_srv_env   CORBA_Server_Environment

#define default_env     dice_default_environment
#define default_srv_env dice_default_server_environment

#define CV              DICE_CV

#ifdef __cplusplus
  }
#endif

#endif
