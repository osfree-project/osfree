
/*
 *@@sourcefile memdebug.c:
 *      memory debugging helpers.
 *
 *      Several things are in here which might turn out to
 *      be useful:
 *
 *      -- Memory block dumping (memdDumpMemoryBlock).
 *
 *      -- Sophisticated heap debugging functions, which
 *         automatically replace malloc() and free() etc.
 *         If the __XWPMEMDEBUG__ #define is set before including
 *         memdebug.h, all those standard library calls
 *         are remapped to use the functions in this file
 *         instead.
 *
 *         At present, malloc(), calloc(), realloc(), strdup()
 *         and free() are supported.
 *
 *         These log every memory allocation made and log
 *         much more data compared to the VAC++ memory
 *         debugging functions. See HEAPITEM for details.
 *         Automatic heap checking is also included, using
 *         special "magic string" which are checked to
 *         detect overwrites.
 *
 *         To be able to trace memory errors, set the global
 *         variable G_pMemdLogFunc to a function which can
 *         write an error string to a meaningful place (the
 *         screen or a file). WARNING: While that error function
 *         is executed, the system might be in a global memory
 *         lock, so DON'T display a message box while in that
 *         function, and DO NOT call malloc() or other memory
 *         functions in there.
 *
 *         These debug functions have been added with V0.9.3
 *         and should now be compiler-independent.
 *
 *         V0.9.6 added realloc() support and fixed a few bugs.
 *
 *         With V0.9.16, most of this was rewritten to be much
 *         faster. This no longer slows down the system enormously.
 *
 *      -- A PM heap debugging window which shows the status
 *         of the heap logging list. See memdCreateMemDebugWindow
 *         (memdebug_win.c) for details.
 *
 *      To enable memory debugging, do the following in each (!)
 *      of your code modules:
 *
 *      1) Include at least <stdlib.h> and <string.h>.
 *
 *      2) Include memdebug.h AFTER those two. This will remap
 *         the malloc() etc. calls to the debug functions in
 *         this file by defining macros for them.
 *
 *         If you don't want those replaced, add
 +              #define DONT_REPLACE_MALLOC
 *         before including memdebug.h.
 *
 *         To avoid calling a debug function for a single call,
 *         place the malloc call (or whatever) in brackets.
 *
 *      That's all. XWorkplace's setup.h does this automatically
 *      if XWorkplace is compiled with debug code.
 *
 *      A couple of WARNINGS:
 *
 *      1)  When free() is invoked, the memory that was allocated
 *          is freed, but not the memory log entry (the HEAPITEM)
 *          to allow tracing what was freed. As a result, the tree
 *          of memory items keeps growing longer. Do not expect
 *          this to work forever, even though things have greatly
 *          improved with V0.9.16.
 *
 *      2)  The replacement functions in this file allocate
 *          extra memory for the magic strings. For example, if
 *          you call malloc(100), more than 100 bytes get allocated
 *          to allow for storing the magic strings to detect
 *          memory overwrites. Two magic strings are allocated,
 *          one before the actual buffer, and one behind it.
 *          The pointer returned is _not_ identical to the one
 *          that was internally allocated.
 *
 *          As a result, YOU MUST NOT confuse the replacement
 *          memory functions with the original ones. If you
 *          use malloc() in one source file and free() the
 *          buffer in another one where debug memory has not
 *          been enabled, you'll get crashes.
 *
 *          As a rule of thumb, enable memory debugging for all
 *          your source files or for none. And make sure everything
 *          is recompiled when you change your mind.
 *
 *@@added V0.9.1 (2000-02-12) [umoeller]
 */

/*
 *      Copyright (C) 2000-2001 Ulrich M”ller.
 *      This program is part of the XWorkplace package.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSERRORS

#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include "helpers\tree.h"

#define DONT_REPLACE_MALLOC             // never do debug memory for this
#define MEMDEBUG_PRIVATE
#include "setup.h"

#ifdef __XWPMEMDEBUG__

#include "helpers\dosh.h"
#include "helpers\except.h"

#include "helpers\memdebug.h"        // included by setup.h already
#include "helpers\stringh.h"

/*
 *@@category: Helpers\C helpers\Heap debugging
 *      See memdebug.c.
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

#define MEMBLOCKMAGIC_HEAD     "\210\203`H&cx$&%\254"
            // size must be a multiple of 4 or dword-alignment will fail;
            // there's an extra 0 byte at the end, so we have a size of 12
            // V0.9.3 (2000-04-17) [umoeller]
#define MEMBLOCKMAGIC_TAIL     "\250\210&%/dfjsk%#,dlhf\223"

HMTX                G_hmtxMallocList = NULLHANDLE;

extern TREE         *G_pHeapItemsRoot = NULL;
extern LONG         G_cHeapItems = 0;

PFNCBMEMDLOG        G_pMemdLogFunc = NULL;

extern ULONG        G_ulItemsReleased = 0;
extern ULONG        G_ulBytesReleased = 0;

/* ******************************************************************
 *
 *   Debug heap management
 *
 ********************************************************************/

/*
 *@@ memdLock:
 *      enables the global memory lock to protect
 *      the global variables here. Use memdUnlock
 *      to unlock again, and lock only for the shortest
 *      possible time. This is only used by the memdebug.c
 *      functions.
 *
 *@@added V0.9.3 (2000-04-10) [umoeller]
 */

BOOL memdLock(VOID)
{
    if (!G_hmtxMallocList)
    {
        // first call:
        if (!DosCreateMutexSem(NULL,
                               &G_hmtxMallocList,
                               0,          // unshared
                               TRUE))      // request now!
        {
            treeInit(&G_pHeapItemsRoot, &G_cHeapItems);
            return TRUE;
        }
    }
    else
        return !DosRequestMutexSem(G_hmtxMallocList,
                                   SEM_INDEFINITE_WAIT);

    return FALSE;
}

/*
 *@@ memdUnlock:
 *      the reverse to memdLock.
 *
 *@@added V0.9.3 (2000-04-10) [umoeller]
 */

VOID memdUnlock(VOID)
{
    DosReleaseMutexSem(G_hmtxMallocList);
}

/*
 *@@ LogError:
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC VOID LogError(const char *pcszFormat,     // in: format string (like with printf)
                     ...)                        // in: additional stuff (like with printf)
{
    if (G_pMemdLogFunc)
    {
        CHAR        szMsg[1000];
        va_list     args;

        va_start(args, pcszFormat);
        vsprintf(szMsg, pcszFormat, args);
        va_end(args);
        G_pMemdLogFunc(szMsg);
    }
}

/*
 *@@ FindHeapItem:
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC PHEAPITEM FindHeapItem(void *p)
{
    return (PHEAPITEM)treeFind(G_pHeapItemsRoot,
                               (ULONG)p,
                               treeCompareKeys);
}

/*
 *@@ FillHeapItem:
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC VOID FillHeapItem(PHEAPITEM pHeapItem,
                         void *prc,
                         size_t stSize,
                         const char *pcszSourceFile, // in: source file name
                         unsigned long ulLine,       // in: source line
                         const char *pcszFunction)   // in: function name
{
    pHeapItem->ulSize = stSize;

    pHeapItem->pcszSourceFile = pcszSourceFile;
    pHeapItem->ulLine = ulLine;
    pHeapItem->pcszFunction = pcszFunction;

    DosGetDateTime(&pHeapItem->dtAllocated);

    pHeapItem->ulTID = doshMyTID();

    pHeapItem->fFreed = FALSE;

    // use the return pointer as the tree sort key
    // V0.9.16 (2001-12-08) [umoeller]
    pHeapItem->Tree.ulKey = (ULONG)prc;
}

/*
 *@@ CheckMagics:
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC VOID CheckMagics(const char *pcszParentFunc,
                        PHEAPITEM pHeapItem,
                        PBYTE p,
                        const char *pcszSourceFile, // in: source file name
                        unsigned long ulLine,       // in: source line
                        const char *pcszFunction)   // in: function name
{
    void    *pBeforeMagic = ((PBYTE)p) - sizeof(MEMBLOCKMAGIC_HEAD);
    ULONG   ulError = 0;

    // check magic string
    if (memcmp(pBeforeMagic,
               MEMBLOCKMAGIC_HEAD,
               sizeof(MEMBLOCKMAGIC_HEAD)))
        ulError = 1;
    else if (memcmp(((PBYTE)p) + pHeapItem->ulSize,
                    MEMBLOCKMAGIC_TAIL,
                    sizeof(MEMBLOCKMAGIC_TAIL)))
        ulError = 2;

    if (ulError)
    {
        LogError("%s: Magic string %s memory block at 0x%lX has been overwritten.\n"
                 "This was detected by the free() call at %s (%s, line %d).\n"
                 "The block was allocated by %s (%s, line %d).",
                 pcszParentFunc,
                 (ulError == 1) ? "before" : "after",
                 p,
                 pcszFunction,
                     pcszSourceFile,
                     ulLine, // free
                 pHeapItem->pcszFunction,
                     pHeapItem->pcszSourceFile,
                     pHeapItem->ulLine);
    }
}

/*
 *@@ memdMalloc:
 *      wrapper function for malloc() to trace malloc()
 *      calls more precisely.
 *
 *      If XWorkplace is compiled with debug code, setup.h
 *      automatically #includes memdebug.h, which maps
 *      malloc to this function so that the source file
 *      etc. parameters automatically get passed.
 *
 *      For each call, we call the default malloc(), whose
 *      return value is returned, and create a HEAPITEM
 *      for remembering the call, which is stored in a global
 *      linked list.
 *
 *@@added V0.9.3 (2000-04-11) [umoeller]
 *@@changed V0.9.16 (2001-12-08) [umoeller]: reworked to use trees now, much faster
 */

void* memdMalloc(size_t stSize,
                 const char *pcszSourceFile, // in: source file name
                 unsigned long ulLine,       // in: source line
                 const char *pcszFunction)   // in: function name
{
    void *prc = NULL;

    if (stSize == 0)
        // malloc(0) called: report error
        LogError(__FUNCTION__ ": Function %s (%s, line %d) called malloc(0).",
                 pcszFunction,
                     pcszSourceFile,
                     ulLine);
    else
        if (memdLock())
        {
            // call default malloc(), but with the additional
            // size of our MEMBLOCKMAGIC strings; we'll return
            // the first byte after the "front" string so we can
            // check for string overwrites
            void *pObj;

            if (pObj = malloc(   sizeof(MEMBLOCKMAGIC_HEAD)
                               + stSize
                               + sizeof(MEMBLOCKMAGIC_TAIL)))
            {
                PHEAPITEM pHeapItem;
                BOOL fInsert = TRUE;

                // store "front" magic string
                memcpy(pObj,
                       MEMBLOCKMAGIC_HEAD,
                       sizeof(MEMBLOCKMAGIC_HEAD));
                // return address: first byte after "front" magic string
                prc = ((PBYTE)pObj) + sizeof(MEMBLOCKMAGIC_HEAD);
                // store "tail" magic string to block which
                // will be returned plus the size which was requested
                memcpy(((PBYTE)prc) + stSize,
                       MEMBLOCKMAGIC_TAIL,
                       sizeof(MEMBLOCKMAGIC_TAIL));

                if (!(pHeapItem = FindHeapItem(prc)))
                    // not re-using old address:
                    // create a new heap item
                    pHeapItem = (PHEAPITEM)malloc(sizeof(HEAPITEM));
                else
                    fInsert = FALSE;

                FillHeapItem(pHeapItem,
                             prc,
                             stSize,
                             pcszSourceFile,
                             ulLine,
                             pcszFunction);

                if (fInsert)
                    // append heap item to linked list
                    if (treeInsert(&G_pHeapItemsRoot,
                                   &G_cHeapItems,
                                   (TREE*)pHeapItem,
                                   treeCompareKeys))
                    {
                        LogError(__FUNCTION__ ": treeInsert failed for memory block at 0x%lX.\n"
                                 "The block was allocated by %s (%s, line %d).",
                                 prc,
                                 pcszFunction,
                                     pcszSourceFile,
                                     ulLine);
                    }
            }

            memdUnlock();
        }

    return prc;
}

/*
 *@@ memdCalloc:
 *      similar to memdMalloc; this is the wrapper for
 *      the calloc() call. This is automatically
 *      remapped also.
 *
 *@@added V0.9.3 (2000-04-11) [umoeller]
 */

void* memdCalloc(size_t num,
                 size_t stSize,
                 const char *pcszSourceFile,
                 unsigned long ulLine,
                 const char *pcszFunction)
{
    void *p = memdMalloc(num * stSize,
                         pcszSourceFile,
                         ulLine,
                         pcszFunction);
    memset(p, 0, num * stSize);
    return p;
}

/*
 *@@ memdFree:
 *      wrapper for the free() call, which is remapped
 *      by setup.h and memdebug.h like memdMalloc
 *      and memdCalloc. This searches the memory object
 *      (p) which was previously allocated on the linked
 *      list of HEAPITEM's and frees it then by calling
 *      the default free().
 *
 *      The HEAPITEM itself is not freed, but only marked
 *      as freed. As a result, the linked list can grow
 *      REALLY large. While memdMalloc does not become
 *      slower with large HEAPITEM lists because it only
 *      appends to the end of the list, which is remembered,
 *      memdFree can become extremely slow because the entire
 *      list needs to be searched with each call.
 *      So call memdReleaseFreed from time to time.
 *
 *@@added V0.9.3 (2000-04-10) [umoeller]
 *@@changed V0.9.16 (2001-12-08) [umoeller]: reworked to use trees now, much faster
 */

void memdFree(void *p,
              const char *pcszSourceFile,
              unsigned long ulLine,
              const char *pcszFunction)
{
    if (memdLock())
    {
        PHEAPITEM pHeapItem;

        // search the list with the pointer which was
        // really returned by the original malloc(),
        // that is, the byte after the magic string
        if (pHeapItem = FindHeapItem(p))
        {
            // the same address may be allocated and freed
            // several times, so check
            if (!pHeapItem->fFreed)
            {
                // found:
                void    *pBeforeMagic = ((PBYTE)p) - sizeof(MEMBLOCKMAGIC_HEAD);

                CheckMagics(__FUNCTION__,
                            pHeapItem,
                            (PBYTE)p,
                            pcszSourceFile,
                            ulLine,
                            pcszFunction);

                // free the real memory item
                free(pBeforeMagic);

                // mark the heap item as freed, but
                // keep it in the list
                pHeapItem->fFreed = TRUE;

            } // if (!pHeapItem->fFreed)
            else
                // memory block has been freed twice:
                LogError(__FUNCTION__ ": Memory block at 0x%lX has been freed twice.\n"
                         "This was detected by the free() call at %s (%s, line %d).\n"
                         "The block was originally allocated by %s (%s, line %d).",
                         p,
                         pcszFunction,
                             pcszSourceFile,
                             ulLine, // free
                         pHeapItem->pcszFunction,
                             pHeapItem->pcszSourceFile,
                             pHeapItem->ulLine);
        }
        else
            // not found:
            LogError(__FUNCTION__ ": free() called with invalid object 0x%lX from %s (%s, line %d).",
                     p,
                     pcszFunction,
                         pcszSourceFile,
                         ulLine);

        memdUnlock();
    }
}

/*
 *@@ memdRealloc:
 *      wrapper function for realloc(). See memdMalloc for
 *      details.
 *
 *@@added V0.9.6 (2000-11-12) [umoeller]
 *@@changed V0.9.12 (2001-05-21) [umoeller]: this reported errors on realloc(0), which is a valid call, fixed
 *@@changed V0.9.16 (2001-12-08) [umoeller]: reworked to use trees now, much faster
 */

void* memdRealloc(void *p,
                  size_t stSize,
                  const char *pcszSourceFile, // in: source file name
                  unsigned long ulLine,       // in: source line
                  const char *pcszFunction)   // in: function name
{
    void *prc = NULL;

    if (!p)
        // p == NULL: this is valid, use malloc() instead
        // V0.9.12 (2001-05-21) [umoeller]
        return memdMalloc(stSize, pcszSourceFile, ulLine, pcszFunction);

    if (memdLock())
    {
        // search the list with the pointer which was
        // really returned by the original malloc(),
        // that is, the byte after the magic string
        PHEAPITEM pHeapItem, pExisting;
        if (pHeapItem = FindHeapItem(p))
        {
            // found:
            if (pHeapItem->fFreed)
            {
                LogError(__FUNCTION__ ": realloc() called with memory block at 0x%lX that was already freed.\n"
                         "This was detected by the realloc() call at %s (%s, line %d).\n"
                         "The block was originally allocated by %s (%s, line %d).",
                         p,
                         pcszFunction,
                             pcszSourceFile,
                             ulLine, // free
                         pHeapItem->pcszFunction,
                             pHeapItem->pcszSourceFile,
                             pHeapItem->ulLine);
            }
            else
            {
                // block is valid:
                void    *pBeforeMagic = ((PBYTE)p) - sizeof(MEMBLOCKMAGIC_HEAD);
                PVOID   pObjNew = 0;
                ULONG   ulError = 0;
                ULONG   cbCopy = 0;

                CheckMagics(__FUNCTION__,
                            pHeapItem,
                            (PBYTE)p,
                            pcszSourceFile,
                            ulLine,
                            pcszFunction);

                // now reallocate!
                pObjNew = malloc(   sizeof(MEMBLOCKMAGIC_HEAD)
                                  + stSize   // new size
                                  + sizeof(MEMBLOCKMAGIC_TAIL));

                // store "front" magic string
                memcpy(pObjNew,
                       MEMBLOCKMAGIC_HEAD,
                       sizeof(MEMBLOCKMAGIC_HEAD));
                // return address: first byte after "front" magic string
                prc = ((PBYTE)pObjNew) + sizeof(MEMBLOCKMAGIC_HEAD);

                // bytes to copy: the smaller of the old and the new size
                cbCopy = pHeapItem->ulSize;
                if (stSize < pHeapItem->ulSize)
                    cbCopy = stSize;

                // copy buffer from old memory object
                memcpy(prc,         // after "front" magic
                       p,
                       cbCopy);

                // store "tail" magic string to block which
                // will be returned plus the size which was requested
                memcpy(((PBYTE)prc) + stSize,
                       MEMBLOCKMAGIC_TAIL,
                       sizeof(MEMBLOCKMAGIC_TAIL));

                // free the old buffer
                free(pBeforeMagic);

                // update the tree, since prc has changed
                treeDelete(&G_pHeapItemsRoot,
                           &G_cHeapItems,
                           (TREE*)pHeapItem);
                // append heap item to linked list
                if (pExisting = FindHeapItem(prc))
                {
                    // a different heap item exists for this address:
                    // delete this one and use that instead; there's
                    // no need to re-insert either
                    free(pHeapItem);
                    pHeapItem = pExisting;
                }

                FillHeapItem(pHeapItem,
                             prc,
                             stSize,
                             pcszSourceFile,
                             ulLine,
                             pcszFunction);

                // insert only if we didn't use an existing item
                if (!pExisting)
                    if (treeInsert(&G_pHeapItemsRoot,
                                   &G_cHeapItems,
                                   (TREE*)pHeapItem,
                                   treeCompareKeys))
                    {
                        LogError(__FUNCTION__ ": treeInsert failed for memory block at 0x%lX.\n"
                                 "The block was allocated by %s (%s, line %d).",
                                 prc,
                                 pcszFunction,
                                     pcszSourceFile,
                                     ulLine);
                    }

            } // if (!pHeapItem->fFreed)
        }
        else
            LogError(__FUNCTION__ ": realloc() called with invalid object from %s (%s, line %d) for object 0x%lX.",
                     pcszFunction,
                         pcszSourceFile,
                         ulLine,
                     p);

        memdUnlock();
    }

    return prc;
}

/*
 *@@ memdReleaseFreed:
 *      goes thru the entire global HEAPITEM's list
 *      and throws out all items which have been freed.
 *      Call this from time to time in order to keep
 *      the system usable. See memdFree() for details.
 *
 *      Returns the no. of HEAPITEM's that have been
 *      released.
 *
 *@@added V0.9.3 (2000-04-11) [umoeller]
 */

unsigned long memdReleaseFreed(void)
{
    BOOL    ulItemsReleased = 0,
            ulBytesReleased = 0;
    if (memdLock())
    {
        /* PHEAPITEM pHeapItem = treeFirst(G_pHeapItemsRoot);

        while (pHeapItem)
        {
            // store next first, because we can change the "next" pointer
            PHEAPITEM   pNext = treeNext(pHeapItem);

            if (pHeapItem->fFreed)
            {
                // item was freed:
                if (pPrevious == NULL)
                    // head of list:
                    G_pHeapItemsRoot = pNext;           // can be NULL
                else
                    // somewhere later:
                    // link next to previous to skip current
                    pPrevious->pNext = pNext;           // can be NULL

                ulItemsReleased++;
                ulBytesReleased += pHeapItem->ulSize;

                if (pHeapItem == G_pHeapItemsLast)
                    // reset "last item" cache
                    G_pHeapItemsLast = NULL;

                free(pHeapItem);
            }
            else
                // item still valid:
                pPrevious = pHeapItem;

            pHeapItem = pNext;
        }
        */
        G_ulItemsReleased += ulItemsReleased;
        G_ulBytesReleased += ulBytesReleased;

        memdUnlock();
    }

    return ulItemsReleased;
}

/* ******************************************************************
 *
 *   XFolder debugging helpers
 *
 ********************************************************************/

#ifdef _PMPRINTF_
    /*
     *@@ memdDumpMemoryBlock:
     *      if _PMPRINTF_ has been #define'd before including
     *      memdebug.h,
     *      this will dump a block of memory to the PMPRINTF
     *      output window. Useful for debugging internal
     *      structures.
     *      If _PMPRINTF_ has been NOT #define'd,
     *      no code will be produced at all. :-)
     */

    /*
    void memdDumpMemoryBlock(PBYTE pb,       // in: start address
                             ULONG ulSize,   // in: size of block
                             ULONG ulIndent) // in: how many spaces to put
                                             //     before each output line
    {
        TRY_QUIET(excpt1)
        {
            PSZ psz;
            if (psz = strhCreateDump(pb, ulSize, ulIndent))
            {
                _Pmpf(("\n%s", psz));
                free(psz);
            }
        CATCH(excpt1)
        {
            _Pmpf(("Crash in " __FUNCTION__ ));
        } END_CATCH();
    }
    */
#endif

#endif

