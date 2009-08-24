#include <string.h>

/* "Parameter is never used in function" off for TC/BC */
#pragma warn -par

#ifdef __cplusplus
extern "C"
#endif
char *pascal near __mkname (char *s, const char prefix[], unsigned n) {
	static char buf[] = "$$$$$$$$.$$$";
	return s ? strcpy (s, buf) : buf;
}

#pragma warn .par
