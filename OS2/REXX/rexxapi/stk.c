/*  Stack switching
 *
 */

#define  INCL_DOSERRORS
#define  INCL_DOSMEMMGR
#define  INCL_DOSPROCESS
#define  INCL_DOSSEMAPHORES
#include <os2.h>

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <rexxsaa.h>

#include "rexxapi.h"
#include "stk.h"

char *logmtx = "\\SEM32\\REXXAPI\LOGMTX";

HMTX hmtx = NULLHANDLE;

// REXX.DLL own stack
char *extra_stack = NULL;
// stack save variable
char *old_stack   = NULL;

extern int InterpreterIdx;

void LoadInterpreter( void );

APIRET APIENTRY __DLLstart_ (HMODULE hmod, ULONG flag);

APIRET APIENTRY dll_initterm (HMODULE hmod, ULONG flag)
{
  APIRET rc;

  // call C startup first
  rc = __DLLstart_(hmod, flag);

  if (!rc)
    return 0;

  if (flag)
    rc = term();
  else
    rc = init();

  if (rc)
    return 0;

  return 1;
}

APIRET APIENTRY init (void)
{
  APIRET rc = NO_ERROR;

  // open/create log mutex and get its handle
  if ( ( rc = DosOpenMutexSem( logmtx, &hmtx ) ) )
  {
    rc = DosCreateMutexSem( logmtx, &hmtx, DC_SEM_SHARED, FALSE );
  }

  if (rc)
    return rc;
  
  if ( InterpreterIdx == -1 )
    LoadInterpreter();

  if (extra_stack)
    return rc;

  rc = DosAllocMem((void **)&extra_stack, 
                   EXTRA_STACK_SIZE, 
                   PAG_READ | PAG_WRITE | PAG_COMMIT);

  if (rc)
    return rc;

  // set to the end of stack
  extra_stack += EXTRA_STACK_SIZE - 4;

  return rc;
}

APIRET APIENTRY term (void)
{
  DosCloseMutexSem(hmtx);

  if (!extra_stack)
    return 0;

  return DosFreeMem(extra_stack - EXTRA_STACK_SIZE + 4);
}
