/*******************************************************/
/* Kernel Abstraction Layer                            */
/*******************************************************/

#include <osfree.h>

// Prototypes
void KalSendToServer(PSZ pszFunctionName, ...);

void KalSendToServer(PSZ pszFunctionName, ...)
{
  printf("Call function %s of OS/2 Personality server\n", pszFunctionName);
  //  Uncommented for testing of compiled dll stubs and to load the dlls in lxloader. */
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

/* Just to make sure this entrypoint is found by lxloader. */
APIRET APIENTRY DosPutMessage2(HFILE hfile, ULONG cbMsg, PCHAR pBuf) {
/* ordinal: MSG.5
  387   DOSCALLS.Dos32PutMessage*/
/*unsigned int APIENTRY DosPutMessage(unsigned int hfile, unsigned int cbMsg, char * pBuf) {*/
        if(pBuf)
                return 0xcafebabe;
        return 0xcafe;
}

/* Two more stubs to keep kal.dll away from importing two entry points from doscalls.dll.
   Which did lead to a loading cycle.
   DOSCALLS.348   Dos32QuerySysInfo
   DOSCALLS.234   Dos32Exit
*/
APIRET APIENTRY DosQuerySysInfo(ULONG iStart, ULONG iLast, PVOID pBuf, ULONG cbBuf) {
}


/*Returns values of static system variables.


#define INCL_DOSMISC
#include <os2.h>

ULONG     iStart;  //  Ordinal of the first system variable to return.
ULONG     iLast;   //  Ordinal of the last system variable to return.
PVOID     pBuf;    //  Address of the data buffer where the system returns the variable values.
ULONG     cbBuf;   //  Length, in bytes, of the data buffer.
APIRET    ulrc;    //  Return Code.

ulrc = DosQuerySysInfo(ULONG iStart, ULONG iLast, PVOID pBuf, ULONG cbBuf);
*/


VOID APIENTRY DosExit(ULONG action, ULONG result) {
}


/*Ends the current thread or process.

DosExit(ULONG action, ULONG result);

#define INCL_DOSPROCESS
#include <os2.h>

ULONG    action;  //  Ends the process and all of its threads.
ULONG    result;  //  Program's completion code.

VOID DosExit(action, result);
*/

