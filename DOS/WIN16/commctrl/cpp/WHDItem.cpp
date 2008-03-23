/*  
	@(#)WHDItem.cpp	1.3
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
#include "WHDItem.h"



CWHDItem::CWHDItem (
    HD_ITEM                             *pItem )

{

    pString = NULL;
    memset ( &HDItem, 0, sizeof ( HD_ITEM ) );
    Assign ( pItem );

}



CWHDItem::~CWHDItem ( )

{

    if ( pString )
        delete [] ( pString );

}



CWHDItem& CWHDItem::Assign (
    HD_ITEM                             *pItem )

{

    if ( pItem->mask & HDI_TEXT )
    {
        if ( pString )
            delete [] ( pString );
        if ( ! ( pString = new char [ pItem->cchTextMax ] ) )
            Throw ( CatchBuffer, WERR_ALLOCATION );
        else
        {
            if ( pItem->fmt & HDF_STRING )
            {
                HDItem.fmt |= HDF_STRING;
                strcpy ( pString, pItem->pszText );
            }
            else
                memcpy ( pString, pItem->pszText, pItem->cchTextMax );
            HDItem.pszText = pString;
        }
    }

    if ( pItem->mask & HDI_FORMAT )
    {
        HDItem.fmt &= ~( HDF_LEFT | HDF_RIGHT | HDF_CENTER );
        pItem->fmt |= HDF_LEFT; // Default
        if ( pItem->fmt & HDF_CENTER )
            HDItem.fmt |= HDF_CENTER;
        else
        if ( pItem->fmt & HDF_RIGHT )
            HDItem.fmt |= HDF_RIGHT;
        // no need to check left since it was the default
    }

    if ( ( pItem->mask & HDI_HEIGHT ) || ( pItem->mask & HDI_WIDTH ) )
        HDItem.cxy = pItem->cxy;

    if ( pItem->mask & HDI_LPARAM )
        HDItem.lParam = pItem->lParam;

    return ( *this );   

}



CWHDItem& CWHDItem::Assign (
    CWHDItem                            &that )

{

    return ( Assign ( &( that.HDItem ) ) );

}


void CWHDItem::Get (
    HD_ITEM                             *pItem )

{

    if ( ( pItem->mask & HDI_TEXT ) && ( pString ) )
        strncpy ( pItem->pszText, pString, min ( ( long )pItem->cchTextMax, ( long )strlen ( pString ) ) );

    if ( pItem->mask & HDI_FORMAT )
        pItem->fmt = HDItem.fmt;

    if ( ( pItem->mask & HDI_HEIGHT ) || ( pItem->mask & HDI_WIDTH ) )
        pItem->cxy = HDItem.cxy;

    if ( pItem->mask & HDI_LPARAM )
        pItem->lParam = HDItem.lParam;

}

