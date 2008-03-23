
/*  WLVColumn.h	1.2 
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


#ifndef CWLVColumn_H
#define CWLVColumn_H



#include "WUtilities.h"
#include "WElement.h"


class CWALVItems;
class CWLVItem;



class CWLVColumn :
    public CWElement
{

    public:

        LV_COLUMN                               LVColumn;

        
        CWLVColumn (
            DWORD                               NumItems = 0,
            LV_COLUMN                           *pColumn = NULL );
        virtual ~CWLVColumn ();

        virtual CWLVColumn& operator= (
            CWLVColumn                          &that )
        {
            return ( Assign ( that ) );
        }
        virtual CWElement& operator= (
            CWElement                           &that )
        {
            return ( ( CWElement& ) operator= ( ( CWLVColumn& ) that ) );
        }
        virtual int Hash ()
        {
            return ( LVColumn.iSubItem );
        }
        virtual char* String ()
        {
            return ( pString );
        }
        CWLVColumn& Assign (
            LV_COLUMN                           *pLVColumn );
        void Get (
            LV_COLUMN                           *pLVColumn );
        void Set (
            LV_COLUMN                           *pLVColumn )
        {
            Assign ( pLVColumn );
        }
        void InsertItem (
            DWORD                               ItemIndex );
        void DeleteItem (
            DWORD                               ItemIndex );
        void GetItem (
            DWORD                               ItemIndex,
            LV_ITEM                             *pLVItem );
        int GetItemTextLength (
            DWORD                               ItemIndex );
        void SetItem (
            DWORD                               ItemIndex,
            LV_ITEM                             *pLVItem );
        DWORD ItemCount ();
        CWALVItems* GetItems ( )
        {
            return ( pItems );
        }
        char* GetItemString (
            DWORD                               ItemIndex );
        DWORD GetItemImageIndex (
            DWORD                               ItemIndex );
        void GetItemPosition (
            DWORD                               ItemIndex,
            LPPOINT                             pPoint );
        void SetItemPosition (
            DWORD                               ItemIndex,
            LPPOINT                             pPoint );


    
    private:

        char                                    *pString;
        CWALVItems                              *pItems;

        CWLVColumn& Assign (
            CWLVColumn                          &that );


};


#endif // #ifndef CWLVColumn_H
