
/* WALVItems.h   - Array of Chunks of memory
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


#ifndef WALVITEMS_H
#define WALVITEMS_H


#include "WUtilities.h"
#include "WArray.h"
#include "WCommCtrl.h"
#include "WLVItem.h"


class EXPORT CWALVItems : 
    private CWArray
{

    public:

        CWALVItems ()
            : CWArray ( sizeof ( CWLVItem* ) )
        {
            pfnCompare = ( PFNWCOMPARE )NULL;    
        }
        virtual ~CWALVItems ()
        {
        }

        void Insert (
            DWORD                               IndexBefore,
            CWLVItem                            *pItem )
        {
            CWArray::Insert ( IndexBefore, ( CWElement** )&pItem );
        }
        void Delete (
            DWORD                               Index )
        {
            CWArray::Delete ( Index );
        }
        CWLVItem* operator[] (
            DWORD                               Index )
        {
            return ( ( ( CWLVItem* )CWArray::operator[]( Index ) ) );
        }
        DWORD Count ()
        { 
            return ( CWArray::Count() ); 
        }
        DWORD Size ()
        {
            return ( CWArray::Size() );
        }
        int Compare (
            DWORD                               Index1,
            DWORD                               Index2,
            LPARAM                              lParam );
        BOOL Sort (
            PFNWCOMPARE                         pfnCompareProc,
            LPARAM                              lParam );
        char* ItemString (
            DWORD                               Index );
        DWORD GetItemImageIndex (
            DWORD                               Index );


    private:

        PFNWCOMPARE                             pfnCompare;

};


#endif /* #ifndef WALVITEMS_H */
