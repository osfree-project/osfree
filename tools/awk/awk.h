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
 *  @file awk.h
 *  @brief Core declarations of the awk interpreter.
 *
 *  Defines the fundamental data types (Awkfloat, Cell, Array,
 *  Node, fa), the token subtypes, the parse tree node kinds,
 *  the global interpreter variables, and pulls in the function
 *  prototypes from proto.h.
 *
 *  @copyright Copyright (C) Lucent Technologies 1997.
 */

/*!
 *  @brief Numeric type used for all awk numbers.
 */
typedef double	Awkfloat;

/* unsigned char is more trouble than it's worth */

/*!
 *  @brief Unsigned 8-bit byte used for regex character tables.
 */
typedef	unsigned char uschar;

/*!
 *  @brief Frees a pointer and clears it.
 *
 *  @param[in,out] a Pointer variable to free (may be NULL).
 *  @def xfree
 */
#define	xfree(a)	{ if ((a) != NULL) { free((char *) a); a = NULL; } }

/*!
 *  @brief Guarantees a non-null string for dprintf.
 *
 *  @param[in] p Pointer that may be NULL.
 *  @def NN
 */
#define	NN(p)	((p) ? (p) : "(null)")

/*!
 *  @brief Enables debug logging of the interpreter.
 *  @def DEBUG
 */
#define	DEBUG
#ifdef	DEBUG
			/* uses have to be doubly parenthesized */
/*!
 *  @brief Debug-only printf wrapper.
 *
 *  @param[in] x Parenthesized printf argument list.
 *  @def dprintf
 */
#	define	dprintf(x)	if (dbg) printf x
#else
#	define	dprintf(x)
#endif

/*!< Buffer used for error reporting context. */
extern	char	errbuf[];

/*!< 1 when compiling the awk program, 0 when running it. */
extern int	compile_time;

/*!< 0 = unsafe, 1 = safe mode. */
extern int	safe;

/*!
 *  @brief Default size of records, fields and buffers.
 *  @def RECSIZE
 */
#define	RECSIZE	(8 * 1024)

/*!< Current record buffer size, initially RECSIZE. */
extern int	recsize;

/*!< Field separator. */
extern char	**FS;
/*!< Record separator. */
extern char	**RS;
/*!< Output field separator. */
extern char	**ORS;
/*!< Output field separator alias (kept for compatibility). */
extern char	**OFS;
/*!< Output format for numbers. */
extern char	**OFMT;
/*!< Number of records read so far. */
extern Awkfloat *NR;
/*!< Number of records read from the current file. */
extern Awkfloat *FNR;
/*!< Number of fields in the current record. */
extern Awkfloat *NF;
/*!< Current input file name. */
extern char	**FILENAME;
/*!< Subscript separator for multidimensional arrays. */
extern char	**SUBSEP;
/*!< Start of the last match, origin 1. */
extern Awkfloat *RSTART;
/*!< Length of the last match. */
extern Awkfloat *RLENGTH;

/*!< Points to $0. */
extern char	*record;
/*!< Line number in the awk program. */
extern int	lineno;
/*!< 1 if an error has occurred. */
extern int	errorflag;
/*!< 1 if the record has been split into fields. */
extern int	donefld;
/*!< 1 if the record is valid (no field changed). */
extern int	donerec;
/*!< Field separator captured at input time. */
extern char	inputFS[];

/*!< Debug level. */
extern int	dbg;

/*!< Beginning of the matched pattern. */
extern	char	*patbeg;
/*!< Length of the matched pattern; set in b.c. */
extern	int	patlen;

/*!
 *  @brief Cell — a variable, constant or temporary value.
 */
typedef struct Cell {
	uschar	ctype;      /*!< OCELL, OBOOL, OJUMP, etc. */
	uschar	csub;       /*!< CCON, CTEMP, CFLD, etc. */
	char	*nval;      /*!< Name (for variables only). */
	char	*sval;      /*!< String value. */
	Awkfloat fval;      /*!< Numeric value. */
	int	 tval;       /*!< Type info: STR|NUM|ARR|FCN|FLD|CON|DONTFREE. */
	struct Cell *cnext; /*!< Next cell in the chain (hash bucket). */
} Cell;

/*!
 *  @brief Array — a symbol table implemented as a hash table.
 */
typedef struct Array {
	int	nelem;      /*!< Number of elements currently in the table. */
	int	size;       /*!< Size of the hash table. */
	Cell	**tab;      /*!< Hash table buckets. */
} Array;

/*!
 *  @brief Initial size of a symbol table.
 *  @def NSYMTAB
 */
#define	NSYMTAB	50

/*!< Main symbol table. */
extern Array	*symtab;

/*!< Cell of the NR variable. */
extern Cell	*nrloc;
/*!< Cell of the FNR variable. */
extern Cell	*fnrloc;
/*!< Cell of the NF variable. */
extern Cell	*nfloc;
/*!< Cell of the RSTART variable. */
extern Cell	*rstartloc;
/*!< Cell of the RLENGTH variable. */
extern Cell	*rlengthloc;

/* Cell.tval values: */

/*!
 *  @brief Numeric value is valid.
 *  @def NUM
 */
#define	NUM	01
/*!
 *  @brief String value is valid.
 *  @def STR
 */
#define	STR	02
/*!
 *  @brief String space is not freeable.
 *  @def DONTFREE
 */
#define DONTFREE 04
/*!
 *  @brief This cell is a constant.
 *  @def CON
 */
#define	CON	010
/*!
 *  @brief This cell is an array.
 *  @def ARR
 */
#define	ARR	020
/*!
 *  @brief This cell is a function name.
 *  @def FCN
 */
#define	FCN	040
/*!
 *  @brief This cell is a field $1, $2, ...
 *  @def FLD
 */
#define FLD	0100
/*!
 *  @brief This cell is $0.
 *  @def REC
 */
#define	REC	0200


/* function types */

/*!
 *  @brief Built-in function: length.
 *  @def FLENGTH
 */
#define	FLENGTH	1
/*!
 *  @brief Built-in function: sqrt.
 *  @def FSQRT
 */
#define	FSQRT	2
/*!
 *  @brief Built-in function: exp.
 *  @def FEXP
 */
#define	FEXP	3
/*!
 *  @brief Built-in function: log.
 *  @def FLOG
 */
#define	FLOG	4
/*!
 *  @brief Built-in function: int.
 *  @def FINT
 */
#define	FINT	5
/*!
 *  @brief Built-in function: system.
 *  @def FSYSTEM
 */
#define	FSYSTEM	6
/*!
 *  @brief Built-in function: rand.
 *  @def FRAND
 */
#define	FRAND	7
/*!
 *  @brief Built-in function: srand.
 *  @def FSRAND
 */
#define	FSRAND	8
/*!
 *  @brief Built-in function: sin.
 *  @def FSIN
 */
#define	FSIN	9
/*!
 *  @brief Built-in function: cos.
 *  @def FCOS
 */
#define	FCOS	10
/*!
 *  @brief Built-in function: atan2.
 *  @def FATAN
 */
#define	FATAN	11
/*!
 *  @brief Built-in function: toupper.
 *  @def FTOUPPER
 */
#define	FTOUPPER 12
/*!
 *  @brief Built-in function: tolower.
 *  @def FTOLOWER
 */
#define	FTOLOWER 13
/*!
 *  @brief Built-in function: fflush.
 *  @def FFLUSH
 */
#define	FFLUSH	14

/* Node: parse tree is made of nodes, with Cell's at bottom */

/*!
 *  @brief Node — a parse tree node.
 *
 *  The trailing field narg is a variable-length array: the actual
 *  number of children is fixed at allocation time.
 */
typedef struct Node {
	int	ntype;         /*!< Node type: NVALUE, NSTAT, NEXPR. */
	struct	Node *nnext;   /*!< Next node in a linked list. */
	int	lineno;        /*!< Source line of the node. */
	int	nobj;          /*!< Operation / token kind. */
	struct	Node *narg[1]; /*!< Children; actual size set at malloc time. */
} Node;

/*!
 *  @brief Null parse tree node.
 *  @def NIL
 */
#define	NIL	((Node *) 0)

/*!< Root of the parse tree. */
extern Node	*winner;
/*!< The empty statement node. */
extern Node	*nullstat;
/*!< The zero&null node used for comparisons. */
extern Node	*nullnode;

/* ctypes */

/*!
 *  @brief Cell type marker: ordinary cell.
 *  @def OCELL
 */
#define OCELL	1
/*!
 *  @brief Cell type marker: boolean result.
 *  @def OBOOL
 */
#define OBOOL	2
/*!
 *  @brief Cell type marker: jump instruction.
 *  @def OJUMP
 */
#define OJUMP	3

/* Cell subtypes: csub */

/*!
 *  @brief Cell subtype: free cell.
 *  @def CFREE
 */
#define	CFREE	7
/*!
 *  @brief Cell subtype: copy of another cell.
 *  @def CCOPY
 */
#define CCOPY	6
/*!
 *  @brief Cell subtype: constant.
 *  @def CCON
 */
#define CCON	5
/*!
 *  @brief Cell subtype: temporary.
 *  @def CTEMP
 */
#define CTEMP	4
/*!
 *  @brief Cell subtype: variable name.
 *  @def CNAME
 */
#define CNAME	3
/*!
 *  @brief Cell subtype: variable.
 *  @def CVAR
 */
#define CVAR	2
/*!
 *  @brief Cell subtype: field.
 *  @def CFLD
 */
#define CFLD	1
/*!
 *  @brief Cell subtype: unknown.
 *  @def CUNK
 */
#define	CUNK	0

/* bool subtypes */

/*!
 *  @brief Boolean subtype: true.
 *  @def BTRUE
 */
#define BTRUE	11
/*!
 *  @brief Boolean subtype: false.
 *  @def BFALSE
 */
#define BFALSE	12

/* jump subtypes */

/*!
 *  @brief Jump subtype: exit.
 *  @def JEXIT
 */
#define JEXIT	21
/*!
 *  @brief Jump subtype: next.
 *  @def JNEXT
 */
#define JNEXT	22
/*!
 *  @brief Jump subtype: break.
 *  @def JBREAK
 */
#define	JBREAK	23
/*!
 *  @brief Jump subtype: continue.
 *  @def JCONT
 */
#define	JCONT	24
/*!
 *  @brief Jump subtype: return.
 *  @def JRET
 */
#define	JRET	25
/*!
 *  @brief Jump subtype: nextfile.
 *  @def JNEXTFILE
 */
#define	JNEXTFILE	26

/* node types */

/*!
 *  @brief Node type: value.
 *  @def NVALUE
 */
#define NVALUE	1
/*!
 *  @brief Node type: statement.
 *  @def NSTAT
 */
#define NSTAT	2
/*!
 *  @brief Node type: expression.
 *  @def NEXPR
 */
#define NEXPR	3


/*!< Stack of pat,pat pattern states. */
extern	int	pairstack[];
/*!< Number of pat,pat patterns in use. */
extern	int	paircnt;

/*!
 *  @brief Tests whether a token is not legal.
 *
 *  @param[in] n Token number.
 *  @def notlegal
 */
#define notlegal(n)	(n <= FIRSTTOKEN || n >= LASTTOKEN || proctab[n-FIRSTTOKEN] == nullproc)
/*!
 *  @brief Tests whether a node is a value node.
 *
 *  @param[in] n Node pointer.
 *  @def isvalue
 */
#define isvalue(n)	((n)->ntype == NVALUE)
/*!
 *  @brief Tests whether a node is an expression node.
 *
 *  @param[in] n Node pointer.
 *  @def isexpr
 */
#define isexpr(n)	((n)->ntype == NEXPR)
/*!
 *  @brief Tests whether a cell is a jump.
 *
 *  @param[in] n Cell pointer.
 *  @def isjump
 */
#define isjump(n)	((n)->ctype == OJUMP)
/*!
 *  @brief Tests whether a cell is an exit jump.
 *
 *  @param[in] n Cell pointer.
 *  @def isexit
 */
#define isexit(n)	((n)->csub == JEXIT)
/*!
 *  @brief Tests whether a cell is a break jump.
 *
 *  @param[in] n Cell pointer.
 *  @def isbreak
 */
#define	isbreak(n)	((n)->csub == JBREAK)
/*!
 *  @brief Tests whether a cell is a continue jump.
 *
 *  @param[in] n Cell pointer.
 *  @def iscont
 */
#define	iscont(n)	((n)->csub == JCONT)
/*!
 *  @brief Tests whether a cell is a next or nextfile jump.
 *
 *  @param[in] n Cell pointer.
 *  @def isnext
 */
#define	isnext(n)	((n)->csub == JNEXT || (n)->csub == JNEXTFILE)
/*!
 *  @brief Tests whether a cell is a return jump.
 *
 *  @param[in] n Cell pointer.
 *  @def isret
 */
#define	isret(n)	((n)->csub == JRET)
/*!
 *  @brief Tests whether a cell is a record $0.
 *
 *  @param[in] n Cell pointer.
 *  @def isrec
 */
#define isrec(n)	((n)->tval & REC)
/*!
 *  @brief Tests whether a cell is a field.
 *
 *  @param[in] n Cell pointer.
 *  @def isfld
 */
#define isfld(n)	((n)->tval & FLD)
/*!
 *  @brief Tests whether a cell has a valid string value.
 *
 *  @param[in] n Cell pointer.
 *  @def isstr
 */
#define isstr(n)	((n)->tval & STR)
/*!
 *  @brief Tests whether a cell has a valid numeric value.
 *
 *  @param[in] n Cell pointer.
 *  @def isnum
 */
#define isnum(n)	((n)->tval & NUM)
/*!
 *  @brief Tests whether a cell is an array.
 *
 *  @param[in] n Cell pointer.
 *  @def isarr
 */
#define isarr(n)	((n)->tval & ARR)
/*!
 *  @brief Tests whether a cell is a function name.
 *
 *  @param[in] n Cell pointer.
 *  @def isfcn
 */
#define isfcn(n)	((n)->tval & FCN)
/*!
 *  @brief Tests whether a cell holds the boolean value true.
 *
 *  @param[in] n Cell pointer.
 *  @def istrue
 */
#define istrue(n)	((n)->csub == BTRUE)
/*!
 *  @brief Tests whether a cell is a temporary.
 *
 *  @param[in] n Cell pointer.
 *  @def istemp
 */
#define istemp(n)	((n)->csub == CTEMP)
/*!
 *  @brief Tests whether a node is a function argument.
 *
 *  @param[in] n Node pointer.
 *  @def isargument
 */
#define	isargument(n)	((n)->nobj == ARG)
/* #define freeable(p)	(!((p)->tval & DONTFREE)) */

/*!
 *  @brief Tests whether a cell's string is freeable.
 *
 *  @param[in] p Cell pointer.
 *  @def freeable
 */
#define freeable(p)	( ((p)->tval & (STR|DONTFREE)) == STR )

/* structures used by regular expression matching machinery, mostly b.c: */

/*!
 *  @brief Number of distinct character values in the DFA tables.
 *
 *  256 handles 8-bit chars; 128 does 7-bit. Watch out in match(), etc.
 *  @def NCHARS
 */
#define NCHARS	(256+1)

/*!
 *  @brief Maximum number of DFA states.
 *  @def NSTATES
 */
#define NSTATES	32

/*!
 *  @brief One row of the regular expression automaton description.
 */
typedef struct rrow {
	long	ltype;       /*!< Leaf type; long avoids pointer warnings on 64-bit. */
	union {
		int i;           /*!< Integer value. */
		Node *np;        /*!< Node pointer. */
		uschar *up;      /*!< Unsigned char pointer (Al stores a pointer in it!). */
	} lval;              /*!< Leaf value. */
	int	*lfollow;    /*!< Pointer to the follow set. */
} rrow;

/*!
 *  @brief Finite automaton built from a regular expression.
 */
typedef struct fa {
	uschar	gototab[NSTATES][NCHARS]; /*!< Transition table. */
	uschar	out[NSTATES];             /*!< Final-state flags. */
	uschar	*restr;                   /*!< Source regular expression string. */
	int	*posns[NSTATES];          /*!< Position sets of each state. */
	int	anchor;                   /*!< Anchored-at-start flag. */
	int	use;                      /*!< LRU counter. */
	int	initstat;                 /*!< Initial state number. */
	int	curstat;                  /*!< Current state number. */
	int	accept;                   /*!< Accepting position index. */
	int	reset;                    /*!< Reset flag when state table overflows. */
	struct	rrow re[1];               /*!< Rows; actual size set at malloc time. */
} fa;


#include "proto.h"
