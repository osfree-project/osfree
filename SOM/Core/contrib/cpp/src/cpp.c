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
 * use "cl.exe /E" as the preprocessor
 * if no files on command line then create a stdin temp file
 * filter output from "cl.exe" to catch that temp file name and
 * replace with "<stdin>"
 * 2ndly, replace the "\\" in those strings with "/"???
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef struct 
{
	size_t _length,_maximum;
	char *_buffer;
} _IDL_SEQUENCE_char;

static char tempFile[256],tempFileEscaped[512];

static HANDLE hCreateTemp(void)
{
	long x=GetCurrentProcessId();
	HANDLE h=INVALID_HANDLE_VALUE;
	char *p=getenv("TEMP");
	if (!p) p=getenv("TMP");
	if (!p) p="/tmp";

	while (h==INVALID_HANDLE_VALUE)
	{
		_snprintf(tempFile,sizeof(tempFile),
				"%s\\~CPP%ld.tmp",p,x);

		h=CreateFile(tempFile,GENERIC_WRITE,0,NULL,
				CREATE_NEW,
				0 /*FILE_ATTRIBUTE_HIDDEN*/,
				NULL);

		x++;
	}

	{
		char *p=tempFileEscaped;
		const char *q=tempFile;

		while (*q)
		{
			char c=*q++;

			switch (c)
			{
			case '\"':
			case '\'':
			case '\\':
				*p++='\\';
				break;
			}

			*p++=c;
		}
	}

	return h;
}

static int has_spaces(const char *p,size_t len)
{
	while (len--)
	{
		char c=*p++;

		if ((c=='\t')||(c==' ')) return 1;
	}

	return 0;
}

static void add_chars(_IDL_SEQUENCE_char *seq,const char *more_buffer,size_t more_length)
{
	if (more_length)
	{
		size_t len=more_length+seq->_length;

		if (len > seq->_maximum)
		{
			seq->_maximum=len+512;
			seq->_buffer=realloc(seq->_buffer,seq->_maximum);
		}

		memcpy(seq->_buffer+seq->_length,more_buffer,more_length);
		seq->_length+=more_length;
	}
}

static void add_seq(_IDL_SEQUENCE_char *seq,_IDL_SEQUENCE_char *more)
{
	add_chars(seq,more->_buffer,more->_length);
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

static void process_hash(HANDLE hOutput,_IDL_SEQUENCE_char *line)
{
	DWORD dw;

	if (line->_length>1)
	{
		char *p=line->_buffer;
		
		if ((!memcmp(p,"# ",2))||
			(!memcmp(p,"#line ",6)))
		{
			unsigned long i=0;
			int do_replace=0;

			/* get to start of filename part */

			while (i < line->_length)
			{
				if (p[i++]=='\"')
				{
					break;
				}
			}

			if (i < line->_length)
			{
				/* compare filename with expected ... */

				size_t k=strlen(tempFile);
				size_t j=strlen(tempFileEscaped);

				if ((i+k) < line->_length)
				{
					if (p[i+k]=='\"')
					{
						if (!memicmp(p+i,tempFile,k))
						{
							do_replace=1;
						}
					}
				}

				if ((i+j) < line->_length)
				{
					if (p[i+j]=='\"')
					{
						if (!memicmp(p+i,tempFileEscaped,j))
						{
							do_replace=1;
						}
					}
				}
			}

			if (do_replace)
			{
				line->_length=i-1; /* truncate to before quote */

				add_str(line,"\"<stdin>\"");
			}
		}
	}
	
	if (line->_length)
	{
		size_t k=line->_length;

		while (k--)
		{
			if (line->_buffer[k]=='~') 
			{
#ifdef _M_IX86
				__asm int 3
#endif
				break;
			}
		}

		WriteFile(hOutput,line->_buffer,(DWORD)(line->_length),&dw,NULL);
	}
}

static void check_buffer(const char *p,int len)
{
	while (len--)
	{
		char c=*p++;

		if ((c<9)||(c>127))
		{
#ifdef _M_IX86
			__asm int 3
#endif
		}
	}
}

static void do_output(HANDLE hOutput,const char *p,int len)
{
static _IDL_SEQUENCE_char hash_line;
int i=0;
static char escape,quote;

	check_buffer(p,len);

	while (i < len)
	{
		char c=p[i];

		if ((c<9)||(c>127))
		{
#ifdef _M_IX86
			__asm int 3
#endif
		}

		if (hash_line._length)
		{
			if ((c=='\r')||(c=='\n'))
			{
#ifdef _M_IX86
				if (escape) __asm int 3
				if (quote) __asm int 3
#endif

				add_chars(&hash_line,p,i);
				len-=i;
				p+=i;
				process_hash(hOutput,&hash_line);
				hash_line._length=0;
				i=0;
			}
			else
			{
				i++;
			}
		}
		else
		{
			if ((!escape) && (!quote) && (c=='#'))
			{
				if (i)
				{
					DWORD dw;

					WriteFile(hOutput,p,i,&dw,NULL);

/*					{
						int k=i;
						while (k--)
						{
							if ((p[k]=='#')||(p[k]=='~'))
							{
								__asm int 3
								break;
							}
						}
					}
*/
					len-=i;
					p+=i;
					i=0;
				}

				add_chars(&hash_line,p,1);

				len--;
				p++;
			}
			else
			{
				if (escape) 
				{
					escape=0;
				}
				else
				{
					if (quote==c)
					{
						quote=0;
					}
					else
					{
						switch (c)
						{
						case '\"':
						case '\'':
							quote=c;
							break;
						case '\\':
							escape=c;
							break;
						}
					}
				}

				i++;
			}
		}
	}

	if (i)
	{
		if (hash_line._length)
		{
			add_chars(&hash_line,p,i);
		}
		else
		{
			DWORD dw;
			WriteFile(hOutput,p,i,&dw,NULL);

#if 0
			{
				int k=i;
				while (k--)
				{
					if ((p[k]=='#')||(p[k]=='~'))
					{
						if (!memcmp(p+k,"#line",5))
						{
#ifdef _M_IX86
							__asm int 3
#endif
						}
						break;
					}
				}
			}
#endif
		}
	}
}

static void myexit(void)
{
	if (tempFile[0])
	{
		DeleteFile(tempFile);
	}
}

int main(int argc,char **argv)
{
	HANDLE hTemp=hCreateTemp();
	item *src=NULL;
	char *outputDir=NULL;
	item *includes=NULL;
	item *defines=NULL;
	int i=1;
	int update=0;
	char *app=argv[0];
	static _IDL_SEQUENCE_char zero={1,1,""};
	_IDL_SEQUENCE_char cmdline={0,0,NULL};
	char *cc=getenv("CC");
	int rc=1;

	atexit(myexit);

	if (!cc) 
	{
		cc="cl.exe";
	}

	while (i < argc)
	{
		char *p=argv[i++];

		if ((*p=='-')||(*p=='/'))
		{
			switch (p[1])
			{
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
			default:
				fprintf(stderr,"%s: unknown switch - \"%s\"\n",app,p);
				return 1;
			}
		}
		else
		{

			item *t=itemNew(p,strlen(p));

			if (src)
			{
				item *p=src;
				while (p->next) p=p->next;
				p->next=t;
			}
			else
			{
				src=t;
			}
		}
	}

	add_str(&cmdline,cc);
	add_str(&cmdline," /E");

	if (defines)
	{
		item *t=defines;
		while (t)
		{
			add_str(&cmdline," /D");
			add_seq(&cmdline,&t->data);
			t=t->next;
		}
	}

	if (includes)
	{
		item *t=includes;
		while (t)
		{
			int x=has_spaces(t->data._buffer,t->data._length);

			add_str(&cmdline," ");

			if (x) add_str(&cmdline,"\"");
			add_str(&cmdline,"/I");
			add_seq(&cmdline,&t->data);
			if (x) add_str(&cmdline,"\"");

			t=t->next;
		}
	}

	add_str(&cmdline," /I.");

	if (src)
	{
		if (hTemp!=INVALID_HANDLE_VALUE)
		{
			_IDL_SEQUENCE_char data={0,0,NULL};
			DWORD dw;
			item *t=src;

			while (t)
			{
				add_str(&data,"#include \"");
				add_seq(&data,&t->data);
				add_str(&data,"\"\n");
				t=t->next;
			}

			WriteFile(hTemp,data._buffer,(DWORD)(data._length),&dw,NULL);
		}
		else
		{
			item *t=src;
			while (t)
			{
				add_str(&cmdline," ");
				add_seq(&cmdline,&t->data);
				t=t->next;
			}
		}
	}
	else
	{
		char buf[256];
		DWORD dw;
		HANDLE hStdInput=GetStdHandle(STD_INPUT_HANDLE);
		while(ReadFile(hStdInput,buf,sizeof(buf),&dw,NULL))
		{
			if (!dw) break;
			WriteFile(hTemp,buf,dw,&dw,NULL);
		}
	}

	if (hTemp!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hTemp);

		hTemp=INVALID_HANDLE_VALUE;

#if 0
		hTemp=CreateFile(tempFile,
				GENERIC_READ,
				0 /*FILE_SHARE_READ*/,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_DELETE_ON_CLOSE,
				NULL);
#endif

		{
			add_str(&cmdline," ");
			add_str(&cmdline,tempFile);
		}
	}

	{
		DWORD dw=(DWORD)cmdline._length;
		BOOL b;
		STARTUPINFO startup;
		PROCESS_INFORMATION pinfo={0,0,0,0};
		HANDLE myinput=INVALID_HANDLE_VALUE;
		HANDLE hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);

		WriteFile(GetStdHandle(STD_ERROR_HANDLE),cmdline._buffer,dw,&dw,NULL);
		WriteFile(GetStdHandle(STD_ERROR_HANDLE),"\r\n",2,&dw,NULL);

		add_seq(&cmdline,&zero);

		memset(&startup,0,sizeof(startup));

		startup.cb=sizeof(startup);
		startup.dwFlags=STARTF_USESTDHANDLES;
		startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
		startup.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
		startup.hStdError=GetStdHandle(STD_ERROR_HANDLE);

		b=CreatePipe(&myinput,
					 &startup.hStdOutput,
					 NULL,
					 4096);

		if (b)
		{
			b=shareable(&startup.hStdOutput);
		}

		if (b)
		{
			b=CreateProcess(NULL,cmdline._buffer,NULL,NULL,TRUE,0,NULL,NULL,
					&startup,&pinfo);

			CloseHandle(startup.hStdOutput);
		}

		if (b)
		{
			DWORD exitCode=1;
			char buf[256];
			DWORD dw;

			while (ReadFile(myinput,buf,sizeof(buf),&dw,NULL))
			{
				if (dw)
				{
					do_output(hStdOutput,buf,dw);
				}
				else
				{
					break;
				}
			}


			WaitForSingleObject(pinfo.hProcess,INFINITE);
			if (GetExitCodeProcess(pinfo.hProcess,&exitCode))
			{
				rc=exitCode;
			}
			CloseHandle(pinfo.hProcess);
			CloseHandle(pinfo.hThread);
		}
	}

	if (hTemp!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hTemp);
	}

	return rc;
}
