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
static char const rcsid[] = 
        "$Id: bin2c.c 1.1 1995/08/10 11:38:39 ska Exp ska $";
#endif

int ascii = 0;          /* force ASCII output */

char *chooseLine(unsigned char c)
{       return (c == '\'' || c == '\\')? "%s'\\%c'":
                   (iscntrl(c) || c == '\xff'
                        || (!isascii(c) && ascii))? "%s'\\x%x'": "%s'%c'";
}

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
