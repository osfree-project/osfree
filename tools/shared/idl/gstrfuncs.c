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
 * removed as lightIDL does not need to be thread-safe.
 */

#include "glib.h"


#if defined(va_copy)
#elif defined(__va_copy)
#define va_copy(dst, src) __va_copy((dst), (src))
#else
#define va_copy(dst, src) (memcpy(&(dst), &(src), sizeof (va_list)))
#endif

gchar*
g_strconcat (const gchar *string1, ...)
{
    guint   l;
    va_list args;
    gchar   *s;
    gchar   *concat;

    g_return_val_if_fail(string1 != NULL, NULL);

    l = 1 + strlen (string1);
    va_start(args, string1);
    s = va_arg(args, gchar*);
    while (s) {
        l += strlen (s);
        s = va_arg (args, gchar*);
    }
    va_end (args);

    concat = g_new0(gchar, l);
    concat[0] = 0;

    strcat(concat, string1);
    va_start(args, string1);
    s = va_arg(args, gchar*);
    while (s) {
        strcat (concat, s);
        s = va_arg (args, gchar*);
    }
    va_end (args);

    return concat;
}

guint
g_printf_string_upper_bound (const gchar* format,
                 va_list      args)
{
    guint len = 1;

    while (*format) {
        gboolean long_int = FALSE;
        gboolean extra_long = FALSE;
        gchar c;

        c = *format++;

        if (c == '%') {
            gboolean done = FALSE;

            while (*format && !done) {
				switch (*format++) {
					gchar *string_arg;

					case '*':
						len += va_arg (args, int);
						break;
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						/* add specified format length, since it might exceed the
						 * size we assume it to have.
						 */
						format -= 1;
						len += strtol (format, (char**) &format, 10);
						break;
					case 'h':
						/* ignore short int flag, since all args have at least the
						 * same size as an int
						 */
						break;
					case 'l':
						if (long_int)
							extra_long = TRUE; /* linux specific */
						else
							long_int = TRUE;
						break;
					case 'q':
					case 'L':
						long_int = TRUE;
						extra_long = TRUE;
						break;
					case 's':
						string_arg = va_arg (args, char *);
						if (string_arg)
							len += strlen (string_arg);
						else {
							/* add enough padding to hold "(null)" identifier */
							len += 16;
						}
						done = TRUE;
						break;
					case 'd':
					case 'i':
					case 'o':
					case 'u':
					case 'x':
					case 'X':
#ifdef  G_HAVE_GINT64
						if (extra_long)
							(void) va_arg (args, gint64);
						else
#endif  /* G_HAVE_GINT64 */
						{
							if (long_int)
								(void) va_arg (args, long);
							else
								(void) va_arg (args, int);
						}
						len += extra_long ? 64 : 32;
						done = TRUE;
						break;
					case 'D':
					case 'O':
					case 'U':
						(void) va_arg (args, long);
						len += 32;
						done = TRUE;
						break;
					case 'e':
					case 'E':
					case 'f':
					case 'g':
#ifdef HAVE_LONG_DOUBLE
						if (extra_long)
							(void) va_arg (args, long double);
						else
#endif  /* HAVE_LONG_DOUBLE */
							(void) va_arg (args, double);
						len += extra_long ? 128 : 64;
						done = TRUE;
						break;
					case 'c':
						(void) va_arg (args, int);
						len += 1;
						done = TRUE;
						break;
					case 'p':
					case 'n':
						(void) va_arg (args, void*);
						len += 32;
						done = TRUE;
						break;
					case '%':
						len += 1;
						done = TRUE;
						break;
					default:
						/* ignore unknow/invalid flags */
						break;
				}
			}
		}
		else
			len += 1;
	}

	return len;
}

gchar*
g_strdup_vprintf (const gchar *format, va_list args1)
{
    gchar *buffer;
	va_list args2;

	va_copy(args2, args1);
	buffer = malloc (sizeof(gchar) * g_printf_string_upper_bound (format, args1));
    vsprintf (buffer, format, args1);
	va_end(args2);

    return buffer;
}

gchar*
g_strdup_printf (const gchar *format, ...)
{
    gchar *buffer;
    va_list args;

    va_start (args, format);
    buffer = g_strdup_vprintf (format, args);
    va_end (args);

    return buffer;
}

