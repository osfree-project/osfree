//INCL_DOSMISC
#include "incl.hpp"
#include <string.h>
#include <stddef.h>

extern "C" {

APIRET os2APIENTRY DosError(ULONG error) {
        if(error&~(FERR_ENABLEHARDERR|FERR_DISABLEEXCEPTION))
                return 87; //invalid parameter

        UINT em = SetErrorMode(0);

        if((error&0x1)==FERR_DISABLEHARDERR)
                em &= ~SEM_FAILCRITICALERRORS;
        else
                em |= SEM_FAILCRITICALERRORS;
        if((error&0x2)==FERR_ENABLEEXCEPTION)
                em &= ~SEM_NOGPFAULTERRORBOX;
        else
                em |= SEM_NOGPFAULTERRORBOX;
        SetErrorMode(em);

        return 0;
}

static void currentTime(DWORD *low, DWORD *high) {
        SYSTEMTIME st_os2base,st_current;
        st_os2base.wYear = 1970;
        st_os2base.wMonth = 1;
        st_os2base.wDay = 0;
        st_os2base.wHour = 0;
        st_os2base.wMinute = 0;
        st_os2base.wSecond = 0;
        st_os2base.wMilliseconds = 0;
        GetSystemTime(&st_current);
        FILETIME ft_os2base,ft_current;
        SystemTimeToFileTime(&st_os2base,&ft_os2base);
        SystemTimeToFileTime(&st_current,&ft_current);
        FILETIME ft;
        ft.dwLowDateTime = ft_current.dwLowDateTime - ft_os2base.dwLowDateTime;
        ft.dwHighDateTime = ft_current.dwHighDateTime - ft_os2base.dwHighDateTime;
        if(ft_os2base.dwLowDateTime>ft_current.dwLowDateTime)
                ft.dwHighDateTime --;
        *low = ft.dwLowDateTime/100 + ft.dwHighDateTime%100;
        *high = ft.dwHighDateTime/100;
}

APIRET os2APIENTRY DosQuerySysInfo(ULONG iStart,
                                   ULONG iLast,
                                   PVOID pBuf,
                                   ULONG cbBuf)
{
        if(iStart>iLast || pBuf==0)
                return 87; //invalid parameter
        if(iLast>26)
                return 87; //invalid parameter
        if((iLast-iStart+1)*sizeof(ULONG) < cbBuf)
                return 111; //buffer overflow


        ULONG *pul = (ULONG*)pBuf;
        for(ULONG i=iStart; i<=iLast; i++,pul++) {
                switch(i) {
                        case QSV_MAX_PATH_LENGTH:
                                //ascii not unicode is assumed
                                *pul = MAX_PATH;
                                break;
                        case QSV_MAX_TEXT_SESSIONS:
                                //NT has no limits
                                *pul = 256;
                                break;
                        case QSV_MAX_PM_SESSIONS:
                                //no PM support yet
                                *pul = 0;
                                break;
                        case QSV_MAX_VDM_SESSIONS:
                                //NT has no limits
                                *pul = 256;
                                break;
                        case QSV_BOOT_DRIVE:
                                //programs usually needs  to query this to make
                                //a default installation directory or, worse,
                                //the location of config.sys
                                {
                                        char buf[MAX_PATH];
                                        GetWindowsDirectory(buf,MAX_PATH);
                                        if(buf[0]>='A' && buf[0]<'Z')
                                                *pul = buf[0]-'A'+1;
                                        else if(buf[0]>='a' && buf[0]<'z')
                                                *pul = buf[0]-'a'+1;
                                        else
                                                *pul = 3; //C: is a good bet
                                }
                                break;
                        case QSV_DYN_PRI_VARIATION:
                                *pul = 1;
                                break;
                        case QSV_MAX_WAIT:
                                //no way to query this under NT
                                *pul = 3;
                                break;
                        case QSV_MIN_SLICE:
                                *pul = 32;
                                break;
                        case QSV_MAX_SLICE:
                                *pul = 1000;
                                break;
                        case QSV_PAGE_SIZE:
                                {
                                        SYSTEM_INFO si;
                                        GetSystemInfo(&si);
                                        *pul = si.dwPageSize;
                                }
                                break;
                        case QSV_VERSION_MAJOR:
                                {
                                        DWORD dw=GetVersion();
                                        *pul = LOBYTE(LOWORD(dw));
                                }
                                break;
                        case QSV_VERSION_MINOR:
                                {
                                        DWORD dw=GetVersion();
                                        *pul = HIBYTE(LOWORD(dw));
                                }
                                break;
                        case QSV_VERSION_REVISION:
                                *pul = 0;
                                break;
                        case QSV_MS_COUNT:
                                *pul = GetTickCount();
                                break;
                        case QSV_TIME_LOW:
                                {
                                        DWORD low,high;
                                        currentTime(&low,&high);
                                        *pul = low;
                                        if(iLast>=QSV_TIME_HIGH)
                                                i++, pul++, *pul=high;
                                }
                                break;
                        case QSV_TIME_HIGH:
                                {
                                        DWORD low,high;
                                        currentTime(&low,&high);
                                        *pul = high;
                                }
                                break;
                        case QSV_TOTPHYSMEM:
                                {
                                        MEMORYSTATUS ms;
                                        ms.dwLength=sizeof(ms);
                                        GlobalMemoryStatus(&ms);
                                        *pul = ms.dwTotalPhys;
                                }
                                break;
                        case QSV_TOTRESMEM:
                                {
                                        *pul = 1024*1024;
                                }
                                break;
                        case QSV_TOTAVAILMEM:
                                {
                                        MEMORYSTATUS ms;
                                        ms.dwLength=sizeof(ms);
                                        GlobalMemoryStatus(&ms);
                                        *pul = ms.dwAvailVirtual;
                                }
                                break;
                        case QSV_MAXPRMEM:
                                *pul = 1024*1024*1024;
                                break;
                        case QSV_MAXSHMEM:
                                *pul = 64*1024*1024;
                                break;
                        case QSV_TIMER_INTERVAL:
                                *pul = 33;
                                break;
                        case QSV_MAX_COMP_LENGTH:
                                *pul = 256;
                                break;
                        case QSV_FOREGROUND_FS_SESSION:
                                *pul = 1;
                                break;
                        case QSV_FOREGROUND_PROCESS:
                                *pul = 0; //N/A
                                break;
                        case 26: //QSV_NUMPROCESSORS
                                {
                                        SYSTEM_INFO si;
                                        GetSystemInfo(&si);
                                        *pul = si.dwNumberOfProcessors;
                                }
                                break;
                }
        }
        return 0;
}

APIRET os2APIENTRY DosScanEnv(PCSZ pszName,
                              PCSZ *ppszValue)
{
        if(pszName==0 || ppszValue==0)
                return 87; //invalid parameter

        char *p = (char*)GetEnvironmentStrings();
        if(!p)
                return 203; //envvar not found

        int nn=strlen(pszName);
        while(*p) {
                if(strnicmp(pszName,p,nn)==0 && p[nn]=='=') {
                        *ppszValue = (PCSZ)p;
                        return 0;
                }
                while(*p) p++;
                p++;
        }

        return 203; //envvar not found
}

static void makeFull(char *full, const char *dir, const char *filename) {
        strcpy(full,dir);
        if(full[0] && full[strlen(full)-1]!='\\')
                strcat(full,"\\");
        strcat(full,filename);
}

static int anyFileExist(const char *spec) {
        WIN32_FIND_DATA fd;
        HANDLE hdir=FindFirstFile(spec,&fd);
        if(hdir!=INVALID_HANDLE_VALUE) {
                FindClose(hdir);
                return 1;
        } else
                return 0;
}

APIRET os2APIENTRY DosSearchPath(ULONG flag,
                                 PCSZ pszPathOrName,
                                 PCSZ pszFilename,
                                 os2PBYTE pBuf,
                                 ULONG /*cbBuf*/)
{
        char dir[MAX_PATH];
        if(flag&SEARCH_CUR_DIRECTORY) {
                GetCurrentDirectory(MAX_PATH,dir);
                makeFull((char*)pBuf,dir,pszFilename);
                if(anyFileExist((char*)pBuf))
                        return 0;
        }
        if(flag&SEARCH_ENVIRONMENT) {
                if(DosScanEnv(pszPathOrName,&pszPathOrName))
                        return 203;
        }

        while(*pszPathOrName) {
                char *d=dir;
                while(*pszPathOrName && *pszPathOrName!=';')
                        *d++ = *pszPathOrName++;
                *d='\0';
                if(*pszPathOrName) pszPathOrName++;

                makeFull((char*)pBuf,dir,pszFilename);
                if(anyFileExist((char*)pBuf))
                        return 0;
        }

        return 2; //file not found
}


}; //extern "C"

