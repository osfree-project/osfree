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

int main (int argc, char **argv)
{
  HMODULE   hmod;
  #define   NAME_LENGTH  20
  char      pszName[NAME_LENGTH];
  ULONG     flag;
  APIRET    rc = NO_ERROR;

  if (rc = DosLoadModule(pszName, NAME_LENGTH, pszModname, &hmod))
  {
    printf("error loading REXXINIT.DLL (rc=%u)\n", rc);
    return rc;
  }
  
  for (;;)
    DosSleep(100000);

  return rc;
}
