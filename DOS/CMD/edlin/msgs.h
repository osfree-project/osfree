/* msgs.h - messages file

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

#define YES             "Yy"

#define G00001  " : "
#define G00002  "O.K.? "
#define G00003  "Entry error"
#define G00004  "%s: %lu line read\n"
#define G00005  "%s: %lu lines read\n"
#define G00006  "%s: %lu line written\n"
#define G00007  "%s: %lu lines written\n"
#define G00008  "%lu:%c%s\n"
#define G00009  "Press <Enter> to continue"
#define G00010  "%lu: "
#define G00011  "Not found"
#define G00012  "%lu: %s\n"
#define G00013  "\nedlin has the following subcommands:\n"
#define G00014  "#                 edit a single line    [#],[#],#m        move"
#define G00015  "a                 append                [#][,#]p          page"
#define G00016  "[#],[#],#,[#]c    copy                  q                 quit"
#define G00017  "[#][,#]d          delete                [#][,#][?]r$,$    replace"
#define G00018  "e<>               end (write & quit)    [#][,#][?]s$      search"
#define G00019  "[#]i              insert                [#]t<>            transfer"
#define G00020  "[#][,#]l          list                  [#]w<>            write\n"
#define G00021  "where $ above is a string, <> is a filename,"
#define G00022  "# is a number (which may be .=current line, $=last line,"
#define G00023  "or either number + or - another number).\n"
#define G00024  ", copyright (c) 2003 Gregory Pietsch"
#define G00025  "This program comes with ABSOLUTELY NO WARRANTY."
#define G00026  "It is free software, and you are welcome to redistribute it"
#define G00027  "under the terms of the GNU General Public License -- either"
#define G00028  "version 2 of the license, or, at your option, any later"
#define G00029  "version.\n"
#define G00030  "Out of memory"
#define G00031  "String length error"
#define G00032  "String position error"
#define G00033  "Invalid user input, use ? for help."
#define G00034  "No filename"
#define G00035  "Buffer too big"
#define G00036  "Invalid buffer position"
#define G00037  "ERROR: %s\n"

#endif

/* END OF FILE */
