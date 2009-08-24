/*==============================================================================
*
*   WABlock.h   - Array of blocks
*
*   @(#)WABlock.h	1.4 12:05:34 9/16/96 /users/sccs/src/win/utilities/s.WABlock.h
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/



#ifndef CWABLOCK_H
#define CWABLOCK_H


#include "WUtilities.h"
#include "WArray.h"
#include "WBlock.h"

class EXPORT CWABlock : 
    private CWArray
{

    public:

        CWABlock ()
            : CWArray ( sizeof ( CWBlock* ) )
        {
        }
        virtual ~CWABlock ()
        {
        }

        void Insert (
            DWORD                               IndexBefore,
            CWBlock                             *pBlock )
        {
            CWArray::Insert ( IndexBefore, ( CWElement** )&pBlock );
        }
        void Delete (
            DWORD                               Index )
        {
            CWArray::Delete ( Index );
        }
        CWBlock* operator[] (
            DWORD                               Index )
        {
            return ( ( ( CWBlock* )CWArray::operator[]( Index ) ) );
        }
        DWORD Count ()
        { 
            return ( CWArray::Count() ); 
        }
        DWORD Size ()
        {
            return ( CWArray::Size() );
        }
        void Sort ()
        {
            CWArray::Sort();
        }
        DWORD DataToIndex (
            CWBlock                             *pBlock )
        {
            return ( CWArray::DataToIndex ( ( CWElement** )&pBlock ) );
        }

};


#endif // #ifndef CWABLOCK_H
