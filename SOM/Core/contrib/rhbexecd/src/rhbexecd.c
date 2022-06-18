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
#	include <windows.h>
#	ifdef HAVE_WINSOCK2_H
#		include <winsock2.h>
#	else
#		ifdef HAVE_WINSOCK_H
#			include <winsock.h>
#		endif
#	endif
#else
#	include <sys/stat.h>
#	include <sys/socket.h>
#	include <sys/wait.h>
#	include <sys/time.h>
#	include <fcntl.h>
#	ifndef INVALID_SOCKET
		typedef int SOCKET;
#		define INVALID_SOCKET    ((SOCKET)-1)
#	endif
#endif
#include <errno.h>
#include <rhbexecd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#	include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif

struct rhbexecd_process
{
	struct rhbexecd_globals *globals;
	struct rhbexecd_process *next;
	unsigned long id;
#ifdef _WIN32
	PROCESS_INFORMATION pinfo;
#else
	pid_t pid;
#endif
};

struct rhbexecd_message
{
	struct rhbexecd_globals *globals;
	struct rhbexecd_message *next;
	short length;
	unsigned char *ptr;
	unsigned char data[1];
};

struct rhbexecd_globals
{
	struct rhbexecd_process *list;
	int quit;
	unsigned char header[4];
	int header_len;
	struct rhbexecd_message *incoming;
	struct rhbexecd_message *outgoing;
	struct rhbexecd_process *deceased;

#ifdef _WIN32
	HANDLE hQuit,
		   hReaperEvent,
		   hReaperThread,
		   hMutex,
		   fdConsole;
#else
	SOCKET fdConsole;
#endif

#ifdef USE_WIN32_IO
	HANDLE hInput,
		   hOutput,
		   hWriteEvent,
		   hWriteThread;
#else
	SOCKET fdInput,
		   fdOutput,
		   fdSignalRead,
		   fdSignalWrite;
#endif

#ifdef _WIN32
	DWORD currentId;
#endif
};

#ifdef USE_WIN32_IO
#else
#	ifdef _WIN32
#		define pipe_read(a,b,c)		recv(a,b,c,0)
#		define pipe_write(a,b,c)	send(a,b,c,0)
#	else
#		define pipe_read(a,b,c)		read(a,b,c)
#		define pipe_write(a,b,c)	write(a,b,c)
#	endif
static void soclose(SOCKET fd)
{
	int i=
#ifdef _WIN32
		closesocket(fd);
#else
		close(fd);
#endif
	if (i)
	{
#ifdef _WIN32
		__asm int 3
#else
		perror("soclose");
#endif
	}
}
#endif

#ifdef _DEBUG
static void rhbexecd_islocked(struct rhbexecd_globals *globals)
{
#	ifdef _WIN32
		if (globals->currentId!=GetCurrentThreadId())
		{
#		ifdef _M_IX86
			__asm int 3
#		endif
		}
#	endif
}
#else
#	define	rhbexecd_islocked(x)
#endif

static void rhbexecd_lock(struct rhbexecd_globals *globals)
{
#ifdef _WIN32
	WaitForSingleObject(globals->hMutex,INFINITE);
	globals->currentId=GetCurrentThreadId();
#endif
}

static void rhbexecd_unlock(struct rhbexecd_globals *globals)
{
#ifdef _WIN32
	globals->currentId=0;
	ReleaseMutex(globals->hMutex);
#endif
}

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

static struct rhbexecd_message *get_last(struct rhbexecd_message *msg)
{
	if (msg)
	{
		while (msg->next) msg=msg->next;
	}

	return msg;
}

static void remove_msg(struct rhbexecd_message **h,struct rhbexecd_message *p)
{
	if (*h==p)
	{
		*h=p->next;
	}
	else
	{
		struct rhbexecd_message *q=*h;
		while (q->next != p)
		{
			q=q->next;
		}
		q->next=p->next;
	}
}

static int rhbexecd_remove(struct rhbexecd_process **h,struct rhbexecd_process *p)
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
			struct rhbexecd_process *q=*h;

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

static void rhbexecd_addmsg(struct rhbexecd_globals *globals,struct rhbexecd_message *msg)
{
	msg->globals=globals;
	msg->next=NULL;

	if (globals->outgoing)
	{
		struct rhbexecd_message *m=globals->outgoing;
		while (m->next) m=m->next;
		m->next=msg;
	}
	else
	{
		globals->outgoing=msg;
	}

#ifdef USE_WIN32_IO
	{
		BOOL b=SetEvent(globals->hWriteEvent);
		if (!b) 
		{
#ifdef _M_IX86
			__asm int 3
#endif
		}
	}
#else
	{
		int i=pipe_write(globals->fdSignalWrite,"",1);

		if (i!=1)
		{
#ifdef _WIN32
#	ifdef _M_IX86
			__asm int 3
#	endif
#else
			perror("pipe_write");
#endif
		}
	}
#endif
}

static void rhbexecd_obituary(struct rhbexecd_process *ps,long rc)
{
	struct rhbexecd_globals *globals=ps->globals;
	
	rhbexecd_islocked(globals);

	if (rhbexecd_remove(&globals->list,ps))
	{
		struct rhbexecd_message *msg=calloc(sizeof(msg[0])+16,1);

#ifdef _WIN32
		ps->pinfo.dwProcessId=0;
		ps->pinfo.dwThreadId=0;
#else
		ps->pid=0;
#endif

		ps->next=globals->deceased;
		globals->deceased=ps;

		msg->length+=put_int32(msg->data+msg->length,16);
		msg->length+=put_int32(msg->data+msg->length,RHBEXECD_OBITUARY);
		msg->length+=put_int32(msg->data+msg->length,ps->id);
		msg->length+=put_int32(msg->data+msg->length,rc);

		msg->ptr=msg->data;

		rhbexecd_addmsg(globals,msg);
	}
#ifdef _WIN32
	else
	{
#	ifdef _M_IX86
		__asm int 3
#	endif
	}
#endif
}

#ifdef USE_WIN32_IO
static void rhbexecd_read_win32(struct rhbexecd_globals *globals)
{
	if (globals->hInput!=INVALID_HANDLE_VALUE)
	{
		int doClose=1;

		if (globals->incoming && globals->incoming->length)
		{
			DWORD dw=globals->incoming->length;

			if (ReadFile(globals->hInput,
				 globals->incoming->ptr,
				 dw,&dw,NULL))
			{
				if (dw)
				{
					globals->incoming->ptr+=dw;
					globals->incoming->length-=(short)dw;
					doClose=0;
				}
			}
			else
			{
				DWORD err=GetLastError();
				fprintf(stderr,"read_msg - error=%ld,%lx\n",err,err);
				fflush(stderr);
			}
		}
		else
		{
			DWORD dw=sizeof(globals->header)-globals->header_len;

			if (ReadFile(globals->hInput,globals->header+globals->header_len,dw,&dw,NULL))
			{
				if (dw)
				{
					doClose=0;

					globals->header_len+=dw;

					if (globals->header_len==sizeof(globals->header))
					{
						DWORD len=get_int32(globals->header);
						struct rhbexecd_message *msg=calloc(1,sizeof(msg[0])+len);
						msg->length=(short)(len-sizeof(globals->header));
						msg->ptr=msg->data;
						memcpy(msg->ptr,globals->header,sizeof(globals->header));
						msg->ptr+=sizeof(globals->header);

						msg->next=globals->incoming;
						globals->incoming=msg;

						globals->header_len=0;
					}
				}
			}
			else
			{
				DWORD err=GetLastError();
				fprintf(stderr,"read header - error=%ld,%lx\n",err,err);
				fflush(stderr);
			}
		}

		if (doClose)
		{
			globals->quit=1;
			globals->hInput=INVALID_HANDLE_VALUE;
		}
	}
}
#else
static void rhbexecd_select(struct rhbexecd_globals *g)
{
	fd_set fdr,fdw;
	SOCKET n=g->fdSignalRead;
	int i;

#ifdef _WIN32
#else
	while (g)
	{
		int s=0;
		pid_t pid=waitpid(0,&s,WNOHANG);

		if ((pid != -1)&&(pid!=0))
		{
			struct rhbexecd_process *proc=g->list;

			while (proc)
			{
				if (proc->pid==pid)
				{
					rhbexecd_obituary(proc,s);
				}

				proc=proc->next;
			}
		}
		else
		{
			break;
		}
	}
#endif

	FD_ZERO(&fdr);
	FD_ZERO(&fdw);

	if (g->fdSignalRead!=INVALID_SOCKET)
	{
		FD_SET(g->fdSignalRead,&fdr);
	}

	if (g->fdInput!=INVALID_SOCKET)
	{
		FD_SET(g->fdInput,&fdr);

		if (g->fdInput > n) n=g->fdInput;
	}

	if ((g->fdOutput!=INVALID_SOCKET) && (g->outgoing))
	{
		FD_SET(g->fdOutput,&fdw);

		if (g->fdOutput > n) n=g->fdOutput;
	}

	i=select(n+1,&fdr,&fdw,NULL,NULL);

	if (i==-1) 
	{
#ifdef _WIN32
		__asm int 3
#else
		if (errno!=EINTR)
		{
			perror("select");
		}
#endif
	}

	if (i > 0)
	{
		if (g->outgoing && 
			(g->fdOutput!=INVALID_SOCKET) && 
			(FD_ISSET(g->fdOutput,&fdw)))
		{
			struct rhbexecd_message *msg=g->outgoing;

			i=pipe_write(g->fdOutput,
					msg->ptr,
					msg->length);

			if (i > 0)
			{
				msg->ptr+=i;
				msg->length-=i;

				if (!msg->length)
				{
					g->outgoing=msg->next;

					free(msg);
				}
			}
			else
			{
				perror("pipe_write");
			}
		}

		if ((g->fdSignalRead!=INVALID_SOCKET) && (FD_ISSET(g->fdSignalRead,&fdr)))
		{
			char buf[1];
			int i=pipe_read(g->fdSignalRead,buf,sizeof(buf));

			if (i < 0)
			{
#ifdef _WIN32
				__asm int 3;
#else
				perror("pipe_read signal read");
#endif
			}
			else
			{
				if (i==0)
				{
					g->quit=1;
				}
			}
		}

		if ((g->fdInput!=INVALID_SOCKET) && (FD_ISSET(g->fdInput,&fdr)))
		{
			int i=-1;

			if (g->incoming && g->incoming->length)
			{
/*				i=pipe_read(g->fdInput,buf,sizeof(buf));*/

				i=pipe_read(g->fdInput,g->incoming->ptr,g->incoming->length);

				if (i > 0)
				{
					g->incoming->length-=i;
					g->incoming->ptr+=i;
				}
				else
				{
					if (i)
					{
#ifdef _WIN32
						__asm int 3;
#else
						perror("pipe_read fdInput message");
#endif
					}
				}
			}
			else
			{
				int k=sizeof(g->header)-g->header_len;

				i=pipe_read(g->fdInput,g->header+g->header_len,k);

				if (i > 0)
				{
					g->header_len+=i;

					if (g->header_len==sizeof(g->header))
					{
						struct rhbexecd_message *msg;
						k=get_int32(g->header);

						msg=malloc(sizeof(*msg)+k);

						msg->next=g->incoming;
						msg->length=k-sizeof(g->header);
						msg->ptr=msg->data+sizeof(g->header);
						msg->globals=g;

						g->incoming=msg;

						memcpy(msg->data,g->header,sizeof(g->header));

						g->header_len=0;
					}
				}
				else
				{
					if (i)
					{
#ifdef _WIN32
						__asm int 3;
#else
						perror("pipe_read fdInput header");
#endif
					}
				}
			}
			
			if (i==0)
			{
				g->quit=1;
			}
		}
	}
}
#endif

#ifdef USE_WIN32_IO
static DWORD CALLBACK rhbexecd_writer(void *pv)
{
struct rhbexecd_globals *globals=pv;

	rhbexecd_lock(globals);

	while ((!globals->quit)||(globals->outgoing))
	{
		struct rhbexecd_message *msg=globals->outgoing;

		if (msg)
		{
			globals->outgoing=msg->next;
		}

		rhbexecd_unlock(globals);

		if (msg)
		{
			if (msg->data != msg->ptr)
			{
#ifdef _M_IX86
				__asm int 3
#endif
			}

			while (msg->length)
			{
				DWORD len=msg->length;
				char *p=msg->ptr;

				if (WriteFile(globals->hOutput,p,len,&len,NULL))
				{
					msg->ptr+=len;
					msg->length-=(short)len;

					if (!len)
					{
#ifdef _M_IX86
						__asm int 3
#endif

						break;
					}
				}
				else
				{
#ifdef _M_IX86
					__asm int 3
#endif

					break;
				}
			}

			free(msg);
		}
		else
		{
			WaitForSingleObject(globals->hWriteEvent,INFINITE);
		}

		rhbexecd_lock(globals);
	}

	rhbexecd_unlock(globals);

	return 0;
}
#endif

#ifdef _WIN32
static DWORD CALLBACK rhbexecd_reaper(void *pv)
{
struct rhbexecd_globals *globals=pv;

	while ((!globals->quit)||(globals->list))
	{
		DWORD dw=0;
		DWORD nCount=0;
		HANDLE handles[MAXIMUM_WAIT_OBJECTS];
		struct rhbexecd_process *ps;

		handles[nCount++]=globals->hReaperEvent;

		rhbexecd_lock(globals);

		ps=globals->list;

		while (ps)
		{
			handles[nCount++]=ps->pinfo.hProcess;

			ps=ps->next;
		}

		rhbexecd_unlock(globals);

		dw=WaitForMultipleObjects(nCount,handles,FALSE,INFINITE);

		if ((dw >= WAIT_OBJECT_0)&&
			(dw < (WAIT_OBJECT_0+nCount)))
		{
			HANDLE h=handles[dw-WAIT_OBJECT_0];

			rhbexecd_lock(globals);

			ps=globals->list;

			while (ps)
			{
				if (h==ps->pinfo.hProcess)
				{
					DWORD dw=0;

					if (!GetExitCodeProcess(ps->pinfo.hProcess,&dw))
					{
#ifdef _M_IX86
						__asm int 3
#endif
						dw=GetLastError();
					}

					CloseHandle(ps->pinfo.hProcess);

					ps->pinfo.hProcess=NULL;

					if (ps->pinfo.hThread);
					{
						CloseHandle(ps->pinfo.hThread);

						ps->pinfo.hThread=NULL;
					}

					rhbexecd_obituary(ps,dw);

					break;
				}

				ps=ps->next;
			}

			rhbexecd_unlock(globals);
		}
	}

	return 0;
}
#else
static struct rhbexecd_globals *gGlobals;

static void rhbexecd_signal(int x)
{
	if (gGlobals)
	{
		SOCKET fd=gGlobals->fdSignalWrite;
		if (fd!=INVALID_SOCKET)
		{
			char buf[1];
			buf[0]=x;
			pipe_write(fd,buf,sizeof(buf));
		}
	}
}
#endif

static void handle_msg(
		struct rhbexecd_globals *globals,
		struct rhbexecd_message *msg)
{
	unsigned long cmd=get_int32(msg->data+4);
	unsigned long id=get_int32(msg->data+8);

	switch (cmd)
	{
	case RHBEXECD_CLOSE:
#ifdef USE_WIN32_IO
		globals->hInput=INVALID_HANDLE_VALUE;
#else
		globals->fdInput=INVALID_SOCKET;
#endif
		globals->quit=1;
		break;
	case RHBEXECD_RELEASE:
		rhbexecd_lock(globals);
		{
			struct rhbexecd_process *proc=globals->list;

			while (proc)
			{
				if (proc->id==id) break;
				proc=proc->next;
			}

			if (proc)
			{
				rhbexecd_remove(&globals->list,proc);
			}
			else
			{
				proc=globals->deceased;

				while (proc)
				{
					if (proc->id==id) break;
					proc=proc->next;
				}

				if (proc)
				{
					rhbexecd_remove(&globals->deceased,proc);
				}
			}

			if (proc)
			{
				free(proc);
			}
		}
		rhbexecd_unlock(globals);
		break;
	case RHBEXECD_KILL:
		rhbexecd_lock(globals);
		{
			struct rhbexecd_process *proc=globals->list;
			while (proc)
			{
				if (proc->id==id) break;
				proc=proc->next;
			}
			if (proc)
			{
#ifdef _WIN32
				if (proc->pinfo.dwProcessId)
				{
					/* apparently CTRL_C_EVENT can be disabled */
					BOOL b=GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT,proc->pinfo.dwProcessId);

					if (!b)
					{
						DWORD err=GetLastError();
#ifdef _M_IX86
						__asm int 3
#endif
						fprintf(stderr,"GenerateConsoleCtrlEvent(%lx) returned %ld\n",
							proc->pinfo.dwProcessId,
							err);

						b=PostThreadMessage(proc->pinfo.dwThreadId,WM_QUIT,0,0);

						if (!b)
						{
							err=GetLastError();

							fprintf(stderr,"PostThreadMessage(%lx) returned %ld\n",
								proc->pinfo.dwThreadId,
								err);
						}
					}
				}
#else
				if (proc->pid)
				{
					int err=kill(proc->pid,SIGTERM);
					if (err)
					{
						perror("kill");
					}
				}
#endif
			}
		}
		rhbexecd_unlock(globals);
		break;
	case RHBEXECD_LAUNCH:
		{
			char *title=NULL;
			char *program=NULL;
			int argc=0;
			char **argv=NULL;
			unsigned char *op=msg->data+12;
			int len=get_int32(op); op+=4;
			program=(void *)op; op+=len;
			argc=get_int32(op); op+=4;
			
			if (argc)
			{
				int i=0;
				argv=calloc(argc+1,sizeof(argv[0]));
				while (i < argc)
				{
					len=get_int32(op); op+=4;
					argv[i++]=(void *)op; op+=len;
				}
			}

			len=get_int32(op); op+=4;
			title=(void *)op; op+=len;

#ifdef _DEBUG
			fprintf(stderr,"exec(%s,%s,%s)\n",program,argv[1],title);
#endif
			RHBOPT_unused(title);

			rhbexecd_lock(globals);

			{
				struct rhbexecd_process *proc=malloc(sizeof(*proc));

				proc->globals=globals;
				proc->id=id;
				proc->next=globals->list;
				globals->list=proc;

#ifdef _WIN32
				{
					STARTUPINFO startup;
					char buf1[1024];
					char buf2[2048];
					char *p=buf2;
					DWORD dwFlags=CREATE_NEW_PROCESS_GROUP;
					int i=0;

					proc->pinfo.dwProcessId=0;
					proc->pinfo.dwThreadId=0;
					proc->pinfo.hProcess=NULL;
					proc->pinfo.hThread=NULL;

					memset(&startup,0,sizeof(startup));
					startup.cb=sizeof(startup);

/*					startup.lpTitle=title;

					if (startup.lpTitle)
					{
						dwFlags|=CREATE_NEW_CONSOLE;
					}
					else
					{
						dwFlags|=DETACHED_PROCESS;
					}
*/
					strncpy(buf1,program,sizeof(buf1));

					while (i < argc)
					{
						char *q=argv[i++];
						int k=(int)strlen(q);

						memcpy(p,q,k);
						p+=k;

						if (i < argc)
						{
							*p++=' ';
						}
					}

					*p=0;

					if (CreateProcess(program,
									buf2,
									NULL,NULL,FALSE,dwFlags,NULL,NULL,&startup,&proc->pinfo))
					{
						/* add as running process */

						BOOL b=SetEvent(globals->hReaperEvent);

						if (!b) 
						{
#ifdef _M_IX86
							__asm int 3
#endif
						}

						proc=NULL;
					}
					else
					{
						/* report as killed */

						rhbexecd_obituary(proc,GetLastError());
					}
				}
#else
				{
					proc->pid=fork();

					if (!proc->pid)
					{
						if (close(globals->fdSignalWrite)) 
						{ 
							perror("close fdSignalWrite"); 
						}

						if (close(globals->fdSignalRead))
						{
							perror("close fdSignalRead");
						}

						if (close(globals->fdOutput))
						{
							perror("close fdOutput");
						}

						if (globals->fdOutput!=globals->fdInput)
						{
							if (close(globals->fdInput))
							{
								perror("close fdInput");
							}
						}

						if (1!=dup2(globals->fdConsole,1))
						{
							perror("dup2 fdConsole stdout");
						}

						if (globals->fdConsole!=2)
						{
							if (2!=dup2(globals->fdConsole,2))
							{
								perror("dup2 fdConsole stderr");
							}

							close(globals->fdConsole);
						}

						{
							int fdNull=open("/dev/null",O_RDONLY);

							if (fdNull==-1)
							{
								perror("/dev/null");
							}
							else
							{
								if (dup2(fdNull,0) < 0)
								{
									perror("dup2 /dev/null stdin");
								}

								close(fdNull);
							}
						}

						execv(program,argv);

						perror(program);

						_exit(1);
					}
					else
					{
						if (proc->pid==-1)
						{
							perror("fork");
							rhbexecd_obituary(proc,-1);
						}
					}
				}
#endif
			}

			rhbexecd_unlock(globals);

			free(argv);
		}
		break;
	}

	free(msg);
}

int rhbexecd_run(
#ifdef USE_WIN32_IO
	HANDLE fdInput,
	HANDLE fdOutput,
#else
	SOCKET fdInput,
	SOCKET fdOutput,
#endif
#ifdef _WIN32
	HANDLE fdConsole
#else
	SOCKET fdConsole
#endif
	)
{
struct rhbexecd_globals globals;
int rc=0;
#ifdef _WIN32
DWORD tid;
BOOL b;
#else
sigset_t sigs;
static struct sigaction sa;
static int sigsinit[]={
	SIGINT,
	SIGTERM,
	SIGQUIT,
	SIGCHLD,
	SIGHUP};
/*	{
		char buf[1024];
		int k=dup(0);
		int i=sprintf(buf,"rhbexecd_run(%d,%d,%d,%d)\n",
			fdInput,fdOutput,fdConsole,k);
		write(fdConsole,buf,i);
		close(k);
	}*/
#endif

	memset(&globals,0,sizeof(globals));

#ifdef _WIN32
	globals.hMutex=CreateMutex(NULL,FALSE,NULL);
	globals.hReaperEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
#endif

#ifdef USE_WIN32_IO
	globals.hInput=fdInput;
	globals.hOutput=fdOutput;
	globals.hWriteEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	globals.hWriteThread=CreateThread(NULL,0,rhbexecd_writer,&globals,0,&tid);
#else
	{
		SOCKET fd[2]={INVALID_SOCKET,INVALID_SOCKET};

		socketpair(AF_UNIX,SOCK_STREAM,0,fd);
		globals.fdSignalRead=fd[0];
		globals.fdSignalWrite=fd[1];

/*		{
			char buf[1024];
			int i=sprintf(buf,"rhbexecd_pipes(%d,%d)\n",
				globals.fdSignalRead,
				globals.fdSignalWrite);
#ifdef _WIN32
			DWORD dw;
			WriteFile(fdConsole,buf,i,&dw,NULL);
#else
			write(fdConsole,buf,i);
#endif
		}*/
	}
	globals.fdInput=fdInput;
	globals.fdOutput=fdOutput;
#endif
	globals.fdConsole=fdConsole;

#ifdef _WIN32
	globals.hReaperThread=CreateThread(NULL,0,rhbexecd_reaper,&globals,0,&tid);

	if (!globals.hReaperThread) 
	{
#ifdef _M_IX86
		__asm int 3
#endif
	}
#else

	gGlobals=&globals;

	sigemptyset(&sigs);

	sa.sa_handler=rhbexecd_signal;
#ifdef SA_RESTART
	sa.sa_flags=SA_RESTART;
#endif

	{
		int i=sizeof(sigsinit)/sizeof(sigsinit[0]);

		while (i--)
		{
			sigaddset(&sigs,sigsinit[i]);
			sigaction(sigsinit[i],&sa,NULL);
		}

		sigprocmask(SIG_UNBLOCK,&sigs,NULL);
	}
#endif

	while ((!globals.quit)||(globals.list))
	{
#ifdef USE_WIN32_IO
		rhbexecd_read_win32(&globals);
#else
		rhbexecd_select(&globals);
#endif
		rhbexecd_lock(&globals);

		while (globals.incoming)
		{ 
			struct rhbexecd_message *last=get_last(globals.incoming);

			if (last->length) break;

			remove_msg(&globals.incoming,last);
			handle_msg(&globals,last);
		}		

		rhbexecd_unlock(&globals);
	}

/*	fprintf(stderr,"rhbexecd finished\n");
	fflush(stderr);
*/

#ifdef _WIN32
	b=SetEvent(globals.hReaperEvent);

	if (!b) 
	{
#ifdef _M_IX86
		__asm int 3
#endif
	}
#else
	sigprocmask(SIG_BLOCK,&sigs,NULL);
	gGlobals=NULL;
#endif

#ifdef USE_WIN32_IO
	b=SetEvent(globals.hWriteEvent);

	if (!b) 
	{
#ifdef _M_IX86
		__asm int 3
#endif
	}
	WaitForSingleObject(globals.hWriteThread,INFINITE);
#endif

#ifdef _WIN32
	WaitForSingleObject(globals.hReaperThread,INFINITE);
	b=CloseHandle(globals.hReaperThread);
	b=CloseHandle(globals.hReaperEvent);
#endif

#ifdef USE_WIN32_IO
	b=CloseHandle(globals.hWriteEvent);
#else
	soclose(globals.fdSignalRead);
	soclose(globals.fdSignalWrite);
#endif

#ifdef _WIN32
	b=CloseHandle(globals.hMutex);
#endif

#ifdef _DEBUG
	if (globals.deceased) 
	{
#ifdef _M_IX86
		__asm int 3
#endif
	}
	if (globals.list)
	{
#ifdef _M_IX86
		__asm int 3
#endif
	}
#endif

	return rc;
}

#ifdef BUILD_RHBEXECD
int main(int argc,char **argv)
{
	return rhbexecd_run(
#ifdef USE_WIN32_IO
		GetStdHandle(STD_INPUT_HANDLE),
		GetStdHandle(STD_OUTPUT_HANDLE),
#else
		(int)GetStdHandle(STD_INPUT_HANDLE),
		(int)GetStdHandle(STD_OUTPUT_HANDLE),
#endif
		GetStdHandle(STD_ERROR_HANDLE));
}
#endif
