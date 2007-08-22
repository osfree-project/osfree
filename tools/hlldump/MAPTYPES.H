/*****************************************************************************/
/* Microsoft Types structures.                                               */
/*****************************************************************************/

/*****************************************************************************/
/* STRUCTURE Record.                                                         */
/* - Variable length fields "BitLen" and "NumMems" are not present in the    */
/*   structure. They would be taken care in the code.                        */
/* - "Pck'd" field is not mapped and so is not present in the structure.     */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x79                */
{
  UCHAR   FID_TypeListIndex;
  USHORT  TypeListIndex;
  UCHAR   FID_NameListIndex;
  USHORT  NameListIndex;
  UCHAR   FID_Name;
  UCHAR   NameLen;
  CHAR    Name[1];
} MS_STRUCT;

/*****************************************************************************/
/* BIT FIELD Record.                                                         */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x5C                */
{
  CHAR  BitSize;
  CHAR  BaseType;
  CHAR  Offset;
} MS_BITFLD;

/*****************************************************************************/
/* USERDEF Record.                                                           */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x5D                */
{
  UCHAR   FID_TypeIndex;
  USHORT  TypeIndex;
  UCHAR   FID_Name;
  UCHAR   NameLen;
  CHAR    Name[1];
} MS_USERDEF;

/*****************************************************************************/
/* POINTER Record.                                                           */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x7A                */
{
  CHAR    Model;
  UCHAR   FID_TypeIndex;
  USHORT  TypeIndex;
  UCHAR   FID_Name;
  UCHAR   NameLen;
  CHAR    Name[1];
} MS_POINTER;

/*****************************************************************************/
/* SCALAR Record.                                                            */
/* - The fields that are present after the "Enum Tag Name" (which is variable*/
/*   length) are not included in the structure ("Name Index", "Lower bound"  */
/*   and "Upper bound") and they will be taken care in the code.             */
/*                                                                           */
/* - Microsoft "Base Type" field will be mapped to "Data Type" in internal   */
/*   format by converting Microsoft designators to primitive types.          */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x7B                */
{
  UCHAR   BitLength;
  UCHAR   BaseType;
  UCHAR   FID_Name;
  UCHAR   NameLen;
  CHAR    Name[1];
} MS_SCALAR;

/*****************************************************************************/
/* ARRAY Record.                                                             */
/* - Variable length field "Length" (appears after TRec) is not included in  */
/*   the structure and will be taken care in the code.                       */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x78                */
{
  UCHAR   FID_ElemType;
  USHORT  ElemType;
  UCHAR   FID_IndexMethod;
  USHORT  IndexMethod;
  UCHAR   FID_Name;
  UCHAR   NameLen;
  CHAR    Name[1];
} MS_ARRAY;

/*****************************************************************************/
/* PROCEDURE/FUNCTION Records.                                               */
/* - Calling conventions field will be mapped to IBM type qualifier byte in  */
/*   the internal format.                                                    */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x75                */
{
  CHAR    Null;
  UCHAR   FID_ReturnType;
  USHORT  ReturnType;
  CHAR    CallConv;
  CHAR    NumParams;
  UCHAR   FID_ParmListIndex;
  USHORT  ParmListIndex;
} MS_PROC;

/*****************************************************************************/
/* SKIP Record.                                                              */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x90                */
{
  USHORT  NewIndex;
} MS_SKIP;

/*****************************************************************************/
/* IBM HLL Types structures.                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* STRUCTURE Record.                                                         */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x79                */
{
  CHAR    TypeQual;
  LONG    ByteSize;
  USHORT  NumMembers;
  UCHAR   FID_TypeListIndex;
  USHORT  TypeListIndex;
  UCHAR   FID_NameListIndex;
  USHORT  NameListIndex;
  UCHAR   FID_Name;
  UCHAR   NameLen;
  CHAR    Name[1];
} HL_STRUCT;

/*****************************************************************************/
/* CLASS Record.                                                             */
/*****************************************************************************/
typedef struct
{
 UCHAR ENC_1stByte;
 UCHAR ENC_2ndByte;
}ENCLEN;

typedef struct                          /* Type Number : 0x40                */
{
  CHAR   TypeQual;
  LONG   ByteSize;
  USHORT NumMembers;
  USHORT TypeListIndex;
  ENCLEN EncLen;
} HL_CLASS;

/*****************************************************************************/
/* Member Function Record. virtual/non-virtual                               */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x45                */
{
  UCHAR   TypeQual;
  UCHAR   Protection;
  UCHAR   FuncType;
  USHORT  SubRecIndex;
  union
  {
   ENCLEN  EncLen;
   struct
   {
    UCHAR  FidSpan;
    UCHAR  VirtNo;                      /* variable. may be 4 bytes.         */
   }vTableIndex;
  }fid;
} HL_MEMFNC;

/*****************************************************************************/
/* Class Member Record.                                                      */
/*  - there may be two ENC names following the offset.                       */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x46                */
{
  UCHAR   TypeQual;
  UCHAR   Protection;
  USHORT  TypeIndex;
  UCHAR   FidSpan;
  UCHAR   Offset;                      /* variable 1,2, or 4 bytes.         */
} HL_CLSMEM;

/*****************************************************************************/
/* Base Class Record.                                                        */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x41                */
{
  UCHAR   TypeQual;
  UCHAR   Protection;
  USHORT  TypeIndex;
  UCHAR   FidSpan;
  UCHAR   Offset;                      /* variable 1,2, or 4 bytes.         */
} HL_BSECLS;

/*****************************************************************************/
/* ClassDef Record.                                                          */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x43                */
{
  UCHAR   TypeQual;
  UCHAR   Protection;
  USHORT  TypeIndex;
  USHORT  ClassType;
} HL_CLSDEF;

/*****************************************************************************/
/* Reference Record.                                                         */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x48                */
{
  UCHAR   TypeQual;
  USHORT  TypeIndex;
} HL_REF;

/*****************************************************************************/
/* Friend record.                                                            */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x42                */
{
  CHAR   TypeQual;
  USHORT TypeIndex;
  ENCLEN EncLen;
} HL_FRIEND;

/*****************************************************************************/
/* BIT FIELD Record.                                                         */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x5C                */
{
  CHAR  TypeQual;
  CHAR  Offset;
  CHAR  FID_Span;
  CHAR  BitSize[1];
} HL_BITFLD;

/*****************************************************************************/
/* TYPEDEF/USERDEF Record.                                                   */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x5D                */
{
  CHAR    TypeQual;
  UCHAR   FID_TypeIndex;
  USHORT  TypeIndex;
  UCHAR   FID_Name;
  UCHAR   NameLen;
  CHAR    Name[1];
} HL_USERDEF;

/*****************************************************************************/
/* POINTER Record.                                                           */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x7A                */
{
  CHAR    TypeQual;
  UCHAR   FID_TypeIndex;
  USHORT  TypeIndex;
  UCHAR   FID_Name;
  UCHAR   NameLen;
  CHAR    Name[1];
} HL_POINTER;

/*****************************************************************************/
/* ENUM Record.                                                              */
/* - Fields "Lower bound", "Upper bound" (which are of variable size) and    */
/*   the following fields are not present in the structure and will be taken */
/*   care in the code.                                                       */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x7B                */
{
  CHAR    TypeQual;
  UCHAR   FID_DataType;
  USHORT  DataType;
  UCHAR   FID_MemListIndex;
  USHORT  MemListIndex;
} HL_ENUM;

/*****************************************************************************/
/* SCALAR Record.                                                            */
/* - Fields other than "DataType" are not used, and so they are not present  */
/*   in the data structure.                                                  */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x51                */
{
  CHAR    TypeQual;
  UCHAR   FID_DataType;
  USHORT  DataType;
} HL_SCALAR;

/*****************************************************************************/
/* ARRAY Record.                                                             */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x78                */
{
  CHAR    TypeQual;
  LONG    ByteSize;
  UCHAR   FID_BoundsIndex;
  USHORT  BoundsTypeIndex;
  UCHAR   FID_ElemType;
  USHORT  ElemType;
  UCHAR   FID_Name;
  UCHAR   NameLen;
  CHAR    Name[1];
} HL_ARRAY;

/*****************************************************************************/
/* PROCEDURE/FUNCTION Record.                                                */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x75                */
{
  CHAR    TypeQual;
  USHORT  NumParams;
  USHORT  MaxParams;
  UCHAR   FID_ReturnType;
  USHORT  ReturnType;
  UCHAR   FID_ParmListIndex;
  USHORT  ParmListIndex;
} HL_PROC;

/*****************************************************************************/
/* #defines for list types.                                                  */
/*****************************************************************************/
#define LIST_ST_TYPES  0x01              /* List record for STRUCT types     */
#define LIST_ST_NAMES  0x02              /* List record for STRUCT names     */
#define LIST_ST_ENUM   0x03              /* List record for ENUM names.      */
