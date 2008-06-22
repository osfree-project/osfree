/*

gbmmem.c - Memory management functions for GBM.

Author: Heiko Nitzsche

History
-------
08-Feb-2008: Initial version

*/

#include "gbmmem.h"

#ifdef __OS2__
  #define INCL_DOSMEMMGR   /* Memory Manager */
  #define INCL_DOSERRORS   /* DOS error values */
  #include <os2.h>
  #include <memory.h>
#else
  #include <memory.h>
#endif

#ifdef NDEBUG
  #undef NDEBUG
#endif
#include <assert.h>

/************************************************************/

void * gbmmem_malloc(size_t size)
{
    void * pBuffer = NULL;

#ifdef __OS2__
    const ULONG allocSize = (ULONG)size;

    /* Try to allocate from high memory first. For thread safety reasons
     * this will be done every time. Updating a static variable would require
     * thread synchronization which is more expensive than trial and error.
     */
    APIRET apiRet = DosAllocMem(&pBuffer, allocSize,
                                PAG_READ | PAG_WRITE | PAG_COMMIT | OBJ_ANY);
    switch(apiRet)
    {
        case NO_ERROR:
            break;

        case ERROR_INVALID_PARAMETER:
            /* try again without highmem request as fallback for older systems */
            apiRet = DosAllocMem(&pBuffer, allocSize,
                                 PAG_READ | PAG_WRITE | PAG_COMMIT);
            if (apiRet != NO_ERROR)
            {
                return NULL;
            }
            break;

        default:
            return NULL;
    }
#else
    pBuffer = malloc(size);
#endif

    return pBuffer;
}

/************************************************************/

void * gbmmem_calloc(size_t num, size_t size)
{
    void * pBuffer = NULL;

#ifdef __OS2__
    const size_t bytes = num * size;
    pBuffer = gbmmem_malloc(bytes);
    if (pBuffer != NULL)
    {
        memset(pBuffer, 0, bytes);
    }
#else
    pBuffer = calloc(num, size);
#endif

    return pBuffer;
}

/************************************************************/

void gbmmem_free(void * buffer)
{
    if (buffer == NULL)
    {
        return;
    }
#ifdef __OS2__
    {
        APIRET apiRet = DosFreeMem(buffer);
        assert(apiRet == NO_ERROR);
    }
#else
    free(buffer);
#endif
}


