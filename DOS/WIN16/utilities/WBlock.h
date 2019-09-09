/*==============================================================================
*
*   WBlock.h    - Blocks of Memory that describe how a CWChunk is divided up.
*
*   @(#)WBlock.h	1.3 11:06:46 9/23/96 /users/sccs/src/win/utilities/s.WBlock.h 
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/



#ifndef WBLOCK_H
#define WBLOCK_H



#include "WChunk.h"
#include "WElement.h"


#define CWB_STATUS_FREE                         0x00000000
#define CWB_STATUS_USED                         0x00000001
#define CWB_STATUS_ISLOCKABLE                   0x00000002
#define CWB_STATUS_LOCKED                       0x00000004



class EXPORT CWBlock :
    public CWElement
{

    public:

        CWBlock (
            CWChunk                             *pNewChunk,
            DWORD                               NewOffset,
            DWORD                               NewSize,
            DWORD                               NewStatus );
        CWBlock (
            CWBlock                             &that );
        virtual ~CWBlock ();

        virtual CWBlock& operator= ( 
            CWBlock                             &that );
        virtual CWElement& operator= (
            CWElement                           &that )
        {
            return ( ( CWElement& ) operator= ( ( CWBlock& ) that ) );
        }
        CWBlock* MoveData ( 
            CWBlock                             *pthat );
        LPBYTE Pointer ()
        {
            return ( (*pChunk)[ ChunkOffset ] );
        }
        CWChunk* Chunk ()
        {
            return ( pChunk );
        }
        void Chunk (
            CWChunk                             *pNewChunk )
        {
            pChunk = pNewChunk;
        }
        DWORD Offset ()
        {
            return ( ChunkOffset );
        }
        void Offset (
            DWORD                               dwOffset );
        DWORD Size ()
        {
            return ( BlockSize );
        }
        void Size (
            DWORD                               dwBytes )
        {
            BlockSize = dwBytes;
        }
        DWORD ChunkSize ()
        {
            return ( pChunk->Size() );
        }
        DWORD Status ()
        {
            return ( BlockStatus );
        }
        DWORD Status (
            DWORD                               NewStatus );
        BOOL IsUsed ()
        {
            return ( BlockStatus & CWB_STATUS_USED );
        }
        BOOL IsFree ()
        {
            return ( ! IsUsed() );
        }
        BOOL IsLocked ()
        {
            return ( ( IsUsed() ) && ( BlockStatus & CWB_STATUS_ISLOCKABLE ) && ( BlockStatus & CWB_STATUS_LOCKED ) );
        }
        BOOL IsMovable ()
        {
            return ( ( IsUsed() ) && ( BlockStatus & CWB_STATUS_ISLOCKABLE ) && ( ! ( BlockStatus & CWB_STATUS_LOCKED ) ) );
        }
        virtual int Hash ()
        {
            return ( Offset() );
        }

        
    private:

        DWORD                                   ChunkOffset;
        DWORD                                   BlockSize;
        DWORD                                   BlockStatus;
        CWChunk                                 *pChunk;

};



#endif // #ifndef WBLOCK_H

