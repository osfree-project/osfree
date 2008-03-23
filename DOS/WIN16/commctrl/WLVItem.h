
/*  WLVItem.h	1.3 
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/



#ifndef CWLVItem_H
#define CWLVItem_H



#include "WUtilities.h"
#include "WElement.h"



#define CWLVID_UNASSIGNEDPOSITION               0x8000



class CWLVItem :
    public CWElement
{

    public:

        LV_ITEM                                 LVItem;

        
        CWLVItem (
            LV_ITEM                             *pItem = NULL );
        virtual ~CWLVItem ();

        virtual CWLVItem& operator= (
            CWLVItem                            &that )
        {
            return ( Assign ( that ) );
        }
        virtual CWElement& operator= (
            CWElement                           &that )
        {
            return ( ( CWElement& ) operator= ( ( CWLVItem& ) that ) );
        }
        virtual int Hash ()
        {
            return ( LVItem.iItem << 16 + LVItem.iSubItem );
        }
        virtual char* String ()
        {
            return ( pString );
        }
        CWLVItem& Assign (
            LV_ITEM                             *pItem );
        void Get (
            LV_ITEM                             *pItem );
        void Set (
            LV_ITEM                             *pItem )
        {
            Assign ( pItem );
        }
        int MaxTextLength ()
        {
            return ( LVItem.cchTextMax );
        }
        int GetTextLength ();
        DWORD GetItemImageIndex ();
        void GetPosition (
            LPPOINT                             pPoint );
        void SetPosition (
            LPPOINT                             pPoint );

    
    private:

        char                                    *pString;
        POINT                                   Position;

        CWLVItem& Assign (
            CWLVItem                            &that );


};


#endif /* #ifndef CWLVItem_H */
