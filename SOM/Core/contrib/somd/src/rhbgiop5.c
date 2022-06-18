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
	#include <process.h>
#endif

#include <rhbsomd.h>
#include <rhbgiops.h>
#include <ipv6test.h>
#include <soms.h>

#ifdef _WIN32
	#ifdef IPV6_WILL_BIND
/*		#undef IPV6_WILL_BIND */
	#endif
#endif

/* RHBGIOPRequestStream 

   this object represents a network connection really...
*/


struct RHBORB_somdd_singleton
{
	IOP_TaggedProfile profile;
	RHBContained *c;
	char *rep_id;
};

RHBOPT_cleanup_begin(RHBORB_somdd_singleton_cleanup,pv)

struct RHBORB_somdd_singleton *data=pv;

	if (data->rep_id) SOMFree(data->rep_id);
	if (data->c) RHBContained_Release(data->c);
	if (data->profile.profile_data._buffer) SOMFree(data->profile.profile_data._buffer);

RHBOPT_cleanup_end

static void somd_singleton(
			RHBORB *somThis,
			char *className,
			Environment *ev,
			char *hostname,
			unsigned short port,
			_IDL_SEQUENCE_octet *arg_object_key,
			_IDL_SEQUENCE_octet *arg_seq)
{
	_IDL_SEQUENCE_octet * RHBOPT_volatile object_key=arg_object_key;
	_IDL_SEQUENCE_octet * RHBOPT_volatile seq=arg_seq;	
	struct RHBORB_somdd_singleton data={{IOP_TAG_INTERNET_IOP,{0,0,NULL}},NULL};
	IIOP_ProfileBody_1_0 body={{1,0},NULL,0,{0,0,NULL}};
	IOP_IOR ior={NULL,{0,0,NULL}};
	any a={NULL,NULL};
	size_t len=strlen(className);

	RHBOPT_cleanup_push(RHBORB_somdd_singleton_cleanup,&data);

	seq->_length=0;
	seq->_maximum=0;
	seq->_buffer=NULL;

	data.rep_id=SOMMalloc(3+len);
	data.rep_id[0]=':';
	data.rep_id[1]=':';
	memcpy(data.rep_id+2,className,len+1);

	data.c=RHBORB_get_contained(somThis,ev,data.rep_id);

	if (data.c)
	{
		ior.type_id=RHBContained_get_id(data.c,ev);
	}
	else
	{
		ior.type_id=data.rep_id;
	}

	if (ev->_major) somExceptionFree(ev);

	ior.profiles._buffer=&data.profile;
	ior.profiles._length=1;
	ior.profiles._maximum=1;

	body.host=hostname;
	body.port=port;
	body.object_key=*object_key;

	a._type=somdTC_IIOP_ProfileBody_1_0;
	a._value=&body;

	data.profile.profile_data=IOP_Codec_encode_value(somThis->iop_codec,ev,&a);

	if (!ev->_major)
	{
		a._type=somdTC_IOP_IOR;
		a._value=&ior;

		*seq=IOP_Codec_encode_value(somThis->iop_codec,ev,&a);
	}

	RHBOPT_cleanup_pop();
}

RHBSocketSink_impl(implsink)

static void throw_RHBSocketError(Environment *ev,RHBSocketError err,const char *file,int line)
{
	if (!ev->_major)
	{
		const char *id=ex_StExcep_COMM_FAILURE;
		StExcep_COMM_FAILURE *ex=SOMMalloc(sizeof(*ex));

		if (ex)
		{
			ex->minor=err;
			ex->completed=MAYBE;
		}

		if (SOM_TraceLevel)
		{
			somPrintf("somSetException(%s) at %s:%d\n",id,file,line);
		}

		somSetException(ev,SYSTEM_EXCEPTION,id,ex);
	}
}

static void RHBSendData_delete(RHBSendData *somThis,RHBGIOPRequestStream *impl)
{
	while (somThis)
	{
		void *pv=somThis;
		somThis=somThis->next;
		SOMFree(pv);

		if (!somd_atomic_dec(&impl->lUsage))
		{
			somPrintf("RHBSendData_delete, usage=%ld\n",(long)impl->lUsage);
		}
	}
}

static void RHBImplementationDef_free_transmit_queue(
		RHBImplementationDef *somThis)
{
	RHBSendData *data=somThis->transmitting.head;

	somThis->transmitting.head=NULL;
	somThis->transmitting.tail=NULL;

	RHBSendData_delete(data,somThis);
}

static void RHBImplementationDef_free_receive_buffer(
		RHBImplementationDef *somThis)
{
	if (somThis->receiving._maximum)
	{
		void *op=somThis->receiving._buffer;
		somThis->receiving._buffer=NULL;
		somThis->receiving._maximum=0;
		somThis->receiving._length=0;
		if (op)
		{
			SOMFree(op);
		}
		somThis->receiving_header=0;
	}
}

static void RHBImplementationDef_delete(RHBImplementationDef *somThis)
{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_destroy(&somThis->p_EventSem);
	#else
		CloseHandle(somThis->h_EventSem);
	#endif
#endif

	RHBDebug_deleted(RHBImplementationDef,somThis)

	SOMFree(somThis);
}

static void rhbGIOPRequestStream_Close(RHBGIOPRequestStream *somThis)
{
	if (!somThis->deleting)
	{
		Environment ev;
		RHBServerData *data=NULL;
		RHBORB *tmp=NULL;

		SOM_InitEnvironment(&ev);

		RHBORB_guard(somThis)

		if (somThis->deleting)
		{
			somThis=NULL;
		}
		else
		{
			data=somThis->server.data;
			somThis->server.data=NULL;

			somThis->deleting=1;

			if (somThis->client_references._length)
			{
				RHBORB_unguard(somThis)
				RHBImplementationDef_remove_client_references(somThis);
				RHBORB_guard(somThis)
			}

			if (somThis->connection.fd)
			{
				RHBSocketRef fd=somThis->connection.fd;
				somThis->connection.fd=NULL;
				/* it's a gracefull termination here */
				somThis->connection.connected=0;
				if (fd)
				{
					/* sends disallowed, FIN will be sent */
		/*			somPrintf("shutdown %p\n",fd);*/

					RHBSocket_Shutdown(fd,0,1);
					RHBSocket_Close(fd);
					RHBSocket_Release(fd);
				}
			}

			if (somThis->connection.resolver)
			{
				RHBResolverRef ref=somThis->connection.resolver;
				somThis->connection.resolver=NULL;

				if (ref) 
				{
					RHBResolver_Cancel(ref);
					RHBResolver_Release(ref);
				}
			}

			tmp=somThis->orb;
			somThis->orb=NULL;

			if (tmp)
			{
				RHBORB_remove_impl(tmp,somThis);
			}

			RHBOPT_ASSERT(!somThis->executing_oneway.first)
			RHBOPT_ASSERT(!somThis->executing_normal.first)
			RHBOPT_ASSERT(!somThis->pending.first)

			if (somThis->address.host)
			{
				SOMFree(somThis->address.host);
				somThis->address.host=NULL;
			}

			if (somThis->receiving._buffer)
			{
				SOMFree(somThis->receiving._buffer);
				somThis->receiving._buffer=NULL;
			}

			SOMD_FreeType(&ev,&somThis->address.addrSeq,somdTC_sequence_SOMD_NetworkAddress);

			RHBImplementationDef_free_transmit_queue(somThis);

			if ((somThis->lUsage) || (somThis->sink.lUsage))
			{
				somThis->deleting=0;
				somThis=NULL;
			}
		}

		RHBORB_unguard(0);

		if (somThis)
		{
			RHBImplementationDef_delete(somThis);
		}

		if (data)
		{
			data->lpVtbl->Release(data);
		}

		if (tmp)
		{
			RHBORB_Release(tmp);
		}
		
		SOM_UninitEnvironment(&ev);
	}
}

static ImplementationDef SOMSTAR rhbGIOPRequestStream_get_wrapper(
			RHBImplementationDef *somThis,
			Environment *ev,
			char *id_hint)
{
	ImplementationDef SOMSTAR wrapper=NULL;

	if (!id_hint)
	{
		RHBORB_guard(0)

		if (somThis->server.data)
		{	
			if (somThis->server.data->implDef)
			{
				wrapper=ImplementationDef_somDuplicateReference(somThis->server.data->implDef);
			}
		}

		RHBORB_unguard(0)
	}

	if (!wrapper)
	{
		wrapper=RHBORB_newImplDef(somThis->orb);

		if (id_hint)
		{
			ImplementationDef__set_impl_id(wrapper,ev,id_hint);
		}

		ImplementationDef__set_c_impl(wrapper,0,somThis);
	}

	return wrapper;
}

static void RHBImplementationDef_check_ending(RHBImplementationDef *somThis)
{
#ifdef USE_THREADS
	if (impl_is_deactivating(somThis))
	{
#ifdef USE_PTHREADS
		pthread_cond_signal(&somThis->p_EventSem);
#else
		if (!SetEvent(somThis->h_EventSem)) 
		{
#ifdef _M_IX86
			__asm int 3;
#endif
		}
#endif
	}
#endif
}

static void RHBImplementationDef_writes_wanted(
			RHBImplementationDef *somThis,
			Environment *ev,boolean flag)
{
	if (somThis->connection.fd)
	{
		RHBSocketRef fd;
		RHBORB_guard(somThis)
		fd=somThis->connection.fd;
		if (fd) 
		{
			RHBSocket_AddRef(fd);
		}
		RHBORB_unguard(somThis)
		if (fd)
		{
			RHBSocket_EnableWrites(fd,flag);
			RHBSocket_Release(fd);
		}
	}
}

static void RHBImplementationDef_clear_executing_oneways(
		RHBImplementationDef *somThis,
		Environment *ev)
{
	RHBRequest *req=NULL;

	do
	{
		RHBORB_guard(somThis)

		req=somThis->executing_oneway.first;

		while (req)
		{
			if (req->request.send_flags & INV_NO_RESPONSE)
			{	
				RHBRequest_AddRef(req);

				RHBRequestList_remove(&somThis->executing_oneway,req);

				break;
			}
			else
			{
				req=req->next;
			}
		}

		RHBORB_unguard(somThis)

		if (req)
		{
			RHBRequest_SetComplete(req,ev);

			RHBRequest_Release(req);
		}

	} while (req);
}

static void RHBImplementationDef_perform_transmit(
		RHBImplementationDef *somThis,
		Environment *ev)
{
	boolean writes_wanted=0,clear_oneways=0,do_close=0;
	long release_count=0;

	RHBORB_guard(somThis)

/*	somThis->connection.write_posted=0;*/

	somThis->connection.writeable=1;	/* assume we are writeable */

	if (!somThis->connection.fd)
	{
		RHBImplementationDef_check_ending(somThis);

		RHBORB_unguard(somThis)

		RHBImplementationDef_interrupt_closed(somThis,ev);

		return;
	}

/*	if (!connection.writeable) printf("not writeable\n");*/

	if (somThis->connection.transmitting)
	{
/*		debug_somPrintf(("already transmitting\n"));*/

		RHBORB_unguard(somThis)

		return;
	}

	somThis->connection.transmitting++;

	while (somThis->connection.writeable)
	{
		if (somThis->transmitting.head)
		{
			RHBSendData *data=somThis->transmitting.head;
			octet *p=data->seekPtr;
			long i=data->toGo;
			RHBSocketRef fd=somThis->connection.fd;
			RHBSocketError sendError=0;

			writes_wanted=1;

			if (i > GIOP_LIMIT_XFER)
			{
				i=GIOP_LIMIT_XFER;
			}

			if (!i)
			{
				SOMD_bomb("zero bytes on transmit");
			}

			somThis->connection.writeable=0;

			if (fd)
			{
				i=RHBSocket_SendData(fd,&sendError,p,i);
			}
			else
			{
				break;
			}

			if (i > 0)
			{
				if (!RHBSocket_IsOneShot(fd))
				{
					somThis->connection.writeable=1;
				}

				data->toGo-=i;
				data->seekPtr+=i;

				if (!data->toGo)
				{
					somThis->transmitting.head=data->next;

					if (!data->next)
					{
						somThis->transmitting.tail=NULL;
					}

					SOMFree(data);

					/* need to drop impl usage at end of this */

					release_count++;
				}
			}
			else
			{
				if (sendError==SOMS_EWOULDBLOCK)
				{
					writes_wanted=1;
				}
				else
				{
					do_close=1;
				}
			}
		}
		else
		{
			if (!RHBRequestList_empty(&somThis->pending))
			{
				RHBRequest *request=NULL;

				do
				{
					request=RHBRequestList_pop(&somThis->pending);

					if (request)
					{
						RHBRequest_send_marshalled(request,ev,somThis);

						if (request->request.send_flags & INV_NO_RESPONSE)
						{
							RHBRequestList_add(&somThis->executing_oneway,request);

							clear_oneways=1;
						}
						else
						{
							RHBRequestList_add(&somThis->executing_normal,request);
						}
					}

				} while (request);
			}
			else
			{
				writes_wanted=0;
				clear_oneways=1;

				break;
			}
		}
	}

	RHBImplementationDef_writes_wanted(somThis,ev,writes_wanted);

	somThis->connection.transmitting--;

	RHBImplementationDef_check_ending(somThis);

	RHBImplementationDef_AddRef(somThis); release_count++;

	RHBORB_unguard(somThis)

	if (clear_oneways)
	{
		RHBImplementationDef_clear_executing_oneways(somThis,ev);
	}

	if (do_close)
	{
		RHBImplementationDef_interrupt_closed(somThis,ev);
	}

	while (release_count--)
	{
		RHBImplementationDef_Release(somThis);
	}
}

static void RHBImplementationDef_kick_transmitter(
		RHBImplementationDef *somThis,
		Environment *ev)
{
#ifdef USE_SELECT
	RHBImplementationDef_perform_transmit(somThis,ev);
#else
	#ifdef USE_PTHREADS
		RHBORB_guard(0)
		RHBImplementationDef_AddRef(somThis);
		#ifdef _PLATFORM_MACINTOSH_
			RHBImplementationDef_perform_transmit(somThis,ev);
		#else
			if (somThis->connection.fd) 
			{
				struct RHBSocket* fd=somThis->connection.fd;
				RHBSocket_AddRef(fd);
				RHBSocket_EnableWrites(fd,1);
				RHBSocket_Release(fd);
				fd=0;
			}
		#endif
		RHBImplementationDef_Release(somThis);
		RHBORB_unguard(0)
	#else
		#ifdef _WIN32
			RHBORB_guard(somThis)
			if (somThis->connection.fd)
			{
				if (somThis->connection.connected)
				{
					RHBSocket_EnableWrites(somThis->connection.fd,1);
				}
				else
				{
					debug_somPrintf(("write:: not connected yet\n"));
				}
			}
			RHBORB_unguard(somThis);
		#else
			RHBImplementationDef_AddRef(somThis);
			RHBImplementationDef_perform_transmit(somThis,ev);
			RHBImplementationDef_Release(somThis);
		#endif
	#endif
#endif
	RHBOPT_unused(ev)
}

static void RHBImplementationDef_interrupt_connected(
			RHBImplementationDef *somThis,
			Environment *ev)
{
/*	debug_somPrintf(("Connection made to %s:%d\n",
		somThis->address.host,
		(int)somThis->address.port));
*/
	RHBORB_guard(somThis)

	somThis->connection.writeable=1;
	somThis->connection.readable=0;
	somThis->connection.connected=1;
	somThis->connection.connecting=0;

	if (somThis->connection.resolver)
	{
		RHBResolverRef ref=somThis->connection.resolver;
		somThis->connection.resolver=NULL;
		if (ref) RHBResolver_Release(ref);
	}

	RHBORB_unguard(somThis)

#ifdef USE_PTHREADS
/*	RHBImplementationDef_interrupt_writeable(somThis,ev);*/
	RHBImplementationDef_kick_transmitter(somThis,ev);
#else
	RHBOPT_unused(ev)
#endif
}

struct RHBGIOPRequestStream_connect_retry
{
	RHBGIOPRequestStream *impl;
	RHBSocketRef fd;
	boolean locked;
	boolean doClose;
	Environment *ev;
};

RHBOPT_cleanup_begin(RHBGIOPRequestStream_connect_retry_cleanup,pv)

struct RHBGIOPRequestStream_connect_retry *data=pv;

	if (data->locked)
	{
		RHBORB_unguard(data->impl);
	}

	if (data->doClose)
	{
		RHBImplementationDef_interrupt_closed(data->impl,data->ev);
	}

	if (data->fd)
	{
		RHBSocket_Release(data->fd);
	}

	RHBImplementationDef_Release(data->impl);

RHBOPT_cleanup_end

static void RHBGIOPRequestStream_connect_retry(
	RHBGIOPRequestStream *somThis,
	Environment *ev)
{
	struct RHBGIOPRequestStream_connect_retry data={NULL,NULL,0,0};
	data.ev=ev;
	RHBOPT_cleanup_push(RHBGIOPRequestStream_connect_retry_cleanup,&data);
	RHBImplementationDef_AddRef(somThis);
	data.impl=somThis;

	data.doClose=1;

	while (!data.ev->_major)
	{
		unsigned long index;

		if (!data.locked)
		{
			data.locked=1;
			RHBORB_guard(somThis)
		}

		if (somThis->connection.fd)
		{
			RHBSocketRef fd=somThis->connection.fd;
			somThis->connection.fd=NULL;
			if (fd)
			{
				data.locked=0;
				RHBORB_unguard(somThis)
				RHBSocket_IsNonBlocking(fd,NULL);
				RHBSocket_Close(fd);
				RHBSocket_Release(fd);
				data.locked=1;
				RHBORB_guard(somThis)
			}
		}

		index=somThis->address.index++;

		if (index < somThis->address.addrSeq._length)
		{
			RHBSocketError errCode=0;

#ifdef _DEBUG_CONNECT
			somPrintf("trying address [%ld/%ld]...\n",
					(long)index,
					(long)somThis->address.addrSeq._length);

			{
				switch (somThis->address.addrSeq._buffer[index].family)
				{
				case AF_INET:
					{
						struct sockaddr_in *in=(void *)
							somThis->address.addrSeq._buffer[index].value._buffer;

						somPrintf("connect(AF_INET,%s:%d)\n",
							inet_ntoa(in->sin_addr),
							ntohs(in->sin_port));

					}
					break;
#ifdef HAVE_SOCKADDR_IN6
				case AF_INET6:
					{
						char buf[256];
						struct sockaddr_in6 *in=(void *)
							somThis->address.addrSeq._buffer[index].value._buffer;
						const char *p=inet_ntop(AF_INET6,
									&in->sin6_addr,
									buf,sizeof(buf));

						somPrintf("connect(AF_INET6,%s:%d)\n",
							p,
							ntohs(in->sin6_port));

					}
					break;
#endif
				}
			}
#endif

			data.doClose=0;

			somThis->connection.fd=RHBProtocol_CreateStream(
					somThis->orb->protocol,
					somThis->address.addrSeq._buffer[index].family);

			if (somThis->connection.fd)
			{
				boolean b=0;

				data.fd=somThis->connection.fd;

				RHBSocket_AddRef(data.fd);

				data.locked=0;
				RHBORB_unguard(somThis);

				/* this can still yield to socket thread... */

				RHBSocket_IsNonBlocking(data.fd,&somThis->sink.vtbl);

				b=RHBSocket_StartConnect(data.fd,&errCode,
						(struct sockaddr *)somThis->address.addrSeq._buffer[index].value._buffer,
						(RHBSocketLength)(somThis->address.addrSeq._buffer[index].value._length));

				if (b || 
					(errCode==SOMS_EWOULDBLOCK) ||
					(errCode==SOMS_EINPROGRESS))
				{
					if (b)
					{
						RHBImplementationDef_interrupt_connected(somThis,data.ev);
					}

					break;
				}

				{
					RHBSocketRef ref=data.fd;
					data.fd=NULL;
					RHBSocket_Release(ref);
				}
			}
		}
		else
		{
#ifdef _DEBUG_CONNECT
			somPrintf("run out of addresses to try [%ld/%ld]...\n",
					(long)index,
					(long)somThis->address.addrSeq._length);
#endif

			throw_RHBSocketError(data.ev,SOMDERROR_CannotConnect,__FILE__,__LINE__);

			data.doClose=1;
		}
	}

	RHBOPT_cleanup_pop();
}

struct RHBGIOPRequestStream_call_connect
{
	RHBGIOPRequestStream *somThis;
	RHBResolverRef resolver;
	boolean locked;
	boolean doClose;
	Environment ev;
};

RHBOPT_cleanup_begin(RHBGIOPRequestStream_call_connect_cleanup,pv)

	struct RHBGIOPRequestStream_call_connect *data=pv;

	if (data->locked)
	{
		RHBORB_unguard(somThis)
	}

	if (data->doClose)
	{
		RHBImplementationDef_interrupt_closed(data->somThis,&data->ev);
	}

	SOM_UninitEnvironment(&(data->ev));

	if (data->resolver)
	{
		RHBResolver_Release(data->resolver);
	}

	RHBImplementationDef_Release(data->somThis);

RHBOPT_cleanup_end

static void RHBLINK RHBGIOPRequestStream_call_connect(
	 RHBImplementationDef *somThis,
	 RHBSocketError errCode_arg,
	 _IDL_SEQUENCE_SOMD_NetworkAddress *hpp)
{
	RHBSocketError RHBOPT_volatile errCode=errCode_arg;
	_IDL_SEQUENCE_SOMD_NetworkAddress addrNull={0,0,NULL};
	struct RHBGIOPRequestStream_call_connect data={NULL,NULL,0,0};

	SOM_InitEnvironment(&(data.ev));

	data.locked=1;

	RHBOPT_cleanup_push(RHBGIOPRequestStream_call_connect_cleanup,&data);

	RHBORB_guard(somThis)

	data.resolver=somThis->connection.resolver;
	somThis->connection.resolver=NULL;

	data.somThis=somThis;
	RHBImplementationDef_AddRef(somThis);

	/* clear previous address if any... */

	if (somThis->address.addrSeq._buffer)
	{
		_IDL_SEQUENCE_SOMD_NetworkAddress addrTemp;
		addrTemp=somThis->address.addrSeq;
		somThis->address.addrSeq=addrNull;

		SOMD_FreeType(&data.ev,&addrTemp,somdTC_sequence_SOMD_NetworkAddress);
	}

	if (hpp)
	{
		somThis->address.index=0;
		/* get a copy of the address information... */ 
		RHBCDR_copy_from(somdTC_sequence_SOMD_NetworkAddress,
						&data.ev,
						&somThis->address.addrSeq,
						hpp);
	}

	RHBSOM_Trace("RHBImplementationDef_call_connect")

	if ((!somThis->connection.connected) && (!somThis->connection.fd))
	{
		if (somThis->orb->closing)
		{
			if (!errCode)
			{
				errCode=SOMDERROR_SocketClose;
			}
		}

		if (errCode)
		{
			data.doClose=1;

			throw_RHBSocketError(&data.ev,errCode,__FILE__,__LINE__);
		}
		else
		{
			RHBGIOPRequestStream_connect_retry(somThis,&data.ev);
		}
	}

	RHBOPT_cleanup_pop();
}

struct RHBImplementationDef_begin_connect
{
	boolean do_close;
	boolean locked;
	RHBResolverRef resolver;
	Environment *ev;
	RHBImplementationDef *somThis;
};

RHBOPT_cleanup_begin(RHBImplementationDef_begin_connect_cleanup,pv)

struct RHBImplementationDef_begin_connect *data=pv;

	if (data->locked)
	{
		RHBORB_unguard(somThis)
	}

	if (data->resolver)
	{
		RHBResolver_Release(data->resolver);
	}

	if (data->do_close)
	{
		RHBImplementationDef_interrupt_closed(data->somThis,data->ev);
	}

RHBOPT_cleanup_end

static boolean RHBImplementationDef_begin_connect(
		RHBImplementationDef *somThis,Environment *ev)
{
struct RHBImplementationDef_begin_connect data={0,0,NULL,NULL,NULL};
boolean RHBOPT_volatile result=0;

	data.somThis=somThis;
	data.ev=ev;

	RHBOPT_cleanup_push(RHBImplementationDef_begin_connect_cleanup,&data);

	if (!somThis->connection.is_listener)
	{
		data.locked=1;
		RHBORB_guard(somThis)

		if (!somThis->is_closing)
		{
			if (!somThis->orb->protocol)
			{
				RHBOPT_throw_StExcep(ev,COMM_FAILURE,SocketError,NO);
			}
			else
			{
				if (somThis->connection.connecting)
				{
					result=1;
				}
				else
				{
					somThis->connection.connecting=1;

					somThis->connection.resolver=RHBProtocol_CreateResolver(
								somThis->orb->protocol,
								somThis->address.host,
								somThis->address.port);

					if (somThis->connection.resolver) 
					{
						RHBResolver_Start(somThis->connection.resolver,
								somThis,RHBGIOPRequestStream_call_connect);

						result=1;
					}
					else
					{
						data.do_close=1;
					}

					if (somThis->connection.connected)
					{
						data.resolver=somThis->connection.resolver;
						somThis->connection.resolver=NULL;
					}
				}
			}
		}
	}

	RHBOPT_cleanup_pop();

	return result;
}


static void RHBImplementationDef_interrupt_writeable(
		RHBImplementationDef *somThis,
		Environment *ev)
{
	somThis->connection.writeable=1;

	RHBImplementationDef_perform_transmit(somThis,ev);
}

static void rhbGIOPRequestStream_cancel_requests(
	 RHBGIOPRequestStream *somThis,
	 Environment *ev,
	 RHBRequestList *list)
{
	RHBSOMD_must_be_guarded

	if (ev)
	{
		while (list->first)
		{
			RHBRequest *rp=list->first;

			RHBRequest_AddRef(rp);

			RHBRequestList_remove(list,rp);

			RHBRequest_SetComplete(rp,ev);

			RHBRequest_Release(rp);
		}
	}
	else
	{
		Environment ev2;
		SOM_InitEnvironment(&ev2);
		RHBOPT_throw_StExcep((&ev2),COMM_FAILURE,SocketError,MAYBE);
		rhbGIOPRequestStream_cancel_requests(somThis,&ev2,list);
		SOM_UninitEnvironment(&ev2);
	}
}

static void rhbGIOPRequestStream_interrupt_closed(
		 RHBImplementationDef *somThis,
		Environment *ev)
{
	RHBSocketRef fd=NULL;
	int extra_release=0; /* this gets set if was accepted */
	RHBResolver *resolver=NULL;

	RHBORB_guard(somThis)

	RHBImplementationDef_free_transmit_queue(somThis);

	resolver=somThis->connection.resolver;
	somThis->connection.resolver=NULL;

	RHBImplementationDef_check_ending(somThis);

	if (somThis->is_closing)
	{
		RHBORB_unguard(somThis)

		return;
	}

	somThis->is_closing=1;

	fd=somThis->connection.fd;
	somThis->connection.fd=NULL;

	somThis->connection.connected=0;
	somThis->connection.connecting=0;

	if (fd)
	{
		RHBSocket_Shutdown(fd,0,2);
		RHBSocket_Close(fd);
		RHBSocket_Release(fd);
		fd=NULL;
	}

	/*
		free the receive and send buffers


	*/

	RHBImplementationDef_free_transmit_queue(somThis);
	RHBImplementationDef_free_receive_buffer(somThis);
	
	if (somThis->outstanding_request_list)
	{
		RHBServerRequest *req=somThis->outstanding_request_list;

		RHBServerRequest_AddRef(req);

		while (req)
		{
			RHBServerRequest *next=req->next;

			if (next) RHBServerRequest_AddRef(next);

			RHBServerRequest_cancel(req,ev);

			RHBServerRequest_Release(req);

			req=next;
		}
	}

	rhbGIOPRequestStream_cancel_requests(somThis,ev,&somThis->pending);
	rhbGIOPRequestStream_cancel_requests(somThis,ev,&somThis->executing_normal);
	rhbGIOPRequestStream_cancel_requests(somThis,ev,&somThis->executing_oneway);

	extra_release+=somThis->server.release_on_close;
	somThis->server.release_on_close=0;

	somThis->is_closing=0;

	RHBORB_unguard(somThis)

	while (extra_release--)
	{
		RHBImplementationDef_Release(somThis);
	}

	if (resolver)
	{
		RHBResolver_Cancel(resolver);
		RHBResolver_Release(resolver);
	}
}

static ORBStatus rhbGIOPRequestStream_queue_request(
			RHBImplementationDef *somThis,
			Environment *ev,
			RHBRequest *request)
{
	ORBStatus status=0;

	RHBORB_guard(somThis)

	RHBRequestList_add(&somThis->pending,request);

	if (somThis->connection.connected)
	{
		RHBORB_unguard(somThis)

		RHBImplementationDef_kick_transmitter(somThis,ev);
	}
	else
	{
		RHBORB_unguard(somThis)

		if (!RHBImplementationDef_begin_connect(somThis,ev))
		{
			RHBORB_guard(somThis)

			RHBOPT_throw_StExcep(ev,COMM_FAILURE,SocketError,NO)

			if (RHBRequestList_remove(&somThis->pending,request))
			{
				debug_somPrintf(("Was pending - now cancelled\n"));
				RHBORB_unguard(somThis)

				RHBRequest_SetComplete(request,ev);

				status=0xffffffff;
			}
			else
			{
				RHBRequestList *list=
						(request->request.send_flags & INV_NO_RESPONSE)
						?
						&somThis->executing_oneway
						:
						&somThis->executing_normal;

				if (RHBRequestList_remove(list,request))
				{
					debug_somPrintf(("Was executing - now cancelled\n"));

					RHBORB_unguard(somThis)

					RHBRequest_SetComplete(request,ev);

					status=0xffffffff;
				}
				else
				{
					debug_somPrintf(("Unknown state..\n"));

					RHBORB_unguard(somThis)
				}
			}

			if (ev->_major) somExceptionFree(ev);
		}
	}

	return status;
}

static void RHBImplementationDef_write_giop(
		   RHBImplementationDef *somThis,
		   Environment *ev,
		   RHBSendData *data)
{
	somd_atomic_inc(&somThis->lUsage);

	data->seekPtr=data->_buffer;
	data->next=NULL;

	if (somThis->transmitting.tail)
	{
		somThis->transmitting.tail->next=data;
		somThis->transmitting.tail=data;
	}
	else
	{
		somThis->transmitting.head=data;
		somThis->transmitting.tail=data;
	}
}

static boolean rhbGIOPRequestStream_accept_requests(RHBGIOPRequestStream *somThis)
{
	if (somThis->orb->closing) 
	{
		debug_somPrintf(("can't accept request as orb is closing\n"));

		return 0;
	}

	if (somThis->server.data)
	{
		if ((!somThis->is_closing)
			&&
			(somThis->connection.connected)
			&&
			(!somThis->server.data->deactivating))
		{
			return 1;
		}
	}

	return 0;
}

static void RHBImplementationDef_received_message(
			RHBImplementationDef *somThis,
			Environment *ev,
			GIOP_MessageHeader *GIOP_header,
			boolean doSwap,
			_IDL_SEQUENCE_octet *dataPtr)
{
#ifdef somNewObject
	RHBORBStream_GIOPInput SOMSTAR input=somNewObject(RHBORBStream_GIOPInput);
#else
	RHBORBStream_GIOPInput SOMSTAR input=RHBORBStream_GIOPInputNew();
#endif

	RHBORBStream_GIOPInput__set_c_orb(input,ev,somThis->orb);
	RHBORBStream_GIOPInput_stream_init_data(input,ev,dataPtr,doSwap);

	dataPtr->_length=0;
	dataPtr->_buffer=NULL;
	dataPtr->_maximum=0;
	dataPtr=NULL;

	switch (GIOP_header->message_type)
	{
		case GIOP_Request:
			if (somThis)
			{
				RHBORB_guard(somThis)

				if (rhbGIOPRequestStream_accept_requests(somThis))
				{
					GIOP_RequestHeader_1_0 request_header;
					RHBServerRequest *req;
					TypeCode tc=somdTC_GIOP_RequestHeader_1_0;

					memset(&request_header,0,sizeof(request_header));
	
					RHBCDR_unmarshal(&somThis->unmarshal_filter,ev,
							&request_header,
							tc,
							input);

					req=RHBServerRequestNew(ev,somThis,GIOP_header,&request_header,input);

					RHBORB_unguard(somThis)

					if (req)
					{
#ifdef USE_THREADS
						RHBORB_StartThreadTask(somThis->orb,&req->task);
#else
						RHBServerRequest_add_to_impl_queue(
							req,ev,
							req->exec_st);
#endif
					}
					else
					{
						Environment ev2;
						SOM_InitEnvironment(&ev2);
						SOMD_FreeType(
								&ev2,
								&request_header,tc);
						SOM_UninitEnvironment(&ev2);
					}
				}
				else
				{
					RHBORB_unguard(somThis)
				}
			}

			break;
		case GIOP_Reply:
			{
				TypeCode tc=somdTC_GIOP_ReplyHeader_1_0;
				GIOP_ReplyHeader_1_0 reply_header;
				RHBRequest *request;

				/* need to decode the reply */
				/* find the appropriate request by sequence number */

				RHBCDR_unmarshal(
						&somThis->unmarshal_filter,
						ev,
						&reply_header,
						tc,
						input);

				RHBORB_guard(somThis)

				request=somThis->executing_normal.first;

				while (request)
				{
					if (request->request.lSequence==reply_header.request_id)
					{
						break;
					}
					else
					{
						request=request->next;
					}
				}

				if (request)
				{
					RHBRequest_AddRef(request);

					if (RHBRequestList_remove(&somThis->executing_normal,request))
					{
						RHBImplementationDef_check_ending(somThis);

						RHBORB_unguard(somThis)

						RHBRequest_reply_received(request,ev,GIOP_header,&reply_header,input);

						RHBRequest_Release(request);

						return;
					}
					else
					{
						SOMD_bomb("not in the list");
					}

					RHBRequest_Release(request);
				}
				else
				{
					RHBSOM_Trace("Does not match with any outstanding requests")
				}

				RHBORB_unguard(somThis)
			}
 
			break;
		case GIOP_CancelRequest:
/*			somPrintf("GIOP_CancelRequest\n");*/
			if (!somThis->is_closing)
			{
				TypeCode tc=somdTC_GIOP_CancelRequestHeader;
				GIOP_CancelRequestHeader cancel_request_header;
				RHBServerRequest *req;

				RHBCDR_unmarshal(
						&somThis->unmarshal_filter,
						ev,
						&cancel_request_header,
						tc,
						input);

				debug_somPrintf(("GIOP_CancelRequest(%ld)\n",cancel_request_header.request_id));

				RHBORB_guard(somThis)

				req=somThis->outstanding_request_list;

				while (req)
				{
					if (req->header.request_id==cancel_request_header.request_id)
					{
						RHBServerRequest_AddRef(req);

						break;
					}

					req=req->next;
				}

				RHBORB_unguard(somThis)

				if (req)
				{
					RHBServerRequest_cancel(req,ev);
					RHBServerRequest_Release(req);
				}
			}
			break;
		case GIOP_LocateRequest:
			RHBSOM_Trace("GIOP_LocateRequest")

			{
				TypeCode tc=somdTC_GIOP_LocateRequestHeader;
				GIOP_LocateRequestHeader locate_request_header;
				RHBLocateRequest *req=NULL;

				memset(&locate_request_header,0,sizeof(locate_request_header));

				RHBCDR_unmarshal(
						&somThis->unmarshal_filter,
						ev,
						&locate_request_header,
						tc,
						input);

				if (somThis->is_closing 
					|| 
					(impl_is_deactivating(somThis)))
				{
/*					somPrintf("I am closing, want to bounce this locate request\n");*/

					SOMD_FreeType(
						ev,
						&locate_request_header,tc);

					tc=NULL;
				}

				if (tc)
				{
					req=RHBLocateRequestNew(ev,
						somThis,
						GIOP_header,
						&locate_request_header,
						input);

					if (req)
					{
						/* this would actually be done in another thread,,,, */

#ifdef USE_THREADS
						RHBORB_StartThreadTask(somThis->orb,&req->task);
#else
						RHBServerRequest_add_to_impl_queue(
							req,ev,
							req->exec_st);
#endif
					}
				}
			}
			break;
		case GIOP_LocateReply:
			debug_somPrintf(("GIOP_LocateReply\n"));
			break;
		case GIOP_CloseConnection:
			debug_somPrintf(("GIOP_CloseConnection\n"));
			break;
		case GIOP_MessageError:
			debug_somPrintf(("GIOP_MessageError\n"));
			break;
	}

	SOMObject_somFree(input);
}

static void RHBImplementationDef_kick_receiver(RHBImplementationDef *somThis,Environment *ev)
{
	boolean close_on_exit=0;

	if (somThis->is_closing)
	{
		return;
	}

	if (!somThis->connection.connected)
	{
		return;
	}

	RHBOPT_ASSERT(!somThis->already_reading)

	if (somThis->already_reading)
	{
		debug_somPrintf(("already reading...%d\n",somThis->already_reading));

		SOMD_bomb("already reading");

		return;
	}

	RHBORB_guard(somThis)

	if (somThis->is_closing)
	{
		RHBORB_unguard(somThis)

		return;
	}

	RHBOPT_ASSERT(!somThis->already_reading)

	if (somThis->already_reading)
	{
		debug_somPrintf(("already reading...%d\n",somThis->already_reading));

		SOMD_bomb("already reading");

		RHBORB_unguard(somThis)

		return;
	}

	somThis->already_reading++;

	while (somThis->connection.readable && (!close_on_exit) && !ev->_major)
	{
		if (somThis->receiving._maximum)
		{	
			RHBSocketError recvError=0;
			long i=somThis->receiving._maximum-somThis->receiving._length;
			RHBSocketRef fd=somThis->connection.fd;

			if (i > GIOP_LIMIT_XFER)
			{
				i=GIOP_LIMIT_XFER;
			}

			if (!fd)
			{
				break;
			}

			RHBSocket_AddRef(fd);

			i=RHBSocket_RecvData(fd,&recvError,
				&somThis->receiving._buffer[
						somThis->receiving._length],i);

			if (RHBSocket_IsOneShot(fd))
			{
				somThis->connection.readable=0;
			}

			RHBSocket_Release(fd);

			fd=NULL;

			if (i > 0)
			{
				somThis->receiving._length+=i;

				/* now let's see if the read is complete.... */

				if (somThis->receiving._length==somThis->receiving._maximum)
				{
					if (somThis->receiving_header)
					{
#ifdef somNewObject
						RHBORBStream_GIOPInput SOMSTAR input=somNewObject(RHBORBStream_GIOPInput);
#else
						RHBORBStream_GIOPInput SOMSTAR input=RHBORBStream_GIOPInputNew();
#endif
						/* decode the header... */

						somThis->receive_doSwap=(boolean)((
								(somThis->receiving._buffer[6] & 1)==
									RHBCDR_machine_type()
								) ? 0 : 1);


						RHBORBStream_GIOPInput__set_c_orb(input,ev,somThis->orb);
						RHBORBStream_GIOPInput_stream_init_data(input,ev,&somThis->receiving,somThis->receive_doSwap);
						somThis->receiving._length=0;
						somThis->receiving._buffer=NULL;
						somThis->receiving._maximum=0;

						RHBCDR_unmarshal(
								&somThis->unmarshal_filter,
								ev,
								&somThis->GIOP_header,
								somdTC_GIOP_MessageHeader,
								input);

						/* one small adjustment, the octet message_type will be 
								one out due to soms enum being one out */

						somThis->GIOP_header.message_type=(octet)RHBCDR_GIOPenumToIDLenum(somThis->GIOP_header.message_type,0,0);

						SOMCDR_CDRInputStream_somFree(input);

						input=NULL;

/*						somPrintf("rx, {m=%4.4s,v={%d,%d},f=%d,t=%d,s=%ld}\n",
								somThis->GIOP_header.magic,
								(int)somThis->GIOP_header.protocol_version.major,
								(int)somThis->GIOP_header.protocol_version.minor,
								(int)somThis->GIOP_header.flags,
								(int)somThis->GIOP_header.message_type,
								somThis->GIOP_header.message_size);*/

						somThis->receiving._buffer=NULL;
						somThis->receiving._maximum=somThis->GIOP_header.message_size;
						somThis->receiving._length=0;

						somThis->receiving_header=0;

						if (memcmp(somThis->GIOP_header.magic,"GIOP",4))
						{
/*							SOMD_bomb("not a GIOP message");*/

							close_on_exit=1;

							break;
						}
						else
						{
							if (somThis->receiving._maximum)
							{
								somThis->receiving._buffer=
									(octet *)
										SOMMalloc(
											somThis->receiving._maximum);

								if (!somThis->receiving._buffer)
								{
									close_on_exit=1;
									break;
								}
							}
						}
					}
					else
					{
						_IDL_SEQUENCE_octet rx=somThis->receiving;
						GIOP_MessageHeader header=somThis->GIOP_header;
						boolean doSwap=somThis->receive_doSwap;
				
						somThis->receiving._length=0;
						somThis->receiving._maximum=0;
						somThis->receiving._buffer=NULL;

						RHBORB_unguard(somThis);

						RHBImplementationDef_received_message(
							somThis,
							ev,
							&header,doSwap,
							&rx);

						if (rx._buffer)
						{
							SOMFree(rx._buffer);
							rx._buffer=NULL;
						}

						RHBORB_guard(somThis)

						if (ev->_major)
						{
							close_on_exit=1;

							break;
						}
					}
				}
			}
			else
			{
				somThis->connection.readable=0;

				if (i)
				{
					/* should check errno here.... */

					if (ev->_major || 
						(recvError!=SOMS_EWOULDBLOCK))
					{
						close_on_exit=1;

						break;
					}
				}
				else
				{
					close_on_exit=1;

					break;
				}
			}
		}	
		else
		{
			close_on_exit=1;

			/* this is a new notification of data */

			if (somThis->receiving._buffer)
			{
				SOMD_bomb("receiving buffer still allocated");
			}

			if (ev->_major)
			{
				somThis->receiving._maximum=0;
				somThis->receiving._length=0;
				somThis->receiving._buffer=NULL;
			}
			else
			{
				somThis->receiving._maximum=12;
				somThis->receiving._length=0;
				somThis->receiving._buffer=SOMMalloc(somThis->receiving._maximum);

				if (somThis->receiving._buffer)
				{
					somThis->receiving_header=1;

					close_on_exit=0;
				}
				else
				{
					somThis->receiving._maximum=0;
				}
			}
		}
	}

	somThis->already_reading--;

	RHBORB_unguard(somThis);

	if (close_on_exit || ev->_major)
	{
		somExceptionFree(ev);

		RHBImplementationDef_interrupt_closed(somThis,ev);

	}
}

static void RHBImplementationDef_interrupt_readable(
		RHBImplementationDef *somThis,
		Environment *ev)
{
	somThis->connection.readable=1;

	RHBImplementationDef_kick_receiver(somThis,ev);
}

static void rhbGIOPRequestStream_remove_outstanding(
			RHBImplementationDef *somThis,
			RHBServerRequest *req)
{
	RHBORB_guard(somThis)

	/* now remove as we are being processed... */

	if (somThis->outstanding_request_list==req)
	{
		somThis->outstanding_request_list=req->next;
	}
	else
	{
		RHBServerRequest *r=somThis->outstanding_request_list;

		if (r)
		{
			while (r->next != req)
			{
				r=r->next;

				if (!r) break;
			}

			if (r) r->next=req->next;
		}
	}

	RHBImplementationDef_check_ending(somThis);

	RHBORB_unguard(somThis)
}

static void rhbGIOPRequestStream_add_outstanding(
			RHBImplementationDef *somThis,
			RHBServerRequest *req)
{
	RHBORB_guard(somThis)

	RHBOPT_ASSERT(somThis->server.data)

	if (somThis->server.data)
	{
		req->next=somThis->outstanding_request_list;
		somThis->outstanding_request_list=req;
	}

	RHBORB_unguard(somThis)
}

struct familyList
{
	short family;
	struct RHBSocketListener *fd;
	unsigned short port;
	boolean has_family;
};

static boolean rhbGIOPRequestStream_begin_listen_family(
	RHBImplementationDef *somThis,
	Environment *ev,
	struct familyList *familyList,
	int familyCount,
	unsigned short port_wanted)
{
	boolean ports_all_match=1;
	int familyIndex=familyCount;

	while (familyIndex--)
	{
		short family=familyList[familyIndex].family;
		struct RHBSocketListener *fd=RHBSocketListenerNew(somThis->orb);

		if (fd)
		{
			RHBORB_sockaddr addr;
			int j=sizeof(addr);
			int addrlen=0;

			familyList[familyIndex].has_family=1;

			memset(&addr,0,j);

			addr.u.generic.sa_family=family;

			switch (addr.u.generic.sa_family)
			{
			case AF_INET:
				addr.u.ipv4.sin_port=ntohs(port_wanted);
				addr.u.ipv4.sin_addr.s_addr=htonl(INADDR_ANY);
	#ifdef HAVE_SOCKADDR_SA_LEN
				addr.u.ipv4.sin_len=sizeof(addr.u.ipv4);
	#endif
				addrlen=sizeof(addr.u.ipv4);
				break;
	#ifdef HAVE_SOCKADDR_IN6
			case AF_INET6:
				{
#ifdef USE_IN6ADDR_ANY_INIT_BRACES1
					static struct in6_addr addrInit={IN6ADDR_ANY_INIT};
#else
#ifdef USE_IN6ADDR_ANY_INIT_BRACES2
					static struct in6_addr addrInit={{IN6ADDR_ANY_INIT}};
#else
#	ifdef HAVE_IN6_ADDR
					static struct in6_addr addrInit=IN6ADDR_ANY_INIT;
#	else
					static struct in_addr6 addrInit={{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
#	endif
#endif
#endif
	#ifdef HAVE_SOCKADDR_SA_LEN
					addr.u.ipv6.sin6_len=sizeof(addr.u.ipv6);
	#endif
					addr.u.ipv6.sin6_port=ntohs(port_wanted);
					addr.u.ipv6.sin6_addr=addrInit;
					addrlen=sizeof(addr.u.ipv6);
				}
				break;
	#endif
			}

			if (fd->vtbl2->begin_listen(fd,
					somThis->server.data,
					family,
					&addr.u.generic,addrlen))
			{
				familyList[familyIndex].port=fd->port;
				familyList[familyIndex].fd=fd;
				port_wanted=fd->port;

				debug_somPrintf(("listen ok,family=%d,port=%d\n",
						family,
						fd->port));
			}
			else
			{
				debug_somPrintf(("listen bad,family=%d,port=%d\n",
						family,
						port_wanted));

				fd->vtbl->Release(&fd->vtbl);

				ports_all_match=0;
			}
		}
	}

	if (ports_all_match)
	{
		unsigned short actual_port=0;

		familyIndex=familyCount;

		while (familyIndex--)
		{
			if (familyList[familyIndex].has_family)
			{
				if (actual_port)
				{
					if (actual_port!=familyList[familyIndex].port)
					{
						ports_all_match=0;
						break;
					}
				}
				else
				{
					actual_port=familyList[familyIndex].port;
				}
			}
		}

		if (ports_all_match)
		{
			debug_somPrintf(("port choosen was %d\n",port_wanted));

			somThis->address.port=actual_port;
		}
	}

	if (!ports_all_match)
	{
		familyIndex=familyCount;

		while (familyIndex--)
		{
			struct RHBSocketListener *fd=familyList[familyIndex].fd;

			familyList[familyIndex].fd=0;

			if (fd)
			{
				fd->vtbl2->end_listen(fd);
				fd->vtbl->Release(&fd->vtbl);
			}
		}
	}

	return ports_all_match;
}


static void rhbGIOPRequestStream_begin_listen(
	 RHBImplementationDef *somThis,
	 Environment *ev,
	 char *impl_id)
{
	RHBORB_guard(somThis)

	if (!somThis->orb->protocol)
	{
		RHBORB_unguard(somThis)

		RHBOPT_throw_StExcep(ev,COMM_FAILURE,SocketError,NO);

		return;
	}

	somThis->connection.is_listener=1;

	while (somThis->connection.is_listener && !ev->_major)
	{
#if defined(IPV6_WILL_ACCEPT_IPV4)
		if (!ev->_major)
		{
			struct familyList familyList[]={{AF_INET6,NULL,0,0}};
			
			if (rhbGIOPRequestStream_begin_listen_family(somThis,ev,
					familyList,sizeof(familyList)/sizeof(familyList[0]),
					somThis->address.port))
			{
				break;
			}

			if (ev->_major) 
			{
				break;
			}
		}
#else
#	if defined(IPV6_WILL_BIND) && defined(HAVE_SOCKADDR_IN6)
		{
			int n=32;
			boolean is_up=0;

			while ((n--)&&(!ev->_major))
			{
				struct familyList familyList[]={{AF_INET,NULL,0,0},{AF_INET6,NULL,0,0}};

				if (rhbGIOPRequestStream_begin_listen_family(somThis,ev,
						familyList,sizeof(familyList)/sizeof(familyList[0]),
						somThis->address.port))
				{
					is_up=1;

					break;
				}

				if (ev->_major) 
				{
					break;
				}

				if (somThis->address.port)
				{
					RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,SocketBind,NO);

					break;
				}
			}

			if (is_up)
			{
				break;
			}
		}
#	endif
#endif

		if (!ev->_major)
		{
			struct familyList familyList[]={{AF_INET,NULL,0,0}};

			if (rhbGIOPRequestStream_begin_listen_family(somThis,ev,
					familyList,sizeof(familyList)/sizeof(familyList[0]),
					somThis->address.port))
			{
				break;
			}

			if (ev->_major) 
			{
				break;
			}
		}

		if (ev->_major) 
		{
			break;
		}

		RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,SocketBind,NO);

		break;
	}

	if (!somThis->server.data->listener_list)
	{
		RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,SocketBind,NO);
	}

	if (!ev->_major)
	{
		if (impl_id)
		{
			/* in theory, should only
				change the hostname if was not already given
				for example by the original IOR */

			if (somThis->address.host)
			{
				if (!somThis->address.host[0])
				{
					SOMFree(somThis->address.host);
					somThis->address.host=NULL;
				}
			}

			if (!somThis->address.host)
			{
				char *host=RHBORB_get_HOSTNAME(somThis->orb);

				if (host)
				{
					if (somThis->address.host)
					{
						if (strcmp(host,somThis->address.host))
						{
							SOMFree(somThis->address.host);
							somThis->address.host=NULL;
						}
					}

					if (!somThis->address.host)
					{
						somThis->address.host=somd_dupl_string(host);
					}
				}
			}
		}
	}

	RHBORB_unguard(somThis)
}

static void RHBImplementationDef_accepted(
	 RHBImplementationDef *somThis,
	 Environment *ev,
	 RHBServerData *server_data,
	 RHBSocketRef fd,
	 struct sockaddr *addr,
	 struct sockaddr *server_addr)
{
	boolean valid=1;

	RHBOPT_ASSERT(server_data)

	if (!fd)
	{ 
		debug_somPrintf(("%s:%d, no fd\n",__FILE__,__LINE__)); 
		return; 
	}

	if (ev->_major)
	{ 
		debug_somPrintf(("%s:%d, exception already raised\n",__FILE__,__LINE__)); 
		return; 
	}

	if (!addr)
	{ 
		debug_somPrintf(("%s:%d, no addr\n",__FILE__,__LINE__)); 
		return; 
	}

	if (!server_addr) 
	{ 
		debug_somPrintf(("%s:%d, no server_addr\n",__FILE__,__LINE__)); 
		return; 
	}
	/* this is ALL guarded by the interrupt accept routine */

	RHBSocket_AddRef(fd);
	RHBImplementationDef_AddRef(somThis);

	somThis->server.data=server_data;
	somThis->server.data->lpVtbl->AddRef(server_data);

	{
/*		octet *op;

		op=(octet *)&addr->sin_addr;

		somPrintf("IIOP: accepted from %d.%d.%d.%d:%d\n",
			(int)op[0],(int)op[1],(int)op[2],(int)op[3],
			(int)ntohs(addr->sin_port));

		op=(octet *)&server_addr->sin_addr;

		somPrintf("IIOP: host address  %d.%d.%d.%d:%d\n",
			(int)op[0],(int)op[1],(int)op[2],(int)op[3],
			(int)ntohs(server_addr->sin_port));
*/
/*		RHBSocket_dump(fd);*/
	}


	somThis->connection.fd=fd;

/*	RHBImplementationDef_AddRef(listener);
	somThis->server.accepted_from=listener;*/
	somThis->server.release_on_close++;
	somThis->connection.is_server=1;
	somThis->connection.connected=1;

	if (addr)
	{
/*		if (somThis->server.client.name)
		{
			SOMFree(somThis->server.client.name);
			somThis->server.client.name=0;
		}
*/
		switch (addr->sa_family)
		{
		case AF_INET:
			{
				struct sockaddr_in *a=(void *)addr;
				somThis->server.client.addr.u.ipv4.sin_family=a->sin_family;
				somThis->server.client.addr.u.ipv4.sin_port=a->sin_port;;
				somThis->server.client.addr.u.ipv4.sin_addr=a->sin_addr;
			}
			break;
#ifdef HAVE_SOCKADDR_IN6
		case AF_INET6:
			{
				struct sockaddr_in6 *a=(void *)addr;
				somThis->server.client.addr.u.ipv6.sin6_family=a->sin6_family;
				somThis->server.client.addr.u.ipv6.sin6_port=a->sin6_port;;
				somThis->server.client.addr.u.ipv6.sin6_addr=a->sin6_addr;
				somThis->server.client.addr.u.ipv6.sin6_flowinfo=a->sin6_flowinfo;
			}
			break;
#endif
		default:
			debug_somPrintf(("%s:%d, I don't know what address the client is from",
					__FILE__,__LINE__));
			valid=0;
			break;
		}
	}
	else
	{
		memset(&somThis->server.client,0,sizeof(somThis->server.client));
	}

	if (server_addr)
	{
		if (somThis->address.host)
		{
			SOMD_bomb("why did this have a previous name?");
			SOMFree(somThis->address.host);
			somThis->address.host=0;
		}

		switch (server_addr->sa_family)
		{
		case AF_INET:
			{
				char buf[32];
				struct sockaddr_in *addr=(void *)server_addr;
				octet *op=(void *)&addr->sin_addr.s_addr;

				somThis->server.addr.u.ipv4=*addr;
				somThis->address.port=ntohs(addr->sin_port);
	
#ifdef HAVE_SNPRINTF
				snprintf
#else
				sprintf
#endif
					(buf,
#ifdef HAVE_SNPRINTF
					sizeof(buf),
#endif
					"%d.%d.%d.%d",
					(int)op[0],
					(int)op[1],
					(int)op[2],
					(int)op[3]);

				somThis->address.host=somd_dupl_string(buf);
			}
			break;
#ifdef HAVE_SOCKADDR_IN6
		case AF_INET6:
			{
				char buf[256];
				struct sockaddr_in6 *addr=(void *)server_addr;

				somThis->server.addr.u.ipv6=*addr;

				somThis->address.port=ntohs(addr->sin6_port);
	
				inet_ntop(addr->sin6_family,&addr->sin6_addr,buf,sizeof(buf));
				somThis->address.host=somd_dupl_string(buf);
			}
			break;
#endif
		default:
			debug_somPrintf(("%s:%d, I don't know what address I am exposed as",
					__FILE__,__LINE__));
			valid=0;
			break;
		}
	}

	/* I already have a count while I am connected .. */

	if (valid)
	{
		RHBSocket_IsNonBlocking(fd,&somThis->sink.vtbl);

#ifdef USE_PTHREADS
		RHBImplementationDef_kick_transmitter(somThis,ev);
#else
		RHBOPT_unused(ev)
#endif
	}
	else
	{
		somThis->connection.fd=NULL;
		RHBSocket_Release(fd);
		RHBImplementationDef_interrupt_closed(somThis,ev);
	}
}

static void RHBImplementationDef_interrupt_accept(
	RHBImplementationDef *somThis,
	RHBSocketRef fd_listen,
	Environment *ev)
{
	boolean do_repeat=0;

	/* this grabbing of the mutex is to ensure that another 
			thread is not still calling 'listen' */

	do
	{
		RHBSocketRef fd;
		RHBORB_sockaddr addr,sock_name;
		RHBSocketLength i=sizeof(addr),i2=sizeof(sock_name);
		RHBImplementationDef *child;
		RHBSocketError acceptError=0;

		RHBORB_guard(somThis)

		memset(&addr,0,i);
		memset(&sock_name,0,i2);

		fd=RHBSocket_Accept(
				fd_listen,&acceptError,
				&addr.u.generic,&i,
				&sock_name.u.generic,&i2);

		do_repeat=(boolean)(RHBSocket_IsOneShot(fd_listen) ? 0 : 1);

		if (!fd)
		{
			RHBORB_unguard(somThis)

			somExceptionFree(ev);

			return;
		}

		if ((!somThis->server.data) || (somThis->orb->closing) || (ev->_major))
		{
			RHBORB_unguard(somThis)

			if (fd) 
			{
				RHBSocket_Close(fd);
				RHBSocket_Release(fd);
			}

			somExceptionFree(ev);

			return;
		}

		child=RHBGIOPRequestStreamNew(somThis->orb,NULL);

		if (child) 
		{
			RHBImplementationDef_accepted(
				child,ev,
				somThis->server.data,
				fd,
				&addr.u.generic,
				&sock_name.u.generic);
		}

		RHBORB_unguard(somThis)

		if (child) RHBImplementationDef_Release(child);

		RHBSocket_Release(fd);

	} while (do_repeat);
}

static void rhbGIOPRequestStream_end_listen(
   RHBImplementationDef *somThis,
   Environment *ev)
{
	RHBORB_guard(0)

	while (somThis->server.data)
	{
		struct RHBSocketListener *p=somThis->server.data->listener_list;
		if (p)
		{
			somThis->server.data->listener_list=p->next;
			p->server=NULL;

			RHBORB_unguard(0)

			p->vtbl2->end_listen(p);
			p->vtbl->Release(&p->vtbl);

			RHBORB_guard(0)
		}
		else
		{
			break;
		}
	}

	RHBORB_unguard(0)

	RHBImplementationDef_interrupt_closed(somThis,ev);

/*	somPrintf("done end listen\n");*/
}

static void rhbGIOPRequestStream_debug_printf(
		RHBImplementationDef *somThis,
		Environment *ev)
{
	RHBOPT_unused(ev)

	if (!somThis->lUsage) SOMD_bomb("eh?");

	dump_somPrintf(("** lUsage=%ld** \n",(long)somThis->lUsage));
	dump_somPrintf(("host=%s\n",somThis->address.host));
	dump_somPrintf(("port=%d\n",(int)somThis->address.port));

	if (somThis->connection.is_server)
	{
		dump_somPrintf(("a server\n"));
	}
	else
	{
		if (somThis->connection.is_listener)
		{
			dump_somPrintf(("a listener\n"));
		}
		else
		{
			dump_somPrintf(("a client\n"));
		}
	}
}

struct RHBImplementationDef_get_target_object_data
{
	SOMDObject SOMSTAR dobj;
	SOMDServer SOMSTAR serv;
	boolean guarded;
};

RHBOPT_cleanup_begin(RHBImplementationDef_get_target_object_cleanup,pv)

	struct RHBImplementationDef_get_target_object_data *data=pv;

	if (data->guarded)
	{
		RHBORB_unguard(somThis)
	}
	if (data->serv)
	{
		somReleaseObjectReference(data->serv);
	}
	if (data->dobj)
	{
		somReleaseObjectReference(data->dobj);
	}

RHBOPT_cleanup_end

static SOMObject SOMSTAR rhbGIOPRequestStream_get_target_object(
		RHBImplementationDef *somThis,
		Environment *ev,
		_IDL_SEQUENCE_octet *object_key,
		SOMDServer SOMSTAR *po)
{
	struct RHBImplementationDef_get_target_object_data data={NULL,NULL,1};
	SOMObject SOMSTAR RHBOPT_volatile retval=NULL;

	RHBORB_guard(somThis)

	RHBOPT_ASSERT(data.guarded)

	RHBOPT_cleanup_push(RHBImplementationDef_get_target_object_cleanup,&data);

	if (somThis->server.data && somThis->connection.connected)
	{
		/* we don't know the interface here */

		data.dobj=RHBORB_get_object(
				somThis->orb,
				ev,
				somThis->orb->somClass_SOMDObject_ref,
				somThis,
				0,
				object_key,
				0);

		if (ev->_major)
		{
			RHBOPT_ASSERT(!data.dobj)
		}
		else
		{
			RHBOPT_ASSERT(data.dobj)
		}

		if (data.dobj)
		{
			data.serv=RHBImplementationDef_acquire_somdServer(somThis,ev);

			RHBOPT_ASSERT(data.serv)

			if (data.serv)
			{
				RHBORB_unguard(somThis)

				data.guarded=0;

				retval=SOMDServer_somdSOMObjFromRef(data.serv,ev,data.dobj);
			}

			if (ev->_major)
			{
				RHBOPT_ASSERT(!retval)
			}
			else
			{
				*po=data.serv;
				data.serv=NULL;

				RHBOPT_ASSERT(retval)
			}
		}
	}

	RHBOPT_cleanup_pop();

	return retval;
}

static SOMDServer SOMSTAR rhbGIOPRequestStream_acquire_somdServer(
		RHBImplementationDef *somThis,
		Environment *ev)
{
	SOMDServer SOMSTAR s=NULL;

	if (somThis)
	{
		RHBORB_guard(somThis)

		if (somThis->server.data)
		{
			s=somThis->server.data->somdServer;

			if (s)
			{
				s=SOMDServer_somDuplicateReference(s);
			}
		}

		if (!s)
		{
			RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,UnexpectedNULL,MAYBE);
		}

		RHBORB_unguard(somThis)
	}

	return s;
}

static void rhbGIOPRequestStream_AddRef(RHBGIOPRequestStream *somThis)
{
/*	if (!somThis->lUsage) bomb("how can we add from nothing?");*/

	somd_atomic_inc(&somThis->lUsage);
}

static void rhbGIOPRequestStream_Release(RHBGIOPRequestStream *somThis)
{
	if (!somd_atomic_dec(&somThis->lUsage)) 
	{
		rhbGIOPRequestStream_Close(somThis);
	}
}

static unsigned long rhbGIOPRequestStream_allocate_sequence(RHBImplementationDef *somThis)
{
	unsigned long ul;

	RHBORB_guard(somThis)

	somThis->transport.nextSequence++;

	ul=somThis->transport.nextSequence;

	RHBORB_unguard(somThis)

	return ul;
}

static void rhbGIOPRequestStream_add_client_ref(
		RHBImplementationDef *somThis,
		SOMObject SOMSTAR somobj)
{
	RHBOPT_ASSERT(somThis)
	RHBOPT_ASSERT(somobj)

	if (somThis /* ->server.data*/)
	{
		unsigned long i=0;

		RHBORB_guard(somThis)

		while (i < somThis->client_references._length)
		{
			if (somThis->client_references._buffer[i].somobj==somobj)
			{
				somThis->client_references._buffer[i].lUsage++;

				RHBORB_unguard(somThis)

				return;
			}

			i++;
		}

		if (i >= somThis->client_references._maximum)
		{
			/* need to extend maximum */

			RHBImplClientRef *buffer=NULL;

			somThis->client_references._maximum+=16;

			i=0;

			buffer=SOMMalloc(sizeof(*buffer)*
					somThis->client_references._maximum);
			while (i < somThis->client_references._length)
			{
				buffer[i].lUsage=somThis->client_references._buffer[i].lUsage;
				buffer[i].somobj=somThis->client_references._buffer[i].somobj;
				i++;
			}

			if (i) SOMFree(somThis->client_references._buffer);

			somThis->client_references._buffer=buffer;
		}

#ifdef _DEBUG
		if (i != somThis->client_references._length) SOMD_bomb("what?");
#endif
		somThis->client_references._buffer[i].lUsage=1;
		somThis->client_references._buffer[i].somobj=somobj;
		somThis->client_references._length++;

		RHBORB_unguard(somThis);
	}
}

static boolean rhbGIOPRequestStream_remove_client_ref(
		RHBImplementationDef *somThis,
		SOMObject SOMSTAR somobj,
		boolean already_guarded)
{
	unsigned long i=0;
	boolean __result=0;

#ifdef USE_THREADS
	if (!already_guarded)
	{
		RHBORB_guard(somThis)
	}
#endif

	while (i < somThis->client_references._length)
	{
		if (somThis->client_references._buffer[i].somobj==somobj)
		{
			somThis->client_references._buffer[i].lUsage--;

			if (!somThis->client_references._buffer[i].lUsage)
			{
				/* remove this item */

				somThis->client_references._length--;

				if (somThis->client_references._length)
				{
					while (i < somThis->client_references._length)
					{
						somThis->client_references._buffer[i].somobj=
							somThis->client_references._buffer[i+1].somobj;
						somThis->client_references._buffer[i].lUsage=
							somThis->client_references._buffer[i+1].lUsage;
						i++;
					}
				}
				else
				{
					SOMFree(somThis->client_references._buffer);
					somThis->client_references._maximum=0;
					somThis->client_references._buffer=NULL;
				}
			}

			__result=1;

			break;
		}
		else
		{
			i++;
		}
	}

#ifdef USE_THREADS
	if (!already_guarded)
	{
		RHBORB_unguard(somThis)
	}
#endif

	return __result;
}

struct RHBImplementationDef_notify_somdd_data
{
	char *impl_id;
	char *impl_server_ior;
	SOMDServer SOMSTAR server;
	_IDL_SEQUENCE_octet key;
	_IDL_SEQUENCE_octet seq;
};

RHBOPT_cleanup_begin(RHBImplementationDef_notify_somdd_cleanup,pv)

struct RHBImplementationDef_notify_somdd_data *data=pv;

	if (data->impl_id) SOMFree(data->impl_id);
	if (data->impl_server_ior) SOMFree(data->impl_server_ior);
	if (data->key._buffer) SOMFree(data->key._buffer);
	if (data->seq._buffer) SOMFree(data->seq._buffer);
	if (data->server) somReleaseObjectReference(data->server);

RHBOPT_cleanup_end

static void rhbGIOPRequestStream_notify_somdd(
		RHBImplementationDef *somThis,
		Environment *ev,
		boolean up)
{
	struct RHBImplementationDef_notify_somdd_data data={
		NULL,NULL,NULL,{0,0,NULL},{0,0,NULL}};

	if (ev->_major)
	{
		return;
	}

	if (!somThis)
	{
		RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,SOMDDNotRunning,NO);

		return;
	}

	if (somThis->server.data)
	{
		if (somThis->server.data->somdd_notified_state == up)
		{
			return;
		}
	}

	if (!somThis->server.data->impl_id._length)
	{
		return;
	}

	RHBOPT_cleanup_push(RHBImplementationDef_notify_somdd_cleanup,&data);

	data.impl_id=SOMMalloc(somThis->server.data->impl_id._length+1);
	data.impl_id[somThis->server.data->impl_id._length]=0;
	memcpy(data.impl_id,somThis->server.data->impl_id._buffer,somThis->server.data->impl_id._length);

	if (strcmp(data.impl_id,RHBORB_get_somdd_implid(somThis->orb)))
	{
		if (up)
		{
			unsigned int i=0;
			char *q=NULL;

			data.server=RHBImplementationDef_acquire_somdServer(somThis,ev);

			debug_somPrintf(("announcing server on %s:%d\n",
				somThis->address.host,somThis->address.port));

			data.key=somdGetDefaultObjectKey(ev,data.impl_id);

			somd_singleton(
					somThis->orb,
					SOMObject_somGetClassName(data.server),
					ev,
					somThis->address.host,
					somThis->address.port,
					&data.key,
					&data.seq);

			i=data.seq._length;
			i<<=1;
			data.impl_server_ior=SOMMalloc(i+5);
			q=data.impl_server_ior;
			*q++='I';
			*q++='O';
			*q++='R';
			*q++=':';

			i=0;

			while (i < data.seq._length)
			{
				*q++=RHBORB_hex_char(somThis->orb,(octet)(data.seq._buffer[i] >> 4));
				*q++=RHBORB_hex_char(somThis->orb,(octet)(data.seq._buffer[i] & 0xf));

				i++;
			}

			*q=0;
		}

		if (!ev->_major)
		{
			SOMOA_notify_somdd(SOMD_SOMOAObject,ev,up,data.impl_id,data.impl_server_ior,0);
		}

		if (ev->_major)
		{
			debug_somPrintf(("notification for impl %s returned %s\n",
					data.impl_id,
					somExceptionId(ev)));
		}
		else
		{
			if (somThis->server.data)
			{
				somThis->server.data->somdd_notified_state=up;
			}
		}
	}

	RHBOPT_cleanup_pop();
}

static void RHBImplementationDef_Callback
		(RHBImplementationDef *impl,
		 RHBSocketRef socket,
		 short cmdWhat,
		 long err)
{
	Environment ev;
	boolean interrupt_somoa=0;

	RHBImplementationDef_AddRef(impl);

	SOM_InitEnvironment(&ev);
	
	if (err && (cmdWhat != RHBSocket_notify_connect))
	{
		RHBOPT_throw_StExcep((&ev),COMM_FAILURE,SocketError,MAYBE);
	}

	switch (cmdWhat)
	{
	case RHBSocket_notify_accept:
		RHBImplementationDef_interrupt_accept(impl,socket,&ev);
		break;

	case RHBSocket_notify_connect:
		if (err)
		{
			RHBGIOPRequestStream_connect_retry(impl,&ev);
		}
		else
		{
			RHBImplementationDef_interrupt_connected(impl,&ev);
		}
		break;
	case RHBSocket_notify_close:
/*		somPrintf("RHBImplementationDef_Callback(%s) - close\n",somExceptionId(&ev));*/
		interrupt_somoa=impl->connection.is_listener;
		RHBImplementationDef_interrupt_closed(impl,&ev);
		if (interrupt_somoa)
		{
			if (SOMD_SOMOAObject)
			{
				SOMOA_interrupt_server_status(
					SOMD_SOMOAObject,
					&ev,
					SOMDERROR_SocketClose);
			}
		}
		break;
	case RHBSocket_notify_read:
		RHBImplementationDef_interrupt_readable(impl,&ev);
		break;
	case RHBSocket_notify_write:
		RHBImplementationDef_interrupt_writeable(impl,&ev);
		break;
	}

	SOM_UninitEnvironment(&ev);

	RHBImplementationDef_Release(impl);
}

static void rhbGIOPRequestStream_remove_client_references(
		RHBImplementationDef *somThis)
{
	RHBORB_guard(somThis)

	if (somThis->client_references._length)
	{
/*		unsigned int i=0;*/
		somd_atomic_inc(&somThis->lUsage);


/*		dump_somPrintf("client left over %d reference%s\n",
				(int)somThis->client_references._length,
				(somThis->client_references._length==1) ? ":" : "s" );
*/
		while (somThis->client_references._length)
		{
			SOMObject SOMSTAR o=somThis->client_references._buffer[0].somobj;

			RHBImplementationDef_remove_client_ref(somThis,o,1);

			RHBORB_unguard(somThis);

/*			somPrefixLevel(1);
			SOMObject_somPrintSelf(o);*/

#ifdef SOMObject_somRelease
			SOMObject_somRelease(o);
#else
			if (SOMObject_somIsA(o,_SOMRefObject))
			{
				SOMRefObject_somRelease(o);
			}
#endif

			RHBORB_guard(somThis)
		}

		somd_atomic_dec(&somThis->lUsage);
	}

	RHBORB_unguard(somThis)
}

static RHBSendData *SOMLINK RHBSendDataNew(_IDL_SEQUENCE_octet *header,_IDL_SEQUENCE_octet *data)
{
	if (header && data)
	{
		RHBSendData *somThis=SOMMalloc(sizeof(*somThis)+header->_length+data->_length);

		if (somThis)
		{
			somThis->toGo=header->_length+data->_length;
			somThis->seekPtr=somThis->_buffer;
			somThis->next=NULL;

			memcpy(somThis->seekPtr,header->_buffer,header->_length);
			memcpy(somThis->seekPtr+header->_length,data->_buffer,data->_length);

			SOMFree(header->_buffer);
			header->_buffer=NULL;
			header->_length=0;
			header->_maximum=0;
			SOMFree(data->_buffer);
			data->_buffer=NULL;
			data->_length=0;
			data->_maximum=0;

			return somThis;
		}
	}

	return NULL;
}

static void rhbGIOPRequestStream_write_GIOP_msg(
		RHBGIOPRequestStream *somThis,
		Environment *ev,
		GIOP_Version *version,
		GIOP_MsgType type,
		_IDL_SEQUENCE_octet *data)
{
	if (somThis->connection.connected)
	{
		GIOP_MessageHeader header={{'G','I','O','P'},{1,0},0,0,0};
#ifdef somNewObject
		RHBORBStream_GIOPOutput SOMSTAR output=somNewObject(RHBORBStream_GIOPOutput);
#else
		RHBORBStream_GIOPOutput SOMSTAR output=RHBORBStream_GIOPOutputNew();
#endif

		RHBORBStream_GIOPOutput__set_c_orb(output,ev,somThis->orb);
		RHBORBStream_GIOPOutput__set_streamed_data(output,ev,NULL);

		if (version->major != 1)
		{
			SOMD_bomb("GIOP major != 1");
		}

		if (version->minor != 0)
		{
			SOMD_bomb("GIOP minor != 0");
		}

		header.protocol_version.major=version->major;
		header.protocol_version.minor=version->minor;

		header.flags=RHBCDR_machine_type();
		header.message_type=(octet)RHBCDR_IDLenumToGIOPenum(type,0,0);
		header.message_size=data->_length;

/*		somPrintf("header.flags=%d\n",(int)header.flags);
		somPrintf("header.message_size=%ld\n",(long)data->_length);*/

		RHBCDR_marshal(
					NULL,
					ev,
					&header,
					somdTC_GIOP_MessageHeader,
					output);

		if (!ev->_major)
		{
			_IDL_SEQUENCE_octet msg_head=RHBORBStream_GIOPOutput__get_streamed_data(output,ev);
			RHBSendData *send_data=RHBSendDataNew(&msg_head,data);

			RHBORB_guard(somThis)

			if (send_data)
			{
				RHBImplementationDef_write_giop(somThis,ev,send_data);
			}
			else
			{
				RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
			}

			RHBORB_unguard(somThis)

			if (msg_head._buffer) SOMFree(msg_head._buffer);

			if (!ev->_major) RHBImplementationDef_kick_transmitter(somThis,ev);
		}

		somReleaseObjectReference(output);
	}
}

SOM_Scope int RHBLINK implsink_QueryInterface(RHBSocketSink *somSelf,void *iid,void **ppv)
{
	return -1;
}

SOM_Scope int RHBLINK implsink_AddRef(RHBSocketSink *somSelf)
{
	struct RHBGIOPRequestStream_sink *somThis=(void *)somSelf;
	somd_atomic_inc(&somThis->lUsage);
	return 0;
}

SOM_Scope int RHBLINK implsink_Release(RHBSocketSink *somSelf)
{
	struct RHBGIOPRequestStream_sink *somThis=(void *)somSelf;

	RHBORB_guard(0)

	if (!somd_atomic_dec(&somThis->lUsage))
	{
		if (!somThis->outer->lUsage)
		{
			RHBImplementationDef *impl=somThis->outer;

			RHBImplementationDef_AddRef(impl);

			RHBORB_unguard(0)

			rhbGIOPRequestStream_Close(impl);

			RHBImplementationDef_Release(impl);

			return 0;
		}
	}

	RHBORB_unguard(0)

	return 0;
}

struct implsink_Notify_data
{
	RHBImplementationDef *impl;
};

RHBOPT_cleanup_begin(implsink_Notify_cleanup,pv)

struct implsink_Notify_data *data=pv;

	if (data->impl) RHBImplementationDef_Release(data->impl);

RHBOPT_cleanup_end

SOM_Scope void RHBLINK implsink_Notify(RHBSocketSink *somSelf,struct RHBSocket *sock,short what,long err)
{
	struct RHBGIOPRequestStream_sink *somThis=(void *)somSelf;
	struct implsink_Notify_data data={NULL};
	RHBOPT_cleanup_push(implsink_Notify_cleanup,&data);

	RHBORB_guard(0)

	data.impl=somThis->outer;

	if (data.impl)
	{
		RHBImplementationDef_AddRef(data.impl);
	}

	RHBORB_unguard(0);

	if (data.impl)
	{
		RHBImplementationDef_Callback(data.impl,sock,what,err);
	}

	RHBOPT_cleanup_pop();
}

static void SOMLINK rhbGIOPRequestStream_read_object(
		SOMCDR_unmarshal_filter *filter,
		Environment *ev,
		void *value,
		TypeCode tc,
		RHBORBStream_GIOPInput SOMSTAR stream)
{
	switch (TypeCode_kind(tc,ev))
	{
	case tk_Principal:
		((Principal SOMSTAR *)value)[0]=RHBORBStream_GIOPInput_read_Principal(stream,ev);
		break;
	case tk_objref:
		((SOMObject SOMSTAR *)value)[0]=RHBORBStream_GIOPInput_read_SOMObject(stream,ev,tc);
		break;
	}
}

static struct RHBGIOPRequestStreamVtbl rhbGIOPRequestStreamVtbl=
{
	rhbGIOPRequestStream_AddRef,
	rhbGIOPRequestStream_Release,
	rhbGIOPRequestStream_interrupt_closed,
	rhbGIOPRequestStream_write_GIOP_msg,
	rhbGIOPRequestStream_add_outstanding,
	rhbGIOPRequestStream_remove_outstanding,
	rhbGIOPRequestStream_get_wrapper,
	rhbGIOPRequestStream_begin_listen,
	rhbGIOPRequestStream_end_listen,
	rhbGIOPRequestStream_remove_client_references,
	rhbGIOPRequestStream_acquire_somdServer,
	rhbGIOPRequestStream_get_target_object,
	rhbGIOPRequestStream_add_client_ref,
	rhbGIOPRequestStream_remove_client_ref,
	rhbGIOPRequestStream_allocate_sequence,
	rhbGIOPRequestStream_queue_request,
	rhbGIOPRequestStream_notify_somdd,
	rhbGIOPRequestStream_debug_printf
};

RHBGIOPRequestStream *RHBGIOPRequestStreamNew(
				RHBORB *the_orb,
				IIOP_ProfileBody_1_0 *profile) 
{
	RHBGIOPRequestStream *somThis=SOMMalloc(sizeof(*somThis));

	somThis->lpVtbl=&rhbGIOPRequestStreamVtbl;

	somThis->server.release_on_close=0;
	somThis->server.data=NULL;

#ifdef USE_SELECT
	somThis->active_server_request=NULL;
	somThis->queued_request_list=NULL;
#endif

	RHBRequestList_init(&somThis->pending);
	RHBRequestList_init(&somThis->executing_oneway);
	RHBRequestList_init(&somThis->executing_normal);

	somThis->deleting=0;
	somThis->is_closing=0;
	somThis->already_reading=0;

	somThis->connection.transmitting=0;

	somThis->sink.outer=somThis;
	somThis->sink.vtbl=&implsink_vtbl;
	somThis->sink.lUsage=0;

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_init(&somThis->p_EventSem,RHBOPT_pthread_condattr_default);
	#else
		somThis->h_EventSem=CreateEvent(NULL,0,0,NULL);
	#endif
#endif

	somThis->outstanding_request_list=NULL;
	somThis->receiving._buffer=NULL;

	somThis->lUsage=1;

	somThis->transport.nextSequence=1;

	somThis->connection.fd=NULL;
	somThis->connection.readable=0;
	somThis->connection.writeable=0;
	somThis->connection.connected=0;
	somThis->connection.connect_attempts=0;
	somThis->connection.is_listener=0;
	somThis->connection.is_server=0;
	somThis->connection.connecting=0;
/*	somThis->connection.write_posted=0;*/
	
	somThis->connection.resolver=NULL;

	somThis->transmitting.head=NULL;
	somThis->transmitting.tail=NULL;

	somThis->receiving_header=0;
	somThis->receiving._length=0;
	somThis->receiving._maximum=0;
	somThis->receiving._buffer=NULL;

	somThis->orb=the_orb;

	somThis->client_references._buffer=NULL;
	somThis->client_references._maximum=0;
	somThis->client_references._length=0;

	somThis->unmarshal_filter.read_object=rhbGIOPRequestStream_read_object;

	RHBORB_AddRef(somThis->orb);

	if (profile)
	{
		somThis->address.host=somd_dupl_string(profile->host);
		somThis->address.port=profile->port;
	}
	else
	{
		somThis->address.host=NULL;
		somThis->address.port=0;
	}

	somThis->address.addrSeq._length=0;
	somThis->address.addrSeq._maximum=0;
	somThis->address.addrSeq._buffer=NULL;

	somThis->server_requests=NULL;

	RHBDebug_created(RHBImplementationDef,somThis)

	RHBORB_add_impl(somThis->orb,somThis);

	return somThis;
}
