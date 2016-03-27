/* Rexx math library, based on a posting about the amiga math
 * library by shanti@cleo.murdoch.edu.au
 *
 * this implementation merely converts its arguments to doubles and calls
 * the corresponding C library. Potential problems include arguments in
 * exponential format, and ignoring the setting of numeric digits.
 * 
 * Copyright 1998-2001, Patrick TJ McPhee
 * Distributed under the terms of the Mozilla Public Licence
 * You can obtain a copy of the licence at http://www.mozilla.org/MPL
 * The Original Code is regmath
 * The Initial Developer is Patrick TJ McPhee
 * $Header: C:/ptjm/rexx/math/tmp/RCS/rexxmath.c 1.4 2001/11/04 22:51:10 ptjm Exp $
 */

#include "rxproto.h"
#include <math.h>

static double cotan(const double arg);
static double csc(const double arg);
static double sec(const double arg);
static double round(const double arg);

#if defined(__EMX__) || defined(_WIN32)
static double acosh(double arg);
static double asinh(double arg);
static double atanh(double arg);
#endif

static struct funclist {
    char * name;
    double (*fnptr)(double);
} mathfuncs[] = {
    "acos", acos,	/* Inverse cosine acos(x)  -1.0 <= x <= 1.0 */
    "acosh", acosh,	/* Inverse hyperbolic cos acosh(x) x >= 1.0 */
    "asin", asin,	/* Inverse sine asin(x) -1.0 <= x <= 1.0 */
    "asinh", asinh,	/* Inverse hyperbolic sin    asinh(x) */
    "atanh", atanh,	/* Inverse hyp. tangent atanh(x) -1.0 < x < 1.0*/
    "ceil", ceil,	/*Lowest integer above x    ceil(x)*/
    "cos", cos,		/*Cosine                    cos(x)*/
    "cosh", cosh,	/*Hyperbolic cosine         cosh(x)*/
    "cot", cotan,       /* cotangent */
    "cotan", cotan,     /* cotangent */
    "csc", csc,         /* cosecant */
    "exp", exp,		/* e to the power x          exp(x) */
    "floor", floor,	/* Highest integer below x   floor(x)       int */
    "int", floor,	/* Highest integer below x   floor(x)       int */
    "ln", log,		/* Log base e ln(x) log x > 0.0 */
    "log", log,		/* Log base e ln(x) log x > 0.0 */
    "log10", log10,	/* Log base 10 log10(x) x > 0.0  */
    "nint", round,      /* nearest integer */
    "sec", sec,         /* secant */
    "sin", sin,         /* sine */
    "sinh", sinh,       /* hyperbolic sine */
    "sqrt", sqrt,       /* square root */
    "tan", tan,         /* tangent */
    "tanh", tanh,       /* hyperbolic tangent */
};

static int findmathfunc(const void * const l, const void * const r)
{
    return strcasecmp(((struct funclist*)l)->name, ((struct funclist*)r)->name);
}

rxfunc(mathfunc1)
{
    struct funclist * theFunc, afunc;
    
    char * arg;

    checkparam(1,1);
    
    afunc.name = fname;
    theFunc = (struct funclist *)bsearch(&afunc, mathfuncs,DIM(mathfuncs)
               , sizeof(*theFunc), findmathfunc);

    /* this shouldn't happen */
    if (theFunc == NULL)
	return 43;

    rxstrdup(arg, argv[0]);

    result->strlength = sprintf(result->strptr, "%.17g", theFunc->fnptr(atof(arg)));

    return 0;

}

/* Inverse tangent           atan(x) */
/* Inv. tangent y/x          atan(y,x)                   y != 0.0, x != 0.0 */
rxfunc(mathatan)
{
    char * arg, *arg2;

    checkparam(1,2);
    
    if (argc == 1) {
	rxstrdup(arg, argv[0]);
	result->strlength = sprintf(result->strptr, "%.17g", atan(atof(arg)));
    }
    else {
	rxstrdup(arg, argv[0]);
	rxstrdup(arg2, argv[1]);
	result->strlength = sprintf(result->strptr, "%.17g", atan2(atof(arg), atof(arg2)));
    }

    return 0;
}

/* Cosecant                  csc(x) */
static double csc(const double arg)
{
    return 1./sin(arg);
}

/* Secant                    sec(x) */
static double sec(const double arg)
{
    return 1./cos(arg);
}

/* unix systems provide inverse hyperbolics, but windows and os/2 don't */
#if defined(__EMX__) || defined(_WIN32)
static double acosh(double arg)
{
   return log(arg + sqrt(pow(arg,2.)-1.));
}

static double asinh(double arg)
{
   return log(arg + sqrt(pow(arg,2.)+1.));
}

static double atanh(double arg)
{
   return log((1. + arg)/(1. - arg)) / 2.;
}
#endif

/* Factorial                 fact(x)                     x >= 0, x < 171 */
rxfunc(mathfact)
{
    char * arg;
    register unsigned int i, iarg, val = 1;

    checkparam(1,1);
    rxstrdup(arg, argv[0]);
    iarg = atoi(arg);

    for (i = 2; i <= iarg; i++) {
	val *= i;
    }

    result->strlength = sprintf(result->strptr, "%u", val);

    return 0;

}

/* Nearest integer to x      nint(x) */
static double round(const double arg)
{
   return floor(arg+.5);
}


static double cotan(const double arg)
{
   return cos(arg)/sin(arg);
}


/* x to the power y          pow(x,y)       power, xtoy  x >= 0.0 */
rxfunc(mathpow)
{
    char * arg, *arg2;

    checkparam(2,2);
    
    rxstrdup(arg, argv[0]);
    rxstrdup(arg2, argv[1]);
    result->strlength = sprintf(result->strptr, "%.17g", pow(atof(arg), atof(arg2)));

    return 0;
}


rxfunc(mathdropfuncs);
rxfunc(mathloadfuncs);

static struct {
    char * name;
    APIRET (APIENTRY*funcptr)(PUCHAR fname, ULONG argc, PRXSTRING argv, PSZ pSomething, PRXSTRING result);
} funclist[] = {
 "atan", mathatan,       /* arctangent */
 "fact", mathfact,       /* factorial */
 "pow", mathpow,         /* power */
 "power", mathpow,       /* ditto */
 "xtoy", mathpow,        /* la mème chose */
 "mathdropfuncs", mathdropfuncs,
 "mathloadfuncs", mathloadfuncs,
};

/* mloadfuncs() */
rxfunc(mathloadfuncs)
{
    register int i;

    checkparam(0,0);

#ifndef DYNAMIC
    for (i = 0; i < DIM(funclist); i++) {
	RexxRegisterFunctionExe(funclist[i].name, funclist[i].funcptr);
    }

    for (i = 0; i < DIM(mathfuncs); i++) {
	RexxRegisterFunctionExe(mathfuncs[i].name, mathfunc1);
    }
#else
    for (i = 0; i < DIM(funclist); i++) {
	RexxRegisterFunctionDll(funclist[i].name, "rexxmath", funclist[i].name);
    }

    for (i = 0; i < DIM(mathfuncs); i++) {
	RexxRegisterFunctionDll(mathfuncs[i].name, "rexxmath", mathfuncs[i].name);
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

    for (i = 0; i < DIM(mathfuncs); i++) {
	RexxDeregisterFunction(mathfuncs[i].name);
    }

    result_zero();
    return 0;
}
