/*  MiniFSD constants and macros
 *
 *
 */

typedef unsigned long  ULONG;
typedef unsigned short USHORT;
typedef USHORT far     *PUSHORT;

#define SELECTOROF(p)    (((PUSHORT)&(p))[1])
#define OFFSETOF(p)      (((PUSHORT)&(p))[0])
#define MAKEP(x, y)      (void far *)(((ULONG)(x) << 16) | (ULONG)(y))

#define NO_ERROR                0
#define ERROR_NEGATIVE_SEEK     131
#define ERROR_SEEK              25
#define ERROR_INVALID_PARAMETER 87
#define ERROR_NOT_SUPPORTED     50
#define ERROR_FILE_NOT_FOUND    2

#define FILE_NORMAL    0x0000
#define FILE_READONLY  0x0001
#define FILE_HIDDEN    0x0002
#define FILE_SYSTEM    0x0004
#define FILE_DIRECTORY 0x0010
#define FILE_ARCHIVED  0x0020

#define FILE_OPEN      0x0001
#define FILE_TRUNCATE  0x0002
#define FILE_CREATE    0x0010

#define OPEN_ACTION_FAIL_IF_EXISTS     0
#define OPEN_ACTION_OPEN_IF_EXISTS     1
#define OPEN_ACTION_REPLACE_IF_EXISTS  2

#define OPEN_ACTION_FAIL_IF_NEW     0x0000
#define OPEN_ACTION_CREATE_IF_NEW   0x0010

#define OPEN_ACCESS_READONLY        0x0000
#define OPEN_ACCESS_WRITEONLY       0x0001
#define OPEN_ACCESS_READWRITE       0x0002
#define OPEN_SHARE_DENYREADWRITE    0x0010
#define OPEN_SHARE_DENYWRITE        0x0020
#define OPEN_SHARE_DENYREAD         0x0030
#define OPEN_SHARE_DENYNONE         0x0040
#define OPEN_FLAGS_NOINHERIT        0x0080
#define OPEN_FLAGS_NO_LOCALITY      0x0000
#define OPEN_FLAGS_SEQUENTIAL       0x0100
#define OPEN_FLAGS_RANDOM           0x0200
#define OPEN_FLAGS_RANDOMSEQUENTIAL 0x0300
#define OPEN_FLAGS_NO_CACHE         0x1000
#define OPEN_FLAGS_FAIL_ON_ERROR    0x2000
#define OPEN_FLAGS_WRITE_THROUGH    0x4000
#define OPEN_FLAGS_DASD             0x8000


