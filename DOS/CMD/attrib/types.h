#ifndef _types_h_
#define _types_h_

#ifndef PROC
# if defined (__BORLANDC__)
#  define PROC __fastcall

# elif defined (__TURBOC__)
#  define PROC pascal

# else
#  define PROC

# endif
#endif

#ifndef _Cdecl
# define _Cdecl
#endif

#define LOCAL static


#ifdef __BORLANDC__
# define fnsplit _fnsplit
#endif

#if _MAX_PATH < 260
# define PATHLEN 260
#else
# define PATHLEN _MAX_PATH
#endif


typedef unsigned char byte;

#endif /* _types_h_ */
