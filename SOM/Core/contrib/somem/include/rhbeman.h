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

/* SOMEEMan internals.... */

#include <timerev.h>
#include <sinkev.h>
#include <workprev.h>

#ifdef _PLATFORM_UNIX_
	#include <sys/types.h>
	#include <sys/time.h>
	#include <unistd.h>
	#include <errno.h>
#endif

typedef enum 
{ 
	eman_cb_none, 
	eman_cb_proc,
	eman_cb_event,
	eman_cb_event_ev 
} RHBEMan_callback_type;

typedef struct RHBEMan_Entry
{
	QElem element;
	SOMEEManQueue *queue;

	struct RHBEMan_Entry *next;

	RHBEMan_callback_type eman_callback_type;
	somToken eman_callback_token;

	union
	{
		struct
		{
			EMRegProc *pfn;
		} eman_proc;
		struct 
		{
			SOMObject SOMSTAR target;
			EMMethodProc *pfn;
		} eman_event;
		struct 
		{
			SOMObject SOMSTAR target;
			EMMethodProcEv *pfn;
			Environment *ev;
		} eman_event_ev;
	} eman_callback;

	long token;

	union
	{
		struct 
		{
			SOCKET s_fd;
			long s_mask;
		} sink;

		struct
		{
			long t_interval;
			long t_count;
			long t_when;
		} timer;

		struct
		{
			corbastring ct_type;
		} client_type;

	} specifics;

} RHBEMan_Entry;



