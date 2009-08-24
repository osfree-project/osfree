/*==============================================================================
*
*   WChunk.h    - A chunk of memory
*
*   @(#)WChunk.h	1.2 15:44:21 8/12/96 /users/sccs/src/win/utilities/s.WChunk.h
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/




#ifndef WCHUNK_H
#define WCHUNK_H



#include "WElement.h"



class EXPORT CWChunk :
    public CWElement
{

    public:

        CWChunk ( 
            DWORD                               dwSize,
            LPBYTE                              pNewData = NULL );
        CWChunk (
            CWChunk                             &that );
        virtual ~CWChunk ();

        DWORD Size ()
        {
            return ( AllocatedSize );
        }
        virtual LPBYTE operator[] ( 
            DWORD                               Offset );
        virtual CWChunk& operator= (
            CWChunk                             &that );
        virtual CWElement& operator= (
            CWElement                           &that )
        {
            return ( ( CWElement& ) operator= ( ( CWChunk& ) that ) );
        }
        virtual int Hash ()
        {
            return ( AllocatedSize );
        }


    private:

        DWORD                                   AllocatedSize;
        LPBYTE                                  pChunk;

};



#endif // #ifndef WCHUNK_H
