/*!
 * @file sedcomp.c
 *
 * @brief Stream editor main and compilation phase.
 *
 * sedcomp.c -- stream editor main and compilation phase.
 *
 * The stream editor compiles its command input (from files or -e
 * options) into an internal form using compile() then executes the
 * compiled form using execute(). main() just initializes data
 * structures, interprets command line options, and calls compile()
 * and execute() in appropriate sequence.
 *
 * The data structure produced by compile() is an array of
 * compiled-command structures (type sedcmd). These contain several
 * pointers into pool[], the regular-expression and text-data pool,
 * plus a command code and g & p flags. In the special case that the
 * command is a label the struct will hold a ptr into the labels
 * array labels[] during most of the compile, until resolve()
 * resolves references at the end.
 *
 * The operation of execute() is described in its source module.
 *
 * ==== Written for the GNU operating system by Eric S. Raymond ====
 */

#include <assert.h>
#include <ctype.h>                      /* isdigit(), isspace() */
#include <unistd.h>                     /* isatty() */
#include <stdio.h>                      /* uses getc, fprintf, fopen, fclose */
#include <stdlib.h>                     /* uses exit */
#include <string.h>                     /* imported string functions */
#include "sed.h"                        /* command type struct & name defines */

/*!
 * @def MAXCMDS
 * @brief Maximum number of compiled commands.
 */
#define MAXCMDS         400

/*!
 * @def MAXLINES
 * @brief Maximum number of numeric addresses.
 */
#define MAXLINES        256

                                        /* main data areas */
/*!
 * @brief Current-line buffer.
 */
char            linebuf[MAXBUF + 3];

/*!
 * @brief Compiled-command table.
 */
sedcmd          cmds[MAXCMDS + 1];

/*!
 * @brief Numeric-addresses table.
 */
long            linenum[MAXLINES];

                                        /* miscellaneous shared variables */
/*!
 * @brief -n option flag.
 */
int             nflag = 0;

/*!
 * @brief Scratch copy of the argument count.
 */
int             eargc;

/*!
 * @brief Bitmask table.
 */
char const      bits[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

                                        /***** module common stuff *****/

/*!
 * @def POOLSIZE
 * @brief Size of string-pool space.
 */
#define POOLSIZE        10000

/*!
 * @def WFILES
 * @brief Maximum number of w output files.
 */
#define WFILES          10

/*!
 * @def RELIMIT
 * @brief Maximum number of characters in a compiled regular
 *        expression.
 */
#define RELIMIT         256

/*!
 * @def MAXDEPTH
 * @brief Maximum {}-nesting level.
 */
#define MAXDEPTH        20

/*!
 * @def MAXLABS
 * @brief Maximum number of labels.
 */
#define MAXLABS         50

/*!
 * @def SKIPWS
 * @brief Advance the pointer @p pc past whitespace.
 *
 * @param pc Pointer to advance past whitespace.
 */
#define SKIPWS(pc)      while( isspace( *pc ) ) pc++

/*!
 * @def ABORT
 * @brief Print @p msg with the current line buffer and exit with
 *        status 2.
 *
 * @param msg Format string printed before exiting.
 */
#define ABORT(msg)      fprintf( stderr, msg, linebuf ), myexit( 2 )

/*!
 * @def IFEQ
 * @brief Consume one character @p v from @p x, leaving the cursor
 *        past it.
 *
 * @param x Pointer to the cursor.
 * @param v Character to match and consume.
 */
#define IFEQ(x, v)      if( *x == v ) x++ , /* do expression */

                                        /* error messages */
/*!
 * @brief Error message: garbled address.
 */
static char const       AGMSG[] = "sed: garbled address %s\n";
/*!
 * @brief Error message: garbled command.
 */
static char const       CGMSG[] = "sed: garbled command %s\n";
/*!
 * @brief Error message: too much text.
 */
static char const       TMTXT[] = "sed: too much text: %s\n";
/*!
 * @brief Error message: no addresses allowed for this command.
 */
static char const       AD1NG[] = "sed: no addresses allowed for %s\n";
/*!
 * @brief Error message: only one address allowed for this command.
 */
static char const       AD2NG[] = "sed: only one address allowed for %s\n";
/*!
 * @brief Error message: too many commands.
 */
static char const       TMCDS[] = "sed: too many commands, last was %s\n";
/*!
 * @brief Error message: cannot open command-file.
 */
static char const       COCFI[] = "sed: cannot open command-file %s\n";
/*!
 * @brief Error message: unknown flag.
 */
static char const       UFLAG[] = "sed: unknown flag %c\n";
/*!
 * @brief Error message: cannot create output file.
 */
static char const       CCOFI[] = "sed: cannot create %s\n";
/*!
 * @brief Error message: undefined label.
 */
static char const       ULABL[] = "sed: undefined label \":%s\"\n";
/*!
 * @brief Error message: too many opening braces.
 */
static char const       TMLBR[] = "sed: too many {'s\n";
/*!
 * @brief Error message: no such command.
 */
static char const       NSCAX[] = "sed: no such command as %s\n";
/*!
 * @brief Error message: too many closing braces.
 */
static char const       TMRBR[] = "sed: too many }'s\n";
/*!
 * @brief Error message: duplicate label.
 */
static char const       DLABL[] = "sed: duplicate label \"%s\"\n";
/*!
 * @brief Error message: too many labels.
 */
static char const       TMLAB[] = "sed: too many labels \"%s\"\n";
/*!
 * @brief Error message: too many w files.
 */
static char const       TMWFI[] = "sed: too many w files\n";
/*!
 * @brief Error message: regular expression too long.
 */
static char const       REITL[] = "sed: RE too long: %s\n";
/*!
 * @brief Error message: too many line numbers.
 */
static char const       TMLNR[] = "sed: too many line numbers\n";
/*!
 * @brief Error message: trailing garbage after command.
 */
static char const       TRAIL[] = "sed: command \"%s\" has trailing garbage\n";
/*!
 * @brief Error message: need more bytes in command.
 */
static char const       NEEDB[] = "sed: error processing: %s\n";
/*!
 * @brief Error message: internal error.
 */
static char const       INERR[] = "sed: internal error: %s\n";
/*!
 * @brief Error message: bad value for match count.
 */
static char const       SMCNT[] = "sed: bad value for match count on s command %s\n";
/*!
 * @brief Error message: invalid character class name.
 */
static char const       UNCLS[] = "sed: invalid character class name %s\n";

/*!
 * @brief Usage text printed when the argument list cannot be parsed.
 */
static char const       *USAGE[] = {
    "Usage: sed [-g] [-n] script file ...",
    "       sed [-g] [-n] -e script ... -f script_file ... file ...",
    "",
    "  -g: global substitute             -n: no default copy,",
    "  script: append to commands,       script_file: append contents to commands",
    "  file: a text file - lines of upto 8K bytes, \\n not needed on last line",
    "        default is to read from standard input",
    "",
    "General command format:",
    "    [addr1][,addr2] [!]function [arguments]",
    "    [addr1][,addr2] [!]s/[RE]/replace/[p|P][g][w file]",
    "",
    "Command list:",
    "    a: append,      b: branch,    c: change,   d/D: delete,",
    "    g/G: get,       h/H: hold,    i: insert,   l: list,",
    "    n/N: next,      p/P: print,   q: quit,     r: read,",
    "    s: substitute,  t/T: test,    w/W: write,  x: xchange,",
    "    y: translate,   :label,       =: number,   {}: grouping",
    "",
    "Regular expression elements:",
    "    \\n, and \\t: newline, and tab",
    "    ^: line begin,             $: line end,",
    "    [a-z$0-9]: class,          [^a-z$]: inverse-class,",
    "    .: one character,          *: 0 or more repeats,",
    "    +: one or more repeats,    \\{n,m\\} n to m repeats,",
    "    \\(..\\): subexpression,     \\1..\\9: matched subexpression,",
    "",
    "Substitution patterns:",
    "    \\n,\\t: newline and tab",
    "    &: match pattern,          \\1..\\9: matched subexpression",
    NULL
};

/*!
 * @struct label
 * @brief Represent a command label.
 */
typedef struct                          /* represent a command label */
{
    char        *name;                  /*!< The label name.                  */
    sedcmd      *last;                  /*!< It's on the label search list.   */
    sedcmd      *link;                  /*!< Pointer to the cmd it labels.    */
}               label;

                                        /* label handling */
/*!
 * @brief Here's the label table.
 */
static label    labels[MAXLABS];

/*!
 * @brief First label is end of script.
 */
static label    *curlab = labels + 1;   /* pointer to current label */
/*!
 * @brief Header for the search list.
 */
static label    *lablst = labels;       /* header for search list */

                                        /* string pool for REs, etc. */
/*!
 * @brief The string pool.
 */
static char     pool[POOLSIZE];
/*!
 * @brief Current pool pointer.
 */
static char     *fp     = pool;
/*!
 * @brief Pointer past pool end.
 */
static char     *poolend  = pool + POOLSIZE;

                                        /* compilation state */
/*!
 * @brief Current command source.
 */
static FILE     *cmdf   = NULL;
/*!
 * @brief Compile pointer.
 */
static char     *cp     = NULL;
/*!
 * @brief Current compiled-cmd pointer.
 */
static sedcmd   *cmdp   = cmds;
/*!
 * @brief Current {}-nesting level.
 */
static int      bdepth  = 0;
/*!
 * @brief Number of tagged patterns in the current RE.
 */
static int      bcount  = 0;
/*!
 * @brief Scratch copy of the argument list.
 */
static char     **eargv;

/* compilation flags */
/*!
 * @brief -e option flag.
 */
static int      eflag = 0;
/*!
 * @brief -g option flag.
 */
static int      gflag = 0;

/*!
 * @brief Compiles the sed script from the current command source.
 */
static void     compile( void );

/*!
 * @brief Compiles a single command.
 *
 * @param[in] cchar Character name of the command.
 *
 * @return 1 if the caller should skip reading the next line of
 *         script; otherwise 0.
 *
 * @retval 1  Caller must skip the next line read.
 * @retval 0  Otherwise.
 */
static int      cmdcomp( register char cchar );

/*!
 * @brief Generates the replacement string for the right-hand side
 *        of the s command.
 *
 * @param[in] rhsp  Place to compile the expression to. Not NULL.
 * @param[in] delim RE end-marker to find.
 *
 * @return Pointer just past the replacement string, or BAD on
 *         failure.
 *
 * @retval BAD  Invalid backreference or missing end delimiter.
 */
static char     *rhscomp( register char *rhsp, register char delim );

/*!
 * @brief Compiles a regular expression to internal form.
 *
 * @param[in] expbuf  Place to compile the expression to. Not NULL.
 * @param[in] redelim RE end-marker to look for.
 *
 * @return Pointer just past the compiled expression, or BAD on
 *         failure.
 *
 * @retval BAD  Malformed regular expression.
 */
static char     *recomp( char *expbuf, char redelim );

/*!
 * @brief Reads the next command from an -e argument or a command
 *        file.
 *
 * @param[in] cbuf Destination buffer. Not NULL.
 *
 * @return 1 on success, 0 when there are no more commands.
 *
 * @retval 1  A command line was read.
 * @retval 0  End of input.
 */
static int      cmdline( register char *cbuf );

/*!
 * @brief Expands an address at *cp into expbuf and returns the
 *        pointer to the following character.
 *
 * @param[in] expbuf Destination buffer for the compiled address.
 *                   Not NULL.
 *
 * @return Pointer just past the address, NULL when no legal address
 *         was found, or BAD on a malformed regular expression.
 *
 * @retval NULL  No legal address was found.
 * @retval BAD   Malformed regular expression.
 */
static char     *getaddress( register char *expbuf );

/*!
 * @brief Accepts multiline input from *cp to *fp, optionally
 *        skipping leading whitespace.
 *
 * @param[in] accept_whitespace Non-zero to keep leading whitespace.
 */
static void     gettext( int accept_whitespace );

/*!
 * @brief Finds the label matching curlab->name.
 *
 * @return Pointer to the matching label, or NULL when no label in
 *         the table matches.
 *
 * @retval NULL  No matching label.
 */
static label    *search( void );

/*!
 * @brief Writes label links into the compiled-command space.
 */
static void     resolve( void );

/*!
 * @brief Compiles a y (transliterate) command.
 *
 * @param[in] ep    Where to compile the table to. Not NULL.
 * @param[in] delim End delimiter to look for.
 *
 * @return Pointer just past the translation table, or BAD on
 *         failure.
 *
 * @retval BAD  Malformed y command, or unequal string lengths.
 */
static char     *ycomp( register char *ep, char delim );

/*!
 * @brief Exits the stream editor on an error condition.
 *
 * @param[in] status Non-zero process exit status.
 */
static void     myexit( int status );

/*!
 * @brief Prints the usage text and exits with status 2.
 */
static void     usage( void );

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#ifndef isblank // OW defines isblank without defining __STDC_VERSION__ W.Briscoe 20041008
/*!
 * @brief Query whether a character is a blank (space or tab).
 *
 * Provided for platforms whose libc does not define isblank().
 *
 * @param[in] c  Character.
 *
 * @return Non-zero if @p c is a space or a tab, zero otherwise.
 *
 * @retval 1  @p c is a space or a tab.
 * @retval 0  Otherwise.
 */
static int isblank( int c )
{
    return( c == ' ' || c == '\t' );
}
#endif
#endif

/*!
 * @brief Main sequence of the stream editor.
 *
 * Initializes the label table and compilation state, interprets the
 * command line options, compiles the script (from -e arguments,
 * -f files, or the first non-option argument), resolves labels and
 * runs the compiled script over each input file in turn.
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
 * @return Process exit status.
 *
 * @retval 0  Success.
 *
 * @note Errors print a diagnostic to stderr and terminate the
 *       process via myexit(2).
 */
int main( int argc, char *argv[] )
{
    static char dummy_name[] = "progend\n";

    lablst->name = dummy_name;  /* Must set so strcmp can be done */
    cmdp->addr1 = pool;         /* 1st addr expand will be at pool start */

    if( argc <= 1 )
        usage();                /* exit immediately if no arguments */
    eargc   = argc;             /* set local copy of argument count */
    eargv   = argv;             /* set local copy of argument list */
    /* scan through the arguments, interpreting each one */
    /* We dont use the OW GetOpt() or the POSIX getopt() as we want to do
    * -e i\ hello -e "s/$/ world" */
    while( --eargc > 0 && **++eargv == '-' ) {
        int const       flag = eargv[0][1];

        /* Support "folded" flags such as -ng rather than -n -g */
        if( eargv[0][2] ) {
            char    *fr = eargv[0]+2;
            char    *to;

            switch( flag ) {
            case 'e':
            case 'f':
                to = fr - 2;
                break;
            case 'g':
            case 'n':
                to = fr - 1;
                break;
            default:
                to = fr;
                break;
            }
                                /* Move up remaining data */
            while( ( *to++ = *fr++ ) != 0 ) ;
            ++eargc, --eargv;   /* Consider rest of argument "again" */
        }

        switch( flag ) {
        case 'e':
            eflag++;
            compile();          /* compile with e flag on */
            eflag = 0;
            break;              /* get another argument */
        case 'f':
            if( --eargc <= 0 )  /* barf if no -f file */
                fprintf( stderr, NEEDB, eargv[0] ), myexit( 2 );
            if( ( cmdf = fopen( *++eargv, "r" ) ) == NULL )
                fprintf( stderr, COCFI, *eargv ), myexit( 2 );
            compile();          /* file is O.K., compile it */
            fclose( cmdf );
            break;              /* go back for another argument */
        case 'g':
            gflag++;            /* set global flag on all s cmds */
            break;
        case 'n':
            nflag++;            /* no print except on p flag or w */
            break;
        default:
            fprintf( stderr, UFLAG, flag );
            eargv++, eargc--;   /* Skip garbage argument */
            break;
        }
    }
    if( cp == NULL ) {          /* no commands have been compiled */
        if( eargc <= 0 )
            usage();            /* exit immediately if no commands */
        eargv--;
        eargc++;
        eflag++;
        compile();
        eflag = 0;
        eargv++;
        eargc--;
    }

    if( bdepth )                /* we have unbalanced squigglies */
        ABORT( TMLBR );

    lablst->link = cmdp;        /* set up header of label linked list */
    resolve();                  /* resolve label table indirections */
    (void)setvbuf( stdout, NULL, _IOLBF, 0 ); /* Improve reactivity in a pipe */
    if( eargc <= 0 ) {           /* if there are no files specified */
        (void)setvbuf( stdin, NULL, _IOLBF, 0 ); /* Improve reactivity in a pipe */
        execute( NULL );        /*   execute commands on stdin only */
    } else while( --eargc >= 0 )  /* else do commands on each file specified */
        execute( *eargv++ );
    return( 0 );                /* everything was O.K. if we got here */
}

/*!
 * @brief Precompile sed commands out of a file.
 *
 * Reads command lines from the current command source (an -e
 * argument, a -f file, or the first non-option argument), parses
 * addresses, negation, the command character and any arguments,
 * and fills the cmds[] table until end of input is reached.
 *
 * @note Terminates via ABORT on any parse error.
 */
static void compile( void )
{
    /*!
     * @def H
     * @brief 128 bit, on if there's code for command.
     */
    #define H       0x80
    /*!
     * @def LOWCMD
     * @brief = '8', lowest char indexed in cmdmask.
     */
    #define LOWCMD  56

    /* indirect through this to get command internal code, if it exists */
    static char const   cmdmask[] = {
        0,     0,      H,      0,      0,       H+EQCMD, 0,           0, /* 89:;<=>? */
        0,     0,      0,      0,      H+CDCMD, 0,       0,       CGCMD, /* @ABCDEFG */
        CHCMD, 0,      0,      0,      0,       0,       CNCMD,       0, /* HIJKLMNO */
        CPCMD, 0,      0,      0,      H+CTCMD, 0,       0,     H+CWCMD, /* PQRSTUVW */
        0,     0,      0,      0,      0,       0,       0,           0, /* XYZ[\]^_ */
        0,     H+ACMD, H+BCMD, H+CCMD, DCMD,    0,       0,        GCMD, /* `abcdefg */
        HCMD,  H+ICMD, 0,      0,      H+LCMD,  0,       NCMD,        0, /* hijklmno */
        PCMD,  H+QCMD, H+RCMD, H+SCMD, H+TCMD,  0,       0,      H+WCMD, /* pqrstuvw */
        XCMD,  H+YCMD, 0,      H+BCMD, 0,       H,       0,           0, /* xyz{|}~  */
    };

    char                ccode;

    for( ;; ) {                         /* main compilation loop */
        if( !cp ) {
            if( !cmdline( cp = linebuf ) )
                break;
            if( *cp == '#' ) {          /* if the first two characters in the script are "#n" , */
                if( cp[1] == 'n' )
                    nflag++;            /* the default output shall be suppressed */
                continue;
            }
        } else if( *cp != ';' )         /* get a new command line */
            if( !cmdline( cp = linebuf ) )
                break;
        SKIPWS( cp );
        if( *cp == '\0' || *cp == '#' ) /* a comment */
            continue;
        while( *cp == ';' || isspace( *cp ) )
            cp++;                       /* ; separates cmds */

                                        /* compile first address */
        if( fp >= poolend )
            ABORT( TMTXT );             /* Not exercised by sedtest.mak */
        if( ( fp = getaddress( cmdp->addr1 = fp ) ) == BAD )
            ABORT( AGMSG );

        if( fp == cmdp->addr1 ) {       /* if empty RE was found */
        } else if( fp == NULL ) {       /* if fp was NULL */
            fp = cmdp->addr1;           /* use current pool location */
            cmdp->addr1 = NULL;
        } else {
            if( *cp == ',' || *cp == ';' ) { /* there's 2nd addr */
                cp++;
                if( fp >= poolend )
                    ABORT( TMTXT );     /* Not exercised by sedtest.mak */
                fp = getaddress( cmdp->addr2 = fp );
                if( fp == BAD || fp == NULL )
                    ABORT( AGMSG );
            } else
                cmdp->addr2 = NULL;     /* no 2nd address */
        }
        if( fp >= poolend )
            ABORT( TMTXT );             /* Not exercised by sedtest.mak */

        SKIPWS( cp );                   /* discard whitespace after address */
        IFEQ( cp, '!' )
            cmdp->flags.allbut = 1;

        SKIPWS( cp );                   /* get cmd char, range-check it */
        if( ( *cp < LOWCMD ) || ( *cp > '~' )
                 || ( ( ccode = cmdmask[*cp - LOWCMD] ) == 0 ) )
            ABORT( NSCAX );

        cmdp->command = ccode & ~H;     /* fill in command value */
        if( ( ccode & H ) == 0 )        /* if no compile-time code */
            cp++;                       /* discard command char */
        else if( cmdcomp( *cp++ ) )     /* execute it; if ret = 1 */
            continue;                   /* skip next line read */

        if( ++cmdp >= cmds + MAXCMDS )
            ABORT( TMCDS );             /* Not exercised by sedtest.mak */

        SKIPWS( cp );                   /* look for trailing stuff */
        if( *cp != '\0' )
            if( *cp == ';' )
                continue;
            else if( *cp != '#' )
                ABORT( TRAIL );
    }
}

/*!
 * @brief Compile a single command.
 *
 * Fills in the current command slot (cmdp) for the command whose
 * character is @p cchar and consumes any command-specific
 * arguments from the compile pointer cp.
 *
 * @param[in] cchar  Character name of the command.
 *
 * @return 1 if the caller should skip reading the next line of
 *         script (used by { and }), 0 otherwise.
 *
 * @retval 1  Caller must skip the next line read.
 * @retval 0  Otherwise.
 *
 * @note Terminates via ABORT on any parse error.
 */
static int cmdcomp( register char cchar ) /* character name of command */
{
    static sedcmd       **cmpstk[MAXDEPTH]; /* current cmd stack for {} */
    static char const   *fname[WFILES]; /* w file name pointers */
    static FILE         *fout[WFILES];  /* w file file ptrs */
    static int          nwfiles = 1;    /* count of open w files */
    int                 i;              /* indexing dummy used in w */
    sedcmd              *sp1;           /* temps for label searches */
    sedcmd              *sp2;           /* temps for label searches */
    label               *lpt;           /* ditto, and the searcher */
    char                redelim;        /* current RE delimiter */

    switch( cchar ) {
    case '{':                           /* start command group */
        cmdp->flags.allbut = !cmdp->flags.allbut;
        cmpstk[bdepth++] = &( cmdp->u.link );
        if( ++cmdp >= cmds + MAXCMDS )
            ABORT( TMCDS );             /* Not exercised by sedtest.mak */
        if( *cp != '\0' )
            *--cp = ';';                /* get next cmd w/o lineread */
        return( 1 );

    case '}':                           /* end command group */
        if( cmdp->addr1 )
            ABORT( AD1NG );             /* no addresses allowed */
        if( --bdepth < 0 )
            ABORT( TMRBR );             /* too many right braces */
        *cmpstk[bdepth] = cmdp - 1;     /* set the jump address - (-1) to allow increment */
        return( 1 );

    case '=':                           /* print current source line number */
        break;

    case 'q':                           /* exit the stream editor */
        if( cmdp->addr2 )
            ABORT( AD2NG );
        break;

    case ':':                           /* label declaration */
        if( cmdp->addr1 )
            ABORT( AD1NG );             /* no addresses allowed */
        curlab->name = fp;
        gettext( 0 );                   /* get the label name */
        if( ( lpt = search() ) != 0 ) { /* does it have a double? */
            if( lpt->link )
                ABORT( DLABL );         /* yes, abort */
        } else {                        /* check label table doesn't overflow */
            if( curlab >= labels + MAXLABS )
                ABORT( TMLAB );         /* Not exercised by sedtest.mak */
            curlab->last = NULL;
            lpt = curlab++;
        }
        lpt->link = cmdp;
        return( 1 );

    case 'b':                           /* branch command */
    case 't':                           /* branch-on-succeed command */
    case 'T':                           /* branch-on-fail command */
        SKIPWS( cp );
        if( *cp == '\0' ) {             /* if branch is to start of cmds... */
                                        /* append command to label last */
            if( ( sp1 = lablst->last ) != 0 ) {
                while( ( sp2 = sp1->u.link ) != 0 )
                    sp1 = sp2;
                sp1->u.link = cmdp;
            } else                      /* lablst->last == NULL */
                lablst->last = cmdp;
            break;
        }
        curlab->name = fp;
        gettext( 0 );                   /* get the label name */
        if( ( lpt = search() ) != 0 ) { /* enter branch to it */
            if( lpt->link )
                cmdp->u.link = lpt->link - 1;
            else {
                sp1 = lpt->last;
                while( ( sp2 = sp1->u.link ) != 0 )
                    sp1 = sp2;
                sp1->u.link = cmdp - 1;
            }
        }
        else {                          /* matching named label not found */
            if( curlab >= labels + MAXLABS ) /* overflow if last */
                ABORT( TMLAB );         /* Not exercised by sedtest.mak */
            curlab->last = cmdp;        /* add the new label */
            curlab->link = NULL;        /* it's forward of here */
            curlab++;                   /* Get another label "object" */
        }
        break;

    case 'a':                           /* append text */
    case 'i':                           /* insert text */
    case 'r':                           /* read file into stream */
        if( cmdp->addr2 )
            ABORT( AD2NG );
                                        /* Drop through */
    case 'c':                           /* change text */
        if( *cp == '\\' && *++cp == '\n' )
            cp++;
        cmdp->u.lhs = fp;
        gettext( 1 );
        break;

    case 'D':                           /* delete current line in pattern space */
        cmdp->u.link = cmds;
        break;

    case 's':                           /* substitute regular expression */
        redelim = *cp++;                /* get delimiter from 1st ch */
        if( ( fp = recomp( cmdp->u.lhs = fp, redelim ) ) == BAD )
            ABORT( CGMSG );
        if( ( cmdp->rhs = fp ) >= poolend )
            ABORT( TMTXT );            /* Not exercised by sedtest.mak */
        if( ( fp = rhscomp( cmdp->rhs, redelim ) ) == BAD )
            ABORT( CGMSG );
        if( gflag )
            cmdp->flags.global++;
        while( *cp == 'g' || *cp == 'p' || *cp == 'P' || isdigit( *cp ) ) {
            IFEQ( cp, 'g' )
                cmdp->flags.global++;
            IFEQ( cp, 'p' )
                cmdp->flags.print = 1;
            IFEQ( cp, 'P' )
                cmdp->flags.print = 2;
            if( isdigit( *cp ) ) {
                i = 0;
                while (isdigit(*cp))
                    i = i*10 + *cp++ - '0';
                if (i == 0 || i >= 2048)
                    ABORT(SMCNT);
                cmdp->flags.nthone = (unsigned)i;
            }
        }
                                        /* Drop through */
    case 'l':                           /* list pattern space */
        if( *cp != 'w' )
            break;                      /* s or l is done */
        cp++;                           /* and execute a w command! */
                                        /* drop through */
    case 'w':                           /* write-pattern-space command */
    case 'W':                           /* write-first-line command */
        if( nwfiles >= WFILES )
            ABORT( TMWFI );
        fout[0] = stdout;               /* Not initialized to humor lint */
        fname[0] = "";                  /* Set so strcmp( x, fname[0] ) OK */
        fname[nwfiles] = (const char*)fp; /* filename is in pool */
        gettext( 0 );
        for( i = nwfiles - 1; i >= 0; i-- ) /* match it in table */
            if( strcmp( fname[nwfiles], fname[i] ) == 0 ) {
                cmdp->fout = fout[i];
                return( 0 );
            }
                                        /* if didn't find one, open new file */
        if( ( cmdp->fout = fopen( fname[nwfiles], "w" ) ) == NULL
                                        /* setvbuf() for -w -r combination */
        || 0 != setvbuf( cmdp->fout, NULL, _IOLBF, 0 ) )
            fprintf( stderr, CCOFI, fname[nwfiles] ), myexit( 2 );
        fout[nwfiles++] = cmdp->fout;
        break;

    case 'y':                           /* transliterate text */
        fp = ycomp( cmdp->u.lhs = fp, *cp++ );    /* compile translit */
        if( fp == BAD )                 /* fail on bad form */
            ABORT( CGMSG );
        if( fp >= poolend )             /* fail on overflow */
            ABORT( TMTXT );             /* Not exercised by sedtest.mak */
        break;

    default:
        fprintf( stderr, INERR, "Unmatched command" ), myexit( 2 );
    }
    return( 0 );                        /* interpreted one command */
}

/*!
 * @brief Generate replacement string for the substitute command
 *        right hand side.
 *
 * Copies the replacement expression from cp to @p rhsp, marking
 * backreferences with bit 0x80 and terminating the string at the
 * RE delimiter @p delim.
 *
 * @param[in]  rhsp   Place to compile the expression to. Not NULL.
 * @param[in]  delim  RE end-marker to find.
 *
 * @return Pointer just past the replacement string, or BAD on
 *         failure.
 *
 * @retval BAD  Invalid backreference or missing end delimiter.
 *
 * @note Uses the shared bcount variable.
 */
static char *rhscomp(
    register char       *rhsp,          /* place to compile expression to */
    register char       delim )         /* RE end-mark to find */
                                        /* uses bcount */
{
    register char       *p = cp;        /* strictly for speed */

    for( ;; )
        if( ( *rhsp = *p++ ) == '\\' ) { /* copy; if it's a \, */
            *rhsp = *p++;               /* copy escaped char */
                                        /* check validity of pattern tag */
            if( *rhsp > bcount + '0' && *rhsp <= '9' )
                return( BAD );
            *rhsp++ |= 0x80;            /* mark the good ones */
        } else if( *rhsp == delim ) {   /* found RE end, hooray... */
            *rhsp++ = '\0';             /* cap the expression string */
            cp = p;
            return( rhsp );             /* pt at 1 past the RE */
        } else if( *rhsp++ == '\0' )    /* last ch not RE end, help! */
            return( BAD );
}

/*!
 * @brief Compile a regular expression to internal form.
 *
 * Reads a regular expression from cp, terminated by @p redelim,
 * and writes the internal compiled form to @p expbuf. Handles
 * tagged subexpressions, *, +, \{n,m\}, character classes and the
 * standard escape sequences.
 *
 * @param[in] expbuf   Place to compile the expression to. Not NULL.
 * @param[in] redelim  RE end-marker to look for.
 *
 * @return Pointer just past the compiled expression, or BAD on
 *         failure.
 *
 * @retval BAD  Malformed regular expression.
 *
 * @note Uses the shared cp and bcount variables; updates them.
 */
static char *recomp(
    char            *expbuf,            /* place to compile it to */
    char            redelim )           /* RE end-marker to look for */
                                        /* uses cp, bcount */
{
    register char   *ep = expbuf;       /* current-compiled-char pointer */
    register char   *sp = cp;           /* source-character ptr */
    register int    c;                  /* current-character */
    char            negclass;           /* all-but flag */
    char            *lastep;            /* ptr to last expr compiled */
    char const      *svclass;           /* start of current char class */
    char            brnest[MAXTAGS+1];  /* bracket-nesting array */
    char            *brnestp;           /* ptr to current bracket-nest */
    char const      *pp;                /* scratch pointer */
    int             tags;               /* # of closed tags */
    char            *obr[MAXTAGS+1] = {0}; /* ep values when \( seen */
    int             opentags = 0;       /* Used to index obr */

    if( *cp == redelim )                /* if first char is RE endmarker */
        return( cp++, *ep++ = CEOF, ep ); /* use existing RE. */

    lastep = NULL;                      /* there's no previous RE */
    brnestp = brnest;                   /* initialize ptr to brnest array */
    tags = bcount = 0;                  /* initialize counters */

    if( ( *ep++ = (int)( *sp == '^' ) ) != 0 ) /* check for start-of-line syntax */
        sp++;

    for( ;; ) {
        if( ep >= expbuf + RELIMIT )    /* match is too large */
            return( cp = sp, BAD );     /* Not exercised by sedtest.mak */
        if( ( c = *sp++ ) == redelim ) {/* found the end of the RE */
            cp = sp;
            if( brnestp != brnest )     /* \(, \) unbalanced */
                return( BAD );
            *ep++ = CEOF;               /* write end-of-pattern mark */
            return( ep );               /* return ptr to compiled RE */
        }
        if( c == '*'
        ||  c == '+'
        ||  c == '\\' && *sp == '{' )   /* if we're a postfix op */
            ;
        else
            lastep = ep;                /* get ready to match last */

        switch( c ) {
        case '\\':
            switch( c = *sp++ ) {
            case '(':                   /* start tagged section */
                if( ++bcount <= MAXTAGS ) { /* bump tag count */
                    *brnestp++ = (char)bcount; /* update tag stack */
                    obr[opentags] = ep; /* Remember for /(.../)* */
                }
                opentags++;
                *ep++ = CBRA;           /* enter tag-start */
                *ep++ = (char)bcount;
                break;
            case ')':                   /* end tagged section */
                if( --opentags < 0 )    /* extra \) */
                    return( cp = sp, BAD );
                *ep++ = CKET;           /* enter end-of-tag */
                if( ++tags <= MAXTAGS ) /* count closed tags */
                    *ep++ = *--brnestp; /* pop tag stack */
                else
                    *ep++ = 0;          /* Placeholder - should not be used */
                break;
            case '\n':                  /* escaped newline no good */
                return( cp = sp, BAD );
            case 'n':                   /* match a newline */
                c = '\n';
                goto defchar;
            case 't':                   /* match a tab */
                c = '\t';
                goto defchar;
            case '\\':                  /* match a literal backslash */
                goto defchar;
            default:
                if( c >= '1' && c <= '9' ) { /* tag use */
                    if( ( c -= '0' ) > tags ) /* too few */
                        return( BAD );
                    *ep++ = CBACK;      /* enter tag mark */
                    *ep++ = (char)c;    /* and the number */
                    break;
                }
#if 1
                /* This allows \ to stop "special" even if it is not. */
                goto defchar;           /* else match \c */
#else
                /* This is IEEE 2001 behavior */
                return( cp = sp, BAD ); /* Otherwise /\?/ && /\\?/ synonymous */
#endif
            case '{':                   /* '}' should balance for vi */
                {
                    int i1 = 0;
                    int i2 = 0;

                    if( !lastep )
                        return( cp = sp, BAD ); /* rep error */
                    *lastep |= MTYPE;
                    if( !isdigit( *sp ) )
                        return( cp = sp, BAD );
                    while( isdigit( *sp ) )
                        i1 = i1*10 + *sp++ - '0';
                    if( i1 > 255 )
                        return( cp = sp, BAD );
                    *ep++ = (char)i1;
                    if( *sp == '\\' && sp[1] == /* '{' vi brace balancing */ '}' )
                        sp += 2, *ep++ = 0;
                    else if( *sp == ',' && sp[1] == '\\' && sp[2] == /* '{' vi brace balancing */ '}' )
                        sp += 3, *ep++ = (char)0xFF;
                    else if( *sp++ ==',') {
                      if ( !isdigit( *sp ) )
                          *ep++ = 255;
                      else {
                        while (isdigit( *sp ) )
                            i2 = i2*10 + *sp++ - '0';
                        *ep++ = (char)(i2-i1);
                      }
                      if( *sp != '\\' || sp[1] != /* '{' vi brace balancing */ '}' || i2 < i1 || i2 > 255 )
                          return( cp = sp, BAD );
                      sp += 2;
                    } else
                        return( cp = sp, BAD );
                }
                goto handle_cket;
            }
            break;

        case '\n':                      /* Can not happen? WFB 20040801 */
        case '\0':
            return( cp = sp, BAD );     /* incomplete regular expression */

        case '.':                       /* match any char except newline */
            *ep++ = CDOT;
            break;

        case '+':                       /* 1 to n repeats of previous pattern */
            if( lastep == NULL )        /* if + not first on line */
                goto defchar;           /*   match a literal + */
#if 0                                   /* Removed constraint WFB 20040804 */
            if( *lastep == CKET )       /* can't iterate a tag */
                return( cp = sp, BAD );
#endif
            pp = ep;                    /* else save old ep */
            while( lastep < pp )        /* so we can blt the pattern */
                *ep++ = *lastep++;
            *lastep |= STAR;            /* flag the copy */
            break;

        case '*':                       /* 0..n repeats of previous pattern */
            if( lastep == NULL )        /* if * isn't first on line */
                goto defchar;           /*   match a literal * */
#if 0                                   /* Removed constraint WFB 20040804 */
            if( *lastep == CKET )       /* can't iterate a tag */
                return( cp = sp, BAD );
#endif
            *lastep |= STAR;            /* flag previous pattern */
            goto handle_cket;

        case '$':                       /* match only end-of-line */
            if( *sp != redelim )        /* if we're not at end of RE */
                goto defchar;           /*   match a literal $ */
            *ep++ = CDOL;               /* insert end-symbol mark */
            break;

        case '[':                       /* begin character set pattern */
            {
                int                     classct; /* class element count */
                register unsigned       uc; /* current-character */

                if( ep + 17 >= expbuf + RELIMIT )
                    ABORT( REITL );     /* Not exercised by sedtest.mak */
                *ep++ = CCL;            /* insert class mark */
                if( ( negclass = (int)( ( uc = *sp++ ) == '^' ) ) != 0 )
                    uc = *sp++;
                svclass = sp;           /* save ptr to class start */
                do {
                    switch( uc ) {
                    case '\0':
                        ABORT( CGMSG );
                    case '-':           /* handle character ranges */
                        if( sp > svclass && *sp != ']' ) {
                            unsigned const  lo = (unsigned)*( sp - 2 );
                            unsigned const  hi = (unsigned)*sp;
                            if( lo > hi )
                                ABORT( CGMSG );
                            if( sp[1] == '-' && sp[2] != ']' )
                                ABORT( CGMSG );
                            for( uc = lo; uc <= hi; uc++ )
                                ep[uc >> 3] |= bits[uc & 7];
                            continue;
                        }
                        break;
                    case '\\':          /* handle escape sequences in sets */
                        if( ( uc = *sp++ ) == 'n' )
                            uc = '\n';
                        else if( uc == 't' )
                            uc = '\t';
                        else
                            --sp, uc = '\\'; /* \n and \t are special, \* is not */
                    case '[':           /* Handle named character class */
                        if( *sp == ':' ) {
                            static const struct {
                                const char *s;
                                int ( *isf )( int c );
                            } t[] = {
                                { "alnum:]", isalnum },
                                { "alpha:]", isalpha },
                                { "blank:]", isblank },
                                { "cntrl:]", iscntrl },
                                { "digit:]", isdigit },
                                { "graph:]", isgraph },
                                { "lower:]", islower },
                                { "print:]", isprint },
                                { "punct:]", ispunct },
                                { "space:]", isspace },
                                { "upper:]", isupper },
                                { "xdigit:]", isxdigit },
                            };
                            size_t si;
                            int ( *isf )( int c );

                            for( si = 0; si < sizeof t / sizeof *t; si++ )
                                if( strncmp( sp+1, t[si].s, strlen( t[si].s ) ) == 0 )
                                    break;
                            if( si >= sizeof t / sizeof *t ) /* Add class */
                                ABORT( UNCLS );
                            sp += 1 + strlen( t[si].s );
                            for( isf = t[si].isf, uc = 1; uc < 128; uc++ )
                                if( isf( (int)uc ) )
                                    ep[uc >> 3] |= bits[uc & 7];
                            continue;
                        }
                        break;
                    }
                                        /* add (maybe translated) char to set */
                    ep[uc >> 3] |= bits[uc & 7];
                } while ( ( uc = *sp++ ) != ']' );
                                        /* invert bitmask if all-but needed */
                if( negclass )
                    for( classct = 0; classct < 16; classct++ )
                        ep[classct] ^= 0xFF;
                ep[0] &= 0xFE;          /* never match ASCII 0 */
                ep += 16;               /* advance ep past set mask */
            }
            break;

        defchar:                        /* match literal character */
        default:                        /* which is what we'd do by default */
            *ep++ = CCHR;               /* insert character mark */
            *ep++ = (char)c;
            break;

        handle_cket:
            switch( *lastep & ~STAR & ~MTYPE ) {
            case CCHR:
            case CDOT:
            case CCL:
                break;
            case CBRA:
                return( cp = sp, BAD );
            case CKET: {
                    /* Make room to insert skip marker in expbuf */
                    char * const    firstep = obr[opentags];
                    int const       width = ep - firstep;
                    int             i = width;

                    if( width >= 256 )
                        ABORT( REITL ); /* Not exercised by sedtest.mak */
                    *firstep |= ( *lastep ^ CKET ) ; /* Mark \( as * or \{ terminated */
                    while( --i >= 2 )
                        firstep[i+1] = firstep[i];
                    firstep[2] = (char)width-1;
                    lastep++;
                    ep++;
                }
                break;
            case CBACK:
                break;
            case CEOF: /* Can't happen - would require * after end of expression WFB 20040813 */
            case CNL:  /* Can't happen - * is literal after special ^ */
            case CDOL: /* Can't happen - $ is literal if not last */
            case CLNUM:/* Can't happen - * after line number is nonsense */
            case CEND: /* Can't happen - CEND is always followed by CEOF */
            default:
                fprintf( stderr, INERR, "Unexpected symbol in RE" ), myexit( 2 );
            }
            break;
        } /* switch( c ) */
    } /* for( ;; ) */
}

/*!
 * @brief Read the next command from an -e argument or a command
 *        file.
 *
 * Copies the next line of script into @p cbuf and terminates it
 * with NUL. Tracks the multi-argument -e state via the eflag
 * variable, and reads from the -f command file otherwise.
 *
 * @param[out] cbuf  Destination buffer. Not NULL.
 *
 * @return 1 on success, 0 when there are no more commands.
 *
 * @retval 1  A command line was read.
 * @retval 0  End of input.
 *
 * @note Uses eflag, eargc, cmdf. Terminates via myexit on missing
 *       -e/-f argument.
 */
static int cmdline( register char *cbuf ) /* uses eflag, eargc, cmdf */
{
    register int        inc;            /* not char because must hold EOF */

    assert( cbuf == cp );
    cbuf--;                             /* so pre-increment points us at cbuf */

                                        /* e command flag is on */
    if( eflag ) {
        register char   *p;             /* ptr to current -e argument */
        static char     *savep;         /* saves previous value of p */

        if( eflag > 0 ) {               /* there are pending -e arguments */
            eflag = -1;
            if( --eargc <= 0 )          /* barf if no argument */
                fprintf( stderr, NEEDB, eargv[0] ), myexit( 2 );
                                        /* else copy next e argument to cbuf */
            p = *++eargv;
            while( ( *++cbuf = *p++ ) != 0 )
                if( *cbuf == '\\' ) {   /* Could not sedtest this! WFB 20040802 */
                    if( ( *++cbuf = *p++ ) == '\0' ) {
                        if( --eargc <= 0 ) /* barf if no argument */
                            fprintf( stderr, NEEDB, eargv[0] ), myexit( 2 );
                        *cbuf = '\n';
                        p = *++eargv;
                    }
                } else if( *cbuf == '\n' ) { /* end of 1 cmd line */
                    *cbuf = '\0';
                    return( savep = p, 1 );
                                        /* we'll be back for the rest... */
                }
                                        /* have string-end;
                                         * can advance to next argument */
            return( savep = NULL, 1 );
        }

        if( ( p = savep ) == NULL )
            return( 0 );

        while( ( *++cbuf = *p++ ) != 0 )
            if( *cbuf == '\\' ) {
                if( ( *++cbuf = *p++ ) == '0' )
                    return( savep = NULL, 0 );
            } else if( *cbuf == '\n' ) {
                *cbuf = '\0';
                return( savep = p, 1 );
            }

        return( savep = NULL, 1 );
    }

                                        /* if no -e flag
                                         * read from command file descriptor */
    while( ( inc = getc( cmdf ) ) != EOF ) { /* get next char */
        switch( *++cbuf = (char)inc ) {
        case '\\':                      /* if it's escape */
            inc = getc( cmdf ),         /* get next char */
            *++cbuf = (char)inc;
            break;
        case '\n':                      /* end on newline */
            return( *cbuf = '\0', 1 );  /* cap the string */
        }
    }

    return( *++cbuf = '\0', cbuf >= cp+1 );       /* end-of-file, no more chars */
}

/*!
 * @brief Expand an address at *cp into expbuf and return the
 *        pointer to the following character.
 *
 * Recognizes $ (end of source), a regular-expression address
 * (introduced by \\ or /), and a decimal numeric address. Numeric
 * addresses are stored in the linenum[] table.
 *
 * @param[in] expbuf  Destination buffer for the compiled address.
 *                    Not NULL.
 *
 * @return Pointer just past the address, NULL when no legal address
 *         was found, or BAD on a malformed regular expression.
 *
 * @retval NULL  No legal address was found.
 * @retval BAD   Malformed regular expression.
 *
 * @note Uses cp and linenum. Terminates via ABORT when the
 *       linenum[] table overflows.
 */
static char *getaddress( register char *expbuf ) /* uses cp, linenum */
{
    static int          numl = 0;       /* current ind in addr-number table */
    register char       *rcp;           /* temp compile ptr for forwd look */
    long                lno;            /* computed value of numeric address */

    switch( *cp ) {
    case '$':                           /* end-of-source address */
        *expbuf++ = CEND;               /* write symbolic end address */
        *expbuf++ = CEOF;               /* and the end-of-address mark (!) */
        cp++;                           /* go to next source character */
        return( expbuf );               /* we're done */
    case '\\':                          /* posix \cBREc address */
        cp++;                           /* Point to delimiter */
    case '/':                           /* start of regular-expression match */
        return( recomp( expbuf, *cp++ ) ); /* compile the RE */
    }
    rcp = cp;
    lno = 0;                            /* now handle a numeric address */
    while( isdigit( *rcp ) )            /* collect digits */
        lno = lno*10 + *rcp++ - '0';    /* compute their value */

    if( rcp > cp ) {                    /* if we caught a number... */
        *expbuf++ = CLNUM;              /* put a numeric-address marker */
        *expbuf++ = (char)numl;         /* and the address table index */
        linenum[numl++] = lno;          /* and set the table entry */
                                        /* Not exercised by sedtest.mak */
        if( numl >= MAXLINES )          /* oh-oh, address table overflow */
            ABORT( TMLNR );             /*   abort with error message */
        *expbuf++ = CEOF;               /* write the end-of-address marker */
        cp = rcp;                       /* point compile past the address */
        return( expbuf );               /* we're done */
    }

    return( NULL );                     /* no legal address was found */
}

/*!
 * @brief Accept multiline input from *cp to *fp, optionally skipping
 *        leading whitespace.
 *
 * Copies text from the compile pointer cp into the string pool at
 * fp, handling backslash escapes and embedded newlines, until a NUL
 * byte is copied. When @p accept_whitespace is zero, leading
 * whitespace at the start is skipped.
 *
 * @param[in] accept_whitespace  Non-zero to keep leading whitespace.
 *
 * @note Uses cp and fp; updates them.
 */
static void gettext( int accept_whitespace )
{
    char                c;

    if( !accept_whitespace )
        SKIPWS( cp );                   /* discard whitespace */
    while( fp < poolend && ( c = *fp++ = *cp++ ) != 0 ) {
        switch( c )
        {
        case '\\':                      /* handle escapes */
            fp[-1] = *cp++;
            break;
        case '\n':                      /* SKIPWS after newline */
            SKIPWS( cp );
            break;
        }
    }
    if( fp >= poolend )
        ABORT( TMTXT );                 /* Not exercised by sedtest.mak */
    --cp;
    return;
}

/*!
 * @brief Find the label matching curlab->name.
 *
 * @return Pointer to the matching label, or NULL when no label in
 *         the table matches.
 *
 * @retval NULL  No matching label.
 *
 * @note Uses the globals lablst and curlab.
 */
static label *search( void )            /* uses globals lablst and curlab */
{
    register label      *rp;
    char const * const  name = curlab->name;
    for( rp = lablst; rp < curlab; rp++ )
        if( strcmp( rp->name, name ) == 0 )
            return( rp );
    return( NULL );
}

/*!
 * @brief Write label links into the compiled-command space.
 *
 * Walks the label table and patches every forward branch whose
 * target was not yet known at parse time. Terminates via myexit(2)
 * if a label was never defined.
 *
 * @note Uses the global lablst.
 */
static void resolve( void )             /* uses global lablst */
{
    register label const        *rp;
    register sedcmd             *rptr;
    register sedcmd             *trptr;
                                        /* loop through the label table */
    for( rp = lablst; rp < curlab; rp++ )
        if( rp->link == NULL )          /* barf if not defined */
            fprintf( stderr, ULABL, rp->name ), myexit( 2 );
        else if( rp->last ) {           /* if last is non-null */
            for( rptr = rp->last; ( trptr = rptr->u.link ) != 0; rptr = trptr )
                rptr->u.link = rp->link;
            rptr->u.link = rp->link - 1;
        }
}

/*!
 * @brief Compile a y (transliterate) command.
 *
 * Builds the 128-byte translation table from the two strings
 * separated by @p delim, fills any unmapped entries as identity
 * mappings and returns a pointer past the table in the string pool.
 *
 * @param[in] ep     Where to compile the table to. Not NULL.
 * @param[in] delim  End delimiter to look for.
 *
 * @return Pointer just past the translation table, or BAD on
 *         failure.
 *
 * @retval BAD  Malformed y command, or unequal string lengths.
 */
static char *ycomp(
    register char       *ep,            /* where to compile to */
    char                delim )         /* end delimiter to look for */
{
    register int        c;
    register char       *tp;
    register char const *sp;

    if( delim == 0 || delim == '\\' || delim == '\n' )
        return( BAD );
                                        /* scan 'from' for invalid chars */
    for( sp = tp = cp; *tp != delim; tp++ ) {
        if( *tp == '\\' )
            tp++;
        if( ( *tp == '\n' ) || ( *tp == '\0' ) )
            return( BAD );
    }
    tp++;                               /* tp points at first char of 'to' */

                                        /* now rescan the 'from' section */
    while( ( c = (char)( *sp++ & 0x7F ) ) != delim ) {
        if( c == '\\' )
            switch( c = *sp++ & 0x7F ) {
            case 'n':                   /* '\\''n' -> '\n' */
                c = '\n';
                break;
            case '\\':
                break;
            default:
                if( c != delim )
                    return( BAD );
                c = delim;
                break;
            }
        if( ep[c] != 0 )
            return( BAD );              /* c has already been mapped */
        if( ( ep[c] = *tp++ & 0x7F ) == '\\' ) {
            switch( *tp ) {
            case 'n':                   /* '\\''n' -> '\n' */
                ep[c] = '\n';
                break;
            case '\\':
                break;
            default:
                if( *tp != delim )
                    return( BAD );
                ep[c] = delim;
                break;
            }
            tp++;
        }
        if( ep[c] == '\0' )
            return( BAD );
    }

    if( *tp != delim )                  /* 'to', 'from' lengths unequal */
        return( BAD );

    cp = ++tp;                          /* point compile ptr past translit */

    for( c = 0; c < 128; c++ )          /* fill in self-map entries in table */
        if( ep[c] == 0 )
            ep[c] = c;

    return( ep + 0x80 );                /* first free location past table end */
}

/*!
 * @brief Exit the stream editor on an error condition.
 *
 * Drains any remaining lines from stdin when stdin is not a
 * terminal, to avoid a race condition with callers such as
 * "echo hello | fail", then calls exit().
 *
 * @param[in] status  Non-zero process exit status.
 *
 * @note The status argument must be non-zero; the assert catches
 *       misuse. This function does not return.
 */
static void myexit( int status )
{
    assert( status != 0 );              /* Call only needed for failures */
    if( !isatty( fileno( stdin ) ) )
        while( fgets( linebuf, MAXBUF, stdin ) != NULL ) ;
    exit( status );
}

/*!
 * @brief Print the usage text and exit with status 2.
 *
 * @note This function does not return.
 */
static void usage( void )
{
    const char * const *cpp;

    for( cpp = USAGE; *cpp != NULL; cpp++ )
        fprintf( stderr, "%s\n", *cpp );

    myexit( 2 );
}

/* sedcomp.c ends here */
