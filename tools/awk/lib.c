/****************************************************************
 * Copyright (C) Lucent Technologies 1997
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name Lucent Technologies or any of
 * its entities not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.
 *
 * LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 ****************************************************************/

/*!
 *  @file lib.c
 *  @brief Input, record splitting and diagnostics for awk.
 *
 *  Reads records from the current input, splits them into fields,
 *  maintains the $0..$NF table, rebuilds $0 from fields, and
 *  provides the diagnostic helpers used across the interpreter.
 *
 *  @copyright Copyright (C) Lucent Technologies 1997.
 */

#define DEBUG
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include "awk.h"
#include "ytab.h"

FILE	*infile	= NULL;      /*!< Current input stream. */
char	*file	= "";        /*!< Name of the current input file. */
char	*record;             /*!< Current record ($0). */
int	recsize	= RECSIZE;   /*!< Size of the record buffer. */
char	*fields;             /*!< Field storage buffer. */
int	fieldssize = RECSIZE; /*!< Size of the field storage buffer. */

Cell	**fldtab;            /*!< Table of fields $0..$NF. */
char	inputFS[100] = " ";  /*!< Field separator captured at input time. */

/*!
 *  @brief Initial number of field slots.
 *  @def MAXFLD
 */
#define	MAXFLD	200

int	nfields	= MAXFLD;    /*!< Last allocated field slot index. */

int	donefld;             /*!< 1 if the current record has been split into fields. */
int	donerec;             /*!< 1 if the current record is valid (no field changed). */

int	lastfld	= 0;         /*!< Last field used in the current record. */
int	argno	= 1;         /*!< Current input argument number. */
extern	Awkfloat *ARGC;     /*!< Number of command-line arguments defined in tran.c. */

static Cell dollar0 = { OCELL, CFLD, NULL, "", 0.0, REC|STR|DONTFREE }; /*!< Prototype cell for $0. */
static Cell dollar1 = { OCELL, CFLD, NULL, "", 0.0, FLD|STR|DONTFREE }; /*!< Prototype cell for $1..$NF. */

/*!
 *  @brief Initializes the record and field buffers.
 *
 *  @param[in] n Initial buffer size.
 */
void recinit(unsigned int n)
{
	record = (char *) malloc(n);
	fields = (char *) malloc(n);
	fldtab = (Cell **) malloc((nfields+1) * sizeof(Cell *));
	if (record == NULL || fields == NULL || fldtab == NULL)
		FATAL("out of space for $0 and fields");

	fldtab[0] = (Cell *) malloc(sizeof (Cell));
	*fldtab[0] = dollar0;
	fldtab[0]->sval = record;
	fldtab[0]->nval = tostring("0");
	makefields(1, nfields);
}

/*!
 *  @brief Creates the cells for fields $n1..$n2.
 *
 *  @param[in] n1 First field index.
 *  @param[in] n2 Last field index.
 */
void makefields(int n1, int n2)
{
	char temp[50];
	int i;

	for (i = n1; i <= n2; i++) {
		fldtab[i] = (Cell *) malloc(sizeof (struct Cell));
		if (fldtab[i] == NULL)
			FATAL("out of space in makefields %d", i);
		*fldtab[i] = dollar1;
		sprintf(temp, "%d", i);
		fldtab[i]->nval = tostring(temp);
	}
}

/*!
 *  @brief Initializes the first input source (file or stdin).
 */
void initgetrec(void)
{
	int i;
	char *p;

	for (i = 1; i < *ARGC; i++) {
		if (!isclvar(p = getargv(i))) {	/* find 1st real filename */
			setsval(lookup("FILENAME", symtab), getargv(i));
			return;
		}
		setclvar(p);	/* a commandline assignment before filename */
		argno++;
	}
	infile = stdin;		/* no filenames, so use stdin */
}

/*!
 *  @brief Reads the next input record.
 *
 *  @param[in,out] pbuf      Pointer to the record buffer.
 *  @param[in,out] pbufsize  Pointer to the record buffer size.
 *  @param[in]     isrecord  Non-zero if the buffer holds $0.
 *
 *  @return Whether a record was read.
 *  @retval 0 End of input reached.
 *  @retval 1 Record read successfully.
 */
int getrec(char **pbuf, int *pbufsize, int isrecord)
{
	int c;
	static int firsttime = 1;
	char *buf = *pbuf;
	int bufsize = *pbufsize;

	if (firsttime) {
		firsttime = 0;
		initgetrec();
	}
	   dprintf( ("RS=<%s>, FS=<%s>, ARGC=%g, FILENAME=%s\n",
		*RS, *FS, *ARGC, *FILENAME) );
	if (isrecord) {
		donefld = 0;
		donerec = 1;
	}
	buf[0] = 0;
	while (argno < *ARGC || infile == stdin) {
		   dprintf( ("argno=%d, file=|%s|\n", argno, file) );
		if (infile == NULL) {	/* have to open a new file */
			file = getargv(argno);
			if (*file == '\0') {	/* it's been zapped */
				argno++;
				continue;
			}
			if (isclvar(file)) {	/* a var=value arg */
				setclvar(file);
				argno++;
				continue;
			}
			*FILENAME = file;
			   dprintf( ("opening file %s\n", file) );
			if (*file == '-' && *(file+1) == '\0')
				infile = stdin;
			else if ((infile = fopen(file, "r")) == NULL)
				FATAL("can't open file %s", file);
			setfval(fnrloc, 0.0);
		}
		c = readrec(&buf, &bufsize, infile);
		if (c != 0 || buf[0] != '\0') {	/* normal record */
			if (isrecord) {
				if (freeable(fldtab[0]))
					xfree(fldtab[0]->sval);
				fldtab[0]->sval = buf;	/* buf == record */
				fldtab[0]->tval = REC | STR | DONTFREE;
				if (is_number(fldtab[0]->sval)) {
					fldtab[0]->fval = atof(fldtab[0]->sval);
					fldtab[0]->tval |= NUM;
				}
			}
			setfval(nrloc, nrloc->fval+1);
			setfval(fnrloc, fnrloc->fval+1);
			*pbuf = buf;
			*pbufsize = bufsize;
			return 1;
		}
		/* EOF arrived on this file; set up next */
		if (infile != stdin)
			fclose(infile);
		infile = NULL;
		argno++;
	}
	*pbuf = buf;
	*pbufsize = bufsize;
	return 0;	/* true end of file */
}

/*!
 *  @brief Advances to the next input file.
 */
void nextfile(void)
{
	if (infile != stdin)
		fclose(infile);
	infile = NULL;
	argno++;
}

/*!
 *  @brief Reads one record from the given input stream.
 *
 *  @param[in,out] pbuf     Pointer to the record buffer.
 *  @param[in,out] pbufsize Pointer to the record buffer size.
 *  @param[in]     inf      Input stream.
 *
 *  @return Whether a record was read.
 *  @retval 0 End of input reached.
 *  @retval 1 Record read successfully.
 */
int readrec(char **pbuf, int *pbufsize, FILE *inf)
{
	int sep, c;
	char *rr, *buf = *pbuf;
	int bufsize = *pbufsize;

	if (strlen(*FS) >= sizeof(inputFS))
		FATAL("field separator %.10s... is too long", *FS);
	strcpy(inputFS, *FS);	/* for subsequent field splitting */
	if ((sep = **RS) == 0) {
		sep = '\n';
		while ((c=getc(inf)) == '\n' && c != EOF)	/* skip leading \n's */
			;
		if (c != EOF)
			ungetc(c, inf);
	}
	for (rr = buf; ; ) {
		for (; (c=getc(inf)) != sep && c != EOF; ) {
			if (rr-buf+1 > bufsize)
				if (!adjbuf(&buf, &bufsize, 1+rr-buf, recsize, &rr, "readrec 1"))
					FATAL("input record `%.30s...' too long", buf);
			*rr++ = c;
		}
		if (**RS == sep || c == EOF)
			break;
		if ((c = getc(inf)) == '\n' || c == EOF) /* 2 in a row */
			break;
		if (!adjbuf(&buf, &bufsize, 2+rr-buf, recsize, &rr, "readrec 2"))
			FATAL("input record `%.30s...' too long", buf);
		*rr++ = '\n';
		*rr++ = c;
	}
	if (!adjbuf(&buf, &bufsize, 1+rr-buf, recsize, &rr, "readrec 3"))
		FATAL("input record `%.30s...' too long", buf);
	*rr = 0;
	   dprintf( ("readrec saw <%s>, returns %d\n", buf, c == EOF && rr == buf ? 0 : 1) );
	*pbuf = buf;
	*pbufsize = bufsize;
	return c == EOF && rr == buf ? 0 : 1;
}

/*!
 *  @brief Returns ARGV[@p n ].
 *
 *  @param[in] n Argument index.
 *
 *  @return Argument string.
 */
char *getargv(int n)
{
	Cell *x;
	char *s, temp[50];
	extern Array *ARGVtab;

	sprintf(temp, "%d", n);
	x = setsymtab(temp, "", 0.0, STR, ARGVtab);
	s = getsval(x);
	   dprintf( ("getargv(%d) returns |%s|\n", n, s) );
	return s;
}

/*!
 *  @brief Sets a variable from a var=value command line argument.
 *
 *  @param[in] s Assignment string of the form "name=value".
 */
void setclvar(char *s)
{
	char *p;
	Cell *q;

	for (p=s; *p != '='; p++)
		;
	*p++ = 0;
	p = qstring(p, '\0');
	q = setsymtab(s, p, 0.0, STR, symtab);
	setsval(q, p);
	if (is_number(q->sval)) {
		q->fval = atof(q->sval);
		q->tval |= NUM;
	}
	   dprintf( ("command line set %s to |%s|\n", s, p) );
}


/*!
 *  @brief Splits the current record into fields $1..$NF.
 */
void fldbld(void)
{
	/* this relies on having fields[] the same length as $0 */
	/* the fields are all stored in this one array with \0's */
	char *r, *fr, sep;
	Cell *p;
	int i, j, n;

	if (donefld)
		return;
	if (!isstr(fldtab[0]))
		getsval(fldtab[0]);
	r = fldtab[0]->sval;
	n = strlen(r);
	if (n > fieldssize) {
		xfree(fields);
		if ((fields = (char *) malloc(n+1)) == NULL)
			FATAL("out of space for fields in fldbld %d", n);
		fieldssize = n;
	}
	fr = fields;
	i = 0;	/* number of fields accumulated here */
	if (strlen(inputFS) > 1) {	/* it's a regular expression */
		i = refldbld(r, inputFS);
	} else if ((sep = *inputFS) == ' ') {	/* default whitespace */
		for (i = 0; ; ) {
			while (*r == ' ' || *r == '\t' || *r == '\n')
				r++;
			if (*r == 0)
				break;
			i++;
			if (i > nfields)
				growfldtab(i);
			if (freeable(fldtab[i]))
				xfree(fldtab[i]->sval);
			fldtab[i]->sval = fr;
			fldtab[i]->tval = FLD | STR | DONTFREE;
			do
				*fr++ = *r++;
			while (*r != ' ' && *r != '\t' && *r != '\n' && *r != '\0');
			*fr++ = 0;
		}
		*fr = 0;
	} else if ((sep = *inputFS) == 0) {		/* new: FS="" => 1 char/field */
		for (i = 0; *r != 0; r++) {
			char buf[2];
			i++;
			if (i > nfields)
				growfldtab(i);
			if (freeable(fldtab[i]))
				xfree(fldtab[i]->sval);
			buf[0] = *r;
			buf[1] = 0;
			fldtab[i]->sval = tostring(buf);
			fldtab[i]->tval = FLD | STR;
		}
		*fr = 0;
	} else if (*r != 0) {	/* if 0, it's a null field */
		/* subtlecase : if length(FS) == 1 && length(RS > 0)
		 * \n is NOT a field separator (cf awk book 61,84).
		 * this variable is tested in the inner while loop.
		 */
		int rtest = '\n';  /* normal case */
		if (strlen(*RS) > 0)
			rtest = '\0';
		for (;;) {
			i++;
			if (i > nfields)
				growfldtab(i);
			if (freeable(fldtab[i]))
				xfree(fldtab[i]->sval);
			fldtab[i]->sval = fr;
			fldtab[i]->tval = FLD | STR | DONTFREE;
			while (*r != sep && *r != rtest && *r != '\0')	/* \n is always a separator */
				*fr++ = *r++;
			*fr++ = 0;
			if (*r++ == 0)
				break;
		}
		*fr = 0;
	}
	if (i > nfields)
		FATAL("record `%.30s...' has too many fields; can't happen", r);
	cleanfld(i+1, lastfld);	/* clean out junk from previous record */
	lastfld = i;
	donefld = 1;
	for (j = 1; j <= lastfld; j++) {
		p = fldtab[j];
		if(is_number(p->sval)) {
			p->fval = atof(p->sval);
			p->tval |= NUM;
		}
	}
	setfval(nfloc, (Awkfloat) lastfld);
	if (dbg) {
		for (j = 0; j <= lastfld; j++) {
			p = fldtab[j];
			printf("field %d (%s): |%s|\n", j, p->nval, p->sval);
		}
	}
}

/*!
 *  @brief Clears the fields @p n1 through @p n2.
 *
 *  @param[in] n1 First field index.
 *  @param[in] n2 Last field index.
 */
void cleanfld(int n1, int n2)
{
	Cell *p;
	int i;

	for (i = n1; i <= n2; i++) {
		p = fldtab[i];
		if (freeable(p))
			xfree(p->sval);
		p->sval = "";
		p->tval = FLD | STR | DONTFREE;
	}
}

/*!
 *  @brief Adds field @p n after the end of the existing last field.
 *
 *  @param[in] n Field index to add.
 */
void newfld(int n)
{
	if (n > nfields)
		growfldtab(n);
	cleanfld(lastfld+1, n);
	lastfld = n;
	setfval(nfloc, (Awkfloat) n);
}

/*!
 *  @brief Returns the cell of the n-th field.
 *
 *  @param[in] n Field index.
 *
 *  @return Cell pointer for field @p n.
 */
Cell *fieldadr(int n)
{
	if (n < 0)
		FATAL("trying to access field %d", n);
	if (n > nfields)	/* fields after NF are empty */
		growfldtab(n);	/* but does not increase NF */
	return(fldtab[n]);
}

/*!
 *  @brief Grows the field table up to at least field @p n.
 *
 *  @param[in] n Minimum field index.
 */
void growfldtab(int n)
{
	int nf = 2 * nfields;

	if (n > nf)
		nf = n;
	fldtab = (Cell **) realloc(fldtab, (nf+1) * (sizeof (struct Cell *)));
	if (fldtab == NULL)
		FATAL("out of space creating %d fields", nf);
	makefields(nfields+1, nf);
	nfields = nf;
}

/*!
 *  @brief Builds fields from a record using a regular expression FS.
 *
 *  @param[in] rec Record string.
 *  @param[in] fs  Field separator regular expression.
 *
 *  @return Number of fields built.
 *  @retval 0 Empty record; no fields produced.
 */
int refldbld(const char *rec, const char *fs)
{
	/* this relies on having fields[] the same length as $0 */
	/* the fields are all stored in this one array with \0's */
	char *fr;
	int i, tempstat, n;
	fa *pfa;

	n = strlen(rec);
	if (n > fieldssize) {
		xfree(fields);
		if ((fields = (char *) malloc(n+1)) == NULL)
			FATAL("out of space for fields in refldbld %d", n);
		fieldssize = n;
	}
	fr = fields;
	*fr = '\0';
	if (*rec == '\0')
		return 0;
	pfa = makedfa(fs, 1);
	   dprintf( ("into refldbld, rec = <%s>, pat = <%s>\n", rec, fs) );
	tempstat = pfa->initstat;
	for (i = 1; ; i++) {
		if (i > nfields)
			growfldtab(i);
		if (freeable(fldtab[i]))
			xfree(fldtab[i]->sval);
		fldtab[i]->tval = FLD | STR | DONTFREE;
		fldtab[i]->sval = fr;
		   dprintf( ("refldbld: i=%d\n", i) );
		if (nematch(pfa, rec)) {
			pfa->initstat = 2;	/* horrible coupling to b.c */
			   dprintf( ("match %s (%d chars)\n", patbeg, patlen) );
			strncpy(fr, rec, patbeg-rec);
			fr += patbeg - rec + 1;
			*(fr-1) = '\0';
			rec = patbeg + patlen;
		} else {
			   dprintf( ("no match %s\n", rec) );
			strcpy(fr, rec);
			pfa->initstat = tempstat;
			break;
		}
	}
	return i;		
}

/*!
 *  @brief Rebuilds $0 from $1..$NF when a field has been modified.
 */
void recbld(void)
{
	int i;
	char *r, *p;

	if (donerec == 1)
		return;
	r = record;
	for (i = 1; i <= *NF; i++) {
		p = getsval(fldtab[i]);
		if (!adjbuf(&record, &recsize, 1+strlen(p)+r-record, recsize, &r, "recbld 1"))
			FATAL("created $0 `%.30s...' too long", record);
		while ((*r = *p++) != 0)
			r++;
		if (i < *NF) {
			if (!adjbuf(&record, &recsize, 2+strlen(*OFS)+r-record, recsize, &r, "recbld 2"))
				FATAL("created $0 `%.30s...' too long", record);
			for (p = *OFS; (*r = *p++) != 0; )
				r++;
		}
	}
	if (!adjbuf(&record, &recsize, 2+r-record, recsize, &r, "recbld 3"))
		FATAL("built giant record `%.30s...'", record);
	*r = '\0';
	   dprintf( ("in recbld inputFS=%s, fldtab[0]=%p\n", inputFS, fldtab[0]) );

	if (freeable(fldtab[0]))
		xfree(fldtab[0]->sval);
	fldtab[0]->tval = REC | STR | DONTFREE;
	fldtab[0]->sval = record;

	   dprintf( ("in recbld inputFS=%s, fldtab[0]=%p\n", inputFS, fldtab[0]) );
	   dprintf( ("recbld = |%s|\n", record) );
	donerec = 1;
}

int	errorflag	= 0;  /*!< Non-zero if any syntax error has been reported. */

/*!
 *  @brief Reports a syntax error via the yacc error callback.
 *
 *  @param[in] s Error message.
 */
void yyerror(const char *s)
{
	SYNTAX(s);
}

/*!
 *  @brief Reports a syntax error and dumps the error context.
 *
 *  @param[in] fmt printf-style format string.
 *  @param[in] ... Additional format arguments.
 */
void SYNTAX(const char *fmt, ...)
{
	extern char *cmdname, *curfname;
	static int been_here = 0;
	va_list varg;

	if (been_here++ > 2)
		return;
	fprintf(stderr, "%s: ", cmdname);
	va_start(varg, fmt);
	vfprintf(stderr, fmt, varg);
	va_end(varg);
	fprintf(stderr, " at source line %d", lineno);
	if (curfname != NULL)
		fprintf(stderr, " in function %s", curfname);
	if (compile_time == 1 && cursource() != NULL)
		fprintf(stderr, " source file %s", cursource());
	fprintf(stderr, "\n");
	errorflag = 2;
	eprint();
}

/*!
 *  @brief Signal handler for floating-point exceptions.
 *
 *  @param[in] n Signal number.
 */
void fpecatch(int n)
{
	FATAL("floating point exception %d", n);
}

extern int bracecnt;  /*!< Number of currently open braces defined in lex.c. */
extern int brackcnt;  /*!< Number of currently open brackets defined in lex.c. */
extern int parencnt;  /*!< Number of currently open parentheses defined in lex.c. */

/*!
 *  @brief Checks that all brackets in the program are balanced.
 */
void bracecheck(void)
{
	int c;
	static int beenhere = 0;

	if (beenhere++)
		return;
	while ((c = input()) != EOF && c != '\0')
		bclass(c);
	bcheck2(bracecnt, '{', '}');
	bcheck2(brackcnt, '[', ']');
	bcheck2(parencnt, '(', ')');
}

/*!
 *  @brief Reports a bracket-count mismatch.
 *
 *  @param[in] n  Difference between opening and closing counts.
 *  @param[in] c1 Opening bracket character.
 *  @param[in] c2 Closing bracket character.
 */
void bcheck2(int n, int c1, int c2)
{
	if (n == 1)
		fprintf(stderr, "\tmissing %c\n", c2);
	else if (n > 1)
		fprintf(stderr, "\t%d missing %c's\n", n, c2);
	else if (n == -1)
		fprintf(stderr, "\textra %c\n", c2);
	else if (n < -1)
		fprintf(stderr, "\t%d extra %c's\n", -n, c2);
}

/*!
 *  @brief Reports a fatal error and exits the interpreter.
 *
 *  @param[in] fmt printf-style format string.
 *  @param[in] ... Additional format arguments.
 */
void FATAL(const char *fmt, ...)
{
	extern char *cmdname;
	va_list varg;

	fflush(stdout);
	fprintf(stderr, "%s: ", cmdname);
	va_start(varg, fmt);
	vfprintf(stderr, fmt, varg);
	va_end(varg);
	error();
	if (dbg > 1)		/* core dump if serious debugging on */
		abort();
	exit(2);
}

/*!
 *  @brief Reports a warning.
 *
 *  @param[in] fmt printf-style format string.
 *  @param[in] ... Additional format arguments.
 */
void WARNING(const char *fmt, ...)
{
	extern char *cmdname;
	va_list varg;

	fflush(stdout);
	fprintf(stderr, "%s: ", cmdname);
	va_start(varg, fmt);
	vfprintf(stderr, fmt, varg);
	va_end(varg);
	error();
}

/*!
 *  @brief Prints the location of the last error.
 */
void error()
{
	extern Node *curnode;

	fprintf(stderr, "\n");
	if (compile_time != 2 && NR && *NR > 0) {
		fprintf(stderr, " input record number %d", (int) (*FNR));
		if (strcmp(*FILENAME, "-") != 0)
			fprintf(stderr, ", file %s", *FILENAME);
		fprintf(stderr, "\n");
	}
	if (compile_time != 2 && curnode)
		fprintf(stderr, " source line number %d", curnode->lineno);
	else if (compile_time != 2 && lineno)
		fprintf(stderr, " source line number %d", lineno);
	if (compile_time == 1 && cursource() != NULL)
		fprintf(stderr, " source file %s", cursource());
	fprintf(stderr, "\n");
	eprint();
}

/*!
 *  @brief Prints the context around the last error.
 */
void eprint(void)
{
	char *p, *q;
	int c;
	static int been_here = 0;
	extern char ebuf[], *ep;

	if (compile_time == 2 || compile_time == 0 || been_here++ > 0)
		return;
	p = ep - 1;
	if (p > ebuf && *p == '\n')
		p--;
	for ( ; p > ebuf && *p != '\n' && *p != '\0'; p--)
		;
	while (*p == '\n')
		p++;
	fprintf(stderr, " context is\n\t");
	for (q=ep-1; q>=p && *q!=' ' && *q!='\t' && *q!='\n'; q--)
		;
	for ( ; p < q; p++)
		if (*p)
			putc(*p, stderr);
	fprintf(stderr, " >>> ");
	for ( ; p < ep; p++)
		if (*p)
			putc(*p, stderr);
	fprintf(stderr, " <<< ");
	if (*ep)
		while ((c = input()) != '\n' && c != '\0' && c != EOF) {
			putc(c, stderr);
			bclass(c);
		}
	putc('\n', stderr);
	ep = ebuf;
}

/*!
 *  @brief Tracks bracket nesting of a character.
 *
 *  @param[in] c Character to classify.
 */
void bclass(int c)
{
	switch (c) {
	case '{': bracecnt++; break;
	case '}': bracecnt--; break;
	case '[': brackcnt++; break;
	case ']': brackcnt--; break;
	case '(': parencnt++; break;
	case ')': parencnt--; break;
	}
}

/*!
 *  @brief Checks errno after a math library call.
 *
 *  @param[in] x Result of the math call.
 *  @param[in] s Name of the math function.
 *
 *  @return Adjusted result.
 */
double errcheck(double x, const char *s)
{

	if (errno == EDOM) {
		errno = 0;
		WARNING("%s argument out of domain", s);
		x = 1;
	} else if (errno == ERANGE) {
		errno = 0;
		WARNING("%s result out of range", s);
		x = 1;
	}
	return x;
}

/*!
 *  @brief Tests whether a string looks like var=value.
 *
 *  @param[in] s String to test.
 *
 *  @return Test result.
 *  @retval 0 The string is not of the form var=value.
 *  @retval 1 The string is a valid var=value assignment.
 */
int isclvar(const char *s)
{
	const char *os = s;

	if (!isalpha((uschar) *s) && *s != '_')
		return 0;
	for ( ; *s; s++)
		if (!(isalnum((uschar) *s) || *s == '_'))
			break;
	return *s == '=' && s > os && *(s+1) != '=';
}

/* strtod is supposed to be a proper test of what's a valid number */
/* appears to be broken in gcc on linux: thinks 0x123 is a valid FP number */
/* wrong: violates 4.10.1.4 of ansi C standard */

#include <math.h>

/*!
 *  @brief Tests whether a string is a valid number.
 *
 *  @param[in] s String to test.
 *
 *  @return Test result.
 *  @retval 0 The string is not a valid number.
 *  @retval 1 The string is a valid number.
 */
int is_number(const char *s)
{
	double r;
	char *ep;
	errno = 0;
	r = strtod(s, &ep);
	if (ep == s || r == HUGE_VAL || errno == ERANGE)
		return 0;
	while (*ep == ' ' || *ep == '\t' || *ep == '\n')
		ep++;
	if (*ep == '\0')
		return 1;
	else
		return 0;
}
