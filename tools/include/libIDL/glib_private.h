/**************************************************************************

    glib_private.h (fake GLib private header)

    Copyright (C) 2007 Guilherme Balena Versiani

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id: glib_private.h,v 1.4 2008/04/28 05:17:21 balena Exp $

***************************************************************************/

/* This is not the GLib header! */

#ifndef __G_LIB_PRIVATE_H__
#define __G_LIB_PRIVATE_H__


#include "glib.h"
#include "ghash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef _WIN32
#define G_DIR_SEPARATOR '\\'
#define G_DIR_SEPARATOR_S "\\"
#define G_SEARCHPATH_SEPARATOR ';'
#define G_SEARCHPATH_SEPARATOR_S ";"
#else  /* _WIN32 */
#define G_DIR_SEPARATOR '/'
#define G_DIR_SEPARATOR_S "/"
#define G_SEARCHPATH_SEPARATOR ':'
#define G_SEARCHPATH_SEPARATOR_S ":"
#endif

/* Microsoft Visual C++ ISO conformance */
#ifdef _WIN32
#define stricmp   _stricmp
#define putenv    _putenv
#define unlink    _unlink
#else
#define stricmp   strcasecmp
#endif

#if defined(_WIN32) && defined(_MSC_VER)
#include <direct.h>
#define getcwd _getcwd
#define __STDC__ 1
#endif

#include "libchash.h"
#include "queue.h"


/* inline */
#if defined(_WIN32) && defined(_MSC_VER)
#define inline __inline
#endif


/* GLib structures */
struct _GSList
{
    gpointer data;
    GSList *next;
};

struct _GString
{
    gchar *str;
    gint len;
    gint alloc;
};


/* GLib values */
#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#ifndef NULL
#define NULL    ((void*) 0)
#endif

/* GLib macros */
#define GPOINTER_TO_INT(x)  ((gint) (x))
#define GPOINTER_TO_UINT(p)     ((guint)(p))
#define GINT_TO_POINTER(x)  ((gpointer) (x))
#define GUINT_TO_POINTER(u)     ((gpointer)(u))

/* GLib functions */
#define g_ascii_strcasecmp      stricmp
#define g_ascii_isspace         isspace
#define g_ascii_isdigit         isdigit

#if defined(_WIN32)
#define g_strdup                _strdup
#else
#define g_strdup                strdup
#endif

#define g_malloc                malloc
#define g_malloc0(x)            calloc(x,1)
#define g_free                  free
#define g_assert                assert

/*
#define g_hash_table_new(x,y)   AllocateHashTable(0,1)

static inline
gpointer g_hash_table_lookup(GHashTable *hash_table,
                             gconstpointer key)
{
    HTItem *bk = HashFind(hash_table, PTR_KEY(hash_table, key));
    if (bk)
        return (gpointer) bk->data;
    return NULL;
}

static inline
gboolean g_hash_table_lookup_extended(GHashTable *hash_table,
                                      gconstpointer lookup_key,
                                      gpointer *orig_key,
                                      gpointer *value)
{
    HTItem *bk = HashFind(hash_table, PTR_KEY(hash_table, lookup_key));
printf("ptrkey=%x\n", PTR_KEY(hash_table, lookup_key));
    if (bk) {
        if (orig_key)
            *orig_key = (gpointer) bk->key;
        if (value)
            *value = (gpointer) bk->data;
        return TRUE;
    }
    return FALSE;
}

#define g_hash_table_insert(hash_table, key, value) \
    HashInsert(hash_table, PTR_KEY(hash_table, key), (ulong) value)

#define g_hash_table_destroy(hash_table) \
    FreeHashTable(hash_table)

#define g_hash_table_remove(hash_table, key) \
    HashDelete(hash_table, PTR_KEY(hash_table, key));
*/
#define g_new0(type, count)     \
    ((type *) g_malloc0((unsigned) sizeof (type) * (count)))
/*
static inline
void g_hash_table_foreach(GHashTable *hash_table,
                          GHFunc      func,
                          gpointer    user_data)
{
    HTItem *itor;
    for (itor = HashFirstBucket(hash_table);
         itor != NULL;
         itor = HashNextBucket(hash_table))
    {
        func((gpointer) itor->key, (gpointer) itor->data, user_data);
    }
}
*/

#define g_return_if_fail(expr)
#define g_return_val_if_fail(expr,val)

static inline
GString* g_string_new(const gchar *init)
{
    GString *str = g_new0(GString, 1);

    if (init) {
        str->len = strlen(init);
        str->str = g_strdup(init);
        str->alloc = str->len + 1;
    }
    else {
        str->len = 0;
        str->str = NULL;
        str->alloc = 0;
    }

    return str;
}

static inline
void g_string_free(GString *str, gboolean free_segment)
{
    if (free_segment && str->alloc > 0)
        g_free(str->str);
    g_free(str);
}

static inline
void g_string_printf(GString *str, const gchar *format, ...)
{
    va_list args;

    if (str->alloc < 2048) {
        str->str = (gchar *) realloc(str->str, 2048);
        str->alloc = 2048;
    }

    va_start(args, format);
    str->len = vsnprintf(str->str, str->alloc, format, args);
    va_end(args);
}

static inline
GString* g_string_append (GString *str, const gchar *val)
{
    int len;

    len = strlen (val);
    str->str = realloc (str->str, str->len + len + 1);
    strcpy (str->str + str->len, val);

    str->len += len;
    return str;
}

static inline
GSList* g_slist_alloc(void)
{
    return g_new0(GSList, 1);
}

static inline
void g_slist_free(GSList *slist)
{
    g_free(slist);
}

static inline
GSList* g_slist_last(GSList *slist)
{
    if (slist) {
        while (slist->next)
                slist = slist->next;
    }
    return slist;
}

static inline
GSList *g_slist_append(GSList *slist, gpointer data)
{
    GSList *new_slist;
    GSList *last;

    new_slist = g_slist_alloc();
    new_slist->data = data;

    if (slist) {
        last = g_slist_last(slist);
        last->next = new_slist;
        return slist;
    }

    return new_slist;
}

static inline
GSList *g_slist_prepend(GSList *slist, gpointer data)
{
    GSList *new_list;

    new_list = g_slist_alloc();
    new_list->data = data;
    new_list->next = slist;

    return new_list;
}

/* GLib tree functions */
GTree* g_tree_new(GCompareFunc key_compare_func);
void g_tree_destroy(GTree *tree);
void g_tree_insert(GTree *tree, gpointer key, gpointer value);
void g_tree_remove(GTree *tree, gpointer key);
gpointer g_tree_lookup(GTree *tree, gpointer key);


/* GLib utils */
gboolean g_path_is_absolute(const gchar *file_name);

/* GLib string functions */
gchar* g_strconcat (const gchar *string1, ...);
gchar* g_strdup_vprintf (const gchar *format, va_list args1);
gchar* g_strdup_printf (const gchar *format, ...);

static inline GSList*
g_slist_remove_link (GSList *list,
                     GSList *link)
{
  GSList *tmp;
  GSList *prev;

  prev = NULL;
  tmp = list;

  while (tmp)
    {
      if (tmp == link)
        {
          if (prev)
            prev->next = tmp->next;
          if (list == tmp)
            list = list->next;

          tmp->next = NULL;
          break;
        }

      prev = tmp;
      tmp = tmp->next;
    }

  return list;
}


#endif // __G_LIB_PRIVATE_H__

/* Modeline for vim: set tw=79 et ts=4: */

