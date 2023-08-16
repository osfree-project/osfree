/*  PMMERGE.DLL init/term: 
 *  (Load the communication backend)
 */

//when cross-compiling for OS/2 under NT
#undef __NT__
// OS/2 includes
#define  INCL_DOSMISC
#define  INCL_DOSPROCESS
#define  INCL_DOSMODULEMGR
#define  INCL_DOSSEMAPHORES
#include <os2.h>
// libc includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <signal.h>
#include <unistd.h>
#include <process.h>
// FreePM includes
#include "F_def.hpp"
#include "FreePM.hpp"

//#define db_print _db_print_

extern HMTX    FREEPM_hmtx;

extern "C" APIRET APIENTRY (*InitServerConnection)(char *remotemachineName) = 0;
extern "C" APIRET APIENTRY (*CloseServerConnection)(void) = 0;
extern "C" APIRET APIENTRY (*startServerThreads)(void *handl) = 0;
extern "C" APIRET APIENTRY (*F_SendCmdToServer)(ULONG obj, int cmd, int data) = 0;
extern "C" APIRET APIENTRY (*F_SendDataToServer)(ULONG obj, void *data, int len) = 0;
extern "C" APIRET APIENTRY (*F_RecvDataFromServer)(ULONG obj, void *data, int *len, int maxlen) = 0;
extern "C" APIRET APIENTRY (*F_SendGenCmdToServer)(ULONG obj, int cmd, int par) = 0;
extern "C" APIRET APIENTRY (*F_SendGenCmdDataToServer)(ULONG , int cmd, int par, void *data, int datalen) = 0;
extern "C" APIRET APIENTRY (*F_RecvCmdFromClient)(ULONG obj, int *ncmd, int *data) = 0;
extern "C" APIRET APIENTRY (*F_RecvDataFromClient)(ULONG obj, void *sqmsg, int *l, int size) = 0;
extern "C" APIRET APIENTRY (*F_SendDataToClient)(ULONG obj, void *data, int len) = 0;
//extern "C" void APIENTRY   (*fatal)(const char *message) = 0;
//extern "C" void APIENTRY   (*_db_print_)(const char *format,...) = 0;
//extern "C" APIRET FreePM_db_level = 0;
//extern "C" APIRET FreePM_debugLevels[MAX_DEBUG_SECTIONS] = {0};

extern "C" int _FreePM_db_level;
extern "C" int _FreePM_debugLevels[MAX_DEBUG_SECTIONS];

void fatal(const char *message);
void _db_print(const char *format,...);

HMODULE   hmodBE = 0;

extern "C" APIRET APIENTRY __DLLstart_ (HMODULE hmod, ULONG flag);


void showSigString(int s) {
        char * str="";
        switch (s) {
                case SIGABRT:  str="SIGABRT"; break;
                case SIGBREAK: str="SIGBREAK"; break;
                case SIGINT:  str="SIGINT"; break;
                case SIGFPE:  str="SIGFPE"; break;
                case SIGSEGV: str="SIGSEGV"; break;
                case SIGILL:  str="SIGILL"; break;
        }
        printf("Signal: %d = %s\n", s, str);
}


extern "C"  void   FreePM_cleanupHandler(int sig)
 {
    char str[100];
    ULONG Wrote;

    _FreePM_NeedToExit = 1; /* всем остальным ниткам надо срочно сваливать */
    sprintf(str, "\nSignal occurred %x.\n\r", sig);
        showSigString(sig);
    DosWrite(2, (PVOID)str, strlen(str), &Wrote);

    DosSleep(10);
    DosBeep(3000,5);
    DosSleep(10);
    DosBeep(4200,4);
    exit(1);
 }

/* На случай ошибки fp      */
extern "C"  void   FreePM_cleanupHandlerFP(int sig)
{  static int raz=0;
  _fpreset();

  if(++raz  > 1000000)
           _FreePM_NeedToExit = 1; /* всем остальным ниткам надо срочно сваливать */

    //debug(1, 0)("FP Signal occurred %i",sig);
    DosBeep(5000,10);
//    DosWrite(2, (PVOID)str, strlen(str), &Wrote);
//    exit(10);
   if (SIG_ERR == signal(SIGFPE, FreePM_cleanupHandler)) {
       perror("Could not set SIGFPE");
       //debug(1, 0)("Could not set SIGFPE");
       exit(10);
    }
  if(++raz  > 2000000)
       exit(20);

}

int SetupSignals(void)
 {
    if (SIG_ERR == signal(SIGABRT, FreePM_cleanupHandler)) {
       perror("Could not set SIGABRT");
       return EXIT_FAILURE;
    }

    if (SIG_ERR == signal(SIGBREAK, FreePM_cleanupHandler)) {
       perror("Could not set SIGBREAK");
       return EXIT_FAILURE;
    }

    if (SIG_ERR == signal(SIGINT, FreePM_cleanupHandler)) {
       perror("Could not set SIGINT");
       return EXIT_FAILURE;
    }

   if (SIG_ERR == signal(SIGFPE, FreePM_cleanupHandlerFP)) {
       perror("Could not set SIGFPE");
       return EXIT_FAILURE;
    }
    if (SIG_ERR == signal(SIGSEGV, FreePM_cleanupHandler)) {
       perror("Could not set SIGSEGV");
       return EXIT_FAILURE;
    }
    if (SIG_ERR == signal(SIGILL, FreePM_cleanupHandler)) {
       perror("Could not set SIGILL");
       return EXIT_FAILURE;
    }
    return 0;
 }

APIRET APIENTRY init (ULONG flag)
{
  PSZ       pszValue;
  char      LoadError[256];
  APIRET    rc;
  int       idd;
  PTIB      ptib;
  PPIB      ppib;

  DosGetInfoBlocks(&ptib, &ppib);
  ppib->pib_ultype = 2; // VIO

  if (!rc)
    return 0;

  if (flag)
  {
    // terminate
    if (hmodBE) DosFreeModule(hmodBE);
  }
  else
  {
    // init
    rc = DosScanEnv("PM_COMM_BACKEND", &pszValue);
    if (rc) pszValue = "PMPIPE";

    if (!(rc = DosLoadModule(LoadError, sizeof(LoadError), pszValue, &hmodBE)))
      return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 1, 0, (PFN *)&InitServerConnection)))
      return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 2, 0, (PFN *)&CloseServerConnection)))
      return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 3, 0, (PFN *)&startServerThreads)))
      return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 4, 0, (PFN *)&F_SendCmdToServer)))
      return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 5, 0, (PFN *)&F_SendDataToServer)))
      return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 6, 0, (PFN *)&F_RecvDataFromServer)))
      return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 7, 0, (PFN *)&F_SendGenCmdToServer)))
      return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 8, 0, (PFN *)&F_SendGenCmdDataToServer)))
      return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 9, 0, (PFN *)&F_RecvCmdFromClient)))
     return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 10, 0, (PFN *)&F_RecvDataFromClient)))
     return 0;

    if (!(rc = DosQueryProcAddr(hmodBE, 11, 0, (PFN *)&F_SendDataToClient)))
     return 0;
/*
    rc = DosQueryProcAddr(hmodBE, 12, 0, (PFN *)&fatal);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 13, 0, (PFN *)&_db_print_);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 14, 0, (PFN *)&FreePM_db_level);

    if (rc)
      return 0;

    rc = DosQueryProcAddr(hmodBE, 15, 0, (PFN *)&FreePM_debugLevels);

    if (rc)
      return 0;
 */
  }

  return 1;
}
