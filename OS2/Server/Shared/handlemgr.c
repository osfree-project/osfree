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

#define INCL_ERRORS
#include <os2.h>

#include <stdarg.h>

#include "handlemgr.h"

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
    free((pHandleTable->ulFirstHandle));
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

    if (!pHandleTable->FirstHandle)
    {
        PVOID pFirstHandleAddr = NULL;
        ULONG ulMaxSize = HandleTable->ulMaxHandleCount * HandleTable->ulHandleSize;

        /* reserve memory for the handles, but don't commit it yet because we
         * probably won't use most of it and it will use up physical memory */
        pFirstHandleAddr = malloc(MaxSize);
        pHandleTable->pFirstHandle = pFirstHandleAddr;
        pHandleTable->ReservedMemory = HandleTable->FirstHandle;
        pHandleTable->ulMaxHandle = (char *)HandleTable->FirstHandle + MaxSize;
    }
    if (!HandleTable->NextFree)
    {
        SIZE_T Offset, CommitSize = 4096; /* one page */
        RTL_HANDLE * FreeHandle = NULL;
        PVOID NextAvailAddr = HandleTable->ReservedMemory;

        if (HandleTable->ReservedMemory >= HandleTable->MaxHandle)
            return STATUS_NO_MEMORY; /* the handle table is completely full */

        for (Offset = 0; Offset < CommitSize; Offset += HandleTable->HandleSize)
        {
            /* make sure we don't go over handle limit, even if we can
             * because of rounding of the table size up to the next page
             * boundary */
            if ((char *)HandleTable->ReservedMemory + Offset >= (char *)HandleTable->MaxHandle)
                break;

            FreeHandle = (RTL_HANDLE *)((char *)HandleTable->ReservedMemory + Offset);

            FreeHandle->Next = (RTL_HANDLE *)((char *)HandleTable->ReservedMemory +
                Offset + HandleTable->HandleSize);
        }

        /* shouldn't happen because we already test for this above, but
         * handle it just in case */
        if (!FreeHandle)
            return STATUS_NO_MEMORY;

        /* set the last handle's Next pointer to NULL so that when we run
         * out of free handles we trigger another commit of memory and
         * initialize the free pointers */
        FreeHandle->Next = NULL;

        HandleTable->NextFree = HandleTable->ReservedMemory;

        HandleTable->ReservedMemory = (char *)HandleTable->ReservedMemory + CommitSize;
    }
    return STATUS_SUCCESS;
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
RTL_HANDLE * APIENTRY HndAllocateHandle(RTL_HANDLE_TABLE * HandleTable, ULONG * HandleIndex)
{
    RTL_HANDLE * ret;

    if (!HandleTable->NextFree && HndpAllocateSomeHandles(HandleTable) != STATUS_SUCCESS)
        return NULL;

    ret = (RTL_HANDLE *)HandleTable->NextFree;
    HandleTable->NextFree = ret->Next;

    if (HandleIndex)
        *HandleIndex = (ULONG)(((PCHAR)ret - (PCHAR)HandleTable->FirstHandle) / HandleTable->HandleSize);

    return ret;
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
BOOL APIENTRY HndFreeHandle(RTL_HANDLE_TABLE * HandleTable, RTL_HANDLE * Handle)
{
    TRACE("(%p, %p)\n", HandleTable, Handle);
    /* NOTE: we don't validate the handle and we don't make Handle->Next even
     * again to signal that it is no longer in user - that is done as a side
     * effect of setting Handle->Next to the previously next free handle in
     * the handle table */
    memset(Handle, 0, HandleTable->HandleSize);
    Handle->Next = (RTL_HANDLE *)HandleTable->NextFree;
    HandleTable->NextFree = Handle;
    return TRUE;
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
BOOL APIENTRY HndIsValidHandle(const RTL_HANDLE_TABLE * HandleTable, const RTL_HANDLE * Handle)
{
//    TRACE("(%p, %p)\n", HandleTable, Handle);
    /* make sure handle is within used region and that it is aligned on
     * a HandleTable->HandleSize boundary and that Handle->Next is odd,
     * indicating that the handle is active */
    if ((Handle >= (RTL_HANDLE *)HandleTable->FirstHandle) &&
      (Handle < (RTL_HANDLE *)HandleTable->ReservedMemory) &&
      !((ULONG_PTR)Handle & (HandleTable->HandleSize - 1)) &&
      ((ULONG_PTR)Handle->Next & 1))
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
BOOL APIENTRY HndIsValidIndexHandle(const RTL_HANDLE_TABLE * HandleTable, ULONG Index, RTL_HANDLE ** ValidHandle)
{
    RTL_HANDLE * Handle;

//    TRACE("(%p, %u, %p)\n", HandleTable, Index, ValidHandle);
    Handle = (RTL_HANDLE *)
        ((char *)HandleTable->FirstHandle + Index * HandleTable->HandleSize);

    if (HndIsValidHandle(HandleTable, Handle))
    {
        *ValidHandle = Handle;
        return TRUE;
    }
    return FALSE;
}
#endif
