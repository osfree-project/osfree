/*
    TE - some size optimizations for __TURBOC__

    as printf() is redefined in PRF.C to use no stream functions,
    rather calls DOS directly, these Stream operations are nowhere used,
    but happen to be in the executable.

    so we define some dummy functions here to save some precious bytes :-)

    this is in no way necessary, but saves us some 1500 bytes
*/

#ifdef __TURBOC__

	#include <stdio.h>

    #define UNREFERENCED_PARAMETER(x) if (x);

    int     cdecl flushall (void){return 0;}

    int     cdecl fprintf  (FILE *__stream, const char *__format, ...)
                             { UNREFERENCED_PARAMETER (__stream);
                               UNREFERENCED_PARAMETER ( __format);    return 0;}
    int     cdecl fseek    (FILE *__stream, long __offset, int __whence)
                             { UNREFERENCED_PARAMETER (__stream);
                               UNREFERENCED_PARAMETER (__offset);
                               UNREFERENCED_PARAMETER ( __whence);
                               return 0;}

    int     cdecl setvbuf  (FILE *__stream, char *__buf, int __type, size_t __size)
                             { UNREFERENCED_PARAMETER (__stream);
                               UNREFERENCED_PARAMETER ( __buf);
                               UNREFERENCED_PARAMETER ( __type);
                               UNREFERENCED_PARAMETER ( __size);   return 0;}

    void    cdecl _xfflush (void){}
    void    cdecl _setupio (void){}

#endif




