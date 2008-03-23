/*==============================================================================
*
*   WString.h   - General String class
*
*   @(#)WString.h	1.2 15:53:19 8/12/96 /users/sccs/src/win/utilities/s.WString.h
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/



#ifndef __WSTRING_H__
#define __WSTRING_H__


#include "Willows.h"



class EXPORT CWString
{

    public:

        CWString ( const char                   *pString = NULL );
        CWString ( const CWString               &that );

        ~CWString ( void );

        CWString& operator= ( char              *pString );
        CWString& operator= ( const CWString    &that );
        operator char*( ) { return ( _pString ); }
        char& operator[] ( int                  Index );

        CWString& operator+ ( char              *pString );
        CWString& operator+ ( const CWString    &that );
        CWString& operator+= ( char             *pString );
        CWString& operator+= ( const CWString   &that );

        int Length ( void )
        {
            return ( _Length );
        }

        CWString stristr ( char                *pString );
        CWString stristr ( const CWString      &that );
	void Lower ();

        void DrawFittedString ( 
            HDC                                 hDC,
            DWORD                               DTFormat,
            int                                 PadFactor,
            LPRECT                              pDrawRect );


    private:

        int                                 _Length;
        char*                               _pString;

        void Assign (
            const char                      *pString,
            int                             Length );

};
        


#endif // __WSTRING_H__
    
