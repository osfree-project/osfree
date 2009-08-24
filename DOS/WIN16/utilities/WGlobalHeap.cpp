/* 
	@(#)WGlobalHeap.cpp	1.7
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
#include "WGlobalHeap.h"
#include "WHandle.h"
#include "WBlock.h"
#include "WAHandle.h"



const DWORD CWHD_INVALID = 0xFFFFFFFF;



CWGlobalHeap::CWGlobalHeap (
    DWORD                               dwSize )
    : CWHeap ( HEAP_GENERATE_EXCEPTIONS, dwSize, dwSize )
{

    pHandles    = new CWAHandle ();
    pFreeHandle = NULL;

}

CWGlobalHeap::~CWGlobalHeap ()

{
    
    DWORD                               HandleIndex;
    DWORD                               HandleCount;
    CWHandle                            *pHandle;
    
    
    if ( pHandles )
    {
        for ( HandleIndex = 0, HandleCount = pHandles->Count();
            HandleIndex < HandleCount; HandleIndex++ )
        {
            pHandle = (*pHandles) [ HandleIndex ];
            delete ( pHandle );
        }
        delete ( pHandles );
    }

}



DWORD CWGlobalHeap::PointerToHandle (
    LPBYTE                              pMem,
    CWHandle                            **ppHandle )

{

    DWORD                               HandleIndex;
    DWORD                               HandleCount;
    CWHandle                            *pHandle;

    CWBlock                             *pBlock = NULL;


    for ( HandleIndex = 0, HandleCount = pHandles->Count();
        HandleIndex < HandleCount; HandleIndex++ )
    {
        pHandle = (*pHandles) [ HandleIndex ];
        if ( ( pBlock = pHandle->Block() ) &&
            ( pBlock->Pointer() == pMem ) )
            break;
    }

    if ( ( HandleIndex < HandleCount ) && 
        ( pBlock ) && ( pBlock->Pointer() == pMem ) )
        *ppHandle = pHandle;
    else
        *ppHandle = ( CWHandle* )pMem;

    return ( HandleIndex < HandleCount ? HandleIndex : CWHD_INVALID );

}

CWHandle* CWGlobalHeap::Alloc (
    UINT                                uFlags,
    DWORD                               dwBytes )

{

    DWORD                               HandleIndex;
    DWORD                               HandleCount;

    DWORD                               Flag     = 0;
    CWHandle                            *pHandle = NULL;
    CWBlock                             *pBlock  = NULL;
	int				ExceptionError, ExceptionError2;
	int *				SaveCatchBuffer, * SaveCatchBuffer2; 


    if ( ! pFreeHandle )
    {
        for ( HandleIndex = 0, HandleCount = pHandles->Count();
            HandleIndex < HandleCount; HandleIndex++ )
        {
            pFreeHandle = (*pHandles) [ HandleIndex ];
            if ( ! pFreeHandle->Block() )
                break;
        }
        if ( HandleIndex >= HandleCount )
        {
            pFreeHandle = new CWHandle ( uFlags );
            pHandles->Insert ( HandleCount, pFreeHandle );
        }
    }
    if ( uFlags & GMEM_ZEROINIT )
        Flag |= HEAP_ZERO_MEMORY;
#ifdef WEXCEPTIONS
    try
    {
        if ( pBlock = CWHeap::Alloc ( Flag, dwBytes ) )
        {
            pFreeHandle->Block ( pBlock );
            pFreeHandle->Flags ( uFlags );
            pHandle     = pFreeHandle;
            pFreeHandle = NULL;
            if ( uFlags & GMEM_MOVEABLE )
                pBlock->Status ( pBlock->Status() | CWB_STATUS_ISLOCKABLE );
        }
    }
    catch ( int                         ExceptionError )
    {
        switch ( ExceptionError )
        {
            default:
                pHandle = NULL;
                break;

            case WERR_ALLOCATION:
                if ( ! ( uFlags & GMEM_NODISCARD ) )
                try
                {
                    CWHeap::Compact( dwBytes );
                    pBlock = CWHeap::Alloc ( Flag, dwBytes );
                    pFreeHandle->Block ( pBlock );
                    pFreeHandle->Flags ( uFlags );
                    pHandle     = pFreeHandle;
                    pFreeHandle = NULL;
                    if ( uFlags & GMEM_MOVEABLE )
                        pBlock->Status ( pBlock->Status() | CWB_STATUS_ISLOCKABLE );
                }
                catch ( int             ExceptionError2 )
                {
                    ExceptionError2 = 0; // this line here to remove waring for unreference local variable
                    pHandle = NULL;
                }
        }
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if( ExceptionError == 0 )
	{
		if ( pBlock = CWHeap::Alloc ( Flag, dwBytes ) )
        	{
	        	pFreeHandle->Block ( pBlock );
                	pFreeHandle->Flags ( uFlags );
                	pHandle     = pFreeHandle;
                	pFreeHandle = NULL;
                	if ( uFlags & GMEM_MOVEABLE )
                	pBlock->Status ( pBlock->Status() | CWB_STATUS_ISLOCKABLE );
                }
	}
	else
	{
		switch ( ExceptionError )
        	{
        	default:
        		pHandle = NULL;
        		break;

	        case WERR_ALLOCATION:
                	if ( ! ( uFlags & GMEM_NODISCARD ) )
                	{
				SaveCatchBuffer2 = CatchBuffer;
				CatchBuffer = (int *)WinMalloc( sizeof( CATCHBUF ) );
				ExceptionError2 = Catch( CatchBuffer ); 
				if (ExceptionError2 == 0)
                		{
                    			CWHeap::Compact( dwBytes );
                    			pBlock = CWHeap::Alloc ( Flag, dwBytes );
                    			pFreeHandle->Block ( pBlock );
                    			pFreeHandle->Flags ( uFlags );
                    			pHandle     = pFreeHandle;
                    			pFreeHandle = NULL;
                    			if ( uFlags & GMEM_MOVEABLE )
                        			pBlock->Status ( pBlock->Status() | 
							CWB_STATUS_ISLOCKABLE );
                		}
				else
                		{
                        		ExceptionError2 = 0; 
                        		pHandle = NULL;
                		}
				WinFree( CatchBuffer );
				CatchBuffer = SaveCatchBuffer2;
			}
		}
	}
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    if ( ( pHandle ) && ( ! ( uFlags & GMEM_MOVEABLE ) ) )
        pHandle = ( CWHandle* )( ( pHandle->Block() )->Pointer() );

    return ( pHandle );

}



UINT CWGlobalHeap::Flags (
    CWHandle                            *pHandle )
{
    return ( 0 );
}



CWHandle* CWGlobalHeap::Free (
    CWHandle                            *pHandle )

{

    CWBlock                             *pBlock;


    PointerToHandle ( ( LPBYTE )pHandle, &pHandle );
    if ( pBlock = pHandle->Block () )
    {
        if ( ( pBlock->IsUsed() ) || ( ! pHandle->LockCount() ) )
        {
            pBlock->Status ( ( DWORD )NULL );
            pHandle->Block ( NULL );
            pHandle->Flags ( ( DWORD )NULL );
            pFreeHandle = pHandle;
            pHandle     = NULL;
        }
    }

    return ( pHandle );

}



LPBYTE CWGlobalHeap::Lock (
    CWHandle                            *pHandle )
{

    CWBlock                             *pBlock;
    CWHandle                            *pRealHandle;

    LPBYTE                              pMem        = NULL;


    if ( PointerToHandle ( ( LPBYTE )pHandle, &pRealHandle ) != CWHD_INVALID )
        pMem = ( LPBYTE ) pHandle;
    else
        if ( pBlock = pHandle->Block() )
        {
            pBlock->Status ( pBlock->Status() | CWB_STATUS_LOCKED );
            pMem = pBlock->Pointer();
            (*pHandle)++;
        }

    return ( pMem );

}



CWHandle* CWGlobalHeap::ReAlloc (
    CWHandle                            *pHandle,
    DWORD                               dwBytes,
    UINT                                uFlags )

{

    CWBlock                             *pBlock;

    DWORD                               Flag = 0;


    PointerToHandle ( ( LPBYTE )pHandle, &pHandle );

    if ( uFlags & GMEM_ZEROINIT )
        Flag |= HEAP_ZERO_MEMORY;

    if ( pBlock = CWHeap::ReAlloc ( Flag, pHandle->Block(), dwBytes ) )
    {
        pHandle->Block ( pBlock );
        if ( uFlags & GMEM_MOVEABLE )
            pBlock->Status ( pBlock->Status() | CWB_STATUS_ISLOCKABLE );
    }
    else
    {
        CWHeap::Compact( dwBytes );
        if ( pBlock = CWHeap::Alloc ( Flag, dwBytes ) )
        {
            pHandle->Block ( pBlock );
            if ( uFlags & GMEM_MOVEABLE )
                pBlock->Status ( pBlock->Status() | CWB_STATUS_ISLOCKABLE );
        }
        else
            pHandle = NULL;
    }

    if ( ( pHandle ) && ( uFlags & GMEM_FIXED ) )
        pHandle = ( CWHandle* )( ( pHandle->Block() )->Pointer() );

    return ( pHandle );

}



DWORD CWGlobalHeap::Size (
    CWHandle                            *pHandle )

{

    CWBlock                             *pBlock;


    PointerToHandle ( ( LPBYTE )pHandle, &pHandle );
    if ( pBlock = pHandle->Block() )
        return ( pBlock->Size () );
    else
        return ( 0 );

}



BOOL CWGlobalHeap::Unlock (
    CWHandle                            *pHandle )

{

    CWBlock                             *pBlock;


    if ( PointerToHandle ( ( LPBYTE )pHandle, &pHandle ) != CWHD_INVALID )
        return ( TRUE );

    if ( ! ( pHandle->LockCount() ) )
        return ( FALSE );

    (*pHandle)--;

    if ( ! ( pHandle->LockCount() ) )
    {
        pBlock = pHandle->Block();
        pBlock->Status ( pBlock->Status() & ~CWB_STATUS_LOCKED );
    }

    return ( TRUE );

}


