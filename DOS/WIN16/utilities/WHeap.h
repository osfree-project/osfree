/*==============================================================================
*
*   WHeap.h - Simulates a heap
*
*   @(#)WHeap.h	1.3 11:18:25 9/23/96 /users/sccs/src/win/utilities/s.WHeap.h 
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/




#ifndef WHEAP_H
#define WHEAP_H



#include "WElement.h"



class CWABlock;
class CWAChunk;
class CWBlock;



class EXPORT CWHeap :
    public CWElement
{

    public:

        CWHeap (
            DWORD                               dwOptions,
            DWORD                               dwInitalSize,
            DWORD                               dwMaximumSize );
        virtual ~CWHeap ();

        virtual CWHeap& operator= (
            CWHeap                              &that );
        virtual CWElement& operator= (
            CWElement                           &that )
        {
            return ( ( CWElement& ) operator= ( ( CWHeap& ) that ) );
        }
        CWBlock* Alloc (
            DWORD                               dwFlags,
            DWORD                               dwBytes );
        CWBlock* ReAlloc (
            DWORD                               dwFlags,
            CWBlock                             *pBlock,
            DWORD                               dwBytes );
        BOOL Free (
            DWORD                               dwFlags,
            CWBlock                             *pBlock );
        DWORD Size (
            DWORD                               dwFlags,
            CWBlock                             *pBlock );
        DWORD Compact (
            DWORD                               dwBytes = 0xFFFFFFFF );
        CWBlock* PointerToBlock (
            LPBYTE                              pMem,
            LPDWORD                             pIndex = NULL );
        void Dump (
            BOOL                                Validate = FALSE );
        int Hash ()
        {
            return ( ( int ) CurrentSize );
        }
        

    private:

        DWORD                                   Options;
        DWORD                                   InitialSize;
        DWORD                                   MaximumSize;
        DWORD                                   IncrementalSize;
        DWORD                                   CurrentSize;
        CWABlock                                *pBlocks;
        CWAChunk                                *pChunks;

        BOOL AddNewChunk (
            DWORD                               dwBytes );

        DWORD CompactFreeBlocks();

};

#endif // #ifndef WHEAP_H

