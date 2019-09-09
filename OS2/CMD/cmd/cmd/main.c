/*
   @file main.c

   @brief CMD.EXE - Command processor.

   (c) 1993 - 1998  Rex C. Conn  All rights reserved
   (c) osFree Project 2003, <http://www.osFree.org>

   @author J.P. Software (http://jpsoft.com)

*/

#include "product.h"

#include <stdio.h>
#include <io.h>
#include "malloc.h"

#include "4all.h"


int main( int argc, char **argv )
{
        // OS/2 2.0 exception handling has to go on the stack!
        EXCEPTIONREGISTRATIONRECORD ExceptionStruct;

        pExceptionStruct = &ExceptionStruct;

        // initialize the OS-specific stuff
        InitOS( argc, argv );

        while ( gnTransient == 0 ) {

                // reset ^C and "stack full" handling
                cv.exception_flag = 0;

                // reset DO / IFF parsing flags
                cv.f.lFlags = 0;

                // reset batch single-stepping flag
                gpIniptr->SingleStep = 0;

                if ( setjmp( cv.env ) != -1 ) {

                        EnableSignals();

                        show_prompt();
                        EnableSignals();

                        gszCmdline[0] = '\0';

                        // get command line from STDIN
                        if (( argc = getline( STDIN, gszCmdline, MAXLINESIZ-1, EDIT_COMMAND )) > 0 ) {

                                // add line to the history list BEFORE
                                //   doing alias & variable expansion
                                addhist( gszCmdline );

                                // parse and execute command
                                (void)command( gszCmdline, 1 );
                                // release memory back to the OS
                                (void)_heapmin();

                        } else if (( argc == 0 ) && ( _isatty( STDIN ) == 0 )) {
                                // if redirected or piped process, quit when
                                //   no more input
                                break;
                        }
                }
        }

        // reset ^C and "stack full" handling
        HoldSignals();
        cv.exception_flag = 0;

        if ( pfnWTERM != NULL )
                (*pfnWTERM)( ghHAB );

        return ( exit_cmd( 1, NULL ) );
}

