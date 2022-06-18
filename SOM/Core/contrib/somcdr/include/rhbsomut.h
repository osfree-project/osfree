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

#ifndef RHBSOMUT_H
#define RHBSOMUT_H


/* #define DEBUG_MARSHAL */


#ifdef BUILD_SOMCDR
	#define SOM_Module_rhbsomuk_Source
	#define SOM_Module_rhbsomus_Source
	#define SOM_Module_somcdr_Source
#endif

#ifndef SOM_TYPECODE
#define SOM_TYPECODE
typedef struct TypeCodeData  *TypeCode;
#endif

#ifdef __cplusplus
	#include <som.xh>
	#include <somderr.h>
#else
	#include <som.h>
	#include <somderr.h>
#endif

#ifndef _IDL_SEQUENCE_octet_defined
	#define _IDL_SEQUENCE_octet_defined
	SOM_SEQUENCE_TYPEDEF(octet);
#endif /* _IDL_SEQUENCE_octet_defined */

#ifdef __cplusplus
extern "C" {
#endif

/* true CORBA C types */

typedef long CORBA_long;
typedef short CORBA_short;
typedef float CORBA_float;
typedef double CORBA_double;
typedef unsigned short CORBA_unsigned_short;
typedef unsigned long CORBA_unsigned_long;
typedef boolean CORBA_boolean;
typedef char CORBA_char;
typedef octet CORBA_octet;
typedef any CORBA_any;

#define RHBCDR_align_delta(pos,by)	((unsigned short)((pos) & (((unsigned short)(by))-1)))
#define RHBCDR_align_diff(pos,by)     ((by) ? ((RHBCDR_align_delta((pos),by)) ? ((unsigned short)(by-RHBCDR_align_delta((pos),by))) : 0) : 0)
#define RHBCDR_align_pos(ppos,by)     if ((by>1) && ((*ppos)&((by)-1))) { *(ppos)=(*(ppos)&(-(long)(by)))+(by); }

#ifdef _PLATFORM_BIG_ENDIAN_
	#define RHBCDR_machine_type()      (0)
#else
	#ifdef _PLATFORM_LITTLE_ENDIAN_
		#define RHBCDR_machine_type()      (1)
	#else
		#define RHBCDR_machine_type()      ((octet)(*((short *)"\001\000")))
	#endif
#endif

SOMEXTERN char *		SOMLINK RHBCDR_cast_any_to_string(Environment *ev,any *a);
SOMEXTERN long			SOMLINK RHBCDR_cast_any_to_long(Environment *ev,any *a);
SOMEXTERN unsigned long SOMLINK RHBCDR_cast_any_to_ulong(Environment *ev,any *a);
SOMEXTERN unsigned long SOMLINK RHBCDR_cast_any_to_enum(Environment *ev,any *a);
SOMEXTERN TypeCode		SOMLINK RHBCDR_cast_any_to_TypeCode(Environment *ev,any *a);
SOMEXTERN SOMObject		SOMSTAR SOMLINK RHBCDR_cast_any_to_SOMObject(Environment *ev,any *a);

SOMEXTERN void SOMLINK RHBCDR_copy_from(TypeCode,Environment *ev,void *target,void *source);
SOMEXTERN any SOMLINK RHBCDR_get_named_element(Environment *ev,any *a,char *str);
SOMEXTERN any SOMLINK RHBCDR_get_nth_element(Environment *ev,any *a,int num);
SOMEXTERN int SOMLINK RHBCDR_count_elements(Environment *ev,any *a);

/* types on the wire */

typedef integer4		GIOP_long;
typedef uinteger4		GIOP_ulong;
typedef uinteger4		GIOP_enum;
typedef short			GIOP_short;
typedef unsigned short	GIOP_ushort;
typedef float			GIOP_float;
typedef double			GIOP_double;
typedef char			GIOP_char;
typedef unsigned char   GIOP_boolean;
typedef unsigned char   GIOP_octet;

SOMEXTERN void SOMLINK RHBCDR_swap_bytes(void *pv,unsigned int len);
SOMEXTERN unsigned long SOMLINK RHBCDR_allocate_octets(_IDL_SEQUENCE_octet *a,Environment *ev,unsigned long length);

typedef struct SOMCDR_unmarshal_filter
{
	void (SOMLINK *read_object)(
		struct SOMCDR_unmarshal_filter *,
		Environment *ev,
		void *,
		TypeCode,
		/* CORBA_DataOutputStream*/ SOMObject SOMSTAR stream);
	void (SOMLINK *free_object)(
		struct SOMCDR_unmarshal_filter *,
		Environment *ev,
		void *,TypeCode);
	struct SOMCDR_unmarshal_filter_data *data;
} SOMCDR_unmarshal_filter;

SOMEXTERN void SOMLINK RHBCDR_unmarshal(
		struct SOMCDR_unmarshal_filter *,
		Environment *ev,
		void *value,
		TypeCode type,
		/* CORBA_DataInputStream */ SOMObject SOMSTAR stream);

typedef struct SOMCDR_marshal_filter
{
	void (SOMLINK *write_object)(
			struct SOMCDR_marshal_filter *,
			Environment *ev,
			any *data,
			SOMObject /*CORBA_DataOutputStream*/ SOMSTAR stream);
	struct SOMCDR_marshal_filter_data *data;
} SOMCDR_marshal_filter;

SOMEXTERN void SOMLINK RHBCDR_marshal(
				struct SOMCDR_marshal_filter *filter,
				Environment *ev,
				void *value,
				TypeCode type,
				SOMObject /*CORBA_DataOutputStream*/ SOMSTAR stream);

typedef struct SOMCDR_marshalling_stream
{
	/* if you call with length zero it means end of stream
		and free */
	unsigned long (SOMLINK *stream_io)(void *stream_data,Environment *ev,octet *buf,unsigned long len);
	void *stream_data;
	unsigned long stream_offset;  /* used for aligning */
} SOMCDR_marshalling_stream;

SOMEXTERN unsigned long SOMLINK RHBCDR_free_data(
				Environment *ev,
				SOMCDR_unmarshal_filter *filter,
				TypeCode type,
				void *value);

SOMEXTERN unsigned long SOMLINK RHBCDR_GIOPenumToIDLenum(GIOP_enum x,Environment *ev,TypeCode t);
SOMEXTERN GIOP_enum SOMLINK RHBCDR_IDLenumToGIOPenum(unsigned long x,Environment *ev,TypeCode t);

SOMEXTERN unsigned long SOMLINK RHBCDR_call_foreign_handler(
				Environment *ev,
				SOMObject SOMSTAR io,
				void *value,
				TypeCode type,
				unsigned long function);

#ifdef __cplusplus
}
#endif

#ifndef RHBSOMUT_unused
#define RHBSOMUT_unused(x)   if (x) { ; }
#endif

#ifdef PRAGMA_IMPORT_SUPPORTED
	#if PRAGMA_IMPORT_SUPPORTED
		#ifdef BUILD_SOMCDR
			#pragma export list RHBCDR_cast_any_to_string
			#pragma export list RHBCDR_cast_any_to_long
			#pragma export list RHBCDR_cast_any_to_ulong
			#pragma export list RHBCDR_cast_any_to_enum
			#pragma export list RHBCDR_cast_any_to_TypeCode
			#pragma export list RHBCDR_cast_any_to_SOMObject
			#pragma export list RHBCDR_free_data
			#pragma export list RHBCDR_copy_from
			#pragma export list RHBCDR_get_named_element
			#pragma export list RHBCDR_get_nth_element
			#pragma export list RHBCDR_count_elements
			#pragma export list RHBCDR_IDLenumToGIOPenum
			#pragma export list RHBCDR_GIOPenumToIDLenum
			#pragma export list RHBCDR_swap_bytes
			#pragma export list RHBCDR_unmarshal
			#pragma export list RHBCDR_marshal
		#else
			#pragma import list RHBCDR_cast_any_to_string
			#pragma import list RHBCDR_cast_any_to_long
			#pragma import list RHBCDR_cast_any_to_ulong
			#pragma import list RHBCDR_cast_any_to_enum
			#pragma import list RHBCDR_cast_any_to_TypeCode
			#pragma import list RHBCDR_cast_any_to_SOMObject
			#pragma import list RHBCDR_free_data
			#pragma import list RHBCDR_copy_from
			#pragma import list RHBCDR_get_named_element
			#pragma import list RHBCDR_get_nth_element
			#pragma import list RHBCDR_count_elements
			#pragma import list RHBCDR_IDLenumToGIOPenum
			#pragma import list RHBCDR_GIOPenumToIDLenum
			#pragma import list RHBCDR_swap_bytes
			#pragma import list RHBCDR_unmarshal
			#pragma import list RHBCDR_marshal
		#endif
	#endif
#endif

#endif /* RHBSOMUT_H */

