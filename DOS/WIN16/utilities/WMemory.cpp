//==============================================================================
//
//    	@(#)WMemory.cpp	1.4
//	Memory
//
//	Copyright (c) 1996 Willows Software Inc. All Rights Reserved
//
//==============================================================================



#include "WErr.h"
#include "WMemory.h"




//-----------------------------------------------------------------------------
//
//  Handle Class
//
//  The handle class is really just a place holder in an array.  The place
//  holder is used to locate the corresponding block of memory.
//
//-----------------------------------------------------------------------------



class CWMBlock; // needed for forward declaration

class CWMHandle
{

public:

    CWMHandle ( CWMBlock                *pCBlock = NULL );
    CWMHandle ( CWMHandle               &that );
    ~CWMHandle ( ) { }

    CWMBlock* Block ( ) { return ( _pCBlock ); }
    void Block ( CWMBlock *pCBlock ) { _pcBlock = pCBlock; }

    
private:

    CWMBlock                            *_pCBlock;

}
//------------------------------------------------------------------------------
//  Usual constructor 
//------------------------------------------------------------------------------
CWMHandle::CWMHandle ( CWMBlock         *pCBlock )
{

    _pCBlock = pcBlock;

}
//------------------------------------------------------------------------------
//  Copy constructor 
//------------------------------------------------------------------------------
CWMHandle::CWMHandle ( CWMHandle        &that )
{

    _pCBlock = that._pCBlock;

}





//------------------------------------------------------------------------------
//
//  Block Class
//
//  The block class is used to define a section of memory.  The block is of
//  a given size and can either be used or free.  If the block is free then
//  its neighboring blocks should not be.  
//
//------------------------------------------------------------------------------
class CWMBlock
{

public:

    CWMBlock ( 
        CWM_HANDLE                      *pCHandle,
        UINT                            NewOffset,
        UINT                            NewSize );
    CWMBlock ( CWMBlock                 &that );
    ~CWMBlock ( ) { }

    CWM_HANDLE* CHandle ( ) { return ( _pCHandle ); }
    void CHandle ( CWM_HANDLE *pCHandle ) { _pCHandle = pCHandle; }
    UINT Offset ( ) { return ( _Offset ); }
    void Offset ( UINT NewOffset ) { _Offset = NewOffset; }
    UINT Size ( ) { return ( _Size ); }
    void Size ( UINT NewSize ) { _Size = NewSize; }
    UINT LockCount ( ) { return ( _LockCount ) }


private:

    CWM_HANDLE                          *_pCHandle;
    UINT                                _Offset;
    UINT                                _Size;
    UINT                                _LockCount;

};
//------------------------------------------------------------------------------
//  Usual constructor 
//------------------------------------------------------------------------------
CWMBlock::CWMBlock (
    CWM_HANDLE                          *pCHandle,
    UINT                                NewOffset,
    UINT                                NewSize )
{

    _pCHandle  = pCHandle;
    _Offset    = NewOffset;
    _Size      = NewSize;
    _LockCount = 0;

}
//------------------------------------------------------------------------------
//  Copy constructor 
//------------------------------------------------------------------------------
CWMBlock::CWMBlock ( CWMBlock           &that )
{

    _pCHandle  = that._pCHandle;
    _Offset    = that._Offset;
    _Size      = that._Size;
    _LockCount = that._LockCount;

}




//------------------------------------------------------------------------------
//
//  Memory
//
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//  Usual constructor 
//------------------------------------------------------------------------------
CWMemory::CWMemory ( WEM_OPTIMIZE       Style )
{

    _Style      = Style;
    _Handle     = cWM_INVALIDHANDLE;
    _ExpandSize = 1;
    _Size       = 0;
    if ( ! ( _hMemory = GlobalAlloc ( GHND, 1 ) ) )
        Throw ( CatchBuffer, WERR_ALLOCATION );
    if ( ! ( _pMemory = GlobalLock ( _hMemory ) ) )
        Throw ( CatchBuffer, WERR_LOCK );
    if ( ! ( _pBlocks = new CWList ( sizeof ( CWMBlock* ) ) ) )
        Throw ( CatchBuffer, WERR_ALLOCATION );
    if ( ! ( _pHandles = new CWList ( sizeof ( CWMHandle* ) ) ) )
        Throw ( CatchBuffer, WERR_ALLOCATION );

}
//------------------------------------------------------------------------------
//  Copy constructor 
//------------------------------------------------------------------------------
CWMemory::CWMemory ( CWMemory           &that )
{

    _Style      = that._Style;
    _Handle     = cWM_INVALIDHANDLE;
    _ExpandSize = that._ExpandSize;
    _Size       = that._Size;
    if ( ! ( _hMemory = GlobalAlloc ( GHND, _Size ) ) )
        Throw ( CatchBuffer, WERR_ALLOCATION );
    if ( ! ( _pMemory = GlobalLock ( _hMemory ) ) )
        Throw ( CatchBuffer, WERR_LOCK );
    memcpy ( pMemory, that._pMemory, Size );
    _pBlocks  = that._pBlocks;
    _pHandles = that._pHandles;

}
//------------------------------------------------------------------------------
//  Destructor
//------------------------------------------------------------------------------
CWMemory::~CWMemory ( )
{

    CWMHandle                           *pHandles;
    CWMBlock                            *pBlocks;
    UINT                                NumElements;
    UINT                                Index;

    if ( _pBlocks )
    {
        _pBlocks->Lock ( &pBlocks );
        NumElements = _pBlocks->ElementCount ();
        for ( Index = 0; Index < NumElements; Index++, pBlocks++ )
            delete ( *pBlocks );
        _pBlocks->Unlock ( &pBlocks );
        delete ( _pBlocks );
    }
    if ( _pHandles )
    {
        _pHandles->Lock ( &pHandles );
        NumElements = _pHandles->ElementCount ();
        for ( Index = 0; Index < NumElements; Index++, pHandles++ )
            delete ( pHandles
        delete ( _pHandles );
    }
    if ( _pMemory )
        GlobalUnlock ( _hMemory );
    if ( _hMemory )
        GlobalFree ( _hMemory );

}
//------------------------------------------------------------------------------
//  Private FindFreeHandle
//------------------------------------------------------------------------------
TWM_HANDLE CWMemory::FindFreeHandle ( void )

{

    CWMHandle                           *pHandles  = NULL;
    TWM_HANDLE                          FreeHandle = cWM_INVALIDHANDLE;
    UINT                                NumHandles;
    UINT                                HandleIndex;
	int				ExceptionError;
	int *			SaveCatchBuffer;


#ifdef WEXCEPTIONS
	int Error;
    try
    {
        HandleIndex = 0;
        if ( NumHandles = _pHandles->ElementCount () ) 
        {
            _pHandles->Lock ( &pHandles );
            while ( ( HandleIndex < NumHandles ) &&
                ( pHandles->Block () ) )
            {
                HandleIndex++;
                pHandles++;
            }
            _pHandles->Unlock ( &pHandles );
        }
        if ( HandleIndex == NumHandles )
        {
            pHandles = new CWMHandle ( );
            _pHandles->Insert ( HandleIndex, &pHandles );
        }
        FreeHandle = ( TWM_HANDLE )HandleIndex;
    }
    catch ( Error )
    {
        if ( Error )
            Throw ( CatchBuffer, Error );
    }
#else
	SaveCatchBuffer = CatchBuffer;
	CatchBuffer = WinMalloc( sizeof( CATCHBUF ) );
	ExceptionError = Catch( CatchBuffer );
	if ( ExceptionError == 0 )
    {
        HandleIndex = 0;
        if ( NumHandles = _pHandles->ElementCount () ) 
        {
            _pHandles->Lock ( &pHandles );
            while ( ( HandleIndex < NumHandles ) &&
                ( pHandles->Block () ) )
            {
                HandleIndex++;
                pHandles++;
            }
            _pHandles->Unlock ( &pHandles );
        }
        if ( HandleIndex == NumHandles )
        {
            pHandles = new CWMHandle ( );
            _pHandles->Insert ( HandleIndex, &pHandles );
        }
        FreeHandle = ( TWM_HANDLE )HandleIndex;
    }
	else
    {
        if ( ExceptionError )
            Throw ( CatchBuffer, ExceptionError );
    }
	WinFree( CatchBuffer );
	CatchBuffer = SaveCatchBuffer;
#endif

    return ( FreeHandle );
    
}


