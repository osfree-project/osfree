/*==============================================================================
//
//  WLVItem.cpp
//
//  @(#)WLVItem.cpp	1.3
//
//  Copyright (c) 1996 Willows Software Inc. All Rights Reserved
//
//============================================================================*/


#include "WCommCtrl.h"
#include "WLVItem.h"



CWLVItem::CWLVItem (
    LV_ITEM                           *pItem )

{

    pString    = NULL;
    Position.x =
    Position.y = CWLVID_UNASSIGNEDPOSITION;
    memset ( &LVItem, 0, sizeof ( LV_ITEM ) );
    LVItem.iImage = -1;
    if ( pItem )
        Assign ( pItem );

}



CWLVItem::~CWLVItem ( )

{

    if ( pString )
        delete [] ( pString );

}



CWLVItem& CWLVItem::Assign (
    LV_ITEM                             *pItem )

{

    if ( pItem->mask & LVIF_TEXT )
    {
        if ( pString )
        {
            delete [] ( pString );
            LVItem.pszText = NULL;
        }
        if ( ! ( pString = new char [ pItem->cchTextMax ] ) )
            Throw ( CatchBuffer, WERR_ALLOCATION );
        else
        {
            strncpy ( pString, pItem->pszText, min ( ( long )pItem->cchTextMax, ( long )strlen ( pItem->pszText ) + 1 ) );
            LVItem.pszText = pString;
        }
    }

    if ( pItem->mask & LVIF_IMAGE )
        LVItem.iImage = pItem->iImage;

    if ( pItem->mask & LVIF_PARAM ) 
        LVItem.lParam = pItem->lParam;

    if ( pItem->mask & LVIF_STATE )
        LVItem.state = pItem->state;

    LVItem.iSubItem = pItem->iSubItem;

    return ( *this );   

}



CWLVItem& CWLVItem::Assign (
    CWLVItem                            &that )

{

    Position.x = that.Position.x;
    Position.y = that.Position.y;
    return ( Assign ( &( that.LVItem ) ) );

}


void CWLVItem::Get (
    LV_ITEM                             *pItem )

{

    if ( ( pItem->mask & LVIF_TEXT ) && ( pString ) )
        strncpy ( pItem->pszText, pString, min ( ( long )pItem->cchTextMax, ( long )strlen ( pString ) + 1 ) );

    if ( pItem->mask & LVIF_IMAGE )
        pItem->iImage = LVItem.iImage;

    if ( pItem->mask & LVIF_PARAM ) 
        pItem->lParam = LVItem.lParam;

    if ( pItem->mask & LVIF_STATE )
        pItem->state = LVItem.state;

}

int CWLVItem::GetTextLength ()

{

    if ( pString )
        return ( strlen ( pString ) );
    else
        return ( 0 );

}



DWORD CWLVItem::GetItemImageIndex ()

{

    return ( ( DWORD )LVItem.iImage );

}



void CWLVItem::GetPosition (
    LPPOINT                             pPoint )

{

    pPoint->x = Position.x;
    pPoint->y = Position.y;

}



void CWLVItem::SetPosition (
    LPPOINT                             pPoint )

{

    Position.x = pPoint->x;
    Position.y = pPoint->y;

}
