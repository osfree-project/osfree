/**************************************************************************
 *
 *  Copyright 1998-2010, Roger Brown
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

/*****************************************************************

header is a 32 byte area containing
===================================

long magic1=0x05179979
long magic2=0x000000c8
long rootOffset=offset of root container record
long freeList=start of free chain
long unknown=
long unknown
long unknown=0x30FC1200/0x31FF3E27= date stamp of emitter?
long unknown

example headers

1. 32 byte empty

05 17 99 79 | 00 00 00 C8 | 00 00 00 00 | 00 00 00 00
90 F9 BF 00 | 00 00 00 00 | 88 F8 12 00 | 00 00 00 00

2. from AIX

05 17 99 79 | 00 00 00 C8 | 00 07 CB 8D | 00 07 31 A2
00 00 1E CC | 00 00 00 DC | 31 FF 3E 27 | 00 00 00 00

3. from win32

05 17 99 79 | 00 00 00 C8 | 00 0F E1 E0 | 00 05 8E D2
00 00 10 A5 | 00 00 00 00 | 30 FC 12 00 | 00 00 00 00


records are not aligned and can be at any offset

Long
====
4 bytes, bigendian

String
======
long stringLen;   -1 means null string
char data[(stringLen > 0) ? (stringLen+1) : 1]

TypeCode
========
long kind (if kind==-1 then it's a NULL typecode)
if (kind != -1)
{
    long n2=0;
	long n3=1;
	long n4=0;
    switch (kind)
	{
	case null,ulong,long,ushort,short,
	     float,double,octet,boolean,char,
		 any,TypeCode,void
		 {
		      
		 }
	case string:
		 {
     	     long stringLen;
		 }
	case objref:
		 {
		     string interfaceId;
		 }
    case pointer:
		 {
		     TypeCode ptrOf;
		 }
    case self:
		 {
		     string selfName;
		 }
	case foreign
	     { 
		     string typeName;
			 string context;
			 long length;
		 }
	case array,sequence:
		{
			TypeCode typeOf
			long length;
		}
	case union:
		{
             string name;
			 TypeCode tcSwitch
			 long elCount;
             struct
			 {
                 long eType;
				 switch (eType)
				 {
				 case TCREGULAR_CASE:
				    long val;
 				 case TCDEFAULT_CASE:
				 }
				 string nameElement;
				 TypeCode tcElement;

			 } elements[elCount];
		}
	case struct:
	    {
		     string name;
			 long elCount;
			 struct
			 {
			      string nameElement;
				  TypeCode tcElement;
			 } elements[elCount];
		}
	case enum:
	    {
		     string name;
			 long elCount;
			 string elNames[elCount];
		}
	}
}

Empty
=====
long lengthOfRecord
long type=10
long offsetNext
.......

Container
=========
long lengthOfRecord
long type=8
long nItems
struct 
{ 
	long offsetContained;
	long typeContained;
	string name;
} items[nItems]

Contained
=========
long length;
long type;
string name;

ConstantDef
===========
Contained cnd;
long zero1;
long definedInOffset;
long zero2;
long modifiersOffset;

ExceptionDef
============
Contained cnd;
long zero1;
long definedInOffset;
long zero2;
long modifiersOffset;

TypeDef
=======
Contained cnd;
long zero1;
long definedInOffset;
long zero2;
long modifiersOffset;

InterfaceDef
============
Contained cnd;
long zero1;
long definedInOffset;
long descendantsOffset;
long modifiersOffset;
long contentsOffset;

InterfaceDef::Descendants
==========================
long length;
long num=9;
long nItems;
long itemOffsets[nItems]	these should be other interfaces

ModuleDef
=========
Contained cnd;
long zero1;
long definedInOffset;
long zero2;
long modifersOffset;
long contentsOffset;

OperationDef
============
Contained cnd;
long zero1=0;
long definedIn;
long zero2=0;
long modifiersOffset;
long contentOffset;
long zero3=0;
long one=1 { NORMAL,ONEWAY ? };

ParameterDef
============
Contained cnd;
long zero1=0
long definedIn;
long zero2=0;
long modifiersOffset;
long parameterMode={INOUT,IN,OUT}

AttributeDef
============
Contained cnd;
long zero1;
long definedIn;
long zero2;
long modifiersOffset;
long attrMode {NORMAL,READONLY}

Modifiers
=========
long len;
long modtype=Contained
long owner;
long zero;
long numModifiers;
struct
{
	string name;
	string value;
} modifiers[numModifiers];
switch (owner->type)
{
case InterfaceDef:
	{
		long numParents;
		string parents[numParents];
		TypeCode instanceData;
	}
case ModuleDef:
	 {
	 }
case AttributeDef:
	{
		TypeCode attribType;
	}
case ParameterDef:
	{
		TypeCode paramType;
	}
case ExceptionDef:
	{
		TypeCode exceptType;
	}
case TypeDef:
	{
		TypeCode typeDef;
	}
case ConstantDef:
	{
		TypeCode constantType;
		string value;
	}
case OperationDef:
	{
		TypeCode returnType;
		long numContexts;
		string contexts[numContexts];
		long numRaises;
		string raises[numRaises];
	}
}

*******************************************************************/

#define kSOMIR_Magic1			0x05179979		
#define kSOMIR_Magic2			0x000000C8

#define kSOMIR_ConstantDef		0
#define kSOMIR_TypeDef			1
#define kSOMIR_ExceptionDef		2
#define kSOMIR_InterfaceDef		3
#define kSOMIR_ModuleDef		4
#define kSOMIR_AttributeDef		5
#define kSOMIR_OperationDef		6
#define kSOMIR_ParameterDef		7
#define kSOMIR_Container		8
#define kSOMIR_Descendant		9
#define kSOMIR_Empty			10
#define kSOMIR_Contained		11

#define kSOMIR_free_root		12
#define kSOMIR_free_next		8

#ifndef __cplusplus

struct SOMIRfile;
struct SOMIR_ContainedData;
struct SOMIR_ContainerData;

struct SOMIRfileVtbl
{
	void (*Release)(struct SOMIRfile *);
	void (*Detach)(struct SOMIRfile *);
	void (*Seek)(struct SOMIRfile *,long);
	struct SOMIR_ContainedData *(*Lookup)(struct SOMIRfile *,const char *);	
	void (*Close)(struct SOMIRfile *);
};

typedef struct SOMIR_alloc SOMIR_alloc;

struct SOMIR_alloc
{
	struct SOMIR_alloc_vtbl *lpVtbl;
	_IDL_SEQUENCE_octet data;
};

struct SOMIR_alloc_vtbl
{
	void *(*alloc)(struct SOMIR_alloc *,size_t,short);
	void *(*realise)(struct SOMIR_alloc *);
};

struct SOMIR_read_stream
{
	struct SOMIR_read_stream_vtbl *lpVtbl;
};

struct SOMIR_read_stream_vtbl
{
	size_t (*read)(struct SOMIR_read_stream *,void *,size_t);
};

struct SOMIR_read_data
{
	struct SOMIR_read_data_vtbl *lpVtbl;
	struct SOMIR_read_stream *stream;
	struct SOMIR_alloc *allocator;
};

struct SOMIR_name_id_pair
{
	char *name;
	char *id;
};

struct SOMIR_read_data_vtbl
{
	long (*read_long)(struct SOMIR_read_data *);
	char *(*read_string)(struct SOMIR_read_data *);
	_IDL_SEQUENCE_string (*read_strings)(struct SOMIR_read_data *);
	TypeCode (*read_TypeCode)(struct SOMIR_read_data *);
	struct SOMIR_name_id_pair (*read_nip)(struct SOMIR_read_data *,char *);
};

struct SOMIR_read_stream_buffer
{
	struct SOMIR_read_stream stream;
	_IDL_SEQUENCE_octet data;
};

struct SOMIR_ContainerItem
{
	long offset;		/* where */
	short type;			/* what type */
	short namelen;		/* length of name */
	char *name;			/* name of item */
};

struct SOMIR_DescendantItem
{
	long offset;
	struct SOMIR_ContainedData *containedData;	/* set when realised */
	long lockCount;								/* references held by container */
};

struct SOMIR_ContainerDataVtbl
{
	void (*AddRef)(struct SOMIR_ContainerData *);
	void (*Release)(struct SOMIR_ContainerData *);
	void (*Detach)(struct SOMIR_ContainerData *);
	struct SOMIR_ContainedData *(*Acquire)(struct SOMIR_ContainerData *,long,long);
};

struct SOMIR_ContainerData
{
	struct SOMIR_ContainerDataVtbl *lpVtbl;
	struct SOMIR_ContainedData *first;
	struct SOMIR_ContainedData *last;
	struct
	{
		unsigned long _length,_maximum;
		struct SOMIR_ContainerItem *_buffer;
	} itemSeq;
	struct SOMIR_ContainedData *defined_in;
	struct SOMIRfile *file;
};


struct SOMIR_ContainedDataVtbl
{
	void (*AddRef)(struct SOMIR_ContainedData *);
	void (*Release)(struct SOMIR_ContainedData *);
	Contained_Description (*describe)(struct SOMIR_ContainedData *,Environment *);
	InterfaceDef_FullInterfaceDescription (*describe_interface)(struct SOMIR_ContainedData *,Environment *);
	struct SOMIR_ContainerData *(*IsContainer)(struct SOMIR_ContainedData *);
};

struct SOMIR_ContainedData
{
	struct SOMIR_ContainedDataVtbl *lpVtbl;

	struct SOMIR_ContainedData *prev;			/* previous peer */
	struct SOMIR_ContainedData *next;			/* next peer */
	struct SOMIR_ContainerData *defined_in;		/* parent */

	rhbatomic_t usage;									/* usage count */
	short type;									/* type of thing */
	long containedOffset;						/* offset for this record */

	_IDL_SEQUENCE_somModifier modifiers;

	union
	{
		TypeDef_TypeDescription typeDesc;
		ExceptionDef_ExceptionDescription exceptionDesc;
		AttributeDef_AttributeDescription attributeDesc;
		ParameterDef_ParameterDescription parameterDesc;

		struct
		{
			ConstantDef_ConstantDescription desc;
			char *value;
		} constantDesc;

		struct
		{
			ModuleDef_ModuleDescription desc;
			struct SOMIR_ContainerData containerData;
		} moduleDesc;

		struct
		{
			OperationDef_OperationDescription desc;
			_IDL_SEQUENCE_string raiseSeq;
			struct SOMIR_ContainedData **exceptions;
			struct SOMIR_ContainerData containerData;
			void *args;
		} operationDesc;

		struct
		{
			InterfaceDef_InterfaceDescription desc;
			_IDL_SEQUENCE_string base_interfaces;
			TypeCode instanceData;
			struct
			{
				unsigned long _length,_maximum;
				struct SOMIR_DescendantItem *_buffer;
			} descendants;
			struct SOMIR_ContainerData containerData;
		} interfaceDesc;
	} description;
};

struct SOMIRheader
{
	long magic1;
	long magic2;
	long masterOffset;
	long freeList;
	long unknown1;
	long unknown2;
	long timeStamp;
	long zero2;
};

struct SOMIR_Repository
{
	struct SOMIR_RepositoryVtbl *lpVtbl;
	struct SOMIRfile *first;
	struct SOMIRfile *last;
	rhbatomic_t inUse;
};

struct SOMIRfile
{
	struct SOMIRfileVtbl *lpVtbl;
	struct SOMIR_read_stream stream;
	struct SOMIRfile *next;
	struct SOMIRfile *prev;
	struct SOMIR_Repository *repository;
#ifdef _WIN32
	void * fd;
#else
	int fd;
#endif
	long fileOffset;
	struct SOMIRheader header;
	struct SOMIR_ContainerData containerData;
	rhbatomic_t usage;
	char name[1];
};

#define offset_of(x,y)	  ((short)(long)(void *)&(((x *)NULL)->y))

/* #define alignment_of(x)   ((short)(long)(void *)&(((struct { octet _o; x _d; } *)NULL)->_d)) */

#define alignment_of(x)   offset_of(struct { octet _o; x _d; },_d)

#define SOMIR_cast_self(priv_t,ptr,el)	(priv_t *)(((char *)ptr)-offset_of(priv_t,el))

extern void irDumpFile(struct SOMIRfile *somThis);

struct SOMIRfile *SOMIRopen(struct SOMIR_Repository *,const char *);

#endif
