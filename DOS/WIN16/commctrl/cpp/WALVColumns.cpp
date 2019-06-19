/*==============================================================================

	WALVColumns.h   1.2 - Array of LV_COLUMNs
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

==============================================================================*/



#include "WCommCtrl.h"
#include "WALVColumns.h"
#include "WALVItems.h"
#include "WLVItem.h"



DWORD CWALVColumns::ItemCount ()

{

    if ( ColumnCount() )
        return ( ( operator[] ( 0 ) )->ItemCount() );
    else
        return ( 0 );

}



void CWALVColumns::InsertColumn (
    DWORD                               IndexBefore,
    LV_COLUMN                           *pLVColumn )

{

    CWLVColumn                          *pColumn;
    
    
    pColumn = new CWLVColumn ( ItemCount (), pLVColumn );
    Insert ( IndexBefore, pColumn );

}

void CWALVColumns::DeleteColumn (
    DWORD                               ColumnIndex )

{

    CWLVColumn                          *pColumn;


    pColumn = ( operator[]( ColumnIndex ) );
    delete ( pColumn );
    Delete ( ColumnIndex );

}


void CWALVColumns::GetColumn (
    DWORD                               ColumnIndex,
    LV_COLUMN                           *pLVColumn )

{

    CWLVColumn                          *pColumn;


    pColumn = ( operator[]( ColumnIndex ) );
    pColumn->Get ( pLVColumn );

}


void CWALVColumns::SetColumn (
    DWORD                               ColumnIndex,
    LV_COLUMN                           *pLVColumn )

{

    CWLVColumn                          *pColumn;


    pColumn = operator[]( ColumnIndex );
    pColumn->Set ( pLVColumn );

}


void CWALVColumns::InsertItemRow (
    DWORD                               IndexBefore )

{

    DWORD                               Column;
    DWORD                               ColumnCount;


    for ( Column = 0, ColumnCount = Count(); Column < ColumnCount; Column++ )
        ( operator[] ( Column ) )->InsertItem ( IndexBefore );

}



void CWALVColumns::DeleteItemRow (
    DWORD                               RowIndex )

{

    DWORD                               Column;
    DWORD                               ColumnCount;


    for ( Column = 0, ColumnCount = Count(); Column < ColumnCount; Column++ )
        ( operator[] ( Column ) )->DeleteItem ( RowIndex );

}



void CWALVColumns::GetItem (
    DWORD                               ColumnIndex,
    DWORD                               ItemIndex,
    LV_ITEM                             *pLVItem )

{

    CWLVColumn                          *pColumn;


    pColumn = ( operator[]( ColumnIndex ) );
    pColumn->GetItem ( ItemIndex, pLVItem );

}


int CWALVColumns::GetItemTextLength (
    DWORD                               ColumnIndex,
    DWORD                               ItemIndex )

{

    CWLVColumn                          *pColumn;


    pColumn = ( operator[]( ColumnIndex ) );
    return ( pColumn->GetItemTextLength ( ItemIndex ) );

}

void CWALVColumns::SetItem (
    DWORD                               ColumnIndex,
    DWORD                               ItemIndex,
    LV_ITEM                             *pLVItem )

{

    CWLVColumn                          *pColumn;


    pColumn = ( operator[]( ColumnIndex ) );
    pColumn->SetItem ( ItemIndex, pLVItem );

}


char* CWALVColumns::GetItemString (
    DWORD                               ColumnIndex,
    DWORD                               ItemIndex )

{

    CWLVColumn                          *pColumn;


    pColumn = ( operator[]( ColumnIndex ) );
    return ( pColumn->GetItemString ( ItemIndex ) );

}


DWORD CWALVColumns::GetItemImageIndex (
    DWORD                               ColumnIndex,
    DWORD                               ItemIndex )

{

    CWLVColumn                          *pColumn;


    pColumn = ( operator[]( ColumnIndex ) );
    return ( pColumn->GetItemImageIndex ( ItemIndex ) );

}


void CWALVColumns::GetItemPosition (
    DWORD                               ColumnIndex,
    DWORD                               ItemIndex,
    LPPOINT                             pPoint )

{

    CWLVColumn                          *pColumn;


    pColumn = ( operator[] ( ColumnIndex ) );
    pColumn->GetItemPosition ( ItemIndex, pPoint );

}


void CWALVColumns::SetItemPosition (
    DWORD                               ColumnIndex,
    DWORD                               ItemIndex,
    LPPOINT                             pPoint )

{

    CWLVColumn                          *pColumn;


    pColumn = ( operator[] ( ColumnIndex ) );
    pColumn->SetItemPosition ( ItemIndex, pPoint );

}
