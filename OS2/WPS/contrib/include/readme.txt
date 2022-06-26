The INCLUDE directory tree contains include files for
most parts of XWorkplace.

Modify setup.h to adjust global compilation #define's. That
file is #include'd with all source files from the subdirectories
of "src\".

The main INCLUDE directory is included in the INCLUDE path by
the makefile. As a result, you have to specify the subdir with
#include, e.g. #include "startshut\apm.h".

See PROGREF.INF for details.


