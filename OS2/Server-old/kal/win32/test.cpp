#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#include <os2.h>
extern "C" {
void initFileTable();
void initProcess(unsigned hinst, const char *lpCmdLine);
void initMemory();
};

#include <stdio.h>

void APIENTRY MyThread(ULONG) {
}

void main(void) {
        initFileTable();
        initProcess(0,0);
        initMemory();
        
        APIRET rc;
        PVOID pv;
        rc = DosAllocSharedMem(&pv, "\\SHAREMEM\\Ivan", 8192, PAG_COMMIT|PAG_READ|PAG_WRITE);
        if(rc==0)
                rc = DosFreeMem(pv);
        printf("rc=%d\n",rc);
}

