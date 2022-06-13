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

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef RHBLINK
	#ifdef _WIN32
		#define RHBLINK __stdcall
	#else
		#define RHBLINK
	#endif
#endif

typedef	struct RHBSocketSinkJumpTable *RHBSocketSink;
typedef long RHBSocketError;
#ifdef USE_THREADS
typedef socklen_t RHBSocketLength;	/* for Winsock/BSD sockets */
#else
typedef long RHBSocketLength;		/* for SOM Sockets */
#endif
typedef struct sockaddr  RHBSocketAddress;
struct RHBGIOPRequestStream;

struct RHBSocketSinkJumpTable
{
	int (RHBLINK * QueryInterface)(RHBSocketSink *,void *,void **);
	int (RHBLINK * AddRef)(RHBSocketSink *);
	int (RHBLINK * Release)(RHBSocketSink *);
	void (RHBLINK * Notify)(RHBSocketSink *,struct RHBSocket *socket,short cmdWhat,long errorCode);
};

#define RHBSocketSink_Release(x)			(*x)->Release(x)
#define RHBSocketSink_AddRef(x)				(*x)->AddRef(x)
#define RHBSocketSink_Notify(x,a,b,c)		(*x)->Notify(x,a,b,c)

#define RHBSocketSink_impl(x) \
	SOM_Scope int RHBLINK x##_QueryInterface(RHBSocketSink *,void *,void **); \
	SOM_Scope int RHBLINK x##_AddRef(RHBSocketSink *); \
	SOM_Scope int RHBLINK x##_Release(RHBSocketSink *); \
	SOM_Scope void RHBLINK x##_Notify(RHBSocketSink *,struct RHBSocket *,short,long); \
	static struct RHBSocketSinkJumpTable x##_vtbl={ \
	x##_QueryInterface,x##_AddRef,x##_Release,x##_Notify };

struct RHBSocketJumpTable
{
	int (RHBLINK * QueryInterface)(struct RHBSocket * sock,void *,void **);
	int (RHBLINK * AddRef)(struct RHBSocket * sock);
	int (RHBLINK * Release)(struct RHBSocket * sock);
	boolean (RHBLINK *StartConnect)(struct RHBSocket * sock,RHBSocketError *,RHBSocketAddress *addr,RHBSocketLength len);
	void (RHBLINK *StartListen)(struct RHBSocket * sock,RHBSocketError *,RHBSocketAddress * port,RHBSocketLength len,RHBSocketAddress *actual,RHBSocketLength *plen);
	void (RHBLINK *Shutdown)(struct RHBSocket * sock,RHBSocketError *,int);
	boolean (RHBLINK *IsConnected)(struct RHBSocket * sock,RHBSocketError *);
	int (RHBLINK *SendData)(struct RHBSocket * sock,RHBSocketError *,const void *data,int len);
	int (RHBLINK *RecvData)(struct RHBSocket * sock,RHBSocketError *,void *data,int len);
	struct RHBSocket * (RHBLINK *Accept)(struct RHBSocket * sock,RHBSocketError *,
				RHBSocketAddress *,RHBSocketLength *len,
				RHBSocketAddress *,RHBSocketLength *);
	void (RHBLINK *Close)(struct RHBSocket * sock);
	void (RHBLINK *EnableWrites)(struct RHBSocket * sock,boolean how);
	void (RHBLINK *IsNonBlocking)(struct RHBSocket * sock,RHBSocketSink *sink);
	boolean (RHBLINK *IsOneShot)(struct RHBSocket * sock);
	void (RHBLINK *dump)(struct RHBSocket * sock);
};

#define RHBSocket_impl(x)   \
SOM_Scope int RHBLINK x##_QueryInterface(RHBSocketRef sock,void *,void **); \
SOM_Scope int RHBLINK x##_AddRef(RHBSocketRef sock); \
SOM_Scope int RHBLINK x##_Release(RHBSocketRef sock); \
SOM_Scope boolean RHBLINK x##_StartConnect(RHBSocketRef sock,RHBSocketError *,RHBSocketAddress *,RHBSocketLength len); \
SOM_Scope void RHBLINK x##_StartListen(RHBSocketRef sock,RHBSocketError *,RHBSocketAddress *,RHBSocketLength len,RHBSocketAddress *actual,RHBSocketLength *plen); \
SOM_Scope void RHBLINK x##_Shutdown(RHBSocketRef sock,RHBSocketError *,int); \
SOM_Scope boolean RHBLINK x##_IsConnected(RHBSocketRef sock,RHBSocketError *); \
SOM_Scope int RHBLINK x##_SendData(RHBSocketRef sock,RHBSocketError *,const void *data,int len); \
SOM_Scope int RHBLINK x##_RecvData(RHBSocketRef sock,RHBSocketError *,void *data,int len); \
SOM_Scope RHBSocketRef RHBLINK x##_Accept(RHBSocketRef sock,RHBSocketError *,RHBSocketAddress *,RHBSocketLength *len,RHBSocketAddress *,RHBSocketLength *); \
SOM_Scope void RHBLINK x##_Close(RHBSocketRef sock); \
SOM_Scope void RHBLINK x##_EnableWrites(RHBSocketRef sock,boolean how); \
SOM_Scope void RHBLINK x##_IsNonBlocking(RHBSocketRef sock,RHBSocketSink *refCon); \
SOM_Scope boolean RHBLINK x##_IsOneShot(RHBSocketRef sock); \
SOM_Scope void RHBLINK x##_dump(RHBSocketRef sock); \
static struct RHBSocketJumpTable x##_JumpTable={ \
x##_QueryInterface,x##_AddRef,x##_Release,  \
	x##_StartConnect,x##_StartListen,x##_Shutdown,x##_IsConnected,  \
	x##_SendData,x##_RecvData,x##_Accept,x##_Close, \
	x##_EnableWrites,x##_IsNonBlocking,x##_IsOneShot,x##_dump \
			};

struct RHBSocket
{
	struct RHBSocketJumpTable *vtbl;
	struct RHBSocketData *impl;
};

#define RHBSocket_Release(x)			x->vtbl->Release(x)
#define RHBSocket_AddRef(x)				x->vtbl->AddRef(x)
#define RHBSocket_StartConnect(x,e,a,b)	x->vtbl->StartConnect(x,e,a,b)
#define RHBSocket_StartListen(x,e,a,b,c,d)    x->vtbl->StartListen(x,e,a,b,c,d)
#define RHBSocket_Shutdown(x,a,b)		x->vtbl->Shutdown(x,a,b)
#define RHBSocket_IsConnected(x)		x->vtbl->IsConnected(x)
#define RHBSocket_SendData(x,a,b,c)     x->vtbl->SendData(x,a,b,c)
#define RHBSocket_RecvData(x,a,b,c)     x->vtbl->RecvData(x,a,b,c)
#define RHBSocket_Accept(x,a,b,c,d,e)   x->vtbl->Accept(x,a,b,c,d,e)
#define RHBSocket_Close(x)				x->vtbl->Close(x)
#define RHBSocket_EnableWrites(x,y)	    x->vtbl->EnableWrites(x,y)
#define RHBSocket_IsNonBlocking(x,y)	x->vtbl->IsNonBlocking(x,y)
#define RHBSocket_IsOneShot(x)          x->vtbl->IsOneShot(x)
/*
#define RHBSocket_dump(x)				x->vtbl->dump(x)
*/
#define RHBSocketGetData(x)   x->impl

#define RHBSocket_notify_read		0x801
#define RHBSocket_notify_write		0x802
#define RHBSocket_notify_connect	0x804
#define RHBSocket_notify_close		0x805
#define RHBSocket_notify_accept		0x806
#define RHBSocket_notify_except		0x807

typedef void (RHBLINK *RHBResolver_Callback)(
		struct RHBGIOPRequestStream *,
		RHBSocketError,
		_IDL_SEQUENCE_SOMD_NetworkAddress *);

struct RHBProtocol
{
	struct RHBProtocolJumpTable *vtbl;
	struct RHBProtocolData *impl;
};

struct RHBProtocolJumpTable
{
	int (RHBLINK * QueryInterface)(struct RHBProtocol *,void *,void **);
	int (RHBLINK * AddRef)(struct RHBProtocol *);
	int (RHBLINK * Release)(struct RHBProtocol *);
	void (RHBLINK * Close)(struct RHBProtocol *);
	struct RHBSocket * (RHBLINK *CreateStream)(struct RHBProtocol *,unsigned short);
	struct RHBResolver * (RHBLINK * CreateResolver)(struct RHBProtocol *somSelf,const char *name,unsigned short port);
	void (RHBLINK * atfork_child)(struct RHBProtocol * somSelf);
};

#define RHBProtocol_impl(x)  \
SOM_Scope int RHBLINK x##_QueryInterface(RHBProtocolRef sock,void *,void **); \
SOM_Scope int RHBLINK x##_AddRef(RHBProtocolRef sock); \
SOM_Scope int RHBLINK x##_Release(RHBProtocolRef sock); \
SOM_Scope void RHBLINK x##_Close(RHBProtocolRef sock); \
SOM_Scope RHBSocketRef RHBLINK x##_CreateStream(RHBProtocolRef sock,unsigned short family); \
SOM_Scope RHBResolver *  RHBLINK x##_CreateResolver(RHBProtocolRef,const char *,unsigned short); \
SOM_Scope void RHBLINK x##_atfork_child(RHBProtocolRef); \
SOM_Scope struct RHBProtocolJumpTable x##_JumpTable={\
  x##_QueryInterface,x##_AddRef,x##_Release,x##_Close,x##_CreateStream,x##_CreateResolver,x##_atfork_child}; 

#define RHBProtocol_AddRef(x)						x->vtbl->AddRef(x)
#define RHBProtocol_Release(x)						x->vtbl->Release(x)
#define RHBProtocol_Close(x)						x->vtbl->Close(x)
#define RHBProtocol_CreateStream(x,a)				x->vtbl->CreateStream(x,a)
#define RHBProtocol_CreateResolver(x,a,c)			x->vtbl->CreateResolver(x,a,c)
#define RHBProtocol_EndNameResolve(x,a)				x->vtbl->EndNameResolve(x,a)
#define RHBProtocol_atfork_child(a)				    a->vtbl->atfork_child(a)

struct RHBResolverJumpTable
{
	int (RHBLINK * QueryInterface)(struct RHBResolver *,void *,void **);
	int (RHBLINK * AddRef)(struct RHBResolver *);
	int (RHBLINK * Release)(struct RHBResolver *);
	void (RHBLINK *Start)(struct RHBResolver *,struct RHBGIOPRequestStream *,RHBResolver_Callback);
	void (RHBLINK *Cancel)(struct RHBResolver *);
};

struct RHBResolver
{
	struct RHBResolverJumpTable *vtbl;
	struct RHBResolverData *impl;
};

#define RHBResolver_impl(x)  \
SOM_Scope int RHBLINK x##_QueryInterface(RHBResolverRef sock,void *,void **); \
SOM_Scope int RHBLINK x##_AddRef(RHBResolverRef sock); \
SOM_Scope int RHBLINK x##_Release(RHBResolverRef sock); \
SOM_Scope void RHBLINK x##_Start(RHBResolverRef sock,struct RHBGIOPRequestStream *,RHBResolver_Callback); \
SOM_Scope void RHBLINK x##_Cancel(RHBResolverRef sock); \
SOM_Scope struct RHBResolverJumpTable x##_JumpTable={\
x##_QueryInterface,x##_AddRef,x##_Release,x##_Start,x##_Cancel};


#define RHBResolver_Release(x)       x->vtbl->Release(x)
#define RHBResolver_AddRef(x)		 x->vtbl->AddRef(x)
#define RHBResolver_Start(x,a,b)     x->vtbl->Start(x,a,b)
#define RHBResolver_Cancel(x)        x->vtbl->Cancel(x)

#ifdef _PLATFORM_MACINTOSH_
	RHBProtocolRef RHBLINK rhbnetot_protocol(void);
	#ifdef BUILD_RHBNETOT
		#pragma	export list rhbnetot_protocol
	#else
		#pragma	import list rhbnetot_protocol
	#endif
#endif	

#ifdef __cplusplus
}
#endif
