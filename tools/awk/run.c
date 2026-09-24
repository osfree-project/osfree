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
 *  @file run.c
 *  @brief Tree-walking interpreter for awk.
 *
 *  Executes the parse tree produced by the parser: statements,
 *  expressions, function calls, I/O redirection, substitution and
 *  the built-in functions. Also contains the printf-style formatting
 *  machinery and the file table used for redirection.
 *
 *  @copyright Copyright (C) Lucent Technologies 1997.
 */

#define DEBUG
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "awk.h"
#include "ytab.h"

/*!
 *  @brief Releases a cell if it is a temporary.
 *
 *  @param[in] x Cell to inspect and possibly release.
 *  @def tempfree
 */
#define tempfree(x)	if (istemp(x)) tfree(x); else

/*
 * #undef tempfree
 *
 * void tempfree(Cell *p) {
 * 	if (p->ctype == OCELL && (p->csub < CUNK || p->csub > CFREE)) {
 * 		WARNING("bad csub %d in Cell %d %s",
 * 			p->csub, p->ctype, p->sval);
 * 	}
 * 	if (istemp(p))
 * 		tfree(p);
 * }
 */

#ifdef _NFILE
#ifndef FOPEN_MAX
/*!
 *  @brief Fallback for the maximum number of open files.
 *  @def FOPEN_MAX
 */
#define FOPEN_MAX _NFILE
#endif
#endif

#ifndef	FOPEN_MAX
/*!
 *  @brief Maximum number of open files.
 *  @def FOPEN_MAX
 */
#define	FOPEN_MAX	40
#endif

#ifndef RAND_MAX
/*!
 *  @brief Upper bound of rand() as guaranteed by ANSI.
 *  @def RAND_MAX
 */
#define RAND_MAX	32767
#endif

jmp_buf env;                        /*!< Jump buffer used by exit. */
extern	int	pairstack[];        /*!< State of each pat,pat statement. */

Node	*winner = NULL;             /*!< Root of the parse tree. */
Cell	*tmps;                      /*!< Free list of temporary cells. */

static Cell	truecell	={ OBOOL, BTRUE, 0, 0, 1.0, NUM };           /*!< Prototype cell for boolean true. */
Cell	*True	= &truecell;                                                 /*!< The boolean true cell. */
static Cell	falsecell	={ OBOOL, BFALSE, 0, 0, 0.0, NUM };          /*!< Prototype cell for boolean false. */
Cell	*False	= &falsecell;                                                /*!< The boolean false cell. */
static Cell	breakcell	={ OJUMP, JBREAK, 0, 0, 0.0, NUM };          /*!< Prototype cell for break. */
Cell	*jbreak	= &breakcell;                                                /*!< The break cell. */
static Cell	contcell	={ OJUMP, JCONT, 0, 0, 0.0, NUM };           /*!< Prototype cell for continue. */
Cell	*jcont	= &contcell;                                                 /*!< The continue cell. */
static Cell	nextcell	={ OJUMP, JNEXT, 0, 0, 0.0, NUM };           /*!< Prototype cell for next. */
Cell	*jnext	= &nextcell;                                                 /*!< The next cell. */
static Cell	nextfilecell	={ OJUMP, JNEXTFILE, 0, 0, 0.0, NUM };  /*!< Prototype cell for nextfile. */
Cell	*jnextfile	= &nextfilecell;                                     /*!< The nextfile cell. */
static Cell	exitcell	={ OJUMP, JEXIT, 0, 0, 0.0, NUM };           /*!< Prototype cell for exit. */
Cell	*jexit	= &exitcell;                                                 /*!< The exit cell. */
static Cell	retcell		={ OJUMP, JRET, 0, 0, 0.0, NUM };            /*!< Prototype cell for return. */
Cell	*jret	= &retcell;                                                  /*!< The return cell. */
static Cell	tempcell	={ OCELL, CTEMP, 0, "", 0.0, NUM|STR|DONTFREE }; /*!< Prototype temporary cell. */

Node	*curnode = NULL;            /*!< Node currently being executed (for debugging). */

/*!
 *  @brief Manages a dynamically sized buffer.
 *
 *  @param[in,out] pbuf    Address of the buffer pointer.
 *  @param[in,out] psiz    Address of the buffer size variable.
 *  @param[in]     minlen  Minimum required length.
 *  @param[in]     quantum Buffer size quantum.
 *  @param[in,out] pbptr   Address of a movable pointer into the buffer,
 *                         or NULL if none.
 *  @param[in]     whatrtn Caller name for failure diagnostics.
 *
 *  @return Whether the buffer is usable.
 *  @retval 0 Reallocation failed.
 *  @retval 1 Buffer is large enough.
 */
int adjbuf(char **pbuf, int *psiz, int minlen, int quantum, char **pbptr,
	const char *whatrtn)
{
	if (minlen > *psiz) {
		char *tbuf;
		int rminlen = quantum ? minlen % quantum : 0;
		int boff = pbptr ? *pbptr - *pbuf : 0;
		/* round up to next multiple of quantum */
		if (rminlen)
			minlen += quantum - rminlen;
		tbuf = (char *) realloc(*pbuf, minlen);
		if (tbuf == NULL) {
			if (whatrtn)
				FATAL("out of memory in %s", whatrtn);
			return 0;
		}
		*pbuf = tbuf;
		*psiz = minlen;
		if (pbptr)
			*pbptr = tbuf + boff;
	}
	return 1;
}

/*!
 *  @brief Starts execution of the parse tree.
 *
 *  @param[in] a Root node of the program.
 */
void run(Node *a)
{
	extern void stdinit(void);

	stdinit();
	execute(a);
	closeall();
}

/*!
 *  @brief Executes one node of the parse tree.
 *
 *  @param[in] u Node to execute.
 *
 *  @return Result cell.
 */
Cell *execute(Node *u)
{
	Cell *(*proc)(Node **, int);
	Cell *x;
	Node *a;

	if (u == NULL)
		return(True);
	for (a = u; ; a = a->nnext) {
		curnode = a;
		if (isvalue(a)) {
			x = (Cell *) (a->narg[0]);
			if (isfld(x) && !donefld)
				fldbld();
			else if (isrec(x) && !donerec)
				recbld();
			return(x);
		}
		if (notlegal(a->nobj))	/* probably a Cell* but too risky to print */
			FATAL("illegal statement");
		proc = proctab[a->nobj-FIRSTTOKEN];
		x = (*proc)(a->narg, a->nobj);
		if (isfld(x) && !donefld)
			fldbld();
		else if (isrec(x) && !donerec)
			recbld();
		if (isexpr(a))
			return(x);
		if (isjump(x))
			return(x);
		if (a->nnext == NULL)
			return(x);
		tempfree(x);
	}
}


/*!
 *  @brief Executes the top-level program.
 *
 *  @param[in] a Program children: BEGIN, main body, END.
 *  @param[in] n Token number.
 *
 *  @return True cell.
 */
Cell *program(Node **a, int n)
{
	Cell *x;

	if (setjmp(env) != 0)
		goto ex;
	if (a[0]) {		/* BEGIN */
		x = execute(a[0]);
		if (isexit(x))
			return(True);
		if (isjump(x))
			FATAL("illegal break, continue, next or nextfile from BEGIN");
		tempfree(x);
	}
	if (a[1] || a[2])
		while (getrec(&record, &recsize, 1) > 0) {
			x = execute(a[1]);
			if (isexit(x))
				break;
			tempfree(x);
		}
  ex:
	if (setjmp(env) != 0)	/* handles exit within END */
		goto ex1;
	if (a[2]) {		/* END */
		x = execute(a[2]);
		if (isbreak(x) || isnext(x) || iscont(x))
			FATAL("illegal break, continue, next or nextfile from END");
		tempfree(x);
	}
  ex1:
	return(True);
}

/*!
 *  @brief Stack frame used for awk function calls.
 */
struct Frame {
	int nargs;      /*!< Number of arguments in this call. */
	Cell *fcncell;  /*!< Cell of the function. */
	Cell **args;    /*!< Array of evaluated arguments. */
	Cell *retval;   /*!< Return value. */
};

/*!
 *  @brief Maximum number of arguments in a call.
 *  @def NARGS
 */
#define	NARGS	50

struct Frame *frame = NULL;  /*!< Base of the stack frames; dynamically allocated. */
int	nframe = 0;          /*!< Number of allocated frames. */
struct Frame *fp = NULL;     /*!< Current frame pointer; the bottom level is unused. */

/*!
 *  @brief Executes a function call.
 *
 *  @param[in] a Children of the call node.
 *  @param[in] n Token number.
 *
 *  @return Return value cell.
 */
Cell *call(Node **a, int n)
{
	static Cell newcopycell = { OCELL, CCOPY, 0, "", 0.0, NUM|STR|DONTFREE };
	int i, ncall, ndef;
	int freed = 0; /* handles potential double freeing when fcn & param share a tempcell */
	Node *x;
	Cell *args[NARGS], *oargs[NARGS];	/* BUG: fixed size arrays */
	Cell *y, *z, *fcn;
	char *s;

	fcn = execute(a[0]);	/* the function itself */
	s = fcn->nval;
	if (!isfcn(fcn))
		FATAL("calling undefined function %s", s);
	if (frame == NULL) {
		fp = frame = (struct Frame *) calloc(nframe += 100, sizeof(struct Frame));
		if (frame == NULL)
			FATAL("out of space for stack frames calling %s", s);
	}
	for (ncall = 0, x = a[1]; x != NULL; x = x->nnext)	/* args in call */
		ncall++;
	ndef = (int) fcn->fval;			/* args in defn */
	   dprintf( ("calling %s, %d args (%d in defn), fp=%d\n", s, ncall, ndef, (int) (fp-frame)) );
	if (ncall > ndef)
		WARNING("function %s called with %d args, uses only %d",
			s, ncall, ndef);
	if (ncall + ndef > NARGS)
		FATAL("function %s has %d arguments, limit %d", s, ncall+ndef, NARGS);
	for (i = 0, x = a[1]; x != NULL; i++, x = x->nnext) {	/* get call args */
		   dprintf( ("evaluate args[%d], fp=%d:\n", i, (int) (fp-frame)) );
		y = execute(x);
		oargs[i] = y;
		   dprintf( ("args[%d]: %s %f <%s>, t=%o\n",
			   i, NN(y->nval), y->fval, isarr(y) ? "(array)" : NN(y->sval), y->tval) );
		if (isfcn(y))
			FATAL("can't use function %s as argument in %s", y->nval, s);
		if (isarr(y))
			args[i] = y;	/* arrays by ref */
		else
			args[i] = copycell(y);
		tempfree(y);
	}
	for ( ; i < ndef; i++) {	/* add null args for ones not provided */
		args[i] = gettemp();
		*args[i] = newcopycell;
	}
	fp++;	/* now ok to up frame */
	if (fp >= frame + nframe) {
		int dfp = fp - frame;	/* old index */
		frame = (struct Frame *)
			realloc((char *) frame, (nframe += 100) * sizeof(struct Frame));
		if (frame == NULL)
			FATAL("out of space for stack frames in %s", s);
		fp = frame + dfp;
	}
	fp->fcncell = fcn;
	fp->args = args;
	fp->nargs = ndef;	/* number defined with (excess are locals) */
	fp->retval = gettemp();

	   dprintf( ("start exec of %s, fp=%d\n", s, (int) (fp-frame)) );
	y = execute((Node *)(fcn->sval));	/* execute body */
	   dprintf( ("finished exec of %s, fp=%d\n", s, (int) (fp-frame)) );

	for (i = 0; i < ndef; i++) {
		Cell *t = fp->args[i];
		if (isarr(t)) {
			if (t->csub == CCOPY) {
				if (i >= ncall) {
					freesymtab(t);
					t->csub = CTEMP;
					tempfree(t);
				} else {
					oargs[i]->tval = t->tval;
					oargs[i]->tval &= ~(STR|NUM|DONTFREE);
					oargs[i]->sval = t->sval;
					tempfree(t);
				}
			}
		} else if (t != y) {	/* kludge to prevent freeing twice */
			t->csub = CTEMP;
			tempfree(t);
		} else if (t == y && t->csub == CCOPY) {
			t->csub = CTEMP;
			tempfree(t);
			freed = 1;
		}
	}
	tempfree(fcn);
	if (isexit(y) || isnext(y))
		return y;
	if (freed == 0) {
		tempfree(y);	/* don't free twice! */
	}
	z = fp->retval;			/* return value */
	   dprintf( ("%s returns %g |%s| %o\n", s, getfval(z), getsval(z), z->tval) );
	fp--;
	return(z);
}

/*!
 *  @brief Copies a cell into a temporary.
 *
 *  @param[in] x Cell to copy.
 *
 *  @return New temporary cell.
 */
Cell *copycell(Cell *x)
{
	Cell *y;

	y = gettemp();
	y->csub = CCOPY;	/* prevents freeing until call is over */
	y->nval = x->nval;	/* BUG? */
	if (isstr(x))
		y->sval = tostring(x->sval);
	y->fval = x->fval;
	y->tval = x->tval & ~(CON|FLD|REC|DONTFREE);	/* copy is not constant or field */
							/* is DONTFREE right? */
	return y;
}

/*!
 *  @brief Returns the n-th argument of the current function.
 *
 *  @param[in] a Children of the argument node.
 *  @param[in] n Token number.
 *
 *  @return Argument cell.
 */
Cell *arg(Node **a, int n)
{

	n = ptoi(a[0]);	/* argument number, counting from 0 */
	   dprintf( ("arg(%d), fp->nargs=%d\n", n, fp->nargs) );
	if (n+1 > fp->nargs)
		FATAL("argument #%d of function %s was not supplied",
			n+1, fp->fcncell->nval);
	return fp->args[n];
}

/*!
 *  @brief Executes break, continue, next, nextfile, return and exit.
 *
 *  @param[in] a Children of the jump node.
 *  @param[in] n Token number.
 *
 *  @return Jump cell.
 *  @retval 0 Internal error (unreachable, FATAL is called instead).
 */
Cell *jump(Node **a, int n)
{
	Cell *y;

	switch (n) {
	case EXIT:
		if (a[0] != NULL) {
			y = execute(a[0]);
			errorflag = (int) getfval(y);
			tempfree(y);
		}
		longjmp(env, 1);
	case RETURN:
		if (a[0] != NULL) {
			y = execute(a[0]);
			if ((y->tval & (STR|NUM)) == (STR|NUM)) {
				setsval(fp->retval, getsval(y));
				fp->retval->fval = getfval(y);
				fp->retval->tval |= NUM;
			}
			else if (y->tval & STR)
				setsval(fp->retval, getsval(y));
			else if (y->tval & NUM)
				setfval(fp->retval, getfval(y));
			else		/* can't happen */
				FATAL("bad type variable %d", y->tval);
			tempfree(y);
		}
		return(jret);
	case NEXT:
		return(jnext);
	case NEXTFILE:
		nextfile();
		return(jnextfile);
	case BREAK:
		return(jbreak);
	case CONTINUE:
		return(jcont);
	default:	/* can't happen */
		FATAL("illegal jump type %d", n);
	}
	return 0;	/* not reached */
}

/*!
 *  @brief Reads the next record from a specific input.
 *
 *  @param[in] a Children of the getline node.
 *  @param[in] n Token number.
 *
 *  @return Result cell holding the number of records read.
 */
Cell *mygetline(Node **a, int n)
{
	Cell *r, *x;
	extern Cell **fldtab;
	FILE *fp;
	char *buf;
	int bufsize = recsize;
	int mode;

	if ((buf = (char *) malloc(bufsize)) == NULL)
		FATAL("out of memory in getline");

	fflush(stdout);	/* in case someone is waiting for a prompt */
	r = gettemp();
	if (a[1] != NULL) {		/* getline < file */
		x = execute(a[2]);		/* filename */
		mode = ptoi(a[1]);
		if (mode == '|')		/* input pipe */
			mode = LE;	/* arbitrary flag */
		fp = openfile(mode, getsval(x));
		tempfree(x);
		if (fp == NULL)
			n = -1;
		else
			n = readrec(&buf, &bufsize, fp);
		if (n <= 0) {
			;
		} else if (a[0] != NULL) {	/* getline var <file */
			x = execute(a[0]);
			setsval(x, buf);
			tempfree(x);
		} else {			/* getline <file */
			setsval(fldtab[0], buf);
			if (is_number(fldtab[0]->sval)) {
				fldtab[0]->fval = atof(fldtab[0]->sval);
				fldtab[0]->tval |= NUM;
			}
		}
	} else {			/* bare getline; use current input */
		if (a[0] == NULL)	/* getline */
			n = getrec(&record, &recsize, 1);
		else {			/* getline var */
			n = getrec(&buf, &bufsize, 0);
			x = execute(a[0]);
			setsval(x, buf);
			tempfree(x);
		}
	}
	setfval(r, (Awkfloat) n);
	free(buf);
	return r;
}

/*!
 *  @brief Returns the NF cell after ensuring fields are built.
 *
 *  @param[in] a Children of the NF node.
 *  @param[in] n Token number.
 *
 *  @return Cell of NF.
 */
Cell *getnf(Node **a, int n)
{
	if (donefld == 0)
		fldbld();
	return (Cell *) a[0];
}

/*!
 *  @brief Evaluates an array subscript expression.
 *
 *  @param[in] a Children: a[0] is the symbol table, a[1] is the list
 *               of subscripts.
 *  @param[in] n Token number.
 *
 *  @return Element cell.
 */
Cell *array(Node **a, int n)
{
	Cell *x, *y, *z;
	char *s;
	Node *np;
	char *buf;
	int bufsz = recsize;
	int nsub = strlen(*SUBSEP);

	if ((buf = (char *) malloc(bufsz)) == NULL)
		FATAL("out of memory in array");

	x = execute(a[0]);	/* Cell* for symbol table */
	buf[0] = 0;
	for (np = a[1]; np; np = np->nnext) {
		y = execute(np);	/* subscript */
		s = getsval(y);
		if (!adjbuf(&buf, &bufsz, strlen(buf)+strlen(s)+nsub+1, recsize, 0, 0))
			FATAL("out of memory for %s[%s...]", x->nval, buf);
		strcat(buf, s);
		if (np->nnext)
			strcat(buf, *SUBSEP);
		tempfree(y);
	}
	if (!isarr(x)) {
		   dprintf( ("making %s into an array\n", NN(x->nval)) );
		if (freeable(x))
			xfree(x->sval);
		x->tval &= ~(STR|NUM|DONTFREE);
		x->tval |= ARR;
		x->sval = (char *) makesymtab(NSYMTAB);
	}
	z = setsymtab(buf, "", 0.0, STR|NUM, (Array *) x->sval);
	z->ctype = OCELL;
	z->csub = CVAR;
	tempfree(x);
	free(buf);
	return(z);
}

/*!
 *  @brief Executes the delete statement.
 *
 *  @param[in] a Children: a[0] is the symbol table, a[1] the list of
 *               subscripts.
 *  @param[in] n Token number.
 *
 *  @return True cell.
 */
Cell *awkdelete(Node **a, int n)
{
	Cell *x, *y;
	Node *np;
	char *s;
	int nsub = strlen(*SUBSEP);

	x = execute(a[0]);	/* Cell* for symbol table */
	if (!isarr(x))
		return True;
	if (a[1] == 0) {	/* delete the elements, not the table */
		freesymtab(x);
		x->tval &= ~STR;
		x->tval |= ARR;
		x->sval = (char *) makesymtab(NSYMTAB);
	} else {
		int bufsz = recsize;
		char *buf;
		if ((buf = (char *) malloc(bufsz)) == NULL)
			FATAL("out of memory in adelete");
		buf[0] = 0;
		for (np = a[1]; np; np = np->nnext) {
			y = execute(np);	/* subscript */
			s = getsval(y);
			if (!adjbuf(&buf, &bufsz, strlen(buf)+strlen(s)+nsub+1, recsize, 0, 0))
				FATAL("out of memory deleting %s[%s...]", x->nval, buf);
			strcat(buf, s);	
			if (np->nnext)
				strcat(buf, *SUBSEP);
			tempfree(y);
		}
		freeelem(x, buf);
		free(buf);
	}
	tempfree(x);
	return True;
}

/*!
 *  @brief Executes the "in" test for array subscripts.
 *
 *  @param[in] a Children: a[0] is the index list, a[1] is the array.
 *  @param[in] n Token number.
 *
 *  @return Boolean result cell.
 */
Cell *intest(Node **a, int n)
{
	Cell *x, *ap, *k;
	Node *p;
	char *buf;
	char *s;
	int bufsz = recsize;
	int nsub = strlen(*SUBSEP);

	ap = execute(a[1]);	/* array name */
	if (!isarr(ap)) {
		   dprintf( ("making %s into an array\n", ap->nval) );
		if (freeable(ap))
			xfree(ap->sval);
		ap->tval &= ~(STR|NUM|DONTFREE);
		ap->tval |= ARR;
		ap->sval = (char *) makesymtab(NSYMTAB);
	}
	if ((buf = (char *) malloc(bufsz)) == NULL) {
		FATAL("out of memory in intest");
	}
	buf[0] = 0;
	for (p = a[0]; p; p = p->nnext) {
		x = execute(p);	/* expr */
		s = getsval(x);
		if (!adjbuf(&buf, &bufsz, strlen(buf)+strlen(s)+nsub+1, recsize, 0, 0))
			FATAL("out of memory deleting %s[%s...]", x->nval, buf);
		strcat(buf, s);
		tempfree(x);
		if (p->nnext)
			strcat(buf, *SUBSEP);
	}
	k = lookup(buf, (Array *) ap->sval);
	tempfree(ap);
	free(buf);
	if (k == NULL)
		return(False);
	else
		return(True);
}


/*!
 *  @brief Executes the ~ and !~ operators and match().
 *
 *  @param[in] a Children of the match node.
 *  @param[in] n Token number.
 *
 *  @return Boolean result, or, for MATCHFCN, the start position.
 */
Cell *matchop(Node **a, int n)
{
	Cell *x, *y;
	char *s, *t;
	int i;
	fa *pfa;
	int (*mf)(fa *, const char *) = match, mode = 0;

	if (n == MATCHFCN) {
		mf = pmatch;
		mode = 1;
	}
	x = execute(a[1]);	/* a[1] = target text */
	s = getsval(x);
	if (a[0] == 0)		/* a[1] == 0: already-compiled reg expr */
		i = (*mf)((fa *) a[2], s);
	else {
		y = execute(a[2]);	/* a[2] = regular expr */
		t = getsval(y);
		pfa = makedfa(t, mode);
		i = (*mf)(pfa, s);
		tempfree(y);
	}
	tempfree(x);
	if (n == MATCHFCN) {
		int start = patbeg - s + 1;
		if (patlen < 0)
			start = 0;
		setfval(rstartloc, (Awkfloat) start);
		setfval(rlengthloc, (Awkfloat) patlen);
		x = gettemp();
		x->tval = NUM;
		x->fval = start;
		return x;
	} else if ((n == MATCH && i == 1) || (n == NOTMATCH && i == 0))
		return(True);
	else
		return(False);
}


/*!
 *  @brief Executes boolean operators ||, && and !.
 *
 *  @param[in] a Children of the boolean node.
 *  @param[in] n Token number.
 *
 *  @return Boolean result cell.
 *  @retval 0 Internal error (unreachable, FATAL is called instead).
 */
Cell *boolop(Node **a, int n)
{
	Cell *x, *y;
	int i;

	x = execute(a[0]);
	i = istrue(x);
	tempfree(x);
	switch (n) {
	case BOR:
		if (i) return(True);
		y = execute(a[1]);
		i = istrue(y);
		tempfree(y);
		if (i) return(True);
		else return(False);
	case AND:
		if ( !i ) return(False);
		y = execute(a[1]);
		i = istrue(y);
		tempfree(y);
		if (i) return(True);
		else return(False);
	case NOT:
		if (i) return(False);
		else return(True);
	default:	/* can't happen */
		FATAL("unknown boolean operator %d", n);
	}
	return 0;	/*NOTREACHED*/
}

/*!
 *  @brief Executes relational operators.
 *
 *  @param[in] a Children of the relational node.
 *  @param[in] n Token number.
 *
 *  @return Boolean result cell.
 *  @retval 0 Internal error (unreachable, FATAL is called instead).
 */
Cell *relop(Node **a, int n)
{
	int i;
	Cell *x, *y;
	Awkfloat j;

	x = execute(a[0]);
	y = execute(a[1]);
	if (x->tval&NUM && y->tval&NUM) {
		j = x->fval - y->fval;
		i = j<0? -1: (j>0? 1: 0);
	} else {
		i = strcmp(getsval(x), getsval(y));
	}
	tempfree(x);
	tempfree(y);
	switch (n) {
	case LT:	if (i<0) return(True);
			else return(False);
	case LE:	if (i<=0) return(True);
			else return(False);
	case NE:	if (i!=0) return(True);
			else return(False);
	case EQ:	if (i == 0) return(True);
			else return(False);
	case GE:	if (i>=0) return(True);
			else return(False);
	case GT:	if (i>0) return(True);
			else return(False);
	default:	/* can't happen */
		FATAL("unknown relational operator %d", n);
	}
	return 0;	/*NOTREACHED*/
}

/*!
 *  @brief Releases a temporary cell.
 *
 *  @param[in] a Cell to release.
 */
void tfree(Cell *a)
{
	if (freeable(a)) {
		   dprintf( ("freeing %s %s %o\n", NN(a->nval), NN(a->sval), a->tval) );
		xfree(a->sval);
	}
	if (a == tmps)
		FATAL("tempcell list is curdled");
	a->cnext = tmps;
	tmps = a;
}

/*!
 *  @brief Allocates a temporary cell from the free list.
 *
 *  @return New temporary cell.
 */
Cell *gettemp(void)
{	int i;
	Cell *x;

	if (!tmps) {
		tmps = (Cell *) calloc(100, sizeof(Cell));
		if (!tmps)
			FATAL("out of space for temporaries");
		for(i = 1; i < 100; i++)
			tmps[i-1].cnext = &tmps[i];
		tmps[i-1].cnext = 0;
	}
	x = tmps;
	tmps = x->cnext;
	*x = tempcell;
	return(x);
}

/*!
 *  @brief Executes the $(expr) operator.
 *
 *  @param[in] a Children of the indirect node.
 *  @param[in] n Token number.
 *
 *  @return Field cell.
 */
Cell *indirect(Node **a, int n)
{
	Cell *x;
	int m;
	char *s;

	x = execute(a[0]);
	m = (int) getfval(x);
	if (m == 0 && !is_number(s = getsval(x)))	/* suspicion! */
		FATAL("illegal field $(%s), name \"%s\"", s, x->nval);
		/* BUG: can x->nval ever be null??? */
	tempfree(x);
	x = fieldadr(m);
	x->ctype = OCELL;	/* BUG?  why are these needed? */
	x->csub = CFLD;
	return(x);
}

/*!
 *  @brief Executes substr(a[0], a[1], a[2]).
 *
 *  @param[in] a Children of the substr node.
 *  @param[in] nnn Token number.
 *
 *  @return Result string cell.
 */
Cell *substr(Node **a, int nnn)
{
	int k, m, n;
	char *s;
	int temp;
	Cell *x, *y, *z = 0;

	x = execute(a[0]);
	y = execute(a[1]);
	if (a[2] != 0)
		z = execute(a[2]);
	s = getsval(x);
	k = strlen(s) + 1;
	if (k <= 1) {
		tempfree(x);
		tempfree(y);
		if (a[2] != 0) {
			tempfree(z);
		}
		x = gettemp();
		setsval(x, "");
		return(x);
	}
	m = (int) getfval(y);
	if (m <= 0)
		m = 1;
	else if (m > k)
		m = k;
	tempfree(y);
	if (a[2] != 0) {
		n = (int) getfval(z);
		tempfree(z);
	} else
		n = k - 1;
	if (n < 0)
		n = 0;
	else if (n > k - m)
		n = k - m;
	   dprintf( ("substr: m=%d, n=%d, s=%s\n", m, n, s) );
	y = gettemp();
	temp = s[n+m-1];	/* with thanks to John Linderman */
	s[n+m-1] = '\0';
	setsval(y, s + m - 1);
	s[n+m-1] = temp;
	tempfree(x);
	return(y);
}

/*!
 *  @brief Executes index(a[0], a[1]).
 *
 *  @param[in] a   Children of the index node.
 *  @param[in] nnn Token number.
 *
 *  @return Result cell with the 1-based position, or 0.
 */
Cell *sindex(Node **a, int nnn)
{
	Cell *x, *y, *z;
	char *s1, *s2, *p1, *p2, *q;
	Awkfloat v = 0.0;

	x = execute(a[0]);
	s1 = getsval(x);
	y = execute(a[1]);
	s2 = getsval(y);

	z = gettemp();
	for (p1 = s1; *p1 != '\0'; p1++) {
		for (q=p1, p2=s2; *p2 != '\0' && *q == *p2; q++, p2++)
			;
		if (*p2 == '\0') {
			v = (Awkfloat) (p1 - s1 + 1);	/* origin 1 */
			break;
		}
	}
	tempfree(x);
	tempfree(y);
	setfval(z, v);
	return(z);
}

/*!
 *  @brief Maximum size of a numeric field when formatting.
 *  @def MAXNUMSIZE
 */
#define	MAXNUMSIZE	50

/*!
 *  @brief printf-style formatting routine.
 *
 *  @param[in,out] pbuf     Pointer to the output buffer.
 *  @param[in,out] pbufsize Pointer to the output buffer size.
 *  @param[in]     s        Format string.
 *  @param[in]     a        Argument list.
 *
 *  @return Number of bytes written.
 */
int format(char **pbuf, int *pbufsize, const char *s, Node *a)
{
	char *fmt;
	char *p, *t;
	const char *os;
	Cell *x;
	int flag = 0, n;
	int fmtwd; /* format width */
	int fmtsz = recsize;
	char *buf = *pbuf;
	int bufsize = *pbufsize;

	os = s;
	p = buf;
	if ((fmt = (char *) malloc(fmtsz)) == NULL)
		FATAL("out of memory in format()");
	while (*s) {
		adjbuf(&buf, &bufsize, MAXNUMSIZE+1+p-buf, recsize, &p, "format");
		if (*s != '%') {
			*p++ = *s++;
			continue;
		}
		if (*(s+1) == '%') {
			*p++ = '%';
			s += 2;
			continue;
		}
		/* have to be real careful in case this is a huge number, eg, %100000d */
		fmtwd = atoi(s+1);
		if (fmtwd < 0)
			fmtwd = -fmtwd;
		adjbuf(&buf, &bufsize, fmtwd+1+p-buf, recsize, &p, "format");
		for (t = fmt; (*t++ = *s) != '\0'; s++) {
			if (!adjbuf(&fmt, &fmtsz, MAXNUMSIZE+1+t-fmt, recsize, &t, 0))
				FATAL("format item %.30s... ran format() out of memory", os);
			if (isalpha((uschar)*s) && *s != 'l' && *s != 'h' && *s != 'L')
				break;	/* the ansi panoply */
			if (*s == '*') {
				x = execute(a);
				a = a->nnext;
				sprintf(t-1, "%d", fmtwd=(int) getfval(x));
				if (fmtwd < 0)
					fmtwd = -fmtwd;
				adjbuf(&buf, &bufsize, fmtwd+1+p-buf, recsize, &p, "format");
				t = fmt + strlen(fmt);
				tempfree(x);
			}
		}
		*t = '\0';
		if (fmtwd < 0)
			fmtwd = -fmtwd;
		adjbuf(&buf, &bufsize, fmtwd+1+p-buf, recsize, &p, "format");

		switch (*s) {
		case 'f': case 'e': case 'g': case 'E': case 'G':
			flag = 'f';
			break;
		case 'd': case 'i':
			flag = 'd';
			if(*(s-1) == 'l') break;
			*(t-1) = 'l';
			*t = 'd';
			*++t = '\0';
			break;
		case 'o': case 'x': case 'X': case 'u':
			flag = *(s-1) == 'l' ? 'd' : 'u';
			break;
		case 's':
			flag = 's';
			break;
		case 'c':
			flag = 'c';
			break;
		default:
			WARNING("weird printf conversion %s", fmt);
			flag = '?';
			break;
		}
		if (a == NULL)
			FATAL("not enough args in printf(%s)", os);
		x = execute(a);
		a = a->nnext;
		n = MAXNUMSIZE;
		if (fmtwd > n)
			n = fmtwd;
		adjbuf(&buf, &bufsize, 1+n+p-buf, recsize, &p, "format");
		switch (flag) {
		case '?':	sprintf(p, "%s", fmt);	/* unknown, so dump it too */
			t = getsval(x);
			n = strlen(t);
			if (fmtwd > n)
				n = fmtwd;
			adjbuf(&buf, &bufsize, 1+strlen(p)+n+p-buf, recsize, &p, "format");
			p += strlen(p);
			sprintf(p, "%s", t);
			break;
		case 'f':	sprintf(p, fmt, getfval(x)); break;
		case 'd':	sprintf(p, fmt, (long) getfval(x)); break;
		case 'u':	sprintf(p, fmt, (int) getfval(x)); break;
		case 's':
			t = getsval(x);
			n = strlen(t);
			if (fmtwd > n)
				n = fmtwd;
			if (!adjbuf(&buf, &bufsize, 1+n+p-buf, recsize, &p, 0))
				FATAL("huge string/format (%d chars) in printf %.30s... ran format() out of memory", n, t);
			sprintf(p, fmt, t);
			break;
		case 'c':
			if (isnum(x)) {
				if (getfval(x))
					sprintf(p, fmt, (int) getfval(x));
				else {
					*p++ = '\0'; /* explicit null byte */
					*p = '\0';   /* next output will start here */
				}
			} else
				sprintf(p, fmt, getsval(x)[0]);
			break;
		default:
			FATAL("can't happen: bad conversion %c in format()", flag);
		}
		tempfree(x);
		p += strlen(p);
		s++;
	}
	*p = '\0';
	free(fmt);
	for ( ; a; a = a->nnext)		/* evaluate any remaining args */
		execute(a);
	*pbuf = buf;
	*pbufsize = bufsize;
	return p - buf;
}

/*!
 *  @brief Executes sprintf(a[0]).
 *
 *  @param[in] a Children of the sprintf node.
 *  @param[in] n Token number.
 *
 *  @return Result string cell.
 */
Cell *awksprintf(Node **a, int n)
{
	Cell *x;
	Node *y;
	char *buf;
	int bufsz=3*recsize;

	if ((buf = (char *) malloc(bufsz)) == NULL)
		FATAL("out of memory in awksprintf");
	y = a[0]->nnext;
	x = execute(a[0]);
	if (format(&buf, &bufsz, getsval(x), y) == -1)
		FATAL("sprintf string %.30s... too long.  can't happen.", buf);
	tempfree(x);
	x = gettemp();
	x->sval = buf;
	x->tval = STR;
	return(x);
}

/*!
 *  @brief Executes printf.
 *
 *  @param[in] a Children: a[0] format args, a[1] redirection, a[2] file.
 *  @param[in] n Token number.
 *
 *  @return True cell.
 */
Cell *awkprintf(Node **a, int n)
{
	FILE *fp;
	Cell *x;
	Node *y;
	char *buf;
	int len;
	int bufsz=3*recsize;

	if ((buf = (char *) malloc(bufsz)) == NULL)
		FATAL("out of memory in awkprintf");
	y = a[0]->nnext;
	x = execute(a[0]);
	if ((len = format(&buf, &bufsz, getsval(x), y)) == -1)
		FATAL("printf string %.30s... too long.  can't happen.", buf);
	tempfree(x);
	if (a[1] == NULL) {
		/* fputs(buf, stdout); */
		fwrite(buf, len, 1, stdout);
		if (ferror(stdout))
			FATAL("write error on stdout");
	} else {
		fp = redirect(ptoi(a[1]), a[2]);
		/* fputs(buf, fp); */
		fwrite(buf, len, 1, fp);
		fflush(fp);
		if (ferror(fp))
			FATAL("write error on %s", filename(fp));
	}
	free(buf);
	return(True);
}

/*!
 *  @brief Executes arithmetic operators (+, -, *, /, %, unary -, **).
 *
 *  @param[in] a Children of the arithmetic node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
Cell *arith(Node **a, int n)
{
	Awkfloat i, j = 0;
	double v;
	Cell *x, *y, *z;

	x = execute(a[0]);
	i = getfval(x);
	tempfree(x);
	if (n != UMINUS) {
		y = execute(a[1]);
		j = getfval(y);
		tempfree(y);
	}
	z = gettemp();
	switch (n) {
	case ADD:
		i += j;
		break;
	case MINUS:
		i -= j;
		break;
	case MULT:
		i *= j;
		break;
	case DIVIDE:
		if (j == 0)
			FATAL("division by zero");
		i /= j;
		break;
	case MOD:
		if (j == 0)
			FATAL("division by zero in mod");
		modf(i/j, &v);
		i = i - j * v;
		break;
	case UMINUS:
		i = -i;
		break;
	case POWER:
		if (j >= 0 && modf(j, &v) == 0.0)	/* pos integer exponent */
			i = ipow(i, (int) j);
		else
			i = errcheck(pow(i, j), "pow");
		break;
	default:	/* can't happen */
		FATAL("illegal arithmetic operator %d", n);
	}
	setfval(z, i);
	return(z);
}

/*!
 *  @brief Raises a value to an integer power.
 *
 *  @param[in] x Base.
 *  @param[in] n Exponent.
 *
 *  @return x to the power n.
 *  @retval 1 Base case when the exponent is not positive.
 */
double ipow(double x, int n)
{
	double v;

	if (n <= 0)
		return 1;
	v = ipow(x, n/2);
	if (n % 2 == 0)
		return v * v;
	else
		return x * v * v;
}

/*!
 *  @brief Executes ++ and --.
 *
 *  @param[in] a Children of the increment node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
Cell *incrdecr(Node **a, int n)
{
	Cell *x, *z;
	int k;
	Awkfloat xf;

	x = execute(a[0]);
	xf = getfval(x);
	k = (n == PREINCR || n == POSTINCR) ? 1 : -1;
	if (n == PREINCR || n == PREDECR) {
		setfval(x, xf + k);
		return(x);
	}
	z = gettemp();
	setfval(z, xf);
	setfval(x, xf + k);
	tempfree(x);
	return(z);
}

/*!
 *  @brief Executes assignment and compound assignment.
 *
 *  @param[in] a Children of the assignment node.
 *  @param[in] n Token number.
 *
 *  @return Assigned cell.
 */
Cell *assign(Node **a, int n)
{
	Cell *x, *y;
	Awkfloat xf, yf;
	double v;

	y = execute(a[1]);
	x = execute(a[0]);
	if (n == ASSIGN) {	/* ordinary assignment */
		if (x == y && !(x->tval & (FLD|REC)))	/* self-assignment: */
			;		/* leave alone unless it's a field */
		else if ((y->tval & (STR|NUM)) == (STR|NUM)) {
			setsval(x, getsval(y));
			x->fval = getfval(y);
			x->tval |= NUM;
		}
		else if (isstr(y))
			setsval(x, getsval(y));
		else if (isnum(y))
			setfval(x, getfval(y));
		else
			funnyvar(y, "read value of");
		tempfree(y);
		return(x);
	}
	xf = getfval(x);
	yf = getfval(y);
	switch (n) {
	case ADDEQ:
		xf += yf;
		break;
	case SUBEQ:
		xf -= yf;
		break;
	case MULTEQ:
		xf *= yf;
		break;
	case DIVEQ:
		if (yf == 0)
			FATAL("division by zero in /=");
		xf /= yf;
		break;
	case MODEQ:
		if (yf == 0)
			FATAL("division by zero in %%=");
		modf(xf/yf, &v);
		xf = xf - yf * v;
		break;
	case POWEQ:
		if (yf >= 0 && modf(yf, &v) == 0.0)	/* pos integer exponent */
			xf = ipow(xf, (int) yf);
		else
			xf = errcheck(pow(xf, yf), "pow");
		break;
	default:
		FATAL("illegal assignment operator %d", n);
		break;
	}
	tempfree(y);
	setfval(x, xf);
	return(x);
}

/*!
 *  @brief Executes string concatenation.
 *
 *  @param[in] a Children of the concatenation node.
 *  @param[in] q Token number.
 *
 *  @return Result string cell.
 */
Cell *cat(Node **a, int q)
{
	Cell *x, *y, *z;
	int n1, n2;
	char *s;

	x = execute(a[0]);
	y = execute(a[1]);
	getsval(x);
	getsval(y);
	n1 = strlen(x->sval);
	n2 = strlen(y->sval);
	s = (char *) malloc(n1 + n2 + 1);
	if (s == NULL)
		FATAL("out of space concatenating %.15s... and %.15s...",
			x->sval, y->sval);
	strcpy(s, x->sval);
	strcpy(s+n1, y->sval);
	tempfree(y);
	z = gettemp();
	z->sval = s;
	z->tval = STR;
	tempfree(x);
	return(z);
}

/*!
 *  @brief Executes a pattern-action statement.
 *
 *  @param[in] a Children: a[0] pattern, a[1] body.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
Cell *pastat(Node **a, int n)
{
	Cell *x;

	if (a[0] == 0)
		x = execute(a[1]);
	else {
		x = execute(a[0]);
		if (istrue(x)) {
			tempfree(x);
			x = execute(a[1]);
		}
	}
	return x;
}

/*!
 *  @brief Executes a pat,pat action statement.
 *
 *  @param[in] a Children: a[0], a[1] patterns, a[2] body, a[3] pair index.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
Cell *dopa2(Node **a, int n)
{
	Cell *x;
	int pair;

	pair = ptoi(a[3]);
	if (pairstack[pair] == 0) {
		x = execute(a[0]);
		if (istrue(x))
			pairstack[pair] = 1;
		tempfree(x);
	}
	if (pairstack[pair] == 1) {
		x = execute(a[1]);
		if (istrue(x))
			pairstack[pair] = 0;
		tempfree(x);
		x = execute(a[2]);
		return(x);
	}
	return(False);
}

/*!
 *  @brief Executes split(a[0], a[1], a[2]).
 *
 *  @param[in] a   Children of the split node; a[3] is the type.
 *  @param[in] nnn Token number.
 *
 *  @return Result cell with the number of elements.
 */
Cell *split(Node **a, int nnn)
{
	Cell *x = 0, *y, *ap;
	char *s;
	int sep;
	char *t, temp, num[50], *fs = 0;
	int n, tempstat, arg3type;

	y = execute(a[0]);	/* source string */
	s = getsval(y);
	arg3type = ptoi(a[3]);
	if (a[2] == 0)		/* fs string */
		fs = *FS;
	else if (arg3type == STRING) {	/* split(str,arr,"string") */
		x = execute(a[2]);
		fs = getsval(x);
	} else if (arg3type == REGEXPR)
		fs = "(regexpr)";	/* split(str,arr,/regexpr/) */
	else
		FATAL("illegal type of split");
	sep = *fs;
	ap = execute(a[1]);	/* array name */
	freesymtab(ap);
	   dprintf( ("split: s=|%s|, a=%s, sep=|%s|\n", s, NN(ap->nval), fs) );
	ap->tval &= ~STR;
	ap->tval |= ARR;
	ap->sval = (char *) makesymtab(NSYMTAB);

	n = 0;
	if (*s != '\0' && (strlen(fs) > 1 || arg3type == REGEXPR)) {	/* reg expr */
		fa *pfa;
		if (arg3type == REGEXPR) {	/* it's ready already */
			pfa = (fa *) a[2];
		} else {
			pfa = makedfa(fs, 1);
		}
		if (nematch(pfa,s)) {
			tempstat = pfa->initstat;
			pfa->initstat = 2;
			do {
				n++;
				sprintf(num, "%d", n);
				temp = *patbeg;
				*patbeg = '\0';
				if (is_number(s))
					setsymtab(num, s, atof(s), STR|NUM, (Array *) ap->sval);
				else
					setsymtab(num, s, 0.0, STR, (Array *) ap->sval);
				*patbeg = temp;
				s = patbeg + patlen;
				if (*(patbeg+patlen-1) == 0 || *s == 0) {
					n++;
					sprintf(num, "%d", n);
					setsymtab(num, "", 0.0, STR, (Array *) ap->sval);
					pfa->initstat = tempstat;
					goto spdone;
				}
			} while (nematch(pfa,s));
		}
		n++;
		sprintf(num, "%d", n);
		if (is_number(s))
			setsymtab(num, s, atof(s), STR|NUM, (Array *) ap->sval);
		else
			setsymtab(num, s, 0.0, STR, (Array *) ap->sval);
  spdone:
		pfa = NULL;
	} else if (sep == ' ') {
		for (n = 0; ; ) {
			while (*s == ' ' || *s == '\t' || *s == '\n')
				s++;
			if (*s == 0)
				break;
			n++;
			t = s;
			do
				s++;
			while (*s!=' ' && *s!='\t' && *s!='\n' && *s!='\0');
			temp = *s;
			*s = '\0';
			sprintf(num, "%d", n);
			if (is_number(t))
				setsymtab(num, t, atof(t), STR|NUM, (Array *) ap->sval);
			else
				setsymtab(num, t, 0.0, STR, (Array *) ap->sval);
			*s = temp;
			if (*s != 0)
				s++;
		}
	} else if (sep == 0) {	/* new: split(s, a, "") => 1 char/elem */
		for (n = 0; *s != 0; s++) {
			char buf[2];
			n++;
			sprintf(num, "%d", n);
			buf[0] = *s;
			buf[1] = 0;
			if (isdigit((uschar)buf[0]))
				setsymtab(num, buf, atof(buf), STR|NUM, (Array *) ap->sval);
			else
				setsymtab(num, buf, 0.0, STR, (Array *) ap->sval);
		}
	} else if (*s != 0) {
		for (;;) {
			n++;
			t = s;
			while (*s != sep && *s != '\n' && *s != '\0')
				s++;
			temp = *s;
			*s = '\0';
			sprintf(num, "%d", n);
			if (is_number(t))
				setsymtab(num, t, atof(t), STR|NUM, (Array *) ap->sval);
			else
				setsymtab(num, t, 0.0, STR, (Array *) ap->sval);
			*s = temp;
			if (*s++ == 0)
				break;
		}
	}
	tempfree(ap);
	tempfree(y);
	if (a[2] != 0 && arg3type == STRING) {
		tempfree(x);
	}
	x = gettemp();
	x->tval = NUM;
	x->fval = n;
	return(x);
}

/*!
 *  @brief Executes the ternary operator a[0] ? a[1] : a[2].
 *
 *  @param[in] a Children of the conditional node.
 *  @param[in] n Token number.
 *
 *  @return Result cell of the chosen branch.
 */
Cell *condexpr(Node **a, int n)
{
	Cell *x;

	x = execute(a[0]);
	if (istrue(x)) {
		tempfree(x);
		x = execute(a[1]);
	} else {
		tempfree(x);
		x = execute(a[2]);
	}
	return(x);
}

/*!
 *  @brief Executes if (a[0]) a[1]; else a[2].
 *
 *  @param[in] a Children of the if node.
 *  @param[in] n Token number.
 *
 *  @return Result cell of the taken branch.
 */
Cell *ifstat(Node **a, int n)
{
	Cell *x;

	x = execute(a[0]);
	if (istrue(x)) {
		tempfree(x);
		x = execute(a[1]);
	} else if (a[2] != 0) {
		tempfree(x);
		x = execute(a[2]);
	}
	return(x);
}

/*!
 *  @brief Executes while (a[0]) a[1].
 *
 *  @param[in] a Children of the while node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
Cell *whilestat(Node **a, int n)
{
	Cell *x;

	for (;;) {
		x = execute(a[0]);
		if (!istrue(x))
			return(x);
		tempfree(x);
		x = execute(a[1]);
		if (isbreak(x)) {
			x = True;
			return(x);
		}
		if (isnext(x) || isexit(x) || isret(x))
			return(x);
		tempfree(x);
	}
}

/*!
 *  @brief Executes do a[0]; while (a[1]).
 *
 *  @param[in] a Children of the do node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
Cell *dostat(Node **a, int n)
{
	Cell *x;

	for (;;) {
		x = execute(a[0]);
		if (isbreak(x))
			return True;
		if (isnext(x) || isexit(x) || isret(x))
			return(x);
		tempfree(x);
		x = execute(a[1]);
		if (!istrue(x))
			return(x);
		tempfree(x);
	}
}

/*!
 *  @brief Executes for (a[0]; a[1]; a[2]) a[3].
 *
 *  @param[in] a Children of the for node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
Cell *forstat(Node **a, int n)
{
	Cell *x;

	x = execute(a[0]);
	tempfree(x);
	for (;;) {
		if (a[1]!=0) {
			x = execute(a[1]);
			if (!istrue(x)) return(x);
			else tempfree(x);
		}
		x = execute(a[3]);
		if (isbreak(x))		/* turn off break */
			return True;
		if (isnext(x) || isexit(x) || isret(x))
			return(x);
		tempfree(x);
		x = execute(a[2]);
		tempfree(x);
	}
}

/*!
 *  @brief Executes for (a[0] in a[1]) a[2].
 *
 *  @param[in] a Children of the in-array node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
Cell *instat(Node **a, int n)
{
	Cell *x, *vp, *arrayp, *cp, *ncp;
	Array *tp;
	int i;

	vp = execute(a[0]);
	arrayp = execute(a[1]);
	if (!isarr(arrayp)) {
		return True;
	}
	tp = (Array *) arrayp->sval;
	tempfree(arrayp);
	for (i = 0; i < tp->size; i++) {	/* this routine knows too much */
		for (cp = tp->tab[i]; cp != NULL; cp = ncp) {
			setsval(vp, cp->nval);
			ncp = cp->cnext;
			x = execute(a[2]);
			if (isbreak(x)) {
				tempfree(vp);
				return True;
			}
			if (isnext(x) || isexit(x) || isret(x)) {
				tempfree(vp);
				return(x);
			}
			tempfree(x);
		}
	}
	return True;
}

/*!
 *  @brief Executes a built-in function.
 *
 *  @param[in] a Children: a[0] function type, a[1] argument list.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
Cell *bltin(Node **a, int n)
{
	Cell *x, *y;
	Awkfloat u;
	int t;
	char *p, *buf;
	Node *nextarg;
	FILE *fp;
	void flush_all(void);

	t = ptoi(a[0]);
	x = execute(a[1]);
	nextarg = a[1]->nnext;
	switch (t) {
	case FLENGTH:
		if (isarr(x))
			u = ((Array *) x->sval)->nelem;	/* GROT.  should be function*/
		else
			u = strlen(getsval(x));
		break;
	case FLOG:
		u = errcheck(log(getfval(x)), "log"); break;
	case FINT:
		modf(getfval(x), &u); break;
	case FEXP:
		u = errcheck(exp(getfval(x)), "exp"); break;
	case FSQRT:
		u = errcheck(sqrt(getfval(x)), "sqrt"); break;
	case FSIN:
		u = sin(getfval(x)); break;
	case FCOS:
		u = cos(getfval(x)); break;
	case FATAN:
		if (nextarg == 0) {
			WARNING("atan2 requires two arguments; returning 1.0");
			u = 1.0;
		} else {
			y = execute(a[1]->nnext);
			u = atan2(getfval(x), getfval(y));
			tempfree(y);
			nextarg = nextarg->nnext;
		}
		break;
	case FSYSTEM:
		fflush(stdout);		/* in case something is buffered already */
		u = (Awkfloat) system(getsval(x)) / 256;   /* 256 is unix-dep */
		break;
	case FRAND:
		/* in principle, rand() returns something in 0..RAND_MAX */
		u = (Awkfloat) (rand() % RAND_MAX) / RAND_MAX;
		break;
	case FSRAND:
		if (isrec(x))	/* no argument provided */
			u = time((time_t *)0);
		else
			u = getfval(x);
		srand((unsigned int) u);
		break;
	case FTOUPPER:
	case FTOLOWER:
		buf = tostring(getsval(x));
		if (t == FTOUPPER) {
			for (p = buf; *p; p++)
				if (islower((uschar) *p))
					*p = toupper((uschar)*p);
		} else {
			for (p = buf; *p; p++)
				if (isupper((uschar) *p))
					*p = tolower((uschar)*p);
		}
		tempfree(x);
		x = gettemp();
		setsval(x, buf);
		free(buf);
		return x;
	case FFLUSH:
		if (isrec(x) || strlen(getsval(x)) == 0) {
			flush_all();	/* fflush() or fflush("") -> all */
			u = 0;
		} else if ((fp = openfile(FFLUSH, getsval(x))) == NULL)
			u = EOF;
		else
			u = fflush(fp);
		break;
	default:	/* can't happen */
		FATAL("illegal function type %d", t);
		break;
	}
	tempfree(x);
	x = gettemp();
	setfval(x, u);
	if (nextarg != 0) {
		WARNING("warning: function has too many arguments");
		for ( ; nextarg; nextarg = nextarg->nnext)
			execute(nextarg);
	}
	return(x);
}

/*!
 *  @brief Executes the print statement.
 *
 *  @param[in] a Children: a[0] argument list, a[1] redirection,
 *               a[2] file.
 *  @param[in] n Token number.
 *
 *  @return True cell.
 */
Cell *printstat(Node **a, int n)
{
	Node *x;
	Cell *y;
	FILE *fp;

	if (a[1] == 0)	/* a[1] is redirection operator, a[2] is file */
		fp = stdout;
	else
		fp = redirect(ptoi(a[1]), a[2]);
	for (x = a[0]; x != NULL; x = x->nnext) {
		y = execute(x);
		fputs(getpssval(y), fp);
		tempfree(y);
		if (x->nnext == NULL)
			fputs(*ORS, fp);
		else
			fputs(*OFS, fp);
	}
	if (a[1] != 0)
		fflush(fp);
	if (ferror(fp))
		FATAL("write error on %s", filename(fp));
	return(True);
}

/*!
 *  @brief Placeholder handler for tokens without an action.
 *
 *  @param[in] a Children of the node (unused).
 *  @param[in] n Token number (unused).
 *
 *  @return Always NULL.
 *  @retval 0 No action is performed.
 */
Cell *nullproc(Node **a, int n)
{
	n = n;
	a = a;
	return 0;
}


/*!
 *  @brief Sets up an output redirection.
 *
 *  @param[in] a Redirection kind.
 *  @param[in] b File name expression.
 *
 *  @return Open stream.
 */
FILE *redirect(int a, Node *b)
{
	FILE *fp;
	Cell *x;
	char *fname;

	x = execute(b);
	fname = getsval(x);
	fp = openfile(a, fname);
	if (fp == NULL)
		FATAL("can't open file %s", fname);
	tempfree(x);
	return fp;
}

/*!
 *  @brief One entry of the open-file table.
 */
struct files {
	FILE	*fp;        /*!< Stream pointer. */
	const char	*fname; /*!< File name. */
	int	mode;           /*!< '|', 'a', 'w' => LE/LT, GT. */
} files[FOPEN_MAX] ={
	{ NULL,  "/dev/stdin",  LT },	/* watch out: don't free this! */
	{ NULL, "/dev/stdout", GT },
	{ NULL, "/dev/stderr", GT }
};

/*!
 *  @brief Initializes the standard streams in the file table.
 */
void stdinit(void)
{
	files[0].fp = stdin;
	files[1].fp = stdout;
	files[2].fp = stderr;
}

/*!
 *  @brief Opens a file or pipe for input or output.
 *
 *  @param[in] a Redirection kind.
 *  @param[in] us File name.
 *
 *  @return Open stream.
 *  @retval NULL The file is not open and FFLUSH was requested.
 */
FILE *openfile(int a, const char *us)
{
	const char *s = us;
	int i, m;
	FILE *fp = 0;

	if (*s == '\0')
		FATAL("null file name in print or getline");
	for (i=0; i < FOPEN_MAX; i++)
		if (files[i].fname && strcmp(s, files[i].fname) == 0) {
			if (a == files[i].mode || (a==APPEND && files[i].mode==GT))
				return files[i].fp;
			if (a == FFLUSH)
				return files[i].fp;
		}
	if (a == FFLUSH)	/* didn't find it, so don't create it! */
		return NULL;

	for (i=0; i < FOPEN_MAX; i++)
		if (files[i].fp == 0)
			break;
	if (i >= FOPEN_MAX)
		FATAL("%s makes too many open files", s);
	fflush(stdout);	/* force a semblance of order */
	m = a;
	if (a == GT) {
		fp = fopen(s, "w");
	} else if (a == APPEND) {
		fp = fopen(s, "a");
		m = GT;	/* so can mix > and >> */
	} else if (a == '|') {	/* output pipe */
		fp = popen(s, "w");
	} else if (a == LE) {	/* input pipe */
		fp = popen(s, "r");
	} else if (a == LT) {	/* getline <file */
		fp = strcmp(s, "-") == 0 ? stdin : fopen(s, "r");	/* "-" is stdin */
	} else	/* can't happen */
		FATAL("illegal redirection %d", a);
	if (fp != NULL) {
		files[i].fname = tostring(s);
		files[i].fp = fp;
		files[i].mode = m;
	}
	return fp;
}

/*!
 *  @brief Returns the name of an open stream.
 *
 *  @param[in] fp Stream.
 *
 *  @return File name, or "???" if unknown.
 */
const char *filename(FILE *fp)
{
	int i;

	for (i = 0; i < FOPEN_MAX; i++)
		if (fp == files[i].fp)
			return files[i].fname;
	return "???";
}

/*!
 *  @brief Executes the close() statement.
 *
 *  @param[in] a Children of the close node.
 *  @param[in] n Token number.
 *
 *  @return Result cell with the close status.
 */
Cell *closefile(Node **a, int n)
{
	Cell *x;
	int i, stat;

	n = n;
	x = execute(a[0]);
	getsval(x);
	stat = -1;
	for (i = 0; i < FOPEN_MAX; i++) {
		if (files[i].fname && strcmp(x->sval, files[i].fname) == 0) {
			if (ferror(files[i].fp))
				WARNING( "i/o error occurred on %s", files[i].fname );
			if (files[i].mode == '|' || files[i].mode == LE)
				stat = pclose(files[i].fp);
			else
				stat = fclose(files[i].fp);
			if (stat == EOF)
				WARNING( "i/o error occurred closing %s", files[i].fname );
			if (i > 2)	/* don't do /dev/std... */
				xfree(files[i].fname);
			files[i].fname = NULL;	/* watch out for ref thru this */
			files[i].fp = NULL;
		}
	}
	tempfree(x);
	x = gettemp();
	setfval(x, (Awkfloat) stat);
	return(x);
}

/*!
 *  @brief Closes all open streams at shutdown.
 */
void closeall(void)
{
	int i, stat;

	for (i = 0; i < FOPEN_MAX; i++) {
		if (files[i].fp) {
			if (ferror(files[i].fp))
				WARNING( "i/o error occurred on %s", files[i].fname );
			if (files[i].mode == '|' || files[i].mode == LE)
				stat = pclose(files[i].fp);
			else
				stat = fclose(files[i].fp);
			if (stat == EOF)
				WARNING( "i/o error occurred while closing %s", files[i].fname );
		}
	}
}

/*!
 *  @brief Flushes all open streams.
 */
void flush_all(void)
{
	int i;

	for (i = 0; i < FOPEN_MAX; i++)
		if (files[i].fp)
			fflush(files[i].fp);
}

/*!
 *  @brief Handles \\& variations in sub/gsub replacement strings.
 *
 *  @param[in,out] pb_ptr   Pointer to the output position.
 *  @param[in,out] sptr_ptr Pointer to the replacement position.
 */
void backsub(char **pb_ptr, char **sptr_ptr);

/*!
 *  @brief Executes the sub() command.
 *
 *  @param[in] a   Children of the sub node.
 *  @param[in] nnn Token number.
 *
 *  @return Boolean result cell.
 */
Cell *sub(Node **a, int nnn)
{
	char *sptr, *pb, *q;
	Cell *x, *y, *result;
	char *t, *buf;
	fa *pfa;
	int bufsz = recsize;

	if ((buf = (char *) malloc(bufsz)) == NULL)
		FATAL("out of memory in sub");
	x = execute(a[3]);	/* target string */
	t = getsval(x);
	if (a[0] == 0)		/* 0 => a[1] is already-compiled regexpr */
		pfa = (fa *) a[1];	/* regular expression */
	else {
		y = execute(a[1]);
		pfa = makedfa(getsval(y), 1);
		tempfree(y);
	}
	y = execute(a[2]);	/* replacement string */
	result = False;
	if (pmatch(pfa, t)) {
		sptr = t;
		adjbuf(&buf, &bufsz, 1+patbeg-sptr, recsize, 0, "sub");
		pb = buf;
		while (sptr < patbeg)
			*pb++ = *sptr++;
		sptr = getsval(y);
		while (*sptr != 0) {
			adjbuf(&buf, &bufsz, 5+pb-buf, recsize, &pb, "sub");
			if (*sptr == '\\') {
				backsub(&pb, &sptr);
			} else if (*sptr == '&') {
				sptr++;
				adjbuf(&buf, &bufsz, 1+patlen+pb-buf, recsize, &pb, "sub");
				for (q = patbeg; q < patbeg+patlen; )
					*pb++ = *q++;
			} else
				*pb++ = *sptr++;
		}
		*pb = '\0';
		if (pb > buf + bufsz)
			FATAL("sub result1 %.30s too big; can't happen", buf);
		sptr = patbeg + patlen;
		if ((patlen == 0 && *patbeg) || (patlen && *(sptr-1))) {
			adjbuf(&buf, &bufsz, 1+strlen(sptr)+pb-buf, 0, &pb, "sub");
			while ((*pb++ = *sptr++) != 0)
				;
		}
		if (pb > buf + bufsz)
			FATAL("sub result2 %.30s too big; can't happen", buf);
		setsval(x, buf);	/* BUG: should be able to avoid copy */
		result = True;;
	}
	tempfree(x);
	tempfree(y);
	free(buf);
	return result;
}

/*!
 *  @brief Executes the gsub() command.
 *
 *  @param[in] a   Children of the gsub node.
 *  @param[in] nnn Token number.
 *
 *  @return Result cell with the number of substitutions.
 */
Cell *gsub(Node **a, int nnn)
{
	Cell *x, *y;
	char *rptr, *sptr, *t, *pb, *q;
	char *buf;
	fa *pfa;
	int mflag, tempstat, num;
	int bufsz = recsize;

	if ((buf = (char *) malloc(bufsz)) == NULL)
		FATAL("out of memory in gsub");
	mflag = 0;	/* if mflag == 0, can replace empty string */
	num = 0;
	x = execute(a[3]);	/* target string */
	t = getsval(x);
	if (a[0] == 0)		/* 0 => a[1] is already-compiled regexpr */
		pfa = (fa *) a[1];	/* regular expression */
	else {
		y = execute(a[1]);
		pfa = makedfa(getsval(y), 1);
		tempfree(y);
	}
	y = execute(a[2]);	/* replacement string */
	if (pmatch(pfa, t)) {
		tempstat = pfa->initstat;
		pfa->initstat = 2;
		pb = buf;
		rptr = getsval(y);
		do {
			if (patlen == 0 && *patbeg != 0) {	/* matched empty string */
				if (mflag == 0) {	/* can replace empty */
					num++;
					sptr = rptr;
					while (*sptr != 0) {
						adjbuf(&buf, &bufsz, 5+pb-buf, recsize, &pb, "gsub");
						if (*sptr == '\\') {
							backsub(&pb, &sptr);
						} else if (*sptr == '&') {
							sptr++;
							adjbuf(&buf, &bufsz, 1+patlen+pb-buf, recsize, &pb, "gsub");
							for (q = patbeg; q < patbeg+patlen; )
								*pb++ = *q++;
						} else
							*pb++ = *sptr++;
					}
				}
				if (*t == 0)	/* at end */
					goto done;
				adjbuf(&buf, &bufsz, 2+pb-buf, recsize, &pb, "gsub");
				*pb++ = *t++;
				if (pb > buf + bufsz)	/* BUG: not sure of this test */
					FATAL("gsub result0 %.30s too big; can't happen", buf);
				mflag = 0;
			}
			else {	/* matched nonempty string */
				num++;
				sptr = t;
				adjbuf(&buf, &bufsz, 1+(patbeg-sptr)+pb-buf, recsize, &pb, "gsub");
				while (sptr < patbeg)
					*pb++ = *sptr++;
				sptr = rptr;
				while (*sptr != 0) {
					adjbuf(&buf, &bufsz, 5+pb-buf, recsize, &pb, "gsub");
					if (*sptr == '\\') {
						backsub(&pb, &sptr);
					} else if (*sptr == '&') {
						sptr++;
						adjbuf(&buf, &bufsz, 1+patlen+pb-buf, recsize, &pb, "gsub");
						for (q = patbeg; q < patbeg+patlen; )
							*pb++ = *q++;
					} else
						*pb++ = *sptr++;
				}
				t = patbeg + patlen;
				if (patlen == 0 || *t == 0 || *(t-1) == 0)
					goto done;
				if (pb > buf + bufsz)
					FATAL("gsub result1 %.30s too big; can't happen", buf);
				mflag = 1;
			}
		} while (pmatch(pfa,t));
		sptr = t;
		adjbuf(&buf, &bufsz, 1+strlen(sptr)+pb-buf, 0, &pb, "gsub");
		while ((*pb++ = *sptr++) != 0)
			;
	done:	if (pb > buf + bufsz)
			FATAL("gsub result2 %.30s too big; can't happen", buf);
		*pb = '\0';
		setsval(x, buf);	/* BUG: should be able to avoid copy + free */
		pfa->initstat = tempstat;
	}
	tempfree(x);
	tempfree(y);
	x = gettemp();
	x->tval = NUM;
	x->fval = num;
	free(buf);
	return(x);
}

/*!
 *  @brief Handles \\& variations in sub/gsub replacement strings.
 *
 *  @param[in,out] pb_ptr   Pointer to the output position.
 *  @param[in,out] sptr_ptr Pointer to the replacement position.
 */
void backsub(char **pb_ptr, char **sptr_ptr)
{
	char *pb = *pb_ptr, *sptr = *sptr_ptr;

	if (sptr[1] == '\\') {
		if (sptr[2] == '\\' && sptr[3] == '&') { /* \\\& -> \& */
			*pb++ = '\\';
			*pb++ = '&';
			sptr += 4;
		} else if (sptr[2] == '&') {	/* \\& -> \ + matched */
			*pb++ = '\\';
			sptr += 2;
		} else {			/* \\x -> \\x */
			*pb++ = *sptr++;
			*pb++ = *sptr++;
		}
	} else if (sptr[1] == '&') {	/* literal & */
		sptr++;
		*pb++ = *sptr++;
	} else				/* literal \ */
		*pb++ = *sptr++;

	*pb_ptr = pb;
	*sptr_ptr = sptr;
}
