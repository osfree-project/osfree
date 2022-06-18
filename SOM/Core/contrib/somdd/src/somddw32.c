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
#include <rhbsomdd.h>
#include <objbase.h>
#include <stdio.h>
#ifndef DONT_USE_SIGNALS
	#include <signal.h>
#endif
#include <somddmsg.h>
#include <regstr.h>
#include <rhbutils.h>
#include <rhbwinex.h>

/* need to maintain a list of environment variables modified
	(1) read out of service config
	(2) passed in on command line
	(3) modified by RPC
	*/

static char szServiceName[]="SOMDD";
static const char *SOM_env[]={"SOMDDIR","SOMBASE","SOMIR","SOMDPORT",NULL};
static const char szWindowsClassName[]="SOMDD";
static const char szEnvironment[]="Environment";
static const char szDescription[]="Description";
static const char achServiceDependancies[]="RpcSs\0Eventlog\0\0";

struct SOMDD_daemon
{
	SERVICE_STATUS_HANDLE h_service;
	SERVICE_STATUS status;
};

static struct SOMDD_daemon *somdd_daemon;
static HANDLE somdd_eventSource;
static PSID somdd_eventSid;

static void somddmsg(BOOL b)
{
	HANDLE h=LoadLibrary("SOMDDMSG");
	if (h)
	{
		typedef HRESULT (CALLBACK *fn)(void);
		fn f=(fn)GetProcAddress(h,
			b ? "DllRegisterServer" : "DllUnregisterServer");
		if (f)
		{
			f();
		}
		FreeLibrary(h);
	}
}

#if defined(USE_THREADS) && !defined(USE_PTHREADS) 
static struct 
{
	int severity;
	WORD type;
} sevMap[]={
	{SOMDD_LOG_OK,EVENTLOG_SUCCESS},
	{SOMDD_LOG_INFO,EVENTLOG_INFORMATION_TYPE},
	{SOMDD_LOG_WARN,EVENTLOG_WARNING_TYPE},
	{SOMDD_LOG_ERROR,EVENTLOG_ERROR_TYPE}
};
void SOMDD_syslog(struct SOMDD_msg *msg,...)
{
	if (somdd_eventSource)
	{
		va_list ap;
		char *args[32];
		char **h=args;
		WORD c=0;
		int i=0;
		WORD type=SOMDD_LOG_INFO;

		va_start(ap,msg);

		i=sizeof(sevMap)/sizeof(sevMap[0]);
		while (i--)
		{
			if (sevMap[i].severity==msg->type)
			{
				type=sevMap[i].type;
				break;
			}
		}

		while (c < (sizeof(args)/sizeof(args[0])))
		{
			char *p=va_arg(ap,char *);
			*h++=p;
			if (!p) break;
			c++;
		}

		if (msg->id)
		{
			c=ReportEvent(somdd_eventSource,
				type,
				0,
				msg->id,
				somdd_eventSid,
				c,
				0,
				args,
				NULL);
		}
		else
		{
#ifdef _DEBUG
#	ifdef _M_IX86
			__asm int 3
#	endif
#endif
		}

		va_end(ap);
	}
}
#endif

#define IsWindowsNT()   (0==(GetVersion() & 0x80000000))

static void WINAPI SOMDD_ServiceCtrlHandler(DWORD dw)
{
	somPrintf("SOMDD_ServiceCtrlHandler(%d)\n",dw);

	switch (dw)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		if (somdd_daemon)
		{
			if (somdd_daemon->h_service)
			{
				switch (somdd_daemon->status.dwCurrentState)
				{
				case SERVICE_RUNNING:
					somdd_daemon->status.dwCurrentState=SERVICE_STOP_PENDING;
					somdd_daemon->status.dwCheckPoint++;
					SetServiceStatus(somdd_daemon->h_service,&somdd_daemon->status);
					break;
				}
			}
		}
		if (SOMD_SOMOAObject)
		{
			SOMOA SOMSTAR somoa=NULL;
#ifdef USE_THREADS
			somToken mutex=SOMDD_AcquireGlobalMutex();
#endif

			if (SOMD_SOMOAObject)
			{
				somoa=SOMOA_somDuplicateReference(SOMD_SOMOAObject);
			}

#ifdef USE_THREADS
			SOMDD_ReleaseGlobalMutex(mutex);
#endif

			if (somoa)
			{
				Environment ev={NO_EXCEPTION,{NULL,NULL},NULL};
				SOMOA_interrupt_server(somoa,&ev);
				SOMOA_somRelease(somoa);
				SOM_UninitEnvironment(&ev);
			}

		}
		break;
	case SERVICE_CONTROL_INTERROGATE:
		if (somdd_daemon)
		{
			if (somdd_daemon->h_service)
			{
				somdd_daemon->status.dwCheckPoint++;
				SetServiceStatus(somdd_daemon->h_service,&somdd_daemon->status);
			}
		}
		break;
	}
}

static HKEY SOMDD_open_service_key(char *szService,boolean writeAccess)
{
HKEY key=NULL;
char buf[256];

	strncpy(buf,REGSTR_PATH_SERVICES,sizeof(buf));
	strncat(buf,"\\",sizeof(buf));
	strncat(buf,szService,sizeof(buf));

	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,buf,0,
		writeAccess ?
		(KEY_CREATE_SUB_KEY|KEY_WRITE) :
	    (KEY_READ),
		&key))
	{
		return key;
	}

	return NULL;
}

static void SOMDD_reg_env(char *szServiceName)
{
HKEY key=SOMDD_open_service_key(szServiceName,1);

	if (key)
	{
		DWORD disp=0;
		HKEY keyenv=NULL;
		char buf[256];
		size_t len=LoadString(NULL,2,buf,sizeof(buf));

		if (len>0)
		{
			len=RegSetValueEx(key,
				szDescription,0,
				REG_SZ,buf,(DWORD)len);
		}

		if (!RegCreateKeyEx(key,
			szEnvironment,
			0,
			0,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&keyenv,
			&disp))
		{
			if (disp==REG_CREATED_NEW_KEY)
			{
				const char **h=SOM_env;

				while (*h)
				{
					const char *e=*h++;
					char env[2048];
					DWORD len=GetEnvironmentVariable(e,env,sizeof(env)-1);
					if (len)
					{
						RegSetValueEx(keyenv,e,0,REG_EXPAND_SZ,env,len+1);
					}
				}
			}

			RegCloseKey(keyenv);
		}

		RegCloseKey(key);
	}
}

static int SOMDD_RegServerNT(char *szServiceName,char *title)
{
	int retval=1;
	char application_name[MAX_PATH];
	DWORD err=0;
	int i=GetModuleFileName(NULL,application_name,sizeof(application_name));
	if (i)
	{
		SC_HANDLE sc=OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE);
		
		if (sc)
		{
			SC_HANDLE h=CreateService(sc,
					szServiceName,
					title,
					SERVICE_ALL_ACCESS,
					SERVICE_WIN32_OWN_PROCESS
	#ifdef _DEBUG
					|SERVICE_INTERACTIVE_PROCESS
	#endif
					,
					SERVICE_DEMAND_START,
					SERVICE_ERROR_IGNORE,
					application_name,
					0,
					NULL,
					achServiceDependancies,
					NULL,
					NULL);

			if (h) 
			{
				CloseServiceHandle(h);

				SOMDD_reg_env(szServiceName);

				somddmsg(TRUE);

				retval=0;
			}
			else
			{
				err=GetLastError();
			}

			CloseServiceHandle(sc);
		}
		else
		{
			err=GetLastError();
		}
	}
	else
	{
		err=GetLastError();
	}

	if (err) 
	{
		rhbutils_print_Win32Error(err,"SOMDD_RegServerNT");
	}

	return retval;
}

static int SOMDD_UnregServerNT(const char *szServiceName)
{
	int retval=1;
	DWORD err=0;
	SC_HANDLE sc=OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE);

	if (sc)
	{
		SC_HANDLE h=OpenService(sc,szServiceName,STANDARD_RIGHTS_REQUIRED);

		if (h)
		{
			if (DeleteService(h)) 
			{
				retval=0;

				somddmsg(FALSE);
			}
			else
			{
				err=GetLastError();
			}

			CloseServiceHandle(h);
		}
		else
		{
			err=GetLastError();
		}

		CloseServiceHandle(sc);
	}
	else
	{
		err=GetLastError();
	}

	if (!err)
	{
		char systemreg[4096],systemdir[4096];

		strncpy(systemreg,"%",sizeof(systemreg));
		strncat(systemreg,REGSTR_VAL_SYSTEMROOT,sizeof(systemreg));
		strncat(systemreg,"%\\System32\\",sizeof(systemreg));
		strncat(systemreg,szServiceName,sizeof(systemreg));
	
		if (ExpandEnvironmentStrings(systemreg,systemdir,sizeof(systemdir)-1))
		{
			RemoveDirectory(systemdir);
		}
	}

	if (err) 
	{
		rhbutils_print_Win32Error(err,"SOMDD_UnregServerNT");
	}

	return retval;
}

static HANDLE CreateNullFile(void)
{
	HANDLE h=CreateFile(
		"NUL",
		GENERIC_WRITE|GENERIC_READ,
		FILE_SHARE_WRITE|FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	return h;
}
/*
void SOMDD_interrupt(void)
{
	SOMDD_event(EVENTLOG_INFORMATION_TYPE,SOMDDMSG_SOMDDInterrupt,NULL);
}

void SOMDD_failed_publish(Environment *ev)
{
	SOMDD_event(EVENTLOG_ERROR_TYPE,SOMDDMSG_SOMDDFailed,somExceptionId(ev),NULL);
}

void SOMDD_failed_initialise(Environment *ev)
{
	SOMDD_event(EVENTLOG_ERROR_TYPE,SOMDDMSG_SOMDDFailed,somExceptionId(ev),NULL);
}
*/
void SOMDD_up(struct SOMDD_daemon *daemon)
{
	if (daemon)
	{
		if (daemon->h_service)
		{
			daemon->status.dwCurrentState=SERVICE_RUNNING;
			daemon->status.dwCheckPoint++;

			SetServiceStatus(daemon->h_service,&daemon->status);
		}
	}

#ifndef _DEBUG
	if (!IsWindowsNT())
	{
		if (FreeConsole())
		{
			HANDLE hStdin=CreateNullFile();
			HANDLE hStdout=CreateNullFile();
			HANDLE hStderr=CreateNullFile();

			BOOL bIn=SetStdHandle(STD_INPUT_HANDLE,hStdin);
			BOOL bOut=SetStdHandle(STD_OUTPUT_HANDLE,hStdout);
			BOOL bErr=SetStdHandle(STD_ERROR_HANDLE,hStderr);
		}
	}
#endif
}

void SOMDD_down(struct SOMDD_daemon *daemon)
{
	SOMDD_AllocConsole();

	if (daemon)
	{
		if (daemon->h_service)
		{
			daemon->status.dwCurrentState=SERVICE_STOP_PENDING;
			daemon->status.dwCheckPoint++;
			SetServiceStatus(daemon->h_service,&daemon->status);
		}
	}
}

static int SOMDD_printf(const char *fmt,...)
{
	HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);

	if (h!=INVALID_HANDLE_VALUE)
	{
		char buf[1024];
		int i;
		DWORD dw;
		va_list args;
		va_start(args,fmt);
		i=vsprintf(buf,fmt,args);
		WriteFile(h,buf,i,&dw,NULL);
		va_end(args);
		return i;
	}

	return -1;
}

static void SOMDD_setenv(char *szServiceName)
{
	HKEY key=SOMDD_open_service_key(szServiceName,0);

	if (key)
	{
		HKEY keyenv=NULL;

		if (!RegOpenKeyEx(key,szEnvironment,0,KEY_QUERY_VALUE,&keyenv))
		{
			DWORD index=0;

			while (1)
			{
				char buf[8192];
				char name[256];
				DWORD nameLen=sizeof(name)-1;
				DWORD bufLen=sizeof(buf)-1;
				DWORD type=0;

				if (!RegEnumValue(keyenv,index++,
					name,&nameLen,
					NULL,
					&type,
					buf,
					&bufLen))
				{
					buf[bufLen]=0;
					switch (type)
					{
					case REG_SZ:
						SetEnvironmentVariable(name,buf);
						SOMDD_printf("set %s=%s\n",name,buf);
						break;
					case REG_EXPAND_SZ:
						{
							char exBuf[16384];
							DWORD dw=ExpandEnvironmentStrings(buf,exBuf,sizeof(exBuf)-1);
							exBuf[dw]=0;
							SetEnvironmentVariable(name,exBuf);
							SOMDD_printf("set %s=%s\n",name,exBuf);
						}
						break;
					default:
						break;
					}
				}
				else
				{
					break;
				}
			}

			RegCloseKey(keyenv);
		}

		RegCloseKey(key);
	}
}


static int SOMDD_run_main(struct SOMDD_daemon *daemon,int argc,char **argv)
{
/*	FARPROC fp=GetProcAddress(GetModuleHandle("OLE32"),"CoInitializeEx");
	HRESULT r=E_FAIL;*/
	int ret=1;
	RHBProcessMgrChild childInfo=RHBPROCESSMGRCHILD_INIT;

	RHBProcessMgr executor={
		RHBPROCESSMGR_INIT_COMMON,
#ifdef USE_SELECT_RHBPROCESSMGR
		RHBPROCESSMGR_INIT
#else
		NULL,
		NULL,
		INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE
#endif
	};

	executor.childInfo=&childInfo;

	if (!RHBProcessMgr_init(&executor))
	{
		if (IsWindowsNT())
		{
			char username[256];
			DWORD dw=sizeof(username);
			somdd_eventSource=RegisterEventSource(NULL,szServiceName);

			if (GetUserName(username,&dw))
			{
				char sid[256],domain[256]={0};
				SID_NAME_USE use=SidTypeUnknown;
				DWORD dwSid=sizeof(sid),dwDomain=sizeof(domain);
				memset(sid,0,sizeof(sid));

				if (LookupAccountName(NULL,
					username,sid,&dwSid,domain,&dwDomain,&use))
				{
					dwSid=GetLengthSid(sid);
				/*	somPrintf("dwSid=%d\n",dwSid);*/
					somdd_eventSid=malloc(dwSid);
					memcpy(somdd_eventSid,sid,dwSid);
				}
			}
		}

	/*	if (fp)
		{
			HRESULT (__stdcall *proc)(void *,DWORD)=(void *)fp;
			r=proc(NULL,4);
		}
		else
		{
			r=CoInitialize(NULL);
		}

		r=SOMDD_init_security();
	*/
		__try
		{
			ret=SOMDD_main(daemon,&executor,argc,argv);
		}
		__finally
		{
			HANDLE h=somdd_eventSource;
			void *pv=somdd_eventSid;
			somdd_eventSid=NULL;
			somdd_eventSource=NULL;
			if (h)
			{
				DeregisterEventSource(h);
			}
			if (pv)
			{
				free(pv);
			}

			RHBProcessMgr_term(&executor);
		}

	/*	if ((r==S_OK)||(r==S_FALSE))
		{
			CoUninitialize();
		}
	*/
	}

	return ret;
}

static void WINAPI SOMDD_ServiceMain(
		DWORD dwArgc,
		LPTSTR *lpszArgv)
{
	struct SOMDD_daemon daemon;
#if defined(USE_PTHREADS) && !defined(DONT_USE_SIGNALS)
	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs,SIGTERM);
	sigaddset(&sigs,SIGINT);
	sigaddset(&sigs,SIGBREAK);
	sigaddset(&sigs,SIGHUP);
#ifdef SIGCHLD
	sigaddset(&sigs,SIGCHLD);
#else
	sigaddset(&sigs,SIGCLD);
#endif
#ifdef SIGIO
	sigaddset(&sigs,SIGIO);
#endif
#ifdef SIGPIPE
	sigaddset(&sigs,SIGPIPE);
#endif
	pthread_sigmask(SIG_BLOCK,&sigs,NULL);
#endif

	memset(&daemon,0,sizeof(daemon));

	daemon.status.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
	daemon.status.dwCurrentState=SERVICE_START_PENDING;
	daemon.status.dwControlsAccepted=SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;

	daemon.status.dwCheckPoint=0;
	daemon.status.dwWaitHint=1000; /* a second */

	__try
	{
		somdd_daemon=&daemon;

		daemon.h_service=RegisterServiceCtrlHandler(szServiceName,SOMDD_ServiceCtrlHandler);

		SOMDD_setenv(szServiceName);

		SetServiceStatus(daemon.h_service,&daemon.status);

		SOMDD_run_main(&daemon,dwArgc,lpszArgv);
	}

	__finally
	{
		somdd_daemon=NULL;
		daemon.status.dwCheckPoint++;
		daemon.status.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
		daemon.status.dwCurrentState=SERVICE_STOPPED;
		daemon.status.dwControlsAccepted=
				SERVICE_ACCEPT_STOP|
				SERVICE_ACCEPT_SHUTDOWN;

		SetServiceStatus(daemon.h_service,&daemon.status);
	}
}

static int SOMDD_WinNT_main(int argc,char **argv,int flags)
{
#ifdef USE_THREADS
BOOL b=0;
SERVICE_TABLE_ENTRY ste[2];

	memset(ste,0,sizeof(ste));

	ste[0].lpServiceName=szServiceName;
	ste[0].lpServiceProc=SOMDD_ServiceMain;

	SetLastError(ERROR_FAILED_SERVICE_CONTROLLER_CONNECT);

	#ifdef _DEBUG
		if (!(flags & SOMDD_FLAGS_DEBUG))
	#endif
		{
			b=StartServiceCtrlDispatcher(ste);
		}

	if (b) 
	{
		return 0;
	}
	else
	{
		DWORD err=GetLastError();

		if (err==ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
		{
			return SOMDD_run_main(NULL,argc,argv);
		}

		rhbutils_print_Win32Error(err,"StartServiceCtrlDispatcher");
	}

	return 1;
#else
	return SOMDD_run_main(NULL,argc,argv);
#endif
}

struct SOMDD_Win95_params
{
	int argc;
	char **argv;
	HWND hwnd;
	int retval;
	HANDLE hThread;
};

static LRESULT CALLBACK SOMDD_Win95_WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
struct SOMDD_Win95_params *params=(void *)GetWindowLongPtr(hwnd,GWLP_USERDATA);

	switch (msg)
	{
	case WM_CREATE:
		if (lParam)
		{
			if (!params)
			{
				LPCREATESTRUCT lpcs=(LPCREATESTRUCT)lParam;
				params=lpcs->lpCreateParams;
				SetWindowLongPtr(hwnd,GWLP_USERDATA,(LPARAM)params);
			}
		}
		break;
	case WM_ENDSESSION:
		if (lParam) return 0;
		SendMessage(hwnd,WM_USER,0,0);
		WaitForSingleObject(params->hThread,INFINITE);
		return 0;
	case WM_USER:
		if (SOMD_SOMOAObject)
		{
			Environment ev;
			SOMOA SOMSTAR n=SOMOA_somDuplicateReference(SOMD_SOMOAObject);
			SOM_InitEnvironment(&ev);
			SOMOA_interrupt_server(n,&ev);
			SOM_UninitEnvironment(&ev);
			SOMOA_somRelease(n);
		}
		return 0;
	case WM_CLOSE:
		SendMessage(hwnd,WM_USER,0,0);
		return 0;
	case WM_PAINT:
		if (hwnd)
		{
			PAINTSTRUCT ps;
			HDC hdc=BeginPaint(hwnd,&ps);
			RECT r;
			GetClientRect(hwnd,&r);
			FillRect(hdc,&r,GetStockObject(WHITE_BRUSH));
			EndPaint(hwnd,&ps);
		}
		return 0;
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static DWORD CALLBACK SOMDD_Win95_thread(void *pv)
{
	struct SOMDD_Win95_params *params=pv;
	__try
	{
		params->retval=SOMDD_run_main(NULL,params->argc,params->argv);
	}
	__finally
	{
		PostMessage(params->hwnd,WM_QUIT,0,0);
	}
	return 0;
}

static int SOMDD_Win95_main(int argc,char **argv,int flags)
{
	WNDCLASS wc;
	ATOM cls=0;
	int retval=1;
	typedef DWORD (__stdcall * LPFNREGISTERSERVICEPROCESS)(DWORD id,DWORD type);
	LPFNREGISTERSERVICEPROCESS RegisterServiceProcess=
		(LPFNREGISTERSERVICEPROCESS)
			GetProcAddress(
				GetModuleHandle("KERNEL32"),
				"RegisterServiceProcess");
	DWORD dwReg=0;
	DWORD pid=GetCurrentProcessId();

	if (RegisterServiceProcess)
	{
		dwReg=RegisterServiceProcess(pid,1);
	}

	memset(&wc,0,sizeof(wc));
	wc.lpszClassName=szWindowsClassName;
	wc.hInstance=GetModuleHandle(NULL);
	wc.lpfnWndProc=SOMDD_Win95_WndProc;
	wc.hIcon=LoadIcon(wc.hInstance,MAKEINTRESOURCE(1));

	cls=RegisterClass(&wc);

	if (cls)
	{
		struct SOMDD_Win95_params params={argc,argv,NULL,retval,NULL};
		char title[32];
		if (!LoadString(wc.hInstance,1,title,sizeof(title)))
		{
			memcpy(title,"SOMDD",6);
		}
		params.hwnd=CreateWindowEx(
			WS_EX_TOOLWINDOW,
			wc.lpszClassName,title,WS_POPUP,
			100,
			100,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			GetDesktopWindow(),
			NULL,
			wc.hInstance,
			&params);

		if (params.hwnd)
		{
			MSG msg;
			DWORD dw=0;

			params.hThread=CreateThread(NULL,0,SOMDD_Win95_thread,&params,0,&dw);

			if (params.hThread)
			{
				while (GetMessage(&msg,0,0,0))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				wc.lpfnWndProc(params.hwnd,WM_USER,0,0);
				WaitForSingleObject(params.hThread,INFINITE);
			}
		}

		if (params.hwnd) DestroyWindow(params.hwnd);

		retval=params.retval;

		UnregisterClass(wc.lpszClassName,wc.hInstance);

		if (params.hThread) 
		{
			BOOL b=FALSE;
			b=CloseHandle(params.hThread);
			RHBOPT_ASSERT(b);
		}
	}

	if (dwReg)
	{
		dwReg=RegisterServiceProcess(pid,0);
	}

	return retval;
}

static int SOMDD_StopServer(char *szServiceName)
{
	int ret=1;

	if (IsWindowsNT())
	{
		SC_HANDLE hMgr=OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT);

		if (hMgr) 
		{
			SC_HANDLE hService=OpenService(hMgr,szServiceName,SERVICE_QUERY_STATUS|SERVICE_STOP|SERVICE_INTERROGATE);

			if (hService)
			{
				SERVICE_STATUS status;

				while (QueryServiceStatus(hService,&status))
				{
					DWORD dw=0;

					if (status.dwCurrentState==SERVICE_STOPPED)
					{
						ret=0;
						break;
					}

					if (status.dwCurrentState==SERVICE_RUNNING)
					{
						if (!ControlService(hService,SERVICE_CONTROL_STOP,&status))
						{
							ret=GetLastError();
							break;
						}
					}

					dw=status.dwWaitHint;

					if (!dw)
					{
						dw=100;
					}

					Sleep(dw);
				}

				CloseServiceHandle(hService);
			}
			else
			{
				ret=GetLastError();
			}

			CloseServiceHandle(hMgr);
		}
		else
		{
			ret=GetLastError();
		}
	}
	else
	{
		HWND hwnd=FindWindow(szWindowsClassName,NULL);
		if (hwnd)
		{
			if (PostMessage(hwnd,WM_USER,0,0))
			{
				ret=0;
			}
			else
			{
				ret=GetLastError();
			}
		}
	}

	if (ret) ret=1;

	return ret;
}

/* Used on 95/95 machines */
static int SOMDD_OpenRunServices(HKEY *key)
{
	int retval=RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"Software\\Microsoft\\Windows\\CurrentVersion\\RunServices",
		0,KEY_ALL_ACCESS,key);

	return retval;
}

static int SOMDD_StartServer(char *szServiceName)
{
	int ret=1;

	if (IsWindowsNT())
	{
		SC_HANDLE hMgr=OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT);

		if (hMgr)
		{
			SC_HANDLE hService=OpenService(hMgr,szServiceName,
					SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_INTERROGATE);

			if (hService)
			{
				SERVICE_STATUS status;

				while (QueryServiceStatus(hService,&status))
				{
					DWORD tm=(DWORD)-1L;

					if (status.dwCurrentState==SERVICE_RUNNING)
					{
						ret=0;
						break;
					}

					switch (status.dwCurrentState)
					{
					case SERVICE_START_PENDING:
						tm=status.dwWaitHint;
						break;
					case SERVICE_STOPPED:
						if (StartService(hService,0,NULL))
						{
							tm=1;
						}
						else
						{
							ret=GetLastError();
						}
					}

					if (tm==(DWORD)-1L) 
					{
						break;
					}

					if (tm==0)
					{
						tm=100;
					}

					Sleep(tm);
				}
				
				if (ret==1)
				{
					ret=GetLastError();
				}

				CloseServiceHandle(hService);
			}
			else
			{
				ret=GetLastError();
			}

			CloseServiceHandle(hMgr);
		}
		else
		{
			ret=GetLastError();
		}
	}
	else
	{
		HKEY key=NULL;
		HWND hwnd=FindWindow(szWindowsClassName,NULL);

		if (hwnd) 
		{
			/* already running... */
			return 0;
		}

		ret=SOMDD_OpenRunServices(&key);

		if (!ret)
		{
			char buf[8192];
			DWORD type=0;
			DWORD len=sizeof(buf);

			ret=RegQueryValueEx(key,szServiceName,NULL,&type,buf,&len);

			if (!ret)
			{
				if (type==REG_SZ)
				{
					STARTUPINFO startup;
					BOOL b;
					PROCESS_INFORMATION info;

					memset(&startup,0,sizeof(startup));
					startup.cb=sizeof(startup.cb);

					GetStartupInfo(&startup);

					b=CreateProcess(NULL,buf,NULL,NULL,0,
						CREATE_NEW_PROCESS_GROUP|DETACHED_PROCESS,
						NULL,
						NULL,
						&startup,
						&info);

					if (b)
					{
						b=CloseHandle(info.hProcess);
						RHBOPT_ASSERT(b);
						b=CloseHandle(info.hThread);
						RHBOPT_ASSERT(b);
					}
					else
					{
						ret=GetLastError();
					}
				}
				else
				{
					ret=1;
				}
			}

			RegCloseKey(key);

		}
	}

	if (ret) ret=1;

	return ret;
}

static int SOMDD_needs_quotes(const char *p,int len)
{
	while (len--)
	{
		if (isspace(*p)) return 1;
		p++;
	}

	return 0;
}

static int SOMDD_RegServer95(const char *szServiceName)
{
	HKEY key=NULL;
	int retval=SOMDD_OpenRunServices(&key);
	if (!retval)
	{
		char mod[MAX_PATH];
		int modlen=GetModuleFileName(NULL,mod,sizeof(mod));
		char buf[8192];
		char *p=buf;
		const char **h=SOM_env;

		memcpy(p,mod,modlen);
		p+=modlen;
		
		while (*h)
		{
			const char *e=*h;
			char ev[8192];
			int len=GetEnvironmentVariable(e,ev,sizeof(ev));
			if (len > 0)
			{
				int q=SOMDD_needs_quotes(ev,len);
				*p++=' ';
				if (q) *p++='\"';
				*p++='/';
				*p++='e';
				memcpy(p,e,strlen(e)+1);
				p+=strlen(p);
				*p++='=';
				memcpy(p,ev,len);
				p+=len;
				if (q) *p++='\"';
			}

			h++;
		}

		*p++=0;

		retval=RegSetValueEx(key,szServiceName,0,REG_SZ,buf,(DWORD)(p-buf));

		RegCloseKey(key);
	}

	if (retval) retval=1;

	return retval;
}

static int SOMDD_UnregServer95(char *szServiceName)
{
	HKEY key=NULL;
	int retval=SOMDD_OpenRunServices(&key);
	if (!retval)
	{
		retval=RegDeleteValue(key,szServiceName);
		RegCloseKey(key);
	}

	if (retval) retval=1;

	return retval;
}

static int SOMDD_main_any(int argc,char **argv)
{
	int flags=0;

	if (argc > 0)
	{
		int i=1;

		while (i < argc)
		{
			char *p=argv[i];

			if ((*p=='/')||(*p=='-'))
			{
				p++;

				if (!strcasecmp(p,"REGSERVER"))
				{
					if (IsWindowsNT())
					{
						char title[32];
						if (!LoadString(GetModuleHandle(NULL),1,title,sizeof(title)))
						{
							strncpy(title,szServiceName,sizeof(title));
						}
						return SOMDD_RegServerNT(szServiceName,title);
					}

					return SOMDD_RegServer95(szServiceName);
				}

				if (!strcasecmp(p,"UNREGSERVER"))
				{
					if (IsWindowsNT())
					{
						return SOMDD_UnregServerNT(szServiceName);
					}

					return SOMDD_UnregServer95(szServiceName);
				}

				if (!strcasecmp(p,"START"))
				{
					return SOMDD_StartServer(szServiceName);
				}

				if (!strcasecmp(p,"STOP"))
				{
					return SOMDD_StopServer(szServiceName);
				}

				if ((*p=='e')||(*p=='E'))
				{
					char *q=++p;
					while (*q)
					{
						if (*q=='=')
						{
							int namelen=(int)(q-p);
							if (namelen)
							{
								char name[256];
								memcpy(name,p,namelen);
								name[namelen]=0;
								q++;
								if (*q)
								{
									SetEnvironmentVariable(name,q);
								}
								else
								{
									SetEnvironmentVariable(name,NULL);
								}
							}

							break;
						}

						if (*q)	q++;
					}
				}
				else
				{
					if (!strcasecmp(p,"DEBUG"))
					{
						flags|=SOMDD_FLAGS_DEBUG;
						p=NULL;
					}
					else
					{
						p--;
						printf("unknown option, %s\n",p);
						return 1;
					}
				}
			}

			if (p)
			{
				i++;
			}
			else
			{
				int k=i;
				while ((k+1) < argc)
				{
					argv[k]=argv[k+1];
					k++;
				}
				argc--;
			}
		}
	}

#if (defined(USE_SELECT) || defined(USE_PTHREADS) ) && !defined(DONT_USE_SIGNALS)
	{
		sigset_t sigs;
		sigemptyset(&sigs);
		sigaddset(&sigs,SIGTERM);
		sigaddset(&sigs,SIGBREAK);
		sigaddset(&sigs,SIGINT);
		sigaddset(&sigs,SIGHUP);
#ifdef SIGCHLD
		sigaddset(&sigs,SIGCHLD);
#else
		sigaddset(&sigs,SIGCLD);
#endif
#ifdef SIGIO
		sigaddset(&sigs,SIGIO);
#endif
#ifdef SIGPIPE
		sigaddset(&sigs,SIGPIPE);
#endif
#ifdef USE_PTHREADS
		pthread_sigmask(SIG_BLOCK,&sigs,NULL);
#else
		sigprocmask(SIG_BLOCK,&sigs,NULL);
#endif
	}
#endif

	if (IsWindowsNT())
	{
		/* must be Win95/98 or WIN32S */

		return SOMDD_WinNT_main(argc,argv,flags);
	}

	return SOMDD_Win95_main(argc,argv,flags);
}


static void SOMDD_unreg_env(char *szServiceName)
{
}

#ifdef _CONSOLE
	/* WIN32 console, SOMDD, use for debugging as well */
	int main(int argc,char **argv)
	{
	int rc=1;

		WSADATA wsd;

		SOMDD_AllocConsole();

		if (!WSAStartup(0x101,&wsd))
		{
			rc=SOMDD_main_any(argc,argv);

			WSACleanup();
		}

		return rc;
	}
#else
	/* WIN32 WinMain, SOMDDSVC use as NT service */
	int WINAPI WinMain(HINSTANCE h,HINSTANCE h2,LPSTR lp,int cmd)
	{
		int rc=1;
		WSADATA wsd;
		char buf[MAX_PATH];
		char *argv[32];
		int argc=0;
		int len=strlen(lp)+1;
		char *args=malloc(len);
		char *p=args;
		memcpy(args,lp,len);

		SOMDD_AllocConsole();

		GetModuleFileName(h,buf,sizeof(buf));

		argv[argc++]=buf;

		while (*p)
		{
			while (isspace(*p))
			{
				p++;

				if (!*p) break;
			}

			if (*p)
			{
				char c=*p;
				switch (c)
				{
				case '\"':
				case '\'':
					p++;
					break;
				default:
					c=0;
				}
				argv[argc++]=p;
				while (*p)
				{
					if (*p==c)
					{
						*p++=0;
						break;
					}
					else
					{
						if (isspace(*p) && !c)
						{
							*p++=0;
							break;
						}
						else
						{
							p++;
						}
					}
				}
			}
		}

		argv[argc]=NULL;

		if (argc)
		{
			int i=0;
			while (i < argc)
			{
				SOMDD_printf("<%s>\n",argv[i++]);
			}
		}

		if (!WSAStartup(0x101,&wsd))
		{
			rc=SOMDD_main_any(argc,argv);

			WSACleanup();
		}

		free(args);

		return rc;
	}
#endif

struct findMyWindow
{
	HWND hwnd;
	DWORD pid;
	DWORD tid;
	const char *text;
};

static BOOL CALLBACK findMyWindow(HWND hwnd,LPARAM pv)
{
	struct findMyWindow *data=(void *)pv;
	DWORD pid=0;
	DWORD tid=GetWindowThreadProcessId(hwnd,&pid);

	if (tid==data->tid)
	{
		if (pid==data->pid)
		{
			char buf[256];
			if (GetWindowText(hwnd,buf,sizeof(buf)))
			{
				if (!strcmp(buf,data->text))
				{
					data->hwnd=hwnd;
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

BOOL SOMDD_AllocConsole(void)
{
	BOOL b=AllocConsole();
	char buf[256];

	if (b)
	{
		size_t len=LoadString(NULL,1,buf,sizeof(buf));

		if (len > 0)
		{
			b=SetConsoleTitle(buf);
		}
	}

	if (b)
	{
		struct findMyWindow data={NULL,GetCurrentProcessId(),GetCurrentThreadId(),buf};
		EnumWindows(findMyWindow,(LPARAM)&data);

		if (data.hwnd)
		{
			HICON icon=LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(1));
			
			if (icon)
			{
				SendMessage(data.hwnd,WM_SETICON,
#ifdef ICON_SMALL
					ICON_SMALL,
#else
					FALSE,
#endif
					(LPARAM)icon);
			}
		}
	}


	return b;
}

