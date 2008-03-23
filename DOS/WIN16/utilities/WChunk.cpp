/* 
	@(#)WChunk.cpp	1.4
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

#include "WChunk.h"



static int Initializer = 0;



CWChunk::CWChunk (

    DWORD                               dwSize,

    LPBYTE                              pNewData )



{



    if ( ! ( pChunk = ( LPBYTE )malloc ( dwSize ) ) )

		Throw ( CatchBuffer, WERR_ALLOCATION );

    AllocatedSize = dwSize;

    memset ( pChunk, Initializer++, dwSize );

    if ( pNewData )

        memcpy ( pChunk, pNewData, dwSize );



}





CWChunk::CWChunk (

    CWChunk                             &that )



{



    if ( ! ( pChunk = ( LPBYTE )malloc ( that.Size() ) ) )

		Throw ( CatchBuffer, WERR_ALLOCATION );

    AllocatedSize = that.Size();

    memcpy ( pChunk, that.pChunk, that.Size() );



}





CWChunk& CWChunk::operator= (

    CWChunk                             &that )

{



    if ( pChunk )

        free ( pChunk );

    AllocatedSize = 0;



    if ( ! ( pChunk = ( LPBYTE )malloc ( that.Size() ) ) )

		Throw ( CatchBuffer, WERR_ALLOCATION );

    AllocatedSize = that.Size();

    memcpy ( pChunk, that.pChunk, that.Size() );



    return ( *this );



}



CWChunk::~CWChunk ()



{



    if ( pChunk )

        free ( pChunk );



}



LPBYTE CWChunk::operator[] (

    DWORD                               Offset )



{



    if ( Offset >= AllocatedSize )

		Throw ( CatchBuffer, WERR_OUTOFBOUNDS );


    return ( &pChunk [ Offset ] );



}


