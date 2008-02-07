/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains helper functions functions:
*       string2long( )
*       string2ulong( )
*
*  Michael Greene, December 2007
*
------------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/******************************************************************************/

#define INCL_DOSMISC

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include <sys\stat.h>
#include <sys\types.h>
#include <fcntl.h>
#include <io.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header


/********************************************************************
* Function:  string2long(string, number)                            *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns FALSE if the number *
*            is not valid, TRUE if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/

bool string2long(char *string, long *number)
{
    unsigned long accumulator;           /* converted number           */
    int      length;                     /* length of number           */
    int      sign;                       /* sign of number             */

    sign = 1;                            /* set default sign           */

    if (*string == '-') {                /* negative?                  */
        sign = -1;                       /* change sign                */
        string++;                        /* step past sign             */
    }

    length = strlen(string);             /* get length of string       */
    if (length == 0 ||                   /* if null string             */
        length > MAX_DIGITS)             /* or too long                */
      return FALSE;                      /* not valid                  */

    accumulator = 0;                     /* start with zero            */

    while (length) {                     /* while more digits          */
        if (!isdigit(*string)) {          /* not a digit?               */
              *number = 255;
              return FALSE;              /* tell caller                */
        }                                 /* add to accumulator         */
        accumulator = accumulator * 10 + (*string - '0');
        length--;                        /* reduce length              */
        string++;                        /* step pointer               */
    }

    *number = accumulator * sign;        /* return the value           */

    return TRUE;                         /* good number                */
}


/********************************************************************
* Function:  string2ulong(string, number)                           *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns FALSE if the number *
*            is not valid, TRUE if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/

bool string2ulong(char *string, unsigned long *number)
{
    unsigned long accumulator;           /* converted number           */
    int      length;                     /* length of number           */

    length = strlen(string);             /* get length of string       */
    if (length == 0 ||                   /* if null string             */
        length > MAX_DIGITS + 1)         /* or too long                */
      return FALSE;                      /* not valid                  */

    accumulator = 0;                     /* start with zero            */

    while (length) {                     /* while more digits          */
      if (!isdigit(*string))             /* not a digit?               */
        return FALSE;                    /* tell caller                */
                                         /* add to accumulator         */
      accumulator = accumulator * 10 + (*string - '0');
      length--;                          /* reduce length              */
      string++;                          /* step pointer               */
    }

    *number = accumulator;               /* return the value           */

    return TRUE;                         /* good number                */
}


/* SetRexxVariable - Set the value of a REXX variable */

unsigned long SetRexxVariable(char *name, char *value)
{
    SHVBLOCK   block;                /* variable pool control block*/

    block.shvcode = RXSHV_SET; // check this RXSHV_SYSET;
    block.shvret=(UCHAR)0;           /* clear return code field    */
    block.shvnext=NULL;              /* no next block              */
                                     /* set variable name string   */
    MAKERXSTRING(block.shvname, name, strlen(name));
                                     /* set value string           */
    MAKERXSTRING(block.shvvalue, value, strlen(value));
    block.shvvaluelen=strlen(value); /* set value length           */

    return RexxVariablePool(&block); /* set the variable           */
}


/*
void logmessage( char *entry)
{
    int outFile = 0;

    outFile = open("test.log", O_WRONLY | O_APPEND | O_CREAT,
         S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);

    write(outFile, entry, strlen(entry));

    close(outFile);
}
*/

