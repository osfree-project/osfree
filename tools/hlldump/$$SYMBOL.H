/*****************************************************************************/
/* Values for the rectype field of the SSrec record in the $$symbols segment */
/*****************************************************************************/
#define   SSBEGIN        0x00
#define   SSPROC         0x01
#define   SSEND          0x02
#define   SSDEF          0x04
#define   SSVAR          0x05
#define   SSREG          0x0D
#define   SSCHGDEF       0x11
#define   SSUSERDEF      0x12
#define   SSPROCCPP      0x1D
#define   SSVARCPP       0x1E
#define   SSMEMFUNC      0x1A
#define   SSENTRY        0x0F

/*****************************************************************************/
/* values for the internal flags for procedure and static var records.       */
/*****************************************************************************/
#define   PROCEDURE      0x01
#define   CPPPROCEDURE   0x02
#define   MEMBERFUNC     0x04
#define   SECONDENTRY    0x08

#define   STATIC         0x01
#define   CPPSTATIC      0x02
/*****************************************************************************/
/* Internal symbol structures.                                               */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x00               */
{
  USHORT  RecLen;
  UCHAR   RecType;
  UINT    BlockOffset;
  UINT    BlockLen;
} SSBegin;

typedef struct                          /* Type Number : 0x01               */
{
  USHORT  RecLen;
  UCHAR   RecType;
  UCHAR   Flags;
  UINT    ProcOffset;
  USHORT  TypeIndex;
  UINT    ProcLen;
  USHORT  DebugStart;
  UINT    DebugEnd;
  USHORT  ClassType;
  UCHAR   NearFar;
  USHORT  NameLen;
  char    Name[1];
} SSProc;

typedef struct                          /* Type Number : 0x04               */
{
  USHORT  RecLen;
  UCHAR   RecType;
  int     FrameOffset;
  USHORT  TypeIndex;
  USHORT  NameLen;
  char    Name[1];
} SSDef;

typedef struct                          /* Type Number : 0x05               */
{
  USHORT  RecLen;
  UCHAR   RecType;
  UCHAR   Flags;
  UINT    Offset;
  USHORT  ObjectNum;
  USHORT  TypeIndex;
  USHORT  NameLen;
  char    Name[1];
} SSVar;

typedef struct                          /* Type Number : 0x0D               */
{
  USHORT  RecLen;
  UCHAR   RecType;
  USHORT  TypeIndex;
  UCHAR   RegNum;
  USHORT  NameLen;
  char    Name[1];
} SSReg;

typedef struct                          /* Type Number : 0x11               */
{
  USHORT  RecLen;
  UCHAR   RecType;
  USHORT  SegNum;
  USHORT  Reserved;
} SSChgDef;

typedef struct                          /* Type Number : 0x12               */
{
  USHORT  RecLen;
  UCHAR   RecType;
  USHORT  TypeIndex;
  USHORT  NameLen;
  char    Name[1];
} SSUserDef;

typedef struct                          /* Type Number : 0x02               */
{
  USHORT  RecLen;
  UCHAR   RecType;
} SSEnd;

typedef struct _SSrec
{
  USHORT  RecLen;
  UCHAR   RecType;
} SSRec;


/* values for the regno field.                                               */

#define SSREGBX16 11                    /*                                605*/
#define SSREGSI16 14                    /*                                605*/
#define SSREGDI16 15                    /*                                605*/
#define SSREGBX32 19                    /*                                605*/
#define SSREGSI32 22                    /*                                605*/
#define SSREGDI32 23                    /*                                605*/

/* The symbol STACKFRAME used by findlvar to indicate that the address of
 * the local variable it found is relative to the stack frame.  The pointer
 * returned in this case has a selector value of STACKFRAME, and a signed
 * offset value that locates the variable relative to the base of the
 * stack frame.  To compute an address, substitute the value of the SS
 * register for the selector, and add the value of the BP register to the
 * offset.
 */
#define STACKFRAME 0xFED8
#define REGISTERADDRESS 0xFFD8
#define REGISTERTYPEADDR 0x40           /* reg type addr bit pattern is   205*/
                                        /* as follows 01000000            205*/
#define REGISTERADDRMASK 0x3FFFFFFF     /* register address mask.         205*/
#define STACKADDRBIT    31              /* EBP relative addr bit.         101*/
#define STACKADDRSIGN   30              /* EBP relative +- bit.           101*/
#define REGADDRBIT      30              /* Register address bit.          205*/
#define REGADDCHECKPOS  24              /* postion of bit from where to   205*/
                                        /* check register address         205*/
