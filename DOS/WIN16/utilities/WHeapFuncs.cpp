/* 
	@(#)WHeapFuncs.cpp	1.3
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
#include "WHeapFuncs.h"
#include "WGlobalHeap.h"
#include "WHandle.h"



static CWGlobalHeap                     WGlobalHeap ( 0x2FF );


#ifdef _cplusplus
extern "C" {
#endif

LPVOID EXPORT WHeapAlloc (
    HANDLE                              hHeap,
    DWORD                               dwFlags,
    DWORD                               dwBytes )
{

    CWBlock                             *pBlock;
    LPVOID                              pMem = NULL;
    

    if ( pBlock = ( ( CWHeap* )hHeap )->Alloc ( dwFlags, dwBytes ) )
        pMem = pBlock->Pointer();

    return ( pMem );

}

UINT EXPORT WHeapCompact (
    HANDLE                              hHeap,
    DWORD                               dwFlags )
{
    return ( 0 );
}

HANDLE EXPORT WHeapCreate (
    DWORD                               flOptions,
    DWORD                               dwInitialSize,
    DWORD                               dwMaximumSize )
{
    return ( ( HANDLE )new CWHeap ( flOptions, dwInitialSize, dwMaximumSize ) );
}

BOOL EXPORT WHeapDestroy (
    HANDLE                              hHeap )
{
    delete ( ( CWHeap* ) hHeap );
    return ( TRUE );
}

BOOL EXPORT WHeapFree (
    HANDLE                              hHeap,
    DWORD                               dwFlags,
    LPVOID                              lpMem )
{

    CWBlock                             *pBlock;

    CWHeap                              *pHeap = ( CWHeap* )hHeap;


    if ( pBlock = pHeap->PointerToBlock ( ( LPBYTE )lpMem ) )
        return ( pHeap->Free ( dwFlags, pBlock ) );
    else
        return ( FALSE );

}

BOOL EXPORT WHeapLock (
    HANDLE                              hHeap )
{
#ifdef LATER
    printf ( "No Heap functions thread support.\n" );
#endif
    return ( FALSE );
}

LPVOID EXPORT WHeapReAlloc (
    HANDLE                              hHeap,
    DWORD                               dwFlags,
    LPVOID                              lpMem,
    DWORD                               dwBytes )
{

    CWBlock                             *pBlock;

    CWHeap                              *pHeap = ( CWHeap* )hHeap;
    LPVOID                              pMem   = NULL;


    if ( pBlock = pHeap->PointerToBlock ( ( LPBYTE )lpMem ) )
        if ( pBlock = pHeap->ReAlloc ( dwFlags, pBlock, dwBytes ) )
            pMem = pBlock->Pointer();

    return ( pMem );

}

DWORD EXPORT WHeapSize (
    HANDLE                              hHeap,
    DWORD                               dwFlags,
    LPCVOID                             lpMem )
{

    CWBlock                             *pBlock;

    CWHeap                              *pHeap = ( CWHeap* )hHeap;

    
    if ( pBlock = pHeap->PointerToBlock ( ( LPBYTE )lpMem ) )
        return ( pBlock->Size() );
    else
        return ( 0 );

}

BOOL EXPORT WHeapUnlock (
    HANDLE                              hHeap )
{
#ifdef LATER
    printf ( "No Heap functions thread support.\n" );
#endif
    return ( FALSE );
}


BOOL EXPORT WHeapValidate (
    HANDLE                              hHeap,
    DWORD                               dwFlags,
    LPCVOID                             pMem )
{
#ifdef LATER
#endif
    return ( FALSE );
}


BOOL EXPORT WHeapWalk (
    HANDLE                              hHeap,
    LPPROCESS_HEAP_ENTRY                lpEntry )
{
#ifdef LATER
#endif
    return ( FALSE );
}

HGLOBAL EXPORT WGlobalAlloc (
    UINT                                uFlags,
    DWORD                               dwBytes )

{

    return ( ( HGLOBAL )WGlobalHeap.Alloc ( uFlags, dwBytes ) );

}

HGLOBAL EXPORT WGlobalDiscard (
    HGLOBAL                             hMem )

{
#ifdef LATER
#endif
    return ( NULL );
}

UINT EXPORT WGlobalFlags (
    HGLOBAL                             hMem )

{
#ifdef LATER
#endif 
    return ( NULL );
}

HGLOBAL EXPORT WGlobalFree (
    HGLOBAL                             hMem )
{

    return ( ( HGLOBAL )WGlobalHeap.Free ( ( CWHandle*) hMem ) );

}

LPVOID EXPORT WGlobalLock (
    HGLOBAL                             hMem )

{

    return ( ( LPVOID )WGlobalHeap.Lock ( ( CWHandle* )hMem ) );

}

HGLOBAL EXPORT WGlobalReAlloc (
    HGLOBAL                             hMem,
    DWORD                               dwBytes,
    UINT                                uFlags )

{

    return ( ( HGLOBAL )WGlobalHeap.ReAlloc ( ( CWHandle* )hMem, dwBytes, uFlags ) );

}

DWORD EXPORT WGlobalSize (
    HGLOBAL                             hMem )

{

    return ( WGlobalHeap.Size ( ( CWHandle* )hMem ) );

}

BOOL EXPORT WGlobalUnlock (
    HGLOBAL                             hMem )

{

    return ( WGlobalHeap.Unlock ( ( CWHandle* )hMem ) );

}

#ifdef _cplusplus
} // #ifdef _cplusplus extern "C"
#endif


