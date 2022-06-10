/*static char *SCCSID = "@(#)unikern.h   1.00 98/03/16";*/
/***************************************************************************
*
* Module Name: UNIKERN.H
*
* Unicode Include File For KEE
*
* Copyright (c) 1998 IBM Corporation
*
****************************************************************************
*
*
*
****************************************************************************/

#define UNIKERN_INCLUDED

/*
 * UCS conversion table types
 */
#define UC_CLASS_SBCS      1
#define UC_CLASS_DBCS      2
#define UC_CLASS_UCS2      5
#define UC_CLASS_UTF8      6
#define UC_CLASS_UPF8      7
#define UC_CLASS_NULL      8

#ifndef _ULS_UNICHAR_DEFINED
    typedef unsigned short UniChar;
    #define _ULS_UNICHAR_DEFINED
#endif

#ifndef _UINT_TYPES
    typedef unsigned short uint16;
    typedef unsigned long  uint32;
    #define _UINT_TYPES
#endif

/*
 * Conversion options - ORd together
 */
#define CVTTYPE_CTRL7F     0x00000001   /* Treat 0x7F as a control  */
#define CVTTYPE_CDRA       0x00000002   /* Use CDRA control mapping */
#define CVTTYPE_PATH       0x00000004   /* Treat string as a path   */
#define CVTTYPE_DBCS       0x00000008   /* Restrict to 2 bytes      */

/*
 * UconvObject - Allocated by the user at ring 0
 */
typedef struct _UconvObj /* UCO */
{
    uint16   eyecatcher;     /* Eyecatcher = 0x6f63             */
    char     class;          /* Type of conversion              */
    char     flag;           /* Entry type flag                 */
    uint16   esid;           /* Encoding scheme                 */
    uint16   codepage;       /* Codepage number                 */
    uint32   displaymask;    /* Display mask                    */
    uint32   converttype;    /* Conversion options              */
    uint16   subchar;        /* Substitution char length        */
    UniChar  subuni;         /* Substitution char in UCS        */
    void *   table;          /* Conversion table (GDT)          */
    void *   starter;        /* Starter table (GDT)             */
    char *   linear;         /* Linear address of table         */
} UconvObj, * PUconvObj;
#define CONVERTEYE  0x6f63

/*
 * Return codes
 */
#define ULS_ILLEGALSEQUENCE  2
#define ULS_INVALID         14
#define ULS_BADOBJECT       15
#define ULS_BUFFERFULL      18

/*
 * Function prototypes
 */

/*

 MOVED to BSEKEE.H

int KernCreateUconvObject ( short codepage, UconvObj * uhand );
int KernStrFromUcs (PUconvObj co, char * target, UniChar * source,
                     long len, long fromlen);
int KernStrToUcs(PUconvObj co, UniChar * outp, char * inp,
                  long size, long fromlen);
*/

