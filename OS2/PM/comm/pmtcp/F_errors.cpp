/*! @file F_errors.cpp
    @brief Error string helpers. Cross-platform: OS/2 and Win32.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#if defined(__OS2__) || defined(__OS2V2__)
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#else
#include "F_tcp.hpp"
#endif

/*! @brief Forward declaration for xstdio_strerror.
    @return Human-readable error string. */
const char * xstdio_strerror(void);

/*! @brief Return the OS/2 error message for an OS/2 error code.

    @param [in] ierr  OS/2 error code.
    @return Human-readable string. */
char * GetOS2ErrorMessage(int ierr)
{
    static char buf[128];
#if defined(__OS2__) || defined(__OS2V2__)
    snprintf(buf, sizeof(buf), "OS/2 error %d", ierr);
#else
    snprintf(buf, sizeof(buf), "error %d: %s", ierr, strerror(ierr));
#endif
    return buf;
}

/*! @brief Return the FreePM error message for a FreePM error code.

    @param [in] ierr  FreePM error code.
    @return Human-readable string. */
char * GetFreePMErrorMessage(int ierr)
{
    return GetOS2ErrorMessage(ierr);
}

/*! @brief strerror-style wrapper for the requested error class.

    @param [in] type  One of the FD_* error class codes.
    @return Human-readable error string. */
const char * xstrerror_type(int type)
{
    (void)type;
    return xstdio_strerror();
}

/*! @brief strerror-style wrapper for the C runtime errno.

    @return Human-readable error string. */
const char * xstdio_strerror(void)
{
    static char buf[256];
    strncpy(buf, strerror(errno), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    return buf;
}

/*! @brief Return the C runtime standard error string.

    @param [in] ierr  C runtime error code.
    @return Human-readable error string. */
const char * StdStrError(int ierr)
{
    static char buf[128];
    strncpy(buf, strerror(ierr), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    return buf;
}
