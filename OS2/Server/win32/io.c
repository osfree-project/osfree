#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "io.h"

#define NO_ERROR 0
#define ERROR_FILE_NOT_FOUND 2

void io_printf(const char* chrFormat, ...)
{
    va_list arg_ptr;

    va_start (arg_ptr, chrFormat);
    vprintf(chrFormat, arg_ptr);
    va_end (arg_ptr);
}

/*! @todo add memory allocation checks
    @todo remove memory allocation like on OS/2 API
*/
int io_load_file(const char * filename, void ** addr, unsigned long * size)
{
  FILE *f;

  f = fopen(filename, "rb");
  if(f) {
    fseek(f, 0, SEEK_END);
    *size = ftell(f);  /* Extract the size of the file and reads it into a buffer.*/
    rewind(f);
    *addr = (void *)malloc(*size+1);
    fread(*addr, *size, 1, f);
    fclose(f);
    return NO_ERROR;
  }

  return ERROR_FILE_NOT_FOUND;
}

