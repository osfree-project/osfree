/*******************************************************/
/* Kernel Abstraction Layer                            */
/*******************************************************/

#include <osfree.h>

// Prototypes
void KalSendToServer(PSZ pszFunctionName, ...);

void KalSendToServer(PSZ pszFunctionName, ...)
{
  printf("Call function %s of OS/2 Personality server\n", pszFunctionName);
}

// These functions are kernel-depended and implemented in KAL.DLL
APIRET APIENTRY DosClose(HFILE hFile)
{
  KalSendToServer("DosClose");
}

APIRET APIENTRY DosOpen(PCSZ pszName, PHFILE phFile, PULONG pulA, ULONG pulB, ULONG pulC, ULONG pulD, ULONG pulE, PEAOP2 peaop2A)
{
  KalSendToServer("DosOpen");
}


APIRET APIENTRY DosRead(HFILE hFile, PVOID pA, ULONG ulA, PULONG pulA)
{
  KalSendToServer("DosRead");
}

APIRET APIENTRY DosWrite(HFILE hFile, PVOID pA, ULONG ulA, PULONG pulA)
{
  KalSendToServer("DosWrite");
}

APIRET APIENTRY DosFindClose(HDIR hDir)
{
  KalSendToServer("DosFindClose");
}

APIRET APIENTRY DosFindFirst(PCSZ pszMask, PHDIR phDir, ULONG ulA, PVOID pA, ULONG ulB, PULONG pulA, ULONG ulC)
{
  KalSendToServer("DosFindFirst");
}

APIRET APIENTRY DosFindNext(HDIR hDir, PVOID pA, ULONG ulA, PULONG pulA)
{
  KalSendToServer("DosFindNext");
}
