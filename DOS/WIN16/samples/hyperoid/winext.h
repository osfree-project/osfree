
#if !defined(WINEXT_H)
#define WINEXT_H

#if defined(__cplusplus)
extern "C" {
#endif 


#if !defined(WINDOWS_H)
#if defined(NULL)
#undef NULL
#endif
#include <windows.h>
#if !defined(NULL)
#define NULL 0
#endif
#define WINDOWS_H
#endif 


#define LARGE huge
/* #define CDECL cdecl */
#define CONST const
#define HNULL (0)
#define LPNULL (0L)

typedef double      DOUBLE;
typedef long double LDOUBLE;



#define npointerdef(o) typedef o * NP ## o
#define lpointerdef(o) typedef o * LP ## o
#define hpointerdef(o) typedef o * HP ## o
#define pointerdef(o) npointerdef(o); lpointerdef(o); hpointerdef(o)

/* npointerdef(BYTE); hptrdef(BYTE); */
npointerdef( CHAR ); lpointerdef( CHAR ); hpointerdef( CHAR );
npointerdef( INT ); hpointerdef( INT );
npointerdef( WORD ); hpointerdef( WORD );
npointerdef( LONG ); hpointerdef( LONG );
npointerdef( DWORD ); hpointerdef( DWORD );
npointerdef( DOUBLE ); lpointerdef( DOUBLE ); hpointerdef( DOUBLE );
npointerdef( LDOUBLE ); lpointerdef( LDOUBLE ); hpointerdef( LDOUBLE );
npointerdef( HANDLE ); hpointerdef( HANDLE );
npointerdef( VOID ); hpointerdef( VOID );


typedef VOID NEAR *NPMEM;
typedef VOID FAR *LPMEM;


/* typedef LONG (FAR PASCAL *WNDPROC)( HWND, unsigned, WORD, LONG ); */





#define GLOBAL      
#define LOCAL       static
#define IMPORT      extern
#define FROM(where) 
#define PROTO       

#define SEGMENT _segment

#if defined(__cplusplus)
}
#endif 


#if defined(__DLL__)
#define DLLEXP EXPORT
#else
#define DLLEXP LARGE
#endif

#endif 

