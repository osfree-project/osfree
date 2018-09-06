/* Prototypes for regutil
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is regutil.
 *
 * $Header: /opt/cvs/Regina/regutil/regtrig.c,v 1.5 2014/06/22 03:53:33 mark Exp $
 */
/**************************************************************************
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
*      SysArcTan2
*
*  This is file contains the trigonometric functions found in the OOREXX project as
*  of 14 Jan 2008.
*  This file prepared by Michael Greene, January 2008
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

#include "regutil.h"
#include <ctype.h>
#include <math.h>

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
#define ARCTANGENT2 3

#define pi  3.14159265358979323846l    /* pi value                   */

#define DEGREES    'D'                 /* degrees option             */
#define RADIANS    'R'                 /* radians option             */
#define GRADES     'G'                 /* grades option              */

#define DEFAULT_PRECISION  9           /* default precision to use   */
#define MAX_PRECISION     16           /* maximum available precision*/

static long ValidateMath(long numargs, RXSTRING  args[],
                          double   *x, unsigned long *precision );
static long ValidateTrig(long numargs, RXSTRING  args[],
                          PRXSTRING retstr, int function );
static long ValidateArcTrig(long numargs, RXSTRING   args[],
                          PRXSTRING  retstr, int function );

/*********************************************************************/
/* Function FormatResult:  Common routine to format a floating point */
/* result for the math functions.                                    */
/*********************************************************************/
/* formatted result  required precision  return string               */
static void FormatResult(double res, unsigned long precision, PRXSTRING result )
{
  if (res == 0)                     /* zero result?               */
  {
    strcpy(result->strptr, "0");       /* make exactly 0             */
    result->strlength = 1;
  }
  else
                                       /* format the result          */
    result->strlength = sprintf( result->strptr, "%.*f", precision, res );
                                       /* end in a period?           */
  if (result->strptr[result->strlength - 1] == '.')
    result->strlength--;               /* remove the period          */
}



/********************************************************************/
/* Functions:           SysSqrt(), SysExp(), SysLog(), SysLog10,    */
/* Functions:           SysSinH(), SysCosH(), SysTanH()             */
/* Description:         Returns function value of argument.         */
/* Input:               One number.                                 */
/* Output:              Value of the function requested for arg.    */
/*                      Returns 0 if the function executed OK,      */
/*                      BADGENERAL otherwise.  The interpreter will fail    */
/*                      if the function returns a negative result.  */
/* Notes:                                                           */
/*   These routines take one to two parameters.                     */
/*   The form of the call is:                                       */
/*   result = func_name(x <, prec> <,angle>)                        */
/*                                                                  */
/********************************************************************/

rxfunc(syssqrt)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* function return code       */

                                       /* validate the inputs        */
  rc = ValidateMath(argc, argv, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(sqrt(x), precision, result);
  return rc;                           /* return error code          */
}

/*==================================================================*/

rxfunc(sysexp)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(argc, argv, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(exp(x), precision, result);
  return rc;                           /* return error code          */
}

/*==================================================================*/

rxfunc(syslog)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(argc, argv, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(log(x), precision, result);
  return rc;                           /* return error code          */
}

/*==================================================================*/

rxfunc(syslog10)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(argc, argv, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(log10(x), precision, result);
  return rc;                           /* return error code          */
}

/*==================================================================*/

rxfunc(syssinh)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(argc, argv, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(sinh(x), precision, result);
  return rc;                           /* return error code          */
}

/*==================================================================*/

rxfunc(syscosh)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(argc, argv, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(cosh(x), precision, result);
  return rc;                           /* return error code          */
}

/*==================================================================*/

rxfunc(systanh)
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(argc, argv, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(tanh(x), precision, result);
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

rxfunc(syspower)
{
  double    x;                         /* input number               */
  double    y;                         /* second input number        */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation code            */
  char     *prec;

  rc = 0;                  /* set default completion     */
  precision = DEFAULT_PRECISION;       /* set max digits count       */

  if (argc < 2 ||                   /* no arguments               */
      argc > 3 ||
      !RXVALIDSTRING(argv[0]) ||       /* first is omitted           */
      !RXVALIDSTRING(argv[1]))         /* second is omitted          */
    rc = BADGENERAL;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(argv[0].strptr, " %lf", &x) != 1)
    rc = BADGENERAL;              /* this is invalid            */
                                       /* convert second input       */
  else if (sscanf(argv[1].strptr, " %lf", &y) != 1)
    rc = BADGENERAL;              /* this is invalid            */
  if (!rc) {                           /* good function call         */
     if (argc == 3) {                 /* have a precision           */
        rxstrdup(prec,argv[2]);
        precision = atol(prec);
     }
                                       /* keep to maximum            */
     precision = min(precision, MAX_PRECISION);
                                       /* format the result          */
     FormatResult(pow(x,y), precision, result);
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

rxfunc(syssin)
{
                                       /* call common routine        */
  return ValidateTrig(argc, argv, result, SINE);
}

/*==================================================================*/

rxfunc(syscos)
{
                                       /* call common routine        */
  return ValidateTrig(argc, argv, result, COSINE);
}

/*==================================================================*/

rxfunc(systan)
{
                                       /* call common routine        */
  return ValidateTrig(argc, argv, result, TANGENT);
}

/*==================================================================*/

rxfunc(syscotan)
{
                                       /* call common routine        */
  return ValidateTrig(argc, argv, result, COTANGENT);
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

rxfunc(syspi)
{
   ULONG     precision;                 /* required precision         */
   char     *prec;

   checkparam(0, 1);

   precision = DEFAULT_PRECISION;       /* set default precision      */
   if (argc == 1 ) {
      rxstrdup(prec,argv[0]);
      precision = atol(prec);
   }
                                       /* keep to maximum            */
   precision = min(precision, MAX_PRECISION);
                                       /* format the result          */
   FormatResult(pi, precision, result); /* format the result          */
   return 0;                /* good result                */
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

rxfunc(sysarcsin)
{
                                       /* call common routine        */
  return ValidateArcTrig(argc, argv, result, ARCSINE);
}

/*==================================================================*/

rxfunc(sysarccos)
{
                                       /* call common routine        */
  return ValidateArcTrig(argc, argv, result, ARCCOSINE);
}

/*==================================================================*/

rxfunc(sysarctan)
{
                                       /* call common routine        */
  return ValidateArcTrig(argc, argv, result, ARCTANGENT);
}

rxfunc(sysarctan2)
{
                                       /* call common routine        */
  return ValidateArcTrig(argc, argv, result, ARCTANGENT2);
}





/*********************************************************************/
/* Function ValidateMath: Common validation routine for math         */
/* that are of the form fn(number, <precision>)                      */
/*********************************************************************/
/* Number of arguments.  Function arguments.  input number  returned precision    */

static long  ValidateMath(long argc,
                          RXSTRING  argv[],
                          double   *x,
                          unsigned long *precision )
{
  LONG      rc;                        /* validation code            */
  char     *prec;

  rc = 0;                  /* set default completion     */
  *precision = DEFAULT_PRECISION;      /* set max digits count       */

  if (argc < 1 ||                   /* no arguments               */
      argc > 2 ||
      !RXVALIDSTRING(argv[0]))         /* first is omitted           */
    rc = BADGENERAL;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(argv[0].strptr, " %lf", x) != 1)
    rc = BADGENERAL;              /* this is invalid            */

  if ( argc == 2 ) {
     rxstrdup(prec,argv[1]);
     *precision = atol(prec);
  }
                                       /* truncate to maximum        */
  *precision = min(*precision, MAX_PRECISION);
  return rc;                           /* return success code        */
}

/*********************************************************************/
/* Function ValidateTrig: Common validation routine for math         */
/* that are of the form fn(number, <precision>, <unit>)              */
/*********************************************************************/
static long ValidateTrig(long argc,/* Number of arguments.       */
                   RXSTRING  argv[],  /* Function arguments.        */
                   PRXSTRING result,  /* return string              */
                   int function )     /* function to perform        */
{
  LONG      rc;                        /* validation code            */
  int       units;                     /* angle type                 */
  double    angle;                     /* working angle              */
  double    nsi;                       /* convertion factor          */
  double    nco;                       /* convertion factor          */
  ULONG     precision;                 /* returned precision         */
  double    res;                       /* result                     */
  char     *prec;

  rc = 0;                  /* set default completion     */
  precision = DEFAULT_PRECISION;       /* set max digits count       */
  units = DEGREES;                     /* default angle is degrees   */
  nsi = 1.;                            /* set default conversion     */
  nco = 1.;                            /* set default conversion     */

  if (argc < 1 ||                   /* no arguments               */
      argc > 3 ||
      !RXVALIDSTRING(argv[0]))         /* first is omitted           */
    rc = BADGENERAL;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(argv[0].strptr, " %lf", &angle) != 1)
    rc = BADGENERAL;              /* this is invalid            */
  else if (argc == 3) {             /* have an option             */
    if (RXZEROLENSTRING(argv[2]))      /* null string?               */
      rc = BADGENERAL;            /* this is invalid            */
    else {                             /* process the options        */
                                       /* get the option character   */
      units = toupper(argv[2].strptr[0]);
                                       /* was it a good option?      */
      if (units != DEGREES && units != RADIANS && units != GRADES)
        rc = BADGENERAL;          /* bad option is error        */
    }
  }
  if (!rc) {                           /* everything went well?      */
                                       /* truncate to maximum        */
    if (argc >= 2 ) {                  /* have a precision           */
      rxstrdup(prec,argv[1]);
      precision = atol(prec);
    }
    rc = 0;
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
        res = nsi * sin(angle);
        break;
      case COSINE:                     /* Cosine function            */
        res = nco * cos(angle);
        break;
      case TANGENT:                    /* Tangent function           */
        res = nsi * nco * tan(angle);
        break;
      case COTANGENT:                  /* cotangent function         */
                                       /* overflow?                  */
        if ((res = tan(angle)) == 0.0)
          rc = BADGENERAL;                     /* this is an error           */
        else
          res = nsi * nco / res; /* real result                */
        break;
    }
    if (!rc)                           /* good result?               */
                                       /* format the result          */
      FormatResult(res, precision, result);
  }
  return rc;                           /* return success code        */
}

/*********************************************************************/
/* Function ValidateATrig: Common validation routine for math        */
/* that are of the form fn(number, <precision>, <units>)             */
/*********************************************************************/
/* Number of arguments. Function arguments. return string  function to perform */

static long ValidateArcTrig(long argc,
                            RXSTRING   argv[],
                            PRXSTRING  result,
                            int function )
{
  LONG      rc;                        /* validation code            */
  int       units;                     /* angle type                 */
  double    angle;                     /* working angle              */
  double    nsi;                       /* convertion factor          */
  double    nco;                       /* convertion factor          */
  ULONG     precision;                 /* returned precision         */
  double    x;                         /* input number               */
  double    y;                         /* input number               */
  char     *prec;
  int maxargs,minargs;
  int precoff, unitoff;

  rc = 0;                  /* set default completion     */
  precision = DEFAULT_PRECISION;       /* set max digits count       */
  units = DEGREES;                     /* default angle is degrees   */
  nsi = 1.;                            /* set default conversion     */
  nco = 1.;                            /* set default conversion     */

  if ( function == ARCTANGENT2 )
  {
     minargs = 2;
     maxargs = 4;
     precoff = 2;
     unitoff = 3;
  }
  else
  {
     minargs = 1;
     maxargs = 3;
     precoff = 1;
     unitoff = 2;
  }
  if (argc < minargs ||                   /* no arguments               */
      argc > maxargs ||
      !RXVALIDSTRING(argv[0]))         /* first is omitted           */
    rc = BADGENERAL;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(argv[0].strptr, " %lf", &x) != 1)
    rc = BADGENERAL;              /* this is invalid            */
  else if (argc == maxargs) {             /* have a unit option             */
    if (RXZEROLENSTRING(argv[unitoff]))      /* null string?               */
      rc = BADGENERAL;            /* this is invalid            */
    else {                             /* process the options        */
                                       /* get the option character   */
      units = toupper(argv[unitoff].strptr[0]);
                                       /* was it a good option?      */
      if (units != DEGREES && units != RADIANS && units != GRADES)
        rc = BADGENERAL;          /* bad option is error        */
    }
  }
  if ( function == ARCTANGENT2
  && sscanf(argv[1].strptr, " %lf", &y) != 1)
    rc = BADGENERAL;              /* this is invalid            */
  if (!rc) {                           /* everything went well?      */
     if (argc > minargs) {                  /* have a precision           */
        rxstrdup(prec,argv[precoff]);
        precision = atol(prec);
     }
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
      case ARCTANGENT2:                /* ArcTangent2 function        */
        angle = atan2(x,y);
        break;
      default:
        /* should not get here */
        angle = 0.0;
        break;
    }
    if (units == DEGREES)              /* have to convert the result?*/
      angle = angle * 180. / pi;       /* make into degrees          */
    else if (units == GRADES)          /* need it in grades?         */
      angle = angle * 200. / pi;       /* convert to base 400        */
                                       /* format the result          */
    FormatResult(angle, precision, result);
  }
  return rc;                           /* return success code        */
}

