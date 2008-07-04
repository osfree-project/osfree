/* msgs.h - messages file for catgets()

   Author: Gregory Pietsch

   DESCRIPTION:

   This file contains #defines for all the message strings in edlin.
   For internationalization fun, just translate the messages in this
   file.

   The strings were generated using ESR's cstrings program and moved
   here.

*/

#ifndef MSGS_H
#define MSGS_H

#ifdef USE_KITTEN
#include "kitten.h"
#else
#include "nl_types.h"
#endif

#define YES     catgets(the_cat, 1, 0, "Yy")

#define G00001  catgets(the_cat, 1, 1, " : ")
#define G00002  catgets(the_cat, 1, 2, "O.K.? ")
#define G00003  catgets(the_cat, 1, 3, "Entry error")
#define G00004  catgets(the_cat, 1, 4, "%s: %lu line read\n")
#define G00005  catgets(the_cat, 1, 5, "%s: %lu lines read\n")
#define G00006  catgets(the_cat, 1, 6, "%s: %lu line written\n")
#define G00007  catgets(the_cat, 1, 7, "%s: %lu lines written\n")
#define G00008  catgets(the_cat, 1, 8, "%lu:%c%s\n")
#define G00009  catgets(the_cat, 1, 9, "Press <Enter> to continue")
#define G00010  catgets(the_cat, 1, 10, "%lu: ")
#define G00011  catgets(the_cat, 1, 11, "Not found")
#define G00012  catgets(the_cat, 1, 12, "%lu: %s\n")
#define G00013  catgets(the_cat, 1, 13, "\nedlin has the following subcommands:\n")
#define G00014  catgets(the_cat, 1, 14, "#                 edit a single line    [#],[#],#m        move")
#define G00015  catgets(the_cat, 1, 15, "a                 append                [#][,#]p          page")
#define G00016  catgets(the_cat, 1, 16, "[#],[#],#,[#]c    copy                  q                 quit")
#define G00017  catgets(the_cat, 1, 17, "[#][,#]d          delete                [#][,#][?]r$,$    replace")
#define G00018  catgets(the_cat, 1, 18, "e<>               end (write & quit)    [#][,#][?]s$      search")
#define G00019  catgets(the_cat, 1, 19, "[#]i              insert                [#]t<>            transfer")
#define G00020  catgets(the_cat, 1, 20, "[#][,#]l          list                  [#]w<>            write\n")
#define G00021  catgets(the_cat, 1, 21, "where $ above is a string, <> is a filename,")
#define G00022  catgets(the_cat, 1, 22, "# is a number (which may be .=current line, $=last line,")
#define G00023  catgets(the_cat, 1, 23, "or either number + or - another number).\n")
#define G00024  catgets(the_cat, 1, 24, ", copyright (c) 2003 Gregory Pietsch")
#define G00025  catgets(the_cat, 1, 25, "This program comes with ABSOLUTELY NO WARRANTY.")
#define G00026  catgets(the_cat, 1, 26, "It is free software, and you are welcome to redistribute it")
#define G00027  catgets(the_cat, 1, 27, "under the terms of the GNU General Public License -- either")
#define G00028  catgets(the_cat, 1, 28, "version 2 of the license, or, at your option, any later")
#define G00029  catgets(the_cat, 1, 29, "version.\n")
#define G00030  catgets(the_cat, 1, 30, "Out of memory")
#define G00031  catgets(the_cat, 1, 31, "String length error")
#define G00032  catgets(the_cat, 1, 32, "String position error")
#define G00033  catgets(the_cat, 1, 33, "Invalid user input, use ? for help.")
#define G00034  catgets(the_cat, 1, 34, "No filename")
#define G00035  catgets(the_cat, 1, 35, "Buffer too big")
#define G00036  catgets(the_cat, 1, 36, "Invalid buffer position")
#define G00037  catgets(the_cat, 1, 37, "ERROR: %s\n")

#ifndef EXTERN
#define EXTERN extern
#endif
EXTERN nl_catd the_cat;

#endif

/* END OF FILE */
