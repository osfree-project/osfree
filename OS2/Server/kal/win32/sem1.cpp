//event semaphores
#define INCL_DOSSEMAPHORES
#include "incl.hpp"
#include <string.h>
#include <stddef.h>

static void ntevname(char *d, const char *s) {
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

APIRET os2APIENTRY DosCreateEventSem (PCSZ  pszName,
                                      PHEV phev,
                                      ULONG /*flAttr*/,
                                      os2BOOL fState)
{
        if(!phev) return 87; //invalid parameter
        if(fState!=TRUE && fState!=FALSE) return 87; //invalid parameter
        char name[256];
        char *pname=0;
        if(pszName) {
                if(strnicmp(pszName,"\\SEM32\\",7)!=0)
                        return 123; //invalid name
                ntevname(name,pszName);
                pname=name;
        } 
        
        HANDLE h = CreateEvent(NULL, //no security
                               TRUE, //manual reset
                               fState,
                               pname
                              );
        if(h==INVALID_HANDLE_VALUE)
                return (APIRET)GetLastError();
        else {
                *phev = (HEV)h;
                return 0;
        }
}

APIRET os2APIENTRY DosOpenEventSem (PCSZ pszName,
                                    PHEV phev)
{
        if(pszName) {
                if(*phev!=0) //*phev must be 0
                        return 87; //error_invalid_parameter
                if(strnicmp(pszName,"\\sem32\\",7)!=0)
                        return 123; //invalid name
                char name[256];
                ntevname(name,pszName);
                HANDLE h=OpenEvent(EVENT_ALL_ACCESS,FALSE,name);
                if(h==NULL)
                        return 187; //sem not found
                *phev = (HEV)h;
                return 0;
        } else {
                //since we cannot determine the source process this is not implemented
                 return 187; //sem not found
        }
}

APIRET os2APIENTRY DosCloseEventSem (HEV hev)
{
        if(CloseHandle((HANDLE)hev))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosResetEventSem (HEV hev,
                                     PULONG pulPostCt)
{
        DWORD dw=WaitForSingleObject((HANDLE)hev,0);
        if(dw==WAIT_TIMEOUT)
                *pulPostCt = 0;
        else if(dw==WAIT_OBJECT_0)
                *pulPostCt = 1;
        else
                return (APIRET)GetLastError();
        if(ResetEvent((HANDLE)hev))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosPostEventSem (HEV hev)
{
        if(SetEvent((HANDLE)hev)) {
                return 0;
        } else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosWaitEventSem (HEV hev,
                                    ULONG ulTimeout)
{
        DWORD dw=WaitForSingleObject((HANDLE)hev, ulTimeout==(ULONG)-1?INFINITE:ulTimeout);
        if(dw==WAIT_TIMEOUT)
                return 640; //timeout
        else if(dw==WAIT_OBJECT_0)
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosQueryEventSem (HEV hev,
                                     PULONG pulPostCt)
{
        DWORD dw=WaitForSingleObject((HANDLE)hev, 0);
        if(dw==WAIT_TIMEOUT) {
                *pulPostCt = 0;
                return 0;
        } else if(dw==WAIT_OBJECT_0) {
                *pulPostCt = 1;
                return 0;
        } else
                return (APIRET)GetLastError();
}

}; //extern "C"

