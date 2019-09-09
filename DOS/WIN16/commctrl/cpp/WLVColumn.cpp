/*  
	@(#)WLVColumn.cpp	1.3
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

#include "WCommCtrl.h"
#include "WLVColumn.h"
#include "WALVItems.h"
#include "WLVItem.h"



CWLVColumn::CWLVColumn (
    DWORD                               NumItems,
    LV_COLUMN                           *pColumn )

{

    CWLVItem                            *pItem;
    DWORD                               ItemIndex;


    pString = NULL;
    pItems  = NULL;
    memset ( &LVColumn, 0, sizeof ( LV_COLUMN ) );
    LVColumn.mask = 0xFFFFFFFF;

    if ( pColumn )
        Assign ( pColumn );

    pItems = new CWALVItems ();
    for ( ItemIndex = 0; ItemIndex < NumItems; ItemIndex++ )
    {
        pItem = new CWLVItem ();
        pItems->Insert ( ItemIndex, pItem );
    }

}



CWLVColumn::~CWLVColumn ( )

{

    DWORD                               ItemCount;
    CWLVItem                            *pItem;

    if ( pString )
        delete [] ( pString );

    if ( ( pItems ) && ( ItemCount = pItems->Count() ) )
    {
        for ( /*-- No Initialization --*/; ItemCount--; /*-- No Itteration --*/ )
        {
            pItem = (*pItems) [ ItemCount ];
            delete ( pItem );
        }
    }

}



CWLVColumn& CWLVColumn::Assign (
    LV_COLUMN                             *pLVColumn )

{

    if ( pLVColumn->mask & LVCF_TEXT )
    {
        if ( pString )
        {
            delete [] ( pString );
            LVColumn.pszText = NULL;
        }
        if ( ! ( pString = new char [ pLVColumn->cchTextMax ] ) )
            Throw ( CatchBuffer, WERR_ALLOCATION );
        else
        {
            strncpy ( pString, pLVColumn->pszText, min ( ( long )pLVColumn->cchTextMax, ( long )strlen ( pLVColumn->pszText ) ) );
            LVColumn.pszText = pString;
        }
    }

    if ( pLVColumn->mask & LVCF_FMT )
        LVColumn.fmt = pLVColumn->fmt;

    if ( pLVColumn->mask & LVCF_WIDTH ) 
        LVColumn.cx = pLVColumn->cx;

    if ( pLVColumn->mask & LVCF_SUBITEM )
        LVColumn.iSubItem = pLVColumn->iSubItem;

    return ( *this );   

}



CWLVColumn& CWLVColumn::Assign (
    CWLVColumn                            &that )

{

    return ( Assign ( &( that.LVColumn ) ) );

}


void CWLVColumn::Get (
    LV_COLUMN                             *pLVColumn )

{

    if ( ( pLVColumn->mask & LVCF_TEXT ) && ( pString ) )
        strncpy ( pLVColumn->pszText, pString, min ( ( long )pLVColumn->cchTextMax, ( long )strlen ( pString ) ) );

    if ( pLVColumn->mask & LVCF_FMT )
        pLVColumn->fmt = LVColumn.fmt;

    if ( pLVColumn->mask & LVCF_WIDTH )
        pLVColumn->cx = LVColumn.cx;

    if ( pLVColumn->mask & LVCF_SUBITEM )
        pLVColumn->iSubItem = LVColumn.iSubItem;

}



void CWLVColumn::GetItem (
    DWORD                               ItemIndex,
    LV_ITEM                             *pLVItem )

{

    ( (*pItems)[ ItemIndex ] )->Get ( pLVItem );

}



int CWLVColumn::GetItemTextLength (
    DWORD                               ItemIndex )

{

    return ( (*pItems)[ ItemIndex ]->GetTextLength() );

}


void CWLVColumn::SetItem (
    DWORD                               ItemIndex,
    LV_ITEM                             *pLVItem )

{

    ( (*pItems)[ ItemIndex ] )->Set ( pLVItem );

}


DWORD CWLVColumn::ItemCount ()

{

    return ( pItems->Count() );

}



void CWLVColumn::InsertItem (
    DWORD                               ItemIndex )

{

    pItems->Insert ( ItemIndex, new CWLVItem() );

}



void CWLVColumn::DeleteItem (
    DWORD                               ItemIndex )

{

    CWLVItem                            *pItem;


    pItem = (*pItems)[ ItemIndex ];
    pItems->Delete ( ItemIndex );
    delete ( pItem );

}



char* CWLVColumn::GetItemString (
    DWORD                               ItemIndex )
{
    return ( pItems->ItemString ( ItemIndex ) );
}


DWORD CWLVColumn::GetItemImageIndex (
    DWORD                               ItemIndex )
{
    return ( pItems->GetItemImageIndex ( ItemIndex ) );
}


void CWLVColumn::GetItemPosition (
    DWORD                               ItemIndex,
    LPPOINT                             pPoint )

{

    CWLVItem                            *pItem;


    pItem = (*pItems)[ ItemIndex ];
    pItem->GetPosition ( pPoint );

}


void CWLVColumn::SetItemPosition (
    DWORD                               ItemIndex,
    LPPOINT                             pPoint )

{

    CWLVItem                            *pItem;


    pItem = (*pItems)[ ItemIndex ];
    pItem->SetPosition ( pPoint );

}
