#include <stdio.h>

int factorial (int x)
{
    if (x >= 2)
		return (factorial(x - 1)) * x;
    else return (1);
}

int main (void)
{ int number, value;

	printf ("Input number: ");
	scanf ("%d", &number);
	value = factorial(number);
	printf("Factorial(%d) = %d\n", number, value);
    return (0);
}

