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
/* code generater for fmi.idl */
#define SOM_Module_fmi_Source
#define SOMPAsciiMediaInterface_Class_Source
#include <rhbsomp.h>
#include <fmi.ih>
/* overridden methods for ::SOMPAsciiMediaInterface */
/* overridden method ::SOMPFileMediaAbstract::sompInitReadWrite */
SOM_Scope void SOMLINK fmi_sompInitReadWrite(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring mediaInfo)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompInitReadWrite(somSelf,ev,mediaInfo);
}
/* overridden method ::SOMPFileMediaAbstract::sompInitReadOnly */
SOM_Scope void SOMLINK fmi_sompInitReadOnly(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring mediaInfo)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompInitReadOnly(somSelf,ev,mediaInfo);
}
/* overridden method ::SOMPFileMediaAbstract::sompSeekPosition */
SOM_Scope void SOMLINK fmi_sompSeekPosition(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ long offset)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompSeekPosition(somSelf,ev,offset);
}
/* overridden method ::SOMPFileMediaAbstract::sompSeekPositionRel */
SOM_Scope void SOMLINK fmi_sompSeekPositionRel(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ long offset)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompSeekPositionRel(somSelf,ev,offset);
}
/* overridden method ::SOMPFileMediaAbstract::sompGetOffset */
SOM_Scope long SOMLINK fmi_sompGetOffset(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev)
{
	long __result;
	__result=SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompGetOffset(somSelf,ev);
	return __result;}
/* overridden method ::SOMPFileMediaAbstract::sompReadBytes */
SOM_Scope void SOMLINK fmi_sompReadBytes(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring byteStream,
	/* in */ long length)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadBytes(somSelf,ev,byteStream,length);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteBytes */
SOM_Scope void SOMLINK fmi_sompWriteBytes(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring byteStream,
	/* in */ long length)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteBytes(somSelf,ev,byteStream,length);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteOctet */
SOM_Scope void SOMLINK fmi_sompWriteOctet(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ octet i1)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteOctet(somSelf,ev,i1);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteShort */
SOM_Scope void SOMLINK fmi_sompWriteShort(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ short i2)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteShort(somSelf,ev,i2);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteUnsignedShort */
SOM_Scope void SOMLINK fmi_sompWriteUnsignedShort(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short u2)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteUnsignedShort(somSelf,ev,u2);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteLong */
SOM_Scope void SOMLINK fmi_sompWriteLong(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ long i4)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteLong(somSelf,ev,i4);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteUnsignedLong */
SOM_Scope void SOMLINK fmi_sompWriteUnsignedLong(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long u4)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteUnsignedLong(somSelf,ev,u4);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteDouble */
SOM_Scope void SOMLINK fmi_sompWriteDouble(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ double f8)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteDouble(somSelf,ev,f8);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteFloat */
SOM_Scope void SOMLINK fmi_sompWriteFloat(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ float f4)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteFloat(somSelf,ev,f4);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteCharacter */
SOM_Scope void SOMLINK fmi_sompWriteCharacter(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ char c)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteCharacter(somSelf,ev,c);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteSomobject */
SOM_Scope void SOMLINK fmi_sompWriteSomobject(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR so,
	/* in */ SOMObject SOMSTAR parentObject)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteSomobject(somSelf,ev,so,parentObject);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteString */
SOM_Scope void SOMLINK fmi_sompWriteString(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring wstring)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteString(somSelf,ev,wstring);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadOctet */
SOM_Scope void SOMLINK fmi_sompReadOctet(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ octet *i1)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadOctet(somSelf,ev,i1);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadShort */
SOM_Scope void SOMLINK fmi_sompReadShort(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ short *i2)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadShort(somSelf,ev,i2);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadUnsignedShort */
SOM_Scope void SOMLINK fmi_sompReadUnsignedShort(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ unsigned short *u2)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadUnsignedShort(somSelf,ev,u2);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadLong */
SOM_Scope void SOMLINK fmi_sompReadLong(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ long *i4)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadLong(somSelf,ev,i4);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadUnsignedLong */
SOM_Scope void SOMLINK fmi_sompReadUnsignedLong(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ unsigned long *i4)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadUnsignedLong(somSelf,ev,i4);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadDouble */
SOM_Scope void SOMLINK fmi_sompReadDouble(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ double *f8)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadDouble(somSelf,ev,f8);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadFloat */
SOM_Scope void SOMLINK fmi_sompReadFloat(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ float *f4)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadFloat(somSelf,ev,f4);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadCharacter */
SOM_Scope void SOMLINK fmi_sompReadCharacter(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ char *c)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadCharacter(somSelf,ev,c);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadSomobject */
SOM_Scope void SOMLINK fmi_sompReadSomobject(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ SOMObject SOMSTAR *so)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadSomobject(somSelf,ev,so);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadString */
SOM_Scope void SOMLINK fmi_sompReadString(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ corbastring *rstring)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadString(somSelf,ev,rstring);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadStringToBuffer */
SOM_Scope void SOMLINK fmi_sompReadStringToBuffer(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring buffer,
	/* in */ long bufsize)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadStringToBuffer(somSelf,ev,buffer,bufsize);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteLine */
SOM_Scope void SOMLINK fmi_sompWriteLine(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring buffer)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteLine(somSelf,ev,buffer);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadLine */
SOM_Scope void SOMLINK fmi_sompReadLine(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring buffer,
	/* in */ long bufsize)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadLine(somSelf,ev,buffer,bufsize);
}
/* overridden method ::SOMPFileMediaAbstract::sompWriteTypeCode */
SOM_Scope void SOMLINK fmi_sompWriteTypeCode(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ TypeCode tc)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompWriteTypeCode(somSelf,ev,tc);
}
/* overridden method ::SOMPFileMediaAbstract::sompReadTypeCode */
SOM_Scope void SOMLINK fmi_sompReadTypeCode(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ TypeCode *tc)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompReadTypeCode(somSelf,ev,tc);
}
/* overridden method ::SOMPMediaInterfaceAbstract::sompOpen */
SOM_Scope void SOMLINK fmi_sompOpen(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompOpen(somSelf,ev);
}
/* overridden method ::SOMPMediaInterfaceAbstract::sompClose */
SOM_Scope void SOMLINK fmi_sompClose(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev)
{
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_sompClose(somSelf,ev);
}
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK fmi_somInit(
	SOMPAsciiMediaInterface SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_somInit(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK fmi_somUninit(
	SOMPAsciiMediaInterface SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPAsciiMediaInterface_parent_SOMPFileMediaAbstract_somUninit(somSelf);
}
/* introduced methods for ::SOMPAsciiMediaInterface */
/* introduced method ::SOMPAsciiMediaInterface::sompInitSpecific */
SOM_Scope void SOMLINK fmi_sompInitSpecific(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ SOMPAsciiMediaInterface_mediaInfoType *mediaInfo)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(mediaInfo)
}
/* introduced method ::SOMPAsciiMediaInterface::sompStat */
SOM_Scope void SOMLINK fmi_sompStat(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* inout */ SOMPStatDef_sompstat *fileStats)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(fileStats)
}
/* introduced method ::SOMPAsciiMediaInterface::sompQueryBlockSize */
SOM_Scope unsigned long SOMLINK fmi_sompQueryBlockSize(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPAsciiMediaInterface::sompGetMediaName */
SOM_Scope corbastring SOMLINK fmi_sompGetMediaName(
	SOMPAsciiMediaInterface SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring toBuffer)
{
	corbastring __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(toBuffer)
	return __result;
}
