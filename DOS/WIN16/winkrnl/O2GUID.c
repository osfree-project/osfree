/*    
	O2GUID.c	1.5
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
#include "windowsx.h"

#define	INITGUID
#include "compobj.h"

DEFINE_OLEGUID(IID_IDispatch,		0x00020400L, 0, 0); 
DEFINE_OLEGUID(IID_IEnumVARIANT,	0x00020404L, 0, 0); 
DEFINE_OLEGUID(IID_ITypeInfo,		0x00020401L, 0, 0); 
DEFINE_OLEGUID(IID_ITypeLib,		0x00020402L, 0, 0); 
DEFINE_OLEGUID(IID_ITypeComp,		0x00020403L, 0, 0); 
DEFINE_OLEGUID(IID_ICreateTypeInfo,	0x00020405L, 0, 0); 
DEFINE_OLEGUID(IID_ICreateTypeLib,	0x00020406L, 0, 0); 
DEFINE_OLEGUID(IID_StdOle,		0x00020430L, 0, 0); 

#undef	INITGUID
