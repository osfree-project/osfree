/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * g_atomic_*: atomic operations.
 * Copyright (C) 2003 Sebastian Wilhelmi
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

#ifndef __G_ATOMIC_H__
#define __G_ATOMIC_H__

//#include <glib/gtypes.h>

//G_BEGIN_DECLS

int     g_atomic_int_exchange_and_add         (volatile int     *atomic,
                                                int               val);
void     g_atomic_int_add                      (volatile int     *atomic,
                                                int               val);
int g_atomic_int_compare_and_exchange     (volatile int     *atomic,
                                                int               oldval,
                                                int               newval);
int g_atomic_pointer_compare_and_exchange (volatile void * *atomic,
                                                void *           oldval,
                                                void *           newval);

int     g_atomic_int_get                      (volatile int     *atomic);
void     g_atomic_int_set                      (volatile int     *atomic,
                                                int               newval);
void * g_atomic_pointer_get                  (volatile void * *atomic);
void     g_atomic_pointer_set                  (volatile void * *atomic,
                                                void *           newval);

#ifndef G_ATOMIC_OP_MEMORY_BARRIER_NEEDED
# define g_atomic_int_get(atomic)               (*(atomic))
# define g_atomic_int_set(atomic, newval)       ((void) (*(atomic) = (newval)))
# define g_atomic_pointer_get(atomic)           (*(atomic))
# define g_atomic_pointer_set(atomic, newval)   ((void) (*(atomic) = (newval)))
#endif /* G_ATOMIC_OP_MEMORY_BARRIER_NEEDED */

#define g_atomic_int_inc(atomic) (g_atomic_int_add ((atomic), 1))
#define g_atomic_int_dec_and_test(atomic)                               \
  (g_atomic_int_exchange_and_add ((atomic), -1) == 1)

//G_END_DECLS

#endif /* __G_ATOMIC_H__ */
