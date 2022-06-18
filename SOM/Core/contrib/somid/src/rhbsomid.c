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

/* $Id$ */

#include <rhbopt.h>

#ifdef _WIN32
	#include <windows.h>
#endif

#define SOMDServer_VA_EXTERN
#define SOMMProxyForObject_VA_EXTERN

#ifdef BUILD_SOM
	#include <somkern.h>
/*	#define Flags		somkern_Flags
	#define ORBStatus	somkern_ORBStatus
	#define NamedValue	somkern_NamedValue*/
#endif

#include <som.h>
#include <somdtype.h>
#include <stexcep.h>
#include <omgestio.h>
#include <formarsh.h>
#include <somderr.h>

/* this is used to marshal the somId type
	backwards and forwards */

#include <rhbsomid.h>

struct somIdMarshal
{
	somId mid;
	somToken token;
};

RHBOPT_cleanup_begin(somIdMarshal_cleanup,pv)

	struct somIdMarshal *data=pv;
	if (data->mid) SOMFree(data->mid);
	if (data->token) SOMFree(data->token);

RHBOPT_cleanup_end

SOMEXTERN void SOMLINK 
	somIdMarshal(
		void *latent_param,
		char *foreign_addr,
		SOMD_marshaling_direction_t direction,
		SOMD_marshaling_op_t function,
		CosStream_StreamIO SOMSTAR stream,
		Environment *ev)
{
	struct somIdMarshal data={NULL,NULL};

	if (ev->_major) return;

	if (!foreign_addr)
	{
		RHBOPT_throw_StExcep(ev,
			MARSHAL,UnexpectedNULL,NO);

		return;
	}

	RHBOPT_cleanup_push(somIdMarshal_cleanup,&data);

	switch (function)
	{
	case SOMD_OpFreeStorage:
		if (foreign_addr)
		{
			somId *pid=(somId *)foreign_addr;
			data.token=*pid;
			*pid=NULL;
		}
		break;
	case SOMD_OpMarshal:
		if (foreign_addr)
		{
			somId *pid=(somId *)foreign_addr;
			somId id=*pid;
			somToken token=NULL;
#ifdef USE_APPLE_SOM		
			data.token=somMakeStringFromId(id);
#else
			char *p=somStringFromId(id);
#endif
			data.mid=somIdFromString("write_string");

/*			CosStream_StreamIO_write_string(stream,ev,p);*/

			somva_SOMObject_somDispatch(stream,&token,data.mid,
				stream,ev,
#ifdef USE_APPLE_SOM		
				data.token
#else
				p
#endif
				);
		}
		break;
	case SOMD_OpDemarshal:
		if (foreign_addr)
		{
			somId *pid=(somId *)foreign_addr;
			data.mid=somIdFromString("read_string");

	/*		token=CosStream_StreamIO_read_string(stream,ev);*/

			*pid=NULL;

			somva_SOMObject_somDispatch(stream,&data.token,data.mid,
				stream,ev);

			if (ev->_major)
			{
				data.token=NULL;
			}
			else
			{
				if (data.token)
				{
					*pid=somIdFromString(data.token);
				}
			}
		}
		break;
	default:
		RHBOPT_throw_StExcep(ev,
			MARSHAL,NotImplemented,NO);
		break;
	}

	RHBOPT_cleanup_pop();
}
