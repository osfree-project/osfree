/*
    ASSIGN: Replace drive letters
    Copyright (C) 1995-97 Steffen Kaiser

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/* $RCSfile: foreach.xc $
   $Locker: ska $	$Name:  $	$State: Exp $

	This is an utility for Micro-C only, it was coded to not use another
	library or source code.

		FOREACH [/e:<ext>[{,<ext>}]] file {<command token>}

	It reads the file, breaks all lines into whitespace separated tokens
	and interpretes them as filenames. The "/E" switch causes to ignore
	all filenames that have a different extension as the given ones

	The command tokens may consist of any arguments, they will be joined
	together delimited by a single space, then scanned for placeholders,
	after substituting the placeholders the line is passed to the
	system() function.

	These placeholders are recognized:
		%fname:		: The filename without path and extension
		%fext:		: The filename's extension without dot
		%name:		: The filename with extension (if no extension, no dot)
		%ename:		: The filename with extension (if no extension, ends in '.')
		%fpath:		: The filename's path without drive and trailing '\\'
		%dfpath:	: The filename's path with drive
		%fdrive:	: The filename's drive without colon
		%file:		: The complete filename
		%rpath:		: The filename's fully-qualified path without drive and root
		%rfile:		: The fully-qualifed filename without drive and root
	If a placeholder is prefixed by another percent sign, the two percent
	signs are replaced by a single one, but the placeholder is ignored.
	If the percent sign of a placeholder is followed by ':', the filename
	is fully-qualified according the current working directory.

*/

#include <portable.h>
#include <stdio.h>

#define NODE_FNAME 1
#define NODE_FEXT 2
#define NODE_NAME 3
#define NODE_ENAME 4
#define NODE_FPATH 5
#define NODE_DFPATH 6
#define NODE_FDRIVE 7
#define NODE_FILE 8
#define NODE_RPATH 9
#define NODE_RFILE 10

#define NODE_MKABSOLUTE 0x80
#define NODE_PERCENT 0x7f
#define NODE_STRING 0xff

#define NODE_AFNAME (NODE_MKABSOLUTE | NODE_FNAME)
#define NODE_AFEXT (NODE_MKABSOLUTE | NODE_FEXT)
#define NODE_ANAME (NODE_MKABSOLUTE | NODE_NAME)
#define NODE_AENAME (NODE_MKABSOLUTE | NODE_ENAME)
#define NODE_AFPATH (NODE_MKABSOLUTE | NODE_FPATH)
#define NODE_ADFPATH (NODE_MKABSOLUTE | NODE_DFPATH)
#define NODE_AFDRIVE (NODE_MKABSOLUTE | NODE_FDRIVE)
#define NODE_AFILE (NODE_MKABSOLUTE | NODE_FILE)
#define NODE_ARPATH (NODE_MKABSOLUTE | NODE_RPATH)
#define NODE_ARFILE (NODE_MKABSOLUTE | NODE_RFILE)

#ifndef lint
static char const rcsid[] = 
	"$Id: foreach.xc 1.1 1996/12/05 03:57:44 ska Exp ska $";
#endif

char drive[3], *path, name[9], *ext;	/* relative filename portions */
char adrive[3], *apath;	/* fully-qualified ones */
char odrive, *opath;	/* original path/drive */

char appName[9];
char buf[2048];			/* The line buffer */
char line[128];			/* command line */
struct ICMDLINE {
	struct ICMDLINE *nxt;
	int i_node;
	char *i_line;
} head;			/* intermediate command line */
struct EXTENSION {
	struct EXTENSION *nxt;
	char *ext_name;
} *ext_head = NULL;		/* all accepted extensions */
struct PLACEHOLDER {
	char *ph_name;
	int ph_node;
} placeholders[12] = {
		int "fname:", NODE_FNAME,
		"fext:", NODE_FEXT,
		"name:", NODE_NAME,
		"ename:", NODE_ENAME,
		"fpath:", NODE_FPATH,
		"dfpath:", NODE_DFPATH,
		"fdrive:", NODE_FDRIVE,
		"file:", NODE_FILE,
		"rpath:", NODE_RPATH,
		"rfile:", NODE_RFILE,
		NULL
};

register error(unsigned args)
{	_format_(nargs() * 2 + &args, buf);
	fprintf("%s: Error: ", appName);
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(30);
}

/* Allocate memory and return an error on failure */
char *dupstr(char *s)
{	char *h;

	if(!(h = strdup(s)))
		error("Memory overflow");
	return h;
}

/* Allocate memory and return an error on failure */
char *getmem(unsigned size)
{	char *h;

	if(!(h = malloc(size)))
		error("Memory overflow");
	return h;
}

/* Add an accepted extension */
void addExt(char *ext)
{	struct EXTENSION *act, *h;

	h = getmem(sizeof(struct EXTENSION));
	if(act = ext_head) {
		while(act->nxt) act = act->nxt;
		act->nxt = h;
	}
	else ext_head = h;
	h->ext_name = dupstr(ext);
	h->nxt = NULL;
}

int extValid(char *ext)
{	struct EXTENSION *act;

	if(!(act = ext_head)) return 1;

	if(!ext) ext = "";
	do if(strcmp(ext, act->ext_name) == 0) return 1;
	while(act = act->nxt);
	return 0;
}

/* Add a string to the line buffer */
char *addString(char *p, char *string)
{	if(strlen(string) > line + sizeof(line) - p)
		error("Command line overflow");
	return stpcpy(p, string);
}

/* Split the filename and invoke the command */
void handleFName(char *fnam)
{	char *p;
	struct ICMDLINE *q;
	char hdrive[2];

	if(fnam[1] == ':') {	/* drive spec */
		*adrive = *drive = *fnam;
		fnam += 2;
	}
	else {
		*drive = 0;
		*adrive = odrive;
	}

	p = strchr(fnam, '\0');
	while(--p >= fnam && *p != '/' && *p != '\\');
	++p;
	if(p != fnam) {			/* path portion */
		p[-1] = 0;
		path = dupstr(fnam);
		if(*fnam != '/' && *fnam != '\\') {
			concat(apath = getmem(strlen(path) + strlen(opath) + 2)
			, opath, "\\", path);
		}
		else apath = dupstr(path);
	}
	else {
		path = NULL;
		apath = dupstr(opath);
	}

	p = strchr(fnam = p, '.');
	if(p != fnam && p) *p = 0;
	memcpy(name, fnam, 8);
	name[8] = 0;
	if(p != fnam && p) {
		ext = dupstr(p + 1);
		ext[3] = 0;
	}
	else ext = NULL;

if(extValid(ext)) {
	q = head;
	p = line;
	hdrive[1] = 0;
	while(q = q->nxt) switch(q->i_node) {
#define addStr(a) p = addString(p, a)
	case NODE_PERCENT:	/* add a percent sign to the line */
		addStr("%");
		break;
	case NODE_STRING:	/* add the string to the line */
		addStr(q->i_line);
		break;
	case NODE_AFNAME:
	case NODE_FNAME:	/* add the filename without extension */
		addStr(name);
		break;
	case NODE_AENAME:
	case NODE_ENAME:	/* add name and extension */
		addStr(name);
		addStr(".");
		/* fall through */
	case NODE_AFEXT:
	case NODE_FEXT:		/* add the extension */
		if(ext) addStr(ext);
		break;
	case NODE_FILE:		/* complete filename */
		if(*drive) addStr(drive);
		if(!path) break;
		addStr(path);
		goto slashName;
	case NODE_AFILE:
		if(*adrive) addStr(adrive);
		addStr(apath);
slashName:
		addStr("\\");
	case NODE_ANAME:
	case NODE_NAME:		/* name and, if exists, extension */
		addStr(name);
		if(ext) {
			addStr(".");
			addStr(ext);
		}
		break;
	case NODE_ADFPATH:
		addStr(adrive);
	case NODE_AFPATH:
		addStr(apath);
		break;
	case NODE_DFPATH:	/* drive and path without trailing \\ */
		if(*drive) addStr(drive);
		/* fall through */
	case NODE_FPATH:	/* path without drive and trailing \\ */
		if(path) addStr(path);
		break;
	case NODE_AFDRIVE:
		*hdrive = *adrive;
		addStr(hdrive);
		break;
	case NODE_FDRIVE:	/* drive without colon */
		if(*hdrive = *drive) addStr(hdrive);
		break;
	case NODE_ARPATH:
	case NODE_RPATH:	/* fully-qualified path without drive & root */
		addStr(apath + 1);
		break;
	case NODE_ARFILE:
	case NODE_RFILE:	/* fully-qualified filename without drive & root */
		addStr(apath + 1);
		goto slashName;
	}

	*p = 0;
	system(line);
}

	free(path);
	free(apath);
	free(ext);
}

struct ICMDLINE *mkNode(struct ICMDLINE *act, int node)
{
	(act = act->nxt = getmem(sizeof(struct ICMDLINE)))->i_node = node;
	return act;
}

struct ICMDLINE *mkStrNode(struct ICMDLINE *act, char *s)
{
	(act = mkNode(act, NODE_STRING))->i_line = dupstr(s);
	return act;
}

/* Decompose the command line into the intermediate command line
	describing where to insert the replacements for the placeholders.
*/
void decomposeArgLine(void)
{	char *p, *s, *q;
	struct ICMDLINE *act;
	struct PLACEHOLDER *ph;
	int mkAbs;

	act = head;
	p = line;
	while(s = strchr(p, '%')) {
		if(s[1] == '%') {	/* ignore this placeholder */
			act = mkNode(act, NODE_PERCENT);
			p += 2;			/* skip "%%" */
		}
		else {				/* check for a known placeholder */
			if(mkAbs = *(q = s + 1) == ':')
				++q;
			for(ph = placeholders; ph->ph_name && !strbeg(q, ph->ph_name); ph += sizeof(struct PLACEHOLDER));
			if(ph->ph_name) {		/* placeholder found */
				if(s != p) {	/* there is a string */
					*s = 0;
					act = mkStrNode(act, p);
				}
				act = mkNode(act, ph->ph_node);
				if(mkAbs) ph->ph_node |= NODE_MKABSOLUTE;
				q += strlen(ph->ph_name);
			}
			/* else: just ignore */
			p = q;
		}
	}
	if(*p)	/* there is a string */
		act = mkStrNode(act, p);
	act->nxt = NULL;
}

void hlpScreen(void)
{
	printf("FOREACH utility\nUseage: %s [/e:<ext>[{,<ext>}]] file {<command token>}\n", appName);
	puts("\nReads file and interpretes each word as a filename.\nThese are the palceholders:");
	puts("\t%fname:		: The filename without path and extension");
	puts("\t%fext:		: The filename's extension without dot");
	puts("\t%name:		: The filename with extension (if no extension, no dot)");
	puts("\t%ename:		: The filename with extension (if no extension, ends in '.')");
	puts("\t%fpath:		: The filename's path without drive and trailing '\\'");
	puts("\t%dfpath:	: The filename's path with drive");
	puts("\t%fdrive:	: The filename's drive without colon");
	puts("\t%file:		: The complete filename");
	puts("\t%rpath:		: The filename's fully-qualified path without drive and root");
	puts("\t%rfile:		: The fully-qualifed filename without drive and root");
	puts("The /E: switch enumerates all accepted extensions.");
	exit(127);
}

/* Retreive the application's name */
void getAppName(char *fnam)
{	char *p;

	p = strchr(fnam, '\0');
	while(--p >= fnam && !strchr("/\\:", *p));
	fnam = p;
	while(*++fnam && *fnam != '.');
	*fnam = 0;
	memcpy(appName, p + 1, 9);
}

main(int argc, char **argv)
{	FILE *f;			/* The filename file */
	char *p;

	drive[2] = adrive[2] = 0;
	drive[1] = adrive[1] = ':';
	if(getdir(buf + 1))
		error("Cannot retreive current directory");
	odrive = 'A' + get_drive();
	*buf = '\\';
	opath = dupstr(buf);

	getAppName(argv[0]);
	if(argc < 3 || argv[1][0] == '/' && strchr("?hH", argv[1][1]))
		hlpScreen();

	if(argv[1][0] == '/') {
		if(tolower(argv[1][1]) != 'e' || !strchr(":=", argv[1][2])
		 || !(p = strtok(argv[1] + 3, ",")))
			error("Unknown or invalid switch %c", argv[1][1]);
		do addExt(p);
		while(p = strtok(NULL, ","));
		argv += 4;
	}
	else ++argv;

	if(!(f = fopen(*argv, "r")))
		error("Cannot open file %s", *argv);

	if(!*++argv) hlpScreen();

	p = line;
	do p = stpcpy(p, *argv), *p++ = ' ';
	while(*++argv);
	p[-1] = 0;
	decomposeArgLine();

	while(fgets(buf, sizeof(buf), f)) {
		if(p = strtok(buf, " \t\n\r"))
			do handleFName(p);
			while(p = strtok(NULL, " \t\n\r"));
	}

	fclose(f);
}
