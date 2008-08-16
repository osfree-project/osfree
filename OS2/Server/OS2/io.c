#include "io.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void io_printf(const char* chrFormat, ...)
{
    va_list arg_ptr;

    va_start (arg_ptr, chrFormat);
    vprintf(chrFormat, arg_ptr);
    va_end (arg_ptr);
}

int io_load_file(char * filename, void ** addr, int * size)
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
    }

  return 0;
}

