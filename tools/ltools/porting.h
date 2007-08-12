#include <stdio.h>
#include <io.h>

#define popen _popen
#define pclose _pclose
#define lseek64 _lseeki64

void test_endianness();
