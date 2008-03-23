/*    
	TwinDeflib.c	1.6
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
#include "windows.h"
#include "Resources.h"
#include "Module.h"

/*
 *	Dummy table for library entry points that must be called
 *	before calling WinMain.  This is only for systems that do
 *	not allow a mechanism for specifying run-time initialization
 *	code.  This table will be used if the application is not
 *	linked with an application-supplied table.  If an application
 *	links against libraries, however, and needs to have us call
 *	the init code directly, then another table will be supplied,
 *	and this object module will not be loaded.
 */


ENTRYTAB TWIN_LibInitTable[] = {
    {0, 0, 0, 0}
};


