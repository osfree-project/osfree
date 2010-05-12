#include <stdio.h>
#include <ctype.h>

int main (int argc, char *argv[])
{ 
	printf ("%c\n", toupper(argv[1][0]));
	return toupper(argv[1][0]);	
}
