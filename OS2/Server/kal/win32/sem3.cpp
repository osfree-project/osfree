//MuxWait semaphores
#define INCL_DOSSEMAPHORES
#include "incl.hpp"
#include <string.h>
#include <stddef.h>
#include "hndltbl.hpp"

extern "C" {

struct MuxWait {
        BOOL waitAll;
        int count;
        SEMRECORD semrec[1];
};

static HandleTable<MuxWait,10,150> mwTable;

APIRET os2APIENTRY DosCreateMuxWaitSem (PCSZ  pszName,
                                        PHMUX phmux,
                                        ULONG cSemRec,
                                        PSEMRECORD pSemRec,
                                        ULONG flAttr)
{
        if(pszName) return 123; //invalid name
        if(*phmux) return 87; //invalid parameter
        if(cSemRec>0 && pSemRec==0)
                return 87; //invalid parameter
        if(flAttr&DC_SEM_SHARED)
                return 8; //shared sem not supported
        if((flAttr&(DCMW_WAIT_ANY|DCMW_WAIT_ALL))==(DCMW_WAIT_ANY|DCMW_WAIT_ALL))
                return 87; //invalid parameter
        if((flAttr&(DCMW_WAIT_ANY|DCMW_WAIT_ALL))==0)
                return 87; //invalid parameter

        MuxWait *mw = (MuxWait*) new char[sizeof(MuxWait)-sizeof(SEMRECORD)+sizeof(SEMRECORD)*cSemRec];
        if(!mw)
                return 8; //not enough memory

        if(flAttr&DCMW_WAIT_ALL)
                mw->waitAll=TRUE;
        else
                mw->waitAll=FALSE;
        mw->count = cSemRec;
        for(int i=0; i<cSemRec; i++) {
                for(int j=0; j<i; j++) {
                        if(pSemRec[i].hsemCur==pSemRec[j].hsemCur) {
                                delete mw;
                                return 284; //duplicate handle
                        }
                }
                mw->semrec[i] = pSemRec[i];
        }

        int idx=mwTable.findAndLockFree();
        if(idx==-1) return 8; //not enough memory
        mwTable[idx] = mw;
        mwTable.unlock(idx);

        *phmux = (HMUX)idx;
        return 0;
}

APIRET os2APIENTRY DosOpenMuxWaitSem (PCSZ  /*pszName*/,
                                      PHMUX /*phmux*/)
{
         return 187; //sem not found
}

APIRET os2APIENTRY DosCloseMuxWaitSem (HMUX hmux)
{
        int idx=(int)hmux;
        mwTable.lock(idx);
        if(!mwTable[idx]) {
                mwTable.unlock(idx);
                return 6; //invalid handle
        }

        delete[] (char*)mwTable[idx];
        mwTable[idx] = 0;
        mwTable.unlock(idx);

        return 0;
}

APIRET os2APIENTRY DosWaitMuxWaitSem (HMUX hmux,
                                      ULONG ulTimeout,
                                      PULONG pulUser)
{
        int idx=(int)hmux;
        mwTable.lock(idx);
        if(!mwTable[idx]) {
                mwTable.unlock(idx);
                return 6; //invalid handle
        }

        MuxWait *mw = mwTable[idx];
        if(mw->count==0) {
                mwTable.unlock(idx);
                return 286; //empty muxwait
        }

        HANDLE q[20];
        HANDLE *lph;
        if(mw->count<=20) {
                lph = q;
        } else {
                lph = new HANDLE[mw->count];
                if(!lph) {
                        mwTable.unlock(idx);
                        return 8; //not enough memory
                }
        }
        for(int i=0; i<mw->count; i++)
                lph[i] = (HANDLE) mw->semrec[i].hsemCur;

        DWORD cObjects = mw->count;
        BOOL waitAll=mw->waitAll;
        mwTable.unlock(idx);

        DWORD dw=WaitForMultipleObjects(cObjects,lph,waitAll,ulTimeout==(ULONG)-1?INFINITE:ulTimeout);
        if(dw==WAIT_TIMEOUT) {
                if(lph!=q) delete[] lph;
                return 640; //timeout
        } else if(dw>=WAIT_ABANDONED_0 && dw<WAIT_ABANDONED_0+cObjects) {
                if(lph!=q) delete[] lph;
                return 105;
        } else if(/*dw>=WAIT_OBJECT_0 &&*/ dw<WAIT_OBJECT_0+cObjects) {
                mwTable.lock(idx);
                if(!mwTable[idx]) {
                        //someone has deleted the muxwait
                        mwTable.unlock(idx);
                        return 6; //invalid handle
                }
                *pulUser = mwTable[idx]->semrec[dw-WAIT_OBJECT_0].ulUser;
                mwTable.unlock(idx);
                if(lph!=q) delete[] lph;
                return 0;
        } else
                return (APIRET)GetLastError();
}

APIRET os2APIENTRY DosAddMuxWaitSem (HMUX hmux,
                                     PSEMRECORD pSemRec)
{
        if(!pSemRec) return 87;
        int idx=(int)hmux;
        mwTable.lock(idx);
        if(!mwTable[idx]) {
                mwTable.unlock(idx);
                return 6; //invalid handle
        }
        MuxWait *old_mw = mwTable[idx];
        MuxWait *new_mw = (MuxWait*)new char[sizeof(MuxWait)-sizeof(SEMRECORD)+sizeof(SEMRECORD)*(old_mw->count+1)];
        if(!new_mw) {
                mwTable.unlock(idx);
                return 8; //not enough memory
        }

        memcpy(new_mw,old_mw, sizeof(MuxWait)-sizeof(SEMRECORD)+sizeof(SEMRECORD)*old_mw->count);
        new_mw->semrec[old_mw->count] = *pSemRec;
        new_mw->count = old_mw->count + 1;
        mwTable[idx] = new_mw;
        mwTable.unlock(idx);
        delete old_mw;

        return 0;
}


APIRET os2APIENTRY DosDeleteMuxWaitSem (HMUX hmux,
                                        HSEM hSem)
{
        int idx=(int)hmux;
        mwTable.lock(idx);
        if(!mwTable[idx]) {
                mwTable.unlock(idx);
                return 6; //invalid handle
        }
        MuxWait *mw=mwTable[idx];
        for(int i=0; i<mw->count; i++) {
                if(mw->semrec[i].hsemCur==hSem)
                        break;
        }
        if(i>=mw->count) {
                mwTable.unlock(idx);
                return 6; //invalid handle
        }

        for(; i<mw->count-1; i++)
                mw->semrec[i] = mw->semrec[i+1];
        mw->count--;
        mwTable.unlock(idx);

        return 0;
}

APIRET os2APIENTRY DosQueryMuxWaitSem (HMUX hmux,
                                       PULONG pcSemRec,
                                       PSEMRECORD pSemRec,
                                       PULONG pflAttr)
{
        if(!pSemRec) return 87;
        int idx=(int)hmux;
        mwTable.lock(idx);
        if(!mwTable[idx]) {
                mwTable.unlock(idx);
                return 6; //invalid handle
        }
        MuxWait *mw=mwTable[idx];

        if(*pcSemRec<mw->count) {
                *pcSemRec = (ULONG)mw->count;
                mwTable.unlock(idx);
                return 289; //param too small
        }

        memcpy(pSemRec,mw->semrec,mw->count*sizeof(SEMRECORD));
        *pcSemRec = (ULONG)mw->count;
        if(pflAttr) *pflAttr=mw->waitAll?DCMW_WAIT_ALL:DCMW_WAIT_ANY;
        mwTable.unlock(idx);
        return 0;
}


};

