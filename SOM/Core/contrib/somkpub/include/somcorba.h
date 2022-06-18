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

 /************************************************
 * corba definitions
 */

#ifndef somcorba_h
#define somcorba_h

#include <string.h>

typedef SOMObject CORBAObject;

#ifndef SOM_BOOLEAN
#	define SOM_BOOLEAN
	typedef unsigned char boolean;
#endif /* SOM_BOOLEAN */

#ifndef TRUE
#	define TRUE   1
#endif

#ifndef FALSE
#	define FALSE   0
#endif

typedef unsigned char octet;
typedef char *string;
typedef char *Identifier;	/* originally in somcorba.h */

typedef enum exception_type 
{
	NO_EXCEPTION,
	USER_EXCEPTION,
	SYSTEM_EXCEPTION,
	exception_type_MAX=2147483647 
} exception_type;

/*********************************************
 * this is at odds with SOM IDL enumeration which starts at one 
 * also cause issues on 64-bit machines 
 */

typedef enum _completion_status
{
	YES,
	NO,
	MAYBE,
	completion_status_MAX=2147483647
} _completion_status;

/***********************************************
 * hence the slight of hand... 
 */

typedef unsigned long completion_status;

typedef struct StExcep 
{
	unsigned long minor;
	completion_status completed;
} StExcep;

typedef struct Environment 
{
	exception_type _major;
	struct 
	{
		char *_exception_name;
		void *_params;
	} exception;
	void *_somdAnchor;
} Environment;

#define SOM_InitEnvironment(ev)  \
		(ev)->_major=NO_EXCEPTION; \
		(ev)->exception._exception_name=0; \
		(ev)->exception._params=0; \
		(ev)->_somdAnchor=0;

#define SOM_UninitEnvironment(ev) \
		if (NO_EXCEPTION!=(ev)->_major) somExceptionFree(ev);

/* this Macro only really works because NO_EXCEPTION==0 */
#define SOM_CreateLocalEnvironment()  \
		(Environment *)SOMCalloc(1,sizeof(Environment))

#define SOM_DestroyLocalEnvironment(ev) \
		{ somExceptionFree(ev) ; SOMFree(ev); }

#ifndef SOM_TYPECODE
	#define SOM_TYPECODE
	typedef void *TypeCode;
#endif /* SOM_TYPECODE */

#define sequence(type) _IDL_SEQUENCE_##type
#define SOM_SEQUENCE_NAME(name,type)   struct name { unsigned long _maximum; unsigned long _length; type     *    _buffer;  }
#define SOM_SEQUENCE(type)   struct { unsigned long _maximum; unsigned long _length; type     *    _buffer; }
#define SOM_SEQUENCE_TYPEDEF(type)            typedef SOM_SEQUENCE(type) sequence(type)
#define SOM_SEQUENCE_TYPEDEF_NAME(type, name) typedef SOM_SEQUENCE(type) name

#ifndef _IDL_SEQUENCE_void_defined
	#define _IDL_SEQUENCE_void_defined
		SOM_SEQUENCE_TYPEDEF(void);
#endif

typedef sequence(void)  GENERIC_SEQUENCE;

#define sequenceLength(s)   (s)._length
#define sequenceMaximum(s)   (s)._maximum
#define sequenceElement(s,i)   (s)._buffer[i]

/* IDL generated enums have UL trailing the number
	in theory we should do the same here
	also, TCKind should be the same type as a generated enum
  */

typedef unsigned long TCKind;

#define TypeCode_tk_null      1
#define TypeCode_tk_void      2
#define TypeCode_tk_short     3
#define TypeCode_tk_long      4
#define TypeCode_tk_ushort    5 
#define TypeCode_tk_ulong     6
#define TypeCode_tk_float     7 
#define TypeCode_tk_double    8 
#define TypeCode_tk_boolean   9 
#define TypeCode_tk_char      10
#define TypeCode_tk_octet     11 
#define TypeCode_tk_any       12 
#define TypeCode_tk_TypeCode  13 
#define TypeCode_tk_Principal 14 
#define TypeCode_tk_objref    15
#define TypeCode_tk_struct    16 
#define TypeCode_tk_union     17 
#define TypeCode_tk_enum      18 
#define TypeCode_tk_string    19
#define TypeCode_tk_sequence  20 
#define TypeCode_tk_array     21
#define TypeCode_tk_alias	  22
#define TypeCode_tk_except	  23
#define TypeCode_tk_longlong  24
#define TypeCode_tk_ulonglong 25
#define TypeCode_tk_longdouble 26
#define TypeCode_tk_wchar	  27
#define TypeCode_tk_wstring   28
#define TypeCode_tk_fixed     29

#define TypeCode_tk_pointer   101
#define TypeCode_tk_self      102
#define TypeCode_tk_foreign   103

#define tk_null       TypeCode_tk_null 
#define tk_void       TypeCode_tk_void
#define tk_short      TypeCode_tk_short 
#define tk_long       TypeCode_tk_long 
#define tk_ushort     TypeCode_tk_ushort 
#define tk_ulong      TypeCode_tk_ulong
#define tk_float      TypeCode_tk_float 
#define tk_double     TypeCode_tk_double 
#define tk_boolean    TypeCode_tk_boolean 
#define tk_char       TypeCode_tk_char
#define tk_octet      TypeCode_tk_octet 
#define tk_any        TypeCode_tk_any 
#define tk_TypeCode   TypeCode_tk_TypeCode 
#define tk_Principal  TypeCode_tk_Principal 
#define tk_objref     TypeCode_tk_objref
#define tk_struct     TypeCode_tk_struct 
#define tk_union      TypeCode_tk_union 
#define tk_enum       TypeCode_tk_enum 
#define tk_string     TypeCode_tk_string
#define tk_sequence   TypeCode_tk_sequence 
#define tk_array      TypeCode_tk_array
#define tk_alias	  TypeCode_tk_alias
#define tk_except	  TypeCode_tk_except
#define tk_longlong   TypeCode_tk_longlong
#define tk_ulonglong  TypeCode_tk_ulonglong
#define tk_longdouble TypeCode_tk_longdouble
#define tk_wchar      TypeCode_tk_wchar
#define tk_wstring    TypeCode_tk_wstring
#define tk_fixed	  TypeCode_tk_fixed
#define tk_pointer    TypeCode_tk_pointer
#define tk_self       TypeCode_tk_self
#define tk_foreign    TypeCode_tk_foreign

typedef struct any 
{
	TypeCode _type;
	void *_value;
} any;

#endif /* somcorba_h */

