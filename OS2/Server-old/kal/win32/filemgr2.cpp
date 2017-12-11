//HDIR-based APIs--------------------------------------------------------
#include "incl.hpp"
#include <string.h>
#include <stddef.h>
#include "hndltbl.hpp"

#include "filemgr0.hpp"


struct ntHDIR {
        HANDLE hdir;
        ULONG flAttribute;
        ULONG ulInfoLevel;
};

static HandleTable<ntHDIR,5,50> DirTable;

static APIRET fillDir(int idx,
                      ntHDIR *ntdir,
                      WIN32_FIND_DATA *firstFD,
                      PVOID pfindbuf,
                      ULONG cbBuf,
                      PULONG pcFilenames
                     );

extern "C" {

APIRET os2APIENTRY DosFindFirst(PCSZ   pszFileSpec,
                                PHDIR  phdir,
                                ULONG  flAttribute,
                                PVOID  pfindbuf,
                                ULONG  cbBuf,
                                PULONG pcFileNames,
                                ULONG  ulInfoLevel)
{
        if(ulInfoLevel!=FIL_STANDARD &&
           ulInfoLevel!=FIL_QUERYEASIZE &&
           ulInfoLevel!=FIL_QUERYEASFROMLIST)
                return 124; //invalid level
        if(ulInfoLevel==FIL_QUERYEASFROMLIST)
                return 254; //invalid ea name
        if(*phdir!=HDIR_SYSTEM && *phdir!=HDIR_CREATE)
                return 87; //invalid parameter
        if(*pcFileNames<1)
                return 87; //invalid parameter
        if(cbBuf<sizeof(FILEFINDBUF3))
                return 111; //buffer overflow

        WIN32_FIND_DATA fd;
        HANDLE hdir = FindFirstFile(pszFileSpec, &fd);
        if(hdir==INVALID_HANDLE_VALUE) {
                *pcFileNames = 0;
                return 18; //error_no_more_files
        }

        int idx=DirTable.findAndLockFree();
        if(idx==-1) {
                FindClose(hdir);
                return 113; //no more search handles
        }

        DirTable[idx] = new ntHDIR;
        if(!DirTable[idx]) {
                DirTable.unlock(idx);
                FindClose(hdir);
                return 113; //no more search handles
        }

        DirTable[idx]->hdir = hdir;
        DirTable[idx]->flAttribute = flAttribute;
        DirTable[idx]->ulInfoLevel = ulInfoLevel;
        *phdir = (HDIR)idx;
        return fillDir(idx,
                       DirTable[idx],
                       &fd,
                       pfindbuf, cbBuf, pcFileNames
                      );
}

APIRET os2APIENTRY DosFindNext(HDIR   hDir,
                               PVOID  pfindbuf,
                               ULONG  cbBuf,
                               PULONG pcFilenames)
{
        if(cbBuf<sizeof(FILEFINDBUF3))
                return 111; //buffer overflow

        int idx=(int)hDir;
        DirTable.lock(idx);
        if(!DirTable[idx]) {
                DirTable.unlock(idx);
                return 6;
        } else
                return fillDir(idx,
                               DirTable[idx],
                               0,
                               pfindbuf, cbBuf, pcFilenames
                              );
}

APIRET os2APIENTRY DosFindClose(HDIR hDir) {
        APIRET rc;
        int idx=(int)hDir;
        DirTable.lock(idx);
        if(DirTable[idx]) {
                FindClose(DirTable[idx]->hdir);
                delete DirTable[idx];
                DirTable[idx] = 0;
                rc = 0;
        } else
                rc = 6;
        DirTable.unlock(idx);
        return rc;
}

}; //extern "C"

static APIRET fillDir(int idx,
                      ntHDIR *ntdir,
                      WIN32_FIND_DATA *firstFD,
                      PVOID pfindbuf,
                      ULONG cbBuf,
                      PULONG pcFilenames
                     )
{
        WIN32_FIND_DATA fd;
        if(firstFD)
                fd=*firstFD;
        else {
                if(!FindNextFile(ntdir->hdir,&fd)) {
                        DirTable.unlock(idx);
                        return 18; //no more files
                }
        }

        APIRET rc=0;
        ULONG maxFilenames=*pcFilenames;
        *pcFilenames=0;

        //find matching file
        FILEFINDBUF4 *prevffb4=0;
        for(;;) {
                int match=1;
                if((fd.dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE)!=0 &&
                   (ntdir->flAttribute&FILE_ARCHIVED)==0)
                        match=0;
                if((fd.dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE)==0 &&
                   (ntdir->flAttribute&MUST_HAVE_ARCHIVED)!=0)
                        match=0;
                if((fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0 &&
                   (ntdir->flAttribute&FILE_DIRECTORY)==0)
                        match=0;
                if((fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0 &&
                   (ntdir->flAttribute&MUST_HAVE_DIRECTORY)!=0)
                        match=0;
                if((fd.dwFileAttributes&FILE_ATTRIBUTE_SYSTEM)!=0 &&
                   (ntdir->flAttribute&FILE_SYSTEM)==0)
                        match=0;
                if((fd.dwFileAttributes&FILE_ATTRIBUTE_SYSTEM)==0 &&
                   (ntdir->flAttribute&MUST_HAVE_SYSTEM)!=0)
                        match=0;
                if((fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)!=0 &&
                   (ntdir->flAttribute&FILE_HIDDEN)==0)
                        match=0;
                if((fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)==0 &&
                   (ntdir->flAttribute&MUST_HAVE_HIDDEN)!=0)
                        match=0;
                if((fd.dwFileAttributes&FILE_ATTRIBUTE_READONLY)!=0 &&
                   (ntdir->flAttribute&FILE_READONLY)==0)
                        match=0;
                if((fd.dwFileAttributes&FILE_ATTRIBUTE_READONLY)==0 &&
                   (ntdir->flAttribute&MUST_HAVE_READONLY)!=0)
                        match=0;
                if(match) {
                        FILEFINDBUF4 *ffb4 = (FILEFINDBUF4*)pfindbuf;
                        ffb4->oNextEntryOffset = 0;
                        ntfiletime2os2(fd.ftCreationTime, &ffb4->fdateCreation, &ffb4->ftimeCreation);
                        ntfiletime2os2(fd.ftLastAccessTime, &ffb4->fdateLastAccess, &ffb4->ftimeLastAccess);
                        ntfiletime2os2(fd.ftLastWriteTime, &ffb4->fdateLastWrite, &ffb4->ftimeLastWrite);
                        ffb4->cbFile = fd.nFileSizeLow;
                        ffb4->cbFileAlloc = fd.nFileSizeLow;
                        ntfileattr2os2(fd.dwFileAttributes,&ffb4->attrFile);
                        ULONG b;
                        if(ntdir->ulInfoLevel==FIL_STANDARD) {
                                ((FILEFINDBUF3*)ffb4)->cchName = (UCHAR)(strlen(fd.cFileName)+1);
                                strcpy(((FILEFINDBUF3*)ffb4)->achName,fd.cFileName);
                                b = offsetof(FILEFINDBUF3,achName) +
                                    ((FILEFINDBUF3*)ffb4)->cchName;
                        } else {
                                ffb4->cbList = 0;
                                ffb4->cchName = (UCHAR)(strlen(fd.cFileName)+1);
                                strcpy(ffb4->achName,fd.cFileName);
                                b  = offsetof(FILEFINDBUF4,achName) +
                                     ffb4->cchName;
                        }
                        if(prevffb4)
                                prevffb4->oNextEntryOffset = (ULONG)(ffb4-prevffb4);
                        prevffb4=ffb4;
                        cbBuf -= b;
                        pfindbuf = (PVOID)(((char*)pfindbuf)+1);
                        ++*pcFilenames;

                        if(cbBuf<sizeof(FILEFINDBUF3) ||
                           *pcFilenames>=maxFilenames) {
                                rc=0;
                                break;
                        }
                } else {
                        if(!FindNextFile(ntdir->hdir,&fd)) {
                                rc = 18; //no more files
                                break;
                        }
                }
        }
        DirTable.unlock(idx);
        return rc;
}
