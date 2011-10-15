/*
 * Internal NT APIs and data structures
 *
 * Copyright (C) the Wine project
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

#ifndef __HANDLEMGR_H
#define __HANDLEMGR_H

#if defined(OS2) || defined(__OS2__)
#include <os2.h>
#endif

#ifdef __LINUX__
#include <gcc_os2def.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

typedef struct _RTL_HANDLE
{
    struct _RTL_HANDLE * pNext;
} HANDLE;

/*Viking: Some porting defines. A bit mixed style between OS/2 and the original handlemgr. */
#define ULONG_PTR         ULONG      /* Makes an integer of a pointer! */
/* typedef ULONG*   ULONG_PTR; */
#define SIZE_T            size_t
#define STATUS_NO_MEMORY  ERROR_NOT_ENOUGH_MEMORY
#define STATUS_SUCCESS    NO_ERROR

#define TRACE  printf
/* End of porting defines */

typedef struct _RTL_HANDLE_TABLE
{
    ULONG ulMaxHandleCount;  /* 0x00 */
    ULONG ulHandleSize;      /* 0x04 */
    PVOID pNextFree;        /* 0x10 */
    PVOID pFirstHandle;     /* 0x14 */
    PVOID pMaxHandle;       /* 0x1c */
    PVOID pReservedMemory;
} HANDLE_TABLE;


APIRET APIENTRY HndInitializeHandleTable(ULONG ulMaxHandleCount, ULONG ulHandleSize, HANDLE_TABLE * phtHandleTable);
APIRET APIENTRY HndDestroyHandleTable(HANDLE_TABLE * phtHandleTable);
APIRET APIENTRY HndAllocateHandle(HANDLE_TABLE * phtHandleTable, ULONG * pulHandleIndex, HANDLE ** pHandle);
APIRET APIENTRY HndFreeHandle(HANDLE_TABLE * HandleTable, HANDLE * pHandle);
APIRET APIENTRY HndIsValidHandle(const HANDLE_TABLE * HandleTable, const HANDLE * pHandle);
APIRET APIENTRY HndIsValidIndexHandle(const HANDLE_TABLE * HandleTable, ULONG ulIndex, HANDLE ** ValidHandle);


#ifdef __cplusplus
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif  /* __HANDLEMGR_H */
