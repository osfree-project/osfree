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
#include <rhbsomd.h>

/* This contains 'Object' and 'InterfaceDef' */

static void rhbObject_somUninit(RHBObject *somThis)
{
	if (somThis->object_key._buffer) 
	{
		SOMFree(somThis->object_key._buffer);
		somThis->object_key._buffer=0;
	}

	if (somThis->impl) 
	{
		RHBImplementationDef_Release(somThis->impl);
		somThis->impl=0;
	}

	if (somThis->iface) 
	{
		RHBInterfaceDef_Release(somThis->iface);
		somThis->iface=0;
	}
}

/*
SOMEXTERN void RHBObject_delete(RHBObject *somThis)
{
	if (somThis->object_key._buffer) 
	{
		SOMFree(somThis->object_key._buffer);
		somThis->object_key._buffer=0;
	}

	if (somThis->impl) 
	{
		RHBImplementationDef_Release(somThis->impl);
		somThis->impl=0;
	}

	if (somThis->iface) 
	{
		RHBInterfaceDef_Release(somThis->iface);
		somThis->iface=0;
	}

	SOMFree(somThis);
}
*/

static ORBStatus rhbObject_create_request(
					RHBObject *somThis,
					SOMDObject SOMSTAR somobj,
					Environment *ev,
					_IDL_SEQUENCE_string *ctx,
					Identifier operation,
					NVList SOMSTAR arg_list,
					NamedValue *result,
					RHBRequest **request,
					Flags flags)
{
	*request=RHBRequestNew(ev,somThis,somobj,
						ctx,operation,arg_list,result,flags);
	return 0;
}

static void rhbInterfaceDef_AddRef(RHBInterfaceDef *somThis)
{
	somd_atomic_inc(&somThis->lUsage);
}

static void rhbInterfaceDef_delete(RHBInterfaceDef *somThis)
{
	RHBDebug_deleted(RHBInterfaceDef,somThis)

	SOMFree(somThis);
}

static void rhbInterfaceDef_Close(
			RHBInterfaceDef *somThis,
			boolean orb_locked)
{
	if (somThis->lUsage)
	{
		return;
	}

	if (somThis->orb)
	{
		RHBORB *orb=somThis->orb;
		somThis->orb=NULL;
		if (orb)
		{
			RHBORB_remove_interface(orb,somThis,orb_locked);
			RHBORB_Release(orb);
		}
	}

	if (somThis->absolute_name) 
	{
		SOMFree(somThis->absolute_name);
		somThis->absolute_name=NULL;
	}

	if (!somThis->lUsage) 
	{
		rhbInterfaceDef_delete(somThis);
	}
}

static void rhbInterfaceDef_Release(RHBInterfaceDef *somThis)
{
	RHBORB_guard(somThis->orb)

	if (!somd_atomic_dec(&somThis->lUsage))
	{
		rhbInterfaceDef_Close(somThis,1);
	}

	RHBORB_unguard(somThis->orb)

}

static SOMClass SOMSTAR rhbInterfaceDef_get_proxy_class(
				RHBInterfaceDef *somThis,
				Environment *ev)
{
	if (!ev->_major)
	{
		char *p=RHBInterfaceDef_get_absolute_name(somThis,ev);

		if (p && !ev->_major)
		{
			return RHBORB_get_proxy_class(somThis->orb,ev,p);
		}
	}

	return NULL;
}

#if 0
struct rhbInterfaceDef_get_absolute_name
{
	char *p;
	Contained SOMSTAR c;
};

RHBOPT_cleanup_begin(rhbInterfaceDef_get_absolute_name_cleanup,pv)

	struct rhbInterfaceDef_get_absolute_name *data=pv;

	if (data->p) SOMFree(data->p);
	if (data->c) somReleaseObjectReference(data->c);

RHBOPT_cleanup_end
#endif

static char * rhbInterfaceDef_get_absolute_name(RHBInterfaceDef *somThis,Environment *ev)
{
#if 0
	struct rhbInterfaceDef_get_absolute_name data={NULL,NULL};
#endif

	if (somThis->id[0]==':')
	{
		if (somThis->id[1]==':')
		{
			return somThis->id;
		}
	}

#if 0
	RHBOPT_cleanup_push(rhbInterfaceDef_get_absolute_name_cleanup,&data);
#endif

	RHBORB_guard(somThis->orb)

	if (!somThis->absolute_name)
	{
		somThis->absolute_name=RHBORB_id_to_abs(somThis->orb,somThis->id);
	}

	RHBORB_unguard(somThis->orb)

#if 0
	if ((!somThis->absolute_name) && !ev->_major) 
	{
		/* need to
		     1. remove "IDL:" preceeding
			 2. split name up based on '/'
			 3. only include up to ":major.minor"
        */

		qwewqewq stop build here 
			hopefully these will seriously simplify things

		data.c=ORB_lookup_id(SOMD_ORBObject,ev,NULL,somThis->id);

		if (data.c && !ev->_major)
		{
			if (!somThis->absolute_name)
			{
				ORB_contained_describe(SOMD_ORBObject,ev,data.c,NULL,NULL,&data.p,NULL);

/*					data.p=Contained__get_absolute_name(data.c,ev);*/

				if (data.p && !ev->_major)
				{
					RHBORB_guard(somThis->orb,key)

					if (!somThis->absolute_name)
					{
						somThis->absolute_name=data.p;
						data.p=NULL;
					}

					RHBORB_unguard(somThis->orb,key)
				}
			}
		}
	}
#endif

#if 0
	RHBOPT_cleanup_pop();
#endif

	return somThis->absolute_name;
}

static struct RHBInterfaceDefVtbl rhbInterfaceDefVtbl={
	rhbInterfaceDef_AddRef,
	rhbInterfaceDef_Release,
	rhbInterfaceDef_get_absolute_name,
	rhbInterfaceDef_get_proxy_class
};

SOMEXTERN RHBInterfaceDef *RHBInterfaceDefNew(
		RHBORB *the_orb,
		corbastring the_name,
		boolean orb_locked)
{
	size_t len=strlen(the_name);
	RHBInterfaceDef *somThis=(RHBInterfaceDef *)SOMMalloc(sizeof(*somThis)+len);
	somThis->lpVtbl=&rhbInterfaceDefVtbl;
	somThis->lUsage=1;
	somThis->absolute_name=NULL;
	somThis->orb=the_orb;

	RHBOPT_ASSERT(memcmp(the_name,"::",2));

	memcpy(somThis->id,the_name,len+1);

	RHBDebug_created(RHBInterfaceDef,somThis)

/*	somPrintf("Created interface '%s'\n",somThis->name);*/
	RHBORB_add_interface(the_orb,somThis,orb_locked);

	RHBORB_AddRef(the_orb);

	return somThis;
}

union RHBObject_dispatch_arg
{
	char char_arg;
	short short_arg;
	long long_arg;
	float float_arg;
	double double_arg;
	void *pointer_arg;
};


static void rhbObject_somDumpSelfInt(
			RHBObject *somThis,
			SOMDObject SOMSTAR somSelf,long level)
{
#ifdef SHOW_MY_PARENTS
	unsigned long l=0;
	_IDL_SEQUENCE_SOMClass parents={0,0,NULL};
	SOMClass SOMSTAR myClass=NULL;
#endif

	RHBORB_guard(somThis->impl)

	if (somThis->iface)
	{
		somPrefixLevel(level);
			dump_somPrintf(("type_id    %s\n",somThis->iface->id));
	}

	if (somThis->impl)
	{
		somPrefixLevel(level);
			dump_somPrintf(("protocol   SOMD_TCPIP, "));
/*		somPrefixLevel(level); */
			dump_somPrintf(("host:%s, ",somThis->impl->address.host));
/*		somPrefixLevel(level);*/
			dump_somPrintf(("port:%d\n",(int)somThis->impl->address.port));
	}

#ifdef _DEBUG
	somPrefixLevel(level);
	dump_somPrintf(("object_key "));
	
	{
		unsigned int i=0;

		while (i < somThis->object_key._length)
		{
			octet c=somThis->object_key._buffer[i];

			if ((c>=' ')&&(c < 0x7f))
			{
				if (c=='\\')
				{
					dump_somPrintf(("\\\\"));
				}
				else
				{
					dump_somPrintf(("%c",c));
				}
			}
			else
			{
				dump_somPrintf(("\\"));
				dump_somPrintf(("%c",((c >> 6)&7)+'0'));
				dump_somPrintf(("%c",((c >> 3)&7)+'0'));
				dump_somPrintf(("%c",((c >> 0)&7)+'0'));
			}

			i++;
		}
	}
	dump_somPrintf(("\n"));
#else
	somPrefixLevel(level);
		dump_somPrintf(("object_key "));
		{
			Environment ev;
			RHBORB *c_orb;
			SOM_InitEnvironment(&ev);
			c_orb=ORB__get_c_orb(SOMD_ORBObject,&ev);
			RHBORB_dump_sequence(c_orb,&somThis->object_key);
				dump_somPrintf(("\n"));
			SOM_UninitEnvironment(&ev);
		}
#endif

#ifdef SHOW_MY_PARENTS
	somPrefixLevel(level);
	dump_somPrintf(("parents\n"));

/*	myClass=SOMObject_somGetClass(somSelf);*/

	myClass=somSelf->mtab->classObject;
	
	parents=SOMClass_somGetParents(myClass);

	while (l < parents._length)
	{
		char *p=SOMClass_somGetName(parents._buffer[l]);

		somPrefixLevel(level+1);
		
		dump_somPrintf(("%s\n",p));

/*		somReleaseClassReference(parents._buffer[l]);*/
		l++;
	}

/*	somReleaseClassReference(myClass);*/

	SOMFree(parents._buffer);
#endif

	RHBORB_unguard(somThis->impl)
}

static SOMObject SOMSTAR rhbObject_get_server_object(
				RHBObject *obj,
				Environment *ev,
				_IDL_SEQUENCE_octet *n)
{
	SOMObject SOMSTAR returned_object=NULL;
	RHBServerRequest *request=NULL;
	RHBORB_ThreadTask *thread=RHBSOMD_get_thread();

	if (thread)
	{
		request=thread->server_request;
	}

	if (n->_length < OBJECTKEY_PREFIX_LEN)
	{
		return NULL;
	}

	if (n->_buffer[0]!=0x3)
	{
		return NULL;
	}

	if (n->_length==OBJECTKEY_PREFIX_LEN)
	{
		returned_object=RHBImplementationDef_acquire_somdServer(
				obj->impl,ev);

		return returned_object;
	}

	RHBORB_guard(request->impl)

	if (!returned_object)
	{
		unsigned int refdata_len=n->_length-OBJECTKEY_PREFIX_LEN;

		if (refdata_len==sizeof(returned_object))
		{
			octet *op=n->_buffer+OBJECTKEY_PREFIX_LEN;
			memcpy(&returned_object,op,refdata_len);
		}

		if (returned_object)
		{
			unsigned int i=request->impl->client_references._length;
			RHBImplClientRef *_buffer=request->impl->client_references._buffer;

			while (i)
			{
				if (_buffer->somobj==returned_object)
				{
					break;
				}
				i--;
				_buffer++;
			}

			if (!i) 
			{
				if (RHBORB_any_client_references(request->orb,returned_object))
				{
#ifdef SOMObject_somDuplicateReference
					returned_object=SOMObject_somDuplicateReference(returned_object);
					RHBImplementationDef_add_client_ref(request->impl,returned_object);
#else
					if (SOMObject_somIsA(returned_object,_SOMRefObject))
					{
						/* should only add ref-counted objects */
						returned_object=SOMRefObject_somDuplicateReference(returned_object);
						RHBImplementationDef_add_client_ref(request->impl,returned_object);
					}
					else
					{
						debug_somPrintf(("warning:: it's not a reference counted object\n"));
					}
#endif
				}
				else
				{
					i=0;
					while (i < obj->object_key._length)
					{
						debug_somPrintf(("%c",obj->object_key._buffer[i]));
						i++;
					}
					debug_somPrintf(("\n"));
					debug_somPrintf(("%s:%d, object %p not considered a safe object\n",
								__FILE__,__LINE__,
								returned_object));
					returned_object=NULL;
				}
			}
		}
	}

	if (returned_object)
	{
/*		SOMObject_somPrintSelf(result);*/

		/* this *MUST* match how the server request
			free's an object */
#ifdef SOMObject_somDuplicateReference
		returned_object=SOMObject_somDuplicateReference(returned_object);
#else
		if (SOMObject_somIsA(returned_object,_SOMRefObject))
		{
			returned_object=SOMRefObject_somDuplicateReference(returned_object);
		}
#endif
	}
	else
	{
		RHBOPT_throw_StExcep(ev,INV_OBJREF,BadObjref,NO);
	}

	RHBORB_unguard(request->impl)

	return returned_object;
}

static char * rhbObject_get_type_id(
		RHBObject *somThis,
		Environment *ev)
{
	char *p="IDL:CORBA/Object:1.0";
	RHBOPT_unused(ev)

/*	somPrintf("Guessing object is a '%s'\n",p);
*/
	return somd_dupl_string(p);
}

static void rhbObject_change_location(
			RHBObject *somThis,
			Environment *ev,
			RHBObject *obj)
{
	RHBImplementationDef *old_impl;
	RHBInterfaceDef *old_iface;

	old_impl=somThis->impl;
	old_iface=somThis->iface;

	somThis->impl=obj->impl;
	somThis->iface=obj->iface;

	if (somThis->iface) RHBInterfaceDef_AddRef(somThis->iface);
	if (somThis->impl) RHBImplementationDef_AddRef(somThis->impl);

	if (somThis->object_key._buffer)
	{
		SOMFree(somThis->object_key._buffer);
		somThis->object_key._buffer=0;
		somThis->object_key._length=0;
		somThis->object_key._maximum=0;
	}

	if (obj->object_key._length)
	{
		somThis->object_key._length=obj->object_key._length;
		somThis->object_key._maximum=obj->object_key._length;
		somThis->object_key._buffer=SOMMalloc(obj->object_key._length);
	}

	if (somThis->object_key._length)
	{
		memcpy(somThis->object_key._buffer,
			obj->object_key._buffer,
			somThis->object_key._length);
	}

	if (old_impl) RHBImplementationDef_Release(old_impl);
	if (old_iface) RHBInterfaceDef_Release(old_iface);
}


static struct RHBObjectVtbl RHBObjectVtbl={
	rhbObject_somUninit,
	rhbObject_get_server_object,
	rhbObject_somDumpSelfInt,
	rhbObject_create_request,
	rhbObject_change_location,
	rhbObject_get_type_id
};

SOMEXTERN void RHBObject_somInit(
				RHBObject *somThis,
				RHBInterfaceDef *iface_def,
				RHBImplementationDef *impl_def,
				_IDL_SEQUENCE_octet *o_key)
{
	somThis->lpVtbl=&RHBObjectVtbl;

	somThis->object_key._length=o_key->_length;
	somThis->object_key._maximum=o_key->_maximum;
	if (somThis->object_key._maximum)
	{
		somThis->object_key._buffer=(octet *)SOMMalloc(somThis->object_key._maximum);
		memcpy(somThis->object_key._buffer,o_key->_buffer,somThis->object_key._length);
	}
	else
	{
		somThis->object_key._buffer=0;
	}

	somThis->impl=impl_def;

	somThis->iface=iface_def;

	if (somThis->impl) 
	{
		/* check if this is an ephemeral server.... */

		if (somThis->impl->server.data)
		{
			if (somThis->impl->server.data->listener)
			{
				somThis->impl=somThis->impl->server.data->listener;
			}
		}

		RHBImplementationDef_AddRef(somThis->impl);
	}

	if (somThis->iface) RHBInterfaceDef_AddRef(somThis->iface);
}
