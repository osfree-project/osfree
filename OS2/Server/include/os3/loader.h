/*  Loader/os2exec types and structs
 *
 *
 */

#ifndef __OS2EXEC_LOADER_H__
#define __OS2EXEC_LOADER_H__

#ifdef __cplusplus
  extern "C" {
#endif

#include <os3/thread.h>
#include <os3/processmgr.h>

typedef struct
{
  void          *ip;
  void          *sp;
  void          *sp_limit;
  unsigned long hmod;
  char          exeflag;
  char          path[0x100];
} os2exec_module_t;

int LoaderExec(char *cmd, char *params, char *vc, l4_os3_task_t *task);
int LoaderExecOS2(char *pName, int consoleno, struct t_os2process *proc);

#ifdef __cplusplus
  }
#endif

#endif
