/* Program: ATTRIB
   Written by: Phil Brutsche
   Copyright: 1998 by Phil Brutsche, under the terms of the GNU GPL 2.
   Maintained by: Brian E. Reifsnyder

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <dos.h>
#include <stdio.h>
#include <string.h>

#include "types.h"


#define VERSION "2.1"

#define EXIT_OK		0
#define E_ACCESS	1
#define E_TARGET	2
#define E_LIST		3
#define E_OPTION	4

#define ALL_FILE (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH)
#define ALL_ATTR (ALL_FILE | _A_SUBDIR)
#define DIR_ATTR ALL_ATTR


/*======================================================================*/

typedef byte ATTR;

/* do_path() requires less than 64 bytes of stack and path length is	*/
/* less than 80 characters (ie. there less than 40 nested directories	*/
/* possible), consequently deepest recursion requires 64*40=2.5K. With	*/
/* path equal to 256 characters deepest recursion requires 64*128=8K.	*/

extern unsigned _Cdecl _stklen = 9*1024u;	/* TC/BC specific	*/

LOCAL byte recurse = 0;
LOCAL ATTR useattr = ALL_FILE;
LOCAL ATTR attr_keep = ~0u, attr_set = 0;

LOCAL ATTR findattr;
LOCAL char path [PATHLEN+4],			/* 4=strlen("\\*.*")	*/
	   mask [PATHLEN];

LOCAL char info [] = "[-----] -> [-----] ";
		   /* _^______^___^ */

#define OLD_ATTR (info+1)
#define NEW_PART (info+8)
#define NEW_ATTR (info+12)

LOCAL unsigned found;
LOCAL byte retcode = EXIT_OK;

/*----------------------------------------------------------------------*/

LOCAL char *PROC adds	(char dst[], const char src[]);
LOCAL ATTR PROC attr2str(char str[], ATTR attr);

LOCAL void PROC do_mask	(char *pathend);
LOCAL void PROC do_path	(char *pathend);

LOCAL char *PROC trimsq	(char *p, char *pend);
LOCAL char *PROC nsplit	(char *pathend);

LOCAL void PROC process	(const char target[]);
LOCAL void PROC list	(const char name[]);

LOCAL void PROC _s_	(const char s[]);
LOCAL void PROC say	(const char s1[], const char s2[]);
LOCAL void PROC error	(byte errcode, const char arg[], const char err[]);

#define sayerror(code,err,arg) error (code,arg,err)


/*======================================================================*/

LOCAL char *PROC adds (char dst[], const char src[]) {
	return stpcpy (dst, src);
}

LOCAL ATTR PROC attr2str (char attr_str[], ATTR attr) {
	char ch;
	ch = '-'; if (attr & _A_SUBDIR) ch = 'D'; attr_str [0] = ch;
	ch = '-'; if (attr & _A_HIDDEN) ch = 'H'; attr_str [1] = ch;
	ch = '-'; if (attr & _A_SYSTEM) ch = 'S'; attr_str [2] = ch;
	ch = '-'; if (attr & _A_RDONLY) ch = 'R'; attr_str [3] = ch;
	ch = '-'; if (attr & _A_ARCH)   ch = 'A'; attr_str [4] = ch;
	return attr;
}

/*----------------------------------------------------------------------*/

LOCAL void PROC do_mask (char *pathend) {
	static struct find_t fi;

	adds (pathend, mask);
	if (_dos_findfirst (path, findattr, &fi))
		return;

	do {	const char *p;
		p = fi.name-1; do p++; while (*p == '.');
		if (*p == '\0') continue;		/* name == dots */

		adds (pathend, fi.name); found++;
	    {	ATTR attr = attr2str (OLD_ATTR, fi.attrib);

		if (*NEW_PART != '\0') {
		    attr = (attr & attr_keep) | attr_set;
		    if (_dos_setfileattr (path,
					  attr2str (NEW_ATTR, attr)
					  & (ATTR)~_A_SUBDIR)) {
			sayerror (E_ACCESS, "access denied", path);
			continue;
	    }	}    }

		say (info, path);
	} while (_dos_findnext (&fi) == 0);
}

LOCAL void PROC do_path (char *pathend) {
	struct find_t fi;

	do_mask (pathend); if (recurse == 0) return;

	adds (pathend, "*.*");
	if (_dos_findfirst (path, DIR_ATTR, &fi))
		return;

	do {	const char *p;
		if ((fi.attrib & _A_SUBDIR) == 0) continue;
		p = fi.name-1; do p++; while (*p == '.');
		if (*p == '\0') continue;		/* name == dots */

		do_path (adds (adds (pathend, fi.name), "\\"));
	} while (_dos_findnext (&fi) == 0);
}

/*----------------------------------------------------------------------*/

LOCAL char *PROC trimsq (char *p, char *pend) {
	do {	if (pend == p) { *pend = '\0'; return p; }
/*!*/		pend--;
	} while (*pend == ' ' || *pend == '\t');/* trailing spaces,	*/

	while (*p == ' ' || *p == '\t') p++;	/* ...leading spaces	*/

	if (p < pend && *p == '"' && *pend == '"')
		p++, pend--;			/* ...and quotes	*/

	pend++, *pend = '\0';
	return p;
}

LOCAL char *PROC nsplit (char *p) {
	char *pend = p; ATTR fattr = ALL_ATTR;

	/*--- scan trailing name */

	for (;;) {
	    if (p == path) break;
/*!*/	    p--;
	    if (*p == '.') continue;		/* skip trailing dots	*/
	    if (*p != '\\' && *p != '/' && *p != ':') {
		pend = NULL;			/* this is filename	*/
		if (*p == '*' || *p == '?')	/* check wildcards	*/
			fattr = useattr;
	    } else {
		p++; break;
	}   }

	/*--- extract name, if present */

    {	char *q = p;
	if (pend) {			/* path="[[<something>]\]{.}"	*/
		if (p < pend) p = pend, *p++ = '\\';
		fattr = useattr, q = "*.*";	/* default mask value	*/
	}

	findattr = fattr, adds (mask, q);
	return p;
}   }

/*----------------------------------------------------------------------*/

LOCAL void PROC process (const char target[]) {
/*	if (strlen(target) > sizeof(path)-5) {
 *		sayerror (E_TARGET, "too long", target);
 *		return;
 *	}
 */	found = 0;
	do_path (nsplit (adds (path, target)));
	if (found == 0) sayerror (E_TARGET, "no targets", target);
}

LOCAL void PROC list (const char name[]) {
	FILE *fp = stdin;
	if (name[0] && (fp = fopen (name, "rt")) == NULL) {
		sayerror (E_LIST, "error open file", name);
		return;
	}

    {	static char line [PATHLEN]; register int eof;
	do {	register char *p = line; int ch;

		do {	ch = fgetc (fp);
			if (ch == EOF || ch == '\n') break;
			*p++ = ch;
		} while (p < &line [sizeof (line)-1]);

		eof = ch, p = trimsq (line, p);
		if (*p) process (strupr (p));
/*!*/	} while (eof -= EOF);
    }
	if (fp != stdin) fclose (fp);
}

/*----------------------------------------------------------------------*/

LOCAL FILE *_s_out;

LOCAL void PROC _s_ (const char s[]) { fputs (s, _s_out); }

LOCAL void PROC say (const char s1[], const char _s2[]) {
/*!*/	register const char *const s2 = _s2;
	_s_out = stdout;
	_s_ (s1); _s_ (s2); _s_ ("\n");
}

LOCAL void PROC error (byte errcode, const char arg[], const char err[]) {
	if (errcode > retcode) retcode = errcode;
	_s_out = stderr;
	_s_ ("ATTRIB: "); _s_ (err); _s_ (": "); _s_ (arg); _s_ ("\n");
}


/*======================================================================*/

#define HELP() \
   say ("ATTRIB v" VERSION " - Displays or changes file attributes.\n" \
	"Copyright (c) 1998-2003, licensed under GPL2.\n\n" \
	"Syntax: ATTRIB { options | [path\][file] | /@[list] }\n\n" \
	"Options:\n\n" \
	"  +H  Sets the Hidden attribute.     -H  Clears the Hidden attribute.\n" \
	"  +S  Sets the System attribute.     -S  Clears the System attribute.\n" \
	"  +R  Sets the Read-only attribute.  -R  Clears the Read-only attribute.\n" \
	"  +A  Sets the Archive attribute.    -A  Clears the Archive attribute.\n\n" \
	"  /S  Process files in all directories in the specified path(es).\n" \
	"  /D  Process directory names for arguments with wildcards.\n" \
	"  /@  Process files, listed in the specified file [or in stdin].\n\n" \
	"Examples:\n\n" \
	"  attrib file -rhs\n" \
	"  attrib +a -r dir1\ dir2\*.dat /s\n" \
	"  attrib -hs/sd /@list.txt *.*", "")

LOCAL byte iscomma = 0;

int _Cdecl main (int argc, char *argv[]) {
    char **p = argv, **q = p;

    while (--argc) {
	char *arg; p++, arg = strupr (*p);

	/* Comma is an `undocumented feature` from MS's ATTRIB. It	*/
	/* basically unsets all the attributes of all the files in	*/
	/* a directory							*/

	if (*arg == ',') iscomma = 1, arg++;

	if (*arg == '+' || *arg == '-') {		/* +-HSRA	*/
		register char ch0 = *arg; arg++;
		do {	ATTR mask = 0;
			if (*arg == 'H') mask = _A_HIDDEN;
			if (*arg == 'S') mask = _A_SYSTEM;
			if (*arg == 'R') mask = _A_RDONLY;
			if (*arg == 'A') mask = _A_ARCH;
			if (mask == 0) {
				sayerror (E_OPTION, "invalid attribute", arg);
				return retcode;
			}

			if (ch0 == '+') attr_set |= mask;
			mask = ~mask, attr_keep &= mask;
			arg++;
		} while (*arg && *arg != '/');
	}

	if (*arg == '/') {				/* /?@SD	*/
		do {	if (*arg == '/') arg++;
			if (*arg == '?') { HELP();	return retcode; }
			if (*arg == '@') { *arg = '/';	break; }
			if (*arg == 'S') { recurse = 1;	continue; }
			if (*arg != 'D') {
				sayerror (E_OPTION, "invalid option", arg);
				return retcode;
			}
			/*if (*arg == 'D')*/ useattr = ALL_ATTR;
		} while (arg++, *arg);
	}

	if (*arg) *q++ = arg;				/* file /@	*/
    } /* while */

    if (attr_keep == (ATTR)~0u) {
	if (iscomma)
		attr_keep = ~ALL_FILE /*, attr_set = 0*/;
	else	*NEW_PART = '\0';
    }

    p = argv;
    /*if (p == q)*/ *q = "*.*";

    do {
	if (**p == '/')	list (&(*p) [1]);
		else	process (*p);
	p++;
    } while (p < q);

    return retcode;
}

/*======================================================================*/

#ifdef __BORLANDC__
# include "setupio.inc"
# include "setvbuf.inc"
# include "malloc.inc"
# include "stdio.inc"
#endif
