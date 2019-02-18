/* logging interface */

/* osFree internal */
#include <os3/io.h>

/* libc includes */
#include <stdarg.h>
#include <stdio.h>

/* l4env includes */
#include <l4/log/l4log.h>

void io_log(const char* chrFormat, ...)
{
    va_list arg_ptr;

    va_start (arg_ptr, chrFormat);
#ifdef DEBUG
    LOG_vprintf(chrFormat, arg_ptr);
#endif
    va_end (arg_ptr);
}
