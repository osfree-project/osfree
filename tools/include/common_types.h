/* common_types.h - common base types for osFree tools, OS/2 API style
 * (C89 + Watcom extensions) */
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

/**
 * @file common_types.h
 * @brief Common base types shared by all osFree tool modules.
 *
 * When compiling for OS/2 (OS2_INCLUDED is defined), the types are
 * provided by os2def.h. Otherwise they are defined locally so the
 * modules can be used in host builds (Windows, Linux).
 *
 * All tool modules should include this header instead of defining
 * the base types locally.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OS2_INCLUDED

typedef unsigned char       UCHAR;
typedef unsigned short      USHORT;
typedef unsigned long       ULONG;
typedef signed   long       LONG;
typedef signed   __int64    LONGLONG;
typedef int                 BOOL;
typedef char                CHAR;
typedef char               *PSZ;
typedef const char         *PCSZ;
typedef void               *PVOID;
typedef const void         *PCVOID;
typedef ULONG               APIRET;

/**
 * @typedef HANDLE
 * @brief Opaque handle value.
 */
typedef PVOID               HANDLE;

typedef UCHAR              *PUCHAR;
typedef USHORT             *PUSHORT;
typedef ULONG              *PULONG;
typedef LONG               *PLONG;
typedef LONGLONG           *PLONGLONG;
typedef BOOL               *PBOOL;
typedef PSZ                *PPSZ;
typedef HANDLE             *PHANDLE;

#define TRUE_        1
#define FALSE_       0

/**
 * @def NULLHANDLE
 * @brief The null handle value.
 */
#define NULLHANDLE   0

#endif /* OS2_INCLUDED */

#ifdef __cplusplus
}
#endif

#endif /* COMMON_TYPES_H */
