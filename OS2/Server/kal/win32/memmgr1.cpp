//memory management APIs
#include "incl.hpp"
#include <string.h>
#include <stddef.h>

static isSharedMem(PVOID pv);
static APIRET dosFreeSharedMem(PVOID pb);

static DWORD os2protect2nt(ULONG flag) {
        switch(flag&(PAG_EXECUTE|PAG_READ|PAG_WRITE|PAG_GUARD)) {
                case PAG_EXECUTE:
                        return PAGE_EXECUTE;
                case PAG_READ:
                        return PAGE_READONLY;
                case PAG_EXECUTE|PAG_READ:
                        return PAGE_EXECUTE_READ;
                case PAG_WRITE:
                        return PAGE_READWRITE;
                case PAG_EXECUTE|PAG_WRITE:
                        return PAGE_EXECUTE_READWRITE;
                case PAG_READ|PAG_WRITE:
                        return PAGE_READWRITE;
                case PAG_EXECUTE|PAG_READ|PAG_WRITE:
                        return PAGE_EXECUTE_READWRITE;
                case PAG_GUARD:
                        return PAGE_GUARD;
                case PAG_EXECUTE|PAG_GUARD:
                        return PAGE_GUARD;
                case PAG_READ|PAG_GUARD:
                        return PAGE_GUARD;
                case PAG_EXECUTE|PAG_READ|PAG_GUARD:
                        return PAGE_GUARD;
                case PAG_WRITE|PAG_GUARD:
                        return PAGE_GUARD;
                case PAG_EXECUTE|PAG_WRITE|PAG_GUARD:
                        return PAGE_GUARD;
                case PAG_READ|PAG_WRITE|PAG_GUARD:
                        return PAGE_GUARD;
                case PAG_EXECUTE|PAG_READ|PAG_WRITE|PAG_GUARD:
                        return PAGE_GUARD;
                case 0:
                default:
                        return PAGE_NOACCESS;

        }
};

static ULONG ntprotect2os2(DWORD protect) {
        switch(protect) {
                case PAGE_READONLY: return PAG_READ;
                case PAGE_READWRITE: return PAG_READ|PAG_WRITE;
                case PAGE_WRITECOPY: return PAG_WRITE;
                case PAGE_EXECUTE: return PAG_EXECUTE;
                case PAGE_EXECUTE_READ: return PAG_EXECUTE|PAG_READ;
                case PAGE_EXECUTE_READWRITE: return PAG_EXECUTE|PAG_READ|PAG_WRITE;
                case PAGE_EXECUTE_WRITECOPY: return PAG_EXECUTE|PAG_WRITE;
                case PAGE_GUARD: return PAG_GUARD;
                case PAGE_NOACCESS: return 0;
                default:
                        return 0;
        }
}

extern "C" {

APIRET os2APIENTRY DosAllocMem(PPVOID ppb,
                               ULONG cb,
                               ULONG flag)
{
        DWORD fdwAllocationType;
        DWORD fdwProtect;
        if(flag&OBJ_TILE)
                return 8; //not enough memory
        if(flag&PAG_COMMIT) {
                fdwAllocationType = MEM_COMMIT;
                fdwProtect = os2protect2nt(flag);
        } else {
                fdwAllocationType = MEM_RESERVE;
                fdwProtect = PAGE_NOACCESS;
        }

        *ppb = (PVOID)VirtualAlloc(NULL, cb, fdwAllocationType, fdwProtect);
        if(*ppb)
                return 0;
        else
                return (APIRET)GetLastError();
}


APIRET os2APIENTRY DosFreeMem(PVOID pb)
{
        if(isSharedMem(pb))
                return dosFreeSharedMem(pb);
        if(VirtualFree(pb,0,MEM_RELEASE))
                return 0;
        else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosSetMem(PVOID pb,
                             ULONG cb,
                             ULONG flag)
{
        if(flag&PAG_COMMIT) {
                //FixMe: keep/remember protection
                if(!VirtualAlloc(pb,cb,MEM_COMMIT,PAGE_READWRITE))
                        return (APIRET)GetLastError();
        } else if(flag&PAG_DECOMMIT) {
                if(!VirtualFree(pb,cb,MEM_DECOMMIT))
                        return (APIRET)GetLastError();
        }
        if(flag&(0x407)!=PAG_DEFAULT) {
                DWORD oldProtect;
                DWORD fdwProtect = os2protect2nt(flag);
                if(!VirtualProtect(pb,cb,fdwProtect,&oldProtect))
                        return (APIRET)GetLastError();
        }
        return 0;
}

APIRET os2APIENTRY DosQueryMem(PVOID pb,
                               PULONG pcb,
                               PULONG pFlag)
{
        MEMORY_BASIC_INFORMATION mbi;
        DWORD ib = VirtualQuery(pb,&mbi,sizeof(mbi));
        if(ib==0)
                return 487; //invalid address

        *pcb = mbi.RegionSize;
        if(mbi.State==MEM_FREE)
                *pFlag = PAG_FREE;
        else if(mbi.State==MEM_RESERVE)
                *pFlag = 0; //FixMe?
        else {
                *pFlag = ntprotect2os2(mbi.AllocationProtect);
                if(isSharedMem(pb))
                        *pFlag |= PAG_SHARED;
        }
        return 0;
}


//Shared memory----------------------------------------------------------------
//NT does not directly have a shared memory concept
//Instead we use the file mapping concept
//Shared memory is limited to 64MB
//Names of file mappings are os2sharemem_filemapping_<base address>

//This (management) pool is allocated and initialized by the first process that
//makes calls to the shared memory API.
//The pool is located in a file mapping object "os2sharemem_pool" and protected
//by a mutex "os2sharemem_mutex"
//The first process also reserves a memory range
//Each process have a thread that handles requests from other processes with
//DosGiveSharedMem
#define MAXSHAREDMEMOBJECTS 64
#define SHAREDMEMBASEADDRESS 0x10520000

//this arrays contains the handles to the file mapping objects
static HANDLE hFileMapping[MAXSHAREDMEMOBJECTS];
static HANDLE hPoolFileMapping;
static HANDLE hPoolMutex;

struct SM_Pool {
        LPVOID BaseAddress;              //base address of 64MB pool
        struct {
                DWORD SM_BaseAddress;
                DWORD SM_Size;
                char  SM_Filemapname[128];
                char  SM_Name[128];
                DWORD SM_ReferenceCount;
        } SM_Object[MAXSHAREDMEMOBJECTS];
};

static int reserveObjectAddresses(int i) {
        return VirtualAlloc((LPVOID)(SHAREDMEMBASEADDRESS+i*1024*1024),
                            1024*1024,
                            MEM_RESERVE,
                            PAGE_NOACCESS
                           )
               !=NULL;
}
static int unreserveObjectAddresses(int i) {
        return VirtualFree((LPVOID)(SHAREDMEMBASEADDRESS+i*1024*1024),
                           0,
                           MEM_RELEASE
                          );
}

static DWORD WINAPI giveMemThread(LPVOID);

void initMemory() {
        for(int i=0; i<MAXSHAREDMEMOBJECTS; i++) {
                reserveObjectAddresses(i);
        }
        for(;;) {
                hPoolMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "os2sharemem_mutex");
                if(hPoolMutex==NULL) {
                        hPoolMutex = CreateMutex(NULL, TRUE, "os2sharemem_mutex");
                        if(hPoolMutex!=NULL)
                                break;
                } else {
                        WaitForSingleObject(hPoolMutex,INFINITE);
                        break;
                }
        }
        
        BOOL init=FALSE;
        for(;;) {
                hPoolFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "os2sharemem_pool");
                if(hPoolFileMapping==NULL) {
                        hPoolFileMapping = CreateFileMapping((HANDLE)0xffffffff,
                                                             NULL,
                                                             PAGE_READWRITE,
                                                             0,
                                                             sizeof(SM_Pool),
                                                             "os2sharemem_pool"
                                                            );
                        if(hPoolFileMapping!=NULL) {
                                init = TRUE;
                                break;
                        }
                } else {
                        break;
                }
        }
        //map pool into address space
        SM_Pool *ppool = (SM_Pool*)MapViewOfFile(hPoolFileMapping, FILE_MAP_ALL_ACCESS, 0,0, sizeof(SM_Pool));

        //initialize pool if needed
        if(init) {
                ppool->BaseAddress = (LPVOID)SHAREDMEMBASEADDRESS;
                for(int i=0; i<MAXSHAREDMEMOBJECTS; i++)
                        ppool->SM_Object[i].SM_ReferenceCount=0;
        }
        UnmapViewOfFile(ppool);

        ReleaseMutex(hPoolMutex);

        //start thread that handles DosGiveMem from other processes
        {
                DWORD tid;
                HANDLE h=CreateThread(NULL, 8192, giveMemThread, 0, 0, &tid);
                CloseHandle(h);
        }
}

static DWORD WINAPI giveMemThread(LPVOID) {
        DWORD pid=GetCurrentProcessId();
        char npname[256];
        wsprintf(npname,"\\\\.\\pipe\\os2sharemem_give_ipc_%08x",pid);

        HANDLE hPipe=CreateNamedPipe(npname,
                                     PIPE_ACCESS_DUPLEX,
                                     PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,
                                     1,
                                     128,
                                     128,
                                     150,
                                     NULL);
        if(hPipe==INVALID_HANDLE_VALUE) {
                ExitThread(0);
        }

        for(;;) {
                BOOL b=ConnectNamedPipe(hPipe,NULL);
                if(!b) {
                        CloseHandle(hPipe);
                        ExitThread(0);
                }
                struct {
                        PVOID objectbaseaddress;
                        ULONG flag;
                } s;
                DWORD bytesRead;
                b=ReadFile(hPipe, &s, sizeof(s), &bytesRead, NULL);
                if(b && bytesRead==4) {
                        APIRET rc=DosGetSharedMem(s.objectbaseaddress,s.flag);
                        DWORD bytesWritten;
                        WriteFile(hPipe, &rc, sizeof(rc), &bytesWritten, NULL);
                }
                DisconnectNamedPipe(hPipe);
        }
        //CloseHandle(hPipe);
        //ExitThread(0);                    
}



//quick test
static isSharedMem(PVOID pv) {
        if((DWORD)pv<SHAREDMEMBASEADDRESS ||
           (DWORD)pv>=SHAREDMEMBASEADDRESS+MAXSHAREDMEMOBJECTS*1024*1024)
                return 0;
        else
                return 1; //most likely, really
}

struct PoolLock {
        SM_Pool *pPool;
};
static SM_Pool *lockPool(PoolLock *pl) {
        //first, get access to the mutex
        WaitForSingleObject(hPoolMutex,INFINITE);
        //map pool into address space
        pl->pPool = (SM_Pool*)MapViewOfFile(hPoolFileMapping, FILE_MAP_ALL_ACCESS, 0,0, sizeof(SM_Pool));

        return pl->pPool;
                
}

static void unlockPool(PoolLock *pl) {
        //unmap pool
        UnmapViewOfFile(pl->pPool);
        //release mutex
        ReleaseMutex(hPoolMutex);
}


APIRET os2APIENTRY DosAllocSharedMem(PPVOID ppb,
                                     PCSZ  pszName,
                                     ULONG cb,
                                     ULONG flag)
{
        if(pszName && strnicmp(pszName,"\\SHAREMEM\\",10)!=0)
                return 123; //error_invalid_name
        if(!ppb)
                return 87; //error_invalid_parameter
        if(cb==0)
                return 87; //error_invalid_parameter
        
        PoolLock pl;
        SM_Pool *ppool=lockPool(&pl);
        if(pszName) {
                //see if it already exists
                for(int i=0; i<MAXSHAREDMEMOBJECTS; i++) {
                        if(stricmp(pszName,ppool->SM_Object[i].SM_Name)==0) {
                                unlockPool(&pl);
                                return 183; //error_already_exists
                        }
                }
        }

        int chunks=(cb+1024*1024-1)/(1024*1024);
        //find a free address range
        int i;
        for(i=0; i<MAXSHAREDMEMOBJECTS; i++) {
                if(ppool->SM_Object[i].SM_ReferenceCount==0) {
                        BOOL ok=TRUE;
                        for(int j=1; j<chunks && ok; j++) {
                                if(ppool->SM_Object[i+j].SM_ReferenceCount!=0)
                                        ok=FALSE;
                        }
                        if(ok) break;
                }
        }
        if(i>=MAXSHAREDMEMOBJECTS) {
                unlockPool(&pl);
                return 8; //error_not_enough_memory
        }

        DWORD objectbaseaddress = (DWORD)ppool->BaseAddress + i*1024*1024;
        char filemapname[128];
        wsprintf(filemapname,"os2sharemem_filemapping_%08x",objectbaseaddress);
        //create file mapping
        hFileMapping[i] = CreateFileMapping((HANDLE)0xffffffff,
                                            NULL,
                                            os2protect2nt(flag),
                                            0,
                                            (DWORD)cb,
                                            filemapname
                                           );
        if(hFileMapping[i]==NULL) {
                unlockPool(&pl);
                return 8; //error_not_enough_memmory
        }
        //map
        unreserveObjectAddresses(i);
        LPVOID lpv = MapViewOfFileEx(hFileMapping[i],
                                     flag&PAG_WRITE ? FILE_MAP_WRITE:FILE_MAP_READ,
                                     0,0,
                                     (DWORD)cb,
                                     (LPVOID)objectbaseaddress
                                    );
        if((DWORD)lpv!=objectbaseaddress) {
                //nt didn't like it
                reserveObjectAddresses(i);
                CloseHandle(hFileMapping[i]);
                hFileMapping[i]=NULL;
                return 8; //error_not_enough_memory
        }
        
        //mark object(s) as used
        for(int j=0; j<chunks; j++)
                ppool->SM_Object[i+j].SM_ReferenceCount=1;
        ppool->SM_Object[i].SM_BaseAddress    = (DWORD)ppool->BaseAddress + i*1024*1024;
        ppool->SM_Object[i].SM_Size           = cb;
        strcpy(ppool->SM_Object[i].SM_Filemapname,filemapname);
        strcpy(ppool->SM_Object[i].SM_Name,pszName?pszName:"");

        *ppb = (PVOID)objectbaseaddress;
        unlockPool(&pl);

        return 0;
}

APIRET os2APIENTRY DosGiveSharedMem(PVOID pb,
                                    PID pid,
                                    ULONG flag)
{
        if(pb==0)
                return 487; //error_invalid_address
        if(pid==GetCurrentProcessId())
                return 303; //error_invalid_procid

        //connect to target process at let it do the mapping
        char npname[256];
        wsprintf(npname,"\\\\.\\pipe\\os2sharemem_give_ipc_%08x",pid);

        struct {
                PVOID objectbaseaddress;
                ULONG flag;
        } s;
        APIRET rc;
        s.objectbaseaddress = pb;
        s.flag = flag;
        DWORD bytesRead;
        BOOL b = CallNamedPipe(npname, &s, sizeof(s), &rc, sizeof(rc), &bytesRead, NMPWAIT_WAIT_FOREVER);
        if(b==FALSE || bytesRead!=sizeof(rc))
                return 303;
        return rc;
}

APIRET os2APIENTRY DosGetSharedMem(PVOID pb,
                                   ULONG flag)
{
        if(!pb)
                return 487; //error_invalid_address
        if(flag&(PAG_READ|PAG_WRITE|PAG_EXECUTE|PAG_GUARD)!=flag)
                return 87; //error_invalid_parameter

        PoolLock pl;
        SM_Pool *ppool=lockPool(&pl);

        for(int i=0; i<MAXSHAREDMEMOBJECTS; i++)
                if(ppool->SM_Object[i].SM_ReferenceCount && ppool->SM_Object[i].SM_BaseAddress==(DWORD)pb)
                        break;
        if(i>=MAXSHAREDMEMOBJECTS) {
                unlockPool(&pl);
                return 487; //error_invalid_address
        }

        if(hFileMapping[i]!=NULL) {
                //already mapped
                unlockPool(&pl);
                return 0;
        }
        
        //open file mapping
        hFileMapping[i] = OpenFileMapping(flag&PAG_WRITE?FILE_MAP_WRITE:FILE_MAP_READ,
                                          FALSE,
                                          ppool->SM_Object[i].SM_Filemapname
                                         );
        if(hFileMapping[i]==NULL) {
                unlockPool(&pl);
                return 8; //error_not_enough_memory
        }
        
        //map
        unreserveObjectAddresses(i);
        LPVOID lpv = MapViewOfFileEx(hFileMapping[i],
                                     flag&PAG_WRITE ? FILE_MAP_WRITE:FILE_MAP_READ,
                                     0,0,
                                     (DWORD)ppool->SM_Object[i].SM_Size,
                                     (LPVOID)ppool->SM_Object[i].SM_BaseAddress
                                    );
        if((DWORD)lpv!=ppool->SM_Object[i].SM_BaseAddress) {
                //nt didn't like it
                reserveObjectAddresses(i);
                CloseHandle(hFileMapping[i]);
                hFileMapping[i]=NULL;
                return 8; //error_not_enough_memory
        }

        unlockPool(&pl);
        return 0;
}

APIRET os2APIENTRY DosGetNamedSharedMem(PPVOID ppb,
                                        PCSZ  pszName,
                                        ULONG flag)
{
        if(!pszName)
                return 8;
        
        PoolLock pl;
        SM_Pool *ppool=lockPool(&pl);
        for(int i=0; i<MAXSHAREDMEMOBJECTS; i++) {
                if(ppool->SM_Object[i].SM_ReferenceCount &&
                   stricmp(ppool->SM_Object[i].SM_Name,pszName)==0)
                break;
        }
        if(i>=MAXSHAREDMEMOBJECTS) {
                unlockPool(&pl);
                return 2; //error_file_not_found
        }

        *ppb = (PVOID)ppool->SM_Object[i].SM_BaseAddress;
        unlockPool(&pl);

        return DosGetSharedMem(*ppb,flag);
}

static APIRET dosFreeSharedMem(PVOID pb) {
        PoolLock pl;
        SM_Pool *ppool=lockPool(&pl);
        
        for(int i=0;
            ppool->SM_Object[i].SM_ReferenceCount==0 || ppool->SM_Object[i].SM_BaseAddress!=(DWORD)pb;
            i++
           );

        //unmap
        UnmapViewOfFile((LPCVOID)pb);
        //close filemap
        CloseHandle(hFileMapping[i]);
        hFileMapping[i]=NULL;
        //reserve mem range again
        reserveObjectAddresses(i);
        ppool->SM_Object[i].SM_ReferenceCount--;

        unlockPool(&pl);
        
        return 0;
}

}; //extern "C"

