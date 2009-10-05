/*    
	ValidateModule.c	1.6
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
#include <string.h>
#include <ctype.h>

#include "windows.h"
#include "Validate.h"

extern BYTE bit_reverse[];

BOOL
ValidateModule(LPCSTR lpModuleName)
{
    BYTE buf[_MAX_PATH];
    LPBYTE ptr;
    int len,i;
	BOOL	Restricted  = TRUE;
	BOOL	ReturnValue = TRUE;

#ifndef RESTRICTED
	Restricted = FALSE;
#endif

	if ( Restricted )
	{
		ReturnValue = FALSE;
		len = strlen(lpModuleName);
		for (i=0; i<len; i++)
			buf[i] = bit_reverse[(BYTE)(toupper(lpModuleName[len-1-i]))];
		buf[i] = 0;

		for(ptr = &AllowedModules[0]; *ptr; ptr += strlen((LPSTR)ptr)+1)
			if (!memcmp(ptr,buf,len))
				ReturnValue = TRUE;
	}

    return ReturnValue;
}

