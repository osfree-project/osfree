//Process API
#define INCL_DOSPROCESS
#include "incl.hpp"
#include <string.h>
#include <stddef.h>


extern "C" {
        
APIRET os2APIENTRY DosWaitChild(ULONG action,
                                ULONG option,
                                PRESULTCODES pres,
                                PPID ppid,
                                PID pid)
{
        if(action!=DCWA_PROCESS && action!=DCWA_PROCESSTREE)
                return 87;
        if(option!=DCWW_WAIT && option!=DCWW_NOWAIT)
                return 87;
        if(!pres)
                return 87;
        if(!ppid)
                return 87;
        if(pid==0)
                return 128; //error_wait_no_children
        HANDLE hProcess;
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION||SYNCHRONIZE,
                               FALSE,
                               (DWORD)pid
                              );
        if(hProcess==NULL ||  //that's what the doc says
           hProcess==INVALID_HANDLE_VALUE) //...but we dont belive it
                return 303; //error_invalid_procid
        if(option==DCWW_WAIT)
                WaitForSingleObject(hProcess,INFINITE);
        DWORD exitCode;
        BOOL b=GetExitCodeProcess(hProcess,&exitCode);
        APIRET rc;
        if(!b) {
                rc = 129; //error_child_not_complete
                pres->codeTerminate = TC_EXIT;
                pres->codeResult = (ULONG)-1;
        } else {
                rc = 0;
                pres->codeTerminate = TC_EXIT;
                pres->codeResult = exitCode;
        }
        CloseHandle(hProcess);
        return rc;
}



APIRET os2APIENTRY DosExecPgm(PCHAR pObjname,
                              LONG cbObjname,
                              ULONG execFlag,
                              PCSZ  pArg,
                              PCSZ  pEnv,
                              PRESULTCODES pRes,
                              PCSZ  pName)
{
        if(!pRes) return 87;
        if(!pName) return 87;

        
        DWORD dwCreationFlags;
        STARTUPINFO sui;

        dwCreationFlags = 0;
        memset(&sui,0,sizeof(sui));
        PROCESS_INFORMATION pi;
        
        dwCreationFlags |= CREATE_DEFAULT_ERROR_MODE;
        switch(execFlag) {
                case EXEC_SYNC:
                        break;
                case EXEC_ASYNC:
                        break;
                case EXEC_ASYNCRESULT:
                        break;
                case EXEC_TRACE:
                        return 87; //debugging not supported
                case EXEC_BACKGROUND:
                        dwCreationFlags |= DETACHED_PROCESS;
                        break;
                case EXEC_LOAD:
                        dwCreationFlags |= CREATE_SUSPENDED;
                        break;
                case EXEC_ASYNCRESULTDB:
                        return 87; //debugging not supported
                default:
                        return 87;
        }
        BOOL b;
        b = CreateProcess(pName,
                          (LPTSTR)pArg,
                          NULL,
                          NULL,
                          TRUE, //inherit handles
                          dwCreationFlags,
                          (LPVOID)pEnv,
                          NULL, //current dir
                          &sui,
                          &pi
                         );
        if(!b) {
                if(cbObjname>=14)
                        strcpy(pObjname,"not_supported");
                else
                        *pObjname='\0';
                return (APIRET)GetLastError();
        }
        if(execFlag==EXEC_SYNC) {
                //wait for process to terminate
                pRes->codeTerminate = TC_EXIT; //the best we can do
                WaitForSingleObject(pi.hProcess,INFINITE);
                GetExitCodeProcess(pi.hProcess, (LPDWORD)&pRes->codeResult);
        } else {
                pRes->codeTerminate = (ULONG)pi.dwProcessId;
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 0;
}


                             

}; //extern ""C"
