/*    
	mac.c	1.10
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
 
#include <A4Stuff.h>
#include <DropInCompilerLinker.h>

#include <Strings.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rc.h"
#include "buffer.h"
#include "define.h"

int extra_lines(void);

/* Compile-time defines */

/* LOG: Enable run-time logging
 * Log messages will go to <basename>.log
 * Due to what I percieve as a bug in the CodeWarrior C library, the .log files all end up
 * in the CodeWarrior directory. */
#undef rLOG

#ifdef rLOG
FILE *fLog;
#endif

/* VERBOSEWARN: Make VPRINT messages appear as warnings */
#undef VERBOSEWARN

/* Functions defined in mac.c */
short DoCompile(void);
void close_files(void);
buffp load_from_refnum(const short refnum, char *filename);
unsigned char *c2pstrcpy(register unsigned char *dst, register char *src);
char *p2cstrcpy(register char *dst, register unsigned char *src);

static int had_error;
static int total_lines;
static int lines_written;
static short exitVal; /* If we broke, here's why */
static CompilerLinkerParameterBlockPtr cpb; /* The wrong way to do it, but only we need it... */
static jmp_buf jmpbuf;

struct binflagvals defbinflags = { 1, little };

int build_app_module = 0;

extern char in_name[];
extern char out_name[];
extern char cpp_module_name[];

static short outRefNum, defRefNum, expRefNum, cppRefNum = 0;
static FSSpec specOut, specDef, specExp, specCpp;

/* For some reason, using just FSWrite gives _horrendous_ performance (~11 minutes to compile
 * RscData.rc, vs. 7.9 seconds on Linux), so we'll try using buffered I/O... */
#define OUT_BUFFER_SIZE 8192
char out_buffer[OUT_BUFFER_SIZE];
unsigned out_buffer_len;
void flush_out_buffer(void);
void buffered_out(char *);
char *strdup(const char *s);

struct macfileinfo
{
	FSSpec filespec;
} *currentfile;

void exitfunc(void)
{
}

pascal short main(CompilerLinkerParameterBlockPtr lcpb)
{
	short result;
	int jmpResult;
	
	/* 68K initialization: set up global world */
	EnterCodeResource();
	
#ifdef rLOG
	fLog = NULL;
#endif
	outRefNum = 0;
	cpb = lcpb;
	result = noErr;
	if (jmpResult = setjmp(jmpbuf))
		result = jmpResult;
	else
	{
		switch(cpb->request)
		{
		case reqInitCompiler:
			break;
		
		case reqTermCompiler:
		break;
		
		case reqCompile:
			result = DoCompile();
			break;
		
		default:
			result = paramErr;
			break;
		}
	}
	
	if (cpb->request == reqCompile)
	{
		if ((jmpResult = setjmp(jmpbuf)) == 0)
			close_files();
	}
	
	/* Take down 68K global world */
	ExitCodeResource();
	
	if (cpb->request == reqCompile)
		return (result == noErr) && !had_error;
	else
		return result;
}

short DoCompile(void)
{
#ifdef rLOG
	char buffer[255];
#endif

	/* Reset global variables */
	had_error = 0;
	total_lines = 0;
	lines_written = 0;
	exitVal = 0;

	/* Define some useful/necessary constants */
	define_define("macintosh", "1");
	define_define("_MAC", NULL);
	define_define("RC_INVOKED", NULL);

	/* Get the input file name from the CodeWarrior information */
	p2cstrcpy(in_name, cpb->sourcefile.name);
	
	/* No output name (yet) -- this gets set in open_files() */
	strcpy(out_name, "");
		
#ifdef rLOG
	/* Open the log file */
	strcpy(buffer, in_name);
	*strrchr(buffer, '.') = '\0';
	strcat(buffer, ".log");
	fLog = fopen(buffer, "w");
#endif
	icompile();
	
	return noErr;
}

void close_files(void)
{
	OSErr err;
	unsigned long now;
	long fPos;
	Str255 str;

	/* Close the output file */
	if (outRefNum)
	{
		/* Flush the internal buffer */
		flush_out_buffer();
		
		/* Set the end-of-file to "here" (wherever that is) */
		err = GetFPos(outRefNum, &fPos);
		if (err != noErr)
			CERROR(("Couldn't set EOF on %s [%d]", out_name, err));
			
		err = SetEOF(outRefNum, fPos);
		if (err != noErr)
			CERROR(("Couldn't set EOF on %s [%d]", out_name, err));

		/* Now close the file */
		err = FSClose(outRefNum);
		if (err != noErr)
			CERROR(("Couldn't close %s [%d]", out_name, err));
			
		/* Change the modification date of the output file to make CodeWarrior compile it */
		if (!had_error)
		{
			GetDateTime(&now);
			err = CWSetModDate(cpb, &specOut, now);
			if (err != noErr)
				CERROR(("Couldn't set modification date on %s [%d]", out_name, err));
		}
	}
	
	/* The Module Definition file (.def) has already been closed */
	/* so don't do it again here */
		
	if (expRefNum)
	{
		err = FSClose(expRefNum);
		if (err != noErr)
			CERROR(("Couldn't close %s [%d]", specExp.name, err));
		err = CWSetModDate(cpb, &specExp, now);
		if (err != noErr)
			CERROR(("Couldn't set modification date on %s [%d]", specExp.name, err));
	}

	/* Flush the file's volume */
	str[0] = 0; /* Empty pascal string */
	err = FlushVol(str, specOut.vRefNum);
	if (err != noErr)
		CERROR(("Couldn't flush drive [%d]", err));
			
#ifdef rLOG
	/* Close the log file */
	if (fLog)
		fclose(fLog);
#endif
}

void dolog(char *str, ...)
{
	va_list ap;
	va_start(ap, str);
#ifdef rLOG
	vfprintf(fLog, str, ap);
	fprintf(fLog, "\n");
#endif
	va_end(ap);
}

void dovprint(char *str, ...)
{
#ifdef VERBOSEWARN
	char message[256];
	CompilerErrorRef cer;
	OSErr err;
#endif
	va_list ap;
	va_start(ap, str);
#ifdef VERBOSEWARN
	vsprintf(message, str, ap);
#endif
	va_end(ap);
#ifdef VERBOSEWARN
	cer.errorfile = ((struct macfileinfo *)get_ext())->filespec;
	cer.linenumber = current_line();
	cer.offset = 0; /* Byte offset into file -- not supported */
	cer.length = 0; /* WHAT exactly caused the error? */
	cer.sync[0] = '\0'; /* No synchronization information */
	cer.synclen = 0;
	cer.syncoffset = 0;
	cer.errorlength = 0;
	cer.erroroffset = 0;
	cer.warning = true;
	
	err = CWErrorRefMessage(cpb, &cer, "", message, kPreCompilerMessage);
	if (err != noErr)
		longjmp(jmpbuf, err);
#endif
}

void doerror(char *str, ...)
{
	char message[256];
	CompilerErrorRef cer;
	OSErr err;
	va_list ap;
	va_start(ap, str);
	vsprintf(message, str, ap);
	va_end(ap);
	
	had_error = 1;
	
	cer.errorfile = ((struct macfileinfo *)get_ext())->filespec;
	cer.linenumber = current_line();
	cer.offset = 0; /* Byte offset into file -- not supported */
	cer.length = 0; /* WHAT exactly caused the error? */
	cer.sync[0] = '\0'; /* No synchronization information */
	cer.synclen = 0;
	cer.syncoffset = 0;
	cer.errorlength = 0;
	cer.erroroffset = 0;
	cer.warning = false;
	
	err = CWErrorRefMessage(cpb, &cer, "", message, kPreCompilerMessage);
	if (err != noErr)
		longjmp(jmpbuf, err);
}

void dofatal(char *str, ...)
{
	char message[256];
	CompilerErrorRef cer;
	va_list ap;
	va_start(ap, str);
	vsprintf(message, str, ap);
	va_end(ap);
	
	had_error = 1;
	
	cer.errorfile = ((struct macfileinfo *)get_ext())->filespec;
	cer.linenumber = current_line();
	cer.offset = 0; /* Byte offset into file -- not supported */
	cer.length = 0; /* WHAT exactly caused the error? */
	cer.sync[0] = '\0'; /* No synchronization information */
	cer.synclen = 0;
	cer.syncoffset = 0;
	cer.errorlength = 0;
	cer.erroroffset = 0;
	cer.warning = false;
	
	CWErrorRefMessage(cpb, &cer, "", message, kPreCompilerMessage);

	longjmp(jmpbuf, exitVal ? exitVal : paramErr); /* Essentially equivalent to an exit() */
}

void dowarning(char *str, ...)
{
	char message[256];
	CompilerErrorRef cer;
	OSErr err;
	va_list ap;
	va_start(ap, str);
	vsprintf(message, str, ap);
	va_end(ap);
	
	cer.errorfile = ((struct macfileinfo *)get_ext())->filespec;
	cer.linenumber = current_line();
	cer.offset = 0; /* Byte offset into file -- not supported */
	cer.length = 0; /* WHAT exactly caused the error? */
	cer.sync[0] = '\0'; /* No synchronization information */
	cer.synclen = 0;
	cer.syncoffset = 0;
	cer.errorlength = 0;
	cer.erroroffset = 0;
	cer.warning = true;
	
	err = CWErrorRefMessage(cpb, &cer, "", message, kPreCompilerMessage);
	if (err != noErr)
		longjmp(jmpbuf, err);
}

void dooutput(char *str, ...)
{
	char output[1024]; /* 1K should be more than enough */
	char message[256]; /* To send to CodeWarrior */
	char *p;
	OSErr err;
	va_list ap;
	va_start(ap, str);
	vsprintf(output, str, ap);
	va_end(ap);
	
	buffered_out(output);
		
	p = output;
	while (*p)
	{
		if ((*p == '\r') || (*p == '\n'))
			if (++lines_written % 50 == 0)
			{
				sprintf(message, "Writing \"%s\"", out_name);
				sprintf(output, "Line: %d", lines_written);
				err = CWShowStatus(cpb, message, output);
				if (err != noErr)
					longjmp(jmpbuf, err);
			}
		p++;
	}	
}

void doeoutput(char *str, ...)
{
	char buf[255];
	long count;

	va_list ap;
	va_start(ap, str);
	if (expRefNum)
		count = vsprintf(buf, str, ap);
		FSWrite(expRefNum, &count, buf);
	va_end(ap);
}

void docppoutput(char *str, ...)
{
	char buf[255];
	long count;

  va_list ap;
  va_start(ap, str);
  if (cppRefNum)
  	count = vsprintf(buf, str, ap);
  	FSWrite(cppRefNum, &count, buf);
  va_end(ap);
}

void open_modulefiles(void)
{
}

char *get_string(char *str, int len)
{
	static buffp buffer = 0;
	char * ptr = 0;
	static char *newPtr = 0;
	
	if(!buffer && defRefNum) {
		buffer = load_from_refnum(defRefNum, "");
		newPtr = buffer->buffer;
	}
	ptr = strtok(newPtr, "\n\r");
	newPtr = ptr + strlen(ptr) + 1;
	strncpy(str, ptr, 256);
	
	return ptr;
}

void flush_out_buffer(void)
{
	OSErr err;
	long count = out_buffer_len;
	err = FSWrite(outRefNum, &count, out_buffer);
	if (err != noErr)
		FATAL(("Couldn't write to %s [%d]", out_name, err));
		
	out_buffer_len = 0;
}

void buffered_out(char *str)
{
	unsigned len = strlen(str);
	if (out_buffer_len + len + 1 > OUT_BUFFER_SIZE)
		flush_out_buffer();
	strcpy(out_buffer + out_buffer_len, str);
	out_buffer_len += len;
}

/* Nabbed from TWIN's StringUtils.c */
/********************************************************************
* c2pstrcpy	-	Pascal string utilities
*
*  copy c string 'src' to pascal string 'dst'
*  Returns the pascal strings pointer.
********************************************************************/
unsigned char *c2pstrcpy(register unsigned char *dst, register char *src)
{
register int i = 0;

	while((dst[i+1] = src[i]) != 0)
		i++;

	dst[0] = i;			/* Set pascal string length*/
	
	return(dst);
}

/********************************************************************
* pstrcpy	-	Pascal string utilities
*
*  copy pascal string 'src' to c string 'dst'
*  Returns the c strings pointer.
********************************************************************/
char *p2cstrcpy(register char *dst, register unsigned char *src)
{
register int i;

	for(i = 0; i < *src; i++)
		dst[i] = src[i+1];

	dst[i] = '\0';			/* Terminate c string */
	
	return(dst);
}

/********************************************************************
* c2pstrcpypath	-	Pascal string utilities
*
*  copy c string 'src' to pascal string 'dst', changing path delimiters
*  to change a valid Unix/DOS path into a (presumably) valid Mac path
*
*  The net effect of this (to satisfy CodeWarrior) is to throw away
*  everything but the filename.  
*  Returns the pascal strings pointer.
********************************************************************/
unsigned char *c2pstrcpypath(register unsigned char *dst, register char *src)
{
	register int i = -1, o = 1;

	while (src[++i])
		if ((src[i] == '/') || (src[i] == '\\')) /* Path separator */
			o = 1; /* Throw away separator, go back to start of output */
		else /* Another character */
			dst[o++] = src[i];

	dst[0] = o - 1;			/* Set pascal string length*/
	
	return(dst);
}

char *load_file(char *filename, int lookHere)
{
	return buff_ind(buff_load(filename, lookHere));
}

buffp load_from_fsspec(const FSSpec *spec, char *filename)
{
	short refnum;

	exitVal = FSpOpenDF(spec, fsRdPerm, &refnum);
	if (exitVal != noErr)
		FATAL(("Couldn't open include file %s", filename));
	
	return(load_from_refnum(refnum, filename));	
}

buffp load_from_refnum(const short refnum, char *filename)
{
	buffp b;
	long size;

	exitVal = GetEOF(refnum, &size);
	if (exitVal != noErr)
	{
		FSClose(refnum);
		FATAL(("Couldn't get EOF on file %s", filename));
	}
	
	b = buff_alloc(size + 1);
	exitVal = FSRead(refnum, &size, b->buffer);
	if (exitVal != noErr)
	{
		FSClose(refnum);
		FATAL(("Couldn't read %d bytes from %s", size, filename));
	}
	*((char *)(b->buffer) + size) = '\0';
	
	exitVal = FSClose(refnum);
	if (exitVal != noErr)
		FATAL(("Couldn't close %s", filename));

	b->len = b->alloc;
	return b;
}

buffp buff_load(char *filename, int lookHere)
{
	buffp b;
	CWFileInfo fileInfo;
	OSErr err;
	short refnum;
	unsigned size;
	Str255 newFile;
	
	fileInfo.fullsearch = lookHere;
	fileInfo.isdependency = true;
	fileInfo.isdependentoffile = kCurrentCompiledFile;
	
	c2pstrcpypath(newFile, filename);
	
// Try plain file name first
	if ((exitVal = CWFindAndLoadFile(cpb, newFile, &fileInfo)) != noErr) {
// If that failed, try converting from a Unix style path name
		if ((exitVal = CWFindAndLoadFile(cpb, c2pstr(filename), &fileInfo)) != noErr)
			FATAL(("Couldn't open include file %s", filename));
	}

	currentfile = (struct macfileinfo *)malloc(sizeof(struct macfileinfo));
	currentfile->filespec = fileInfo.filespec;
	
	if (fileInfo.sourcehandle)
	{
		b = buff_alloc(fileInfo.sourcehandlesize);
		/* Copy from fileInfo.sourcehandle to b->buffer */
		memcpy(b->buffer, *fileInfo.sourcehandle, fileInfo.sourcehandlesize);
		b->len = b->alloc;
		return b;
	}
	else
		return load_from_fsspec(&fileInfo.filespec, filename);
}


void open_files(void)
{
	char *temp;
	OSErr err, err1;
	char filename[256];
	char *p;

	
	/* First things first: make sure we know what file we're in */
	currentfile = (struct macfileinfo *)get_mem(sizeof(struct macfileinfo));
	currentfile->filespec = cpb->sourcefile;
	
	/* The input data can come from memory _or_ from the disk -- figure out which one it is */
	if (cpb->sourcehandle)
	{
		HLock(cpb->sourcehandle);
		include_init_string(*cpb->sourcehandle);
		HUnlock(cpb->sourcehandle);
	}
	else
		include_init_string((char *)buff_ind(load_from_fsspec(&cpb->sourcefile, in_name)));
	
/*
*	Open Resource Output File
*	Must be named <basename>.res.c, where <basename> is
*	the base name of the resource file.  The resource
*	output file must be in the project.
*	Failure to find the resource output file in the project
*	is a FATAL error.
*/
	strcpy(out_name, in_name);
	temp = strrchr(out_name, '.');
	if (temp)
		strcpy(temp, ".res.c");
	else
		strcat(out_name, ".res.c");
	
	if(!SearchProjectForFile(cpb, &specOut, out_name))
		FATAL(("Couldn't find resource output file (%s) in project", out_name));
		
	err = FSpOpenDF(&specOut, fsWrPerm, &outRefNum);
	if (err == fnfErr) /* Couldn't find output file, try to create it */
	{
		err = FSpCreate(&specOut, 'CWIE', 'TEXT', -1 /* smSystemScript */);
		if (err != noErr)
			FATAL(("Couldn't create %s [%d]", out_name, err));
		err = FSpOpenDF(&specOut, fsWrPerm, &outRefNum);
	}
	if (err != noErr)
		FATAL(("Couldn't open %s [%d]", out_name, err));
	
/*
*	Open Module Definition File
*	Must be named <basename>.def, where <basename> is
*	the base name of the resource file.  The module
*	definition file must be in the project.
*	Failure to find the file will cause the project to be
*	processes as an application (not an error).
*/
	strcpy(filename,in_name);
	p = strrchr(filename,'.');
	*p = 0;
	strcat(filename,".def");
	err = noErr;
	if(SearchProjectForFile(cpb, &specDef, filename))
		err = FSpOpenDF(&specDef, fsRdPerm, &defRefNum);
	if (err != noErr)
		FATAL(("Couldn't open %s [%d]", filename, err));

/*
*	Open Exports File
*	Must be named <basename>.exp, where <basename> is
*	the name of the Project File.  The exports file
*	must be in the project.
*	Failure to locate the exports file in the project
*	supresses generation of exports (not an error).
*/
	p2cstrcpy(filename, cpb->targetfile.name);
	strcat(filename, ".exp");
	err = noErr;
	if(SearchProjectForFile(cpb, &specExp, filename)) {
		err = FSpOpenDF(&specExp, fsWrPerm, &expRefNum);
		if (err != noErr) {
			err1 = FSpOpenDF(&specExp, fsRdPerm, &expRefNum);
			if (err1 != noErr) {
				FATAL(("Could not open exports file, %s. [%d]", filename, err1));
			} else {
				WARNING(("Exports file, %s, not writable [%d]. Using existing exports file.", filename, err));
				expRefNum = 0;	/* zero out expRefNum so we don't try to write to it */
			}
		}
	}
		
/*
*	Open C++ module file
*/
	if(strlen(cpp_module_name) > 0)
	{
	  strcpy(filename, cpp_module_name);
	  err = noErr;
	  if(SearchProjectForFile(cpb, &specCpp, filename))
	  	err = FSpOpenDF(&specCpp, fsWrPerm, &cppRefNum);
	  if (err != noErr)
	  	FATAL(("Couldn't open %s [%d]", filename, err));
	}
}

void at_start_of_macro(void)
{
	set_ext((void *)currentfile);
}

void at_start_of_file(void)
{
	set_ext((void *)currentfile);
}

void at_end_of_file(void)
{
	struct macfileinfo *fileinfo;
	Handle hFile;
	
	/* Maintain the completed-file line count */
	total_lines += current_line();
	
	/* Close the input buffer we used for this file; free the macfileinfo struct */
	fileinfo = (struct macfileinfo *)get_ext();
	free_mem(fileinfo);
}

void update_info(void)
{
	OSErr err;
	err = CWDisplayLines(cpb, total_lines + extra_lines());
	if (err != noErr)
		longjmp(jmpbuf, err);
}

void *get_mem(unsigned size)
{
	void *ptr = malloc(size);
	if (!ptr)
		FATAL(("Out of memory"));
	return ptr;
}
	
void free_mem(void *p)
{
	free(p);
}

/*************************************************
*	strdup
*
*	Duplicates a string, allocating the memory required to do so.
*************************************************/
char *strdup(const char *s)
{
int len = strlen(s);
char *s2;

	if (len > 0) {
		s2 = (char *)malloc(len);
		strcpy(s2, s);
		return(s2);		
	}
	else
		return(NULL);
}

/*************************************************
*	strcasecmp
*
*	Implements a non-case sensitive string compare.
*************************************************/
int strcasecmp(const char *s1, const char *s2)
{
char *n1 = (char *)s1, *n2 = (char *)s2;
	
	if (s1 == NULL || s2 == NULL)
		return 1;
	
	while (*n1) {
		if (toupper(*n1++) != toupper(*n2++))
			return (*(n1-1)-*(n2-1));
	}

	return *n2;

}

/*************************************************
*	SearchProjectForFile
*
*	Iterate through the files in the project
*	looking for fielname.  If found, stuff
*	the FSSpec into fsSpec and return.  If not
*	found, return fsSpec = 0
*************************************************/

int
SearchProjectForFile(CompilerLinkerParameterBlockPtr cpb, FSSpecPtr fsSpec, const char *filename)
{
	ProjectFileInfo fileInfo;
	int i;
	char cFilename[256];
	
	for(i=0; i < cpb->numfiles; i++) {
		CWGetFileInfo(cpb, i, &fileInfo);
		if (strcasecmp(p2cstr(fileInfo.filespec.name), filename) == 0) {
			c2pstr((char *)fileInfo.filespec.name);
			memcpy(fsSpec, &fileInfo.filespec, sizeof(FSSpec));
			return(1);
		}
	}
			memset(fsSpec, '\0', sizeof(FSSpec));
			return(0);
}
