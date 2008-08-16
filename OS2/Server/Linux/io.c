#include <io.h>
#include <stdarg.h>

void io_printf(const char* chrFormat, ...)
{
    va_list arg_ptr;
    void *tb;

    va_start (arg_ptr, chrFormat);
    vprintf(chrFormat, arg_ptr);
    va_end (arg_ptr);
}
