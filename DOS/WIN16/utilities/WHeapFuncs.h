/* @(#)WHeapFuncs.h	1.4 11:42:37 12/13/96 /users/sccs/src/win/utilities/s.WHeapFuncs.h */
#ifndef WHEAPFUNCS_H
#define WHEAPFUNCS_H

#include "WUtilities.h"

#ifndef _WINDOWS
#define HeapAlloc                       WHeapAlloc
#define HeapCompact                     WHeapCompact
#define HeapCreate                      WHeapCreate
#define HeapDestroy                     WHeapDestroy
#define HeapFree                        WHeapFree
#define HeapLock                        WHeapLock
#define HeapReAlloc                     WHeapReAlloc
#define HeapSize                        WHeapSize
#define HeapUnlock                      WHeapUnlock
#define HeapValidate                    WHeapValidate
#define HeapWalk                        WHeapWalk
#define GlobalAlloc                     WGlobalAlloc
#undef GlobalDiscard
#define GlobalDiscard                   WGlobalDiscard
#define GlobalFlags                     WGlobalFlags
#define GlobalFree                      WGlobalFree
#define GlobalLock                      WGlobalLock
#define GlobalReAlloc                   WGlobalReAlloc
#define GlobalSize                      WGlobalSize
#define GlobalUnlock                    WGlobalUnlock
#endif /*--- #ifndef _WINDOWS ---*/

#ifdef _cplusplus
extern "C" {
#endif 

LPVOID EXPORT WHeapAlloc (
    HANDLE                              hHeap,
    DWORD                               dwFlags,
    DWORD                               dwBytes );

UINT EXPORT WHeapCompact (
    HANDLE                              hHeap,
    DWORD                               dwFlags );

HANDLE EXPORT WHeapCreate (
    DWORD                               flOptions,
    DWORD                               dwInitialSize,
    DWORD                               dwMaximumSize );

BOOL EXPORT WHeapDestroy (
    HANDLE                              hHeap );

BOOL EXPORT WHeapFree (
    HANDLE                              hHeap,
    DWORD                               dwFlags,
    LPVOID                              lpMem );

BOOL EXPORT WHeapLock (
    HANDLE                              hHeap );

LPVOID EXPORT WHeapReAlloc (
    HANDLE                              hHeap,
    DWORD                               dwFlags,
    LPVOID                              lpMem,
    DWORD                               dwBytes );

DWORD EXPORT WHeapSize (
    HANDLE                              hHeap,
    DWORD                               dwFlags,
    LPCVOID                             lpMem );

BOOL EXPORT WHeapUnlock (
    HANDLE                              hHeap );

BOOL EXPORT WHeapValidate (
    HANDLE                              hHeap,
    DWORD                               dwFlags,
    LPCVOID                             pMem );

BOOL EXPORT WHeapWalk (
    HANDLE                              hHeap,
    LPPROCESS_HEAP_ENTRY                lpEntry );

HGLOBAL EXPORT WGlobalAlloc (
    UINT                                uFlags,
    DWORD                               dwBytes );

HGLOBAL EXPORT WGlobalDiscard (
    HGLOBAL                             hMem );

UINT EXPORT WGlobalFlags (
    HGLOBAL                             hMem );                   

HGLOBAL EXPORT WGlobalFree (
    HGLOBAL                             hMem );

LPVOID EXPORT WGlobalLock (
    HGLOBAL                             hMem );

HGLOBAL EXPORT WGlobalReAlloc (
    HGLOBAL                             hMem,
    DWORD                               dwBytes,
    UINT                                uFlags );

DWORD EXPORT WGlobalSize (
    HGLOBAL                             hMem );

BOOL EXPORT WGlobalUnlock (
    HGLOBAL                             hMem );

#ifdef _cplusplus
} /*--- End extern "C" ---*/
#endif

#endif /*--- #ifndef WHEAPFUNCS_H ---*/

