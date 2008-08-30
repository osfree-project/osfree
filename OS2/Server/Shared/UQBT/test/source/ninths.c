/*
    Simple program to test floating point manipulation.
    Compile with:
    % cc -xO4 -xinline= -o test/sparc/ninths test/source/ninths.c
    Tests:
     1) overlapping registers (sparc source)
     2) and generating parameters to varargs library functions
     3) floating point constants from text segment (note: when the offset
       is zero, as happens here, it is a special case)
     4) Handling incoming doubleword parameters, with different alignments
*/
#include <stdio.h>
/* These parameters will shadow to memory at %sp+68, %sp+76, and %sp+80
   respectively for the Sun cc compiler (which seems to always shadow at
   these addresses, even if they aren't needed for vararg reasons).
   The the first is not double word aligned and so can't be read or written
   with a load or store double intruction. The last is double word aligned,
   and so can be read with a floating double word load. However, because of
   the dummy integer parameter, it can't be written with an integer double
   word store (the registers involved are %o3 and %o4).
   These provide good tests for the translator. */
double myDivide(double top, int dummy, double bottom)
{
    return top/bottom;
}

int main()
{
    double oneNinth = myDivide(1.0, 0, 9.0);

    printf("Two ninths is %.12f\n", oneNinth*2);
    return 0;
}
