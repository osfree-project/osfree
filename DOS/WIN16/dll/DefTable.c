/*    
	DefTable.c	1.2	1/29/96 Default Module Description Table	
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

 */

#include <stdio.h>

#include "windows.h"
#include "kerndef.h"
#include "Resources.h"
#include "Module.h"
#include "Kernel.h"

extern long LibMain();
extern TYPEINFO *hsmt_resource_table;

static ENTRYTAB entry_tab_default[] =
{	/* 000 */	{ 0, 0, 0, LibMain },
	/* end */	{ 0, 0, 0, 0 }
};


 /* Module Descriptor for appl: */

static MODULEDSCR mod_dscr_default =
{	"Default",
	entry_tab_default,
	0,
	(long*)&hsmt_resource_table
};

MODULETAB ModuleDescriptorTable = {
	MODULE_SYSTEM, &mod_dscr_default
};

