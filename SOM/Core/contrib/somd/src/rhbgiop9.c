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

/* this contains RequestList */

static void rhbRequestList_add(RHBRequestList *somThis,RHBRequest *request)
{
	RHBSOMD_must_be_guarded

	if (request->container) SOMD_bomb("Should not have a container\n");

	request->container=somThis;

	request->next=0;

	if (somThis->first)
	{
		if (somThis->first->container != somThis)
		{
			SOMD_bomb("bad first item");
		}
		if (somThis->last->container != somThis)
		{
			SOMD_bomb("bad last item");
		}

		somThis->last->next=request;
		somThis->last=request;
	}
	else
	{
		somThis->first=request;
		somThis->last=request;
	}
}

static boolean rhbRequestList_remove(RHBRequestList *somThis,RHBRequest *request)
{
	RHBSOMD_must_be_guarded

	if (!somThis) 
	{
		return 0;
	}

	if (request)
	{
		if (!request->container)
		{
			/* it's not in any container */

			return 0;
		}

		RHBOPT_ASSERT(request->container==somThis)

		if (request==somThis->first)
		{
			somThis->first=request->next;

			if (!somThis->first)
			{
				somThis->last=0;
			}

			request->container=0;

			return 1;
		}
		else
		{
			RHBRequest *s=somThis->first;

			while (s)
			{
				if (s->next==request)
				{
					s->next=request->next;

					if (!s->next)
					{
						somThis->last=s;
					}

					request->container=0;

					return 1;
				}

				s=s->next;
			}
		}
	}

	return 0;
}

static boolean rhbRequestList_empty(RHBRequestList *somThis)
{
	if (somThis->first) return 0;
	return 1;
}

static RHBRequest *rhbRequestList_pop(RHBRequestList *somThis)
{
	RHBSOMD_must_be_guarded

	if (somThis->first)
	{
		RHBRequest *p=somThis->first;

		RHBRequestList_remove(somThis,p);

		return p;
	}

	return NULL;
}

static struct RHBRequestListVtbl rhbRequestListVtbl={
	rhbRequestList_add,
	rhbRequestList_remove,
	rhbRequestList_empty,
	rhbRequestList_pop
};

SOMEXTERN void RHBRequestList_init(RHBRequestList *somThis)
{
	somThis->lpVtbl=&rhbRequestListVtbl;
	somThis->first=0;
	somThis->last=0;
}
