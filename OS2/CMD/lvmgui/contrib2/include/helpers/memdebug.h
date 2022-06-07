
/*
 *@@sourcefile memdebug.h:
 *      header file for memdebug.c.
 *      See remarks there.
 *
 *      The following macros are used:
 *
 *      --  __XWPMEMDEBUG__: if defined, memory debugging is generally
 *                           enabled. This must be set in setup.h.
 *
 *      --  __DEBUG_MALLOC_ENABLED__: malloc etc. have been replaced
 *                           with memdMalloc etc. This is automatically
 *                           defined by this header if __XWPMEMDEBUG__
 *                           is defined, unless DONT_REPLACE_MALLOC
 *                           is also defined.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\memdebug.h"
 */

/*      Copyright (C) 2000 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef MEMDEBUG_HEADER_INCLUDED
    #define MEMDEBUG_HEADER_INCLUDED

    #ifndef __stdlib_h           // <stdlib.h>
        // #error stdlib.h must be included before memdebug.h.
        typedef unsigned int size_t;
    #endif

    #ifndef NULL
       #if (defined(__EXTENDED__)  || defined( __cplusplus ))
          #define NULL 0
       #else
          #define NULL ((void *)0)
       #endif
    #endif

    typedef void (FNCBMEMDLOG)(const char*);        // message
    typedef FNCBMEMDLOG *PFNCBMEMDLOG;

    // global variable for memory error logger func
    extern PFNCBMEMDLOG    G_pMemdLogFunc;

    /* ******************************************************************
     *
     *   Private declarations
     *
     ********************************************************************/

    #ifdef MEMDEBUG_PRIVATE

        BOOL memdLock(VOID);

        VOID memdUnlock(VOID);

        /*
         *@@ HEAPITEM:
         *      informational structure created for each
         *      malloc() call by memdMalloc. These are stored
         *      in a global linked list (G_pHeapItemsRoot).
         *
         *      We cannot use the linklist.c functions for
         *      managing the linked list because these use
         *      malloc in turn, which would lead to infinite
         *      loops.
         *
         *@@added V0.9.3 (2000-04-11) [umoeller]
         */

        typedef struct _HEAPITEM
        {
            // struct _HEAPITEM    *pNext;     // next item in linked list or NULL if last

            TREE                Tree;           // tree node for tree.* functions;
                                                // ulKey has the pointer that is returned
                                                // by memdMalloc and points to after the
                                                // magic string (in the buffer that was
                                                // really allocated). Using this as the
                                                // tree sort key allows us to do fast
                                                // searches in memdFree.

            // void                *pAfterMagic; // memory pointer returned by memdMalloc;
                                              // this points to after the magic string

            unsigned long       ulSize;     // requested size (without magic head and tail)

            const char          *pcszSourceFile;    // as passed to memdMalloc
            unsigned long       ulLine;             // as passed to memdMalloc
            const char          *pcszFunction;      // as passed to memdMalloc

            DATETIME            dtAllocated;        // system date/time at time of memdMalloc call

            ULONG               ulTID;      // thread ID that memdMalloc was running on

            BOOL                fFreed;     // TRUE only after item has been freed by memdFree
        } HEAPITEM, *PHEAPITEM;

        extern TREE         *G_pHeapItemsRoot;
        extern LONG         G_cHeapItems;
        extern ULONG        G_ulItemsReleased;
        extern ULONG        G_ulBytesReleased;

    #endif // MEMDEBUG_PRIVATE

    /* ******************************************************************
     *
     *   Publics
     *
     ********************************************************************/

    void* memdMalloc(size_t stSize,
                     const char *pcszSourceFile,
                     unsigned long ulLine,
                     const char *pcszFunction);

    void* memdCalloc(size_t num,
                     size_t stSize,
                     const char *pcszSourceFile,
                     unsigned long ulLine,
                     const char *pcszFunction);

    void memdFree(void *p,
                  const char *pcszSourceFile,
                  unsigned long ulLine,
                  const char *pcszFunction);

    void* memdRealloc(void *p,
                      size_t stSize,
                      const char *pcszSourceFile,
                      unsigned long ulLine,
                      const char *pcszFunction);

    unsigned long memdReleaseFreed(void);

    #ifdef __XWPMEMDEBUG__

        #ifndef DONT_REPLACE_MALLOC

            #ifdef malloc
                #undef malloc
            #endif
            #define malloc(ul) memdMalloc(ul, __FILE__, __LINE__, __FUNCTION__)

            #ifdef calloc
                #undef calloc
            #endif
            #define calloc(n, size) memdCalloc(n, size, __FILE__, __LINE__, __FUNCTION__)

            #ifdef realloc
                #undef realloc
            #endif
            #define realloc(p, ul) memdRealloc(p, ul, __FILE__, __LINE__, __FUNCTION__)

            #ifdef free
                #undef free
            #endif
            #define free(p) memdFree(p, __FILE__, __LINE__, __FUNCTION__)

            #ifdef __string_h
                // string.h included and debugging is on:
                // redefine strdup to use memory debugging
                #define strdup(psz)                                 \
                    strcpy( (char*)memdMalloc(strlen(psz) + 1, __FILE__, __LINE__, __FUNCTION__), psz)
                    // the original crashes also if psz is NULL
            #endif

            // tell other headers that these have been replaced
            #define __DEBUG_MALLOC_ENABLED__

        #endif
    #endif

    #ifdef PM_INCLUDED
        /********************************************************************
         *
         *   XFolder debugging helpers
         *
         ********************************************************************/

        #ifdef _PMPRINTF_
            void memdDumpMemoryBlock(PBYTE pb,
                                     ULONG ulSize,
                                     ULONG ulIndent);
        #else
            // _PMPRINTF not #define'd: do nothing
            #define memdDumpMemoryBlock(pb, ulSize, ulIndent)
        #endif

        /* ******************************************************************
         *
         *   Heap debugging window
         *
         ********************************************************************/

        MRESULT EXPENTRY memd_fnwpMemDebug(HWND hwndClient, ULONG msg, MPARAM mp1, MPARAM mp2);

        VOID memdCreateMemDebugWindow(VOID);
    #endif

#endif

#if __cplusplus
}
#endif

