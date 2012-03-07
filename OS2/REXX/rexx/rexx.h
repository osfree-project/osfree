/*  REXX.DLL functions prototypes
 *
 */

#include <rexxsaa.h>

void TraceString( char *fmt, ... );
#define debug printf

// To extract offset or selector from any FAR (16:16) pointer
#define OFFSETOF16(p)   (((PWORD)&(p))[0])
#define SEGMENTOF16(p)  (((PWORD)&(p))[1])

// To convert a tiled 16:16 address to a 0:32 address
#define MAKEFLATP(fp)   ((PVOID)((SEGMENTOF16(fp)&~7)<<13 | OFFSETOF16(fp)))

typedef unsigned short WORD;            // w

typedef WORD FAR *PWORD;                // pw

typedef USHORT _Far16 *PUSHORT16;

typedef SHORT _Far16 *PSHORT16;

#pragma pack(2)

typedef char _Far16 * PCH16;
typedef char _pascal _Far16 * PSZ16;

typedef struct {
   ULONG           strlength;          /*   length of string         */
   PCH16           strptr;             /*   far pointer to string    */
   } RXSTRING16;

typedef RXSTRING16 _Far16 *PRXSTRING16;       /* pointer to a RXSTRING      */

typedef struct shvnode {
    struct shvnode _Far16 *shvnext;      /* pointer to the next block   */
    RXSTRING16         shvname;       /* Pointer to the name buffer  */
    RXSTRING16         shvvalue;      /* Pointer to the value buffer */
    ULONG              shvnamelen;    /* Length of the name value    */
    ULONG              shvvaluelen;   /* Length of the fetch value   */
    UCHAR              shvcode;       /* Function code for this block*/
    UCHAR              shvret;        /* Individual Return Code Flags*/
    } SHVBLOCK16;

typedef SHVBLOCK16 _Far16 *PSHVBLOCK16;

typedef struct {
   PSZ16 sysexit_name;                 /* subcom enviro for sysexit  */
   SHORT sysexit_code;                 /* sysexit function code      */
   } RXSYSEXIT16;

typedef RXSYSEXIT16 _Far16 *PRXSYSEXIT16;     /* pointer to a RXSYSEXIT     */

#pragma pack()

USHORT _Far16 _Pascal RXTRACERESET(LONG pid, LONG tid);

USHORT _Far16 _Pascal RXVAR(PSHVBLOCK16 _Far16 PSHV);

SHORT _Far16 _Pascal REXXSAA(
         SHORT argc,                        /* Num of args passed to rexx */
         PRXSTRING16 argv,                  /* Array of args passed to rex*/
         PSZ16  path,                       /* [d:][path] filename[.ext]  */
         PRXSTRING16 buf,                   /* Loc of rexx proc in memory */
         PSZ16 env,                         /* ASCIIZ initial environment.*/
         SHORT type,                        /* type (command,subrtn,funct)*/
         PRXSYSEXIT16 sysexit,              /* SysExit env. names &  codes*/
         PSHORT16      retc,                /* Ret code from if numeric   */
         PRXSTRING16   retv);               /* Retvalue from the rexx proc*/

USHORT _Far16 _Pascal RXHALTSET(
         LONG PID,                         /* Process Id                  */
         LONG TID);                        /* Thread Id                   */

USHORT _Far16 _Pascal RXTRACESET(
         LONG PID,                         /* Process Id                  */
         LONG TID);                        /* Thread Id                   */

// This function not found in any of OS/2 Toolkits
USHORT _Far16 _Pascal RXBREAKCLEANUP(VOID);

LONG   APIENTRY ReginaRexxStart(LONG ,                        /* Num of args passed to rexx */
         PRXSTRING,                    /* Array of args passed to rex */
         PSZ,                          /* [d:][path] filename[.ext]  */
         PRXSTRING,                    /* Loc of rexx proc in memory */
         PSZ,                          /* ASCIIZ initial environment.*/
         LONG ,                        /* type (command,subrtn,funct) */
         PRXSYSEXIT,                   /* SysExit env. names &  codes */
         PSHORT,                       /* Ret code from if numeric   */
         PRXSTRING );                  /* Retvalue from the rexx proc */

APIRET APIENTRY RexxBreakCleanup(VOID);
