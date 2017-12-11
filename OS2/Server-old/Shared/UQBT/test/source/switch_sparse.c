#include <stdio.h>

int main()
{
	int n;
	printf("Input a number, please: ");
	scanf("%d", &n);
	switch(n)
	{
		case 2: printf("Two!\n"); break;
		case 20: printf("Twenty!\n"); break;
		case 200: printf("Two Hundred!\n"); break;
		case 2000: printf("Two thousand!\n"); break;
		case 20000: printf("Twenty thousand!\n"); break;
		case 200000: printf("Two hundred thousand!\n"); break;
		case 2000000: printf("Two million!\n"); break;
		case 20000000: printf("Twenty million!\n"); break;
		case 200000000: printf("Two hundred million!\n"); break;
		case 2000000000: printf("Two billion!\n"); break;
		default: printf("Other!\n");
	}
    return 0;
}
