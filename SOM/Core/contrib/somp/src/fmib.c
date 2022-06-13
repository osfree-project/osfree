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

#include <rhbopt.h>
/* code generater for fmib.idl */
#define SOM_Module_fmib_Source
#define SOMPBinaryFileMedia_Class_Source
#include <rhbsomp.h>
#include <fmib.ih>
/* overridden methods for ::SOMPBinaryFileMedia */
/* overridden method ::SOMPFileMediaAbstract::sompWriteOctet */
SOM_Scope void SOMLINK fmib_sompWriteOctet(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* in */ octet i1)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompWriteOctet(somSelf,ev,i1);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteShort */
SOM_Scope void SOMLINK fmib_sompWriteShort(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* in */ short i2)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompWriteShort(somSelf,ev,i2);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteUnsignedShort */
SOM_Scope void SOMLINK fmib_sompWriteUnsignedShort(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short u2)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompWriteUnsignedShort(somSelf,ev,u2);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteLong */
SOM_Scope void SOMLINK fmib_sompWriteLong(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* in */ long i4)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompWriteLong(somSelf,ev,i4);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteUnsignedLong */
SOM_Scope void SOMLINK fmib_sompWriteUnsignedLong(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long u4)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompWriteUnsignedLong(somSelf,ev,u4);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteDouble */
SOM_Scope void SOMLINK fmib_sompWriteDouble(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* in */ double f8)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompWriteDouble(somSelf,ev,f8);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteFloat */
SOM_Scope void SOMLINK fmib_sompWriteFloat(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* in */ float f4)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompWriteFloat(somSelf,ev,f4);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteString */
SOM_Scope void SOMLINK fmib_sompWriteString(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring wstring)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompWriteString(somSelf,ev,wstring);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadOctet */
SOM_Scope void SOMLINK fmib_sompReadOctet(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* inout */ octet *i1)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompReadOctet(somSelf,ev,i1);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadShort */
SOM_Scope void SOMLINK fmib_sompReadShort(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* inout */ short *i2)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompReadShort(somSelf,ev,i2);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadUnsignedShort */
SOM_Scope void SOMLINK fmib_sompReadUnsignedShort(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* inout */ unsigned short *u2)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompReadUnsignedShort(somSelf,ev,u2);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadLong */
SOM_Scope void SOMLINK fmib_sompReadLong(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* inout */ long *i4)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompReadLong(somSelf,ev,i4);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadUnsignedLong */
SOM_Scope void SOMLINK fmib_sompReadUnsignedLong(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* inout */ unsigned long *i4)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompReadUnsignedLong(somSelf,ev,i4);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadDouble */
SOM_Scope void SOMLINK fmib_sompReadDouble(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* inout */ double *f8)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompReadDouble(somSelf,ev,f8);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadFloat */
SOM_Scope void SOMLINK fmib_sompReadFloat(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* inout */ float *f4)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompReadFloat(somSelf,ev,f4);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadString */
SOM_Scope void SOMLINK fmib_sompReadString(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* inout */ corbastring *rstring)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompReadString(somSelf,ev,rstring);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadStringToBuffer */
SOM_Scope void SOMLINK fmib_sompReadStringToBuffer(
	SOMPBinaryFileMedia SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring buffer,
	/* in */ long bufsize)
{
	SOMPBinaryFileMedia_parent_SOMPAsciiMediaInterface_sompReadStringToBuffer(somSelf,ev,buffer,bufsize);
}
/* introduced methods for ::SOMPBinaryFileMedia */
