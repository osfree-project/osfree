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
#include <rhbsomex.h>
#include <somref.h>
#include <somd.h>
#include <somos.h>
#include <somosutl.h>
#include <signal.h>
#ifdef _WIN32
	#include <process.h>
	#include <windows.h>  /* only for WM_QUIT & PeekMessage */
#else
	#include <unistd.h>
#endif

#define SOMDServer_somDuplicateReference(x)		SOMRefObject_somDuplicateReference(x)

static int global_quit;

#ifndef USE_PTHREADS
static void sig_quit(int sig)
{
	Environment ev;
	SOMOA SOMSTAR somoa;

	global_quit=1;

	somPrintf("somossvr: pid %d, signal %d\n",getpid(),sig);

	SOM_InitEnvironment(&ev);
	somoa=SOMD_SOMOAObject;

	if (somoa)
	{
		SOMOA_interrupt_server(somoa,&ev);
	}

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

	if (global_quit) return 1;

	return 0;
}

static 
#ifdef HAVE_ATEXIT_VOID
	void
#else
	int
#endif
	call_somos_exit(void)
{
	static boolean has_exitted;

	if (!has_exitted)
	{
		has_exitted=1;
		somos_exit(0);
	}

#ifndef HAVE_ATEXIT_VOID
	return 0; 
#endif
}

static int SOMOSSVR_run(Environment *ev,int argc,char **argv)
{
	boolean initialize_mode=0;

#ifdef somNewObject
	SOMD_SOMOAObject=somNewObject(SOMOA);
#else
	SOMD_SOMOAObject=SOMOANew();
#endif

	if (SOMD_SOMOAObject)
	{
		SOMDServer SOMSTAR server=NULL;
		boolean impl_ready=0;

#ifdef USE_SELECT
	#ifdef _PLATFORM_UNIX_
		sigset_t sigs;
		sigemptyset(&sigs);
		sigaddset(&sigs,SIGTERM);
		sigaddset(&sigs,SIGINT);
	#endif
		signal(SIGINT,sig_quit);	/* call our fn's */
		signal(SIGTERM,sig_quit);
	#ifdef _PLATFORM_UNIX_
		sigprocmask(SIG_UNBLOCK,&sigs,NULL); 
	#endif
#else
	#ifdef USE_PTHREADS
	#else
		signal(SIGINT,sig_quit);
		signal(SIGTERM,sig_quit);
	#endif
#endif
		if ((argc > 1) && !check_for_quit())
		{
			char *p=argv[1];

			if (!strcmp(p,"-a"))
			{
				char *alias_name=argv[2];

				SOMD_ImplDefObject=ImplRepository_find_impldef_by_alias(SOMD_ImplRepObject,ev,alias_name);
			}
			else
			{
				char *impl_name=argv[1];

				SOMD_ImplDefObject=ImplRepository_find_impldef(SOMD_ImplRepObject,ev,impl_name);
			}
		}

		if (!SOMD_ImplDefObject)
		{
			RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadParm,NO);
		}

		if ((!ev->_major) && !check_for_quit())
		{
			somos_init_services(initialize_mode);

			somPrintf("about to call impl_is_ready\n");

			SOMOA_impl_is_ready(SOMD_SOMOAObject,ev,SOMD_ImplDefObject);

			somPrintf("impl_is_ready returned\n");

			if (!ev->_major) impl_ready=1;
		}

		if (!ev->_major && !check_for_quit())
		{
			server=SOMDServer_somDuplicateReference(SOMD_ServerObject);
			somos_init_services_afterimpl(initialize_mode);

			somPrintf("begining request loop\n");

			SOMOA_execute_request_loop(SOMD_SOMOAObject,ev,SOMD_WAIT);

			somPrintf("out of request loop\n");
		}

		if (impl_ready)
		{
			Environment ev2;

			SOM_InitEnvironment(&ev2);
			somPrintf("deactivate impl\n");

			SOMOA_deactivate_impl(SOMD_SOMOAObject,&ev2,SOMD_ImplDefObject);

			SOM_UninitEnvironment(&ev2);
		}

		if (server)
		{
			if (SOMDServer_somIsA(server,_somOS_Server))
			{
				Environment ev2;
				SOM_InitEnvironment(&ev2);
				somPrintf("passivating active objects\n");
				somOS_Server_passivate_all_objects(server,&ev2);
				SOM_UninitEnvironment(&ev2);
			}

			somReleaseObjectReference(server);
		}

#ifdef USE_SELECT
	#ifdef _PLATFORM_UNIX_
		sigprocmask(SIG_BLOCK,&sigs,NULL);
	#else
		signal(SIGINT,SIG_IGN);
		signal(SIGTERM,SIG_IGN);
	#endif
#else
	#ifdef USE_PTHREADS
	#else
		signal(SIGINT,SIG_IGN);
		signal(SIGTERM,SIG_IGN);
	#endif
#endif

		if (SOMD_ImplDefObject)
		{
			somReleaseObjectReference(SOMD_ImplDefObject);
		}

		if (SOMD_SOMOAObject)
		{
			SOMOA SOMSTAR somoa=SOMD_SOMOAObject;
			SOMD_SOMOAObject=NULL;
			if (somoa)
			{
				somReleaseObjectReference(somoa);
			}
		}
	}

	if (ev->_major)
	{
		somPrintf("finished with exception %s\n",somExceptionId(ev));
	}

	return 0;
}

int SOMOSSVR_main(int argc,char **argv)
{
/*	SOMClassMgr SOMSTAR mgr; */
	Environment ev;
	int rc;
#ifdef _PLATFORM_UNIX_
	sigset_t sigs;
#endif

#ifdef _PLATFORM_UNIX_
	/* we don't was SIGTERM or SIGINT affecting this process
		until we are in the event loop */
	sigemptyset(&sigs);
	sigaddset(&sigs,SIGINT);
	sigaddset(&sigs,SIGTERM);
#ifdef USE_PTHREADS
	pthread_sigmask(SIG_BLOCK,&sigs,NULL);
#else
	sigprocmask(SIG_BLOCK,&sigs,NULL);
#endif

#else
	#ifdef _WIN32
		signal(SIGINT,SIG_IGN);
		signal(SIGTERM,SIG_IGN);
	#endif
#endif

	/* mgr= */ somEnvironmentNew();

	SOM_InitEnvironment(&ev);

	somos_init_logging();

	somos_setup();

#ifdef HAVE_ATEXIT
	atexit(call_somos_exit);
#endif

	SOMD_Init(&ev);
	SOMD_NoORBfree();

	rc=SOMOSSVR_run(&ev,argc,argv);

	if (ev._major)
	{
		somPrintf("server finished with %s\n",somExceptionId(&ev));
		somExceptionFree(&ev);
	}

	SOMD_Uninit(&ev);

	call_somos_exit();

	SOM_UninitEnvironment(&ev);

	somEnvironmentEnd();

	return rc;
}


