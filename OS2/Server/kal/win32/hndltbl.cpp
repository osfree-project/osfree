#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "hndltbl.hpp"

HandleTable_::HandleTable_(int g, int s)
  : gran(g),
    maxsize(s),
    sidx(0)
{
        table = new void* [maxsize];
        hMutex = new unsigned long [gran];

        for(int i=0; i<maxsize; i++)
                table[i] = 0;
        for(i=0; i<gran; i++)
                hMutex[i] = (unsigned long)CreateMutex(0,FALSE,0);
}

HandleTable_::~HandleTable_() {
        delete[] table;
        for(int i=0; i<gran; i++)
                CloseHandle((HANDLE)hMutex[i]);
        delete[] hMutex;
}

void HandleTable_::lockAll() {
        WaitForMultipleObjects(gran, (HANDLE*)hMutex, TRUE, INFINITE);
}

void HandleTable_::unlockAll() {
        for(int i=0; i<gran; i++)
                ReleaseMutex((HANDLE)hMutex[i]);
}

void HandleTable_::lock(int idx) {
        WaitForSingleObject((HANDLE)hMutex[idx%gran], INFINITE);
}

void HandleTable_::unlock(int idx) {
        ReleaseMutex((HANDLE)hMutex[idx%gran]);
}

int HandleTable_::findAndLockFree() {
        lockAll();
        int idx;
        for(idx=sidx; idx<maxsize && table[idx]; idx++)
                ;
        if(idx>=maxsize)
                for(idx=0; idx<sidx && idx<maxsize && table[idx]; idx++)
                        ;
        if(idx<maxsize && table[idx]==0) {
                for(int i=0; i<gran; i++)
                        if(i!=idx%gran)
                                ReleaseMutex((HANDLE)hMutex[i]);
                return idx;
        } else {
                unlockAll();
                return -1;
        }
}


