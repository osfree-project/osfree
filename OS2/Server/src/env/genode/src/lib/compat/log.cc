/* logging interface */

/* osFree internal */
#include <os3/io.h>

/* Genode includes */
#include <base/printf.h>

/* libc includes */
#include <stdarg.h>

extern "C"
void io_log(const char* chrFormat, ...)
{
    va_list arg_ptr;

    va_start (arg_ptr, chrFormat);
    Genode::vprintf(chrFormat, arg_ptr);
    va_end (arg_ptr);
}
