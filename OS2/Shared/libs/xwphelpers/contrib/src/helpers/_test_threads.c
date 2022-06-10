
#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSESMGR
#define INCL_DOSQUEUES
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#pragma hdrstop


#define HF_STDOUT 1

#define LIMIT 2000

typedef struct _THREAD
{
    PCSZ    pcszThreadName;
    ULONG   ulPrtyClass,
            ulPrtyDelta;
    CHAR    szMsg[10];
    TID     tid;
} THREAD, *PTHREAD;

/*
 *@@ Print:
 *      writes the given string to stdout.
 */

VOID Print(PCSZ pcsz)
{
    ULONG cbWritten;
    DosWrite(HF_STDOUT,
             (PVOID)pcsz,
             strlen(pcsz),
             &cbWritten);
}

/*
 *@@ fnThread:
 *      thread func.
 */

VOID APIENTRY fnThread(ULONG ul)
{
    PTHREAD pThread = (PTHREAD)ul;
    Print(pThread->pcszThreadName);
    Print(" started.");
    for (ul = 0;
         ul < LIMIT;
         ++ul)
    {
        Print(pThread->szMsg);
        DosSleep(0);
    }

    Print(pThread->pcszThreadName);
    Print(" ended.");

    pThread->tid = 0;
}

/*
 *@@ StartThread:
 *      starts a thread suspended with the
 *      given priority.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

VOID StartThread(PTHREAD pThread)
{
    DosCreateThread(&pThread->tid,
                    fnThread,
                    (ULONG)pThread,
                    CREATE_SUSPENDED,
                    0x4000);
    DosSetPriority(PRTYS_THREAD,
                   pThread->ulPrtyClass,
                   pThread->ulPrtyDelta,
                   pThread->tid);
}

/*
 *@@ main:
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

int main (int argc, char *argv[])
{
    THREAD  tFS0 =
                {
                    "Foreground server +0",
                    4,  // PRTYC_FOREGROUNDSERVER
                    0,  // delta
                    "f0",
                    0   // tid
                },
            tTC0 =
                {
                    "Time-critical +0",
                    3,  // PRTYC_TIMECRITICAL
                    0,  // delta
                    "t0",
                    0   // tid
                },
            tTC31 =
                {
                    "Time-critical +31",
                    3,  // PRTYC_TIMECRITICAL
                    31,  // delta
                    "t31",
                    0   // tid
                };

    // foreground server thread (class 4)
    StartThread(&tFS0);
    // time-critical thread (class 3)
    StartThread(&tTC0);
    StartThread(&tTC31);

    DosResumeThread(tFS0.tid);
    DosSleep(0);
    DosResumeThread(tTC0.tid);
    DosSleep(0);
    DosResumeThread(tTC31.tid);
    DosSleep(0);

    while (tFS0.tid || tTC0.tid || tTC31.tid)
        DosSleep(0);

    return 0;
}
