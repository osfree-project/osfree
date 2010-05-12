/*
    Simple program to test floating point manipulation.
    Compile with gcc, no optimisation.
    Note the sparc version is compiled from a slightly modified assembler
    source, see test/sparc/thirds.s
    Tests overlapping registers (sparc source), and generating parameters
    to varargs library functions
*/
#include <stdio.h>
int main()
{
    double oneThird = 1.0/3.0;

    printf("Two thirds is %.12f\n", oneThird*2);
    return 0;
}
