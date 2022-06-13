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

/*******************************************************************
 * the major difference between the UNIX/Windows implementation
 * and the Mac, is that the Mac uses AppleEvents from the servers
 * to the daemon, additionally, servers are started using
 * LaunchApplication rather than fork/CreateProcess

 * with UNIX server death is detected by signal SIGCLD then waitpid()
 * with Windows, a thread sits waiting for the server to die
 * with Macintosh it gets an obituary AppleEvent
 * 
 */

/********************************************************************
 * corba specifies the following ports
 *
 * 1. corba-iiop		683			iiop
 * 2. corba-iiop-ssl	684			iiop-ssl
 * 3. corba-loc		    2809		corbaloc (LocateRequest handler)
 *
 */

#include <rhbopt.h>
#include <rhbsomdd.h>
#include <rhbsomuk.h>
#include <time.h>
#include <containd.h>
#include <containr.h>
#include <repostry.h>
#include <somestio.h>
/* #include <somdsvr.h> */

#ifdef _WIN32
	#include <direct.h>
	#include <io.h>
	#include <stdio.h>
	#include <process.h>
#endif

#ifdef _PLATFORM_UNIX_
	#include <sys/resource.h>
	#include <sys/types.h>
	#include <sys/time.h>
	#ifdef __AIX__
		#include <sys/select.h>
	#endif
	#include <unistd.h>
	#include <pwd.h>
	#include <sys/param.h>
	#include <sys/wait.h>
	#include <sys/socket.h>
	#include <syslog.h>
#endif

#define SOM_Module_somddsrv_Source
#define SOM_Module_implreps_Source

#include <somddsrv.h>
#include <implreps.h>

SOMDD_DEFMSG(SOMDD_LOGMSG_FAILED_ACTIVATE,ERROR,
			 SOMDDMSG_SOMDDActivateFailed,
			 "somdd failed to activate with exception %1")
SOMDD_DEFMSG(SOMDD_LOGMSG_INTERRUPT,OK,
			 SOMDDMSG_SOMDDInterrupt,
			 "somdd was interrupted")
SOMDD_DEFMSG(SOMDD_LOGMSG_READY,OK,
			 SOMDDMSG_SOMDDReady,
			 "SOMDD Ready")
SOMDD_DEFMSG(SOMDD_LOGMSG_SHUTDOWN,OK,
			 SOMDDMSG_SOMDDShutdown,
			 "SOMDD shutting down")

#define stdout_somPrintf	somPrintf

static boolean somdd_quit_flag;
boolean SOMDD_running;

#ifdef USE_THREADS
#ifdef _DEBUG
	#ifdef USE_PTHREADS
		static pthread_t somdd_mutex_owner,zero_tid;
	#else
		#define zero_tid			(0)
		static DWORD somdd_mutex_owner;
	#endif
#endif
static struct SOMDD_run_r_t *SOMDD_active;
#endif

#ifdef _WIN32
typedef DWORD (__stdcall *QueueUserAPC_t)(
		void (__stdcall *APCProc)(ULONG),
		HANDLE hThread,
		ULONG dwData);
#endif

static void drop_class(SOMClassMgr SOMSTAR mgr,SOMClass SOMSTAR cls)
{
	if (mgr && cls)
	{
		boolean b=0;
		_IDL_SEQUENCE_SOMClass seq=SOMClassMgr__get_somRegisteredClasses(mgr);

		if (seq._buffer)
		{
			unsigned long i=seq._length;

			while (i--)
			{
				SOMClass SOMSTAR other=seq._buffer[i];
				if (other==cls)
				{
					b=1;
				}
				else
				{
					if (SOMClass_somDescendedFrom(other,cls))
					{
						b=0;

						SOMFree(seq._buffer);
						seq._buffer=NULL;

						drop_class(mgr,other);

						seq=SOMClassMgr__get_somRegisteredClasses(mgr);
						i=seq._length;
					}
				}
			}

			SOMFree(seq._buffer);

			if (b)
			{
/*				somPrintf("SOMClassMgr_somUnregisterClass(%s)\n",
					SOMClass_somGetName(cls));*/

				SOMClassMgr_somUnregisterClass(mgr,cls);
			}
		}
	}
}

static SOMClass SOMSTAR SOMDD_Server_classObject=NULL;

#ifdef USE_THREADS
#else
	static SOMEEMan SOMSTAR SOMDD_SOMEEManObject;
	static void SOMLINK SOMDD_Callback(void *,void *);
#endif

#ifdef _WIN32
	#ifndef _CONSOLE
		static int SOMLINK SOMDD_OutChar(char c);
	#endif
#endif

#ifndef _PLATFORM_MACINTOSH_
char *SOMDD_get_repos_filename(int argc,char **argv)
{
	static char buf[256];

	if (argc > 1)
	{
		int i=1;

		while (i < argc)
		{
			if (!strcmp(argv[i],"-f"))
			{
				if ((i+1) < argc)
				{
					return argv[i+1];
				}
			}

			i++;
		}
	}

	if (somutgetpath(buf)) return NULL;

	strncat(buf,"impl.db",sizeof(buf)-1);

	RHBSOM_Trace(buf);

	return buf;
}
#endif

#ifdef USE_THREADS
struct
{
#ifdef USE_PTHREADS
	pthread_mutex_t mutex;
#else
	HANDLE mutex;
#endif
	const char *file;
	int line;
} SOMDD_serialize={
#ifdef USE_PTHREADS
	#ifdef RHBOPT_PTHREAD_MUTEX_INITIALIZER
		RHBOPT_PTHREAD_MUTEX_INITIALIZER
	#else
		{0}
	#endif
#else
		NULL
#endif
	,__FILE__,__LINE__
};
#endif


static void somdd_bootstrap(Environment *ev,int argc,char **argv)
{
	char *f=SOMDD_get_repos_filename(argc,argv);

	if (f)
	{
	#ifdef somNewObject
		somStream_StreamIO SOMSTAR stream=somNewObject(somStream_MemoryStreamIO);
	#else
		somStream_StreamIO SOMSTAR stream=somStream_MemoryStreamIONew();
	#endif
		SOMObject SOMSTAR old_implrep=NULL;
		FILE *fp=fp=fopen(f,"rb");

	/*	somPrintf("Loading ImplRepository from %s\n",f);*/
	/*	somPrintf("%s\n",f);*/

		if (fp)
		{
			_IDL_SEQUENCE_octet seq={0,0,NULL};
			unsigned long len;
			fseek(fp,0,SEEK_END);
			len=ftell(fp);
			fseek(fp,0,SEEK_SET);
			seq._length=len;
			seq._maximum=seq._length;
			seq._buffer=SOMMalloc(seq._length);
			fread(seq._buffer,seq._length,1,fp);
			fclose(fp);
			somStream_StreamIO_set_buffer(stream,ev,&seq);
			if (seq._buffer) SOMFree(seq._buffer);
		}
		else
		{
			char *p=NULL;
			ImplementationDef SOMSTAR somdd_impl=NULL;
			SOMClass SOMSTAR cls=NULL;

	#ifdef _PLATFORM_MACINTOSH_
			SOMDD_prepare_repos_file(f);
	#endif

			cls=ImplementationDef_somGetClass(SOMD_ImplDefObject);

			somdd_impl=SOMClass_somNew(cls);

			somReleaseClassReference(cls);

			p=ImplementationDef__get_impl_id(SOMD_ImplDefObject,ev);

			ImplementationDef__set_impl_id(somdd_impl,ev,p);

			SOMFree(p);

			p=SOMObject_somGetClassName(somdd_impl);
			CosStream_StreamIO_write_unsigned_long(stream,ev,1);
			CosStream_StreamIO_write_string(stream,ev,p);

			ImplementationDef_externalize_to_stream(
				somdd_impl,
				ev,
				stream);

			somReleaseObjectReference(somdd_impl);

			CosStream_StreamIO_write_unsigned_long(stream,ev,2); /* configured classes */
			CosStream_StreamIO_write_string(stream,ev,"ImplRepository");
			CosStream_StreamIO_write_string(stream,ev,"ImplementationDef");

			/* write out repository */

			/* rewind so stream can be read from */

			fp=fopen(f,"wb");

			if (fp)
			{
				_IDL_SEQUENCE_octet seq=somStream_StreamIO_get_buffer(stream,ev);
				fwrite(seq._buffer,seq._length,1,fp);
				fclose(fp);
				SOMFree(seq._buffer);
			}

			somStream_StreamIO_reset(stream,ev);
		}

		old_implrep=SOMD_ImplRepObject;

	#ifdef somNewObject
		SOMD_ImplRepObject=somNewObject(ImplRepositoryServer);
	#else
		SOMD_ImplRepObject=ImplRepositoryServerNew();
	#endif

		ImplRepositoryServer_init_from_stream(SOMD_ImplRepObject,ev,stream);

		if (!ev->_major)
		{
			ImplRepositoryServer__set_implrep_filename(SOMD_ImplRepObject,ev,f);
		}

		if (stream) somReleaseObjectReference(stream);

		if (old_implrep) somReleaseObjectReference(old_implrep);
	}
	else
	{
		RHBOPT_throw_StExcep(ev,INITIALIZE,BadEnvironment,NO);
	}
}

#ifdef USE_THREADS
RHBOPT_cleanup_begin(SOMDD_WaitInWaitingRoom_cleanup,pv)

RHBServerPatient *thunk=pv;
RHBServerWaitingRoom *room=thunk->room;
RHBServerPatient *n=room->list;

	if (n==thunk)
	{
		room->list=thunk->next;
	}
	else
	{
		while (n->next != thunk)
		{
			n=n->next;

			if (!n) 
			{
				RHBOPT_ASSERT(n)
				break;
			}
		}

		if (n) n->next=thunk->next;
	}

#ifdef USE_PTHREADS
	pthread_cond_destroy(&thunk->event);
#else
	CloseHandle(thunk->h_event);
#endif
RHBOPT_cleanup_end

void SOMDD_WaitInWaitingRoom(RHBServerWaitingRoom *room,SOMDD_MutexRef m)
{
RHBServerPatient thunk;
#ifdef USE_PTHREADS
	time_t tv;
	struct timespec ts;
#endif

	RHBOPT_ASSERT(m)

	thunk.room=room;

#ifdef USE_PTHREADS
	pthread_cond_init(&thunk.event,RHBOPT_pthread_condattr_default);
#else
	thunk.h_event=CreateEvent(NULL,0,0,NULL);
#endif

	RHBOPT_cleanup_push(SOMDD_WaitInWaitingRoom_cleanup,&thunk);

	thunk.next=room->list;
	room->list=&thunk;

#ifdef USE_PTHREADS
	time(&tv);
	ts.tv_sec=tv+30;
	ts.tv_nsec=0;
	pthread_cond_timedwait(&thunk.event,m,&ts);
#else
	ReleaseMutex(m);
	WaitForSingleObject(thunk.h_event,30000);
	WaitForSingleObject(m,INFINITE);
#endif

	RHBOPT_cleanup_pop();
}
#endif

#ifdef USE_THREADS
void SOMDD_NotifyWaitingRoom(RHBServerWaitingRoom *room,SOMDD_MutexRef m)
{
RHBServerPatient *n=room->list;

	while (n)
	{
#ifdef USE_PTHREADS
		pthread_cond_signal(&n->event);
#else
		if (!SetEvent(n->h_event))
		{
#ifdef _M_IX86
			__asm int 3;
#endif
		}
#endif
		n=n->next;
	}
	RHBOPT_unused(m)
}
#endif

#ifdef USE_SELECT
static void SOMLINK SOMDD_Callback(void *a,void *b)
{
	Environment ev;
	ImplRepositoryServer SOMSTAR rep;
	static char spinChars[]="-\\|/";
	static int spin;
	RHBOPT_unused(a)
	RHBOPT_unused(b)

	SOM_InitEnvironment(&ev);
	SOMOA_execute_request_loop(SOMD_SOMOAObject,&ev,SOMD_NO_WAIT);
	somPrintf("%c\010",spinChars[0x3&(spin++)]);
	SOM_UninitEnvironment(&ev);

	SOM_InitEnvironment(&ev);
	rep=SOMD_ImplRepObject;
	if (rep)
	{
		ImplRepositoryServer_handle_requests(rep,&ev,RESP_NO_WAIT);
	}
	SOM_UninitEnvironment(&ev);
}

static void SOMDD_signal_handler(int i)
{
	static int ignore;

	if (ignore) return;

	if (SOMD_SOMOAObject)
	{
		Environment ev;
		SOMOA SOMSTAR ref=SOMOA_somDuplicateReference(SOMD_SOMOAObject);

		if (ref)
		{

			SOM_InitEnvironment(&ev);
			switch (i)
			{
#ifdef SIGHUP
			case SIGHUP:
#endif
			case SIGINT:
				{
					somId id=somIdFromString("interrupt_server_status");
					somToken tok=NULL;
					somva_SOMObject_somDispatch(ref,&tok,id,ref,&ev,SOMDERROR_SocketClose);
					SOMFree(id);
				}
				break;
			default:
				somdd_quit_flag=1;
				ignore=1;
				SOMOA_interrupt_server(ref,&ev);
				break;
			}

			SOM_UninitEnvironment(&ev);

			somReleaseObjectReference(ref);
		}
	}
}
#endif

#ifdef _WIN32
	/* main exists is separate file for WIN32 */
#else
	#ifdef _PLATFORM_MACINTOSH_
/* main() currently exists in a separate file.. */
	#else /* _PLATFORM_MACINTOSH_ */
	/* UNIX process */
		int main(int argc,char **argv)
		{
			int doFork;
			int i;
			sigset_t sigs;
			RHBProcessMgrChild childInfo=RHBPROCESSMGRCHILD_INIT;
			RHBProcessMgr executor={
				RHBPROCESSMGR_INIT_COMMON,
					RHBPROCESSMGR_INIT};

			executor.childInfo=&childInfo;

			sigemptyset(&sigs);
			sigaddset(&sigs,SIGINT);
			sigaddset(&sigs,SIGTERM);
			sigaddset(&sigs,SIGIO);
#ifdef SIGPIPE
			sigaddset(&sigs,SIGPIPE);
#endif
#ifdef SIGQUIT
			sigaddset(&sigs,SIGQUIT);
#endif
#ifdef SIGBREAK
			sigaddset(&sigs,SIGBREAK);
#endif
#ifdef SIGCHLD
			sigaddset(&sigs,SIGCHLD);
#else
			sigaddset(&sigs,SIGCLD);
#endif
			sigaddset(&sigs,SIGHUP);
#ifdef USE_PTHREADS
			pthread_sigmask(SIG_BLOCK,&sigs,NULL);
#else
			sigprocmask(SIG_BLOCK,&sigs,NULL);
#endif

			/* the doFork flag is to spawn off as
				a daemon detached from any console */

			doFork=1;

			doFork=0;

			i=1;

			while (i < argc)
			{
				if (!strcmp(argv[i],"-init"))
				{
					doFork=0;
				}
				else
				{
					if (!strcmp(argv[i],"-fork"))
					{
						doFork=1;
					}
				}

				i++;
			}

			if (doFork)
			{
#ifdef RLIMIT_NOFILE
				struct rlimit resourceLimit={0};
#endif
				int pipes[2]={-1,-1};
				struct passwd *pw;

				pw=getpwuid(getuid());

				if (pw)
				{
					printf("somdd starting as user '%s'\n",pw->pw_name);
				}

/*				if (isatty(1))
				{
					char *tty=ttyname(1);
					if (tty)
					{
						printf("stdout is '%s'\n",tty);
					}
				}
*/
				i=pipe(pipes);

				if (i==-1)
				{
					fprintf(stderr,"%s:%d, pipe creation failed with %d\n",
							__FILE__,__LINE__,errno);

					return 1;
				}

				i=fork();

				if (i==-1)
				{
					fprintf(stderr,"%s:%d, fork() failed\n",__FILE__,__LINE__);
					return 1;
				}

				if (i) 
				{
		/*			printf("fork started process %d\n",i);*/

					close(pipes[1]); /* we don't need the write end of the pipe */

					/* read the pid that was piped back */

					if (read(pipes[0],(void *)&i,sizeof(i)) != sizeof(i))
					{
						fprintf(stderr,"reading pid failed, %d\n",errno);

						return 1;
					}

					close(pipes[0]);

					if (argc > 0)
					{
						printf("%s spawned as %d\n",argv[0],i);
					}
					else
					{
						printf("somdd(%d) spawned as %d\n",(int)getpid(),i);
					}

					i=wait(NULL);

			/*		printf("child process %d finished\n",i);
					printf("process %d finished\n",getpid());
			*/
					return 0;
				}

#ifdef RLIMIT_NOFILE
				resourceLimit.rlim_max=0;
				getrlimit(RLIMIT_NOFILE,&resourceLimit);
				i=resourceLimit.rlim_max;
#else
				i=FD_SETSIZE;
#endif

				/* close all files except the pipe we need to write
						the daemons pid back */

				if (i <= pipes[1])
				{
#ifdef _DEBUG
					printf("%d is a rather low number of fds\n",i);
#endif
					/* should be at least five,stdin,stdout,stderr,2 pipes */

#ifdef NOFILE
					i=NOFILE;
#else
					i=64;
#endif
				}

				while (i--)
				{
					if (i != pipes[1])
					{
						close(i);
					}
				}

				/* this creates a new process session with this process
					as the group leader, which then quits and
					leaves the daemon totally isolated from a tty
					*/

				setsid();

				i=fork();

				if (i==-1)
				{
					return 1;
				}

				if (i)
				{
					/* as the parent, write back pid of what was started */

					write(pipes[1],(void *)&i,sizeof(i));
					close(pipes[1]);

					return 0;
				}

				close(pipes[1]);

				/* change current dir to root so mount and umount won't
					affect the state of the daemon */
				/* reopen stdin, stdout and stderr as bit-bucket */

				i=chdir("/");
				i=open("/dev/null",O_RDWR);
				dup(i);
				dup(i);
			}

			i=RHBProcessMgr_init(&executor);

			if (!i)
			{
				i=SOMDD_main(0,&executor,argc,argv);

				RHBProcessMgr_term(&executor);
			}

			return i;
		}
	#endif /* not _PLATFORM_MACINTOSH_ */
#endif

#ifndef _PLATFORM_MACINTOSH_
#ifdef _WIN32
#define getpid()     (int)GetCurrentThreadId()
#endif
#endif

#ifdef _WIN32
static int must_quote(const char *p)
{
	while (*p)
	{
		int c=*p++;
		if (isspace(c))
		{
			return 1;
		}
	}
	return 0;
}
#endif

#ifdef _WIN32
	#ifndef _CONSOLE
static int SOMLINK SOMDD_OutChar(char c)
{
static char buf[256];
static int len;

	buf[len++]=c;
	buf[len]=0;

	if ((len==sizeof(buf))||(c < ' '))
	{
		OutputDebugString(buf);
		buf[0]=0;
		len=0;
	}

	return 1;
}
	#endif
#endif

#ifdef _PLATFORM_UNIX_
void SOMDD_up(struct SOMDD_daemon *daemon)
{
}
void SOMDD_down(struct SOMDD_daemon *daemon)
{
}
#endif

static boolean check_for_quit(void)
{
#ifdef _WIN32_x
	MSG msg;
	if (PeekMessage(&msg,(HWND)-1L,WM_QUIT,WM_QUIT,PM_NOREMOVE))
	{
		somPrintf("PeekMessage reports a WM_QUIT waiting\n");
		return 1;
	}
#endif

	if (somdd_quit_flag) return 1;

	return 0;
}

#if defined(_WIN32S) && defined(_DEBUG) && defined(RHBOPT_SHARED_DATA)
static int SOMLINK my_out_aslm(char c)
{
	HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
	if (h && (h!=INVALID_HANDLE_VALUE))
	{
		DWORD dw=0;
		char buf[1];
		buf[0]=c;
		if (WriteFile(h,buf,1,&dw,NULL))
		{
			return (int)dw;
		}
	}

	return 0;
}
static DWORD CALLBACK my_test_aslm(void *pv)
{
	SOMClassMgr SOMSTAR mgr=somEnvironmentNew();
	Environment ev;
	SOM_InitEnvironment(&ev);
	somSetOutChar(my_out_aslm);
	SOMClassMgr_somDumpSelf(mgr,0);
	SOM_UninitEnvironment(&ev);
	somEnvironmentEnd();
	return 0;
}
#endif

static ORBStatus SOMDD_run(
		struct SOMDD_daemon *daemon,
		RHBProcessMgr *procMgr,
		Environment *ev,int argc,char **argv)
{
ORBStatus status=SOMDERROR_NoSOMDInit;

	SOMD_Init(ev);

	if (ev->_major) 
	{
		return status;
	}

	status=SOMDERROR_ServerNotActive;

#ifndef USE_THREADS
	SOMD_RegisterCallback(SOMDD_SOMEEManObject,SOMDD_Callback);
#endif

	RHBOPT_ASSERT(!SOMD_ImplDefObject)

	if (!SOMD_ImplDefObject)
	{
		/* this is where we manufacture the SOMDD implementationdef
		*/

		SOMD_ImplDefObject=SOMDD_create_initial_ImplDef(ev);

		if ((!SOMD_ImplDefObject) && !ev->_major)
		{
			_IDL_SEQUENCE_ImplementationDef seq=ImplRepository_find_impldef_by_class(SOMD_ImplRepObject,ev,"ImplRepository");

			if (!ev->_major)
			{
				if (seq._length)
				{
					SOMD_ImplDefObject=seq._buffer[0];
					seq._buffer[0]=NULL;

					while (seq._length--)
					{
						ImplementationDef SOMSTAR impl=seq._buffer[seq._length];
						seq._buffer[seq._length]=NULL;

						if (impl) somReleaseObjectReference(impl);
					}
				}

				if (seq._maximum)
				{
					SOMFree(seq._buffer);
				}
			}
		}
	}

	if (!SOMD_SOMOAObject)
	{
#ifdef somNewObject
		SOMD_SOMOAObject=somNewObject(SOMOA);
#else
		SOMD_SOMOAObject=SOMOANew();
#endif
	}

	if (ev->_major || check_for_quit())
	{
		if (ev->_major)
		{
			stdout_somPrintf("SOMDD - exception - %s\n",somExceptionId(ev));
			somExceptionFree(ev);
		}
	}
	else
	{
		boolean activated=0;

		RHBSOM_Trace("SOMDD - calling somdd_bootstrap")

		somdd_bootstrap(ev,argc,argv);

		if (!ev->_major)
		{
			ImplRepositoryServer__set_process_mgr(SOMD_ImplRepObject,ev,procMgr);
			procMgr->implrep=SOMD_ImplRepObject;
		}

		if (!ev->_major)
		{
			char *p=SOMClass_somGetName(SOMDD_Server_classObject);

			ImplementationDef__set_impl_server_class(SOMD_ImplDefObject,ev,p);

			SOMOA_impl_is_ready(SOMD_SOMOAObject,ev,SOMD_ImplDefObject);

			if (!ev->_major)
			{
				Environment ev2;

				SOM_InitEnvironment(&ev2);

				ImplementationDef__set_impl_server_class(
					SOMD_ImplDefObject,&ev2,0);

				SOM_UninitEnvironment(&ev2);
			}
		}

		if (ev->_major)
		{
			char *p=somExceptionId(ev);

			RHBSOM_Trace(p);
#ifdef _PLATFORM_MACINTOSH_
			SOMDD_failed(p);
#else
			stdout_somPrintf("SOMDD failed with exception %s\n",p);

			SOMDD_syslog(&SOMDD_LOGMSG_FAILED_ACTIVATE,p,NULL);
#endif
			somExceptionFree(ev);
		}
		else
		{
			activated=1;

			stdout_somPrintf("SOMDD - Ready\n");

			RHBSOM_Trace("SOMDD - SOMDD_running=1")

			SOMDD_running=1;

			SOMDD_up(daemon);

			{
	#if defined(_WIN32S) && defined(_DEBUG) && defined(RHBOPT_SHARED_DATA)
				DWORD tid=0;
				HANDLE h=CreateThread(NULL,0,my_test_aslm,NULL,0,&tid);
				if (h)
				{
					WaitForSingleObject(h,INFINITE);
					CloseHandle(h);
				}
	#endif

				SOMDD_syslog(&SOMDD_LOGMSG_READY,NULL);

				status=SOMOA_execute_request_loop(SOMD_SOMOAObject,ev,SOMD_WAIT);

				SOMDD_running=0;

				RHBSOM_Trace("SOMDD - SOMDD_running=0")
			}

			switch (status)
			{
			case SOMDERROR_Interrupt:
				stdout_somPrintf("SOMDERROR_Interrupt\n");
				SOMDD_syslog(&SOMDD_LOGMSG_INTERRUPT,NULL);
				break;

			case SOMDERROR_ServerInterrupt:
				/* means that SOMOA::interrupt_server() called\n */
				stdout_somPrintf("SOMDERROR_ServerInterrupt\n");
				SOMDD_syslog(&SOMDD_LOGMSG_INTERRUPT,NULL);
				break;

			case SOMDERROR_SocketClose:
				stdout_somPrintf("SOMDERROR_SocketClose\n");
				break;

			default:
				stdout_somPrintf("execute_request_loop returned %ld,0x%lx\n",
						(long)status);
				break;
			}

			SOMDD_syslog(&SOMDD_LOGMSG_SHUTDOWN,NULL);

			SOMDD_down(daemon);
		}

		RHBSOM_Trace("SOMDD - ImplRepository_kill_all")

		if (SOMD_ImplRepObject && 
		    (SOMD_ImplRepObject==procMgr->implrep))
		{
			Environment ev2;

			RHBOPT_ASSERT(SOMObject_somIsA(SOMD_ImplRepObject,_ImplRepositoryServer));

			SOM_InitEnvironment(&ev2);

			ImplRepositoryServer_kill_all(SOMD_ImplRepObject,&ev2);
			procMgr->implrep=NULL;
			ImplRepositoryServer__set_process_mgr(SOMD_ImplRepObject,&ev2,NULL);
			SOM_UninitEnvironment(&ev2);
		}

		RHBSOM_Trace("SOMDD - deactivate impl")

		if (activated)
		{
			SOMOA_deactivate_impl(SOMD_SOMOAObject,ev,SOMD_ImplDefObject);	
		}

		RHBSOM_Trace("SOMDD - impl deactivated")
	}

	RHBSOM_Trace("SOMDD - ending")

	if (SOMD_ImplDefObject)
	{
		ImplementationDef SOMSTAR idef=SOMD_ImplDefObject;

		SOMD_ImplDefObject=NULL;

		if (idef)
		{
			somReleaseObjectReference(idef);
		}
	}

	if (SOMD_ImplRepObject)
	{
		SOMClass SOMSTAR cls=somNewClassReference(ImplRepositoryServer);

		if (SOMObject_somIsA(SOMD_ImplRepObject,cls))
		{
			if (ImplRepositoryServer__get_implrep_changed(SOMD_ImplRepObject,ev))
			{
				somStream_StreamIO SOMSTAR stream=NULL;
				char *f=SOMDD_get_repos_filename(argc,argv);

				RHBSOM_Trace("Saving ImplRepository");

#ifdef somNewObject
				stream=somNewObject(somStream_MemoryStreamIO);
#else
				stream=somStream_MemoryStreamIONew();
#endif

				ImplRepositoryServer_save_to_stream(SOMD_ImplRepObject,ev,stream);

				if (!ev->_major)
				{
					_IDL_SEQUENCE_octet seq=somStream_StreamIO_get_buffer(stream,ev);

					if (!ev->_major)
					{
						FILE *fp=fopen(f,"wb");
					
						if (fp)
						{
							octet *op=seq._buffer;
							size_t ul=seq._length;

							while (ul)
							{
								size_t w=fwrite(op,1,ul,fp);

								ul-=w;
								op+=w;

								if (!w)
								{
									RHBSOM_Trace("Error while writing\n");
								}
							}

							fclose(fp);
						}

						SOMFree(seq._buffer);
					}
				}

				if (ev->_major)
				{
					stdout_somPrintf("saving ImplRepository caused an exception, %s\n",
							somExceptionId(ev));
					somExceptionFree(ev);
				}

				somReleaseObjectReference(stream);
			}
		}

		somReleaseClassReference(cls);
	}

	if (SOMD_ImplRepObject)
	{
		ImplRepositoryServer SOMSTAR rep=SOMD_ImplRepObject;
		SOMClass SOMSTAR cls=somNewClassReference(ImplRepositoryServer);
		if (SOMObject_somIsA(rep,cls))
		{
#ifdef USE_THREADS
			SOMDD_MutexRef m=SOMDD_AcquireGlobalMutex();
			ImplRepositoryServer_close(rep,ev,m);
			SOMDD_ReleaseGlobalMutex(m);
#else
			while (!ImplRepositoryServer_handle_requests(rep,ev,0))
			{
			}
#endif
		}
		somReleaseClassReference(cls);
	}

	if (SOMD_SOMOAObject)
	{
#ifdef USE_THREADS
		SOMDD_MutexRef m=SOMDD_AcquireGlobalMutex();
#endif
		SOMObject SOMSTAR ref=SOMD_SOMOAObject;

		SOMD_SOMOAObject=NULL;

#ifdef USE_THREADS
		SOMDD_ReleaseGlobalMutex(m);
#endif
		if (ref)
		{
			somReleaseObjectReference(ref);
		}
	}

#ifdef USE_SELECT
	SOMD_RegisterCallback(NULL,NULL);
#endif

	SOMD_Uninit(ev);

	return status;
}

#if defined(USE_SELECT) && !defined(_PLATFORM_MACINTOSH_) 
static RHBProcessMgr *SOMDD_processMgr;
static void SOMDD_signal_event_handler(int i)
{
	if (SOMDD_processMgr)
	{
		RHBProcessMgrChild *child=SOMDD_processMgr->childInfo;

		if (child->emanObject)
		{
			Environment ev={NO_EXCEPTION,{NULL,NULL},NULL};
			struct emanEvent *event=NULL;

			switch (i)
			{
			case SIGTERM:
				event=&child->termEvent;
				break;
#ifdef SIGQUIT
			case SIGQUIT:
				event=&child->quitEvent;
				break;
#endif
#ifdef SIGHUP
			case SIGHUP:
#endif
			case SIGINT:
				event=&child->hupEvent;
				break;
			}

			if (event)
			{
				if (event->event && !event->queued)
				{
					event->queued=1;
					SOMEEMan_someQueueEvent(
						child->emanObject,
						&ev,
						event->event);
				}
			}

			somExceptionFree(&ev);
		}
	}
}
static void SOMLINK SOMDD_Quit_Callback(SOMEEvent SOMSTAR obj, void *pv)
{
	Environment ev={NO_EXCEPTION,{NULL,NULL},NULL};
	struct emanEvent *data=SOMEClientEvent_somevGetEventClientData(obj,&ev);
	data->queued=0;
/*	SOMObject_somDumpSelf(obj,0);*/
	SOMDD_signal_handler(data->sig);
	SOM_UninitEnvironment(&ev);
}

static void SOMDD_install_signal_event(RHBProcessMgr *mgr,
									   Environment *ev,
									   int sig,
									   char *name,
									   struct emanEvent *e,
									   EMRegProc *proc)
{
	RHBProcessMgrChild *list=mgr->childInfo;
#ifndef DONT_USE_SIGNALS
	static struct sigaction somdd_reaper;
	sigset_t sigs;
#endif

	e->regData=
#ifdef somNewObject
		somNewObject(SOMEEMRegisterData);
#else
		SOMEEMRegisterDataNew();
#endif

	e->event=
#ifdef somNewObject
		somNewObject(SOMEClientEvent);
#else
		SOMEClientEventNew();
#endif

	SOMEClientEvent_somevSetEventClientType(e->event,ev,name);
	SOMEClientEvent_somevSetEventClientData(e->event,ev,e);

	e->sig=sig;

	SOMEEMRegisterData_someClearRegData(e->regData,ev);
	somva_SOMEEMRegisterData_someSetRegDataEventMask(e->regData,ev,EMClientEvent,NULL);
	SOMEEMRegisterData_someSetRegDataClientType(e->regData,ev,name);
	e->token=SOMEEMan_someRegisterProc(list->emanObject,ev,e->regData,proc,list);

#ifndef DONT_USE_SIGNALS
	sigemptyset(&sigs);
	somdd_reaper.sa_handler=SOMDD_signal_event_handler;
#ifdef SA_RESTART
	somdd_reaper.sa_flags=SA_RESTART;
#endif
#ifdef SA_NOCLDSTOP
#ifdef SIGCHLD
	if (sig==SIGCHLD)
#else
	if (sig==SIGCLD)
#endif
	somdd_reaper.sa_flags|=SA_NOCLDSTOP;
#endif
	sigaction(sig,&somdd_reaper,NULL);
	sigaddset(&sigs,sig);
	sigprocmask(SIG_UNBLOCK,&sigs,NULL);
#endif
}

static void SOMDD_remove_signal_event(RHBProcessMgr *mgr,Environment *ev,struct emanEvent *e)
{
	RHBProcessMgrChild *list=mgr->childInfo;
#ifndef DONT_USE_SIGNALS
	if (e->sig)
	{
		sigset_t sigs;
		sigemptyset(&sigs);
		sigaddset(&sigs,e->sig);
		sigprocmask(SIG_BLOCK,&sigs,NULL);
	}
#endif

	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);

	RHBOPT_ASSERT(!(e->queued));

	if (e->token)
	{
		SOMEEMan_someUnRegister(list->emanObject,ev,e->token);
		e->token=0;
	}

	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);

	if (e->event)
	{
		SOMEClientEvent_somRelease(e->event);
		e->event=NULL;
	}

	if (e->regData)
	{
		SOMEEMRegisterData_somRelease(e->regData);
		e->regData=NULL;
	}
}

static void SOMDD_install_signals(RHBProcessMgr *mgr,SOMEEMan SOMSTAR eman)
{
	RHBProcessMgrChild *list=mgr->childInfo;
	Environment ev;

	RHBOPT_ASSERT(!SOMDD_processMgr);

	SOMDD_processMgr=mgr;

	SOM_InitEnvironment(&ev);

	list->emanObject=eman;

#if defined(SIGQUIT) && !defined(_WIN32)
	SOMDD_install_signal_event(mgr,&ev,SIGQUIT,"SIGQUIT",&list->quitEvent,SOMDD_Quit_Callback);
#endif
	SOMDD_install_signal_event(mgr,&ev,SIGTERM,"SIGTERM",&list->termEvent,SOMDD_Quit_Callback);
#if defined(SIGHUP) && !defined(_WIN32)
	SOMDD_install_signal_event(mgr,&ev,SIGHUP,"SIGHUP",&list->hupEvent,SOMDD_Quit_Callback);
#else
	SOMDD_install_signal_event(mgr,&ev,SIGINT,"SIGINT",&list->hupEvent,SOMDD_Quit_Callback);
#endif

	SOM_UninitEnvironment(&ev);
}

static void SOMDD_remove_signals(RHBProcessMgr *mgr,SOMEEMan SOMSTAR eman)
{
	RHBProcessMgrChild *list=mgr->childInfo;
	Environment ev;

	RHBOPT_ASSERT(SOMDD_processMgr==mgr);

	SOM_InitEnvironment(&ev);

	SOMDD_remove_signal_event(mgr,&ev,&list->quitEvent);
	SOMDD_remove_signal_event(mgr,&ev,&list->hupEvent);
	SOMDD_remove_signal_event(mgr,&ev,&list->termEvent);

	SOM_UninitEnvironment(&ev);

	list->emanObject=NULL;

	SOMDD_processMgr=NULL;
}
#endif

#ifdef USE_THREADS
struct SOMDD_run_r_t
{
	int argc;
	char **argv;
	int complete;
	ORBStatus status;
#ifdef DONT_USE_SIGNALS
	#ifdef USE_PTHREADS
		pthread_cond_t pEvent;
	#else
		HANDLE hEvent;
	#endif
#else
	#ifdef USE_PTHREADS
		int pid_main;
		pthread_t pthr_main;
	#else
		#error
	#endif
#endif

#ifdef USE_PTHREADS
	pthread_t pthr_child;
	void *child_exitCode;
#else
	DWORD dw_child;
	HANDLE h_child;
	DWORD child_exitCode;
#endif
	Environment *ev;
	struct SOMDD_daemon *daemon;
	RHBProcessMgr *pmgr;
};

#if defined(USE_THREADS) && !defined(DONT_USE_SIGNALS) 
static void SOMDD_reaper_handler(int i)
{
/*	const char *p="<SOMDD.SIG>, bubbling up\n";
	write(1,p,strlen(p));*/
	if (SOMDD_active)
	{
#ifdef USE_PTHREADS
		pthread_kill(SOMDD_active->pthr_main,i);
#else
		if (!SetEvent(SOMDD_active->hEvent)) __asm int 3
#endif
	}
}
#endif

RHBOPT_cleanup_begin(SOMDD_run_r_thr_cleanup,pv)

	struct SOMDD_run_r_t *data=pv;
	SOMDD_MutexRef m=SOMDD_AcquireGlobalMutex();
	int sig=SIGINT;

	data->complete=1;

#ifdef DONT_USE_SIGNALS
	#ifdef USE_PTHREADS
		pthread_cond_signal(&data->pEvent);
	#else
		if (!SetEvent(data->hEvent))
		{
#ifdef _M_IX86
			__asm int 3;
#endif
		}
	#endif
#else
	#ifdef USE_PTHREADS
		pthread_kill(data->pthr_main,sig);
		kill(data->pid_main,sig);
	#else
		if (!SetEvent(data->hEvent))
		{
#ifdef _M_IX86
			__asm int 3;
#endif
		}
	#endif
#endif

	SOMDD_ReleaseGlobalMutex(m);

RHBOPT_cleanup_end

#if defined(USE_PTHREADS) 
static void *SOMDD_run_r_thr(void *pv)
#else
static DWORD CALLBACK SOMDD_run_r_thr(void *pv)
#endif
{
	struct SOMDD_run_r_t *data=pv;
	ORBStatus status=data->status;

	RHBOPT_cleanup_push(SOMDD_run_r_thr_cleanup,pv);

#if !defined(USE_PTHREADS) 
	if (!SetEvent(data->hEvent)) 
	{
#ifdef _M_IX86
		__asm int 3;
#endif
	}
#endif

	status=SOMDD_run(data->daemon,data->pmgr,data->ev,data->argc,data->argv);

/*	somPrintf("SOMDD_run() returned with %d\n",status);*/

	if (!data->status)
	{
		data->status=status;
	}

	RHBOPT_cleanup_pop();

#if defined(USE_PTHREADS) 
	return data;
#else
	return GetCurrentThreadId();
#endif
}

#ifndef DONT_USE_SIGNALS
static void SOMDD_run_r_signal(int i)
{
	if (SOMDD_active)
	{
#ifdef USE_PTHREADS
		pthread_kill(SOMDD_active->pthr_main,i);
#else
		if (!SetEvent(SOMDD_active->h_main)) 
		{
#ifdef _M_IX86
			__asm int 3;
#endif
		}
#endif
	}
}
#endif

#if (defined(USE_SELECT)||defined(USE_PTHREADS)) && !defined(DONT_USE_SIGNALS)
static void cleanup_signals(sigset_t *set)
{
	sigset_t s;

	while (!sigpending(&s))
	{
		int i=sizeof(s)<<3;
		int doWait=0;

		while (i-- && !doWait)
		{
			if (!i) break;

			switch (i)
			{
			case SIGTERM:
			case SIGINT:
#ifdef SIGBREAK
			case SIGBREAK:
#endif
#ifdef SIGQUIT
			case SIGQUIT:
#endif
#ifdef SIGCHLD
			case SIGCHLD:
#else
	#ifdef SIGCLD
			case SIGCLD:
	#endif
#endif
#ifdef SIGPIPE
			case SIGPIPE:
#endif
#ifdef SIGIO
			case SIGIO:
#endif
				if (sigismember(&s,i)&&
					sigismember(set,i))
				{
					doWait=1;
					break;
				}
				break;
			}
		}

		if (doWait)
		{
			int i=0;

			sigwait(set,&i);

/*			if (i) somPrintf("cleaned up signal(%d)\n",i);*/
		}
		else
		{
			break;
		}
	}
}
#endif

static ORBStatus SOMDD_run_r(
		struct SOMDD_daemon *daemon,
		RHBProcessMgr *executor,
		Environment *ev,int argc,char **argv)
{
	struct SOMDD_run_r_t data;
	SOMDD_MutexRef m=SOMDD_AcquireGlobalMutex();
#if (defined(USE_PTHREADS) ) && !defined(DONT_USE_SIGNALS)
	sigset_t set;
#endif

#ifndef DONT_USE_SIGNALS
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
#ifdef SA_RESTART
	sa.sa_flags=SA_RESTART;
#endif
	sa.sa_handler=SOMDD_run_r_signal;
	sigaction(SIGTERM,&sa,NULL);
	sigaction(SIGINT,&sa,NULL);
#ifdef SIGHUP
	sigaction(SIGHUP,&sa,NULL);
#endif
#ifdef SIGBREAK
	sigaction(SIGBREAK,&sa,NULL);
#endif
#ifdef SIGQUIT
	sigaction(SIGQUIT,&sa,NULL);
#endif
#endif

#if (defined(USE_PTHREADS) ) && !defined(DONT_USE_SIGNALS)
	sigemptyset(&set);
	sigaddset(&set,SIGTERM);
	sigaddset(&set,SIGINT);
#ifdef SIGHUP
	sigaddset(&set,SIGHUP);
#endif
#if defined(SIGBREAK) && !defined(_WIN32)
	sigaddset(&set,SIGBREAK);
#endif
#if defined(SIGQUIT) && !defined(_WIN32)
	sigaddset(&set,SIGQUIT);
#endif /* SIGQUIT */
#ifdef SIGCHLD
	sigaddset(&set,SIGCHLD);
#else /* SIGCHLD */
#ifdef SIGCLD
	sigaddset(&set,SIGCLD);
#endif /* SIGCLD */
#endif /* !SIGCHLD */
#endif /* USE_PTHREADS */

	data.argc=argc;
	data.argv=argv;
	data.complete=0;
	data.status=0;
	data.ev=ev;
	data.daemon=daemon;
	data.pmgr=executor;

#ifdef USE_PTHREADS
	data.child_exitCode=NULL;
	#ifdef DONT_USE_SIGNALS
		pthread_cond_init(&data.pEvent,RHBOPT_pthread_condattr_default);
	#else
		data.pid_main=getpid();
		data.pthr_main=pthread_self();
	#endif
	if (pthread_create(&data.pthr_child,RHBOPT_pthread_attr_default,SOMDD_run_r_thr,&data))
	{
		SOMDD_ReleaseGlobalMutex(m);
		RHBOPT_throw_StExcep(ev,NO_RESOURCES,CouldNotStartProcess,NO);
		return SOMDERROR_CouldNotStartProcess;
	}
#else
	data.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	data.child_exitCode=(DWORD)~0;
	data.h_child=CreateThread(NULL,0,SOMDD_run_r_thr,&data,0,&data.dw_child);
	if (!data.h_child)
	{
		SOMDD_ReleaseGlobalMutex(m);
		RHBOPT_throw_StExcep(ev,NO_RESOURCES,CouldNotStartProcess,NO);
		return SOMDERROR_CouldNotStartProcess;
	}
#endif

	SOMDD_active=&data;

	while (!data.complete)
	{
		boolean doQuit=0;
#ifdef DONT_USE_SIGNALS
	#ifdef USE_PTHREADS
		pthread_cond_wait(&data.pEvent,m);
	#else
		SOMDD_ReleaseGlobalMutex(m);

		WaitForSingleObjectEx(data.hEvent,INFINITE,TRUE);

		m=SOMDD_AcquireGlobalMutex();
	#endif
#else
	#if defined(USE_PTHREADS) 
		int i=0,j=0,e=0;
		SOMDD_ReleaseGlobalMutex(m);

		j=sigwait(&set,&i);

		e=errno;

/*		somPrintf("SOMDD sigwait returned signal=%d,rc=%d,e=%d\n",
				i,j,e);*/

		RHBOPT_unused(j);
		RHBOPT_unused(e);

		switch (i)
		{
#ifdef SIGHUP
		case SIGHUP:
			stdout_somPrintf("SIGHUP\n");
			/* Darwin 7.0.1 preceeds every signal with a SIGHUP */
			{
				sigset_t s;
				sigemptyset(&s);
				if (!sigpending(&s))
				{
					int nPending=0;
#ifdef NSIG
					int n=NSIG;
#else
					int n=sizeof(s)<<3;
#endif
					while (n--)
					{
						if (n)
						{
							if (sigismember(&s,n) && sigismember(&set,n))
							{
								nPending++;

								switch (n)
								{
#ifdef SIGCHLD
								case SIGCHLD:
									RHBSOM_Trace("SIGCHLD pending\n");
									break;
#else
	#ifdef SIGCLD
								case SIGCLD:
									RHBSOM_Trace("SIGCLD pending\n");
									break;
	#endif
#endif
#ifdef SIGPIPE
								case SIGPIPE:
									RHBSOM_Trace("SIGPIPE pending\n");
									break;
#endif
#ifdef SIGQUIT
								case SIGQUIT:
									RHBSOM_Trace("SIGQUIT pending\n");
									break;
#endif
#ifdef SIGINT
								case SIGINT:
									RHBSOM_Trace("SIGINT pending\n");
									break;
#endif
#ifdef SIGBREAK
								case SIGBREAK:
									RHBSOM_Trace("SIGBREAK pending\n");
									break;
#endif
								case SIGTERM:
									RHBSOM_Trace("SIGTERM pending\n");
									break;
								default:
									stdout_somPrintf("signal(%d) pending\n",n);
									break;
								}
							}
						}
					}

/*					somPrintf("%d signals still pending...\n",nPending);*/

					if (!nPending)
					{
						data.status=SOMDERROR_SocketClose;
						doQuit=1;
					}
				}
			}
			break;
#endif
#ifdef SIGBREAK
		case SIGBREAK:
			stdout_somPrintf("SIGBREAK\n");
			break;
#endif
#ifdef SIGQUIT
		case SIGQUIT:
			stdout_somPrintf("SIGQUIT\n");
			doQuit=1;
			break;
#endif
		case SIGTERM:
			stdout_somPrintf("SIGTERM\n");
			doQuit=1;
			break;
		case SIGINT:
			stdout_somPrintf("SIGINT\n");
#if defined(_WIN32) && defined(_DEBUG) && defined(_CONSOLE)
			doQuit=1;
#endif
			break;
#ifdef SIGCHLD
		case SIGCHLD:
			stdout_somPrintf("SIGCHLD\n");
#else
	#ifdef SIGCLD
		case SIGCLD:
			stdout_somPrintf("SIGCLD\n");
	#endif
#endif
			break;
		default:
			stdout_somPrintf("unexpected signal(%d)\n",i);
			break;
		}

		m=SOMDD_AcquireGlobalMutex();
	#else
		#error dont know how to deal with this case
	#endif
#endif /* DONT_USE_SIGNALS */

		if (doQuit && !data.complete)
		{
			SOMOA SOMSTAR somoa=NULL;

			RHBSOM_Trace("server loop interrupting SOMOA\n");

			if (SOMD_SOMOAObject)
			{
				somoa=SOMOA_somDuplicateReference(SOMD_SOMOAObject);
			}

			if (somoa)
			{
				Environment ev2;
				SOM_InitEnvironment(&ev2);
				RHBSOM_Trace("interrupting somoa\n");
				SOMOA_interrupt_server(somoa,&ev2);
				SOMOA_somRelease(somoa);
				SOM_UninitEnvironment(&ev2);
			}
			else
			{
				RHBSOM_Trace("SOMOA has disappeared\n");
				break;
			}
		}
	}

	if (SOMDD_active==&data)
	{
		SOMDD_active=NULL;
	}

	SOMDD_ReleaseGlobalMutex(m);

#if (defined(USE_PTHREADS)) && !defined(DONT_USE_SIGNALS)
	cleanup_signals(&set);
#endif

	RHBSOM_Trace("joining SOMOA thread..\n");

#ifdef USE_PTHREADS
	pthread_join(data.pthr_child,&data.child_exitCode);

	#ifdef DONT_USE_SIGNALS
		pthread_cond_destroy(&data.pEvent);
	#endif
#else
	if (WAIT_OBJECT_0!=WaitForSingleObject(data.h_child,INFINITE))
	{
#ifdef _M_IX86
		__asm int 3;
#endif
	}
	if (!GetExitCodeThread(data.h_child,&data.child_exitCode))
	{
#ifdef _M_IX86
		__asm int 3;
#endif
	}
	if (!CloseHandle(data.h_child)) 
	{
#ifdef _M_IX86
		__asm int 3;
#endif
	}
	if (!CloseHandle(data.hEvent)) 
	{
#ifdef _M_IX86
		__asm int 3;
#endif
	}
#endif

	RHBSOM_Trace("SOMOA thread joined.\n");

#if (defined(USE_PTHREADS)) && !defined(DONT_USE_SIGNALS)
	cleanup_signals(&set);
#endif

	return data.status;
}
#endif

#ifdef _PLATFORM_MACINTOSH_
RHBProcess *RHBProcessNew(ProcessSerialNumber *psn)
{
	RHBProcess *p=RHBSOMDD_global_process_list.first;

	while (p)
	{
		Boolean b=0;

		if (!SameProcess(&p->psn,psn,&b))
		{
			if (b)
			{
				p->usage++;

				return p;
			}
		}

		p=p->next;
	}

	p=SOMMalloc(sizeof(*p));

	if (p)
	{
		/* always create with '2' */

		p->usage=2;
		p->psn=*psn;
		p->next=RHBSOMDD_global_process_list.first;
		RHBSOMDD_global_process_list.first=p;
		p->list=&RHBSOMDD_global_process_list;
	}

	return p;
}
RHBProcess *RHBProcessFind(ProcessSerialNumber *psn)
{
	RHBProcess *p=RHBSOMDD_global_process_list.first;

	while (p)
	{
		Boolean b=0;

		if (!SameProcess(&p->psn,psn,&b))
		{
			if (b)
			{
				return p;
			}
		}

		p=p->next;
	}

	return NULL;
}
#endif

#if defined(USE_THREADS) && !defined(DONT_USE_SIGNALS)
#if defined(USE_PTHREADS)
static void SOMDD_signal_thread(int sig)
{
	somPrintf("## signal %d, exiting thread ##\n",sig);
#ifdef USE_PTHREADS
	pthread_exit(RHBOPT_PTHREAD_CANCELED);
#else
	pth_exit(PTH_CANCELED);
#endif
}
#endif
#endif

#if defined(_WIN32) && defined(DONT_USE_SIGNALS)
static BOOL CALLBACK somdd_console_ctrl(DWORD dw)
{
	somPrintf("somdd_console_ctrl(%lx)\n",dw);

	switch (dw)
	{
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
		if (&(SOMD_SOMOAObject))
		{
			if (&SOMD_SOMOAObject)
			{
				Environment ev;
				SOM_InitEnvironment(&ev);
				SOMOA_interrupt_server(SOMD_SOMOAObject,&ev);
				SOM_UninitEnvironment(&ev);
			}
		}
		return 1;
	case CTRL_LOGOFF_EVENT:
		return 1;
	}

	return 0;
}
#endif

int SOMDD_main(struct SOMDD_daemon *daemon,
			   RHBProcessMgr *executor,
			   int argc,char **argv)
{
	Environment ev;
	ORBStatus status=0;
#if defined(USE_THREADS) && !defined(_PLATFORM_MACINTOSH_)
	#if	!defined(DONT_USE_SIGNALS) && !(defined(USE_THREADS)&&!defined(USE_PTHREADS))
		static struct sigaction somdd_reaper;
		sigset_t sigs;
	#endif
#endif /* USE_THREADS||_PLATFORM_MACINTOSH_ */

#ifdef _WIN32
	#ifdef DONT_USE_SIGNALS
		SetConsoleCtrlHandler(somdd_console_ctrl,1);
	#else
		/* disable these signals until we are in the SOMOA loop */
		/* the UNIX main() will already have blocked these two */
		signal(SIGINT,SIG_IGN);
		signal(SIGTERM,SIG_IGN);
	#endif
#endif /* _WIN32 */
 
#if defined(USE_THREADS) && !defined(DONT_USE_SIGNALS)
	#if defined(USE_PTHREADS)
	{
static struct sigaction sa;
#ifdef SA_RESTART
		sa.sa_flags=SA_RESTART;
#endif
		sa.sa_handler=SOMDD_signal_thread;
#ifdef SIGBUS
		sigaction(SIGBUS,&sa,NULL);
#endif
		sigaction(SIGSEGV,&sa,NULL);
		sigaction(SIGILL,&sa,NULL);
		sigaction(SIGFPE,&sa,NULL);
		sigaction(SIGABRT,&sa,NULL);
	}
	#endif
#endif

	RHBOPT_unused(daemon)

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		#ifdef RHBOPT_PTHREAD_MUTEX_INTIALIZER
		#else
			pthread_mutex_init(&SOMDD_serialize.mutex,RHBOPT_pthread_mutexattr_default);
		#endif
	#else
			SOMDD_serialize.mutex=CreateMutex(NULL,0,NULL);
	#endif
#endif

	SOMDD_Server_classObject=NULL;

#ifdef _WIN32
#else
	#ifdef _PLATFORM_MACINTOSH_
	#else
		openlog("somdd",
			/* opt */
		#ifdef LOG_PERROR
				LOG_PERROR|
		#endif
				LOG_PID,
		/* facility */ LOG_USER);
	#endif
#endif

#ifdef USE_APPLE_SOM
	somEnvironmentNew();
#else
	somMainProgram();
#endif

#ifdef _WIN32
	#ifndef _CONSOLE
		somSetOutChar(SOMDD_OutChar);
	#endif
#endif

	SOM_InitEnvironment(&ev);

#ifdef USE_THREADS

	#if (defined(USE_PTHREADS) ) && !defined(DONT_USE_SIGNALS)
			/* install this before doing any forking
				to say we want this signal delivered */
			sigemptyset(&sigs);
			somdd_reaper.sa_handler=SOMDD_reaper_handler;
		#ifdef SA_RESTART
			somdd_reaper.sa_flags=SA_RESTART;
		#endif
		#ifdef SA_NOCLDSTOP
			somdd_reaper.sa_flags|=SA_NOCLDSTOP;
		#endif
		#ifdef SIGCLD
			sigaction(SIGCLD,&somdd_reaper,NULL);
			sigaddset(&sigs,SIGCLD);
		#else
			sigaction(SIGCHLD,&somdd_reaper,NULL);
			sigaddset(&sigs,SIGCHLD);
		#endif
	#endif
#else
	#ifdef somNewObject
		SOMDD_SOMEEManObject=somNewObject(SOMDDEventMgr);
	#else
		SOMDD_SOMEEManObject=SOMDDEventMgrNew();
	#endif
	SOMDD_install_signals(executor,SOMDD_SOMEEManObject);
#endif

	SOMDD_Server_classObject=somNewClassReference(SOMDD_Server);

	if (!RHBProcessMgr_begin(executor,&ev))
	{
		do
		{
#ifdef USE_THREADS
			status=SOMDD_run_r(daemon,executor,&ev,argc,argv);
#else
			status=SOMDD_run(daemon,executor,&ev,argc,argv);
#endif
		} while (status==SOMDERROR_SocketClose);
	
		RHBProcessMgr_end(executor);
	}

#ifdef USE_THREADS
#else
	SOMDD_remove_signals(executor,SOMDD_SOMEEManObject);
	somReleaseObjectReference(SOMDD_SOMEEManObject);
	SOMDD_SOMEEManObject=NULL;
#endif

	if (SOMDD_Server_classObject)
	{
		somReleaseClassReference(SOMDD_Server_classObject);
		SOMDD_Server_classObject=NULL;
	}

	SOM_UninitEnvironment(&ev);

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		#ifdef RHBOPT_PTHREAD_MUTEX_INITIALIZER
		#else
			pthread_mutex_destroy(&SOMDD_serialize.mutex);
		#endif
	#else
		if (SOMDD_serialize.mutex)
		{
			CloseHandle(SOMDD_serialize.mutex);
			SOMDD_serialize.mutex=NULL;
		}
	#endif
#endif

	RHBSOM_Trace("SOMDD somEnvironmentEnd")

	{
		SOMClassMgr SOMSTAR mgr=somGetClassManagerReference();

		SOMClassMgr__set_somInterfaceRepository(mgr,NULL);

#ifndef USE_APPLE_SOM
		drop_class(mgr,SOMDD_ServerMgrClassData.classObject);
		drop_class(mgr,SOMDD_ServerClassData.classObject);
		drop_class(mgr,ImplRepositoryServerClassData.classObject);
#ifdef _SOMDDREP_DynamicRepository
		drop_class(mgr,SOMDDREP_DynamicRepository.classObject);
#endif
#ifndef USE_THREADS
		drop_class(mgr,SOMDDEventMgrClassData.classObject);
#endif
#endif

		somReleaseClassManagerReference(mgr);
	}

	stdout_somPrintf("SOMDD - Ended\n");

	somEnvironmentEnd();

	return 0;
}


#if defined(USE_PTHREADS) || !defined(USE_THREADS)

#ifdef LOG_INFO
static struct 
{
	int severity;
	int type;
} sevMap[]={
	{SOMDD_LOG_OK,LOG_INFO},
	{SOMDD_LOG_INFO,LOG_INFO},
	{SOMDD_LOG_WARN,LOG_WARNING},
	{SOMDD_LOG_ERROR,LOG_ERR}
};
#endif

void SOMDD_syslog(struct SOMDD_msg *msg,...)
{
		va_list ap;
		char *args[32];
		char **h=args;
		int n=0;
		int i=0;
		int type=0;

		va_start(ap,msg);

#ifdef LOG_DEBUG
		i=sizeof(sevMap)/sizeof(sevMap[0]);
		while (i--)
		{
			if (sevMap[i].severity==msg->type)
			{
				type=sevMap[i].type;
				break;
			}
		}
#endif

		while (n < (sizeof(args)/sizeof(args[0])))
		{
			char *p=va_arg(ap,char *);
			*h++=p;
			if (!p) break;
			n++;
		}

		if (msg->text)
		{
			char buf[1024];
			char *p=buf;
			const char *t=msg->text;

			while (*t && (p < (buf+sizeof(buf)-1)))
			{
				char c=*t++;
				switch (c)
				{
				case '%':
					if (*t)
					{
						c=*t++;
						if ((c>'0')&&(c<='9'))
						{
							int k=c-'0'-1;
							if (k < n)
							{
								int l=((buf+sizeof(buf))-p)-1;
								const char *q=args[k];
								int u=strlen(q);
								if (u > l) u=l;
								memcpy(p,q,u);
								p+=u;
							}
						}
						else
						{
							*p++=c;
						}
					}
					break;
				default:
					*p++=c;
					break;
				}
			}

			if (p < (buf+sizeof(buf)))
			{
				*p=0;

#ifdef _WIN32
				stdout_somPrintf("somdd(%d): %s\n",
					(int)GetCurrentProcessId(),
					buf);
#else
	#ifdef _PLATFORM_MACINTOSH_
				somPrintf("somdd: %s\n",buf);
	#else
				syslog(type,"%s",buf);
	#endif
#endif
			}
		}
		else
		{
			RHBOPT_ASSERT(0)
		}

		va_end(ap);
}
#endif

#ifdef USE_THREADS
#ifdef SOMDD_AcquireGlobalMutex
SOMDD_MutexRef SOMDD_AcquireGlobalMutexDebug(const char *file,int line)
#else
SOMDD_MutexRef SOMDD_AcquireGlobalMutex(void)
#endif
{
#ifdef USE_PTHREADS
	pthread_mutex_lock(&SOMDD_serialize.mutex);
#ifdef _DEBUG
	somdd_mutex_owner=pthread_self();
#endif
	return &SOMDD_serialize.mutex;
#else
	RHBOPT_ASSERT(SOMDD_serialize.mutex);
	WaitForSingleObject(SOMDD_serialize.mutex,INFINITE);
#ifdef _DEBUG
	somdd_mutex_owner=GetCurrentThreadId();
#endif
	return SOMDD_serialize.mutex;
#endif
}
#endif

#ifdef USE_THREADS
void SOMDD_ReleaseGlobalMutex(SOMDD_MutexRef m)
{
#ifdef _DEBUG
	somdd_mutex_owner=zero_tid;
#endif
#ifdef USE_PTHREADS
	pthread_mutex_unlock(m);
#else
	if (!ReleaseMutex(m)) 
	{
#ifdef _M_IX86
		__asm int 3;
#endif
	}
#endif
}
#endif





