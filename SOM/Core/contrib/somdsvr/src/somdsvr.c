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
	#include <windows.h>
#endif

#include <rhbsomex.h>

#ifdef __cplusplus
	#include <som.xh>
	#include <somir.xh>
	#include <somd.xh>
	#include <impldef.xh>
	#include <implrep.xh>
	#include <emtypes.xh>
	#include <emregdat.xh>
	#include <eman.xh>
#else
	#include <som.h>
	#include <somir.h>
	#include <somd.h>
	#include <impldef.h>
	#include <implrep.h>
	#include <emtypes.h>
	#include <emregdat.h>
	#include <eman.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <rhbsomex.h>

#ifdef _DEBUG
	#include <sys/types.h>
	#include <sys/stat.h>
#endif

#ifdef _WIN32
	#include <process.h>
#endif

#if defined(_PLATFORM_UNIX_) && defined(__AUX__)
	#ifdef __cplusplus
		extern "C" int getpid(void);
	#else
		extern int getpid();
	#endif
#else
	#ifdef HAVE_UNISTD_H
		#include <unistd.h>
	#endif
#endif

#ifdef __cplusplus
/* mappings from C mappings */
#define ImplRepository_find_impldef_by_alias(a,b,c)				a->find_impldef_by_alias(b,c)
#define ImplRepository_find_impldef(a,b,c)						a->find_impldef(b,c)
#define SOMObject_somFree(a)									a->somFree()
#define SOMOA_interrupt_server(a,b)								a->interrupt_server(b)
#define SOMOA_deactivate_impl(a,b,c)							a->deactivate_impl(b,c)
#define SOMOA_impl_is_ready(a,b,c)								a->impl_is_ready(b,c)
#define ImplementationDefNew()									new ImplementationDef
#define SOMOANew()												new SOMOA
#define SOMObject_somPrintSelf(a)								a->somPrintSelf()
#define ImplementationDef__get_impl_hostname(a,b)				a->_get_impl_hostname(b)
#define ImplementationDef__get_impl_id(a,b)						a->_get_impl_id(b)
#define SOMOA_execute_request_loop(a,b,c)						a->execute_request_loop(b,c)
#define SOMObject_somDumpSelf(a,b)								a->somDumpSelf(b)
#define SOMEEManNew()											new SOMEEMan
#define SOMOA_execute_next_request(a,b,c)						a->execute_next_request(b,c)
#define SOMOA_somDuplicateReference(a)							a->somDuplicateReference()
#define SOMOA_somRelease(a)										a->somRelease()
#define SOMEEMRegisterDataNew()									new SOMEEMRegisterData
#define SOMEEMan_someUnRegister(a,b,c)							a->someUnRegister(b,c)
#define SOMEEMRegisterData_someClearRegData(a,b)				a->someClearRegData(b)
#define SOMEEMRegisterData_someSetRegDataEventMask(a,b,c,d)		a->someSetRegDataEventMask(b,c,d)
#define SOMEEMRegisterData_someSetRegDataSink(a,b,c)			a->someSetRegDataSink(b,c)
#define SOMEEMRegisterData_someSetRegDataSinkMask(a,b,c)		a->someSetRegDataSinkMask(b,c)
#define SOMEEMRegisterData_someSetRegDataClientType(a,b,c)		a->someSetRegDataClientType(b,c)
#define SOMEEMan_someRegisterProc(a,b,c,d,e)					a->someRegisterProc(b,c,d,e)
#define SOMEEMan_someQueueEvent(a,b,c)							a->someQueueEvent(b,c)
#define SOMEClientEventNew()									new SOMEClientEvent
#define SOMEClientEvent_somevSetEventClientType(a,b,c)			a->somevSetEventClientType(b,c)
#define SOMEClientEvent_somevSetEventClientData(a,b,c)			a->somevSetEventClientData(b,c)
#endif

#ifdef _WIN32
typedef void (* signal_handler_t)(int);
#else
#if defined(__cplusplus)
extern "C"
{
#endif
typedef void (* signal_handler_t)(int);

#ifdef USE_THREADS
static
	#if defined(USE_PTHREADS)
		void *
	#else
		DWORD CALLBACK 
	#endif
		run_impl_r(void *pv);

	#ifdef USE_PTHREADS
		#ifdef HAVE_PTHREAD_ATFORK
			static void somdsvr_prepare(void);
			static void somdsvr_parent(void);
			static void somdsvr_child(void);
		#endif
	#endif

	#if  defined(USE_PTHREADS)
	static void signal_thread(int sig);
	#endif
#endif

#if defined(__cplusplus)
}
#endif
#endif

struct run_impl_t
{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_t thr_main,thr_child;
		int pid_main;
	#else
		DWORD dw_main,dw_child;
		HANDLE h_child;
	#endif
	Environment *ev;
	int completed;
#else
	#ifdef USE_POSIX_SIGNALS
		sigset_t pending;
	#else
		#error catch for moment
	#endif
	char *eventName;
	SOMEClientEvent SOMSTAR clientEvent;
	SOMEEMRegisterData SOMSTAR regData;
	long token;
	SOMEEMan SOMSTAR emanObject;
	Environment ev;
#endif
};

static struct run_impl_t *g_run_impl;
static boolean somdsvr_quit_flag;
static boolean check_for_quit(void);

#if defined(_WIN32) && defined(USE_THREADS)
static DWORD dwProcess;
#endif

static int main_pid;

#ifndef RHBSOM_Trace
#define  RHBSOM_Trace(x)     { char *p; p=x; somPrintf("%s:%d,%s\n",__FILE__,__LINE__,p); }
#endif

#ifdef __cplusplus
	extern "C" {
#endif
#ifdef HAVE_ATEXIT
static 
	#ifdef HAVE_ATEXIT_VOID
		void 
	#endif
	atexit_callback(void)
{
	if (getpid()==main_pid)
	{
		if (SOMD_SOMOAObject)
		{
			if (SOMD_ImplDefObject)
			{
				Environment ev;
				SOM_InitEnvironment(&ev);
				SOMOA_deactivate_impl(SOMD_SOMOAObject,&ev,SOMD_ImplDefObject);
				SOM_UninitEnvironment(&ev);
			}
		}
	}
}
#endif

static void my_signal_handler(int i)
{
#ifdef _DEBUG
	{
		char buf[256];
#	ifdef HAVE_SNPRINTF
		int k=snprintf(buf,sizeof(buf),"%ld:%s:my_signal_handler(%d)\n",
					(long)getpid(),__FILE__,i);
#	else
		int k=sprintf(buf,"%ld:%s,my_signal_handler(%d)\n",
					(long)getpid(),__FILE__,i);
#	endif
#	ifdef _WIN32
		DWORD dw=0;
		WriteFile(GetStdHandle(STD_ERROR_HANDLE),buf,k,&dw,NULL);
#	else
		write(2,buf,k);
#	endif
	}
#endif

	if (g_run_impl)
	{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_kill(g_run_impl->thr_main,i);
	#else
		BOOL b=PostThreadMessage(g_run_impl->dw_main,WM_QUIT,0,0);
		RHBOPT_ASSERT(b)
	#endif
#else
	#ifdef USE_POSIX_SIGNALS
		sigaddset(&g_run_impl->pending,i);
	#else
		#error catch for moment
	#endif

		SOMEEMan_someQueueEvent(g_run_impl->emanObject,&g_run_impl->ev,g_run_impl->clientEvent);
#endif
	}
}
#ifdef __cplusplus
	}
#endif

#ifndef USE_THREADS
static void SOMLINK somdsvr_callback(void *,void *);
#endif

#ifdef USE_SIGNALS
	#if defined(HAVE_SA_HANDLER_VOID_INT) || defined(_WIN32)
		#define alloc_handler(x,y)		(y)
	#else
		typedef void (*sa_handler_t)(void);

		struct signal_mapper_t
		{
			signal_handler_t handler;
			int signum;
			sa_handler_t mapper;
		};

		#define DEF_SIG_HANDLER(x)	\
			static void sig_handler_##x(void); \
			static struct signal_mapper_t sig_mapper_##x={NULL,x,sig_handler_##x}; \
			static void sig_handler_##x(void) { sig_mapper_##x.handler(x); }

		#define DEF_SIG_ENTRY(x)   &sig_mapper_##x

		DEF_SIG_HANDLER(SIGABRT)
		#ifdef SIGBUS
		DEF_SIG_HANDLER(SIGBUS)
		#endif
		DEF_SIG_HANDLER(SIGFPE)
		#ifdef SIGHUP
		DEF_SIG_HANDLER(SIGHUP)
		#endif
		DEF_SIG_HANDLER(SIGILL)
		DEF_SIG_HANDLER(SIGINT)
		#ifdef SIGQUIT
		DEF_SIG_HANDLER(SIGQUIT)
		#endif
		#ifdef SIGBREAK
		DEF_SIG_HANDLER(SIGBREAK)
		#endif
		DEF_SIG_HANDLER(SIGSEGV)
		DEF_SIG_HANDLER(SIGTERM)

		static sa_handler_t alloc_handler(int num,signal_handler_t handler)
		{
			static struct signal_mapper_t *map[]={
				DEF_SIG_ENTRY(SIGABRT),
		#ifdef SIGBUS
				DEF_SIG_ENTRY(SIGBUS),
		#endif
				DEF_SIG_ENTRY(SIGFPE),
		#ifdef SIGHUP
				DEF_SIG_ENTRY(SIGHUP),
		#endif
				DEF_SIG_ENTRY(SIGILL),
				DEF_SIG_ENTRY(SIGINT),
		#ifdef SIGQUIT
				DEF_SIG_ENTRY(SIGQUIT),
		#endif
		#ifdef SIGBREAK
				DEF_SIG_ENTRY(SIGBREAK),
		#endif
				DEF_SIG_ENTRY(SIGSEGV),
				DEF_SIG_ENTRY(SIGTERM)
			};
			int x=sizeof(map)/sizeof(map[0]);
			struct signal_mapper_t **p=map;

			while (x--)
			{
				struct signal_mapper_t *q=*p++;

				if (q->signum==num)
				{
					q->handler=handler;
					return q->mapper;
				}
			}

			RHBOPT_ASSERT(!num)

			return NULL;
		}
	#endif /* HAVE_SA_HANDLER_VOID_INT */
#endif /* USE_SIGNALS */

static void install_signal(int signum,signal_handler_t handler)
{
#ifdef USE_SIGNALS
	#ifdef USE_SIGACTION
		struct sigaction sa;
		memset(&sa,0,sizeof(sa));
		sa.sa_handler=alloc_handler(signum,handler);
	#ifdef SA_RESTART
		sa.sa_flags=SA_RESTART;
	#endif
		sigaction(signum,&sa,NULL);
	#else
		signal(signum,handler);
	#endif
#endif
}

#ifdef USE_THREADS
#if defined(USE_PTHREADS)
static void *run_impl_r(void *pv)
#else
static DWORD CALLBACK run_impl_r(void *pv)
#endif
{
	struct run_impl_t *data=(struct run_impl_t *)pv;
	SOMOA_execute_request_loop(SOMD_SOMOAObject,data->ev,SOMD_WAIT);
	data->completed=1;
#ifdef USE_PTHREADS
	kill(data->pid_main,SIGTERM);
	pthread_kill(data->thr_main,SIGTERM);
	return NULL;
#else
	PostThreadMessage(data->dw_main,WM_QUIT,0,0);
	return 0;
#endif
}
#else
static void SOMLINK sink_callback(SOMEEvent SOMSTAR a,void *pv)
{
	struct run_impl_t *data=(struct run_impl_t *)pv;
#ifdef NSIG
	int i=NSIG;
#else
	int i=sizeof(data->pending)<<3;
#endif

	SOMObject_somDumpSelf(a,0);

	while (i--)
	{
		switch (i)
		{
		case SIGINT:
#ifdef SIGQUIT
		case SIGQUIT:
#endif
		case SIGTERM:
#ifdef SIGBREAK
		case SIGBREAK:
#endif
#ifdef SIGHUP
		case SIGHUP:
#endif
			{
#ifdef USE_SIGNALS
				boolean b=sigismember(&data->pending,i);
				sigdelset(&data->pending,i);
#else
				boolean b=0;
#endif
				if (b)
				{
					somPrintf("sink_callback(%d)\n",i);
					switch (i)
					{
			#ifdef SIGBREAK
					case SIGBREAK:
			#endif
			#ifdef SIGQUIT
					case SIGQUIT:
			#endif
					case SIGTERM:
						switch (i)
						{
						case SIGTERM: somPrintf("SIGTERM\n"); break;
						case SIGQUIT: somPrintf("SIGQUIT\n"); break;
			#ifdef SIGBREAK
						case SIGBREAK: somPrintf("SIGBREAK\n"); break;
			#endif
						default: somPrintf("signal(%d)\n",i); break;
						}

						somPrintf("%s:%d quitting due to %d...\n",__FILE__,__LINE__,i);
						somdsvr_quit_flag=1;
						if (SOMD_SOMOAObject)
						{
							Environment ev;
							SOM_InitEnvironment(&ev);
							SOMOA_interrupt_server(SOMD_SOMOAObject,&ev);
							SOM_UninitEnvironment(&ev);
						}
						break;
					case SIGINT:
						somPrintf("SIGINT\n");
						break;
			#ifdef SIGHUP
					case SIGHUP:
						somPrintf("SIGHUP\n");
						break;
			#endif
					}
				}
			}
		}
	}
}
#endif

static void run_impl(ImplementationDef SOMSTAR impl,
				Environment *ev,
				int argc,
				char **argv
#ifndef USE_THREADS
				,SOMEEMan SOMSTAR eman
#endif
				)
{
	RHBOPT_unused(argc);

	if (ev->_major)
	{
		somPrintf("%s\n",somExceptionId(ev));
		return;
	}

	SOMD_ImplDefObject=impl;

/*	SOMObject_somDumpSelf(impl,0);*/

	RHBSOM_Trace("run_impl, creating SOMOA_impl_is_ready")

	SOMOA_impl_is_ready(SOMD_SOMOAObject,ev,impl);

	if (ev->_major)
	{
		somPrintf("%s\n",somExceptionId(ev));
	}
	else
	{
		char *q=ImplementationDef__get_impl_id(impl,ev);

		if (q)
		{
			somPrintf("%s %s ready\n",argv[0],q);

			SOMFree(q);
		}

		if (!somdsvr_quit_flag)
		{
			struct run_impl_t data;
#ifdef USE_THREADS
	#if defined(USE_PTHREADS)
			sigset_t sigs;
			void *pv=NULL;
	#endif
			data.completed=0;
			data.ev=ev;
	#if defined(USE_PTHREADS)
			sigemptyset(&sigs);
			sigaddset(&sigs,SIGINT);
			sigaddset(&sigs,SIGTERM);
	#ifdef SIGQUIT
			sigaddset(&sigs,SIGQUIT);
	#endif
	#ifdef SIGBREAK
			sigaddset(&sigs,SIGBREAK);
	#endif
#endif

#ifdef USE_PTHREADS
			data.pid_main=getpid();
			data.thr_main=pthread_self();
			pthread_create(&data.thr_child,RHBOPT_pthread_attr_default,run_impl_r,&data);
#else
			data.dw_main=GetCurrentThreadId();
			data.h_child=CreateThread(NULL,0,run_impl_r,&data,0,&data.dw_child);
#endif
			g_run_impl=&data;

			while (!data.completed)
			{
				int quit=0;
				int i=0;
#ifdef USE_PTHREADS
				sigwait(&sigs,&i);
				switch (i)
				{
#ifdef SIGBREAK
				case SIGBREAK:
#endif
				case SIGTERM:
				case SIGQUIT:
					quit=1;
					break;
				}
#else
				MSG msg;
				if (GetMessage(&msg,0,0,0))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				if (msg.message==WM_QUIT) quit=1;
#endif
				if (quit && SOMD_SOMOAObject && !data.completed)
				{
					Environment ev2;
					SOM_InitEnvironment(&ev2);
					SOMOA_interrupt_server(SOMD_SOMOAObject,&ev2);
					SOM_UninitEnvironment(&ev2);
				}
			}
			g_run_impl=NULL;
#ifdef USE_PTHREADS
			pthread_join(data.thr_child,&pv);
#else
			WaitForSingleObject(data.h_child,INFINITE);
			CloseHandle(data.h_child);
#endif
#else
			SOM_InitEnvironment(&data.ev);
			data.emanObject=eman;
			data.eventName=(char *)"signal";
			data.clientEvent=SOMEClientEventNew();
			SOMEClientEvent_somevSetEventClientType(data.clientEvent,ev,data.eventName);
			SOMEClientEvent_somevSetEventClientData(data.clientEvent,ev,&data);
	#ifdef USE_POSIX_SIGNALS
			sigemptyset(&data.pending);
	#else
		#error catch for moment
	#endif
			data.regData=SOMEEMRegisterDataNew();
			SOMEEMRegisterData_someClearRegData(data.regData,ev);
#ifdef __cplusplus
			data.regData->someSetRegDataEventMask(ev,EMClientEvent,NULL);
#else
			somva_SOMEEMRegisterData_someSetRegDataEventMask(data.regData,ev,EMClientEvent,NULL);
#endif
			SOMEEMRegisterData_someSetRegDataClientType(data.regData,ev,data.eventName);
			data.token=SOMEEMan_someRegisterProc(eman,ev,data.regData,sink_callback,&data);
			g_run_impl=&data;
			SOMOA_execute_request_loop(SOMD_SOMOAObject,ev,SOMD_WAIT);
			g_run_impl=NULL;
			SOMEEMan_someUnRegister(eman,ev,data.token);
			SOMObject_somFree(data.regData);
			SOMObject_somFree(data.clientEvent);
			SOM_UninitEnvironment(&data.ev);
#endif
		}
	}

	if (!ev->_major)
	{
		SOMOA_deactivate_impl(SOMD_SOMOAObject,ev,impl);
	}

	SOMD_ImplDefObject=NULL;
}

#ifndef USE_THREADS
static void SOMLINK somdsvr_callback(void *a,void *b)
{
	Environment ev;
/*	somPrintf("somdsvr_callback(%s,%d)\n",__FILE__,__LINE__);*/
	RHBOPT_unused(a);
	RHBOPT_unused(b);
	SOM_InitEnvironment(&ev);
	if (SOMD_SOMOAObject) SOMOA_execute_next_request(SOMD_SOMOAObject,&ev,SOMD_NO_WAIT);
	SOM_UninitEnvironment(&ev);
}
#endif

static boolean check_for_quit(void)
{
#ifdef _WIN32
	MSG msg;
	if (PeekMessage(&msg,(HWND)-1L,WM_QUIT,WM_QUIT,PM_NOREMOVE))
	{
		somPrintf("PeekMessage reports a WM_QUIT waiting\n");
		return 1;
	}
#endif

	if (somdsvr_quit_flag) return 1;

	return 0;
}

#ifdef USE_THREADS
	#if defined(USE_PTHREADS)
	static void signal_thread(int sig)
	{
		RHBOPT_unused(sig);
	#ifdef USE_PTHREADS
		#ifdef PTHREAD_CANCELED
			pthread_exit((char *)PTHREAD_CANCELED);
		#else
			pthread_exit((char *)-1);
		#endif
	#else
		pth_exit(PTH_CANCELED);
	#endif
	}
	#endif
	#ifdef USE_PTHREADS
		#ifdef HAVE_PTHREAD_ATFORK
			static void somdsvr_prepare(void)
			{
/*				somStartCriticalSection();*/
			}
			static void somdsvr_parent(void)
			{
/*				somEndCriticalSection();*/
			}
			static void somdsvr_child(void)
			{
/*				somEndCriticalSection();*/
			}
		#endif
	#endif
#endif

int main(int argc,char **argv)
{
Environment ev;
/*SOMClassMgr SOMSTAR clsMgr;*/
int i=1;
ImplementationDef SOMSTAR impl=NULL;
#ifdef USE_SELECT
SOMEEMan SOMSTAR eman=NULL;
#endif

#ifdef USE_POSIX_SIGNALS
	{
		sigset_t sigs;
		sigemptyset(&sigs);
		sigaddset(&sigs,SIGTERM);
		sigaddset(&sigs,SIGINT);
		sigaddset(&sigs,SIGHUP);
	#ifdef SIGBREAK
		sigaddset(&sigs,SIGBREAK);
	#endif
	#ifdef SIGQUIT
		sigaddset(&sigs,SIGQUIT);
	#endif

		install_signal(SIGINT,my_signal_handler);
		install_signal(SIGTERM,my_signal_handler);
		install_signal(SIGHUP,my_signal_handler);
	#ifdef SIGQUIT
		install_signal(SIGQUIT,my_signal_handler);
	#endif
	#ifdef SIGBREAK
		install_signal(SIGBREAK,my_signal_handler);
	#endif

	#ifdef USE_PTHREADS
		pthread_sigmask(SIG_BLOCK,&sigs,NULL);
	#else
		sigprocmask(SIG_BLOCK,&sigs,NULL);
	#endif

	}
#else
	signal(SIGTERM,my_signal_handler);
	signal(SIGINT,my_signal_handler);
	signal(SIGBREAK,my_signal_handler);
#endif

#if defined(_WIN32) && defined(_DEBUG) && (0)
	Sleep(5000);
#endif

#ifdef USE_THREADS
	#ifdef USE_SIGACTION
	{
static struct sigaction sa;
#ifdef SA_RESTART
		sa.sa_flags=SA_RESTART;
#endif
#ifdef SIGBUS
		sa.sa_handler=alloc_handler(SIGBUS,signal_thread);
		sigaction(SIGBUS,&sa,NULL);
#endif
		sa.sa_handler=alloc_handler(SIGSEGV,signal_thread);
		sigaction(SIGSEGV,&sa,NULL);
		sa.sa_handler=alloc_handler(SIGILL,signal_thread);
		sigaction(SIGILL,&sa,NULL);
		sa.sa_handler=alloc_handler(SIGFPE,signal_thread);
		sigaction(SIGFPE,&sa,NULL);
		sa.sa_handler=alloc_handler(SIGABRT,signal_thread);
		sigaction(SIGABRT,&sa,NULL);
	}
	#else
		#if defined(USE_PTHREADS)
			#error
		#endif
	#endif
#endif

	main_pid=getpid();

#ifdef HAVE_ATEXIT
	atexit(atexit_callback);
#else
	#ifdef _WIN32
		#error what happened here?
	#endif
#endif

	RHBSOM_Trace("main, about to call somEnvironmentNew")

	/* clsMgr= */ somMainProgram();

	RHBSOM_Trace("main, about to call SOM_InitEnvironment")

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		#ifdef HAVE_PTHREAD_ATFORK
			pthread_atfork(
				somdsvr_prepare,
				somdsvr_parent,
				somdsvr_child);
		#endif
	#endif
#endif

	SOM_InitEnvironment(&ev);

	RHBSOM_Trace("main, about to call SOMD_Init")

	SOMD_Init(&ev);

	if (ev._major)
	{
		somPrintf("somdsvr: %s\n",somExceptionId(&ev));
		somExceptionFree(&ev);

		somEnvironmentEnd();

		return 1;
	}

	RHBSOM_Trace("main, SOMD_Init returned")

	if (ev._major)
	{
		char *p=somExceptionId(&ev);
		somPrintf("%s\n",p);
		somExceptionFree(&ev);
	}

#ifdef USE_SELECT
	RHBSOM_Trace("starting eman")

	eman=SOMEEManNew();
	if (eman)
	{
		SOMD_RegisterCallback(eman,somdsvr_callback);
	}
#endif

	SOMD_SOMOAObject=SOMOANew();

	if (SOMD_SOMOAObject)
	{
#if defined(USE_POSIX_SIGNALS) && !defined(USE_THREADS)
		sigset_t old_mask,sigs;
		sigemptyset(&sigs);
		sigaddset(&sigs,SIGINT);
		sigaddset(&sigs,SIGTERM);
		sigprocmask(SIG_UNBLOCK,&sigs,&old_mask);
#endif

		if (SOMD_ImplRepObject && !check_for_quit())
		{
			while (i < argc)
			{
				if (0==strcmp(argv[i],"-a"))
				{
					i++;

					if (i < argc)
					{
						char *a;
						a=argv[i];
						if (a)
						{
							impl=ImplRepository_find_impldef_by_alias(
									SOMD_ImplRepObject,
									&ev,
									a);

							if (!impl)
							{
								if (!ev._major)
								{
									somSetException(&ev,SYSTEM_EXCEPTION,ex_StExcep_BAD_PARAM,0);
								}
							}
						}
					}
				}
				else
				{
					impl=ImplRepository_find_impldef(
							SOMD_ImplRepObject,
							&ev,
							argv[i]);

					if (!impl)
					{
						if (!ev._major)
						{
							somSetException(&ev,SYSTEM_EXCEPTION,ex_StExcep_BAD_PARAM,0);
						}
					}
				}

				i++;
			}
		}

		RHBSOM_Trace("main, checking environment again")

		if (!ev._major)
		{
			RHBSOM_Trace("main, trying to run server")

			if (impl)
			{
				run_impl(impl,&ev,argc,argv
#ifndef USE_THREADS
				,eman
#endif
				);

				somReleaseObjectReference(impl);
			}
		}
		
		#if defined(USE_POSIX_SIGNALS) && !defined(USE_THREADS)
			sigprocmask(SIG_BLOCK,&sigs,NULL);			
		#endif

		if (SOMD_SOMOAObject)
		{
			SOMObject SOMSTAR obj=NULL;
/*			somStartCriticalSection();*/
			obj=SOMD_SOMOAObject;
			SOMD_SOMOAObject=NULL;
/*			somEndCriticalSection();*/
			if (obj) somReleaseObjectReference(obj);
		}
	}

	if (ev._major)
	{
		char *p=somExceptionId(&ev);
		somPrintf("%s\n",p);
		somExceptionFree(&ev);
		impl=0;
	}

#ifdef USE_SELECT
	if (eman)
	{
		SOMD_RegisterCallback(NULL,NULL);
		somReleaseObjectReference(eman);
		eman=NULL;
	}
#endif

	RHBSOM_Trace("main, calling SOMD_Uninit")

	SOMD_Uninit(&ev);

	RHBSOM_Trace("main, calling SOM_UninitEnvironment")

	SOM_UninitEnvironment(&ev);

	RHBSOM_Trace("main, calling somEnvironmentEnd")

	somPrintf("%s finished\n",argv[0]);

	somEnvironmentEnd();

	RHBSOM_Trace("main, ended")

	return 0;
}

#if defined(_WIN32) && !defined(_CONSOLE)
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrev,LPSTR lpCmdLine,INT nCmdShow)
{
	char buf[256];
	if (GetModuleFileName(hInstance,buf,sizeof(buf)))
	{
		char *p=lpCmdLine;
		char *argv[3]={buf,NULL,NULL};
		int argc=1;
		if (p) 
		{
			while (*p)
			{
				if (*p > ' ') break;
				p++;
			}
		}
		if (p) if (*p) argv[argc++]=p;
		return main(argc,argv);
	}

	return 1;
}
#endif
