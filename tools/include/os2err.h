/* os2err.h - OS/2 error codes for host and OS/2 builds
 * (C89 + Watcom extensions) */
#ifndef OS2ERR_H
#define OS2ERR_H

/**
 * @file os2err.h
 * @brief OS/2 error code constants.
 *
 * Provides the OS/2 error codes used by the tools.
 *
 * If the application has defined INCL_DOSERRORS before including
 * os2.h, the Toolkit has already pulled in bseerr.h and the error
 * codes are provided by the Toolkit. In that case this header does
 * nothing. Otherwise (host builds, or OS/2 builds that did not
 * request the Toolkit error codes) the constants are defined here.
 *
 * Only the codes actually used by the tools are declared. New codes
 * must be added with their exact numeric value taken from the OS/2
 * reference.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INCL_DOSERRORS

/** @brief Operation completed successfully. Value: 0. */
#define NO_ERROR                    0

/** @brief The handle is not valid. Value: 6. */
#define ERROR_INVALID_HANDLE        6

/** @brief Not enough memory to process the request. Value: 8. */
#define ERROR_NOT_ENOUGH_MEMORY     8

/** @brief One or more parameters are not valid. Value: 87. */
#define ERROR_INVALID_PARAMETER     87

/** @brief No data is available (container is empty). Value: 232. */
#define ERROR_NO_DATA               232

/** @brief No more items (cursor out of range). Value: 259. */
#define ERROR_NO_MORE_ITEMS         259

#endif /* INCL_DOSERRORS */

#ifdef __cplusplus
}
#endif

#endif /* OS2ERR_H */
