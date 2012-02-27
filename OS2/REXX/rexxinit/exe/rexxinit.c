/*  REXXINIT.DLL starter
 *  2012, Feb 27
 *  author: valerius
 */

#define  INCL_DOSMODULEMGR
#define  INCL_DOSPROCESS
#define  INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>

char pszModname[] = "REXXINIT";
char pszLoadErr[] = "error loading REXXINIT.DLL (rc=%u)\n";

int main (int argc, char **argv)
{
  HMODULE   hmod;
  char      pszName[20];
  ULONG     flag;
  APIRET    rc = NO_ERROR;

  if (rc = DosLoadModule(pszName, 0x14, pszModname, &hmod))
  {
    printf("error loading REXXINIT.DLL (rc=%u)\n", rc);
    return rc;
  }
  
  for (;;)
    DosSleep(100000);

  return rc;
}
