//memory management APIs
#include "incl.hpp"
#include <string.h>
#include <stddef.h>
#include "spinsem.hpp"

struct FreeHeapBlock;
struct HeapHeader {
        ULONG flags;            //flags to DosSubSetMem
        ULONG size;             //in bytes
        FreeHeapBlock *firstFree;
        SpinMutexSemaphore lock;
        char _filler[ 64 - sizeof(ULONG)*2 - sizeof(FreeHeapBlock*) - sizeof(SpinMutexSemaphore) ];
};

struct FreeHeapBlock {
        FreeHeapBlock *next;
        ULONG size;
};

extern "C" {

APIRET os2APIENTRY DosSubSetMem(PVOID pbBase,
                                ULONG flag,
                                ULONG cb)
{
        if(!pbBase)
                return 87; //invalid parameter
        
        cb = cb&0xFFFFFFF8; //round down to multiple of 8 bytes

        if(flag&DOSSUB_GROW) {
                APIRET rc;
                HeapHeader *hh=(HeapHeader*)pbBase;
                if(hh->flags&DOSSUB_SERIALIZE)
                        hh->lock.Request();
                if(cb<hh->size) {
                        rc = 310; //dossub_shrink
                } else if(cb==hh->size) {
                        rc = 0;
                } else {
                        //find end
                        FreeHeapBlock *p=hh->firstFree,*prev=0;
                        while(p && p->next)
                                prev=p, p=p->next;
                        if(p)
                                p->size += cb - hh->size;
                        else {
                                FreeHeapBlock *end = (FreeHeapBlock*)(((char*)pbBase)+hh->size);
                                end->next = 0;
                                end->size = cb - hh->size;
                                hh->firstFree = end;
                        }
                        hh->size = cb;
                }
                if(hh->flags&DOSSUB_SERIALIZE)
                        hh->lock.Release();
                return rc;
        } else if(flag&DOSSUB_INIT) {
                if(cb<sizeof(HeapHeader))
                return 87; //too small

                HeapHeader *hh=(HeapHeader*)pbBase;
                hh->flags = flag;
                hh->size = cb;
                if(flag&DOSSUB_SERIALIZE)
                        if(!hh->lock.Initialize())
                                return (APIRET)GetLastError();
                hh->firstFree = (FreeHeapBlock*)(hh+1);
                hh->firstFree->next = 0;
                hh->firstFree->size = cb - sizeof(*hh);

                return 0;
        } else
                return 1; //shared memory not supported (yet)
}

APIRET os2APIENTRY DosSubUnsetMem(PVOID pbBase) {
        if(!pbBase)
                return 87; //invalid parameter
        HeapHeader *hh=(HeapHeader*)pbBase;
        if(hh->flags&DOSSUB_SERIALIZE)
                hh->lock.Finalize();
        return 0;
}

APIRET os2APIENTRY DosSubAllocMem(PVOID pbBase,
                                  PPVOID ppb,
                                  ULONG cb)
{
        if(!pbBase)
                return 87; //invalid parameter
        if(cb==0)
                return 87; //invalid parameter

        cb = (cb+7)&0xFFFFFFF8;

        HeapHeader *hh=(HeapHeader*)pbBase;
        if(hh->flags&DOSSUB_SERIALIZE)
                hh->lock.Request();

        FreeHeapBlock *p=hh->firstFree,*prev=0;
        while(p) {
                if(p->size>=cb)
                        break;
                prev=p, p=p->next;
        }
        APIRET rc;
        if(p) {
                if(p->size>cb) {
                        //split free block
                        FreeHeapBlock *n = (FreeHeapBlock*)(((char*)p)+cb);
                        n->next = p->next;
                        n->size = p->size - cb;
                        if(prev)
                                prev->next = n;
                        else
                                hh->firstFree = n;
                } else {
                        //use block
                        if(prev)
                                prev->next = p->next;
                        else
                                hh->firstFree = p->next;
                }
                *ppb = (PVOID)p;
                rc = 0;
        } else
                rc = 311; //dossub_nomem

        if(hh->flags&DOSSUB_SERIALIZE)
                hh->lock.Release();

        return rc;
}

APIRET os2APIENTRY DosSubFreeMem(PVOID pbBase,
                                 PVOID pb,
                                 ULONG cb)
{
        if(pbBase==0 ||
           pb==0)
                return 87; //invalid parameter
        if(cb==0)
                return 0;
        if((char*)pb < (char*)pbBase+sizeof(HeapHeader))
                return 87; //invalid parameter FixMe

        HeapHeader *hh=(HeapHeader*)pbBase;
        if(hh->flags&DOSSUB_SERIALIZE)
                hh->lock.Request();

        APIRET rc;

        if((char*)pb+cb >= (char*)pbBase+hh->size) {
                rc = 87; //FixMe
        } else {
                FreeHeapBlock *p;
                if(hh->firstFree) {
                        p = hh->firstFree;
                        if((char*)p < (char*)pb) {
                                while(p->next && (char*)p->next<(char*)pb)
                                        p=p->next;
                                if((char*)pb == (char*)p+p->size) {
                                        //join with previous
                                        p->size += cb;
                                } else {
                                        ((FreeHeapBlock*)pb)->next = p->next;
                                        ((FreeHeapBlock*)pb)->size = cb;
                                        p->next = (FreeHeapBlock*)pb;
                                        p=p->next;
                                }
                        } else {
                                //insert at front
                                p = (FreeHeapBlock*)pb;
                                p->size = cb;
                                p->next = hh->firstFree;
                                hh->firstFree = p;
                        }
                        if((char*)p+p->size == (char*)p->next) {
                                //join with next
                                p->size += p->next->size;
                                p->next = p->next->next;
                        }
                } else {
                        p = (FreeHeapBlock*)pb;
                        p->next = hh->firstFree;
                        p->size = cb;
                        hh->firstFree = p;
                }
                rc = 0;
        }

        if(hh->flags&DOSSUB_SERIALIZE)
                hh->lock.Release();

        return rc;
}

}; //extern "C"
