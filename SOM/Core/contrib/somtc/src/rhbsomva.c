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

#ifdef SOMDLLEXPORT
	#define SOM_IMPORTEXPORT_somtc  SOMDLLEXPORT
#else
	#define SOM_IMPORTEXPORT_somtc  
#endif

#include <rhbvargs.h>
#include <somtc.h>

#ifndef _IDL_SEQUENCE_octet_defined
	#define _IDL_SEQUENCE_octet_defined
	SOM_SEQUENCE_TYPEDEF(octet);
#endif /* _IDL_SEQUENCE_octet_defined */

typedef enum va_type { type_int, type_long, type_double, type_pointer } va_type;

typedef struct somva_element
{
	va_type type;
	union
	{
		int arg_int;
		long arg_long;
		double arg_double;
		void *arg_pointer;
	} u;
} somva_element;

typedef struct somvabuf
{
	RHBVARGS vargs;

	int alloc_len;
	SOM_SEQUENCE(somva_element) elements;
} somvabuf;

SOMEXTERN void * SOMLINK somVaBuf_create(char *memPtr, int len)
{
	somvabuf *args;

	SOM_IgnoreWarning(memPtr);
	SOM_IgnoreWarning(len);

	args=SOMMalloc(sizeof(*args));

	if (args)
	{
		args->alloc_len=0;
		args->elements._length=0;
		args->elements._maximum=0;
		args->elements._buffer=0;
	}

	return args;
}

SOMEXTERN void   SOMLINK somVaBuf_get_valist(void *vabuf, va_list *va_ptr)
{
#if defined(HAVE_VA_LIST_OVERFLOW_ARG_AREA) || defined(HAVE_VA_LIST___VA_OVERFLOW_ARG_AREA)
	#ifdef HAVE_VA_LIST___VA_GP_OFFSET
		(*va_ptr)->__va_gp_offset=0;
		(*va_ptr)->__va_fp_offset=0;
	#else
		#ifdef HAVE_VA_LIST_GPR
			(*va_ptr)->gpr=0;
			(*va_ptr)->fpr=0;
		#else
			(*va_ptr)->gp_offset=0;
			(*va_ptr)->fp_offset=0;
		#endif
	#endif
	#ifdef HAVE_VA_LIST___VA_OVERFLOW_ARG_AREA
		(*va_ptr)->__va_overflow_arg_area=NULL;
		(*va_ptr)->__va_reg_sve_area=NULL;
	#else
		(*va_ptr)->overflow_arg_area=NULL;
		(*va_ptr)->reg_save_area=NULL;
	#endif
#else
	#if (defined(RHBVARGS_LIST_BASE) && defined(RHBVARGS_LIST_OFFSET))
		(*va_ptr).RHBVARGS_LIST_BASE=NULL;
		(*va_ptr).RHBVARGS_LIST_OFFSET=0;
	#else
		RHBVARGS_VA_LIST_PTR(*va_ptr)=0;
	#endif
#endif

	if (vabuf)
	{
		somvabuf *args=vabuf;

		if (!args->alloc_len)
		{
			unsigned int i=args->elements._length;
			somva_element *_buffer=args->elements._buffer;

			if (!i)
			{
				return;
			}

			args->alloc_len=sizeof(_buffer->u)*i;

			RHBVARGS_alloc(args->vargs,args->alloc_len);

			while (i--)
			{
				switch (_buffer->type)
				{
				case type_int:
					RHBVARGS_push(args->vargs,int,_buffer->u.arg_int);
					break;
				case type_long:
					RHBVARGS_push(args->vargs,long,_buffer->u.arg_long);
					break;
				case type_double:
					RHBVARGS_push(args->vargs,double,_buffer->u.arg_double);
					break;
				case type_pointer:
					RHBVARGS_push(args->vargs,void *,_buffer->u.arg_pointer);
					break;
				}

				_buffer++;
			}
		}

#if defined(HAVE_VA_LIST_OVERFLOW_ARG_AREA) || defined(HAVE_VA_LIST___VA_OVERFLOW_ARG_AREA)
	#ifdef HAVE_VA_LIST___VA_GP_OFFSET
			(*va_ptr)->__va_gp_offset=args->vargs.args->__va_gp_offset;
			(*va_ptr)->__va_fp_offset=args->vargs.args->__va_fp_offset;
	#else
		#ifdef HAVE_VA_LIST_GPR
			(*va_ptr)->gpr=args->vargs.args->gpr;
			(*va_ptr)->fpr=args->vargs.args->fpr;
		#else
			(*va_ptr)->gp_offset=args->vargs.args->gp_offset;
			(*va_ptr)->fp_offset=args->vargs.args->fp_offset;
		#endif
	#endif

	(*va_ptr)->RHBVARGS_LIST_OVERFLOW_ARG_AREA=args->vargs.args->RHBVARGS_LIST_OVERFLOW_ARG_AREA;
	(*va_ptr)->RHBVARGS_LIST_REG_SAVE_AREA=args->vargs.args->RHBVARGS_LIST_REG_SAVE_AREA;
#else
	*va_ptr=args->vargs.args;
#endif
	}
}

SOMEXTERN void   SOMLINK somVaBuf_destroy(void *vabuf)
{
	somvabuf *args=vabuf;

	if (args)
	{
		if (args->alloc_len)
		{
			RHBVARGS_free(args->vargs);
		}
		if (args->elements._buffer) SOMFree(args->elements._buffer);
		SOMFree(args);
	}
}

#define element_arg(x,t)   x->type=type_##t; x->u.arg_##t
#define element_value(p,t)	*((t *)p)

SOMEXTERN long   SOMLINK somVaBuf_add(void *vabuf , void *memPtr, int tk_type)
{
	somvabuf *args=vabuf;
	somva_element *_buffer;

	if (!args) return 0;

	if (args->elements._length==args->elements._maximum)
	{
		unsigned int i=args->elements._length;

		args->elements._maximum+=4;

		_buffer=SOMMalloc(sizeof(*_buffer) * args->elements._maximum);

		while (i--)
		{
			_buffer[i].type=args->elements._buffer[i].type;

			switch (_buffer[i].type)
			{
			case type_int:
				_buffer[i].u.arg_int=args->elements._buffer[i].u.arg_int;
				break;
			case type_long:
				_buffer[i].u.arg_long=args->elements._buffer[i].u.arg_long;
				break;
			case type_double:
				_buffer[i].u.arg_double=args->elements._buffer[i].u.arg_double;
				break;
			case type_pointer:
				_buffer[i].u.arg_pointer=args->elements._buffer[i].u.arg_pointer;
				break;
			default:
				RHBOPT_ASSERT(!"unknown type");
				break;
			}
		}
		
		if (args->elements._buffer) SOMFree(args->elements._buffer);
		args->elements._buffer=_buffer;
	}

	_buffer=&args->elements._buffer[args->elements._length];


#define	element_copy(st,vt) element_arg(_buffer,vt)=element_value(memPtr,st)
#define element_case(st,vt) case tk_##st: element_copy(st,vt); break;
#define element_case2(st,rt,vt) case tk_##st: element_copy(rt,vt); break;

	switch(tk_type)
	{
	element_case(octet,int)
	element_case(boolean,int)
	element_case(char,int)
	element_case(short,int)
	element_case(long,long)
	element_case(float,double)
	element_case(double,double)
	element_case(TypeCode,pointer)
	element_case2(ushort,unsigned short,int)
	element_case2(ulong,unsigned long,long)
	element_case2(pointer,void *,pointer)
	element_case2(string,char *,pointer)
	element_case2(objref,SOMObject SOMSTAR,pointer)

	/* in theory our SOM enums are *supposed* to be longs no matter the platform ....
		using TCKind here because it's the only enum we know about 
		*/

	case tk_enum:
		if (sizeof(TCKind) > sizeof(int))
		{
			element_arg(_buffer,long)=element_value(memPtr,TCKind);
		}
		else
		{
			element_arg(_buffer,int)=element_value(memPtr,TCKind);
		}
		break;
	default:
		RHBOPT_ASSERT(!"unhandled type")
		return 0;
	}

	args->elements._length++;

	return 1;
}

SOMEXTERN void * SOMLINK somvalistGetTarget(va_list args)
{
	/*
#if defined(HAVE_VA_LIST_OVERFLOW_ARG_AREA)
	va_list args_copy;
	args_copy[0]=args[0];
	return va_arg(args_copy,void *);
#else
	return va_arg(args,void *);
#endif
	*/

	va_list args_copy;
	void *retval;

	va_copy(args_copy,args);

	retval=va_arg(args_copy,void *);

	va_end(args_copy);

	return retval;
}

SOMEXTERN void   SOMLINK somvalistSetTarget(va_list args, void* tgt)
{
	RHBVARGS vargs;
	
/*	{
		SOMObject SOMSTAR obj=tgt;
		somPrintf("somvalistSetTarget(%s)\n",obj->mtab->className);
	}
*/

#if defined(HAVE_VA_LIST_OVERFLOW_ARG_AREA) || defined(HAVE_VA_LIST___VA_OVERFLOW_ARG_AREA)
/*	somPrintf("somvalistSetTarget(gpr=%d)\n",args->gpr);*/
	#ifdef HAVE_VA_LIST_GPR
		if (args->gpr < RHBVARGS_GPRN)
		{
			__va_regsave_t *regs=(void *)args->reg_save_area;

			regs->__gp_save[args->gpr]=(long)tgt;

			return;
 		}
	#else
		#ifdef __x86_64__
			if (args->RHBVARGS_LIST_GP_OFFSET < RHBVARGS_GPRN)
			{
				void **p=(void *)(((char *)(void *)args->RHBVARGS_LIST_REG_SAVE_AREA)+args->RHBVARGS_LIST_GP_OFFSET);
				*p=tgt;
				return;
			}
		#else
			#error __x86_64__ not defined
		#endif
	#endif
	vargs.push[0]=args[0];
#else
	vargs.push=args;
#endif

	RHBVARGS_push(vargs,void *,tgt);
}
