/*=============================================================================

	@(#)WALVItems.cpp	1.5 Array of Items
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
#include "WALVItems.h"



BOOL CWALVItems::Sort (
    PFNWCOMPARE                         pfnCompareProc,
    LPARAM                              lParam )

{
    int ExceptionError;
    int * SaveCatchBuffer; 

    pfnCompare = pfnCompareProc;
#ifdef WEXCEPTIONS
    try
    {
        CWArray::Sort ( 0, WAD_SORTALL, lParam );
    }
    catch ( int                         ExceptionError )
    {
        ExceptionError = 0;
        return ( FALSE );
    }
#else
    SaveCatchBuffer = CatchBuffer;
    CatchBuffer = (int *)WinMalloc(sizeof(CATCHBUF));

    ExceptionError = Catch(CatchBuffer);
    if (ExceptionError == 0)
    {
	CWArray::Sort ( 0, WAD_SORTALL, lParam );
    }
    else
    {
	WinFree(CatchBuffer);
	CatchBuffer = SaveCatchBuffer;
	return FALSE;
    }
    WinFree(CatchBuffer);
    CatchBuffer = SaveCatchBuffer; 
#endif

    return ( TRUE );

}



int CWALVItems::Compare (
    DWORD                               Index1,
    DWORD                               Index2,
    LPARAM                              lParam )

{

    CWLVItem                            *pItem1;
    CWLVItem                            *pItem2;


    if ( ! pfnCompare )
        return ( CWArray::Compare ( Index1, Index2, lParam ) );

    pItem1 = operator[]( Index1 );
    pItem2 = operator[]( Index2 );
    return ( (*pfnCompare)( pItem1->LVItem.lParam, pItem2->LVItem.lParam, lParam ) );

}



char* CWALVItems::ItemString (
    DWORD                               Index )

{

    CWLVItem                            *pItem = operator[]( Index );

    return ( pItem->String() );

}



DWORD CWALVItems::GetItemImageIndex (
    DWORD                               Index )

{

    CWLVItem                            *pItem = operator[]( Index );

    return ( pItem->GetItemImageIndex() );

}

