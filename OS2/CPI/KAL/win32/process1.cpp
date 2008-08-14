//misc thread-stuff
#define INCL_DOSPROCESS
#include "incl.hpp"
#include <string.h>
#include <stddef.h>
#include "hndltbl.hpp"
#include "spinsem.hpp"

extern "C" {

//For unknown reasons DosBeep() is in the INCL_DOSPROCESS section
APIRET os2APIENTRY DosBeep(ULONG /*freq*/,
                           ULONG /*dur*/)
{
        //FixMe: build wave in memory and play it
        return 395; //ERROR_INVALID_FREQUENCY
}


//OS/2 specifies that the first (main) thread has an id of 1
//NT specifies diddly-squat
//This forces us to use a mapping table

struct ThreadReg {
        TID    os2ThreadId;
        HANDLE ntThreadHandle;
        DWORD  ntThreadId;
        TIB tib;
        TIB2 tib2;
        ULONG cbStack;
        ULONG param;
        PFNTHREAD pfn;
};

#define MAXTHREADS 4096
static PIB pib; //process information block
static TID nextTID;  //next TID to use
static tlsi_ThreadReg;
static SpinMutexSemaphore lock;
static struct { TID tid; ThreadReg *r; } regs[MAXTHREADS];

static void registerThread(ThreadReg *r) {
        lock.Request();
        for(int i=r->os2ThreadId%MAXTHREADS; regs[i].tid; i=(i+1)%MAXTHREADS)
                ;
        regs[i].tid = r->os2ThreadId;
        regs[i].r=r;
        lock.Release();
}

static void deregisterThread(TID tid) {
        lock.Request();
        for(int i=tid%MAXTHREADS; regs[i].tid!=tid; i=(i+1)%MAXTHREADS)
                ;
        regs[i].tid=0;
        regs[i].r=0;
        lock.Release();
}

static ThreadReg *findThread(TID tid) {
        lock.Request();
        for(int i=tid%MAXTHREADS,q=0; regs[i].tid!=tid && q<=MAXTHREADS; i=(i+1)%MAXTHREADS,q++)
                ;
        lock.Release();
        if(q<=MAXTHREADS)
                return regs[i].r;
        else
                return 0;
}

void initProcess(unsigned hinst, const char *lpCmdLine) {
        pib.pib_ulpid      = (ULONG)GetCurrentProcessId();
        pib.pib_ulppid     = 0; //parent unknown
        pib.pib_hmte       = (ULONG)hinst;
        pib.pib_pchcmd     = (PCHAR)lpCmdLine;
        pib.pib_pchenv     = (PCHAR)GetEnvironmentStrings();
        pib.pib_flstatus   = 0;
        pib.pib_ultype     = 2; //VIO windowable

        tlsi_ThreadReg = TlsAlloc();
        nextTID = 2;
        lock.Initialize();
        memset(regs,0,sizeof(regs));
}



static DWORD WINAPI threadStartHelper(LPVOID);


VOID os2APIENTRY DosExit(ULONG action,
                         ULONG result)
{
        if(action==EXIT_THREAD)
                ExitThread(result);
        else
                ExitProcess(result);
}

APIRET os2APIENTRY DosCreateThread(PTID ptid,
                                   PFNTHREAD pfn,
                                   ULONG param,
                                   ULONG flag,
                                   ULONG cbStack)
{
        if((!ptid) ||
           (!pfn) ||
           (flag&~3))
                return ERROR_INVALID_PARAMETER; //invalid parameter

        ThreadReg *r = new ThreadReg;
        if(!r)
                return ERROR_NOT_ENOUGH_MEMORY; //not enough memory
        if(cbStack==0)
                cbStack = 4096;
        else
                cbStack = (cbStack+4095)&0xFFFFF000;

        lock.Request();
        r->os2ThreadId = nextTID++;
        lock.Release();
        r->ntThreadHandle = 0; //set on create
        r->ntThreadId = 0;
        r->tib.tib_pexchain = 0; //no exception chain
        r->tib.tib_pstack = 0; //set in thread
        r->tib.tib_pstacklimit = 0; //set in thread
        r->tib.tib_version = 2; //FixMe: verify this value in OS/2
        r->tib.tib_ptib2 = &r->tib2;
        r->tib.tib_ordinal = 0; //not set 
        r->tib2.tib2_ultid = r->os2ThreadId;
        r->tib2.tib2_ulpri = PRTYC_REGULAR;
        r->tib2.tib2_version = 2; //FixMe: verify this value under os/2
        r->tib2.tib2_usMCCount = 0;
        r->tib2.tib2_fMCForceFlag = 0;
        r->cbStack = cbStack;
        r->param = param;
        r->pfn = pfn;

        *ptid = r->os2ThreadId;

        r->ntThreadHandle = CreateThread(NULL,
                                         (DWORD)cbStack,
                                         threadStartHelper,
                                         (LPVOID)r,
                                         flag&1?CREATE_SUSPENDED:0,
                                         &r->ntThreadId
                                        );
        if(r->ntThreadHandle==INVALID_HANDLE_VALUE) {
                delete r;
                return (APIRET)GetLastError();
        } else
                return 0;
}


static DWORD WINAPI threadStartHelper(LPVOID pv) {
        ThreadReg *r=(ThreadReg*)pv;
        TlsSetValue(tlsi_ThreadReg,r);
        r->tib.tib_pstack = (PVOID)&r;
        r->tib.tib_pstacklimit = (PVOID) (((char*)&r)-r->cbStack);
        registerThread(r);
        (*(r->pfn))(r->param);
        deregisterThread(r->os2ThreadId);
        delete r;
        ExitThread(0);
        return 0; //never reached
}


APIRET os2APIENTRY DosResumeThread(TID tid) {
        ThreadReg *r=findThread(tid);
        if(!r)
                return 309; //ERROR_INVALID_THREADID;
        else {
                if(ResumeThread(r->ntThreadHandle)==0xFFFFFFFF)
                        return (APIRET)GetLastError();
                else
                        return 0;
        }
}

APIRET os2APIENTRY DosSuspendThread(TID tid) {
        ThreadReg *r=findThread(tid);
        if(!r)
                return 309; //ERROR_INVALID_THREADID;
        else {
                if(SuspendThread(r->ntThreadHandle)==0xFFFFFFFF)
                        return (APIRET)GetLastError();
                else
                        return 0;
        }
}

APIRET os2APIENTRY DosGetInfoBlocks(PTIB *pptib,
                                    PPIB *pppib)
{
        if(pppib)
                *pppib = &pib;
        if(pptib) {
                ThreadReg *r=(ThreadReg*)TlsGetValue(tlsi_ThreadReg);
                *pptib = &r->tib;
        }
        return 0;
}

/*
APIRET APIENTRY  DosSetPriority(ULONG scope,
                                ULONG ulClass,
                                LONG  delta,
                                ULONG PorTid);
*/

APIRET os2APIENTRY DosKillThread(TID tid) {
        ThreadReg *r=findThread(tid);
        if(!r)
                return 309; //ERROR_INVALID_THREADID;
        else {
                if(TerminateThread(r->ntThreadHandle,0)) {
                        deregisterThread(tid);
                        delete r;
                        return 0;
                } else
                        return (APIRET)GetLastError();
        }
}

APIRET os2APIENTRY DosWaitThread(PTID ptid,
                                 ULONG option)
{
        if(ptid==0)
                return ERROR_INVALID_PARAMETER;
        if(*ptid==1)
                return 309; //ERROR_INVALID_THREADID;
        if(*ptid==0) {
                //wait for any thread
                //build an array of thread handles
                HANDLE hThread[MAXTHREADS];
                TID tid[MAXTHREADS];
                lock.Request();
                DWORD cObjects=0;
                for(int i=0; i<MAXTHREADS; i++) {
                        if(regs[i].r) {
                                hThread[cObjects] = regs[i].r->ntThreadHandle;
                                tid[cObjects] = regs[i].tid;
                                cObjects++;
                        }
                }
                lock.Release();
                DWORD dw=WaitForMultipleObjects(cObjects,
                                                hThread,
                                                FALSE,
                                                option==DCWW_WAIT?INFINITE:0
                                               );
                if(dw==WAIT_TIMEOUT)
                        return 294; //thread not terminated
                else if(/*dw>=WAIT_OBJECT_0 && */dw<WAIT_OBJECT_0+cObjects) {
                        *ptid = tid[dw-WAIT_OBJECT_0];
                        return 0;
                }
                //some sort of error
                return 294; //ERROR_THREAD_NOT_TERMINATED;
        } else {
                //wait for single thread
                lock.Request();
                ThreadReg *r=findThread(*ptid);
                if(!r) {
                        lock.Release();
                        if(*ptid<nextTID)
                                return 0; //TID is valid so it must have terminated
                        else
                                return 309; //ERROR_INVALID_THREADID;
                }
                HANDLE hThread = r->ntThreadHandle;
                lock.Release();
                DWORD dw=WaitForSingleObject(hThread,option==DCWW_WAIT?INFINITE:0);
                if(dw==WAIT_TIMEOUT)                      
                        return 294; //ERROR_THREAD_NOT_TERMINATED;
                else
                        return 0; //in any case the thread has terminated
        }
}


APIRET os2APIENTRY DosSleep(ULONG msec) {
        Sleep(msec);
        return 0;
}

}; //extern "C"
