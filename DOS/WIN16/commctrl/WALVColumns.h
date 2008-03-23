
/* WALVColumns.h   - Array of Chunks of memory
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



#ifndef WACOLUMN_H
#define WACOLUMN_H


#include "WUtilities.h"
#include "WArray.h"
#include "WCommCtrl.h"
#include "WLVColumn.h"


class EXPORT CWALVColumns : 
    private CWArray
{

    public:

        CWALVColumns ()
            : CWArray ( sizeof ( CWLVColumn* ) )
        {
        }
        virtual ~CWALVColumns ()
        {
        }

        CWLVColumn* operator[] (
            DWORD                               Index )
        {
            return ( ( ( CWLVColumn* )CWArray::operator[]( Index ) ) );
        }
        DWORD ColumnCount ()
        { 
            return ( CWArray::Count() ); 
        }
        DWORD ItemCount ();
        void InsertColumn (
            DWORD                               IndexBefore,
            LV_COLUMN                           *pLVColumn );
        void DeleteColumn (
            DWORD                               ColumnIndex );
        void GetColumn (
            DWORD                               ColumnIndex,
            LV_COLUMN                           *pLVColumn );
        void SetColumn (
            DWORD                               ColumnIndex,
            LV_COLUMN                           *pLVColumn );
        void InsertItemRow (
            DWORD                               IndexBefore );
        void DeleteItemRow (
            DWORD                               RowIndex );
        void GetItem (
            DWORD                               ColumnIndex,
            DWORD                               ItemIndex,
            LV_ITEM                             *pLVItem );
        int GetItemTextLength (
            DWORD                               ColumnIndex,
            DWORD                               ItemIndex );
        void SetItem (
            DWORD                               ColumnIndex,
            DWORD                               ItemIndex,
            LV_ITEM                             *pLVItem );
        char* GetItemString (
            DWORD                               ColumnIndex,
            DWORD                               ItemIndex );
        DWORD GetItemImageIndex (
            DWORD                               ColumnIndex,
            DWORD                               ItemIndex );
        void SetItemPosition (
            DWORD                               ColumnIndex,
            DWORD                               ItemIndex,
            LPPOINT                             pPoint );
        void GetItemPosition (
            DWORD                               ColumnIndex,
            DWORD                               ItemIndex,
            LPPOINT                             pPoint );



    private:

        void Insert (
            DWORD                               IndexBefore,
            CWLVColumn                          *pColumn )
        {
            CWArray::Insert ( IndexBefore, ( CWElement** )&pColumn );
        }
        void Delete (
            DWORD                               Index )
        {
            CWArray::Delete ( Index );
        }

};


#endif /* #ifndef WACOLUMN_H */
