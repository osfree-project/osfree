//dos queues
#include "incl.hpp"
#include <string.h>
#include <stddef.h>

//Queues are implemented by native named pipes and emulated shared memory
//a HQUEUE (32bit) is really a pointer to a Queue structure or a ClientEnd



#define MAXQUEUES 10
#define MAXELEMENTS 64  /*since max 64 shared objects are allowed by memmgr1.cpp it makes no sence to have more elements*/
struct QueueElement {
        PID putter;
        PVOID base;
        ULONG request;
        ULONG priority;
};

struct Queue {
        DWORD  used;             //0=unused
        char   name[256];       //name of queue (without \\queue prefix)
        HANDLE hmtx_modify;     //mutex for modifying the queue
        HANDLE hev_notempty;    //signalled when not empty
        HANDLE hPipe;
        HANDLE hThread;         //handle of thread reading from named pipe
        DWORD  elements;
        ULONG priority_algorithm;
        HEV    uhev_notempty;
        QueueElement e[MAXELEMENTS];
} queue[MAXQUEUES];

#define MAXCLIENTENDS 40
struct ClientEnd {
        DWORD used;
        char npname[256];
} clientend[MAXCLIENTENDS];

struct qt_parm {
        Queue *q;
        HANDLE started;
};

static void qname2pname(char *d, const char *s) {
        strcpy(d,"\\\\.\\pipe\\os2queue_");
        while(*d) d++;
        while(*s) {
                if(*s=='\\') //backslash not allowed in NT pipe names
                        *d++ = '!';
                else
                        *d++ = *s;
                s++;
        }
        *d = '\0';
}

static DWORD WINAPI queueThread(LPVOID pv) {
        qt_parm *p=(qt_parm*)pv;
        Queue *q=p->q;
        //create named pipe
        char npname[256];
        qname2pname(npname,p->q->name);
        HANDLE hPipe = CreateNamedPipe(npname,
                                       PIPE_ACCESS_DUPLEX,
                                       PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,
                                       1,
                                       sizeof(APIRET),
                                       sizeof(QueueElement)+4,
                                       150,
                                       NULL);
        q->hPipe = hPipe;
        SetEvent(p->started);
        if(hPipe==INVALID_HANDLE_VALUE)
                ExitThread(0);
        
        for(;;) {
                //connect
                BOOL b=ConnectNamedPipe(hPipe,NULL);
                if(!b) break;
                //read element
                struct {
                        DWORD op;
                        QueueElement e;
                } e;
                DWORD bytes;
                b = ReadFile(hPipe, &e, sizeof(e), &bytes, NULL);
                if(!b)
                        goto disconnect; //should never happen
                if(e.op==0) {
                        //query pid of owner (which is us)
                        DWORD pid=GetCurrentProcessId();
                        WriteFile(hPipe, &pid, sizeof(pid), &bytes, NULL);
                        goto disconnect;
                }
                if(e.op==1) {
                        //query number of elements
                        DWORD el=q->elements;
                        WriteFile(hPipe, &el, sizeof(el), &bytes, NULL);
                        goto disconnect;
                }
                APIRET rc;
                WaitForSingleObject(q->hmtx_modify,INFINITE);
                if(q->elements==MAXELEMENTS) {
                        rc=334; //error_que_not_memory
                        WriteFile(hPipe, &rc, sizeof(rc), &bytes, NULL);
                        ReleaseMutex(q->hmtx_modify);
                        goto disconnect;
                }
                //rc=DosGetSharedMem(e.e.base,PAG_READ|PAG_WRITE);
                //if(rc) {
                //        WriteFile(hPipe, &rc, sizeof(rc), &bytes, NULL);
                //        ReleaseMutex(q->hmtx_modify);
                //        goto disconnect;
                //}
                //put into queue
                switch(q->priority_algorithm) {
                        case QUE_FIFO: {
                                q->e[q->elements++] = e.e;
                                break;
                        }
                        case QUE_LIFO: {
                                for(int i=q->elements; i>0; i--)
                                        q->e[i] = q->e[i-1];
                                q->e[0] = e.e;
                                q->elements++;
                                break;
                        }
                        case QUE_PRIORITY: {
                                q->e[q->elements++] = e.e;
                                for(int i=q->elements-1; i>0 && q->e[i].priority>q->e[i-1].priority; i--) {
                                        QueueElement tmp=q->e[i];
                                        q->e[i] = q->e[i-1];
                                        q->e[i-1] = tmp;
                                }
                                break;
                        }
                }
                //signal stuff
                SetEvent(q->hev_notempty);
                if(q->uhev_notempty!=0)
                        DosPostEventSem(q->uhev_notempty);
                rc = 0;
                WriteFile(hPipe, &rc, sizeof(rc), &bytes, NULL);

                ReleaseMutex(q->hmtx_modify);
disconnect:     
                DisconnectNamedPipe(hPipe);
        }
        CloseHandle(hPipe);
        ExitThread(0);
        return 0;
}


APIRET os2APIENTRY DosCreateQueue(PHQUEUE phq,
                                  ULONG priority,
                                  PCSZ  pszName)
{
        if(!phq)
                return 87; //error_invalid_parameter
        if(priority!=QUE_FIFO && priority!=QUE_LIFO && priority!=QUE_PRIORITY)
                return 87; //error_invalid_parameter
        if(!pszName)
                return 87; //error_invalid_parameter
        if(strnicmp(pszName,"\\QUEUES\\",8)!=0)
                return 335; //error_que_invalid_name
        pszName += 8;

        //find a free slot
        int i;
        for(i=0; i<MAXQUEUES; i++) {
                if(InterlockedExchange((LPLONG)&queue[i].used,1)==0)
                        break;
        }
        if(i>=MAXQUEUES)
                return 334; //error_que_no_memory

        Queue *q=queue+i;
        strcpy(q->name,pszName);
        q->hmtx_modify = CreateMutex(NULL, FALSE, NULL);
        q->hev_notempty = CreateEvent(NULL, TRUE, FALSE, NULL);
        q->hPipe = INVALID_HANDLE_VALUE;
        q->elements = 0;
        q->priority_algorithm = priority;
        q->uhev_notempty = 0;

        qt_parm parm;
        parm.q = q;
        parm.started = CreateEvent(NULL,TRUE,FALSE,NULL);
        DWORD tid;
        q->hThread = CreateThread(NULL, 8192, queueThread, (LPVOID)&parm, 0, &tid);
        WaitForSingleObject(parm.started,INFINITE);
        CloseHandle(parm.started);

        if(q->hPipe==INVALID_HANDLE_VALUE) {
                //queue thread could not create named pipe
                WaitForSingleObject(q->hThread,INFINITE);
                CloseHandle(q->hThread);
                CloseHandle(q->hmtx_modify);
                CloseHandle(q->hev_notempty);
                q->used = 0;
                return 332; //error_que_duplicate (most likely)
        }

        *phq = (HQUEUE)q;
        return 0;
}


APIRET os2APIENTRY DosOpenQueue(PPID ppid,
                                PHQUEUE phq,
                                PCSZ  pszName)
{
        if(!ppid || !phq || !pszName)
                return 87;
        if(strnicmp(pszName,"\\QUEUES\\",8)!=0)
                return 335; //error_que_invalid_name

        char npname[256];
        qname2pname(npname,pszName+8);

        {
                DWORD op=0;
                DWORD rc;
                DWORD bytes;
                BOOL b=CallNamedPipe(npname,
                                     &op,
                                     sizeof(op),
                                     &rc,
                                     sizeof(rc),
                                     &bytes,
                                     NMPWAIT_WAIT_FOREVER
                                    );
                if(!b || bytes!=sizeof(rc))
                        return 343; //error_que_name_not_exist
                *ppid = (PID)rc;
        }
        
        for(int i=0; i<MAXCLIENTENDS; i++) {
                if(InterlockedExchange((LPLONG)&clientend[i].used,1)==0)
                        break;
        }
        if(i>=MAXCLIENTENDS)
                return 334; //error_que_no_memory

        strcpy(clientend[i].npname,npname);

        *phq = (HQUEUE)(clientend+i);

        return 0;
}

APIRET os2APIENTRY DosCloseQueue(HQUEUE hq) {
        LPVOID p=(LPVOID)hq;
        if(p>=queue && p<queue+MAXQUEUES) {
                //close server end
                Queue *q=(Queue*)hq;
                if(q->used==0) return 337;

                CloseHandle(q->hPipe);
                WaitForSingleObject(q->hThread,INFINITE);
                CloseHandle(q->hThread);
                CloseHandle(q->hmtx_modify);
                CloseHandle(q->hev_notempty);

                q->used = 0;
                return 0;
        } else if(p>=clientend & p<clientend+MAXCLIENTENDS) {
                ClientEnd *ce=(ClientEnd*)hq;
                if(ce->used==0) return 337;
                ce->used=0;
                return 0;
        } else
                return 337; //errur_que_invalid_handle
}



APIRET os2APIENTRY DosPurgeQueue(HQUEUE hq) {
        Queue *q=(Queue*)hq;
        if(q>=queue && q<queue+MAXQUEUES && q->used==1) {
                WaitForSingleObject(q->hmtx_modify,INFINITE);
                for(int i=0; i<q->elements; i++)
                        if(q->e[i].putter!=GetCurrentProcessId())
                                DosFreeMem(q->e[i].base);
                q->elements = 0;
                ResetEvent(q->hev_notempty);
                if(q->uhev_notempty!=0) {
                        ULONG ul;
                        DosResetEventSem(q->uhev_notempty,&ul);
                }
                ReleaseMutex(q->hmtx_modify);
                return 0;
        } else
                return 337;
}

APIRET os2APIENTRY DosQueryQueue(HQUEUE hq,
                                 PULONG pcbEntries)
{
        if(!pcbEntries)
                return 87;
        Queue *q=(Queue*)hq;
        if(q>=queue && q<queue+MAXQUEUES && q->used==1) {
                WaitForSingleObject(q->hmtx_modify,INFINITE);
                *pcbEntries = q->elements;
                ReleaseMutex(q->hmtx_modify);
                return 0;
        }
        ClientEnd *ce=(ClientEnd*)hq;
        if(ce>=clientend && ce<clientend+MAXCLIENTENDS && ce->used) {
                DWORD op=1;
                DWORD rc;
                DWORD bytes;
                BOOL b=CallNamedPipe(ce->npname,
                                     &op,
                                     sizeof(op),
                                     &rc,
                                     sizeof(rc),
                                     &bytes,
                                     NMPWAIT_WAIT_FOREVER
                                    );
                if(!b || bytes!=sizeof(rc))
                        return 343; //error_que_name_not_exist
                *pcbEntries = (ULONG)rc;
                return 0;
        }
        return 337;
}


APIRET os2APIENTRY DosReadQueue(HQUEUE hq,
                                PREQUESTDATA pRequest,
                                PULONG pcbData,
                                PPVOID ppbuf,
                                ULONG element,
                                BOOL32 wait,
                                PBYTE ppriority,
                                HEV hsem);

APIRET os2APIENTRY DosPeekQueue(HQUEUE hq,
                                PREQUESTDATA pRequest,
                                PULONG pcbData,
                                PPVOID ppbuf,
                                PULONG element,
                                BOOL32 nowait,
                                PBYTE ppriority,
                                HEV hsem);

APIRET os2APIENTRY DosWriteQueue(HQUEUE hq,
                                 ULONG request,
                                 ULONG /*cbData*/,
                                 PVOID pbData,
                                 ULONG priority)
{
        LPVOID p=(LPVOID)hq;
        if(p>=queue && p<queue+MAXQUEUES) {
                Queue *q=(Queue*)p;
                if(!q->used) return 337;
                if(q->elements==MAXELEMENTS) return 334;
                WaitForSingleObject(q->hmtx_modify,INFINITE);
                if(q->elements==MAXELEMENTS) {
                        ReleaseMutex(q->hmtx_modify);
                        return 334;
                }
                QueueElement e;
                e.putter = GetCurrentProcessId();
                e.base = pbData;
                e.request = request;
                e.priority = priority;
                switch(q->priority_algorithm) {
                        case QUE_FIFO: {
                                q->e[q->elements++] = e;
                                break;
                        }
                        case QUE_LIFO: {
                                for(int i=q->elements; i>0; i--)
                                        q->e[i] = q->e[i-1];
                                q->e[0] = e;
                                q->elements++;
                                break;
                        }
                        case QUE_PRIORITY: {
                                q->e[q->elements++] = e;
                                for(int i=q->elements-1; i>0 && q->e[i].priority>q->e[i-1].priority; i--) {
                                        QueueElement tmp=q->e[i];
                                        q->e[i] = q->e[i-1];
                                        q->e[i-1] = tmp;
                                }
                                break;
                        }
                }
                //signal stuff
                SetEvent(q->hev_notempty);
                if(q->uhev_notempty!=0)
                        DosPostEventSem(q->uhev_notempty);
                ReleaseMutex(q->hmtx_modify);
                return 0;
        } else if(p>=clientend && p<clientend+MAXCLIENTENDS) {
                ClientEnd *ce=(ClientEnd*)p;
                if(!ce->used) return 337;
                struct {
                        DWORD op;
                        QueueElement e;
                } e;
                e.op = 2; //put
                e.e.putter = GetCurrentProcessId();
                e.e.base = pbData;
                e.e.request = request;
                e.e.priority = priority>15?15:priority;
                
                DWORD rc;
                DWORD bytes;
                BOOL b=CallNamedPipe(ce->npname,
                                     &e,
                                     sizeof(e),
                                     &rc,
                                     sizeof(rc),
                                     &bytes,
                                     NMPWAIT_WAIT_FOREVER
                                    );
                if(!b || bytes!=sizeof(rc))
                        return 337;
                else
                        return 0;
        } else
                return 337;
}
