#include "$$SYMBOL.H"

/*****************************************************************************/
/* MSC C 6.0 32 bit structures.                                              */
/*****************************************************************************/
#define SSBEGIN32   0x80
#define SSPROC32    0x81
#define SSEND32     0x82
#define SSDEF32     0x84
#define SSVAR32     0x85
#define SSREG32     0x8D
#define SSCHGDEF32  0x91
#define SSUSERDEF32 0x12

#define SSREGBX32 19                    /*                                605*/
#define SSREGSI32 22                    /*                                605*/
#define SSREGDI32 23                    /*                                605*/

typedef struct                          /* Type Number : 0x80               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  UINT   BlockOffset;
  USHORT BlockLen;
} SSBegin32;

typedef struct                          /* Type Number : 0x81               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  UINT   ProcOffset;
  USHORT TypeIndex;
  USHORT ProcLen;
  USHORT DebugStart;
  USHORT DebugEnd;
  USHORT Reserved;
  UCHAR  NearFar;
  CHAR Name[1];
} SSProc32;

typedef struct                          /* Type Number : 0x84               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  int    FrameOffset;
  USHORT TypeIndex;
  CHAR Name[1];
} SSDef32;

typedef struct                          /* Type Number : 0x85               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  UINT   Offset;
  USHORT ObjectNum;
  USHORT TypeIndex;
  CHAR Name[1];
} SSVar32;

typedef struct                          /* Type Number : 0x8D               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  USHORT TypeIndex;
  UCHAR  RegNum;
  CHAR Name[1];
} SSReg32;

typedef struct                          /* Type Number : 0x91               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  USHORT SegNum;
  USHORT Reserved;
} SSChgDef32;

typedef struct                          /* Type Number : 0x92               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  USHORT TypeIndex;
  CHAR Name[1];
} SSUserDef32;

typedef struct                          /* Type Number : 0x82               */
{
  UCHAR RecLen;
  UCHAR RecType;
} SSEnd32;

#define SSproc   void                   /*                                101*/

typedef struct
{
  UCHAR RecLen;
  UCHAR RecType;
} SSrec;

/*****************************************************************************/
/* MSC C 6.0/IBM C2 1.1 16 bit structures.                                   */
/*****************************************************************************/

typedef struct                          /* Type Number : 0x00               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  USHORT BlockOffset;
  USHORT BlockLen;
} SSBegin16;

typedef struct                          /* Type Number : 0x01               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  USHORT ProcOffset;
  USHORT TypeIndex;
  USHORT ProcLen;
  USHORT DebugStart;
  USHORT DebugEnd;
  USHORT Reserved;
  UCHAR  NearFar;
  UCHAR Name[1];
} SSProc16;

typedef struct                          /* Type Number : 0x04               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  USHORT FrameOffset;
  USHORT TypeIndex;
  UCHAR Name[1];
} SSDef16;

typedef struct                          /* Type Number : 0x05               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  USHORT Offset;
  USHORT ObjectNum;
  USHORT TypeIndex;
  CHAR Name[1];
} SSVar16;

typedef struct                          /* Type Number : 0x0D               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  USHORT TypeIndex;
  UCHAR  RegNum;
  CHAR Name[1];
} SSReg16;

typedef struct                          /* Type Number : 0x11               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  USHORT SegNum;
  USHORT Reserved;
} SSChgDef16;

typedef struct                          /* Type Number : 0x12               */
{
  UCHAR  RecLen;
  UCHAR  RecType;
  USHORT TypeIndex;
  UCHAR Name[1];
} SSUserDef16;

typedef struct                          /* Type Number : 0x02               */
{
  UCHAR RecLen;
  UCHAR RecType;
} SSEnd16;


/*****************************************************************************/
/* HLL symbol structures.                                                    */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x00               */
{
  UINT   BlockOffset;
  UINT   BlockLen;
} SSBeginHLL;

typedef struct                          /* Type Number : 0x01               */
{
  UINT   ProcOffset;
  USHORT TypeIndex;
  UINT   ProcLen;
  USHORT DebugStart;
  UINT   DebugEnd;
  USHORT Reserved;
  UCHAR  NearFar;
  CHAR Name[1];
} SSProcHLL;

typedef struct                          /* Type Number : 0x04               */
{
  int    FrameOffset;
  USHORT TypeIndex;
  CHAR Name[1];
} SSDefHLL;

typedef struct                          /* Type Number : 0x05               */
{
  UINT   Offset;
  USHORT ObjectNum;
  USHORT TypeIndex;
  CHAR Name[1];
} SSVarHLL;

typedef struct                          /* Type Number : 0x0D               */
{
  USHORT TypeIndex;
  UCHAR  RegNum;
  CHAR Name[1];
} SSRegHLL;

typedef struct                          /* Type Number : 0x11               */
{
  USHORT SegNum;
  USHORT Reserved;
} SSChgDefHLL;

typedef struct                          /* Type Number : 0x12               */
{
  USHORT TypeIndex;
  CHAR Name[1];
} SSUserDefHLL;

