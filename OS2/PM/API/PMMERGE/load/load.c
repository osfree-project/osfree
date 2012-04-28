#define INCL_DOSERRORS
#define INCL_DOSMODULEMGR
#define INCL_WINLOAD
#include <os2.h>

HLIB APIENTRY Win32LoadLibrary(HAB hab, PCSZ pszLibName)
{
  APIRET rc;
  HMODULE hmod;

  rc=DosLoadModule(NULL, 0, pszLibName, &hmod);

  if (rc!=NO_ERROR) hmod=NULLHANDLE;

  return (HLIB)hmod;
};

PFNWP APIENTRY Win32LoadProcedure(HAB a, HLIB b, PCSZ c)
{
  APIRET rc;
  PFN ModuleAddr;

  rc = DosQueryProcAddr(b,         /* Handle to module           */
                        0,                   /* No ProcName specified      */
                        c,                 /* ProcName (not specified)   */
                        &ModuleAddr);         /* Address returned */

  if (rc!=NO_ERROR) return (PFNWP)NULL;
  return (PFNWP)ModuleAddr;
};

BOOL  APIENTRY Win32DeleteLibrary(HAB a, HLIB hlib)
{
  APIRET rc;

  rc=DosFreeModule(hlib);

  if (rc==NO_ERROR) return TRUE;
  return FALSE;
};

BOOL  APIENTRY Win32DeleteProcedure(HAB a, PFNWP b)
{
  return TRUE;
};
