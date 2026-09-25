/*!
 *  @file bin2c.c
 *  @brief Binary to C unstructured array converter.
 *
 *  Reads a binary file and emits a C source fragment declaring an
 *  unsigned char array whose bytes are the contents of the input
 *  file. Options control whether non-ASCII bytes are emitted as
 *  hexadecimal escapes.
 *
 *  Target compiler: Micro-C v3.13, Borland C++ v3.1, v4.0.
 *
 *  @copyright Copyright (C) 1995 Steffen Kaiser.
 */

/* $RCSfile: bin2c.c $
   $Locker: ska $       $Name:  $       $State: Exp $

        Binary to C unstructured array converter. 

        Target compiler: Micro-C v3.13, Borland C++ v3.1, v4.0

   $Log: bin2c.c $
   Revision 1.1  1995/08/10 11:38:39  ska
   Initial revision

*/

#include <stdio.h>
#ifdef _MICROC_
#include <file.h>
#else
#include <ctype.h>
#endif
#include <getopt.h>
//#include "yerror.h"

#ifndef lint
/*!
 *  @brief RCS identification string.
 */
static char const rcsid[] = 
        "$Id: bin2c.c 1.1 1995/08/10 11:38:39 ska Exp ska $";
#endif

int ascii = 0;          /*!< Non-zero to force ASCII (escaped) output. */

/*!
 *  @brief Chooses the printf format for one byte.
 *
 *  Quotes and backslashes are emitted as escaped literals. Control
 *  bytes, 0xFF and, when @ref ascii is set, any non-ASCII byte are
 *  emitted as hexadecimal escapes. Everything else is emitted
 *  verbatim.
 *
 *  @param[in] c Byte to classify.
 *
 *  @return printf format string to use for @p c.
 */
char *chooseLine(unsigned char c)
{       return (c == '\'' || c == '\\')? "%s'\\%c'":
                   (iscntrl(c) || c == '\xff'
                        || (!isascii(c) && ascii))? "%s'\\x%x'": "%s'%c'";
}

/*!
 *  @brief Reads a binary file and writes a C array declaration.
 *
 *  Usage: bin2c [-A] name file
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument vector.
 *
 *  @return Exit status.
 *  @retval 0 Successful completion or a usage error was reported.
 */
int main(int argc, char **argv)
{       int c, i;
        FILE *fin;
        char *p;

        while((c = getopt(argc, argv, "AH?", "")) != EOF) 
                switch(c) {
                        case 'A': ascii = 1; break;
                        default: ;//hlpScreen();
                }

        if(!argv[optind++] || !argv[optind])
		{ printf("Missing arguments\n\r"); exit(0);}
        if(argv[optind + 1])
        {        printf("Too many arguments\n\r"); exit(0);}
        if((fin = fopen(argv[optind], "rb")) == NULL)
                printf("Error opening file %s\n\r", argv[optind]);

        printf("unsigned char %s[] = {\n", argv[optind-1]);

        p = "\t", i = 8;
        while((c = fgetc(fin)) != EOF) {
                printf(chooseLine(c), p, c);
                if(--i) p = ", ";
                else p = ",\n\t", i = 8;
        }
        printf("\n};\n");

        fflush(stdout);
        return 0;
}
