/* Test for procedures that return whatever their callee returns.
    Note: compile with:
    cc -xO4 -xinline= -o test/sparc/returncallee test/source/returncallee.c
    gcc does not use the move/call/move idiom that we want to see (so there
    is no explicit use of the return value in add4)
    For pentium, this tests a tiny function (add2) with no prologue (only case
    I've ever seen)
*/
#include <stdio.h>

int add2(int a, int b)
{
    return a+b;
}

/* This should compile in sparc to a move/call/move sequence */
int add4(int a, int b, int c, int d)
{
    return add2(a+b, c+d);
}

int main()
{
    printf("Fifty five is %d\n", add4(10, 12, 14, 19));
    return 0;
}

