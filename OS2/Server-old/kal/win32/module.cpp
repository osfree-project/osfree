#define INCL_DOSMODULEMGR
#include "incl.hpp"
#include <string.h>
#include <stddef.h>

extern "C" {

APIRET os2APIENTRY DosLoadModule(PCSZ  pszName, //error in api: this should have been PSZ
                                 ULONG cbName,
                                 PCSZ  pszModname,
                                 PHMODULE phmod)
{
        *phmod = (os2HMODULE)LoadLibrary(pszModname);
        if(!*phmod) {
                if(pszName) {
                        if(cbName>12)
                                strcpy((PSZ)pszName,"not_emulated");
                        else
                                *(PSZ)pszName = '\0';
                }
                return (APIRET)GetLastError();
        } else
                return 0;
}

APIRET os2APIENTRY DosFreeModule(os2HMODULE hmod)
{
        if(FreeLibrary((HINSTANCE)hmod))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosQueryProcAddr(os2HMODULE hmod,
                                    ULONG ordinal,
                                    PCSZ  pszName,
                                    PFN* ppfn)
{
        LPCSTR lpszProc;
        if(ordinal)
                lpszProc = MAKEINTRESOURCE(ordinal);
        else
                lpszProc = pszName;
        FARPROC fp = GetProcAddress((HMODULE)hmod, lpszProc);
        if(fp) {
                *ppfn = (PFN)fp;
                return 0;
        } else {
                if(GetLastError()==ERROR_INVALID_HANDLE)
                        return ERROR_INVALID_HANDLE;
                if(ordinal)
                        return ERROR_INVALID_ORDINAL;
                else
                        return ERROR_INVALID_NAME;
        }
}

APIRET os2APIENTRY DosQueryModuleHandle(PCSZ  pszModname,
                                        PHMODULE phmod)
{
        *phmod = (os2HMODULE)GetModuleHandle(pszModname);
        if(*phmod)
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosQueryModuleName(os2HMODULE hmod,
                                      ULONG cbName,
                                      PCHAR pch)
{
        if(GetModuleFileName((HINSTANCE)hmod,(LPTSTR)pch,cbName)!=0)
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosQueryProcType(os2HMODULE hmod,
                                    ULONG ordinal,
                                    PCSZ  pszName,
                                    PULONG pulproctype)
{
        LPCSTR lpszProc;
        if(ordinal)
                lpszProc = MAKEINTRESOURCE(ordinal);
        else
                lpszProc = pszName;
        FARPROC fp = GetProcAddress((HINSTANCE)hmod, lpszProc);
        if(fp) {
                *pulproctype = PT_32BIT; //only type supported
                return 0;
        } else {
                if(GetLastError()==ERROR_INVALID_HANDLE)
                        return ERROR_INVALID_HANDLE;
                if(ordinal)
                        return ERROR_INVALID_ORDINAL;
                else
                        return ERROR_INVALID_NAME;
        }
}

};
