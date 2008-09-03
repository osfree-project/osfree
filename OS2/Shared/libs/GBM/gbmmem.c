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

/**
 * Changes the size of a previously reserved storage block. The ptr argument points
 * to the beginning of the block. The contents of the block are unchanged up to the
 * shorter of the new and old sizes. gbmmem_realloc allocates the new block from
 * the same heap the original block was in.
 * If ptr is NULL, gbmmem_realloc reserves a block of storage of size bytes from the
 * current thread's default heap (equivalent to calling gbmmem_malloc(size)).
 * If size is 0 and the ptr is not NULL, gbmmem_realloc frees the storage allocated
 * to ptr and returns NULL.
 *
 * @param ptr   The pointer of the block to be resized.
 * @param size  The new size of the block in bytes.
 *
 * @return The pointer to the buffer or NULL.
 *
 * @note gbmmem_realloc returns a pointer to the reallocated storage block.
 * The storage location of the block may be moved by the gbmmem_realloc function.
 * Thus, the ptr argument to gbmmem_realloc is not necessarily the same as the
 * return value.
 * If size is 0, realloc returns NULL. If there is not enough storage to
 * expand the block to the given size, the original block is unchanged
 * and gbmmem_realloc returns NULL.
 */

/************************************************************/

void * gbmmem_realloc(void *ptr, size_t size)
{
#ifdef __OS2__
    if (ptr == NULL)
    {
        return gbmmem_malloc(size);
    }
    if ((ptr != NULL) && (size == 0))
    {
        gbmmem_free(ptr);
        return NULL;
    }
    if (ptr != NULL)
    {
        /* get the size of the old memory block */
        ULONG oldSize = 0xffffffffUL;
        ULONG attribs = 0;
        if (DosQueryMem(ptr, &oldSize, &attribs) == NO_ERROR)
        {
            void * newPtr = gbmmem_malloc(size);
            if (newPtr != NULL)
            {
                memcpy(newPtr, ptr, (oldSize < (ULONG)size) ? oldSize : size);
                gbmmem_free(ptr);
            }
            return newPtr;
        }
    }
    return NULL;
#endif

    return realloc(ptr, size);
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


