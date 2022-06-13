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

/* IDL compiler */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rhbsc.h>

char double_quote='\"';
char single_quote='\'';

RHBmodule::RHBmodule(RHBPreprocessor *pp,const char *name) :
RHBelement("RHBmodule",pp,name)
{

}

void RHBmodule::build(RHBPreprocessor *pp)
{
	char buf[256];
	RHBelement_new_scope scope(pp,this);

	buf[0]=0;

	while (1)
	{
		RHBelement *e;

		if (!buf[0])
		{
			if (!pp->get_token(buf,sizeof(buf)))
			{
				pp->err("Failed while building module",id);

				return;
			}
		}

		if (0==strcmp(buf,"}"))
		{
			pp->get_token(buf,sizeof(buf));

			if (strcmp(buf,";"))
			{
				pp->err("module should tail with a ;",buf);

				return;
			}

			buf[0]=0;

			return;
		}

		e=read_token(pp,buf,sizeof(buf));

		if (!e)
		{
			pp->err("failed to understand token",buf);
			return;
		}
	}
}

RHBrepository::RHBrepository(RHBPreprocessor *pp,const char *name) :
/*RHBelement("RHBrepository",pp,name),*/
	RHBmodule(pp,name),
	_pp(*pp)
{
	RHBbase_type::boot_root_types(/*pp*/0,this);
}

void RHBrepository::build(RHBPreprocessor *pp)
{
	char buf[1024];
	RHBelement_new_scope scope(pp,this);

	buf[0]=0;

	while (1)
	{
		RHBelement *e;

		if (!buf[0])
		{
			if (!pp->get_token(buf,sizeof(buf)))
			{
				return;
			}
		}

		e=read_token(pp,buf,sizeof(buf));

		if (!e)
		{
			pp->err("token failed to build",buf);

			return;
		}
	}
}

RHBelement * RHBelement::read_token(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	if (!buf[0])
	{
		return 0;
	}

	if (!strcmp(buf,"interface"))
	{
		return read_interface(pp,buf,buflen);
	}

	if (!strcmp(buf,"typedef"))
	{
		return read_typedef(pp,buf,buflen);
	}

	if (!strcmp(buf,"exception"))
	{
		return read_exception(pp,buf,buflen);
	}

	if (!strcmp(buf,"const"))
	{
		return read_constant(pp,buf,buflen);
	}

	if (!strcmp(buf,"module"))
	{
		return read_module(pp,buf,buflen);
	}

	if (!strcmp(buf,"#pragma"))
	{
		return read_pragma(pp,buf,buflen);
	}

	if (0==strcmp(buf,"enum"))
	{
		return read_enum(pp,buf,buflen);
	}

	if (0==strcmp(buf,"struct"))
	{
		RHBtype *t=read_struct(pp,buf,buflen);

		if (strcmp(buf,";"))
		{
			pp->err("should be ';'",buf);
		}
		else
		{
			buf[0]=0;
		}

		return t;
	}

	if (0==strcmp(buf,"union"))
	{
		RHBtype *t=read_union(pp,buf,buflen);

		if (strcmp(buf,";"))
		{
			pp->err("should be ';'",buf);
		}
		else
		{
			buf[0]=0;
		}

		return t;
	}

	return 0;
}

void RHBelement::reset_defined_in(RHBPreprocessor *pp)
{
	if (pp)
	{
		defined_in=pp->CurrentFile();
		defined_line=pp->CurrentLine();
		emit_root=pp->GetEmitRoot();
	}
	else
	{
		defined_in=0;
		emit_root=0;
		defined_line=0;
	}
}

RHBelement *RHBelement::_mem_first;

void RHBelement::delete_all(RHBelement *ex)
{
	while (_mem_first)
	{
		RHBelement *el;

		el=_mem_first;

/*		printf("el==%s::%s\n",el->class_name,el->id);*/

		if (el->_parent)
		{
			el->_parent->remove(el);
		}

		el->destroy();

		if (ex != el)
		{
			delete el;
		}
		else
		{
			printf("dont delete root\n");
		}
	}
}

void RHBelement::destroy()
{
	RHBelement *el;

	while (_children)
	{
		el=_children;

		remove(el);

		el->destroy();

		delete el;
	}

	el=_mem_first;

	if (el==this)
	{
		_mem_first=_mem_next;
	}
	else
	{
		while (el->_mem_next != this)
		{
			el=el->_mem_next;
		}

		el->_mem_next=_mem_next;
	}
}

RHBelement::RHBelement(const char *cls,RHBPreprocessor *pp,const char *name) 
{
	class_name=cls;
	_children=NULL;
/*	_last=0;*/
	_parent=NULL;
	_next=NULL;
	id=NULL;
	prefix=NULL;
	global_id=NULL;
	version=NULL;

	_mem_next=_mem_first;
	_mem_first=this;

	if (pp)
	{
		defined_in=pp->CurrentFile();
		defined_line=pp->CurrentLine();
		emit_root=pp->GetEmitRoot();
	}
	else
	{
		defined_in=0;
		emit_root=0;
		defined_line=0;
	}

	if (name)
	{
		if (*name)
		{
/*			if (!isalnum(*name))
			{
				if (*name!='_')
				{
					pp->err(cls,name);
					bomb("RHBelement::RHBelement()");
				}
			}
			*/
			size_t len=strlen(name)+1;
			id=new char [len];

			strncpy(id,name,len);

#ifdef _DEBUG
/*			if (!strcmp(id,"string"))
			{
				printf("string created %p\n",this);
			}*/
#endif
		}
	}
}

RHBelement::~RHBelement() 
{
	if (id) 
	{
		delete id;
		id=NULL;
	}

	if (prefix) 
	{
		delete prefix;
		prefix=NULL;
	}

	if (global_id) 
	{
		delete global_id;
		global_id=NULL;
	}

	if (version) 
	{
		delete version;
		version=NULL;
	}
}

RHBinterface * RHBelement::read_interface(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	RHBinterface *ifac;
	char b[256];
	RHBelement *el;

	if (!buf[0]) return 0;

	if (strcmp(buf,"interface"))
	{
		return 0;
	}

	buf[0]=0;

	pp->get_token(b,sizeof(b));

	strncpy(buf,b,buflen);

	ifac=NULL;

	el=find_named_element(pp,b,sizeof(b));

	if (el)
	{
		ifac=el->is_interface();
	}

	if (ifac)
	{
		if (ifac->parent() != this)
		{
/*			printf("## ignoring interface %s, creating %s::%s\n",
					ifac->id,
					id,
					ifac->id);
*/
			ifac=new RHBinterface(pp,ifac->id);

			add(pp,ifac);
		}
	}

	if (!ifac)
	{
		ifac=new RHBinterface(pp,buf);

		add(pp,ifac);

		b[0]=0;
	}

	buf[0]=0;

	if (!b[0])
	{
		pp->get_token(b,sizeof(b));
	}

	if (0==strcmp(b,";"))
	{
		return ifac;
	}

	if (ifac->next())
	{
		if (ifac->parent()==this)
		{
			remove(ifac);
		}
		else
		{
			ifac=new RHBinterface(pp,ifac->id);
		}

		add(pp,ifac);
	}

	if (0==strcmp(b,":"))
	{
		/* construct parent list */
		b[0]=0;

		while (1)
		{
			RHBinterface *f=0;

			if (!b[0])
			{
				pp->get_token(b,sizeof(b));
			}

			if (!strcmp(b,"{"))
			{
				break;
			}

			el=find_named_element(pp,b,sizeof(b));

			if (el)
			{
				f=el->is_interface();
			}

			if (!f)
			{
				pp->err("Can't find interface ",b);

				return 0;
			}

			ifac->_parents.add(f);

			if (!b[0])
			{
				pp->get_token(b,sizeof(b));
			}
			if (strcmp(b,","))
			{
				break;
			}

			b[0]=0;
		}
	}

	if (0==strcmp(b,"{"))
	{
		/* changed the defined_in now */

		ifac->reset_defined_in(pp);

		ifac->build(pp);
	}

	return ifac;
}

RHBtype * RHBelement::read_type(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	RHBtype *rt=NULL;

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	if ((0==strcmp(buf,"signed"))||(0==strcmp(buf,"unsigned")))
	{
		RHBtype *t2;
		RHBqualified_type *qt;

		qt=new RHBqualified_type(pp,buf);

		buf[0]=0;

		t2=read_type(pp,buf,buflen);

		qt->base_type=t2;

		return qt;
	}

	if (0==strcmp(buf,"enum"))
	{
		/* need to build an enum */

		return read_enum(pp,buf,buflen);
	}

	if (0==strcmp(buf,"string"))
	{	
		RHBstring_type *t=0;

		buf[0]=0;

		pp->get_token(buf,buflen);

		if (0==strcmp(buf,"<"))
		{
			t=new RHBstring_type(pp,"string");
			/* read a number */

			pp->get_token(buf,buflen);

			t->length=read_number(pp,buf,buflen);

			if (!buf[0])
			{
				pp->get_token(buf,buflen);
			}

			if (strcmp(buf,">"))
			{
				pp->err("length should tail with >",buf);
			}

			buf[0]=0;
		}
		else
		{
			char b2[256];
			RHBelement *el;
			RHBtype *tt;

			strncpy(b2,"string",sizeof(b2));
			el=find_named_element(0,b2,sizeof(b2));
			tt=el->is_type();
			t=tt->is_string();
/*			if (!t) __asm int 3;*/
			return tt;
		}

		if (!buf[0])
		{
			pp->get_token(buf,buflen);
		}

		return t;
	}

	if (0==strcmp(buf,"struct"))
	{
		rt=read_struct(pp,buf,buflen);
	}
	else
	{
		if (0==strcmp(buf,"union"))
		{
			rt=read_union(pp,buf,buflen);
		}
		else
		{
			if (0==strcmp(buf,"sequence"))
			{
				RHBsequence_type *ts;

				pp->get_token(buf,buflen);

				if (strcmp(buf,"<"))
				{
					pp->err("sequence should follow with <",buf);

					return 0;
				}

				pp->get_token(buf,buflen);

/*				if (0==strcmp(buf,"banana_type"))
				{
					printf("building banana_type\n");
				}
*/
				rt=read_type(pp,buf,buflen);

				if (!rt)
				{
					pp->err("Failed to read sequence type",buf);

					return 0;
				}

				if (rt->is_pointer())
				{
					pp->err("Can't have a sequence of pointers - wrap with TypeDef\n",buf);

					return 0;
				}

				ts=new RHBsequence_type(pp,rt->id);

				ts->sequence_of=rt;

				if (!buf[0])
				{
					pp->get_token(buf,buflen);
				}

				if (!strcmp(buf,","))
				{
					pp->get_token(buf,buflen);

					ts->length=read_number(pp,buf,buflen);

					if (!buf[0])
					{
						pp->get_token(buf,buflen);
					}
				}
				
				if (strcmp(buf,">"))
				{
					pp->err("sequence should tail with >",buf);

					return 0;
				}

				buf[0]=0;
				rt=ts;
			}
			else
			{
				RHBelement *el;

				rt=0;

				el=find_named_element(pp,buf,buflen);

				if (!el)
				{
					return 0;
				}

				if (!buf[0])
				{
					pp->get_token(buf,buflen);
				}

				if (el)
				{
					rt=el->is_type();
				}
				else
				{
					pp->err("Failed to find type",buf);
					return 0;
				}

/*				if (el->is_modifier())
				{
					printf("Modifier again %s\n",el->id);
				}*/
			}
		}
	}

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

/*	while ((0==strcmp(buf,"*"))||(0==strcmp(buf,"&")))
	{
		RHBpointer_type *t2;

		t2=new RHBpointer_type(0);

		strcpy(t2->type,buf);

		buf[0]=0;
		t2->pointer_to=t;

		t=t2;

		pp->get_token(buf);
	}
*/
	return rt;
}

RHBtype * RHBelement::read_typedef(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	RHBtype *t;
	RHBtype_def *t2;

	if (!buf[0])
	{
		return 0;
	}

	if (strcmp(buf,"typedef"))
	{
		return 0;
	}

	buf[0]=0;

	pp->get_token(buf,buflen);

	t=read_type(pp,buf,buflen);

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	while (0==strcmp(buf,"*"))
	{
		t=wrap_with_pointer(pp,buf,buflen,t);

		if (!buf[0])
		{
			pp->get_token(buf,buflen);
		}
	}

	t2=new RHBtype_def(pp,buf);

	pp->get_token(buf,buflen);

	if (0==strcmp(buf,"["))
	{
		t=wrap_as_array(pp,buf,buflen,t);
	}

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	if (0==strcmp(buf,";"))
	{
		RHBelement *e;

		t2->alias_for=t;

		e=_children;

		while (e)
		{
			if (0==strcmp(t2->id,e->id))
			{
				const char *old_file=e->defined_in->id;
				int old_line=e->defined_line;

				printf("## previous def in %s:%d\n",old_file,old_line);

				pp->err("Duplication of typedef",buf);

				buf[0]=0;

				return e->is_type();
			}

			e=e->_next;
		}

		add(pp,t2);

		buf[0]=0;

		return t2;
	}

	pp->err("typedef error near",buf);

	delete t2;

	return 0;
}

RHBmodule * RHBelement::read_module(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	RHBmodule *m;
	RHBelement *el;
	char name[64];

	if (strcmp(buf,"module"))
	{
		pp->err("Should say 'module'",buf);

		return 0;
	}

	pp->get_token(buf,buflen);

	m=0;

	strncpy(name,buf,sizeof(name));

	el=find_named_element(pp,buf,buflen);

	if (el)
	{
		m=el->is_module();
	}

	if (!m)
	{
		m=new RHBmodule(pp,name);

		add(pp,m);

		buf[0]=0;
	}

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	if (0==strcmp(buf,";"))
	{
		buf[0]=0;

		return m;
	}

	if (strcmp(buf,"{"))
	{
		pp->err("module should open with braces",buf);

		return 0;
	}

	buf[0]=0;

	m->build(pp);

	return m;
}

RHBexception::RHBexception(RHBPreprocessor *pp,const char *name)
:/*	RHBelement("RHBexception",pp,name),*/
	RHBtype(pp,name)
{
}

void RHBexception::build(RHBPreprocessor *pp)
{
	char buf[256];

	buf[0]=0;

	while (1)
	{
		RHBtype *t;
		RHBstruct_element *s;

		if (!buf[0])
		{
			pp->get_token(buf,sizeof(buf));
		}

		if (0==strcmp(buf,"}"))
		{
			pp->get_token(buf,sizeof(buf));

			if (strcmp(buf,";"))
			{
				pp->err("exception should tail with ;",buf);
				return;
			}

			return;
		}

		t=read_type(pp,buf,sizeof(buf));

		if (!t)
		{
			pp->err("exception should contain types",buf);
			return;
		}

		if (!buf[0])
		{
			pp->get_token(buf,sizeof(buf));
		}

		while (0==strcmp(buf,"*"))
		{
			t=wrap_with_pointer(pp,buf,sizeof(buf),t);

			if (!buf[0])
			{
				pp->get_token(buf,sizeof(buf));
			}
		}

		s=new RHBstruct_element(pp,buf);

		s->element_type=t;

		add(pp,s);

		pp->get_token(buf,sizeof(buf));

		while (0==strcmp(buf,"["))
		{
			s->element_type=wrap_as_array(pp,buf,sizeof(buf),s->element_type);

			if (!buf[0])
			{
				pp->get_token(buf,sizeof(buf));
			}
		}

		if (strcmp(buf,";"))
		{
			pp->err("exception parameter should tail with a ;",buf);
			return;
		}

		buf[0]=0;
	}
}

RHBexception * RHBelement::read_exception(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	RHBexception *e;

	if (strcmp(buf,"exception"))
	{
		pp->err("Should be the word exception",buf);

		return 0;
	}

	pp->get_token(buf,buflen);

	e=new RHBexception(pp,buf);

	add(pp,e);

	pp->get_token(buf,buflen);

	if (0==strcmp(buf,";"))
	{
		pp->err("exception should open with {",buf);

		return e;
	}

	if (strcmp(buf,"{"))
	{
		pp->err("exception should open with {",buf);

		return 0;
	}

	buf[0]=0;

	e->build(pp);

	return e;
}

RHBconstant * RHBelement::read_constant(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	RHBtype *t;
	RHBconstant *c;

	if (strcmp(buf,"const"))
	{
		pp->err("should say const",buf);

		return 0;
	}

	pp->get_token(buf,buflen);

/*	if (!strcmp(buf,"completion_status"))
	{
		printf("looking..\n");
	}
*/
	t=read_type(pp,buf,buflen);

	if (!t)
	{
		pp->err("Failed to read type for constant",buf);

		return 0;
	}

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

#ifdef _DEBUG__XX
	if (!strcmp(buf,"kODNoEditor"))
	{
		__asm int 3;
	}
#endif

	c=new RHBconstant(pp,buf);
	add(pp,c);

	buf[0]=0;

	c->constant_type=t;

	c->read_c_value(pp,buf,buflen);

/*	printf("added constant %s\n",c->id);*/

	return c;
}

RHBmodule * RHBelement::is_module()
{
	return 0;
}

RHBinterface * RHBelement::is_interface()
{
	return 0;
}

RHBtype * RHBelement::is_type()
{
	return 0;
}

RHBparameter * RHBelement::is_parameter()
{
	return 0;
}

RHBinterface::RHBinterface(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBinterface",pp,name), */RHBtype(pp,name)
{
	instanceData=0;
	classData=0;
	cclassData=0;
}

RHBattribute::RHBattribute(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBattribute",pp,name),*/ RHBtype(pp,name)
{
	readonly=0;
}

RHBconstant::RHBconstant(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBconstant",pp,name), */
/*  RHBtype(pp,name),*/
  RHBvalue(pp,name)
{
	value_string=0;
	const_val=0;
	const_typecode_val=0;
	is_private=0;
}

RHBtype::RHBtype(RHBPreprocessor *pp,const char *name)
: RHBelement("RHBtype",pp,name)
{
}

RHBelement * RHBelement::find_named_element(RHBPreprocessor *pp,char *n,size_t nlen)
{
	RHBelement *e=_children;

	if (!n) bomb("find_named_element with no name");

	while (e)
	{
		if (0==strcmp(e->id,n))
		{
			if (!pp)
			{
				n[0]=0;

				return e;
			}

			pp->get_token(n,nlen);

			if (strcmp(n,"::"))
			{
				return e;
			}

			pp->get_token(n,nlen);

			e=e->_children;
		}
		else
		{
			e=e->_next;
		}
	}

	if (_parent)
	{
		e=_parent->find_named_element(pp,n,nlen);
	}

/*	if (!e)
	{
		if (is_interface())
		{
			RHBinterface *iface=is_interface();
			int i=iface->_parents.length();
			int j=0;

			while (j < i)
			{
				RHBinterface *i2=iface->_parents.get(j)->is_interface();
				j++;
				e=i2->find_named_element(pp,n);
				if (e) return e;
			}
		}
	}
*/
	return e;
}

static void append_name(char *buf,size_t buflen,RHBelement *e)
{
/*	if (e->parent)
	{
		e->parent->append_name(buf,e->parent);
	}
*/
	if (buf[0])
	{
		strncat(buf,"_",buflen);
	}

	strncat(buf,e->id,buflen);
}

void RHBinterface::build(RHBPreprocessor *pp)
{
	char buf[256];

	if (instanceData)
	{
		pp->err("Interface is already built\n",id);
	}
	else
	{
		buf[0]=0;

		append_name(buf,sizeof(buf),this);

		strncat(buf,"Data",sizeof(buf)-1);

		instanceData=new RHBstruct(pp,buf);
	}

	{
		size_t k=0;

		while (k < _parents.length())
		{
			RHBinterface *par=_parents.get(k)->is_interface();
			if (par)
			{
				inherit_operations(pp,par);
			}
			k++;
		}
	}

	buf[0]=0;

	while (1)
	{
		RHBelement *e;
		boolean readOnly=0;
		boolean oneWay=0;

		if (!buf[0])
		{
			if (!pp->get_token(buf,sizeof(buf)))
			{
				pp->err("Could not complete interface",id);

				return;
			}
		}

		if (0==strcmp(buf,"}"))
		{
			build_instance_data(pp);

			/* this should be the end */

			pp->get_token(buf,sizeof(buf));

			if (strcmp(buf,";"))
			{
				pp->err("Should be a ;",buf);

				return;
			}

			return;
		}

		if (0==strcmp(buf,"readonly"))
		{
			buf[0]=0;

			pp->get_token(buf,sizeof(buf));

			readOnly=1;
		}

		if (0==strcmp(buf,"attribute"))
		{
			RHBtype *t;
			RHBattribute *attr;

			pp->get_token(buf,sizeof(buf));

			t=read_type(pp,buf,sizeof(buf));

			if (!t) 
			{
				pp->err("Failed to read type of attribute",buf);

				return;
			}

			while (t)
			{
				RHBtype *t2=t;

				if (!buf[0])
				{
					pp->get_token(buf,sizeof(buf));
				}

				while (0==strcmp(buf,"*"))
				{
					t2=wrap_with_pointer(pp,buf,sizeof(buf),t2);

					if (!buf[0])
					{
						pp->get_token(buf,sizeof(buf));
					}
				}

				attr=new RHBattribute(pp,buf);

				add(pp,attr);

				attr->readonly=readOnly;

				pp->get_token(buf,sizeof(buf));

				while (0==strcmp(buf,"["))
				{
					t2=wrap_as_array(pp,buf,sizeof(buf),t2);

					if (!buf[0])
					{
						pp->get_token(buf,sizeof(buf));
					}
				}

				attr->attribute_type=t2;

				if (strcmp(buf,";"))
				{
					if (strcmp(buf,","))
					{
						pp->err("Error reading attribute",buf);

						return;
					}
				}
				else
				{
					t=0;
				}

				buf[0]=0;

				attr->generate_accessors(pp,this);

/*				RHBstruct_element *ie;

				ie=new RHBstruct_element(pp,attr->id);

				ie->element_type=attr->attribute_type;

				instanceData->add(pp,ie);*/
			}

			continue;
		}

		if (0==strcmp(buf,"implementation"))
		{
			buf[0]=0;

			read_implementation(pp,buf,sizeof(buf));

			continue;
		}

		if (0==strcmp(buf,"oneway"))
		{
			buf[0]=0;

			pp->get_token(buf,sizeof(buf));

			oneWay=1;
		}

		e=read_token(pp,buf,sizeof(buf));

		if (!e)
		{
			RHBtype *t;
			RHBoperation *op;

			t=read_type(pp,buf,sizeof(buf));

			if (!t)
			{
				pp->err("Should be a type",buf);
				return;
			}

			while (t)
			{
				char name[256];
				RHBtype *t2=t;

				if (!buf[0])
				{
					pp->get_token(buf,sizeof(buf));
				}

				while (0==strcmp(buf,"*"))
				{
					t2=wrap_with_pointer(pp,buf,sizeof(buf),t2);

					if (!buf[0])
					{
						pp->get_token(buf,sizeof(buf));
					}
				}

				strncpy(name,buf,sizeof(name));

				pp->get_token(buf,sizeof(buf));

				if (0==strcmp(buf,"("))
				{
					op=new RHBoperation(pp,name);

					op->oneWay=oneWay;

					add(pp,op);

					op->return_type=t2;

					op->build(pp);

					buf[0]=0;

					t=0;
				}
				else
				{
					RHBstruct_element *e2;

					while (0==strcmp(buf,"["))
					{
						t2=wrap_as_array(pp,buf,sizeof(buf),t2);
						if (!buf[0])
						{
							pp->get_token(buf,sizeof(buf));
						}
					}

					e2=new RHBstruct_element(pp,name);

					e2->element_type=t2;

					add(pp,e2);

					if (strcmp(buf,","))
					{
						if (strcmp(buf,";"))
						{
							pp->err("wrong terminator...",buf);

							return;
						}
						else
						{
							t=0;
						}
					}

					buf[0]=0;
				}
			}
		}
	}
}

void RHBinterface::build_instance_data(RHBPreprocessor *pp)
{
	RHBelement *el;

	el=children();

	while (el)
	{
		if (el->is_type())
		{
			if (el->is_type()->is_attribute())
			{
				RHBattribute *attr=el->is_type()->is_attribute();
				if (!attr->has_modifier_value("nodata"))
				{
					RHBstruct_element *ie;

					ie=new RHBstruct_element(pp,attr->id);

					ie->element_type=attr->attribute_type;

					instanceData->add(pp,ie);
				}
/*				else
				{
					printf("%s::%s has no data\n",id,attr->id);
				}*/
			}
		}

		el=el->next();
	}
}

RHBbase_type::RHBbase_type(
		RHBPreprocessor *pp,
		const char *name,
		RHBtype *va_type,
		boolean in_by_ref,
		boolean floating)
: /*RHBelement("RHBbase_type",pp,name),*/RHBtype(pp,name)
{
	_floating=floating;
	_va_type=va_type;
	_in_by_ref=in_by_ref;
}

RHBtype_def::RHBtype_def(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBtype_def",pp,name),*/RHBtype(pp,name)
{
	alias_for=0;
}

void RHBelement::remove(RHBelement *ch)
{
	RHBelement *c;

	if (ch->_parent != this)
	{
		bomb("mismatched parent in RHBelement::remove");

		exit(1);
	}

	ch->_parent=0;

	c=_children;

	if (c==ch)
	{
		_children=ch->_next;
	}
	else
	{
		while (c->_next != ch)
		{
			c=c->_next;
		}

		c->_next=ch->_next;
	}

	ch->_next=0;
}

void RHBelement::add(RHBPreprocessor *pp,RHBelement *child)
{
	if (isdigit(child->id[0]))
	{
		bomb("RHBelement::add::isdigit()");
	}

	if (!isalpha(child->id[0]))
	{
		if (child->id[0]!='_')
		{
			pp->err("element not alpha-numeric",child->id);

			bomb("RHBelement::add::!isalpha()");
		}
	}

	if (!child->_parent)
	{
		if (child->_next)
		{
			child->_next=0;
		}

		if (_children)
		{
			RHBelement *e;

			e=_children;

			while (e)
			{
				if (0==strcmp(e->id,child->id))
				{
					char buf[1024];

					snprintf(buf,sizeof(buf),"named item already defined in %s:%ld",
							e->defined_in->id,(long)e->defined_line);
					pp->err(buf,e->id);

					bomb("RHBelement::add");
				}

				if (e->_next)
				{
					e=e->_next;
				}
				else
				{
					break;
				}
			}

			e->_next=child;
		}
		else
		{
			_children=child;
		}

		child->_parent=this;
	}
	else
	{
			bomb("RHBelement::add");
	}

	child->gen_global_id();
}

void RHBelement::add_nocheck(RHBPreprocessor *pp,RHBelement *child)
{
	if (!child->_parent)
	{
		if (child->_next)
		{
			child->_next=0;
		}

		if (_children)
		{
			RHBelement *e=_children;

			while (e)
			{
				if (e->id && child->id && (0==strcmp(e->id,child->id)))
				{
					char buf[1024];

					snprintf(buf,sizeof(buf),"named item already defined in %s:%ld",
							e->defined_in->id,(long)e->defined_line);
					pp->err(buf,e->id);

					bomb("RHBelement::add");
				}

				if (e->_next)
				{
					e=e->_next;
				}
				else
				{
					break;
				}
			}

			e->_next=child;
		}
		else
		{
			_children=child;
		}

		child->_parent=this;
	}
	else
	{
		bomb("RHBelement::add_nocheck");
	}
}

RHBtype *RHBtype::is_type()
{
	return this;
}

RHBinterface *RHBinterface::is_interface()
{
	return this;
}

void RHBtype::build(RHBPreprocessor *  /*pp*/)
{

}

RHBqualified_type::RHBqualified_type(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBqualified_type",pp,name),*/RHBtype(pp,name)
{
}

RHBpointer_type::RHBpointer_type(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBpointer_type",pp,name),*/RHBtype(pp,name)
{
}

RHBelement * RHBelement::read_pragma(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	if (strcmp(buf,"#pragma"))
	{
		pp->err("Should have said pragma...\n",buf);
		return 0;
	}

	pp->get_token(buf,buflen);

	if (0==strcmp(buf,"somemittypes"))
	{
		pp->get_token(buf,buflen);

		if (!strcmp(buf,"on"))
		{
			pp->SetEmitRoot(RHBshould_emit_on);
		}
		else
		{
			if (!strcmp(buf,"off"))
			{
				pp->SetEmitRoot(RHBshould_emit_off);
			}
			else
			{
				pp->SetEmitRoot(RHBshould_emit_default);
			}
		}

		buf[0]=0;

		return this;
	}

	if (0==strcmp(buf,"modifier"))
	{
		RHBtype *t;

		pp->get_token(buf,buflen);

		t=read_type(pp,buf,buflen);

		if (!t)
		{
			pp->err("Should have been a type",buf);

			return 0;
		}

		if (!buf[0])
		{
			pp->get_token(buf,buflen);
		}

		if (strcmp(buf,":"))
		{
			pp->err("Should have been a ':'",buf);

			return 0;
		}

/*		pp->get_token(buf,buflen);

		RHBelement *e=t->find_named_element(pp,buf,buflen);

		if (!e)
		{
			e=new RHBmodifier(pp,buf);
			t->add(pp,e);
		}

		pp->get_token(buf,buflen);

		if (strcmp(buf,"="))
		{
			pp->err("Should have been '='",buf);

			return 0;
		}*/

		t->read_modifier(pp,buf,buflen,0);

		return this;
	}

	if (0==strcmp(buf,"prefix"))
	{
		pp->get_token(buf,buflen);
		pp->scope->_set_prefix(buf);

		if (pp->scope != this)
		{
			bomb("hmm?");
		}

		set_pragma_modifier(pp,"prefix",buf);

		buf[0]=0;

		return this;
	}

	if (0==strcmp(buf,"ID"))
	{
		RHBelement *id2;

		pp->get_token(buf,buflen);

		id2=find_named_element(pp,buf,buflen);

		id2->_set_global_id(buf);

		id2->set_pragma_modifier(pp,"ID",buf);

		buf[0]=0;

		return this;
	}

	if (0==strcmp(buf,"version"))
	{
		RHBelement *id2;
		char *p;
		char c;

		pp->get_token(buf,buflen);

		id2=find_named_element(pp,buf,buflen);

		p=&buf[strlen(buf)];

		do
		{
			c=pp->get_non_whitespace();

			if (c)
			{
				*p=c;
				p++;
			}

		} while (c);

		*p=0;
		id2->_set_version(buf);

		id2->set_pragma_modifier(pp,"version",buf);

		buf[0]=0;

		return this;
	}

#if 0
	fprintf(stderr,"#pragma %s ....\n",buf);
	fflush(stderr);
#endif

	buf[0]=0;

	pp->pragma_clear();

	return this;
}

RHBenum * RHBelement::read_enum(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	RHBenum *t;

	if (!buf[0])
	{
		return 0;
	}

	if (strcmp(buf,"enum"))
	{
		return 0;
	}

	pp->get_token(buf,buflen);

	t=new RHBenum(pp,buf);

	add(pp,t);

	pp->get_token(buf,buflen);

	if (0==strcmp(buf,";"))
	{
		buf[0]=0;

		return t;
	}
	
	if (0==strcmp(buf,"{"))
	{
		long val=1;		/* for SOM enums begin at one */

		buf[0]=0;

		do
		{

			pp->get_token(buf,buflen);

			RHBenum_val *tv=new RHBenum_val(pp,buf);
			char b[256];
			int i=snprintf(b,sizeof(b),"%ld",(long)val);

			tv->value_string=new char[i+1];
			strncpy(tv->value_string,b,i+1);

			val++;

			t->add(pp,tv);

			if (!pp->get_token(buf,buflen))
			{
				return 0;
			}

		} while (strcmp(buf,"}"));

		buf[0]=0;
	}

	pp->get_token(buf,buflen);

	if (0==strcmp(buf,";"))
	{
		buf[0]=0;

		return t;
	}

	return t;
}

RHBvalue::RHBvalue(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBvalue",pp,name),*/RHBtype(pp,name)
{
	value_string=0;
	was_quoted=0;
	assigned_value=NULL;
}

RHBvalue::~RHBvalue()
{
	if (value_string)
	{
		delete value_string;
		value_string=NULL;
	}
}

RHBenum::RHBenum(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBenum",pp,name),*/RHBtype(pp,name)
{
}

RHBenum_val::RHBenum_val(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBenum_val",pp,name),
  RHBtype(pp,name),*/
  RHBvalue(pp,name)
{
	value_string=0;
}

RHBtype * RHBelement::read_struct(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	RHBstruct *t=0;

	if (!buf[0])
	{
		return 0;
	}

	if (strcmp(buf,"struct"))
	{
		return 0;
	}

	buf[0]=0;

	pp->get_token(buf,buflen);

	if (strcmp(buf,"{"))
	{
		t=new RHBstruct(pp,buf);

		add(pp,t);
		pp->get_token(buf,buflen);
	}
	else
	{
		t=new RHBstruct(pp,0);
	}

	if (0==strcmp(buf,"{"))
	{
		RHBstruct_element *el;

		buf[0]=0;

		while (1)
		{
			if (!buf[0])
			{
				pp->get_token(buf,buflen);
			}

			RHBtype *t2=read_type(pp,buf,buflen);

			if (!buf[0])
			{
				pp->get_token(buf,buflen);
			}

			while (1)
			{
				RHBtype *t3=t2;

				while (0==strcmp(buf,"*"))
				{
					t3=wrap_with_pointer(pp,buf,buflen,t3);

					if (!buf[0])
					{
						pp->get_token(buf,buflen);
					}
				}

				el=new RHBstruct_element(pp,buf);

				el->element_type=t3;

				t->add(pp,el);

				if (!pp->get_token(buf,buflen))
				{
					return 0;
				}

				while (0==strcmp(buf,"["))
				{
					el->element_type=wrap_as_array(pp,buf,buflen,el->element_type);
	
					if (!buf[0])
					{
						pp->get_token(buf,buflen);
					}
				}

				if (strcmp(buf,","))
				{
					break;
				}

				pp->get_token(buf,buflen);
			}

			if (strcmp(buf,";"))
			{
				pp->err("building a struct, should be ;",buf);

				return 0;
			}

			if (!pp->get_token(buf,buflen))
			{
				return 0;
			}

			if (0==strcmp(buf,"}"))
			{
				break;
			}
		}

		buf[0]=0;
	}

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	return t;
}

RHBstruct::RHBstruct(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBstruct",pp,name),*/RHBtype(pp,name)
{
}

RHBstruct_element::RHBstruct_element(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBstruct_element",pp,name),*/RHBtype(pp,name)
{
}

RHBunion::RHBunion(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBunion",pp,name),*/RHBtype(pp,name)
{
	default_index=-1;
}

RHBunion_element::RHBunion_element(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBunion_element",pp,name),*/RHBtype(pp,name)
{
}

RHBarray::RHBarray(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBarray",pp,name),*/RHBtype(pp,name)
{
	elements=0;
}

RHBsequence_type::RHBsequence_type(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBsequence",pp,name),*/RHBtype(pp,name)
{
	length=0;
}

RHBstring_type::RHBstring_type(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBstring_type",pp,name),*/RHBtype(pp,name)
{
	length=0;
}

RHBtype *RHBelement::wrap_as_array(RHBPreprocessor *pp,char *buf,size_t buflen,RHBtype *t)
{
	RHBvalue *elements[256];
	int nDimensions=0;

	if (!buf[0]) 
	{
		pp->get_token(buf,buflen);
	}

	while (!strcmp(buf,"["))
	{
/*		RHBarray *ta;*/

		pp->get_token(buf,buflen);
/*
		ta=new RHBarray(pp,buf);*/

		/* this may be a constant */

/*		ta->elements=read_number(pp,buf);

		ta->array_of=t;*/

		elements[nDimensions++]=read_number(pp,buf,buflen);

		if (!buf[0])
		{
			pp->get_token(buf,buflen);
		}

		if (strcmp(buf,"]"))
		{
			pp->err("Should be ']'",buf);

			return 0;
		}

		pp->get_token(buf,buflen);

/*		t=ta;*/
	}

	if (nDimensions)
	{
		int i=nDimensions;

		while (i--)
		{
			RHBarray *ta=new RHBarray(pp,"0");
			ta->elements=elements[i];
			ta->array_of=t;
			t=ta;
		}
	}

	return t;
}

RHBconstant *RHBelement::is_constant()
{
	return 0;
}

RHBconstant *RHBconstant::is_constant()
{
	return this;
}

void RHBelement::dump_ids(const char *p)
{
	RHBelement *c;

	char buf[1024];

	buf[0]=0;

	if (p) strncpy(buf,p,sizeof(buf));

	if (id)
	{
		strncat(buf,"::",sizeof(buf)-1);
		strncat(buf,id,sizeof(buf)-1);
	}

/*	printf("%s [%s]",buf,class_name);*/

	if (defined_in)
	{
/*		printf("%s",defined_in->id);*/
	}

/*	printf("\n");*/

	c=_children;

	while (c)
	{
		c->dump_ids(buf);
		c=c->_next;
	}

}

RHBelement_sequence::RHBelement_sequence()
{
	_buffer=0;
	_length=0;
	_maximum=0;
}

RHBelement_sequence::~RHBelement_sequence()
{
	if (_buffer) delete _buffer;
}

RHBelement *RHBelement_sequence::get(unsigned long i)
{
	if (i >= _length) return 0;
	return _buffer[i];
}

RHBoperation::RHBoperation(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBoperation",pp,name),*/RHBtype(pp,name)
{
	oneWay=0;
	return_type=0;
}

RHBparameter::RHBparameter(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBparameter",pp,name),*/RHBtype(pp,name)
{
}

RHBmodifier::RHBmodifier(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBmodifier",pp,name),*/RHBtype(pp,name)
{
}

void RHBmodifier::build(RHBPreprocessor *  /*pp*/)
{
}

void RHBoperation::build(RHBPreprocessor *pp)
{
	char buf[256];

/*	opening round bracket should have been detected already
buf[0]=0;

	pp->get_token(buf);

	if (strcmp("(",buf))
	{
		pp->err("Operation should start with a '('",buf);

		return;
	}
*/

	buf[0]=0;

	while (1)
	{
		char mode[6];
		RHBtype *t;
		RHBparameter *param;

		if (!buf[0])
		{
			if (!pp->get_token(buf,sizeof(buf)))
			{
				pp->err("failed while building operation ",id);

				return;
			}
		}

		if (0==strcmp(buf,")"))
		{
			buf[0]=0;

			break;
		}

		if (strcmp(buf,"in"))
		{
			if (strcmp(buf,"out"))
			{
				if (strcmp(buf,"inout"))
				{
					pp->err("parameter should in in,inout or out",buf);

					return;
				}
			}
		}

		strncpy(mode,buf,sizeof(mode));

		buf[0]=0;

		pp->get_token(buf,sizeof(buf));

		t=read_type(pp,buf,sizeof(buf));

		if (!t)
		{
			pp->err("failed to read parameter type",buf);

			return;
		}

		if (!buf[0])
		{
			pp->get_token(buf,sizeof(buf));
		}

		while (0==strcmp(buf,"*"))
		{
			t=wrap_with_pointer(pp,buf,sizeof(buf),t);

			if (!buf[0])
			{
				pp->get_token(buf,sizeof(buf));
			}
		}

		param=new RHBparameter(pp,buf);
		param->parameter_type=t;
		strncpy(param->mode,mode,sizeof(param->mode));
		add(pp,param);

		pp->get_token(buf,sizeof(buf));

		if (0==strcmp(buf,")"))
		{
			/* end of parameter list */

			buf[0]=0;

			break;
		}

		while (0==strcmp(buf,"["))
		{
			param->parameter_type=wrap_as_array(pp,buf,sizeof(buf),
					param->parameter_type);

			if (!buf[0])
			{
				pp->get_token(buf,sizeof(buf));
			}
		}

		if (strcmp(buf,","))
		{
			pp->err("parameters should be separated by ,",buf);

			return;
		}

		buf[0]=0;
	}

	/* we've read the list, now do the raises and context */

	while (1)
	{
		if (!buf[0])
		{
			pp->get_token(buf,sizeof(buf));
		}

		if (0==strcmp(buf,";"))
		{
			return;
		}

		if (0==strcmp(buf,"raises"))
		{
			buf[0]=0;

			pp->get_token(buf,sizeof(buf));
		
			if (strcmp(buf,"("))
			{
				pp->err("raises list should open with a (",buf);
				buf[0]=0;
				return;
			}

			buf[0]=0;

			while (1)
			{
				RHBelement *t;

				if (!buf[0])
				{
					pp->get_token(buf,sizeof(buf));
				}

				if (0==strcmp(buf,")"))
				{
					buf[0]=0;
					break;
				}

				t=find_named_element(pp,buf,sizeof(buf));

				if (!t)
				{
					pp->err("did not find exception",buf);
					buf[0]=0;
					return;
				}

				if (!t->is_exception())
				{
					pp->err("did not find exception",buf);
					buf[0]=0;
					return;
				}

				exception_list.add(t);

				if (strcmp(buf,","))
				{
					if (strcmp(buf,")"))
					{
						pp->err("exception list error",buf);
						buf[0]=0;
						return;
					}
				}
				else
				{
					buf[0]=0;
				}
			}
		}
		else
		{
			if (0==strcmp(buf,"context"))
			{
				pp->get_token(buf,sizeof(buf));

				if (strcmp(buf,"("))
				{
					pp->err("context should be followed by a (",buf);
					buf[0]=0;
					return;
				}

				buf[0]=0;

				while (1)
				{
					if (!buf[0])
					{
						pp->get_token(buf,sizeof(buf));
					}

					RHBcontext_string *str;

					str=new RHBcontext_string(pp,0);

					size_t x=strlen(buf)+1;

					str->value=new char[x];

					strncpy(str->value,buf,x);

					context_list.add(str);

					pp->get_token(buf,sizeof(buf));

					if (!strcmp(buf,")"))
					{
						buf[0]=0;
						break;
					}

					if (strcmp(buf,","))
					{
						pp->err("context strings need comma separator",buf);

						buf[0]=0;

						return;
					}

					buf[0]=0;
				}
			}
			else
			{
				pp->err("unknown trailing operation keyword",buf);
				buf[0]=0;
				return;
			}
		}
	}
}

void RHBinterface::read_implementation(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	boolean contents=0;

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	if (strcmp(buf,"{"))
	{
		pp->err("Should start implementation with {",buf);

		return;
	}

	buf[0]=0;

	while (1)
	{
		boolean passthru_flag=0;
		RHBelement *e=0;

		char name[256];

		if (!buf[0])
		{
			pp->get_token(buf,buflen);
		}

		if (0==strcmp(buf,"}"))
		{
			if (!contents)
			{
				pp->err("implementation is empty",buf);
			}

			buf[0]=0;

			break;
		}

		contents=1;

		if (0==strcmp(buf,"passthru"))
		{
			pp->get_token(buf,buflen);

			e=new RHBmodifier(pp,buf);

			passthru_list.add(e);
	
			buf[0]=0;

			passthru_flag=1;
		}
		else
		{
			e=read_type(pp,buf,buflen);

			if (e)
			{
				RHBtype *t=e->is_type();

				if (!buf[0])
				{
					pp->get_token(buf,buflen);
				}

				if (t && (strcmp(buf,":")))
				{
					while (1)
					{
						RHBtype *t2;
						RHBstruct_element *e2;

						if (!buf[0])
						{
							pp->get_token(buf,buflen);
						}

						t2=t;

						while (0==strcmp(buf,"*"))
						{
							t2=wrap_with_pointer(pp,buf,buflen,t2);

							if (!buf[0])
							{
								pp->get_token(buf,buflen);
							}
						}

#if defined(_DEBUG) && defined(_WIN32) && 0
						printf("debugging: implementation token: %s\n",buf);

						if (!isalpha(buf[0]))
						{
							__asm int 3
						}
#endif
						
						e2=new RHBstruct_element(pp,buf);

						e2->element_type=t2;

						pp->get_token(buf,buflen);

						while (0==strcmp(buf,"["))
						{
							e2->element_type=wrap_as_array(pp,buf,buflen,
									e2->element_type);

							if (!buf[0])
							{
								pp->get_token(buf,buflen);
							}
						}

						instanceData->add(pp,e2);

						if (0==strcmp(buf,";"))
						{
							buf[0]=0;
							break;
						}

						if (strcmp(buf,","))
						{
							pp->err("Should be comma or semicolon",buf);

							buf[0]=0;

							break;
						}

						buf[0]=0;
					}

					continue;
				}
			}
		}

		if (!e)
		{
			if (!buf[0])
			{
				pp->get_token(buf,buflen);
			}

			strncpy(name,buf,sizeof(name));
	
			if (!e)
			{
				e=find_named_element(pp,buf,buflen);
			}
		}

		if (!e)
		{
			e=new RHBmodifier(pp,name);

			add(pp,e);

			buf[0]=0;
		}

		if (!buf[0])
		{
			pp->get_token(buf,buflen);
		}

		e->read_modifier(pp,buf,buflen,passthru_flag);

		if (e->is_called("metaclass"))
		{
			const char *meta=e->modifier_data.get(0);
			char buf2[256];
			RHBelement *em;

			strncpy(buf2,meta,sizeof(buf2));

			remove_quotes(buf2);

/*			printf("'%s' has '%s' as meta class\n",
				id,buf);
*/
			em=find_named_element(0,buf2,sizeof(buf2));

			if (em)
			{
				RHBinterface *m=em->is_interface();

				if (m)
				{
					m->_metaclass_of.add(this);
				}
				else
				{
					printf("'%s' is not an interface\n",meta);
				}
			}
			else
			{
				printf("can't find metaclass '%s'\n",meta);
			}
		}
	}

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	if (strcmp(buf,";"))
	{
		pp->err("should tail implementation with ;",buf);

		return;
	}

	buf[0]=0;
}

boolean RHBelement::is_called(const char *buf)
{
	if (buf[0])
	{
		if (id)
		{
			if (!strcmp(buf,id)) return 1;
		}
	}
	return 0;
}

int RHBelement_sequence::index(RHBelement *e)
{
	int i=0;

	while (1)
	{
		if (_buffer[i]==e) return i;
		i++;
	}
}

void RHBelement_sequence::add(RHBelement *e)
{
	if (_length==_maximum)
	{
		RHBelement **buf;
		size_t i;
		typedef RHBelement * RHBelements;
		_maximum+=32;

		buf=new RHBelements[_maximum];

		i=0;

		while (i < _length)
		{
			buf[i]=_buffer[i];
			i++;
		}

		if (i)
		{
			delete _buffer;
		}

		_buffer=buf;
	}

	_buffer[_length]=e;
	_length++;
}


RHBmodule *RHBmodule::is_module()
{
	return this;
}

void RHBconstant::read_c_value(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	if (strcmp(buf,"="))
	{
		if (strcmp(buf,";"))
		{
			pp->err("error in constant",id);

			return;
		}

		buf[0]=0;

		return;
	}

	buf[0]=0;
	const_typecode_val=0;

	if (is_const_TypeCode())
	{
		const_typecode_val=read_type(pp,buf,buflen);

		if (!const_typecode_val)
		{
			if (!strcmp(buf,"null"))
			{
				buf[0]=0;
			}
		}
	}
	else
	{
		RHBenum *en=is_const_enum();
		if (en)
		{
			pp->get_token(buf,buflen);
			RHBelement *t=en->find_named_element(NULL,buf,buflen);
			if (t)
			{
				const_val=t->is_value();
				buf[0]=0;
			}
			else
			{
				pp->err("Error in constant",buf);
				return;
			}
		}
		else
		{
			if (is_numeric())
			{
				const_val=read_number(pp,buf,buflen);
			}
			else
			{
				pp->get_token(buf,buflen);

				if (buf[0]=='\"')
				{
					size_t i=strlen(buf)-2; /* strip two quotes */

					was_quoted=1;

					if (((int)i) >=0)
					{
						if (i > 0)
						{
							memmove(buf,buf+1,i);
						}
						buf[i]=0;
					}

					size_t x=strlen(buf)+1;
					value_string=new char[x];
					strncpy(value_string,buf,x);
					buf[0]=0;
				}
				else
				{
					RHBelement *t=find_named_element(pp,buf,buflen);

					if (t)
					{
						assigned_value=t->is_constant();

						if (!assigned_value)
						{
							pp->err("Error in constant",buf);
						}

						const_val=assigned_value;
					}
					else
					{
						const_val=read_number(pp,buf,buflen);
					}
				}
			}
		}
	}

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	if (0==strcmp(buf,";"))
	{
		buf[0]=0;

		return;
	}

	pp->err("Error in constant",buf);
}

boolean RHBconstant::is_const_TypeCode()
{
	if (constant_type->is_TypeCode())
	{
		return 1;
	}

	return 0;
}

boolean RHBconstant::is_numeric()
{
	RHBtype *t;
	RHBbase_type *bt;

	t=constant_type;

	while (t->is_typedef())
	{
		t=t->is_typedef()->alias_for;
	}

	while (t->is_qualified())
	{
		t=t->is_qualified()->base_type;
	}

	if (t->is_enum())
	{
		return 1;
	}

	bt=t->is_base_type();

	if (!bt) 
	{
		return 0;
	}

	if (0==strcmp(bt->id,"string")) return 0;

	return 1;
}

#if 0
static void print_id(RHBelement *id)
{
	if (id->parent())
	{
		print_id(id->parent());
		printf("::");
	}
	if (id->id) printf("%s",id->id);
}
#endif

void RHBelement::read_modifier(RHBPreprocessor *pp,char *buf,size_t buflen,boolean passthru_flag)
{
	RHBelement *modifier_root=this;
	RHBelement *current=this;

	if (is_modifier()) 
	{
		current=NULL;
	}

	while (1)
	{
		if (!buf[0])
		{
			if (!pp->get_token(buf,buflen))
			{
				return;
			}
		}

		if (!strcmp(buf,":"))
		{
			buf[0]=0;

			if (!pp->get_token(buf,buflen))
			{
				return;
			}

			RHBelement *e=NULL;
			
			if (current)
			{
				modifier_root->find_named_element(pp,buf,buflen);
			}

			if (!e)
			{
				if (modifier_root->is_modifier())
				{
					current=NULL;

					int k=modifier_root->modifier_data.length();
					
					if (k)
					{
						const char *p=modifier_root->modifier_data.get(k-1);

						if (strcmp(p,","))
						{
							modifier_root->add_modifier_value(",");
						}
					}

					modifier_root->add_modifier_value(buf);

					buf[0]=0;

					break;
				}
				else
				{
					e=new RHBmodifier(pp,buf);

					current->add(pp,e);
					modifier_root=current;

					buf[0]=0;
				}
			}

			current=e;
		}
		else
		{
			break;
		}
	}

	while (1)
	{
		/* assumes current is the working modifier,
			and buf is the following token */

		if (!buf[0])
		{
			if (!pp->get_token(buf,buflen))
			{
				return;
			}
		}


		if (!strcmp(buf,"="))
		{
			/* assignment */

			buf[0]=0;

			if (passthru_flag)
			{
				while (pp->get_token(buf,buflen))
				{
					if (!strcmp(buf,";"))
					{
						buf[0]=0;
						break;
					}
					else
					{
						if (current)
						{
							current->add_modifier_value(buf);
						}
						else
						{
							modifier_root->add_modifier_value(buf);
						}
						buf[0]=0;
					}
				}

				return;
			}

			if (!pp->get_token(buf,buflen))
			{
				return;
			}

			if (current)
			{
				current->add_modifier_value(buf);
			}
			else
			{
				if (modifier_root->modifier_data.length())
				{
					pp->err("reading modifier",buf);

					return;
				}
				else
				{
					modifier_root->add_modifier_value(buf);
				}
			}

			buf[0]=0;

			if (!pp->get_token(buf,buflen))
			{
				return;
			}
		}

		if (!strcmp(buf,","))
		{
			buf[0]=0;

			if (!pp->get_token(buf,buflen))
			{
				return;
			}

			RHBelement *e=NULL;
			
			if (current)
			{
				e=modifier_root->has_named_element(buf);
			}

			if (!e)
			{
				if (modifier_root->is_modifier())
				{
					current=NULL;

					int k=modifier_root->modifier_data.length();
					
					if (k)
					{
						const char *p=modifier_root->modifier_data.get(k-1);

						if (strcmp(p,","))
						{
							modifier_root->add_modifier_value(",");
						}
					}

					modifier_root->add_modifier_value(buf);

					buf[0]=0;
				}
				else
				{
					e=new RHBmodifier(pp,buf);

					modifier_root->add(pp,e);

					buf[0]=0;
				}
			}

			current=e;
		}
		else
		{
			if (!strcmp(buf,";"))
			{
				buf[0]=0;
				break;
			}
			else
			{
				pp->err("reading modifier",buf);

				return;
			}
		}
	}

	if (!current)
	{
		current=modifier_root;
	}

/*	print_id(current); printf("=");

	int k=0;

	while (k < current->modifier_data.length())
	{
		const char *p=current->modifier_data.get(k++);

		printf("%s",p);

		if (k < current->modifier_data.length())
		{
			printf("|");
		}

	}
	printf("\n");*/
}

#if 0
void RHBelement::read_modifier(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	if (!buf[0])
	{
		if (!pp->get_token(buf,buflen))
		{
			return;
		}
	}

	if (0==strcmp(buf,":"))
	{
		char name[256];

		buf[0]=0;

		pp->get_token(name,sizeof(name));

		pp->get_token(buf,buflen);

		if (0==strcmp(buf,":"))
		{
			RHBelement *e=find_named_element(pp,name,sizeof(name));

			if (!e)
			{
				e=new RHBmodifier(pp,name);
				add(pp,e);
			}
			
			if (!buf[0])
			{
				pp->get_token(buf,buflen);
			}
			e->read_modifier(pp,buf,buflen);
		}
		else
		{
			add_modifier_value(name);

			while (1)
			{
				if (!buf[0])
				{
					pp->get_token(buf,buflen);
				}

				if (0==strcmp(buf,";"))
				{
					buf[0]=0;
					return;
				}

				add_modifier_value(buf);

				buf[0]=0;
			}
		}
	}
	else
	{
		if (0==strcmp(buf,"="))
		{
			while (1)
			{
				pp->get_token(buf,buflen);

				if (0==strcmp(buf,";"))
				{
					buf[0]=0;
					return;
				}

				add_modifier_value(buf);
			}
		}
		else
		{
			if (0==strcmp(buf,";"))
			{
				buf[0]=0;
				return;
			}

			pp->err("reading modifier",buf);
		}
	}
}
#endif

void RHBelement::add_modifier_value(const char *buf)
{
/*	char str[1024];

	build_id(str);

	strcat(str,"<");
	strcat(str,class_name);
	strcat(str,">");

	printf("..<%s>+=<%s>\n",str,buf);
	if (!strcmp(buf,"nodata"))
	{
		bomb("RHBelement::add_modifier_value::nodata");
	}*/

	modifier_data.add(buf);
}

int RHBelement::has_modifier_value(const char *p)
{
	RHBelement *el=children();

	while (el)
	{
		RHBmodifier *mod=el->is_modifier();

		if (mod)
		{
			if (!strcmp(mod->id,p))
			{
				return 1;
			}
		}

		el=el->next();
	}

	int j=modifier_data.length();

	while (j--)
	{
		if (0==strcmp(modifier_data.get(j),p))
		{
			return 1;
		}
	}

	return 0;
}

const char * RHBmodifier_list::get(int i)
{
		if (i < _length)
		{
			return _buffer[i];
		}
		return 0;
}

void RHBmodifier_list::add(const char *p)
{
	if (p)
	{
		if (*p)
		{
			size_t x=strlen(p)+1;
			char *q=new char[x];
			strncpy(q,p,x);

			if (_length==_maximum)
			{
				int i=0;

				_maximum+=32;

				char **_buf=new string[_maximum];

				while (i < _length)
				{
					_buf[i]=_buffer[i];
					i++;
				}
				if (i) delete _buffer;
				_buffer=_buf;
			}
			_buffer[_length]=q;
			_length++;
		}
	}
}

RHBelement *RHBelement_sequence::contains(RHBelement *e)
{
	long i=_length;

	while (i--)
	{
		if (_buffer[i]==e) return e;
	}

	return 0;
}

RHBmodifier_list::RHBmodifier_list()
{
	_length=0;
	_buffer=NULL;
	_maximum=0;
}

RHBmodifier_list::~RHBmodifier_list()
{
	int i=0;

	while (i < _length)
	{
		delete _buffer[i];

		i++;
	}

	if (i)
	{
		delete _buffer;
	}
}

void RHBelement::build_id(char *buf,size_t buflen)
{
	if (_parent) 
	{
		_parent->build_id(buf,buflen);
	}
	else
	{
		*buf=0;
	}

	if (id)
	{
		strncat(buf,"::",buflen);
		strncat(buf,id,buflen);
	}
}

RHBexception *RHBelement::is_exception()
{
	return 0;
}

RHBexception *RHBexception::is_exception()
{
	return this;
}

RHBinclude_file::RHBinclude_file(RHBPreprocessor *pp,const char *name)
: RHBelement("RHBinclude_file",pp,name)
{
	included_during=pp->scope;
}

RHBcontext_string::RHBcontext_string(RHBPreprocessor *pp,const char *name)
: RHBelement("RHBcontext_string",pp,name)
{
	value=NULL;
}

RHBcontext_string::~RHBcontext_string()
{
	if (value)
	{
		delete value;
		value=NULL;
	}
}

void RHBcontext_string::build(RHBPreprocessor * /*pp*/)
{
}

void RHBinclude_file::build(RHBPreprocessor * /*pp*/)
{
}

RHBtype *RHBelement::wrap_with_pointer(RHBPreprocessor *pp,char *buf,size_t buflen,RHBtype *t)
{
	RHBpointer_type *t2;
	if (strcmp(buf,"*"))
	{
		pp->err("Should be a *",buf);

		return 0;
	}

	t2=new RHBpointer_type(pp,0);

	strncpy(t2->type,buf,sizeof(t2->type));

	buf[0]=0;

	t2->pointer_to=t;

	pp->get_token(buf,buflen);

	return t2;
}

RHBinclude_file *RHBelement::is_include_file()
{
	return 0;
}

RHBinclude_file *RHBinclude_file::is_include_file()
{
	return this;
}

void RHBattribute::generate_accessors(RHBPreprocessor *pp,RHBinterface *iface)
{
	RHBattribute_accessor *aa;
	char buf[256];

	if (!readonly)
	{
		RHBparameter *param;
		strncpy(buf,"_set_",sizeof(buf));
		strncat(buf,id,sizeof(buf)-1);
		aa=new RHBattribute_accessor(pp,buf);
		aa->for_attribute=this;
		iface->add(pp,aa);
		param=new RHBparameter(pp,id);
		param->parameter_type=attribute_type;
		strncpy(param->mode,"in",sizeof(param->mode));

		aa->add(pp,param);

		RHBelement *el=aa;

		while (el->parent())
		{
			el=el->parent();
		}

		el=el->children();

		while (el)
		{
			if (!strcmp(el->id,"void"))
			{
				break;
			}
			el=el->next();
		}

		aa->return_type=el->is_type();
	}
	strncpy(buf,"_get_",sizeof(buf));
	strncat(buf,id,sizeof(buf)-1);
	aa=new RHBattribute_accessor(pp,buf);
	aa->for_attribute=this;
	iface->add(pp,aa);
	aa->return_type=attribute_type;
}

RHBattribute_accessor::RHBattribute_accessor(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBattribute_accessor",pp,name),
  RHBtype(pp,name),*/
  RHBoperation(pp,name)
{
}

RHBvalue * RHBelement::is_value()
{
	return 0;
}

RHBvalue * RHBvalue::is_value()
{
	return this;
}

long RHBvalue::numeric_value()
{
	if (value_string) return atol(value_string);

	return 0;
}

RHBequation::RHBequation(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBequation",pp,name),
	RHBtype(pp,name),
	RHBvalue(pp,name),*/
	RHBconstant(pp,name)
{
	val1=0;
	val2=0;
	op[0]=0;
}

RHBvalue * RHBelement::read_number(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	RHBvalue *v=NULL;

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	if (0==strcmp(buf,"("))
	{
		buf[0]=0;

		v=read_number(pp,buf,buflen);

		if (strcmp(buf,")"))
		{
			pp->err("Should be closing bracket",buf);

			return 0;
		}

		buf[0]=0;

		return v;
	}

	if (isdigit(buf[0]))
	{
		v=new RHBconstant(pp,0);

		size_t x=strlen(buf)+1;
		v->value_string=new char [x];

		strncpy(v->value_string,buf,x);

		buf[0]=0;
	}
	else
	{
		if (buf[0]=='\'')
		{
			unsigned long l=0;
			char *p=buf+1;

			while ((*p) && (*p!='\''))
			{
				unsigned char c=*p++;
				l<<=8;
				l+=c;
			}

			v=new RHBconstant(pp,0);
			int max_len=12;
			v->value_string=new char[max_len];
			snprintf(v->value_string,max_len,"0x%08lX",(unsigned long)l);
			buf[0]=0;
		}
		else
		{
			if ((buf[0]=='-')||(buf[0]=='~'))
			{
				/* make a leading equation */
			}
			else
			{
				RHBconstant *cn=is_constant();
				RHBelement *el=NULL;

				if (cn)
				{
					RHBtype *ct=cn->constant_type;

					el=ct->find_named_element(pp,buf,buflen);
				}
				else
				{
					el=find_named_element(pp,buf,buflen);
				}
				if (!el)
				{	
					pp->err("No element near ",buf);
				}

				v=el->is_value();
			}
		}
	}

	do
	{
		if (!buf[0])
		{
			pp->get_token(buf,buflen);
		}

		if (strcmp(buf,"*"))
		{
			if (strcmp(buf,"-"))
			{
				if (strcmp(buf,"+"))
				{
					if (strcmp(buf,"/"))
					{
						if (strcmp(buf,"~"))
						{
							return v;
						}
					}
				}
			}
		}

		RHBequation *eq=new RHBequation(pp,0);

		strncpy(eq->op,buf,sizeof(eq->op));

		buf[0]=0;

		eq->val1=v;

		eq->val2=read_number(pp,buf,buflen);

		v=eq;

	} while (v);

	return v;
}

long RHBconstant::numeric_value()
{
	if (value_string)
	{
		if (value_string[0]=='0')
		{
			if (!value_string[1])
			{
				return 0;
			}
			if ((value_string[1]=='x')||(value_string[1]=='X'))
			{
				return read_hex_value(&value_string[2]);
			}
			return read_octal_value(&value_string[2]);
		}

		return atol(value_string);
	}

	if (const_val)
	{
		return const_val->numeric_value();
	}

	return 0;
}

long RHBequation::numeric_value()
{
	long v1=0,v2=0;

	if (val1) v1=val1->numeric_value();
	if (val2) v2=val2->numeric_value();

	if (!strcmp(op,"+"))
	{
		return v1+v2;
	}

	if (!strcmp(op,"*"))
	{
		return v1*v2;
	}

	if (!strcmp(op,"-"))
	{
		return v1-v2;
	}

	if (!strcmp(op,"/"))
	{
		return v1/v2;
	}

	return 0;
}

RHBmodifier * RHBmodifier::is_modifier()
{
	return this;
}

RHBmodifier * RHBelement::is_modifier()
{
	return 0;
}

RHBtype_def * RHBtype::is_typedef()
{
	return 0;
}

RHBenum * RHBtype::is_enum()
{
	return 0;
}

RHBstruct * RHBtype::is_struct()
{
	return 0;
}

RHBtype_def * RHBtype_def::is_typedef()
{
	return this;
}

RHBenum * RHBenum::is_enum()
{
	return this;
}

RHBstruct * RHBstruct::is_struct()
{
	return this;
}

RHBstruct_element *RHBstruct_element::is_struct_element()
{
	return this;
}

RHBstruct_element *RHBtype::is_struct_element()
{
	return 0;
}

RHBarray *RHBarray::is_array()
{
	return this;
}

RHBarray *RHBtype::is_array()
{
	return 0;
}
RHBpointer_type * RHBtype::is_pointer()
{
	return 0;
}

RHBpointer_type * RHBpointer_type::is_pointer()
{
	return this;
}

RHBstring_type *RHBtype::is_string()
{
	return 0;
}

RHBstring_type *RHBstring_type::is_string()
{
	return this;
}

RHBqualified_type *RHBtype::is_qualified()
{
	return 0;
}

RHBqualified_type *RHBqualified_type::is_qualified()
{
	return this;
}

RHBsequence_type *RHBtype::is_sequence()
{
	return 0;
}

RHBsequence_type *RHBsequence_type::is_sequence()
{
	return this;
}

RHBunion * RHBtype::is_union()
{
	return 0;
}

RHBunion_element * RHBtype::is_union_element()
{
	return 0;
}

RHBtype *RHBelement::read_union(RHBPreprocessor *pp,char *buf,size_t buflen)
{
	RHBunion *u;
	RHBelement *el;
	int count=0;

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	if (strcmp(buf,"union"))
	{
		pp->err("This is supposed to say union",buf);
		return 0;
	}

	pp->get_token(buf,buflen);

	el=find_named_element(pp,buf,buflen);

	if (el)
	{
		RHBtype *t;

		t=el->is_type();
		u=t->is_union();
	}
	else
	{
		u=new RHBunion(pp,buf);
		add(pp,u);
	}

	pp->get_token(buf,buflen);

	if (strcmp(buf,"switch"))
	{
		return u;
	}

	pp->get_token(buf,buflen);

	if (strcmp(buf,"("))
	{
		pp->err("This is supposed to open with bracket\n",buf);
		return 0;
	}

	pp->get_token(buf,buflen);

	u->switch_type=read_type(pp,buf,buflen);

	if (!buf[0])
	{
		pp->get_token(buf,buflen);
	}

	if (strcmp(buf,")"))
	{
		pp->err("This is supposed to close with bracket\n",buf);
		return 0;
	}

	pp->get_token(buf,buflen);

	if (strcmp(buf,"{"))
	{
		pp->err("This is supposed to open with braces\n",buf);
		return 0;
	}

	buf[0]=0;

	while (u)
	{
		RHBunion_element *uel;
		RHBvalue *val;
		RHBtype *tp;

		if (!buf[0])
		{
			if (!pp->get_token(buf,buflen))
			{
				return 0;
			}
		}

		if (0==strcmp(buf,"}"))
		{
			buf[0]=0;
			pp->get_token(buf,buflen);

			return u;
		}

		val=0;

		if (0==strcmp(buf,"default"))
		{
			u->default_index=count;
			buf[0]=0;
		}
		else
		{
			RHBelement *type=u->switch_type;

			if (strcmp(buf,"case"))
			{
				pp->err("Should say 'case'\n",buf);

				return 0;
			}

			buf[0]=0;

			pp->get_token(buf,buflen);

			val=type->read_number(pp,buf,buflen);
		}

		if (!buf[0])
		{
			pp->get_token(buf,buflen);
		}

		if (strcmp(buf,":"))
		{
			pp->err("Should say ':'\n",buf);

			return 0;
		}

		pp->get_token(buf,buflen);

		tp=read_type(pp,buf,buflen);

		if (!tp)
		{
			pp->err("Should have been a type\n",buf);

			return 0;
		}

		if (!buf[0])
		{
			pp->get_token(buf,buflen);
		}

		uel=new RHBunion_element(pp,buf);

		uel->switch_value=val;
		uel->element_type=tp;
			
		/* adding as a sequence, not as
			a named list,
			elements can have duplicate
			entries */
		u->elements.add(uel);
		count++;

		pp->get_token(buf,buflen);

		if (strcmp(buf,";"))
		{
			pp->err("Should have been a colon\n",buf);

			return 0;
		}

		buf[0]=0;
	}

	return u;
}

RHBunion * RHBunion::is_union()
{
	return this;
}

RHBunion_element * RHBunion_element::is_union_element()
{
	return this;
}

RHBoperation *RHBelement::is_operation()
{
	return 0;
}

RHBoperation *RHBoperation::is_operation()
{
	return this;
}

boolean RHBoperation::can_auto_stub()
{
	int has_floats=0;
	RHBelement *el=children();

	while (el)
	{
		RHBparameter *param=el->is_parameter();

		if (param)
		{
			if (RHBheader_emitter::is_in_floating(param))
			{
				has_floats++;
			}
		}
		el=el->next();
	}

	if (has_floats < 14)
	{
		return 1;
	}

	return 0;
}

RHBbase_type *RHBtype::is_base_type()
{
	return 0;
}

RHBbase_type *RHBbase_type::is_base_type()
{
	return this;
}

RHBparameter *RHBparameter::is_parameter()
{
	return this;
}

RHBparameter *RHBoperation::get_parameter(int i)
{
	RHBelement *el;
	el=children();

	while (el)
	{
		RHBparameter *param;

		param=el->is_parameter();

		if (param)
		{
			if (!i) return param;

			i--;
		}

		el=el->next();
	}

	return 0;
}


RHBany_type *RHBtype::is_any()
{
	return 0;
}

RHBTypeCode_type *RHBtype::is_TypeCode()
{
	return 0;
}

RHBTypeCode_type::RHBTypeCode_type(RHBPreprocessor *pp,
			const char *name,
			RHBtype *va_type,
			boolean in_by_ref)
: /*RHBelement("RHBTypeCode_type",pp,name),RHBtype(pp,name),*/RHBbase_type(pp,name,va_type,in_by_ref,0)
{
}

RHBany_type::RHBany_type(RHBPreprocessor *pp,
			const char *name,
			RHBtype *va_type,
			boolean in_by_ref)
: /*RHBelement("RHBany_type",pp,name),RHBtype(pp,name),*/RHBbase_type(pp,name,va_type,in_by_ref,0)
{
}

RHBany_type * RHBany_type::is_any()
{
	return this;
}

RHBTypeCode_type * RHBTypeCode_type::is_TypeCode()
{
	return this;
}

RHBrepository *RHBrepository::is_repository()
{
	return this;
}

RHBrepository *RHBelement::is_repository()
{
	return 0;
}

void RHBinterface::list_all_operations(RHBelement_sequence *seq)
{
	size_t i=0;

	while (i < _parents.length())
	{
		RHBinterface *iface;

		iface=_parents.get(i)->is_interface();

		iface->list_all_operations(seq);

		i++;
	}

	RHBelement *el=children();

	while (el)
	{
		RHBoperation *op;

		op=el->is_operation();

		if (op)
		{
			if (!seq->contains(op))
			{
				seq->add(op);
			}
		}

		el=el->next();
	}
}

void RHBelement::_set_prefix(const char *buf)
{
	char b[256];
	char *q=b;

	if (buf)
	{
		const char *p;
		p=buf;
		while (*p)
		{
			if (*p != 0x22)
			{
				*q=*p;
				q++;
			}

			p++;
		}
	}

	*q=0;

	if (prefix) delete prefix;

	prefix=0;

	if (b[0])
	{
		size_t x=strlen(b)+1;
		prefix=new char[x];
		strncpy(prefix,b,x);
	}
}

RHBelement_new_scope::RHBelement_new_scope(RHBPreprocessor *p,RHBelement *e)
{
	pp=p;
	old=p->scope;
	p->scope=e;
}

RHBelement_new_scope::~RHBelement_new_scope()
{
	pp->scope=old;
}

void RHBelement::_set_global_id(const char *buf)
{
	char b[256];
	char *q=b;

	if (buf)
	{
		const char *p=buf;

		while (*p)
		{
			if (*p != 0x22)
			{
				*q=*p;
				q++;
			}

			p++;
		}
	}

	*q=0;

	if (global_id) delete global_id;
	global_id=0;

	if (b[0])
	{
		size_t x=strlen(b)+1;
		global_id=new char[x];
		strncpy(global_id,b,x);
	}
}

void RHBelement::_set_version(const char *buf)
{
	char b[256];
	char *q=b;

	if (buf)
	{
		const char *p=buf;

		while (*p)
		{
			if (*p != 0x22)
			{
				*q=*p;
				q++;
			}

			p++;
		}
	}

	*q=0;

	if (version) delete version;
	version=0;

	if (b[0])
	{
		size_t x=strlen(b)+1;
		version=new char[x];
		strncpy(version,b,x);
	}

	gen_global_id();
}

void RHBelement::gen_global_id()
{
	unsigned int i;
	RHBelement *a[256];
	RHBelement *te;
	const char *p;
	char buf[256];

	buf[0]=0;

	if (!is_module())
	{
		if (!is_interface())
		{
			if (!is_operation())
			{
				if (!is_exception())
				{
					RHBtype *t=is_type();
		
					if (!t)
					{
						return;
					}

					if (!t->is_attribute())
					{
						if (!t->is_struct())
						{
							if (!t->is_typedef())
							{
								if (!t->is_union())
								{
									if (!t->is_enum())
									{
										return;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	i=0;

	te=this;

	while (te)
	{
		if ((te->id)||(te->prefix))
		{
			a[i]=te;
			i++;
		}

		if (te->prefix)
		{
			te=0;
		}
		else
		{
			te=te->parent();
		}
	}

	if (!i) return;

	strncpy(buf,"IDL:",sizeof(buf));

	p=0;

	while (i--)
	{
		te=a[i];

		if (te->prefix)
		{
			if (p) strncat(buf,"/",sizeof(buf)-1);
			p=te->prefix;
			strncat(buf,p,sizeof(buf)-1);
		}
		else
		{
			if (te->id)
			{
				if (p) strncat(buf,"/",sizeof(buf)-1);
				p=te->id;
				strncat(buf,p,sizeof(buf)-1);
			}
		}
	}

	p=version;

	if (!p) 
	{
		p="1.0";
	}

	strncat(buf,":",sizeof(buf)-1);
	strncat(buf,p,sizeof(buf)-1);

/*	printf("%s\n",buf);*/

	_set_global_id(buf);
}

void RHBelement::set_pragma_modifier(
		RHBPreprocessor * /*pp*/,
		const char * /*name*/,
		const char * /*value*/)
{
/*	if (value)
	{
		if (*value)
		{
			RHBelement *mod;

			mod=new RHBmodifier(pp,name);
			add(pp,mod);
			mod->add_modifier_value(value);
		}
	}
*/
}

RHBcontext_string *RHBcontext_string::is_context()
{
	return this;
}

RHBcontext_string *RHBelement::is_context()
{
	return 0;
}

void RHBelement::remove_quotes(char *buf)
{
	char tmp[256];
	char *p;
	strncpy(tmp,buf,sizeof(tmp));
	p=tmp;
	while (*p)
	{
		if ((*p!=double_quote)&&(*p!=single_quote))
		{
			*buf++=*p;
		}
		p++;
	}
	*buf=0;
}

boolean RHBinterface::is_subclass_of(RHBinterface *other)
{
	if (other)
	{
		if (this==other) return 1;

		long i=_parents.length();

		while (i--)
		{
			if (_parents.get(i)->is_interface()->is_subclass_of(other))
			{
				return 1;
			}
		}
	}

	return 0;
}

const char *RHBelement::get_modifier_string(const char *name)
{
	RHBelement *el;
	char buf[256];

	strncpy(buf,name,sizeof(buf));

	el=find_named_element(0,buf,sizeof(buf));

	if (el)
	{
		RHBmodifier *mod=el->is_modifier();
		if (mod)
		{
			return mod->modifier_data.get(0);
		}
	}

	return 0;
}

RHBenum *RHBconstant::is_const_enum()
{
	if (constant_type)
	{
		return constant_type->is_enum();
	}

	return NULL;
}

RHBinherited_operation *RHBelement::is_inherited_operation()
{
	return NULL;
}

void RHBinterface::inherit_operations(RHBPreprocessor *pp,RHBinterface *iface)
{
	if (iface)
	{
		RHBelement *el=iface->children();

		while (el)
		{
			RHBoperation *op=el->is_operation();

			if (op)
			{
				RHBelement *e=has_named_element(op->id);

				if (!e)
				{
					RHBinherited_operation *iop=new RHBinherited_operation(pp,op->id);

					iop->original=op;

					add(pp,iop);
				}
			}

			el=el->next();
		}

		size_t k=0;

		while (k < iface->_parents.length())
		{
			inherit_operations(pp,iface->_parents.get(k)->is_interface());

			k++;
		}
	}
}

RHBinherited_operation::RHBinherited_operation(RHBPreprocessor *pp,const char *name)
: /*RHBelement("RHBmodifier",pp,name),*/RHBtype(pp,name)
{
}

void RHBinherited_operation::build(RHBPreprocessor *  /*pp*/)
{
}

RHBinherited_operation *RHBinherited_operation::is_inherited_operation()
{
	return this;
}

RHBelement *RHBelement::has_named_element(const char *p)
{
	RHBelement *el=children();

	while (el)
	{
		if (!strcmp(el->id,p)) break;

		el=el->next();
	}

	return el;
}

RHBinclude_list::RHBinclude_list(RHBPreprocessor *pp,const char *name)
: RHBelement("RHBinclude_list",pp,name)
{
}

void RHBinclude_list::build(RHBPreprocessor * /*pp*/)
{
}

RHBinclude_file * RHBinclude_list::included(RHBPreprocessor *pp,const char *b,RHBinclude_file *current)
{
	RHBelement *e=_children;

	if (b)
	{
		const char *p=b+strlen(b);

		while (p > b)
		{
			char c=*--p;

			if ((c==':')||(c=='/')||(c=='\\'))
			{
				b=p+1;
				break;
			}
		}
	}

	if (pp->scope)
	{
		pp->scope->_set_prefix(0);
	}

	while (e)
	{
		if (e->id)
		{
			if (0==strcmp(e->id,b))
			{
				return e->is_include_file();
			}
		}
		else
		{
			if (!b) 
			{
				return e->is_include_file();
			}

			if (!b[0]) 
			{
				return e->is_include_file();
			}
		}

		e=e->next();
	}

	RHBinclude_file *f=new RHBinclude_file(pp,b);

	if (!f->defined_in)
	{
		f->defined_in=current;
	}

/*	p=f->id;*/

	add_nocheck(pp,f);

	return f;
}
