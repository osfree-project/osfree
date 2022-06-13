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
#include <somd.h>
#include <unotypes.h>
#include <regior.h>
/* #include <rhbsomut.h> */
#include <containr.h>
#include <containd.h>
#include <repostry.h>
#include <typedef.h>

extern somToken 
	regimplTC__IOP_IOR,
	regimplTC__IIOP_ProfileBody_1_0;

static TypeCode IOP_IOR_tc(Environment *ev)
{
	return (TypeCode)(void *)&regimplTC__IOP_IOR;
}

static TypeCode IIOP_ProfileBody_tc(Environment *ev)
{
	return (TypeCode)(void *)&regimplTC__IIOP_ProfileBody_1_0;
}

static IOP_Codec SOMSTAR RHB_create_codec(Environment *ev)
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

ORBStatus RHB_create_ior(
		_IDL_SEQUENCE_octet *seq,
		Environment *ev,
		char *type_id,
		char *hostname,
		short port,
		_IDL_SEQUENCE_octet *object_key)
{
	IIOP_ProfileBody_1_0 body;
	IOP_IOR ior;
	IOP_TaggedProfile profile;
	any a;
	IOP_Codec SOMSTAR codec=RHB_create_codec(ev);

	seq->_length=0;
	seq->_maximum=0;
	seq->_buffer=0;

	ior.type_id=type_id;
	ior.profiles._buffer=&profile;
	ior.profiles._length=1;
	ior.profiles._maximum=1;

	profile.tag=IOP_TAG_INTERNET_IOP;
	profile.profile_data._length=0;
	profile.profile_data._maximum=0;
	profile.profile_data._buffer=0;

	body.iiop_version.major=1;
	body.iiop_version.minor=0;
	body.host=hostname;
	body.port=port;
	body.object_key=*object_key;

	a._type=IIOP_ProfileBody_tc(ev);
	a._value=&body;

	if (!ev->_major)
	{
		profile.profile_data=IOP_Codec_encode_value(codec,ev,&a);

		TypeCode_free(a._type,ev);

		a._type=IOP_IOR_tc(ev);
		a._value=&ior;

		if (!ev->_major)
		{
			*seq=IOP_Codec_encode_value(codec,ev,&a);	

			TypeCode_free(a._type,ev);
		}
	
		SOMFree(profile.profile_data._buffer);
	}
	
	somReleaseObjectReference(codec);

	return 0;
}

char *RHB_ior_string_from_octets(_IDL_SEQUENCE_octet *o)
{
	static char hex_nybble[16]="0123456789ABCDEF";

	char *result=0;
	if (o)
	{
		if (o->_length)
		{
			char *p=result=SOMMalloc(5+(o->_length<<1));

			if (p)
			{
				unsigned int i=o->_length;
				octet *op=o->_buffer;

				memcpy(p,"IOR:",4);
				p+=4;

				while (i--)
				{
					octet o=*op++;
					*p++=hex_nybble[0xf & (o>>4)];
					*p++=hex_nybble[0xf & o];
				}

				*p=0;
			}
		}
	}

	return result;
}

static octet nybble_value(char c)
{
	static char hex_nyb_u[16]="0123456789ABCDEF";
	static char hex_nyb_l[16]="0123456789abcdef";
	octet o=0;

	while (o < 16)
	{
		if (c==hex_nyb_u[o]) return o;
		if (c==hex_nyb_l[o]) return o;

		o++;
	}

	return 0;
}

_IDL_SEQUENCE_octet RHB_octets_from_ior_string(char *p)
{
	_IDL_SEQUENCE_octet result={0,0,0};

	if (p)
	{
		int i=(int)strlen(p);

		if (i > 6)
		{
			if (!memcmp(p,"IOR:",4))
			{
				p+=4;
				i-=4;

				i>>=1;

				result._buffer=SOMMalloc(i);
				if (result._buffer)
				{
					octet *op=result._buffer;
					result._length=i;
					result._maximum=i;

					while (i--)
					{
						octet o=nybble_value(*p++)<<4;
						o+=nybble_value(*p++);
						*op++=o;
					}
				}
			}
		}
	}

	return result;
}

void RHB_print_ior(
		_IDL_SEQUENCE_octet *seq,
		Environment *ev,long level)
{
	any a;
	TypeCode tc_IOP_IOR=IOP_IOR_tc(ev);
	Environment ev2;
	IOP_Codec SOMSTAR codec=RHB_create_codec(ev);

	if (ev->_major) return;

	SOM_InitEnvironment(&ev2);

	a=IOP_Codec_decode_value(codec,ev,seq,tc_IOP_IOR);

	if (!ev->_major)
	{
		unsigned int i=0;
		IOP_IOR *ior=a._value;

		somPrintf("%s\n",ior->type_id);

		while ((i < ior->profiles._length) && !ev->_major)
		{
			IOP_TaggedProfile *profile=&ior->profiles._buffer[i];

			switch (profile->tag)
			{
			case IOP_TAG_INTERNET_IOP:
				{
					TypeCode tc_IIOP_ProfileBody=IIOP_ProfileBody_tc(ev);

					if (!ev->_major)
					{
						any b=IOP_Codec_decode_value(codec,ev,&profile->profile_data,tc_IIOP_ProfileBody);

						if (!ev->_major)
						{
							IIOP_ProfileBody_1_0 *body=b._value;
							somPrefixLevel(level);
							somPrintf("tag:TAG_INTERNET_IOP, host:%s, port:%d\n",
								body->host,(int)body->port);

							SOMD_FreeType(&ev2,&b,TC_any);
						}

						TypeCode_free(tc_IIOP_ProfileBody,&ev2);
					}
				}
				break;
			default:
				somPrefixLevel(level);
				somPrintf("tag:%lx\n",profile->tag);
			}

			i++;
		}

		SOMD_FreeType(&ev2,&a,TC_any);
	}

	TypeCode_free(tc_IOP_IOR,&ev2);

	SOM_UninitEnvironment(&ev2);

	IOP_Codec_somFree(codec);
}



