/*
 * Handles Manager
 *
 * Copyright (C) 2008 osFree
 * Copyright (C) 2004 Robert Shearman
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#if defined(OS2) || defined(__OS2__)
/* OS/2 API includes */
#define INCL_ERRORS
#include <os2.h>
#endif

/* osFree internal */
#include <os3/handlemgr.h>

/* libc includes */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/**************************************************************************
 *      HndInitializeHandleTable
 *
 * Initializes a handle table.
 *
 * PARAMS
 *  ulMaxHandleCount [I] The maximum number of handles the handle table will support.
 *  ulHandleSize     [I] The size of each handle.
 *  pHandleTable     [I/O] The handle table.
 *
 * RETURNS
 *  NO_ERROR                 If all success
 *  ERROR_INVALID_PARAMETER  If invalid parameter passed
 *
 * SEE
 *  HndDestroyHandleTable().
 */
APIRET APIENTRY HndInitializeHandleTable(ULONG ulMaxHandleCount, ULONG ulHandleSize, HANDLE_TABLE * pHandleTable)
{
    // Check arguments
    if ((!pHandleTable)||
         (!ulMaxHandleCount)||
         (!ulHandleSize)) return ERROR_INVALID_PARAMETER;

    memset(pHandleTable, 0, sizeof(*pHandleTable));
    pHandleTable->ulMaxHandleCount = ulMaxHandleCount;
    pHandleTable->ulHandleSize = ulHandleSize;

    return NO_ERROR;
}

/**************************************************************************
 *      HndDestroyHandleTable
 *
 * Destroys a handle table and frees associated resources.
 *
 * PARAMS
 *  pHandleTable    [I] The handle table.
 *
 * RETURNS
 *  Any status code returned by NtFreeVirtualMemory().
 *
 * SEE
 *  HndInitializeHandleTable().
 */
APIRET APIENTRY HndDestroyHandleTable(HANDLE_TABLE * pHandleTable)
{
    // Check arguments
    if (!pHandleTable) return ERROR_INVALID_PARAMETER;
    // @todo: Here we need to make checks to free linked memory in
    // struct members
    free((pHandleTable->pFirstHandle));
    return NO_ERROR;
}

/**************************************************************************
 *      HndpAllocateSomeHandles   (internal)
 *
 * Reserves memory for the handles if not previously done and commits memory
 * for a batch of handles if none are free and adds them to the free list.
 *
 * PARAMS
 *  pHandleTable    [I/O] The handle table.
 *
 * RETURNS
 *  NTSTATUS code.
 */
APIRET APIENTRY HndpAllocateSomeHandles(HANDLE_TABLE * pHandleTable)
{
    // Check arguments
    if (!pHandleTable) return ERROR_INVALID_PARAMETER;

    if (!pHandleTable->pFirstHandle)
    {
        PVOID pFirstHandleAddr = NULL;
        ULONG MaxSize = pHandleTable->ulMaxHandleCount * pHandleTable->ulHandleSize;

        /* reserve memory for the handles, but don't commit it yet because we
         * probably won't use most of it and it will use up physical memory */
        pFirstHandleAddr = malloc(MaxSize);
        pHandleTable->pFirstHandle = pFirstHandleAddr;
        pHandleTable->pReservedMemory = pHandleTable->pFirstHandle;
        pHandleTable->pMaxHandle = (char *)pHandleTable->pFirstHandle + MaxSize;
    }
    if (!pHandleTable->pNextFree)
    {
        SIZE_T Offset, CommitSize = 4096; /* one page */
        HANDLE * FreeHandle = NULL;
        PVOID NextAvailAddr = pHandleTable->pReservedMemory;

        if (pHandleTable->pReservedMemory >= pHandleTable->pMaxHandle)
            return ERROR_NOT_ENOUGH_MEMORY; /* the handle table is completely full */

        for (Offset = 0; Offset < CommitSize; Offset += pHandleTable->ulHandleSize)
        {
            /* make sure we don't go over handle limit, even if we can
             * because of rounding of the table size up to the next page
             * boundary */
            if ((char *)pHandleTable->pReservedMemory + Offset >= (char *)pHandleTable->pMaxHandle)
                break;

            FreeHandle = (HANDLE *)((char *)pHandleTable->pReservedMemory + Offset);

            FreeHandle->pNext = (HANDLE *)((char *)pHandleTable->pReservedMemory +
                Offset + pHandleTable->ulHandleSize);
        }

        /* shouldn't happen because we already test for this above, but
         * handle it just in case */
        if (!FreeHandle)
            return ERROR_NOT_ENOUGH_MEMORY;

        /* set the last handle's Next pointer to NULL so that when we run
         * out of free handles we trigger another commit of memory and
         * initialize the free pointers */
        FreeHandle->pNext = NULL;

        pHandleTable->pNextFree = pHandleTable->pReservedMemory;

        pHandleTable->pReservedMemory = (char *)pHandleTable->pReservedMemory + CommitSize;
    }
    return NO_ERROR;
}

/**************************************************************************
 *      HndAllocateHandle   (NTDLL.@)
 *
 * Allocates a handle from the handle table.
 *
 * PARAMS
 *  HandleTable    [I/O] The handle table.
 *  HandleIndex    [O] Index of the handle returned. Optional.
 *
 * RETURNS
 *  Success: Pointer to allocated handle.
 *  Failure: NULL.
 *
 * NOTES
 *  A valid handle must have the bit set as indicated in the code below
 *  otherwise subsequent HndIsValidHandle() calls will fail.
 *
 *  static inline void HndpMakeHandleAllocated(RTL_HANDLE * Handle)
 *  {
 *    ULONG_PTR *AllocatedBit = (ULONG_PTR *)(&Handle->Next);
 *    *AllocatedBit = *AllocatedBit | 1;
 *  }
 *
 * SEE
 *  HndFreeHandle().
 */
APIRET APIENTRY APIENTRY HndAllocateHandle(HANDLE_TABLE * HandleTable, ULONG * HandleIndex, HANDLE *Handle)
{
    if (!HandleTable->pNextFree && HndpAllocateSomeHandles(HandleTable) != NO_ERROR)
        return ERROR_TOO_MANY_HANDLES;

    Handle = (HANDLE *)HandleTable->pNextFree;
    HandleTable->pNextFree = Handle->pNext;

    if (HandleIndex)
        *HandleIndex = (ULONG)(((PCHAR)Handle - (PCHAR)HandleTable->pFirstHandle) / HandleTable->ulHandleSize);

    return NO_ERROR;
}

/**************************************************************************
 *      HndFreeHandle   (NTDLL.@)
 *
 * Frees an allocated handle.
 *
 * PARAMS
 *  HandleTable    [I/O] The handle table.
 *  Handle         [I] The handle to be freed.
 *
 * RETURNS
 *  Success: TRUE.
 *  Failure: FALSE.
 *
 * SEE
 *  HndAllocateHandle().
 */
BOOL APIENTRY HndFreeHandle(HANDLE_TABLE * HandleTable, HANDLE * Handle)
{
//    TRACE("(%p, %p)\n", HandleTable, Handle);
    /* NOTE: we don't validate the handle and we don't make Handle->Next even
     * again to signal that it is no longer in user - that is done as a side
     * effect of setting Handle->Next to the previously next free handle in
     * the handle table */
    memset(Handle, 0, HandleTable->ulHandleSize);
    Handle->pNext = (HANDLE *)HandleTable->pNextFree;
    HandleTable->pNextFree = Handle;
    return NO_ERROR;
}

/**************************************************************************
 *      HndIsValidHandle   (NTDLL.@)
 *
 * Determines whether a handle is valid or not.
 *
 * PARAMS
 *  HandleTable    [I] The handle table.
 *  Handle         [I] The handle to be tested.
 *
 * RETURNS
 *  Valid: TRUE.
 *  Invalid: FALSE.
 */
BOOL APIENTRY HndIsValidHandle(const HANDLE_TABLE * HandleTable, const HANDLE * Handle)
{
    //TRACE("(%p, %p)\n", HandleTable, Handle);
    /* make sure handle is within used region and that it is aligned on
     * a HandleTable->HandleSize boundary and that Handle->Next is odd,
     * indicating that the handle is active */
    if ((Handle >= (HANDLE *)HandleTable->pFirstHandle) &&
      (Handle < (HANDLE *)HandleTable->pReservedMemory) &&
      !((ULONG_PTR)Handle & (HandleTable->ulHandleSize - 1)) &&
      ((ULONG_PTR)Handle->pNext & 1))
        return TRUE;
    else
        return FALSE;
}

/**************************************************************************
 *      HndIsValidIndexHandle   (NTDLL.@)
 *
 * Determines whether a handle index is valid or not.
 *
 * PARAMS
 *  HandleTable    [I] The handle table.
 *  Index          [I] The index of the handle to be tested.
 *  ValidHandle    [O] The handle Index refers to.
 *
 * RETURNS
 *  Valid: TRUE.
 *  Invalid: FALSE.
 */
BOOL APIENTRY HndIsValidIndexHandle(const HANDLE_TABLE * HandleTable, ULONG Index, HANDLE ** ValidHandle)
{
    HANDLE * Handle;

//    TRACE("(%p, %u, %p)\n", HandleTable, Index, ValidHandle);
    Handle = (HANDLE *)
        ((char *)HandleTable->pFirstHandle + Index * HandleTable->ulHandleSize);

    if (HndIsValidHandle(HandleTable, Handle))
    {
        *ValidHandle = Handle;
        return TRUE;
    }
    return FALSE;
}
/* #endif */
