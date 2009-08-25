/*****************************************************************************/
/* Compatibility garbage.                                                    */
/*****************************************************************************/

#define TG_UNKNOWN  0
#define TG_SCALAR   1
#define TG_POINTER  2
#define TG_STRUCT   3
#define TG_ARRAY    4
#define TG_ENUM     5
#define TG_BITFLD   6
#define TG_CONSTANT 7                   /* constant                       243*/
#define TG_CLASS    8
#define TG_REF      9

#define ADDRESS_CONSTANT   0xFFFF       /* fake value to tell addr const     */
#define VALUE_CONSTANT     0xFFFE       /* fake value to tell value const    */

#define TFLD_UINT   0x7C
#define TFLD_INT    0x7D
#define TFLD_CHAR   0x6F

#define FLDID_VOID       0x81

/*****************************************************************************/
/* Macros for type record handling.                                          */
/*****************************************************************************/
#define NextTrec(p) ((Trec *) ((char *)(p) + ((Trec *)(p))->RecLen + 2))
#define PrimitiveType(t) ((t)&0x9F)

#define MD_BITS        0x60
#define TYP_BITS       0x1C
#define SZ_BITS        0x03
#define PRIMITIVE_PTR  MD_BITS

/*****************************************************************************/
/* Values come from the TD_POINTER flags byte. Note that we do not           */
/* support a 16:32 pointer. We map it away since we can't do anything        */
/* with it anyway.                                                           */
/*****************************************************************************/
#define PTR_0_16    0
#define PTR_0_32    1
#define PTR_16_16   2

/*****************************************************************************/
/*                      RECORD TYPE DEFINITIONS                              */
/*****************************************************************************/

#define T_SCALAR    0x51
#define T_BITFLD    0x5C
#define T_TYPDEF    0x5D
#define T_PROC      0x75
#define T_ENTRY     0x53
#define T_FUNCTION  0x54
#define T_ARRAY     0x78
#define T_STRUCT    0x79
#define T_PTR       0x7A
#define T_ENUM      0x7B
#define T_LIST      0x7F
#define T_NULL      0x80
#define T_SKIP      0x90
#define T_CLASS     0x40
#define T_MEMFNC    0x45
#define T_CLSMEM    0x46
#define T_BSECLS    0x41
#define T_REF       0x48
#define T_CLSDEF    0x43
#define T_FRIEND    0x42

/*****************************************************************************/
/* Primitive types;                                                         */
/*****************************************************************************/
#define TYPE_CHAR      0x80             /* 8-bit  signed.                    */
#define TYPE_SHORT     0x81             /* 16-bit signed.                    */
#define TYPE_LONG      0x82             /* 32-bit signed.                    */
#define TYPE_UCHAR     0x84             /* 8-bit  unsigned.                  */
#define TYPE_USHORT    0x85             /* 16-bit unsigned.                  */
#define TYPE_ULONG     0x86             /* 32-bit unsigned.                  */
#define TYPE_FLOAT     0x88             /* 32-bit real.                      */
#define TYPE_DOUBLE    0x89             /* 64-bit real.                      */
#define TYPE_LDOUBLE   0x8A             /* 80-bit real.                      */
#define TYPE_VOID      0x97             /* void.                             */

/*****************************************************************************/
/* Primitive near pointer 0:32 definitions.                                  */
/*****************************************************************************/
#define TYPE_PCHAR      0xA0            /* 0:32 near ptr to 8-bit  signed.   */
#define TYPE_PSHORT     0xA1            /* 0:32 near ptr to 16-bit signed.   */
#define TYPE_PLONG      0xA2            /* 0:32 near ptr to 32-bit signed.   */
#define TYPE_PUCHAR     0xA4            /* 0:32 near ptr to 8-bit  unsigned. */
#define TYPE_PUSHORT    0xA5            /* 0:32 near ptr to 16-bit unsigned. */
#define TYPE_PULONG     0xA6            /* 0:32 near ptr to 32-bit unsigned. */
#define TYPE_PFLOAT     0xA8            /* 0:32 near ptr to 32-bit real.     */
#define TYPE_PDOUBLE    0xA9            /* 0:32 near ptr to 64-bit real.     */
#define TYPE_PLDOUBLE   0xAA            /* 0:32 near ptr to 80-bit real.     */
#define TYPE_PVOID      0xB7            /* 0:32 near ptr to void.            */

/*****************************************************************************/
/* Primitive far pointer definitions.                                        */
/*****************************************************************************/
#define TYPE_FPCHAR     0xC0            /* far ptr to 8-bit  signed.         */
#define TYPE_FPSHORT    0xC1            /* far ptr to 16-bit signed.         */
#define TYPE_FPLONG     0xC2            /* far ptr to 32-bit signed.         */
#define TYPE_FPUCHAR    0xC4            /* far ptr to 8-bit  unsigned.       */
#define TYPE_FPUSHORT   0xC5            /* far ptr to 16-bit unsigned.       */
#define TYPE_FPULONG    0xC6            /* far ptr to 32-bit unsigned.       */
#define TYPE_FPFLOAT    0xC8            /* far ptr to 32-bit real.           */
#define TYPE_FPDOUBLE   0xC9            /* far ptr to 64-bit real.           */
#define TYPE_FPLDOUBLE  0xCA            /* far ptr to 80-bit real.           */
#define TYPE_FPVOID     0xD7            /* far ptr to void.                  */

/*****************************************************************************/
/* Primitive near pointer 0:16 definitions. ( Using MS huge model bits. )    */
/*****************************************************************************/
#define TYPE_N16PCHAR    0xE0           /* 0:16 near ptr to 8-bit  signed.   */
#define TYPE_N16PSHORT   0xE1           /* 0:16 near ptr to 16-bit signed.   */
#define TYPE_N16PLONG    0xE2           /* 0:16 near ptr to 32-bit signed.   */
#define TYPE_N16PUCHAR   0xE4           /* 0:16 near ptr to 8-bit  unsigned. */
#define TYPE_N16PUSHORT  0xE5           /* 0:16 near ptr to 16-bit unsigned. */
#define TYPE_N16PULONG   0xE6           /* 0:16 near ptr to 32-bit unsigned. */
#define TYPE_N16PFLOAT   0xE8           /* 0:16 near ptr to 32-bit real.     */
#define TYPE_N16PDOUBLE  0xE9           /* 0:16 near ptr to 64-bit real.     */
#define TYPE_N16PLDOUBLE 0xEA           /* 0:16 near ptr to 80-bit real.     */
#define TYPE_N16PVOID    0xF7           /* 0:16 near ptr to void.            */

/*****************************************************************************/
/* Prefix structure of all type records.                                     */
/*****************************************************************************/
typedef struct
{
  USHORT RecLen;
  UCHAR  RecType;
} Trec;


/*****************************************************************************/
/* - type qualifier masks and mnemonics for Class Member records.            */
/*****************************************************************************/
#define CLSMEM_PRIVATE      0x00
#define CLSMEM_PROTECTED    0x01
#define CLSMEM_PUBLIC       0x02

#define SCLSMEM_PRIVATE     "Pri"
#define SCLSMEM_PROTECTED   "Pro"
#define SCLSMEM_PUBLIC      "Pub"

#define CLSMEM_STATIC       0x01
#define CLSMEM_VTABLE       0x02
#define CLSMEM_VBASE        0x04
#define CLSMEM_CONST        0x08
#define CLSMEM_VOL          0x10
#define CLSMEM_SELF         0x20

#define SCLSMEM_STATIC      "St"
#define SCLSMEM_VTABLE      "Vt"
#define SCLSMEM_VBASE       "Vb"
#define SCLSMEM_CONST       "Co"
#define SCLSMEM_VOL         "Vo"
#define SCLSMEM_SELF        "Se"

/*****************************************************************************/
/* - type qualifier and masks for friend class/function records.             */
/*****************************************************************************/
#define FRIEND_CLASS        0x01

#define SFRIEND_FUNCTION    "FriendFnc"
#define SFRIEND_CLASS       "FriendCls"
