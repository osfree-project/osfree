   #ifndef __stringe
     #define __stringe
// version:
// 07/03/97 - added right function
//          - added pos and lastpos
// 07/15/97 - cleaned up some memory management

#if __cplusplus
   extern "C" {
   #endif

int words (char *source);

char *word (char *target, char *source, int pos);

char *strtoupper (char *target, char *source);

char *strtolower (char *target, char *source);

char *strstrncase (char *haystack, char *needle);

char *strip (char *target, char *source, char *oper);

char *left (char *target, char *source, int len);

char *right (char *target, char *source, int len);

char *fillstrx(char *target, int len, char datachar);

char *fillstr(char *target, int len);

char *strinit(char *target, int len, char datachar);

char *substr (char *target, char *source, int pos,
			int len);
		
char *mid (char *target, char *source, int pos);

	
int pos(char *findstr, char *source);

int lastpos(char *findstr, char *source);
	
char *parseword(char *target, char *source);

char *justify (char * target, char * source, int tmax);

char *justifynum (char *target, int source, long tmax);
	
char * quoted(char * target, char * source);

#if __cplusplus
   }
   #endif
#endif
