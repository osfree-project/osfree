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
 *  @file parse.c
 *  @brief Parse tree construction helpers for awk.
 *
 *  Provides the node allocation primitives used by the yacc actions,
 *  the value/expression/statement wrappers, array and pair-pattern
 *  support, function definition bookkeeping and small pointer
 *  conversion utilities.
 *
 *  @copyright Copyright (C) Lucent Technologies 1997.
 */

#define DEBUG
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "awk.h"
#include "ytab.h"

/*!
 *  @brief Allocates a parse tree node with @p n children.
 *
 *  @param[in] n Number of children of the node.
 *
 *  @return Pointer to the newly allocated node.
 */
Node *nodealloc(int n)
{
	Node *x;

	x = (Node *) malloc(sizeof(Node) + (n-1)*sizeof(Node *));
	if (x == NULL)
		FATAL("out of space in nodealloc");
	x->nnext = NULL;
	x->lineno = lineno;
	return(x);
}

/*!
 *  @brief Marks a node as a statement.
 *
 *  @param[in] a Node to convert.
 *
 *  @return The same node with ntype set to NSTAT.
 */
Node *exptostat(Node *a)
{
	a->ntype = NSTAT;
	return(a);
}

/*!
 *  @brief Creates a node with one child.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *
 *  @return New node.
 */
Node *node1(int a, Node *b)
{
	Node *x;

	x = nodealloc(1);
	x->nobj = a;
	x->narg[0]=b;
	return(x);
}

/*!
 *  @brief Creates a node with two children.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *
 *  @return New node.
 */
Node *node2(int a, Node *b, Node *c)
{
	Node *x;

	x = nodealloc(2);
	x->nobj = a;
	x->narg[0] = b;
	x->narg[1] = c;
	return(x);
}

/*!
 *  @brief Creates a node with three children.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *
 *  @return New node.
 */
Node *node3(int a, Node *b, Node *c, Node *d)
{
	Node *x;

	x = nodealloc(3);
	x->nobj = a;
	x->narg[0] = b;
	x->narg[1] = c;
	x->narg[2] = d;
	return(x);
}

/*!
 *  @brief Creates a node with four children.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *  @param[in] e Fourth child.
 *
 *  @return New node.
 */
Node *node4(int a, Node *b, Node *c, Node *d, Node *e)
{
	Node *x;

	x = nodealloc(4);
	x->nobj = a;
	x->narg[0] = b;
	x->narg[1] = c;
	x->narg[2] = d;
	x->narg[3] = e;
	return(x);
}

/*!
 *  @brief Creates a statement node with one child.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *
 *  @return New statement node.
 */
Node *stat1(int a, Node *b)
{
	Node *x;

	x = node1(a,b);
	x->ntype = NSTAT;
	return(x);
}

/*!
 *  @brief Creates a statement node with two children.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *
 *  @return New statement node.
 */
Node *stat2(int a, Node *b, Node *c)
{
	Node *x;

	x = node2(a,b,c);
	x->ntype = NSTAT;
	return(x);
}

/*!
 *  @brief Creates a statement node with three children.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *
 *  @return New statement node.
 */
Node *stat3(int a, Node *b, Node *c, Node *d)
{
	Node *x;

	x = node3(a,b,c,d);
	x->ntype = NSTAT;
	return(x);
}

/*!
 *  @brief Creates a statement node with four children.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *  @param[in] e Fourth child.
 *
 *  @return New statement node.
 */
Node *stat4(int a, Node *b, Node *c, Node *d, Node *e)
{
	Node *x;

	x = node4(a,b,c,d,e);
	x->ntype = NSTAT;
	return(x);
}

/*!
 *  @brief Creates an expression node with one child.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *
 *  @return New expression node.
 */
Node *op1(int a, Node *b)
{
	Node *x;

	x = node1(a,b);
	x->ntype = NEXPR;
	return(x);
}

/*!
 *  @brief Creates an expression node with two children.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *
 *  @return New expression node.
 */
Node *op2(int a, Node *b, Node *c)
{
	Node *x;

	x = node2(a,b,c);
	x->ntype = NEXPR;
	return(x);
}

/*!
 *  @brief Creates an expression node with three children.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *
 *  @return New expression node.
 */
Node *op3(int a, Node *b, Node *c, Node *d)
{
	Node *x;

	x = node3(a,b,c,d);
	x->ntype = NEXPR;
	return(x);
}

/*!
 *  @brief Creates an expression node with four children.
 *
 *  @param[in] a Node operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *  @param[in] e Fourth child.
 *
 *  @return New expression node.
 */
Node *op4(int a, Node *b, Node *c, Node *d, Node *e)
{
	Node *x;

	x = node4(a,b,c,d,e);
	x->ntype = NEXPR;
	return(x);
}

/*!
 *  @brief Wraps a Cell pointer into a value node.
 *
 *  @param[in] a Cell to wrap.
 *  @param[in] b Cell subtype (CUNK, CCON, CVAR, ...).
 *
 *  @return New value node.
 */
Node *celltonode(Cell *a, int b)
{
	Node *x;

	a->ctype = OCELL;
	a->csub = b;
	x = node1(0, (Node *) a);
	x->ntype = NVALUE;
	return(x);
}

/*!
 *  @brief Makes $0 into a Node.
 *
 *  @return Node representing the current record.
 */
Node *rectonode(void)
{
	extern Cell *literal0;
	return op1(INDIRECT, celltonode(literal0, CUNK));
}

/*!
 *  @brief Turns a value node into an array node.
 *
 *  @param[in] p Value node to convert.
 *
 *  @return The same node, now representing an array.
 */
Node *makearr(Node *p)
{
	Cell *cp;

	if (isvalue(p)) {
		cp = (Cell *) (p->narg[0]);
		if (isfcn(cp))
			SYNTAX( "%s is a function, not an array", cp->nval );
		else if (!isarr(cp)) {
			xfree(cp->sval);
			cp->sval = (char *) makesymtab(NSYMTAB);
			cp->tval = ARR;
		}
	}
	return p;
}

/*!
 *  @brief Maximum number of pat,pat statements.
 *  @def PA2NUM
 */
#define PA2NUM	50

int	paircnt;         /*!< Number of pat,pat statements currently in use. */
int	pairstack[PA2NUM]; /*!< State of each pat,pat statement. */

/*!
 *  @brief Creates a pat,pat pattern statement node.
 *
 *  @param[in] a First pattern.
 *  @param[in] b Second pattern.
 *  @param[in] c Statement body.
 *
 *  @return New statement node.
 */
Node *pa2stat(Node *a, Node *b, Node *c)
{
	Node *x;

	x = node4(PASTAT2, a, b, c, itonp(paircnt));
	if (paircnt++ >= PA2NUM)
		SYNTAX( "limited to %d pat,pat statements", PA2NUM );
	x->ntype = NSTAT;
	return(x);
}

/*!
 *  @brief Concatenates two linked lists of nodes.
 *
 *  @param[in] a First list (may be NULL).
 *  @param[in] b Second list (may be NULL).
 *
 *  @return Head of the concatenated list.
 */
Node *linkum(Node *a, Node *b)
{
	Node *c;

	if (errorflag)	/* don't link things that are wrong */
		return a;
	if (a == NULL)
		return(b);
	else if (b == NULL)
		return(a);
	for (c = a; c->nnext != NULL; c = c->nnext)
		;
	c->nnext = b;
	return(a);
}

/*!
 *  @brief Turns on the FCN bit in a function definition.
 *
 *  @param[in] v  Cell of the function name.
 *  @param[in] vl Argument list node.
 *  @param[in] st Function body node.
 */
void defn(Cell *v, Node *vl, Node *st)
{
	Node *p;
	int n;

	if (isarr(v)) {
		SYNTAX( "`%s' is an array name and a function name", v->nval );
		return;
	}
	if (isarg(v->nval) != -1) {
		SYNTAX( "`%s' is both function name and argument name", v->nval );
		return;
	}

	v->tval = FCN;
	v->sval = (char *) st;
	n = 0;	/* count arguments */
	for (p = vl; p; p = p->nnext)
		n++;
	v->fval = n;
	dprintf( ("defining func %s (%d args)\n", v->nval, n) );
}

/*!
 *  @brief Tests whether @p s is an argument of the current function.
 *
 *  @param[in] s Name to test.
 *
 *  @return Argument number.
 *  @retval -1 The name is not an argument.
 */
int isarg(const char *s)
{
	extern Node *arglist;
	Node *p = arglist;
	int n;

	for (n = 0; p != 0; p = p->nnext, n++)
		if (strcmp(((Cell *)(p->narg[0]))->nval, s) == 0)
			return n;
	return -1;
}

/*!
 *  @brief Converts a pointer to an integer.
 *
 *  @param[in] p Pointer to convert.
 *
 *  @return Integer representation of the pointer.
 */
int ptoi(void *p)
{
	return (int) (long) p;	/* swearing that p fits, of course */
}

/*!
 *  @brief Converts an integer back to a pointer.
 *
 *  @param[in] i Integer to convert.
 *
 *  @return Node pointer corresponding to the integer.
 */
Node *itonp(int i)
{
	return (Node *) (long) i;
}
