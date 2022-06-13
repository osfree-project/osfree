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

#include <somcdr.h>

typedef struct RHBCDR_encapsulation_write
{
	struct RHBCDR_encapsulation_write *parent_ctx;
	SOMCDR_CDRInputStream_StreamState stream;
	struct
	{
		SOMCDR_marshalling_stream *st; /* previous stream */
	} stack;
	char *rep_id;
	char *name;
	unsigned long tc_start_offset;
	unsigned long buffer_start;
	TCKind kind;
	/* used for read/write callbacks */

	unsigned long stream_data_io_ptr;
	_IDL_SEQUENCE_octet stream_data;
	Environment *ev;
} RHBCDR_encapsulation_write;

typedef struct RHBCDR_encapsulation_read
{
	struct RHBCDR_encapsulation_read *parent_ctx;
	SOMCDR_CDRInputStream_StreamState stream;
	struct
	{
		SOMCDR_CDRInputStream_StreamState *st; /* previous stream */
	} stack;
	TCKind kind;
	char *rep_id;
	char *name;
	int nest;
	unsigned long tc_start_offset;
	unsigned long encap_length;
	SOMCDR_StructMemberSeq *struct_data;
	SOMCDR_UnionMemberSeq *union_data;
	SOMCDR_EnumMemberSeq *enum_data;
	TypeCode encap_tc;
} RHBCDR_encapsulation_read;

#define RHBCDR_begin_read_encap(ev,rt,kind) \
	if (!ev->_major) {  RHBCDR_encapsulation_read __encap; \
		current_ctx=__RHBCDR_begin_read_encap(&__encap,current_ctx,ev,rt,kind) ;\
		rt=&__encap.stream; \
		RHBOPT_cleanup_push(__RHBCDR_end_read_encap,&__encap); {

#define RHBCDR_end_read_encap   \
	} RHBOPT_cleanup_pop(); }

#define RHBCDR_begin_write_encap(ot,ev,kind) \
{ RHBCDR_encapsulation_write __encap_thunk; \
	if (__RHBCDR_begin_write_encap(&__encap_thunk,current_ctx,ot,ev,kind)) { \
	RHBCDR_encapsulation_write *current_ctx=&__encap_thunk; \
	SOMCDR_marshalling_stream * RHBOPT_volatile ot=&__encap_thunk.stream.stream; \
	RHBOPT_cleanup_push(__RHBCDR_end_write_encap,&__encap_thunk); {

#define RHBCDR_end_write_encap  \
	} __RHBCDR_commit_write_encap(&__encap_thunk); RHBOPT_cleanup_pop(); } }
