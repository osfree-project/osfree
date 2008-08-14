//mutex semaphores
#define INCL_DOSSEMAPHORES
#include "incl.hpp"
#include <string.h>
#include <stddef.h>

static void ntmtxname(char *d, const char *s) {
        strcpy(d,"os2ev_");
        s+=7; //skip "\sem32\"
        while(*d) d++;
        while(*s) {
                if(*s=='\\')
                        *d++='!';
                else
                        *d++=*s;
                s++;
        }
        *d='\0';
}

extern "C" {

APIRET os2APIENTRY DosCreateMutexSem (PCSZ  pszName,
                                      PHMTX phmtx,
                                      ULONG /*flAttr*/,
                                      os2BOOL fState)
{
        if(!phmtx) return 87; //invalid parameter
        if(fState!=TRUE && fState!=FALSE) return 87; //invalid parameter

        char name[256];
        char *pname=0;
        if(pszName) {
                if(strnicmp(pszName,"\\SEM32\\",7)!=0)
                        return 123; //invalid name
                ntmtxname(name,pszName);
                pname=name;
        } 

        HANDLE h=CreateMutex(NULL, fState, pname);
        if(h==INVALID_HANDLE_VALUE)
                return (APIRET)GetLastError();
        else {
                *phmtx = (HMTX)h;
                return 0;
        }
}

APIRET os2APIENTRY DosOpenMutexSem (PCSZ  pszName,
                                    PHMTX phmtx)
{
        if(pszName) {
                if(*phmtx!=0) //*phev must be 0
                        return 87; //error_invalid_parameter
                if(strnicmp(pszName,"\\sem32\\",7)!=0)
                        return 123; //invalid name
                char name[256];
                ntmtxname(name,pszName);
                HANDLE h=OpenMutex(MUTEX_ALL_ACCESS,FALSE,name);
                if(h==NULL)
                        return 187; //sem not found
                *phmtx = (HEV)h;
                return 0;
        } else {
                //since we cannot determine the source process this is not implemented
                 return 187; //sem not found
        }
}

APIRET os2APIENTRY DosCloseMutexSem (HMTX hmtx) {
        if(CloseHandle((HANDLE)hmtx))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosRequestMutexSem (HMTX hmtx,
                                       ULONG ulTimeout)
{
        DWORD dw=WaitForSingleObject((HANDLE)hmtx, ulTimeout==(ULONG)-1?INFINITE:ulTimeout);
        if(dw==WAIT_OBJECT_0)
                return 0;
        else if(dw==WAIT_TIMEOUT)
                return 640; //timeout
        else if(dw==WAIT_ABANDONED)
                return 105; //sem owner died
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosReleaseMutexSem (HMTX hmtx) {
        if(ReleaseMutex((HANDLE)hmtx))
                return 0;
        else
                return (APIRET)GetLastError();
}


/* Not supported at all:
APIRET APIENTRY DosQueryMutexSem (HMTX hmtx,
                                     PID *ppid,
                                     TID *ptid,
                                     PULONG pulCount)
{
}
*/

}; //extern "C"
