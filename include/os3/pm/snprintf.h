/* 
 $Id: snprintf.h,v 1.1.1.2 2002/09/05 17:55:51 evgen2 Exp $ 
*/
/* snprintf.h */
#ifndef FREEPM_SNPRINTF
   #define FREEPM_SNPRINTF

#include <stdarg.h>

int snprintf(char *str, size_t count, const char *fmt,...);
int vsnprintf(char *str, size_t count, const char *fmt, va_list arg);
#endif
    /* FREEPM_SNPRINTF */

