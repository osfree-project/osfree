/*
*
*   Copyright (c) International Business Machines  Corp., 2000
*
*   This program is free software;  you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY;  without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
*   the GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program;  if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* Module: Bad_Block_Relocation_Feature.h
*/

/*
* Change History:
*
*/

/*
* Description:  This module defines basic types used in the declaration of
*               disk structures.
*
*/

#ifndef LVM_TYPES_H_INCLUDED

#define LVM_TYPES_H_INCLUDED 1

#include "lvm_gbls.h"       /* INTEGER16, INTEGER32, INTEGER, CARDINAL16, CARDINAL32,
CARDINAL, REAL32, REAL64, BYTE, BOOLEAN, TRUE, FALSE, ADDRESS */

#if defined(__IBMCPP__) || defined(__IBMC__)
#else
#define _System
#endif

#include "lvm_list.h"

/* The following types are used in the declaration of disk structures.  Disk structures
have a defined size and internal structure which must be matched exactly.              */

/* 8 bytes. */
#ifdef LONG_LONG_SUPPORTED

typedef long long QuadWord;

#else

typedef struct _QuadWord {
unsigned long  High32Bits;
unsigned long  Low32Bits;
} QuadWord;

#endif

/* 4 bytes */
typedef unsigned long DoubleWord;

/* 2 bytes */
typedef short unsigned int Word;

/* 1 byte */
typedef unsigned char Byte;

/* The following types are used internally by LVM.  */

/* Define a Partition Sector Number.  A Partition Sector Number is relative to the start of a partition.
The first sector in a partition is PSN 0. */
typedef unsigned long PSN;

/* Define a Logical Sector Number.  A Logical Sector Number is relative to the start of a volume.
The first sector in a volume is LSN 0. */
typedef unsigned long LSN;

/* Define a Logical Block Address.  A Logical Block Address is relative to the start of a
physical device - a disk drive.  The first sector on a disk drive is LBA 0. */
typedef unsigned long LBA;

/* XLATOFF */

/* The following define sets the maximum number of LVM classes for which structures and storage will be reserved. */
#define MAXIMUM_LVM_CLASSES   3

/* The following enum defines the various LVM classes to which a "feature" may belong.
An LVM Plugin is used to implement a "feature", so "plugin" and "feature" are really synonyms. */
typedef enum _LVM_Classes {
Partition_Class,           /* For "features" which must operate on a partition level - i.e. Bad Block Relocation. */
Aggregate_Class,           /* For "features" which combine partitions into a single logical entity - i.e. Drive Linking. */
Volume_Class              /* For "features" which operate best on a volume level - i.e. encryption, mirroring etc. */
} LVM_Classes;

/* An LVM plugin may belong to one or more classes.  For each class to which it belongs, certain attributes must be defined.
This structure tracks those attributes for a class.                                                                        */
typedef struct _LVM_Class_Attributes{
BOOLEAN      ClassMember;      /* TRUE if a member of this class, FALSE otherwise. */
BOOLEAN      GlobalExclusive;  /* TRUE if this plugin can not work with any other plugin - i.e. it
must be the only "feature" on the volume, besides the built in feature of BBR. */
BOOLEAN      TopExclusive;     /* TRUE if this plugin must be the topmost plugin in this class. */
BOOLEAN      BottomExclusive;  /* TRUE if this plugin must be the bottommost plugin in this class. */
BOOLEAN      ClassExclusive;   /* TRUE if this plugin will not work with any other plugin in this class. */
CARDINAL32   Weight_Factor;    /* A value between 1 and 100 which is used to guide the LVM interfaces when attempting to
establish a default ordering for plugins within this class.  A value of 1
indicates that this plugin wants to be as close to the bottom of the plugins
in this class as possible.  A value of 100 means that this plugin wants to
be as close to being the topmost plugin in this class as possible.  This value
is only used if none of the "exclusive" flags are set.                                    */
} LVM_Class_Attributes;

/* The following enum specifies the interface types that LVM supports, and hence any plugin must support. */
typedef enum _LVM_Interface_Types{
PM_Interface,
VIO_Interface,           /* LVM.EXE is a VIO app. since it is used during install, and during recovery scenarios where PM/Java may not be available. */
Java_Interface          /* The LVM GUI is written in Java. */
} LVM_Interface_Types;

#define MAXIMUM_LVM_INTERFACE_TYPES  3

/* The following structures define what functions must be supported for each interface type. */
typedef struct _LVM_OS2_Native_Support{
void (* _System Create_and_Configure) ( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code);    /* Native OS/2 */
void (* _System Display_Status) ( ADDRESS Volume_Handle, CARDINAL32 * Error_Code );              /* Native OS/2 */
void (* _System Control_Panel) (ADDRESS Volume_Handle, CARDINAL32 * Error_Code );                /* Native OS/2 */
void (* _System Help_Panel) (CARDINAL32 Help_Index, CARDINAL32 * Error_Code);                    /* Native OS/2 */
} LVM_OS2_Native_Support;

typedef struct _LVM_Interface_Support {
BOOLEAN  Interface_Supported;
union {
char *                   Java_Interface_Class;        /* Java */
LVM_OS2_Native_Support   VIO_PM_Calls;                      /* VIO/PM */
};
} LVM_Interface_Support;

/* The following define the default help indicies which must be supported by the Help_Panel function.  NOTE:  Index
values from 0 to 100 are reserved by LVM.  The Plugin may, for its own use, use any values above 100.               */
#define HELP_PLUGIN_DESCRIPTION    0

/* The following define the maximum length of the names which can be used to represent a feature in LVM.  The
maximum name length for a feature is 30 characters plus the trailing NULL character.  For command line parsing,
though, a shorter name is preferable!  Thus, the "short" name for a feature will be limited to 10 characters
plus the trailing NULL character.  The "short" name will be used for command line purposes, while the regular
name will be used by all other interfaces.                                                                        */
#define MAX_FEATURE_NAME_LENGTH        31
#define MAX_FEATURE_SHORT_NAME_LENGTH  11
#define MAX_OEM_INFO_LENGTH           255

/* The following definitions are used to control and access the various "features" available through the LVM Engine, such as Drive Linking and BBR. */
typedef struct _Feature_ID_Data {
char         Name[MAX_FEATURE_NAME_LENGTH];             /* Feature Name, for use in menus and command line parsing. */
char         Short_Name[MAX_FEATURE_SHORT_NAME_LENGTH]; /* The name/code used to represent this feature during command line parsing. */
char         OEM_Info[MAX_OEM_INFO_LENGTH];             /* Name and copyright info. of the manufacturer, i.e. IBM, Vinca, etc. */
CARDINAL32   ID;                                        /* Numeric Feature ID. */
CARDINAL32   Major_Version_Number;                      /* The version number of this feature. */
CARDINAL32   Minor_Version_Number;                      /* The version number of this feature. */
CARDINAL32   LVM_Major_Version_Number;                  /* The version of LVM that this feature was designed to work with. */
CARDINAL32   LVM_Minor_Version_Number;                  /* The version of LVM that this feature was designed to work with. */
LVM_Classes  Preferred_Class;                           /* The class from which this "feature" prefers to be chosen.  Encryption can be performed
at the partition level or the volume level, and may therefore belong to both the
Partition_Class and the Volume_Class.  However, it is preferrable for it to be used
on the volume level instead of at the partition level.  Thus, its perferred class would
be the Volume_Class, but it would still be a member of both the Volume_Class and the
Partition_Class.                                                                              */
LVM_Class_Attributes   ClassData[MAXIMUM_LVM_CLASSES];  /* The attributes for each of the LVM classes that this "feature" is in. */
LVM_Interface_Support  Interface_Support[MAXIMUM_LVM_INTERFACE_TYPES];  /* The functions and classes for each of the video modes that LVM can run it. */
} Feature_ID_Data;

/* The following defines the TAG value used to identify an item of type Feature_ID_Data in a DLIST. */
#define FEATURE_ID_DATA_TAG 354385972

/* The following definitions are used for command line processing.  As the command line is processed,
the command line is first broken up into tokens.  Each token has a "characterization", which indicates
what the token is thought to be.                                                                        */

typedef enum {
LVM_AcceptableCharsStr,
LVM_All,
LVM_BestFit,
LVM_BootDOS,
LVM_BootOS2,
LVM_Bootable,
LVM_CR,
LVM_CRI,
LVM_Compatibility,
LVM_Drive,
LVM_Existing,
LVM_Expand,
LVM_FS,
LVM_FirstFit,
LVM_Freespace,
LVM_FromEnd,
LVM_FromLargest,
LVM_FromSmallest,
LVM_FromStart,
LVM_LVM,
LVM_LastFit,
LVM_Logical,
LVM_New,
LVM_NoBoot,
LVM_NonBootable,
LVM_NotBootable,
LVM_Partition,
LVM_Primary,
LVM_RB,
LVM_Size,
LVM_Unusable,
LVM_Unused,
LVM_Volume,
LVM_Volumes,
LVM_Comma,
LVM_Number,
LVM_Colon,
LVM_Space,
LVM_Tab,
LVM_MultiSpace,
LVM_MultiTab,
LVM_String,
LVM_FileNameStr,
LVM_SemiColon,
LVM_Eof,
LVM_Separator,
LVM_Open_Paren,                    /* ( */
LVM_Close_Paren,                   /* ) */
LVM_Open_Bracket,                  /* [ */
LVM_Close_Bracket,                 /* ] */
LVM_Open_Brace,                    /* { */
LVM_Close_Brace,                   /* } */
LVM_EQ_Sign,                       /* = */
LVM_Bootmgr,
LVM_Create,
LVM_Delete,
LVM_DriveLetter,
LVM_File,
LVM_Hide,
LVM_Install,
LVM_NewMBR,
LVM_Query,
LVM_RediscoverPRM,
LVM_SetName,
LVM_SetStartable,
LVM_SI,
LVM_SlashSize,
LVM_StartLog
} Token_Characterizations;

typedef struct _LVM_Token{
char *                   TokenText;  /* The actual text of the token. */
Token_Characterizations  TokenType;  /* What the token is thought to be. */
CARDINAL32               Position;   /* The position of the first character of the token on the command line. */
} LVM_Token;

#define LVM_TOKEN_TAG    28387473
/* XLATON */

#endif
