/*==============================================================================
*
*   WElement.h  - General purpose class to be derived from in use with CWArray
*
*   @(#)WElement.h	1.2 15:45:11 8/12/96 /users/sccs/src/win/utilities/s.WElement.h
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/




#ifndef CWELEMENT_H
#define CWELEMENT_H



#include "WUtilities.h"



#define DWELEMENT_LESSTHAN              -1
#define DWELEMENT_EQUALTO               0
#define DWELEMENT_GREATERTHAN           1



class EXPORT CWElement
{

    public:

        CWElement ()
        {
        }
        virtual ~CWElement ()
        {
        }

        virtual CWElement& operator= (
            CWElement                           &that ) = 0;
        virtual int Hash () = 0;
        virtual int Compare (
            CWElement                           &that );

        virtual BOOL operator== (
            CWElement                           &that )
        {
            return ( Compare ( that ) == DWELEMENT_EQUALTO );
        }
        virtual BOOL operator!= (
            CWElement                           &that )
        {
            return ( Compare ( that ) != DWELEMENT_EQUALTO );
        }
        virtual BOOL operator< (
            CWElement                           &that )
        {
            return ( Compare ( that ) < DWELEMENT_EQUALTO );
        }
        virtual BOOL operator<= (
            CWElement                           &that )
        {
            return ( Compare ( that ) <= DWELEMENT_EQUALTO );
        }
        virtual BOOL operator> (
            CWElement                           &that )
        {
            return ( Compare ( that ) > DWELEMENT_EQUALTO );
        }
        virtual BOOL operator>= (
            CWElement                           &that )
        {
            return ( Compare ( that ) >= DWELEMENT_EQUALTO );
        }

};


#endif // #ifndef CWELEMENT_H
