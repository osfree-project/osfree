/*  Stack switching
 *
 */

#define  INCL_DOSERRORS
#define  INCL_DOSMEMMGR
#define  INCL_DOSPROCESS
#include <os2.h>

#include "config.h"

#include <rexxsaa.h>

#include <stdio.h>

#include "rexxapi.h"
#include "stk.h"

//char me[] = "REXX.DLL";

// REXX.DLL own stack
char *extra_stack = NULL;
// stack save variable
char *old_stack   = NULL;

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

  if (extra_stack)
    return rc;

  rc = DosAllocMem((void **)&extra_stack, 
                   EXTRA_STACK_SIZE, 
                   PAG_READ | PAG_WRITE | 
                   PAG_COMMIT | OBJ_TILE);
  if (rc)
    return rc;

  // set to the end of stack
  extra_stack += EXTRA_STACK_SIZE - 4;

  return rc;
}

APIRET APIENTRY term (void)
{
  if (!extra_stack)
    return 0;

  return DosFreeMem(extra_stack);
}
