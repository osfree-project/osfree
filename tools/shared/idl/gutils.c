/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1998  Peter Mattis, Spencer Kimball and Josh MacDonald
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
 * removed as lightIDL does not need to be thread-safe.
 */

#include "glib.h"

gboolean
g_path_is_absolute(const gchar *file_name)
{
    if (file_name[0] == G_DIR_SEPARATOR)
        return TRUE;

#ifdef _WIN32
    if (isalpha(file_name[0]) && file_name[1] == ':' && file_name[2] == G_DIR_SEPARATOR)
        return TRUE;
#endif

    return FALSE;
}
