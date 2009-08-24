#ifndef DLFCN_H
#define DLFCN_H

#include <features.h>

/*
 * Various defines and so forth for the dynamic linker
 */

/* For dlopen () */
#define RTLD_LAZY	1
#define RTLD_NOW	2
#define RTLD_GLOBAL	0x100

/* For dlsym */
#define RTLD_NEXT	((void *)-1)

__BEGIN_DECLS

/* The usual prototypes.  We use void * instead of the actual
 * datatype - the user does not manipulate the handles at all.
 */
 
extern void * dlopen __P((__const char * __filename, int __flag));
extern __const char * dlerror __P((void));
extern void * dlsym __P((void *, __const char *));
extern int dlclose __P((void *));

__END_DECLS

#endif
