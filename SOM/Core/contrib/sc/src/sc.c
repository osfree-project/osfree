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

/**************************************************************
 *
 * SMEMIT default emitters
 * SMINCLUDE include dirs for IDL files
 * SMKNOWNEXTS add headers to user written emitters
 * SOMIR for IR emitter
 * SMTMP for temporary variables
 */

#include <rhbopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

typedef struct 
{
	size_t _length,_maximum;
	char *_buffer;
} _IDL_SEQUENCE_char;

static int has_spaces(const char *p,size_t len)
{
	while (len--)
	{
		char c=*p++;

		if ((c=='\t')||(c==' ')) return 1;
	}

	return 0;
}

static void add_seq(_IDL_SEQUENCE_char *seq,_IDL_SEQUENCE_char *more)
{
	if (more->_length)
	{
		size_t len=more->_length+seq->_length;

		if (len > seq->_maximum)
		{
			seq->_maximum=len+512;
			seq->_buffer=realloc(seq->_buffer,seq->_maximum);
		}

		memcpy(seq->_buffer+seq->_length,more->_buffer,more->_length);
		seq->_length+=more->_length;
	}
}

static void add_str(_IDL_SEQUENCE_char *seq,char *p)
{
	_IDL_SEQUENCE_char data={0,0,NULL};
	data._buffer=p;
	data._length=strlen(p);
	data._maximum=data._length;
	add_seq(seq,&data);
}

typedef struct item
{
	struct item *next;
	_IDL_SEQUENCE_char data;
} item;

static item *itemNew(char *data,size_t len)
{
	item *p=calloc(1,sizeof(*p));
	_IDL_SEQUENCE_char d={0,0,NULL};
	d._buffer=data;
	d._length=len;
	add_seq(&p->data,&d);
	return p;
}

static void add_many(item **h,char *p)
{
	if (!p)
	{
		fprintf(stderr,"%s:%d, NULL arg in addmany()\n",__FILE__,__LINE__);

#ifdef _M_IX86
		__asm int 3
#else
		exit(1);
#endif
		return;
	}

	while (*p)
	{
		unsigned long len=0;
		
		while (p[len])
		{
			if (p[len]==
#ifdef _WIN32
				';'
#else
				':'
#endif
				)
			{
				break;
			}
			len++;
		}

		if (len)
		{
			item *t=itemNew(p,len);

			if (*h)
			{
				item *p=*h;
				while (p->next) p=p->next;
				p->next=t;
			}
			else
			{
				*h=t;
			}

			p+=len;
		}

		if (*p) p++;
	}
}

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

static int load_somir(const char *app,const char *f)
{
	int retVal=1;
	HMODULE hMod=GetModuleHandle(NULL);
	if (hMod)
	{
		HRSRC info=FindResource(hMod,"som.ir","SC");
		if (info)
		{
			DWORD dw=SizeofResource(hMod,info);

			if (dw)
			{
				HGLOBAL res=LoadResource(hMod,info);
				if (res)
				{
					LPVOID lp=LockResource(res);

					if (lp)
					{
						HANDLE fd=CreateFile(f,
							GENERIC_WRITE,0,NULL,
							CREATE_NEW,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

						if (fd!=INVALID_HANDLE_VALUE)
						{
							DWORD dw2;

							if (WriteFile(fd,lp,dw,&dw2,NULL))
							{
								retVal=0;
							}

							CloseHandle(fd);
						}
					}
				}
			}
		}
	}

	if (retVal)
	{
		fprintf(stderr,"%s: failed to create \"%s\"\n",app,f);
	}

	return retVal;
}

int main(int argc,char **argv)
{
	item *emitters=NULL;
	item *idls=NULL;
	char *outputDir=NULL;
	item *includes=NULL;
	item *defines=NULL;
	item *modifiers=NULL;
	int i=1;
	int update=0;
	char *app=argv[0];
	static _IDL_SEQUENCE_char zero={1,1,""};

	add_many(&defines,"__SOMIDL__");

#ifdef _PLATFORM_WIN32_
	add_many(&defines,"_PLATFORM_WIN32_");
#else
#error missing _PLATFORM_WIN32_
#endif

#ifdef _PLATFORM_X11_
	add_many(&defines,"_PLATFORM_X11_");
#endif


	while (i < argc)
	{
		char *p=argv[i++];

		if ((*p=='-')
#ifdef _WIN32
			||(*p=='/')
#endif
			)
		{
			switch (p[1])
			{
			case 'p':
				add_many(&defines,"__PRIVATE__");
				break;
			case 'D':
				if (p[2])
				{
					add_many(&defines,p+2);
				}
				else
				{
					add_many(&defines,argv[i++]);
				}
				break;
			case 'm':
				if (p[2])
				{
					add_many(&modifiers,p+2);
				}
				else
				{
					add_many(&modifiers,argv[i++]);
				}
				break;
			case 'I':
				if (p[2])
				{
					add_many(&includes,p+2);
				}
				else
				{
					add_many(&includes,argv[i++]);
				}
				break;
			case 's':
			case 'e':
				if (p[2])
				{
					add_many(&emitters,p+2);
				}
				else
				{
					add_many(&emitters,argv[i++]);
				}
				break;
			case 'd':
			case 'o':
				if (outputDir)
				{
					fprintf(stderr,"%s: output dir already set\n",app);
					return 1;
				}
				if (p[2])
				{
					outputDir=p+2;
				}
				else
				{
					outputDir=argv[i++];
				}
				break;
			case 'u':
				update=1;
				break;
			default:
				fprintf(stderr,"%s: unknown switch - \"%s\"\n",app,p);
				return 1;
			}
		}
		else
		{
			item *t=itemNew(p,strlen(p));
			if (idls)
			{
				item *p=idls;
				while (p->next) p=p->next;
				p->next=t;
			}
			else
			{
				idls=t;
			}
		}
	}

	if (!emitters)
	{
		char *p=getenv("SMEMIT");

		if (!p)
		{
			p="h;ih";
		}

		add_many(&emitters,p);
	}

	{
		char *p=getenv("SMINCLUDE");

		if (p)
		{
			add_many(&includes,p);
		}
	}

	if (idls)
	{
		item *idl=idls;

		while (idl)
		{
			item *emitter=emitters;

			while (emitter)
			{
				_IDL_SEQUENCE_char somcpp={0,0,NULL};
				_IDL_SEQUENCE_char somipc={0,0,NULL};
				_IDL_SEQUENCE_char idlname={0,0,NULL};
				char buf[512];
				long len=GetModuleFileName(NULL,buf,sizeof(buf));
				item *t;
				size_t ul=0,ul2=0;
				int appPathSpaces=has_spaces(buf,len);

				/* get to start of file name */

				while (len--)
				{
					if (
#ifdef _WIN32
						(buf[len]=='/')||
#endif
						(buf[len]=='\\'))
					{
						len++;
						break;
					}
				}

				if (appPathSpaces)
				{
					add_str(&somipc,"\"");
					add_str(&somcpp,"\"");
				}

				strncpy(buf+len,"somipc.exe",sizeof(buf)-len);

				add_str(&somipc,buf);

				strncpy(buf+len,"somcpp.exe",sizeof(buf)-len);

				add_str(&somcpp,buf);

				if (appPathSpaces)
				{
					add_str(&somipc,"\"");
					add_str(&somcpp,"\"");
				}

				ul=idl->data._length;

				while (ul)
				{
					ul--;

					if ((idl->data._buffer[ul]=='/')
						||
						(idl->data._buffer[ul]=='\\'))
					{
						ul++;
						break;
					}
				}

				while ((ul+ul2) < idl->data._length)
				{
					if (idl->data._buffer[ul+ul2]=='.')
					{
						break;
					}

					ul2++;
				}

				if (ul2)
				{
					_IDL_SEQUENCE_char d={0,0,NULL};
					d._length=ul2;
					d._buffer=idl->data._buffer+ul;

					add_seq(&idlname,&d);
				}

				t=defines;

				while (t)
				{
					add_str(&somcpp," -D");
					add_seq(&somcpp,&t->data);

					t=t->next;
				}

				t=includes;

				while (t)
				{
					int x=has_spaces(t->data._buffer,t->data._length);
					add_str(&somcpp," ");
					if (x) add_str(&somcpp,"\"");
					add_str(&somcpp,"-I");
					add_seq(&somcpp,&t->data);
					if (x) add_str(&somcpp,"\"");

					t=t->next;
				}

				{
					add_str(&somipc," -s");
					add_seq(&somipc,&emitter->data);
				}

				add_str(&somipc," -o ");

				if ((emitter->data._length==2)&&(!memcmp(emitter->data._buffer,"ir",2)))
				{
					char *ir=getenv("SOMIR");
					char *filename;
					long attr;

					if (!ir) 
					{
						ir="som.ir";
						fprintf(stderr,"%s: warning, SOMIR not set, defaulting to \"%s\"\n",app,ir);
					}

					filename=ir+strlen(ir);

					while (filename > ir)
					{
						filename--;
						if (filename[0]==';') { filename++; break; }
					}

/*					fprintf(stderr,"%s: ir output file is \"%s\"\n",app,filename);*/

					attr=GetFileAttributes(filename);

					if (attr < 0)
					{
						fprintf(stderr,"%s,\"%s\" does not exist\n",app,filename);

						if (load_somir(app,filename))
						{
							return 1;
						}
					}

					add_str(&somipc,filename);
					add_str(&somipc," ");
				}
				else
				{
					if (outputDir)
					{
						add_str(&somipc,outputDir);
						add_str(&somipc,"\\");
					}

					add_seq(&somipc,&idlname);
					add_str(&somipc,".");
					add_seq(&somipc,&emitter->data);
				}

				t=modifiers;

				while (t)
				{
					add_str(&somipc," -m ");
					add_seq(&somipc,&t->data);

					t=t->next;
				}

				add_seq(&somcpp,&zero);
				add_seq(&somipc,&zero);

#if 1
				printf("somcpp: %s\n",somcpp._buffer);
				printf("somipc: %s\n",somipc._buffer);
#endif

				{
					STARTUPINFO cpp_startup,somipc_startup;
					PROCESS_INFORMATION cpp_pinfo={0,0,0,0},somipc_pinfo={0,0,0,0};
					BOOL b=TRUE;
					HANDLE hSource=INVALID_HANDLE_VALUE;
					DWORD err=0;
					DWORD cppExitCode=1;

					memset(&cpp_startup,0,sizeof(cpp_startup));
					memset(&somipc_startup,0,sizeof(somipc_startup));

					cpp_startup.cb=sizeof(cpp_startup);
					cpp_startup.dwFlags=STARTF_USESTDHANDLES;
					cpp_startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
					cpp_startup.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
					cpp_startup.hStdError=GetStdHandle(STD_ERROR_HANDLE);

					somipc_startup.cb=sizeof(somipc_startup);
					somipc_startup.dwFlags=STARTF_USESTDHANDLES;
					somipc_startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
					somipc_startup.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
					somipc_startup.hStdError=GetStdHandle(STD_ERROR_HANDLE);

					b=CreatePipe(&cpp_startup.hStdInput,
						         &hSource,
								 NULL,
								 4096);

					b=CreatePipe(&somipc_startup.hStdInput,
						         &cpp_startup.hStdOutput,
								 NULL,
								 4096);

					b=shareable(&cpp_startup.hStdInput);
					b=shareable(&cpp_startup.hStdOutput);

					b=CreateProcess(NULL,somcpp._buffer,NULL,NULL,TRUE,0,NULL,NULL,
								&cpp_startup,&cpp_pinfo);

					if (!b)
					{
						err=GetLastError();
					}

					CloseHandle(cpp_startup.hStdInput);
					CloseHandle(cpp_startup.hStdOutput);

					if (!b)
					{
						CloseHandle(hSource);
						CloseHandle(somipc_startup.hStdInput);

						fprintf(stderr,"%s: exec(%s) failed\n",app,somcpp._buffer);
						return 1;
					}

					b=shareable(&somipc_startup.hStdInput);

					b=CreateProcess(NULL,somipc._buffer,NULL,NULL,TRUE,0,NULL,NULL,
						&somipc_startup,&somipc_pinfo);

					if (!b)
					{
						err=GetLastError();
					}

					CloseHandle(somipc_startup.hStdInput);

					if (b)
					{
						_IDL_SEQUENCE_char source={0,0,NULL};
						DWORD ul=0;

						add_str(&source,"#include \"");
						add_seq(&source,&idl->data);
						add_str(&source,"\"\r\n");

						while (ul < source._length)
						{
							if (source._buffer[ul]=='\\')
							{
								source._buffer[ul]='/';
							}
							ul++;
						}

/*						WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),
									source._buffer,source._length,&ul,NULL);*/
						WriteFile(hSource,source._buffer,(DWORD)source._length,&ul,NULL);
					}

					CloseHandle(hSource);

					WaitForSingleObject(cpp_pinfo.hProcess,INFINITE);
					GetExitCodeProcess(cpp_pinfo.hProcess,&cppExitCode);
					CloseHandle(cpp_pinfo.hProcess);
					CloseHandle(cpp_pinfo.hThread);

					if (b)
					{
						DWORD somipcExitCode=1;
						WaitForSingleObject(somipc_pinfo.hProcess,INFINITE);
						GetExitCodeProcess(somipc_pinfo.hProcess,&somipcExitCode);
						CloseHandle(somipc_pinfo.hProcess);
						CloseHandle(somipc_pinfo.hThread);

						if (somipcExitCode || cppExitCode)
						{
							return cppExitCode ? cppExitCode : somipcExitCode;
						}
					}
					else
					{
						fprintf(stderr,"%s: exec(%s) failed\n",app,somipc._buffer);

						return 1;
					}
				}

				emitter=emitter->next;
			}

			idl=idl->next;
		}
	}

	return 0;
}
