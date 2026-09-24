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
 *  @file proto.h
 *  @brief Function prototypes shared by the awk sources.
 *
 *  Declares the interfaces of every translation unit that makes up
 *  the interpreter: parser, lexer, regex compiler, symbol table,
 *  I/O layer and the tree-walking evaluator.
 *
 *  @copyright Copyright (C) Lucent Technologies 1997.
 */

/*!
 *  @brief Dummy yacc end-of-input hook.
 *
 *  @return Always 1.
 */
extern	int	yywrap(void);

/*!
 *  @brief Records the name of the function currently being defined.
 *
 *  @param[in] p Cell of the function name.
 */
extern	void	setfname(Cell *);

/*!
 *  @brief Tests whether a node is a constant value.
 *
 *  @param[in] p Node to test.
 *
 *  @return Non-zero if the node is a constant.
 */
extern	int	constnode(Node *);

/*!
 *  @brief Returns the string of a string-literal node.
 *
 *  @param[in] p Node to inspect.
 *
 *  @return Pointer to the string value.
 */
extern	char	*strnode(Node *);

/*!
 *  @brief Wraps a node so that it is guaranteed non-null.
 *
 *  @param[in] n Node to wrap.
 *
 *  @return Node suitable for a boolean context.
 */
extern	Node	*notnull(Node *);

/*!
 *  @brief Runs the yacc-generated parser.
 *
 *  @return 0 on success, 1 on failure.
 */
extern	int	yyparse(void);

/*!
 *  @brief Returns the next token from the lexer.
 *
 *  @return Token number.
 */
extern	int	yylex(void);

/*!
 *  @brief Requests a regular expression from the lexer on the next call.
 */
extern	void	startreg(void);

/*!
 *  @brief Returns the next input character of the program.
 *
 *  @return Next character, or 0 at end of input.
 */
extern	int	input(void);

/*!
 *  @brief Pushes a character back onto the input.
 *
 *  @param[in] c Character to push back.
 */
extern	void	unput(int);

/*!
 *  @brief Pushes a string back onto the input.
 *
 *  @param[in] s String to push back.
 */
extern	void	unputstr(const char *);

/*!
 *  @brief Lexer lookahead entry point.
 *
 *  @return Next token.
 */
extern	int	yylook(void);

/*!
 *  @brief Lexer pushback entry point.
 *
 *  @param[in,out] pstate Lexer state pointer.
 *  @param[in]     c      Character to push back.
 *
 *  @return Pushed-back character.
 */
extern	int	yyback(int *, int);

/*!
 *  @brief Lexer low-level input entry point.
 *
 *  @return Next input character.
 */
extern	int	yyinput(void);

/*!
 *  @brief Returns a DFA for a regular expression.
 *
 *  @param[in] s      Regular expression.
 *  @param[in] anchor Non-zero for anchored matching.
 *
 *  @return Pointer to the finite automaton.
 */
extern	fa	*makedfa(const char *, int);

/*!
 *  @brief Builds a DFA for a regular expression.
 *
 *  @param[in] s      Regular expression.
 *  @param[in] anchor Non-zero for anchored matching.
 *
 *  @return Pointer to the finite automaton.
 */
extern	fa	*mkdfa(const char *, int);

/*!
 *  @brief Initializes the initial state of a DFA.
 *
 *  @param[in] f      Finite automaton.
 *  @param[in] anchor Non-zero for anchored matching.
 *
 *  @return Initial state number.
 */
extern	int	makeinit(fa *, int);

/*!
 *  @brief Sets up parent pointers and leaf indices of a tree.
 *
 *  @param[in] p Root of the subtree.
 */
extern	void	penter(Node *);

/*!
 *  @brief Frees a regular expression parse tree.
 *
 *  @param[in] p Root of the subtree.
 */
extern	void	freetr(Node *);

/*!
 *  @brief Reads and evaluates a hex byte.
 *
 *  @param[in,out] pp Pointer to the current parse position.
 *
 *  @return Parsed byte value.
 */
extern	int	hexstr(char **);

/*!
 *  @brief Processes an escape sequence after a backslash.
 *
 *  @param[in,out] pp Pointer to the current parse position.
 *
 *  @return Value of the escaped character.
 */
extern	int	quoted(char **);

/*!
 *  @brief Expands a character class into a string.
 *
 *  @param[in] argp Input string with the class content.
 *
 *  @return Pointer to the expanded string.
 */
extern	char	*cclenter(const char *);

/*!
 *  @brief Reports a regular expression overflow.
 *
 *  @param[in] s Error context.
 */
extern	void	overflo(const char *);

/*!
 *  @brief Builds follow sets for each leaf of a tree.
 *
 *  @param[in] f Finite automaton.
 *  @param[in] v Current node.
 */
extern	void	cfoll(fa *, Node *);

/*!
 *  @brief Collects initially active leaves of a node.
 *
 *  @param[in] p Regular expression node.
 *
 *  @return Whether the node matches the empty string.
 */
extern	int	first(Node *);

/*!
 *  @brief Collects leaves that can follow a node.
 *
 *  @param[in] v Regular expression node.
 */
extern	void	follow(Node *);

/*!
 *  @brief Tests whether a character occurs in a string.
 *
 *  @param[in] c    Character.
 *  @param[in] sarg Character string.
 *
 *  @return 1 if the character is in the string, otherwise 0.
 */
extern	int	member(int, const char *);

/*!
 *  @brief Shortest-match test of a DFA.
 *
 *  @param[in] f  Finite automaton.
 *  @param[in] p0 Input string.
 *
 *  @return 1 on match, otherwise 0.
 */
extern	int	match(fa *, const char *);

/*!
 *  @brief Longest-match test of a DFA.
 *
 *  @param[in] f  Finite automaton.
 *  @param[in] p0 Input string.
 *
 *  @return 1 on match, otherwise 0.
 */
extern	int	pmatch(fa *, const char *);

/*!
 *  @brief Non-empty match test of a DFA.
 *
 *  @param[in] f  Finite automaton.
 *  @param[in] p0 Input string.
 *
 *  @return 1 on match, otherwise 0.
 */
extern	int	nematch(fa *, const char *);

/*!
 *  @brief Parses a regular expression.
 *
 *  @param[in] p Regular expression string.
 *
 *  @return Root of the parse tree.
 */
extern	Node	*reparse(const char *);

/*!
 *  @brief Parses a regular expression (top level).
 *
 *  @return Root of the parse tree.
 */
extern	Node	*regexp(void);

/*!
 *  @brief Parses a primary regular expression.
 *
 *  @return Parse tree node.
 */
extern	Node	*primary(void);

/*!
 *  @brief Parses a concatenation of regular expressions.
 *
 *  @param[in] np Left operand.
 *
 *  @return Parse tree node.
 */
extern	Node	*concat(Node *);

/*!
 *  @brief Parses an alternation of regular expressions.
 *
 *  @param[in] np Left operand.
 *
 *  @return Parse tree node.
 */
extern	Node	*alt(Node *);

/*!
 *  @brief Parses unary postfix operators.
 *
 *  @param[in] np Operand.
 *
 *  @return Parse tree node.
 */
extern	Node	*unary(Node *);

/*!
 *  @brief Lexical analyzer of regular expressions.
 *
 *  @return Next token.
 */
extern	int	relex(void);

/*!
 *  @brief Computes a DFA transition.
 *
 *  @param[in] f Finite automaton.
 *  @param[in] s Current state.
 *  @param[in] c Input character.
 *
 *  @return Target state number.
 */
extern	int	cgoto(fa *, int, int);

/*!
 *  @brief Frees a finite automaton.
 *
 *  @param[in] f Pointer to the automaton.
 */
extern	void	freefa(fa *);

/*!
 *  @brief Reads one character from the program source.
 *
 *  @return Next character, or EOF.
 */
extern	int	pgetc(void);

/*!
 *  @brief Returns the name of the current source file.
 *
 *  @return Source file name, or NULL.
 */
extern	char	*cursource(void);

/*!
 *  @brief Allocates a parse tree node.
 *
 *  @param[in] n Number of children.
 *
 *  @return New node.
 */
extern	Node	*nodealloc(int);

/*!
 *  @brief Marks a node as a statement.
 *
 *  @param[in] a Node to convert.
 *
 *  @return The same node.
 */
extern	Node	*exptostat(Node *);

/*!
 *  @brief Creates a node with one child.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *
 *  @return New node.
 */
extern	Node	*node1(int, Node *);

/*!
 *  @brief Creates a node with two children.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *
 *  @return New node.
 */
extern	Node	*node2(int, Node *, Node *);

/*!
 *  @brief Creates a node with three children.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *
 *  @return New node.
 */
extern	Node	*node3(int, Node *, Node *, Node *);

/*!
 *  @brief Creates a node with four children.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *  @param[in] e Fourth child.
 *
 *  @return New node.
 */
extern	Node	*node4(int, Node *, Node *, Node *, Node *);

/*!
 *  @brief Creates a statement node with three children.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *
 *  @return New statement node.
 */
extern	Node	*stat3(int, Node *, Node *, Node *);

/*!
 *  @brief Creates an expression node with two children.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *
 *  @return New expression node.
 */
extern	Node	*op2(int, Node *, Node *);

/*!
 *  @brief Creates an expression node with one child.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *
 *  @return New expression node.
 */
extern	Node	*op1(int, Node *);

/*!
 *  @brief Creates a statement node with one child.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *
 *  @return New statement node.
 */
extern	Node	*stat1(int, Node *);

/*!
 *  @brief Creates an expression node with three children.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *
 *  @return New expression node.
 */
extern	Node	*op3(int, Node *, Node *, Node *);

/*!
 *  @brief Creates an expression node with four children.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *  @param[in] e Fourth child.
 *
 *  @return New expression node.
 */
extern	Node	*op4(int, Node *, Node *, Node *, Node *);

/*!
 *  @brief Creates a statement node with two children.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *
 *  @return New statement node.
 */
extern	Node	*stat2(int, Node *, Node *);

/*!
 *  @brief Creates a statement node with four children.
 *
 *  @param[in] a Operation kind.
 *  @param[in] b First child.
 *  @param[in] c Second child.
 *  @param[in] d Third child.
 *  @param[in] e Fourth child.
 *
 *  @return New statement node.
 */
extern	Node	*stat4(int, Node *, Node *, Node *, Node *);

/*!
 *  @brief Wraps a Cell into a value node.
 *
 *  @param[in] a Cell to wrap.
 *  @param[in] b Cell subtype.
 *
 *  @return New value node.
 */
extern	Node	*celltonode(Cell *, int);

/*!
 *  @brief Makes $0 into a Node.
 *
 *  @return Node representing the current record.
 */
extern	Node	*rectonode(void);

/*!
 *  @brief Turns a value node into an array node.
 *
 *  @param[in] p Value node to convert.
 *
 *  @return The same node.
 */
extern	Node	*makearr(Node *);

/*!
 *  @brief Creates a pat,pat statement node.
 *
 *  @param[in] a First pattern.
 *  @param[in] b Second pattern.
 *  @param[in] c Statement body.
 *
 *  @return New statement node.
 */
extern	Node	*pa2stat(Node *, Node *, Node *);

/*!
 *  @brief Concatenates two linked lists of nodes.
 *
 *  @param[in] a First list.
 *  @param[in] b Second list.
 *
 *  @return Head of the concatenated list.
 */
extern	Node	*linkum(Node *, Node *);

/*!
 *  @brief Defines a function.
 *
 *  @param[in] v  Cell of the function name.
 *  @param[in] vl Argument list.
 *  @param[in] st Function body.
 */
extern	void	defn(Cell *, Node *, Node *);

/*!
 *  @brief Tests whether a name is an argument of the current function.
 *
 *  @param[in] s Name to test.
 *
 *  @return Argument number, or -1.
 */
extern	int	isarg(const char *);

/*!
 *  @brief Returns the printable name of a token.
 *
 *  @param[in] n Token number.
 *
 *  @return Printable token name.
 */
extern	char	*tokname(int);

/*!
 *  @brief Handler table indexed by token number.
 */
extern	Cell	*(*proctab[])(Node **, int);

/*!
 *  @brief Converts a pointer to an integer.
 *
 *  @param[in] p Pointer to convert.
 *
 *  @return Integer representation of the pointer.
 */
extern	int	ptoi(void *);

/*!
 *  @brief Converts an integer to a Node pointer.
 *
 *  @param[in] i Integer to convert.
 *
 *  @return Node pointer.
 */
extern	Node	*itonp(int);

/*!
 *  @brief Initializes the built-in variables.
 */
extern	void	syminit(void);

/*!
 *  @brief Fills ARGV and ARGC.
 *
 *  @param[in] ac Argument count.
 *  @param[in] av Argument vector.
 */
extern	void	arginit(int, char **);

/*!
 *  @brief Fills ENVIRON from the process environment.
 *
 *  @param[in] envp Environment vector.
 */
extern	void	envinit(char **);

/*!
 *  @brief Allocates a new symbol table.
 *
 *  @param[in] n Initial table size.
 *
 *  @return New symbol table.
 */
extern	Array	*makesymtab(int);

/*!
 *  @brief Frees a symbol table.
 *
 *  @param[in] ap Cell of the array.
 */
extern	void	freesymtab(Cell *);

/*!
 *  @brief Removes an element from an array.
 *
 *  @param[in] ap Cell of the array.
 *  @param[in] s  Subscript.
 */
extern	void	freeelem(Cell *, const char *);

/*!
 *  @brief Finds or creates an entry in a symbol table.
 *
 *  @param[in] n Name.
 *  @param[in] s String value.
 *  @param[in] f Numeric value.
 *  @param[in] t Type flags.
 *  @param[in] tp Symbol table.
 *
 *  @return Cell of the entry.
 */
extern	Cell	*setsymtab(const char *, const char *, double, unsigned int, Array *);

/*!
 *  @brief Computes a hash value.
 *
 *  @param[in] s String to hash.
 *  @param[in] n Table size.
 *
 *  @return Hash value in the range [0, n).
 */
extern	int	hash(const char *, int);

/*!
 *  @brief Grows and rehashes a symbol table.
 *
 *  @param[in] tp Symbol table.
 */
extern	void	rehash(Array *);

/*!
 *  @brief Looks up a name in a symbol table.
 *
 *  @param[in] s  Name.
 *  @param[in] tp Symbol table.
 *
 *  @return Cell of the entry, or NULL.
 */
extern	Cell	*lookup(const char *, Array *);

/*!
 *  @brief Sets the numeric value of a cell.
 *
 *  @param[in] vp Cell.
 *  @param[in] f  New numeric value.
 *
 *  @return The new numeric value.
 */
extern	double	setfval(Cell *, double);

/*!
 *  @brief Reports an invalid use of a variable.
 *
 *  @param[in] vp Cell.
 *  @param[in] rw Operation being attempted.
 */
extern	void	funnyvar(Cell *, const char *);

/*!
 *  @brief Sets the string value of a cell.
 *
 *  @param[in] vp Cell.
 *  @param[in] s  New string value.
 *
 *  @return Pointer to the stored string.
 */
extern	char	*setsval(Cell *, const char *);

/*!
 *  @brief Returns the numeric value of a cell.
 *
 *  @param[in] vp Cell.
 *
 *  @return Numeric value.
 */
extern	double	getfval(Cell *);

/*!
 *  @brief Returns the string value of a cell.
 *
 *  @param[in] vp Cell.
 *
 *  @return String value.
 */
extern	char	*getsval(Cell *);

/*!
 *  @brief Returns the string value of a cell for print.
 *
 *  @param[in] vp Cell.
 *
 *  @return String value.
 */
extern	char	*getpssval(Cell *);

/*!
 *  @brief Copies a string into freshly allocated memory.
 *
 *  @param[in] s Source string.
 *
 *  @return Pointer to the copy.
 */
extern	char	*tostring(const char *);

/*!
 *  @brief Reads a string up to a delimiter with escape handling.
 *
 *  @param[in] is    Source string.
 *  @param[in] delim Terminating delimiter.
 *
 *  @return Newly allocated decoded string.
 */
extern	char	*qstring(const char *, int);

/*!
 *  @brief Initializes the record and field buffers.
 *
 *  @param[in] n Initial buffer size.
 */
extern	void	recinit(unsigned int);

/*!
 *  @brief Initializes the first input source.
 */
extern	void	initgetrec(void);

/*!
 *  @brief Creates the cells for a range of fields.
 *
 *  @param[in] n1 First field index.
 *  @param[in] n2 Last field index.
 */
extern	void	makefields(int, int);

/*!
 *  @brief Grows the field table.
 *
 *  @param[in] n Minimum field index.
 */
extern	void	growfldtab(int n);

/*!
 *  @brief Reads the next input record.
 *
 *  @param[in,out] pbuf     Record buffer.
 *  @param[in,out] pbufsize Record buffer size.
 *  @param[in]     isrecord Non-zero if the buffer holds $0.
 *
 *  @return 1 if a record was read, otherwise 0.
 */
extern	int	getrec(char **, int *, int);

/*!
 *  @brief Advances to the next input file.
 */
extern	void	nextfile(void);

/*!
 *  @brief Reads one record from an input stream.
 *
 *  @param[in,out] buf      Record buffer.
 *  @param[in,out] bufsize  Record buffer size.
 *  @param[in]     inf      Input stream.
 *
 *  @return 1 if a record was read, otherwise 0.
 */
extern	int	readrec(char **buf, int *bufsize, FILE *inf);

/*!
 *  @brief Returns ARGV[n].
 *
 *  @param[in] n Argument index.
 *
 *  @return Argument string.
 */
extern	char	*getargv(int);

/*!
 *  @brief Sets a variable from a var=value command line argument.
 *
 *  @param[in] s Assignment string.
 */
extern	void	setclvar(char *);

/*!
 *  @brief Splits the current record into fields.
 */
extern	void	fldbld(void);

/*!
 *  @brief Clears a range of fields.
 *
 *  @param[in] n1 First field index.
 *  @param[in] n2 Last field index.
 */
extern	void	cleanfld(int, int);

/*!
 *  @brief Adds a new field.
 *
 *  @param[in] n Field index.
 */
extern	void	newfld(int);

/*!
 *  @brief Builds fields using a regular expression FS.
 *
 *  @param[in] rec Record string.
 *  @param[in] fs  Field separator regular expression.
 *
 *  @return Number of fields built.
 */
extern	int	refldbld(const char *, const char *);

/*!
 *  @brief Rebuilds $0 from $1..$NF.
 */
extern	void	recbld(void);

/*!
 *  @brief Returns the cell of the n-th field.
 *
 *  @param[in] n Field index.
 *
 *  @return Cell pointer.
 */
extern	Cell	*fieldadr(int);

/*!
 *  @brief Reports a syntax error.
 *
 *  @param[in] s Error message.
 */
extern	void	yyerror(const char *);

/*!
 *  @brief Signal handler for floating-point exceptions.
 *
 *  @param[in] n Signal number.
 */
extern	void	fpecatch(int);

/*!
 *  @brief Checks bracket balance in the program.
 */
extern	void	bracecheck(void);

/*!
 *  @brief Reports a bracket-count mismatch.
 *
 *  @param[in] n  Difference between opening and closing counts.
 *  @param[in] c1 Opening bracket character.
 *  @param[in] c2 Closing bracket character.
 */
extern	void	bcheck2(int, int, int);

/*!
 *  @brief Reports a syntax error with printf-style arguments.
 *
 *  @param[in] fmt Format string.
 *  @param[in] ... Additional arguments.
 */
extern	void	SYNTAX(const char *, ...);

/*!
 *  @brief Reports a fatal error and exits.
 *
 *  @param[in] fmt Format string.
 *  @param[in] ... Additional arguments.
 */
extern	void	FATAL(const char *, ...);

/*!
 *  @brief Reports a warning.
 *
 *  @param[in] fmt Format string.
 *  @param[in] ... Additional arguments.
 */
extern	void	WARNING(const char *, ...);

/*!
 *  @brief Prints the location of the last error.
 */
extern	void	error(void);

/*!
 *  @brief Prints the context around the last error.
 */
extern	void	eprint(void);

/*!
 *  @brief Tracks bracket nesting of a character.
 *
 *  @param[in] c Character to classify.
 */
extern	void	bclass(int);

/*!
 *  @brief Checks errno after a math library call.
 *
 *  @param[in] x Result of the math call.
 *  @param[in] s Name of the math function.
 *
 *  @return Adjusted result.
 */
extern	double	errcheck(double, const char *);

/*!
 *  @brief Tests whether a string looks like var=value.
 *
 *  @param[in] s String to test.
 *
 *  @return Non-zero if the string is an assignment.
 */
extern	int	isclvar(const char *);

/*!
 *  @brief Tests whether a string is a valid number.
 *
 *  @param[in] s String to test.
 *
 *  @return Non-zero if the string is a valid number.
 */
extern	int	is_number(const char *);

/*!
 *  @brief Grows a dynamically managed buffer.
 *
 *  @param[in,out] pb     Pointer to the buffer pointer.
 *  @param[in,out] sz     Pointer to the buffer size.
 *  @param[in]     min    Minimum required length.
 *  @param[in]     q      Allocation quantum.
 *  @param[in,out] pbp    Moving pointer inside the buffer (or NULL).
 *  @param[in]     what   Caller name for diagnostics.
 *
 *  @return 1 on success, 0 on allocation failure.
 */
extern	int	adjbuf(char **pb, int *sz, int min, int q, char **pbp, const char *what);

/*!
 *  @brief Runs the parse tree.
 *
 *  @param[in] a Root node of the program.
 */
extern	void	run(Node *);

/*!
 *  @brief Executes one parse tree node.
 *
 *  @param[in] u Node to execute.
 *
 *  @return Result cell.
 */
extern	Cell	*execute(Node *);

/*!
 *  @brief Executes the top-level program node.
 *
 *  @param[in] a Children of the program node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*program(Node **, int);

/*!
 *  @brief Executes a function call.
 *
 *  @param[in] a Children of the call node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*call(Node **, int);

/*!
 *  @brief Copies a cell into a temporary.
 *
 *  @param[in] x Cell to copy.
 *
 *  @return New temporary cell.
 */
extern	Cell	*copycell(Cell *);

/*!
 *  @brief Returns the n-th function argument.
 *
 *  @param[in] a Children of the argument node.
 *  @param[in] n Token number.
 *
 *  @return Argument cell.
 */
extern	Cell	*arg(Node **, int);

/*!
 *  @brief Executes break, continue, next, nextfile, return, exit.
 *
 *  @param[in] a Children of the jump node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*jump(Node **, int);

/*!
 *  @brief Reads a record from a specific input.
 *
 *  @param[in] a Children of the getline node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*mygetline(Node **, int);

/*!
 *  @brief Returns NF.
 *
 *  @param[in] a Children of the NF node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*getnf(Node **, int);

/*!
 *  @brief Evaluates an array subscript.
 *
 *  @param[in] a Children of the array node.
 *  @param[in] n Token number.
 *
 *  @return Element cell.
 */
extern	Cell	*array(Node **, int);

/*!
 *  @brief Executes the delete statement.
 *
 *  @param[in] a Children of the delete node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*awkdelete(Node **, int);

/*!
 *  @brief Executes the "in" test.
 *
 *  @param[in] a Children of the in node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*intest(Node **, int);

/*!
 *  @brief Executes ~ and match().
 *
 *  @param[in] a Children of the match node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*matchop(Node **, int);

/*!
 *  @brief Executes boolean operators.
 *
 *  @param[in] a Children of the boolean node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*boolop(Node **, int);

/*!
 *  @brief Executes relational operators.
 *
 *  @param[in] a Children of the relational node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*relop(Node **, int);

/*!
 *  @brief Releases a temporary cell.
 *
 *  @param[in] a Cell to release.
 */
extern	void	tfree(Cell *);

/*!
 *  @brief Allocates a temporary cell.
 *
 *  @return New temporary cell.
 */
extern	Cell	*gettemp(void);

/*!
 *  @brief Evaluates $(a[0]).
 *
 *  @param[in] a Children of the indirect node.
 *  @param[in] n Token number.
 *
 *  @return Field cell.
 */
extern	Cell	*field(Node **, int);

/*!
 *  @brief Executes the $(expr) operator.
 *
 *  @param[in] a Children of the indirect node.
 *  @param[in] n Token number.
 *
 *  @return Field cell.
 */
extern	Cell	*indirect(Node **, int);

/*!
 *  @brief Executes substr().
 *
 *  @param[in] a Children of the substr node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*substr(Node **, int);

/*!
 *  @brief Executes index().
 *
 *  @param[in] a Children of the index node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*sindex(Node **, int);

/*!
 *  @brief printf-style formatting routine.
 *
 *  @param[in,out] pbuf     Output buffer.
 *  @param[in,out] pbufsize Output buffer size.
 *  @param[in]     s        Format string.
 *  @param[in]     a        Argument list.
 *
 *  @return Number of bytes written.
 */
extern	int	format(char **, int *, const char *, Node *);

/*!
 *  @brief Executes sprintf().
 *
 *  @param[in] a Children of the sprintf node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*awksprintf(Node **, int);

/*!
 *  @brief Executes printf.
 *
 *  @param[in] a Children of the printf node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*awkprintf(Node **, int);

/*!
 *  @brief Executes arithmetic operators.
 *
 *  @param[in] a Children of the arithmetic node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*arith(Node **, int);

/*!
 *  @brief Raises a value to an integer power.
 *
 *  @param[in] x Base.
 *  @param[in] n Exponent.
 *
 *  @return x to the power n.
 */
extern	double	ipow(double, int);

/*!
 *  @brief Executes ++ and --.
 *
 *  @param[in] a Children of the increment node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*incrdecr(Node **, int);

/*!
 *  @brief Executes assignment operators.
 *
 *  @param[in] a Children of the assignment node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*assign(Node **, int);

/*!
 *  @brief Executes string concatenation.
 *
 *  @param[in] a Children of the concatenation node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*cat(Node **, int);

/*!
 *  @brief Executes a pattern-action statement.
 *
 *  @param[in] a Children of the pattern-action node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*pastat(Node **, int);

/*!
 *  @brief Executes a pat,pat action statement.
 *
 *  @param[in] a Children of the pat,pat node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*dopa2(Node **, int);

/*!
 *  @brief Executes split().
 *
 *  @param[in] a Children of the split node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*split(Node **, int);

/*!
 *  @brief Executes the ternary operator ?:.
 *
 *  @param[in] a Children of the conditional node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*condexpr(Node **, int);

/*!
 *  @brief Executes the if statement.
 *
 *  @param[in] a Children of the if node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*ifstat(Node **, int);

/*!
 *  @brief Executes the while statement.
 *
 *  @param[in] a Children of the while node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*whilestat(Node **, int);

/*!
 *  @brief Executes the do..while statement.
 *
 *  @param[in] a Children of the do node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*dostat(Node **, int);

/*!
 *  @brief Executes the for statement.
 *
 *  @param[in] a Children of the for node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*forstat(Node **, int);

/*!
 *  @brief Executes the "for (x in array)" statement.
 *
 *  @param[in] a Children of the in-array node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*instat(Node **, int);

/*!
 *  @brief Executes a built-in function.
 *
 *  @param[in] a Children of the built-in node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*bltin(Node **, int);

/*!
 *  @brief Executes the print statement.
 *
 *  @param[in] a Children of the print node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*printstat(Node **, int);

/*!
 *  @brief Empty handler used as a placeholder in proctab.
 *
 *  @param[in] a Children of the node.
 *  @param[in] n Token number.
 *
 *  @return NULL.
 */
extern	Cell	*nullproc(Node **, int);

/*!
 *  @brief Sets up an output redirection.
 *
 *  @param[in] a Redirection kind.
 *  @param[in] b File name expression.
 *
 *  @return Stream, or NULL on failure.
 */
extern	FILE	*redirect(int, Node *);

/*!
 *  @brief Opens a file or pipe for input or output.
 *
 *  @param[in] a Redirection kind.
 *  @param[in] s File name.
 *
 *  @return Stream, or NULL on failure.
 */
extern	FILE	*openfile(int, const char *);

/*!
 *  @brief Returns the name of an open stream.
 *
 *  @param[in] fp Stream.
 *
 *  @return File name, or "???" if unknown.
 */
extern	const char	*filename(FILE *);

/*!
 *  @brief Executes the close() statement.
 *
 *  @param[in] a Children of the close node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*closefile(Node **, int);

/*!
 *  @brief Closes all open streams.
 */
extern	void	closeall(void);

/*!
 *  @brief Executes sub().
 *
 *  @param[in] a Children of the sub node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*sub(Node **, int);

/*!
 *  @brief Executes gsub().
 *
 *  @param[in] a Children of the gsub node.
 *  @param[in] n Token number.
 *
 *  @return Result cell.
 */
extern	Cell	*gsub(Node **, int);

/*!
 *  @brief Runs a command and opens a pipe.
 *
 *  @param[in] s Command string.
 *  @param[in] m Mode.
 *
 *  @return Pipe stream, or NULL on failure.
 */
extern	FILE	*popen(const char *, const char *);

/*!
 *  @brief Closes a pipe opened by popen.
 *
 *  @param[in] f Pipe stream.
 *
 *  @return Exit status of the command.
 */
extern	int	pclose(FILE *);
