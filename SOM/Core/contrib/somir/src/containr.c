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

#define Container_Class_Source
#include <rhbsomir.h>
#include <containr.ih>

SOM_Scope void SOMLINK containr_somDumpSelf(
	Container SOMSTAR somSelf,
	/* in */ long level)
{
#ifdef Container_parent_SOMRefObject_somDumpSelf
	Container_parent_SOMRefObject_somDumpSelf(somSelf,level);
#else
	Container_parent_SOMObject_somDumpSelf(somSelf,level);
#endif
}

SOM_Scope void SOMLINK containr_somDumpSelfInt(
	Container SOMSTAR somSelf,
	/* in */ long level)
{
/*	Container_parent_SOMRefObject_somDumpSelfInt(somSelf,level);*/
	_IDL_SEQUENCE_Contained fContents;
	Environment ev;
/*	ContainerData *somThis=
			ContainerGetData(somSelf);
*/
	SOM_InitEnvironment(&ev);

	fContents=Container_contents(somSelf,&ev,"all",1);

	if (fContents._length)
	{
		unsigned long m=0;
		char *p="";

		somPrefixLevel(level);

		if (fContents._length > 1) p="s";

		somPrintf("contains %ld item%s:\n",fContents._length,p);

		while (m < fContents._length)
		{
			somPrintf("\n");

			SOMObject_somDumpSelf(
					fContents._buffer[m],
					level+1);

			Contained_somRelease(fContents._buffer[m]);

			m++;
		}
	}

	if (fContents._buffer) SOMFree(fContents._buffer);

	SOM_UninitEnvironment(&ev);
}

struct containr_enum
{
	Container SOMSTAR somSelf;
	Environment *ev;
	struct SOMIR_ContainedData *cnd;
	struct SOMIR_ContainerData *cnr;
	struct SOMIR_Repository *rep;
	void (*cleanup)(struct containr_enum *);
	void *refcon;
	char *search_name;
	boolean cancelled;
	short limit_type;
	boolean exclude_inherited;
};

RHBOPT_cleanup_begin(containr_enum_cleanup,pv)

struct containr_enum *data=pv;

	if (data->cnd) data->cnd->lpVtbl->Release(data->cnd);

	SOMIR_UNLOCK

RHBOPT_cleanup_end

static void containr_enum(Container SOMSTAR somSelf,
						  Environment *ev,
						  char *search_name,
						  Container_InterfaceName limit_type,
						  boolean exclude_inherited,
						  boolean (*callback_fn)(struct containr_enum *,
							  struct SOMIR_ContainerItem *),
						  void *refcon,
						  void (*preflight)(struct containr_enum *),
						  void (*cleanup)(struct containr_enum *))
{
	struct containr_enum data={NULL,NULL,NULL,
							   NULL,NULL,NULL,
							   NULL,
							   NULL,
							   1,0,0};

	data.somSelf=somSelf;
	data.ev=ev;
	data.exclude_inherited=exclude_inherited;
	data.cleanup=cleanup;
	data.refcon=refcon;
	data.search_name=search_name;

	SOMIR_LOCK

	RHBOPT_cleanup_push(containr_enum_cleanup,&data);

	data.cnd=Contained__get_containedData(somSelf,ev);

	if (data.cnd)
	{
		data.rep=data.cnd->defined_in->file->repository;
		data.cnr=data.cnd->lpVtbl->IsContainer(data.cnd);
		data.limit_type=data.rep->lpVtbl->TypeFromName(limit_type);

		preflight(&data);

		if (data.cnr)
		{
			unsigned long i=data.cnr->itemSeq._length;
			struct SOMIR_ContainerItem *item=data.cnr->itemSeq._buffer;

			/* loop round and acquire each item */

			while (i--)
			{
				if (
					(
						(data.limit_type==kSOMIR_Empty)
						||
						(data.limit_type==item->type)
					)
					&&
					(
						(!search_name)
						||
						(!strcmp(search_name,item->name))
					)
				)
				{
					if (!callback_fn(&data,item)) break;
				}

				item++;
			}
		}
	}

	data.cancelled=0;

	RHBOPT_cleanup_pop();
}

static void containr_contents_startup(struct containr_enum *data)
{
	unsigned long matches=0;
	unsigned long i=data->cnr->itemSeq._length;
	struct SOMIR_ContainerItem *item=data->cnr->itemSeq._buffer;

	while (i--)
	{
		if ((item->type==data->limit_type)||(data->limit_type==kSOMIR_Empty))
		{
			matches++;
		}

		item++;
	}

	if (matches)
	{
		_IDL_SEQUENCE_Contained *seq=data->refcon;
		seq->_buffer=SOMCalloc(matches,sizeof(seq->_buffer[0]));

		if (seq->_buffer)
		{
			seq->_maximum=matches;
		}
	}
}

static boolean containr_contents_enum(struct containr_enum *data,
								   struct SOMIR_ContainerItem *item)
{
	_IDL_SEQUENCE_Contained *seq=data->refcon;
	struct SOMIR_ContainedData *cobj=data->cnr->lpVtbl->Acquire(
							data->cnr,item->offset,item->type);

	RHBOPT_ASSERT(cobj);

	if (cobj)
	{
		/* wrap */
		Contained SOMSTAR obj=data->rep->lpVtbl->Wrap(cobj,data->ev);

		RHBOPT_ASSERT(obj);

		/* add to sequence */

		if (!Container_seqContained_add(data->somSelf,data->ev,seq,obj))
		{
			Contained_somFree(obj);
		}

		cobj->lpVtbl->Release(cobj);
	}

	return 1;
}

static void containr_contents_cleanup(struct containr_enum *ce)
{
}

SOM_Scope _IDL_SEQUENCE_Contained SOMLINK containr_contents(
	Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ Container_InterfaceName limit_type,
	/* in */ boolean exclude_inherited)
{
	_IDL_SEQUENCE_Contained seq={0,0,NULL};

	if (ev->_major==NO_EXCEPTION)
	{
		containr_enum(somSelf,
				  ev,
				  NULL,
				  limit_type,
				  exclude_inherited,
				  containr_contents_enum,
				  &seq,
				  containr_contents_startup,
				  containr_contents_cleanup);
	}

	return seq;
}

static void containr_describe_contents_startup(
			struct containr_enum *data)
{
}

static boolean containr_describe_contents_enum(
			struct containr_enum *data,
			struct SOMIR_ContainerItem *item)
{
	_IDL_SEQUENCE_Contained *seq=data->refcon;
	struct SOMIR_ContainedData *cobj=data->cnr->lpVtbl->Acquire(
							data->cnr,item->offset,item->type);

	RHBOPT_ASSERT(cobj);

	if (cobj)
	{
		/* wrap */
		Contained SOMSTAR obj=data->rep->lpVtbl->Wrap(cobj,data->ev);

		RHBOPT_ASSERT(obj);

		/* add to sequence */

		if (!Container_seqContained_add(data->somSelf,data->ev,seq,obj))
		{
			Contained_somFree(obj);
		}

		cobj->lpVtbl->Release(cobj);
	}

	return 1;
}

static void containr_describe_contents_cleanup(
			struct containr_enum *data)
{
}

SOM_Scope _IDL_SEQUENCE_Container_ContainerDescription SOMLINK containr_describe_contents(
	Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ Container_InterfaceName limit_type,
	/* in */ boolean exclude_inherited,
	/* in */ long max_returned_objs)
{
	_IDL_SEQUENCE_Container_ContainerDescription ret={0,0,NULL};

	if (ev->_major==NO_EXCEPTION)
	{
		containr_enum(somSelf,
				  ev,
				  NULL,
				  limit_type,
				  exclude_inherited,
				  containr_describe_contents_enum,
				  &ret,
				  containr_describe_contents_startup,
				  containr_describe_contents_cleanup);
	}	

	return ret;
}

static void containr_lookup_name_startup(
			struct containr_enum *data)
{
}

static boolean containr_lookup_name_enum(
			struct containr_enum *data,
			struct SOMIR_ContainerItem *item)
{
	_IDL_SEQUENCE_Contained *seq=data->refcon;
	struct SOMIR_ContainedData *cobj=data->cnr->lpVtbl->Acquire(
							data->cnr,item->offset,item->type);

	RHBOPT_ASSERT(cobj);

	if (cobj)
	{
		/* wrap */
		Contained SOMSTAR obj=data->rep->lpVtbl->Wrap(cobj,data->ev);

		RHBOPT_ASSERT(obj);

		/* add to sequence */

		if (!Container_seqContained_add(data->somSelf,data->ev,seq,obj))
		{
			Contained_somFree(obj);
		}

		cobj->lpVtbl->Release(cobj);
	}

	return 1;
}

static void containr_lookup_name_cleanup(
			struct containr_enum *data)
{
}

SOM_Scope _IDL_SEQUENCE_Contained SOMLINK containr_lookup_name(
	Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ Identifier search_name,
	/* in */ long levels_to_search,
	/* in */ Container_InterfaceName limit_type,
	/* in */ boolean exclude_inherited)
{
	_IDL_SEQUENCE_Contained ret={0,0,NULL};

	if (ev->_major==NO_EXCEPTION)
	{
		containr_enum(somSelf,
				  ev,
				  search_name,
				  limit_type,
				  exclude_inherited,
				  containr_lookup_name_enum,
				  &ret,
				  containr_lookup_name_startup,
				  containr_lookup_name_cleanup);
	}	

	return ret;
}

SOM_Scope void SOMLINK containr_somInit(
	Container SOMSTAR somSelf)
{
/*	ContainerData *somThis=ContainerGetData(somSelf);*/

#ifdef Container_parent_SOMRefObject_somInit
	Container_parent_SOMRefObject_somInit(somSelf);
#else
	Container_parent_SOMObject_somInit(somSelf);
#endif
}

SOM_Scope void SOMLINK containr_somUninit(
	Container SOMSTAR somSelf)
{
/*	ContainerData *somThis=ContainerGetData(somSelf);*/

#ifdef Container_parent_SOMRefObject_somUninit
	Container_parent_SOMRefObject_somUninit(somSelf);
#else
	Container_parent_SOMObject_somUninit(somSelf);
#endif
}

SOM_Scope void SOMLINK containr_somDestruct(
	Container SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
/*	ContainerData *somThis;*/
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	Container_BeginDestructor

	containr_somUninit(somSelf);

	Container_EndDestructor
}

SOM_Scope boolean SOMLINK containr_seqContained_add(
		Container SOMSTAR somSelf,
		Environment *ev,
		_IDL_SEQUENCE_Contained *seq,
		Contained SOMSTAR obj)
{
	boolean retVal=0;

	RHBOPT_ASSERT(seq);
	RHBOPT_ASSERT(obj);
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);

	if (seq && obj && (ev->_major==NO_EXCEPTION))
	{
		unsigned long i=seq->_length;

		while (i--)
		{
			if (seq->_buffer[i]==obj)
			{
				obj=NULL;
			}
		}

		if (obj)
		{
			if (seq->_maximum == seq->_length)
			{
				unsigned int i=seq->_length;
				Contained SOMSTAR *_buffer=SOMMalloc(sizeof(obj)*(seq->_maximum+=4));

				while (i--)
				{
					_buffer[i]=seq->_buffer[i];
				}

				if (seq->_buffer) SOMFree(seq->_buffer);
				seq->_buffer=_buffer;
			}

			seq->_buffer[seq->_length]=obj;
			seq->_length++;

			retVal=1;
		}
	}

	return retVal;
}

SOM_Scope void SOMLINK containr_seqContained_free(
		Container SOMSTAR somSelf,
		Environment *ev,
		_IDL_SEQUENCE_Contained *seq)
{
	unsigned long i=seq->_length;
	Contained SOMSTAR *_buffer=seq->_buffer;
	seq->_buffer=NULL;
	seq->_length=0;
	seq->_maximum=0;

	while (i--)
	{
		Contained SOMSTAR cnd=_buffer[i];
		if (cnd) Contained_somFree(cnd);
	}
	if (_buffer)
	{
		SOMFree(_buffer);
	}
}

static void SOMLINK containr_clsInit(SOMClass SOMSTAR somSelf)
{
	SOMIR_DLL_INIT
}

