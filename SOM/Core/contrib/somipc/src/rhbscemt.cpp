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

#include <rhbopt.h>
/* base emitter */

/* IDL compiler */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <rhbsc.h>


RHBemitter::RHBemitter(RHBrepository *rr)
{
	_rr=rr;
	idl_name=0;
}

boolean RHBemitter::must_generate(RHBtype *)
{
	return 1;
}

RHBoutput::~RHBoutput()
{
}

void RHBemitter::get_ir_name(RHBelement *element,char *buf,size_t buflen)
{
	if (element->parent())
	{
		get_ir_name(element->parent(),buf,buflen);
		if (strcmp(buf,"::"))
		{
			strncat(buf,"::",buflen);
		}
		strncat(buf,element->id,buflen);
	}
	else
	{
		strncpy(buf,"::",buflen);
		if (element->id)
		{
			strncat(buf,element->id,buflen);
		}
	}
}

boolean RHBemitter::ShouldEmit(RHBelement *t)
{
	if (!t->defined_in) return 0;

	if (strcmp(idl_name,t->defined_in->id))
	{
		/* this allows a module statement to
		wrap an include file,
		ideally should only emit include
		files if included during processing
		of the root level */

		if (t->parent() != GetRepository())
		{
			return ShouldEmit(t->parent());
		}

		return 0;
	}

	if (t->parent() != GetRepository())
	{
		return ShouldEmit(t->parent());
	}

	if (t->is_interface())
	{
		return 1;
	}

	if (t->is_module())
	{
		return 1;
	}

	return t->emit_root(t);
}


boolean RHBemitter::CheckTrueCORBA(RHBtype *type)
{
	if (!type) return 1;

	if (type->is_operation())
	{
		RHBoperation *op=type->is_operation();
		
		if (op->return_type)
		{
			if (!CheckTrueCORBA(op->return_type))
			{
/*				char buf[256];

				get_ir_name(op,buf);

				printf("%s was vetoed\n",buf);
*/
				return 0;
			}
		}

		int i;

		i=0;

		while (op->get_parameter(i))
		{
			RHBparameter *param=op->get_parameter(i);

			i++;

			if (!CheckTrueCORBA(param->parameter_type))
			{
/*				char buf[256];

				get_ir_name(param,buf);

				printf("%s was vetoed\n",buf);
*/
				return 0;
			}
		}

		return 1;
	}

	if (type->is_typedef())
	{
		if (type->is_typedef()->alias_for->is_pointer())
		{
			char buf[256];
			get_ir_name(type,buf,sizeof(buf));

			if (!strcmp(buf,"::SOMFOREIGN"))
			{
				return 1;
			}
		}

		return CheckTrueCORBA(type->is_typedef()->alias_for);
	}

	if (type->is_sequence())
	{
		return CheckTrueCORBA(type->is_sequence()->sequence_of);
	}

	if (type->is_array())
	{
		return CheckTrueCORBA(type->is_array()->array_of);
	}

	if (type->is_pointer())
	{
		RHBtype *typ=type->is_pointer()->pointer_to;

		if (typ->is_struct()) return 1;
		if (typ->is_union()) return 1;

		return CheckTrueCORBA(type->is_pointer()->pointer_to);
	}

	if (type->is_struct())
	{
		RHBelement *el;

		el=type->children();

		while (el)
		{
			if (el->is_type())
			{
				if (!CheckTrueCORBA(el->is_type()))
				{
					return 0;
				}
			}

			el=el->next();
		}
	}

	if (type->is_struct_element())
	{
		return CheckTrueCORBA(type->is_struct_element()->element_type);
	}

	return 1;
}

void RHBemitter::get_global_id(RHBelement *element,char *buf,size_t buflen)
{
	RHBelement *a[256];
	int i=0;
	char *p;

	if (element->global_id)
	{
		strncpy(buf,element->global_id,buflen);
		return;
	}

	strncpy(buf,"IDL:",buflen);

	while (element)
	{
		a[i++]=element;
		if (element->prefix)
		{
			element=0;
		}
		else
		{
			element=element->parent();
		}
	}

	p=0;

	while (i--)
	{
		if (a[i]->prefix)
		{
			if (p) strncat(buf,"/",buflen);
			p=a[i]->prefix;
			strncat(buf,p,buflen);
		}

		if (a[i]->id)
		{
			if (p) strncat(buf,"/",buflen);
			p=a[i]->id;
			strncat(buf,p,buflen);
		}
	}

	strncat(buf,":1.0",buflen);
}

const char *RHBemitter::short_file_name(const char *long_file_name)
{
	const char *p;

	p=long_file_name;

	while (*p) p++;

	while (p > long_file_name)
	{
		p--;
		switch (*p)
		{
		case ':':
		case '/':
		case '\\':
			p++;
			return p;
		}
	}

	return long_file_name;
}

const char *RHBemitter::get_corbastring()
{
	const char *p="corbastring";

	if (_rr)
	{
		char buf[256];
		RHBelement *el;

		strncpy(buf,p,sizeof(buf));

		el=_rr->find_named_element(0,buf,sizeof(buf));

		if (el)
		{
			if (el->is_modifier())
			{
				return p;
			}
		}
	}

	return "string";
}

void RHBemitter::get_c_name(RHBelement *element,char *buf,size_t buflen)
{
	if (element->parent())
	{
		get_c_name(element->parent(),buf,buflen);
		if (buf[0])
		{
			strncat(buf,"_",buflen);
		}
		strncat(buf,element->id,buflen);
	}
	else
	{
		if (element->id)
		{
			strncpy(buf,element->id,buflen);
		}
		else
		{
			buf[0]=0;
		}
	}
}

RHBtype *RHBemitter::unwind_typedef(RHBtype *t)
{
	while (t)
	{
		RHBtype_def *td;
		RHBqualified_type *qt;

		qt=t->is_qualified();

		if (qt)
		{
			break;

/*			t=qt->base_type;*/
		}

		td=t->is_typedef();

		if (!td) return t;

		t=td->alias_for;
	}

	return t;
}

RHBtype *RHBemitter::is_SOMFOREIGN(RHBtype *t)
{
	RHBtype_def *td=t->is_typedef();

	if (td)
	{
		char buf[256];

		get_ir_name(td,buf,sizeof(buf));

		if (!strcmp(buf,"::SOMFOREIGN"))
		{
			return t;
		}

/*		t=td;

		td=td->alias_for->is_typedef();*/
	}

	return 0;
}

const char *RHBemitter::unquote(char *tgt,const char *src)
{
	char *p=tgt;

	while (*src)
	{
		char c=*src++;
		if ((c!=double_quote)&&(c!=single_quote))
		{
			*p++=c;
		}
	}

	*p=0;

	return tgt;
}

RHBemitter::~RHBemitter()
{
}

void RHBemitter::out_printf(RHBoutput *out,const char *fmt,...)
{
	char buf[4096];
	va_list ap;
	va_start(ap,fmt);
	int i=vsnprintf(buf,sizeof(buf),fmt,ap);
	out->write(buf,i);
	va_end(ap);
}

