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
#include <rhbsomex.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <som.h>
#include <somtc.h>
#include <somtcnst.h>
#include <somir.h>
#include <attribdf.h>
#include <paramdef.h>
#include <operatdf.h>
#include <intfacdf.h>
#include <typedef.h>
#include <constdef.h>
#include <moduledf.h>
#include <rhbmtut.h>

#include <somirfmt.h>

struct ttycols
{
	size_t x;
	size_t tabstop;
};

static void dump_label(struct ttycols *pNest,char *label)
{
	size_t labelLen=strlen(label);

	while ((pNest->x+labelLen+2)<(pNest->tabstop))
	{
		somPrintf(" ");
		pNest->x++;
	}

	pNest->x+=somPrintf("%s: ",label);
}

static void dump_description(struct ttycols *pNest,any *desc,Environment *ev)
{
	switch (TypeCode_kind(desc->_type,ev))
	{
	case tk_struct:
		{
			long offset=0;
			octet *data=desc->_value;
			long index=1;
			long params=TypeCode_param_count(desc->_type,ev)>>1;

			pNest->tabstop+=4;

			while (params--)
			{
				any name=TypeCode_parameter(desc->_type,ev,index);
				any type=TypeCode_parameter(desc->_type,ev,index+1);
				char *label=*(char **)name._value;
				TypeCode tc=*(TypeCode *)type._value;
				long len=TypeCode_size(tc,ev);
				short align=TypeCode_alignment(tc,ev);
				any a;

				if (align)
				{
					long mask=align-1;
					if (offset & mask)
					{
						offset=(offset+align)&~mask;
					}
				}

				if (pNest->x)
				{
					somPrintf("\n");
					pNest->x=0;
				}

				dump_label(pNest,label);

				a._type=tc;
				a._value=data+offset;

				dump_description(pNest,&a,ev);
				
				offset+=len;

				index+=2;
			}

			pNest->tabstop-=4;
		}
		break;
	case tk_any:
		{
			any *a=desc->_value;
			dump_description(pNest,a,ev);
		}
		break;
	case tk_long:
		{
			long val=*(long *)desc->_value;
			somPrintf("%ld",val);
		}
		break;
	case tk_string:
		{
			char *val=*(char **)desc->_value;
			somPrintf("%s",val ? val : "");
		}
		break;
	case tk_TypeCode:
		{
			TypeCode val=*(TypeCode *)desc->_value;
			if (val)
			{
				TypeCode_print(val,ev);
			}
		}
		break;
	case tk_enum:
		{
			TCKind e=*(TCKind *)desc->_value;
			any e2=TypeCode_parameter(desc->_type,ev,e);
			char *p=*(char **)e2._value;
			somPrintf("%s",p);
		}
		break;
	case tk_sequence:
		{
			GENERIC_SEQUENCE *seq=desc->_value;
			any type=TypeCode_parameter(desc->_type,ev,0);
			TypeCode tc=*(TypeCode *)type._value;
			long len=TypeCode_size(tc,ev);
			short align=TypeCode_alignment(tc,ev);
			octet *buffer=seq->_buffer;
			unsigned long i=0;
			size_t mask=align-1;
			size_t offset=0;
			somPrintf("[%ld,%ld]",seq->_length,seq->_maximum);

			pNest->tabstop+=4;

			while (i < seq->_length)
			{
				any a;
				char label[22];

				if (pNest->x)
				{
					somPrintf("\n");

					pNest->x=0;
				}

				snprintf(label,sizeof(label),"%ld",i);

				dump_label(pNest,label);

				if (offset & mask)
				{
					offset=(offset+align)&~mask;
				}

				a._type=tc;
				a._value=buffer+offset;

				dump_description(pNest,&a,ev);

				offset+=len;

				i++;
			}

			pNest->tabstop-=4;
		}
		break;
	default:
		TypeCode_print(desc->_type,ev);
		RHBOPT_ASSERT(!ev);
		break;
	}
}

static void dump_modifiers(struct SOMIR_ContainedData *cnd)
{
	unsigned long i=0;
	somPrintf("\tmodifiers[%ld,%ld]\n",cnd->modifiers._length,cnd->modifiers._maximum);
	while (i < cnd->modifiers._length)
	{
		char *p=cnd->modifiers._buffer[i].name;
		if (p)
		{
			somPrintf("\tname: \"%s\"\n",p);
		}
		else
		{
			somPrintf("\tname: NULL\n");
		}
		p=cnd->modifiers._buffer[i].value;
		if (p)
		{
			somPrintf("\tvalue: \"%s\"\n",p);
		}
		else
		{
			somPrintf("\tvalue: NULL\n");
		}
		i++;
	}
}

static void irDumpContained(struct SOMIRfile *somThis,
							struct SOMIR_ContainerData *defined_in,
							long parent,
							long offsetContained,
							long typeContained,
							const char *name)
{
	if (somThis)
	{
		static struct ttycols nesting={0,12};

		struct SOMIR_ContainedData *cnd=
				defined_in->lpVtbl->Acquire(defined_in,offsetContained,typeContained);
	
		{
			Environment ev={NO_EXCEPTION,{NULL,NULL}};
			Contained_Description desc=cnd->lpVtbl->describe(cnd,&ev);
			struct SOMIR_ContainerData *cnr=cnd->lpVtbl->IsContainer(cnd);

			dump_description(&nesting,&desc.value,&ev);

			if (nesting.x)
			{
				somPrintf("\n");
				nesting.x=0;
			}

			SOMFree(desc.value._value);

			dump_modifiers(cnd);

			if (cnr)
			{
				unsigned long i=0;

				while (i < cnr->itemSeq._length)
				{
					irDumpContained(somThis,
						cnr,
						offsetContained,
						cnr->itemSeq._buffer[i].offset,
						cnr->itemSeq._buffer[i].type,
						cnr->itemSeq._buffer[i].name);

					i++;
				}
			}
		}

		if (cnd->type==kSOMIR_InterfaceDef)
		{
			Environment ev={NO_EXCEPTION,{NULL,NULL},NULL};
			InterfaceDef_FullInterfaceDescription desc=
				cnd->lpVtbl->describe_interface(cnd,&ev);
			any a;
			
			a._type=TC_FullInterfaceDescription;
			a._value=&desc;

			dump_description(&nesting,&a,&ev);

			if (desc.name) SOMFree(desc.name);
			if (desc.operation._buffer) SOMFree(desc.operation._buffer);
			if (desc.attributes._buffer) SOMFree(desc.attributes._buffer);
		}

		cnd->lpVtbl->Release(cnd);
	}
}

void irDumpFile(struct SOMIRfile *somThis)
{
	unsigned long i=0;

	while (i < somThis->containerData.itemSeq._length)
	{
		irDumpContained(somThis,&somThis->containerData,0,
			somThis->containerData.itemSeq._buffer[i].offset,
			somThis->containerData.itemSeq._buffer[i].type,
			somThis->containerData.itemSeq._buffer[i].name);

		i++;
	}

#if 0
	if (somThis->header.freeList)
	{
		long freeItem=somThis->header.freeList;
		long freeCount=0;
		long smallest=0x7fffffff;
		long largest=0;

		printf("\nunknown=%ld,%ld\n",somThis->unknown1,somThis->unknown2);

		while (freeItem)
		{
			long len,type,next;
			irSeek(somThis,freeItem);
			len=irReadLong(somThis);
			type=irReadLong(somThis);
			next=irReadLong(somThis);

			printf("free[%ld]: %ld,%ld,%ld\n",freeItem,len,type,next);

			if (type != SOMIR_Empty)
			{
				__asm int 3;
			}

			if (len > largest) largest=len;
			if (len < smallest) smallest=len;

			freeCount+=len;

			freeItem=next;
		}

		printf("unknown1=%ld,unknown=%ld,freeCount=%ld,largest=%ld,smallest=%ld\n",
				somThis->unknown1,
				somThis->unknown2,
				freeCount,
				largest,
				smallest);
	}
#endif
}

