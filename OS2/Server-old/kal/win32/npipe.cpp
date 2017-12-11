//named-pipeAPIs--------------------------------------------------------
//pipe semaphores and unblocking pipes are not supported
#include "incl.hpp"
#include <string.h>
#include <stddef.h>
#include "hndltbl.hpp"

#include "filemgr1.hpp"

extern "C" {

APIRET os2APIENTRY DosCallNPipe(PCSZ  pszName,
                                PVOID pInbuf,
                                ULONG cbIn,
                                PVOID pOutbuf,
                                ULONG cbOut,
                                PULONG pcbActual,
                                ULONG msec)
{
        BOOL b;
        DWORD nTimeOut;
        if(msec==(ULONG)-1)
                nTimeOut = NMPWAIT_WAIT_FOREVER;
        else if(msec==0)
                nTimeOut = NMPWAIT_USE_DEFAULT_WAIT;
        else
                nTimeOut = msec;
        b = CallNamedPipe(pszName, pInbuf, cbIn, pOutbuf, cbOut, (LPDWORD)pcbActual, nTimeOut);
        if(b)
                return 0;
        else
                return (APIRET)GetLastError();
}


APIRET os2APIENTRY DosConnectNPipe(HPIPE hpipe)
{
        APIRET rc;
        int idx=(int)hpipe;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                BOOL b;
                HANDLE h = FileTable[idx]->ntFileHandle;
                FileTable.unlock(idx);
                b = ConnectNamedPipe(h, NULL);
                if(b)
                        rc = 0;
                else
                        rc = (APIRET)GetLastError();
        } else {
                rc = 6; //invalid handle value
                FileTable.unlock(idx);
        }
        return rc;
}


APIRET os2APIENTRY DosDisConnectNPipe(HPIPE hpipe)
{
        APIRET rc;
        int idx=(int)hpipe;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                BOOL b;
                HANDLE h = FileTable[idx]->ntFileHandle;
                FileTable.unlock(idx);
                b = DisconnectNamedPipe(h);
                if(b)
                        rc = 0;
                else
                        rc = (APIRET)GetLastError();
        } else {
                rc = 6; //invalid handle value
                FileTable.unlock(idx);
        }
        return rc;
}


APIRET os2APIENTRY DosCreateNPipe(PCSZ  pszName,
                                  PHPIPE pHpipe,
                                  ULONG openmode,
                                  ULONG pipemode,
                                  ULONG cbInbuf,
                                  ULONG cbOutbuf,
                                  ULONG msec)
{
        char name[512];
        if(!pszName) return 87; //invalid parameter
        if(strnicmp(pszName,"\\pipe\\",6)!=0)
                return 87;
        if(!pszName[6])
                return 87;
        if(!pHpipe)
                return 87;
                
        strcpy(name,"\\\\.\\pipe\\");
        strcat(name,pszName+6);

        DWORD dwOpenMode = 0;
        if(openmode&NP_NOWRITEBEHIND) dwOpenMode |= FILE_FLAG_WRITE_THROUGH;
        if((openmode&0x3) == NP_ACCESS_INBOUND)
                dwOpenMode |= PIPE_ACCESS_INBOUND;
        else if((openmode&0x3) == NP_ACCESS_OUTBOUND)
                dwOpenMode |= PIPE_ACCESS_OUTBOUND;
        else if((openmode&0x3) == NP_ACCESS_DUPLEX)
                dwOpenMode |= PIPE_ACCESS_DUPLEX;
        else
                return 87;

        DWORD dwPipeMode=0;
        if(pipemode&NP_NOWAIT)
                return 8; //nonblocking pipes not support => out of memory
        if((pipemode&0x0c00) == NP_TYPE_BYTE)
                dwPipeMode |= PIPE_TYPE_BYTE;
        else if((pipemode&0x0c00) == NP_TYPE_MESSAGE)
                dwPipeMode |= PIPE_TYPE_MESSAGE;
        else
                return 87;
        if((pipemode&0x0300) == NP_READMODE_BYTE)
                dwPipeMode |= PIPE_READMODE_BYTE;
        else if((pipemode&0x0300) == NP_READMODE_MESSAGE)
                dwPipeMode |= PIPE_READMODE_MESSAGE;
        else
                return 87;
        DWORD nMaxInstances;
        if((dwPipeMode&0xff) == 0xff)
                nMaxInstances = PIPE_UNLIMITED_INSTANCES;
        else if((dwPipeMode&0xff) == 0x00)
                return 87; //invalid parameter
        else
                nMaxInstances = dwPipeMode&0xff;
        DWORD nDefaultTimeout;
        if(msec==0)
                nDefaultTimeout = 50;
        else
                nDefaultTimeout = msec;
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        if(openmode&NP_NOINHERIT)
                sa.bInheritHandle = FALSE;
        else
                sa.bInheritHandle = TRUE;
        
        HANDLE h;
        h = CreateNamedPipe(name,
                            dwOpenMode,
                            dwPipeMode,
                            nMaxInstances,
                            cbInbuf,
                            cbOutbuf,
                            nDefaultTimeout,
                            &sa
                           );                            
        if(h==INVALID_HANDLE_VALUE)
                return (APIRET)GetLastError();

        int idx=FileTable.findAndLockFree();
        if(idx==-1) return 4; //too many open files
        ntFILE *ntFile=new ntFILE;
        if(!ntFile) {
                FileTable.unlock(idx);
                return 8; //not enough memory
        }
        ntFile->ntFileHandle = h;
        ntFile->mode = 0;
        FileTable[idx] = ntFile;
        FileTable.unlock(idx);
        *pHpipe = (HPIPE)idx;
        return 0;
}


APIRET os2APIENTRY DosPeekNPipe(HPIPE hpipe,
                                PVOID pBuf,
                                ULONG cbBuf,
                                PULONG pcbActual,
                                PAVAILDATA pAvail,
                                PULONG pState)
{
        APIRET rc;
        int idx=(int)hpipe;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                BOOL b;
                HANDLE h = FileTable[idx]->ntFileHandle;
                FileTable.unlock(idx);
                DWORD cbAvail,cbMessage;
                b = PeekNamedPipe(h,
                                  pBuf,
                                  cbBuf,
                                  (LPDWORD)pcbActual,
                                  &cbAvail,
                                  &cbMessage
                                 );
                if(b) {
                        rc = 0;
                        pAvail->cbpipe = (USHORT)cbAvail;
                        pAvail->cbmessage = (USHORT)cbMessage;
                        *pState = 0; //we cannot determine the state under NT
                } else
                        rc = (APIRET)GetLastError();
        } else {
                rc = 6; //invalid handle value
                FileTable.unlock(idx);
        }
        return rc;
}


APIRET os2APIENTRY DosQueryNPHState(HPIPE hpipe,
                                    PULONG pState)
{
        if(!pState)
                return 87;
        APIRET rc;
        int idx=(int)hpipe;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                *pState = 0;
                DWORD state,curInstances,maxCollectionCount,collectDataTimeout;
                BOOL b = GetNamedPipeHandleState(FileTable[idx]->ntFileHandle,
                                                 &state,
                                                 &curInstances,
                                                 &maxCollectionCount,
                                                 &collectDataTimeout,
                                                 NULL,0
                                                );
                if(b) {
                        *pState |= NP_WAIT;
                        if(state&PIPE_READMODE_MESSAGE)
                                *pState |= NP_READMODE_MESSAGE;
                        DWORD flags,outBufferSize,inBufferSize,maxInstances;
                        b = GetNamedPipeInfo(FileTable[idx]->ntFileHandle,
                                             &flags,&outBufferSize,
                                             &inBufferSize,
                                             &maxInstances
                                            );
                        if(b) {
                                if(flags&PIPE_SERVER_END)
                                        *pState |= NP_END_SERVER;
                                else
                                        *pState |= NP_END_CLIENT;
                                if(flags&PIPE_TYPE_MESSAGE)
                                        *pState |= NP_TYPE_MESSAGE;
                                else
                                        *pState |= NP_TYPE_BYTE;
                                if(maxInstances == PIPE_UNLIMITED_INSTANCES ||
                                   maxInstances>0xff)
                                        *pState |= 0xff;
                                else
                                        *pState |= maxInstances;
                                rc = 0;
                        } else
                                rc = (APIRET)GetLastError();
                } else
                        rc = (APIRET)GetLastError();
        } else
                rc = 230; //error_bad_pipe
        
        FileTable.unlock(idx);
        return rc;        
}


APIRET os2APIENTRY DosQueryNPipeInfo(HPIPE hpipe,
                                     ULONG infolevel,
                                     PVOID pBuf,
                                     ULONG cbBuf)
{
        if(infolevel!=1)
                return 124; //error_invalid_level;
        if(!pBuf)
                return 87;
        if(cbBuf<sizeof(PIPEINFO))
                return 111; //error_buffer_overflow
        APIRET rc;
        int idx=(int)hpipe;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                DWORD state,curInstances,maxCollectionCount,collectDataTimeout;
                DWORD flags,outBufferSize,inBufferSize,maxInstances;
                if(GetNamedPipeHandleState(FileTable[idx]->ntFileHandle,
                                           &state,
                                           &curInstances,
                                           &maxCollectionCount,
                                           &collectDataTimeout,
                                           NULL,0
                                          )
                   &&
                   GetNamedPipeInfo(FileTable[idx]->ntFileHandle,
                                    &flags,&outBufferSize,
                                    &inBufferSize,
                                    &maxInstances
                                   )
                                   )
                {
                        PIPEINFO *ppi = (PIPEINFO*)pBuf;
                        ppi->cbOut     = (USHORT)outBufferSize;
                        ppi->cbIn      = (USHORT)inBufferSize;
                        ppi->cbMaxInst = (BYTE)maxInstances;
                        ppi->cbCurInst = (BYTE)curInstances;
                        ppi->cbName    = 0;
                        rc = 0;
                } else
                        rc = (APIRET)GetLastError();
                
        } else
                rc = 230; //error_bad_pipe
        FileTable.unlock(idx);
        return rc;        
}


APIRET os2APIENTRY DosQueryNPipeSemState(HSEM /*hsem*/,
                                         PPIPESEMSTATE /*pnpss*/,
                                         ULONG /*cbBuf*/)
{
        return 87; //not supported (yet)
}

/*
APIRET APIENTRY  DosRawReadNPipe(PCSZ  pszName,
                                 ULONG cb,
                                 PULONG pLen,
                                 PVOID pBuf);
APIRET APIENTRY  DosRawWriteNPipe(PCSZ  pszName,
                                  ULONG cb);
*/
APIRET os2APIENTRY DosSetNPHState(HPIPE hpipe,
                                  ULONG state)
{
        if(state&NP_NOWAIT)
                return 87; //not supported
        APIRET rc;
        int idx=(int)hpipe;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                DWORD dwMode = 0;
                if(state&NP_READMODE_MESSAGE)
                        dwMode |= PIPE_READMODE_MESSAGE;
                else
                        dwMode |= PIPE_READMODE_BYTE;
                BOOL b = SetNamedPipeHandleState(FileTable[idx]->ntFileHandle,
                                                 &dwMode,
                                                 NULL,
                                                 NULL
                                                );
                if(b)
                        rc = 0;
                else
                        rc = (APIRET)GetLastError();
                                                 
        } else
                rc = 230; //error_bad_pipe
        FileTable.unlock(idx);
        return rc;
}


APIRET os2APIENTRY DosSetNPipeSem(HPIPE /*hpipe*/,
                                HSEM /*hsem*/,
                                ULONG /*key*/)
{
        return 1;       //error_invalid_function
}


APIRET os2APIENTRY DosTransactNPipe(HPIPE hpipe,
                                    PVOID pOutbuf,
                                    ULONG cbOut,
                                    PVOID pInbuf,
                                    ULONG cbIn,
                                    PULONG pcbRead)
{
        APIRET rc;
        int idx=(int)hpipe;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                BOOL b;
                b = TransactNamedPipe(FileTable[idx]->ntFileHandle,
                                      pOutbuf, cbOut,
                                      pInbuf, cbIn,
                                      (LPDWORD)pcbRead,
                                      NULL
                                     );
                if(b)
                        rc = 0;
                else
                        rc = (APIRET)GetLastError();
        } else
                rc =  6; //invalid handle
        FileTable.unlock(idx);
        return rc;
}


APIRET os2APIENTRY DosWaitNPipe(PCSZ  pszName,
                                ULONG msec)
{
        DWORD nTimeOut;
        if(msec==(ULONG)-1)
                nTimeOut = NMPWAIT_WAIT_FOREVER;
        else if(msec==0)
                nTimeOut = NMPWAIT_USE_DEFAULT_WAIT;
        else
                nTimeOut = msec;
        BOOL b = WaitNamedPipe(pszName, nTimeOut);
        if(b)
                return 0;
        else
                return (APIRET)GetLastError();
}

}; //extern "C"

