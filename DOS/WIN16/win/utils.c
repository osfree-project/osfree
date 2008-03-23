/*    
	utils.c	2.20
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */


#include "windows.h"
#include "utils.h"
#include "Log.h"
#include "compat.h"
#include "ctype.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
 
static int      opnlog_fd;
static char     opnlog_fmt[64*1024];

#define LOG_INIT	0
#define LOG_NOOP	1
#define LOG_LOGGING	2

static int opnlog_state = LOG_INIT;
static int opnlog_apidepth;

static int
opnlog()
{
        char	*fn;

	if (opnlog_state == LOG_LOGGING)
		return(opnlog_fd); 

	if ( (fn = (char *)getenv("WIN_OPENLOG")) )
	{
		char *p;

		if( *fn == '-') 
			return (opnlog_fd = 1);

		/* get rid of old style mask... */
		if ((p = strchr(fn,' ')))
			*p = 0;	
		return (opnlog_fd = creat(fn, 0666));
	}

	return (opnlog_fd = 0);
}

/*
 *   logstr should be called like
 *	  logstr(logtype, format, arg1, arg2...);	
 */


void TWIN_vslogstr(unsigned long flg, va_list args);

/*
 *
 *	MessagePrintf(HWND hwnd, LPCSTR caption, UINT flags, LPCSTR fmt...)
 *
 */


int
MessagePrintf(HWND hwnd,LPCSTR caption, DWORD flags, ...)
{
	va_list args;
	char   *fmt;
	int	rc;
	
	va_start(args, flags);

	fmt = va_arg(args,char *);

	vsprintf(opnlog_fmt,fmt,args);

	rc = MessageBox(hwnd,opnlog_fmt,caption,flags);

	va_end(args);

	return rc;
}

void
TWIN_opntrap()
{
}

void
DumpMemory(char *s,char *p,int n)
{
	int i,j;
	char x;
	char   *chrptr;
	char	buffer[132];
	char	temp[80];

	if(n==0)
		return;

	printf("%s:\n",s);
	for(i=0;i<n;i++) {
		sprintf(buffer,"%x: ",(unsigned int) p);
		chrptr = p;

		for(j=0;j<16;j++) {
			x=*p++;
			x &= 0xff;
			sprintf(temp,"%2.2x ",x&0xff);
			strcat(buffer,temp);
		}		
		strcat(buffer,"   ");
		p = chrptr;
		chrptr = buffer + strlen(buffer);
		for(j=0;j<16;j++) {
			if(isalnum(*p))
				*chrptr++ = *p;
			else	*chrptr++ = '.';
			p++;
		}	
		*chrptr = 0;

		i += 16;
		strcat(buffer,"\n");
		printf("%s",buffer);
	}
}

extern int GetCompatibilityFlags(int);

void
TWIN_vslogstr (unsigned long flg, va_list args)
{
	static int sequence;
	char  local[80];
	char  buffer[80];
	static int apicount,bincount;
	int  level = 0;
	int  fd = opnlog_fd;
	char *api = 0;
	char *header = "";
	char *fmt;

	switch(opnlog_state) {
	case LOG_INIT:
		opnlog_state = LOG_NOOP;
		if ( opnlog() )
			opnlog_state = LOG_LOGGING;
		break;

	case LOG_NOOP:
		break;

	case LOG_LOGGING:
		/* point to fmt and args... */
		fmt = va_arg(args, char *);

		switch(HIWORD(flg)) {
		/* generic print logging */
		case 0:

			switch(flg) {
			case 0:
				return;

			/* these remain to be expunged...*/
			case LF_LOG:
				if(GetCompatibilityFlags(0) & WD_LOGGING) {
					fd = 2;
					header = "LOG";
				}
				break;

			case LF_CONSOLE:
				if(GetCompatibilityFlags(0) & WD_CONSOLE) {
					fd = 1;
					header = 0;
				}
				break;

			case LF_VERBOSE:
				if(GetCompatibilityFlags(0) & WD_VERBOSE) {
					fd = 1;
					header = 0;
				}
			case LF_SYSTEM:
				if(GetCompatibilityFlags(0) & WD_SYSTEM) {
					fd = 1;
					header = 0;
				}
				break;
			}
			break;

		/* special notifications */	
		case 1:
			fd = 2;
			switch(flg) {
			case LF_WARNING:
				header = "LIB WARNING";
				break;
			case LF_ERROR:
				header = "LIB ERROR  ";
				break;
			case LF_DEBUG:
				header = "";
				break;
			}
			TWIN_opntrap();	
			break;

		/* decoding of api call/return */
		case 2:
			/* logging a call, increment counter  */
			switch(flg) {
			case LF_APICALL:
				level = 1;
				api = "CALL";
				break;	

			/* returning, decrement it */
			case LF_APIRET:
				level = -1;
				api = "RET ";
				break;

			/* returning w/ failure, decrement it */
			case LF_APIFAIL:
				level = -1;
				api = "FAIL";
				break;	

			/* just logging the thing, do nothing */
			case LF_API:
				level = 0;
				api = "LOG ";
				break;	

			/* window api stubbed */
			case LF_APISTUB:
				level = 0;
				api = "STUB";
				break;
			}	
			
			if(level < 0)
				apicount--;
			sprintf(local,"API %s %3d",api,apicount);
			if(level > 0)
				apicount++;
				
			header = local;
			if(opnlog_apidepth && apicount > opnlog_apidepth)
				return;
			
			break;

		/* decoding of binary interface */
		case 3:
			switch(flg) {
			case LF_BINCALL:
				api = "BIN CALL";
				level = +1;
				break;	
			
			case LF_BINRET:
				api = "BIN RET ";
				level = -1;
				break;
			
			case LF_INTCALL:
				api = "INT CALL";
				level = 0;
				break;

			case LF_INTRET:	
				api = "INT RET ";
				level = 0;
				break;
			
			case LF_INT86:	
				api = "BIN INT ";
				level = 0;
				break;
			}
		
		
			if(level < 0)
				bincount--;
			sprintf(local,"%s %3d",api,bincount);
			if(level > 0)
				bincount++;
			header = local;
			break;

		/* decoding of window messages sent/recieved */
		case 4:
			header = "WIN MSG";
			break;
		}

		/* print out remainder of message locally */
		vsprintf(opnlog_fmt,fmt,args);

		if(header) {
			sprintf(buffer,"%5d %-12s ",sequence++,header);
			write(fd, buffer, strlen(buffer));
		}
		write(fd, opnlog_fmt, strlen(opnlog_fmt));
	}
}

/*VARARGS */
void
logstr(unsigned long flg, ...)
{
	va_list args;

	va_start(args, flg);

	TWIN_vslogstr(flg, args);

	va_end(args);
}

void	
TWIN_ResolveAddr(FARPROC p, char *bar)
{
#ifdef TWIN_HASDLADR
	extern void dladdrtosym(FARPROC, char *);
	dladdrtosym(p, bar);
#else
	sprintf(bar,"%p",p);
#endif	
}


