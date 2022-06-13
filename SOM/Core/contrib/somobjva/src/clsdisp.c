/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/*
 * $Id$
 */

#define SOMObject_VA_EXTERN
#define SOM_Module_somobj_Source
#define SOMDLLEXPORT

#include <rhbopt.h>
#include <som.h>

boolean SOMLINK somva_SOMObject_somClassDispatch(SOMObject SOMSTAR somSelf,
		SOMClass SOMSTAR clsObj,
		somToken *retValue,
		somId methodId,
		...)
{
	boolean __result;
	va_list ap;
	va_start(ap,methodId);
	__result=SOMObject_somClassDispatch(somSelf,clsObj,retValue,methodId,ap);
	va_end(ap);
	return __result;
}
