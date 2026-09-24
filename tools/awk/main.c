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
 *  @file main.c
 *  @brief Entry point of the awk interpreter.
 *
 *  Parses the command line, sets up the symbol table, initializes
 *  the input machinery and runs the program: BEGIN, main loop over
 *  records, END. The program text may come from the command line or
 *  from one or more files given with -f.
 *
 *  @copyright Copyright (C) Lucent Technologies 1997.
 */

/*!
 *  @brief Version string reported by -V and -d.
 */
const char      *version = "version 20040207";

#define DEBUG
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "awk.h"
#include "ytab.h"

#ifdef __GNUC__
extern  char    **environ;
#endif
extern  int     nfields;         /*!< Number of fields in the current record. */

int     dbg     = 0;             /*!< Debug level, set by -d. */
char    *cmdname;                /*!< Program name (argv[0]) used in error messages. */
extern  FILE    *yyin;           /*!< Lexer input file. */
char    *lexprog;                /*!< Points to the program text if it was given in the command line. */
extern  int errorflag;           /*!< Non-zero if a syntax error has been reported. */
int     compile_time = 2;        /*!< Error-printing phase: 2 = cmdline, 1 = compile, 0 = running. */

char    *pfile[20];              /*!< Program file names collected from -f. */
int     npfile = 0;              /*!< Number of collected program file names. */
int     curpfile = 0;            /*!< Index of the program file currently being read. */

int     safe    = 0;             /*!< 1 if safe mode was requested with -safe. */

/*!
 *  @brief Entry point of the awk interpreter.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument vector.
 *
 *  @return Exit status of the program.
 *  @retval 0 Successful completion.
 *  @retval 1 Usage error (no arguments).
 *  @retval 2 Fatal error reported by FATAL.
 */
int main(int argc, char *argv[])
{
        const char *fs = NULL;

        setlocale(LC_CTYPE, "");
        setlocale(LC_NUMERIC, "C"); /* for parsing cmdline & prog */
        cmdname = argv[0];
        if (argc == 1) {
                fprintf(stderr, "Usage: %s [-f programfile | 'program'] [-Ffieldsep] [-v var=value] [files]\n", cmdname);
                exit(1);
        }
        signal(SIGFPE, fpecatch);
        yyin = NULL;
        symtab = makesymtab(NSYMTAB);
        while (argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
                if (strcmp(argv[1], "--") == 0) {       /* explicit end of args */
                        argc--;
                        argv++;
                        break;
                }
                switch (argv[1][1]) {
                case 's':
                        if (strcmp(argv[1], "-safe") == 0)
                                safe = 1;
                        break;
                case 'f':       /* next argument is program filename */
                        argc--;
                        argv++;
                        if (argc <= 1)
                                FATAL("no program filename");
                        pfile[npfile++] = argv[1];
                        break;
                case 'F':       /* set field separator */
                        if (argv[1][2] != 0) {  /* arg is -Fsomething */
                                if (argv[1][2] == 't' && argv[1][3] == 0)       /* wart: t=>\t */
                                        fs = "\t";
                                else if (argv[1][2] != 0)
                                        fs = &argv[1][2];
                        } else {                /* arg is -F something */
                                argc--; argv++;
                                if (argc > 1 && argv[1][0] == 't' && argv[1][1] == 0)   /* wart: t=>\t */
                                        fs = "\t";
                                else if (argc > 1 && argv[1][0] != 0)
                                        fs = &argv[1][0];
                        }
                        if (fs == NULL || *fs == '\0')
                                WARNING("field separator FS is empty");
                        break;
                case 'v':       /* -v a=1 to be done NOW.  one -v for each */
                        if (argv[1][2] == '\0' && --argc > 1 && isclvar((++argv)[1]))
                                setclvar(argv[1]);
                        break;
                case 'm':       /* more memory: -mr=record, -mf=fields */
                                /* no longer supported */
                        WARNING("obsolete option %s ignored", argv[1]);
                        break;
                case 'd':
                        dbg = atoi(&argv[1][2]);
                        if (dbg == 0)
                                dbg = 1;
                        printf("awk %s\n", version);
                        break;
                case 'V':       /* added for exptools "standard" */
                        printf("awk %s\n", version);
                        exit(0);
                        break;
                default:
                        WARNING("unknown option %s ignored", argv[1]);
                        break;
                }
                argc--;
                argv++;
        }
        /* argv[1] is now the first argument */
        if (npfile == 0) {      /* no -f; first argument is program */
                if (argc <= 1) {
                        if (dbg)
                                exit(0);
                        FATAL("no program given");
                }
                   dprintf( ("program = |%s|\n", argv[1]) );
                lexprog = argv[1];
                argc--;
                argv++;
        }
        recinit(recsize);
        syminit();
        compile_time = 1;
        argv[0] = cmdname;      /* put prog name at front of arglist */
           dprintf( ("argc=%d, argv[0]=%s\n", argc, argv[0]) );
        arginit(argc, argv);
        if (!safe)
                envinit(environ);
        yyparse();
        setlocale(LC_NUMERIC, ""); /* back to whatever it is locally */
        if (fs)
                *FS = qstring(fs, '\0');
           dprintf( ("errorflag=%d\n", errorflag) );
        if (errorflag == 0) {
                compile_time = 0;
                run(winner);
        } else
                bracecheck();
        return(errorflag);
}

/*!
 *  @brief Reads one character from the awk program source.
 *
 *  @return Next character of the program.
 *  @retval EOF All program files have been exhausted.
 */
int pgetc(void)
{
        int c;

        for (;;) {
                if (yyin == NULL) {
                        if (curpfile >= npfile)
                                return EOF;
                        if (strcmp(pfile[curpfile], "-") == 0)
                                yyin = stdin;
                        else if ((yyin = fopen(pfile[curpfile], "r")) == NULL)
                                FATAL("can't open file %s", pfile[curpfile]);
                        lineno = 1;
                }
                if ((c = getc(yyin)) != EOF)
                        return c;
                if (yyin != stdin)
                        fclose(yyin);
                yyin = NULL;
                curpfile++;
        }
}

/*!
 *  @brief Returns the name of the current source file of the program.
 *
 *  @return Source file name.
 *  @retval NULL The program was given in the command line.
 */
char *cursource(void)
{
        if (npfile > 0)
                return pfile[curpfile];
        else
                return NULL;
}
