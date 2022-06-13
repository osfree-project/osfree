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
/* code generater for fma.idl */
#define SOM_Module_fma_Source
#define SOMPFileMediaAbstract_Class_Source
#include <rhbsomp.h>
#include <fma.ih>
/* overridden methods for ::SOMPFileMediaAbstract */
/* overridden method ::SOMPMediaInterfaceAbstract::sompOpen */
SOM_Scope void SOMLINK fma_sompOpen(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev)
{
	SOMPFileMediaAbstract_parent_SOMPMediaInterfaceAbstract_sompOpen(somSelf,ev);
}
/* overridden method ::SOMPMediaInterfaceAbstract::sompClose */
SOM_Scope void SOMLINK fma_sompClose(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev)
{
	SOMPFileMediaAbstract_parent_SOMPMediaInterfaceAbstract_sompClose(somSelf,ev);
}
/* introduced methods for ::SOMPFileMediaAbstract */
/* introduced method ::SOMPFileMediaAbstract::sompInitReadWrite */
SOM_Scope void SOMLINK fma_sompInitReadWrite(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring mediaInfo)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(mediaInfo)
}
/* introduced method ::SOMPFileMediaAbstract::sompInitReadOnly */
SOM_Scope void SOMLINK fma_sompInitReadOnly(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring mediaInfo)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(mediaInfo)
}
/* introduced method ::SOMPFileMediaAbstract::sompSeekPosition */
SOM_Scope void SOMLINK fma_sompSeekPosition(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ long offset)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(offset)
}
/* introduced method ::SOMPFileMediaAbstract::sompSeekPositionRel */
SOM_Scope void SOMLINK fma_sompSeekPositionRel(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ long offset)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(offset)
}
/* introduced method ::SOMPFileMediaAbstract::sompGetOffset */
SOM_Scope long SOMLINK fma_sompGetOffset(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev)
{
	long __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPFileMediaAbstract::sompReadBytes */
SOM_Scope void SOMLINK fma_sompReadBytes(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring byteStream,
	/* in */ long length)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(byteStream)
	RHBOPT_unused(length)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteBytes */
SOM_Scope void SOMLINK fma_sompWriteBytes(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring byteStream,
	/* in */ long length)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(byteStream)
	RHBOPT_unused(length)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteOctet */
SOM_Scope void SOMLINK fma_sompWriteOctet(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ octet i1)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i1)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteShort */
SOM_Scope void SOMLINK fma_sompWriteShort(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ short i2)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i2)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteUnsignedShort */
SOM_Scope void SOMLINK fma_sompWriteUnsignedShort(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short u2)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(u2)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteLong */
SOM_Scope void SOMLINK fma_sompWriteLong(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ long i4)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i4)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteUnsignedLong */
SOM_Scope void SOMLINK fma_sompWriteUnsignedLong(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long u4)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(u4)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteDouble */
SOM_Scope void SOMLINK fma_sompWriteDouble(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ double f8)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(f8)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteFloat */
SOM_Scope void SOMLINK fma_sompWriteFloat(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ float f4)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(f4)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteCharacter */
SOM_Scope void SOMLINK fma_sompWriteCharacter(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ char c)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(c)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteSomobject */
SOM_Scope void SOMLINK fma_sompWriteSomobject(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR so,
	/* in */ SOMObject SOMSTAR parentObject)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(so)
	RHBOPT_unused(parentObject)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteString */
SOM_Scope void SOMLINK fma_sompWriteString(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring wstring)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(wstring)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadOctet */
SOM_Scope void SOMLINK fma_sompReadOctet(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ octet *i1)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i1)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadShort */
SOM_Scope void SOMLINK fma_sompReadShort(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ short *i2)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i2)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadUnsignedShort */
SOM_Scope void SOMLINK fma_sompReadUnsignedShort(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ unsigned short *u2)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(u2)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadLong */
SOM_Scope void SOMLINK fma_sompReadLong(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ long *i4)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i4)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadUnsignedLong */
SOM_Scope void SOMLINK fma_sompReadUnsignedLong(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ unsigned long *i4)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i4)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadDouble */
SOM_Scope void SOMLINK fma_sompReadDouble(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ double *f8)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(f8)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadFloat */
SOM_Scope void SOMLINK fma_sompReadFloat(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ float *f4)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(f4)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadCharacter */
SOM_Scope void SOMLINK fma_sompReadCharacter(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ char *c)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(c)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadSomobject */
SOM_Scope void SOMLINK fma_sompReadSomobject(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ SOMObject SOMSTAR *so)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(so)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadString */
SOM_Scope void SOMLINK fma_sompReadString(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ corbastring *rstring)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(rstring)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadStringToBuffer */
SOM_Scope void SOMLINK fma_sompReadStringToBuffer(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring buffer,
	/* in */ long bufsize)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(buffer)
	RHBOPT_unused(bufsize)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteLine */
SOM_Scope void SOMLINK fma_sompWriteLine(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring buffer)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(buffer)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadLine */
SOM_Scope void SOMLINK fma_sompReadLine(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring buffer,
	/* in */ long bufsize)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(buffer)
	RHBOPT_unused(bufsize)
}
/* introduced method ::SOMPFileMediaAbstract::sompWriteTypeCode */
SOM_Scope void SOMLINK fma_sompWriteTypeCode(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ TypeCode tc)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(tc)
}
/* introduced method ::SOMPFileMediaAbstract::sompReadTypeCode */
SOM_Scope void SOMLINK fma_sompReadTypeCode(
	SOMPFileMediaAbstract SOMSTAR somSelf,
	Environment *ev,
	/* inout */ TypeCode *tc)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(tc)
}
