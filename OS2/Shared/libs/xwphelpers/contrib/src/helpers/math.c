
/*
 *@@sourcefile math.c:
 *      some math helpers.
 *
 *      This file is new with V0.9.14 (2001-07-07) [umoeller]
 *      Unless marked otherwise, these things are based
 *      on public domain code found at
 *      "ftp://ftp.cdrom.com/pub/algorithms/c/".
 *
 *      Usage: All C programs.
 *
 *      Function prefix:
 *
 *      --  math*: semaphore helpers.
 *
 *@@added V0.9.14 (2001-07-07) [umoeller]
 *@@header "helpers\math.h"
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/math.h"
#include "helpers/linklist.h"
#include "helpers/standards.h"

#pragma hdrstop

/*
 *@@category: Helpers\Math helpers
 *      see math.c.
 */


/*
 *@@ mathGCD:
 *      returns the greatest common denominator that
 *      evenly divides m and n.
 *
 *      For example, mathGCD(10, 12) would return 2.
 *
 *      Modern Euclidian algorithm (The Art of Computer
 *      Programming, 4.5.2).
 */

int mathGCD(int a, int b)
{
    int r;

    while (b)
    {
        r = a % b;
        a = b;
        b = r;
    }

    return a;

}

/*
 *@@ mathIsSquare:
 *      returns 1 if x is a perfect square, that is, if
 *
 +      sqrt(x) ^ 2 ==x
 */

int mathIsSquare(int x)
{
    int t;
    int z = x % 4849845; // 4849845 = 3*5*7*11*13*17*19
    double r;
    // do some quick tests on x to see if we can quickly
    // eliminate it as a square using quadratic-residues.
    if (z % 3 == 2)
        return 0;

    t =  z % 5;
    if((t==2) || (t==3))
        return 0;
    t =  z % 7;
    if((t==3) || (t==5) || (t==6))
        return 0;
    t = z % 11;
    if((t==2) || (t==6) || (t==7) || (t==8) || (t==10))
        return 0;
    t = z % 13;
    if((t==2) || (t==5) || (t==6) || (t==7) || (t==8) || (t==11))
        return 0;
    t = z % 17;
    if((t==3) || (t==5) || (t==6) || (t==7) || (t==10) || (t==11) || (t==12) || (t==14))
        return 0;
    t = z % 19;
    if((t==2) || (t==3) || (t==8) || (t==10) || (t==12) || (t==13) || (t==14) || (t==15) || (t==18))
        return 0;

    // If we get here, we'll have to just try taking
    // square-root & compare its square...
    r = sqrt(abs(x));
    if (r*r == abs(x))
        return 1;

    return 0;
}

/*
 *@@ mathFindFactor:
 *      returns the smallest factor > 1 of n or 1 if n is prime.
 *
 *      From "http://tph.tuwien.ac.at/~oemer/doc/quprog/node28.html".
 */

int mathFindFactor(int n)
{
    int i,
        max;
    if (n <= 0)
        return 0;

    max = floor(sqrt(n));

    for (i=2;
         i <= max;
         i++)
    {
        if (n % i == 0)
            return i;
    }
    return 1;
}

/*
 *@@ testprime:
 *      returns 1 if n is a prime number.
 *
 *      From "http://tph.tuwien.ac.at/~oemer/doc/quprog/node28.html".
 */

int mathIsPrime(unsigned n)
{
    int i,
        max = floor(sqrt(n));

    if (n <= 1)
        return 0;

    for (i=2;
         i <= max;
         i++)
    {
        if (n % i == 0)
            return 0;
    }

    return 1;
}

/*
 *@@ mathFactorBrute:
 *      calls pfnCallback with every integer that
 *      evenly divides n ("factor").
 *
 *      pfnCallback must be declared as:
 *
 +          int pfnCallback(int iFactor,
 +                          int iPower,
 +                          void *pUser);
 *
 *      pfnCallback will receive the factor as its
 *      first parameter and pUser as its third.
 *      The second parameter will always be 1.
 *
 *      The factor will not necessarily be prime,
 *      and it will never be 1 nor n.
 *
 *      If the callback returns something != 0,
 *      computation is stopped.
 *
 *      Returns the no. of factors found or 0 if
 *      n is prime.
 *
 *      Example: mathFactor(42) will call the
 *      callback with
 *
 +          2 3 6 7 14 21
 *
 +      This func is terribly slow.
 */

int mathFactorBrute(int n,                           // in: integer to factor
                    PFNFACTORCALLBACK pfnCallback,   // in: callback func
                    void *pUser)                     // in: user param for callback
{
    int rc = 0;

    if (n > 2)
    {
        int i;
        int max = n / 2;

        for (i = 2;
             i <= max;
             i++)
        {
            if ((n % i) == 0)
            {
                rc++;
                // call callback with i as the factor
                if (pfnCallback(i,
                                1,
                                pUser))
                    // stop then
                    break;
            }
        }
    }

    return rc;
}

/*
 *@@ mathFactorPrime:
 *      calls pfnCallback for every prime factor that
 *      evenly divides n.
 *
 *      pfnCallback must be declared as:
 *
 +          int pfnCallback(int iFactor,
 +                          int iPower,
 +                          void *pUser);
 *
 *      pfnCallback will receive the prime as its
 *      first parameter, its power as its second,
 *      and pUser as its third.
 *
 *      For example, with n = 200, pfnCallback will
 *      be called twice:
 *
 *      1)  iFactor = 2, iPower = 3
 *
 *      2)  iFactor = 5, iPower = 2
 *
 *      because 2^3 * 5^2 is 200.
 *
 *      Returns the no. of times that the callback
 *      was called. This will be the number of
 *      prime factors found or 0 if n is prime
 *      itself.
 */

int mathFactorPrime(double n,
                    PFNFACTORCALLBACK pfnCallback,   // in: callback func
                    void *pUser)                     // in: user param for callback
{
    int rc = 0;

    double d;
    double k;

    if (n <= 3)
       // this is a prime for sure
       return 0;

    d = 2;
    for (k = 0;
         fmod(n, d) == 0;
         k++)
    {
        n /= d;
    }

    if (k)
    {
        rc++;
        pfnCallback(d,
                    k,
                    pUser);
    }

    for (d = 3;
         d * d <= n;
         d += 2)
    {
        for (k = 0;
             fmod(n,d) == 0.0;
             k++ )
        {
            n /= d;
        }

        if (k)
        {
            rc++;
            pfnCallback(d,
                        k,
                        pUser);
        }
    }

    if (n > 1)
    {
        if (!rc)
            return 0;

        rc++;
        pfnCallback(n,
                    1,
                    pUser);
    }

    return rc;
}

/*
 *@@ mathGCDMulti:
 *      finds the greatest common divisor for a
 *      whole array of integers.
 *
 *      For example, if you pass in three integers
 *      1000, 1500, and 1800, this would return 100.
 *      If you only pass in 1000 and 1500, you'd
 *      get 500.
 *
 *      Use the fact that:
 *
 *         gcd(u1, u2, ..., un) = gcd(u1, gcd(u2, ..., un))
 *
 *      Greatest common divisor of n integers (The
 *      Art of Computer Programming, 4.5.2).
 */

int mathGCDMulti(int *paNs,             // in: array of integers
                 int cNs)               // in: array item count (NOT array size)
{
    int d = paNs[cNs-1];
    int k = cNs-1;

    while (    (d != 1)
            && (k > 0)
          )
    {
        d = mathGCD(paNs[k-1], d);
        k--;
    }

    return d;
}


