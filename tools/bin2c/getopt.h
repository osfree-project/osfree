/* $Id: GETOPT.H 2.1 1996/11/08 03:18:16 ska Exp $
   $Locker:  $	$Name:  $	$State: Exp $

	Free-Dos library declaration header file.

   $Log: GETOPT.H $
   Revision 2.1  1996/11/08 03:18:16  ska
   chg: Makefile: automatically detect, when switching compiler or mem model etc.
   fix: mcb_walk: evaluating _FLAGS after destroying them by a compare
   add: env_noSpace: check if environment can hold another variable
   fix: env_change: don't delete the variable if environment too small
   fix: env_strput: check env size, ¡f a new string is appended
   chg: environ.h: prototype: putenv(char *var) to conform to DOS quasi-standard

   Revision 2.0  1996/08/01 08:22:35  ska
   Release Version 2

   Target compilers: Micro-C, Borland C v2.0, v3.1, v4.52

   Revision 1.2  1996/07/30 05:15:53  ska
   fix: revised: segm == 0 on all environment functions
   add: the environment string functions
   fix: getoptg(): last option preserves the boolean state flag if MARKEMPTY is set

   Revision 1.1  1996/07/25 08:32:37  ska
   Initial revision

   Revision 1.2  1995/08/30 07:11:14  ska
   chg: adjusted prototypes to ANSI spec.

   Revision 1.1  1995/06/28 15:56:02  ska
   Initial revision

*/

#ifndef __FREEDOS_H
#define __FREEDOS_H

#include <portable.h>

extern int	optind, 	/* index into argv[] array */
			optchar;	/* index into argv[optind][] array */
extern char *optarg;	/* address of argument of an argumented option */
						/* for getopt1(): address of set/unset modifier or
								NULL, if non set */

/* Global control variables for getoptG() */
extern char   *opt1st,		/* non-argumented options */
			  *opt2nd, 		/* argumented options */
			  *opt3rd, 		/* special argumented options */
			  *optSWCHAR, 	/* switch characters, defaults to "/=" */
			  *optARGCHAR, 	/* argument characters, defaults to ":=" */
			  *optBOOL;		/* set/clear characters, defaults to "+-" */
extern unsigned optFlags;	/* Hold the following flags: */
#define GETOPT_NOHELPSCREEN 0x100	/* Don't invoke hlpScreen() for '?' */
#define GETOPT_IGNORE 0x80	/* skip non-option arguments */
#define GETOPT_MARKEMPTY 0x40	/* place '\0' behind switchar to mark the
									argument as used */
#define GETOPT_ADVANCE 0x3f	/* skip (optFlags & GETOPT_ADVANCE) arguments when
								this option is used up */

int getopt(int argc, char **argv, char *opt, char *argopt);
int getopt1(int argc, char **argv, char *opt, char *argopt);
int getopt2(int argc, char **argv, char *opt, char *argopt, char *specopt);
int getoptG(int argc, char **argv);
/* get next option:
	argc: from main()
	argv: from main()
	opt: non-argumented options, upper-cased
	argopt: argumented options, upper-cased
	specopt: special argumented options, upper-cased
*/

#endif
