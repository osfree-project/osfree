
/*
 *@@sourcefile linklist.h:
 *      header file for linklist.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include "helpers\linklist.h"
 */

/*      Copyright (C) 1997-2001 Ulrich M”ller.
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

#ifndef LINKLIST_HEADER_INCLUDED
    #define LINKLIST_HEADER_INCLUDED

    #ifndef XWPENTRY
        #error You must define XWPENTRY to contain the standard linkage for the XWPHelpers.
    #endif

    #include "helpers\simples.h"
            // V0.9.19 (2002-06-13) [umoeller]

    /* LISTITEM was used before V0.9.0 */
    /* typedef struct _LISTITEM {
        struct LIST_STRUCT   *pNext, *pPrevious;
        unsigned long        ulSize;
    } LISTITEM, *PLISTITEM; */

    /*
     *@@ LISTNODE:
     *      this defines one item in a LINKLIST.
     *
     *      pItemData points to the actual data,
     *      whatever format this is in...
     *
     *      See linklist.c for more on how
     *      to use these.
     *
     *@@added V0.9.0
     */

    typedef struct _LISTNODE
    {
        void                *pItemData;
        struct _LISTNODE    *pNext,
                            *pPrevious;
    } LISTNODE, *PLISTNODE;

    /*
     *@@ LINKLIST:
     *      the "root" of a linked list.
     *
     *      See linklist.c for more on how
     *      to use these.
     *
     *@@added V0.9.0
     */

    typedef struct _LINKLIST
    {
        unsigned long   ulMagic;         // integrity check
        unsigned long   ulCount;         // no. of items on list
        PLISTNODE       pFirst,          // first node
                        pLast;           // last node
        BOOL            fItemsFreeable; // as in lstCreate()
    } LINKLIST, *PLINKLIST;

    #define LINKLISTMAGIC 0xf124        // could be anything

    typedef signed short XWPENTRY FNSORTLIST(void*, void*, void*);
                            // changed V0.9.0 (99-10-22) [umoeller]
    typedef FNSORTLIST *PFNSORTLIST;

    /*
     *@@ FOR_ALL_NODES:
     *      helper macro to iterator over all nodes in
     *      a list.
     *
     *      Usage:
     +
     +          PLINKLIST pll = ...;
     +          PLISTNODE pNode;
     +
     +          FOR_ALL_NODES(pll, pNode)
     +          {
     +              PVOID pData = (PVOID)pNode->pItemData;
     +          }
     +
     *      This really is a "for" loop, so you can "break"
     *      out of the loop.
     *
     *@@added V0.9.9 (2001-04-01) [umoeller]
     */

    #define FOR_ALL_NODES(pll, pNode) for (pNode = lstQueryFirstNode(pll); (pNode); pNode = pNode->pNext)

    /* ******************************************************************
     *
     *   List base functions
     *
     ********************************************************************/

    void* XWPENTRY lstMalloc(size_t size);
    typedef void* XWPENTRY LSTMALLOC(size_t size);
    typedef LSTMALLOC *PLSTMALLOC;

    void* XWPENTRY lstStrDup(const char *pcsz);
    typedef void* XWPENTRY LSTSTRDUP(const char *pcsz);
    typedef LSTSTRDUP *PLSTSTRDUP;

    void XWPENTRY lstInit(PLINKLIST pList, BOOL fItemsFreeable);
    typedef void XWPENTRY LSTINIT(PLINKLIST pList, BOOL fItemsFreeable);
    typedef LSTINIT *PLSTINIT;

    #if (defined(__DEBUG_MALLOC_ENABLED__) && !defined(DONT_REPLACE_LIST_MALLOC)) // setup.h, helpers\memdebug.c
        PLINKLIST XWPENTRY lstCreateDebug(BOOL fItemsFreeable,
                                          const char *file,
                                          unsigned long line,
                                          const char *function);
        typedef PLINKLIST XWPENTRY LSTCREATEDEBUG(BOOL fItemsFreeable,
                                                  const char *file,
                                                  unsigned long line,
                                                  const char *function);
        typedef LSTCREATEDEBUG *PLSTCREATEDEBUG;

        #define lstCreate(b) lstCreateDebug((b), __FILE__, __LINE__, __FUNCTION__)
    #else
        PLINKLIST XWPENTRY lstCreate(BOOL fItemsFreeable);
        typedef PLINKLIST XWPENTRY LSTCREATE(BOOL fItemsFreeable);
        typedef LSTCREATE *PLSTCREATE;
    #endif

    BOOL XWPENTRY lstFree(PLINKLIST *ppList);
    typedef BOOL XWPENTRY LSTFREE(PLINKLIST *ppList);
    typedef LSTFREE *PLSTFREE;

    BOOL XWPENTRY lstClear(PLINKLIST pList);
    typedef BOOL XWPENTRY LSTCLEAR(PLINKLIST pList);
    typedef LSTCLEAR *PLSTCLEAR;

    long XWPENTRY lstCountItems(const LINKLIST *pList);
    typedef long XWPENTRY LSTCOUNTITEMS(const LINKLIST *pList);
    typedef LSTCOUNTITEMS *PLSTCOUNTITEMS;

    PLISTNODE XWPENTRY lstQueryFirstNode(const LINKLIST *pList);
    typedef PLISTNODE XWPENTRY LSTQUERYFIRSTNODE(PLINKLIST pList);
    typedef LSTQUERYFIRSTNODE *PLSTQUERYFIRSTNODE;

    PLISTNODE XWPENTRY lstQueryLastNode(const LINKLIST *pList);
    typedef PLISTNODE XWPENTRY LSTQUERYLASTNODE(PLINKLIST pList);
    typedef LSTQUERYLASTNODE *PLSTQUERYLASTNODE;

    PLISTNODE XWPENTRY lstNodeFromIndex(PLINKLIST pList, unsigned long ulIndex);
    typedef PLISTNODE XWPENTRY LSTNODEFROMINDEX(PLINKLIST pList, unsigned long ulIndex);
    typedef LSTNODEFROMINDEX *PLSTNODEFROMINDEX;

    PLISTNODE XWPENTRY lstNodeFromItem(PLINKLIST pList, void* pItemData);
    typedef PLISTNODE XWPENTRY LSTNODEFROMITEM(PLINKLIST pList, void* pItemData);
    typedef LSTNODEFROMITEM *PLSTNODEFROMITEM;

    void* XWPENTRY lstItemFromIndex(PLINKLIST pList, unsigned long ulIndex);
    typedef void* XWPENTRY LSTITEMFROMINDEX(PLINKLIST pList, unsigned long ulIndex);
    typedef LSTITEMFROMINDEX *PLSTITEMFROMINDEX;

    unsigned long lstIndexFromItem(PLINKLIST pList, void *pItemData);
    typedef unsigned long LSTINDEXFROMITEM(PLINKLIST pList, void *pItemData);
    typedef LSTINDEXFROMITEM *PLSTINDEXFROMITEM;

    #if (defined(__DEBUG_MALLOC_ENABLED__) && !defined(DONT_REPLACE_LIST_MALLOC)) // setup.h, helpers\memdebug.c
        PLISTNODE XWPENTRY lstAppendItemDebug(PLINKLIST pList,
                                              void* pNewItemData,
                                              const char *file,
                                              unsigned long line,
                                              const char *function);
        #define lstAppendItem(pl, pd) lstAppendItemDebug((pl), (pd), __FILE__, __LINE__, __FUNCTION__)
    #else
        PLISTNODE XWPENTRY lstAppendItem(PLINKLIST pList, void* pNewItemData);
        typedef PLISTNODE XWPENTRY LSTAPPENDITEM(PLINKLIST pList, void* pNewItemData);
        typedef LSTAPPENDITEM *PLSTAPPENDITEM;
    #endif

    PLISTNODE XWPENTRY lstInsertItemBefore(PLINKLIST pList,
                                           void* pNewItemData,
                                           unsigned long ulIndex);
    typedef PLISTNODE XWPENTRY LSTINSERTITEMBEFORE(PLINKLIST pList,
                                                   void* pNewItemData,
                                                   unsigned long ulIndex);
    typedef LSTINSERTITEMBEFORE *PLSTINSERTITEMBEFORE;

    PLISTNODE XWPENTRY lstInsertItemAfterNode(PLINKLIST pList,
                                              void* pNewItemData,
                                              PLISTNODE pNodeInsertAfter);
    typedef PLISTNODE XWPENTRY LSTINSERTITEMAFTERNODE(PLINKLIST pList,
                                                      void* pNewItemData,
                                                      PLISTNODE pNodeInsertAfter);
    typedef LSTINSERTITEMAFTERNODE *PLSTINSERTITEMAFTERNODE;

    BOOL XWPENTRY lstRemoveNode(PLINKLIST pList, PLISTNODE pRemoveNode);
    typedef BOOL XWPENTRY LSTREMOVENODE(PLINKLIST pList, PLISTNODE pRemoveNode);
    typedef LSTREMOVENODE *PLSTREMOVENODE;

    BOOL XWPENTRY lstRemoveItem(PLINKLIST pList, void* pRemoveItem);
    typedef BOOL XWPENTRY LSTREMOVEITEM(PLINKLIST pList, void* pRemoveItem);
    typedef LSTREMOVEITEM *PLSTREMOVEITEM;

    BOOL XWPENTRY lstSwapNodes(PLISTNODE pNode1, PLISTNODE pNode2);
    typedef BOOL XWPENTRY LSTSWAPNODES(PLISTNODE pNode1, PLISTNODE pNode2);
    typedef LSTSWAPNODES *PLSTSWAPNODES;

    /* ******************************************************************
     *
     *   List sorting
     *
     ********************************************************************/

    BOOL XWPENTRY lstQuickSort(PLINKLIST pList,
                               PFNSORTLIST pfnSort,
                               void* pStorage);

    BOOL XWPENTRY lstBubbleSort(PLINKLIST pList,
                                PFNSORTLIST pfnSort,
                                void* pStorage);

    /* ******************************************************************
     *
     *   List pseudo-stacks
     *
     ********************************************************************/

    PLISTNODE lstPush(PLINKLIST pList,
                      void* pNewItemData);

    PLISTNODE lstPop(PLINKLIST pList);

#endif

#if __cplusplus
}
#endif

