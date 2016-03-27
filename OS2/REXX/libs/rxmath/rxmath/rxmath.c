/* Rexx math library, based on IBM's rexx math library rxmath
 *
 * this implementation merely converts its arguments to doubles and calls
 * the corresponding C library function. Potential problems include arguments in
 * exponential format, and ignoring the setting of numeric digits.
 * It seems like IBM's library does the same thing.
 * Note that this is a sub-set of my previous rexxmath library, plus the value
 * of pi.
 * 
 * Copyright 1998,2001, Patrick TJ McPhee
 * Distributed under the terms of the Mozilla Public Licence
 * You can obtain a copy of the licence at http://www.mozilla.org/MPL
 * The Original Code is regmath
 * The Initial Developer is Patrick TJ McPhee
 * $Header: C:/ptjm/rexx/math/tmp/RCS/rxmath.c 1.3 2001/11/04 23:23:42 ptjm Exp $
 */

#include "rxproto.h"
#include <math.h>
#include <stdio.h>
#include <signal.h>

static double cotan(const double arg);

#define PI 3.141592653589793238462643383279502884197169399375

static struct funclist {
    char * name;
    double (*fnptr)(double);
} mathfuncs[] = {
    "rxcalccosh", cosh,	/*Hyperbolic cosine         cosh(x)*/
    "rxcalcexp", exp,		/* e to the power x          exp(x) */
    "rxcalclog", log,		/* Log base e ln(x) log x > 0.0 */
    "rxcalclog10", log10,	/* Log base 10 log10(x) x > 0.0  */
    "rxcalcsinh", sinh,
    "rxcalcsqrt", sqrt,
    "rxcalctanh", tanh,
},

  /* trig functions where the angle is an argument */
  trigfuncs[] = {
    "rxcalccos", cos,		/*Cosine                    cos(x)*/
    "rxcalccotan", cotan,
    "rxcalcsin", sin,
    "rxcalctan", tan,
},
  /* trig functions where the angle is the result */
  trigresults[] = {
    "rxcalcarccos", acos,	/* Inverse cosine acos(x)  -1.0 <= x <= 1.0 */
    "rxcalcarcsin", asin,	/* Inverse sine asin(x) -1.0 <= x <= 1.0 */
    "rxcalcarctan", atan,       /* Inverse tangent atan(x) */
};

static int findmathfunc(const void * const l, const void * const r)
{
    return strcasecmp((char *) l, ((struct funclist*)r)->name);
}

static int setresult(char * str, double dbl, int argc, PRXSTRING argv)
{
   char * precisions, *c1, *c2;
   int precision;

   if (argc > 0 && argv[0].strptr) {
      rxstrdup(precisions, argv[0]);
      precision = atoi(precisions);
      if (precision > 16)
         precision = 16;
   }
   else {
      /* should query value of numeric digits */

      precision = 16;
   }

   return sprintf(str, "%.*g", precision, dbl);
}

rxfunc(mathfunc1)
{
    struct funclist * theFunc;
    char * arg;

    checkparam(1,2);
    
    theFunc = (struct funclist *)bsearch(fname, mathfuncs,DIM(mathfuncs)
               , sizeof(*theFunc), findmathfunc);

    /* this shouldn't happen */
    if (theFunc == NULL)
	return BADGENERAL;

    rxstrdup(arg, argv[0]);

    result->strlength = setresult(result->strptr, theFunc->fnptr(atof(arg)), argc - 1, argv+1);

    return 0;

}

rxfunc(trigfunc1)
{
    struct funclist * theFunc;
    double argval;
    char * arg, *precarg;

    checkparam(1,3);
    
    theFunc = (struct funclist *)bsearch(fname, trigfuncs,DIM(trigfuncs)
               , sizeof(*theFunc), findmathfunc);

    /* this shouldn't happen */
    if (theFunc == NULL)
	return BADGENERAL;

    rxstrdup(arg, argv[0]);

    /* argument might be in degrees or grades */
    if (argc > 2) {
       switch (argv[2].strptr[0]) {
          /* degrees -- convert to radians */
          case 'd':
          case 'D':
             argval = atof(arg) * PI/180.;
             break;

          /* radians */
          case 'r':
          case 'R':
             argval = atof(arg);
             break;

          /* grades -- convert to radians */
          case 'g':
          case 'G':
             argval = atof(arg) * PI/200.;
             break;
          default:
             return BADARGS;
       }
    }
    else {
       /* default is degrees */
       argval = atof(arg) * PI/180.;
    }

    result->strlength = setresult(result->strptr, theFunc->fnptr(argval), argc - 1, argv+1);

    return 0;

}

rxfunc(trigfunc2)
{
    struct funclist * theFunc;
    double res;
    char * arg, *precarg;

    checkparam(1,3);
    
    theFunc = (struct funclist *)bsearch(fname, trigresults,DIM(trigresults)
               , sizeof(*theFunc), findmathfunc);

    /* this shouldn't happen */
    if (theFunc == NULL)
	return BADGENERAL;

    rxstrdup(arg, argv[0]);

    res = theFunc->fnptr(atof(arg));

    /* result could be needed in degrees or grades */
    if (argc > 2) {
       switch (argv[2].strptr[0]) {
          /* degrees -- convert radians to degrees */
          case 'd':
          case 'D':
             res *= 180./PI;
             break;

          /* radians -- do nothing */
          case 'r':
          case 'R':
             break;

          /* grades -- convert radians to grades */
          case 'g':
          case 'G':
             res *= 200./PI;
             break;

          default:
             return BADARGS;
       }
    }
    /* degrees is the default */
    else {
       res *= 180./PI;
    }

    result->strlength = setresult(result->strptr, res, argc - 1, argv+1);

    return 0;
}

static double cotan(const double arg)
{
   return cos(arg)/sin(arg);
}


/* x to the power y          pow(x,y)       power, xtoy  x >= 0.0 */
rxfunc(mathpow)
{
    char * arg, *arg2;

    checkparam(2,3);
    
    rxstrdup(arg, argv[0]);
    rxstrdup(arg2, argv[1]);
    result->strlength = setresult(result->strptr, pow(atof(arg), atof(arg2)), argc - 2, argv+2);

    return 0;
}

/* pi (a constant!) */
rxfunc(mathpi)
{
    static const char pi[] = "3.141592653589793238462643383279502884197169399375";
    char * precisions;
    int precision = 16;

    checkparam(0,1);

    if (argc == 1) {
      rxstrdup(precisions, argv[0]);
      precision = atoi(precisions);
      if (precision > 16)
         precision = 16;
    }

    /* account for the radix */
    precision++;

    result->strlength = precision;
    memcpy(result->strptr, pi, precision);

    return 0;
}


rxfunc(mathdropfuncs);
rxfunc(mathloadfuncs);

static struct {
    char * name;
    APIRET (APIENTRY*funcptr)(PUCHAR fname, ULONG argc, PRXSTRING argv, PSZ pSomething, PRXSTRING result);
} funclist[] = {
 "rxcalcpi", mathpi,
 "rxcalcpower", mathpow,
 "mathdropfuncs", mathdropfuncs,
 "mathloadfuncs", mathloadfuncs,
};

/* mloadfuncs() */
rxfunc(mathloadfuncs)
{
    register int i;

    signal(SIGFPE, SIG_IGN);

    if (argc > 0) {
       puts("Rexx Math Functions for Regina\nCopyright 2001, Patrick TJ McPhee");
       fflush(stdout);
    }

#ifndef DYNAMIC
    for (i = 0; i < DIM(funclist); i++) {
	RexxRegisterFunctionExe(funclist[i].name, funclist[i].funcptr);
    }

    for (i = 0; i < DIM(trigfuncs); i++) {
	RexxRegisterFunctionExe(trigfuncs[i].name, trigfunc1);
    }

    for (i = 0; i < DIM(trigresults); i++) {
	RexxRegisterFunctionExe(trigresults[i].name, trigfunc2);
    }

    for (i = 0; i < DIM(mathfuncs); i++) {
	RexxRegisterFunctionExe(mathfuncs[i].name, mathfunc1);
    }
#else
    for (i = 0; i < DIM(funclist); i++) {
	RexxRegisterFunctionDll(funclist[i].name, "rxmath", funclist[i].name);
    }

    for (i = 0; i < DIM(trigfuncs); i++) {
	RexxRegisterFunctionDll(trigfuncs[i].name, "rxmath", trigfuncs[i].name);
    }

    for (i = 0; i < DIM(trigresults); i++) {
	RexxRegisterFunctionDll(trigresults[i].name, "rxmath", trigresults[i].name);
    }

    for (i = 0; i < DIM(mathfuncs); i++) {
	RexxRegisterFunctionDll(mathfuncs[i].name, "rxmath", mathfuncs[i].name);
    }
#endif

    result_zero();

    return 0;
}

/* mathdropfuncs() */
rxfunc(mathdropfuncs)
{
    register int i;
    checkparam(0,0);

    for (i = 0; i < DIM(funclist); i++) {
	RexxDeregisterFunction(funclist[i].name);
    }

    for (i = 0; i < DIM(trigfuncs); i++) {
	RexxDeregisterFunction(trigfuncs[i].name);
    }

    for (i = 0; i < DIM(trigresults); i++) {
	RexxDeregisterFunction(trigresults[i].name);
    }

    for (i = 0; i < DIM(mathfuncs); i++) {
	RexxDeregisterFunction(mathfuncs[i].name);
    }

    result_zero();
    return 0;
}
