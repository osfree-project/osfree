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
#include <rhbgiops.h>
#include <somestio.h>

#ifndef SOM_Module_principl_Source
#define SOM_Module_principl_Source
#endif
#define Principal_Class_Source

#include "principl.ih"

#define FLAGS_PACKET_SET		1
#define FLAGS_HOSTNAME_SET		2
#define FLAGS_USERNAME_SET		4

SOM_Scope void  SOMLINK principl__set_userName(
		Principal SOMSTAR somSelf, 
		Environment *ev, 
		corbastring userName)
{
    PrincipalData *somThis=PrincipalGetData(somSelf);

	if (ev && !ev->_major)
	{
		if (somThis->flags & FLAGS_USERNAME_SET)
		{
			RHBOPT_throw_StExcep(ev,NO_PERMISSION,NoAccess,NO);
		}
		else
		{
			somThis->flags|=FLAGS_USERNAME_SET;

			if (somThis->userName)
			{
				SOMFree(somThis->userName);

				somThis->userName=NULL;
			}

			somThis->userName=somd_dupl_string(userName);
		}
	}
}

SOM_Scope void  SOMLINK principl__set_hostName(
		Principal SOMSTAR somSelf, 
		Environment *ev, 
		corbastring hostName)
{
    PrincipalData *somThis=PrincipalGetData(somSelf);

	if (ev && !ev->_major)
	{
		if (somThis->flags & FLAGS_HOSTNAME_SET)
		{
			RHBOPT_throw_StExcep(ev,NO_PERMISSION,NoAccess,NO);
		}
		else
		{
			somThis->flags|=FLAGS_HOSTNAME_SET;

			if (somThis->hostName)
			{
				SOMFree(somThis->hostName);

				somThis->hostName=NULL;
			}

			somThis->hostName=somd_dupl_string(hostName);
		}
	}
}


SOM_Scope corbastring SOMLINK principl__get_userName(
	Principal SOMSTAR somSelf,
	Environment *ev)
{
	/* dual_owned result */
    PrincipalData *somThis=PrincipalGetData(somSelf);

	RHBOPT_unused(ev)

	return somThis->userName;
}

SOM_Scope corbastring SOMLINK principl__get_hostName(
	Principal SOMSTAR somSelf,
	Environment *ev)
{
	/* dual_owned result */
    PrincipalData *somThis = PrincipalGetData(somSelf);

	RHBOPT_unused(ev)

	return somThis->hostName;
}

#ifdef Principal_somDestruct
SOM_Scope void SOMLINK principl_somDestruct(
	Principal SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	PrincipalData *somThis;

	Principal_BeginDestructor
#else
SOM_Scope void SOMLINK principl_somUninit(
	Principal SOMSTAR somSelf)
{
    PrincipalData *somThis = PrincipalGetData(somSelf);
#endif

	if (somThis->userName)
	{
		SOMFree(somThis->userName);
		somThis->userName=NULL;
	}
	if (somThis->hostName)
	{
		SOMFree(somThis->hostName);
		somThis->hostName=NULL;
	}
	if (somThis->packet._buffer)
	{
		SOMFree(somThis->packet._buffer);
		somThis->packet._buffer=NULL;
	}

#ifdef Principal_somDestruct
	Principal_EndDestructor
#else
#	ifdef Principal_parent_SOMRefObject_somUninit
	Principal_parent_SOMRefObject_somUninit(somSelf);
#	else
	Principal_parent_SOMObject_somUninit(somSelf);
#	endif
#endif
}


#ifdef Principal_somDefaultInit
SOM_Scope void SOMLINK principl_somDefaultInit(
	Principal SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	PrincipalData *somThis;

	Principal_BeginInitializer_somDefaultInit
	Principal_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);

	somThis->flags=0;
	somThis->packet._maximum=0;
	somThis->packet._length=0;
	somThis->packet._buffer=NULL;
	somThis->userName=NULL;
	somThis->hostName=NULL;
}
#else
SOM_Scope void SOMLINK principl_somInit(
	Principal SOMSTAR somSelf)
{
#ifdef Principal_parent_SOMRefObject_somInit
	Principal_parent_SOMRefObject_somInit(somSelf);
#else
	Principal_parent_SOMObject_somInit(somSelf);
#endif
	RHBOPT_unused(somSelf)
}
#endif

struct principl_init_principal
{
	char *user;
	char *host;
	somStream_MemoryStreamIO SOMSTAR stream;
};

RHBOPT_cleanup_begin(principl_init_principal_cleanup,pv)

struct principl_init_principal *data=pv;

	if (data->stream) somReleaseObjectReference(data->stream);
	if (data->user) SOMFree(data->user);
	if (data->host) SOMFree(data->host);

RHBOPT_cleanup_end

SOM_Scope void SOMLINK principl_read_Principal(
	Principal SOMSTAR somSelf,
	Environment *ev,
	/* in */ _IDL_SEQUENCE_octet *init_data)
{
	if (ev && (!ev->_major))
	{
		PrincipalData *somThis=PrincipalGetData(somSelf);

		if (somThis->flags & FLAGS_PACKET_SET)
		{
			RHBOPT_throw_StExcep(ev,NO_PERMISSION,NoAccess,NO);
		}
		else
		{
			struct principl_init_principal data={NULL,NULL,NULL};

			RHBOPT_cleanup_push(principl_init_principal_cleanup,&data);

			if (somThis->packet._buffer)
			{
				octet *op=somThis->packet._buffer;
				somThis->packet._buffer=NULL;
				somThis->packet._length=0;
				somThis->packet._maximum=0;
				SOMFree(op);
			}

			somThis->flags|=FLAGS_PACKET_SET;

			if (init_data && (init_data->_length) && (init_data->_buffer))
			{
				somThis->packet._buffer=SOMMalloc(init_data->_length);

				if (somThis->packet._buffer)
				{
					somThis->packet._length=init_data->_length;
					somThis->packet._maximum=init_data->_length;
					memcpy(somThis->packet._buffer,init_data->_buffer,init_data->_length);

					data.stream=somStream_MemoryStreamIONew();

					if (data.stream)
					{
						somStream_MemoryStreamIO_set_buffer(data.stream,ev,init_data);

						data.user=somStream_MemoryStreamIO_read_string(data.stream,ev);
						data.host=somStream_MemoryStreamIO_read_string(data.stream,ev);
					}
				}
				else
				{
					RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
				}
			}

			if (!ev->_major)
			{
				Principal__set_userName(somSelf,ev,data.user);
			}

			if (!ev->_major)
			{
				Principal__set_hostName(somSelf,ev,data.host);
			}

			RHBOPT_cleanup_pop();
		}
	}
}

struct principl_somdMarshal
{
	somStream_MemoryStreamIO SOMSTAR stream;
	_IDL_SEQUENCE_octet out;
};

RHBOPT_cleanup_begin(principl_somdMarshal_cleanup,pv)

struct principl_somdMarshal *data=pv;

	if (data->stream) somReleaseObjectReference(data->stream);

RHBOPT_cleanup_end

SOM_Scope _IDL_SEQUENCE_octet SOMLINK principl_write_Principal(Principal SOMSTAR somSelf,
			Environment *ev)
{
	PrincipalData *somThis=PrincipalGetData(somSelf);
	struct principl_somdMarshal data={NULL,{0,0,NULL}};

	RHBOPT_cleanup_push(principl_somdMarshal_cleanup,&data);

	if (ev && (!ev->_major))
	{
		if (!(somThis->flags & 1))
		{
			/* dual_owned_result so do not need to free */

			char *userName=Principal__get_userName(somSelf,ev);
			char *hostName=Principal__get_hostName(somSelf,ev);

			data.stream=somStream_MemoryStreamIONew();

			if (data.stream)
			{
				somStream_MemoryStreamIO_write_string(data.stream,ev,userName);
				somStream_MemoryStreamIO_write_string(data.stream,ev,hostName);

				somThis->packet=somStream_MemoryStreamIO_get_buffer(data.stream,ev);

				if (!ev->_major)
				{
					somThis->flags|=1;
				}
			}
			else
			{
				RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
			}
		}

		if ((somThis->flags & 1)&&(!ev->_major))
		{
			if (somThis->packet._length && somThis->packet._buffer)
			{
				data.out._buffer=SOMMalloc(somThis->packet._length);

				if (data.out._buffer)
				{
					memcpy(data.out._buffer,somThis->packet._buffer,somThis->packet._length);

					data.out._length=somThis->packet._length;
					data.out._maximum=somThis->packet._length;
				}
				else
				{
					RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
				}
			}
		}
	}

	RHBOPT_cleanup_pop();

	return data.out;
}

