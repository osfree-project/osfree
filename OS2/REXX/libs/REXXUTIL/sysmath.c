/**************************************************************************
*
*             eCS RexxUtil Library Replacement Project
*
*  Contains the following functions:
*      SysPi
*      SysSqrt
*      SysExp
*      SysLog
*      SysLog10
*      SysSinH
*      SysCosH
*      SysTanH
*      SysPower
*      SysSin
*      SysCos
*      SysTan
*      SysCotan
*      SysArcSin
*      SysArcCos
*      SysArcTan
*
*  This is file contains the math functions found in the OOREXX project as
*  of 14 Jan 2008. I have done little to no formating changes.
*
*  Michael Greene, January 2008
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define  INCL_BASE
#include <os2.h>

#include <rexxdefs.h>  // rexxsaa.h include in this header


/*********************************************************************/
/*  Various definitions used by the math functions                   */
/*********************************************************************/
#define SINE        0                  /* trig function defines...   */
#define COSINE      3                  /* the ordering is important, */
#define TANGENT     1                  /* as these get transformed   */
#define COTANGENT   2                  /* depending on the angle     */
#define MAXTRIG     3                  /* value                      */
#define ARCSINE     0                  /* defines for arc trig       */
#define ARCCOSINE   1                  /* functions.  Ordering is    */
#define ARCTANGENT  2                  /* not as important here      */

#define pi  3.14159265358979323846l    /* pi value                   */

#define DEGREES    'D'                 /* degrees option             */
#define RADIANS    'R'                 /* radians option             */
#define GRADES     'G'                 /* grades option              */

#define DEFAULT_PRECISION  9           /* default precision to use   */
#define MAX_PRECISION     16           /* maximum available precision*/


static void FormatResult(double result, unsigned long precision,
                          PRXSTRING retstr );
static long  ValidateMath(long numargs, RXSTRING  args[],
                          double   *x, unsigned long *precision );
static long ValidateTrig(long numargs, RXSTRING  args[],
                          PRXSTRING retstr, int function );
static long ValidateArcTrig(long numargs, RXSTRING   args[],
                          PRXSTRING  retstr, int function );


RexxFunctionHandler SysPi;
RexxFunctionHandler SysSqrt;
RexxFunctionHandler SysExp;
RexxFunctionHandler SysLog;
RexxFunctionHandler SysLog10;
RexxFunctionHandler SysSinh;
RexxFunctionHandler SysCosh;
RexxFunctionHandler SysTanh;
RexxFunctionHandler SysPower;
RexxFunctionHandler SysSin;
RexxFunctionHandler SysCos;
RexxFunctionHandler SysTan;
RexxFunctionHandler SysCotan;
RexxFunctionHandler SysArcSin;
RexxFunctionHandler SysArcCos;
RexxFunctionHandler SysArcTan;


/********************************************************************/
/* Functions:           SysSqrt(), SysExp(), SysLog(), SysLog10,    */
/* Functions:           SysSinH(), SysCosH(), SysTanH()             */
/* Description:         Returns function value of argument.         */
/* Input:               One number.                                 */
/* Output:              Value of the function requested for arg.    */
/*                      Returns 0 if the function executed OK,      */
/*                      40 otherwise.  The interpreter will fail    */
/*                      if the function returns a negative result.  */
/* Notes:                                                           */
/*   These routines take one to two parameters.                     */
/*   The form of the call is:                                       */
/*   result = func_name(x <, prec> <,angle>)                        */
/*                                                                  */
/********************************************************************/

unsigned long SysSqrt(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* function return code       */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(sqrt(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/

unsigned long SysExp(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(exp(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/

unsigned long SysLog(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(log(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/

unsigned long SysLog10(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(log10(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/

unsigned long SysSinH(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(sinh(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/

unsigned long SysCosH(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(cosh(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/

unsigned long SysTanH(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(tanh(x), precision, retstr);
  return rc;                           /* return error code          */
}


/********************************************************************/
/* Functions:           SysPower()                                  */
/* Description:         Returns function value of arguments.        */
/* Input:               Two numbers.                                */
/* Output:              Value of the x to the power y.              */
/*                      Returns 0 if the function executed OK,      */
/*                      -1 otherwise.  The interpreter will fail    */
/*                      if the function returns a negative result.  */
/* Notes:                                                           */
/*   This routine takes two to three parameters.                    */
/*   The form of the call is:                                       */
/*   result = func_name(x, y <, prec>)                              */
/*                                                                  */
/********************************************************************/

unsigned long SysPower(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
  double    x;                         /* input number               */
  double    y;                         /* second input number        */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation code            */

  rc = VALID_ROUTINE;                  /* set default completion     */
  precision = DEFAULT_PRECISION;       /* set max digits count       */

  if (numargs < 2 ||                   /* no arguments               */
      numargs > 3 ||
      !RXVALIDSTRING(args[0]) ||       /* first is omitted           */
      !RXVALIDSTRING(args[1]))         /* second is omitted          */
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs == 3 &&             /* have a precision           */
      !string2ulong(args[2].strptr, &precision))
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(args[0].strptr, " %lf", &x) != 1)
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* convert second input       */
  else if (sscanf(args[1].strptr, " %lf", &y) != 1)
    rc = INVALID_ROUTINE;              /* this is invalid            */
  if (!rc) {                           /* good function call         */
                                       /* keep to maximum            */
    precision = min(precision, MAX_PRECISION);
                                       /* format the result          */
    FormatResult(pow(x,y), precision, retstr);
  }
  return rc;                           /* return error code          */
}


/********************************************************************/
/* Functions:           RxSin(), RxCos(), RxTan(), RxCotan()        */
/* Description:         Returns trigonometric angle value.          */
/* Input:               Angle in radian or degree or grade          */
/* Output:              Trigonometric function value for Angle.     */
/*                      Returns 0 if the function executed OK,      */
/*                      -1 otherwise.  The interpreter will fail    */
/*                      if the function returns a negative result.  */
/* Notes:                                                           */
/*   These routines take one to three parameters.                   */
/*   The form of the call is:                                       */
/*   x = func_name(angle <, prec> <, [R | D | G]>)                  */
/*                                                                  */
/********************************************************************/

unsigned long SysSin(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
                                       /* call common routine        */
  return ValidateTrig(numargs, args, retstr, SINE);
}

/*==================================================================*/

unsigned long SysCos(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
                                       /* call common routine        */
  return ValidateTrig(numargs, args, retstr, COSINE);
}

/*==================================================================*/

unsigned long SysTan(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
                                       /* call common routine        */
  return ValidateTrig(numargs, args, retstr, TANGENT);
}

/*==================================================================*/

unsigned long SysCotan(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
                                       /* call common routine        */
  return ValidateTrig(numargs, args, retstr, COTANGENT);
}


/********************************************************************/
/* Functions:           SysPi()                                     */
/* Description:         Returns value of pi for given precision     */
/* Input:               Precision.   Default is 9                   */
/* Output:              Value of the pi to given precision          */
/* Notes:                                                           */
/*   This routine takes one parameters.                             */
/*   The form of the call is:                                       */
/*   result = syspi(<precision>)                                    */
/*                                                                  */
/********************************************************************/

unsigned long SysPi(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
  ULONG     precision;                 /* required precision         */

  precision = DEFAULT_PRECISION;       /* set default precision      */
  if (numargs > 1 ||                   /* too many arguments?        */
      (numargs == 1 &&                 /* bad precision?             */
      !string2ulong(args[0].strptr, &precision)))
    return INVALID_ROUTINE;            /* bad routine                */
                                       /* keep to maximum            */
  precision = min(precision, MAX_PRECISION);
                                       /* format the result          */
  FormatResult(pi, precision, retstr); /* format the result          */
  return VALID_ROUTINE;                /* good result                */
}


/********************************************************************/
/* Functions:           SysArcSin(), SysArcCos(), SysArcTan()       */
/* Description:         Returns angle from trigonometric value.     */
/* Input:               Angle in radian or degree or grade          */
/* Output:              Angle for matching trigonometric value.     */
/*                      Returns 0 if the function executed OK,      */
/*                      -1 otherwise.  The interpreter will fail    */
/*                      if the function returns a negative result.  */
/* Notes:                                                           */
/*   These routines take one to three parameters.                   */
/*   The form of the call is:                                       */
/*   a = func_name(arg <, prec> <, [R | D | G]>)                    */
/*                                                                  */
/********************************************************************/

unsigned long SysArcSin(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
                                       /* call common routine        */
  return ValidateArcTrig(numargs, args, retstr, ARCSINE);
}

/*==================================================================*/

unsigned long SysArcCos(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
                                       /* call common routine        */
  return ValidateArcTrig(numargs, args, retstr, ARCCOSINE);
}

/*==================================================================*/

unsigned long SysArcTan(unsigned char *name,
                           unsigned long numargs,
                           RXSTRING args[],
                           char *queuename,
                           RXSTRING *retstr)
{
                                       /* call common routine        */
  return ValidateArcTrig(numargs, args, retstr, ARCTANGENT);
}





/*********************************************************************/
/* Function FormatFloat:  Common routine to format a floating point  */
/* result for the math functions                                     */
/*********************************************************************/
/* formatted result  required precision  return string              */

static void FormatResult(double result, unsigned long precision, PRXSTRING retstr )
{
  if (result == 0)                     /* zero result?               */
    strcpy(retstr->strptr, "0");       /* make exactly 0             */
  else
                                       /* format the result          */
    _gcvt(result, precision, retstr->strptr);
                                       /* set the length             */
  retstr->strlength = strlen(retstr->strptr);
                                       /* end in a period?           */
  if (retstr->strptr[retstr->strlength - 1] == '.')
    retstr->strlength--;               /* remove the period          */
}


/*********************************************************************/
/* Function ValidateMath: Common validation routine for math         */
/* that are of the form fn(number, <precision>)                      */
/*********************************************************************/
/* Number of arguments.  Function arguments.  input number  returned precision    */

static long  ValidateMath(long numargs,
                          RXSTRING  args[],
                          double   *x,
                          unsigned long *precision )
{
  LONG      rc;                        /* validation code            */

  rc = VALID_ROUTINE;                  /* set default completion     */
  *precision = DEFAULT_PRECISION;      /* set max digits count       */

  if (numargs < 1 ||                   /* no arguments               */
      numargs > 2 ||
      !RXVALIDSTRING(args[0]))         /* first is omitted           */
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs == 2 &&             /* have a precision           */
      !string2ulong(args[1].strptr, precision))
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(args[0].strptr, " %lf", x) != 1)
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* truncate to maximum        */
  *precision = min(*precision, MAX_PRECISION);
  return rc;                           /* return success code        */
}

/*********************************************************************/
/* Function ValidateTrig: Common validation routine for math         */
/* that are of the form fn(number, <precision>, <unit>)              */
/*********************************************************************/
static long ValidateTrig(long numargs,/* Number of arguments.       */
                   RXSTRING  args[],  /* Function arguments.        */
                   PRXSTRING retstr,  /* return string              */
                   int function )     /* function to perform        */
{
  LONG      rc;                        /* validation code            */
  INT       units;                     /* angle type                 */
  double    angle;                     /* working angle              */
  double    nsi;                       /* convertion factor          */
  double    nco;                       /* convertion factor          */
  ULONG     precision;                 /* returned precision         */
  double    result;                    /* result                     */

  rc = VALID_ROUTINE;                  /* set default completion     */
  precision = DEFAULT_PRECISION;       /* set max digits count       */
  units = DEGREES;                     /* default angle is degrees   */
  nsi = 1.;                            /* set default conversion     */
  nco = 1.;                            /* set default conversion     */

  if (numargs < 1 ||                   /* no arguments               */
      numargs > 3 ||
      !RXVALIDSTRING(args[0]))         /* first is omitted           */
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs >= 2 &&             /* have a precision           */
      RXVALIDSTRING(args[1]) &&        /* and it is real string      */
      !string2ulong(args[1].strptr, &precision))
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(args[0].strptr, " %lf", &angle) != 1)
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs == 3) {             /* have an option             */
    if (RXZEROLENSTRING(args[2]))      /* null string?               */
      rc = INVALID_ROUTINE;            /* this is invalid            */
    else {                             /* process the options        */
                                       /* get the option character   */
      units = toupper(args[2].strptr[0]);
                                       /* was it a good option?      */
      if (units != DEGREES && units != RADIANS && units != GRADES)
        rc = INVALID_ROUTINE;          /* bad option is error        */
    }
  }
  if (!rc) {                           /* everything went well?      */
                                       /* truncate to maximum        */
    precision = min(precision, MAX_PRECISION);
    if (units == DEGREES) {            /* need to convert degrees    */
      nsi = (angle < 0.) ? -1. : 1.;   /* get the direction          */
      angle = fmod(fabs(angle), 360.); /* make modulo 360            */
      if (angle <= 45.)                /* less than 45?              */
        angle = angle * pi / 180.;
      else if (angle < 135.) {         /* over on the other side?    */
        angle = (90. - angle) * pi / 180.;
        function = MAXTRIG - function; /* change the function        */
        nco = nsi;                     /* swap around the conversions*/
        nsi = 1.;
      }
      else if (angle <= 225.) {        /* around the other way?      */
        angle = (180. - angle) * pi / 180.;
        nco = -1.;
      }
      else if (angle < 315.) {         /* close to the origin?       */
        angle = (angle - 270.) * pi / 180.;
        function = MAXTRIG - function; /* change the function        */
        nco = -nsi;
        nsi = 1.;
      }
      else
        angle = (angle - 360.) * pi / 180.;
    }
    else if (units == GRADES) {        /* need to convert degrees    */
      nsi = (angle < 0.) ? -1. : 1.;   /* get the direction          */
      angle = fmod(fabs(angle), 400.); /* make modulo 400            */
      if (angle <= 50.)
        angle = angle * pi / 200.;
      else if (angle < 150.) {
        angle = (100. - angle) * pi / 200.;
        function = MAXTRIG - function; /* change the function        */
        nco = nsi;                     /* swap the conversions       */
        nsi = 1.;
      }
      else if (angle <= 250.) {
        angle = (200. - angle) * pi / 200.;
        nco = -1.;
      }
      else if (angle < 350.) {
        angle = (angle - 300.) * pi / 200.;
        function = MAXTRIG - function; /* change the function        */
        nco = -nsi;
        nsi = 1.;
      }
      else
        angle = (angle - 400.) * pi / 200.;
    }
    switch (function) {                /* process the function       */
      case SINE:                       /* Sine function              */
        result = nsi * sin(angle);
        break;
      case COSINE:                     /* Cosine function            */
        result = nco * cos(angle);
        break;
      case TANGENT:                    /* Tangent function           */
        result = nsi * nco * tan(angle);
        break;
      case COTANGENT:                  /* cotangent function         */
                                       /* overflow?                  */
        if ((result = tan(angle)) == 0.0)
          rc = 40;                     /* this is an error           */
        else
          result = nsi * nco / result; /* real result                */
        break;
    }
    if (!rc)                           /* good result?               */
                                       /* format the result          */
      FormatResult(result, precision, retstr);
  }
  return rc;                           /* return success code        */
}

/*********************************************************************/
/* Function ValidateATrig: Common validation routine for math        */
/* that are of the form fn(number, <precision>, <units>)             */
/*********************************************************************/
/* Number of arguments. Function arguments. return string  function to perform */

static long ValidateArcTrig(long numargs,
                            RXSTRING   args[],
                            PRXSTRING  retstr,
                            int function )
{
  LONG      rc;                        /* validation code            */
  INT       units;                     /* angle type                 */
  double    angle;                     /* working angle              */
  double    nsi;                       /* convertion factor          */
  double    nco;                       /* convertion factor          */
  ULONG     precision;                 /* returned precision         */
  double    x;                         /* input number               */

  rc = VALID_ROUTINE;                  /* set default completion     */
  precision = DEFAULT_PRECISION;       /* set max digits count       */
  units = DEGREES;                     /* default angle is degrees   */
  nsi = 1.;                            /* set default conversion     */
  nco = 1.;                            /* set default conversion     */

  if (numargs < 1 ||                   /* no arguments               */
      numargs > 3 ||
      !RXVALIDSTRING(args[0]))         /* first is omitted           */
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs >= 2 &&             /* have a precision           */
      RXVALIDSTRING(args[1]) &&        /* and it is real string      */
      !string2ulong(args[1].strptr, &precision))
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(args[0].strptr, " %lf", &x) != 1)
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs == 3) {             /* have an option             */
    if (RXZEROLENSTRING(args[2]))      /* null string?               */
      rc = INVALID_ROUTINE;            /* this is invalid            */
    else {                             /* process the options        */
                                       /* get the option character   */
      units = toupper(args[2].strptr[0]);
                                       /* was it a good option?      */
      if (units != DEGREES && units != RADIANS && units != GRADES)
        rc = INVALID_ROUTINE;          /* bad option is error        */
    }
  }
  if (!rc) {                           /* everything went well?      */
                                       /* truncate to maximum        */
    precision = min(precision, MAX_PRECISION);
    switch (function) {                /* process the function       */
      case ARCSINE:                    /* ArcSine function           */
        angle = asin(x);
        break;
      case ARCCOSINE:                  /* ArcCosine function         */
        angle = acos(x);
        break;
      case ARCTANGENT:                 /* ArcTangent function        */
        angle = atan(x);
        break;
    }
    if (units == DEGREES)              /* have to convert the result?*/
      angle = angle * 180. / pi;       /* make into degrees          */
    else if (units == GRADES)          /* need it in grades?         */
      angle = angle * 200. / pi;       /* convert to base 400        */
                                       /* format the result          */
    FormatResult(angle, precision, retstr);
  }
  return rc;                           /* return success code        */
}

