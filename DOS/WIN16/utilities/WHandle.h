/*==============================================================================
*
*   WHandle.h   - To be used as handles by calling routines.
*
*   @(#)WHandle.h	1.2 15:48:24 8/12/96 /users/sccs/src/win/utilities/s.WHandle.h
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/




#ifndef CWHANDLE_H
#define CWHANDLE_H



#include "WElement.h"



class CWBlock;



class EXPORT CWHandle :
    public CWElement
{

    public:

        CWHandle (
            UINT                                NewFlags,
            CWBlock                             *pNewBlock = NULL );
        CWHandle (
            CWHandle                            &that );
        virtual ~CWHandle ()
        {
        }

        virtual CWHandle& operator= (
            CWHandle                            &that );
        virtual CWElement& operator= (
            CWElement                           &that )
        {
            return ( ( CWElement& ) operator= ( ( CWHandle& ) that ) );
        }
        UINT LockCount ()
        {
            return ( Locks );
        }
        UINT operator++ (int)
        {
            return ( ++Locks );
        }
        UINT operator-- (int)
        {
            return ( --Locks );
        }
        CWBlock* Block ()
        {
            return ( pBlock );
        }
        void Block ( 
            CWBlock                             *pNewBlock )
        {
            pBlock = pNewBlock;
        }
        UINT Flags ()
        {
            return ( Flag );
        }
        void Flags ( 
            UINT                                Flags )
        {
            Flag = Flags;
        }
        int Hash ()
        {
            return ( ( int )pBlock );
        }


        
    private:

        CWBlock                                 *pBlock;
        UINT                                    Locks;
        UINT                                    Flag;

};



#endif // #ifndef CWHANDLE_H
