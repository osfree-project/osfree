#ifndef _OW_DLFCN_H_
#define _OW_DLFCN_H_

#ifdef __cplusplus
extern "C" {
#endif
/* Defines from bits/dlfcn.h. */

/* Declarations to compile lx_loader with Open Watcom (on Linux?). */


/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY	0x00001	/* Lazy function call binding.  */
#define RTLD_NOW	0x00002	/* Immediate function call binding.  */


/* Prototypes from dlfcn.h. */
#define __restrict 
/* Open the shared object FILE and map it in; return a handle that can be
   passed to `dlsym' to get symbol values from it.  */
extern void *dlopen (const char *__file, int __mode); /* __THROW; */

/* Unmap and close a shared object opened by `dlopen'.
   The handle cannot be used again after calling `dlclose'.  */
extern int dlclose (void *__handle) ; /*__THROW __nonnull ((1)); */

/* Find the run-time address in the shared object HANDLE refers to
   of the symbol called NAME.  */
extern void *dlsym (void *__restrict __handle,
		    const char *__restrict __name); /* __THROW __nonnull ((2)); */

/* When any of the above functions fails, call this function
   to return a string describing the error.  Each call resets
   the error string so that a following call returns null.  */
extern char *dlerror (void); /* __THROW; */

#ifdef __cplusplus
};
#endif

#endif
