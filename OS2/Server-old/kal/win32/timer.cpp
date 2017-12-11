#include "incl.hpp"
#include <string.h>
#include <stddef.h>
#include "hndltbl.hpp"

struct Timer {
        HANDLE hThread;
        HSEM hsem;
        ULONG msec;
        int repeat;
};

static HandleTable<Timer,1,50> TimerTable;


static DWORD WINAPI timerThread(LPVOID pv) {
        Timer *t=(Timer*)pv;
        for(;;) {
                Sleep(t->msec);
                DosPostEventSem((HEV)t->hsem);
                if(!t->repeat) break;
        }
        ExitThread(0);
        return 0;
}


static APIRET createTimer(ULONG msec, HSEM hsem, PHTIMER phtimer, int repeat) {
        if(msec==0 || !phtimer)
                return 87; //error_invalid_parameter
        if(hsem==0)
                return 323; //error_ts_semhandle
                
        Timer *t=new Timer;
        if(!t)
                return 324;
        int idx = TimerTable.findAndLockFree();
        if(idx==-1) {
                delete t;
                return 324; //error_ts_notimer
        }

        t->msec = msec;
        t->repeat = repeat;
        t->hsem = hsem;
        
        DWORD tid;
        t->hThread = CreateThread(NULL, 4096, timerThread, (LPVOID)t, 0, &tid);
        if(t->hThread==NULL) {
                TimerTable.unlock(idx);
                delete t;
                return 324; //error_ts_notimer
        }

        TimerTable[idx] = t;
        TimerTable.unlock(idx);

        *phtimer = (HTIMER)idx;
        return 0;
}

extern "C" {

APIRET os2APIENTRY DosAsyncTimer(ULONG msec,
                                 HSEM hsem,
                                 PHTIMER phtimer)
{
        return createTimer(msec,hsem,phtimer,0);
}

APIRET os2APIENTRY DosStartTimer(ULONG msec,
                                 HSEM hsem,
                                 PHTIMER phtimer)
{
        return createTimer(msec,hsem,phtimer,1);
}

APIRET os2APIENTRY DosStopTimer(HTIMER htimer)
{
        int idx=(int)htimer;
        TimerTable.lock(idx);
        Timer *t=TimerTable[idx];
        if(t) {
                TimerTable[idx] = 0;
                TimerTable.unlock(idx);
                t->repeat=0;
                if(WaitForSingleObject(t->hThread, 32)!=WAIT_OBJECT_0)
                        TerminateThread(t->hThread,0);
                CloseHandle(t->hThread);
                delete t;
                return 0;
        } else {
                TimerTable.unlock(idx);
                return 326; //error_ts_handle
        }
}



};

