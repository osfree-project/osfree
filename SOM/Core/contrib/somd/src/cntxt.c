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

#define Context_Class_Source

#include <rhbopt.h>
#include <rhbsomd.h>

#ifndef SOM_Module_cntxt_Source
#define SOM_Module_cntxt_Source
#endif

#include "cntxt.ih"


static boolean prop_name_match(char *one,char *two)
{
	while ((*one)==(*two))
	{
		if (!*one) 
		{
			return 1;
		}

		one++;
		two++;
	}

	if (*two=='*')
	{
		return 1;
	}

	if ((!(*one)) && (!(*two)))
	{
		return 1;
	}

	return 0;
}

SOM_Scope ORBStatus  SOMLINK cntxt_set_one_value(Context SOMSTAR somSelf, 
                                                  Environment *ev, 
                                                 Identifier prop_name, 
                                                 corbastring value)
{
    ContextData *somThis = ContextGetData(somSelf);
	unsigned int i=0;

	RHBOPT_unused(ev)

	while (i < somThis->data.items._length)
	{
		if (prop_name_match(somThis->data.items._buffer[i].name,prop_name))
		{
			if (somThis->data.items._buffer[i].value)
			{
				SOMFree(somThis->data.items._buffer[i].value);
			}
			somThis->data.items._buffer[i].value=somd_dupl_string(value);
			return 0;
		}

		i++;
	}
	
	if (somThis->data.items._length >= somThis->data.items._maximum)
	{
		Context_ContextItem *n;

		somThis->data.items._maximum+=4;

		n=SOMMalloc(sizeof(*n)*somThis->data.items._maximum);

		i=somThis->data.items._length;

		while (i--)
		{
			n[i].name=somThis->data.items._buffer[i].name;
			n[i].value=somThis->data.items._buffer[i].value;
		}

		if (somThis->data.items._buffer)
		{
			SOMFree(somThis->data.items._buffer);
		}

		somThis->data.items._buffer=n;
	}

	somThis->data.items._buffer[somThis->data.items._length].name=
			somd_dupl_string(prop_name);
	somThis->data.items._buffer[somThis->data.items._length].value=
			somd_dupl_string(value);

	somThis->data.items._length++;
    return 0;
}

SOM_Scope ORBStatus  SOMLINK cntxt_set_values(
		Context SOMSTAR somSelf, 
		Environment *ev, 
		NVList SOMSTAR values)
{
	long i=0;
	long count=0;

	NVList_get_count(values,ev,&count);

	while (count--)
	{
		TypeCode tc=NULL;
		corbastring *arg=NULL;
		char *name=NULL;
		Flags flags=0;
		long len=0;

		if (!NVList_get_item(
					values,
					ev,
					i,
					&name,
					&tc,
					(void **)(void *)&arg,
					&len,
					&flags))
		{
			if (tcEqual(TC_string,ev,tc))
			{
				cntxt_set_one_value(somSelf,ev,name,*arg);
			}
		}

		i++;
	}

    return 0;
}

SOM_Scope ORBStatus  SOMLINK cntxt_get_values(Context SOMSTAR somSelf, 
                                               Environment *ev, 
                                              Identifier start_scope, 
                                              Flags op_flags, 
                                              Identifier prop_name, 
                                              NVList SOMSTAR* values)
{
    ContextData *somThis = ContextGetData(somSelf);
	unsigned int i=0;
	NVList SOMSTAR list=NULL;

	*values=list;

	while (i < somThis->data.items._length)
	{
		if (prop_name_match(somThis->data.items._buffer[i].name,prop_name))
		{
			if (!list)
			{
#ifdef somNewObject
				list=somNewObject(NVList);
#else
				list=NVListNew();
#endif
			}

			NVList_add_item(list,ev,
					somThis->data.items._buffer[i].name,
					TC_string,
					&somThis->data.items._buffer[i].value,
					0,
					IN_COPY_VALUE);
		}

		i++;
	}

	*values=list;

	if (!(op_flags & CTX_RESTRICT_SCOPE))
	{
		if (somThis->context_parent)
		{
			return Context_get_values(somThis->context_parent,ev,start_scope,op_flags,prop_name,values);
		}
	}

    return 0;
}

SOM_Scope ORBStatus  SOMLINK cntxt_delete_values(Context SOMSTAR somSelf, 
                                                  Environment *ev, 
                                                 Identifier prop_name)
{
	ContextData *somThis = ContextGetData(somSelf);
	unsigned int i;

	RHBOPT_unused(ev)

	i=somThis->data.items._length;

	while (i--)
	{
		if (i < somThis->data.items._length)
		{
			if (prop_name_match(somThis->data.items._buffer[i].name,prop_name))
			{
				unsigned int j;

				/* delete this one */

				if (somThis->data.items._buffer[i].name)
				{
					SOMFree(somThis->data.items._buffer[i].name);
				}

				if (somThis->data.items._buffer[i].value)
				{
					SOMFree(somThis->data.items._buffer[i].value);
				}

				j=i;

				somThis->data.items._length--;

				while (j < somThis->data.items._length)
				{
					somThis->data.items._buffer[j].name=
						somThis->data.items._buffer[j+1].name;
					somThis->data.items._buffer[j].value=
						somThis->data.items._buffer[j+1].value;
					j++;
				}

				somThis->data.items._buffer[j].name=0;
				somThis->data.items._buffer[j].value=0;
			}
		}
	}

	if (!somThis->data.items._length)
	{
		if (somThis->data.items._maximum)
		{
			SOMFree(somThis->data.items._buffer);
			somThis->data.items._buffer=0;
			somThis->data.items._maximum=0;
		}
	}

    return 0;
}

SOM_Scope ORBStatus  SOMLINK cntxt_create_child(
		Context SOMSTAR somSelf, 
		Environment *ev, 
		Identifier ctx_name, 
		Context SOMSTAR * child_ctx)
{
	Context SOMSTAR ch;
	SOMClass SOMSTAR me;

	RHBOPT_unused(ev)

	me=SOMObject_somGetClass(somSelf);

	ch=SOMClass_somNew(me);

	Context__set_context_name(ch,ev,ctx_name);

	somReleaseClassReference(me);

	Context__set_context_parent(ch,ev,somSelf);

	*child_ctx=ch;

    return 0;
}

SOM_Scope ORBStatus  SOMLINK cntxt_destroy(
		Context SOMSTAR somSelf, 
		Environment *ev, 
		Flags flags)
{
    ContextData *somThis = ContextGetData(somSelf);
    
	if (!(flags & CTX_DELETE_DESCENDENTS))
	{
		if (somThis->data.children._length)
		{
			RHBOPT_throw_StExcep(ev,BAD_CONTEXT,BadContext,NO);

			return (ORBStatus)SOMDERROR_BadContext;
		}
	}

	Context_somRelease(somSelf);

    return 0;
}


static void remove_child(_IDL_SEQUENCE_Context *seq,Context SOMSTAR o)
{
	unsigned int i;

	i=0;

	while (i < seq->_length)
	{
		if (seq->_buffer[i]==o)
		{
			while ((i+1) < seq->_length)
			{
				seq->_buffer[i]=
					seq->_buffer[i+1];
			}

			seq->_length--;
		}

		i++;
	}

	if (!seq->_length)
	{
		if (seq->_maximum)
		{
			SOMFree(seq->_buffer);
			seq->_buffer=0;
			seq->_maximum=0;
		}
	}
}

#ifdef Context_somDestruct
SOM_Scope void SOMLINK cntxt_somDestruct(
	Context SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	Context SOMSTAR parent;
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	ContextData *somThis;

	Context_BeginDestructor
#else
SOM_Scope void SOMLINK cntxt_somUninit(Context SOMSTAR somSelf)
{
	ContextData *somThis=ContextGetData(somSelf);
	Context SOMSTAR parent;
#endif

	parent=somThis->context_parent;
	somThis->context_parent=0;

	if (parent)
	{
		ContextData *somThat=ContextGetData(parent);

		remove_child(&somThat->data.children,somSelf);

		Context_somRelease(parent);
	}

	if (somThis->data.items._maximum)
	{
		unsigned long i=somThis->data.items._length;

		while (i--)
		{
			if (somThis->data.items._buffer[i].name)
			{
				SOMFree(somThis->data.items._buffer[i].name);
				somThis->data.items._buffer[i].name=0;
			}
			if (somThis->data.items._buffer[i].value)
			{
				SOMFree(somThis->data.items._buffer[i].value);
				somThis->data.items._buffer[i].value=0;
			}
		}

		SOMFree(somThis->data.items._buffer);
		somThis->data.items._buffer=NULL;
	}

	if (somThis->data.children._maximum)
	{
		unsigned long i=somThis->data.children._length;
		while (i--)
		{
			if (i < somThis->data.children._length)
			{
				Context SOMSTAR ch;

				ch=somThis->data.children._buffer[i];
				
				remove_child(&somThis->data.children,ch);

				Context_somRelease(ch);
			}

			i--;
		}
	}

	if (somThis->context_name)
	{
		SOMFree(somThis->context_name);
		somThis->context_name=0;
	}

#ifdef Context_somDestruct
	Context_EndDestructor
#else
#	ifdef Context_parent_SOMRefObject_somUninit
		Context_parent_SOMRefObject_somUninit(somSelf);
#	else
		Context_parent_SOMObject_somUninit(somSelf);
#	endif
#endif
}

static void cntxt_init(ContextData *somThis)
{
	somThis->context_parent=NULL;
	somThis->data.items._maximum=0;
	somThis->data.items._length=0;
	somThis->data.items._buffer=NULL;
	somThis->context_name=NULL;
}

#ifdef Context_somDefaultInit
SOM_Scope void SOMLINK cntxt_somDefaultInit(
	Context SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	ContextData *somThis;

	Context_BeginInitializer_somDefaultInit
	Context_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);

	cntxt_init(somThis);
}
#else
SOM_Scope void SOMLINK cntxt_somInit(Context SOMSTAR somSelf)
{
	ContextData *somThis=ContextGetData(somSelf);

	cntxt_init(somThis);

#ifdef Context_parent_SOMRefObject_somInit
	Context_parent_SOMRefObject_somInit(somSelf);
#else
	Context_parent_SOMObject_somInit(somSelf);
#endif
}
#endif

SOM_Scope corbastring SOMLINK cntxt__get_context_name(
	Context SOMSTAR somSelf,
	Environment *ev)
{
	ContextData *somThis=ContextGetData(somSelf);

	RHBOPT_unused(ev)

	return somd_dupl_string(somThis->context_name);
}

SOM_Scope void SOMLINK cntxt__set_context_name(
	Context SOMSTAR somSelf,
	Environment *ev,
	corbastring context_name)
{
	ContextData *somThis=ContextGetData(somSelf);
	RHBOPT_unused(ev)

	if (somThis->context_name) SOMFree(somThis->context_name);

	somThis->context_name=somd_dupl_string(context_name);
}

static void dump_string(char *p)
{
	if (p)
	{
		char c='\"';
		dump_somPrintf(("%c%s%c",c,p,c));
	}
}

SOM_Scope void SOMLINK cntxt_somDumpSelfInt(Context SOMSTAR somSelf,long depth)
{
	unsigned int i=0;
	ContextData *somThis=ContextGetData(somSelf);

	while (i < somThis->data.items._length)
	{
		char *name=somThis->data.items._buffer[i].name;
		char *value=somThis->data.items._buffer[i].value;

		somPrefixLevel(depth);
		dump_somPrintf(("[%d] : %s=",i,name));
		dump_string(value);
		dump_somPrintf(("\n",i,name));
		i++;
	}
}

SOM_Scope void SOMLINK cntxt__set_context_parent(Context SOMSTAR somSelf,Environment *ev,Context SOMSTAR context_parent)
{
	ContextData *somThis=ContextGetData(somSelf);
	
	RHBOPT_unused(ev)

	if (somThis->context_parent) Context_somFree(somThis->context_parent);

	somThis->context_parent=context_parent;

	if (context_parent)
	{
		somThis->context_parent=Context_somDuplicateReference(context_parent);
	}
}

SOM_Scope Context SOMSTAR SOMLINK cntxt__get_context_parent(Context SOMSTAR somSelf,Environment *ev)
{
	ContextData *somThis=ContextGetData(somSelf);

	RHBOPT_unused(ev)

	if (somThis->context_parent) 
	{
		return Context_somDuplicateReference(somThis->context_parent);
	}

	return NULL;
}

