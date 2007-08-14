/*
 * $Source: /netlabs.cvs/osfree/src/cmd/xcopy/defines.h,v $
 * $Revision: 1.1 $
 * $Date: 2003/10/20 13:59:42 $
 * $Author: prokushev $
 *
 * Nice shortcuts
 *
 * $Log: defines.h,v $
 * Revision 1.1  2003/10/20 13:59:42  prokushev
 * + XCOPY
 * * clean target
 *
 * Revision 1.2  1997/09/22 23:20:05  vitus
 * define PRIVATE to empty string in non-debug versions
 *
 * Revision 1.1  1996/09/02 22:08:19  vitus
 * Initial revision
 */


#if !defined(PRIVATE)
# if !defined(NDEBUG)
#  define PRIVATE
#  define PUBLIC
# else
#  define PRIVATE	static
#  define PUBLIC
# endif
#endif


#if defined(__GNUC__)
# define UNREFERENCED(var) static void *const use_##var = (&use_##var, &var, 0)
# define USE_OS2_TOOLKIT_HEADERS
#endif
