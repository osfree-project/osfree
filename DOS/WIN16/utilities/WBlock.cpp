/* 
	@(#)WBlock.cpp	1.4
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
#include "WBlock.h"



CWBlock::CWBlock (
    CWChunk                             *pNewChunk,
    DWORD                               NewOffset,
    DWORD                               NewSize,
    DWORD                               NewStatus )

{

    pChunk      = pNewChunk;
    ChunkOffset = NewOffset;
    BlockSize   = NewSize;
    BlockStatus = NewStatus;

}

CWBlock::CWBlock (
    CWBlock                             &that )

{

    *this = that;

}


CWBlock::~CWBlock ()
{
}


CWBlock& CWBlock::operator= ( CWBlock   &that )
{
    this->Chunk ( that.Chunk() );
    this->Offset ( that.Offset() );
    this->Size ( that.Size() );
    this->Status ( that.Status() );

    return ( *this );
}


CWBlock* CWBlock::MoveData ( CWBlock    *pthat )
{

    CWBlock                             TempThisBlock = *this;
    CWBlock                             TempThatBlock = *pthat;
    CWBlock                             *pNewBlock    = NULL;


    if ( this->Size() == pthat->Size() )
    {
        memmove ( pthat->Pointer(), this->Pointer(), this->Size() );
        *this = *pthat;
        this->Status ( TempThisBlock.Status() );
        *pthat = TempThisBlock;
        pthat->Status ( TempThatBlock.Status() );
    }
    else
    {
        memmove ( pthat->Pointer() + pthat->Size() - this->Size(),
            this->Pointer(), this->Size() );
        pNewBlock = new CWBlock ( this->Chunk(), this->Offset(), this->Size(), CWB_STATUS_FREE );
        this->Chunk ( pthat->Chunk() );
        this->Offset ( pthat->Offset() + pthat->Size() - this->Size() );
        pthat->Size ( pthat->Size() - this->Size() );
    }

    return ( pNewBlock );

}


DWORD CWBlock::Status (
    DWORD                               NewStatus )

{

    DWORD                               OldStatus = BlockStatus;

    BlockStatus = NewStatus;

    return ( OldStatus );

}


void CWBlock::Offset (
    DWORD                               dwOffset )
{
    DWORD ChunkSize = pChunk->Size();

    if ( dwOffset >= pChunk->Size() )
        Throw( CatchBuffer, WERR_OUTOFBOUNDS );
    else
        ChunkOffset = dwOffset;
}


