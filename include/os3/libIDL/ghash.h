/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __G_HASH_H__
#define __G_HASH_H__

//#include <glib/gtypes.h>
#include "glist.h"
#include "gprimes.h"

//G_BEGIN_DECLS

typedef struct _GHashTable  GHashTable;

typedef int  (*GHRFunc)  (void *  key,
                               void *  value,
                               void *  user_data);

typedef struct _GHashTableIter GHashTableIter;

struct _GHashTableIter
{
  /*< private >*/
  void *      dummy1;
  void *      dummy2;
  void *      dummy3;
  int           dummy4;
  int      dummy5;
  void *      dummy6;
};

/* Hash tables
 */
GHashTable* g_hash_table_new               (GHashFunc       hash_func,
                                            GEqualFunc      key_equal_func);
GHashTable* g_hash_table_new_full          (GHashFunc       hash_func,
                                            GEqualFunc      key_equal_func,
                                            GDestroyNotify  key_destroy_func,
                                            GDestroyNotify  value_destroy_func);
void        g_hash_table_destroy           (GHashTable     *hash_table);
void        g_hash_table_insert            (GHashTable     *hash_table,
                                            void *        key,
                                            void *        value);
void        g_hash_table_replace           (GHashTable     *hash_table,
                                            void *        key,
                                            void *        value);
int    g_hash_table_remove            (GHashTable     *hash_table,
                                            const void *   key);
void        g_hash_table_remove_all        (GHashTable     *hash_table);
int    g_hash_table_steal             (GHashTable     *hash_table,
                                            const void *   key);
void        g_hash_table_steal_all         (GHashTable     *hash_table);
void *    g_hash_table_lookup            (GHashTable     *hash_table,
                                            const void *   key);
int    g_hash_table_lookup_extended   (GHashTable     *hash_table,
                                            const void *   lookup_key,
                                            void *       *orig_key,
                                            void *       *value);
void        g_hash_table_foreach           (GHashTable     *hash_table,
                                            GHFunc          func,
                                            void *        user_data);
void *    g_hash_table_find              (GHashTable     *hash_table,
                                            GHRFunc         predicate,
                                            void *        user_data);
unsigned int       g_hash_table_foreach_remove    (GHashTable     *hash_table,
                                            GHRFunc         func,
                                            void *        user_data);
unsigned int       g_hash_table_foreach_steal     (GHashTable     *hash_table,
                                            GHRFunc         func,
                                            void *        user_data);
unsigned int       g_hash_table_size              (GHashTable     *hash_table);
GList *     g_hash_table_get_keys          (GHashTable     *hash_table);
GList *     g_hash_table_get_values        (GHashTable     *hash_table);

void        g_hash_table_iter_init         (GHashTableIter *iter,
                                            GHashTable     *hash_table);
int    g_hash_table_iter_next         (GHashTableIter *iter,
                                            void *       *key,
                                            void *       *value);
GHashTable* g_hash_table_iter_get_hash_table (GHashTableIter *iter);
void        g_hash_table_iter_remove       (GHashTableIter *iter);
void        g_hash_table_iter_steal        (GHashTableIter *iter);

/* keeping hash tables alive */
GHashTable* g_hash_table_ref               (GHashTable     *hash_table);
void        g_hash_table_unref             (GHashTable     *hash_table);

#ifndef G_DISABLE_DEPRECATED

/* The following two functions are deprecated and will be removed in
 * the next major release. They do no good. */
#define g_hash_table_freeze(hash_table) ((void)0)
#define g_hash_table_thaw(hash_table) ((void)0)

#endif /* G_DISABLE_DEPRECATED */

/* Hash Functions
 */
int g_str_equal (const void *  v1,
                      const void *  v2);
unsigned int    g_str_hash  (const void *  v);

int g_int_equal (const void *  v1,
                      const void *  v2);
unsigned int    g_int_hash  (const void *  v);

/* This "hash" function will just return the key's address as an
 * unsigned integer. Useful for hashing on plain addresses or
 * simple integer values.
 * Passing NULL into g_hash_table_new() as GHashFunc has the
 * same effect as passing g_direct_hash().
 */
unsigned int    g_direct_hash  (const void *  v) ;
int g_direct_equal (const void *  v1,
                         const void *  v2);

//G_END_DECLS

#endif /* __G_HASH_H__ */

