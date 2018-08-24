/***************************************************************
  
        bwbasic.h       Header File
                        for Bywater BASIC Interpreter
  
                        Copyright (c) 1993, Ted A. Campbell
                        Bywater Software
  
                        email: tcamp@delphi.com
  
        Copyright and Permissions Information:
  
        All U.S. and international rights are claimed by the author,
        Ted A. Campbell.
  
   This software is released under the terms of the GNU General
   Public License (GPL), which is distributed with this software
   in the file "COPYING".  The GPL specifies the terms under
   which users may copy and use the software in this distribution.
  
   A separate license is available for commercial distribution,
   for information on which you should contact the author.
  
***************************************************************/

/*---------------------------------------------------------------*/
/* NOTE: Modifications marked "JBV" were made by Jon B. Volkoff, */
/* 11/1995 (eidetics@cerf.net).                                  */
/*                                                               */
/* Those additionally marked with "DD" were at the suggestion of */
/* Dale DePriest (daled@cadence.com).                            */
/*                                                               */
/* Version 3.00 by Howard Wulf, AF5NE                            */
/*                                                               */
/* Version 3.10 by Howard Wulf, AF5NE                            */
/*                                                               */
/* Version 3.20 by Howard Wulf, AF5NE                            */
/*                                                               */
/*---------------------------------------------------------------*/


#define   TRUE     -1
#define   FALSE     0
#define   NDEBUG    1

#include         <assert.h>
#include         <ctype.h>
#include         <errno.h>
#include         <float.h>
#include         <limits.h>
#include         <locale.h>
#include         <math.h>
#include         <setjmp.h>
#include         <signal.h>
#include         <stdarg.h>
#include         <stddef.h>
#include         <stdio.h>
#include         <stdlib.h>
#include         <string.h>
#include         <time.h>


/***************************************************************
  
                          Definitions
  
***************************************************************/

/***************************************************************
  
            Define Major Hardware Implementation
           
            TTY is the default implementation.
            It is the most minimal, but the most
            universal hardware implementation.
           
            If you use TTY then check the settings
            in bwx_tty.c for your system.
         
  
***************************************************************/



/*
**
**  Here is where you should #include any implementation 
**  specific files to #define your preferences
** 
**
#include "bwb_user.h"
**
**
**
**
*/

/*
**
** Attempt to detect the Operating System
**
*/
#if defined(HAVE_UNIX)
/*
**
** UNIX-style Operating Systems
**
*/
#elif defined(HAVE_MSDOS)
/*
**
** MSDOS-style Operating Systems
**
*/
#elif defined(HAVE_CMS)
/*
**
** GCC 3 for CMS on Hercules
**
*/
#elif defined(HAVE_MVS)
/*
**
** GCC 3 for MVS on Hercules
**
*/
#elif defined(__CMS__)

#define HAVE_CMS TRUE

#elif defined(__MVS__)

#define HAVE_MVS TRUE

#elif defined(_DOS)

#define HAVE_MSDOS  TRUE

#elif defined(_WIN16)

#define HAVE_MSDOS  TRUE

#elif defined(_WIN32)

#define HAVE_MSDOS  TRUE

#elif defined(_WIN64)

#define HAVE_MSDOS  TRUE

#elif defined(__unix__)

#define HAVE_UNIX TRUE

#elif defined(__unix)

#define HAVE_UNIX TRUE

#elif defined(__APPLE__)

#define HAVE_UNIX TRUE

#elif defined(__MACH__)

#define HAVE_UNIX TRUE

#elif defined(__BORLANDC__)

#define HAVE_MSDOS  TRUE

#elif defined(__DMC__)

#define HAVE_MSDOS  TRUE

#elif defined(__WATCOM__)

#define HAVE_MSDOS  TRUE

#else
/*
**
** Unable to detect the operaating syystem
**
*/
#endif

/*-------------------------------------------------------------
                ENVIRONMENT specific configuration                
-------------------------------------------------------------*/

#if HAVE_UNIX
/*
**
** UNIX-style Operating Systems
**
*/
/* 
**
** If I understand correctly, my development environment is
** GCC 4.9 for Linux via CCTools for Android in a BusyBox 
** environment under a Java simulation of an ARM processor 
** on a Barnes & Noble $69 Nook Tablet.  What could possibly 
** go wrong?   When you finish laughing, it actually works.
** 
** I compile with: gcc -ansi -o bwbasic bw*.c
**
*/
#include <unistd.h>
#include <sys/stat.h>

#ifndef   DIRECTORY_CMDS
#define   DIRECTORY_CMDS     TRUE
#endif /* DIRECTORY_CMDS */

#ifndef   MKDIR_ONE_ARG
#define   MKDIR_ONE_ARG      FALSE
#endif /* MKDIR_ONE_ARG */

#ifndef   PERMISSIONS
#define   PERMISSIONS        0x0644
#endif /* PERMISSIONS */

#ifndef   DEF_EDITOR
#define   DEF_EDITOR         "nano"
#endif /* DEF_EDITOR */

#ifndef   HAVE_UNIX_GCC
#if       __GNUC__
#define   HAVE_UNIX_GCC      TRUE
#else
#define   HAVE_UNIX_GCC      FALSE
#endif /* __GNUC__ */
#endif /* HAVE_UNIX_GCC */

#endif /* HAVE_UNIX */

/*-----------------------------------------------------------*/

#if HAVE_MSDOS
/*
**
** MSDOS-style Operating Systems
**
*/
#if       __WATCOM__
#include  <direct.h>             /* OpenWatcom 1.9 for DOS */
#else
#include  <dir.h>                /* all others */
#endif /* __WATCOM__ */

#ifndef   DIRECTORY_CMDS
#define   DIRECTORY_CMDS     TRUE
#endif /* DIRECTORY_CMDS */

#ifndef   MKDIR_ONE_ARG
#define   MKDIR_ONE_ARG      TRUE
#endif /* MKDIR_ONE_ARG */

#ifndef   PERMISSIONS
#define   PERMISSIONS        0
#endif /* PERMISSIONS */

#ifndef   DEF_EDITOR
#define   DEF_EDITOR         "edit"
#endif /* DEF_EDITOR */

#ifndef   DEF_FILES
#define   DEF_FILES          "dir /w"
#endif /* DEF_FILES */

#ifndef   HAVE_UNIX_GCC
#define   HAVE_UNIX_GCC      FALSE
#endif /* HAVE_UNIX_GCC */

#endif /* HAVE_MSDOS */

/*-----------------------------------------------------------*/

#if       HAVE_CMS
/*
**
** GCC 3 for CMS on Hercules
**
*/
#include "bwd_six.h"

#ifndef   PROFILENAME
#define   PROFILENAME        "PROFILE BAS"
#endif /* PROFILENAME */

#if       FALSE
/*
**
** bwBASIC 3.10:  OPTION STDERR "LPRINT OUT"
**
*/
#ifndef   LPRINTFILENAME
#define   LPRINTFILENAME     "LPRINT OUT"
#endif /* LPRINTFILENAME */
#endif /* FALSE */

#if       FALSE
/*
**
** bwBASIC 3.10:  OPTION STDERR "ERROR OUT"
**
*/
#ifndef   ERRFILE
#define   ERRFILE            "ERROR OUT"
#endif /* ERRFILE */
#endif /* FALSE */

#if       FALSE
/*
**
** bwBASIC 3.20:  OPTION PROMPT "bwBASIC:" + CHR$(10)
**
*/
#ifndef   DEF_PROMPT
#define   DEF_PROMPT         "bwBASIC:\n"
#endif /* DEF_PROMPT */
#endif /* FALSE */

#ifndef   DEF_EXTENSION
#define   DEF_EXTENSION      " BAS"
#endif /* DEF_EXTENSION */

#ifndef   DIRECTORY_CMDS
#define   DIRECTORY_CMDS     FALSE
#endif /* DIRECTORY_CMDS */

#ifndef   HAVE_UNIX_GCC
#define   HAVE_UNIX_GCC      FALSE
#endif /* HAVE_UNIX_GCC */

#endif /* HAVE_CMS */

/*---------------------------------------------------------------*/

#ifdef    HAVE_MVS
/*
**
** GCC 3 for MVS on Hercules
**
*/
#include "bwd_six.h"

#ifndef   PROFILENAME
#define   PROFILENAME        "dd:profile"
#endif /* PROFILENAME */

#if       FALSE
/*
**
** bwBASIC 3.10:  OPTION STDERR "dd:lprint"
**
*/
#ifndef   LPRINTFILENAME
#define   LPRINTFILENAME     "dd:lprint"
#endif /* LPRINTFILENAME */
#endif /* FALSE */

#if       FALSE
/*
**
** bwBASIC 3.10:  OPTION STDERR "dd:errout"
**
*/
#ifndef   ERRFILE
#define   ERRFILE            "dd:errout"
#endif /* ERRFILE */
#endif /* FALSE */

#if       FALSE
/*
**
** bwBASIC 3.20:  OPTION PROMPT "bwBASIC:" + CHR$(10)
**
*/
#ifndef   DEF_PROMPT
#define   DEF_PROMPT         "bwBASIC:\n"
#endif /* DEF_PROMPT */
#endif /* FALSE */

#ifndef   DEF_EXTENSION
#define   DEF_EXTENSION      ""
#endif /* DEF_EXTENSION */

#ifndef   DIRECTORY_CMDS
#define   DIRECTORY_CMDS     FALSE
#endif /* DIRECTORY_CMDS */

#ifndef   HAVE_UNIX_GCC
#define   HAVE_UNIX_GCC      FALSE
#endif /* HAVE_UNIX_GCC */

#endif /* HAVE_MVS */


/***************************************************************
  
         This ends the section of definitions that
         users of bwBASIC might want to specify. 
         The following are internally defined.
        
         Note that you might want to #define the default
         FILES command and the default EDITOR above.
  
***************************************************************/



/*
**
**
**           If you want to change any of these values, 
**           then you should #define them above.
**
**
*/

#ifndef   HAVE_UNIX_GCC
/*
** TRUE:
** sleep() is an intrinsic C functions in GCC using -ansi 
** FALSE:
** sleep() is defined in bwb_int.c to do nothing 
**
*/
#define   HAVE_UNIX_GCC      FALSE
#endif /* HAVE_UNIX_GCC */

#ifndef   PROFILE
/*
**
** TRUE:
** automatically execute PROFILENAME line-by-line before parsing command line parameters
** FALSE:
** do not automatically execute PROFILENAME
**
*/
#define   PROFILE            TRUE
#endif /* PROFILE */

#ifndef   PROFILENAME
/* 
**
** Filename for PROFILE, only used when PROFILE == TRUE 
** This file contains the various OPTION commands
**
*/
#define   PROFILENAME        "profile.bas"
#endif /* PROFILENAME */

#if       FALSE
/*
**
** bwBASIC 3.10: OPTION STDERR "err.out" 
** bwBASIC 3.10: OPTION STDERR "LPRINT.OUT" 
**
*/
#ifndef   REDIRECT_STDERR
/*
**
** redirect STDERR
**
*/
#define   REDIRECT_STDERR    FALSE
#endif /* REDIRECT_STDERR */
#ifndef   ERRFILE
/*
**
** Filename for redirected stderr
**
*/
#define   ERRFILE            "err.out"
#endif /* ERRFILE */
#ifndef   LPRINTFILENAME
/*
**
** Filename for LPRINT output
**
*/
#define   LPRINTFILENAME     "LPRINT.OUT"
#endif /* LPRINTFILENAME */
#endif /* FALSE */

#ifndef   DIRECTORY_CMDS
/*
**
** enable MKDIR, CHDIR, RMDIR 
**
*/
#define   DIRECTORY_CMDS      TRUE
#endif /* DIRECTORY_CMDS */

#ifndef   MKDIR_ONE_ARG
/*
**
** TRUE if your mkdir has one argument
** only used when DIRECTORY_CMDS == TRUE 
**
*/
#define   MKDIR_ONE_ARG       FALSE
#endif /* MKDIR_ONE_ARG */

#ifndef   PERMISSIONS
/* 
** 
** permissions to set in a Unix-type system
** only used when MKDIR_ONE_ARG == FALSE 
**
*/
#define   PERMISSIONS        0x0644
#endif /* PERMISSIONS */

#if       FALSE
/*
**
** bwBASIC 3.20: OPTION PROMPT "bwBASIC: "
**
*/
#ifndef   DEFVNAME_PROMPT
/*
**
** variable name for PROMPT
**
*/
#define   DEFVNAME_PROMPT     "BWB.PROMPT$"
#endif /* DEFVNAME_PROMPT */
#endif /* FALSE */

#ifndef   DEF_PROMPT
/*
**
** default value for OPTION PROMPT
**
*/
#define   DEF_PROMPT          "bwBASIC: "
#endif /* DEF_PROMPT */

#ifndef   DEF_EXTENSION
/*
**
** default value for OPTION EXTENSION 
**
*/
#define   DEF_EXTENSION          ".bas"
#endif /* DEF_EXTENSION */

#if       FALSE
/* 
**
** bwBASIC 3.20: OPTION EDIT "vi" 
**
*/
#ifndef   DEFVNAME_EDITOR
/*
**
** variable name for EDIT command
**
*/
#define   DEFVNAME_EDITOR    "BWB.EDITOR$"
#endif /* DEFVNAME_EDITOR */
#endif /* FALSE */

#ifndef   DEF_EDITOR
/*
**
** default value for OPTION EDIT
**
**/
#define   DEF_EDITOR          "vi"
#endif /* DEF_EDITOR */

#if FALSE
/*
**
** bwBASIC 3.20: OPTION FILES "ls -Fx"
**
*/
#ifndef   DEFVNAME_FILES
/*
**
** variable name for FILES command
**
*/
#define   DEFVNAME_FILES      "BWB.FILES$"
#endif /* DEFVNAME_FILES */
#endif /* FALSE */

#ifndef   DEF_FILES
/*
**
** default value for OPTION FILES 
**
*/
#define   DEF_FILES           "ls -Fx"
#endif /* DEF_FILES */

#if       FALSE
/*
**
** bwBASIC 3.20: not used 
**
*/
#ifndef   DEFVNAME_COLORS
/* 
**
** variable name for COLORS 
**
*/
#define   DEFVNAME_COLORS     "BWB.COLORS"
#endif /* DEFVNAME_COLORS */
#ifndef   DEF_COLORS
/*
**
** default value for COLORS 
**
*/
#define   DEF_COLORS          256
#endif /* DEF_COLORS */
#endif /* FALSE */

#if       FALSE
/*
**
** bwBASIC 3.20: not used 
**
*/
#ifndef   DEFVNAME_IMPL
/*
**
** variable name for IMPLEMENTATION 
**
*/
#define   DEFVNAME_IMPL      "BWB.IMPLEMENTATION$"
#endif /* DEFVNAME_IMPL */
#ifndef   IMP_IDSTRING
/*
**
** default value for IMPLEMENTATION 
**
*/
#define   IMP_IDSTRING       "TTY"
#endif /* IMP_IDSTRING */
#endif /* FALSE */

#if       FALSE
/*
**
** bwBASIC 3.20: OPTION RENUM "renum" 
**
*/
#ifndef   DEFVNAME_RENUM
/*
**
** variable name for RENUM command 
**
*/
#define   DEFVNAME_RENUM     "BWB.RENUM$"
#endif /* DEFVNAME_RENUM */
#endif /* FALSE */

#ifndef   DEF_RENUM
/*
**
** default value for OPTION RENUM 
**
*/
#define   DEF_RENUM          "renum"
#endif /* DEF_RENUM */

#ifndef   EXECLEVELS
/*
**
** maximum EXEC stack levels
** only used to prevent run away recursion, such as:
** 100 GOSUB 100 
**
*/
#define   EXECLEVELS         255
#endif /* EXECLEVELS */

#ifndef   MAX_DIMS
/*
**
** maximum number of array dimensions 
**
*/
#define   MAX_DIMS           3
#endif /* MAX_DIMS */

#if       FALSE
/*
**
** bwBASIC 3.10: not used 
**
*/
#ifndef   ESTACKSIZE
/*
**
** maximum number of elements in expression stack 
**
*/
#define   ESTACKSIZE         64
#endif /* ESTACKSIZE */
#endif /* FALSE */

/*
** ============================================================
**
**
**           UNLESS YOU ARE WORKING ON THE INTERNALS, 
**                 YOU SHOULD NOT GO BEYOND HERE.
**         CHANGING ANY OF THESE VALUES IS NOT SUPPORTED.
**
** ============================================================
*/



/*
The relationship of numeric values is REQUIRED to be:
  
  MAXDBL >= MAXSNG >= MAXCUR >= MAXLNG >= MAXINT > MAXLEN >= MAXBYT > 0
  MINDBL <= MINSNG <= MINCUR <= MINLNG <= MININT < 0
  MINLEN == MINBYT == MINDEV == 0
  
  MAXDEV >  0
  MAXBYT == 255 
  MAXINT >= 32000
  MININT <= -32000
  MAXSNG >= 1E37
  MINSNG <= -1E37
*/

/*

   switch( TypeCode )
   {
   case ByteTypeCode:
      break;
   case IntegerTypeCode:
      break;
   case LongTypeCode:
      break;
   case CurrencyTypeCode:
      break;
   case SingleTypeCode:
      break;
   case DoubleTypeCode:
      break;
   case StringTypeCode:
      break;
   default:
      { WARN_INTERNAL_ERROR; return ...; }
   }

*/


/*-------------------------------------------------------------
                        BASIC BYTE
                        OPTION PUNCT BYTE "~"
-------------------------------------------------------------*/
#ifndef   ByteTypeCode
typedef unsigned char ByteType;
#define   MINBYT                     0
#define   MAXBYT                     ( UCHAR_MAX )
#define   ByteTypeCode               '1'
#endif /* ByteTypeCode */

/*-------------------------------------------------------------
                        BASIC INTEGER
                        OPTION PUNCT INTEGER "%"
-------------------------------------------------------------*/
#ifndef   IntegerTypeCode
typedef int IntegerType;
#define   MININT                     ( INT_MIN    )
#define   MAXINT                     ( INT_MAX    )
/*
**
** minimum USER line number
** must be > 0
**
*/
#define   MINLIN                     1
/*
**
** maximum USER line number
** must be < MAXINT 
**
*/
#define   MAXLIN                     ( MAXINT - 1 )
/*
**
** number of digits for line numbers
** line numbers 1 to 99999 use 5 digits
**
*/
#define   LineNumberDigits           5
/*
**
** default maximum line length, must be < MAXINT 
** OPTION LINE 255
**
*/
#define   MAX_LINE_LENGTH           255
#define   IntegerTypeCode            '2'
#endif /* IntegerTypeCode */

/*-------------------------------------------------------------
                        BASIC LONG
                        OPTION PUNCT LONG "&"
-------------------------------------------------------------*/
#ifndef   LongTypeCode
typedef long LongType;
#define   MINLNG                     ( LONG_MIN )
#define   MAXLNG                     ( LONG_MAX )

#define   HexScanFormat          "%lx%n"
#define   OctScanFormat          "%lo%n"
#define   LongTypeCode               '3'
#endif /* LongTypeCode */

/*-------------------------------------------------------------
                        BASIC CURRENCY
                        OPTION PUNCT CURRENCY "@"
-------------------------------------------------------------*/
#ifndef   CurrencyTypeCode
/*
**
** Deprecated, to be removed in bwBASIC 3.30
**
*/
typedef long CurrencyType;
#define   MINCUR                     ( LONG_MIN )
#define   MAXCUR                     ( LONG_MAX )
#define   CurrencyTypeCode           '4'
#endif /* CurrencyTypeCode */

/*-------------------------------------------------------------
                        BASIC SINGLE
                        OPTION PUNCT SINGLE "!"
-------------------------------------------------------------*/
#ifndef   SingleTypeCode
typedef float SingleType;
#define   MINSNG                     ( -FLT_MAX )
#define   MAXSNG                     (  FLT_MAX )
#define   SingleTypeCode             '5'
#endif /* SingleTypeCode */

/*-------------------------------------------------------------
                        BASIC DOUBLE
                        OPTION PUNCT DOUBLE "#"
-------------------------------------------------------------*/
#ifndef   DoubleTypeCode
typedef double DoubleType;
#define   MINDBL                     ( -DBL_MAX )
#define   MAXDBL                     (  DBL_MAX )
#define   DecScanFormat           "%lg%n"
#define   DoubleTypeCode              '6'
#endif /* DoubleTypeCode */

/*-------------------------------------------------------------
                        BASIC STRING
                        OPTION PUNCT STRING "$"
-------------------------------------------------------------*/
#ifndef   StringTypeCode
/*
**
** minimum length of a BASIC string
** must be zero
**
*/
#define   MINLEN                    0
/*
**
** maximum length of a BASIC string
** must be < MAXINT
*/
#define   MAXLEN                    255
#define   StringTypeCode            '7'
#endif /* StringTypeCode */


/*-------------------------------------------------------------
                        BASIC DEVICE
-------------------------------------------------------------*/
#ifndef   MINDEV
/*
** 
** minimum numbwe of OPEN files
** must be zero
**
*/
#define   MINDEV                     0
#endif /* MINDEV */

#ifndef   MAXDEV
/*
** 
** maximum numbwe of OPEN files
** not counting stdin, stdout, stderr
**
*/
#define   MAXDEV                     ( FOPEN_MAX - 3 )
#endif /* MAXDEV */

#if       FALSE
/* 
**
** bwBASIC 3.20: OPTION PUNCT FILENUM "#" 
**
*/
#ifndef   FileNumberPrefix
#define   FileNumberPrefix           '#'
#endif /* FileNumberPrefix */
#endif /* FALSE */


/*-------------------------------------------------------------
                        BASIC MISCELLANEOUS
-------------------------------------------------------------*/


#ifndef   NameLengthMax
/*
**
** maximum BASIC keyword length
** Applies to ALL keywords in BASIC, incuding:
** variables, functions, subroutines, commands and labels.
**
*/
#define   NameLengthMax           40
#endif /* NameLengthMax */

#ifndef   SIGNIFICANT_DIGITS
/* 
** 
** default setting for OPTION DIGITS
** minimum is 6 for BASIC 
** OPTION DIGITS 6
**
*/
#define   SIGNIFICANT_DIGITS     6
#endif /* SIGNIFICANT_DIGITS */

#ifndef   EXPONENT_DIGITS
/*
**
** number of exrad digits
** minimum value is 2 for BASIC 
**
*/
#if       (DBL_MAX_10_EXP) < 100
/*
**
** Exponenet range is within 1E-99 to 1E+99
** ANSI C89 requires at least 1E-37 to 1E+37 
**
*/
#define   EXPONENT_DIGITS        2
#elif     (DBL_MAX_10_EXP) < 1000
/*
**
** Exponenet range is within 1E-999 to 1E+999
** includes ANSI/IEEE Std 754-1985 
**
*/
#define   EXPONENT_DIGITS        3
#elif     (DBL_MAX_10_EXP) < 10000
/*
**
** Exponenet range is within 1E-9999 to 1E+9999
**
*/
#define   EXPONENT_DIGITS        4
#elif     (DBL_MAX_10_EXP) < 100000
/*
**
** Exponenet range is within 1E-99999 to 1E+99999
**
*/
#define   EXPONENT_DIGITS        5
#else
/*
**
** Exponenet range is within 1E-999999 to 1E+999999
**
*/
#define   EXPONENT_DIGITS        6
#endif
#endif /* EXPONENT_DIGITS */

#ifndef   ZONE_WIDTH
/*
**
** default setting for OPTION ZONE
** ECMA-55, Section 14.4 
**
*/
#define   ZONE_WIDTH ( SIGNIFICANT_DIGITS + EXPONENT_DIGITS + 6 )
#endif /* ZONE_WIDTH */

#ifndef   MINIMUM_DIGITS
/*
**
** minimum setting for OPTION DIGITS 
**
*/
#define   MINIMUM_DIGITS         1
#endif /* MINIMUM_DIGITS */

#ifndef   MAXIMUM_DIGITS
/*
**
** maximum setting for OPTION DIGITS 
**
*/
#define   MAXIMUM_DIGITS         ( DBL_DIG )
#endif /* MAXIMUM_DIGITS */

#ifndef   MINIMUM_SCALE
/*
**
** minimum setting for OPTION SCALE
** zero disables scaling 
**
*/
#define   MINIMUM_SCALE          0
#endif /* MINIMUM_SCALE */

#ifndef   MAXIMUM_SCALE
/*
** 
** maximum setting for OPTION SCALE 
**
*/
#define   MAXIMUM_SCALE          ( DBL_DIG )
#endif /* MAXIMUM_SCALE */

#ifndef   MINIMUM_ZONE
/* 
**
** minimum setting for OPTION ZONE 
**
*/
#define   MINIMUM_ZONE           1
#endif /* MINIMUM_ZONE */

#ifndef   MAXIMUM_ZONE
/* 
**
** maximum setting for OPTION ZONE 
**
*/
#define   MAXIMUM_ZONE           79
#endif /* MAXIMUM_ZONE */

#ifndef   NUMLEN
/* 
**
** small buffer for converting numbers to strings
** FormatBasicNumber, sprintf("%f"), sprintf("%d") and so on 
**
*/
#define   NUMLEN                 64
#endif /* NUMLEN */

#ifndef   MAX_ERR_LENGTH
/*
**
** maximum length of ERROR$ 
**
*/
#define   MAX_ERR_LENGTH          63
#endif /* MAX_ERR_LENGTH */

#ifndef   MIN
/* 
**
** minimum of two numbers 
**
*/
#define   MIN( X, Y )            ( (X) < (Y) ? (X) : (Y) )
#endif /* MIN */

#ifndef   MAX
/*
**
** maximum of two numbers 
**
*/
#define   MAX( X, Y )            ( (X) > (Y) ? (X) : (Y) )
#endif /* MAX */

#if       FALSE
/*
**
** bwBASIC 3.20: not used 
**
*/
#ifndef   PRN_SPC
/*
**
** internal code for SPC(X), next character is X.
**
*/
#define   PRN_SPC                0x01
#endif /* PRN_SPC */
#endif /* FALSE */

#if       FALSE
/*
**
** bwBASIC 3.20: not used 
**
*/
#ifndef   PRN_TAB
/*
**
** internal code for TAB(X), next character is X.
**
*/
#define   PRN_TAB                0x02
#endif /* PRN_TAB */
#endif /* FALSE */

#ifndef   NulChar
/*
**
** the character that terminates a C string, commonly CHR$(0)
**
*/
#define   NulChar                 '\0'        /* NUL */
#endif /* NulChar */

#ifndef   THE_PRICE_IS_RIGHT
/* 
**
** enable various simple techinques to improve execution speed  
**
*/
#define   THE_PRICE_IS_RIGHT      TRUE
#endif /* THE_PRICE_IS_RIGHT */

#ifndef   MAINTAINER_BUFFER_LENGTH
/*
**
** The length of buffers for MAINTINER commands
**
**/
#define   MAINTAINER_BUFFER_LENGTH       1023
#endif /* MAINTAINER_BUFFER_LENGTH */

/*-------------------------------------------------------------
                        FUNCTION PARAMETER TYPES
-------------------------------------------------------------*/
/* 
** Bit 0 is the first parameter, bit 31 is the 32nd parameter.  
** If the parameter is a string then the bit is set.
** If the parameter is a number then the bit is clear.
*/
typedef unsigned long ParamBitsType;

#ifndef   MAX_FARGS
#define   MAX_FARGS  ( sizeof( ParamBitsType ) * CHAR_BIT / 1 )
#endif /* MAX_FARGS */


#define PSTR( N )              ( 1 << ( N - 1 ) )        /* STRING parameter # 1 is bit 0 SET */
#define PNUM( N )              ( 0              )        /* NUMBER parameter # 1 is bit 0 CLR */
#define PNONE                  ( 0              )        /* function has no parameters */

#define P1STR PSTR(1)
#define P2STR PSTR(2)
#define P3STR PSTR(3)
#define P4STR PSTR(4)

#define P1NUM PNUM(1)
#define P2NUM PNUM(2)
#define P3NUM PNUM(3)
#define P4NUM PNUM(4)


/*-------------------------------------------------------------
                        FUNCTION PARAMETER TESTS
-------------------------------------------------------------*/
/* 
** Nibble 0 is the first parameter, 
** Nibble 7 is the 8th parameter.  
** Each nibble is P1ANY and so on. 
*/
typedef unsigned long ParamTestType;
#define MAX_TESTS  ( sizeof( ParamTestType ) * CHAR_BIT / 4 )

/*            0x87654321 <- Parameter Number (max # of Range-Checked parameters to INTRINSIC functions is 8) */
#define P1ERR 0X00000000        /* INTERNAL ERROR */
#define P1ANY 0X00000001        /* X is any valid number    , A$ is any valid string      */
#define P1BYT 0x00000002        /* MIN_BYT  <= X <= MAX_BYT , LEN(A$) >= sizeof(char)     */
#define P1INT 0x00000003        /* SHRT_MIN <= X <= SHRT_MAX, LEN(A$) >= sizeof(int)      */
#define P1LNG 0x00000004        /* LONG_MIN <= X <= LONG_MAX, LEN(A$) >= sizeof(long)     */
#define P1CUR 0x00000005        /* LONG_MIN <= X <= LONG_MAX, LEN(A$) >= sizeof(long)     */
#define P1FLT 0x00000006        /* MIN_FLT  <= X <= MAX_FLT , LEN(A$) >= sizeof(float)    */
#define P1DBL 0x00000007        /* MIN_DBL  <= X <= MAX_DBL , LEN(A$) >= sizeof(double)   */
#define P1DEV 0x00000008        /* RESERVED                 , RESERVED                    */
#define P1LEN 0x00000009        /* MIN_STR  <= X <= MAX_STR , RESERVED                    */
#define P1POS 0x0000000A        /* MIN_STR  <  X <= MAX_STR , RESERVED                    */
#define P1COM 0x0000000B        /* X in (1,2,3,4) COMx      , RESERVED                    */
#define P1LPT 0x0000000C        /* X in (0,1,2,3) PRN,LPTx  , RESERVED                    */
#define P1GTZ 0x0000000D        /* X  >    0                , RESERVED                    */
#define P1GEZ 0x0000000E        /* X  >=   0                , RESERVED                    */
#define P1NEZ 0x0000000F        /* X  <>   0                , RESERVED                    */

#define P2ERR (P1ERR << 4)
#define P2ANY (P1ANY << 4)
#define P2BYT (P1BYT << 4)
#define P2INT (P1INT << 4)
#define P2LNG (P1LNG << 4)
#define P2CUR (P1CUR << 4)
#define P2FLT (P1FLT << 4)
#define P2DBL (P1DBL << 4)
#define P2DEV (P1DEV << 4)
#define P2LEN (P1LEN << 4)
#define P2POS (P1POS << 4)
#define P2COM (P1COM << 4)
#define P2LPT (P1LPT << 4)
#define P2GTZ (P1GTZ << 4)
#define P2GEZ (P1GEZ << 4)
#define P2NEZ (P1NEZ << 4)

#define P3ERR (P1ERR << 8)
#define P3ANY (P1ANY << 8)
#define P3BYT (P1BYT << 8)
#define P3INT (P1INT << 8)
#define P3LNG (P1LNG << 8)
#define P3CUR (P1CUR << 8)
#define P3FLT (P1FLT << 8)
#define P3DBL (P1DBL << 8)
#define P3DEV (P1DEV << 8)
#define P3LEN (P1LEN << 8)
#define P3POS (P1POS << 8)
#define P3COM (P1COM << 8)
#define P3LPT (P1LPT << 8)
#define P3GTZ (P1GTZ << 8)
#define P3GEZ (P1GEZ << 8)
#define P3NEZ (P1NEZ << 8)

#define P4ERR (P1ERR << 12)
#define P4ANY (P1ANY << 12)
#define P4BYT (P1BYT << 12)
#define P4INT (P1INT << 12)
#define P4LNG (P1LNG << 12)
#define P4CUR (P1CUR << 12)
#define P4FLT (P1FLT << 12)
#define P4DBL (P1DBL << 12)
#define P4DEV (P1DEV << 12)
#define P4LEN (P1LEN << 12)
#define P4POS (P1POS << 12)
#define P4COM (P1COM << 12)
#define P4LPT (P1LPT << 12)
#define P4GTZ (P1GTZ << 12)
#define P4GEZ (P1GEZ << 12)
#define P4NEZ (P1NEZ << 12)





/***************************************************************
  
                       Structures
  
***************************************************************/

/*-------------------------------------------------------------
                        OPTION VERSION bitmask
-------------------------------------------------------------*/
typedef unsigned long OptionVersionType;        /* upto 32 BASIC dialects */

/*-------------------------------------------------------------
                        Result of attempt to parse
-------------------------------------------------------------*/
typedef enum
{
  RESULT_UNPARSED = 1,
  RESULT_ERROR,
  RESULT_SUCCESS
} ResultType;

/*-------------------------------------------------------------
                        STRINGS
-------------------------------------------------------------*/
struct StringStructure
{
  size_t length;                /* length of string */
  char *sbuffer;                /* pointer to string buffer */
};
typedef struct StringStructure StringType;


/*-------------------------------------------------------------
                        VARIABLES
-------------------------------------------------------------*/
union ValuePtrUnion
{
  DoubleType *Number;                /* memory for number */
  StringType *String;                /* memory for string */
};
typedef union ValuePtrUnion ValuePtrType;

struct VariableStructure
{
  struct VariableStructure *next;      /* next variable in chain */
  char name[NameLengthMax + 1];        /* name */
  ValuePtrType Value;                  /* memory for values */
  size_t array_units;                  /* total number of units of memory */
  int LBOUND[MAX_DIMS];                /* lower bound for each dimension */
  int UBOUND[MAX_DIMS];                /* upper bound for each dimension */
  int VINDEX[MAX_DIMS];                /* current index in each dimension */
  int dimensions;                      /* number of dimensions, 0 == scalar */
  unsigned char VariableFlags;         /* VARIABLE_... */
  char VariableTypeCode;               /* StringTypeCode and so on */
};
typedef struct VariableStructure VariableType;
#define VAR_IS_STRING( v ) ( v->VariableTypeCode == StringTypeCode )
/* VariableFlags */
#define VARIABLE_RESERVED_01 0x01        /* reserved for future use */
#define VARIABLE_RESERVED_02 0x02        /* reserved for future use */
#define VARIABLE_RESERVED_04 0x04        /* reserved for future use */
#define VARIABLE_CONSTANT    0x08        /* CONST variable: value cannot be changed again */
#define VARIABLE_VIRTUAL     0x10        /* VIRTUAL variable: values are stored in disk file */
#define VARIABLE_DISPLAY     0x20        /* DSP variable: display every assignment */
#define VARIABLE_PRESET      0x40        /* preset variable: CLEAR should not delete */
#define VARIABLE_COMMON      0x80        /* COMMON variable: available to CHAINed program */


/*-------------------------------------------------------------
                        LINES
-------------------------------------------------------------*/
struct LineStructure
{
  struct LineStructure *next;      /* pointer to next line in chain */
  struct LineStructure *OtherLine; /* pointer to other line in loops */
  char *buffer;                    /* buffer to hold the line */
  int number;                      /* line number */
  int position;                    /* current position in line */
  int cmdnum;                      /* C_... */
  int Startpos;                    /* start of rest of line read from buffer */
  int Indention;                   /* structure command indention level */
  int IncludeLevel;                /* %INCLUDE file level */
  unsigned char LineFlags;         /* LINE_... */
};
typedef struct LineStructure LineType;
/* LineFlags */
#define LINE_EXECUTED        0x01        /* line was executed */
#define LINE_NUMBERED        0x02        /* line was manually numbered */
#define LINE_BREAK           0x04        /* BREAK line */
#define LINE_USER            0x08        /* USER line executing from console */
#define LINE_RESERVED_10     0x10        /* reserved for future use */
#define LINE_RESERVED_20     0x20        /* reserved for future use */
#define LINE_RESERVED_40     0x40        /* reserved for future use */
#define LINE_RESERVED_80     0x80        /* reserved for future use */


/*-------------------------------------------------------------
               USER FUNCTIONS, SUBS and LABELS
-------------------------------------------------------------*/
struct UserFunctionStructure
{
  struct UserFunctionStructure *next;
  char *name;
  char ReturnTypeCode;                    /* StringTypeCode and so on */
  unsigned char ParameterCount;           /* 0..MAX_FARGS, 255 == VARIANT (...) */
  ParamBitsType ParameterTypes;           /* parameter signature, must hold MAX_FARGS bits */
  LineType *line;                         /* points to the FUNCTION, SUB, or LABEL line */
  int startpos;                           /* starting position in line */
  VariableType *local_variable;           /* local variables for this function or sub */
  OptionVersionType OptionVersionBitmask; /* OPTION VERSION bitmask */
};
typedef struct UserFunctionStructure UserFunctionType;


/*-------------------------------------------------------------
                      INTRINSIC FUNCTIONS
-------------------------------------------------------------*/
struct IntrinsicFunctionStructure
{
  const int FunctionID;                      /* F_... */
  const char *Syntax;                        /* the syntax of the function */
  const char *Description;                   /* the description of the function */
  const char *Name;                          /* the name of the function, such as "MID$" */
  const char ReturnTypeCode;                 /* StringTypeCode and so on */
  const unsigned char ParameterCount;        /* 0..MAX_FARGS, 255 == VARIANT (...) */
  const ParamBitsType ParameterTypes;        /* parameter signature, holds MAX_FARGS bits */
  const ParamTestType ParameterTests;        /* parameter checks, must hold 8 nibbles, only first 8 parameters */
  OptionVersionType OptionVersionBitmask;    /* OPTION VERSION bitmask */
};
typedef struct IntrinsicFunctionStructure IntrinsicFunctionType;
extern IntrinsicFunctionType IntrinsicFunctionTable[ /* NUM_FUNCTIONS */ ];
extern const size_t NUM_FUNCTIONS;


/*-------------------------------------------------------------
                        INTRINSIC COMMANDS
-------------------------------------------------------------*/
struct CommandStructure
{
  const int CommandID;                     /* C_... */
  const char *Syntax;                      /* the syntax of the command */
  const char *Description;                 /* the description of the command */
  const char *name;                        /* the name of the command, such as "PRINT" */
  OptionVersionType OptionVersionBitmask;  /* OPTION VERSION bitmask */
};
typedef struct CommandStructure CommandType;
extern CommandType IntrinsicCommandTable[ /* NUM_COMMANDS  */ ];
extern const size_t NUM_COMMANDS;


/*-------------------------------------------------------------
                        FILES
-------------------------------------------------------------*/
struct FileStructure
{
  struct FileStructure *next;
  int FileNumber;              /* 1-based file number used to OPEN the file ( 0 for SYSIN/SYSOUT, -1 for SYSPRN */
  int DevMode;                 /* DEVMODE_... */
  int width;                   /* 1-based width for OUTPUT and APPEND; reclen for RANDOM; not used for INPUT or BINARY */
  int col;                     /* 1-based current column for OUTPUT and APPEND */
  int row;                     /* 1-based current row for OUTPUT and APPEND */
  int EOF_LineNumber;          /* IF END # filenumber THEN EOF_linenumber */
  FILE *cfp;                   /* C file pointer for this device */
  char *buffer;                /* pointer to character buffer for RANDOM */
  char delimit;                /* DELIMIT for READ and WRITE */
  char *FileName;              /* File Name */
};
typedef struct FileStructure FileType;
/* DevMode */
#define DEVMODE_CLOSED     0x00
#define DEVMODE_INPUT      0x01
#define DEVMODE_OUTPUT     0x02
#define DEVMODE_RANDOM     0x04
#define DEVMODE_APPEND     0x08
#define DEVMODE_VIRTUAL    0x10
#define DEVMODE_BINARY     0x20
#define DEVMODE_X40        0x40 /* reserved for future use */
#define DEVMODE_X80        0x80 /* reserved for future use */
#define DEVMODE_READ    (DEVMODE_INPUT  | DEVMODE_RANDOM                  | DEVMODE_BINARY )
#define DEVMODE_WRITE   (DEVMODE_OUTPUT | DEVMODE_RANDOM | DEVMODE_APPEND | DEVMODE_BINARY )


/*-------------------------------------------------------------
                        FIELDS in a RANDOM file
-------------------------------------------------------------*/
struct FieldStructure
{
  /* only used for RANDOM files in dialects with a FIELD command */
  struct FieldStructure *next;
  FileType *File;
  int FieldOffset;
  int FieldLength;
  VariableType *Var;
  int VINDEX[MAX_DIMS];                /* current position in array */
};
typedef struct FieldStructure FieldType;


/*-------------------------------------------------------------
                        VIRTUAL VARIABLES
-------------------------------------------------------------*/
struct VirtualStructure
{
  /* only used for VIRTUAL variables in dialects with a DIM # command */
  struct VirtualStructure *next;
  int FileNumber;                /* note: the file can be opened AFTER the variable is declared */
  size_t FileOffset;             /* byte offset from the beginning of the file */
  int FileLength;                /* sizeof( DoubleType ) or Fixed String Length */
  VariableType *Variable;        /* the variable */
};
typedef struct VirtualStructure VirtualType;


/*-------------------------------------------------------------
                        STACK
-------------------------------------------------------------*/
struct StackStructure
{
  struct StackStructure *next;
  LineType *line;                /* line for execution */
  int ExecCode;                  /* EXEC_... */
  VariableType *local_variable;  /* local variables of FUNCTION and variable of FOR */
  DoubleType for_step;           /* STEP value of FOR */
  DoubleType for_target;         /* TO value of FOR */
  LineType *LoopTopLine;         /* top line of FOR, DO, WHILE, UNTIL */
  int OnErrorGoto;               /* ON ERROR GOTO line */
};
typedef struct StackStructure StackType;
/* ExecCode */
#define EXEC_NORM        0
#define EXEC_GOSUB       1
#define EXEC_FOR         2
#define EXEC_FUNCTION    3
#define EXEC_POPPED      4


/*-------------------------------------------------------------
                        VERSION
-------------------------------------------------------------*/
struct VersionStructure
{
  const char *Name;                /* OPTION VERSION name$ */
  const OptionVersionType OptionVersionValue;        /* ( D64 ,...) */
  const char *ID;                  /* ("D64",...) */
  const char *Description;         /* short description of this dialect */
  const char *ReferenceTitle;      /* Reference document */
  const char *ReferenceAuthor;     /* Reference document */
  const char *ReferenceCopyright;  /* Reference document */
  const char *ReferenceURL1;       /* Reference document */
  const char *ReferenceURL2;       /* Reference document */
  unsigned short OptionFlags;
  int OptionReclenInteger;         /* OPTION RECLEN      integer: Default RANDOM record length */
  int OptionBaseInteger;           /* OPTION BASE        integer: Default lower bound for arrays */
  char *OptionDateFormat;          /* OPTION DATE        format$: strftime() format for DATE$ */
  char *OptionTimeFormat;          /* OPTION TIME        format$: strftime() format for TIME$ */
  char OptionStringChar;           /* OPTION STRING        char$: Suffix for StringTypeCode  , commonly '$' */
  char OptionDoubleChar;           /* OPTION DOUBLE        char$: Suffix for DoubleTypeCode  , commonly '#' */
  char OptionSingleChar;           /* OPTION SINGLE        char$: Suffix for SingleTypeCode  , commonly '!' */
  char OptionCurrencyChar;         /* OPTION CURRENCY      char$: Suffix for CurrencyTypeCode, commonly '@' */
  char OptionLongChar;             /* OPTION LONG          char$: Suffix for LongTypeCode    , commonly '&' */
  char OptionIntegerChar;          /* OPTION INTEGER       char$: Suffix for IntegerTypeCode , commonly '%' */
  char OptionByteChar;             /* OPTION BYTE          char$: Suffix for ByteTypeCode    , commonly '~' */
  char OptionQuoteChar;            /* OPTION QUOTE         char$: Quote, commonly a double-quote */
  char OptionCommentChar;          /* OPTION COMMENT       char$: Comment, commonly a single-quote */
  char OptionStatementChar;        /* OPTION STATEMENT     char$: Statement Seperator, commonly ':' */
  char OptionPrintChar;            /* OPTION PRINT         char$: PRINT Statement, commonly '?' */
  char OptionInputChar;            /* OPTION INPUT         char$: INPUT Statement, commonly '!' */
  char OptionImageChar;            /* OPTION IMAGE         char$: IMAGE Statement, commonly ':' */
  char OptionLparenChar;           /* OPTION LPAREN        char$: Left Parenthesis, commonly '(' */
  char OptionRparenChar;           /* OPTION RPAREN        char$: Right Parenthesis, commonly ')' */
  char OptionFilenumChar;          /* OPTION FILENUM       char$: File Number, commonly '#' */
  char OptionAtChar;               /* OPTION AT            char$: PRINT AT, commonly '@' */
  char OptionUsingDigit;           /* OPTION USING DIGIT   char$: PRINT USING Digit placeholder, commonly '#' */
  char OptionUsingComma;           /* OPTION USING COMMA   char$: PRINT USING Comma, such as thousands and millions, commonly ',' */
  char OptionUsingPeriod;          /* OPTION USING PERIOD  char$: PRINT USING Period, such as between dollars and cents, commonly '.' */
  char OptionUsingPlus;            /* OPTION USING PLUS    char$: PRINT USING Plus  sign, positive value, commonly '+' */
  char OptionUsingMinus;           /* OPTION USING MINUS   char$: PRINT USING Minus sign, negative value, commonly '-' */
  char OptionUsingExrad;           /* OPTION USING EXRAD   char$: PRINT USING Exponential format, commonly '^' */
  char OptionUsingDollar;          /* OPTION USING DOLLAR  char$: PRINT USING Currency symbol, such as dollar, commonly '$' */
  char OptionUsingFiller;          /* OPTION USING FILLER  char$: PRINT USING Filler, such as payroll checks, commonly '*' */
  char OptionUsingLiteral;         /* OPTION USING LITERAL char$: PRINT USING Literal, commonly '_' */
  char OptionUsingFirst;           /* OPTION USING FIRST   char$: PRINT USING First character of a string, commonly '!' */
  char OptionUsingAll;             /* OPTION USING ALL     char$: PRINT USING All of a string, commonly '&' */
  char OptionUsingLength;          /* OPTION USING LENGTH  char$: PRINT USING Length of a string, commonly '%' */
};
typedef struct VersionStructure VersionType;
extern VersionType bwb_vertable[ /* NUM_VERSIONS  */ ];        /* table of predefined versions  */
extern const size_t NUM_VERSIONS;        /* upto 32 BASIC dialects */
/* OptionVersionValue */
#define B15 (1UL<< 0)                /* Bywater    */
#define S70 (1UL<< 1)                /* CALL/360   */
#define C77 (1UL<< 2)                /* CBASIC-II  */
#define D64 (1UL<< 3)                /* Dartmouth  */
#define E78 (1UL<< 4)                /* ECMA-55    */
#define E86 (1UL<< 5)                /* ECMA-116   */
#define G74 (1UL<< 6)                /* GCOS       */
#define HB1 (1UL<< 7)                /* Handbook1  */
#define HB2 (1UL<< 8)                /* Handbook2  */
#define H80 (1UL<< 9)                /* Heath      */
#define G65 (1UL<<10)                /* Mark-I     */
#define G67 (1UL<<11)                /* Mark-II    */
#define M80 (1UL<<12)                /* MBASIC     */
#define D73 (1UL<<13)                /* PDP-8      */
#define D70 (1UL<<14)                /* PDP-11     */
#define R86 (1UL<<15)                /* RBASIC     */
#define D71 (1UL<<16)                /* RSTS-11    */
#define I70 (1UL<<17)                /* System/360 */
#define I73 (1UL<<18)                /* System/370 */
#define T80 (1UL<<19)                /* TRS-80     */
#define V09 (1UL<<20)                /* Vintage    */
#define T79 (1UL<<21)                /* XBASIC     */
#define H14 (1UL<<22)                /* Haart      */
#define B93 (1UL<<23)                /* Bywater-2  */

/* OptionFlags */
#define OPTION_STRICT_ON      0x0001        /* Do NOT allow implicit DIM */
#define OPTION_ANGLE_DEGREES  0x0002        /* Use degrees instead of radians */
#define OPTION_BUGS_ON        0x0004        /* ON ... , FOR ..., VAL("X") */
#define OPTION_LABELS_ON      0x0008        /* Labels allowed */
#define OPTION_COMPARE_TEXT   0x0010        /* Case-Insensitive string comparison */
#define OPTION_BUGS_BOOLEAN   0x0020        /* Boolean returns 1 or 0 instead of bitwise */
#define OPTION_COVERAGE_ON    0x0040        /* track BASIC lines actually executed */
#define OPTION_TRACE_ON       0x0080        /* dump BASIC stack trace when FATAL error */
#define OPTION_ERROR_GOSUB    0X0100        /* error causes GOSUB instead of GOTO */
#define OPTION_EXPLICIT_ON    0x0200        /* all variables must be declared via DIM */
#define OPTION_ANGLE_GRADIANS 0x0400        /* Use gradians instead of radians */
#define OPTION_RESERVED_0800  0x0800        /* reserved for future use */
#define OPTION_RESERVED_1000  0x1000        /* reserved for future use */
#define OPTION_RESERVED_2000  0x2000        /* reserved for future use */
#define OPTION_RESERVED_4000  0x4000        /* reserved for future use */
#define OPTION_RESERVED_8000  0x8000        /* reserved for future use */
#define IS_CHAR( X, Y ) ( (X) == (Y) && (Y) != NulChar )        /* used with the Option...Char settings */


/*-------------------------------------------------------------
                        VARIANT
-------------------------------------------------------------*/
struct VariantStructure
{
  char VariantTypeCode;   /* StringTypeCode and so on, NulChar == unknown */
  DoubleType Number;      /* only when VariamtTypeCode != StringTypeCode */
  size_t Length;          /* only when VariantTypeCode == StringTypeCode */
  char *Buffer;           /* only when VariantTypeCode == StringTypeCode */
};
typedef struct VariantStructure VariantType;
#define CLEAR_VARIANT( X ) bwb_memset( (X), 0, sizeof( VariantType ) )
#define RELEASE_VARIANT( X ) if( (X)->Buffer != NULL ) { free( (X)->Buffer ); (X)->Buffer = NULL; (X)->Length = 0; }
/* 
any routine changing the TypeCode is required to calloc() or free() the Buffer, 
this is checked by is_string_type() and is_number_type() in bwb_exp.c 
*/


/*-------------------------------------------------------------
                        GLOBAL
-------------------------------------------------------------*/
struct GlobalStruct
{
  /* these values are not changed by OPTION VERSION */
  jmp_buf mark;                        /* break_handler() jumps here */
  int program_run;                     /* has the command-line program been run? */
  VersionType *CurrentVersion;         /* OPTION VERSION */
  LineType *ThisLine;                  /* currently executing line */
  int IsScanRequired;                  /* TRUE == program needs to be rescanned */
  int IsTraceOn;                       /* TRUE == TRON, FALSE == TROFF */
  int IsInteractive;                   /* always TRUE */
  int IsCommandLineFile;               /* TRUE == executing a program from the Operating System command line */
  int IsErrorPending;                  /* TRUE == ERR, ERL and ERROR$ have been set */
  int IsTimerOn;                       /* TRUE == ON TIMER ... GOSUB ... enabled */
  int IsPrinter;                       /* CBASIC-II: LPRINTER sets to TRUE, CONSOLE sets to FALSE */
  int LPRINT_NULLS;                    /* LPRINT_FILE_NUMBER */
  int SCREEN_ROWS;                     /* CONSOLE_FILE_NUMBER */
  int AutomaticLineNumber;             /* AUTO */
  int AutomaticLineIncrement;          /* AUTO */
  int LastInputCount;                  /* 0 == none */
  int LastLineNumber;                  /* LIST, LLIST */
  int LastFileNumber;                  /* 0 == none */
  int StartTimeInteger;                /* seconds after midnight when RUN was executed */
  int NextValidLineNumber;             /* verify line number order */
  int StackDepthInteger;               /* stack depth count */
  LineType *DataLine;                  /* current DATA line for READ */
  int DataPosition;                    /* current position in DataLine */
  int ERR;                             /* ERR */
  LineType *ERL;                       /* ERL */
  char *ERROR4;                        /* ERROR$ */
  DoubleType LastDeterminant;          /* 0 == error */
  DoubleType RESIDUE;                  /* for RBASIC's RESIDUE function */
  FILE *ExternalInputFile;             /* --TAPE tapefile.inp */
  int OnTimerLineNumber;               /* ON TIMER ... GOSUB ... */
  DoubleType OnTimerCount;             /* ON TIMER ... GOSUB ... */
  DoubleType OnTimerExpires;           /* ON TIMER ... GOSUB ... */
  LineType *ContinueLine;              /* CONT */
  int IncludeLevel;                    /* %INCLUDE */
  DoubleType OptionSleepDouble;        /* OPTION SLEEP    double  */
  int OptionIndentInteger;             /* OPTION INDENT   integer */
  int OptionTerminalType;              /* OPTION TERMINAL type    */
  int OptionRoundType;                 /* OPTION ROUND    type    */
  char *OptionEditString;              /* OPTION EDIT     string$ */
  char *OptionFilesString;             /* OPTION FILES    string$ */
  char *OptionRenumString;             /* OPTION RENUM    string$ */
  char *OptionPromptString;            /* OPTION PROMPT   string$ */
  char *OptionExtensionString;         /* OPTION EXTENSION string$ */
  int OptionScaleInteger;              /* OPTION SCALE    integer */
  int OptionDigitsInteger;             /* OPTION DIGITS   integer */
  int OptionZoneInteger;               /* OPTION ZONE     integer */
  LineType *StartMarker;               /* BASIC program starting marker */
  LineType *UserMarker;                /* USER (immediate mode) starting marker */
  LineType *EndMarker;                 /* ending marker for both StartMarker and UserMarker */
  char *MaxLenBuffer;                  /* MAXLEN, for bwb_exp.c */
  char *NumLenBuffer;                  /* NUMLEN, for FormatBasicNumber(), sprintf("%d") and so on */
  char *ConsoleOutput;                 /* MAX_LINE_LENGTH, usually prompts and error messages */
  char *ConsoleInput;                  /* MAX_LINE_LENGTH, usually console and INPUT */
  VariableType *VariableHead;          /* USER Variable list */
  UserFunctionType *UserFunctionHead;  /* USER function-sub-label list */
  StackType *StackHead;                /* USER Stack list */
  FileType *FileHead;                  /* USER file list (not SYSIN, SYSOUT, or SYSPRN) */
  FieldType *FieldHead;                /* USER field list */
  VirtualType *VirtualHead;            /* USER Virtual cross-reference list */
  FileType *SYSIN;                     /* FileNumber = 0, INPUT, usually stdin, OPTION SYSIN  */
  FileType *SYSOUT;                    /* FileNumber = 0, PRINT, usually stdout, OPTION SYSOUT */
  FileType *SYSPRN;                    /* FileNumber < 0, LPRINT, usually stderr, OPTION SYSPRN */
  FileType *CurrentFile;               /* current file for PRINT or INPUT */
  char *UseParameterString;            /* CHAIN ... USE */
  int DefaultVariableType[26];         /* VariableTypeCode, A-Z */
#if THE_PRICE_IS_RIGHT
  int CommandStart[26];                /* IntrinsicCommandTable->name[0], A-Z */
  int IntrinsicFunctionStart[26];      /* IntrinsicFunctionTable->name[0], A-Z */
#endif                                 /* THE_PRICE_IS_RIGHT */
  char *COMMAND4[10];                  /* command line parameters, COMMAND$(0-9) */
  char *ProgramFilename;               /* last BASIC program filename */
};
typedef struct GlobalStruct GlobalType;
extern GlobalType *My;

#define PARAM_NUMBER   *argn->Value.Number
#define PARAM_LENGTH    argn->Value.String->length
#define PARAM_BUFFER    argn->Value.String->sbuffer
#define RESULT_NUMBER  *argv->Value.Number
#define RESULT_BUFFER   argv->Value.String->sbuffer
#define RESULT_LENGTH   argv->Value.String->length


/***************************************************************
  
                    Compiler Specific Prototypes
  
***************************************************************/


#if __WATCOM__
/*
**
** OpenWatcom 1.9 for DOS does not declare putenv()
**
*/
extern int putenv (const char *buffer);
#endif /*  __WATCOM__ */

#if HAVE_UNIX_GCC
/*
**
** sleep() is an intrinsic C functions in GCC using -ansi 
**
*/
#else
/*
**
** sleep() is defined in bwb_int.c to do nothing 
**
*/
extern unsigned int sleep (unsigned int X);
#endif /* HAVE_UNIX_GCC */

#ifndef isnan
#define isnan(x) (x != x)
#endif

#ifndef isinf
#define isinf(x) (!isnan(x) && isnan(x - x))
#endif

/***************************************************************
  
                      Function Prototypes
  
***************************************************************/

/*-------------------------------------------------------------
                        BASIC COMMANDS
-------------------------------------------------------------*/
extern LineType *bwb_APPEND (LineType * l);
extern LineType *bwb_AS (LineType * L);
extern LineType *bwb_AUTO (LineType * L);
extern LineType *bwb_BACKSPACE (LineType * Line);
extern LineType *bwb_BREAK (LineType * l);
extern LineType *bwb_BUILD (LineType * L);
extern LineType *bwb_BYE (LineType * l);
extern LineType *bwb_CALL (LineType * l);
extern LineType *bwb_CASE (LineType * l);
extern LineType *bwb_CASE_ELSE (LineType * l);
extern LineType *bwb_CHAIN (LineType * l);
extern LineType *bwb_CHANGE (LineType * l);
extern LineType *bwb_CLEAR (LineType * l);
extern LineType *bwb_CLOAD (LineType * Line);
extern LineType *bwb_CLOAD8 (LineType * l);
extern LineType *bwb_CLOSE (LineType * Line);
extern LineType *bwb_CLR (LineType * l);
extern LineType *bwb_CMDS (LineType * l);
extern LineType *bwb_COMMON (LineType * l);
extern LineType *bwb_CONSOLE (LineType * l);
extern LineType *bwb_CONST (LineType * L);
extern LineType *bwb_CONT (LineType * l);
extern LineType *bwb_CONTINUE (LineType * l);
extern LineType *bwb_COPY (LineType * Line);
extern LineType *bwb_CREATE (LineType * l);
extern LineType *bwb_CSAVE (LineType * Line);
extern LineType *bwb_CSAVE8 (LineType * l);
extern LineType *bwb_DATA (LineType * Line);
extern LineType *bwb_DEC (LineType * L);
extern LineType *bwb_DEF (LineType * l);
extern LineType *bwb_DEF8LBL (LineType * l);
extern LineType *bwb_DEFBYT (LineType * l);
extern LineType *bwb_DEFCUR (LineType * l);
extern LineType *bwb_DEFDBL (LineType * l);
extern LineType *bwb_DEFINT (LineType * l);
extern LineType *bwb_DEFLNG (LineType * l);
extern LineType *bwb_DEFSNG (LineType * l);
extern LineType *bwb_DEFSTR (LineType * l);
extern LineType *bwb_DELETE (LineType * l);
extern LineType *bwb_DELIMIT (LineType * l);
extern LineType *bwb_DIM (LineType * l);
extern LineType *bwb_DISPLAY (LineType * Line);
extern LineType *bwb_DO (LineType * l);
extern LineType *bwb_DOS (LineType * l);
extern LineType *bwb_DSP (LineType * l);
extern LineType *bwb_EDIT (LineType * Line);
extern LineType *bwb_ELSE (LineType * l);
extern LineType *bwb_ELSEIF (LineType * l);
extern LineType *bwb_END (LineType * l);
extern LineType *bwb_END_FUNCTION (LineType * l);
extern LineType *bwb_END_IF (LineType * l);
extern LineType *bwb_END_SELECT (LineType * l);
extern LineType *bwb_END_SUB (LineType * l);
extern LineType *bwb_ERASE (LineType * l);
extern LineType *bwb_EXCHANGE (LineType * l);
extern LineType *bwb_EXIT (LineType * l);
extern LineType *bwb_EXIT_DO (LineType * l);
extern LineType *bwb_EXIT_FOR (LineType * l);
extern LineType *bwb_EXIT_FUNCTION (LineType * l);
extern LineType *bwb_EXIT_REPEAT (LineType * l);
extern LineType *bwb_EXIT_SUB (LineType * l);
extern LineType *bwb_EXIT_WHILE (LineType * l);
extern LineType *bwb_FEND (LineType * l);
extern LineType *bwb_FIELD (LineType * l);
extern LineType *bwb_FILE (LineType * l);
extern LineType *bwb_FILES (LineType * l);
extern LineType *bwb_FLEX (LineType * l);
extern LineType *bwb_FNCS (LineType * l);
extern LineType *bwb_FNEND (LineType * l);
extern LineType *bwb_FOR (LineType * l);
extern LineType *bwb_FUNCTION (LineType * l);
extern LineType *bwb_GET (LineType * Line);
extern LineType *bwb_GO (LineType * L);
extern LineType *bwb_GOODBYE (LineType * l);
extern LineType *bwb_GOSUB (LineType * l);
extern LineType *bwb_GOTO (LineType * l);
extern LineType *bwb_GO_SUB (LineType * l);
extern LineType *bwb_GO_TO (LineType * l);
extern LineType *bwb_HELP (LineType * l);
extern LineType *bwb_IF (LineType * l);
extern LineType *bwb_IF8THEN (LineType * l);
extern LineType *bwb_IF_END (LineType * l);
extern LineType *bwb_IF_MORE (LineType * l);
extern LineType *bwb_IMAGE (LineType * L);
extern LineType *bwb_INC (LineType * L);
extern LineType *bwb_INPUT (LineType * Line);
extern LineType *bwb_INPUT_LINE (LineType * Line);
extern LineType *bwb_LET (LineType * L);
extern LineType *bwb_LINE (LineType * Line);
extern LineType *bwb_LINE_INPUT (LineType * Line);
extern LineType *bwb_LIST (LineType * l);
extern LineType *bwb_LISTNH (LineType * l);
extern LineType *bwb_LLIST (LineType * l);
extern LineType *bwb_LOAD (LineType * Line);
extern LineType *bwb_LOCAL (LineType * l);
extern LineType *bwb_LOOP (LineType * l);
extern LineType *bwb_LPRINT (LineType * l);
extern LineType *bwb_LPRINTER (LineType * l);
extern LineType *bwb_LPT (LineType * l);
extern LineType *bwb_LSET (LineType * l);
extern LineType *bwb_MAINTAINER (LineType * l);
extern LineType *bwb_MAINTAINER_CMDS (LineType * l);
extern LineType *bwb_MAINTAINER_CMDS_HTML (LineType * l);
extern LineType *bwb_MAINTAINER_CMDS_ID (LineType * l);
extern LineType *bwb_MAINTAINER_CMDS_MANUAL (LineType * l);
extern LineType *bwb_MAINTAINER_CMDS_SWITCH (LineType * l);
extern LineType *bwb_MAINTAINER_CMDS_TABLE (LineType * l);
extern LineType *bwb_MAINTAINER_DEBUG (LineType * l);
extern LineType *bwb_MAINTAINER_DEBUG_OFF (LineType * l);
extern LineType *bwb_MAINTAINER_DEBUG_ON (LineType * l);
extern LineType *bwb_MAINTAINER_FNCS (LineType * l);
extern LineType *bwb_MAINTAINER_FNCS_HTML (LineType * l);
extern LineType *bwb_MAINTAINER_FNCS_ID (LineType * l);
extern LineType *bwb_MAINTAINER_FNCS_MANUAL (LineType * l);
extern LineType *bwb_MAINTAINER_FNCS_SWITCH (LineType * l);
extern LineType *bwb_MAINTAINER_FNCS_TABLE (LineType * l);
extern LineType *bwb_MAINTAINER_MANUAL (LineType * l);
extern LineType *bwb_MAINTAINER_STACK (LineType * l);
extern LineType *bwb_MARGIN (LineType * l);
extern LineType *bwb_MAT (LineType * l);
extern LineType *bwb_MAT_GET (LineType * Line);
extern LineType *bwb_MAT_INPUT (LineType * Line);
extern LineType *bwb_MAT_PRINT (LineType * l);
extern LineType *bwb_MAT_PUT (LineType * l);
extern LineType *bwb_MAT_READ (LineType * Line);
extern LineType *bwb_MAT_WRITE (LineType * l);
extern LineType *bwb_MERGE (LineType * l);
extern LineType *bwb_MID4 (LineType * l);
extern LineType *bwb_MON (LineType * l);
extern LineType *bwb_NAME (LineType * l);
extern LineType *bwb_NEW (LineType * l);
extern LineType *bwb_NEXT (LineType * l);
extern LineType *bwb_OF (LineType * L);
extern LineType *bwb_OLD (LineType * Line);
extern LineType *bwb_ON (LineType * l);
extern LineType *bwb_ON_ERROR (LineType * l);
extern LineType *bwb_ON_ERROR_GOSUB (LineType * l);
extern LineType *bwb_ON_ERROR_GOTO (LineType * l);
extern LineType *bwb_ON_ERROR_RESUME (LineType * l);
extern LineType *bwb_ON_ERROR_RESUME_NEXT (LineType * l);
extern LineType *bwb_ON_ERROR_RETURN (LineType * l);
extern LineType *bwb_ON_ERROR_RETURN_NEXT (LineType * l);
extern LineType *bwb_ON_TIMER (LineType * l);
extern LineType *bwb_OPEN (LineType * l);
extern LineType *bwb_OPTION (LineType * l);
extern LineType *bwb_OPTION_ANGLE (LineType * l);
extern LineType *bwb_OPTION_ANGLE_DEGREES (LineType * l);
extern LineType *bwb_OPTION_ANGLE_GRADIANS (LineType * l);
extern LineType *bwb_OPTION_ANGLE_RADIANS (LineType * l);
extern LineType *bwb_OPTION_ARITHMETIC (LineType * l);
extern LineType *bwb_OPTION_ARITHMETIC_DECIMAL (LineType * l);
extern LineType *bwb_OPTION_ARITHMETIC_FIXED (LineType * l);
extern LineType *bwb_OPTION_ARITHMETIC_NATIVE (LineType * l);
extern LineType *bwb_OPTION_PUNCT_AT (LineType * l);
extern LineType *bwb_OPTION_BASE (LineType * l);
extern LineType *bwb_OPTION_BUGS (LineType * l);
extern LineType *bwb_OPTION_BUGS_BOOLEAN (LineType * l);
extern LineType *bwb_OPTION_BUGS_OFF (LineType * l);
extern LineType *bwb_OPTION_BUGS_ON (LineType * l);
extern LineType *bwb_OPTION_PUNCT_BYTE (LineType * l);
extern LineType *bwb_OPTION_PUNCT_COMMENT (LineType * l);
extern LineType *bwb_OPTION_COMPARE (LineType * l);
extern LineType *bwb_OPTION_COMPARE_BINARY (LineType * l);
extern LineType *bwb_OPTION_COMPARE_DATABASE (LineType * l);
extern LineType *bwb_OPTION_COMPARE_TEXT (LineType * l);
extern LineType *bwb_OPTION_COVERAGE (LineType * l);
extern LineType *bwb_OPTION_COVERAGE_OFF (LineType * l);
extern LineType *bwb_OPTION_COVERAGE_ON (LineType * l);
extern LineType *bwb_OPTION_PUNCT_CURRENCY (LineType * l);
extern LineType *bwb_OPTION_DATE (LineType * l);
extern LineType *bwb_OPTION_DIGITS (LineType * l);
extern LineType *bwb_OPTION_DISABLE (LineType * l);
extern LineType *bwb_OPTION_DISABLE_COMMAND (LineType * l);
extern LineType *bwb_OPTION_DISABLE_FUNCTION (LineType * l);
extern LineType *bwb_OPTION_DISABLE_OPERATOR (LineType * l);
extern LineType *bwb_OPTION_PUNCT_DOUBLE (LineType * l);
extern LineType *bwb_OPTION_EDIT (LineType * l);
extern LineType *bwb_OPTION_ENABLE (LineType * l);
extern LineType *bwb_OPTION_ENABLE_COMMAND (LineType * l);
extern LineType *bwb_OPTION_ENABLE_FUNCTION (LineType * l);
extern LineType *bwb_OPTION_ENABLE_OPERATOR (LineType * l);
extern LineType *bwb_OPTION_ERROR (LineType * l);
extern LineType *bwb_OPTION_ERROR_GOSUB (LineType * l);
extern LineType *bwb_OPTION_ERROR_GOTO (LineType * l);
extern LineType *bwb_OPTION_EXPLICIT (LineType * l);
extern LineType *bwb_OPTION_PUNCT_FILENUM (LineType * l);
extern LineType *bwb_OPTION_FILES (LineType * l);
extern LineType *bwb_OPTION_PUNCT_IMAGE (LineType * l);
extern LineType *bwb_OPTION_IMPLICIT (LineType * l);
extern LineType *bwb_OPTION_INDENT (LineType * l);
extern LineType *bwb_OPTION_PUNCT_INPUT (LineType * l);
extern LineType *bwb_OPTION_PUNCT_INTEGER (LineType * l);
extern LineType *bwb_OPTION_LABELS (LineType * l);
extern LineType *bwb_OPTION_LABELS_OFF (LineType * l);
extern LineType *bwb_OPTION_LABELS_ON (LineType * l);
extern LineType *bwb_OPTION_PUNCT_LONG (LineType * l);
extern LineType *bwb_OPTION_PUNCT_LPAREN (LineType * l);
extern LineType *bwb_OPTION_PUNCT_PRINT (LineType * l);
extern LineType *bwb_OPTION_PROMPT (LineType * l);
extern LineType *bwb_OPTION_PUNCT_QUOTE (LineType * l);
extern LineType *bwb_OPTION_RECLEN (LineType * l);
extern LineType *bwb_OPTION_RENUM (LineType * l);
extern LineType *bwb_OPTION_ROUND (LineType * l);
extern LineType *bwb_OPTION_ROUND_BANK (LineType * l);
extern LineType *bwb_OPTION_ROUND_MATH (LineType * l);
extern LineType *bwb_OPTION_ROUND_TRUNCATE (LineType * l);
extern LineType *bwb_OPTION_PUNCT_RPAREN (LineType * l);
extern LineType *bwb_OPTION_SCALE (LineType * l);
extern LineType *bwb_OPTION_PUNCT_SINGLE (LineType * l);
extern LineType *bwb_OPTION_SLEEP (LineType * l);
extern LineType *bwb_OPTION_PUNCT_STATEMENT (LineType * l);
extern LineType *bwb_OPTION_STDERR (LineType * l);
extern LineType *bwb_OPTION_STDIN (LineType * l);
extern LineType *bwb_OPTION_STDOUT (LineType * l);
extern LineType *bwb_OPTION_STRICT (LineType * l);
extern LineType *bwb_OPTION_STRICT_OFF (LineType * l);
extern LineType *bwb_OPTION_STRICT_ON (LineType * l);
extern LineType *bwb_OPTION_PUNCT_STRING (LineType * l);
extern LineType *bwb_OPTION_TERMINAL (LineType * l);
extern LineType *bwb_OPTION_TERMINAL_ADM (LineType * l);
extern LineType *bwb_OPTION_TERMINAL_ANSI (LineType * l);
extern LineType *bwb_OPTION_TERMINAL_NONE (LineType * l);
extern LineType *bwb_OPTION_TIME (LineType * l);
extern LineType *bwb_OPTION_TRACE (LineType * l);
extern LineType *bwb_OPTION_TRACE_OFF (LineType * l);
extern LineType *bwb_OPTION_TRACE_ON (LineType * l);
extern LineType *bwb_OPTION_USING (LineType * l);
extern LineType *bwb_OPTION_USING_ALL (LineType * l);
extern LineType *bwb_OPTION_USING_COMMA (LineType * l);
extern LineType *bwb_OPTION_USING_DIGIT (LineType * l);
extern LineType *bwb_OPTION_USING_DOLLAR (LineType * l);
extern LineType *bwb_OPTION_USING_EXRAD (LineType * l);
extern LineType *bwb_OPTION_USING_FILLER (LineType * l);
extern LineType *bwb_OPTION_USING_FIRST (LineType * l);
extern LineType *bwb_OPTION_USING_LENGTH (LineType * l);
extern LineType *bwb_OPTION_USING_LITERAL (LineType * l);
extern LineType *bwb_OPTION_USING_MINUS (LineType * l);
extern LineType *bwb_OPTION_USING_PERIOD (LineType * l);
extern LineType *bwb_OPTION_USING_PLUS (LineType * l);
extern LineType *bwb_OPTION_VERSION (LineType * l);
extern LineType *bwb_OPTION_ZONE (LineType * l);
extern LineType *bwb_PAUSE (LineType * l);
extern LineType *bwb_PDEL (LineType * l);
extern LineType *bwb_POP (LineType * l);
extern LineType *bwb_PRINT (LineType * l);
extern LineType *bwb_PTP (LineType * l);
extern LineType *bwb_PTR (LineType * l);
extern LineType *bwb_PUT (LineType * Line);
extern LineType *bwb_QUIT (LineType * l);
extern LineType *bwb_READ (LineType * Line);
extern LineType *bwb_RECALL (LineType * l);
extern LineType *bwb_REM (LineType * L);
extern LineType *bwb_RENAME (LineType * l);
extern LineType *bwb_RENUM (LineType * Line);
extern LineType *bwb_RENUMBER (LineType * Line);
extern LineType *bwb_REPEAT (LineType * l);
extern LineType *bwb_REPLACE (LineType * Line);
extern LineType *bwb_RESET (LineType * Line);
extern LineType *bwb_RESTORE (LineType * Line);
extern LineType *bwb_RESUME (LineType * l);
extern LineType *bwb_RETURN (LineType * l);
extern LineType *bwb_RSET (LineType * l);
extern LineType *bwb_RUN (LineType * L);
extern LineType *bwb_RUNNH (LineType * L);
extern LineType *bwb_SAVE (LineType * l);
extern LineType *bwb_SCRATCH (LineType * l);
extern LineType *bwb_SELECT (LineType * l);
extern LineType *bwb_SELECT_CASE (LineType * l);
extern LineType *bwb_STEP (LineType * L);
extern LineType *bwb_STOP (LineType * l);
extern LineType *bwb_STORE (LineType * l);
extern LineType *bwb_SUB (LineType * l);
extern LineType *bwb_SUBEND (LineType * l);
extern LineType *bwb_SUBEXIT (LineType * l);
extern LineType *bwb_SUB_END (LineType * l);
extern LineType *bwb_SUB_EXIT (LineType * l);
extern LineType *bwb_SWAP (LineType * l);
extern LineType *bwb_SYSTEM (LineType * l);
extern LineType *bwb_TEXT (LineType * l);
extern LineType *bwb_THEN (LineType * L);
extern LineType *bwb_TIMER (LineType * l);
extern LineType *bwb_TIMER_OFF (LineType * l);
extern LineType *bwb_TIMER_ON (LineType * l);
extern LineType *bwb_TIMER_STOP (LineType * l);
extern LineType *bwb_TLOAD (LineType * Line);
extern LineType *bwb_TO (LineType * L);
extern LineType *bwb_TRACE (LineType * l);
extern LineType *bwb_TRACE_OFF (LineType * l);
extern LineType *bwb_TRACE_ON (LineType * l);
extern LineType *bwb_TSAVE (LineType * Line);
extern LineType *bwb_TTY (LineType * l);
extern LineType *bwb_TTY_IN (LineType * l);
extern LineType *bwb_TTY_OUT (LineType * l);
extern LineType *bwb_UNTIL (LineType * l);
extern LineType *bwb_USE (LineType * l);
extern LineType *bwb_VARS (LineType * l);
extern LineType *bwb_vector (LineType * l);
extern LineType *bwb_WEND (LineType * l);
extern LineType *bwb_WHILE (LineType * l);
extern LineType *bwb_WRITE (LineType * l);
extern LineType *bwb_OPTION_PUNCT (LineType * l);
extern LineType *bwb_OPTION_EXTENSION (LineType * l);

/*-------------------------------------------------------------
                        INTERNALS
-------------------------------------------------------------*/
extern int binary_get_put (VariableType * Variable, int IsPUT);
extern int buff_is_eol (char *buffer, int *position);
extern int buff_peek_array_dimensions (char *buffer, int *position,
                                       int *n_params);
extern int buff_peek_char (char *buffer, int *position, char find);
extern int buff_peek_EqualChar (char *buffer, int *position);
extern int buff_peek_LparenChar (char *buffer, int *position);
extern int buff_peek_QuoteChar (char *buffer, int *position);
#if FALSE /* kepp this ... */
extern int buff_peek_word (char *buffer, int *position, char *find);
#endif
extern int buff_read_array_dimensions (char *buffer, int *position,
                                       int *n_params, int params[]);
extern int buff_read_array_redim (char *buffer, int *position,
                                  int *dimensions, int LBOUND[],
                                  int UBOUND[]);
extern ResultType buff_read_decimal_constant (char *buffer, int *position,
                                              VariantType * X,
                                              int IsConsoleInput);
extern int buff_read_expression (char *buffer, int *position,
                                 VariantType * X);
extern ResultType buff_read_hexadecimal_constant (char *buffer, int *position,
                                                  VariantType * X,
                                                  int IsConsoleInput);
extern int buff_read_index_item (char *buffer, int *position, int Index,
                                 int *Value);
extern int buff_read_integer_expression (char *buffer, int *position,
                                         int *Value);
extern int buff_read_label (char *buffer, int *position, char *label);
extern int buff_read_letter_sequence (char *buffer, int *position, char *head,
                                      char *tail);
extern int buff_read_line_number (char *buffer, int *position, int *linenum);
extern int buff_read_line_sequence (char *buffer, int *position, int *head,
                                    int *tail);
extern VariableType *buff_read_matrix (char *buffer, int *position);
extern int buff_read_numeric_expression (char *buffer, int *position,
                                         DoubleType * Value);
extern ResultType buff_read_octal_constant (char *buffer, int *position,
                                            VariantType * X,
                                            int IsConsoleInput);
extern VariableType *buff_read_scalar (char *buffer, int *position);
extern int buff_read_string_expression (char *buffer, int *position,
                                        char **Value);
extern char buff_read_type_declaration (char *buffer, int *position);
extern int buff_read_varname (char *buffer, int *position, char *varname);
extern int buff_skip_AtChar (char *buffer, int *position);
extern int buff_skip_char (char *buffer, int *position, char find);
extern int buff_skip_CommaChar (char *buffer, int *position);
extern void buff_skip_eol (char *buffer, int *position);
extern int buff_skip_EqualChar (char *buffer, int *position);
extern int buff_skip_FilenumChar (char *buffer, int *position);
extern int buff_skip_LparenChar (char *buffer, int *position);
extern int buff_skip_MinusChar (char *buffer, int *position);
extern int buff_skip_PlusChar (char *buffer, int *position);
extern int buff_skip_RparenChar (char *buffer, int *position);
extern int buff_skip_SemicolonChar (char *buffer, int *position);
extern char buff_skip_seperator (char *buffer, int *position);
extern void buff_skip_spaces (char *buffer, int *position);
extern int buff_skip_StarChar (char *buffer, int *position);
extern int buff_skip_word (char *buffer, int *position, char *find);
extern void bwb_clrexec (void);
extern void bwb_decexec (void);
extern void bwb_execline (void);
extern void bwb_fclose (FILE * file);
extern void bwb_file_open (char A, int x, char *B, int y);
extern int bwb_fload (char *FileName);
extern int bwb_freeline (LineType * l);
extern int bwb_incexec (void);
extern int bwb_isalnum (int C);
extern int bwb_isalpha (int C);
#if FALSE /* keep this ... */
extern int bwb_iscntrl (int C);
#endif
extern int bwb_isdigit (int C);
extern int bwb_isgraph (int C);
#if FALSE /* keep this ... */
extern int bwb_islower (int C);
#endif
extern int bwb_isprint (int C);
extern int bwb_ispunct (int C);
#if FALSE /* keep this ... */
extern int bwb_isspace (int C);
#endif
#if FALSE /* keep this ... */
extern int bwb_isupper (int C);
#endif
extern int bwb_isxdigit (int C);
extern int bwb_is_eof (FILE * fp);
extern void bwb_mainloop (void);
#if FALSE /* keep this ... */
extern void *bwb_memchr (const void *s, int c, size_t n);
#endif
extern int bwb_memcmp (const void *s1, const void *s2, size_t n);
extern void *bwb_memcpy (void *s1, const void *s2, size_t n);
#if FALSE /* keep this ... */
extern void *bwb_memmove (void *s1, const void *s2, size_t n);
#endif
extern void *bwb_memset (void *s, int c, size_t n);
extern LineType *bwb_option_punct_char (LineType * l, char *c);
extern LineType *bwb_option_range_integer (LineType * l, int *Integer,
                                           int MinVal, int MaxVal);
extern double bwb_rint (double x);
extern int bwb_scan (void);
extern char *bwb_strcat (char *s1, const char *s2);
extern char *bwb_strchr (const char *s, int c);
extern int bwb_strcmp (const char *s1, const char *s2);
extern char *bwb_strcpy (char *s1, const char *s2);
extern char *bwb_strdup (char *s);
extern char *bwb_strdup2 (char *s, char *t);
extern int bwb_stricmp (const char *s1, const char *s2);
extern size_t bwb_strlen (const char *s);
#if FALSE /* keep this ... */
extern char *bwb_strncat (char *s1, const char *s2, size_t n);
#endif
#if FALSE /* keep this ... */
extern int bwb_strncmp (const char *s1, const char *s2, size_t n);
#endif
extern char *bwb_strncpy (char *s1, const char *s2, size_t n);
extern int bwb_strnicmp (const char *s1, const char *s2, size_t n);
extern char *bwb_strrchr (const char *s, int c);
extern int bwb_tolower (int C);
extern int bwb_toupper (int C);
extern void bwb_xnew (LineType * l);
extern void bwx_CLS (void);
extern void bwx_COLOR (int Fore, int Back);
extern int bwx_Error (int ERR, char *ErrorMessage);
extern int bwx_input (char *prompt, int IsDisplayQuestionMark, char *answer,
                      int MaxLen);
extern void bwx_LOCATE (int Row, int Col);
extern void bwx_STOP (int IsShowMessage);
extern void bwx_terminate (void);
extern DoubleType bwx_TIMER (DoubleType Seconds);
extern char Char_to_TypeCode (char C);
extern void CleanLine (char *buffer);
extern void clear_virtual_by_file (int FileNumber);
extern void Determinant (VariableType * v);
extern void DumpAllCommandHtmlTable (FILE * file);
extern void DumpAllCommandSwitchStatement (FILE * file);
extern void DumpAllCommandSyntax (FILE * file, int IsXref,
                                  OptionVersionType OptionVersionValue);
extern void DumpAllCommandTableDefinitions (FILE * file);
extern void DumpAllCommandUniqueID (FILE * file);
extern void DumpAllFuctionTableDefinitions (FILE * file);
extern void DumpAllFunctionHtmlTable (FILE * file);
extern void DumpAllFunctionSwitch (FILE * file);
extern void DumpAllFunctionSyntax (FILE * file, int IsXref,
                                   OptionVersionType OptionVersionValue);
extern void DumpAllFunctionUniqueID (FILE * file);
extern void DumpAllOperatorSyntax (FILE * file, int IsXref,
                                   OptionVersionType OptionVersionValue);
extern void DumpHeader (FILE * file);
extern void DumpOneCommandSyntax (FILE * file, int IsXref, int n);
extern void DumpOneFunctionSyntax (FILE * file, int IsXref, int n);
extern void DumpOneOperatorSyntax (FILE * file, int IsXref, int n);
extern void field_close_file (FileType * File);
extern void field_free_variable (VariableType * Var);
extern void field_get (FileType * File);
extern void field_put (FileType * File);
extern void file_clear (FileType * F);
extern FileType *file_new (void);
extern int file_next_number (void);
extern FileType *find_file_by_name (char *FileName);
extern FileType *find_file_by_number (int FileNumber);
extern LineType *find_line_number (int number);
extern void FixDescription (FILE * file, const char *left, const char *right);
extern void FormatBasicNumber (DoubleType Input, char *Output);
extern int GetOnError (void);
extern void IntrinsicFunctionDefinitionCheck (IntrinsicFunctionType * f);
extern void IntrinsicFunctionSyntax (IntrinsicFunctionType * f, char *Syntax);
extern void IntrinsicFunctionUniqueID (IntrinsicFunctionType * f,
                                       char *UniqueID);
extern VariableType *IntrinsicFunction_deffn (int argc, VariableType * argv,
                                              UserFunctionType * f);
extern VariableType *IntrinsicFunction_execute (int argc, VariableType * argv,
                                                IntrinsicFunctionType * f);
extern IntrinsicFunctionType *IntrinsicFunction_find_exact (char *name,
                                                            int
                                                            ParameterCount,
                                                            ParamBitsType
                                                            ParameterTypes);
extern int IntrinsicFunction_init (void);
extern int IntrinsicFunction_name (char *name);
extern int InvertMatrix (VariableType * vOut, VariableType * vIn);
extern int IsLastKeyword (LineType * l, char *Keyword);
extern int IsLike (char *buffer, int *buffer_count, int buffer_Length,
                   char *pattern, int *pattern_count, int pattern_Length);
extern int is_empty_string (char *Buffer);
extern int line_is_eol (LineType * line);
#if FALSE /* kepp this ... */
extern int line_peek_array_dimensions (LineType * line, int *n_params);
#endif
#if FALSE /* kepp this ... */
extern int line_peek_char (LineType * line, char find);
#endif
extern int line_peek_EqualChar (LineType * line);
extern int line_peek_LparenChar (LineType * line);
extern int line_peek_QuoteChar (LineType * line);
#if FALSE /* kepp this ... */
extern int line_peek_word (LineType * line, char *find);
#endif
#if FALSE /* kepp this ... */
extern int line_read_array_dimensions (LineType * line, int *n_params,
                                       int params[]);
#endif
extern int line_read_array_redim (LineType * line, int *dimensions,
                                  int LBOUND[], int UBOUND[]);
extern int line_read_expression (LineType * line, VariantType * X);
extern int line_read_index_item (LineType * line, int Index, int *Value);
extern int line_read_integer_expression (LineType * line, int *Value);
extern int line_read_label (LineType * line, char *label);
extern int line_read_letter_sequence (LineType * line, char *head,
                                      char *tail);
extern int line_read_line_number (LineType * line, int *linenum);
extern int line_read_line_sequence (LineType * line, int *head, int *tail);
extern VariableType *line_read_matrix (LineType * line);
extern int line_read_numeric_expression (LineType * line, DoubleType * Value);
extern VariableType *line_read_scalar (LineType * line);
extern int line_read_string_expression (LineType * line, char **Value);
extern char line_read_type_declaration (LineType * line);
extern int line_read_varname (LineType * line, char *varname);
extern int line_skip_AtChar (LineType * line);
extern int line_skip_char (LineType * line, char find);
extern int line_skip_CommaChar (LineType * line);
extern void line_skip_eol (LineType * line);
extern int line_skip_EqualChar (LineType * line);
extern int line_skip_FilenumChar (LineType * line);
extern int line_skip_LparenChar (LineType * line);
extern int line_skip_MinusChar (LineType * line);
extern int line_skip_PlusChar (LineType * line);
extern int line_skip_RparenChar (LineType * line);
extern int line_skip_SemicolonChar (LineType * line);
extern char line_skip_seperator (LineType * line);
extern void line_skip_spaces (LineType * line);
extern int line_skip_StarChar (LineType * line);
extern int line_skip_word (LineType * line, char *find);
extern void line_start (LineType * l);
extern int main (int argc, char **argv);
extern VariableType *mat_find (char *name);
extern int NumberValueCheck (ParamTestType ParameterTests, DoubleType X);
extern void OptionVersionSet (int i);
extern void ResetConsoleColumn (void);
extern void SetOnError (int LineNumber);
extern void SortAllCommands (void);
extern void SortAllFunctions (void);
extern void SortAllOperators (void);
extern int StringLengthCheck (ParamTestType ParameterTests, int s);
extern int str_btob (StringType * d, StringType * s);
extern int str_cmp (StringType * a, StringType * b);
extern int str_match (char *A, int A_Length, char *B, int B_Length,
                      int I_Start);
extern char TypeCode_to_Char (char TypeCode);
extern int UserFunction_add (LineType * l);
extern int UserFunction_addlocalvar (UserFunctionType * f, VariableType * v);
extern UserFunctionType *UserFunction_find_exact (char *name,
                                                  unsigned char
                                                  ParameterCount,
                                                  ParamBitsType
                                                  ParameterTypes);
extern int UserFunction_init (void);
extern int UserFunction_name (char *name);
extern int VarTypeIndex (char C);
extern VariableType *var_chain (VariableType * argv);
extern void var_CLEAR (void);
extern int var_delcvars (void);
extern VariableType *var_find (char *name, int dimensions, int IsImplicit);
extern VariableType *var_free (VariableType * variable);
extern int var_get (VariableType * variable, VariantType * variant);
extern int var_init (void);
extern int var_make (VariableType * variable, char TypeCode);
extern char var_nametype (char *name);
extern VariableType *var_new (char *name, char TypeCode);
extern int var_set (VariableType * variable, VariantType * variant);


/*-------------------------------------------------------------
                        COMMANDS (CommandID)
-------------------------------------------------------------*/
#define C_DEF8LBL                       -1        /* DEF*LBL                        */
/* COMMANDS */
#define C_APPEND                         1 /* APPEND                         */
#define C_AS                             2 /* AS                             */
#define C_AUTO                           3 /* AUTO                           */
#define C_BACKSPACE                      4 /* BACKSPACE                      */
#define C_BREAK                          5 /* BREAK                          */
#define C_BUILD                          6 /* BUILD                          */
#define C_BYE                            7 /* BYE                            */
#define C_CALL                           8 /* CALL                           */
#define C_CASE                           9 /* CASE                           */
#define C_CASE_ELSE                     10 /* CASE ELSE                      */
#define C_CHAIN                         11 /* CHAIN                          */
#define C_CHANGE                        12 /* CHANGE                         */
#define C_CLEAR                         13 /* CLEAR                          */
#define C_CLOAD                         14 /* CLOAD                          */
#define C_CLOAD8                        15 /* CLOAD*                         */
#define C_CLOSE                         16 /* CLOSE                          */
#define C_CLR                           17 /* CLR                            */
#define C_CMDS                          18 /* CMDS                           */
#define C_COMMON                        19 /* COMMON                         */
#define C_CONSOLE                       20 /* CONSOLE                        */
#define C_CONST                         21 /* CONST                          */
#define C_CONT                          22 /* CONT                           */
#define C_CONTINUE                      23 /* CONTINUE                       */
#define C_COPY                          24 /* COPY                           */
#define C_CREATE                        25 /* CREATE                         */
#define C_CSAVE                         26 /* CSAVE                          */
#define C_CSAVE8                        27 /* CSAVE*                         */
#define C_DATA                          28 /* DATA                           */
#define C_DEC                           29 /* DEC                            */
#define C_DEF                           30 /* DEF                            */
#define C_DEFBYT                        31 /* DEFBYT                         */
#define C_DEFCUR                        32 /* DEFCUR                         */
#define C_DEFDBL                        33 /* DEFDBL                         */
#define C_DEFINT                        34 /* DEFINT                         */
#define C_DEFLNG                        35 /* DEFLNG                         */
#define C_DEFSNG                        36 /* DEFSNG                         */
#define C_DEFSTR                        37 /* DEFSTR                         */
#define C_DELETE                        38 /* DELETE                         */
#define C_DELIMIT                       39 /* DELIMIT                        */
#define C_DIM                           40 /* DIM                            */
#define C_DISPLAY                       41 /* DISPLAY                        */
#define C_DO                            42 /* DO                             */
#define C_DOS                           43 /* DOS                            */
#define C_DSP                           44 /* DSP                            */
#define C_EDIT                          45 /* EDIT                           */
#define C_ELSE                          46 /* ELSE                           */
#define C_ELSEIF                        47 /* ELSEIF                         */
#define C_END                           48 /* END                            */
#define C_END_FUNCTION                  49 /* END FUNCTION                   */
#define C_END_IF                        50 /* END IF                         */
#define C_END_SELECT                    51 /* END SELECT                     */
#define C_END_SUB                       52 /* END SUB                        */
#define C_ERASE                         53 /* ERASE                          */
#define C_EXCHANGE                      54 /* EXCHANGE                       */
#define C_EXIT                          55 /* EXIT                           */
#define C_EXIT_DO                       56 /* EXIT DO                        */
#define C_EXIT_FOR                      57 /* EXIT FOR                       */
#define C_EXIT_FUNCTION                 58 /* EXIT FUNCTION                  */
#define C_EXIT_REPEAT                   59 /* EXIT REPEAT                    */
#define C_EXIT_SUB                      60 /* EXIT SUB                       */
#define C_EXIT_WHILE                    61 /* EXIT WHILE                     */
#define C_FEND                          62 /* FEND                           */
#define C_FIELD                         63 /* FIELD                          */
#define C_FILE                          64 /* FILE                           */
#define C_FILES                         65 /* FILES                          */
#define C_FLEX                          66 /* FLEX                           */
#define C_FNCS                          67 /* FNCS                           */
#define C_FNEND                         68 /* FNEND                          */
#define C_FOR                           69 /* FOR                            */
#define C_FUNCTION                      70 /* FUNCTION                       */
#define C_GET                           71 /* GET                            */
#define C_GO                            72 /* GO                             */
#define C_GO_SUB                        73 /* GO SUB                         */
#define C_GO_TO                         74 /* GO TO                          */
#define C_GOODBYE                       75 /* GOODBYE                        */
#define C_GOSUB                         76 /* GOSUB                          */
#define C_GOTO                          77 /* GOTO                           */
#define C_HELP                          78 /* HELP                           */
#define C_IF                            79 /* IF                             */
#define C_IF_END                        80 /* IF END                         */
#define C_IF_MORE                       81 /* IF MORE                        */
#define C_IF8THEN                       82 /* IF*THEN                        */
#define C_IMAGE                         83 /* IMAGE                          */
#define C_INC                           84 /* INC                            */
#define C_INPUT                         85 /* INPUT                          */
#define C_INPUT_LINE                    86 /* INPUT LINE                     */
#define C_LET                           87 /* LET                            */
#define C_LINE                          88 /* LINE                           */
#define C_LINE_INPUT                    89 /* LINE INPUT                     */
#define C_LIST                          90 /* LIST                           */
#define C_LISTNH                        91 /* LISTNH                         */
#define C_LLIST                         92 /* LLIST                          */
#define C_LOAD                          93 /* LOAD                           */
#define C_LOCAL                         94 /* LOCAL                          */
#define C_LOOP                          95 /* LOOP                           */
#define C_LPRINT                        96 /* LPRINT                         */
#define C_LPRINTER                      97 /* LPRINTER                       */
#define C_LPT                           98 /* LPT                            */
#define C_LSET                          99 /* LSET                           */
#define C_MAINTAINER                   100 /* MAINTAINER                     */
#define C_MAINTAINER_CMDS              101 /* MAINTAINER CMDS                */
#define C_MAINTAINER_CMDS_HTML         102 /* MAINTAINER CMDS HTML           */
#define C_MAINTAINER_CMDS_ID           103 /* MAINTAINER CMDS ID             */
#define C_MAINTAINER_CMDS_MANUAL       104 /* MAINTAINER CMDS MANUAL         */
#define C_MAINTAINER_CMDS_SWITCH       105 /* MAINTAINER CMDS SWITCH         */
#define C_MAINTAINER_CMDS_TABLE        106 /* MAINTAINER CMDS TABLE          */
#define C_MAINTAINER_DEBUG             107 /* MAINTAINER DEBUG               */
#define C_MAINTAINER_DEBUG_OFF         108 /* MAINTAINER DEBUG OFF           */
#define C_MAINTAINER_DEBUG_ON          109 /* MAINTAINER DEBUG ON            */
#define C_MAINTAINER_FNCS              110 /* MAINTAINER FNCS                */
#define C_MAINTAINER_FNCS_HTML         111 /* MAINTAINER FNCS HTML           */
#define C_MAINTAINER_FNCS_ID           112 /* MAINTAINER FNCS ID             */
#define C_MAINTAINER_FNCS_MANUAL       113 /* MAINTAINER FNCS MANUAL         */
#define C_MAINTAINER_FNCS_SWITCH       114 /* MAINTAINER FNCS SWITCH         */
#define C_MAINTAINER_FNCS_TABLE        115 /* MAINTAINER FNCS TABLE          */
#define C_MAINTAINER_MANUAL            116 /* MAINTAINER MANUAL              */
#define C_MAINTAINER_STACK             117 /* MAINTAINER STACK               */
#define C_MARGIN                       118 /* MARGIN                         */
#define C_MAT                          119 /* MAT                            */
#define C_MAT_GET                      120 /* MAT GET                        */
#define C_MAT_INPUT                    121 /* MAT INPUT                      */
#define C_MAT_PRINT                    122 /* MAT PRINT                      */
#define C_MAT_PUT                      123 /* MAT PUT                        */
#define C_MAT_READ                     124 /* MAT READ                       */
#define C_MAT_WRITE                    125 /* MAT WRITE                      */
#define C_MERGE                        126 /* MERGE                          */
#define C_MID4                         127 /* MID$                           */
#define C_MON                          128 /* MON                            */
#define C_NAME                         129 /* NAME                           */
#define C_NEW                          130 /* NEW                            */
#define C_NEXT                         131 /* NEXT                           */
#define C_OF                           132 /* OF                             */
#define C_OLD                          133 /* OLD                            */
#define C_ON                           134 /* ON                             */
#define C_ON_ERROR                     135 /* ON ERROR                       */
#define C_ON_ERROR_GOSUB               136 /* ON ERROR GOSUB                 */
#define C_ON_ERROR_GOTO                137 /* ON ERROR GOTO                  */
#define C_ON_ERROR_RESUME              138 /* ON ERROR RESUME                */
#define C_ON_ERROR_RESUME_NEXT         139 /* ON ERROR RESUME NEXT           */
#define C_ON_ERROR_RETURN              140 /* ON ERROR RETURN                */
#define C_ON_ERROR_RETURN_NEXT         141 /* ON ERROR RETURN NEXT           */
#define C_ON_TIMER                     142 /* ON TIMER                       */
#define C_OPEN                         143 /* OPEN                           */
#define C_OPTION                       144 /* OPTION                         */
#define C_OPTION_ANGLE                 145 /* OPTION ANGLE                   */
#define C_OPTION_ANGLE_DEGREES         146 /* OPTION ANGLE DEGREES           */
#define C_OPTION_ANGLE_GRADIANS        147 /* OPTION ANGLE GRADIANS          */
#define C_OPTION_ANGLE_RADIANS         148 /* OPTION ANGLE RADIANS           */
#define C_OPTION_ARITHMETIC            149 /* OPTION ARITHMETIC              */
#define C_OPTION_ARITHMETIC_DECIMAL    150 /* OPTION ARITHMETIC DECIMAL      */
#define C_OPTION_ARITHMETIC_FIXED      151 /* OPTION ARITHMETIC FIXED        */
#define C_OPTION_ARITHMETIC_NATIVE     152 /* OPTION ARITHMETIC NATIVE       */
#define C_OPTION_BASE                  153 /* OPTION BASE                    */
#define C_OPTION_BUGS                  154 /* OPTION BUGS                    */
#define C_OPTION_BUGS_BOOLEAN          155 /* OPTION BUGS BOOLEAN            */
#define C_OPTION_BUGS_OFF              156 /* OPTION BUGS OFF                */
#define C_OPTION_BUGS_ON               157 /* OPTION BUGS ON                 */
#define C_OPTION_COMPARE               158 /* OPTION COMPARE                 */
#define C_OPTION_COMPARE_BINARY        159 /* OPTION COMPARE BINARY          */
#define C_OPTION_COMPARE_DATABASE      160 /* OPTION COMPARE DATABASE        */
#define C_OPTION_COMPARE_TEXT          161 /* OPTION COMPARE TEXT            */
#define C_OPTION_COVERAGE              162 /* OPTION COVERAGE                */
#define C_OPTION_COVERAGE_OFF          163 /* OPTION COVERAGE OFF            */
#define C_OPTION_COVERAGE_ON           164 /* OPTION COVERAGE ON             */
#define C_OPTION_DATE                  165 /* OPTION DATE                    */
#define C_OPTION_DIGITS                166 /* OPTION DIGITS                  */
#define C_OPTION_DISABLE               167 /* OPTION DISABLE                 */
#define C_OPTION_DISABLE_COMMAND       168 /* OPTION DISABLE COMMAND         */
#define C_OPTION_DISABLE_FUNCTION      169 /* OPTION DISABLE FUNCTION        */
#define C_OPTION_DISABLE_OPERATOR      170 /* OPTION DISABLE OPERATOR        */
#define C_OPTION_EDIT                  171 /* OPTION EDIT                    */
#define C_OPTION_ENABLE                172 /* OPTION ENABLE                  */
#define C_OPTION_ENABLE_COMMAND        173 /* OPTION ENABLE COMMAND          */
#define C_OPTION_ENABLE_FUNCTION       174 /* OPTION ENABLE FUNCTION         */
#define C_OPTION_ENABLE_OPERATOR       175 /* OPTION ENABLE OPERATOR         */
#define C_OPTION_ERROR                 176 /* OPTION ERROR                   */
#define C_OPTION_ERROR_GOSUB           177 /* OPTION ERROR GOSUB             */
#define C_OPTION_ERROR_GOTO            178 /* OPTION ERROR GOTO              */
#define C_OPTION_EXPLICIT              179 /* OPTION EXPLICIT                */
#define C_OPTION_EXTENSION             180 /* OPTION EXTENSION               */
#define C_OPTION_FILES                 181 /* OPTION FILES                   */
#define C_OPTION_IMPLICIT              182 /* OPTION IMPLICIT                */
#define C_OPTION_INDENT                183 /* OPTION INDENT                  */
#define C_OPTION_LABELS                184 /* OPTION LABELS                  */
#define C_OPTION_LABELS_OFF            185 /* OPTION LABELS OFF              */
#define C_OPTION_LABELS_ON             186 /* OPTION LABELS ON               */
#define C_OPTION_PROMPT                187 /* OPTION PROMPT                  */
#define C_OPTION_PUNCT                 188 /* OPTION PUNCT                   */
#define C_OPTION_PUNCT_AT              189 /* OPTION PUNCT AT                */
#define C_OPTION_PUNCT_BYTE            190 /* OPTION PUNCT BYTE              */
#define C_OPTION_PUNCT_COMMENT         191 /* OPTION PUNCT COMMENT           */
#define C_OPTION_PUNCT_CURRENCY        192 /* OPTION PUNCT CURRENCY          */
#define C_OPTION_PUNCT_DOUBLE          193 /* OPTION PUNCT DOUBLE            */
#define C_OPTION_PUNCT_FILENUM         194 /* OPTION PUNCT FILENUM           */
#define C_OPTION_PUNCT_IMAGE           195 /* OPTION PUNCT IMAGE             */
#define C_OPTION_PUNCT_INPUT           196 /* OPTION PUNCT INPUT             */
#define C_OPTION_PUNCT_INTEGER         197 /* OPTION PUNCT INTEGER           */
#define C_OPTION_PUNCT_LONG            198 /* OPTION PUNCT LONG              */
#define C_OPTION_PUNCT_LPAREN          199 /* OPTION PUNCT LPAREN            */
#define C_OPTION_PUNCT_PRINT           200 /* OPTION PUNCT PRINT             */
#define C_OPTION_PUNCT_QUOTE           201 /* OPTION PUNCT QUOTE             */
#define C_OPTION_PUNCT_RPAREN          202 /* OPTION PUNCT RPAREN            */
#define C_OPTION_PUNCT_SINGLE          203 /* OPTION PUNCT SINGLE            */
#define C_OPTION_PUNCT_STATEMENT       204 /* OPTION PUNCT STATEMENT         */
#define C_OPTION_PUNCT_STRING          205 /* OPTION PUNCT STRING            */
#define C_OPTION_RECLEN                206 /* OPTION RECLEN                  */
#define C_OPTION_RENUM                 207 /* OPTION RENUM                   */
#define C_OPTION_ROUND                 208 /* OPTION ROUND                   */
#define C_OPTION_ROUND_BANK            209 /* OPTION ROUND BANK              */
#define C_OPTION_ROUND_MATH            210 /* OPTION ROUND MATH              */
#define C_OPTION_ROUND_TRUNCATE        211 /* OPTION ROUND TRUNCATE          */
#define C_OPTION_SCALE                 212 /* OPTION SCALE                   */
#define C_OPTION_SLEEP                 213 /* OPTION SLEEP                   */
#define C_OPTION_STDERR                214 /* OPTION STDERR                  */
#define C_OPTION_STDIN                 215 /* OPTION STDIN                   */
#define C_OPTION_STDOUT                216 /* OPTION STDOUT                  */
#define C_OPTION_STRICT                217 /* OPTION STRICT                  */
#define C_OPTION_STRICT_OFF            218 /* OPTION STRICT OFF              */
#define C_OPTION_STRICT_ON             219 /* OPTION STRICT ON               */
#define C_OPTION_TERMINAL              220 /* OPTION TERMINAL                */
#define C_OPTION_TERMINAL_ADM          221 /* OPTION TERMINAL ADM            */
#define C_OPTION_TERMINAL_ANSI         222 /* OPTION TERMINAL ANSI           */
#define C_OPTION_TERMINAL_NONE         223 /* OPTION TERMINAL NONE           */
#define C_OPTION_TIME                  224 /* OPTION TIME                    */
#define C_OPTION_TRACE                 225 /* OPTION TRACE                   */
#define C_OPTION_TRACE_OFF             226 /* OPTION TRACE OFF               */
#define C_OPTION_TRACE_ON              227 /* OPTION TRACE ON                */
#define C_OPTION_USING                 228 /* OPTION USING                   */
#define C_OPTION_USING_ALL             229 /* OPTION USING ALL               */
#define C_OPTION_USING_COMMA           230 /* OPTION USING COMMA             */
#define C_OPTION_USING_DIGIT           231 /* OPTION USING DIGIT             */
#define C_OPTION_USING_DOLLAR          232 /* OPTION USING DOLLAR            */
#define C_OPTION_USING_EXRAD           233 /* OPTION USING EXRAD             */
#define C_OPTION_USING_FILLER          234 /* OPTION USING FILLER            */
#define C_OPTION_USING_FIRST           235 /* OPTION USING FIRST             */
#define C_OPTION_USING_LENGTH          236 /* OPTION USING LENGTH            */
#define C_OPTION_USING_LITERAL         237 /* OPTION USING LITERAL           */
#define C_OPTION_USING_MINUS           238 /* OPTION USING MINUS             */
#define C_OPTION_USING_PERIOD          239 /* OPTION USING PERIOD            */
#define C_OPTION_USING_PLUS            240 /* OPTION USING PLUS              */
#define C_OPTION_VERSION               241 /* OPTION VERSION                 */
#define C_OPTION_ZONE                  242 /* OPTION ZONE                    */
#define C_PAUSE                        243 /* PAUSE                          */
#define C_PDEL                         244 /* PDEL                           */
#define C_POP                          245 /* POP                            */
#define C_PRINT                        246 /* PRINT                          */
#define C_PTP                          247 /* PTP                            */
#define C_PTR                          248 /* PTR                            */
#define C_PUT                          249 /* PUT                            */
#define C_QUIT                         250 /* QUIT                           */
#define C_READ                         251 /* READ                           */
#define C_RECALL                       252 /* RECALL                         */
#define C_REM                          253 /* REM                            */
#define C_RENAME                       254 /* RENAME                         */
#define C_RENUM                        255 /* RENUM                          */
#define C_RENUMBER                     256 /* RENUMBER                       */
#define C_REPEAT                       257 /* REPEAT                         */
#define C_REPLACE                      258 /* REPLACE                        */
#define C_RESET                        259 /* RESET                          */
#define C_RESTORE                      260 /* RESTORE                        */
#define C_RESUME                       261 /* RESUME                         */
#define C_RETURN                       262 /* RETURN                         */
#define C_RSET                         263 /* RSET                           */
#define C_RUN                          264 /* RUN                            */
#define C_RUNNH                        265 /* RUNNH                          */
#define C_SAVE                         266 /* SAVE                           */
#define C_SCRATCH                      267 /* SCRATCH                        */
#define C_SELECT                       268 /* SELECT                         */
#define C_SELECT_CASE                  269 /* SELECT CASE                    */
#define C_STEP                         270 /* STEP                           */
#define C_STOP                         271 /* STOP                           */
#define C_STORE                        272 /* STORE                          */
#define C_SUB                          273 /* SUB                            */
#define C_SUB_END                      274 /* SUB END                        */
#define C_SUB_EXIT                     275 /* SUB EXIT                       */
#define C_SUBEND                       276 /* SUBEND                         */
#define C_SUBEXIT                      277 /* SUBEXIT                        */
#define C_SWAP                         278 /* SWAP                           */
#define C_SYSTEM                       279 /* SYSTEM                         */
#define C_TEXT                         280 /* TEXT                           */
#define C_THEN                         281 /* THEN                           */
#define C_TIMER                        282 /* TIMER                          */
#define C_TIMER_OFF                    283 /* TIMER OFF                      */
#define C_TIMER_ON                     284 /* TIMER ON                       */
#define C_TIMER_STOP                   285 /* TIMER STOP                     */
#define C_TLOAD                        286 /* TLOAD                          */
#define C_TO                           287 /* TO                             */
#define C_TRACE                        288 /* TRACE                          */
#define C_TRACE_OFF                    289 /* TRACE OFF                      */
#define C_TRACE_ON                     290 /* TRACE ON                       */
#define C_TSAVE                        291 /* TSAVE                          */
#define C_TTY                          292 /* TTY                            */
#define C_TTY_IN                       293 /* TTY IN                         */
#define C_TTY_OUT                      294 /* TTY OUT                        */
#define C_UNTIL                        295 /* UNTIL                          */
#define C_USE                          296 /* USE                            */
#define C_VARS                         297 /* VARS                           */
#define C_WEND                         298 /* WEND                           */
#define C_WHILE                        299 /* WHILE                          */
#define C_WRITE                        300 /* WRITE                          */


/*-------------------------------------------------------------
                        FUNCTIONS (FunctionID)
-------------------------------------------------------------*/

/* FUNCTIONS */
#define F_ABS_X_N                        1 /* N  = ABS( X )                  */
#define F_ACOS_X_N                       2 /* N  = ACOS( X )                 */
#define F_ACS_X_N                        3 /* N  = ACS( X )                  */
#define F_ACSD_X_N                       4 /* N  = ACSD( X )                 */
#define F_ACSG_X_N                       5 /* N  = ACSG( X )                 */
#define F_ANGLE_X_Y_N                    6 /* N  = ANGLE( X, Y )             */
#define F_ARCCOS_X_N                     7 /* N  = ARCCOS( X )               */
#define F_ARCSIN_X_N                     8 /* N  = ARCSIN( X )               */
#define F_ARCTAN_X_N                     9 /* N  = ARCTAN( X )               */
#define F_ARGC_N                        10 /* N  = ARGC                      */
#define F_ARGT4_X_S                     11 /* S$ = ARGT$( X )                */
#define F_ARGV_X_N                      12 /* N  = ARGV( X )                 */
#define F_ARGV4_X_S                     13 /* S$ = ARGV$( X )                */
#define F_ASC_A_N                       14 /* N  = ASC( A$ )                 */
#define F_ASC_A_X_N                     15 /* N  = ASC( A$, X )              */
#define F_ASCII_A_N                     16 /* N  = ASCII( A$ )               */
#define F_ASIN_X_N                      17 /* N  = ASIN( X )                 */
#define F_ASN_X_N                       18 /* N  = ASN( X )                  */
#define F_ASND_X_N                      19 /* N  = ASND( X )                 */
#define F_ASNG_X_N                      20 /* N  = ASNG( X )                 */
#define F_ATAN_X_N                      21 /* N  = ATAN( X )                 */
#define F_ATN_X_N                       22 /* N  = ATN( X )                  */
#define F_ATND_X_N                      23 /* N  = ATND( X )                 */
#define F_ATNG_X_N                      24 /* N  = ATNG( X )                 */
#define F_BASE_N                        25 /* N  = BASE                      */
#define F_BIN4_X_S                      26 /* S$ = BIN$( X )                 */
#define F_BIN4_X_Y_S                    27 /* S$ = BIN$( X, Y )              */
#define F_CATALOG_N                     28 /* N  = CATALOG                   */
#define F_CATALOG_A_N                   29 /* N  = CATALOG( A$ )             */
#define F_CCUR_X_N                      30 /* N  = CCUR( X )                 */
#define F_CDBL_X_N                      31 /* N  = CDBL( X )                 */
#define F_CEIL_X_N                      32 /* N  = CEIL( X )                 */
#define F_CHAR_X_Y_S                    33 /* S$ = CHAR( X, Y )              */
#define F_CHAR4_X_S                     34 /* S$ = CHAR$( X )                */
#define F_CHDIR_A_N                     35 /* N  = CHDIR( A$ )               */
#define F_CHR_X_S                       36 /* S$ = CHR( X )                  */
#define F_CHR4_X_S                      37 /* S$ = CHR$( X )                 */
#define F_CIN_X_N                       38 /* N  = CIN( X )                  */
#define F_CINT_X_N                      39 /* N  = CINT( X )                 */
#define F_CLG_X_N                       40 /* N  = CLG( X )                  */
#define F_CLK_X_N                       41 /* N  = CLK( X )                  */
#define F_CLK_X_S                       42 /* S$ = CLK( X )                  */
#define F_CLK4_S                        43 /* S$ = CLK$                      */
#define F_CLNG_X_N                      44 /* N  = CLNG( X )                 */
#define F_CLOG_X_N                      45 /* N  = CLOG( X )                 */
#define F_CLOSE_N                       46 /* N  = CLOSE                     */
#define F_CLOSE_X_N                     47 /* N  = CLOSE( X )                */
#define F_CLS_N                         48 /* N  = CLS                       */
#define F_CNTRL_X_Y_N                   49 /* N  = CNTRL( X, Y )             */
#define F_CODE_A_N                      50 /* N  = CODE( A$ )                */
#define F_COLOR_X_Y_N                   51 /* N  = COLOR( X, Y )             */
#define F_COMMAND4_S                    52 /* S$ = COMMAND$                  */
#define F_COMMAND4_X_S                  53 /* S$ = COMMAND$( X )             */
#define F_COS_X_N                       54 /* N  = COS( X )                  */
#define F_COSD_X_N                      55 /* N  = COSD( X )                 */
#define F_COSG_X_N                      56 /* N  = COSG( X )                 */
#define F_COSH_X_N                      57 /* N  = COSH( X )                 */
#define F_COT_X_N                       58 /* N  = COT( X )                  */
#define F_COUNT_N                       59 /* N  = COUNT                     */
#define F_CSC_X_N                       60 /* N  = CSC( X )                  */
#define F_CSH_X_N                       61 /* N  = CSH( X )                  */
#define F_CSNG_X_N                      62 /* N  = CSNG( X )                 */
#define F_CUR_X_Y_S                     63 /* S$ = CUR( X, Y )               */
#define F_CVC_A_N                       64 /* N  = CVC( A$ )                 */
#define F_CVD_A_N                       65 /* N  = CVD( A$ )                 */
#define F_CVI_A_N                       66 /* N  = CVI( A$ )                 */
#define F_CVL_A_N                       67 /* N  = CVL( A$ )                 */
#define F_CVS_A_N                       68 /* N  = CVS( A$ )                 */
#define F_DAT4_S                        69 /* S$ = DAT$                      */
#define F_DATE_N                        70 /* N  = DATE                      */
#define F_DATE4_S                       71 /* S$ = DATE$                     */
#define F_DATE4_X_S                     72 /* S$ = DATE$( X )                */
#define F_DEG_N                         73 /* N  = DEG                       */
#define F_DEG_X_N                       74 /* N  = DEG( X )                  */
#define F_DEGREE_N                      75 /* N  = DEGREE                    */
#define F_DEGREE_X_N                    76 /* N  = DEGREE( X )               */
#define F_DET_N                         77 /* N  = DET                       */
#define F_DIGITS_X_N                    78 /* N  = DIGITS( X )               */
#define F_DIGITS_X_Y_N                  79 /* N  = DIGITS( X, Y )            */
#define F_DIM_N                         80 /* N  = DIM( ... )                */
#define F_DPEEK_X_N                     81 /* N  = DPEEK( X )                */
#define F_DPOKE_X_Y_N                   82 /* N  = DPOKE( X, Y )             */
#define F_EDIT4_A_X_S                   83 /* S$ = EDIT$( A$, X )            */
#define F_ENVIRON_A_N                   84 /* N  = ENVIRON( A$ )             */
#define F_ENVIRON4_A_S                  85 /* S$ = ENVIRON$( A$ )            */
#define F_EOF_X_N                       86 /* N  = EOF( X )                  */
#define F_EPS_X_N                       87 /* N  = EPS( X )                  */
#define F_ERL_N                         88 /* N  = ERL                       */
#define F_ERR_N                         89 /* N  = ERR                       */
#define F_ERR4_S                        90 /* S$ = ERR$                      */
#define F_ERRL_N                        91 /* N  = ERRL                      */
#define F_ERRN_N                        92 /* N  = ERRN                      */
#define F_ERROR_X_N                     93 /* N  = ERROR( X )                */
#define F_ERROR_X_A_N                   94 /* N  = ERROR( X, A$ )            */
#define F_ERROR4_S                      95 /* S$ = ERROR$                    */
#define F_EXAM_X_N                      96 /* N  = EXAM( X )                 */
#define F_EXEC_A_N                      97 /* N  = EXEC( A$ )                */
#define F_EXF_N                         98 /* N  = EXF( ... )                */
#define F_EXP_X_N                       99 /* N  = EXP( X )                  */
#define F_FALSE_N                      100 /* N  = FALSE                     */
#define F_FETCH_X_N                    101 /* N  = FETCH( X )                */
#define F_FILEATTR_X_Y_N               102 /* N  = FILEATTR( X, Y )          */
#define F_FILES_N                      103 /* N  = FILES                     */
#define F_FILES_A_N                    104 /* N  = FILES( A$ )               */
#define F_FILL_X_Y_N                   105 /* N  = FILL( X, Y )              */
#define F_FIX_X_N                      106 /* N  = FIX( X )                  */
#define F_FLOAT_X_N                    107 /* N  = FLOAT( X )                */
#define F_FLOW_N                       108 /* N  = FLOW                      */
#define F_FP_X_N                       109 /* N  = FP( X )                   */
#define F_FRAC_X_N                     110 /* N  = FRAC( X )                 */
#define F_FRE_N                        111 /* N  = FRE                       */
#define F_FRE_A_N                      112 /* N  = FRE( A$ )                 */
#define F_FRE_X_N                      113 /* N  = FRE( X )                  */
#define F_FREE_N                       114 /* N  = FREE                      */
#define F_FREE_X_N                     115 /* N  = FREE( X )                 */
#define F_FREE_A_N                     116 /* N  = FREE( A$ )                */
#define F_FREEFILE_N                   117 /* N  = FREEFILE                  */
#define F_GET_X_N                      118 /* N  = GET( X )                  */
#define F_GET_X_Y_N                    119 /* N  = GET( X, Y )               */
#define F_GRAD_N                       120 /* N  = GRAD                      */
#define F_GRADIAN_N                    121 /* N  = GRADIAN                   */
#define F_HCS_X_N                      122 /* N  = HCS( X )                  */
#define F_HEX_A_N                      123 /* N  = HEX( A$ )                 */
#define F_HEX4_X_S                     124 /* S$ = HEX$( X )                 */
#define F_HEX4_X_Y_S                   125 /* S$ = HEX$( X, Y )              */
#define F_HOME_N                       126 /* N  = HOME                      */
#define F_HSN_X_N                      127 /* N  = HSN( X )                  */
#define F_HTN_X_N                      128 /* N  = HTN( X )                  */
#define F_INCH4_S                      129 /* S$ = INCH$                     */
#define F_INDEX_A_B_N                  130 /* N  = INDEX( A$, B$ )           */
#define F_INITIALIZE_N                 131 /* N  = INITIALIZE                */
#define F_INKEY4_S                     132 /* S$ = INKEY$                    */
#define F_INP_X_N                      133 /* N  = INP( X )                  */
#define F_INPUT4_X_S                   134 /* S$ = INPUT$( X )               */
#define F_INPUT4_X_Y_S                 135 /* S$ = INPUT$( X, Y )            */
#define F_INSTR_A_B_N                  136 /* N  = INSTR( A$, B$ )           */
#define F_INSTR_A_B_X_N                137 /* N  = INSTR( A$, B$, X )        */
#define F_INSTR_X_A_B_N                138 /* N  = INSTR( X, A$, B$ )        */
#define F_INT_X_N                      139 /* N  = INT( X )                  */
#define F_INT5_X_N                     140 /* N  = INT%( X )                 */
#define F_IP_X_N                       141 /* N  = IP( X )                   */
#define F_KEY_S                        142 /* S$ = KEY                       */
#define F_KEY4_S                       143 /* S$ = KEY$                      */
#define F_KILL_A_N                     144 /* N  = KILL( A$ )                */
#define F_LBOUND_N                     145 /* N  = LBOUND( ... )             */
#define F_LCASE4_A_S                   146 /* S$ = LCASE$( A$ )              */
#define F_LEFT_A_X_S                   147 /* S$ = LEFT( A$, X )             */
#define F_LEFT4_A_X_S                  148 /* S$ = LEFT$( A$, X )            */
#define F_LEN_A_N                      149 /* N  = LEN( A$ )                 */
#define F_LGT_X_N                      150 /* N  = LGT( X )                  */
#define F_LIN_X_S                      151 /* S$ = LIN( X )                  */
#define F_LN_X_N                       152 /* N  = LN( X )                   */
#define F_LNO_X_N                      153 /* N  = LNO( X )                  */
#define F_LOC_X_N                      154 /* N  = LOC( X )                  */
#define F_LOCATE_X_Y_N                 155 /* N  = LOCATE( X, Y )            */
#define F_LOCK_X_N                     156 /* N  = LOCK( X )                 */
#define F_LOF_X_N                      157 /* N  = LOF( X )                  */
#define F_LOG_X_N                      158 /* N  = LOG( X )                  */
#define F_LOG10_X_N                    159 /* N  = LOG10( X )                */
#define F_LOG2_X_N                     160 /* N  = LOG2( X )                 */
#define F_LOGE_X_N                     161 /* N  = LOGE( X )                 */
#define F_LOWER4_A_S                   162 /* S$ = LOWER$( A$ )              */
#define F_LPOS_N                       163 /* N  = LPOS                      */
#define F_LTRIM4_A_S                   164 /* S$ = LTRIM$( A$ )              */
#define F_LTW_X_N                      165 /* N  = LTW( X )                  */
#define F_LWIDTH_X_N                   166 /* N  = LWIDTH( X )               */
#define F_MATCH_A_B_X_N                167 /* N  = MATCH( A$, B$, X )        */
#define F_MAX_A_B_S                    168 /* S$ = MAX( A$, B$ )             */
#define F_MAX_X_Y_N                    169 /* N  = MAX( X, Y )               */
#define F_MAXBYT_N                     170 /* N  = MAXBYT                    */
#define F_MAXCUR_N                     171 /* N  = MAXCUR                    */
#define F_MAXDBL_N                     172 /* N  = MAXDBL                    */
#define F_MAXDEV_N                     173 /* N  = MAXDEV                    */
#define F_MAXINT_N                     174 /* N  = MAXINT                    */
#define F_MAXLEN_N                     175 /* N  = MAXLEN                    */
#define F_MAXLEN_A_N                   176 /* N  = MAXLEN( A$ )              */
#define F_MAXLNG_N                     177 /* N  = MAXLNG                    */
#define F_MAXLVL_N                     178 /* N  = MAXLVL                    */
#define F_MAXNUM_N                     179 /* N  = MAXNUM                    */
#define F_MAXSNG_N                     180 /* N  = MAXSNG                    */
#define F_MEM_N                        181 /* N  = MEM                       */
#define F_MID_A_X_S                    182 /* S$ = MID( A$, X )              */
#define F_MID_A_X_Y_S                  183 /* S$ = MID( A$, X, Y )           */
#define F_MID4_A_X_S                   184 /* S$ = MID$( A$, X )             */
#define F_MID4_A_X_Y_S                 185 /* S$ = MID$( A$, X, Y )          */
#define F_MIN_X_Y_N                    186 /* N  = MIN( X, Y )               */
#define F_MIN_A_B_S                    187 /* S$ = MIN( A$, B$ )             */
#define F_MINBYT_N                     188 /* N  = MINBYT                    */
#define F_MINCUR_N                     189 /* N  = MINCUR                    */
#define F_MINDBL_N                     190 /* N  = MINDBL                    */
#define F_MINDEV_N                     191 /* N  = MINDEV                    */
#define F_MININT_N                     192 /* N  = MININT                    */
#define F_MINLNG_N                     193 /* N  = MINLNG                    */
#define F_MINNUM_N                     194 /* N  = MINNUM                    */
#define F_MINSNG_N                     195 /* N  = MINSNG                    */
#define F_MKC4_X_S                     196 /* S$ = MKC$( X )                 */
#define F_MKD4_X_S                     197 /* S$ = MKD$( X )                 */
#define F_MKDIR_A_N                    198 /* N  = MKDIR( A$ )               */
#define F_MKI4_X_S                     199 /* S$ = MKI$( X )                 */
#define F_MKL4_X_S                     200 /* S$ = MKL$( X )                 */
#define F_MKS4_X_S                     201 /* S$ = MKS$( X )                 */
#define F_MOD_X_Y_N                    202 /* N  = MOD( X, Y )               */
#define F_NAME_A_B_N                   203 /* N  = NAME( A$, B$ )            */
#define F_NAME_N                       204 /* N  = NAME( ... )               */
#define F_NOFLOW_N                     205 /* N  = NOFLOW                    */
#define F_NOTRACE_N                    206 /* N  = NOTRACE                   */
#define F_NULL_X_N                     207 /* N  = NULL( X )                 */
#define F_NUM_N                        208 /* N  = NUM                       */
#define F_NUM_A_N                      209 /* N  = NUM( A$ )                 */
#define F_NUM4_X_S                     210 /* S$ = NUM$( X )                 */
#define F_OCT4_X_S                     211 /* S$ = OCT$( X )                 */
#define F_OCT4_X_Y_S                   212 /* S$ = OCT$( X, Y )              */
#define F_OPEN_A_X_B_N                 213 /* N  = OPEN( A$, X, B$ )         */
#define F_OPEN_A_X_B_Y_N               214 /* N  = OPEN( A$, X, B$, Y )      */
#define F_ORD_A_N                      215 /* N  = ORD( A$ )                 */
#define F_OUT_X_Y_N                    216 /* N  = OUT( X, Y )               */
#define F_PAD_X_N                      217 /* N  = PAD( X )                  */
#define F_PAUSE_X_N                    218 /* N  = PAUSE( X )                */
#define F_PDL_X_N                      219 /* N  = PDL( X )                  */
#define F_PEEK_X_N                     220 /* N  = PEEK( X )                 */
#define F_PI_N                         221 /* N  = PI                        */
#define F_PI_X_N                       222 /* N  = PI( X )                   */
#define F_PIN_X_N                      223 /* N  = PIN( X )                  */
#define F_POKE_X_Y_N                   224 /* N  = POKE( X, Y )              */
#define F_POS_N                        225 /* N  = POS                       */
#define F_POS_X_N                      226 /* N  = POS( X )                  */
#define F_POS_A_B_N                    227 /* N  = POS( A$, B$ )             */
#define F_POS_A_B_X_N                  228 /* N  = POS( A$, B$, X )          */
#define F_PRECISION_X_N                229 /* N  = PRECISION( X )            */
#define F_PTR_N                        230 /* N  = PTR( ... )                */
#define F_PUT_X_N                      231 /* N  = PUT( X )                  */
#define F_PUT_X_Y_N                    232 /* N  = PUT( X, Y )               */
#define F_RAD_N                        233 /* N  = RAD                       */
#define F_RAD_X_N                      234 /* N  = RAD( X )                  */
#define F_RADIAN_N                     235 /* N  = RADIAN                    */
#define F_RAN_N                        236 /* N  = RAN                       */
#define F_RAN_X_N                      237 /* N  = RAN( X )                  */
#define F_RANDOM_N                     238 /* N  = RANDOM                    */
#define F_RANDOM_X_N                   239 /* N  = RANDOM( X )               */
#define F_RANDOMIZE_N                  240 /* N  = RANDOMIZE                 */
#define F_RANDOMIZE_X_N                241 /* N  = RANDOMIZE( X )            */
#define F_REMAINDER_X_Y_N              242 /* N  = REMAINDER( X, Y )         */
#define F_RENAME_A_B_N                 243 /* N  = RENAME( A$, B$ )          */
#define F_REPEAT4_X_A_S                244 /* S$ = REPEAT$( X, A$ )          */
#define F_REPEAT4_X_Y_S                245 /* S$ = REPEAT$( X, Y )           */
#define F_RESET_N                      246 /* N  = RESET                     */
#define F_RESIDUE_N                    247 /* N  = RESIDUE                   */
#define F_RIGHT_A_X_S                  248 /* S$ = RIGHT( A$, X )            */
#define F_RIGHT4_A_X_S                 249 /* S$ = RIGHT$( A$, X )           */
#define F_RMDIR_A_N                    250 /* N  = RMDIR( A$ )               */
#define F_RND_N                        251 /* N  = RND                       */
#define F_RND_X_N                      252 /* N  = RND( X )                  */
#define F_ROUND_X_Y_N                  253 /* N  = ROUND( X, Y )             */
#define F_RTRIM4_A_S                   254 /* S$ = RTRIM$( A$ )              */
#define F_SCALE_X_N                    255 /* N  = SCALE( X )                */
#define F_SEC_X_N                      256 /* N  = SEC( X )                  */
#define F_SEEK_X_N                     257 /* N  = SEEK( X )                 */
#define F_SEEK_X_Y_N                   258 /* N  = SEEK( X, Y )              */
#define F_SEG_X_N                      259 /* N  = SEG( X )                  */
#define F_SEG_A_X_Y_S                  260 /* S$ = SEG( A$, X, Y )           */
#define F_SEG4_A_X_Y_S                 261 /* S$ = SEG$( A$, X, Y )          */
#define F_SGN_X_N                      262 /* N  = SGN( X )                  */
#define F_SHELL_A_N                    263 /* N  = SHELL( A$ )               */
#define F_SIN_X_N                      264 /* N  = SIN( X )                  */
#define F_SIND_X_N                     265 /* N  = SIND( X )                 */
#define F_SING_X_N                     266 /* N  = SING( X )                 */
#define F_SINH_X_N                     267 /* N  = SINH( X )                 */
#define F_SIZE_A_N                     268 /* N  = SIZE( A$ )                */
#define F_SIZE_N                       269 /* N  = SIZE( ... )               */
#define F_SLEEP_X_N                    270 /* N  = SLEEP( X )                */
#define F_SNH_X_N                      271 /* N  = SNH( X )                  */
#define F_SPA_X_S                      272 /* S$ = SPA( X )                  */
#define F_SPACE_X_S                    273 /* S$ = SPACE( X )                */
#define F_SPACE4_X_S                   274 /* S$ = SPACE$( X )               */
#define F_SPC_X_S                      275 /* S$ = SPC( X )                  */
#define F_SQR_X_N                      276 /* N  = SQR( X )                  */
#define F_SQRT_X_N                     277 /* N  = SQRT( X )                 */
#define F_STR_X_Y_S                    278 /* S$ = STR( X, Y )               */
#define F_STR4_X_S                     279 /* S$ = STR$( X )                 */
#define F_STRING_X_Y_S                 280 /* S$ = STRING( X, Y )            */
#define F_STRING4_X_A_S                281 /* S$ = STRING$( X, A$ )          */
#define F_STRING4_X_Y_S                282 /* S$ = STRING$( X, Y )           */
#define F_STRIP4_A_S                   283 /* S$ = STRIP$( A$ )              */
#define F_STUFF_X_Y_N                  284 /* N  = STUFF( X, Y )             */
#define F_TAB_X_S                      285 /* S$ = TAB( X )                  */
#define F_TAN_X_N                      286 /* N  = TAN( X )                  */
#define F_TAND_X_N                     287 /* N  = TAND( X )                 */
#define F_TANG_X_N                     288 /* N  = TANG( X )                 */
#define F_TANH_X_N                     289 /* N  = TANH( X )                 */
#define F_TI_N                         290 /* N  = TI                        */
#define F_TI4_S                        291 /* S$ = TI$                       */
#define F_TIM_N                        292 /* N  = TIM                       */
#define F_TIM_X_N                      293 /* N  = TIM( X )                  */
#define F_TIME_N                       294 /* N  = TIME                      */
#define F_TIME_X_N                     295 /* N  = TIME( X )                 */
#define F_TIME4_S                      296 /* S$ = TIME$                     */
#define F_TIME4_X_S                    297 /* S$ = TIME$( X )                */
#define F_TIMER_N                      298 /* N  = TIMER                     */
#define F_TOP_N                        299 /* N  = TOP                       */
#define F_TRACE_N                      300 /* N  = TRACE                     */
#define F_TRACE_X_N                    301 /* N  = TRACE( X )                */
#define F_TRIM4_A_S                    302 /* S$ = TRIM$( A$ )               */
#define F_TROFF_N                      303 /* N  = TROFF                     */
#define F_TRON_N                       304 /* N  = TRON                      */
#define F_TRUE_N                       305 /* N  = TRUE                      */
#define F_TRUNCATE_X_Y_N               306 /* N  = TRUNCATE( X, Y )          */
#define F_UBOUND_N                     307 /* N  = UBOUND( ... )             */
#define F_UCASE4_A_S                   308 /* S$ = UCASE$( A$ )              */
#define F_UNLOCK_X_N                   309 /* N  = UNLOCK( X )               */
#define F_UNSAVE_A_N                   310 /* N  = UNSAVE( A$ )              */
#define F_UPPER4_A_S                   311 /* S$ = UPPER$( A$ )              */
#define F_USR_N                        312 /* N  = USR( ... )                */
#define F_USR0_N                       313 /* N  = USR0( ... )               */
#define F_USR1_N                       314 /* N  = USR1( ... )               */
#define F_USR2_N                       315 /* N  = USR2( ... )               */
#define F_USR3_N                       316 /* N  = USR3( ... )               */
#define F_USR4_N                       317 /* N  = USR4( ... )               */
#define F_USR5_N                       318 /* N  = USR5( ... )               */
#define F_USR6_N                       319 /* N  = USR6( ... )               */
#define F_USR7_N                       320 /* N  = USR7( ... )               */
#define F_USR8_N                       321 /* N  = USR8( ... )               */
#define F_USR9_N                       322 /* N  = USR9( ... )               */
#define F_UUF_N                        323 /* N  = UUF( ... )                */
#define F_VAL_A_N                      324 /* N  = VAL( A$ )                 */
#define F_VARPTR_N                     325 /* N  = VARPTR( ... )             */
#define F_VTAB_X_N                     326 /* N  = VTAB( X )                 */
#define F_WAIT_X_N                     327 /* N  = WAIT( X )                 */
#define F_WAIT_X_Y_N                   328 /* N  = WAIT( X, Y )              */
#define F_WAIT_X_Y_Z_N                 329 /* N  = WAIT( X, Y, Z )           */
#define F_WIDTH_X_N                    330 /* N  = WIDTH( X )                */
#define F_WIDTH_X_Y_N                  331 /* N  = WIDTH( X, Y )             */
#define F_ZONE_X_N                     332 /* N  = ZONE( X )                 */
#define F_ZONE_X_Y_N                   333 /* N  = ZONE( X, Y )              */

/*-------------------------------------------------------------
                        WARNINGS
-------------------------------------------------------------*/
#define WARN_CLEAR                           bwx_Error(  0, NULL )
#define WARN_NEXT_WITHOUT_FOR                bwx_Error(  1, NULL )
#define WARN_SYNTAX_ERROR                    bwx_Error(  2, NULL )
#define WARN_RETURN_WITHOUT_GOSUB            bwx_Error(  3, NULL )
#define WARN_OUT_OF_DATA                     bwx_Error(  4, NULL )
#define WARN_ILLEGAL_FUNCTION_CALL           bwx_Error(  5, NULL )
#define WARN_OVERFLOW                        bwx_Error(  6, NULL )
#define WARN_OUT_OF_MEMORY                   bwx_Error(  7, NULL )
#define WARN_UNDEFINED_LINE                  bwx_Error(  8, NULL )
#define WARN_SUBSCRIPT_OUT_OF_RANGE          bwx_Error(  9, NULL )
#define WARN_REDIMENSION_ARRAY               bwx_Error( 10, NULL )
#define WARN_DIVISION_BY_ZERO                bwx_Error( 11, NULL )
#define WARN_ILLEGAL_DIRECT                  bwx_Error( 12, NULL )
#define WARN_TYPE_MISMATCH                   bwx_Error( 13, NULL )
#define WARN_OUT_OF_STRING_SPACE             bwx_Error( 14, NULL )
#define WARN_STRING_TOO_LONG                 bwx_Error( 15, NULL )
#define WARN_STRING_FORMULA_TOO_COMPLEX      bwx_Error( 16, NULL )
#define WARN_CANT_CONTINUE                   bwx_Error( 17, NULL )
#define WARN_UNDEFINED_USER_FUNCTION         bwx_Error( 18, NULL )
#define WARN_NO_RESUME                       bwx_Error( 19, NULL )
#define WARN_RESUME_WITHOUT_ERROR            bwx_Error( 20, NULL )
#define WARN_UNPRINTABLE_ERROR               bwx_Error( 21, NULL )
#define WARN_MISSING_OPERAND                 bwx_Error( 22, NULL )
#define WARN_LINE_BUFFER_OVERFLOW            bwx_Error( 23, NULL )
#define WARN_FOR_WITHOUT_NEXT                bwx_Error( 26, NULL )
#define WARN_BAD_DATA                        bwx_Error( 27, NULL )        /* NEW in bwBASIC 3.20 */
#define WARN_UNASSIGNED_28                   bwx_Error( 28, NULL )
#define WARN_WHILE_WITHOUT_WEND              bwx_Error( 29, NULL )
#define WARN_WEND_WITHOUT_WHILE              bwx_Error( 30, NULL )
#define WARN_EXIT_FUNCTION_WITHOUT_FUNCTION  bwx_Error( 31, NULL )
#define WARN_END_FUNCTION_WITHOUT_FUNCTION   bwx_Error( 32, NULL )
#define WARN_EXIT_SUB_WITHOUT_SUB            bwx_Error( 33, NULL )
#define WARN_END_SUB_WITHOUT_SUB             bwx_Error( 34, NULL )
#define WARN_EXIT_FOR_WITHOUT_FOR            bwx_Error( 35, NULL )
#define WARN_FIELD_OVERFLOW                  bwx_Error( 50, NULL )
#define WARN_INTERNAL_ERROR                  bwx_Error( 51, NULL )
#define WARN_BAD_FILE_NUMBER                 bwx_Error( 52, NULL )
#define WARN_FILE_NOT_FOUND                  bwx_Error( 53, NULL )
#define WARN_BAD_FILE_MODE                   bwx_Error( 54, NULL )
#define WARN_FILE_ALREADY_OPEN               bwx_Error( 55, NULL )
#define WARN_UNASSIGNED_56                   bwx_Error( 56, NULL )
#define WARN_DISK_IO_ERROR                   bwx_Error( 57, NULL )
#define WARN_FILE_ALREADY_EXISTS             bwx_Error( 58, NULL )
#define WARN_UNASSIGNED_59                   bwx_Error( 59, NULL )
#define WARN_UNASSIGNED_60                   bwx_Error( 60, NULL )
#define WARN_DISK_FULL                       bwx_Error( 61, NULL )
#define WARN_INPUT_PAST_END                  bwx_Error( 62, NULL )
#define WARN_BAD_RECORD_NUMBER               bwx_Error( 63, NULL )
#define WARN_BAD_FILE_NAME                   bwx_Error( 64, NULL )
#define WARN_UNASSIGNED_65                   bwx_Error( 65, NULL )
#define WARN_DIRECT_STATEMENT_IN_FILE        bwx_Error( 66, NULL )
#define WARN_TOO_MANY_FILES                  bwx_Error( 67, NULL )
#define WARN_UNASSIGNED_68                   bwx_Error( 68, NULL )
#define WARN_VARIABLE_NOT_DECLARED           bwx_Error( 70, NULL )
#define WARN_ADVANCED_FEATURE                bwx_Error( 73, NULL )




/* EOF */
