//filehandle-based APIs--------------------------------------------------------
#include "incl.hpp"
#include <string.h>
#include <stddef.h>
#include "hndltbl.hpp"

#include "filemgr0.hpp"

#include "filemgr1.hpp"

HandleTable<ntFILE,5,50> FileTable;

extern "C" {

void initFileTable() {
        FileTable[0] = new ntFILE;
        FileTable[0]->ntFileHandle = GetStdHandle(STD_INPUT_HANDLE);
        FileTable[1] = new ntFILE;
        FileTable[1]->ntFileHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        FileTable[2] = new ntFILE;
        FileTable[2]->ntFileHandle = GetStdHandle(STD_ERROR_HANDLE);
}



APIRET os2APIENTRY DosOpen(PCSZ   pszFileName,
                           PHFILE pHf,
                           PULONG pulAction,
                           ULONG  cbFile,
                           ULONG  ulAttribute,
                           ULONG  fsOpenFlags,
                           ULONG  fsOpenMode,
                           PEAOP2 peaop2)
{
        int idx=FileTable.findAndLockFree();
        if(idx==-1) return 4; //too many open files
        ntFILE *ntFile=new ntFILE;
        if(!ntFile) return 8; //not enough memory
        if(peaop2) return 282; //eas not supported
        char szName[MAX_PATH];
        DWORD fdwAccess;
        DWORD fdwShareMode;
        LPSECURITY_ATTRIBUTES lpsa;
        DWORD fdwCreate;
        DWORD fdwAttrsAndFlags;
        HANDLE hTemplateFile;

        //szName, FixMe: named pipe namespace
        strcpy(szName,pszFileName);
        //fdwAcess
        if((fsOpenMode&0x7)==OPEN_ACCESS_READONLY)
                fdwAccess = GENERIC_READ;
        else if((fsOpenMode&0x7)==OPEN_ACCESS_WRITEONLY)
                fdwAccess = GENERIC_WRITE;
        else if((fsOpenMode&0x7)==OPEN_ACCESS_READWRITE)
                fdwAccess = GENERIC_READ|GENERIC_WRITE;
        else
                return 87; //invalid parameter
        //fdwShareMode
        if((fsOpenMode&0x70)==OPEN_SHARE_DENYREADWRITE)
                fdwShareMode = 0;
        else if((fsOpenMode&0x70)==OPEN_SHARE_DENYWRITE)
                fdwShareMode = FILE_SHARE_READ;
        else if((fsOpenMode&0x70)==OPEN_SHARE_DENYREAD)
                fdwShareMode = FILE_SHARE_WRITE;
        else if((fsOpenMode&0x70)==OPEN_SHARE_DENYNONE)
                fdwShareMode = FILE_SHARE_READ+FILE_SHARE_WRITE;
        else 
                return 87; //invalid parameter
        //lpsa
        SECURITY_ATTRIBUTES sa;
        if((fsOpenMode&OPEN_FLAGS_NOINHERIT))
                lpsa = 0;
        else {
                memset(&sa,0,sizeof(sa));
                sa.nLength = sizeof(sa);
                sa.bInheritHandle = TRUE;
                lpsa = &sa;
        }
        //fdwCreate
        switch(fsOpenFlags&0xff) {
                case OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_FAIL_IF_EXISTS:
                        return 1; //FixMe
                case OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS:
                        fdwCreate = OPEN_EXISTING;
                        break;
                case OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_REPLACE_IF_EXISTS:
                        fdwCreate = TRUNCATE_EXISTING;
                        break;
                case OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_FAIL_IF_EXISTS:
                        fdwCreate = CREATE_NEW;
                        break;
                case OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS:
                        fdwCreate = OPEN_ALWAYS;
                        break;
                case OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_REPLACE_IF_EXISTS:
                        fdwCreate = CREATE_ALWAYS;
                        break;
                default:
                        return 87; //invalid parameter, FixMe: is this legal under os/2?
        }
        //fdwAttrsAndFlags
        fdwAttrsAndFlags = 0;
        if(ulAttribute&FILE_ARCHIVED)
                fdwAttrsAndFlags |= FILE_ATTRIBUTE_ARCHIVE;
        if(ulAttribute&FILE_SYSTEM)
                fdwAttrsAndFlags |= FILE_ATTRIBUTE_SYSTEM;
        if(ulAttribute&FILE_HIDDEN)
                fdwAttrsAndFlags |= FILE_ATTRIBUTE_HIDDEN;
        if(ulAttribute&FILE_READONLY)
                fdwAttrsAndFlags |= FILE_ATTRIBUTE_READONLY;
        if(fsOpenMode&OPEN_FLAGS_WRITE_THROUGH)
                fdwAttrsAndFlags |= FILE_FLAG_WRITE_THROUGH;
        if(fsOpenMode&OPEN_FLAGS_NO_CACHE)
                fdwAttrsAndFlags |= FILE_FLAG_WRITE_THROUGH;
        if((fsOpenMode&0x700)==OPEN_FLAGS_SEQUENTIAL)
                fdwAttrsAndFlags |= FILE_FLAG_SEQUENTIAL_SCAN;
        if((fsOpenMode&0x700)==OPEN_FLAGS_RANDOM)
                fdwAttrsAndFlags |= FILE_FLAG_RANDOM_ACCESS;
        //hTemplateFile
        hTemplateFile = 0;

        HANDLE hf = CreateFile(szName, fdwAccess, fdwShareMode, lpsa, fdwCreate, fdwAttrsAndFlags, hTemplateFile);
        if(hf==INVALID_HANDLE_VALUE)
                return (APIRET)GetLastError();

        //->pulAction
        if(fdwCreate==CREATE_ALWAYS || fdwCreate==OPEN_ALWAYS) {
                if(GetLastError()==183/*ERROR_ALREADY_EXIST*/)
                        *pulAction = fdwCreate==CREATE_ALWAYS?FILE_CREATED:FILE_EXISTED;
                else
                        *pulAction = FILE_CREATED;
        } else {
                *pulAction = FILE_EXISTED;
        }
        if((fsOpenFlags&0x0f)==OPEN_ACTION_REPLACE_IF_EXISTS) {
                //set file size
                SetFilePointer(hf,cbFile,0,0);
                SetEndOfFile(hf); //FixMe
                SetFilePointer(hf,0,0,0);
        }
        *pHf = (os2HFILE)idx;

        ntFile->ntFileHandle = hf;
        ntFile->mode = fsOpenMode;
        FileTable[idx] = ntFile;
        FileTable.unlock(idx);
        
        return 0;
}

APIRET os2APIENTRY DosClose(os2HFILE hFile) {
        int idx=(int)hFile;
        FileTable.lock(idx);
        APIRET rc;
        if(FileTable[idx]) {
                CloseHandle(FileTable[idx]->ntFileHandle);
                delete FileTable[idx];
                FileTable[idx] = 0;
                rc = 0;
        } else {
                rc = 6; //invalid handle
        }
        FileTable.unlock(idx);
        return rc;
}

APIRET os2APIENTRY DosCreatePipe(PHFILE phfRead,
                                 PHFILE phfWrite,
                                 ULONG cb)
{
        if(phfRead==0 || phfWrite==0)
                return 87; //invalid parameter
        
        ntFILE *ntFileR=new ntFILE;
        if(!ntFileR) return 8; //not enough memory
        ntFILE *ntFileW=new ntFILE;
        if(!ntFileW) {
                delete ntFileR;
                return 8; //not enough memory
        }
        int idxR=FileTable.findAndLockFree();
        if(idxR==-1) {
                delete ntFileR;
                delete ntFileW;
                return 4; //too many open files
        }
        int idxW=FileTable.findAndLockFree();
        if(idxW==-1) {
                delete ntFileR;
                delete ntFileW;
                FileTable.unlock(idxR);
                return 4; //too many open files
        }
        
        HANDLE hReadPipe,hWritePipe;
        SECURITY_ATTRIBUTES sa;
        memset(&sa,0,sizeof(sa));
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;       //os/2 pipes are by default inherited, nt pipes isn't
        if(CreatePipe(&hReadPipe,&hWritePipe,&sa,(DWORD)cb)) {
                ntFileR->ntFileHandle = hReadPipe;
                ntFileR->mode = OPEN_ACCESS_READONLY;
                ntFileW->ntFileHandle = hWritePipe;
                ntFileW->mode = OPEN_ACCESS_WRITEONLY;
                FileTable[idxR] = ntFileR;
                FileTable[idxW] = ntFileW;
                FileTable.unlock(idxR);
                FileTable.unlock(idxW);
                *phfRead  = (os2HFILE)idxR;
                *phfWrite = (os2HFILE)idxW;
                return 0;
        } else {
                FileTable.unlock(idxR);
                FileTable.unlock(idxW);
                delete ntFileR;
                delete ntFileW;
                return (APIRET)GetLastError();
        }
}


APIRET os2APIENTRY DosRead(os2HFILE hFile,
                           PVOID pBuffer,
                           ULONG cbRead,
                           PULONG pcbActual)
{
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                HANDLE hf=FileTable[idx]->ntFileHandle;
                FileTable.unlock(idx);
                if(ReadFile(hf,
                            (LPVOID)pBuffer,
                            (DWORD)cbRead,
                            (LPDWORD)pcbActual,
                            0
                           ))
                        return 0;
                else
                        return (APIRET)GetLastError();
        } else {
                FileTable.unlock(idx);
                return 6; //invalid handle
        }
}

APIRET os2APIENTRY DosWrite(os2HFILE hFile,
                            PVOID pBuffer,
                            ULONG cbWrite,
                            PULONG pcbActual)
{
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                HANDLE hf=FileTable[idx]->ntFileHandle;
                FileTable.unlock(idx);
                if(WriteFile(hf,
                            (LPVOID)pBuffer,
                            (DWORD)cbWrite,
                            (LPDWORD)pcbActual,
                            0
                           ))
                        return 0;
                else
                        return (APIRET)GetLastError();
        } else {
                FileTable.unlock(idx);
                return 6; //invalid handle
        }        
}

APIRET os2APIENTRY DosSetFilePtr(os2HFILE hFile,
                                 LONG ib,
                                 ULONG method,
                                 PULONG ibActual)
{
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                FileTable.unlock(idx);
                HANDLE hf=FileTable[idx]->ntFileHandle;
                DWORD dw = SetFilePointer(hf,
                                          (DWORD)ib,
                                          0,
                                          (LONG)method
                                         );
                if(dw==(DWORD)-1)
                        return (APIRET)GetLastError();
                else {
                        *ibActual = (ULONG)dw;
                        return 0;
                }        
        } else {
                FileTable.unlock(idx);
                return 6; //invalid handle
        }        
}

APIRET os2APIENTRY DosSetFileSize(os2HFILE hFile,
                                  ULONG cbSize)
{
        APIRET rc;
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                APIRET rc;
                HANDLE hf=FileTable[idx]->ntFileHandle;
                DWORD current = SetFilePointer(hf,0,0,1);
                SetFilePointer(hf,(DWORD)cbSize,0,0);
                if(SetEndOfFile(hf))
                        rc = 0;
                else
                        rc = (APIRET)GetLastError();
                SetFilePointer(hf,current,0,0);
        } else {
                rc = 6; //invalid handle
        }
        FileTable.unlock(idx);
        return rc;
}

APIRET os2APIENTRY DosResetBuffer(os2HFILE hFile) {
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                HANDLE hf=FileTable[idx]->ntFileHandle;
                FileTable.unlock(idx);
                if(FlushFileBuffers(hf))
                        return 0;
                else
                        return (APIRET)GetLastError();
        } else {
                FileTable.unlock(idx);
                return 6; //invalid handle
        }
}

APIRET os2APIENTRY DosDupHandle(os2HFILE hFile,
                                PHFILE pHfile)
{
        int srcIdx = (int)hFile;
        FileTable.lock(srcIdx);
        if(!FileTable[srcIdx]) {
                FileTable.unlock(srcIdx);
                return 1; //FixMe
        }

        int dstIdx;
        if(*pHfile==(os2HFILE)-1) {
                dstIdx = FileTable.findAndLockFree();
                if(dstIdx==-1) {
                        FileTable.unlock(srcIdx);
                        return 4; //too many open files
                }
        } else {
                dstIdx = (int)*pHfile;
                FileTable.lock(dstIdx);
        }
        if(srcIdx==dstIdx) {
                //no-op
                FileTable.unlock(srcIdx);
                return 0;
        }
        if(FileTable[dstIdx]) {
                CloseHandle(FileTable[dstIdx]->ntFileHandle);
                delete FileTable[dstIdx];
                FileTable[dstIdx] = 0;
        }

        APIRET rc;
        HANDLE target;
        if(DuplicateHandle(GetCurrentProcess(),
                           FileTable[srcIdx]->ntFileHandle,
                           GetCurrentProcess(),
                           &target,
                           0,
                           TRUE,
                           DUPLICATE_SAME_ACCESS
                          ))
        {
                rc = 0;
                FileTable[dstIdx] = new ntFILE;
                FileTable[dstIdx]->ntFileHandle = target;
                FileTable[dstIdx]->mode = FileTable[srcIdx]->mode;
                if(dstIdx==0)
                        SetStdHandle(STD_INPUT_HANDLE,target);
                else if(dstIdx==1)
                        SetStdHandle(STD_OUTPUT_HANDLE,target);
                else if(dstIdx==2)
                        SetStdHandle(STD_ERROR_HANDLE,target);
        } else
                rc = 6; //invalid handle
        FileTable.unlock(srcIdx);
        FileTable.unlock(dstIdx);

        return rc;
}

APIRET os2APIENTRY DosQueryFHState(os2HFILE hFile,
                                   PULONG pMode)
{
        APIRET rc;
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                rc = 0;
                *pMode = FileTable[idx]->mode;
        } else
                rc = 6; //invalid handle
        FileTable.unlock(idx);
        return rc;
}

APIRET os2APIENTRY DosSetFHState(os2HFILE hFile,
                                 ULONG mode)
{
        if(mode&0x077f)
                return 1; //FixMe: invalid flags
        APIRET rc;
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                ULONG oldmode = FileTable[idx]->mode;
                ULONG newmode = (FileTable[idx]->mode&0x077f)|mode;
                if((oldmode&OPEN_FLAGS_NOINHERIT)!=(newmode&OPEN_FLAGS_NOINHERIT)) {
                        //inheritance changed - duplicate and change
                        HANDLE target;
                        if(DuplicateHandle(GetCurrentProcess(),
                                           FileTable[idx]->ntFileHandle,
                                           GetCurrentProcess(),
                                           &target,
                                           0,
                                           newmode&OPEN_FLAGS_NOINHERIT?FALSE:TRUE,
                                           DUPLICATE_SAME_ACCESS
                                          ))
                        {
                                CloseHandle(FileTable[idx]->ntFileHandle);
                                FileTable[idx]->ntFileHandle = target;
                        } else
                                rc = (APIRET)GetLastError();
                } else
                        rc = 0;
                FileTable[idx]->mode = newmode;
        } else
                rc = 6; //invalid handle
        FileTable.unlock(idx);
        return rc;
}

APIRET os2APIENTRY DosQueryHType(os2HFILE hFile,
                                 PULONG pType,
                                 PULONG pAttr)
{
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                HANDLE hf=FileTable[idx]->ntFileHandle;
                FileTable.unlock(idx);
                DWORD t=GetFileType(hf);
                *pType = 0;
                switch(t) {
                        case FILE_TYPE_DISK: *pType = 0; break;
                        case FILE_TYPE_CHAR: *pType = 1; break;
                        case FILE_TYPE_PIPE: *pType = 2; break;
                        case FILE_TYPE_UNKNOWN:
                        default:
                                *pType = 3;
                }
                //We always set the network bit because we cannot retrieve device attributes
                *pType |= 0x80;
                *pAttr = 0;
                return 0;
        } else {
                FileTable.unlock(idx);
                return 6; //invalid handle
        }
}

APIRET os2APIENTRY DosSetMaxFH(ULONG cFH) {
        if(cFH==50)
                return 0;
        else
                return 87; //invalid parameter
}

APIRET os2APIENTRY DosSetRelMaxFH(PLONG /*pcbReqCount*/,
                                  PULONG pcbCurMaxFH)
{
        *pcbCurMaxFH = 50;
        return 0;
}

APIRET os2APIENTRY DosQueryFileInfo(os2HFILE hFile,
                                    ULONG ulInfoLevel,
                                    PVOID pInfo,
                                    ULONG cbInfoBuf)
{
        if(ulInfoLevel!=FIL_STANDARD &&
           ulInfoLevel!=FIL_QUERYEASIZE &&
           ulInfoLevel!=FIL_QUERYEASFROMLIST)
                return 124; //invalid level

        APIRET rc;
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(!FileTable[idx]) {
                rc = 6; //invalid handle
                goto done;
        }
        if(ulInfoLevel==FIL_QUERYEASFROMLIST) {
                rc = 254; //invalid EA name
                goto done;
        }
        BY_HANDLE_FILE_INFORMATION bhfi;
        if(!GetFileInformationByHandle(FileTable[idx]->ntFileHandle,&bhfi)) {
                rc = (APIRET)GetLastError();
                goto done;
        }

        FILESTATUS4 *fs4;
        fs4=(FILESTATUS4 *)pInfo;
        if(cbInfoBuf>offsetof(FILESTATUS4,ftimeCreation))
          ntfiletime2os2(bhfi.ftCreationTime, &fs4->fdateCreation, &fs4->ftimeCreation);
        if(cbInfoBuf>offsetof(FILESTATUS4,ftimeLastAccess))
          ntfiletime2os2(bhfi.ftLastAccessTime, &fs4->fdateLastAccess, &fs4->ftimeLastAccess);
        if(cbInfoBuf>offsetof(FILESTATUS4,ftimeLastWrite))
          ntfiletime2os2(bhfi.ftLastWriteTime, &fs4->fdateLastWrite, &fs4->ftimeLastWrite);
        if(cbInfoBuf>offsetof(FILESTATUS4,cbFile))
          fs4->cbFile = (ULONG)bhfi.nFileSizeLow;
        if(cbInfoBuf>offsetof(FILESTATUS4,cbFileAlloc))
          fs4->cbFileAlloc = (ULONG)bhfi.nFileSizeLow;
        if(cbInfoBuf>offsetof(FILESTATUS4,attrFile))
          ntfileattr2os2(bhfi.dwFileAttributes, &fs4->attrFile);
        if(ulInfoLevel==FIL_QUERYEASIZE)
                fs4->cbList = 0;
done:
        FileTable.unlock(idx);
        return rc;
}

APIRET os2APIENTRY DosSetFileInfo(os2HFILE hFile,
                                  ULONG ulInfoLevel,
                                  PVOID pInfoBuf,
                                  ULONG cbInfoBuf)
{
        if(ulInfoLevel!=FIL_STANDARD &&
           ulInfoLevel!=FIL_QUERYEASIZE)
                return 124; //invalid level

        APIRET rc;
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(!FileTable[idx]) {
                rc = 6; //invalid handle
                goto done;
        }
        if(ulInfoLevel==FIL_STANDARD) {
                FILESTATUS3 *fs3=(FILESTATUS3*)pInfoBuf;
                FILETIME *lpftCreation=0;   FILETIME ftCreation;
                FILETIME *lpftLastAccess=0; FILETIME ftLastAccess;
                FILETIME *lpftLastWrite=0;  FILETIME ftLastWrite;
                if(cbInfoBuf>offsetof(FILESTATUS3,ftimeCreation) &&
                   (*(USHORT*)(&fs3->fdateCreation)!=0 || *(USHORT*)(&fs3->ftimeCreation)!=0))
                {
                  os22ntfiletime(fs3->fdateCreation,fs3->ftimeCreation,&ftCreation);
                  lpftCreation=&ftCreation;
                }
                if(cbInfoBuf>offsetof(FILESTATUS3,ftimeLastAccess) &&
                   (*(USHORT*)(&fs3->fdateLastAccess)!=0 || *(USHORT*)(&fs3->ftimeLastAccess)!=0))
                {
                  os22ntfiletime(fs3->fdateLastAccess,fs3->ftimeCreation,&ftLastAccess);
                  lpftLastAccess=&ftLastAccess;
                }
                if(cbInfoBuf>offsetof(FILESTATUS3,ftimeLastWrite) &&
                   (*(USHORT*)(&fs3->fdateLastWrite)!=0 || *(USHORT*)(&fs3->ftimeLastWrite)!=0))
                {
                  os22ntfiletime(fs3->fdateLastWrite,fs3->ftimeCreation,&ftLastWrite);
                  lpftLastWrite=&ftLastWrite;
                }
                if(!SetFileTime(FileTable[idx]->ntFileHandle,
                                lpftCreation,
                                lpftLastAccess,
                                lpftLastWrite
                               )) {
                        rc = (APIRET)GetLastError();
                        goto done;
                }
                //attributes cannot be changed
                rc = 0;
        }
done:
        FileTable.unlock(idx);
        return rc;
}

APIRET os2APIENTRY DosSetFileLocks(os2HFILE hFile,
                                   PFILELOCK pflUnlock,
                                   PFILELOCK pflLock,
                                   ULONG /*timeout*/,
                                   ULONG flags)
{
        APIRET rc;
        int idx=(int)hFile;
        FileTable.lock(idx);
        if(FileTable[idx]) {
                rc=0;
                if(pflLock) {
                        OVERLAPPED o;
                        memset(&o,0,sizeof(o));
                        o.Offset = pflLock->lOffset;
                        o.OffsetHigh = 0;
                        if(!LockFileEx(FileTable[idx]->ntFileHandle,
                                       flags&1?0:LOCKFILE_EXCLUSIVE_LOCK,
                                       0,
                                       pflLock->lRange, 0,
                                       0
                                    )) {
                                rc = (APIRET)GetLastError();
                        }
                }
                if(rc==0 && pflUnlock) {
                        if(!UnlockFile(FileTable[idx]->ntFileHandle,
                                       pflUnlock->lOffset, 0,
                                       pflUnlock->lRange, 0
                                    )) {
                                UnlockFile(FileTable[idx]->ntFileHandle,
                                           pflLock->lOffset,0,
                                           pflLock->lRange,0
                                          );
                                rc = (APIRET)GetLastError();
                        }
                }
        } else
                rc = 6; //invalid handle
        FileTable.unlock(idx);
        return rc;
}

APIRET os2APIENTRY DosCancelLockRequest(os2HFILE /*hFile*/,
                                        PFILELOCK /*pflLock*/)
{
        return 50; //error_not_supported
}

}; //extern "C"

