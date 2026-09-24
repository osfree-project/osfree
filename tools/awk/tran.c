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
 *  @file tran.c
 *  @brief Symbol table and value conversion for awk.
 *
 *  Implements the hash-table symbol table used for variables and
 *  arrays, the built-in variables (FS, RS, NR, NF, ...) and the
 *  conversions between the numeric and string representations of
 *  a Cell.
 *
 *  @copyright Copyright (C) Lucent Technologies 1997.
 */

#define	DEBUG
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "awk.h"
#include "ytab.h"

/*!
 *  @brief Rehash when the table gets this many times full.
 *  @def FULLTAB
 */
#define	FULLTAB	2

/*!
 *  @brief Growth factor used when rehashing.
 *  @def GROWTAB
 */
#define	GROWTAB 4

Array	*symtab;             /*!< Main symbol table. */

char	**FS;                /*!< Initial field separator. */
char	**RS;                /*!< Initial record separator. */
char	**OFS;               /*!< Output field separator. */
char	**ORS;               /*!< Output record separator. */
char	**OFMT;              /*!< Output format for numbers. */
char	**CONVFMT;           /*!< Format for conversions in getsval. */
Awkfloat *NF;                /*!< Number of fields in the current record. */
Awkfloat *NR;                /*!< Number of the current record. */
Awkfloat *FNR;               /*!< Number of the current record in the current file. */
char	**FILENAME;          /*!< Name of the current input file. */
Awkfloat *ARGC;              /*!< Number of command-line arguments. */
char	**SUBSEP;            /*!< Subscript separator for a[i,j,k]. */
Awkfloat *RSTART;            /*!< Start of the last match, origin 1. */
Awkfloat *RLENGTH;           /*!< Length of the last match. */

Cell	*nrloc;              /*!< Cell of the NR variable. */
Cell	*nfloc;              /*!< Cell of the NF variable. */
Cell	*fnrloc;             /*!< Cell of the FNR variable. */
Array	*ARGVtab;            /*!< Symbol table backing ARGV. */
Array	*ENVtab;             /*!< Symbol table backing ENVIRON. */
Cell	*rstartloc;          /*!< Cell of the RSTART variable. */
Cell	*rlengthloc;         /*!< Cell of the RLENGTH variable. */
Cell	*symtabloc;          /*!< Cell of the SYMTAB variable. */

Cell	*nullloc;            /*!< Guaranteed empty cell. */
Node	*nullnode;           /*!< Zero-and-null node used for comparisons. */
Cell	*literal0;           /*!< Cell of the literal zero. */

extern Cell **fldtab;        /*!< Table of fields $0..$NF defined in lib.c. */

/*!
 *  @brief Initializes the symbol table with the built-in variables.
 */
void syminit(void)
{
	literal0 = setsymtab("0", "0", 0.0, NUM|STR|CON|DONTFREE, symtab);
	/* this is used for if(x)... tests: */
	nullloc = setsymtab("$zero&null", "", 0.0, NUM|STR|CON|DONTFREE, symtab);
	nullnode = celltonode(nullloc, CCON);

	FS = &setsymtab("FS", " ", 0.0, STR|DONTFREE, symtab)->sval;
	RS = &setsymtab("RS", "\n", 0.0, STR|DONTFREE, symtab)->sval;
	OFS = &setsymtab("OFS", " ", 0.0, STR|DONTFREE, symtab)->sval;
	ORS = &setsymtab("ORS", "\n", 0.0, STR|DONTFREE, symtab)->sval;
	OFMT = &setsymtab("OFMT", "%.6g", 0.0, STR|DONTFREE, symtab)->sval;
	CONVFMT = &setsymtab("CONVFMT", "%.6g", 0.0, STR|DONTFREE, symtab)->sval;
	FILENAME = &setsymtab("FILENAME", "", 0.0, STR|DONTFREE, symtab)->sval;
	nfloc = setsymtab("NF", "", 0.0, NUM, symtab);
	NF = &nfloc->fval;
	nrloc = setsymtab("NR", "", 0.0, NUM, symtab);
	NR = &nrloc->fval;
	fnrloc = setsymtab("FNR", "", 0.0, NUM, symtab);
	FNR = &fnrloc->fval;
	SUBSEP = &setsymtab("SUBSEP", "\034", 0.0, STR|DONTFREE, symtab)->sval;
	rstartloc = setsymtab("RSTART", "", 0.0, NUM, symtab);
	RSTART = &rstartloc->fval;
	rlengthloc = setsymtab("RLENGTH", "", 0.0, NUM, symtab);
	RLENGTH = &rlengthloc->fval;
	symtabloc = setsymtab("SYMTAB", "", 0.0, ARR, symtab);
	symtabloc->sval = (char *) symtab;
}

/*!
 *  @brief Sets up ARGV and ARGC.
 *
 *  @param[in] ac Argument count.
 *  @param[in] av Argument vector.
 */
void arginit(int ac, char **av)
{
	Cell *cp;
	int i;
	char temp[50];

	ARGC = &setsymtab("ARGC", "", (Awkfloat) ac, NUM, symtab)->fval;
	cp = setsymtab("ARGV", "", 0.0, ARR, symtab);
	ARGVtab = makesymtab(NSYMTAB);	/* could be (int) ARGC as well */
	cp->sval = (char *) ARGVtab;
	for (i = 0; i < ac; i++) {
		sprintf(temp, "%d", i);
		if (is_number(*av))
			setsymtab(temp, *av, atof(*av), STR|NUM, ARGVtab);
		else
			setsymtab(temp, *av, 0.0, STR, ARGVtab);
		av++;
	}
}

/*!
 *  @brief Sets up the ENVIRON variable from the process environment.
 *
 *  @param[in] envp Environment vector.
 */
void envinit(char **envp)
{
	Cell *cp;
	char *p;

	cp = setsymtab("ENVIRON", "", 0.0, ARR, symtab);
	ENVtab = makesymtab(NSYMTAB);
	cp->sval = (char *) ENVtab;
	for ( ; *envp; envp++) {
		if ((p = strchr(*envp, '=')) == NULL)
			continue;
		if( p == *envp ) /* no left hand side name in env string */
			continue;
		*p++ = 0;	/* split into two strings at = */
		if (is_number(p))
			setsymtab(*envp, p, atof(p), STR|NUM, ENVtab);
		else
			setsymtab(*envp, p, 0.0, STR, ENVtab);
		p[-1] = '=';	/* restore in case env is passed down to a shell */
	}
}

/*!
 *  @brief Allocates a new symbol table of size @p n.
 *
 *  @param[in] n Initial number of buckets.
 *
 *  @return Newly allocated symbol table.
 */
Array *makesymtab(int n)
{
	Array *ap;
	Cell **tp;

	ap = (Array *) malloc(sizeof(Array));
	tp = (Cell **) calloc(n, sizeof(Cell *));
	if (ap == NULL || tp == NULL)
		FATAL("out of space in makesymtab");
	ap->nelem = 0;
	ap->size = n;
	ap->tab = tp;
	return(ap);
}

/*!
 *  @brief Frees a symbol table and all of its elements.
 *
 *  @param[in] ap Cell that holds the array.
 */
void freesymtab(Cell *ap)
{
	Cell *cp, *temp;
	Array *tp;
	int i;

	if (!isarr(ap))
		return;
	tp = (Array *) ap->sval;
	if (tp == NULL)
		return;
	for (i = 0; i < tp->size; i++) {
		for (cp = tp->tab[i]; cp != NULL; cp = temp) {
			xfree(cp->nval);
			if (freeable(cp))
				xfree(cp->sval);
			temp = cp->cnext;	/* avoids freeing then using */
			free(cp); 
			tp->nelem--;
		}
		tp->tab[i] = 0;
	}
	if (tp->nelem != 0)
		WARNING("can't happen: inconsistent element count freeing %s", ap->nval);
	free(tp->tab);
	free(tp);
}

/*!
 *  @brief Removes a single element from an array.
 *
 *  @param[in] ap Cell that holds the array.
 *  @param[in] s  Subscript of the element to remove.
 */
void freeelem(Cell *ap, const char *s)
{
	Array *tp;
	Cell *p, *prev = NULL;
	int h;
	
	tp = (Array *) ap->sval;
	h = hash(s, tp->size);
	for (p = tp->tab[h]; p != NULL; prev = p, p = p->cnext)
		if (strcmp(s, p->nval) == 0) {
			if (prev == NULL)	/* 1st one */
				tp->tab[h] = p->cnext;
			else			/* middle somewhere */
				prev->cnext = p->cnext;
			if (freeable(p))
				xfree(p->sval);
			free(p->nval);
			free(p);
			tp->nelem--;
			return;
		}
}

/*!
 *  @brief Finds or creates an entry in a symbol table.
 *
 *  @param[in] n  Name of the entry.
 *  @param[in] s  Initial string value.
 *  @param[in] f  Initial numeric value.
 *  @param[in] t  Type flags.
 *  @param[in] tp Symbol table.
 *
 *  @return Cell of the found or created entry.
 */
Cell *setsymtab(const char *n, const char *s, Awkfloat f, unsigned t, Array *tp)
{
	int h;
	Cell *p;

	if (n != NULL && (p = lookup(n, tp)) != NULL) {
		   dprintf( ("setsymtab found %p: n=%s s=\"%s\" f=%g t=%o\n",
			p, NN(p->nval), NN(p->sval), p->fval, p->tval) );
		return(p);
	}
	p = (Cell *) malloc(sizeof(Cell));
	if (p == NULL)
		FATAL("out of space for symbol table at %s", n);
	p->nval = tostring(n);
	p->sval = s ? tostring(s) : tostring("");
	p->fval = f;
	p->tval = t;
	p->csub = CUNK;
	p->ctype = OCELL;
	tp->nelem++;
	if (tp->nelem > FULLTAB * tp->size)
		rehash(tp);
	h = hash(n, tp->size);
	p->cnext = tp->tab[h];
	tp->tab[h] = p;
	   dprintf( ("setsymtab set %p: n=%s s=\"%s\" f=%g t=%o\n",
		p, p->nval, p->sval, p->fval, p->tval) );
	return(p);
}

/*!
 *  @brief Computes the hash value of a string.
 *
 *  @param[in] s String to hash.
 *  @param[in] n Number of buckets.
 *
 *  @return Hash value in the range [0, n).
 */
int hash(const char *s, int n)
{
	unsigned hashval;

	for (hashval = 0; *s != '\0'; s++)
		hashval = (*s + 31 * hashval);
	return hashval % n;
}

/*!
 *  @brief Grows and rehashes a symbol table.
 *
 *  @param[in] tp Symbol table to grow.
 */
void rehash(Array *tp)
{
	int i, nh, nsz;
	Cell *cp, *op, **np;

	nsz = GROWTAB * tp->size;
	np = (Cell **) calloc(nsz, sizeof(Cell *));
	if (np == NULL)		/* can't do it, but can keep running. */
		return;		/* someone else will run out later. */
	for (i = 0; i < tp->size; i++) {
		for (cp = tp->tab[i]; cp; cp = op) {
			op = cp->cnext;
			nh = hash(cp->nval, nsz);
			cp->cnext = np[nh];
			np[nh] = cp;
		}
	}
	free(tp->tab);
	tp->tab = np;
	tp->size = nsz;
}

/*!
 *  @brief Looks up a name in a symbol table.
 *
 *  @param[in] s  Name to look up.
 *  @param[in] tp Symbol table.
 *
 *  @return Cell of the entry.
 *  @retval NULL The name is not present in the table.
 */
Cell *lookup(const char *s, Array *tp)
{
	Cell *p;
	int h;

	h = hash(s, tp->size);
	for (p = tp->tab[h]; p != NULL; p = p->cnext)
		if (strcmp(s, p->nval) == 0)
			return(p);	/* found it */
	return(NULL);			/* not found */
}

/*!
 *  @brief Sets the numeric value of a cell.
 *
 *  @param[in] vp Cell to modify.
 *  @param[in] f  New numeric value.
 *
 *  @return The new numeric value.
 */
Awkfloat setfval(Cell *vp, Awkfloat f)
{
	int fldno;

	if ((vp->tval & (NUM | STR)) == 0) 
		funnyvar(vp, "assign to");
	if (isfld(vp)) {
		donerec = 0;	/* mark $0 invalid */
		fldno = atoi(vp->nval);
		if (fldno > *NF)
			newfld(fldno);
		   dprintf( ("setting field %d to %g\n", fldno, f) );
	} else if (isrec(vp)) {
		donefld = 0;	/* mark $1... invalid */
		donerec = 1;
	}
	if (freeable(vp))
		xfree(vp->sval); /* free any previous string */
	vp->tval &= ~STR;	/* mark string invalid */
	vp->tval |= NUM;	/* mark number ok */
	   dprintf( ("setfval %p: %s = %g, t=%o\n", vp, NN(vp->nval), f, vp->tval) );
	return vp->fval = f;
}

/*!
 *  @brief Reports an invalid use of a variable or function name.
 *
 *  @param[in] vp Cell being misused.
 *  @param[in] rw Operation being attempted.
 */
void funnyvar(Cell *vp, const char *rw)
{
	if (isarr(vp))
		FATAL("can't %s %s; it's an array name.", rw, vp->nval);
	if (vp->tval & FCN)
		FATAL("can't %s %s; it's a function.", rw, vp->nval);
	WARNING("funny variable %p: n=%s s=\"%s\" f=%g t=%o",
		vp, vp->nval, vp->sval, vp->fval, vp->tval);
}

/*!
 *  @brief Sets the string value of a cell.
 *
 *  @param[in] vp Cell to modify.
 *  @param[in] s  New string value.
 *
 *  @return Pointer to the stored string.
 */
char *setsval(Cell *vp, const char *s)
{
	char *t;
	int fldno;

	   dprintf( ("starting setsval %p: %s = \"%s\", t=%o\n", vp, NN(vp->nval), s, vp->tval) );
	if ((vp->tval & (NUM | STR)) == 0)
		funnyvar(vp, "assign to");
	if (isfld(vp)) {
		donerec = 0;	/* mark $0 invalid */
		fldno = atoi(vp->nval);
		if (fldno > *NF)
			newfld(fldno);
		   dprintf( ("setting field %d to %s (%p)\n", fldno, s, s) );
	} else if (isrec(vp)) {
		donefld = 0;	/* mark $1... invalid */
		donerec = 1;
	}
	t = tostring(s);	/* in case it's self-assign */
	vp->tval &= ~NUM;
	vp->tval |= STR;
	if (freeable(vp))
		xfree(vp->sval);
	vp->tval &= ~DONTFREE;
	   dprintf( ("setsval %p: %s = \"%s (%p)\", t=%o\n", vp, NN(vp->nval), t,t, vp->tval) );
	return(vp->sval = t);
}

/*!
 *  @brief Returns the numeric value of a cell.
 *
 *  @param[in] vp Cell to read.
 *
 *  @return Numeric value of the cell.
 */
Awkfloat getfval(Cell *vp)
{
	if ((vp->tval & (NUM | STR)) == 0)
		funnyvar(vp, "read value of");
	if (isfld(vp) && donefld == 0)
		fldbld();
	else if (isrec(vp) && donerec == 0)
		recbld();
	if (!isnum(vp)) {	/* not a number */
		vp->fval = atof(vp->sval);	/* best guess */
		if (is_number(vp->sval) && !(vp->tval&CON))
			vp->tval |= NUM;	/* make NUM only sparingly */
	}
	   dprintf( ("getfval %p: %s = %g, t=%o\n", vp, NN(vp->nval), vp->fval, vp->tval) );
	return(vp->fval);
}

/*!
 *  @brief Returns the string value of a cell using a chosen format.
 *
 *  @param[in] vp  Cell to read.
 *  @param[in] fmt Format used for non-integral numbers.
 *
 *  @return String value of the cell.
 */
 static char *get_str_val(Cell *vp, char **fmt)        /* get string val of a Cell */

{
	char s[100];	/* BUG: unchecked */
	double dtemp;

	if ((vp->tval & (NUM | STR)) == 0)
		funnyvar(vp, "read value of");
	if (isfld(vp) && donefld == 0)
		fldbld();
	else if (isrec(vp) && donerec == 0)
		recbld();
	if (isstr(vp) == 0) {
		if (freeable(vp))
			xfree(vp->sval);
		if (modf(vp->fval, &dtemp) == 0)	/* it's integral */
			sprintf(s, "%.30g", vp->fval);
		else
			sprintf(s, *fmt, vp->fval);
		vp->sval = tostring(s);
		vp->tval &= ~DONTFREE;
		vp->tval |= STR;
	}
	   dprintf( ("getsval %p: %s = \"%s (%p)\", t=%o\n", vp, NN(vp->nval), vp->sval, vp->sval, vp->tval) );
	return(vp->sval);
}

/*!
 *  @brief Returns the string value of a cell using CONVFMT.
 *
 *  @param[in] vp Cell to read.
 *
 *  @return String value of the cell.
 */
char *getsval(Cell *vp)       /* get string val of a Cell */
{
      return get_str_val(vp, CONVFMT);
}

/*!
 *  @brief Returns the string value of a cell for print using OFMT.
 *
 *  @param[in] vp Cell to read.
 *
 *  @return String value of the cell.
 */
char *getpssval(Cell *vp)     /* get string val of a Cell for print */
{
      return get_str_val(vp, OFMT);
}


/*!
 *  @brief Copies a string into freshly allocated memory.
 *
 *  @param[in] s Source string.
 *
 *  @return Pointer to the newly allocated copy.
 */
char *tostring(const char *s)	/* make a copy of string s */
{
	char *p;

	p = (char *) malloc(strlen(s)+1);
	if (p == NULL)
		FATAL("out of space in tostring on %s", s);
	strcpy(p, s);
	return(p);
}

/*!
 *  @brief Reads a string up to @p delim, decoding escape sequences.
 *
 *  @param[in] is    Source string.
 *  @param[in] delim Terminating delimiter character.
 *
 *  @return Newly allocated decoded string.
 */
char *qstring(const char *is, int delim)	/* collect string up to next delim */
{
	const char *os = is;
	int c, n;
	uschar *s = (uschar *) is;
	uschar *buf, *bp;

	if ((buf = (uschar *) malloc(strlen(is)+3)) == NULL)
		FATAL( "out of space in qstring(%s)", s);
	for (bp = buf; (c = *s) != delim; s++) {
		if (c == '\n')
			SYNTAX( "newline in string %.20s...", os );
		else if (c != '\\')
			*bp++ = c;
		else {	/* \something */
			c = *++s;
			if (c == 0) {	/* \ at end */
				*bp++ = '\\';
				break;	/* for loop */
			}	
			switch (c) {
			case '\\':	*bp++ = '\\'; break;
			case 'n':	*bp++ = '\n'; break;
			case 't':	*bp++ = '\t'; break;
			case 'b':	*bp++ = '\b'; break;
			case 'f':	*bp++ = '\f'; break;
			case 'r':	*bp++ = '\r'; break;
			default:
				if (!isdigit(c)) {
					*bp++ = c;
					break;
				}
				n = c - '0';
				if (isdigit(s[1])) {
					n = 8 * n + *++s - '0';
					if (isdigit(s[1]))
						n = 8 * n + *++s - '0';
				}
				*bp++ = n;
				break;
			}
		}
	}
	*bp++ = 0;
	return (char *) buf;
}
