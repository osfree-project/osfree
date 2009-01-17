
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <alloca.h>


#include <rexxdefs.h>  // rexxsaa.h include in this header

RexxFunctionHandler RxMessageBox;


unsigned long RxMessageBox(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
    char buffer[256];


    strcpy(buffer, "RxMessageBox\n");
    logmessage(buffer);


    return VALID_ROUTINE;

}
