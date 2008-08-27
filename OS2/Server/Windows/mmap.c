#include <windows.h>
#include <sys/mman.h>

/* getpagesize for windows */
long getpagesize (void) {
    static long g_pagesize = 0;
    if (! g_pagesize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        g_pagesize = system_info.dwPageSize;
    }
    return g_pagesize;
}
long getregionsize (void) {
    static long g_regionsize = 0;
    if (! g_regionsize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        g_regionsize = system_info.dwAllocationGranularity;
    }
    return g_regionsize;
}

/* Wait for spin lock */
int slwait (int *sl) {
    while (InterlockedCompareExchange ((void **) sl, (void *) 1, (void *) 0) != 0)
        Sleep (0);
    return 0;
}
/* Release spin lock */
int slrelease (int *sl) {
    InterlockedExchange (sl, 0);
    return 0;
}

static int g_sl;

/* mmap for windows */
void *mmap (void *ptr, long size, long prot, long type, long handle, long arg) {
    static long g_pagesize;
    static long g_regionsize;
    /* Wait for spin lock */
    slwait (&g_sl);
    /* First time initialization */
    if (! g_pagesize)
        g_pagesize = getpagesize ();
    if (! g_regionsize)
        g_regionsize = getregionsize ();
    /* Allocate this */
    ptr = VirtualAlloc (ptr, size,
                        MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
    if (! ptr) {
        ptr = MMAP_FAILURE;
        goto mmap_exit;
    }
mmap_exit:
    /* Release spin lock */
    slrelease (&g_sl);
    return ptr;
}
/* munmap for windows */
long munmap (void *ptr, long size) {
    static long g_pagesize;
    static long g_regionsize;
    int rc = MUNMAP_FAILURE;
    /* Wait for spin lock */
    slwait (&g_sl);
    /* First time initialization */
    if (! g_pagesize)
        g_pagesize = getpagesize ();
    if (! g_regionsize)
        g_regionsize = getregionsize ();
    /* Free this */
    if (! VirtualFree (ptr, 0,
                       MEM_RELEASE))
        goto munmap_exit;
    rc = 0;
munmap_exit:
    /* Release spin lock */
    slrelease (&g_sl);
    return rc;
}
