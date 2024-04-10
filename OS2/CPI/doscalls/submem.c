/* --------------------------------------

Copyright (c) 1998-2004 Ivan Skytte Jørgensen

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use
of this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software in
       a product, an acknowledgment in the product documentation would be
       appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
---------------------------------------------------*/	

#define INCL_DOSSPINLOCK

#include "kal.h"

struct FreeHeapBlock;

struct HeapHeader {
        ULONG flags;            //flags to DosSubSetMem
        ULONG size;             //in bytes
        struct FreeHeapBlock *firstFree;
		HSPINLOCK lock;
//        SpinMutexSemaphore lock;
        char _filler[ 64 - sizeof(ULONG)*2 - sizeof(struct FreeHeapBlock*) - sizeof(HSPINLOCK /*SpinMutexSemaphore*/) ];
};

struct FreeHeapBlock {
        struct FreeHeapBlock *next;
        ULONG size;
};

APIRET APIENTRY  DosSubSetMem(PVOID pbBase,
                              ULONG flag,
                              ULONG cb)
{
        if(!pbBase)
                return 87; //invalid parameter
        
        cb = cb&0xFFFFFFF8; //round down to multiple of 8 bytes

        if(flag&DOSSUB_GROW) {
                APIRET rc;
                struct HeapHeader *hh=(struct HeapHeader*)pbBase;
                if(hh->flags&DOSSUB_SERIALIZE)
						DosAcquireSpinLock(hh->lock);
                        ;//hh->lock.Request();
                if(cb<hh->size) {
                        rc = 310; //dossub_shrink
                } else if(cb==hh->size) {
                        rc = 0;
                } else {
                        //find end
                        struct FreeHeapBlock *p=hh->firstFree,*prev=0;
                        while(p && p->next)
                                prev=p, p=p->next;
                        if(p)
                                p->size += cb - hh->size;
                        else {
                                struct FreeHeapBlock *end = (struct FreeHeapBlock*)(((char*)pbBase)+hh->size);
                                end->next = 0;
                                end->size = cb - hh->size;
                                hh->firstFree = end;
                        }
                        hh->size = cb;
                }
                if(hh->flags&DOSSUB_SERIALIZE)
                        DosReleaseSpinLock(hh->lock);//hh->lock.Release();
                return rc;
        } else if(flag&DOSSUB_INIT) {
                struct HeapHeader *hh=(struct HeapHeader*)pbBase;
                if(cb<sizeof(struct HeapHeader))
                return 87; //too small

                hh->flags = flag;
                hh->size = cb;
                if(flag&DOSSUB_SERIALIZE)
					if (DosCreateSpinLock(&hh->lock))
					//if(!hh->lock.Initialize())
                                return 1;//(APIRET)GetLastError();
                hh->firstFree = (struct FreeHeapBlock*)(hh+1);
                hh->firstFree->next = 0;
                hh->firstFree->size = cb - sizeof(*hh);

                return 0;
        } else
                return 1; //shared memory not supported (yet)
}


APIRET APIENTRY  DosSubAllocMem(PVOID pbBase,
                                PPVOID ppb,
                                ULONG cb)
{
        struct HeapHeader *hh=(struct HeapHeader*)pbBase;
        struct FreeHeapBlock *p=hh->firstFree,*prev=0;
        APIRET rc;

        if(!pbBase)
                return 87; //invalid parameter
        if(cb==0)
                return 87; //invalid parameter

        cb = (cb+7)&0xFFFFFFF8;

        if(hh->flags&DOSSUB_SERIALIZE)
                DosAcquireSpinLock(hh->lock);//hh->lock.Request();

        while(p) {
                if(p->size>=cb)
                        break;
                prev=p, p=p->next;
        }

        if(p) {
                if(p->size>cb) {
                        //split free block
                        struct FreeHeapBlock *n = (struct FreeHeapBlock*)(((char*)p)+cb);
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
                DosReleaseSpinLock(hh->lock);//hh->lock.Release();

        return rc;
}


APIRET APIENTRY  DosSubFreeMem(PVOID pbBase,
                               PVOID pb,
                               ULONG cb)
{
        struct HeapHeader *hh=(struct HeapHeader*)pbBase;
        APIRET rc;

        if(pbBase==0 ||
           pb==0)
                return 87; //invalid parameter
        if(cb==0)
                return 0;
        if((char*)pb < (char*)pbBase+sizeof(struct HeapHeader))
                return 87; //invalid parameter FixMe

        if(hh->flags&DOSSUB_SERIALIZE)
                DosAcquireSpinLock(hh->lock);//hh->lock.Request();


        if((char*)pb+cb >= (char*)pbBase+hh->size) {
                rc = 87; //FixMe
        } else {
                struct FreeHeapBlock *p;
                if(hh->firstFree) {
                        p = hh->firstFree;
                        if((char*)p < (char*)pb) {
                                while(p->next && (char*)p->next<(char*)pb)
                                        p=p->next;
                                if((char*)pb == (char*)p+p->size) {
                                        //join with previous
                                        p->size += cb;
                                } else {
                                        ((struct FreeHeapBlock*)pb)->next = p->next;
                                        ((struct FreeHeapBlock*)pb)->size = cb;
                                        p->next = (struct FreeHeapBlock*)pb;
                                        p=p->next;
                                }
                        } else {
                                //insert at front
                                p = (struct FreeHeapBlock*)pb;
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
                        p = (struct FreeHeapBlock*)pb;
                        p->next = hh->firstFree;
                        p->size = cb;
                        hh->firstFree = p;
                }
                rc = 0;
        }

        if(hh->flags&DOSSUB_SERIALIZE)
                DosReleaseSpinLock(hh->lock);//hh->lock.Release();

        return rc;
}


APIRET APIENTRY  DosSubUnsetMem(PVOID pbBase)
{
        struct HeapHeader *hh=(struct HeapHeader*)pbBase;

        if(!pbBase)
                return 87; //invalid parameter
        if(hh->flags&DOSSUB_SERIALIZE)
                DosFreeSpinLock(hh->lock);//hh->lock.Finalize();
        return 0;
}
