/* Test for procedures that only return their parameter.
    Note: compile with -fno-inline or equivalent!
    Note: for pentium target, compile with -S and edit the assembler to
      align the "World\0" string on a 4 byte boundary
    Only really an issue with sparc source machine, but when translating
    pentium <-> sparc, the 6 character (including null) strings test the
    swap macros */
/* 26 Oct 01 - Mike: Replaced address of string (in stack) with constant 99
                (can't compare output if depends on stack address) */

#include <stdio.h>

typedef struct myStructTag {
    char a[16];
    char b[16];
} myStruct;

/* The following function should compile to just a ret-leaf instruction (on
    Sparc source machines) */
char* getFirstStr(struct myStructTag* p)
{
    return p->a;
}

int main()
{
    myStruct s;
    strcpy(s.a, "Hello");
    printf("s.a = '%s' %d\n", s.a, 99);
    strcpy(s.b, "World");
    printf("s.b = '%s' %d\n", s.b, 99);
    printf("Elements are %s and %s\n", getFirstStr(&s), s.b);
    return 0;
}
