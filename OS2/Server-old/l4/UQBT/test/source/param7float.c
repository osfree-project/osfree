/* In sparc, when the 7th parameter is a float, it is passed in memory, and
  it's usually an integer load instruction that sets it. But in the callee,
  the parameter is loaded directly into a floating point register, so it's
  prototyped as a float. Passing an integer parameter to a proc with an argument
  declared as float causes a conversion, destroying the data.
  So we need full type analysis, or in the interim a sneaky cast using pointers
  to work around the problem.
*/

/* $Revision: 1.2 $
 * Make with gcc -O4 -fno-inline ... */

/*
 * 12 Feb 01 - Mike: Modified to print 13 digits after the decimal point, to
 *              make evident a fault with the way doubles are handled.
 */

void work(int a1, int a2, int a3, int a4, int a5, int a6, float f)
{
    /* Need to use the first 6 arguments, or gcc will throw them away! */
    printf("Burn up arguments: %d %d %d %d %d %d\n", a1, a2, a3, a4, a5, a6);
    /*  Doubles have 52 bits of mantissa, which is about 15 decimal digits of
        precision, so 55 can be safely printed with 13 digits after the
        decimal point.*/
    printf("Fifty five is %.13f\n", f);
}

int main()
{
    work(1, 2, 3, 4, 5, 6, 55.0);
    return 0;
}
