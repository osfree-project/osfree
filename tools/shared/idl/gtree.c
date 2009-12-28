/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
 * Modified by Guilherme Balena Versiani 2007. All thread-safety were
 * removed as lightIDL does not need to be thread-safe. Next versions
 * could have all GLib original code removed.
 */

#include "glib.h"


typedef struct _GRealTree  GRealTree;
typedef struct _GTreeNode  GTreeNode;

struct _GRealTree
{
    GTreeNode *root;
    GCompareFunc key_compare;
};

struct _GTreeNode
{
    gint balance;      /* height (left) - height (right) */
    GTreeNode *left;   /* left subtree */
    GTreeNode *right;  /* right subtree */
    gpointer key;      /* key for this node */
    gpointer value;    /* value stored at this node */
};


static GTreeNode* g_tree_node_new                   (gpointer        key,
                                                     gpointer        value);
static void       g_tree_node_destroy               (GTreeNode      *node);
static GTreeNode* g_tree_node_insert                (GTreeNode      *node,
                                                     GCompareFunc    compare,
                                                     gpointer        key,
                                                     gpointer        value,
                                                     gint           *inserted);
static GTreeNode* g_tree_node_remove                (GTreeNode      *node,
                                                     GCompareFunc    compare,
                                                     gpointer        key);
static GTreeNode* g_tree_node_balance               (GTreeNode      *node);
static GTreeNode* g_tree_node_remove_leftmost       (GTreeNode      *node,
                                                     GTreeNode     **leftmost);
static GTreeNode* g_tree_node_restore_left_balance  (GTreeNode      *node,
                                                     gint            old_balance);
static GTreeNode* g_tree_node_restore_right_balance (GTreeNode      *node,
                                                     gint            old_balance);
static gpointer   g_tree_node_lookup                (GTreeNode      *node,
                                                     GCompareFunc    compare,
                                                     gpointer        key);
static GTreeNode* g_tree_node_rotate_left           (GTreeNode      *node);
static GTreeNode* g_tree_node_rotate_right          (GTreeNode      *node);


static GTreeNode*
g_tree_node_new(gpointer key,
                gpointer value)
{
    GTreeNode *node;

    node = g_new0(GTreeNode, 1);
    node->balance = 0;
    node->left = NULL;
    node->right = NULL;
    node->key = key;
    node->value = value;
    return node;
}

static void
g_tree_node_destroy(GTreeNode *node)
{
    if (node) {
        g_tree_node_destroy(node->right);
        g_tree_node_destroy(node->left);
        g_free(node);
    }
}

GTree* g_tree_new(GCompareFunc key_compare_func)
{
    GRealTree *rtree;

    rtree = g_new0(GRealTree, 1);
    rtree->key_compare = key_compare_func;

    return (GTree*) rtree;
}

void g_tree_destroy(GTree *tree)
{
    GRealTree *rtree;

    rtree = (GRealTree*) tree;
    g_tree_node_destroy(rtree->root);
    g_free(rtree);
}

void g_tree_insert(GTree *tree, gpointer key, gpointer value)
{
    GRealTree *rtree;
    gint inserted;

    rtree = (GRealTree*) tree;
    inserted = FALSE;
    rtree->root = g_tree_node_insert(rtree->root, rtree->key_compare,
                    key, value, &inserted);
}

void g_tree_remove(GTree *tree, gpointer key)
{
    GRealTree *rtree;

    rtree = (GRealTree*) tree;
    rtree->root = g_tree_node_remove(rtree->root, rtree->key_compare, key);
}

gpointer g_tree_lookup(GTree *tree, gpointer key)
{
    GRealTree *rtree;

    rtree = (GRealTree*) tree;
    return g_tree_node_lookup(rtree->root, rtree->key_compare, key);
}

static GTreeNode*
g_tree_node_insert(GTreeNode *node, GCompareFunc compare, gpointer key,
                   gpointer value, gint *inserted)
{
    gint old_balance;
    gint cmp;

    if (!node) {
        *inserted = TRUE;
        return g_tree_node_new(key, value);
    }

    cmp = (*compare)(key, node->key);
    if (cmp == 0) {
        *inserted = FALSE;
        node->value = value;
        return node;
    }

    if (cmp < 0) {
        if (node->left) {
            old_balance = node->left->balance;
            node->left = g_tree_node_insert(node->left, compare, key, value, inserted);

            if ((old_balance != node->left->balance) && node->left->balance)
                node->balance -= 1;
        }
        else {
            *inserted = TRUE;
            node->left = g_tree_node_new(key, value);
            node->balance -= 1;
        }
    }
    else if (cmp > 0) {
        if (node->right) {
            old_balance = node->right->balance;
            node->right = g_tree_node_insert(node->right, compare, key, value, inserted);

            if ((old_balance != node->right->balance) && node->right->balance)
                node->balance += 1;
        }
        else {
            *inserted = TRUE;
            node->right = g_tree_node_new(key, value);
            node->balance += 1;
        }
    }

    if (*inserted) {
        if ((node->balance < -1) || (node->balance > 1))
            node = g_tree_node_balance (node);
    }

    return node;
}

static GTreeNode*
g_tree_node_remove(GTreeNode *node, GCompareFunc compare, gpointer key)
{
    GTreeNode *new_root;
    gint old_balance;
    gint cmp;

    if (!node)
        return NULL;

    cmp = (*compare)(key, node->key);
    if (cmp == 0) {
        GTreeNode *garbage;

        garbage = node;
        if (!node->right)
            node = node->left;
        else {
            old_balance = node->right->balance;
            node->right = g_tree_node_remove_leftmost(node->right, &new_root);
            new_root->left = node->left;
            new_root->right = node->right;
            new_root->balance = node->balance;
            node = g_tree_node_restore_right_balance(new_root, old_balance);
        }

        g_free(garbage);
    }
    else if (cmp < 0) {
        if (node->left) {
            old_balance = node->left->balance;
            node->left = g_tree_node_remove(node->left, compare, key);
            node = g_tree_node_restore_left_balance(node, old_balance);
        }
    }
    else if (cmp > 0) {
        if (node->right) {
            old_balance = node->right->balance;
            node->right = g_tree_node_remove (node->right, compare, key);
            node = g_tree_node_restore_right_balance (node, old_balance);
        }
    }

    return node;
}

static GTreeNode*
g_tree_node_balance(GTreeNode *node)
{
    if (node->balance < -1) {
        if (node->left->balance > 0)
            node->left = g_tree_node_rotate_left(node->left);
        node = g_tree_node_rotate_right(node);
    }
    else if (node->balance > 1) {
        if (node->right->balance < 0)
            node->right = g_tree_node_rotate_right(node->right);
        node = g_tree_node_rotate_left(node);
    }

    return node;
}

static GTreeNode*
g_tree_node_remove_leftmost(GTreeNode *node, GTreeNode **leftmost)
{
    gint old_balance;

    if (!node->left) {
        *leftmost = node;
        return node->right;
    }

    old_balance = node->left->balance;
    node->left = g_tree_node_remove_leftmost(node->left, leftmost);
    return g_tree_node_restore_left_balance(node, old_balance);
}

static GTreeNode*
g_tree_node_restore_left_balance(GTreeNode *node, gint old_balance)
{
    if (!node->left)
        node->balance += 1;
    else if ((node->left->balance != old_balance) &&
             (node->left->balance == 0))
        node->balance += 1;

    if (node->balance > 1)
        return g_tree_node_balance(node);
    return node;
}

static GTreeNode*
g_tree_node_restore_right_balance(GTreeNode *node, gint old_balance)
{
    if (!node->right)
        node->balance -= 1;
    else if ((node->right->balance != old_balance) &&
             (node->right->balance == 0))
        node->balance -= 1;

    if (node->balance < -1)
        return g_tree_node_balance(node);
    return node;
}

static gpointer
g_tree_node_lookup(GTreeNode *node, GCompareFunc  compare, gpointer key)
{
    gint cmp;

    if (!node)
        return NULL;

    cmp = (*compare)(key, node->key);
    if (cmp == 0)
        return node->value;

    if (cmp < 0) {
        if (node->left)
            return g_tree_node_lookup (node->left, compare, key);
    }
    else if (cmp > 0) {
        if (node->right)
            return g_tree_node_lookup (node->right, compare, key);
    }

    return NULL;
}

static GTreeNode*
g_tree_node_rotate_left (GTreeNode *node)
{
    GTreeNode *left;
    GTreeNode *right;
    gint a_bal;
    gint b_bal;

    left = node->left;
    right = node->right;

    node->right = right->left;
    right->left = node;

    a_bal = node->balance;
    b_bal = right->balance;

    if (b_bal <= 0) {
        if (a_bal >= 1)
            right->balance = b_bal - 1;
        else
            right->balance = a_bal + b_bal - 2;
        node->balance = a_bal - 1;
    }
    else {
        if (a_bal <= b_bal)
            right->balance = a_bal - 2;
        else
            right->balance = b_bal - 1;
        node->balance = a_bal - b_bal - 1;
    }

    return right;
}

static GTreeNode*
g_tree_node_rotate_right (GTreeNode *node)
{
    GTreeNode *left;
    gint a_bal;
    gint b_bal;

    left = node->left;

    node->left = left->right;
    left->right = node;

    a_bal = node->balance;
    b_bal = left->balance;

    if (b_bal <= 0) {
        if (b_bal > a_bal)
            left->balance = b_bal + 1;
        else
            left->balance = a_bal + 2;
        node->balance = a_bal - b_bal + 1;
    }
    else {
        if (a_bal <= -1)
            left->balance = b_bal + 1;
        else
            left->balance = a_bal + b_bal + 2;
        node->balance = a_bal + 1;
    }

    return left;
}
