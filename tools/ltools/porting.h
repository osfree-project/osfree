#define _POSIX_SOURCE

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define popen _popen
#define pclose _pclose
#define lseek64 _lseeki64

void test_endianness();
