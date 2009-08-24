//==============================================================================
//
// @(#)WString.cpp	1.5
//
//  Copyright (c) 1996 Willows Software Inc. All Rights Reserved
//
//==============================================================================




#include "string.h"
#include "WUtilities.h"
#include "WString.h"
#include "WErr.h"

void CWString::Assign (
    const char                          *pString,
    int                                 Length )
{

    if ( _pString )
        delete [] ( _pString );
    _pString = NULL;
    if ( _Length = Length )
    {
        _pString = new char [ _Length + 1 ];
        strcpy ( _pString, pString );
    }

}



CWString::CWString ( 
    const char                          *pString )
{

    _pString = NULL;
    _Length  = 0;
    if ( pString )
        Assign ( pString, strlen ( pString ) );

}



CWString::CWString ( const CWString         &that )
{

    _pString = NULL;
    _Length  = 0;
    if ( that._Length )
        Assign ( that._pString, that._Length );

}



CWString::~CWString ( void )
{

    if ( _pString )
        delete [] ( _pString );

}



CWString& CWString::operator= ( char          *pString )
{

    if ( pString )
        Assign ( pString, strlen ( pString ) );
    else
    {
        if ( _pString )
            delete [] ( _pString );
        _pString = NULL;
        _Length  = 0;
    }

    return ( *this );

}



CWString& CWString::operator= ( const CWString &that )
{

    return ( this->operator= ( that._pString ) );

}



char& CWString::operator[] ( int             Index )
{

    if ( ! _pString )
        Throw ( CatchBuffer, WERR_NULL );

    if ( ( Index < 0 ) || ( Index >= _Length ) )
        Throw ( CatchBuffer, WERR_OUTOFBOUNDS );

    return ( _pString [ Index ] );

}



CWString& CWString::operator+= ( char         *pString )
{

    CWString                     TempString ( _pString );

    if ( _pString )
        delete [] ( _pString );
    _pString = NULL;
    if ( _Length += strlen ( pString ) )
    {
        _pString = new char ( _Length + 1 );
        strcpy ( _pString, TempString );
        strcat ( _pString, pString );
    }

    return ( *this );

}



CWString& CWString::operator+= ( const CWString    &that )
{

    return ( this->operator+= ( that._pString ) );

}



CWString& CWString::operator+ ( char         *pString )
{

    CWString                     TempString ( _pString );

    if ( _pString )
        delete [] ( _pString );
    _pString = NULL;
    if ( _Length += strlen ( pString ) )
    {
        _pString = new char ( _Length + 1 );
        strcpy ( _pString, TempString );
        strcat ( _pString, pString );
    }

    return ( *this );

}



CWString& CWString::operator+ ( const CWString    &that )
{

    return ( this->operator+ ( that._pString ) );

}


CWString CWString::stristr ( 
    char                                *pString )

{

    CWString                            SubString ( pString );
    CWString                            WholeString ( _pString );


    SubString.Lower ();
    WholeString.Lower ();

    return ( CWString ( strstr ( WholeString, SubString ) ) );

}




void CWString::Lower ()

{

	int                             Index = _Length;

	for ( Index = _Length; Index--; /* No itteration */ )
		if ( ( 'A' <= _pString [ Index ] ) && ( _pString [ Index ] <= 'Z' ) )
			_pString [ Index ] += ( ( int )'a' - ( int )'A' );

}


CWString CWString::stristr ( const CWString      &that )

{

    return ( stristr ( that._pString ) );

}


void CWString::DrawFittedString (
    HDC                                 hDC,
    DWORD                               DTFormat,
    int                                 PadFactor,
    LPRECT                              pDrawRect )

{

    RECT                                CalcRect;
    char                                *pFittedString;
    BOOL                                Drawn;

    int                                 DDDOffset = Length();


    /*--- Allocate enough memory to accomadate the string to be printed plus a ... and
		  terminating NULL ---*/
    pFittedString = new char [ DDDOffset + 4 ];
    strcpy ( pFittedString, _pString );
	do
	{
		/*--- find out what area is required to print the string ---*/
    memset ( &CalcRect, 0, sizeof ( CalcRect ) );
		DrawText ( hDC, pFittedString, strlen ( pFittedString ), &CalcRect, 
				   DTFormat | DT_CALCRECT );
		/*--- if the string can be drawn in the rect available then draw 
			  it ---*/
		if ( Drawn = ( ( CalcRect.right - CalcRect.left ) <= 
				       ( pDrawRect->right - pDrawRect->left - PadFactor ) ) )
			DrawText ( hDC, pFittedString, strlen ( pFittedString ), pDrawRect, 
				           DTFormat );
		else
		{
			if ( ! DDDOffset )
			{ /*--- if the string can not be shortened any more then 
					draw it ---*/
				Drawn = TRUE;
				DrawText ( hDC, pFittedString, strlen ( pFittedString ), 
						   pDrawRect, DTFormat );
			}
			else
			{ /*--- shorten the text of the string by 1 and append ... 
					with a terminating NULL from that point ---*/
				pFittedString [ DDDOffset     ] = '.';
				pFittedString [ DDDOffset + 1 ] = '.';
				pFittedString [ DDDOffset + 2 ] = '.';
				pFittedString [ DDDOffset + 3 ] = '\0';
				DDDOffset--;
			}
		}
	} while ( ! Drawn );
    delete [] ( pFittedString );

}


