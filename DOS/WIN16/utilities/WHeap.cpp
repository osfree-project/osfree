/* 
	@(#)WHeap.cpp	1.6
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*/
#include "WUtilities.h"
#include "WHeap.h"
#include "WBlock.h"
#include "WChunk.h"
#include "WABlock.h"
#include "WAChunk.h"



CWHeap::CWHeap (
    DWORD                               dwOptions,
    DWORD                               dwInitialSize,
    DWORD                               dwMaximumSize )

{

    IncrementalSize = 1;
    CurrentSize     = 0;
    Options         = dwOptions;
    InitialSize     = dwInitialSize;
    MaximumSize     = dwMaximumSize;
    pBlocks         = NULL;
    pChunks         = NULL;

    pBlocks = new CWABlock ();
    pChunks = new CWAChunk ();

    AddNewChunk( dwInitialSize );

}


CWHeap::~CWHeap ()

{

    UINT                                Index;

    if ( pBlocks )
    {
        for ( Index = 0; Index < pBlocks->Count(); Index++ )
            delete ( (*pBlocks)[Index] );
        delete ( pBlocks );
    }
    if ( pChunks )
    {
        for ( Index = 0; Index < pChunks->Count(); Index++ )
            delete ( (*pChunks)[Index] );
        delete ( pChunks );
    }

}



CWHeap& CWHeap::operator= (
    CWHeap                              &that )

{
    Throw ( CatchBuffer, WERR_INTERNAL );
    return ( *this );
}


CWBlock* CWHeap::Alloc (
    DWORD                               dwFlags,
    DWORD                               dwBytes )

{

    UINT                                BlockCount;
    UINT                                BlockIndex;
    CWBlock                             *pNewBlock;

    CWBlock                             *pBlock    = NULL;
	int				ExceptionError;
	int *				SaveCatchBuffer;



#ifdef WEXCEPTIONS
    try
    {
        for ( BlockIndex = 0, BlockCount = pBlocks->Count();
            BlockIndex < BlockCount; BlockIndex++ )
        {
            pBlock = (*pBlocks) [ BlockIndex ];
            if ( ( pBlock->IsFree() ) && ( pBlock->Size() >= dwBytes ) )
                break;
        }

        if ( ( Bl ckIndex < BlockCount ) && ( pBlock ) )
        {
            if ( pBlock->Size() > dwBytes )
            {
                pNewBlock = new CWBlock ( pBlock->Chunk(), pBlock->Offset() + pBlock->Size() - dwBytes, dwBytes, CWB_STATUS_USED );
                pBlock->Size ( pBlock->Size() - dwBytes );
                pBlocks->Insert ( BlockIndex + 1, pNewBlock );
                pBlock = pNewBlock;
            }
            else
                pBlock->Status ( CWB_STATUS_USED );
        }
        else
        {
            AddNewChunk( dwBytes );
            pBlock = Alloc ( dwFlags, dwBytes );
        }
    }
    catch ( UINT                        ExceptionError )
    {
        if ( dwFlags & HEAP_GENERATE_EXCEPTIONS )
            Throw ( CatchBuffer, ExceptionError );
        else
            pBlock = NULL;
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if (ExceptionError == 0)
    {
        for ( BlockIndex = 0, BlockCount = pBlocks->Count();
            BlockIndex < BlockCount; BlockIndex++ )
        {
            pBlock = (*pBlocks) [ BlockIndex ];
            if ( ( pBlock->IsFree() ) && ( pBlock->Size() >= dwBytes ) )
                break;
        }

        if ( ( BlockIndex < BlockCount ) && ( pBlock ) )
        {
            if ( pBlock->Size() > dwBytes )
            {
                pNewBlock = new CWBlock ( pBlock->Chunk(), pBlock->Offset() + pBlock->Size() - dwBytes, dwBytes, CWB_STATUS_USED );
                pBlock->Size ( pBlock->Size() - dwBytes );
                pBlocks->Insert ( BlockIndex + 1, pNewBlock );
                pBlock = pNewBlock;
            }
            else
                pBlock->Status ( CWB_STATUS_USED );
        }
        else
        {
            AddNewChunk( dwBytes );
            pBlock = Alloc ( dwFlags, dwBytes );
        }
    }
	else
    {
        if ( dwFlags & HEAP_GENERATE_EXCEPTIONS )
            Throw ( CatchBuffer, ExceptionError );
        else
            pBlock = NULL;
    }
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif
    if ( ( pBlock ) && ( dwFlags & HEAP_ZERO_MEMORY ) )
        memset ( pBlock->Pointer(), 0, dwBytes );

    return ( pBlock );

}



CWBlock* CWHeap::ReAlloc (
    DWORD                               dwFlags,
    CWBlock                             *pBlock,
    DWORD                               dwBytes )

{


    DWORD                               BlockIndex;
    int                                 ExceptionError;

    CWBlock                             *pPrevBlock = NULL;
    CWBlock                             *pNextBlock = NULL;
    CWBlock                             *pNewBlock  = NULL;
    int                                 ErrorCode   = 0;
	int *			SaveCatchBuffer;

    if ( pBlock->Size() == dwBytes )
        return ( pBlock );

    if ( pBlock->IsLocked() )
        ErrorCode = WERR_LOCK;
    else
#if WEXCEPTIONS
    try
    {
        BlockIndex = pBlocks->DataToIndex ( pBlock );
        if ( pBlock->Size() > dwBytes )
        {
            pNewBlock  = new CWBlock ( pBlock->Chunk(), pBlock->Offset() + dwBytes, pBlock->Size() - dwBytes, CWB_STATUS_FREE );
            pBlock->Size ( dwBytes );
            pBlocks->Insert ( BlockIndex + 1, pNewBlock );
        }
        else
        {
            if ( BlockIndex )
                pPrevBlock = (*pBlocks) [ BlockIndex - 1 ];
            if ( BlockIndex + 1 < pBlocks->Count() )
                pNextBlock = (*pBlocks) [ BlockIndex + 1 ];
            if ( ( pNextBlock ) && ( pNextBlock->IsFree() ) && ( pBlock->Size() + pNextBlock->Size() >= dwBytes ) )
            {
                if ( dwFlags & HEAP_ZERO_MEMORY )
                    memset ( pNextBlock->Pointer(), 0, pNextBlock->Size() );
                pNextBlock->Size ( pNextBlock->Size() + pBlock->Size() - dwBytes );
                pNextBlock->Offset ( pBlock->Offset() + dwBytes );
                pBlock->Size ( dwBytes );
                if ( ! pNextBlock->Size() )
                {
                    pBlocks->Delete ( BlockIndex + 1 );
                    delete ( pNextBlock );
                }
            }
            else
            if ( ( pPrevBlock ) && ( pPrevBlock->IsFree() ) && ( pBlock->Size() + pPrevBlock->Size() >= dwBytes ) )
            {
                memmove ( pPrevBlock->Pointer(), pBlock->Pointer(), pBlock->Size() );
                if ( dwFlags & HEAP_ZERO_MEMORY )
                    memset ( pPrevBlock->Pointer() + pBlock->Size(), 0, pPrevBlock->Size() );
                pBlock->Size ( pBlock->Size() + pPrevBlock->Size() - dwBytes );
                pBlock->Offset ( pPrevBlock->Offset() + dwBytes );
                pPrevBlock->Size ( dwBytes );
                pPrevBlock->Status ( pBlock->Status() );
                pBlock->Status ( CWB_STATUS_FREE );
                if ( ! pBlock->Size() )
                {
                    pBlocks->Delete ( BlockIndex );
                    delete ( pBlock );
                }
                pBlock = pPrevBlock;
            }
            else
            if ( ( pNextBlock ) && ( pNextBlock->IsFree() ) &&
                ( pPrevBlock ) && ( pPrevBlock->IsFree() ) &&
                ( pBlock->Size() + pPrevBlock->Size() + pNextBlock->Size() >= dwBytes ) )
            {
                memmove ( pPrevBlock->Pointer(), pBlock->Pointer(), pBlock->Size() );
                if ( dwFlags & HEAP_ZERO_MEMORY )
                    memset ( pPrevBlock->Pointer() + pBlock->Size(), 0, pPrevBlock->Size() + pNextBlock->Size() );
                pNextBlock->Size ( pPrevBlock->Size() + pBlock->Size() + pNextBlock->Size() - dwBytes );
                pNextBlock->Offset ( pPrevBlock->Offset() + dwBytes );
                pPrevBlock->Size ( dwBytes );
                pPrevBlock->Status ( pBlock->Status() );
                if ( ! pNextBlock->Size() )
                {
                    pBlocks->Delete ( BlockIndex + 1 );
                    delete ( pNextBlock );
                }
                pBlocks->Delete ( BlockIndex );
                delete ( pBlock );
                pBlock = pPrevBlock;
            }
            else
            if ( pNewBlock = Alloc ( dwFlags, dwBytes ) )
            {
                memmove ( pNewBlock->Pointer(), pBlock->Pointer(), pBlock->Size() );
                pNewBlock->Status ( pBlock->Status() );
                pBlock->Status ( CWB_STATUS_FREE );
                pBlock = pNewBlock;
            }
        }
    }
    catch ( ExceptionError )
    {
        if ( dwFlags & HEAP_GENERATE_EXCEPTIONS )
            Throw ( CatchBuffer, ExceptionError );
        else
            ErrorCode = ExceptionError;
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0)
    {
        BlockIndex = pBlocks->DataToIndex ( pBlock );
        if ( pBlock->Size() > dwBytes )
        {
            pNewBlock  = new CWBlock ( pBlock->Chunk(), pBlock->Offset() + dwBytes, pBlock->Size() - dwBytes, CWB_STATUS_FREE );
            pBlock->Size ( dwBytes );
            pBlocks->Insert ( BlockIndex + 1, pNewBlock );
        }
        else
        {
            if ( BlockIndex )
                pPrevBlock = (*pBlocks) [ BlockIndex - 1 ];
            if ( BlockIndex + 1 < pBlocks->Count() )
                pNextBlock = (*pBlocks) [ BlockIndex + 1 ];
            if ( ( pNextBlock ) && ( pNextBlock->IsFree() ) && ( pBlock->Size() + pNextBlock->Size() >= dwBytes ) )
            {
                if ( dwFlags & HEAP_ZERO_MEMORY )
                    memset ( pNextBlock->Pointer(), 0, pNextBlock->Size() );
                pNextBlock->Size ( pNextBlock->Size() + pBlock->Size() - dwBytes );
                pNextBlock->Offset ( pBlock->Offset() + dwBytes );
                pBlock->Size ( dwBytes );
                if ( ! pNextBlock->Size() )
                {
                    pBlocks->Delete ( BlockIndex + 1 );
                    delete ( pNextBlock );
                }
            }
            else
            if ( ( pPrevBlock ) && ( pPrevBlock->IsFree() ) && ( pBlock->Size() + pPrevBlock->Size() >= dwBytes ) )
            {
                memmove ( pPrevBlock->Pointer(), pBlock->Pointer(), pBlock->Size() );
                if ( dwFlags & HEAP_ZERO_MEMORY )
                    memset ( pPrevBlock->Pointer() + pBlock->Size(), 0, pPrevBlock->Size() );
                pBlock->Size ( pBlock->Size() + pPrevBlock->Size() - dwBytes );
                pBlock->Offset ( pPrevBlock->Offset() + dwBytes );
                pPrevBlock->Size ( dwBytes );
                pPrevBlock->Status ( pBlock->Status() );
                pBlock->Status ( CWB_STATUS_FREE );
                if ( ! pBlock->Size() )
                {
                    pBlocks->Delete ( BlockIndex );
                    delete ( pBlock );
                }
                pBlock = pPrevBlock;
            }
            else
            if ( ( pNextBlock ) && ( pNextBlock->IsFree() ) &&
                ( pPrevBlock ) && ( pPrevBlock->IsFree() ) &&
                ( pBlock->Size() + pPrevBlock->Size() + pNextBlock->Size() >= dwBytes ) )
            {
                memmove ( pPrevBlock->Pointer(), pBlock->Pointer(), pBlock->Size() );
                if ( dwFlags & HEAP_ZERO_MEMORY )
                    memset ( pPrevBlock->Pointer() + pBlock->Size(), 0, pPrevBlock->Size() + pNextBlock->Size() );
                pNextBlock->Size ( pPrevBlock->Size() + pBlock->Size() + pNextBlock->Size() - dwBytes );
                pNextBlock->Offset ( pPrevBlock->Offset() + dwBytes );
                pPrevBlock->Size ( dwBytes );
                pPrevBlock->Status ( pBlock->Status() );
                if ( ! pNextBlock->Size() )
                {
                    pBlocks->Delete ( BlockIndex + 1 );
                    delete ( pNextBlock );
                }
                pBlocks->Delete ( BlockIndex );
                delete ( pBlock );
                pBlock = pPrevBlock;
            }
            else
            if ( pNewBlock = Alloc ( dwFlags, dwBytes ) )
            {
                memmove ( pNewBlock->Pointer(), pBlock->Pointer(), pBlock->Size() );
                pNewBlock->Status ( pBlock->Status() );
                pBlock->Status ( CWB_STATUS_FREE );
                pBlock = pNewBlock;
            }
        }
    }
	else
    {
        if ( dwFlags & HEAP_GENERATE_EXCEPTIONS )
            Throw ( CatchBuffer, ExceptionError );
        else
            ErrorCode = ExceptionError;
    }
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer; 
#endif

    if ( ErrorCode )
        pBlock = NULL;

    return ( pBlock );
    
}


BOOL CWHeap::Free (
    DWORD                               dwFlags,
    CWBlock                             *pBlock )

{

    int                                 ErrorCode = 0;


    if ( pBlock->IsLocked() )
        ErrorCode = WERR_LOCK;
    else
    if ( pBlock->IsFree() )
        ErrorCode = WERR_NULL;
    else
        pBlock->Status ( ( DWORD )NULL );

    if ( ( ErrorCode ) && ( Options & HEAP_GENERATE_EXCEPTIONS ) )
        Throw ( CatchBuffer, ErrorCode );

    return ( ! ErrorCode );

}


DWORD CWHeap::Size (
    DWORD                               dwFlags,
    CWBlock                             *pBlock )

{

    DWORD                               MemSize   = 0;
    int                                 ErrorCode = 0;

    
    if ( ! ( pBlock->IsUsed() ) )
        ErrorCode = WERR_NULL;
    else
        MemSize = pBlock->Size();

    if ( ( ErrorCode ) && ( dwFlags & HEAP_GENERATE_EXCEPTIONS ) ) 
        Throw ( CatchBuffer, ErrorCode );

    return ( MemSize );

}



BOOL CWHeap::AddNewChunk ( 
    DWORD                               dwBytes )

{

    CWChunk                             *pNewChunk;
    CWBlock                             *pNewBlock;


    if ( ( MaximumSize ) && ( CurrentSize + dwBytes > MaximumSize ) )
            Throw ( CatchBuffer, WERR_ALLOCATION );
    IncrementalSize = max ( dwBytes, IncrementalSize * 2 );
    if ( ( MaximumSize ) && ( CurrentSize + IncrementalSize > MaximumSize ) )
        IncrementalSize = MaximumSize - CurrentSize;
    pNewChunk       = new CWChunk ( IncrementalSize );
    pChunks->Insert ( pChunks->Count(), pNewChunk );
    pNewBlock       = new CWBlock ( pNewChunk, 0, IncrementalSize, ( DWORD )NULL );
    pBlocks->Insert ( 0, pNewBlock );
    CurrentSize += IncrementalSize;

    return ( TRUE );

}



CWBlock* CWHeap::PointerToBlock (
    LPBYTE                              pMem,
    LPDWORD                             pIndex )

{

    UINT                                BlockCount;
    UINT                                BlockIndex;

    CWBlock                             *pBlock = NULL;


    for ( BlockIndex = 0, BlockCount = pBlocks->Count();
        BlockIndex < BlockCount; BlockIndex++ )
    {
        pBlock = (*pBlocks) [ BlockIndex ];
        if ( pMem == pBlock->Pointer() )
            break;
    }

    if ( BlockIndex == BlockCount )
        pBlock = NULL;

    if ( pIndex )
        *pIndex = BlockIndex;

    return ( pBlock );

}



DWORD CWHeap::Compact( 
    DWORD                               dwBytes )

{

    UINT                                BlockCount;
    UINT                                BlockIndex;
    UINT                                FreeIndex;
    DWORD                               LargestFreeBlock;
    DWORD                               BlockSize;
    DWORD                               BlockOffset;
    BOOL                                BlocksMoved;
    BOOL                                MoveableBlocks;

    CWBlock                             *pBlock          = NULL;
    CWBlock                             *pFreeBlock      = NULL;
    CWBlock                             *pNewBlock       = NULL;
    CWBlock                             *pNewFreeBlock   = NULL;
    DWORD                               PrevFreeBlock    = 0;


    LargestFreeBlock = CompactFreeBlocks();
    if ( LargestFreeBlock < dwBytes )    
    do
    {
        BlocksMoved      = FALSE;
        BlockCount       = pBlocks->Count();
        FreeIndex        = BlockCount ? BlockCount - 1 : 0;
        MoveableBlocks   = TRUE;
        while ( ( FreeIndex ) && ( MoveableBlocks ) )
        {
            while ( FreeIndex )
                if ( ( pFreeBlock = (*pBlocks) [ FreeIndex ] )->IsFree() ) 
                    break;
                else
                    FreeIndex--;
            MoveableBlocks = FALSE;
            for ( BlockIndex = 0; BlockIndex < FreeIndex; BlockIndex++ )
            {
                pBlock = (*pBlocks) [ BlockIndex ];
                if ( ( pBlock->IsMovable () ) &&
                    ( pFreeBlock->Size() >= pBlock->Size() ) )
                {
                    if ( pNewBlock = pBlock->MoveData ( pFreeBlock ) )
                        pBlocks->Insert ( BlockIndex++, pNewBlock );
                    BlocksMoved = TRUE;
                }
                else
                if ( pBlock->IsMovable () )
                    MoveableBlocks = TRUE;
            }
            if ( FreeIndex )
                FreeIndex--;
        }
        LargestFreeBlock = CompactFreeBlocks();
    } while ( ( dwBytes > LargestFreeBlock ) && ( BlocksMoved ) && ( MoveableBlocks ) );
    if ( dwBytes > LargestFreeBlock )
    {
        do
        {
            BlocksMoved = FALSE;
            for ( BlockIndex = 0, BlockCount = pBlocks->Count();
                BlockIndex < BlockCount; BlockIndex++ )
            {
                pBlock = (*pBlocks)[ BlockIndex ];
                if ( ( pBlock->IsMovable() ) && ( ( FreeIndex = BlockIndex + 1 ) < BlockCount ) )
                {
                    pNewFreeBlock = 
                    pFreeBlock    = (*pBlocks)[ FreeIndex ];
                    while ( ( pNewFreeBlock->IsFree() ) && ( FreeIndex < BlockCount ) )
                    {
                        if ( ++FreeIndex < BlockCount )
                        {
                            pNewFreeBlock = (*pBlocks)[ FreeIndex ];
                            if ( pNewFreeBlock->IsFree() )
                                pFreeBlock = pNewFreeBlock;
                        }
                    }
                    if ( pFreeBlock->IsFree() )
                    {
                        BlockSize = pBlock->Size();
                        memmove ( pFreeBlock->Pointer() + pFreeBlock->Size() - BlockSize,
                            pBlock->Pointer(), BlockSize );
                        BlockOffset = pBlock->Offset();
                        pBlock->Offset ( pBlock->Offset() + pFreeBlock->Size() );
                        pFreeBlock->Offset ( BlockOffset );
                        BlocksMoved = TRUE;
                    }
                }
            }
            LargestFreeBlock = CompactFreeBlocks();
        } while ( ( dwBytes > LargestFreeBlock ) && ( BlocksMoved ) );
    }
    if ( dwBytes > LargestFreeBlock )
    {
        do
        {
            BlocksMoved = FALSE;
            for ( BlockIndex = 0, BlockCount = pBlocks->Count(); 
                ( dwBytes > LargestFreeBlock ) && ( BlockIndex < BlockCount ); 
                BlockIndex++ )
            {
                pBlock = (*pBlocks)[ BlockIndex ];
                if ( ( pBlock->IsMovable() ) && ( BlockIndex ) )
                {
                    pFreeBlock = (*pBlocks) [ BlockIndex - 1 ];
                    if ( pFreeBlock->IsFree() )
                    {
                        BlockSize = pBlock->Size() + pFreeBlock->Size();
                        for ( FreeIndex = 0; FreeIndex < BlockCount; FreeIndex++ )
                        {
                            if ( FreeIndex == BlockIndex - 1 )
                                continue;
                            pNewFreeBlock = (*pBlocks)[ FreeIndex ];
                            if ( ( pNewFreeBlock->IsFree() ) && 
                                ( BlockSize > pNewFreeBlock->Size() ) &&
                                ( pBlock->Size() <= pNewFreeBlock->Size() ) )
                            {
                                if ( pNewBlock = pBlock->MoveData ( pNewFreeBlock ) )
                                    pBlocks->Insert ( BlockIndex++, pNewBlock );
                                LargestFreeBlock = CompactFreeBlocks();
                                BlockCount       = pBlocks->Count();
                                BlocksMoved      = TRUE;
                                FreeIndex        = BlockCount;
                            }
                        }
                    }
                }
            }
        } while ( ( BlocksMoved ) && ( dwBytes > LargestFreeBlock ) );
    }

    return ( LargestFreeBlock );

}



DWORD CWHeap::CompactFreeBlocks()

{

    UINT                                BlockCount;
    UINT                                BlockIndex;

    CWBlock                             *pBlock          = NULL;
    CWBlock                             *pNextBlock      = NULL;
    DWORD                               LargestFreeBlock = 0;


    pBlocks->Sort();

    for ( BlockIndex = 0, BlockCount = pBlocks->Count();
        BlockIndex < BlockCount; BlockIndex++ )
    {
        pBlock = (*pBlocks) [ BlockIndex ];
        if ( pBlock->IsFree () )
        {
            while ( ( BlockIndex + 1 < BlockCount ) && 
                ( ( pNextBlock = (*pBlocks)[ BlockIndex + 1 ] )->IsFree() ) )
            {
                pBlock->Size ( pBlock->Size() + pNextBlock->Size() );
                pBlocks->Delete ( BlockIndex + 1 );
                delete ( pNextBlock );
                BlockCount--;
            }
            if ( pBlock->Size() > LargestFreeBlock )
                LargestFreeBlock = pBlock->Size();
        }
    }

    return ( LargestFreeBlock );

}



void CWHeap::Dump ( 
    BOOL                                Validate )
{

    UINT                                BlockCount;
    UINT                                BlockIndex;

    CWBlock                             *pBlock = NULL;
    CWBlock                             *pPrevBlock = NULL;


    OutputDebugString ( "CWHeap::Dump\n" );
    for ( BlockIndex = 0, BlockCount = pBlocks->Count();
        BlockIndex < BlockCount; BlockIndex++ )
        {
            pBlock = (*pBlocks) [ BlockIndex ];
            sprintf ( WUDebugString, "[ %3d ] %10d %10d %x\n", BlockIndex, pBlock->Offset(), pBlock->Size(), pBlock->Status() );
            OutputDebugString ( WUDebugString );
            if ( ( Validate ) && ( pPrevBlock ) )
            {
                if ( pPrevBlock->Offset() + pPrevBlock->Size() != pBlock->Offset() )
                    OutputDebugString ( "***** Heap Error *****\n" );
            }
            pPrevBlock = pBlock;
        }

    OutputDebugString ( "\n" );

}


