/*==============================================================================
*
*   WGlobalHeap.h   - Emulates the global heap
*
*   @(#)WGlobalHeap.h	1.2 15:45:54 8/12/96 /users/sccs/src/win/utilities/s.WGlobalHeap.h
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/




#ifndef CWGLOBALHEAP_H
#define CWGLOBALHEAP_H



#include "WHeap.h"



class CWHandle;
class CWAHandle;



class CWGlobalHeap :
    private CWHeap
{

    public:

        CWGlobalHeap (
            DWORD                               dwSize );
        virtual ~CWGlobalHeap ();

        CWHandle* Alloc (
            UINT                                uFlags,
            DWORD                               dwBytes );

        UINT Flags (
            CWHandle                            *pHandle );

        CWHandle* Free (
            CWHandle                            *pHandle );

        LPBYTE Lock (
            CWHandle                            *pHandle );

        CWHandle* ReAlloc (
            CWHandle                            *pHandle,
            DWORD                               dwBytes,
            UINT                                uFlags );

        DWORD Size (
            CWHandle                            *pHandle );

        BOOL Unlock (
            CWHandle                            *pHandle );

        DWORD PointerToHandle (
            LPBYTE                              pMem,
            CWHandle                            **ppHandle );


    private:

        CWAHandle                               *pHandles;
        CWHandle                                *pFreeHandle;


};



#endif // #ifndef CWGLOBALHEAP_H
