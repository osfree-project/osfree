#include <stdio.h>

#ifdef __WATCOMC__
#include <io.h>
#endif

#define popen _popen
#define pclose _pclose
#define lseek64 _lseeki64

void test_endianness();
