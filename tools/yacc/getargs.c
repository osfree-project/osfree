/****************************************************************
 * Parse command line arguments for bison,
 * Copyright (C) 1984, 1986, 1989, 1992 Free Software Foundation, Inc.
 *
 * This file is part of Bison, the GNU Compiler Compiler.
 *
 * Bison is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Bison is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bison; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ****************************************************************/

/*!
 *  @file getargs.c
 *  @brief Parses command line arguments for Bison.
 *
 *  Recognizes the short and long options described by the POSIX yacc
 *  specification plus Bison's own extensions, sets the corresponding
 *  global flags, records the input grammar file name in @ref infile,
 *  and prints the usage message on error or on --help.
 *
 *  @copyright Copyright (C) 1984, 1986, 1989, 1992 Free Software Foundation, Inc.
 *             Licensed under the GNU General Public License v2 or later.
 */


#include <stdio.h>
#include "getopt.h"
#include "system.h"
#include "files.h"

int verboseflag;        /*!< Non-zero when -v or --verbose was given. */
int definesflag;        /*!< Non-zero when -d or --defines was given. */
int debugflag;          /*!< Non-zero when -t or --debug was given. */
int nolinesflag;        /*!< Non-zero when -l or --no-lines was given. */
int noparserflag = 0;   /*!< Non-zero when -n or --no-parser was given. */
int toknumflag = 0;     /*!< Non-zero when -k or --token-table was given. */
int rawtoknumflag = 0;  /*!< Non-zero when -r or --raw was given. */
char *spec_name_prefix; /*!< Name prefix set by -p or --name-prefix. */
char *spec_file_prefix; /*!< File prefix set by -b or --file-prefix. */
extern int fixed_outfiles;/*!< Non-zero when -y or --yacc was given. */
  
/*!
 *  @brief Prints the usage message to the given stream.
 *
 *  @param[in] stream Output stream.
 */
void usage PARAMS((FILE *));

/*!
 *  @brief Parses the command line arguments.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument vector.
 */
void getargs PARAMS((int, char *[]));

/*!< Name of the running program, used in diagnostics. */
extern char *program_name;

/*!
 *  @brief Reports a warning with one string argument.
 *
 *  @param[in] fmt Format string.
 *  @param[in] arg Argument for the format.
 */
extern void warns PARAMS((char *, char *));	/* main.c */

/*!
 *  @brief Long-option table accepted by getopt_long().
 */
struct option longopts[] =
{
  {"debug", 0, &debugflag, 1},
  {"defines", 0, &definesflag, 1},
  {"file-prefix", 1, 0, 'b'},
  {"fixed-output-files", 0, &fixed_outfiles, 1},
  {"help", 0, 0, 'h'},
  {"name-prefix", 1, 0, 'p'}, /* was 'a';  apparently unused -wjh */
  {"no-lines", 0, &nolinesflag, 1},
  {"no-parser", 0, &noparserflag, 1},
  {"output", 1, 0, 'o'},
  {"output-file", 1, 0, 'o'},
  {"raw", 0, &rawtoknumflag, 1},
  {"token-table", 0, &toknumflag, 1},
  {"verbose", 0, &verboseflag, 1},
  {"version", 0, 0, 'V'},
  {"yacc", 0, &fixed_outfiles, 1},
  {0, 0, 0, 0}
};

/*!
 *  @brief Prints the usage message to the given stream.
 *
 *  @param[in] stream Output stream.
 */
void
usage (FILE *stream)
{
  fprintf (stream, _("\
Usage: %s [-dhklntvyV] [-b file-prefix] [-o outfile] [-p name-prefix]\n\
       [--debug] [--defines] [--fixed-output-files] [--no-lines]\n\
       [--verbose] [--version] [--help] [--yacc]\n\
       [--no-parser] [--token-table]\n\
       [--file-prefix=prefix] [--name-prefix=prefix]\n\
       [--output=outfile] grammar-file\n\n\
Report bugs to bug-bison@gnu.org\n"),
	   program_name);
}

/*!
 *  @brief Parses the command line arguments.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument vector.
 */
void
getargs (int argc, char *argv[])
{
  register int c;

  verboseflag = 0;
  definesflag = 0;
  debugflag = 0;
  noparserflag = 0;
  rawtoknumflag = 0;
  toknumflag = 0;
  fixed_outfiles = 0;

  while ((c = getopt_long (argc, argv, "yvdhrltknVo:b:p:", longopts, (int *)0))
	 != EOF)
    {
      switch (c)
	{
	case 0:
	  /* Certain long options cause getopt_long to return 0.  */
	  break;

	case 'y':
	  fixed_outfiles = 1;
	  break;
	  
	case 'h':
	  usage (stdout);
	  exit (0);

	case 'V':
	  printf ("%s\n", VERSION_STRING);
	  exit (0);
	  
	case 'v':
	  verboseflag = 1;
	  break;
	  
	case 'd':
	  definesflag = 1;
	  break;
	  
	case 'l':
	  nolinesflag = 1;
	  break;
	  
	case 'k':
	  toknumflag = 1;
	  break;

	case 'r':
	  rawtoknumflag = 1;
	  break;
	  
	case 'n':
	  noparserflag = 1;
	  break;
	  
	case 't':
	  debugflag = 1;
	  break;
	  
	case 'o':
	  spec_outfile = optarg;
	  break;
	  
	case 'b':
	  spec_file_prefix = optarg;
	  break;
	  
	case 'p':
	  spec_name_prefix = optarg;
	  break;
	  
	default:
	  usage (stderr);
	  exit (1);
	}
    }

  if (optind == argc)
    {
      fprintf(stderr, _("%s: no grammar file given\n"), program_name);
      exit(1);
    }
  if (optind < argc - 1)
    fprintf(stderr, _("%s: extra arguments ignored after '%s'\n"),
		program_name, argv[optind]);

  infile = argv[optind];
}
