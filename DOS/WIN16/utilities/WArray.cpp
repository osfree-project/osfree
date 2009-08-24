/*==============================================================================

*

*   WArray.cpp  - General purpose array

*

*   @(#)WArray.cpp	1.5 12:28:39 11/11/96 /users/sccs/src/win/utilities/s.WArray.cpp

*

*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.

*



    The array consists of a chunk of memory that is dynamically allocated at

    runtime.  This allows the array to grow as required.  The the currently

    allocated chunk is not big enough then a new larger chunk is allocated

    and all the original data is copied over.  Because of this any pointers

    to data contained within the array may be invalid after an insert.  Note

    that this design is to be used not to store the data for each element but

    rather the pointer to each element.  This means that no matter how large

    the data only the pointer needs to be remembered.  This also makes is very

    easy when data needs to be shuffled around during an insert or delete.

*

==============================================================================*/









#include "WUtilities.h"

#include <stdio.h>

#include "WChunk.h"

#include "WArray.h"





/*------------------------------------------------------------------------------

*

*   Constructor for CWArray

*

------------------------------------------------------------------------------*/

CWArray::CWArray (

    DWORD                               InitialElementSize,

    DWORD                               InitialElementCount )

{



    ElementCount      = InitialElementCount;

    ElementSize       = InitialElementSize;

    ElementsAllocated = 0;

    IncrementCount    = 1;

    pChunk            = NULL;



    /*--- Allocate enough space for atleast one element ---*/

    if ( ! ( pChunk = new CWChunk ( ( ElementCount ? ElementCount : 1 ) * ElementSize ) ) )

        Throw ( CatchBuffer, WERR_ALLOCATION );

    else

        ElementsAllocated = ( ElementCount ? ElementCount : 1 );



}

/*------------------------------------------------------------------------------

*

*   Constructor for CWArray

*

------------------------------------------------------------------------------*/

CWArray::~CWArray ()

{



    /*--- Delete the chunk of memory if there is one ---*/

    if ( pChunk )

        delete ( pChunk );



}







void CWArray::Insert (

    DWORD                               IndexBefore,

    CWElement                           **ppElement )

{



    CWChunk                            *pNewChunk;





    if ( IndexBefore > ElementCount )

	 	Throw( CatchBuffer, WERR_OUTOFBOUNDS );

    else

    {

        if ( ElementCount < ElementsAllocated )

        {

            if ( IndexBefore < ElementCount )

                memmove ( (*pChunk) [ ( IndexBefore + 1 ) * ElementSize ], 

                    (*pChunk) [ IndexBefore * ElementSize ], 

                    ( ElementCount - IndexBefore ) * ElementSize );

            memcpy ( (*pChunk) [ IndexBefore * ElementSize ], 

                ( LPBYTE )ppElement, ElementSize );

            ElementCount++;

        }

        else

        {

            IncrementCount *= 2;

            if ( ! ( pNewChunk = new CWChunk ( ( ElementsAllocated + IncrementCount ) * ElementSize ) ) )

				Throw ( CatchBuffer, WERR_ALLOCATION );

            else

            {

                memcpy ( (*pNewChunk)[ 0 ], (*pChunk) [ 0 ], IndexBefore * ElementSize );

                memcpy ( (*pNewChunk)[ IndexBefore * ElementSize ], 

                    ( LPBYTE )ppElement, ElementSize );

                if ( IndexBefore < ElementCount )

                    memcpy ( (*pNewChunk)[ ( IndexBefore + 1 ) * ElementSize ], 

                        (*pChunk) [ IndexBefore * ElementSize ], 

                        ( ElementCount - IndexBefore ) * ElementSize );

                ElementCount++;

                ElementsAllocated += IncrementCount;

                delete ( pChunk );

                pChunk = pNewChunk;

            }

        }

    }

}



void CWArray::Delete (

    DWORD                               Index )

{



    if ( ( Index >= ElementCount ) || ( ! ElementCount ) )

		Throw ( CatchBuffer, WERR_OUTOFBOUNDS );

    else

    {

        if ( Index + 1 != ElementCount )

            memmove ( (*pChunk) [ Index * ElementSize ], 

                (*pChunk) [ ( Index + 1 ) * ElementSize ],

                ( ElementCount - Index - 1 ) * ElementSize );

        ElementCount--;

    }

}



CWElement* CWArray::operator[] (

    DWORD                               Index )

{



    CWElement                           **ppElement;





    

    ppElement = NULL;



    if ( ( Index >= ElementCount ) || ( ! ElementCount ) )

		Throw ( CatchBuffer, WERR_OUTOFBOUNDS );

    else

        ppElement = ( CWElement** )( (*pChunk) [ Index * ElementSize ] );



    return ( *ppElement );



}







DWORD CWArray::DataToIndex (

    CWElement                           **ppElement )



{



    DWORD                               Index;



    LPBYTE                              pElementData = (*pChunk)[ 0 ];





    for ( Index = 0; Index < Count(); Index++, pElementData += ElementSize )

        if ( ! memcmp ( ( LPBYTE )ppElement, pElementData, ElementSize ) )

            break;



    if ( Index >= Count() )

		Throw ( CatchBuffer, WERR_OUTOFBOUNDS );

    return ( Index );



}





int CWArray::Compare (

    CWElement                           *pElement1,

    CWElement                           *pElement2,

    LPARAM                              lParam )



{



    return ( pElement2->Hash() - pElement1->Hash() );



}





void CWArray::Exchange (

    DWORD                               Index1,

    DWORD                               Index2,

    LPARAM                              lParam )



{



    LPBYTE                              pTemp = NULL;





    if ( ! ( pTemp = ( LPBYTE )malloc ( ElementSize ) ) )

		Throw ( CatchBuffer, WERR_ALLOCATION );

    memcpy ( pTemp, (*pChunk)[ Index1 * ElementSize ], ElementSize );

    memcpy ( (*pChunk)[ Index1 * ElementSize ], (*pChunk)[ Index2 * ElementSize ], ElementSize );

    memcpy ( (*pChunk)[ Index2 * ElementSize ], pTemp, ElementSize );



    if ( pTemp )

        free ( pTemp );



}







void CWArray::Sort (

    DWORD                               Head,

    DWORD                               Tail,

    LPARAM                              lParam )



{



    int                                 MiddleHash;

    DWORD                               Middle;

    DWORD                               Start;

    DWORD                               Finish;



    LPBYTE                              pTemp  = NULL;



    

    if ( Tail == WAD_SORTALL )

        Tail = Count() - 1;



    Middle = ( DWORD )( ( Head + Tail ) / 2 );

    Start  = Head;

    Finish = Tail;

    

    if ( ( Count() <= 1 ) || ( Tail <= Head ) || 

        ( Tail >= Count() ) || ( Head >= Count() ) )

        return;



    if ( ! ( pTemp = ( LPBYTE )malloc ( ElementSize ) ) )

		Throw ( CatchBuffer, WERR_ALLOCATION );

    if ( Tail - Head == 1 )

    {

        if ( Compare ( operator[]( Start ), operator[]( Finish ), lParam ) < 0 )

            Exchange ( Start, Finish, lParam );

    }

    else

    {

        MiddleHash = (operator[]( Middle ) )->Hash();

        do

        {

            while ( ( Start < Finish ) && 

                ( Compare ( operator[]( Start ), operator[]( Middle ), lParam ) >= 0 ) )

                Start++;

            while ( ( Start < Finish ) && 

                ( Compare ( operator[]( Middle ), operator[]( Finish ), lParam ) >= 0 ) )

                Finish--;

            if ( ( Start < Finish ) &&

                ( Compare ( operator[]( Start ), operator[]( Finish ), lParam ) < 0 ) )

                Exchange ( Start, Finish, lParam );

        } while ( Start < Finish );

    }



    if ( pTemp )

        free ( pTemp );



    if ( Finish < Start )

    {

        if ( Finish != Tail )

            Sort ( Head, Finish, lParam );

        if ( Finish != Head )

            Sort ( Finish, Tail, lParam );

    }

    else

    if ( Start != Head )

    {

        if ( Start != Tail ) 

            Sort ( Head, Start, lParam );

        if ( Start != Head ) 

            Sort ( Start, Tail, lParam );

    }





}



