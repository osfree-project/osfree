/*    
	rc.c	2.10
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


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*	platform-independent entry points to resource compiler

 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "windows.h"
#include "Resources.h"
#include "rc.h"
#include "define.h"
#include "restypes.h"

char in_name[512];
char out_name[512];
char export_name[512];
char cpp_module_name[512];
char libentry_name[256];
char tbl_name[256];
extern char include_path[];

unsigned alloc_size = 0;

void print_resources(void);

void icompile(void)
{
  init_resource_table(); /* Initialize (empty) resource table */
  include_init();        /* Initialize include stack */
  open_files();          /* Set up yyin, output file */
  yyparse();             /* Parse resource file */
  print_resources();     /* Print resources into C file */
  cleanup_resources();   /* Free allocated memory */
  clean_defines();
}

void init_resource_table(void)
{
  TYPEINFO *type_ptr;
  int i;

  CALLOC(resource_table, MAX_TYPES, TYPEINFO);
  type_ptr = resource_table;
  /* initialise the resource table */
  for (i = 0; i < MAX_TYPES; i++) {
    type_ptr->rcstypename = (char *)NULL;
    type_ptr->rcscount = 0;
    type_ptr->rcsinfo = (NAMEINFO *)NULL;
    type_ptr++;
  }
}

