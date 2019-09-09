
#include "windows.h"
#include "pefile.h"

#define MAKEPTR(t,b,offset) 	t((int)b + (int)offset)


extern int LoadPE(char *);
