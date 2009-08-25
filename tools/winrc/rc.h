/*************************************************************************

	@(#)rc.h	2.13

	generic include file for resource compiler

  
    Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

**************************************************************************/

/* When I last checked, this was rc 2.0.1.  It might change on us, though... */
#ifndef RC_VERSION
#define RC_VERSION "2.0.1"
#endif

/* Some systems don't define NULL where they should.  Let's do it here... */
#ifndef NULL
#define NULL ((void *)0)
#endif

extern char tbl_name[];

int yyparse(void);
int zzparse(void);
char *current_file(void);
int current_line(void);
void icompile(void);
void include_init(void);
void init_resource_table(void);
void print_resources(void);
void cleanup_resources(void);

void *get_ext(void);
void *set_ext(void *);

int istrcmp(const char *str1, const char *str2);

int alignval (char *str);

/* Callbacks into system-dependent file (cmdline.c, mac.c, ...) */
char *load_file(char *filename, int lookHere);
void open_files(void);
void update_info(void);
void at_start_of_macro(void);
void at_start_of_file(void);
void at_end_of_file(void);

/* System-provided memory-management functions */
void *get_mem(unsigned size);
void free_mem(void *p);

/* Memory allocation macros */
#define NALLOC(n, s, t) (n) = (t)get_mem(s)
#define CALLOC(n, c, t) NALLOC((n), (c) * sizeof(t), t *)
#define ALLOC(n, t) NALLOC((n), sizeof(t), t *)

/* Macros for printing various types of things */
#define LOG(x) dolog x
#define VPRINT(x) dovprint x
#define CERROR(x) doerror x
#define FATAL(x) dofatal x
#define WARNING(x) dowarning x
#define COUTPUT(x) dooutput x
#define EOUTPUT(x) doeoutput x
#define CPPOUTPUT(x) docppoutput x

void dolog(char *, ...);
void dovprint(char *, ...);
void doerror(char *, ...);
void dofatal(char *, ...);
void dowarning(char *, ...);
void dooutput(char *, ...);
void doeoutput(char *, ...);
void docppoutput(char *, ...);

