#include <l4/os3/io.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(L4API_l4v2)

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

#elif defined(L4API_l4f)

#include <l4/log/log.h>

void io_log(const char* chrFormat, ...)
{
    va_list arg_ptr;

    va_start (arg_ptr, chrFormat);
    LOG_vprintf(chrFormat, arg_ptr);
    va_end (arg_ptr);
}

#else
#error "Not implemented!"
#endif
