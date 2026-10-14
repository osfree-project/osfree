/*  $Id: mkctxt.c 189 2001-04-12 00:09:06Z skaus $

	Create the various file from the CONTEXT.X file
	(See there for syntax description)

	Useage: MKCTXT filename[.X]

	Produces:
	filename.h_c: contains the structure in C syntax
	filename.inc: contains the definition of the structure in assembly
	filename.def: contains the EQUs of the offsets of a value into the structure
		two definitions per offset:
		<value> - offset from the very beginning of the context
		<package>_<value> - offset of the value relative to the beginning
			of the specific package

  	$Log$
  	Revision 1.3  2001/04/12 00:09:06  skaus
  	chg: New structure
  	chg: If DEBUG enabled, no available commands are displayed on startup
  	fix: PTCHSIZE also patches min extra size to force to have this amount
  	   of memory available on start
  	bugfix: CALL doesn't reset options
  	add: PTCHSIZE to patch heap size
  	add: VSPAWN, /SWAP switch, .SWP resource handling
  	bugfix: COMMAND.COM A:\
  	bugfix: CALL: if swapOnExec == ERROR, no change of swapOnExec allowed
  	add: command MEMORY
  	bugfix: runExtension(): destroys command[-2]
  	add: clean.bat
  	add: localized CRITER strings
  	chg: use LNG files for hard-coded strings (hangForEver(), init.c)
  		via STRINGS.LIB
  	add: DEL.C, COPY.C, CBREAK.C: STRINGS-based prompts
  	add: fixstrs.c: prompts & symbolic keys
  	add: fixstrs.c: backslash escape sequences
  	add: version IDs to DEFAULT.LNG and validation to FIXSTRS.C
  	chg: splitted code apart into LIB\*.c and CMD\*.c
  	bugfix: IF is now using error system & STRINGS to report errors
  	add: CALL: /N

  	Revision 1.1.2.2  2001/02/18 21:08:24  skaus
  	add: command WHICH
  	fix: BUILD.BAT and accompanying makefiles for TC++ v1.01
  	
  	Revision 1.1.2.1  2001/02/18 17:59:37  skaus
  	bugfix: KSSF: restore parentPSP on exit
  	chg: Using STRINGS resource for all non-interactive messages
  	chg: moving all assembly files to NASM
  	
  	Revision 1.1.2.1  2000/12/17 21:57:37  skaus
  	intermediate update 1
  	
*/

/*!
 *  @file mkctxt.c
 *  @brief Generator for context-structure definitions from *.X files.
 *
 *  Reads a context description file (a *.X file) and produces three
 *  derived files next to it: a C include (*.h_c), an assembly include
 *  (*.inc) and an assembly offsets definition (*.def). The description
 *  language is a small keyword-based format (typedef / package /
 *  structure / byte / word / pointer / reference / constant) documented
 *  in CONTEXT.X.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static char *typename = NULL;	/*!< Current typedef name. */
static char *pkg = NULL;	/*!< Current package name. */
static char *stru = NULL;	/*!< Current structure name. */
static unsigned pkgOffs = 0, offset = 0;	/*!< Package-relative and overall offsets. */
static char *fnam = NULL;	/*!< Filename with spare room for extension. */
static char *fext;			/*!< Pointer to extension of fnam. */
static char buf[1024];		/*!< Input line buffer. */
static unsigned lineNr = 0;	/*!< Current input line number. */
static FILE *fincc, *finca, *fdefa, *fx;	/*!< Output files and input file. */


/*!
 *  @brief Skips leading whitespace in a string.
 *
 *  @param[in] p Input string.
 *
 *  @return Pointer to the first non-whitespace character.
 */
static char *skipws(char *p)
{	while(*p && isspace(*p))
		++p;
	return p;
}

/*!
 *  @brief Advances both offsets by the size of a field.
 *
 *  @param[in] size Size in bytes of the field just emitted.
 */
static void incOffs(int size)
{	offset += size;
	pkgOffs += size;
}

/*!
 *  @brief Emits a #define / %define pair for a constant.
 *
 *  @param[in] name  Symbolic name of the constant.
 *  @param[in] value Textual value of the constant.
 */
static void dumpDefine(char *name, char *value)
{	long i;

	i = strtol(value, 0, 0);
	fprintf(fincc, "#define %s %ld\n", name, i);
	fprintf(finca, "%%define %s %ld\n", name, i);
}

/*!
 *  @brief Emits EQUs for an offset symbol.
 *
 *  Emits a global offset definition, a per-package offset definition
 *  when a package is active, and a label in the .def file.
 *
 *  @param[in] symbol Symbolic name of the field/structure.
 */
static void dumpEQU(char *symbol)
{	fprintf(finca, "?%s EQU %u\n", symbol, offset);
	fprintf(fdefa, "??%s:\n", symbol);

	if(pkg && pkg != symbol)
		fprintf(finca, "?%s_%s EQU %u\n", pkg, symbol, pkgOffs);
}

/*!
 *  @brief Emits indentation according to the active nesting levels.
 */
static void indent(void)
{	if(typename) putc('\t', fincc);
	if(pkg)	putc('\t', fincc);
	if(stru)	putc('\t', fincc);
}

/*!
 *  @brief Closes the currently open structure, if any.
 */
static void endStru(void)
{	if(stru) {
		fprintf(fincc, "\t\t} %s;\n", stru);
		free(stru);
		stru = NULL;
	}
}

/*!
 *  @brief Closes the currently open package, if any.
 */
static void endPkg(void)
{	if(pkg) {
		fprintf(fincc, "\t} %s;\n", pkg);
		free(pkg);
		pkg = NULL;
	}
}

/*!
 *  @brief Closes the currently open typedef, if any.
 */
static void endType(void)
{	if(typename) {
		fprintf(fincc, "} %s;\n\n#include <algndflt.h>\n", typename);
		free(typename);
		typename = NULL;
	}
}

/*!
 *  @brief Emits a single field of a structure.
 *
 *  Emits the offset definition, the C declaration and the assembly
 *  descriptor for one field.
 *
 *  @param[in] name    Field name.
 *  @param[in] value   Initial value / description for the assembly side.
 *  @param[in] Ctype   C type specifier.
 *  @param[in] asmType Assembly descriptor letter ('B', 'W', 'D').
 */
static void dumpItem(char *name, char *value, char *Ctype, char asmType)
{	dumpEQU(name);
	indent();
	fprintf(fincc, "%s %s;\n", Ctype, name);
	fprintf(fdefa, "\tD%c %s\n", asmType, value);
}

/*!
 *  @brief Fetches the next whitespace-delimited word from the input line.
 *
 *  Convenience wrapper around word_() that hides the @c p variable.
 *
 *  @def word
 */
#define word word_(&p)

/*!
 *  @brief Implementation of the @c word macro.
 *
 *  @param[in,out] Xp Address of the current line pointer.
 *
 *  @return Pointer to the next word inside the line buffer.
 */
static char *word_(char **Xp)
{	char *p, *q;

	q = p = skipws(*Xp);
	if(!*p || *p == '#') {
		printf("Syntax error in line %u: Missing argument\n", lineNr);
		exit(62);
	}
	while(*++q && !isspace(*q));
	if(*q) {
		*q = '\0';
		*Xp = skipws(q + 1);
	} else
		*Xp = q;
	return p;
}

/*!
 *  @brief Tests whether the next word matches the given keyword.
 *
 *  Convenience wrapper around match_() that hides the @c p variable.
 *
 *  @def match
 *  @param[in] w Keyword to match (case-insensitive).
 */
#define match(w) match_(&p, (w))

/*!
 *  @brief Implementation of the @c match macro.
 *
 *  @param[in,out] Xp Address of the current line pointer.
 *  @param[in]     w  Keyword to match (case-insensitive).
 *
 *  @return Non-zero when the keyword matched and was consumed.
 *  @retval 0 The next word is not the requested keyword.
 *  @retval 1 The next word is the requested keyword; it was consumed.
 */
static int match_(char **Xp, char *w)
{	char *p, *q;

	q = p = skipws(*Xp);
	if(!*p || *p == '#') {
		printf("Syntax error in line %u: Missing keyword\n", lineNr);
		exit(62);
	}
	while(*++q && !isspace(*q));
	if(strlen(w) == q - p && memicmp(w, p, q - p) == 0) {
		/* equal */
		if(*q)	*Xp = skipws(q + 1);
		else	*Xp = q;
		return 1;
	}
	return 0;
}

/*!
 *  @brief Entry point of MKCTXT.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument vector; argv[1] is the .X file name.
 *
 *  @return Exit status.
 *  @retval 0   Success.
 *  @retval 40  Input file could not be opened.
 *  @retval 41  C include file could not be created.
 *  @retval 42  Assembly include file could not be created.
 *  @retval 43  Assembly definition file could not be created.
 *  @retval 50  Read error from the input file.
 *  @retval 51  Write error to the C include file.
 *  @retval 52  Write error to the assembly include file.
 *  @retval 53  Write error to the assembly definition file.
 *  @retval 60  Syntax error: invalid keyword.
 *  @retval 61  Syntax error: too many arguments.
 *  @retval 62  Syntax error: missing argument or keyword.
 *  @retval 120 Out of memory.
 *  @retval 127 Wrong usage.
 */
main(int argc, char **argv)
{
	char *p, *name, *value;
	int err;

	if(argc != 2) {
		puts("MKCTXT - Create definition file from *.X files\n\n"
			"Useage: MKCTXT filename[.X]\n");
		return 127;
	}

	if((fnam = strdup(argv[1])) == NULL
	 || (fnam = realloc(fnam, strlen(fnam) + 4)) == NULL) {
	 	puts("Out of memory");
	 	return 120;
	}

	fext = strchr(fnam, '\0');
	while(--fext >= fnam && *fext != '.' && *fext != '/' && *fext != '\\');
	if(fext < fnam || *fext != '.') {
		fext = strchr(fnam, '\0');
		strcpy(fext++, ".x");
	} else ++fext;

	if((fx = fopen(fnam, "rt")) == NULL) {
		printf("Cannot open input file: %s\n", fnam);
		return 40;
	}
	strcpy(fext, "h_c");
	if((fincc = fopen(fnam, "wt")) == NULL) {
		printf("Cannot create C include file: %s\n", fnam);
		return 41;
	}
	strcpy(fext, "inc");
	if((finca = fopen(fnam, "wt")) == NULL) {
		printf("Cannot create assembly include file: %s\n", fnam);
		return 42;
	}
	strcpy(fext, "def");
	if((fdefa = fopen(fnam, "wt")) == NULL) {
		printf("Cannot create assembly definition file: %s\n", fnam);
		return 43;
	}

	/* Dump the headers into the files */
	/* C include file: autogenerated notice and one-byte alignment */
	fputs("/* This is an autogenerated file.\n"
		"\tAny modifications are lost once MKCTXT runs the next time.\n"
		"\tModify the particular *.X source file instead. */\n\n", fincc);
	fputs(";; This is an autogenerated file.\n"
		";;\tAny modifications are lost once MKCTXT runs the next time.\n"
		";;\tModify the particular *.X source file instead. \n\n", finca);
	fputs(";; This is an autogenerated file.\n"
		";;\tAny modifications are lost once MKCTXT runs the next time.\n"
		";;\tModify the particular *.X source file instead. \n\n", fdefa);

	while(fgets(buf, sizeof(buf), fx)) {
		++lineNr;
		p = skipws(buf);
		if(!*p) {
			endStru();
			continue;
		}
		if(*p == '#')
			continue;
		if(match("typedef")) {
			endStru();
			endPkg();
			endType();
			typename = strdup(word);
			if(!typename) {
				puts("Out of memory");
				return 120;
			}
		
			fprintf(fincc, "\n#include <algnbyte.h>\n\n"
			 "typedef struct {\t/* type %s */\n", typename);
		} else if(match("package")) {
			pkgOffs = 0;
			endStru();
			endPkg();
			indent();
			pkg = strdup(word);
			if(!pkg) {
				puts("Out of memory");
				return 120;
			}
			fprintf(fincc, "struct {\t/* package %s */\n", pkg);
			dumpEQU(pkg);
		} else if(match("structure")) {
			endStru();
			indent();
			stru = strdup(word);
			if(!stru) {
				puts("Out of memory");
				return 120;
			}
			fprintf(fincc, "struct {\t/* structure %s */\n", stru);
			dumpEQU(stru);
		} else if(match("byte")) {
			name = word;
			value = word;
			dumpItem(name, value, "byte", 'B');
			incOffs(1);
		} else if(match("word")) {
			name = word;
			value = word;
			dumpItem(name, value, "word", 'W');
			incOffs(2);
		} else if(match("pointer")) {
			name = word;
			value = word;
			dumpItem(name, value, "void far*", 'D');
			incOffs(4);
		} else if(match("reference")) {
			name = word;
			dumpEQU(name);
			indent();
			fprintf(fincc, "unsigned char %s[];\n", name);
		} else if(match("constant")) {
			name = word;
			value = word;
			dumpDefine(name, value);
		} else {
			printf("Syntax error in line %u: Invalid keyword\n", lineNr);
			return 60;
		}

		if(*p && *p != '#') {
			printf("Syntax error in line %u: Too many arguments\n", lineNr);
			return 61;
		}
	}

	endStru();
	endPkg();
	endType();

	fflush(fincc);
	fflush(finca);
	fflush(fdefa);

	err = 0;
	if(ferror(fx)) {
		strcpy(fext, "x");
		printf("Read error from: %s\n", fnam);
		err = 50;
	}
	if(ferror(fincc)) {
		strcpy(fext, "h_c");
		printf("Write error to: %s\n", fnam);
		err = 51;
	}
	if(ferror(finca)) {
		strcpy(fext, "inc");
		printf("Write error to: %s\n", fnam);
		err = 52;
	}
	if(ferror(fdefa)) {
		strcpy(fext, "def");
		printf("Write error to: %s\n", fnam);
		err = 53;
	}
	
	fclose(fx);
	fclose(fincc);
	fclose(finca);
	fclose(fdefa);

	return err;
}
