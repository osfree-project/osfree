//various filename-based APIs
#include "incl.hpp"
#include <string.h>
#include <stddef.h>
#include "filemgr0.hpp"

extern "C" {

APIRET os2APIENTRY DosDelete(PCSZ pszFile)
{
        if(!pszFile) return ERROR_INVALID_PARAMETER;
        if(DeleteFile(pszFile))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosForceDelete(PCSZ pszFile)
{
        if(!pszFile) return ERROR_INVALID_PARAMETER;
        if(DeleteFile(pszFile))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosMove(PCSZ  pszOld,
                           PCSZ  pszNew)
{
        if(pszOld==0 || pszNew==0) return ERROR_INVALID_PARAMETER;
        if(MoveFile(pszOld,pszNew))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosCopy(PCSZ  pszOld,
                           PCSZ  pszNew,
                           ULONG option)
{
        if(pszOld==0 || pszNew==0) return ERROR_INVALID_PARAMETER;
        if(option&DCPY_APPEND) //append is not supported
                return ERROR_INVALID_PARAMETER;
        if(CopyFile(pszOld, pszNew, option&DCPY_EXISTING?FALSE:TRUE))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosEditName(ULONG /*metalevel*/,
                                PCSZ  /*pszSource*/,
                                PCSZ  /*pszEdit*/,
                                os2PBYTE /*pszTarget*/,
                                ULONG /*cbTarget*/)
{
        return 1; //FixMe please!
}

APIRET os2APIENTRY DosCreateDir(PCSZ  pszDirName,
                                PEAOP2 peaop2)
{
        if(pszDirName) return ERROR_INVALID_PARAMETER;
        if(peaop2)
                return ERROR_INVALID_PARAMETER;
        if(CreateDirectory(pszDirName,0))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosDeleteDir(PCSZ  pszDir)
{
        if(pszDir) return ERROR_INVALID_PARAMETER;
        if(RemoveDirectory(pszDir))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosSetDefaultDisk(ULONG disknum)
{
        if(disknum<1 || disknum>26)
                return ERROR_INVALID_DRIVE;
        char dir[MAX_PATH];
        char env[4];
        env[0] ='=';
        env[1] = (char)('A'-1+disknum);
        env[2] = ':';
        env[3] = '\0';
        if(!GetEnvironmentVariable(env,dir,MAX_PATH)) {
                dir[0] = (char)('A'-1+disknum);
                dir[1] = ':';
                dir[2] = '\\';
                dir[3] = '\0';
        }
        if(SetCurrentDirectory(dir)) {
                SetEnvironmentVariable(env,dir);
                return 0;
        } else
                return (APIRET)GetLastError(); //FixMe
}

APIRET os2APIENTRY DosQueryCurrentDisk(PULONG pdisknum,
                                       PULONG plogical)
{
        if(!pdisknum)
                return ERROR_INVALID_PARAMETER; //invalid parameter
 
        char dir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH,dir);
        if(dir[0]>='A' && dir[0]<='Z')
                *pdisknum = dir[0]-'A'+1;
        else if(dir[0]>='a' && dir[0]<='z')
                *pdisknum = dir[0]-'a'+1;
        else //trouble!
                return 1; //FixMe
        if(plogical)
                *plogical = (ULONG)GetLogicalDrives();
        return 0;
}

APIRET os2APIENTRY DosSetCurrentDir(PCSZ pszDir)
{
        if(!pszDir) return ERROR_INVALID_PARAMETER;
        char curdir[MAX_PATH];
        int curdrive;
        GetCurrentDirectory(MAX_PATH,curdir);
        if(curdir[0]>='A' && curdir[0]<='Z' && curdir[1]==':')
                curdrive = curdir[0]-'A'+1;
        else if(curdir[0]>='a' && curdir[0]<='z' && curdir[1]==':')
                curdrive = curdir[0]-'a'+1;
        else
                curdrive = 0;
        int drive;
        if(pszDir[0]>='A' && pszDir[0]<='Z' && pszDir[1]==':')
                drive = pszDir[0]-'A'+1;
        else if(pszDir[0]>='a' && pszDir[0]<='z' && pszDir[1]==':')
                drive = pszDir[0]-'a'+1;
        else
                drive = 0;
        if(drive==0 || drive==curdrive) {
                if(!SetCurrentDirectory(pszDir))
                        return (APIRET)GetLastError();
        }
        if(drive!=0) {
                //set env
                char env[4];
                env[0] = '=';
                env[1] = (char)(drive+'A'-1);
                env[2] = ':';
                env[3] = '\0';
                SetEnvironmentVariable(env,pszDir+2);
        }
        return 0;
}

APIRET os2APIENTRY DosQueryCurrentDir(ULONG disknum,
                                      os2PBYTE pBuf,
                                      PULONG pcbBuf)
{
        GetCurrentDirectory(*pcbBuf,(char*)pBuf);
        if((disknum==0) ||
           (pBuf[0]>='A' && pBuf[0]<='Z' && disknum==pBuf[0]-'A'+1) ||
           (pBuf[0]>='a' && pBuf[0]<='z' && disknum==pBuf[0]-'a'+1))
        {
                *pcbBuf = strlen(pBuf);
                return 0;
        }

        //use env
        char env[4];
        env[0] = '=';
        env[1] = (char)(disknum+'A'+1);
        env[2] = ':';
        env[3] = '\0';
        if(!GetEnvironmentVariable(env,pBuf,*pcbBuf)) {
                pBuf[0] = '\\';
                pBuf[1] = '\0';
        }
        return 0;
}

os2BOOL verify=0;
APIRET os2APIENTRY DosQueryVerify(os2PBOOL pBool) {
        *pBool = verify;
        return 0;
}

APIRET os2APIENTRY DosSetVerify(os2BOOL b) {
        verify=b;
        return 0;
}

APIRET os2APIENTRY DosQueryPathInfo(PCSZ  pszPathName,
                                    ULONG ulInfoLevel,
                                    PVOID pInfoBuf,
                                    ULONG cbInfoBuf)
{
        if(ulInfoLevel!=FIL_STANDARD &&
           ulInfoLevel!=FIL_QUERYEASIZE &&
           ulInfoLevel!=FIL_QUERYEASFROMLIST &&
           ulInfoLevel!=FIL_QUERYFULLNAME)
                return ERROR_INVALID_LEVEL;
        if(ulInfoLevel==FIL_QUERYEASFROMLIST)
                return ERROR_INVALID_EA_NAME;
        if(ulInfoLevel==FIL_QUERYFULLNAME) {
                LPTSTR pszFilePart;
                DWORD chars=GetFullPathName(pszPathName,cbInfoBuf,(LPTSTR)pInfoBuf,&pszFilePart);
                if(chars==0)
                        return 1; //FixMe
                if(chars==cbInfoBuf)
                        return ERROR_BUFFER_OVERFLOW;
                return 0;
        }
        if((ulInfoLevel==FIL_STANDARD && cbInfoBuf<sizeof(FILESTATUS3)) ||
           (ulInfoLevel==FIL_QUERYEASIZE && cbInfoBuf<sizeof(FILESTATUS4)))
                return ERROR_BUFFER_OVERFLOW;

        WIN32_FIND_DATA fd;
        HANDLE hdir=FindFirstFile(pszPathName,&fd);
        if(!hdir)
                return ERROR_PATH_NOT_FOUND;
        //verify that it is the only file
        WIN32_FIND_DATA fd2;
        if(FindNextFile(hdir,&fd2)) {
                FindClose(hdir);
                return ERROR_PATH_NOT_FOUND;
        }
        FindClose(hdir);

        FILESTATUS4 *fs4=(FILESTATUS4*)pInfoBuf;

        if(cbInfoBuf>offsetof(FILESTATUS4,ftimeCreation))
          ntfiletime2os2(fd.ftCreationTime, &fs4->fdateCreation, &fs4->ftimeCreation);
        if(cbInfoBuf>offsetof(FILESTATUS4,ftimeLastAccess))
          ntfiletime2os2(fd.ftLastAccessTime, &fs4->fdateLastAccess, &fs4->ftimeLastAccess);
        if(cbInfoBuf>offsetof(FILESTATUS4,ftimeLastWrite))
          ntfiletime2os2(fd.ftLastWriteTime, &fs4->fdateLastWrite, &fs4->ftimeLastWrite);
        if(cbInfoBuf>offsetof(FILESTATUS4,cbFile))
          fs4->cbFile = (ULONG)fd.nFileSizeLow;
        if(cbInfoBuf>offsetof(FILESTATUS4,cbFileAlloc))
          fs4->cbFileAlloc = (ULONG)fd.nFileSizeLow;
        if(cbInfoBuf>offsetof(FILESTATUS4,attrFile))
          ntfileattr2os2(fd.dwFileAttributes, &fs4->attrFile);
        if(ulInfoLevel==FIL_QUERYEASIZE)
                fs4->cbList = 0;
        return 0;
}

APIRET os2APIENTRY DosSetPathInfo(PCSZ  /*pszPathName*/,
                                  ULONG /*ulInfoLevel*/,
                                  PVOID /*pInfoBuf*/,
                                  ULONG /*cbInfoBuf*/,
                                  ULONG /*flOptions*/)
{
        return 1; //FixMe
}

APIRET os2APIENTRY DosEnumAttribute(ULONG  /*ulRefType*/,
                                    PVOID  /*pvFile*/,
                                    ULONG  /*ulEntry*/,
                                    PVOID  /*pvBuf*/,
                                    ULONG  /*cbBuf*/,
                                    PULONG pulCount,
                                    ULONG  /*ulInfoLevel*/)
{
        //easy: NT files cannot have any EAs
        *pulCount = 0;
        return 0;
}


}; //extern "C"
