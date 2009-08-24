/*==============================================================================
*
*   WAHandle.h   - Array of handles
*
*   @(#)WAHandle.h	1.2 15:40:52 8/12/96 /users/sccs/src/win/utilities/s.WAHandle.h
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/



#ifndef CWAHANDLE_H
#define CWAHANDLE_H


#include "WUtilities.h"
#include "WArray.h"
#include "WHandle.h"

class EXPORT CWAHandle : 
    private CWArray
{

    public:

        CWAHandle ()
            : CWArray ( sizeof ( CWHandle* ) )
        {
        }
        virtual ~CWAHandle ()
        {
        }

        void Insert (
            DWORD                               IndexBefore,
            CWHandle                            *pHandle )
        {
            CWArray::Insert ( IndexBefore, ( CWElement** )&pHandle );
        }
        void Delete (
            DWORD                               Index )
        {
            CWArray::Delete ( Index );
        }
        CWHandle* operator[] (
            DWORD                               Index )
        {
            return ( ( ( CWHandle* )CWArray::operator[]( Index ) ) );
        }
        DWORD Count ()
        { 
            return ( CWArray::Count() ); 
        }
        DWORD Size ()
        {
            return ( CWArray::Size() );
        }

};


#endif // #ifndef CWAHANDLE_H
