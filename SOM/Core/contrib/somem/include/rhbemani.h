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

#ifdef BUILD_SOMEM
	#define SOM_Module_eman_Source
	#define SOM_Module_workprev_Source
	#define SOM_Module_sinkev_Source
	#define SOM_Module_timerev_Source
	#define SOM_Module_clientev_Source
	#define SOM_Module_emregdat_Source
	#define SOM_Module_event_Source
#endif

#define SOMEEMRegisterData_VA_EXTERN

#ifdef _PLATFORM_UNIX_
	#include <sys/types.h>
	#include <sys/socket.h>
	#ifndef SOCKET
		#define SOCKET    int
	#endif
	#ifndef INVALID_SOCKET
		#define INVALID_SOCKET   ((SOCKET)-1)
	#endif
#endif

#ifdef _WIN32
	#include <winsock.h>
#else
	#ifndef SOCKET
		#define SOCKET    int
	#endif
	#ifndef INVALID_SOCKET
		#define INVALID_SOCKET   ((SOCKET)-1)
	#endif
#endif

#include <rhbsomex.h>

#ifndef USE_APPLE_SOM
	#include <somref.h>
#endif

#ifdef _PLATFORM_MACINTOSH_
	#include <OSUtils.h>
	#include <Processes.h>
	#include <AppleEvents.h>
	#define rhbeman_Enqueue(x,y)    Enqueue(x,y)
	#define rhbeman_Dequeue(x,y)    Dequeue(x,y)
#else
	#define RHBEMAN_SINKS
	#if (!defined(_WIN32S)) && (!defined(_WIN16)) && defined(RHBEMAN_SINKS)
		#define USE_EMAN_SOCKETPAIR
	#endif
	typedef struct QElem
	{
		struct QElem *qLink;
		short qType;
	} QElem, *QElemPtr;
	typedef struct QHdr
	{
		short qFlags;
		QElemPtr qHead;
		QElemPtr qTail;
	} QHdr, *QHdrPtr;
#endif

#include <event.h>
#include <rhbsomex.h>

typedef struct SOMEClientEventQL
{
	QElem qElem;	/* queue element */
	QHdrPtr header;	/* which queue this is in */
	SOMObject SOMSTAR event; /* actual object */
	void (*dequeue)(struct SOMEClientEventQL *);
	void *client_data;
	char *client_type;
} SOMEClientEventQL;

typedef struct SOMEEManQueue
{
	QHdr queue;
	long queue_type;
} SOMEEManQueue;

typedef struct SOMEEManPrivateData
{
	QHdr cl_events;
	struct eman_running *eman_running;
	struct eman_blocking *eman_blocking;
	SOMEEManQueue timer_queue,dead_timer_queue,sink_queue,client_types_queue,workproc_queue;
#ifdef USE_THREADS
	somToken running_list_mutex;
	#ifdef USE_PTHREADS
		pthread_mutex_t eman_sem;
		pthread_mutex_t eman_mtx;
		pthread_cond_t eman_event;
		pthread_t eman_current,mainThread;
		int eman_current_nesting,eman_kick_on_release;
	#else
		HANDLE eman_sem,eman_event;
		int eman_current_nesting,eman_kick_on_release;
	#endif
#endif
#ifdef _PLATFORM_MACINTOSH_
	struct rhbmtask *task;
	AEEventHandlerUPP task_upp;
	ProcessSerialNumber psn;
#endif
#ifdef USE_EMAN_SOCKETPAIR
	SOCKET eman_sock_write;
	SOCKET eman_sock_read;
#endif
	long last_token;
	struct RHBEMan_Entry *list;
} SOMEEManPrivateData;
