/*==============================================================================
*
*   WArray.h    - General purpose array
*
*   @(#)WArray.h	1.1 17:29:11 7/11/96 /users/sccs/src/win/utilities/s.WArray.h
*
*   Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
*
==============================================================================*/




#ifndef WARRAY_H
#   define WARRAY_H

#   define WAD_SORTALL                      (( DWORD )-1)


    typedef int ( CALLBACK *PFNWCOMPARE )( DWORD, DWORD, LPARAM );

    class CWChunk;
    class CWElement;
    class EXPORT CWArray
    {
                      
        public:

            CWArray (
                DWORD                               InitialElementSize,
                DWORD                               InitialElementCount = 0 );
            virtual ~CWArray ();

            virtual void Insert (
                DWORD                               IndexBefore,
                CWElement                           **pElement );
            virtual void Delete (
                DWORD                               Index );
            CWElement* operator[] (
                DWORD                               Index );
            virtual DWORD Count () 
                { return ( ElementCount ); }
            virtual DWORD Size ()
                { return ( ElementSize ); }
            virtual void Sort (
                DWORD                               Head           = 0,
                DWORD                               Tail           = WAD_SORTALL,
                LPARAM                              lParam         = ( LPARAM )NULL );
            virtual DWORD DataToIndex (
                CWElement                           **pElement );
            virtual int Compare (
                CWElement                           *pElement1,
                CWElement                           *pElement2,
                LPARAM                              lParam );
            virtual int Compare (
                DWORD                               Index1,
                DWORD                               Index2,
                LPARAM                              lParam )
            {
                return ( Compare ( operator[]( Index1 ), operator[]( Index2 ), lParam ) );
            }
            virtual void Exchange (
                DWORD                               Index1,
                DWORD                               Index2,
                LPARAM                              lParam );


        private:

            DWORD                                   ElementCount;
            DWORD                                   ElementSize;
            DWORD                                   ElementsAllocated;
            DWORD                                   IncrementCount;
            CWChunk                                 *pChunk;

    };


#endif // #ifndef WARRAY_H
