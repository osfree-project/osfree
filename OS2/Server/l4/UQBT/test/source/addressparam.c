/* Test for taking the address of a variable that might be considered as a
    parameter to a child function (e.g. v8, v9 to printf)
    For pentium input binaries, this should be nothing special
    Note: compile without inlining (-fno-inline for gcc, -xinline= for cc)
*/
#include <stdio.h>

int add2(int* a, int* b)
{
    return *a + *b;
}

int main()
{
    int v8 = 50, v9 = 5;
    /* Seem to need to send about 6 pararameters to a call to printf, so
        that v8 looks like it might be an additional parameter */
    printf("Testing, %d, %d, %d, %d, %d...\n", 1, 2, 3, 4, 5);
    printf("Fifty five is %d\n", add2(&v8, &v9));
    return 0;
}


