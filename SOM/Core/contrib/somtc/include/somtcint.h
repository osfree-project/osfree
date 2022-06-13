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

#if (!defined(BUILD_SOMTC)) && (!defined(BUILD_SOMIR)) && (!defined(BUILD_SOMD)) && (!defined(BUILD_IBMSOMIR))
#	error should be internal to somtc/somir/somd
#endif

#ifndef SOMTC_VERSION

#define SOMTC_VERSION_1			0x01
#define SOMTC_FLAGS_MALLOC      0x01
#define SOMTC_FLAGS_ZENUM	    0x10
#define SOMTC_VERSION			SOMTC_VERSION_1

#if 0 && !defined(tk_exception)
	#define tk_exception    tk_struct
#endif

struct TypeCode_base
{
	TCKind tag;
	short align;
	unsigned char version;
	unsigned char flags;
};

struct TypeCode_struct_member
{
	char *mbrName;
	TypeCode mbrTc;
};

struct TypeCode_struct
{
	TCKind tag;
	short align;
	unsigned char version;
	unsigned char flags;
	long rptCount;
	char *structName;
	struct TypeCode_struct_member *mbrs;
};

struct TypeCode_exception
{
	TCKind tag;
	short align;
	unsigned char version;
	unsigned char flags;
	long rptCount;
	char *structName;
	struct TypeCode_struct_member *mbrs;
};

struct TypeCode_enum
{
	TCKind tag;
	short align;
	unsigned char version;
	unsigned char flags;
    long rptCount; 
	char *enumName;
	char **enumId;
};

struct TypeCode_sequence
{
	TCKind tag;
	short align;
	unsigned char version;
	unsigned char flags;
	TypeCode tc;
	long maxBound;
};

struct TypeCode_array
{
	TCKind tag;
	short align;
	unsigned char version;
	unsigned char flags;
	TypeCode tc;
	long maxBound;
};

struct TypeCode_objref
{
	TCKind tag;
	short align;
	unsigned char version;
	unsigned char flags;
	char *interfaceId;
};

struct TypeCode_union_member
{
	long labelFlag;
	union {
		unsigned short s;
		unsigned long l;
		char c;
	} labelValue;
	char *mbrName;
	TypeCode mbrTc;
};

struct TypeCode_union
{
	TCKind tag;
	short align;
	unsigned char version;
	unsigned char flags;
	long rptCount;
	char *unionName;
	TypeCode swTc;
	struct TypeCode_union_member *mbrs;
};

struct TypeCode_string
{
   TCKind tag; 
   short align; 
   unsigned char version; 
   unsigned char flags; 
   long maxBound; 
};

struct TypeCode_pointer
{
   TCKind tag;
   short align;
   unsigned char version;
   unsigned char flags;
   TypeCode tc;
};

struct TypeCode_foreign
{
   TCKind tag;
   short align;
   unsigned char version;
   unsigned char flags;
   char *typeName;
   char *context;
   long length;
};

struct TypeCode_self
{
   TCKind tag;
   short align;
   unsigned char version;
   unsigned char flags;
   char *selfName;
};

#if defined(_WIN32) && defined(_DEBUG)
union SOM_TYPECODE
{
	struct TypeCode_base any;
	struct TypeCode_objref objref;
	struct TypeCode_union union_t;
	struct TypeCode_struct struct_t;
	struct TypeCode_enum enum_t;
	struct TypeCode_self self;
	struct TypeCode_foreign foreign;
	struct TypeCode_pointer pointer_t;
	struct TypeCode_sequence sequence_t;
	struct TypeCode_array array_t;
	struct TypeCode_string string_t;
};
#	define TypeCode_peek(x)     (&((x)->any))
#else
#	define TypeCode_peek(x)     ((struct TypeCode_base *)x)
#endif


#endif /* SOMTC_VERSION */
