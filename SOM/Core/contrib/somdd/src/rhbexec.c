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

/* manages the child processes */

#include <rhbopt.h>

#define stdout_somPrintf	somPrintf

#ifdef _WIN32
#	include <direct.h>
#	include <io.h>
#	include <process.h>
#endif

#include <rhbsomdd.h>
#include <rhbexecd.h>

#ifdef USE_SELECT_RHBPROCESSMGR
static int set_non_blocking(
		SOCKET fd)
{
#ifdef _WIN32
		BOOL ul=1;
		int i=ioctlsocket(fd,FIONBIO,(unsigned long *)&ul);
		RHBOPT_ASSERT(!i);
		return i;
#else /* _WIN32 */
#	ifdef __OS2__
#		error need appropriate call here
#	else
#		ifdef HAVE_FCNTL_F_SETFL_O_NDELAY
			return fcntl(fd,F_SETFL,O_NDELAY);
#		else
			int ul=1;
			return ioctl(fd,FIONBIO,(char *)&ul);
#		endif
#	endif
#endif /* _WIN32 */
}
#endif

#ifdef USE_SELECT_RHBPROCESSMGR
static void soclose(SOCKET fd)
{
	RHBOPT_ASSERT(fd!=INVALID_SOCKET);

	if (fd != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(fd);
#else
		close(fd);
#endif
	}
}
#endif

static long get_int32(unsigned char *op)
{
	return (((long)op[0])<<24) |
		   (((long)op[1])<<16) |
		   (((long)op[2])<<8) |
		   (((long)op[3]));
}

static int put_int32(unsigned char *op,long value)
{
	*op++=(unsigned char)(value >> 24);
	*op++=(unsigned char)(value >> 16);
	*op++=(unsigned char)(value >> 8);
	*op++=(unsigned char)(value);
	return 4;
}

static int put_string(unsigned char *op,char *str)
{
	long len=(long)strlen(str)+1;
	int rc=put_int32(op,len);
	memcpy(op+rc,str,len);
	return rc+len;
}

static boolean confirm_program(char *program,size_t proglen)
{
	char *path;
#ifdef _WIN32
	char magic=';';
	int X_OK=0;
#else
	char magic=':';
#endif

	if (!program[0]) return 0;
	if (program[0]=='/') 
	{
		if (-1 != access(program,X_OK))
		{
			return 1;
		}

		return 0;
	}

#ifdef _WIN32
	if (program[0]=='\\') return 1;
	if (program[1]==':')
	{
		return 1;
	}
#endif

	path=getenv("PATH");

#ifndef _WIN32
	if (!path)
	{
		path="/bin:/usr/bin:";
	}
#endif

	if (path)
	{
		while (*path)
		{
			char buf[1024];
			char *q;

			q=buf;

			while ((*path)&&(*path !=magic))
			{
				*q=*path;
				path++;
				q++;
			}
			*q=0;
			if (*path) path++;

			if (!buf[0])
			{
				getcwd(buf,sizeof(buf));
			}

			if (buf[0])
			{
				if (!strcmp(buf,"."))
				{
					getcwd(buf,sizeof(buf));
				}
#ifdef _WIN32
				strncat(buf,"\\",sizeof(buf)-1);
#else
				strncat(buf,"/",sizeof(buf)-1);
#endif
				strncat(buf,program,sizeof(buf)-1);

				if (-1 != access(buf,X_OK))
				{
#ifdef _DEBUG
/*					stdout_somPrintf("confirm_program(%s) is %s\n",program,buf);*/
#endif
					strncpy(program,buf,proglen);

					return 1;
				}
			}
		}
	}

	{
		char buf[1024];
#ifdef _WIN32
		char *p=buf;
		GetModuleFileName(0,buf,sizeof(buf));
		p+=strlen(p);
		while (p > buf)
		{
			p--;
			if ((*p=='/')||(*p=='\\'))
			{
				*p=0; break;
			}
		}
		strncat(buf,"\\",sizeof(buf)-1);
#else
		getcwd(buf,sizeof(buf));
		strncat(buf,"/",sizeof(buf)-1);
#endif
		strncat(buf,program,sizeof(buf)-1);
		if (-1 != access(buf,X_OK))
		{
			strncpy(program,buf,proglen);
			return 1;
		}
	}

	return 0;
}

typedef struct RHBProcessData
{
	RHBProcess comobj;
	struct RHBProcessData *next;
	unsigned long id;
	rhbatomic_t usage;
	RHBProcessMgr *mgr;
} RHBProcessData;

static int process_Print(RHBProcess *somSelf)
{
	RHBProcessData *somThis=(void *)somSelf;
	stdout_somPrintf("{");
	stdout_somPrintf("usage=%ld,id=%ld",(long)somThis->usage,(long)somThis->id);
	stdout_somPrintf("}\n");
	return 0;
}

boolean SOMDD_SameProcess(RHBProcess *one,RHBProcess *two)
{
	if (one && two)
	{
	#ifdef _PLATFORM_MACINTOSH_
		Boolean b=0;
		if (one==two) return 1;
		if (!SameProcess(&one->psn,&two->psn,&b))
		{
			return b;
		}
	#else
		if (one==two) return 1;
	#endif
	}
	return 0;
}

#ifdef _WIN32
static BOOL shareable(HANDLE *ph)
{
	return DuplicateHandle(
		GetCurrentProcess(),
		*ph,
		GetCurrentProcess(),
		ph,
		0,
		TRUE,
		DUPLICATE_SAME_ACCESS|DUPLICATE_CLOSE_SOURCE);
}
#endif

#if defined(_WIN32) && !defined(USE_SELECT_RHBPROCESSMGR)
static DWORD CALLBACK SOMDD_executor_write(void *pv)
{
RHBProcessMgr *somThis=pv;
SOMDD_MutexRef mtx=SOMDD_AcquireGlobalMutex();

	while ((!somThis->bQuitWrite))
	{
		struct SOMDD_message *msg=somThis->outgoing;
		
		if (msg)
		{
			somThis->outgoing=msg->next;
		}

		SOMDD_ReleaseGlobalMutex(mtx);

		if (msg)
		{
			RHBOPT_ASSERT(msg->ptr==msg->data);

			while (msg->length)
			{
				DWORD len=msg->length;
				BOOL b=WriteFile(somThis->hClientWrite,msg->ptr,len,&len,NULL);

				if (b)
				{
					msg->length-=len;
					msg->ptr+=len;
				}
				else
				{
					RHBOPT_ASSERT(b);
					break;
				}
			}

			SOMFree(msg);
		}
		else
		{
			WaitForSingleObject(somThis->hWriteEvent,INFINITE);
		}

		mtx=SOMDD_AcquireGlobalMutex();	
	}

	somThis->bQuitWrite=FALSE;

	SOMDD_ReleaseGlobalMutex(mtx);

	return 0;
}
#endif

#ifdef USE_THREADS
#else
static void sink_add(RHBProcessMgr *somThis,
					 Environment *ev,
					 struct emanSink *event,
					 SOCKET fd,
					 long mask,
					 EMRegProc evp)
{
	RHBOPT_ASSERT(!event->mgr);
	RHBOPT_ASSERT(!event->regData);
	RHBOPT_ASSERT(somThis->childInfo->emanObject);

	event->mgr=somThis;
	event->mask=mask;

#ifdef somNewObject
	event->regData=somNewObject(SOMEEMRegisterData);
#else
	event->regData=SOMEEMRegisterDataNew();
#endif

	SOMEEMRegisterData_someClearRegData(event->regData,ev);
	somva_SOMEEMRegisterData_someSetRegDataEventMask(event->regData,ev,EMSinkEvent,NULL);
	SOMEEMRegisterData_someSetRegDataSink(event->regData,ev,fd);
	SOMEEMRegisterData_someSetRegDataSinkMask(event->regData,ev,mask);

	event->token=SOMEEMan_someRegisterProc(
				somThis->childInfo->emanObject,
				ev,
				event->regData,
				evp,
				event);
}
static void sink_remove(RHBProcessMgr *somThis,
						Environment *ev,
						struct emanSink *event)
{
	if (event->mgr==somThis)
	{
		if (event->regData)
		{
			SOMEEMan_someUnRegister(somThis->childInfo->emanObject,ev,event->token);
			somReleaseObjectReference(event->regData);
			event->regData=NULL;
		}
		event->mgr=NULL;
	}
}
#endif

static int process_remove(struct RHBProcessData **h,struct RHBProcessData *p)
{
	if (*h)
	{
		if (*h==p)
		{
			*h=p->next;
			return 1;
		}
		else
		{
			struct RHBProcessData *q=*h;

			while (q->next)
			{
				if (q->next==p)
				{
					q->next=p->next;
					return 1;
				}

				q=q->next;
			}
		}
	}

	return 0;
}

#ifdef USE_SELECT_RHBPROCESSMGR
#	ifdef _WIN32
#		define pipe_read(fd,buf,len)		recv(fd,buf,len,0)
#		define pipe_write(fd,buf,len)		send(fd,buf,len,0)
#	else
#		define pipe_read(fd,buf,len)		read(fd,buf,len)
#		define pipe_write(fd,buf,len)		write(fd,buf,len)
#	endif
#	ifdef USE_THREADS
#	else
		static void SOMLINK sink_write(SOMObject SOMSTAR,void *);
#	endif
#endif


static void process_sendmsg(RHBProcessMgr *somThis,
							struct SOMDD_message *msg)
{
	msg->next=NULL;

	RHBOPT_ASSERT((msg->data==msg->ptr))

	if (somThis->outgoing)
	{
		struct SOMDD_message *p=somThis->outgoing;
		while (p->next) p=p->next;
		p->next=msg;
	}
	else
	{
		somThis->outgoing=msg;
	}

#ifdef USE_SELECT_RHBPROCESSMGR
	#ifdef USE_THREADS
		pipe_write(somThis->fdSignalWrite,"\000",1);
	#else
		if (!somThis->childInfo->sinks[1].regData)
		{
			Environment ev;
			SOM_InitEnvironment(&ev);
			sink_add(somThis,&ev,somThis->childInfo->sinks+1,
					 somThis->fdClientWrite,
					 EMInputWriteMask,sink_write);
			SOM_UninitEnvironment(&ev);
		}
	#endif
#else
	SetEvent(somThis->hWriteEvent);
#endif
}

static void handle_incoming(RHBProcessMgr *somThis,
							struct SOMDD_message *msg,
#ifdef USE_THREADS
							SOMDD_MutexRef mtx,
#endif
							RHBProcess **hProcess)
{
	RHBProcess *proc=NULL;
	unsigned long code=get_int32(msg->data+4);
	unsigned long id=get_int32(msg->data+8);
	struct RHBProcessData *p=somThis->running;

	while (p)
	{
		if (p->id==id) 
		{
			rhbatomic_inc(&p->usage);
			proc=&p->comobj;
			break;
		}

		p=p->next;
	}

	switch (code)
	{
	case RHBEXECD_OBITUARY:
		{
			long rc=get_int32(msg->data+12);

			if (process_remove(&somThis->running,p))
			{
				struct SOMDD_message *msg2=SOMMalloc(sizeof(*msg2)+16);
				msg2->ptr=msg2->data;
				msg2->length=12;
				put_int32(msg2->ptr,12);
				put_int32(msg2->ptr+4,RHBEXECD_RELEASE);
				put_int32(msg2->ptr+8,id);

				process_sendmsg(somThis,msg2);

				p->next=somThis->zombies;
				somThis->zombies=p;
			}

			if (somThis->implrep)
			{
#ifndef USE_THREADS
				somToken mtx=NULL;
#endif
				Environment ev;
				SOM_InitEnvironment(&ev);
				ImplRepositoryServer_obituary(
					somThis->implrep,
					&ev,
					proc,
					rc,
					mtx);
				SOM_UninitEnvironment(&ev);
			}
		}
		break;
	}

	*hProcess=proc;

/*	if (proc)
	{
		proc->lpVtbl->Release(proc);
	}
	*/
}

#if defined(_WIN32) && !defined(USE_SELECT_RHBPROCESSMGR)
static DWORD CALLBACK SOMDD_executor_read(void *pv)
{
RHBProcessMgr *executor=pv;
HANDLE fd=executor->hClientRead;

	if (fd != INVALID_HANDLE_VALUE)
	{
		DWORD dw;
		BOOL b=FALSE;
		long headerlen=0;
		octet header[12];
		struct SOMDD_message *msg=NULL;

		while (executor->hClientRead != INVALID_HANDLE_VALUE)
		{
			if (msg)
			{
				if (!msg->length)
				{
					RHBProcess *pProcess=NULL;
					SOMDD_MutexRef mtx=SOMDD_AcquireGlobalMutex();
					msg->length=get_int32((msg->ptr=msg->data));

					handle_incoming(executor,msg,mtx,&pProcess);

					SOMDD_ReleaseGlobalMutex(mtx);

					free(msg);
					msg=NULL;
					headerlen=0;
					if (pProcess)
					{
						pProcess->lpVtbl->Release(pProcess);
					}
				}
				else
				{
					b=ReadFile(fd,msg->ptr,msg->length,&dw,NULL);

					if (!b) break;
					if (!dw) break;

					msg->ptr+=dw;
					msg->length-=dw;
				}
			}
			else
			{
				b=ReadFile(fd,header+headerlen,sizeof(header)-headerlen,&dw,NULL);

				if (!b) break;
				if (!dw) break;

				headerlen+=dw;

				if (headerlen==sizeof(header))
				{
					headerlen=get_int32(header);

					msg=malloc(sizeof(*msg)+headerlen);

					memcpy((msg->ptr=msg->data),header,sizeof(header));
					msg->ptr+=sizeof(header);
					msg->length=headerlen-sizeof(header);
					msg->next=NULL;
				}
			}
		}

		b=CloseHandle(fd);

		RHBOPT_ASSERT(b);
	}

	return 0;
}
#endif

#if defined(USE_LINKED_RHBEXECD) && defined(_WIN32)
static DWORD CALLBACK rhbexecd_r(void *pv)
{
	RHBProcessMgr *somThis=pv;
	BOOL b=FALSE;

	rhbexecd_run(
#ifdef USE_SELECT_RHBPROCESSMGR
		somThis->childInfo->fdServerRead,
		somThis->childInfo->fdServerWrite,
#else
		somThis->childInfo->hServerRead,
		somThis->childInfo->hServerWrite,
#endif
		somThis->childInfo->hServerConsole);

#ifdef USE_SELECT_RHBPROCESSMGR
	soclose(somThis->childInfo->fdServerRead);

	if (somThis->childInfo->fdServerRead != somThis->childInfo->fdServerWrite)
	{
		soclose(somThis->childInfo->fdServerWrite);
	}

	somThis->childInfo->fdServerRead=INVALID_SOCKET;
	somThis->childInfo->fdServerWrite=INVALID_SOCKET;
#else
	b=CloseHandle(somThis->childInfo->hServerRead);

	RHBOPT_ASSERT(b);

	if (somThis->childInfo->hServerRead != somThis->childInfo->hServerWrite)
	{
		b=CloseHandle(somThis->childInfo->hServerWrite);

		RHBOPT_ASSERT(b);
	}

	somThis->childInfo->hServerRead=INVALID_HANDLE_VALUE;
	somThis->childInfo->hServerWrite=INVALID_HANDLE_VALUE;
#endif

	return 0;
}
#endif

int RHBProcessMgr_init(RHBProcessMgr *somThis)
{
#ifdef _WIN32
	BOOL b=FALSE;
#endif
	int err=-1;
	struct RHBProcessMgrChild *child=somThis->childInfo;

#ifdef USE_SELECT_RHBPROCESSMGR
	int rc=0;
	SOCKET stdin_pair[2]={INVALID_SOCKET};
	SOCKET stdout_pair[2]={INVALID_SOCKET};

	err=socketpair(AF_UNIX,SOCK_STREAM,0,stdin_pair);

	if (err) 
	{ 
		err=errno;
		perror("socketpair(stdin_pair)"); 
		return err ? err : 1;
	}

	err=socketpair(AF_UNIX,SOCK_STREAM,0,stdout_pair);

	if (err) 
	{ 
		err=errno;
		perror("socketpair(stdout_pair)"); 
		return err ? err : 1;
	}

	set_non_blocking(stdin_pair[0]);
	set_non_blocking(stdin_pair[1]);
	set_non_blocking(stdout_pair[0]);
	set_non_blocking(stdout_pair[1]);

#	ifdef _WIN32
		child->hServerConsole=GetStdHandle(STD_ERROR_HANDLE);
#	else
		child->fdServerError=2;
#	endif
	child->fdServerWrite=stdout_pair[1];
	child->fdServerRead=stdin_pair[0];

	somThis->fdClientRead=stdout_pair[0];
	somThis->fdClientWrite=stdin_pair[1];

#	ifdef _WIN32
		child->psinfo.hProcess=CreateThread(NULL,0,rhbexecd_r,somThis,0,
			&child->psinfo.dwProcessId);
#	else /*_WIN32 */
		child->pidServer=fork();

		if (child->pidServer==-1)
		{
			err=errno;
			perror("fork");

			return err ? err : 1;
		}

		if (!child->pidServer)
		{
			err=close(stdin_pair[1]);		/* write end */

			if (err)
			{
				perror("close stdin_pair child");
				_exit(1);
			}

			err=close(stdout_pair[0]);		/* read end */

			if (err)
			{
				perror("close stdout_pair child");
				_exit(1);
			}

			_exit(rhbexecd_run(
				stdin_pair[0],
				stdout_pair[1],
				2));
		}

		err=close(stdin_pair[0]);			/* read end */

		if (err) 
		{ 
			err=errno;
			perror("close stdin_pair parent"); 
			return err ? err : 1;
		}

		err=close(stdout_pair[1]);			/* write end */

		if (err) 
		{ 
			err=errno;
			perror("close stdout_pair parent"); 
			return err ? err : 1;
		}
#	endif /* _WIN32 */
#else /* USE_SELECT_RHBPROCESSMGR */
	int rc=1;
#	ifdef USE_LINKED_RHBEXECD
	b=CreatePipe(&child->hServerRead,&somThis->hClientWrite,NULL,4096);
	RHBOPT_ASSERT(b);
	b=CreatePipe(&somThis->hClientRead,&child->hServerWrite,NULL,4096);
	RHBOPT_ASSERT(b);
	child->hServerConsole=GetStdHandle(STD_ERROR_HANDLE);
	child->psinfo.hProcess=CreateThread(NULL,0,rhbexecd_r,somThis,0,&child->psinfo.dwProcessId);
#	else /* USE_LINKED_RHBEXECD */
	char buf[1024];
	char *p;
	STARTUPINFO startup;

	GetModuleFileName(NULL,buf+1,sizeof(buf)-2);

	buf[0]='\"';

	p=buf+strlen(buf);

	while (--p > buf)
	{
		if (*p=='.')
		{
			strncpy(p,"exe.exe",sizeof(buf)-(p-buf));
			break;
		}
	}

	strncat(buf,"\" /executor",sizeof(buf));

	memset(&startup,0,sizeof(startup));

	startup.cb=sizeof(startup);

	startup.dwFlags=STARTF_USESTDHANDLES;

	startup.hStdInput=INVALID_HANDLE_VALUE;
	startup.hStdOutput=INVALID_HANDLE_VALUE;
	startup.hStdError=GetStdHandle(STD_ERROR_HANDLE);

	b=CreatePipe(&startup.hStdInput,
				 &executor->hClientWrite,
				 NULL,
				 4096);

	RHBOPT_ASSERT(b);

	b=CreatePipe(&executor->hClientRead,
				 &startup.hStdOutput,
				 NULL,
				 4096);

	RHBOPT_ASSERT(b);

	b=shareable(&startup.hStdInput);

	RHBOPT_ASSERT(b);

	b=shareable(&startup.hStdOutput);

	RHBOPT_ASSERT(b);

	b=CreateProcess(NULL,buf,NULL,NULL,TRUE,0,NULL,NULL,&startup,&executor->psinfo);

	RHBOPT_ASSERT(b);

	if (b)
	{
		CloseHandle(executor->psinfo.hThread);
	}
	else
	{
		executor=NULL;
	}

	b=CloseHandle(startup.hStdInput);

	RHBOPT_ASSERT(b);

	b=CloseHandle(startup.hStdOutput);

	RHBOPT_ASSERT(b);
#	endif /* USE_LINKED_RHBEXECD */

	if (somThis)
	{
		DWORD tid=0;

		somThis->hWriteEvent=CreateEvent(NULL,FALSE,FALSE,NULL);

		RHBOPT_ASSERT(somThis->hWriteEvent);

		child->psinfo.hThread=CreateThread(NULL,0,SOMDD_executor_read,somThis,
			0,&child->psinfo.dwThreadId);

		RHBOPT_ASSERT(child->psinfo.hThread);

		rc=0;
	}
#endif /* USE_SELECT_RHBPROCESSMGR */

	return rc;
}

void RHBProcessMgr_term(RHBProcessMgr *somThis)
{
#ifdef _WIN32
	BOOL b=FALSE;
#endif /* _WIN32 */

	struct RHBProcessMgrChild *child=somThis->childInfo;

#ifdef USE_SELECT_RHBPROCESSMGR
	if (somThis->fdClientWrite != INVALID_SOCKET)
	{
		/* this should tell the server to shutdown */

		soclose(somThis->fdClientWrite);

		somThis->fdClientWrite=INVALID_SOCKET;
	}

	while (somThis->fdClientRead != INVALID_SOCKET)
	{
		fd_set fdr;
		int i;

		FD_ZERO(&fdr);
		FD_SET(somThis->fdClientRead,&fdr);

		i=select(somThis->fdClientRead+1,&fdr,NULL,NULL,NULL);

		if (i > 0)
		{
			if (FD_ISSET(somThis->fdClientRead,&fdr))
			{
				char buf[1];
				i=pipe_read(somThis->fdClientRead,buf,sizeof(buf));

				if (i < 1)
				{
					if ((!i) || 
#ifdef _WIN32
						(WSAGetLastError()!=WSAEWOULDBLOCK)
#else
						(errno!=EWOULDBLOCK)
#endif
						)
					{
						soclose(somThis->fdClientRead);
						somThis->fdClientRead=INVALID_SOCKET;
					}
				}
			}
		}
		else
		{
			if (i)
			{
#ifdef _WIN32
				__asm int 3;
#else
				perror("RHBProcessMgr_term select"); 
#endif
			}
		}
	}
#else /* USE_SELECT_RHBPROCESSMGR */
	if (somThis->hClientWrite!=INVALID_HANDLE_VALUE)
	{
		b=CloseHandle(somThis->hClientWrite);

		RHBOPT_ASSERT(b)

		somThis->hClientWrite=INVALID_HANDLE_VALUE;
	}
#endif

#ifdef _WIN32
	if (child->psinfo.hThread)
	{
		WaitForSingleObject(child->psinfo.hThread,INFINITE);

		b=CloseHandle(child->psinfo.hThread);

		RHBOPT_ASSERT(b);

		child->psinfo.hThread=NULL;
	}

	if (child->psinfo.hProcess)
	{
		WaitForSingleObject(child->psinfo.hProcess,INFINITE);

		b=CloseHandle(child->psinfo.hProcess);

		RHBOPT_ASSERT(b);

		child->psinfo.hProcess=NULL;
	}
#else
	if (child->pidServer && (child->pidServer!=-1))
	{
		int s=0;
		pid_t rc=-1;
/*		fprintf(stderr,"waiting for %d\n",child->pidServer);*/
		rc=waitpid(child->pidServer,&s,0);
		if (rc==-1)
		{
			perror("waitpid");
		}
/*		fprintf(stderr,"waited for %d, rc=%d, s=%d\n",
				child->pidServer,(int)rc,s);*/
	}
#endif

#ifdef USE_SELECT_RHBPROCESSMGR
#else
	if (somThis->hWriteEvent)
	{
		b=CloseHandle(somThis->hWriteEvent);
	
		RHBOPT_ASSERT(b);

		somThis->hWriteEvent=NULL;
	}
#endif
}

static int process_QI(RHBProcess *somSelf,void *iid,void **ref)
{
/*struct RHBProcessData *somThis=(void *)somSelf;*/

	return -1;
}

static int process_AddRef(RHBProcess *somSelf)
{
struct RHBProcessData *somThis=(void *)somSelf;

	return rhbatomic_inc(&somThis->usage);
}

static int process_Release(RHBProcess *somSelf)
{
struct RHBProcessData *somThis=(void *)somSelf;

	if (!rhbatomic_dec(&somThis->usage))
	{
#ifdef USE_THREADS
		SOMDD_MutexRef mtx=SOMDD_AcquireGlobalMutex();
#endif
		if (process_remove(&somThis->mgr->zombies,somThis))
		{
			SOMFree(somThis);
		}
	
#ifdef USE_THREADS
		SOMDD_ReleaseGlobalMutex(mtx);
#endif

		return 0;
	}

	return 1;
}

static int process_Equal(RHBProcess *somSelf,RHBProcess *other)
{
	if (somSelf==other) return 1;

	return 0;
}

static int process_Kill(RHBProcess *somSelf
#ifdef USE_THREADS
			,SOMDD_MutexRef ref
#endif
			)
{
	struct RHBProcessData *somThis=(void *)somSelf;
	struct SOMDD_message *msg=SOMMalloc(sizeof(*msg)+12);
	msg->ptr=msg->data;
	msg->length=12;

	put_int32(msg->data,12);
	put_int32(msg->data+4,RHBEXECD_KILL);
	put_int32(msg->data+8,somThis->id);

	process_sendmsg(somThis->mgr,msg);

	return 1;
}

static struct RHBProcessVtbl processVtbl={
	process_QI,
	process_AddRef,
	process_Release,
	process_Equal,
	process_Kill,
	process_Print
};

static unsigned long process_id(
	struct RHBProcessData **h,
	unsigned long id)
{
	struct RHBProcessData *p=*h;

	while (p)
	{
		if (id==p->id)
		{
			id++;
			p=*h;
		}
		else
		{
			p=p->next;
		}
	}

	return id;
}


RHBProcess *RHBProcessNew(
	  RHBProcessMgr *somThis,
	  Environment *ev,
	  char *programname,
	  char *impl_id,
	  char *alias
#ifdef USE_THREADS
	  ,SOMDD_MutexRef mutex
#endif
	  )
{
	struct RHBProcessData *retVal=NULL;
	unsigned long id=0;
	octet buf[4096];
	char confname[1024];
	octet *op=buf;

	id=process_id(&somThis->running,id);
	id=process_id(&somThis->zombies,id);

	op+=put_int32(op,0);
	op+=put_int32(op,RHBEXECD_LAUNCH);
	op+=put_int32(op,id);

	strncpy(confname,programname,sizeof(confname));

	if (confirm_program(confname,sizeof(confname)))
	{
		struct SOMDD_message *msg;
		long len;

		op+=put_string(op,confname);		
		op+=put_int32(op,2);
		op+=put_string(op,programname);
		op+=put_string(op,impl_id);
		op+=put_string(op,alias);

		len=(long)(op-buf);

		put_int32(buf,len);

		msg=SOMMalloc(len+sizeof(*msg));

		msg->next=NULL;
		memcpy((msg->ptr=msg->data),buf,(msg->length=len));

		process_sendmsg(somThis,msg);

		retVal=SOMMalloc(sizeof(*retVal));
		retVal->usage=1;
		retVal->id=id;
		retVal->next=somThis->running;
		retVal->mgr=somThis;
		retVal->comobj.lpVtbl=&processVtbl;
		somThis->running=retVal;
	}
	else
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,CouldNotStartProcess,NO);
	}

	if (retVal)
	{
		return &retVal->comobj;
	}

	return NULL;
}

#ifdef USE_SELECT_RHBPROCESSMGR
static void process_select_write(RHBProcessMgr *somThis
#ifdef USE_THREADS
		,SOMDD_MutexRef mutex
#endif
		)
{
	struct SOMDD_message *msg=somThis->outgoing;

	if (msg)
	{
		int i=-1;
		
		if (somThis->fdClientWrite!=INVALID_SOCKET)
		{
			i=pipe_write(somThis->fdClientWrite,msg->ptr,msg->length);

			if (i > 0)
			{
				msg->length-=i;
				msg->ptr+=i;

				if (!msg->length)
				{
					somThis->outgoing=msg->next;

					SOMFree(msg);
				}
			}
			else
			{
				RHBOPT_ASSERT(i!=-1);
			}
		}

		if (i <= 0)
		{
#ifdef _WIN32
			__asm int 3
#else
			perror("process_select_write");
#endif
		}
	}
}

static void process_select_read(RHBProcessMgr *somThis,
#ifdef USE_THREADS
								SOMDD_MutexRef mutex,
#endif
								RHBProcess **hProcess)
{
	if (somThis->fdClientRead != INVALID_SOCKET)
	{
		int i=-1;

		if (somThis->incoming && somThis->incoming->length)
		{
			struct SOMDD_message *msg=somThis->incoming;

			i=pipe_read(somThis->fdClientRead,
				        msg->ptr,
						msg->length);

			if (i > 0)
			{
				msg->ptr+=i;
				msg->length-=i;

				if (!msg->length)
				{
					msg->length=get_int32((msg->ptr=msg->data));

					somThis->incoming=msg->next;
					somThis->header_len=0;

					handle_incoming(somThis,msg,
#ifdef USE_THREADS
						mutex,
#endif
						hProcess);

					free(msg);
				}
			}
		}
		else
		{
			int k=sizeof(somThis->header)-somThis->header_len;

			i=pipe_read(somThis->fdClientRead,
				        somThis->header+somThis->header_len,
						k);

			if (i > 0)
			{
				somThis->header_len+=i;

				if (somThis->header_len==sizeof(somThis->header))
				{
					int m=get_int32(somThis->header);
					struct SOMDD_message *msg=malloc(sizeof(*msg)+m);

					msg->next=somThis->incoming;
					somThis->incoming=msg;
					msg->ptr=msg->data+sizeof(somThis->header);
					msg->length=m-sizeof(somThis->header);
					memcpy(msg->data,somThis->header,sizeof(somThis->header));
				}
			}
		}

		if (i <= 0)
		{
			if (i)
			{
#ifdef _WIN32
				__asm int 3
#else
				perror("process_select_read");
#endif
			}
			else
			{
				SOCKET fd=somThis->fdClientRead;
				somThis->fdClientRead=INVALID_SOCKET;
				soclose(fd);

#ifdef USE_THREADS
#else
				{
					Environment ev;
					SOM_InitEnvironment(&ev);
					sink_remove(somThis,&ev,somThis->childInfo->sinks);
					SOM_UninitEnvironment(&ev);
				}
#endif

#ifdef _WIN32
				{
					BOOL b=GenerateConsoleCtrlEvent(
						CTRL_BREAK_EVENT,
						GetCurrentProcessId());

					RHBOPT_ASSERT(b);
				}
#else
				kill(getpid(),SIGTERM);
#endif
			}
		}
	}
}

#	ifdef USE_THREADS
static void process_select_signal(RHBProcessMgr *somThis
				,SOMDD_MutexRef mutex
				)
{
	char buf[1];
	int i=pipe_read(somThis->fdSignalRead,buf,sizeof(buf));
	if (i)
	{
		RHBOPT_ASSERT(i==1);
	}
}
#	endif
#endif


#ifdef USE_SELECT_RHBPROCESSMGR
#	ifdef USE_THREADS
static void *process_select(void *pv)
{
RHBProcessMgr *somThis=pv;
SOMDD_MutexRef mtx=SOMDD_AcquireGlobalMutex();

	while (!somThis->bQuitWrite)
	{
		RHBProcess *pProcess=NULL;
		fd_set fdr,fdw;
		SOCKET n=somThis->fdSignalRead;
		int i=-1;

		FD_ZERO(&fdr);
		FD_ZERO(&fdw);

		RHBOPT_ASSERT(somThis->fdSignalRead != INVALID_SOCKET);
		RHBOPT_ASSERT(somThis->fdSignalWrite != INVALID_SOCKET);

		FD_SET(somThis->fdSignalRead,&fdr);

		if (somThis->outgoing && (somThis->fdClientWrite != INVALID_SOCKET))
		{
			FD_SET(somThis->fdClientWrite,&fdw);

			if (somThis->fdClientWrite > n) n=somThis->fdClientWrite;
		}

		if (somThis->fdClientRead != INVALID_SOCKET)
		{
			FD_SET(somThis->fdClientRead,&fdr);

			if (somThis->fdClientRead > n) n=somThis->fdClientRead;
		}

		SOMDD_ReleaseGlobalMutex(mtx);

		i=select(n+1,&fdr,&fdw,NULL,NULL);

		mtx=SOMDD_AcquireGlobalMutex();

		if (i > 0)
		{
			if (somThis->fdSignalRead!= INVALID_SOCKET)
			{
				if (FD_ISSET(somThis->fdSignalRead,&fdr))
				{
					process_select_signal(somThis,mtx);
				}
			}

			if (somThis->fdClientRead != INVALID_SOCKET)
			{
				if (FD_ISSET(somThis->fdClientRead,&fdr))
				{
					process_select_read(somThis,mtx,&pProcess);
				}
			}

			if ((somThis->fdClientWrite != INVALID_SOCKET) && (somThis->outgoing))
			{
				if (FD_ISSET(somThis->fdClientWrite,&fdw))
				{
					process_select_write(somThis,mtx);
				}
			}
		}

		if (pProcess)
		{
			SOMDD_ReleaseGlobalMutex(mtx);

			pProcess->lpVtbl->Release(pProcess);

			mtx=SOMDD_AcquireGlobalMutex();
		}
	}

	somThis->bQuitWrite=0;

	pthread_cond_signal(&somThis->childInfo->eventSelect);

	SOMDD_ReleaseGlobalMutex(mtx);

	return NULL;
}
#	else	/* USE_THREADS */
static void SOMLINK sink_read(SOMObject SOMSTAR obj,void *pv)
{
	struct emanSink *sink=pv;
	RHBProcessMgr *somThis=sink->mgr;
	RHBProcess *pProcess=NULL;
	process_select_read(somThis,&pProcess);
	if (pProcess)
	{
		pProcess->lpVtbl->Release(pProcess);
	}
}
static void SOMLINK sink_write(SOMObject SOMSTAR obj,void *pv)
{
	struct emanSink *sink=pv;
	RHBProcessMgr *somThis=sink->mgr;

	if ((sink->mask==EMInputWriteMask)&&(!somThis->outgoing))
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		sink_remove(somThis,&ev,sink);
		SOM_UninitEnvironment(&ev);
	}
	else
	{
		process_select_write(somThis);
	}
}
#	endif	/* USE_THREADS */
#else /* USE_SELECT_RHBPROCESSMGR */
#endif /* USE_SELECT_RHBPROCESSMGR */

int RHBProcessMgr_begin(RHBProcessMgr *somThis,
						Environment *ev)
{
	int rc=0;
#ifdef USE_SELECT_RHBPROCESSMGR
#	ifdef USE_THREADS
	SOCKET pair[2]={INVALID_SOCKET,INVALID_SOCKET};
	
	int err=socketpair(AF_UNIX,SOCK_STREAM,0,pair);

	if (!err)
	{
		somThis->fdSignalRead=pair[0];
		somThis->fdSignalWrite=pair[1];

		pthread_cond_init(&somThis->childInfo->eventSelect,
			RHBOPT_pthread_condattr_default);
		err=pthread_create(&somThis->childInfo->tidSelect,
			RHBOPT_pthread_attr_default,
			process_select,
			somThis);
	}
	if (err)
	{
		err=errno;
		perror("processmgr_begin");
		rc=err ? err : 1;
	}
#	else
	{
		RHBOPT_ASSERT(somThis->childInfo->emanObject);

		sink_add(somThis,ev,somThis->childInfo->sinks,
				 somThis->fdClientRead,
				 EMInputReadMask,sink_read);
		sink_add(somThis,ev,somThis->childInfo->sinks+1,
				 somThis->fdClientWrite,
				 EMInputWriteMask,sink_write);
	}
#	endif
#else  /* USE_SELECT_RHBPROCESSMGR */
	DWORD tid;
	RHBOPT_ASSERT(!(somThis->hWriteThread));

	somThis->hWriteThread=CreateThread(NULL,0,SOMDD_executor_write,somThis,0,&tid);

	RHBOPT_ASSERT(somThis->hWriteThread);

	if (!somThis->hWriteThread)
	{
		rc=GetLastError();
		if (!rc) rc=1;
	}
#endif /* USE_SELECT_RHBPROCESSMGR */
	return rc;
}

void RHBProcessMgr_end(RHBProcessMgr *somThis)
{
#ifdef USE_THREADS
	SOMDD_MutexRef mtx=SOMDD_AcquireGlobalMutex();
#endif

	somThis->bQuitWrite=TRUE;

#ifdef USE_SELECT_RHBPROCESSMGR
#	ifdef USE_THREADS
	pipe_write(somThis->fdSignalWrite,"",1);

	while (somThis->bQuitWrite)
	{
		pthread_cond_wait(&somThis->childInfo->eventSelect,mtx);
	}

	SOMDD_ReleaseGlobalMutex(mtx);

	{
		void *pv=NULL;
		pthread_join(somThis->childInfo->tidSelect,&pv);
		pthread_cond_destroy(&somThis->childInfo->eventSelect);
	}

	if (somThis->fdSignalRead!=INVALID_SOCKET)
	{
		soclose(somThis->fdSignalRead);
		somThis->fdSignalRead=INVALID_SOCKET;
	}

	if (somThis->fdSignalWrite!=INVALID_SOCKET)
	{
		soclose(somThis->fdSignalWrite);
		somThis->fdSignalWrite=INVALID_SOCKET;
	}
#	else /* USE_THREADS */

	while (somThis->outgoing)
	{
		process_select_write(somThis);
	}

	while (somThis->zombies)
	{
		sink_read(NULL,somThis->childInfo->sinks);
	}

	{
		Environment ev;
		int i=sizeof(somThis->childInfo->sinks)/sizeof(somThis->childInfo->sinks[0]);
		SOM_InitEnvironment(&ev);

		while (i--)
		{
			sink_remove(somThis,&ev,somThis->childInfo->sinks+i);
		}

		SOM_UninitEnvironment(&ev);
	}
#	endif /* USE_THREADS */

#else /* USE_SELECT_RHBPROCESSMGR */
	SOMDD_ReleaseGlobalMutex(mtx);

	if (somThis->hWriteThread)
	{
		BOOL b=SetEvent(somThis->hWriteEvent);

		RHBOPT_ASSERT(b);

		WaitForSingleObject(somThis->hWriteThread,INFINITE);

		b=CloseHandle(somThis->hWriteThread);

		RHBOPT_ASSERT(b);

		somThis->hWriteThread=NULL;
	}
#endif

	RHBOPT_ASSERT(!somThis->incoming)
	RHBOPT_ASSERT(!somThis->outgoing)
	RHBOPT_ASSERT(!somThis->zombies)
	RHBOPT_ASSERT(!somThis->running)
}

RHBProcess *RHBProcessMgr_Find(
	   RHBProcessMgr *somThis,
	   unsigned long pid
#ifdef USE_THREADS
		,SOMDD_MutexRef mutex
#endif
	   )
{
	return NULL;
}


