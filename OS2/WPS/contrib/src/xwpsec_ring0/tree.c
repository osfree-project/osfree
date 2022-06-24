
/*
 *@@sourcefile util_tree.c:
 *      a copy of src\helpers\tree.c, with slight omissions
 *      to make up for ring 0.
 *
 *      See strat_init_base.c for an introduction.
 */

/*
 *      Original coding by Thomas Niemann, placed in the public domain
 *      (see http://epaperpress.com/sortsearch/index.html).
 *
 *      This implementation Copyright (C) 2001 Ulrich M”ller.
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

/*
 *@@category: Helpers\C helpers\Red-black balanced binary trees
 *      See tree.c.
 */

#include <string.h>

#include "helpers\tree.h"

#define LEAF &sentinel           // all leafs are sentinels
static TREE sentinel = { LEAF, LEAF, 0, BLACK};

/*
A binary search tree is a red-black tree if:

1. Every node is either red or black.
2. Every leaf (nil) is black.
3. If a node is red, then both its children are black.
4. Every simple path from a node to a descendant leaf contains the same
   number of black nodes.
*/

/*
 *@@ treeInit:
 *      initializes the root of a tree.
 *
 *      If (plCount != NULL), *plCount is set to null also.
 *      This same plCount pointer can then be passed to
 *      treeInsert and treeDelete also to automatically
 *      maintain a tree item count.
 *
 *@@changed V0.9.16 (2001-10-19) [umoeller]: added plCount
 */

void treeInit(TREE **root,
              PLONG plCount)            // out: tree item count, set to 0 (ptr can be NULL)
{
    *root = LEAF;

    if (plCount)
        *plCount = 0;       // V0.9.16 (2001-10-19) [umoeller]
}

/*
 *@@ treeCompareKeys:
 *      standard comparison func if the TREE.ulKey
 *      field really is a ULONG.
 */

int TREEENTRY treeCompareKeys(unsigned long  ul1, unsigned long ul2)
{
    if (ul1 < ul2)
        return -1;
    if (ul1 > ul2)
        return +1;
    return 0;
}

/*
 *@@ treeCompareStrings:
 *      standard comparison func if the TREE.ulKey
 *      field really is a string pointer (PCSZ).
 *
 *      This runs strcmp internally, but can handle
 *      NULL pointers without crashing.
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 */

int TREEENTRY treeCompareStrings(unsigned long  ul1, unsigned long ul2)
{
    #define p1 (const char*)(ul1)
    #define p2 (const char*)(ul2)

    if (p1 && p2)
    {
        int i = strcmp(p1, p2);
        if (i < 0) return (-1);
        if (i > 0) return (+1);
    }
    else if (p1)
        // but p2 is NULL: p1 greater than p2 then
        return (+1);
    else if (p2)
        // but p1 is NULL: p1 less than p2 then
        return (-1);

    // return 0 if strcmp returned 0 above or both strings are NULL
    return 0;
}

/*
 *@@ rotateLeft:
 *      private function during rebalancing.
 */

static void rotateLeft(TREE **root,
                       TREE *x)
{
   /**************************
    *  rotate node x to left *
    **************************/

    TREE *y = x->right;

    // establish x->right link
    x->right = y->left;
    if (y->left != LEAF)
        y->left->parent = x;

    // establish y->parent link
    if (y != LEAF)
        y->parent = x->parent;
    if (x->parent)
    {
        if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
    }
    else
        *root = y;

    // link x and y
    y->left = x;
    if (x != LEAF)
        x->parent = y;
}

/*
 *@@ rotateRight:
 *      private function during rebalancing.
 */

static void rotateRight(TREE **root,
                        TREE *x)
{

   /****************************
    *  rotate node x to right  *
    ****************************/

    TREE *y = x->left;

    // establish x->left link
    x->left = y->right;
    if (y->right != LEAF)
        y->right->parent = x;

    // establish y->parent link
    if (y != LEAF)
        y->parent = x->parent;
    if (x->parent)
    {
        if (x == x->parent->right)
            x->parent->right = y;
        else
            x->parent->left = y;
    }
    else
        *root = y;

    // link x and y
    y->right = x;
    if (x != LEAF)
        x->parent = y;
}

/*
 *@@ insertFixup:
 *      private function during rebalancing.
 */

static void insertFixup(TREE **root,
                        TREE *x)
{
   /*************************************
    *  maintain Red-Black tree balance  *
    *  after inserting node x           *
    *************************************/

    // check Red-Black properties
    while (    x != *root
            && x->parent->color == RED
          )
    {
        // we have a violation
        if (x->parent == x->parent->parent->left)
        {
            TREE *y = x->parent->parent->right;
            if (y->color == RED)
            {
                // uncle is RED
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            }
            else
            {
                // uncle is BLACK
                if (x == x->parent->right)
                {
                    // make x a left child
                    x = x->parent;
                    rotateLeft(root,
                               x);
                }

                // recolor and rotate
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rotateRight(root,
                            x->parent->parent);
            }
        }
        else
        {
            // mirror image of above code
            TREE *y = x->parent->parent->left;
            if (y->color == RED)
            {
                // uncle is RED
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            }
            else
            {
                // uncle is BLACK
                if (x == x->parent->left)
                {
                    x = x->parent;
                    rotateRight(root,
                                x);
                }
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rotateLeft(root,
                           x->parent->parent);
            }
        }
    }
    (*root)->color = BLACK;
}

/*
 *@@ treeInsert:
 *      inserts a new tree node into the specified
 *      tree, using the specified comparison function
 *      for sorting.
 *
 *      "x" specifies the new tree node which must
 *      have been allocated by the caller. x->ulKey
 *      must already contain the node's key (data)
 *      which the sort function can understand.
 *
 *      This function will then set the parent,
 *      left, right, and color members. In addition,
 *      if (plCount != NULL), *plCount is raised by
 *      one.
 *
 *      Returns 0 if no error. Might return
 *      STATUS_DUPLICATE_KEY if a node with the
 *      same ulKey already exists.
 *
 *@@changed V0.9.16 (2001-10-19) [umoeller]: added plCount
 */

int treeInsert(TREE **root,                     // in: root of the tree
               PLONG plCount,                   // in/out: item count (ptr can be NULL)
               TREE *x,                         // in: new node to insert
               FNTREE_COMPARE *pfnCompare)      // in: comparison func
{
    TREE    *current,
            *parent;

    unsigned long key = x->ulKey;

    // find future parent
    current = *root;
    parent = 0;

    while (current != LEAF)
    {
        int iResult;
        if (0 == (iResult = pfnCompare(key, current->ulKey))) // if (compEQ(key, current->key))
            return STATUS_DUPLICATE_KEY;

        parent = current;
        current = (iResult < 0)    // compLT(key, current->key)
                    ? current->left
                    : current->right;
    }

    // set up new node
    /* if ((x = malloc (sizeof(*x))) == 0)
        return STATUS_MEM_EXHAUSTED; */
    x->parent = parent;
    x->left = LEAF;
    x->right = LEAF;
    x->color = RED;
    // x->key = key;
    // x->rec = *rec;

    // insert node in tree
    if (parent)
    {
        if (pfnCompare(key, parent->ulKey) < 0) // (compLT(key, parent->key))
            parent->left = x;
        else
            parent->right = x;
    }
    else
        *root = x;

    insertFixup(root,
                x);
    // lastFind = NULL;

    if (plCount)
        (*plCount)++;       // V0.9.16 (2001-10-19) [umoeller]

    return STATUS_OK;
}

/*
 *@@ deleteFixup:
 *
 */

static void deleteFixup(TREE **root,
                        TREE *tree)
{
    TREE    *s;

    while (    tree != *root
            && tree->color == BLACK
          )
    {
        if (tree == tree->parent->left)
        {
            s = tree->parent->right;
            if (s->color == RED)
            {
                s->color = BLACK;
                tree->parent->color = RED;
                rotateLeft(root, tree->parent);
                s = tree->parent->right;
            }
            if (    (s->left->color == BLACK)
                 && (s->right->color == BLACK)
               )
            {
                s->color = RED;
                tree = tree->parent;
            }
            else
            {
                if (s->right->color == BLACK)
                {
                    s->left->color = BLACK;
                    s->color = RED;
                    rotateRight(root, s);
                    s = tree->parent->right;
                }
                s->color = tree->parent->color;
                tree->parent->color = BLACK;
                s->right->color = BLACK;
                rotateLeft(root, tree->parent);
                tree = *root;
            }
        }
        else
        {
            s = tree->parent->left;
            if (s->color == RED)
            {
                s->color = BLACK;
                tree->parent->color = RED;
                rotateRight(root, tree->parent);
                s = tree->parent->left;
            }
            if (    (s->right->color == BLACK)
                 && (s->left->color == BLACK)
               )
            {
                s->color = RED;
                tree = tree->parent;
            }
            else
            {
                if (s->left->color == BLACK)
                {
                    s->right->color = BLACK;
                    s->color = RED;
                    rotateLeft(root, s);
                    s = tree->parent->left;
                }
                s->color = tree->parent->color;
                tree->parent->color = BLACK;
                s->left->color = BLACK;
                rotateRight (root, tree->parent);
                tree = *root;
            }
        }
    }
    tree->color = BLACK;

   /*************************************
    *  maintain Red-Black tree balance  *
    *  after deleting node x            *
    *************************************/

    /* while (    x != *root
            && x->color == BLACK
          )
    {
        if (x == x->parent->left)
        {
            TREE *w = x->parent->right;
            if (w->color == RED)
            {
                w->color = BLACK;
                x->parent->color = RED;
                rotateLeft(root,
                           x->parent);
                w = x->parent->right;
            }
            if (    w->left->color == BLACK
                 && w->right->color == BLACK
               )
            {
                w->color = RED;
                x = x->parent;
            }
            else
            {
                if (w->right->color == BLACK)
                {
                    w->left->color = BLACK;
                    w->color = RED;
                    rotateRight(root,
                                w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rotateLeft(root,
                           x->parent);
                x = *root;
            }
        }
        else
        {
            TREE *w = x->parent->left;
            if (w->color == RED)
            {
                w->color = BLACK;
                x->parent->color = RED;
                rotateRight(root,
                            x->parent);
                w = x->parent->left;
            }
            if (    w->right->color == BLACK
                 && w->left->color == BLACK
               )
            {
                w->color = RED;
                x = x->parent;
            }
            else
            {
                if (w->left->color == BLACK)
                {
                    w->right->color = BLACK;
                    w->color = RED;
                    rotateLeft(root,
                               w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rotateRight(root,
                            x->parent);
                x = *root;
            }
        }
    }
    x->color = BLACK; */
}

/*
 *@@ treeDelete:
 *      removes the specified node from the tree.
 *      Does not free() the node though.
 *
 *      In addition, if (plCount != NULL), *plCount is
 *      decremented.
 *
 *      Returns 0 if the node was deleted or
 *      STATUS_INVALID_NODE if not.
 *
 *@@changed V0.9.16 (2001-10-19) [umoeller]: added plCount
 */

int treeDelete(TREE **root,         // in: root of the tree
               PLONG plCount,       // in/out: item count (ptr can be NULL)
               TREE *tree)          // in: tree node to delete
{
    TREE        *y,
                *d;
    nodeColor   color;

    if (    (!tree)
         || (tree == LEAF)
       )
        return STATUS_INVALID_NODE;

    if (    (tree->left  == LEAF)
         || (tree->right == LEAF)
       )
        // d has a TREE_NULL node as a child
        d = tree;
    else
    {
        // find tree successor with a TREE_NULL node as a child
        d = tree->right;
        while (d->left != LEAF)
            d = d->left;
    }

    // y is d's only child, if there is one, else TREE_NULL
    if (d->left != LEAF)
        y = d->left;
    else
        y = d->right;

    // remove d from the parent chain
    if (y != LEAF)
        y->parent = d->parent;
    if (d->parent)
    {
        if (d == d->parent->left)
            d->parent->left  = y;
        else
            d->parent->right = y;
    }
    else
        *root = y;

    color = d->color;

    if (d != tree)
    {
        // move the data from d to tree; we do this by
        // linking d into the structure in the place of tree
        d->left   = tree->left;
        d->right  = tree->right;
        d->parent = tree->parent;
        d->color  = tree->color;

        if (d->parent)
        {
            if (tree == d->parent->left)
                d->parent->left  = d;
            else
                d->parent->right = d;
        }
        else
            *root = d;

        if (d->left != LEAF)
            d->left->parent = d;

        if (d->right != LEAF)
            d->right->parent = d;
    }

    if (    (y != LEAF)
         && (color == BLACK)
       )
        deleteFixup(root,
                    y);

    if (plCount)
        (*plCount)--;       // V0.9.16 (2001-10-19) [umoeller]

    return (STATUS_OK);
}

/*
 *@@ treeFind:
 *      finds the tree node with the specified key.
 *      Returns NULL if none exists.
 */

TREE* treeFind(TREE *root,                    // in: root of the tree
               unsigned long key,             // in: key to find
               FNTREE_COMPARE *pfnCompare)    // in: comparison func
{
   /*******************************
    *  find node containing data  *
    *******************************/

    TREE *current = root;
    while (current != LEAF)
    {
        int iResult;
        if (0 == (iResult = pfnCompare(key, current->ulKey)))
            return (current);
        else
        {
            current = (iResult < 0) // compLT (key, current->key)
                ? current->left
                : current->right;
        }
    }

    return 0;
}

/*
 *@@ treeFirst:
 *      finds and returns the first node in a (sub-)tree.
 *
 *      See treeNext for a sample usage for traversing a tree.
 */

TREE* treeFirst(TREE *r)
{
    TREE    *p;

    if (    (!r)
         || (r == LEAF)
       )
        return NULL;

    p = r;
    while (p->left != LEAF)
        p = p->left;

    return p;
}

/*
 *@@ treeLast:
 *      finds and returns the last node in a (sub-)tree.
 */

TREE* treeLast(TREE *r)
{
    TREE    *p;

    if (    (!r)
         || (r == LEAF))
        return NULL;

    p = r;
    while (p->right != LEAF)
        p = p->right;

    return p;
}

/*
 *@@ treeNext:
 *      finds and returns the next node in a tree.
 *
 *      Example for traversing a whole tree:
 *
 +          TREE    *TreeRoot;
 +          ...
 +          TREE* pNode = treeFirst(TreeRoot);
 +          while (pNode)
 +          {
 +              ...
 +              pNode = treeNext(pNode);
 +          }
 *
 *      This runs through the tree items in sorted order.
 */

TREE* treeNext(TREE *r)
{
    TREE    *p,
            *child;

    if (    (!r)
         || (r == LEAF)
       )
        return NULL;

    p = r;
    if (p->right != LEAF)
        return treeFirst (p->right);
    else
    {
        p = r;
        child   = LEAF;
        while (    (p->parent)
                && (p->right == child)
              )
        {
            child = p;
            p = p->parent;
        }
        if (p->right != child)
            return p;
        else
            return NULL;
    }
}

/*
 *@@ treePrev:
 *      finds and returns the previous node in a tree.
 */

TREE* treePrev(TREE *r)
{
    TREE    *p,
            *child;

    if (    (!r)
         || (r == LEAF))
        return NULL;

    p = r;
    if (p->left != LEAF)
        return treeLast (p->left);
    else
    {
        p = r;
        child   = LEAF;
        while ((p->parent)
           &&  (p->left == child))
        {
            child = p;
            p = p->parent;
        }
        if (p->left != child)
            return p;
        else
            return NULL;
    }
}


