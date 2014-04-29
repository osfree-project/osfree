#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <process.h>
#include <conio.h>

#define INCL_DOS
#include <os2.h>

#define MAX_OPEN 1


INT main(INT iArgc, PSZ rgArgv[])
{
 struct {
    USHORT iType;
    USHORT cbName;
    CHAR   szName[CCHMAXPATH];
    USHORT cbFSDName;
    CHAR   szFSDName[CCHMAXPATH];
    USHORT cbFSAData;
    CHAR   rgFSAData[CCHMAXPATH];
 }  fsqbf;
USHORT cbBuf;
USHORT rc;

    cbBuf = sizeof(fsqbf);

    rc = DosQFSAttach(rgArgv[1],
                     0,
                     FSAIL_QUERYNAME,
                     (PBYTE)&fsqbf,
                     &cbBuf,
                     0L);
   if (rc)
      printf("Retco = %u\n", rc);
   else
      {
      PSZ pszFS = fsqbf.szName;
      pszFS = pszFS + fsqbf.cbName + 1;
      pszFS = pszFS + 2;
      printf("=%s\n", pszFS);
      }

   return 0;

}

