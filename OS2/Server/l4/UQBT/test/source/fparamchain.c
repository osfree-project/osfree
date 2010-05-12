/* In Sparc, this demonstrates how an empty procedure (just a return statement)
    can pass on multiple arguments. Note that as of gcc 2.95.2.1, gcc generates
    a save/restore and copies args from %i* to %o*, which does not demonstrate
    what we want */
/* 11 Oct 01 - Mike: Changed types to be more testing for pa-risc */
/* Compile with cc -xO4 -xinline= ... */

void addem(double a, int b, float c, int* res)
{
    *res = (int)a+b+(int)c;
}

void passem(double a, int b, float c, int* res)
{
    addem(a, b, c, res);
}

int main()
{
    int res;
    passem(5., 10, 40., &res);
    printf("Fifty five is %d\n", res);
    return 0;
}
