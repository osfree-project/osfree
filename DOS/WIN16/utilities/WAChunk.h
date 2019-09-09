/*==============================================================================
*
*   WAChunk.h   - Array of Chunks of memory
*
*   @(#)WAChunk.h	1.2 15:40:10 8/12/96 /users/sccs/src/win/utilities/s.WAChunk.h
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/



#ifndef WACHUNK_H
#define WACHUNK_H


#include "WUtilities.h"
#include "WArray.h"
#include "WChunk.h"

class EXPORT CWAChunk : 
    private CWArray
{

    public:

        CWAChunk ()
            : CWArray ( sizeof ( CWChunk ) )
        {
        }
        virtual ~CWAChunk ()
        {
        }

        void Insert (
            DWORD                               IndexBefore,
            CWChunk                             *pChunk )
        {
            CWArray::Insert ( IndexBefore, ( CWElement** )&pChunk );
        }
        void Delete (
            DWORD                               Index )
        {
            CWArray::Delete ( Index );
        }
        CWChunk* operator[] (
            DWORD                               Index )
        {
            return ( ( ( CWChunk* )CWArray::operator[]( Index ) ) );
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


#endif // #ifndef WACHUNK_H

