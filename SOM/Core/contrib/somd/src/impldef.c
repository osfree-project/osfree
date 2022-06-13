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
#include <somuutil.h>
#include <somuuuid.h>

#ifndef SOM_Module_impldef_Source
#define SOM_Module_impldef_Source
#endif
#define ImplementationDef_Class_Source
#include "impldef.ih"

#include <rhbgiops.h>

static char *impldef_dupl_string(const char *p)
{
	/* dont return empty strings */
	if (p)
	{
		if (*p)
		{
			size_t len=strlen(p)+1;
			char *q=SOMMalloc(len);
			if (q)
			{
				memcpy(q,p,len);
				return q;
			}
		}
	}

	return NULL;
}

static char * impldef_create_DSOM_guid(void)
{
	octet uuid[16];
	char str[64];
	char *p;

	somCreateUUID(uuid);

	p=somUUIDToString(uuid,str);

	return impldef_dupl_string(p);
}

SOM_Scope void SOMLINK impldef__set_c_impl(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			RHBImplementationDef *c_impl)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	RHBOPT_unused(ev)

	if (c_impl != somThis->c_impl)
	{
		RHBImplementationDef *old;

		RHBORB_guard(0)

		old=somThis->c_impl;
		somThis->c_impl=c_impl;

		RHBORB_unguard(0)

		if (c_impl) 
		{
			RHBImplementationDef_AddRef(c_impl);
		}

		if (old)
		{
			RHBImplementationDef_Release(old);
		}
	}
}

SOM_Scope corbastring SOMLINK impldef__get_impl_id(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);
	char *impl_id=NULL;

	if (!ev->_major)
	{
		if (!somThis->impl_id)
		{
			impl_id=impldef_create_DSOM_guid();

			RHBORB_guard(0)

			if (impl_id)
			{
				if (somThis->impl_id)
				{
					SOMFree(impl_id);
				}
				else
				{
					somThis->impl_id=impl_id;
				}
			}

			impl_id=impldef_dupl_string(somThis->impl_id);

			RHBORB_unguard(0)
		}
		else
		{
			RHBORB_guard(0)
			impl_id=impldef_dupl_string(somThis->impl_id);
			RHBORB_unguard(0)
		}
	}

	return impl_id;
}

SOM_Scope corbastring SOMLINK impldef__get_impl_alias(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return NULL;

	return impldef_dupl_string(somThis->impl_alias);
}

SOM_Scope corbastring SOMLINK impldef__get_impl_program(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);
	char *impl_program;

	if (ev->_major) return NULL;

	RHBORB_guard(0)

	impl_program=impldef_dupl_string(somThis->impl_program);

	RHBORB_unguard(0)

	return impl_program;
}

SOM_Scope Flags SOMLINK impldef__get_impl_flags(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return 0;

	return somThis->impl_flags;
}

SOM_Scope corbastring SOMLINK impldef__get_impl_server_class(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);
	char *impl_server_class;

	if (ev->_major) return NULL;

	RHBORB_guard(0)

	impl_server_class=impldef_dupl_string(somThis->impl_server_class);

	RHBORB_unguard(0)

	return impl_server_class;
}


SOM_Scope corbastring SOMLINK impldef__get_impl_refdata_file(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return NULL;

	return impldef_dupl_string(somThis->impl_refdata_file);
}

SOM_Scope corbastring SOMLINK impldef__get_impl_refdata_bkup(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return NULL;

	return impldef_dupl_string(somThis->impl_refdata_bkup);
}

SOM_Scope corbastring SOMLINK impldef__get_impl_hostname(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);
	char *impl_hostname;

	if (ev->_major) return NULL;

	RHBORB_guard(0)

	if (somThis->c_impl)
	{
		if (somThis->c_impl->address.host)
		{
			impl_hostname=impldef_dupl_string(somThis->c_impl->address.host);
		}
		else
		{
			impl_hostname=impldef_dupl_string(somThis->impl_hostname);
		}
	}
	else
	{
		impl_hostname=impldef_dupl_string(somThis->impl_hostname);
	}

	RHBORB_unguard(0)

	return impl_hostname;
}

SOM_Scope corbastring SOMLINK impldef__get_config_file(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);
	char *config_file;
	
	if (ev->_major) return NULL;

	RHBORB_guard(0)

	config_file=impldef_dupl_string(somThis->config_file);

	RHBORB_unguard(0)

	return config_file;
}

SOM_Scope corbastring SOMLINK impldef__get_impldef_class(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);
	char *impldef_class;

	if (ev->_major) return NULL;

	RHBORB_guard(0)

	impldef_class=impldef_dupl_string(somThis->impldef_class);

	RHBORB_unguard(0)

	return impldef_class;
}

SOM_Scope ImplementationDef_octet_seqP SOMLINK impldef__get_svr_objref(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return NULL;

	return somThis->svr_objref;
}

SOM_Scope corbastring SOMLINK impldef__get_protocols(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)

	if (ev->_major) return NULL;

	return impldef_dupl_string("SOMD_TCPIP");
}

SOM_Scope ImplementationDef_seq_prot_responses *SOMLINK impldef__get_registrar_resp(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)

	if (ev->_major) return NULL;

	return 0;
}

SOM_Scope void SOMLINK impldef__set_impl_id(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring impl_id)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	if (somThis->impl_id) SOMFree(somThis->impl_id);

	somThis->impl_id=impldef_dupl_string(impl_id);
}

SOM_Scope void SOMLINK impldef__set_impl_alias(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring impl_alias)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	if (somThis->impl_alias) SOMFree(somThis->impl_alias);
	somThis->impl_alias=impldef_dupl_string(impl_alias);
}

SOM_Scope void SOMLINK impldef__set_impl_program(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring impl_program)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	if (somThis->impl_program) SOMFree(somThis->impl_program);
	somThis->impl_program=impldef_dupl_string(impl_program);
}

SOM_Scope void SOMLINK impldef__set_impl_flags(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ Flags impl_flags)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	somThis->impl_flags=impl_flags;
}

SOM_Scope void SOMLINK impldef__set_impl_server_class(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring impl_server_class)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	if (somThis->impl_server_class)
	{
		SOMFree(somThis->impl_server_class);
	}

	somThis->impl_server_class=impldef_dupl_string(impl_server_class);
}

SOM_Scope void SOMLINK impldef__set_impl_refdata_file(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring impl_refdata_file)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	if (somThis->impl_refdata_file)
	{
		SOMFree(somThis->impl_refdata_file);
	}

	somThis->impl_refdata_file=impldef_dupl_string(impl_refdata_file);
}

SOM_Scope void SOMLINK impldef__set_impl_refdata_bkup(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring impl_refdata_bkup)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	if (somThis->impl_refdata_bkup)
	{
		SOMFree(somThis->impl_refdata_bkup);
	}

	somThis->impl_refdata_bkup=impldef_dupl_string(impl_refdata_bkup);
}

SOM_Scope void SOMLINK impldef__set_impl_hostname(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring impl_hostname)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	if (somThis->impl_hostname)
	{
		SOMFree(somThis->impl_hostname);
	}

	somThis->impl_hostname=impldef_dupl_string(impl_hostname);
}

SOM_Scope void SOMLINK impldef__set_config_file(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring config_file)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	if (somThis->config_file)
	{
		SOMFree(somThis->config_file);
	}

	somThis->config_file=impldef_dupl_string(config_file);
}

SOM_Scope void SOMLINK impldef__set_impldef_class(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring impldef_class)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	if (somThis->impldef_class)
	{
		SOMFree(somThis->impldef_class);
	}

	somThis->impldef_class=impldef_dupl_string(impldef_class);
}

SOM_Scope void SOMLINK impldef__set_svr_objref(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplementationDef_octet_seqP svr_objref)
{
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	if (ev->_major) return;

	RHBORB_guard(0)

	if (somThis->svr_objref)
	{
		octet *op=somThis->svr_objref->_buffer;
		if (op) SOMFree(op);
		SOMFree(somThis->svr_objref);
	}

	somThis->svr_objref=svr_objref;

	RHBORB_unguard(0)
}

SOM_Scope void SOMLINK impldef__set_protocols(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring protocols)
{
	if (ev->_major) return;

	RHBOPT_unused(somSelf)
	RHBOPT_unused(protocols)
}

SOM_Scope void SOMLINK impldef__set_registrar_resp(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplementationDef_seq_prot_responses *registrar_resp)
{
	RHBOPT_unused(somSelf)

	if (ev->_major) return;

	RHBOPT_unused(registrar_resp)
}

static corbastring ImplDef_attributes[]={
	"impl_id",
	"impl_alias",
	"impl_program",
	"impl_server_class",
	"impl_hostname",
	"impl_refdata_file",
	"impl_refdata_bkup",
	"config_file",
	"protocols",
	NULL
};

SOM_Scope void SOMLINK impldef_internalize_from_stream(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosStream_StreamIO SOMSTAR stream,
	/* in */ CosLifeCycle_FactoryFinder SOMSTAR ff)
{
	struct
	{
		unsigned short impl_version;
		corbastring impl_alias;
		corbastring impl_id;
		corbastring impldef_class;
		corbastring impl_program;
		unsigned long impl_flags;
		boolean secure;
		corbastring impl_server_class;
		corbastring impl_refdata_file;
		corbastring impl_refdata_bkup;
		corbastring impl_hostname;
		corbastring config_file;
		_IDL_SEQUENCE_octet *ior;
		corbastring protocols;
	} temp;

	memset(&temp,0,sizeof(temp));

	temp.impl_version=CosStream_StreamIO_read_unsigned_short(stream,ev);

	if (ev->_major) return;

	if (temp.impl_version != 0x1e) 
	{
		somSetException(ev,USER_EXCEPTION,ex_CosStream_StreamDataFormatError,0);

		return;
	}

#define impldef_destream(t,x)  temp.x=0; if (!ev->_major) temp.x=CosStream_StreamIO_read_##t(stream,ev)
#define impldef_apply(x)	   ImplementationDef__set_##x(somSelf,ev,temp.x) 
#define impldef_clear(x)       if (temp.x) { SOMFree(temp.x); temp.x=0; }

	impldef_destream(string,impl_alias);
	impldef_destream(string,impl_id);
	impldef_destream(string,impldef_class);
	impldef_destream(string,impl_program);
	temp.impl_flags=CosStream_StreamIO_read_unsigned_long(stream,ev);
	temp.secure=CosStream_StreamIO_read_boolean(stream,ev);
	impldef_destream(string,impl_server_class);
	impldef_destream(string,impl_refdata_file);
	impldef_destream(string,impl_refdata_bkup);
	impldef_destream(string,impl_hostname);
	impldef_destream(string,config_file);

	if (!ev->_major)
	{
		long length=CosStream_StreamIO_read_long(stream,ev);

		if (length)
		{
			unsigned long i=0;
			temp.ior=SOMMalloc(sizeof(*temp.ior));
			temp.ior->_length=(unsigned long)length;
			temp.ior->_maximum=temp.ior->_length;
			temp.ior->_buffer=SOMMalloc(temp.ior->_length);

			while (i < temp.ior->_length)
			{
				temp.ior->_buffer[i]=CosStream_StreamIO_read_octet(stream,ev);
				i++;
			}
		}
	}
	impldef_destream(string,protocols);
	if (!ev->_major)
	{
		impldef_apply(impl_alias);
		impldef_apply(impl_id);
		impldef_apply(impldef_class);
		impldef_apply(impl_program);
		impldef_apply(impl_flags);
		impldef_apply(impl_server_class);
		impldef_apply(impl_refdata_file);
		impldef_apply(impl_refdata_bkup);
		impldef_apply(impl_hostname);
		impldef_apply(config_file);
		impldef_apply(protocols);
		if (!ev->_major)
		{
			ImplementationDef__set_svr_objref(somSelf,ev,temp.ior);
		}
		if (!ev->_major)
		{
			temp.ior=0;
		}
	}

	impldef_clear(impl_alias);
	impldef_clear(impl_id);
	impldef_clear(impldef_class);
	impldef_clear(impl_program);
	impldef_clear(impl_server_class);
	impldef_clear(impl_refdata_file);
	impldef_clear(impl_refdata_bkup);
	impldef_clear(impl_hostname);
	impldef_clear(config_file);
	impldef_clear(protocols);

	if (temp.ior)
	{
		SOMFree(temp.ior->_buffer);
		SOMFree(temp.ior);
	}

#undef impldef_destream
#undef impldef_apply
#undef impldef_clear
}

SOM_Scope void SOMLINK impldef_externalize_to_stream(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosStream_StreamIO SOMSTAR stream)
{
	ImplementationDef_octet_seqP ior=0;
	unsigned long ul=0;

#define stream_string(x) if (!ev->_major) { \
	char *p=ImplementationDef__get_##x(somSelf,ev); \
	if (!ev->_major) { \
			CosStream_StreamIO_write_string(stream,ev,p); \
			if (p) SOMFree(p); } }

	CosStream_StreamIO_write_unsigned_short(stream,ev,0x1e);

	stream_string(impl_alias);
	stream_string(impl_id);
	stream_string(impldef_class);
	stream_string(impl_program);

	ul=ImplementationDef__get_impl_flags(somSelf,ev);
	CosStream_StreamIO_write_unsigned_long(stream,ev,ul);

	/* I think this is 'secure' */
	CosStream_StreamIO_write_boolean(stream,ev,0);

	stream_string(impl_server_class);
	stream_string(impl_refdata_file);
	stream_string(impl_refdata_bkup);
	stream_string(impl_hostname);
	stream_string(config_file);

	/* length of octet sequence */

	ior=ImplementationDef__get_svr_objref(somSelf,ev);
	if (ior && !ev->_major)
	{
		octet *op=ior->_buffer;
		CosStream_StreamIO_write_long(stream,ev,ior->_length);
		ul=ior->_length;
		while (ul--)
		{
			CosStream_StreamIO_write_octet(stream,ev,*op);
			op++;
		}
	}
	else
	{
		CosStream_StreamIO_write_long(stream,ev,0);
	}

	stream_string(protocols);
#undef stream_string
}

#ifdef ImplementationDef_BeginDestructor
SOM_Scope void SOMLINK impldef_somDestruct(
	ImplementationDef SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	ImplementationDefData *somThis;
	RHBImplementationDef *old;
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	ImplementationDef_BeginDestructor
#else
SOM_Scope void SOMLINK impldef_somUninit(
	ImplementationDef SOMSTAR somSelf)
{
	RHBImplementationDef *old;
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);
#endif

	RHBORB_guard(0)

	if (SOMD_ImplDefObject==somSelf)
	{
		SOMD_ImplDefObject=NULL;
	}

	old=somThis->c_impl;
	somThis->c_impl=NULL;

	if (somThis->impl_id)
	{
		SOMFree(somThis->impl_id);
		somThis->impl_id=NULL;
	}

	if (somThis->impl_alias)
	{
		SOMFree(somThis->impl_alias);
		somThis->impl_alias=NULL;
	}

	if (somThis->impl_server_class)
	{
		SOMFree(somThis->impl_server_class);
		somThis->impl_server_class=NULL;
	}

	if (somThis->impl_program)
	{
		SOMFree(somThis->impl_program);
		somThis->impl_program=NULL;
	}

	if (somThis->impl_hostname)
	{
		SOMFree(somThis->impl_hostname);
		somThis->impl_hostname=NULL;
	}

	if (somThis->impl_refdata_file)
	{
		SOMFree(somThis->impl_refdata_file);
		somThis->impl_refdata_file=NULL;
	}

	if (somThis->impl_refdata_bkup)
	{
		SOMFree(somThis->impl_refdata_bkup);
		somThis->impl_refdata_bkup=NULL;
	}

	if (somThis->svr_objref)
	{
		octet *op=somThis->svr_objref->_buffer;

		if (op)
		{
			SOMFree(op);
		}

		SOMFree(somThis->svr_objref);

		somThis->svr_objref=NULL;
	}

	if (somThis->config_file)
	{
		SOMFree(somThis->config_file);
		somThis->config_file=NULL;
	}

	if (somThis->impldef_class)
	{
		SOMFree(somThis->impldef_class);
		somThis->impldef_class=NULL;
	}

	RHBORB_unguard(0)

#ifdef ImplementationDef_EndDestructor
	ImplementationDef_EndDestructor
#else
	ImplementationDef_parent_CosStream_Streamable_somUninit(somSelf);
#endif

	if (old)
	{
		RHBImplementationDef_Release(old);
	}
}

#ifdef ImplementationDef_BeginInitializer_somDefaultInit
SOM_Scope void SOMLINK impldef_somDefaultInit(
		ImplementationDef SOMSTAR somSelf,
		somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	ImplementationDefData *somThis;

	ImplementationDef_BeginInitializer_somDefaultInit
	ImplementationDef_Init_CosStream_Streamable_somDefaultInit(somSelf,ctrl);
	RHBOPT_unused(somThis);
}
#else
SOM_Scope void SOMLINK impldef_somInit(
	ImplementationDef SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	ImplementationDef_parent_CosStream_Streamable_somInit(somSelf);
}
#endif

SOM_Scope void SOMLINK impldef_somDumpSelfInt(
	ImplementationDef SOMSTAR somSelf,
	/* in */ long level)
{
	long l=0;

	while (ImplDef_attributes[l])
	{
		char buf[256];
		char *p=ImplDef_attributes[l];
		somVaBuf vabuf;
		va_list ap;
		somId id;
		int i;
		Environment ev2;
		Environment *ev=&ev2;

		SOM_InitEnvironment(ev);

		somPrefixLevel(level);
		i=dump_somPrintf(("%s",p));

		while (i < 20)
		{
			dump_somPrintf((" "));
			i++;
		}

		dump_somPrintf((": "));

		strncpy(buf,"_get_",sizeof(buf)-1);
		strncat(buf,p,sizeof(buf)-1);

		p=NULL;
	
		vabuf=somVaBuf_create(NULL,0);

		somVaBuf_add(vabuf,&somSelf,tk_pointer);
		somVaBuf_add(vabuf,&ev,tk_pointer);
		somVaBuf_get_valist(vabuf,&ap);

		id=somIdFromString(buf);

		ImplementationDef_somDispatch(somSelf,(somToken *)(void *)&p,id,ap);

		SOMFree(id);
		if (p)
		{
			dump_somPrintf(("%s\n",p));
			SOMFree(p);
		}
		else
		{
			dump_somPrintf(("\n"));
		}
		l++;
		SOM_UninitEnvironment(ev);
		somVaBuf_destroy(vabuf);
	}

	ImplementationDef_parent_CosStream_Streamable_somDumpSelfInt(somSelf,level);
}

static RHBImplementationDef * ImplementationDef_create_from_hostname(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			RHBORB *c_orb)
{
	RHBImplementationDef *implDef=NULL;
	IIOP_ProfileBody_1_0 body;
	ImplementationDefData *somThis=ImplementationDefGetData(somSelf);

	memset(&body,0,sizeof(body));

	if (somThis->svr_objref)
	{
		/* read the IOR first */
		unsigned int i=0;		
		any a=IOP_Codec_decode_value(c_orb->iop_codec,ev,somThis->svr_objref,somdTC_IOP_IOR);
		IOP_IOR *ior=a._value;

		while (i < ior->profiles._length)
		{
			if (ior->profiles._buffer[i].tag==IOP_TAG_INTERNET_IOP)
			{
				any anybody=IOP_Codec_decode_value(c_orb->iop_codec,ev,
						&ior->profiles._buffer[i].profile_data,
						somdTC_IIOP_ProfileBody_1_0);

				if (!ev->_major)
				{
					IIOP_ProfileBody_1_0 *bodyRef=anybody._value;
					body=*bodyRef;
					TypeCode_free(anybody._type,ev);
					SOMFree(anybody._value);
				}

				break;
			}

			i++;
		}

		SOMD_FreeType(ev,&a,TC_any);
	}
	else
	{
		body.host=ImplementationDef__get_impl_hostname(somSelf,ev);

		if (ev->_major) 
		{
			return 0;
		}

		if (!body.host)
		{
			char *p=RHBORB_get_HOSTNAME(c_orb);

			body.host=impldef_dupl_string(p);
		}

		if (!body.port)
		{
			if (somThis->impl_id)
			{
				if (!strcmp(somThis->impl_id,RHBORB_get_somdd_implid(c_orb)))
				{
					body.port=RHBORB_get_SOMDPORT(c_orb);

/*				somPrintf("identified comms with daemon, %s:%d\n",
						body.host,(int)body.port);*/
				}
			}
		}

		body.iiop_version.major=1;
		body.iiop_version.minor=0;
		body.object_key._length=0;
		body.object_key._maximum=0;
		body.object_key._buffer=NULL;
	}

	if (body.port && body.host)
	{
		implDef=RHBORB_get_impl(c_orb,ev,&body);
	}
	else
	{
		implDef=RHBGIOPRequestStreamNew(c_orb,&body);
	}

	ImplementationDef__set_c_impl(somSelf,ev,implDef);

	RHBImplementationDef_Release(implDef);

	if (body.host) 
	{
		SOMFree(body.host);
	}

	if (body.object_key._buffer)
	{
		SOMFree(body.object_key._buffer);
	}

	return implDef;
}

SOM_Scope RHBImplementationDef * SOMLINK impldef__get_c_impl(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev)
{
	RHBImplementationDef *impl=NULL;
	RHBORB *c_orb=ORB__get_c_orb(SOMD_ORBObject,ev);

	if (somSelf && !RHBORB_isSOMDObject(c_orb,somSelf))
	{
		ImplementationDefData *somThis=ImplementationDefGetData(somSelf);
		RHBORB_guard(0)

		if ((!somThis->c_impl) && !ev->_major)
		{
			RHBORB_unguard(0)

			ImplementationDef_create_from_hostname(somSelf,ev,c_orb);

			RHBORB_guard(0)
		}

		if (!ev->_major)
		{
			impl=somThis->c_impl;

			if (impl) 
			{
				RHBImplementationDef_AddRef(impl);
			}
		}

		RHBORB_unguard(0)
	}

	return impl;
}

SOM_Scope CosLifeCycle_Key SOMLINK impldef__get_external_form_id(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	CosLifeCycle_Key key={0,0,0};

	if (!ev->_major)
	{
		char *id=ImplementationDef__get_impl_id(somSelf,ev);

		if (!ev->_major)
		{
			if (id)
			{
				key._buffer=SOMMalloc(sizeof(key._buffer[0]));
				key._length=1;
				key._maximum=1;

				id=ImplementationDef_somGetClassName(somSelf);

				key._buffer[0].id=impldef_dupl_string(id);
				key._buffer[0].kind=NULL;
			}
		}
	}

	return key;
}

static unsigned long RHBSOMD_chksum_bytes(void *pv,size_t t)
{
	unsigned long sum=0;
	octet *op=pv;

	while (t--)
	{
		octet o=(octet)(unsigned long)(sum >> 24);
		sum<<=24;
		sum+=o;
		sum+=*op++;
	}

	return sum;
}

SOM_Scope unsigned long SOMLINK impldef__get_constant_random_id(
		ImplementationDef SOMSTAR somSelf,
		Environment *ev)
{
	char *id=ImplementationDef__get_impl_id(somSelf,ev);

	if (id)
	{
		unsigned long l=RHBSOMD_chksum_bytes(id,strlen(id));
		SOMFree(id);
		return l;
	}

	return 0;
}

struct impldef_is_identical
{
	somId id;
	char *p;
	char *q;
};

RHBOPT_cleanup_begin(impldef_is_identical_cleanup,pv)

struct impldef_is_identical *data=pv;

	if (data->id) SOMFree(data->id);
	if (data->p) SOMFree(data->p);
	if (data->q) SOMFree(data->q);

RHBOPT_cleanup_end

SOM_Scope boolean SOMLINK impldef_is_identical(
	CosObjectIdentity_IdentifiableObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosObjectIdentity_IdentifiableObject SOMSTAR other_object)
{
	struct impldef_is_identical data={NULL,NULL,NULL};
	boolean RHBOPT_volatile b=0;

	RHBOPT_cleanup_push(impldef_is_identical_cleanup,&data);

	if (other_object && !ev->_major)
	{
		data.id=somIdFromString("_get_impl_id");

		b=somva_SOMObject_somDispatch(other_object,
					(somToken *)(void *)&data.p,
					data.id,
					other_object,ev);

		if (b && !ev->_major)
		{
			b=0;

			if (data.p)
			{
				data.q=ImplementationDef__get_impl_id(somSelf,ev);

				if (ev->_major)
				{
					data.q=NULL;
				}
				else
				{
					if (data.q)
					{
						b=(boolean)((!strcmp(data.p,data.q)) ? 1 : 0); 
					}
				}
			}
		}
	}

	RHBOPT_cleanup_pop();

	if (ev->_major) return 0;

	return b;
}


SOM_Scope unsigned short SOMLINK impldef__get_impl_version(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return 0x1E;
}

SOM_Scope void SOMLINK impldef__set_impl_version(
	ImplementationDef SOMSTAR somSelf,
	Environment *ev,
	unsigned short impl_version)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);

	/* I reckon this means '3.0', eg decimal 30 */

	if (impl_version != 0x1E)
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,BadVersion,NO);
	}
}

