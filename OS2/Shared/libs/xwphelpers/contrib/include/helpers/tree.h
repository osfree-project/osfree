
/*
 *@@sourcefile tree.h:
 *      header file for tree.c (red-black balanced binary trees).
 *      See remarks there.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef XWPTREE_INCLUDED
    #define XWPTREE_INCLUDED

    #include "helpers/simples.h"
            // V0.9.19 (2002-06-13) [umoeller]

    typedef enum { BLACK, RED } nodeColor;

    /*
     *@@ TREE:
     *      tree node.
     *
     *      To use the tree functions, all your structures
     *      must have a TREE structure as their first member.
     *
     *      Example:
     *
     +      typedef struct _MYTREENODE
     +      {
     +          TREE        tree;
     +          char        acMyData[1000];
     +      } MYTREENODE, *PMYTREENODE;
     *
     *      See tree.c for an introduction to the tree functions.
     */

    typedef struct _TREE
    {
        struct _TREE    *left,
                        *right,
                        *parent;
        nodeColor       color;          // the node's color (BLACK, RED)

        ULONG           ulKey;          // the node's key (data)

    } TREE, *PTREE;

    #if defined(__IBMC__) || defined(__IBMCPP__)
        #define TREEENTRY _Optlink
    #else
        // EMX or whatever: doesn't know calling conventions
        #define TREEENTRY
    #endif

    #define STATUS_OK                   0
    #define STATUS_DUPLICATE_KEY        -1
    #define STATUS_INVALID_NODE         -2

    typedef int TREEENTRY FNTREE_COMPARE(ULONG ul1, ULONG ul2);

    //  Function prototypes
    void treeInit(TREE **root,
                  PLONG plCount);

    int TREEENTRY treeCompareKeys(ULONG  ul1, ULONG ul2);

    int TREEENTRY treeCompareStrings(ULONG  ul1, ULONG ul2);

    int treeInsert(TREE **root,
                   PLONG plCount,
                   TREE *x,
                   FNTREE_COMPARE *pfnCompare);

    int treeDelete(TREE **root,
                   PLONG plCount,
                   TREE *z);

    TREE* treeFind(TREE *root,
                   ULONG key,
                   FNTREE_COMPARE *pfnCompare);

    TREE* treeFirst(TREE *r);

    TREE* treeLast(TREE *r);

    TREE* treeNext(TREE *r);

    TREE* treePrev(TREE *r);

    TREE** treeBuildArray(TREE* pRoot,
                          PLONG plCount);

#endif

#if __cplusplus
}
#endif

