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
#ifdef _WIN32
	#define SOMDLLEXPORT
#endif
#include <rhbsomdd.h>

static TypeCode somddTC_IIOP_ProfileBody(IIOP_Version *vers)
{
	switch (vers->major)
	{
	case 1:
		switch (vers->minor)
		{
		case 0:
			return somddTC_IIOP_ProfileBody_1_0;
		case 1:
			return somddTC_IIOP_ProfileBody_1_1;
		}
		break;
	}

	return NULL;
}

IOP_TaggedProfile SOMDD_create_IIOP_tagged_profile(
		IOP_Codec SOMSTAR codec,
		Environment *ev,
		IIOP_ProfileBody_1_1 *body)
{
	any a={NULL,NULL};
	IOP_TaggedProfile seq={IOP_TAG_INTERNET_IOP,{0,0,NULL}};

	a._type=somddTC_IIOP_ProfileBody(&body->iiop_version);

	if (a._type)
	{
		a._value=body;
		seq.profile_data=IOP_Codec_encode_value(codec,ev,&a);
	}
	else
	{
		RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE);
	}

	return seq;
}

_IDL_SEQUENCE_octet SOMDD_create_IOP_IOR(
		IOP_Codec SOMSTAR codec,
		Environment *ev,
		IOP_IOR *ior)
{
any a={NULL,NULL};

	a._value=ior;
	a._type=somddTC_IOP_IOR;

	return IOP_Codec_encode_value(codec,ev,&a);
}

IOP_Codec SOMSTAR SOMDD_create_codec(Environment *ev)
{
	if (!ev->_major)
	{
		IOP_CodecFactory SOMSTAR factory=ORB_resolve_initial_references(
				SOMD_ORBObject,ev,
				"CodecFactory");

		if (!ev->_major)
		{
			if (factory)
			{
				IOP_Encoding enc={IOP_ENCODING_CDR_ENCAPS,1,0};
				IOP_Codec SOMSTAR codec=IOP_CodecFactory_create_codec(factory,ev,&enc);

				somReleaseObjectReference(factory);

				if (codec) return codec;
			}

			RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,NO);
		}
	}

	return NULL;
}


_IDL_SEQUENCE_octet *SOMDD_create_initial_ior(
		IOP_Codec SOMSTAR codec,
		Environment *ev,
		unsigned short iiop_port,
		char *impl_id)
{
	IIOP_ProfileBody_1_1 body={{1,0},NULL,0,{0,0,NULL},{0,0,NULL}};
	_IDL_SEQUENCE_octet *seqP=NULL;
	_IDL_SEQUENCE_octet seq={0,0,NULL};
	IOP_TaggedProfile profile={IOP_TAG_INTERNET_IOP,{0,0,NULL}};
	IOP_IOR ior={"::SOMDServer",{1,1,NULL}};

	ior.profiles._buffer=&profile;

	body.object_key=somdGetDefaultObjectKey(ev,impl_id);
	body.port=iiop_port;

	if (!ev->_major)
	{
		profile=SOMDD_create_IIOP_tagged_profile(codec,ev,&body);

		if (!ev->_major)
		{
			seq=SOMDD_create_IOP_IOR(codec,ev,&ior);
		}
	}

	if (body.object_key._buffer) SOMFree(body.object_key._buffer);
	if (profile.profile_data._buffer) SOMFree(profile.profile_data._buffer);

	if (seq._length && !ev->_major)
	{
		seqP=SOMMalloc(sizeof(*seqP));
		*seqP=seq;
		seq._buffer=NULL;
	}

	if (seq._buffer) SOMFree(seq._buffer);

	return seqP;
}

ImplementationDef SOMSTAR SOMDD_create_initial_ImplDef(Environment *ev)
{
	ImplementationDef SOMSTAR def=ImplementationDefNew();
	IOP_Codec SOMSTAR codec=SOMDD_create_codec(ev);
	unsigned short port=0;
	_IDL_SEQUENCE_octet key=somdGetDefaultObjectKey(ev,NULL);
	char *impl_id=(char *)(key._buffer+1);
	_IDL_SEQUENCE_octet *seqP=NULL;
	char *p=somutgetshellenv("SOMDPORT","[SOMD_TCPIP]");

	if (p)
	{
		port=(unsigned short)atol(p);
	}

	if (!port) 
	{
#ifdef RHBOPT_SOMDPORT
		port=RHBOPT_SOMDPORT;
#else
#	error RHBOPT_SOMDPORT
		port=3053;
#endif
	}

	seqP=SOMDD_create_initial_ior(codec,ev,port,impl_id);

	ImplementationDef__set_impl_id(def,ev,impl_id);
	ImplementationDef__set_svr_objref(def,ev,seqP);

	if (key._buffer) SOMFree(key._buffer);

	somReleaseObjectReference(codec);

	return def;
}

#if 0
SOMDObject SOMSTAR SOMDD_create_object_for_impl(
			SOMDObject SOMSTAR obj,
			Environment *ev,
			ImplementationDef SOMSTAR def)
{
	/* need to test this with a persistent server */

	return NULL;
}
#endif
